// Integration test: LEDOutput reacting to traffic state events from an upstream source.
// Covers: correct activation, re-entrant guard, stop/join safety, fan-out to multiple
// LEDs, downstream propagation, and concurrent broadcast behaviour.

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "../LEDOutput.h"

// Simulates an upstream event source (e.g. CrowdAnalyser) by exposing eventCallback.
class FakeEventSource : public TrafficEventHandler {
public:
    void broadcast(TrafficState state) {
        eventCallback(state);
    }

protected:
    void worker() override {}
};

// Replaces GPIO with a counter + condition variable so tests can observe worker start/stop.
// worker() blocks until runState leaves RUNNING, mimicking the real blink loop.
class TestableLED : public LEDOutput {
public:
    std::atomic<int> workerCallCount{0};
    std::mutex startMtx;
    std::condition_variable startCv;

    TestableLED(const ConfigLoader& config, TrafficState state)
        : LEDOutput(config, /*skipInit=*/true, state) {}

    // Returns true if workerCallCount reaches n within ms milliseconds.
    bool waitForWorkerCount(int n, int ms = 500) {
        std::unique_lock<std::mutex> lk(startMtx);
        return startCv.wait_for(lk, std::chrono::milliseconds(ms),
            [this, n] { return workerCallCount.load() >= n; });
    }

protected:
    void initHardware() override {}

    void worker() override {
        {
            std::lock_guard<std::mutex> lk(startMtx);
            workerCallCount++;
        }
        startCv.notify_all();
        // hold the thread open so tests can call stop() against a live worker
        while (runState == RunState::RUNNING) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
};

// Tracks how many times stop() is forwarded to a downstream subscriber.
class CountRunnable : public Runnable {
public:
    std::atomic<int> stopCount{0};
    void run(TrafficState) override {}
    void stop(TrafficState) override { stopCount++; }
};

// --- tests ---

TEST(LEDIntegration, MatchingStateActivatesWorker) {
    ConfigLoader config;
    TestableLED led(config, TrafficState::TRAFFIC);

    led.run(TrafficState::TRAFFIC);

    EXPECT_TRUE(led.waitForWorkerCount(1));
    EXPECT_EQ(led.workerCallCount.load(), 1);

    led.stop(TrafficState::TRAFFIC);
}

TEST(LEDIntegration, NonMatchingStateDoesNotActivateWorker) {
    ConfigLoader config;
    TestableLED led(config, TrafficState::TRAFFIC);

    // CROWDED is not the indication state — run() should go through stop(), not start a thread
    led.run(TrafficState::CROWDED);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(led.workerCallCount.load(), 0);
}

TEST(LEDIntegration, ReentrantRunWhileRunningIsNoop) {
    ConfigLoader config;
    TestableLED led(config, TrafficState::TRAFFIC);

    led.run(TrafficState::TRAFFIC);
    ASSERT_TRUE(led.waitForWorkerCount(1));

    // fire matching state again while worker is already running
    led.run(TrafficState::TRAFFIC);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // workerThread.joinable() guard in run() must block the second start
    EXPECT_EQ(led.workerCallCount.load(), 1);

    led.stop(TrafficState::TRAFFIC);
}

TEST(LEDIntegration, StopJoinsWorkerWithoutHanging) {
    ConfigLoader config;
    TestableLED led(config, TrafficState::TRAFFIC);

    led.run(TrafficState::TRAFFIC);
    ASSERT_TRUE(led.waitForWorkerCount(1));

    // stop() must set runState, wait for worker to exit, then return
    led.stop(TrafficState::TRAFFIC);

    // if we reach here the join completed — no hang
    SUCCEED();
}

TEST(LEDIntegration, StateTransitionStopsThenRestarts) {
    ConfigLoader config;
    TestableLED led(config, TrafficState::TRAFFIC);

    // first run
    led.run(TrafficState::TRAFFIC);
    ASSERT_TRUE(led.waitForWorkerCount(1));

    // non-matching state triggers stop() synchronously — thread is joined before run() returns
    led.run(TrafficState::CROWDED);

    // restart with matching state; joinable() should now be false so a new thread starts
    led.run(TrafficState::TRAFFIC);
    EXPECT_TRUE(led.waitForWorkerCount(2));
    EXPECT_GE(led.workerCallCount.load(), 2);

    led.stop(TrafficState::TRAFFIC);
}

TEST(LEDIntegration, FanOutActivatesOnlyMatchingLED) {
    ConfigLoader config;
    FakeEventSource source;

    TestableLED trafficLed(config, TrafficState::TRAFFIC);
    TestableLED crowdedLed(config, TrafficState::CROWDED);

    source.registerEventRunnable(trafficLed);
    source.registerEventRunnable(crowdedLed);

    // broadcast CROWDED — only crowdedLed's indication state matches
    source.broadcast(TrafficState::CROWDED);

    EXPECT_TRUE(crowdedLed.waitForWorkerCount(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(crowdedLed.workerCallCount.load(), 1);
    EXPECT_EQ(trafficLed.workerCallCount.load(), 0); // must stay off

    crowdedLed.stop(TrafficState::CROWDED);
}

TEST(LEDIntegration, StopPropagatesDownstreamToRegisteredRunnable) {
    ConfigLoader config;
    TestableLED led(config, TrafficState::TRAFFIC);
    CountRunnable downstream;

    led.registerEventRunnable(downstream);

    led.run(TrafficState::TRAFFIC);
    ASSERT_TRUE(led.waitForWorkerCount(1));

    led.stop(TrafficState::TRAFFIC);

    // LEDOutput::stop() iterates eventHandlers and calls stop() on each
    EXPECT_EQ(downstream.stopCount.load(), 1);
}

// Fires the same state from two threads simultaneously.
// LEDOutput::run() has no mutex around the joinable() check, so this is a known
// data race — we verify the system does not deadlock or crash regardless.
TEST(LEDIntegration, ConcurrentBroadcastsDoNotDeadlock) {
    ConfigLoader config;
    FakeEventSource source;
    TestableLED led(config, TrafficState::TRAFFIC);
    source.registerEventRunnable(led);

    std::thread t1([&source] { source.broadcast(TrafficState::TRAFFIC); });
    std::thread t2([&source] { source.broadcast(TrafficState::TRAFFIC); });
    t1.join();
    t2.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // regardless of how many workers started, stop() must return cleanly
    led.stop(TrafficState::TRAFFIC);
    SUCCEED();
}

// Rapid sequence of matching and non-matching states to exercise run/stop churn.
TEST(LEDIntegration, RapidStateCyclingNoDeadlock) {
    ConfigLoader config;
    TestableLED led(config, TrafficState::STAMPEDE);

    for (int i = 0; i < 5; i++) {
        led.run(TrafficState::STAMPEDE);  // start (or no-op if already running)
        led.run(TrafficState::NO_TRAFFIC); // stop (non-matching)
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    led.stop(TrafficState::STAMPEDE);
    SUCCEED();
}

// Destructor fires while the worker thread is still running.
// ~LEDOutput() sets runState = STOPPED and joins — if the join is missing or racy this hangs.
TEST(LEDIntegration, DestructorWhileWorkerRunning) {
    ConfigLoader config;
    {
        TestableLED led(config, TrafficState::TRAFFIC);
        led.run(TrafficState::TRAFFIC);
        ASSERT_TRUE(led.waitForWorkerCount(1));
        // led goes out of scope here; destructor must join the worker cleanly
    }
    SUCCEED();
}

// Calling stop() on a LED that was never started must not crash.
// workerThread is default-constructed (not joinable) and request is nullptr.
TEST(LEDIntegration, StopBeforeRunIsNoop) {
    ConfigLoader config;
    TestableLED led(config, TrafficState::TRAFFIC);
    led.stop(TrafficState::TRAFFIC);
    EXPECT_EQ(led.workerCallCount.load(), 0);
}

// Two LEDs with the same indication state both registered to the same source.
// A single broadcast must activate both — verifies fan-out doesn't stop at the first subscriber.
TEST(LEDIntegration, MultipleLEDsSameStateAllActivate) {
    ConfigLoader config;
    FakeEventSource source;
    TestableLED led1(config, TrafficState::CROWDED);
    TestableLED led2(config, TrafficState::CROWDED);

    source.registerEventRunnable(led1);
    source.registerEventRunnable(led2);

    source.broadcast(TrafficState::CROWDED);

    EXPECT_TRUE(led1.waitForWorkerCount(1));
    EXPECT_TRUE(led2.waitForWorkerCount(1));

    led1.stop(TrafficState::CROWDED);
    led2.stop(TrafficState::CROWDED);
}