#ifndef OPERATING_CONSOLE_H
#define OPERATING_CONSOLE_H

#include <string>
#include <vector>
#include "MPData.h"
#include "Command.h"

using namespace std;




class OperatingConsole {
public:
	pthread_t thread;
	OperatingConsole(){}
	void writeToFile(string fileName, Command command);
	static void* run(void* arguments);
	void OperatingConsoleClient(MPData MsgToSend);
};

//void OperatingConsoleClient(Command commandToSend);

#endif // OPERATING_CONSOLE_H
