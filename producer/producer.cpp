#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); // change to VM IP

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    std::ifstream in("video.mp4", std::ios::binary);

    char buffer[1024];
    while (!in.eof()) {
        in.read(buffer, sizeof(buffer));
        send(sock, buffer, in.gcount(), 0);
    }

    std::cout << "File sent.\n";
    in.close();
    close(sock);
    return 0;
}
