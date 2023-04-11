//#include "aircraft.h"
#include <iostream>
#include <string>
#include <string.h>
//#include <pthread>
#include "aircraft.h"
#include "MPData.h"
#include <chrono>
//extern int sigwait(const sigset_t *__set, int *__sig);

using namespace std;


int arrivalTime, x ,y ,z , xSpeed, ySpeed, zSpeed;

aircraft::aircraft() {
	// TODO Auto-generated constructor stub
	//m_thread = std::thread(&Clock::run, this);
}
aircraft::aircraft(AircraftData datating)
{
	//const char* ccx = data.ID.c_str();
	//pthread_setname_np(this->thread, ccx);
	data = datating;
	//clk = &clock_in;
	prevClock = datating.arrivalTime;
	//m_thread = thread(&aircraft::run);
}

aircraft::~aircraft() {
	// TODO Auto-generated destructor stub
}

void aircraft::test_print()
{
	//cout<<"Arrival Time: "<< data.arrivalTime<<endl;
	cout<<"ID: "<< data.ID<<endl;
	cout<<"X: "<< data.x<<endl;
	cout<<"Y: "<< data.y<<endl;
	cout<<"Z: "<< data.z<<endl;
	cout<<"X Speed: "<< data.xSpeed<<endl;
	cout<<"Y Speed: "<< data.ySpeed<<endl;
	cout<<"Z Speed: "<< data.zSpeed<<endl;
}
void* aircraft::updatePosition(void* args)//
{
	aircraft* aircraftPointer = (aircraft*) args;

	cTimer clk = cTimer(1,0);
	//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	aircraftPointer->arrived = true;
	while(true)
	{
		//wait for signal from timer

		cout<<"\nAircraft "<< aircraftPointer->data.ID << " is calling updatePositon at "<<clk.count<<" seconds(relative)"<<endl;

		aircraftPointer->data.x += (aircraftPointer->data.xSpeed);
		aircraftPointer->data.y += (aircraftPointer->data.ySpeed);
		aircraftPointer->data.z += (aircraftPointer->data.zSpeed);
		//	aircraftPointer->test_print();


		// Sending data to Radar
		MPData MsgToSend;
		MsgToSend.channelName = "AC-RADAR";
		MsgToSend.aircraft = aircraftPointer->data;
		cout<<"\nAircraft "<< aircraftPointer->data.ID << " Sent to Radar"<<endl; //TORemove: remove after testing
		aircraftPointer->AC_Client(MsgToSend);


		clk.waitTimer();

	}
}
struct thread_args {    /* Used as argument to the start routine thread_start() */
	int period_sec;    //desired period of the thread in seconds
	int period_msec;   //desired period of the thread in milliseconds
};
void aircraft::changeSpeed(int whichSpeed, int newValue)//when commands are received from the operator console
{

	if(whichSpeed == 1)
		{
		cout <<"AC " << data.ID << " Old X Speed is" << data.xSpeed << endl;
		data.xSpeed = newValue;
		cout <<"AC " << data.ID << " New X Speed is" << data.xSpeed << endl;
	}

	else if (whichSpeed == 2)
	{
		cout <<"AC " << data.ID << " Old Y Speed is" << data.ySpeed << endl;
		data.ySpeed = newValue;
		cout <<"AC " << data.ID << " New Y Speed is" << data.ySpeed << endl;
	}
	else if(whichSpeed == 3)
	{
		cout <<"AC " << data.ID << " Old Z Speed is" << data.zSpeed << endl;
		data.zSpeed = newValue;
		cout <<"AC " << data.ID << " New Z Speed is" << data.zSpeed << endl;
	}


}

void aircraft::AC_Client(MPData MsgToSend){

	int server_coid; //server connection ID.
	if ((server_coid = name_open(MsgToSend.channelName.c_str(), 0)) == -1) {
		perror("Error occurred while attaching the channel");
	}

	if (MsgSend(server_coid, &MsgToSend, sizeof(MsgToSend), NULL,0) == -1) {
		printf("Error while sending the message from Client");
	}

}
void* aircraft::run(void *a)
{
	while(1 != 0)
	{
		//updatePosition(a);
	}
}


// RECEIVING COMMANDS FROM COMMUNICATION SYSTEMS

void* aircraft::ReceivingCommandsRoutine(void* args){

	aircraft * aircraftPointer = (aircraft *) args;

	// receives command from Operator Console
	string channelName = "COMMSYS-A" + to_string(aircraftPointer->data.ID);
	aircraftPointer ->ReceivingCommandsServer(channelName);

	return NULL;
}

void aircraft::ReceivingCommandsServer(string channelName){

	// Variables
	MPData MsgToRecieve;

	name_attach_t *attach;
	int rcvid;


	if ((attach = name_attach(NULL, channelName.c_str(), 0)) == NULL) {
		perror("Error occurred while creating the channel TestAircraftServer");
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


		// TODO: Change What to do after receiving a message from Communication System
		// Change speed function
		changeSpeed(MsgToRecieve.command.attributes,MsgToRecieve.command.newValue);


		MsgReply(rcvid, EOK, 0, 0);
	}

	/* Remove the name from the space */
	name_detach(attach, 0);
};




