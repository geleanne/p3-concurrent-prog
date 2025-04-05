#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <filesystem>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

void handleClient(int socket_fd, int client_id) {
    std::string filename = "web/uploads/video_" + std::to_string(client_id) + ".mp4";
    std::ofstream out(filename, std::ios::binary);

    char buffer[1024];
    int bytesRead;
    while ((bytesRead = read(socket_fd, buffer, sizeof(buffer))) > 0) {
        out.write(buffer, bytesRead);
    }
    out.close();
    close(socket_fd);
    std::cout << "[Thread " << client_id << "] File saved to " << filename << "\n";
}

int main() {
    int server_fd;
    sockaddr_in address;
    int addrlen = sizeof(address);
    fs::create_directories("web/uploads"); // upload directory

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // accept from any IP
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    std::cout << "Consumer listening on port 8080...\n";

    int client_id = 0;
    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket >= 0) {
            std::thread(handleClient, new_socket, client_id++).detach();  // run in background
        } else {
            std::cerr << "Failed to accept connection.\n";
        }
    }
    close(server_fd);
    return 0;
}
