/** -------------------------------------------------------

File name:	tcp_server_agent.h
Purpose:	header for class (for more information view the class comments)
Version:	1.0		11/23/2017
Auther:		Ahmed Elaraby

---------------------------------------------------------*/

#pragma once
#include "app.h"
#include "tcp_server.h"

class tcp_server_agent
{
public:
	//int * agent_socket;
	tcp_server_agent(unsigned short _port, int * _agent_socket);
	~tcp_server_agent();
	//void tcp_send_agent(char * _array, int _len);
private:
	char tcp_init_agent();
};