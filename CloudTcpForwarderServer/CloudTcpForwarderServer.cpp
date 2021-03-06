/** -------------------------------------------------------

File name:	CloudTcpForwarderAgent.cpp
Purpose:	main class for the application
Version:	1.0		11/23/2017
Auther:		Ahmed Elaraby

---------------------------------------------------------*/


#include "app.h"
#include "stdafx.h"
#include "stdio.h"
#include <string.h>

#include <stdio.h>
#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <string>

#include "datastruct.h"
#include "tcp_server_agent.h"
#include <Windows.h>

//#include <pthread.h>


//void *task1(void *);
tcp_server * srv;
tcp_server_agent * agent;

int agent_socket = -1;

int agent_port;
int client_port;

static int connFd;

std::mutex is_sending;
//std::mutex is_sending_agent;

DWORD WINAPI clientthread(__in LPVOID lpParameter)
{
	//printf("Thread inside %d \n", GetCurrentThreadId());
	srv = new tcp_server(client_port,&agent_socket);
	return 0;
}

DWORD WINAPI agentthread(__in LPVOID lpParameter)
{
	//printf("Thread inside %d \n", GetCurrentThreadId());
	agent = new tcp_server_agent(agent_port, &agent_socket);
	return 0;
}

int main(int args_count, char* args[])
{

	debug("started...\n");
	debug("start parse arguments..\n");

	agent_port = atoi(args[1]);
	client_port = atoi(args[2]);

	debug("Agent Listen port   %d\n",agent_port);
	debug("Client Listen port   %d\n", client_port);

	init_log(0);

	write_log("Started\n",0);

	HANDLE clienthandle;
	DWORD clientthreadid;

	HANDLE agenthandle;
	DWORD agentthreadid;

	clienthandle = CreateThread(0, 0, clientthread, 0, 0, &clientthreadid);
	agenthandle = CreateThread(0, 0, agentthread, 0, 0, &agentthreadid);
	
	for (;;) Sleep(20000); // just wait forever
	
	printf("closed!\n");

}
