/** -------------------------------------------------------

File name:	tcp_server.h
Purpose:	header for class (for more information view the class comments)
Version:	1.0		11/23/2017
Auther:		Ahmed Elaraby

---------------------------------------------------------*/

#pragma once

#include "app.h"
#include <stdio.h>
#include <mutex>            // lock object

#ifdef _WIN_
#include <winsock2.h>
#define read	recv
#define write	send
#define socklen_t	int
#else
#include <sys/socket.h>
#endif
#include <stdlib.h>


#define TRUE   1
#define FALSE  0

#define TCP_RESULT_SUCCESS		0
#define TCP_RESULT_REC_TIMEOUT	1
#define TCP_RESULT_REC_ERROR	2
#define TCP_RESULT_WRITE_ERROR	3
#define TCP_RESULT_INIT_BINDING_ERROR 		4
#define TCP_RESULT_INIT_OPEN_SOCKET_ERROR	5
#define TCP_RESULT_INIT_PORT_ERROR			6

class tcp_server
{
public:
	tcp_server(unsigned short _port, int * _agent_socket);
	~tcp_server();
	int active_client_index;
	uint8_t command_index;
	//char tcp_accept(void);
private:
	char tcp_init();
};

extern int client_socket[MAX_TCP_CLIENTS];
extern void tcp_send_respond(int i, bool _lock);
 