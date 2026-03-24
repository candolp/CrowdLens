# UofG-real-time-crowd-movement-monitoring-system
A Real Time Crowd Movement Flow Monitoring System created using C++ on Raspberry Pi.

## short description
Current : Brach  develop_IR_senor_reading

## System Architecture
The system is Designed to be modular.
The Modules are:
1. IR Sensor
2. Camera Capture
3. AI Detection
4. Hardware Output
5. Notification

The Starts from Demo Module.

### Setup
In the Demo's Main application Objects are created from each module. multiple objects can be created from a single module. can be created from each module.
The modules extends [TrafficEventHandler.h](src/Common/TrafficEventHandler.h) class. This class provides the interface for all the modules. Worker Threat, callback function and a start and stop function are provided.
Any class extending this class can be used as a module. which can be registered in another moules as an event handler.

### System Flow
The Crowd Movement Flow is monitored by the IR Sensor. 
The Camera Capture is used to capture the image.
The AI Detection is used to detect the crowd movement.
The Hardware Output is used to send the data to the Raspberry Pi.
The Notification is used to send the data to the user.

The latest Branch is develop_IR_senor_reading