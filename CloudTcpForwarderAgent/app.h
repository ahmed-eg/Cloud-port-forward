/** -------------------------------------------------------

	File name:	app.h
	Purpose:	the main header file for all project, which include all common configurations and definitions
	Version:	1.0		11/23/2017
	Auther:		Ahmed Elaraby

---------------------------------------------------------*/

#pragma once

#define _WIN_	1	

#include <sys/types.h>
#include "..\CloudTcpForwarderServer\logs.h"
#include <mutex>          // std::mutex


// you can adjust the level of output messages by commenting the printf on eacg level
// debug is the mose important messages and debug3 is the less important
#define debug			printf
#define debug1			printf
#define debug2			//printf
#define debug3			//printf

#define PACKET_HEADER	0x03

extern std::mutex is_sending;
extern std::mutex access_lock; // access var
//extern std::mutex is_sending_agent;
extern unsigned char * agent_connected;
extern unsigned char * clients_connected;
extern unsigned char * data_sent_count;
extern unsigned char * data_rec_count;
extern void int32_to_array(int32_t _par, char * _array);
extern void add_sent(int _data);
extern void add_rec(int _data);

enum ePacket_Status {

	csFinshedSuccess=0,
	csUnknown,
	csNew,
	csUnderProcess,
	csNoResponce,
	csInvalidParameters,
	csError

};

#define BUFFER_SIZE				254
#define PACKET_DATA_SIZE		250

struct sPacket {

	unsigned char	packet_index;
	int		 		packet_socket;
	char 			data[PACKET_DATA_SIZE];
	unsigned char 	data_length;
	unsigned char 	data_length_temp;
	enum ePacket_Status	status;

};
