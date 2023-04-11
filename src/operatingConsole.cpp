#include "operatingConsole.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <unistd.h>



void OperatingConsole::writeToFile(string fileName, Command command){

	// Write data to output file
	ofstream oFile(fileName);

	//string commandReadOrChange;

	//string commandattributes;

	//string commandString;
	string newValue;

	// Write to the file

		string commandReadOrChange;
		string commandAttributes;
		string commandString;

		if(command.readOrChange == 0)
			commandReadOrChange = "Read";
		else if(command.readOrChange == 1)
			commandReadOrChange = "change";
		else
			commandReadOrChange = "Error no read nor change";


		if(command.attributes == 1)
			commandAttributes = "x speed";
		else if(command.attributes == 2)
			commandAttributes = "y speed";
		else if(command.attributes == 3)
			commandAttributes = "z speed";
		else
			commandAttributes = "Error: unknown attribute";


		if (command.readOrChange == 0)
			commandString = "ID: " + to_string(command.ID) + ", Command: " + commandReadOrChange;

		else if (command.readOrChange == 1)
			commandString = "ID: " + to_string(command.ID) + ", Command: " + commandReadOrChange + ", Attribute: " + commandAttributes + "new value: " + to_string(command.newValue) ;


		oFile <<commandString<< endl;


	// Close the file
	oFile.close();

}

void *OperatingConsole::run( void *arguments) {

	OperatingConsole *OCPointer = (OperatingConsole *)arguments;

	Command command;
	// Read input from the user
	cout << "Enter 0 to read object, 1 to change object: "<< endl;
	cin >> command.readOrChange;

	cout << "Enter aircraft ID: "<< endl;;
	cin >> command.ID;

	if (command.readOrChange == 1) {
		cout << "Enter a number of the attribute to read or change:\n" << endl;;
		cout << "1 - x"<< endl;;
		cout << "2 - y"<< endl;;
		cout << "3 - z"<< endl;;
		cin >> command.attributes;
		cout << "Enter new value: "<<endl;
		cin >> command.newValue;
	}


	//Write the command to log file
	string fileName = "Commands_Log_File.txt";
	OCPointer->writeToFile(fileName, command);

	// Create message to send to server (CS - Handling Commands Thread)
	MPData MsgToSend;
	MsgToSend.channelName = "OC-CS";
	MsgToSend.command = command;
	OCPointer->OperatingConsoleClient(MsgToSend);


	pthread_exit(NULL);
	return NULL;

};


void OperatingConsole::OperatingConsoleClient(MPData MsgToSend){

	int server_coid; //server connection ID.  // change channel name
	if ((server_coid = name_open(MsgToSend.channelName.c_str(), 0)) == -1) {
		perror("Error occurred while attaching the channel");
	}

	if (MsgSend(server_coid, &MsgToSend, sizeof(MsgToSend), NULL,0) == -1) {
		printf("Error while sending the message from Client");
	}


}
