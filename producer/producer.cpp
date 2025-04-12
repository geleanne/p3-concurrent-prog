#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <thread>
#include <filesystem>
#include <unordered_set>
#include <fstream>

namespace fs = std::filesystem;

const size_t CHUNK_SIZE = 1024 * 1024; // 1 MB
const char* CONSUMER_IP = "172.18.0.2"; // Replace with your consumer container IP
const int RETRY_LIMIT = 3;

std::unordered_set<std::string> sentFiles;
const std::string sentFileLog = "shared/sent_files.txt";

// Load previously sent filenames
void loadSentFiles() {
    std::ifstream file(sentFileLog);
    std::string name;
    while (getline(file, name)) {
        sentFiles.insert(name);
    }
}

// Append filename to log
void logSentFile(const std::string& name) {
    std::ofstream file(sentFileLog, std::ios::app);
    file << name << "\n";
}

// Send file in chunks
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
                std::cerr << "[ERROR] Chunk send failed\n";
                delete[] buffer;
                return;
            }
            sent += bytes;
        }
    }
    delete[] buffer;
}

// Check if queue is full
bool shouldSkipDueToQueue(int sock) {
    char response[64] = {0};
    recv(sock, response, sizeof(response), MSG_DONTWAIT);
    return std::string(response).find("QUEUE_FULL") != std::string::npos;
}

// Send videos in folder
void sendVideosFromFolder(const std::string& folderPath) {
    if (!fs::exists(folderPath)) {
        std::cerr << "[WARN] Folder does not exist: " << folderPath << "\n";
        return;
    }

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".mp4") continue;

        std::string filePath = entry.path().string();
        std::string filename = entry.path().filename().string();

        if (sentFiles.count(filename)) {
            std::cout << "[SKIP] Duplicate file: " << filePath << "\n";
            continue;
        }

        int attempts = 0;
        while (attempts < RETRY_LIMIT) {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                std::cerr << "[ERROR] Socket creation failed: " << filePath << "\n";
                break;
            }

            sockaddr_in serv_addr{};
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(8080);
            inet_pton(AF_INET, CONSUMER_IP, &serv_addr.sin_addr);

            if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                std::cerr << "[ERROR] Connection failed: " << filePath << "\n";
                close(sock);
                break;
            }

            if (shouldSkipDueToQueue(sock)) {
                std::cerr << "[WARN] Queue full for: " << filePath << " (Retrying...)\n";
                close(sock);
                ++attempts;
                sleep(1);
                continue;
            }

            std::ifstream in(filePath, std::ios::binary);
            if (!in) {
                std::cerr << "[ERROR] Failed to open: " << filePath << "\n";
                close(sock);
                break;
            }

            size_t fileSize = fs::file_size(filePath);
            std::cout << "[INFO] Sending: " << filePath << " (" << fileSize << " bytes)... ";

            sendFileInChunks(in, sock, fileSize);
            shutdown(sock, SHUT_WR);
            sleep(1);

            std::cout << "✅ Done.\n";
            in.close();
            close(sock);
            sentFiles.insert(filename);
            logSentFile(filename);
            break;
        }
    }
}

int main() {
    loadSentFiles();
    int numProducers;
    std::cout << "[INPUT] Enter number of producer threads (p): ";
    std::cin >> numProducers;

    std::vector<std::thread> threads;
    for (int i = 0; i < numProducers; ++i) {
        std::string folder = "shared/videos" + std::to_string(i + 1);
        if (!fs::exists(folder)) {
            std::cerr << "[WARN] Skipping: Folder not found - " << folder << "\n";
            continue;
        }

        threads.emplace_back([folder]() {
            try {
                sendVideosFromFolder(folder);
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Thread exception: " << folder << " - " << e.what() << "\n";
            } catch (...) {
                std::cerr << "[ERROR] Unknown error in thread: " << folder << "\n";
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
