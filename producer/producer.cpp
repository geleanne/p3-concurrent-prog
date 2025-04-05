#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <thread>
#include <filesystem>

namespace fs = std::filesystem;

// Function to send all videos from a folder over TCP
void sendVideosFromFolder(const std::string& folderPath) {
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string filePath = entry.path().string();

            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                std::cerr << "Socket creation failed for: " << filePath << "\n";
                continue;
            }

            sockaddr_in serv_addr{};
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(8080);

            // TODO: if we use vm, cange this to the consumer VM IP 
            inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); 

            if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                std::cerr << "Connection failed for: " << filePath << "\n";
                close(sock);
                continue;
            }
            std::ifstream in(filePath, std::ios::binary);
            if (!in) {
                std::cerr << "Failed to open file: " << filePath << "\n";
                close(sock);
                continue;
            }

            char buffer[1024];
            while (!in.eof()) {
                in.read(buffer, sizeof(buffer));
                send(sock, buffer, in.gcount(), 0);
            }

            // try signal end of transmission
            shutdown(sock, SHUT_WR);
            sleep(1); // try allow time for consumer to finish reading

            std::cout << "Sent: " << filePath << std::endl;
            in.close();
            close(sock);
        }
    }
}

int main() {
    int numProducers;
    std::cout << "Enter number of producer threads: ";
    std::cin >> numProducers;

    std::vector<std::thread> threads;

    for (int i = 0; i < numProducers; ++i) {
        std::string folder = "shared/videos" + std::to_string(i + 1);
        threads.emplace_back([folder]() {
            sendVideosFromFolder(folder);
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
