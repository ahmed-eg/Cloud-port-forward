/** -------------------------------------------------------

	File name:	tcp_client_agent.h
	Purpose:	header for class
	Version:	1.0		11/23/2017
	Auther:		Ahmed Elaraby

---------------------------------------------------------*/

#pragma once

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "tcp_client.h"
#include <stdlib.h>
#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include <stdint.h>

#pragma comment(lib, "Ws2_32.lib")

class tcp_client_agent
{
public:
	tcp_client_agent(char * _ip_address, uint16_t _port, char * _endpoing_server, uint16_t _endpoint_port);
	~tcp_client_agent();
private:
	void tcp_init_agent(void);
	char *		agent_server;
	uint16_t	agent_port;
	char *		endpoing_server;
	uint16_t	endpoint_port;
};

