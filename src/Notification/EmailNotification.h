#pragma once

#include <string>
#include <curl/curl.h>
#include <vector>
#include <memory>
#include "../Common/TrafficEventHandler.h"
#include "../Common/ConfigLoader.h"

class EmailNotification : public TrafficEventHandler {
public:
    /**
     * Constructor for EmailNotification.
     * @param config Configuration loader to retrieve SMTP settings.
     */
    explicit EmailNotification(const ConfigLoader& config);
    ~EmailNotification() override;

    /**
     * Loads configuration settings from the provided ConfigLoader.
     * @param config Configuration loader containing SMTP and notification settings.
     */
    void loadConfig(const ConfigLoader& config);

    /**
     * Reacts to traffic state changes.
     * Starts the email sending process if the state matches the configured indicationState.
     * @param state The current traffic state.
     */
    void run(TrafficState state) override;

    /**
     * Worker thread function that performs the actual email sending.
     * Overrides the pure virtual worker() in TrafficEventHandler.
     */
    void worker() override;

    // Kept for backward compatibility or manual triggering
    bool sendAlert(const std::string& subject, const std::string& body);

private:
    std::string smtpUrl;
    std::string fromAddr;
    std::string toAddr;
    std::string username;
    std::string password;
    std::string defaultSubject;
    std::string defaultBody;
    TrafficState indicationState = TrafficState::STAMPEDE;

    // Holds the lines of the email payload during a send operation
    struct UploadContext {
        std::vector<std::string> lines;
        size_t index = 0;
    };

    static size_t payloadSource(void* ptr, size_t size, size_t nmemb, void* userp);
};