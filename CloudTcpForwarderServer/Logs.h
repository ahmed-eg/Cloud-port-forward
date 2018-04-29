/** -------------------------------------------------------

	File name:	logs.h
	Purpose:	header for logging source
	Version:	1.0		11/23/2017
	Auther:		Ahmed Elaraby

---------------------------------------------------------*/
#pragma once

extern int init_log(char filename);

extern void write_log(char * msg, int len);

extern void close_log(void);


