#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "aircraft.h"
#include <list>
#include <vector>
#include <fstream>
#include "Radar.cpp"
#include "cTimer.h"
#include "ComputerSystem.h"
#include "operatingConsole.h"
#include "CommSys.cpp"


using namespace std;

// Main Global variables
vector<aircraft> aircrafts;
int arrivedAircraftsCount = 0;

struct thread_args {    /* Used as argument to the start routine thread_start() */
	int period_sec;    //desired period of the thread in seconds
	int period_msec;   //desired period of the thread in milliseconds
	Radar* RadarPointer;
	ComputerSystem* ComputerSystemPointer;
	CommSys* CommuncationSystemPointer;
	OperatingConsole* OperatorConsolePointer;


};
vector<aircraft> read_input();
aircraft create_aircraft(string s);



void *thread_start (void *arg) {
	struct thread_args *targs = ( struct thread_args * ) arg;
	//create radar,computer system, etc threads here

	int period_sec=targs->period_sec;
	int period_msec=targs->period_msec;
	Radar ATCRadar = (* targs->RadarPointer);
	ComputerSystem ATCComputerSystem = (* targs-> ComputerSystemPointer);
	CommSys ATCCommunicationSystem = (*targs-> CommuncationSystemPointer);
	OperatingConsole ATCOperatorConsole = (*targs->OperatorConsolePointer);

	int arrival_index = 0;//goes through vector of aircrafts and marks where in the ordered list the program should start aircraft threads

	int count = 0;
	cTimer timer(period_sec,period_msec); //initialize, set, and start the 1 second timer
	read_input();
	cout<<"\nSIZE:"<<aircrafts.size()<<endl;

	pthread_attr_t attr;
	/* Initialize attributes */
	int err_no;
	err_no = pthread_attr_init(&attr);
	if (err_no!=0)
		printf("ERROR from pthread_attr_init() is %d \n", err_no);
	err_no = pthread_attr_setschedpolicy(&attr,SCHED_SPORADIC);
	if (err_no!=0)
		printf("ERROR from pthread_attr_setschedpolicy() is %d \n", err_no);

	//====================================CS Thread Create =========================================================
	//=====================================================================================================
	err_no = pthread_create(&ATCComputerSystem.detectViolationsThread, &attr, ComputerSystem::ComputerSystemDetectViolationsRoutine, &ATCComputerSystem); //create the thread
	if (err_no != 0){
		printf("ERROR when creating the ATC Computer System detect Violations thread \n");
	}
	// Wait to create RADAR-CS channel
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	//====================================RADAR Thread Create =========================================================
	//=====================================================================================================
	err_no = pthread_create(&ATCRadar.thread, &attr, Radar::radarStartRoutine, &ATCRadar); //create the thread
	if (err_no != 0){
		printf("ERROR when creating the ATC Radar thread \n");
	}
	// Wait to create AC-RADAR channel
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	//====================================COMMUNICATION SYSTEM Thread Create =========================================================
	//=====================================================================================================
	err_no = pthread_create(&ATCCommunicationSystem.thread, &attr, CommSys::CommSysThreadRun, &ATCCommunicationSystem); //create the thread
	if (err_no != 0){
		printf("ERROR when creating the ATC Communication System thread \n");
	}
	//Wait to create COMMSYS-A# channel;
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	//====================================CS Handling CommandsThread  Create =========================================================
	//=====================================================================================================
	err_no = pthread_create(&ATCComputerSystem.handlingCommandsThread, &attr, ComputerSystem::HandlingCommands, &ATCComputerSystem); //create the thread
	if (err_no != 0){
		printf("ERROR when creating the ATC Computer System Handling Commands thread \n");
	}
	//Wait to create CS-COMMSYS channel;
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	//====================================OC Thread  Create =========================================================
	//=====================================================================================================
	err_no = pthread_create(&ATCOperatorConsole.thread, &attr, OperatingConsole::run, &ATCOperatorConsole); //create the thread
	if (err_no != 0){
		printf("ERROR when creating the ATC Operator Console thread \n");
	}
	//Wait to create OC-CS channel;
	std::this_thread::sleep_for(std::chrono::milliseconds(100));


	while(true){
		cout << "==============================================START CLK CYCLE============================================================"<< endl;
		cout<<"\nCLK time: "<<timer.count*1000<<" ms"<<endl;

		//====================================AC MAIN =========================================================
		//=====================================================================================================
		if((arrival_index < aircrafts.size() && aircrafts[arrival_index].data.arrivalTime <= (timer.count*1000)) && (aircrafts[arrival_index].arrived == false ))
		{
			int err;
			cout<<"\n aircraft "<<aircrafts[arrival_index].data.ID<<" thread is being created "<<"at "<<timer.count<<endl;
			arrivedAircraftsCount++; // Needed For Radar to know how many aircrafts will receive messages
			//---------------------------------------Update Position Thread---------------------------------------
			err = pthread_create(&aircrafts[arrival_index].thread, &attr, aircraft::updatePosition, &aircrafts[arrival_index]);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			// --------------------------------------Receiving Commands Thread-------------------------------------
			err_no = pthread_create(&aircrafts[arrival_index].receivingCommandsThread, &attr, aircraft::ReceivingCommandsRoutine, &aircrafts[arrival_index]); //create the thread
			if (err_no != 0){
				printf("ERROR when creating the Aircraft 2nd thread \n");
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			arrival_index++;

			if (err != 0){
				printf("ERROR when creating the aircraft thread \n");
			}
		}
		cout << "===============================================END CLK CYCLE=============================================================="<< endl;
		timer.waitTimer();

	}//end_while


	return NULL;
}//end_thread_start()



int main (int argc, char* argv[]) {

	// Initializing ATC Modules
	Radar ATCRadar = Radar(&arrivedAircraftsCount);
	ComputerSystem ATCComputerSystem = ComputerSystem();
	CommSys ATCCommuncationSystem = CommSys();
	OperatingConsole ATCOperatorConsole = OperatingConsole();

	//input arguments of the thread_start routine
	struct thread_args targs;
	targs.period_sec=1;
	targs.period_msec=0;
	targs.RadarPointer = &ATCRadar;
	targs.ComputerSystemPointer = &ATCComputerSystem;
	targs.CommuncationSystemPointer = &ATCCommuncationSystem;
	targs.OperatorConsolePointer = &ATCOperatorConsole;


	pthread_t thread_id;//ID of the thread
	int err_no;
	err_no = pthread_create(&thread_id, NULL, &thread_start, &targs); //create the thread
	if (err_no != 0){
		printf("ERROR when creating the thread \n");
	}
	//-----------------------------
	for(int i = 0; i<aircrafts.size();i++)
	{
		int err_no = pthread_join(aircrafts[i].thread, NULL); //force the main to wait for the termination of the thread
		if (err_no != 0){
			printf("ERROR when joining the thread\n");
		}
		arrivedAircraftsCount--;
	}
	//--------------------------------------
	err_no = pthread_join(thread_id, NULL); //force the main to wait for the termination of the thread
	if (err_no != 0){
		printf("ERROR when joining the thread\n");
	}
	pthread_exit(EXIT_SUCCESS);
}

vector<aircraft> read_input()
{
	string line,s;
	ifstream myfile;
	myfile.open("input.txt");
	aircraft a;
	//create_aircraft("hello,");
	getline(myfile,line);//to ignore first line
	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
		{
			//cout << line << '\n';
			a = create_aircraft(line);
			//cout<<"\n----------------------\n"<< a.data.ID<<"\n---------------------\n";

			aircrafts.push_back(a);
		}
		myfile.close();
	}

	else cout << "Unable to open file";
	cout<<"\nBefore sort: ";
	for(int i = 0; i< aircrafts.size();i++)
	{
		sleep(0.2);
		cout<<"\nID: "<<aircrafts[i].data.ID<< "\n";
	}
	sort( aircrafts.begin(), aircrafts.end(), []( aircraft &a, aircraft &b){ return (a.data.arrivalTime < b.data.arrivalTime);});
	cout<<"\nFront of aircrafts vector: "<<aircrafts.front().data.ID;

	cout<<"\nAfter sort: ";
	for(int i = 0; i< aircrafts.size();i++)
	{
		sleep(0.2);
		cout<<"\nID: "<<aircrafts[i].data.ID<< "\n";
	}

}

aircraft create_aircraft(string s)
{
	int ID;

	int time, x, y, z, speedX, speedY, speedZ;
	list<int> comma_locations;
	s.erase(std::remove_if(s.begin(), s.end(), ::isspace),
			s.end());
	for(int i=0;i<s.length();i++)
	{
		//cout<<s[i];
		char c = s[i];

		//int res = c.compare(comma);
		if(c==',')
		{
			comma_locations.push_front(i);
		}
	}
	time = stoi(s.substr(0,comma_locations.back()));
	int temp = comma_locations.back();
	//cout<<'\n'<<"time: "<<time;

	comma_locations.pop_back();

	int len = comma_locations.back()-temp-1;
	ID = stoi(s.substr(temp+1, len));
	//cout<<'\n'<<"ID: "<<ID;
	temp = comma_locations.back();

	comma_locations.pop_back();

	len = comma_locations.back()-temp-1;
	x = stoi(s.substr(temp+1, len));
	//cout<<'\n'<<"X: "<<x;
	temp = comma_locations.back();

	comma_locations.pop_back();

	len = comma_locations.back()-temp-1;
	y= stoi(s.substr(temp+1, len));
	//cout<<'\n'<<"Y: "<<y;
	temp = comma_locations.back();

	comma_locations.pop_back();

	len = comma_locations.back()-temp-1;
	z = stoi(s.substr(temp+1, len));
	//cout<<'\n'<<"Z: "<<z;
	temp = comma_locations.back();

	comma_locations.pop_back();

	len = comma_locations.back()-temp-1;
	speedX = stoi(s.substr(temp+1, len));
	//cout<<'\n'<<"Speed X: "<<speedX;
	temp = comma_locations.back();

	comma_locations.pop_back();

	len = comma_locations.back()-temp-1;
	speedY= stoi(s.substr(temp+1, len));
	//cout<<'\n'<<"Speed Y: "<<speedY;
	temp = comma_locations.back();

	comma_locations.pop_back();

	len = comma_locations.back()-temp-1;
	speedZ = stoi(s.substr(temp+1, len));
	//cout<<'\n'<<"Speed Z: "<<speedZ;
	temp = comma_locations.back();

	AircraftData d;
	d = (AircraftData){.ID = ID, .arrivalTime = time, .x = x , .y = y , .z = z , .xSpeed = speedX, .ySpeed = speedY, .zSpeed = speedZ};
	aircraft a = aircraft(d);

	return a;
}
