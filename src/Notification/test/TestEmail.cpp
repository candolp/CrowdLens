// test_EmailNotification.cpp
// Compile example:
// g++ test_EmailNotification.cpp EmailNotification.cpp -o test_email -lcurl -std=c++20

#include "EmailNotification.h"
#include <iostream>
#include <cstdlib>

// ─── Simple Test Framework ─────────────────────────────────────────────

static int passed = 0;
static int failed = 0;

#define ASSERT_TRUE(expr, msg)                                          \
    do {                                                                \
        if (expr) {                                                     \
            std::cout << "  [PASS] " << msg << "\n";                    \
            ++passed;                                                   \
        } else {                                                        \
            std::cerr << "  [FAIL] " << msg << "\n";                    \
            ++failed;                                                   \
        }                                                               \
    } while (0)

#define ASSERT_FALSE(expr, msg) ASSERT_TRUE(!(expr), msg)

// ─── Test 1: Config stored correctly ───────────────────────────────────

void test_configStoredCorrectly()
{
    std::cout << "\n[TEST] Config stored correctly\n";

    EmailNotification::Config cfg;
    cfg.fromAddr = "example@test.com";
    cfg.toAddr   = "receiver@test.com";
    cfg.username = "user";
    cfg.password = "pass";
    cfg.smtpUrl  = "smtp://smtp.example.com:587";

    EmailNotification notifier(cfg);

    ASSERT_TRUE(true, "EmailNotification constructed without exception");
}

// ─── Test 2: Empty config should not throw ─────────────────────────────

void test_emptyCredentialsConstruction()
{
    std::cout << "\n[TEST] Empty credentials accepted at construction\n";

    EmailNotification::Config cfg;

    bool threw = false;

    try {
        EmailNotification notifier(cfg);
    }
    catch (...) {
        threw = true;
    }

    ASSERT_FALSE(threw, "Constructor does not throw on empty config");
}

// ─── Test 3: Invalid SMTP server should fail ───────────────────────────

void test_sendAlertFailsOnBadUrl()
{
    std::cout << "\n[TEST] sendAlert returns false on bad SMTP URL\n";

    EmailNotification::Config cfg;
    cfg.fromAddr = "a@b.com";
    cfg.toAddr   = "c@d.com";
    cfg.username = "user";
    cfg.password = "pass";
    cfg.smtpUrl  = "smtp://invalid.nonexistent.host:587";

    EmailNotification notifier(cfg);

    bool result = notifier.sendAlert("Test subject", "Test body");

    ASSERT_FALSE(result, "sendAlert returns false for unreachable host");
}

#ifdef LIVE_TEST
// ─── Optional Live Email Test ──────────────────────────────────────────
// Requires environment variables:
// SMTP_USER
// SMTP_PASS
// SMTP_TO

void test_liveEmailSend()
{
    std::cout << "\n[TEST] Live email send (LIVE_TEST enabled)\n";

    const char* user = std::getenv("SMTP_USER");
    const char* pass = std::getenv("SMTP_PASS");
    const char* to   = std::getenv("SMTP_TO");

    if (!user || !pass || !to) {
        std::cout << "  [SKIP] SMTP environment variables not set\n";
        return;
    }

    EmailNotification::Config cfg;
    cfg.fromAddr = user;
    cfg.toAddr   = to;
    cfg.username = user;
    cfg.password = pass;
    cfg.smtpUrl  = "smtp://smtp.gmail.com:587";

    EmailNotification notifier(cfg);

    bool result = notifier.sendAlert(
        "CrowdLens Test",
        "Live test email from CrowdLens CI."
    );

    ASSERT_TRUE(result, "Live email sent successfully");
}

#endif

// ─── Entry Point ───────────────────────────────────────────────────────

int main()
{
    std::cout << "====== EmailNotification Tests ======\n";

    test_configStoredCorrectly();
    test_emptyCredentialsConstruction();
    test_sendAlertFailsOnBadUrl();

#ifdef LIVE_TEST
    test_liveEmailSend();
#endif

    std::cout << "\n=================================\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";

    return (failed == 0) ? 0 : 1;
}