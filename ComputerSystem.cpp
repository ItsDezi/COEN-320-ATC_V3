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
ComputerSystem::ComputerSystem(int periodicComputations): periodicComputations_(periodicComputations)
{
     dataDisplay = DataDisplay(); // initializing dataDisplay
}

//Destructor
ComputerSystem::~ComputerSystem(){
}

static void* ComputerSystem::ComputerSystemStartRoutine(void* args){


		ComputerSystem* computerSystemPointer = (ComputerSystem *) args;

		// Receive Data from Radar
		computerSystemPointer->ComputerSystemServer("RADAR - CS");

		return NULL;
	}

//CLIENT SIDE: FROM CS - DATADISPLAY
//Send data to DataDisplay
MPData MsgToSend;
MsgToSend.channelName = "CS - DDISP";
MsgToSend.allAircraftsInformation = allAircraftInformationPointer -> data;
allAircraftInformationPointer -> DDISP_Client(MsgToSend);

void ComputerSystem::computerSystemClient(MPData MsgToSend){

		int server_coid; //server connection ID.
		if ((server_coid = name_open(MsgToSend.channelName.c_str(), 0)) == -1) {
			perror("Error occurred while attaching the channel");
		}

		if (MsgSend(server_coid, &MsgToSend, sizeof(MsgToSend), NULL,0) == -1) {
			printf("Error while sending the message from Client");
		}

	};

#endif


//SERVER SIDE: RADAR - CS
void ComputerSystem::ComputerSystemServer(string channelName){

		// Variables
		ComputerSystem MsgToRecieve;

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


			// TODO: Change What to do after receiving a message
			cout << MsgToRecieve.aircraft.ID <<endl;
            cout << MsgToRecieve.aricraft.x << endl;
            cout << MsgToRecieve.aricraft.y << endl;
            cout << MsgToRecieve.aricraft.z << endl;
            cout << MsgToRecieve.aricraft.xSpeed << endl;
            cout << MsgToRecieve.aricraft.ySpeed << endl;
            cout << MsgToRecieve.aricraft.zSpeed << endl;
            cout << MsgToRecieve.aricraft.arrivalTime << endl;

            DDISP_Client(MsgToSend);

            //This is supposed to send a message to data display in case a violations occur.
            //not sure if this is correct
		    computerViolations(MsgToRecieve.aircraft);
			//TODO send it to computer system


			MsgReply(rcvid, EOK, 0, 0);
		}

		/* Remove the name from the space */
		name_detach(attach, 0);
	};

//Calculate future position and then pass to computeViolations to predict violations
void ComputerSystem::calculateFuturePosition(AircraftData* aircraft, double time_ ) {
    double speed_x = aircraft->xSpeed;
    double speed_y = aircraft->ySpeed;
    double speed_z = aircraft->zSpeed;

    //double time_ = 180; // time interval in seconds from t0 -> t0 + 180

    //Calculte new position based on the aircraft speed and variable time
    //use -> to access AircraftData struct
    //update the position of an aircraft based on its current time and speed.
    //takes current position and do addition to the multiplication of its speed and current time and store in position_x.
    aircraft->x = aircraft->x + speed_x * 180;
    aircraft->y = aircraft->y + speed_y * 180;
    aircraft->z = aircraft->z + speed_z * 180;
}


bool ComputerSystem::computeViolations(vector<AircraftData>& aircraft) {
    bool violationDetected = false;

        // Receive data from message passing channel:

 // to check for Violations: ( Check for constraints )
        for (int i = 0; i < aircraft.size(); i++) {
            for (int j = i + 1; j < aircraft.size(); j++) {

                // Calculate future position if t != current time based on speed (as vector and time)
                // as parameters and returns new position:
                calculateFuturePosition(&aircraft[i], 180);
                calculateFuturePosition(&aircraft[j], 180);

                //Retrieve updated positions from calculateFuturePosition function:
                double position_i_x = aircraft[i].x;
                double position_i_y = aircraft[i].y;
                double position_i_z = aircraft[i].z;
                double position_j_x = aircraft[j].x;
                double position_j_y = aircraft[j].y;
                double position_j_z = aircraft[j].z;

                double position_x = abs(aircraft[i].x - aircraft[j].x);
                double position_y = abs(aircraft[i].y - aircraft[j].y);
                double position_z = abs(aircraft[i].z - aircraft[j].z);
                double distance_x = sqrt((position_x * position_x));
                double distance_y = sqrt((position_y * position_y));
                double distance_z = sqrt((position_z * position_z));
                double distance_total = sqrt((position_x * position_x) + (position_y * position_y) + (position_z * position_z));

                if (position_x < 3000 || position_y < 3000 || position_z < 1000) {
                    violationDetected = true;
                    cout << endl;
                    cout << "WARNING! Violation detected between aircrat ID " << aircraft[i].ID << " and aircraft ID "<< aircraft[j].ID << endl;
                    cout << "Here is the required information:  " << endl;
                    cout << "* The distance between them is:  " << distance_total << endl;

                    //Show axis violation:
                    if (position_x < 3000) {
                        cout << "* Violation is on X-axis: The distance in the X-axis is: " << distance_x << endl;

                    }
                    if (position_y < 3000) {
                        cout << "* Violation is on Y-axis: The distance in the Y-axis is: " << distance_y << endl;

                    }
                    if (position_z < 1000) {
                        cout << "* Violation is detected on the Z-axis: The distance in the Z-axis is:  " << distance_z << endl;

                    }
                }
            }
        }

    return violationDetected;
}

//we are iterating over the time interval from  0 to 180 seconds
// and for each iteration we are calling the computeViolations function
// with the aircraft vector and the current time interval.
// If any violations occur in the next 180 sec the computeViolations function will take care of it.
void ComputerSystem::computeViolationsFor3Minutes(vector<AircraftData>& aircraft) {
    for (int i = 0; i < 180; i++) {
        // Check for violations in the next 180 seconds
        bool violationDetected = computeViolations(aircraft);

        if (violationDetected) {
            // If a violation is detected break out of the loop and exit the function
            cout << "Violation detected in 180 seconds" << endl;
            break;
        }

        // Calculate future positions for all aircraft
        for (int j = 0; j < aircraft.size(); j++) {
            calculateFuturePosition(&aircraft[j], 1);


        }
    }
}

