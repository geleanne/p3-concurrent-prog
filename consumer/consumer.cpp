#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <filesystem>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <vector>
#include <chrono>

namespace fs = std::filesystem;

const int PORT = 8080;
const int MAX_QUEUE_SIZE = 5;

std::mutex queue_mutex;
std::condition_variable queue_cv;
std::queue<int> socket_queue;
bool running = true;

void handle_client(int client_socket) {
    auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    std::string filename = "web/uploads/video_" + std::to_string(timestamp) + ".mp4";
    std::ofstream out(filename, std::ios::binary);

    if (!out.is_open()) {
        std::cerr << "Failed to create file: " << filename << "\n";
        close(client_socket);
        return;
    }

    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        out.write(buffer, bytesRead);
    }

    std::cout << "File received and saved to " << filename << "\n";
    out.close();
    close(client_socket);
}

void consumer_thread() {
    while (running) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cv.wait(lock, [] { return !socket_queue.empty() || !running; });

        if (!running && socket_queue.empty()) return;

        int client_socket = socket_queue.front();
        socket_queue.pop();
        lock.unlock();

        handle_client(client_socket);
    }
}

int main() {
    fs::create_directories("web/uploads");

    int server_fd;
    sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "Waiting for connections on port " << PORT << "...\n";

    const int num_threads = 4;
    std::vector<std::thread> consumers;
    for (int i = 0; i < num_threads; ++i) {
        consumers.emplace_back(consumer_thread);
    }

    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        std::unique_lock<std::mutex> lock(queue_mutex);
        if (socket_queue.size() >= MAX_QUEUE_SIZE) {
            std::cerr << "Queue full, dropping connection (leaky bucket)\n";
            close(new_socket);
        } else {
            socket_queue.push(new_socket);
            queue_cv.notify_one();
        }
    }

    running = false;
    queue_cv.notify_all();
    for (auto& t : consumers) {
        t.join();
    }

    close(server_fd);
    return 0;
}