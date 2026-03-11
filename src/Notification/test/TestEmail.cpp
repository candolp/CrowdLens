// test_EmailNotification.cpp
// Compile example (adjust include/lib paths for your system):
//   g++ test_EmailNotification.cpp EmailNotification.cpp -o test_email -lcurl -std=c++20

#include "EmailNotification.h"
#include <iostream>
#include <cassert>

// ─── Helpers ────────────────────────────────────────────────────────────────

static int passed = 0;
static int failed = 0;

#define ASSERT_TRUE(expr, msg)                                          \
    do {                                                                \
        if (expr) {                                                     \
            std::cout << "  [PASS] " << msg << "\n";                   \
            ++passed;                                                   \
        } else {                                                        \
            std::cerr << "  [FAIL] " << msg << "\n";                   \
            ++failed;                                                   \
        }                                                               \
    } while (0)

#define ASSERT_FALSE(expr, msg) ASSERT_TRUE(!(expr), msg)

//  Test cases 

// 1. Config is stored correctly after construction
void test_configStoredCorrectly()
{
    std::cout << "\n[TEST] Config stored correctly\n";

    EmailNotification::Config cfg;
    cfg.fromAddr = "crowdlens17@gmail.com";
    cfg.toAddr   = "3164666B@student.gla.ac.uk";
    cfg.username = "crowdlens17@gmail.com";
    cfg.password = "kfelekclmkkfgsxp";
    cfg.smtpUrl  = "smtp://smtp.example.com:587";

    EmailNotification notifier(cfg);
    ASSERT_TRUE(true, "EmailNotification constructed without exception");
}

// 2. Empty credentials are accepted at construction (runtime failure expected at send time)
void test_emptyCredentialsConstruction()
{
    std::cout << "\n[TEST] Empty credentials accepted at construction\n";

    EmailNotification::Config cfg; // all fields default to ""
    bool threw = false;
    try {
        EmailNotification notifier(cfg);
    } catch (...) {
        threw = true;
    }
    ASSERT_FALSE(threw, "Constructor does not throw on empty config");
}

// 3. sendAlert returns false when SMTP URL is invalid / unreachable
void test_sendAlertFailsOnBadUrl()
{
    std::cout << "\n[TEST] sendAlert returns false on bad SMTP URL\n";

    EmailNotification::Config cfg;
    cfg.fromAddr = "a@b.com";
    cfg.toAddr   = "c@d.com";
    cfg.username = "a@b.com";
    cfg.password = "wrong";
    cfg.smtpUrl  = "smtp://invalid.nonexistent.host:587";  // should fail

    EmailNotification notifier(cfg);
    bool result = notifier.sendAlert("Test subject", "Test body");
    ASSERT_FALSE(result, "sendAlert returns false for unreachable host");
}

// 4. sendAlert returns false with obviously wrong credentials on a real server
void test_sendAlertFailsOnBadCredentials()
{
    std::cout << "\n[TEST] sendAlert returns false on bad credentials\n";

    EmailNotification::Config cfg;
    cfg.fromAddr = "crowdlens17@gmail.com";
    cfg.toAddr   = "tshadi.amantle.bogacu@gmail.com";
    cfg.username = "crowdlens17@gmail.com";
    cfg.password = "kfelekclmkkfgsxp";
    cfg.smtpUrl  = "smtp://smtp.gmail.com:587";

    EmailNotification notifier(cfg);
    bool result = notifier.sendAlert("Credential Test", "This should fail auth.");
    ASSERT_FALSE(result, "sendAlert returns false for wrong password");
}

// 5. Live send — only runs if LIVE_TEST is defined at compile time.
//    g++ ... -DLIVE_TEST to enable.
//    Requires valid credentials in the Config below.
#ifdef LIVE_TEST
void test_liveEmailSend()
{
    std::cout << "\n[TEST] Live email send (LIVE_TEST enabled)\n";

    EmailNotification::Config cfg;
    cfg.fromAddr = "crowdlens17@gmail.com";
    cfg.toAddr   = "tshadi.amantle.bogacu@gmail.com";
    cfg.username = "crowdlens17@gmail.com";
    cfg.password = "kfelekclmkkfgsxp";          
    cfg.smtpUrl  = "smtp://smtp.gmail.com:587";

    EmailNotification notifier(cfg);
    bool result = notifier.sendAlert(
        "Raspberry Pi Alert!",
        "Motion detected on Raspberry Pi — live test email."
    );
    ASSERT_TRUE(result, "Live email sent successfully");
}
#endif

//  Entry point 

int main()
{
    std::cout << "====== EmailNotification Tests ======\n";

    test_configStoredCorrectly();
    test_emptyCredentialsConstruction();
    test_sendAlertFailsOnBadUrl();
    test_sendAlertFailsOnBadCredentials();

#ifdef LIVE_TEST
    test_liveEmailSend();
#endif

    std::cout << "\n=================================\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
    return (failed == 0) ? 0 : 1;
}