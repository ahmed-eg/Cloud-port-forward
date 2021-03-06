#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdlib.h>
#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include <stdint.h>

#include <winsock2.h>
#define read	recv
#define write	send
#define socklen_t	int

#define debugl			printf
#define debugIL			printf
#define debug			printf

#pragma comment(lib, "Ws2_32.lib")


#define request  "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.75 Safari/537.36\r\nUpgrade-Insecure-Requests: 1\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\nDNT: 1\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9,ar;q=0.8\r\nAlexaToolbar-ALX_NS_PH: AlexaToolbar/alx-4.0.1\r\n\r\n"


void connect_and_get();
void listen_and_send();

char buffer[10000];
int buffer_len;


int main()
{
	connect_and_get();
	debug2("Got information %d \n", buffer_len);
	for (;;) {
		listen_and_send();
	}

	for (;;)Sleep(200);
    return 0;
}

//
//void error(const char *msg)
//{
//	perror(msg);
//	exit(1);
//}

void connect_and_get() {
	buffer_len = 0;
	
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	//int respond_socket;
	int sockfd, n;
	//char buffer[513];
	//char is_closed = 0;
	HANDLE recthandle0;
	DWORD recthreadid0;
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
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		debug1("ERROR opening socket\n");
	server = gethostbyname("ebtco.com");
	if (server == NULL) {
		debug1(stderr, "ERROR, no such host\n");
		return;
	}
	//bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	//bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
	serv_addr.sin_port = htons(80);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		debug1("ERROR connecting\n");

	
	write(sockfd, request, strlen(request), 0);
	int r = 0;
	buffer_len = 0;
	
	timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	struct fd_set set;
	FD_ZERO(&set); /* clear the set */
	FD_SET(sockfd, &set); /* add our file descriptor to the set */

	for (;;) {
	
		int rv = select(sockfd + 1, &set, NULL, NULL, &timeout);
		if (rv == SOCKET_ERROR)
		{
			break;
			// select error...
		}
		else if (rv == 0)
		{
			debug1("timeout \n");
			break;
			// timeout, socket does not have anything to read
		}
		else
		{
			// socket has something to read
			r = read(sockfd, &buffer[buffer_len], 512, 0);
			if (r == SOCKET_ERROR)
			{
				// read failed...
				break;
			}
			else if (r == 0)
			{
				// peer disconnected...
				break;
			}
			else
			{
				// read successful...
				buffer_len += r;
			}
		}

		//r= read(sockfd, &buffer[buffer_len], 512, 0);
		//if (r > 0) {
		//	buffer_len += r;
		//}
		
		
	}

}

void listen_and_send() {
	//buffer_len = 0;
	
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char recbuffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		debug1("ERROR opening socket");
	
	//portno = atoi(1111);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(1111);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0)
		debug1("ERROR on binding\n");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd,
		(struct sockaddr *) &cli_addr,
		&clilen);
	if (newsockfd < 0)
		debug1("ERROR on accept\n");
	//bzero(buffer, 256);
	debug2("new connection\n");
	n = read(newsockfd, recbuffer, 255,0);
	debug1("\nreceived %d\n", n);
	recbuffer[n] = 0;
	debug1(recbuffer);

	if (n < 0) debug1("ERROR reading from socket\n");
	//printf("Here is the message: %s\n", buffer);
	n = write(newsockfd, &buffer[n], (buffer_len - n), 0);

	/*while (n < buffer_len)
	{
		n += write(newsockfd, &buffer[n], (buffer_len -n), 0);
	}*/
	if (n < 0) debug1("ERROR writing to socket\n");
	else debug2("sent %d \n",n);
	closesocket(newsockfd);
	closesocket(sockfd);

}