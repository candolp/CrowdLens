//
// Created by Amantle on 20/03/2026.
//

#ifndef CROWDLENS_EMAILNOTIFICATION_H
#define CROWDLENS_EMAILNOTIFICATION_H

#include <string>
#include <curl/curl.h>

#include "../Common/TrafficEventHandler.h"
#include "../Common/ConfigLoader.h"

/**
 * EmailNotification - sends alert emails based on TrafficState.
 *
 *   - Each instance is bound to one _indicationState (CROWDED or STAMPEDE).
 *   - run() starts the worker only when the propagated state matches _indicationState.
 *   - worker() sends the appropriate email once, then exits.
 *   - Two instances can be registered for CROWDED (warning) and STAMPEDE (critical).
 */
class EmailNotification : public TrafficEventHandler
{
public:

    struct Config {
        std::string smtpServer = "smtp.gmail.com";
        int         smtpPort   = 587;
        std::string fromAddr;
        std::string toAddr;      
        std::string username;
        std::string password;
    };


    EmailNotification();

    EmailNotification(const ConfigLoader& config, const TrafficState& indicationState);
    EmailNotification(const ConfigLoader& config, bool skipInit, const TrafficState& indicationState);
    EmailNotification(const Config& directConfig, const TrafficState& indicationState);

    ~EmailNotification();

    // ------------------------------------------------------------------ //
    //  TrafficEventHandler overrides  (mirrors LEDOutput public interface)
    // ------------------------------------------------------------------ //

    /**
     * Loads / reloads SMTP settings from a ConfigLoader.
     * Reads the "Notifications:" section of config.yaml.
     */
    void loadConfig(const ConfigLoader& config);

    /**
     * Called by the broadcaster with the new traffic state.
     * Starts the worker thread only when state == _indicationState;
     * otherwise calls stop().
     */
    void run(TrafficState state) override;

    void stop(TrafficState traffic_state) override;


    /**
     * Sends one email with the given subject and body to all configured recipients.
     * @return true if every recipient received the mail successfully.
     */
    bool sendAlert(const std::string& subject, const std::string& body);

protected:
    // ------------------------------------------------------------------ //
    //  Worker — runs in the background thread launched by run()
    // ------------------------------------------------------------------ //

    /**
     * Sends the appropriate email once based on traffic_state, then exits.
     */
    void worker() override;

private:
    // ------------------------------------------------------------------ //
    //  Internal state 
    // ------------------------------------------------------------------ //

    Config       config_;
    bool         available_      = false;

    /** The state for which this notifier will fire */
    TrafficState _indicationState = TrafficState::CROWDED;

    // libcurl upload context
    struct UploadContext {
        std::string payload;
        size_t      offset = 0;
    };

    static size_t payloadSource(void* ptr, size_t size, size_t nmemb, void* userp);

    bool sendToOne(const std::string& recipient,
                   const std::string& subject,
                   const std::string& body);
};

#endif //CROWDLENS_EMAILNOTIFICATION_H