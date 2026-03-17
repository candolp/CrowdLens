#include "EmailNotification.h"
#include <iostream>
#include <cstring>

// Constructor: stores the configuration and initialises libcurl globally.
EmailNotification::EmailNotification(const Config& cfg) : config(cfg) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

// Destructor: release global libcurl resources.
EmailNotification::~EmailNotification() {
    curl_global_cleanup();
}

bool EmailNotification::sendAlert(const std::string& subject, const std::string& body) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[EmailNotification] Failed to initialise curl handle\n";
        return false;
    }

    // Build the RFC 2822 payload as a single contiguous string.
    UploadContext ctx;
    ctx.payload =
        "To: "      + config.toAddr   + "\r\n"
        "From: "    + config.fromAddr + "\r\n"
        "Subject: " + subject         + "\r\n"
        "\r\n"
        + body + "\r\n";

    // Build MAIL FROM / RCPT TO addresses once and store in std::string so the
    // C-string pointer remains valid for the lifetime of the curl handle.
    const std::string mailFrom = "<" + config.fromAddr + ">";
    const std::string mailTo   = "<" + config.toAddr   + ">";

    struct curl_slist* recipients = nullptr;
    CURLcode res = CURLE_OK;

    // Authentication
    curl_easy_setopt(curl, CURLOPT_USERNAME, config.username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, config.password.c_str());

    // SMTP server
    curl_easy_setopt(curl, CURLOPT_URL,      config.smtpUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL,  (long)CURLUSESSL_ALL);

    // Envelope sender/recipient
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mailFrom.c_str());

    recipients = curl_slist_append(recipients, mailTo.c_str());
    if (!recipients) {
        std::cerr << "[EmailNotification] Failed to build recipient list\n";
        curl_easy_cleanup(curl);
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    // Payload callback
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, EmailNotification::payloadSource);
    curl_easy_setopt(curl, CURLOPT_READDATA,     &ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD,       1L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "[EmailNotification] Send failed: " << curl_easy_strerror(res) << "\n";
    } else {
        std::cout << "[EmailNotification] Alert sent successfully.\n";
    }

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK);
}

// Callback used by libcurl to stream the email payload into its send buffer.
size_t EmailNotification::payloadSource(void* ptr, size_t size, size_t nmemb, void* userp) {
    auto* ctx = static_cast<UploadContext*>(userp);

    const size_t bufSize   = size * nmemb;
    const size_t remaining = ctx->payload.size() - ctx->offset;

    if (bufSize == 0 || remaining == 0)
        return 0;

    const size_t len = std::min(bufSize, remaining);
    std::memcpy(ptr, ctx->payload.data() + ctx->offset, len);
    ctx->offset += len;
    return len;
}