#include "httplib.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <sstream>
#include <fstream>

namespace fs = std::filesystem;
using namespace httplib;

// Generate dynamic HTML with upload form and video grid
std::string generateHTML() {
    std::ostringstream html;

    html << "<!DOCTYPE html><html><head><title>Uploaded Videos</title>"
         << "<style>"
         << "body { font-family: sans-serif; text-align: center; background: #f5f5f5; }"
         << "form { margin: 20px; }"
         << ".video-grid { display: flex; flex-wrap: wrap; gap: 15px; justify-content: center; padding: 20px; }"
         << "video { width: 300px; border-radius: 8px; border: 2px solid #ccc; }"
         << "</style></head><body>";

    html << "<h1>Uploaded Videos</h1>";

    // Upload form
    html << "<form method='POST' action='/upload' enctype='multipart/form-data'>"
         << "<input type='file' name='video' accept='video/mp4' required />"
         << "<button type='submit'>Upload</button></form>";

    // Video grid
    html << "<div class='video-grid'>";
    for (const auto& file : fs::directory_iterator("web/uploads")) {
        if (file.path().extension() == ".mp4") {
            std::string filename = file.path().filename().string();
            html << "<video src='/uploads/" << filename << "' controls muted></video>";
        }
    }
    html << "</div></body></html>";
    return html.str();
}

int main() {
    httplib::Server svr;

    // uploaded files
    svr.set_mount_point("/uploads", "./web/uploads");

    // dynamic homepage
    svr.Get("/", [](const Request& req, Response& res) {
        res.set_content(generateHTML(), "text/html");
    });

    // file uploads
    svr.Post("/upload", [](const Request& req, Response& res) {
        const auto& files = req.files;
        auto it = files.find("video");

        if (it != files.end()) {
            const auto& file = it->second;
            std::string filename = "web/uploads/" + file.filename;

            std::ofstream ofs(filename, std::ios::binary);
            ofs << file.content;
            ofs.close();

            std::cout << "✅ Uploaded: " << filename << "\n";
            res.set_redirect("/");
        } else {
            res.status = 400;
            res.set_content("No file uploaded.", "text/plain");
        }
    });

    std::cout << "Server running at http://localhost:8080\n";

    if (!svr.listen("0.0.0.0", 8080)) {
        std::cerr << "Failed to bind to port 8080. Maybe it's already in use? Try 'lsof -i :8080' then 'kill <PID>'\n";
    }
    return 0;
}