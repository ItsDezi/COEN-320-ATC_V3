/*
 * Radar2.cpp
 *
 *  Created on: Apr. 6, 2023
 *      Author: Bisho
 */
#ifndef RADAR_CPP_
#define RADAR_CPP_

#include <list>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <sys/dispatch.h>
#include "AircraftData.h"
#include "MPData.h"


class Radar {

public:

	// Attributes
	vector<AircraftData> inRangeAircrafts;
	int* activeAircraftsCount;
	pthread_t thread;
	// Methods
	Radar(int* num) {
		activeAircraftsCount = num;
	}
	Radar(){}
	~Radar() {
		// TODO Auto-generated destructor stub
	}

	static void* radarStartRoutine(void* args){


		Radar* radarPointer = (Radar *) args;

		// Receive Data from Aircrafts
		radarPointer->radarServer("AC-RADAR");

		return NULL;
	}


	void detectAircraftsInRange(AircraftData aircraft){


		if ((aircraft.x > 0 && aircraft.x <100000)&&
				(aircraft.y > 0 && aircraft.y <100000)&&
				(aircraft.z > 15000 && aircraft.z <40000)){
			inRangeAircrafts.push_back(aircraft);
		}
	}

	void printAircraftsInRange(){
		cout << "//============================START RADAR==================================//"<<endl;
		for(AircraftData aircraft: inRangeAircrafts){
			cout << "AC" << aircraft.ID << " is in Radar Range."<< endl;
		}
		cout << "==============================END RADAR===================================="<<endl;
	}



	void radarServer(string channelName){

		// Variables
		MPData MsgToRecieve;

		// Variable for Receive
		int aircraftsReceived = 0;

		name_attach_t *attach;
		int rcvid;


		if ((attach = name_attach(NULL, channelName.c_str(), 0)) == NULL) {
			perror("Error occurred while creating the channel");
		}

//Server Routine
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


			// TODO: Change What to do after receiving a message
			// Check if the aircraft is in range
			detectAircraftsInRange(MsgToRecieve.aircraft);
			aircraftsReceived++;

			//	Send to Computer System once received from all Aircrafts.
			if ((*activeAircraftsCount) !=0 && aircraftsReceived == (*activeAircraftsCount)){
				//TODO send it to computer system
				printAircraftsInRange(); // TODO: remove it after testing.
				inRangeAircrafts.clear();
				aircraftsReceived = 0;
			}

			MsgReply(rcvid, EOK, 0, 0);
		}

		/* Remove the name from the space */
		name_detach(attach, 0);
	};

	void radarClient(MPData MsgToSend){

		int server_coid; //server connection ID.
		if ((server_coid = name_open(MsgToSend.channelName.c_str(), 0)) == -1) {
			perror("Error occurred while attaching the channel");
		}

		if (MsgSend(server_coid, &MsgToSend, sizeof(MsgToSend), NULL,0) == -1) {
			printf("Error while sending the message from Client");
		}

	};

};
#endif
