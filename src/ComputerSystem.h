#ifndef COMPUTERSYSTEM_H_
#define COMPUTERSYSTEM_H_
//#define _DEFAULT_SOURCE
#include <iostream>
#include <vector>
#include <pthread.h>
#include "AircraftData.h"
#include "MPData.h"
//#include "Radar.cpp"
//#include "DataDisplay.h"
using namespace std;



class ComputerSystem {

public:


    vector<AircraftData> aircraft;

    //bool for computeViolations function:
    bool violationDetected ;

    //pthread to detect alert to operator:
    pthread_t detectViolationsThread;
    pthread_t handlingCommandsThread;

    // //DataDisplay Instance:
//    DataDisplay dataDisplay;


    //constructor: We need to know the number of aircrafts in the air to determine if any violations occur
	ComputerSystem();

    //Destructor:
	~ComputerSystem();

	//To calculate and detect Violations
    //const : to not modify contents in aricraft.
    // This function only reads -> no need to modify anything
    bool computeViolations(vector<AircraftData> aircraft,int futureTime);

    void ComputerSystemServer(string channelName);

    static void* ComputerSystemDetectViolationsRoutine(void* args);

    void computerSystemClient(MPData MsgToSend);

    //To calculate periodic computations
    void periodicComputations();

    //Function to calculate future position:
    void calculateFuturePosition(AircraftData* aircraft, double time_);

    //Function to calculate the prediction for the next 3 minutes:
    void computeViolationsFor3Minutes(vector<AircraftData> aircraft);

    // Function to run for the handling thread that is responsible for receiving from OC
    static void* HandlingCommands(void* args);

    // Function that is the server for handling commands thread
    void CommandsComputerSystemServer(string channelName);

    //Function to send commands to Communication System
    void ComputerSystem_CommSys_Client(MPData MsgToSend);




};

#endif
