#include "EmailNotification.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

// Constructor: stores the configuration and initialises libcurl globally.
EmailNotification::EmailNotification(const ConfigLoader& cfg) {
    loadConfig(cfg);
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

EmailNotification::~EmailNotification() {
    stop(TrafficState::NO_TRAFFIC);
}

void EmailNotification::loadConfig(const ConfigLoader& config) {
    // Try the new structure first, then fallback to the existing structure in src/config.yaml
    smtpUrl = config.getValue("notification:email:smtp_url", "");
    if (smtpUrl.empty()) {
        std::string server = config.getValue("Notifications:smtp_server", "smtp.gmail.com");
        std::string port = config.getValue("Notifications:smtp_port", "587");
        smtpUrl = "smtp://" + server + ":" + port;
    }

    fromAddr = config.getValue("notification:email:from_addr", config.getValue("Notifications:sender_email", ""));
    
    // For now, take the first recipient if it's a list (curl needs one for MAIL_RCPT usually unless looped)
    std::string fullToAddr = config.getValue("notification:email:to_addr", config.getValue("Notifications:receiver_email", ""));
    size_t commaPos = fullToAddr.find(',');
    toAddr = (commaPos != std::string::npos) ? fullToAddr.substr(0, commaPos) : fullToAddr;

    username = config.getValue("notification:email:username", config.getValue("Notifications:smtp_username", ""));
    password = config.getValue("notification:email:password", config.getValue("Notifications:smtp_password", ""));
    
    defaultSubject = config.getValue("notification:email:default_subject", "CrowdLens Alert");
    defaultBody = config.getValue("notification:email:default_body", "Unusual traffic detected.");

    std::string triggerState = config.getValue("notification:email:trigger_state", "STAMPEDE");
    if (triggerState == "TRAFFIC") indicationState = TrafficState::TRAFFIC;
    else if (triggerState == "CROWDED") indicationState = TrafficState::CROWDED;
    else if (triggerState == "STAMPEDE") indicationState = TrafficState::STAMPEDE;
    else indicationState = TrafficState::NO_TRAFFIC;
}

void EmailNotification::run(TrafficState state) {
    // Only handle the event if the propagated state matches the expected state for action
    if (indicationState == state) {
        if (runState != RunState::RUNNING) {
            traffic_state = state;
            runState = RunState::RUNNING;
            workerThread = std::thread(&EmailNotification::worker, this);
        }
    } else {
        // Stopping the notification if the state no longer matches
        stop(state);
    }
}

void EmailNotification::worker() {
    std::cout << "[EmailNotification] Triggered by state update. Sending email..." << std::endl;
    
    // We send the alert once when triggered
    bool success = sendAlert(defaultSubject, defaultBody);
    
    if (success) {
        std::cout << "[EmailNotification] Alert sent successfully." << std::endl;
    } else {
        std::cerr << "[EmailNotification] Failed to send alert." << std::endl;
    }

    // After sending once, we can stop the worker or wait to be stopped.
    // Setting runState to STOPPED allows the thread to be joined by stop() or finished.
    runState = RunState::STOPPED;
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
        "To: "      + toAddr   + "\r\n",
        "From: "    + fromAddr + "\r\n",
        "Subject: " + subject  + "\r\n",
        "\r\n",
        body + "\r\n"
    };

    struct curl_slist* recipients = nullptr;
    CURLcode res = CURLE_OK;

    // Set SMTP authentication credentials
    curl_easy_setopt(curl, CURLOPT_USERNAME,    username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD,    password.c_str());
    
    // Set the SMTP server URL
    curl_easy_setopt(curl, CURLOPT_URL,         smtpUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL,     (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM,   ("<" + fromAddr + ">").c_str());

    // Add recipient to the SMTP envelope
    recipients = curl_slist_append(recipients, ("<" + toAddr + ">").c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT,   recipients);

    // Provide callback function to supply the email payload
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, EmailNotification::payloadSource);
    curl_easy_setopt(curl, CURLOPT_READDATA,    &ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD,      1L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "[EmailNotification] Send failed: " << curl_easy_strerror(res) << std::endl;
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