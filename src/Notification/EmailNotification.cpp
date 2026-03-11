#include "EmailNotification.h"
#include <iostream>
#include <cstring>

// Constructor: stores the configuration and initialises libcurl globally.
EmailNotification::EmailNotification(const Config& cfg) : config(cfg) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

bool EmailNotification::sendAlert(const std::string& subject, const std::string& body) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[EmailNotification] Failed to initialise curl handle" << std::endl;
        return false;
    }

    // Build the RFC 2822 payload line-by-line
    UploadContext ctx;
    ctx.lines = {
        "To: "      + config.toAddr   + "\r\n",
        "From: "    + config.fromAddr + "\r\n",
        "Subject: " + subject         + "\r\n",
        "\r\n",
        body + "\r\n"
    };

    struct curl_slist* recipients = nullptr;
    CURLcode res = CURLE_OK;

    // Set SMTP authentication credentials
    curl_easy_setopt(curl, CURLOPT_USERNAME,    config.username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD,    config.password.c_str());
    
    // Set the SMTP server URL
    curl_easy_setopt(curl, CURLOPT_URL,         config.smtpUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL,     (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM,   ("<" + config.fromAddr + ">").c_str());

    // Add recipient to the SMTP envelope
    recipients = curl_slist_append(recipients, ("<" + config.toAddr + ">").c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT,   recipients);

    // Provide callback function to supply the email payload
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, EmailNotification::payloadSource);
    curl_easy_setopt(curl, CURLOPT_READDATA,    &ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD,      1L);

    res = curl_easy_perform(curl);

    // Handle log success or failure
    if (res != CURLE_OK) {
        std::cerr << "[EmailNotification] Send failed: " << curl_easy_strerror(res) << std::endl;
    } else {
        std::cout << "[EmailNotification] Alert sent successfully." << std::endl;
    }

    // Clean up allocated resources
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK);
}

// Callback used by libcurl to read the email payload.
size_t EmailNotification::payloadSource(void* ptr, size_t size, size_t nmemb, void* userp) {
    auto* ctx = static_cast<UploadContext*>(userp);

    if (size == 0 || nmemb == 0 || ctx->index >= ctx->lines.size())
        return 0;

    const std::string& line = ctx->lines[ctx->index++];
    size_t len = std::min(line.size(), size * nmemb);
    memcpy(ptr, line.data(), len);
    return len;
}