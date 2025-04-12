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

const size_t CHUNK_SIZE = 1024 * 1024; // 1 MB

void sendFileInChunks(std::ifstream& in, int sock, size_t fileSize) {
    char* buffer = new char[CHUNK_SIZE];
    size_t totalSent = 0;

    while (totalSent < fileSize) {
        size_t toRead = std::min(CHUNK_SIZE, fileSize - totalSent);
        in.read(buffer, toRead);
        size_t bytesRead = in.gcount();
        totalSent += bytesRead;

        size_t sent = 0;
        while (sent < bytesRead) {
            ssize_t bytes = send(sock, buffer + sent, bytesRead - sent, 0);
            if (bytes < 0) {
                std::cerr << "Error sending chunk\n";
                delete[] buffer;
                return;
            }
            sent += bytes;
        }
    }

    delete[] buffer;
}

void sendVideosFromFolder(const std::string& folderPath) {
    if (!fs::exists(folderPath)) {
        std::cerr << "❌ Folder does not exist: " << folderPath << "\n";
        return;
    }

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string filePath = entry.path().string();

            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                std::cerr << "❌ Socket creation failed for: " << filePath << "\n";
                continue;
            }

            sockaddr_in serv_addr{};
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(8080);
            inet_pton(AF_INET, "172.18.0.2", &serv_addr.sin_addr); // Docker name

            if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                std::cerr << "❌ Connection failed for: " << filePath << "\n";
                close(sock);
                continue;
            }

            std::ifstream in(filePath, std::ios::binary);
            if (!in) {
                std::cerr << "❌ Failed to open file: " << filePath << "\n";
                close(sock);
                continue;
            }

            size_t fileSize = fs::file_size(entry.path());
            std::cout << "📤 Sending " << filePath << " (" << fileSize << " bytes)... ";

            sendFileInChunks(in, sock, fileSize);

            shutdown(sock, SHUT_WR);
            sleep(1);

            std::cout << "✅ Done.\n";

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
        if (!fs::exists(folder)) {
            std::cerr << "⚠️ Skipping: Folder not found - " << folder << "\n";
            continue;
        }

        threads.emplace_back([folder]() {
            try {
                sendVideosFromFolder(folder);
            } catch (const std::exception& e) {
                std::cerr << "❗ Exception in thread for " << folder << ": " << e.what() << "\n";
            } catch (...) {
                std::cerr << "❗ Unknown error in thread for " << folder << "\n";
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
