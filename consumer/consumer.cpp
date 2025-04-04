#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int server_fd, new_socket;
    sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    std::cout << "Waiting for connection...\n";

    new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    std::ofstream out("received_video.mp4", std::ios::binary);

    char buffer[1024] = {0};
    int bytesRead;
    while ((bytesRead = read(new_socket, buffer, 1024)) > 0) {
        out.write(buffer, bytesRead);
    }

    std::cout << "File received.\n";
    out.close();
    close(new_socket);
    close(server_fd);
    return 0;
}
