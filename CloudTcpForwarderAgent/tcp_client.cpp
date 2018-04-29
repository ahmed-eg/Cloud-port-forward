/** -------------------------------------------------------

	File name:	tcp_client.cpp
	Purpose:	connect to destination device and forword the responce to the server via tcp_client_agent.cpp
	Version:	1.0		11/23/2017
	Auther:		Ahmed Elaraby

---------------------------------------------------------*/

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "tcp_client.h"
#include <stdlib.h>
#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include <stdint.h>

#pragma comment(lib, "Ws2_32.lib")

//#define request  "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.75 Safari/537.36\r\nUpgrade-Insecure-Requests: 1\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\nDNT: 1\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9,ar;q=0.8\r\nAlexaToolbar-ALX_NS_PH: AlexaToolbar/alx-4.0.1\r\n\r\n"

tcp_client::tcp_client(char * _ip_address, uint16_t _port, int _respond_socket)
{
	recthread_started = 0;
	port = _port;
	ip_address = _ip_address;
	respond_socket = _respond_socket;
	lock();
	is_closed = 0;
	*clients_connected++;
	tcp_init();
}

void error(const char *msg)
{
	//perror(msg);
	debug1(msg);
	//exit(0);
}

void send_agent(sPacket * _packet, int _socket) {

	if (_packet == nullptr) { debug2("send_agent: (Client) _packet is null\n"); return; }
	is_sending.lock();
	static char arr[BUFFER_SIZE];
	int n = 0;
	arr[0] = PACKET_HEADER;
	arr[1] = _packet->packet_index;
	arr[2] = _packet->data_length;
	arr[3] = (char)_packet->status;
	memcpy(&arr[4], &_packet->data[0], _packet->data_length);
	n= write(_socket, &arr[0], (_packet->data_length + 4), 0);
	add_sent(n);
	if (n < (_packet->data_length + 4))
		debug2("sent only %d/%d\n", n, (_packet->data_length + 4));
	//else debug2("s[%d] ", n);
	is_sending.unlock();
}

void send_close_command(unsigned char _client_index, int _socket) {
	//----------- when close connection, send close command
	sPacket * packet = new sPacket;
	packet->data_length = 0;
	packet->status = ePacket_Status::csFinshedSuccess;
	packet->packet_index = _client_index;
	packet->packet_socket = 0;
	send_agent(packet, _socket);
	debug2("send close connection command %d\n", _client_index);
	delete packet;
}

//sPacket * parse_packet(char * data, int len) {
//
//	if (data[0] == PACKET_HEADER) {
//		sPacket * p = new sPacket;
//		p->packet_index = (unsigned char)data[1];
//		p->data_length = (unsigned char)data[2];
//		p->status = (ePacket_Status)data[3];
//		memcpy(p->data, &data[4], len - 4);
//		return p;
//	}
//	else {
//		debug("xxxxxxxxxxxxxxxxx");
//		return nullptr;
//	}
//
//}

///=================================================
/// read from client endpoint and send for server
///=================================================
DWORD WINAPI recthread(__in LPVOID lpParameter)
{
	//sPacket * packet;
	int n;
	tcp_client * current = (tcp_client*)lpParameter;
	char buffer[BUFFER_SIZE];
	if (current->recthread_started)return 0;
	current->recthread_started = 1;
	int rec_n = 0;
	while (!current->get_is_closed()) {
		
		n = read(current->sockfd, &buffer[4], PACKET_DATA_SIZE, 0);
		
		// check if no 
		if (n <= 0) //SOCKET_ERROR)
		{
			current->set_is_closed(1); // marke the socket as closed so it will terminate the client object
			break;
		}
		else {

			is_sending.lock();
			buffer[0] = PACKET_HEADER;
			buffer[1] = current->client_index;
			buffer[2] = (unsigned char)n;
			buffer[3] = (char)ePacket_Status::csNew;
			//memcpy(&arr[4], &_packet->data[0], _packet->data_length);
			rec_n = write(current->respond_socket, &buffer[0], (n + 4), 0);
			add_rec(rec_n);
			if (rec_n < (n + 4))
				debug2("sent only %d/%d\n", rec_n, (n + 4));
			//else debug2("s[%d] ", n);
			is_sending.unlock();

			/*packet = new sPacket;
			packet->data_length = (char)n;
			packet->status = ePacket_Status::csNew;
			packet->packet_index = client_index;
			packet->packet_socket = 0;
			memcpy(&packet->data[0], &buffer[0], n);

			send_agent(packet);*/

			//write(respond_socket, &buffer[0], n, 0);
			//buffer[n] = 0;
			//debug(&buffer[0]);
			debug3("------ client rec length %d - %d\n", n, rec_n);

			//delete packet;
		}
		//}
	}
	current->set_is_closed(1);
	
	send_close_command(current->client_index,current->respond_socket);

	//current->unlock();
	debug2("colsed (recthread) %d\n", current->client_index);
	//CloseHandle(recthandle0);
	return 0;
}

void tcp_client::tcp_init(void) {
		
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	/////////////////////////////////////////////////////
	///   Error handler
	/////////////////////////////////////////////////////
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		debug("WSAStartup failed with error: %d\n", err);
		return ;
	}
	/////////////////////////////////////////////////////
	
	//portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket\n");
	server = gethostbyname(ip_address);
	if (server == NULL) {
		//fprintf(stderr, "ERROR, no such host\n");
		return;
	}
	//bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	//bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
	serv_addr.sin_port = htons(port);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting\n");
	
	//printf("Please enter the message: ");
	//bzero(buffer, 256);
	//fgets(buffer, 255, stdin);
	//n = write(sockfd, request, strlen(request), 0);
	//if (n < 0)
	//	error("ERROR writing to socket");
	//bzero(buffer, 256);
	
	//DWORD WINAPI recthread(__in LPVOID lpParameter)
	//
	recthandle0 = CreateThread(0, 0, recthread, this, 0, &recthreadid0);
	
	//return 0;

}

void tcp_client::tcp_write_packet(sPacket * p) {
	
	if (p == nullptr) { debug2("tcp_write_packet: p is null"); return; }

	if (p->status == ePacket_Status::csNew) {
		client_index = p->packet_index;
		is_sending.lock();
		int	n0 = write(sockfd, p->data, p->data_length, 0);
		is_sending.unlock();
		add_rec(n0);
		if (n0 < 0) {
			is_closed = 1;
			//send_close_command();
			unlock();
			debug2("ERROR writing to socket (client tcp_write_packet) %d\n", n0);
		}
		else //debug("sent %d\n", _len);
		{
			//_req[_len] = 0;
			//debug(_req);
			debug3("--------- client sent %d\n", n0);
		}
	}
	else if (p->status == ePacket_Status::csFinshedSuccess) {
		shutdown(sockfd,0);
		closesocket(sockfd);
		is_closed = 1;
	}
	else {
		debug3("packet status is incorrect\n");
	}

}

void tcp_client::tcp_write(char * _req, int _len) {
	//write(sockfd, request, strlen(request), 0);
	//return;
	is_sending.lock();
	int	n0 = write(sockfd, _req, _len, 0);
	is_sending.unlock();
	add_rec(n0);
	if (n0 < 0) {
		
		is_closed = 1;
		//send_close_command();
		unlock();
		debug2("ERROR writing to socket (client tcp_write) %d\n", n0);
	}
	else //debug("sent %d\n", _len);
	{
		//_req[_len] = 0;
		//debug(_req);
		debug3("--------- client sent %d\n", _len);
	}
}

char tcp_client::get_is_closed() {
	//if (this < (void*)0xffff) {	return 1; }
	if (!this) { return 1; }
	//if (&is_closed == nullptr) { return 1; }
	//char _busy;
	//access_lock.lock();
	return is_closed;
	//access_lock.unlock();
	//return _busy;
}

void tcp_client::set_is_closed(char _value) {
	
	//if (this < (void*)0xffff) { return; }
	if (!this) { return ; }
	//if (&is_closed == nullptr) return;
	//access_lock.lock();
	is_closed = _value;
	//access_lock.unlock();

}

tcp_client::~tcp_client()
{
	//if (this < (void*)0xffff) {	return ;}
	*clients_connected--;
	if (!this ) { return ; }	
	debug2("Dispose client class\n");
	//send_close_command();
	if (&recthandle0 != nullptr) {
		TerminateThread(recthandle0, 0); // Dangerous source of errors!
		CloseHandle(recthandle0);
	}
	//closesocket(sockfd);
	if (&sockfd != nullptr) {
		shutdown(sockfd, 0);
	}
}
