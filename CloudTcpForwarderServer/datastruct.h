/** -------------------------------------------------------

File name:	app.h
Purpose:	data structure definiton
Version:	1.0		11/23/2017
Auther:		Ahmed Elaraby

---------------------------------------------------------*/

#pragma once

#include "app.h"
#include "tcp_server.h"

#define PACKET_HEADER	0x03

enum ePacket_Status {

	csFinshedSuccess=0,
	csUnknown,
	csNew,
	csUnderProcess,
	csNoResponce,
	csInvalidParameters,
	csError

};

struct sPacket {

	unsigned char	packet_index;
	int		 		packet_socket;
	char 			data[PACKET_DATA_SIZE];
	unsigned char 	data_length;
	enum ePacket_Status	status;
	unsigned char data_length_temp;

};


