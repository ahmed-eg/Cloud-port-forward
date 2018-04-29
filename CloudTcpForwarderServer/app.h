/** -------------------------------------------------------

File name:	app.h
Purpose:	the main header file for all project, which include all common configurations and definitions
Version:	1.0		11/23/2017
Auther:		Ahmed Elaraby

---------------------------------------------------------*/

#pragma once
#define _WIN_	1

#define MAX_TCP_CLIENTS	30 // must change in Agent Project
#define MAX_TCP_AGENT	3

#include <sys/types.h>
#include "tcp_server.h"
#include "tcp_server_agent.h"
#include "Logs.h"
#include <mutex>          // std::mutex

#define BUFFER_SIZE				254
#define PACKET_DATA_SIZE		250


// you can adjust the level of output messages by commenting the printf on eacg level
// debug is the mose important messages and debug3 is the less important
#define debug			printf
#define debug1			printf
#define debug2			//printf
#define debug3			//printf

//make sure that only one thread is sending at a time
extern std::mutex is_sending;