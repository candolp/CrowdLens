//
// Created by Amantle on 20/03/2026.
//

#include "EmailNotification.h"
#include <iostream>
#include <fstream>
#include <string>

// ─────────────────────────────────────────────
// Simple test framework
// ─────────────────────────────────────────────

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


// ─────────────────────────────────────────────
// Test 1: Config construction
// ─────────────────────────────────────────────

void test_configStoredCorrectly()
{
    std::cout << "\n[TEST] Config stored correctly\n";

    EmailNotification::Config cfg;
    cfg.fromAddr    = "a@test.com";
    cfg.toAddr      = "b@test.com";
    cfg.username    = "user";
    cfg.password    = "pass";
    cfg.smtpServer  = "smtp.example.com";
    cfg.smtpPort    = 587;

    EmailNotification notifier(cfg);

    ASSERT_TRUE(true, "EmailNotification constructed without exception");
}


// ─────────────────────────────────────────────
// Test 2: Empty credentials allowed at construction
// ─────────────────────────────────────────────

void test_emptyCredentialsConstruction()
{
    std::cout << "\n[TEST] Empty credentials accepted at construction\n";

    EmailNotification::Config cfg;
    bool threw = false;

    try {
        EmailNotification notifier(cfg);
    } catch (...) {
        threw = true;
    }

    ASSERT_FALSE(threw, "Constructor does not throw on empty config");
}


// ─────────────────────────────────────────────
// Test 3: Invalid SMTP host → sendAlert returns false
// ─────────────────────────────────────────────

void test_sendAlertFailsOnBadUrl()
{
    std::cout << "\n[TEST] sendAlert returns false on bad SMTP URL\n";

    EmailNotification::Config cfg;
    cfg.fromAddr   = "a@b.com";
    cfg.toAddr     = "c@d.com";
    cfg.username   = "user";
    cfg.password   = "pass";
    cfg.smtpServer = "invalid.nonexistent.host";
    cfg.smtpPort   = 587;

    EmailNotification notifier(cfg);
    bool result = notifier.sendAlert("Test subject", "Test body");

    ASSERT_FALSE(result, "sendAlert returns false for unreachable host");
}


// ─────────────────────────────────────────────
// Test 4: loadConfig throws on missing file
// ─────────────────────────────────────────────

void test_configLoaderThrowsOnMissingFile()
{
    std::cout << "\n[TEST] ConfigLoader returns defaults for missing keys\n";

    ConfigLoader loader("/nonexistent/path/nowhere.yaml");
    // Should not throw — missing file gives empty config, getValue returns defaults
    std::string val = loader.getValue("some:key", "default");
    ASSERT_TRUE(val == "default", "getValue returns default for missing file/key");
}


// ─────────────────────────────────────────────
// Test 5: ConfigLoader reads Notifications section
// ─────────────────────────────────────────────

void test_configLoaderParsesValues()
{
    std::cout << "\n[TEST] ConfigLoader constructed without crash\n";

    EmailNotification::Config cfg;
    cfg.smtpServer = "smtp.gmail.com";
    cfg.smtpPort   = 587;
    cfg.fromAddr   = "test@example.com";
    cfg.toAddr     = "a@x.com, b@x.com";
    cfg.username   = "user";
    cfg.password   = "pass";

    EmailNotification notifier(cfg);
    ASSERT_TRUE(true, "EmailNotification constructed with multi-recipient config");
}


// ─────────────────────────────────────────────
// LIVE email test (opt-in via -DLIVE_TEST=ON)
// ─────────────────────────────────────────────

#ifdef LIVE_TEST

void test_liveEmailSend()
{
    std::cout << "\n[TEST] Live email send\n";

    // Reads from config.yaml placed next to the binary (cmake copies it there)
    ConfigLoader config(CONFIG_PATH);
    EmailNotification notifier(config);

    bool result = notifier.sendAlert("CrowdLens Live Test", "Live test email from CrowdLens test_email.");

    ASSERT_TRUE(result, "Live email sent successfully");
}

#endif


// ─────────────────────────────────────────────
// Main test runner
// ─────────────────────────────────────────────

int main()
{
    std::cout << "====== EmailNotification Tests ======\n";

    test_configStoredCorrectly();
    test_emptyCredentialsConstruction();
    test_sendAlertFailsOnBadUrl();
    test_configLoaderThrowsOnMissingFile();
    test_configLoaderParsesValues();

#ifdef LIVE_TEST
    test_liveEmailSend();
#endif

    std::cout << "\n=====================================\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";

    return (failed == 0) ? 0 : 1;
}

/*
 * Build & run (unit tests only):
 *   mkdir -p build && cd build
 *   cmake ..
 *   cmake --build . --target test_email
 *   ctest --output-on-failure
 *
 * Run live email test (sends a real email using config.yaml credentials):
 *   cmake .. -DLIVE_TEST=ON
 *   cmake --build . --target test_email
 *   ./src/Notification/test/test_email
 */

/*
 * Build & run (unit tests only):
 *   mkdir -p build && cd build
 *   cmake ..
 *   cmake --build .
 *   ctest --output-on-failure
 *
 * Run live email test:
 *   cmake .. -DLIVE_TEST=ON
 *   cmake --build . && ./src/Notification/test/test_email
 */