#ifndef __CPP_TIMER_CALLBACK
#define __CPP_TIMER_CALLBACK
#include "CppTimer.h"
#include <vector>

// Demo which creates a callback interface as the abstract class "Runnable".
// This then allows to register a callback.

class CppTimerCallbackInterface : public CppTimer {
    
public:
    // An interface which can be added
    // to the receiving class. We call it runnable
    // it contains a single method called "run"!
    class Runnable {
    public:
	virtual void run() = 0;
    };

    // The subscriber registers it.
    void registerEventRunnable(Runnable &h) {
	cppTimerEventRunnables.push_back(&h);
    }

protected:
    // overrides the abstract timer event
    void timerEvent() override {
	for(auto & r : cppTimerEventRunnables) {
	    r->run();
	}
    }

    // vector of all the subscribers
    std::vector<Runnable*> cppTimerEventRunnables;
};


#endif
