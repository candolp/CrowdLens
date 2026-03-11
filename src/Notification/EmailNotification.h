#pragma once

#include <string>
#include <curl/curl.h>
#include <vector>
class EmailNotification {
public:
    struct Config {
        std::string smtpUrl   = "smtp://smtp.gmail.com:587";
        std::string fromAddr;
        std::string toAddr;
        std::string username;
        std::string password;
    };

    explicit EmailNotification(const Config& cfg);
    ~EmailNotification() = default;

    // Send an alert with a custom subject and body
    bool sendAlert(const std::string& subject, const std::string& body);

private:
    Config config;

    // Holds the lines of the email payload during a send operation
    struct UploadContext {
        std::vector<std::string> lines;
        size_t index = 0;
    };

    static size_t payloadSource(void* ptr, size_t size, size_t nmemb, void* userp);
};