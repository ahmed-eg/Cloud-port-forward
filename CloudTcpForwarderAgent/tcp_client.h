/** -------------------------------------------------------

   File name:	tcp_client.h
   Purpose:		header for class
   Version:		1.0		11/23/2017
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
#include <mutex> 

class tcp_client
{
public:
	tcp_client(char * _ip_address, uint16_t _port, int _respond_socket);
	void tcp_write_packet(sPacket * p);
	void lock() {};// { is_busy.lock(); } //TODO: re-enable lock and unlock functions
	void unlock() {};// { is_busy.unlock(); }
	~tcp_client();
	char recthread_started = 0;	
	char get_is_closed();
	void set_is_closed(char _value);
	unsigned char client_index;
	int respond_socket;
	int sockfd;
private:
	void tcp_write(char * _req, int _len);
	void tcp_init(void );
	std::mutex is_busy;
	char * ip_address;
	uint16_t port;	
	HANDLE recthandle0;
	DWORD recthreadid0;
	char is_closed = 0;
	
};
