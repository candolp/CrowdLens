#ifndef __CPP_TIMER_CALLBACK
#define __CPP_TIMER_CALLBACK
#include <thread>

#include <vector>

#include "Runnable.h"

// Demo which creates a callback interface as the abstract class "Runnable".
// This then allows to register a callback.

class TrafficEventHandler : public Runnable {
    
public:
	// Registers a runnable event handler to be notified of traffic state changes
	// @param h Reference to a Runnable object that will receive traffic state updates
	void registerEventRunnable(Runnable &h) {
	eventHandlers.push_back(&h);
    }


protected:
	// the traffic_state
	TrafficState traffic_state = TrafficState::NO_TRAFFIC;

    /**
     * Overrides the abstract timer event from Runnable base class
     * Iterates through all registered event handlers and invokes their run() method
	 * with the current traffic state, effectively broadcasting state changes to all subscribers
	**/
	virtual void worker() override {
	for(auto & r : eventHandlers) {
	    r->run(traffic_state);
	}
    }

	std::thread workerThread;

	RunState runState = RunState::STOPPED;

    // vector of all the subscribers
    std::vector<Runnable*> eventHandlers;
};


#endif
