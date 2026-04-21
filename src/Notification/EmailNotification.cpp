//
// Created by Amantle on 20/03/2026.
//

#include "EmailNotification.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string trimStr(const std::string& s)
{
    const std::string ws = " \t\r\n";
    const size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return {};
    const size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

static std::vector<std::string> splitCSV(const std::string& s)
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, ','))
    {
        std::string trimmed = trimStr(token);
        if (!trimmed.empty())
            result.push_back(trimmed);
    }
    return result;
}

// ---------------------------------------------------------------------------
// Constructors  (mirroring LEDOutput constructor signatures exactly)
// ---------------------------------------------------------------------------

EmailNotification::EmailNotification()
{
    throw std::runtime_error("EmailNotification requires configuration or direct config struct");
}

EmailNotification::EmailNotification(const ConfigLoader& config, const TrafficState& indicationState)
{
    EmailNotification::loadConfig(config);
    available_        = true;
    _indicationState  = indicationState;
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

EmailNotification::EmailNotification(const ConfigLoader& config, bool skipInit, const TrafficState& indicationState)
{
    EmailNotification::loadConfig(config);
    if (!skipInit)
    {
        available_       = true;
        _indicationState = indicationState;
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
}

EmailNotification::EmailNotification(const Config& directConfig, const TrafficState& indicationState)
{
    config_           = directConfig;
    available_        = true;
    _indicationState  = indicationState;
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

EmailNotification::~EmailNotification()
{
    curl_global_cleanup();
}

// ---------------------------------------------------------------------------
// loadConfig  (mirrors LEDOutput::loadConfig)
// ---------------------------------------------------------------------------

void EmailNotification::loadConfig(const ConfigLoader& config)
{
    config_.smtpServer = config.getValue("Notifications:smtp_server", "smtp.gmail.com");
    config_.smtpPort   = std::stoi(config.getValue("Notifications:smtp_port", "587"));
    config_.fromAddr   = config.getValue("Notifications:sender_email",   "");
    config_.toAddr     = config.getValue("Notifications:receiver_email", "");
    config_.username   = config.getValue("Notifications:smtp_username",  "");
    config_.password   = config.getValue("Notifications:smtp_password",  "");
}

// ---------------------------------------------------------------------------
// run() 
// ---------------------------------------------------------------------------

void EmailNotification::run(TrafficState state)
{
    // Only handle the event if the propagated state matches the expected state for action
    if (_indicationState == state)
    {
        traffic_state = state;
        runState      = RunState::RUNNING;
        workerThread  = std::thread(&EmailNotification::worker, this);
    }
    else
    {
        // Stop this notifier because its indication state is no longer active
        stop(state);
    }
}

// ---------------------------------------------------------------------------
// worker()  (runs once in background thread — email is one-shot, unlike LED loop)
// ---------------------------------------------------------------------------

void EmailNotification::worker()
{
    try
    {
        if (traffic_state == TrafficState::CROWDED)
        {
            const std::string subject =
                "[CrowdLens WARNING] Elevated crowd density detected";

            const std::string body =
                "Dear Team,\r\n\r\n"
                "CrowdLens has detected an elevated crowd density level.\r\n"
                "Please monitor the situation and consider deploying crowd-management "
                "measures to prevent further escalation.\r\n\r\n"
                "This is an automated warning from the CrowdLens monitoring system.\r\n\r\n"
                "Regards,\r\nCrowdLens";

            sendAlert(subject, body);
        }
        else if (traffic_state == TrafficState::STAMPEDE)
        {
            const std::string subject =
                "[CrowdLens CRITICAL] Stampede risk detected - immediate action required";

            const std::string body =
                "Dear Team,\r\n\r\n"
                "CRITICAL ALERT: CrowdLens has detected conditions consistent with "
                "a potential stampede (STAMPEDE state).\r\n\r\n"
                "IMMEDIATE action is required:\r\n"
                "  - Activate emergency protocols\r\n"
                "  - Evacuate the affected area\r\n"
                "  - Contact emergency services if necessary\r\n\r\n"
                "This is an automated critical alert from the CrowdLens monitoring system.\r\n\r\n"
                "Regards,\r\nCrowdLens";

            sendAlert(subject, body);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "[EmailNotification] worker exception: " << e.what() << "\n";
    }

    runState = RunState::STOPPED;
}

// ---------------------------------------------------------------------------
// stop() 
// ---------------------------------------------------------------------------

void EmailNotification::stop(TrafficState ts)
{
    runState = RunState::STOPPED;

    if (workerThread.joinable())
        workerThread.join();
}

// ---------------------------------------------------------------------------
// sendAlert — dispatches to every recipient
// ---------------------------------------------------------------------------

bool EmailNotification::sendAlert(const std::string& subject, const std::string& body)
{
    const auto recipients = splitCSV(config_.toAddr);
    if (recipients.empty())
    {
        std::cerr << "[EmailNotification] No recipients configured.\n";
        return false;
    }

    bool allOk = true;
    for (const auto& recipient : recipients)
    {
        if (!sendToOne(recipient, subject, body))
            allOk = false;
    }
    return allOk;
}

// ---------------------------------------------------------------------------
// sendToOne — libcurl SMTP send to a single address
// ---------------------------------------------------------------------------

bool EmailNotification::sendToOne(const std::string& recipient,
                                  const std::string& subject,
                                  const std::string& body)
{
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "[EmailNotification] Failed to initialise curl handle\n";
        return false;
    }

    const std::string smtpUrl =
        "smtp://" + config_.smtpServer + ":" + std::to_string(config_.smtpPort);

    UploadContext ctx;
    ctx.payload =
        "To: "      + recipient        + "\r\n"
        "From: "    + config_.fromAddr + "\r\n"
        "Subject: " + subject          + "\r\n"
        "\r\n"
        + body + "\r\n";

    const std::string mailFrom = "<" + config_.fromAddr + ">";
    const std::string mailTo   = "<" + recipient        + ">";

    struct curl_slist* rcptList = nullptr;
    CURLcode res = CURLE_OK;

    curl_easy_setopt(curl, CURLOPT_USERNAME,  config_.username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD,  config_.password.c_str());
    curl_easy_setopt(curl, CURLOPT_URL,       smtpUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL,   (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mailFrom.c_str());

    rcptList = curl_slist_append(rcptList, mailTo.c_str());
    if (!rcptList)
    {
        std::cerr << "[EmailNotification] Failed to build recipient list\n";
        curl_easy_cleanup(curl);
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT,    rcptList);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, EmailNotification::payloadSource);
    curl_easy_setopt(curl, CURLOPT_READDATA,     &ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD,       1L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
        std::cerr << "[EmailNotification] Send to <" << recipient
                  << "> failed: " << curl_easy_strerror(res) << "\n";
    else
        std::cout << "[EmailNotification] Alert sent to <" << recipient << ">.\n";

    curl_slist_free_all(rcptList);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK);
}

// ---------------------------------------------------------------------------
// libcurl read callback
// ---------------------------------------------------------------------------

size_t EmailNotification::payloadSource(void* ptr, size_t size, size_t nmemb, void* userp)
{
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