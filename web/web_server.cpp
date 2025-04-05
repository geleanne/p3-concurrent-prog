#include "httplib.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <sstream>

namespace fs = std::filesystem;
using namespace httplib;

// function to generate the full HTML with video grid
// can fix this to whatever later
std::string generateHTML() {
    std::ostringstream html;

    html << "<!DOCTYPE html><html><head><title>Uploaded Videos</title>"
         << "<style>"
         << "body { font-family: sans-serif; text-align: center; background: #f5f5f5; }"
         << ".video-grid { display: flex; flex-wrap: wrap; gap: 15px; justify-content: center; padding: 20px; }"
         << "video { width: 300px; border-radius: 8px; border: 2px solid #ccc; }"
         << "</style></head><body>";

    html << "<h1>Uploaded Videos</h1><div class='video-grid'>";

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

    svr.set_mount_point("/uploads", "./web/uploads");

    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(generateHTML(), "text/html");
    });

    std::cout << "Server running at http://localhost:8080\n";

    // Catch fail
    if (!svr.listen("0.0.0.0", 8080)) {
        std::cerr << "Failed to bind to port 8080. Maybe it's already in use? Try 'lsof -i :8080' then 'kill <PID#>'\n";
    }

    return 0;
}

