#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    int server_fd, new_socket;
    sockaddr_in address;
    int addrlen = sizeof(address);

    // Ensure upload directory exists
    fs::create_directories("web/uploads");

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Accept from any IP
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        return 1;
    }

    std::cout << "✅ Waiting for connection on port 8080...\n";

    // Loop to accept multiple uploads
    while (true) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        std::string filename = "web/uploads/video_" + std::to_string(rand()) + ".mp4";
        std::ofstream out(filename, std::ios::binary);

        char buffer[1024];
        int bytesRead;
        while ((bytesRead = read(new_socket, buffer, 1024)) > 0) {
            out.write(buffer, bytesRead);
        }

        std::cout << "📥 File received and saved to " << filename << "\n";

        out.close();
        close(new_socket);
    }

    close(server_fd);
    return 0;
}
