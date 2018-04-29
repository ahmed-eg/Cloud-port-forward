/** -------------------------------------------------------

File name:	tcp_client_agent.cpp
Purpose:	listen for agent and keep the connection alive for any client
Version:	1.0		11/23/2017
Auther:		Ahmed Elaraby

---------------------------------------------------------*/

#include "stdafx.h"
#include "tcp_server_agent.h"
#include "stdafx.h"
#include "tcp_server.h"
#include "datastruct.h"
#include <stdlib.h>
#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include <stdint.h>

#pragma comment(lib, "Ws2_32.lib")

uint16_t agent_port;
//int sockfd_agent;
int * agent_socket;
int agent_socketlst[MAX_TCP_AGENT];

//std::mutex tcp_commands_lockers[MAX_TCP_AGENT];

//set of socket descriptors
fd_set readfds_agent;

tcp_server_agent::tcp_server_agent(unsigned short _port, int * _agent_socket)
{
	agent_port = _port;
	agent_socket = _agent_socket;
	tcp_init_agent();
}

tcp_server_agent::~tcp_server_agent()
{
}

sPacket * parse_packet(char * data, int len, int socket) {

	sPacket * p = nullptr;
	int rec;

	if (socket <= 0) {
		debug2("XXXXXXXXXXXXXXXXXXX        socket invalid !!!");
		return nullptr;
	}
	

	if (len < 4) {
		debug2("XXXXXXXXXXXXXXXXXXX        heaer less than 4   %d!!!\n", (4 - len));

		while (len < 4) {
			rec = read(socket, &data[len], (4 -len), 0);
			if (rec <= 0) { return nullptr; }
			len += rec;
		}
		//delete p;
		//return nullptr;
	}

	for (int i = 0; i < len; i++) {

		if (data[i] == PACKET_HEADER) {
			p = new sPacket;
			p->packet_index = (unsigned char)data[i + 1];
			p->data_length = (unsigned char)data[i + 2];
			p->status = (ePacket_Status)data[i + 3];
			p->data_length_temp = (len - (i + 4));
			if (p->data_length_temp < p->data_length)
			{
				//debug2(" =====================  packet not competed!  %d/%d\n", p->data_length_temp , p->data_length);
				if (p->data_length_temp > 0)memcpy(p->data, &data[i + 4], p->data_length_temp);
				while (p->data_length_temp < p->data_length) {
					rec = read(socket, &p->data[p->data_length_temp], (p->data_length - p->data_length_temp), 0);
					if (rec <= 0) { return nullptr;}
					p->data_length_temp += rec;
				}
			}
			else { memcpy(p->data, &data[i + 4], p->data_length); }

			debug3("Packet OK        :)   %d \n", p->data_length);
			return p;
		}
		else {
			debug1(".");
			debug3("yyyyyyyyyyyyyyyyyyyyyyyyy cannot parse packet %d/%d \n", i, len);
			//return nullptr;
			continue;
		}

	}//end for

	return p;	
}

void send_to_client(sPacket * p) {
	if (p == nullptr) { debug2("send_to_client, p == null"); return; }
	

	if (p->status == ePacket_Status::csNew) {
is_sending.lock();
		int c = client_socket[p->packet_index];
		debug3("index/socket_id   %d/%d \n", p->packet_index , c);
		write(c, p->data, p->data_length, 0);
is_sending.unlock();
	}
	else if (p->status == ePacket_Status::csFinshedSuccess) {
		shutdown(client_socket[p->packet_index], 0);
		closesocket(client_socket[p->packet_index]);
		client_socket[p->packet_index] = -1;
		debug2("send_to_client: close connection with client %d \n", p->packet_index);
	}
	else { debug2("send_to_client: packet status not new %d \n" , (int)p->status); }

	
}

char tcp_server_agent::tcp_init_agent() {
	
	// Listen connection without thread
	int opt = TRUE;
	int master_socket, addrlen, new_socket,
		activity, i, valread, sd;
	sPacket * packet;
	int max_sd;
	struct sockaddr_in address;
	int16_t rec = 0;
	char data[513];
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
	for (i = 0; i < MAX_TCP_AGENT; i++)
	{
		agent_socketlst[i] = 0;
	}


	//create a master socket
	if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
	{
		const DWORD last_error = WSAGetLastError();
		debug1("agent:  socket failed %d", (int)master_socket);
		return 0;
		//exit(EXIT_FAILURE);
	}


	int res = setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt));
	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	if (res < 0)
	{
		debug1("agent: setsockopt %d", res);
		return 0;
		//exit(EXIT_FAILURE);
	}

	//type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(agent_port);

	//bind the socket to localhost port 8888
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		debug1("agent: bind failed\n");
		//exit(EXIT_FAILURE);
	}
	debug1("agent: Listener on port %d \n", agent_port);

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(master_socket, 3) < 0)
	{
		debug1("agent: listen\n");
		//exit(EXIT_FAILURE);
	}

	//accept the incoming connection
	addrlen = sizeof(address);
	debug1("agent: Waiting for connections ...\n");

	for (;;)
	{
		//clear the socket set
		FD_ZERO(&readfds_agent);

		//add master socket to set
		FD_SET(master_socket, &readfds_agent);
		max_sd = master_socket;

		//add child sockets to set
		for (i = 0; i < MAX_TCP_AGENT; i++)
		{
			//socket descriptor
			sd = agent_socketlst[i];

			//if valid socket descriptor then add to read list
			if (sd > 0)
				FD_SET(sd, &readfds_agent);

			//highest file descriptor number, need it for the select function
			if (sd > max_sd)
				max_sd = sd;
		}


		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		activity = select(max_sd + 1, &readfds_agent, NULL, NULL, NULL);

		if ((activity < 0))//&& (errno!=EINTR)
		{
			//debugl("select error");
		}

		//If something happened on the master socket ,
		//then its an incoming connection
		if (FD_ISSET(master_socket, &readfds_agent))
		{

			if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
			{
				//debugl("accept");
				exit(EXIT_FAILURE);
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

			//add new socket to array of sockets
			for (i = 0; i < MAX_TCP_AGENT; i++)
			{
				//if position is empty
				if (agent_socketlst[i] == 0)
				{
					debug1("New Agent Connected %d \n", new_socket);
					agent_socketlst[i] = new_socket;
					if (*agent_socket < 0) *agent_socket = new_socket;
					//sprintf(msg,"Adding to list of sockets as %d\n" , i);
					//debugl(msg);
					break;
				}
			}

		}

		//else its some IO operation on some other socket
		for (i = 0; i < MAX_TCP_AGENT; i++)
		{
			sd = agent_socketlst[i];

			if (FD_ISSET(sd, &readfds_agent))
			{
				
				//else{debugl("commmand already exist");}

				//cmd = tcp_commands[i];
				
				rec = read(sd, &data[0], 4, 0);

				if (agent_socketlst[i] != *agent_socket) {
					is_sending.lock();
					send(agent_socketlst[i], "Server Busy!", 12, 0);
					debug1("send Server busy!\n");
					is_sending.unlock();
					//closesocket(agent_socketlst[i]);
					shutdown(agent_socketlst[i], 0);
					agent_socketlst[i] = 0;

				}
				else					
				//Check if it was for closing , and also read the
				//incoming message
				if (rec > 0)
				{

					//if (cmd->BufferLength == 0) { debugl("|TCP----------------------------------------------\n"); }

					//cmd->BufferLength += rec;
					//data[rec] = 0;
					//debugl(data);
					packet = parse_packet(data, rec, sd);
					if (packet != nullptr) {
						send_to_client(packet);
						delete packet;
					}
					else { debug3("Packet is null\n"); }
					//if (*active_client_socket >= 0)
						//send(*active_client_socket, &data[0], rec, 0);

				}
				else {

					//getpeername(sd, (struct sockaddr*)&address,						(socklen_t*)&addrlen);
					debug2("agent: agent disconnected \n");
					//closesocket(sd);
					shutdown(sd, 0);
					agent_socketlst[i] = 0;
					*agent_socket = -1;
				}
				
			}
		}
	}

	return 0;
}


//
//void tcp_send_agent(char * _array, int _len) {
//	is_sending_agent.lock();
//	send(*agent_socket, _array, _len, 0);
//	is_sending_agent.unlock();
//}

void tcp_send_respond_agent(int i, bool _lock) {
	//debugl("sending the responce to the waiting tcp client ");
	//debugIL("connection index =", i);
	//debugIL("socket ID =", agent_socket[i]);
	//debugIL("responce length =", commands[i]->RespondLength);
	//if (_lock) { tcp_commands_lockers[i].lock(); }
	//debugIL("sending the responce to the waiting tcp client : ", send(agent_socket, (char*)tcp_commands[i]->Respond, tcp_commands[i]->RespondLength, 0));
	//if (_lock)tcp_commands_lockers[i].unlock();
	// don't delete the clitn after respond
	//delete commands[i];
	//commands[i] = nullptr;
}