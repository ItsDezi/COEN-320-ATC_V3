#ifndef SRC_AIRCRAFT_H_
#define SRC_AIRCRAFT_H_
#include <iostream>
#include <string>
#include <string.h>
#include <pthread.h>
#include "aircraft.h"
#include <chrono>
#include "AircraftData.h"
#include <unistd.h>
#include "cTimer.h"
#include <chrono>
#include <thread>
//#include "conditionalvariables.h"
#include "MPData.h"
using namespace std;

class aircraft {
public:
	int ID;
	bool arrived = false;
	aircraft();
	pthread_t thread;
	pthread_t receivingCommandsThread;
	virtual ~aircraft();
	AircraftData data;
	double prevClock;
	aircraft(AircraftData dataTing);
	void test_print();
	static void* updatePosition(void* args);
	void changeSpeed(int, int);
	void AC_Client(MPData);
	static void* run(void *a);
	static void* ReceivingCommandsRoutine(void* args);
	void ReceivingCommandsServer(string channelName);

};

#endif /* SRC_AIRCRAFT_H_ */
