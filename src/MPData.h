#ifndef MPDATA_H_
#define MPDATA_H_

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include "AircraftData.h"
#include <vector>

using namespace std;


struct MPData{
	_pulse hdr;
	string channelName;
	AircraftData aircraft;
	vector<AircraftData> allAircrafts;
};

#endif
