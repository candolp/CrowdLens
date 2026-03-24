#include "EmailNotification.h"
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

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

void writeTestConfig(const std::string& filename) {
    std::ofstream ofs(filename);
    ofs << "notification:\n";
    ofs << "  email:\n";
    ofs << "    smtp_url: smtp://smtp.example.com:587\n";
    ofs << "    from_addr: a@test.com\n";
    ofs << "    to_addr: b@test.com\n";
    ofs << "    username: user\n";
    ofs << "    password: pass\n";
    ofs.close();
}

// ─────────────────────────────────────────────
// Test 1: Config construction
// ─────────────────────────────────────────────

void test_configStoredCorrectly()
{
    std::cout << "\n[TEST] Config stored correctly\n";

    const std::string testFile = "test_config.yaml";
    writeTestConfig(testFile);
    
    ConfigLoader config(testFile);
    EmailNotification notifier(config);

    ASSERT_TRUE(true,"EmailNotification constructed without exception");
    std::filesystem::remove(testFile);
}


// ─────────────────────────────────────────────
// Test 2: Empty credentials allowed
// ─────────────────────────────────────────────

void test_emptyCredentialsConstruction()
{
    std::cout << "\n[TEST] Empty credentials accepted at construction\n";

    ConfigLoader config("nonexistent.yaml"); // Should result in default values
    bool threw=false;

    try{
        EmailNotification notifier(config);
    }
    catch(...){
        threw=true;
    }

    ASSERT_FALSE(threw,"Constructor does not throw on empty config");
}


// ─────────────────────────────────────────────
// Test 3: Invalid SMTP host should fail
// ─────────────────────────────────────────────

void test_sendAlertFailsOnBadUrl()
{
    std::cout << "\n[TEST] sendAlert returns false on bad SMTP URL\n";

    const std::string testFile = "test_bad_url.yaml";
    std::ofstream ofs(testFile);
    ofs << "notification:\n  email:\n    smtp_url: smtp://invalid.nonexistent.host:587\n";
    ofs.close();

    ConfigLoader config(testFile);
    EmailNotification notifier(config);

    bool result=notifier.sendAlert("Test subject","Test body");

    ASSERT_FALSE(result,"sendAlert returns false for unreachable host");
    std::filesystem::remove(testFile);
}

// ─────────────────────────────────────────────
// Main test runner
// ─────────────────────────────────────────────

int main()
{
    std::cout<<"====== EmailNotification Tests ======\n";

    test_configStoredCorrectly();
    test_emptyCredentialsConstruction();
    test_sendAlertFailsOnBadUrl();

    std::cout<<"\n=================================\n";
    std::cout<<"Results: "<<passed<<" passed, "<<failed<<" failed\n";

    return (failed==0)?0:1;
}

// To run test :
// cd ~/Documents/MSc/RTEP/emailnotification/CrowdLens
// rm -rf build
// mkdir build
// cd build
// cmake 
// ./src/Notification/test/test_email 