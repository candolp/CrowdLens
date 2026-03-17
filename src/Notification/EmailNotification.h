#pragma once

#include <string>
#include <curl/curl.h>

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

    // Destructor calls curl_global_cleanup() to release libcurl resources.
    ~EmailNotification();

    // Send an alert email with a custom subject and body.
    // Returns true on success, false on any curl error.
    bool sendAlert(const std::string& subject, const std::string& body);

private:
    Config config;

    // Holds the full RFC-2822 payload as a single string during a send.
    struct UploadContext {
        std::string payload;
        size_t      offset = 0;
    };

    // libcurl read callback — copies payload bytes into curl's buffer.
    static size_t payloadSource(void* ptr, size_t size, size_t nmemb, void* userp);
};