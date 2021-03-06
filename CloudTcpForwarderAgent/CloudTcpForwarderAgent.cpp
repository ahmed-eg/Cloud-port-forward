/** -------------------------------------------------------

	File name:	CloudTcpForwarderAgent.cpp
	Purpose:	main class for the application
	Version:	1.0		11/23/2017
	Auther:		Ahmed Elaraby

---------------------------------------------------------*/

#include "stdafx.h"
#include "tcp_client.h"
#include "tcp_client_agent.h"
#include <Windows.h>
#include "app.h"

char* agent_server;
int agent_port;
char* endpoint_server;
int endpoint_port;

unsigned char * agent_connected;
unsigned char * clients_connected;
unsigned char * data_sent_count;
unsigned char * data_rec_count;
int32_t  _data_sent_count=0;
int32_t _data_rec_count=0;

std::mutex is_sending;
std::mutex access_lock;
//std::mutex is_sending_agent;

DWORD WINAPI clientthread(__in LPVOID lpParameter)
{
	for (;;) {
		//try to connect to server
		tcp_client_agent client_agent(agent_server, agent_port, endpoint_server, endpoint_port);
		client_agent.~tcp_client_agent(); // the code will stop here untill it's disconnected
		// loop to try to connect to server again as agent
	}
	return 0;
}

//=====================
//==  saving some configuration in memory to pass information 
//    bewteen the Agent project (c++) 
//    and the ManagmentAgent (c#)
//=====================
#define BUF_SIZE 1024
//TODO: make the file path dynamic
TCHAR szFile[] = TEXT("C:\\OneDrive\\Documents\Visual Studio 2017\\Projects\\Cloud\\Debug\\Mem.mp"); 
TCHAR szName[] = TEXT("CloudAgent");
TCHAR szMsg[] = TEXT("Message from first process.");
unsigned char szCount = 20;
void mem_access(void);

int main(int args_count, char* args[])
{
	HANDLE clienthandle;
	DWORD clientthreadid;
	
	// init the event log for debuggin only
	init_log(1);
	
	//initilize the memory access
	mem_access();

	// make sure that the application called by 4 arguments 
	// server name & server port & client IP/name & client port
	if (args_count >= 4) {
		
		debug("Trying to parse arguments %d \n", args_count);
		agent_server = new char[strlen(args[1])+1];
		memcpy(agent_server, args[1], strlen(args[1]));
		agent_server[strlen(args[1])] = 0;
		agent_port = atoi(args[2]);

		endpoint_server = new char[strlen(args[3])+1];
		memcpy(endpoint_server, args[3], strlen(args[3]));
		endpoint_server[strlen(args[3])] = 0;
		endpoint_port = atoi(args[4]);

		debug("Agent Server = %s    Port = %d\n", agent_server, agent_port);
		debug("Endpoint Server = %s    Port = %d\n", endpoint_server, endpoint_port);

		clienthandle = CreateThread(0, 0, clientthread, 0, 0, &clientthreadid);
	}
	else { debug("Invalid Arguments !! \n"); }

	for (;;) Sleep(20000); // just wait forever
    return 0;
}

void mem_access(void) {
	HANDLE hMapFile;
	unsigned char* pBuf;

	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		BUF_SIZE,                // maximum object size (low-order DWORD)
		szName);                 // name of mapping object

	if (hMapFile == NULL)
	{
		debug("Error ............\n");
		_tprintf(TEXT("Could not create file mapping object (%d).\n"),
			GetLastError());
		return ;
	}
	pBuf = (unsigned char*)MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		BUF_SIZE);

	if (pBuf == NULL)
	{
		debug("Error ............\n");
		_tprintf(TEXT("Could not map view of file (%d).\n"),
			GetLastError());
		CloseHandle(hMapFile);
		return ;
	}

	pBuf[0] = szCount;
	
	agent_connected = &pBuf[0];
	clients_connected =&pBuf[1];
	data_sent_count= &pBuf[2];
	data_rec_count= &pBuf[6];

	*data_sent_count = 0;
	*data_rec_count = 0;
	_data_sent_count = 0;
	_data_rec_count = 0;

	*agent_connected = 0;
	*clients_connected = 0;
	
	debug("mapping done successfully with c# project       :)\n");
	return ;
}

void int32_to_array(int32_t _par, char * _array)
{

	_array[0] = _par & 0x000000ff;
	_array[1] = (_par & 0x0000ff00) >> 8;
	_array[2] = (_par & 0x00ff0000) >> 16;
	_array[3] = (_par & 0xff000000) >> 24;

	/*for (int i = 0; i < 4; i++)
		_array[3 - i] = (_par >> (i * 8));*/
}

void add_sent(int _data) {
	_data_sent_count += _data;
	int32_to_array(_data_sent_count, (char*)data_sent_count);
}

void add_rec(int _data) {
	_data_rec_count += _data;
	int32_to_array(_data_rec_count, (char*)data_rec_count);
}
