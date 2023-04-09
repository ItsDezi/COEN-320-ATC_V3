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
	//cTimer clk;
	bool arrived = false;
	//thread m_thread;
	aircraft();
	pthread_t thread;
	virtual ~aircraft();
	AircraftData data;
	double prevClock;
	aircraft(AircraftData dataTing);
	void test_print();
	static void* updatePosition(void* args);
	void changeSpeed();
	void AC_Client(MPData);
	static void* run(void *a);

};

#endif /* SRC_AIRCRAFT_H_ */
