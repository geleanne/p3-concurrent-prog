#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <filesystem>
#include <thread>
#include <vector>
#include <chrono>
#include "../shared/queue.h" 

namespace fs = std::filesystem;

const int PORT = 8080;

ThreadSafeQueue<int>* socket_queue;
bool running = true;

// Handles each incoming client connection ( -->saves video to uploads/)
void handle_client(int client_socket) {
    auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    std::string filename = "web/uploads/video_" + std::to_string(timestamp) + ".mp4";
    std::ofstream out(filename, std::ios::binary);

    if (!out.is_open()) {
        std::cerr << "[ERROR] Failed to create file: " << filename << "\n";
        close(client_socket);
        return;
    }

    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        out.write(buffer, bytesRead);
    }

    std::cout << "[INFO] File received and saved to " << filename << "\n";
    out.close();
    close(client_socket);
}

// Thread function: pulls sockets from the queue and handles them
void consumer_thread() {
    while (running) {
        int client_socket = socket_queue->dequeue();  // Block until available
        handle_client(client_socket);
    }
}

int main() {
    fs::create_directories("web/uploads");

    int c, q;
    std::cout << "[INPUT] Enter number of consumer threads (c): ";
    std::cin >> c;
    std::cout << "[INPUT] Enter max queue size (q): ";
    std::cin >> q;

    socket_queue = new ThreadSafeQueue<int>(q);  // Initialize queue with size q

    // Setup socket server
    int server_fd;
    sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("[ERROR] Socket creation failed");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("[ERROR] Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("[ERROR] Listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "[INFO] Waiting for connections on port " << PORT << "...\n";

    // Start consumer threads
    std::vector<std::thread> consumers;
    for (int i = 0; i < c; ++i) {
        consumers.emplace_back(consumer_thread);
    }

    // Main accept loop
    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket < 0) {
            perror("[WARN] Accept failed");
            continue;
        }

        if (socket_queue->isFull()) {
            const char* msg = "QUEUE_FULL";
            send(new_socket, msg, strlen(msg), 0);  // Notify producer
            std::cerr << "[WARN] Queue full, notified producer and dropped connection\n";
            close(new_socket);
        } else {
            socket_queue->enqueue(new_socket);
        }
    }

    // Cleanup (won't reached in normal execution)
    running = false;
    for (auto& t : consumers) {
        t.join();
    }

    close(server_fd);
    delete socket_queue;
    return 0;
}
