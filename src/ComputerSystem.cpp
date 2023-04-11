#include "ComputerSystem.h"
#include <iostream>
#include <pthread.h>
#include <cmath>
#include "AircraftData.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include "MPData.h"
#include <sys/dispatch.h>
#include <sys/neutrino.h>
using namespace std;


//Constructor:
ComputerSystem::ComputerSystem()
{
	//     dataDisplay = DataDisplay(); // initializing dataDisplay
}

//Destructor
ComputerSystem::~ComputerSystem(){
}

void* ComputerSystem::ComputerSystemDetectViolationsRoutine(void* args){

	ComputerSystem* computerSystemPointer = (ComputerSystem *) args;

	// Receive Data from Radar
	computerSystemPointer->ComputerSystemServer("RADAR-CS");

	return NULL;
}


void ComputerSystem::computerSystemClient(MPData MsgToSend){

	int server_coid; //server connection ID.
	if ((server_coid = name_open(MsgToSend.channelName.c_str(), 0)) == -1) {
		perror("Error occurred while attaching the channel");
	}

	if (MsgSend(server_coid, &MsgToSend, sizeof(MsgToSend), NULL,0) == -1) {
		printf("Error while sending the message from Client");
	}

};




//SERVER SIDE: RADAR - CS
void ComputerSystem::ComputerSystemServer(string channelName){

	// Variables
	MPData MsgToRecieve;

	name_attach_t *attach;
	int rcvid;


	if ((attach = name_attach(NULL, channelName.c_str(), 0)) == NULL) {
		perror("Error occurred while creating the channel");
	}


	while (true) {

		rcvid = MsgReceive(attach->chid, &MsgToRecieve, sizeof(MsgToRecieve), NULL);


		if (rcvid == -1) {/* Error condition, exit */
			break;
		}

		if (rcvid == 0) {/* Pulse received */
			switch (MsgToRecieve.hdr.code) {
			case _PULSE_CODE_DISCONNECT:
				/*
				 * A client disconnected all its connections (called
				 * name_close() for each name_open() of our name) or
				 * terminated
				 */
				ConnectDetach(MsgToRecieve.hdr.scoid);
				break;
			case _PULSE_CODE_UNBLOCK:
				/*
				 * REPLY blocked client wants to unblock (was hit by
				 * a signal or timed out).  It's up to you if you
				 * reply now or later.
				 */
				break;
			default:
				/*
				 * A pulse sent by one of your processes or a
				 * _PULSE_CODE_COIDDEATH or _PULSE_CODE_THREADDEATH
				 * from the kernel?
				 */
				break;
			}
			continue;
		}

		/* name_open() sends a connect message, must EOK this */
		if (MsgToRecieve.hdr.type == _IO_CONNECT ) {
			MsgReply( rcvid, EOK, NULL, 0 );
			continue;
		}

		/* Some other QNX IO message was received; reject it */
		if (MsgToRecieve.hdr.type > _IO_BASE && MsgToRecieve.hdr.type <= _IO_MAX ) {
			MsgError( rcvid, ENOSYS );
			continue;
		}

		//What to do after receiving the list of aircrafts in range.
		// //TODO: 1.send the vector to Data Display

		computeViolationsFor3Minutes(MsgToRecieve.aircraftsInRange);


		MsgReply(rcvid, EOK, 0, 0);
	}

	/* Remove the name from the space */
	name_detach(attach, 0);
};

//Calculate future position and then pass to computeViolations to predict violations
void ComputerSystem::calculateFuturePosition(AircraftData* aircraft, double time ) {
	double speed_x = aircraft->xSpeed;
	double speed_y = aircraft->ySpeed;
	double speed_z = aircraft->zSpeed;


	aircraft->x = aircraft->x + speed_x * time;
	aircraft->y = aircraft->y + speed_y * time;
	aircraft->z = aircraft->z + speed_z * time;
}


bool ComputerSystem::computeViolations(vector<AircraftData> aircraft, int futureTime) {
	bool violationDetected = false;

	// Receive data from message passing channel:

	// to check for Violations: ( Check for constraints )
	for (int i = 0; i < aircraft.size(); i++) {
		for (int j = i + 1; j < aircraft.size(); j++) {

			// Calculate future position if t != current time based on speed (as vector and time)
			// as parameters and returns new position:
			calculateFuturePosition(&aircraft[i], futureTime);
			calculateFuturePosition(&aircraft[j], futureTime);



			double diff_position_x = abs(aircraft[i].x - aircraft[j].x);
			double diff_position_y = abs(aircraft[i].y - aircraft[j].y);
			double diff_position_z = abs(aircraft[i].z - aircraft[j].z);

			if (diff_position_x < 3000 && diff_position_y < 3000 && diff_position_z < 1000) {
				violationDetected = true;
				cout << endl;
				cout << "WARNING! Violation detected between aircraft ID " << aircraft[i].ID << " and aircraft ID "<< aircraft[j].ID << endl;
				cout << "Here is the required information:  " << endl;

				//Show axis violation:
				// TODO: Remove after test
				cout <<"AC "<< aircraft[i].ID<<" Future x-Axis:  "<<aircraft[i].x<<endl;
				cout <<"AC "<< aircraft[i].ID<<" Future y-Axis:  "<<aircraft[i].y<<endl;
				cout <<"AC "<< aircraft[i].ID<<" Future z-Axis:  "<<aircraft[i].z<<endl;

				cout <<"AC "<< aircraft[j].ID<<" Future x-Axis:  "<<aircraft[j].x<<endl;
				cout <<"AC "<< aircraft[j].ID<<" Future y-Axis:  "<<aircraft[j].y<<endl;
				cout <<"AC "<< aircraft[j].ID<<" Future z-Axis:  "<<aircraft[j].z<<endl;

				cout << "* Violation is on X-axis: The distance in the X-axis is: " << diff_position_x << endl;

				cout << "* Violation is on Y-axis: The distance in the Y-axis is: " << diff_position_y << endl;

				cout << "* Violation is on the Z-axis: The distance in the Z-axis is:  " << diff_position_z << endl;


			}
		}
	}

	return violationDetected;
}

void ComputerSystem::computeViolationsFor3Minutes(vector<AircraftData> aircraft) {


	//	for(AircraftData aircraftItem : aircraft){
	//		cout <<"AC " <<aircraftItem.ID << " received by Computer System." << endl;
	//	}


	for (int t = 0; t < 180; t++) {
		// Check for violations in the next 180 seconds
		bool violationDetected = computeViolations(aircraft,t);

		if (violationDetected) {
			// If a violation is detected break out of the loop and exit the function
			cout << "Violation detected and it will happen in " <<t << " seconds from now." << endl;
		}
	}

}


// HANDLING COMMANDS FROM OPERATOR CONSOLE

 void* ComputerSystem::HandlingCommands(void* args){

		ComputerSystem * computerSystem = (ComputerSystem*) args;

		// receives command from Operator Console
		computerSystem ->CommandsComputerSystemServer("OC-CS");

		return NULL;
	}

 void ComputerSystem::CommandsComputerSystemServer(string channelName){

 		// Variables
 		MPData MsgToRecieve;

 		name_attach_t *attach;
 		int rcvid;


 		if ((attach = name_attach(NULL, channelName.c_str(), 0)) == NULL) {
 			perror("Error occurred while creating the channel from Commands Computer System Server ");
 		}


 		while (true) {

 			rcvid = MsgReceive(attach->chid, &MsgToRecieve, sizeof(MsgToRecieve), NULL);


 			if (rcvid == -1) {/* Error condition, exit */
 				break;
 			}

 			if (rcvid == 0) {/* Pulse received */
 				switch (MsgToRecieve.hdr.code) {
 				case _PULSE_CODE_DISCONNECT:
 					/*
 					 * A client disconnected all its connections (called
 					 * name_close() for each name_open() of our name) or
 					 * terminated
 					 */
 					ConnectDetach(MsgToRecieve.hdr.scoid);
 					break;
 				case _PULSE_CODE_UNBLOCK:
 					/*
 					 * REPLY blocked client wants to unblock (was hit by
 					 * a signal or timed out).  It's up to you if you
 					 * reply now or later.
 					 */
 					break;
 				default:
 					/*
 					 * A pulse sent by one of your processes or a
 					 * _PULSE_CODE_COIDDEATH or _PULSE_CODE_THREADDEATH
 					 * from the kernel?
 					 */
 					break;
 				}
 				continue;
 			}

 			/* name_open() sends a connect message, must EOK this */
 			if (MsgToRecieve.hdr.type == _IO_CONNECT ) {
 				MsgReply( rcvid, EOK, NULL, 0 );
 				continue;
 			}

 			/* Some other QNX IO message was received; reject it */
 			if (MsgToRecieve.hdr.type > _IO_BASE && MsgToRecieve.hdr.type <= _IO_MAX ) {
 				MsgError( rcvid, ENOSYS );
 				continue;
 			}


 			// TODO: Change What to do after receiving a message from Operator Console
 			MPData MsgToSend = MsgToRecieve;
 			if(MsgToSend.command.readOrChange == 0) // read command
 			{
 				// TODO:  send data to data display
 			}
 			else if (MsgToSend.command.readOrChange == 1) // change command
 			{
 				// Send the command to Communication System
 				MsgToSend.channelName = "CS-COMMSYS";
 				ComputerSystem_CommSys_Client(MsgToSend);
 			}
 			else {
 				cout << "Error in the command type read or change.";
 			}

 			MsgReply(rcvid, EOK, 0, 0);
 		}

 		/* Remove the name from the space */
 		name_detach(attach, 0);
 	};

 	void ComputerSystem::ComputerSystem_CommSys_Client(MPData MsgToSend){

 		int server_coid; //server connection ID.
 		if ((server_coid = name_open(MsgToSend.channelName.c_str(), 0)) == -1) {
 			perror("Error occurred while attaching the channel ComputerSystem_CommSys_Client");
 		}

 		if (MsgSend(server_coid, &MsgToSend, sizeof(MsgToSend), NULL,0) == -1) {
 			printf("Error while sending the message from Client");
 		}

 	};



