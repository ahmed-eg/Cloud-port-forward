/** -------------------------------------------------------

File name:	tcp_client_agent.cpp
Purpose:	connect to server as agent
Version:	1.0		11/23/2017
Auther:		Ahmed Elaraby

---------------------------------------------------------*/

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "tcp_client_agent.h"

//connect to multibale servers
tcp_client * clients[5];

void error_agent(const char *msg)
{
	//perror(msg);
	debug1(msg);
	//exit(0);
}

tcp_client_agent::tcp_client_agent(char * _ip_address, uint16_t _port, char * _endpoing_server, uint16_t _endpoint_port)
{
	debug("Start Conncting to Agent Listner!\n");
	agent_port = _port;
	agent_server = _ip_address;
	endpoing_server = _endpoing_server;
	endpoint_port = _endpoint_port;
	tcp_init_agent();
}


int sockfd4, n4;
char buffer4[513];
tcp_client * cls = nullptr;

sPacket * parse_packet_agent(char * data, int len, int socket) {

	sPacket * p = nullptr;
	int rec;

	if (len < 4) {
		debug2("XXXXXXXXXXXXXXXXXXX        heaer less than 4 !!!");
		return nullptr;
	}

	if (socket <= 0) {
		debug2("XXXXXXXXXXXXXXXXXXX        socket invalid !!!");
		return nullptr;
	}
	for (int i = 0; i < len; i++) {
		
		if (data[i] == PACKET_HEADER) {
			p = new sPacket;
			p->packet_index = (unsigned char)data[i+1];
			p->data_length = (unsigned char)data[i+2];
			p->status = (ePacket_Status)data[i+3];
			p->data_length_temp = (len - (i + 4));
			if (p->data_length_temp < p->data_length)
			{
				//debug2(" =====================  packet not competed!  %d/%d\n", p->data_length_temp , p->data_length);
				if (p->data_length_temp > 0)memcpy(p->data, &data[i+4], p->data_length_temp);
				while (p->data_length_temp < p->data_length) {
					rec = read(socket, &p->data[p->data_length_temp], (p->data_length - p->data_length_temp), 0);
					if (rec <= 0) return nullptr;
					p->data_length_temp += rec;
				}
			}
			else { memcpy(p->data, &data[i+4], p->data_length); }
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

//DWORD WINAPI recthread4(__in LPVOID lpParameter)
//{
//	return 0;
//	int n3;
//	char buffer3[513];
//
//	while (n3 = read(sockfd4, &buffer3[0], 512, 0) > 0) 
//	{
//		//cls->tcp_write(&buffer3[0], n3);
//	}
//	
//	return 0;
//}

void tcp_client_agent::tcp_init_agent(void) {
		
	struct sockaddr_in serv_addr;
	struct hostent *server;
	*agent_connected = 0;
	debug1("Started \n");

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
		debug1("WSAStartup failed with error: %d\n", err);
		return;
	}
	/////////////////////////////////////////////////////


	//portno = atoi(argv[2]);
	sockfd4 = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd4 < 0)
		error_agent("ERROR opening socket");
	server = gethostbyname(agent_server);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		return;
	}
	//bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	//bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
	serv_addr.sin_port = htons(agent_port);
	if (connect(sockfd4, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		error_agent("ERROR connecting\n");
		//TODO: reconnect again
	}
	
	sPacket * packet = nullptr;

	HANDLE recthandle;
	DWORD recthreadid;
	char * is_client_closed;
	*agent_connected = 1;
	//memset(&buffer4[0], 0, 513);
	do {

		n4 = read(sockfd4, &buffer4[0], 4, 0);

		if (n4 > 0) {
			
			//write_log(buffer4, n4);
			packet = parse_packet_agent(buffer4, n4, sockfd4);
			
			if (packet != nullptr && packet->status == ePacket_Status::csNew) {
				packet->packet_socket = sockfd4;
				cls = clients[packet->packet_index];

				if (cls == nullptr ) {
					debug2("Start Connect to client ...   index:%d \n", packet->packet_index);
					//debug3("client class is null =%d \n", (char)(cls== nullptr));
					//cls = new tcp_client("ebtco.com", 80, sockfd4);
					cls = new tcp_client(endpoing_server, endpoint_port, sockfd4);
					clients[packet->packet_index] = cls;
					debug2("client connected!\n");
				}
				else if (cls->get_is_closed()) {
					debug2("client closed!!\n");
					cls->~tcp_client();
					cls = new tcp_client(endpoing_server, endpoint_port, sockfd4);
					clients[packet->packet_index] = cls;
				}

				//if (n4 > 0) cls->tcp_write(&buffer4[0], n4);
				cls->tcp_write_packet(packet);

				delete packet;
			}
			else if (packet != nullptr && packet->status == ePacket_Status::csFinshedSuccess)
			{
				debug3("Packet already dissconnected\n");
				delete packet;
			}
			else
			{
				buffer4[n4] = 0;
				debug2("%s \n",buffer4);
			}
		}
		//recthandle = CreateThread(0, 0, recthread4, 0, 0, &recthreadid);

		//is_client_closed = cls->get_is_closed();
		// wait untill the connection with client is closed
		//cls->lock();
		//while (!(*is_client_closed));

		//TerminateThread(recthandle, 0); // Dangerous source of errors!
		//CloseHandle(recthandle);
		
		
	} while (n4 > 0);
	
	debug1("Close Connection with Server as Agent\n");
	//the main function will re-connect when return from this function

	// clean and return
	shutdown(sockfd4, 0);
	closesocket(sockfd4);
	*agent_connected = 0;
	//return 0;

}

tcp_client_agent::~tcp_client_agent()
{
	//distroy all functions
	*agent_connected = 0;
	for (int i = 0; i < 20; i++) {
		if (clients[i] != nullptr)
		{
			delete clients[i];  
			clients[i] = nullptr;
		}
	}

}
