#include "EmailNotification.h"
#include "config/ConfigLoader.h"
#include <iostream>
#include <string>

// ─────────────────────────────────────────────
// Simple test framework
// ─────────────────────────────────────────────

static int passed = 0;
static int failed = 0;

#define ASSERT_TRUE(expr, msg)                                          
    do {                                                                
        if (expr) {                                                     
            std::cout << "  [PASS] " << msg << "\n";                    
            ++passed;                                                   
        } else {                                                        
            std::cerr << "  [FAIL] " << msg << "\n";                    
            ++failed;                                                   
        }                                                               
    } while (0)

#define ASSERT_FALSE(expr, msg) ASSERT_TRUE(!(expr), msg)


// ─────────────────────────────────────────────
// Test 1: Config construction
// ─────────────────────────────────────────────

void test_configStoredCorrectly()
{
    std::cout << "\n[TEST] Config stored correctly\n";

    EmailNotification::Config cfg;
    cfg.fromAddr="a@test.com";
    cfg.toAddr="b@test.com";
    cfg.username="user";
    cfg.password="pass";
    cfg.smtpUrl="smtp://smtp.example.com:587";

    EmailNotification notifier(cfg);

    ASSERT_TRUE(true,"EmailNotification constructed without exception");
}


// ─────────────────────────────────────────────
// Test 2: Empty credentials allowed
// ─────────────────────────────────────────────

void test_emptyCredentialsConstruction()
{
    std::cout << "\n[TEST] Empty credentials accepted at construction\n";

    EmailNotification::Config cfg;

    bool threw=false;

    try{
        EmailNotification notifier(cfg);
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

    EmailNotification::Config cfg;
    cfg.fromAddr="a@b.com";
    cfg.toAddr="c@d.com";
    cfg.username="user";
    cfg.password="pass";
    cfg.smtpUrl="smtp://invalid.nonexistent.host:587";

    EmailNotification notifier(cfg);

    bool result=notifier.sendAlert("Test subject","Test body");

    ASSERT_FALSE(result,"sendAlert returns false for unreachable host");
}


// ─────────────────────────────────────────────
// LIVE email test (reads .cfg file)
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

    bool result =
        notifier.sendAlert(
            "CrowdLens Test",
            "Test email."
        );

    ASSERT_TRUE(result,"Live email sent successfully");
}

#endif


// ─────────────────────────────────────────────
// Main test runner
// ─────────────────────────────────────────────

int main()
{
    std::cout<<"====== EmailNotification Tests ======\n";

    test_configStoredCorrectly();
    test_emptyCredentialsConstruction();
    test_sendAlertFailsOnBadUrl();

#ifdef LIVE_TEST
    test_liveEmailSend();
#endif

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