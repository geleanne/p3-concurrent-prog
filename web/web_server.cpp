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

    html << "<!DOCTYPE html><html><head><title>Uploaded Videos</title>"
         << "<link rel='stylesheet' href='/style.css'>"
         << "<style>"
         << ".video-card {"
         << "  margin-bottom: 20px;"
         << "  text-align: center;"
         << "}"
         << ".video-card video {"
         << "  width: 300px;"
         << "  border-radius: 10px;"
         << "  border: 2px solid #ccc;"
         << "  transition: transform 0.2s ease;"
         << "  background: black;"
         << "}"
         << ".video-card video:hover {"
         << "  transform: scale(1.05);"
         << "  border-color: #888;"
         << "}"
         << ".video-label {"
         << "  font-size: 14px;"
         << "  margin-top: 5px;"
         << "  color: #333;"
         << "}"
         << "</style>"
         << "</head><body>";

    html << "<h1>Producer-Consumer Media Upload</h1>";

    html << "<p class='description'>"
         << "This project is a simulation of a media upload system built in C++. "
         << "Videos are uploaded via producers and received by consumers running on separate VM. "
         << "The consumer web interface (HTML/CSS) previews the first 10 seconds of each video. "
         << "Deployment is done locally using a lightweight C++ HTTP server."
         << "</p>";

    html << "<form method='POST' action='/upload' enctype='multipart/form-data'>"
         << "<input type='file' name='video' accept='video/mp4' required />"
         << "<button type='submit'>Upload</button></form>";

    html << "<div class='video-grid'>";
    bool hasVideo = false;
    for (const auto& file : fs::directory_iterator("web/uploads")) {
        if (file.path().extension() == ".mp4") {
            hasVideo = true;
            std::string filename = file.path().filename().string();
            html << "<div class='video-card'>"
                 << "<video src='/uploads/" << filename << "#t=0,10' muted loop controls></video>"
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