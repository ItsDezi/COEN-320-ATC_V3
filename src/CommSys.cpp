#ifndef COMMSYS_H_
#define COMMSYS_H_
#include <pthread.h>
#include "MPData.h"
#include <sys/dispatch.h>

class CommSys {
public:

	pthread_t thread;

	CommSys(){};
	virtual ~CommSys(){};

	static void* CommSysThreadRun(void * args){

		CommSys* commSys = (CommSys *) args;

		commSys-> commSysServer("CS-COMMSYS");
		return NULL;
	}

	void commSysServer(string channelName){

		// Variables
		MPData MsgToRecieve;

		name_attach_t *attach;
		int rcvid;


		if ((attach = name_attach(NULL, channelName.c_str(), 0)) == NULL) {
			perror("Error occurred while creating the channel commSysServer");
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


			// TODO: Change What to do after receiving a message from Computer System
			MPData MsgToSend = MsgToRecieve;
			MsgToSend.channelName = "COMMSYS-A" + to_string(MsgToSend.command.ID);
			CommSysClient(MsgToSend);





			MsgReply(rcvid, EOK, 0, 0);
		}

		/* Remove the name from the space */
		name_detach(attach, 0);
	};


	void CommSysClient(MPData MsgToSend){

		int server_coid; //server connection ID.
		if ((server_coid = name_open(MsgToSend.channelName.c_str(), 0)) == -1) {
			perror("Error occurred while attaching the channel CommSysClient");
		}

		if (MsgSend(server_coid, &MsgToSend, sizeof(MsgToSend), NULL,0) == -1) {
			printf("Error while sending the message from CommSysClient");
		}

	};



};

#endif /* COMMSYS_H_ */
