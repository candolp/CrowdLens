#include "EmailNotification.h"
#include "config/ConfigLoader.h"
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
    cfg.fromAddr = "a@test.com";
    cfg.toAddr   = "b@test.com";
    cfg.username = "user";
    cfg.password = "pass";
    cfg.smtpUrl  = "smtp://smtp.example.com:587";

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
    cfg.fromAddr = "a@b.com";
    cfg.toAddr   = "c@d.com";
    cfg.username = "user";
    cfg.password = "pass";
    cfg.smtpUrl  = "smtp://invalid.nonexistent.host:587";

    EmailNotification notifier(cfg);
    bool result = notifier.sendAlert("Test subject", "Test body");

    ASSERT_FALSE(result, "sendAlert returns false for unreachable host");
}


// ─────────────────────────────────────────────
// Test 4: loadConfig throws on missing file
// ─────────────────────────────────────────────

void test_configLoaderThrowsOnMissingFile()
{
    std::cout << "\n[TEST] loadConfig throws std::runtime_error on missing file\n";

    bool threw = false;
    try {
        loadConfig("/nonexistent/path/nowhere.cfg");
    } catch (const std::runtime_error&) {
        threw = true;
    } catch (...) {}

    ASSERT_TRUE(threw, "loadConfig throws runtime_error for missing file");
}


// ─────────────────────────────────────────────
// Test 5: loadConfig parses key=value correctly
// (writes a temp file, reads it back, checks values)
// ─────────────────────────────────────────────

void test_configLoaderParsesValues()
{
    std::cout << "\n[TEST] loadConfig parses key=value pairs correctly\n";

    const std::string tmpPath = "/tmp/crowdlens_test.cfg";

    // Write a temp config
    {
        std::ofstream f(tmpPath);
        f << "# comment line\n";
        f << "KEY_A = hello\n";
        f << "KEY_B=world\n";
        f << "KEY_C = base64==\n";   // value contains '='
        f << "\n";                   // blank line
    }

    auto cfg = loadConfig(tmpPath);

    ASSERT_TRUE(cfg["KEY_A"] == "hello",    "KEY_A parses to 'hello'");
    ASSERT_TRUE(cfg["KEY_B"] == "world",    "KEY_B parses to 'world'");
    ASSERT_TRUE(cfg["KEY_C"] == "base64==", "KEY_C preserves '=' inside value");
    ASSERT_TRUE(cfg.count("KEY_A") == 1,    "Comment line is not parsed as a key");
}


// ─────────────────────────────────────────────
// LIVE email test (opt-in via -DLIVE_TEST=ON)
// ─────────────────────────────────────────────

#ifdef LIVE_TEST

void test_liveEmailSend()
{
    std::cout << "\n[TEST] Live email send\n";

    auto conf = loadConfig("../src/Notification/config/EmailCredentials.cfg");

    EmailNotification::Config cfg;
    cfg.fromAddr = conf["FROM_ADDR"];
    cfg.toAddr   = conf["TO_ADDR"];
    cfg.username = conf["SMTP_USER"];
    cfg.password = conf["SMTP_PASS"];
    cfg.smtpUrl  = conf["SMTP_URL"];

    EmailNotification notifier(cfg);

    bool result = notifier.sendAlert("CrowdLens Test", "Test email.");

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
 *   cmake --build .
 *   ctest --output-on-failure
 *
 * Run live email test:
 *   cmake .. -DLIVE_TEST=ON
 *   cmake --build . && ./src/Notification/test/test_email
 */