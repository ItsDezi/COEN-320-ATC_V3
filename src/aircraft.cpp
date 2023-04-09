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
void aircraft::changeSpeed()//when commands are received from the operator console
{

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
