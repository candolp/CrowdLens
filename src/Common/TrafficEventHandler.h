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

	void run(TrafficState state) override
	{
		traffic_state = state;
		runState = RunState::RUNNING;
		// Starting thread
		workerThread = std::thread(&TrafficEventHandler::worker, this);
	}

	/**
	 * Stops the traffic event handler and notifies all registered event handlers.
	 * Terminates the worker thread and propagates the stop signal to all subscribers.
	 * @param traffic_state The final traffic state to be passed to all event handlers during shutdown
	 */
	void stop(TrafficState traffic_state) override {
		runState = RunState::STOPPED;
		for(auto & r : eventHandlers)
		{
			r->stop(traffic_state);
		}

		if (workerThread.joinable()) workerThread.join();
	}


protected:
	// the traffic_state
	TrafficState traffic_state = TrafficState::NO_TRAFFIC;

    /**
     * Overrides the abstract timer event from Runnable base class
     * Iterates through all registered event handlers and invokes their run() method
	 * with the current traffic state, effectively broadcasting state changes to all subscribers
	**/
	virtual void worker()=0;
	/**
	 * Notifies all registered event handlers of a traffic state change.
	 * Broadcasts the traffic state update to all subscribers by invoking their run() method.
	 * @param trafficState The current traffic state to be propagated to all event handlers
	 */
	virtual void eventCallback(TrafficState trafficState) {
		for(auto & r : eventHandlers) {
			r->run(trafficState);
		}
	}


	std::thread workerThread;

	RunState runState = RunState::STOPPED;

    // vector of all the subscribers
    std::vector<Runnable*> eventHandlers;


};


#endif
