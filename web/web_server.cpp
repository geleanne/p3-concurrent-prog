#include "httplib.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <sstream>
#include <fstream>

namespace fs = std::filesystem;
using namespace httplib;

// Generate dynamic HTML with upload form and video grid
// basically acts as the index.html but makes it easier for reloading
// updated
std::string generateHTML() {
    std::ostringstream html;

    html << "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        << "<title>Uploaded Videos</title>"
        << "<link rel='stylesheet' href='/style.css'>"
        << "</head><body>";


    html << "<h1>📤 Upload a Video</h1>";
    html << "<p class='description'>Select an MP4 file below and upload it to preview.</p>";

    html << "<form method='POST' action='/upload' enctype='multipart/form-data'>"
         << "<input type='file' name='video' accept='video/mp4' required />"
         << "<button type='submit'>Upload</button></form>";

    html << "<h2>🎞 Uploaded Videos</h2>";
    html << "<div class='video-grid'>";

    bool hasVideo = false;
    for (const auto& file : fs::directory_iterator("web/uploads")) {
        if (file.path().extension() == ".mp4") {
            hasVideo = true;
            std::string filename = file.path().filename().string();
            html << "<div class='video-card'>"
                 << "<video src='/uploads/" << filename << "#t=0,10' muted loop controls></video>"
                 << "<div class='overlay'>🔍 Previewing...</div>"
                 << "<div class='video-label'>" << filename << "</div>"
                 << "</div>";
        }
    }

    if (!hasVideo) {
        html << "<p>No videos uploaded yet.</p>";
    }

    html << "</div></body></html>";
    return html.str();
}


int main() {
    httplib::Server svr;

    // Serve video uploads and style.css
    svr.set_mount_point("/uploads", "./web/uploads");
    svr.set_mount_point("/", "./web"); // serves style.css from /web

    // GET /
    svr.Get("/", [](const Request& req, Response& res) {
        res.set_content(generateHTML(), "text/html");
    });

    // POST /upload
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
        std::cerr << "Failed to bind to port 8080. Maybe it's already in use?\n";
    }
    return 0;
}