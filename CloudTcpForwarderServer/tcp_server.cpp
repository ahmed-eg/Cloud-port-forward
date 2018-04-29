/** -------------------------------------------------------

File name:	tcp_client.cpp
Purpose:	linstning for any connection and connect it to any avibale agent
Version:	1.0		11/23/2017
Auther:		Ahmed Elaraby

---------------------------------------------------------*/

#include "stdafx.h"
#include "tcp_server.h"
#include "datastruct.h"
#include <stdlib.h>
#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include <stdint.h>

#pragma comment(lib, "Ws2_32.lib")

uint16_t port;
int sockfd;

uint8_t command_index = 0;

int active_client_index =-1;
sPacket* tcp_packets[MAX_TCP_CLIENTS];
//sCommand * tcp_commands[MAX_TCP_CLIENTS];
//std::mutex tcp_commands_lockers[MAX_TCP_CLIENTS];

//set of socket descriptors
fd_set readfds;
int client_socket[MAX_TCP_CLIENTS];//max_clients = 30;
int * agent_socket_s;
//void * get_in_addr(struct sockaddr * sa)
//{
//	if (sa->sa_family == AF_INET)
//	{
//		return &(((struct sockaddr_in *)sa)->sin_addr);
//	}
//
//	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
//}

void send_agent(sPacket * _packet);
void send_close_connection(int i);

tcp_server::tcp_server(unsigned short _port, int * _agent_socket)
{
	port = _port;
	agent_socket_s = _agent_socket;
	tcp_init();
}

tcp_server::~tcp_server()
{
	//close(sockfd);
}

char tcp_server::tcp_init() {
	// Listen connection without thread
	int opt = TRUE;
	int master_socket, addrlen, new_socket,
		activity, i, valread, sd;
	sPacket * packet;
	int max_sd;
	struct sockaddr_in address;
	int16_t rec = 0;
	char data[BUFFER_SIZE];

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
		printf("WSAStartup failed with error: %d\n", err);
		return 1;
	}
/////////////////////////////////////////////////////
	
	//char buffer[1025];  //data buffer of 1K

	//a message
	//char *message = "ECHO Daemon v1.0 \r\n";

	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < MAX_TCP_CLIENTS; i++)
	{
		client_socket[i] = 0;
		//tcp_commands[i] = nullptr;
	}

	
	//create a master socket
	if ((master_socket = socket(AF_INET, SOCK_STREAM,  0)) <= 0)
	{
		const DWORD last_error = WSAGetLastError();
		debug1("socket failed %d", (int)master_socket);
		//debugl(last_error);
		return 0;
		//exit(EXIT_FAILURE);
	}

	
	int res = setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt));
	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	if (res < 0)
	{
		debug1("setsockopt %d",res);
		return 0;
		//exit(EXIT_FAILURE);
	}

	//type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	//bind the socket to localhost port 8888
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		debug1("bind failed\n");
		//exit(EXIT_FAILURE);
	}
	debug("Listener on port %d \n", port);

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(master_socket, 3) < 0)
	{
		debug1("listen\n");
		//exit(EXIT_FAILURE);
	}

	//accept the incoming connection
	addrlen = sizeof(address);
	debug("Waiting for connections ...\n");

	for (;;)
	{
		//clear the socket set
		FD_ZERO(&readfds);

		//add master socket to set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;

		//add child sockets to set
		for (i = 0; i < MAX_TCP_CLIENTS; i++)
		{
			//socket descriptor
			sd = client_socket[i];

			//if valid socket descriptor then add to read list
			if (sd > 0)
				FD_SET(sd, &readfds);

			//highest file descriptor number, need it for the select function
			if (sd > max_sd)
				max_sd = sd;
		}

		
		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0))//&& (errno!=EINTR)
		{
			//debugl("select error");
		}

		//If something happened on the master socket ,
		//then its an incoming connection
		if (FD_ISSET(master_socket, &readfds))
		{

			if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
			{
				//debug1("accept");
				//exit(EXIT_FAILURE);
			}

			//inform user of socket number - used in send and receive commands
			//sprintf(msg, "New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs
			//		(address.sin_port));
			//debugl(msg);
			//send new connection greeting message
			//if( send(new_socket, message, strlen(message), 0) != strlen(message) )
			//{
			//	debugl("send");
			//}

			//debugl("Welcome message sent successfully");

			char currentIndex = -1;
			//add new socket to array of sockets
			for (i = 0; i < MAX_TCP_CLIENTS; i++)
			{
				//if position is empty
				if (client_socket[i] == 0)
				{
					debug2("New Client %d         index    %d\n", new_socket, i);
					client_socket[i] = new_socket;
					currentIndex = i;
					//sprintf(msg,"Adding to list of sockets as %d\n" , i);
					//debugl(msg);
					break;
				}
			}

			if (currentIndex < 0) {
				for (i = 0; i < MAX_TCP_CLIENTS; i++)
				{
					//if position is empty
					if (client_socket[i] <= 0)
					{
						debug2("New Client %d      index   %d rep\n", new_socket,i);
						client_socket[i] = new_socket;
						currentIndex = i;
						//sprintf(msg,"Adding to list of sockets as %d\n" , i);
						//debugl(msg);
						break;
					}
				}
			}

			if (currentIndex >= MAX_TCP_CLIENTS - 1) {
				//memset(client_socket, 0, MAX_TCP_CLIENTS);
				for (i = 0; i < MAX_TCP_CLIENTS; i++)
				{
					client_socket[i] = 0;
				}
				client_socket[0] = new_socket;
			}

		}

		//else its some IO operation on some other socket
		for (i = 0; i < MAX_TCP_CLIENTS; i++)
		{
			sd = client_socket[i];

			if (FD_ISSET(sd, &readfds))
			{
				//tcp_commands_lockers[i].lock();
				if (tcp_packets[i] == nullptr) {
				///	//debugl("new command ");
					tcp_packets[i] = new sPacket;
				}
				////else{debugl("commmand already exist");}

				packet = tcp_packets[i];
				
				//Check if it was for closing , and also read the
				//incoming message

				// keep the first 4 bytes for the header and start from number 5 which is index 4
				rec = read(sd, &data[4], PACKET_DATA_SIZE, 0);
				
				//cmd->CommandStatus != eCommand_Status::csFinshedSuccess &&

				if (rec > 0)
				{
					if (*agent_socket_s < 0) {
						is_sending.lock();
						write(sd, "No Agent Found!", 15, 0);
						is_sending.unlock();
						shutdown(sd, 0);
						closesocket(sd);
						client_socket[i] = (-1);
						
					}
					else {

						is_sending.lock();
						data[0] = PACKET_HEADER;
						data[1] = (unsigned char)i;
						data[2] = (unsigned char)rec;
						data[3] = (char)ePacket_Status::csNew;
						write(*agent_socket_s, &data[0], (rec + 4), 0);
						is_sending.unlock();
					}
					/*packet->data_length = (unsigned char)rec;
					packet->status = ePacket_Status::csNew;
					packet->packet_index = (unsigned char)i;
					packet->packet_socket = sd;
					memcpy(&packet->data[0], data, rec);					
					send_agent(packet);*/
				}
				else {
					send_close_connection(i);
					//if (*active_client_socket_s == sd) *active_client_socket_s = -1;
					//getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
					debug2("Client disconnected %d \n", client_socket[i]);
					shutdown(sd, 0);
					closesocket(sd);
					client_socket[i] = (-1);
				}

				//tcp_commands_lockers[i].unlock();

			}
		}
	}

	return 0;
}

void send_agent(sPacket * _packet) {
	
	if (_packet == nullptr) { debug2("send_agent: _packet is null\n"); return; }

	if(_packet->data_length == 0 && _packet->status == ePacket_Status::csNew) { debug2("send_agent: _packet data_length == 0\n"); return; }
	int n = 0;
	is_sending.lock();
	if (*agent_socket_s >= 0)
	{	
		char arr[BUFFER_SIZE];
		arr[0] = PACKET_HEADER;
		arr[1] = (char)_packet->packet_index;
		arr[2] = (char)_packet->data_length;
		arr[3] = (char)_packet->status;
		memcpy(&arr[4], &_packet->data[0], _packet->data_length);
		n = send(*agent_socket_s, &arr[0], (_packet->data_length + 4), 0);
		if (n < (_packet->data_length + 4))
			debug2("XXX sent only %d/%d\n", n, (_packet->data_length + 4));
		//else debug2("[%d] ", n);

		write_log(&arr[0], _packet->data_length + 4);

	}
	else
	{
		if (_packet->status == ePacket_Status::csNew)
			send(_packet->packet_socket, "No Active Agents!", 17, 0);
	}
	is_sending.unlock();
}

void tcp_send_respond(int i, bool _lock) {
	//debugl("sending the responce to the waiting tcp client ");
	//debugIL("connection index =", i);
	//debugIL("socket ID =", client_socket[i]);
	//debugIL("responce length =", commands[i]->RespondLength);
	//if (_lock) { tcp_commands_lockers[i].lock(); }
	//debugIL("sending the responce to the waiting tcp client : ", send(client_socket[i], (char*)tcp_commands[i]->Respond, tcp_commands[i]->RespondLength, 0));
	//if (_lock)tcp_commands_lockers[i].unlock();
	// don't delete the clitn after respond
	//delete commands[i];
	//commands[i] = nullptr;
}

void send_close_connection(int i) {

	sPacket packet;
	packet.data_length = 0;
	packet.status = ePacket_Status::csFinshedSuccess;
	packet.packet_index = (unsigned char)i;
	packet.packet_socket = -1;
	send_agent(&packet);
}
