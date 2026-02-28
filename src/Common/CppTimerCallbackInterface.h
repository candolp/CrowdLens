#ifndef __CPP_TIMER_CALLBACK
#define __CPP_TIMER_CALLBACK
#include "CppTimer.h"
#include <vector>

#include "Runnable.h"

// Demo which creates a callback interface as the abstract class "Runnable".
// This then allows to register a callback.

class CppTimerCallbackInterface : public CppTimer {
    
public:

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
