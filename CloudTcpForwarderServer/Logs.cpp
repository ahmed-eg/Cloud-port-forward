/** -------------------------------------------------------

File name:	logs.cpp
Purpose:	saving the eventlog in text file
Version:	1.0		11/23/2017
Auther:		Ahmed Elaraby

---------------------------------------------------------*/

#include "stdafx.h"
#include "Logs.h"
#include <windows.h>
#include <stdio.h>
//#include "datastruct.h"
#include <stdlib.h>  
#include <locale.h> 

HANDLE hAppend;
DWORD dwBytesRead, dwBytesWritten, dwPos;

int init_log(char filename)
{
#ifndef DEBUGING // if not debug mode then return
	return 0;
#endif
	LPCWSTR file;
	
	if (filename == 0)
		file = L"server.txt";
	else //if (filename == 1)
		file = L"agent.txt";

	// Open the existing file, or if the file does not exist,
	// create a new file.
	hAppend = CreateFile(file, // open mytestfiletwo.txt
		GENERIC_WRITE, // open for writing
		0, // do not share
		NULL, // default security
		OPEN_ALWAYS, // open or create
		FILE_ATTRIBUTE_NORMAL, // normal file
		NULL); // no attribute template

	if (hAppend == INVALID_HANDLE_VALUE)
		printf("Could not open %S lol!.\n", file);
	else
	{
		printf("%S opened/created successfully.\n", file);
	}

	return 0;
}

void write_log(char * msg,int len) {

#ifndef DEBUGING // if not debug mode then return
	return;
#endif

	// Append the first file to the end of the second file.
	// Lock the second file to prevent another process from
	// accessing it while writing to it. Unlock the file when writing is finished.

	dwPos = SetFilePointer(hAppend, 0, NULL, FILE_END);

	LockFile(hAppend, dwPos, 0, dwBytesRead, 0);// != 0)
		//printf("Locking ...\n");
	
	if (msg[0] == 0x03) //PACKET_HEADER
	{ 
		WriteFile(hAppend, (const char*)"[HEADER] ", 9, &dwBytesWritten, NULL);
		WriteFile(hAppend, &msg[4], len-4, &dwBytesWritten, NULL);
	}
	else
	{
		WriteFile(hAppend, (const char*)"[DATA] ", 7, &dwBytesWritten, NULL);
		WriteFile(hAppend, msg, len, &dwBytesWritten, NULL);
	}

	WriteFile(hAppend, "\r\n\r\n\r\n", 6, &dwBytesWritten, NULL);

	UnlockFile(hAppend, dwPos, 0, dwBytesRead, 0);//!= 0)
		//printf("Unlocking ...\n");
	
}

void close_log(void) {

#ifndef DEBUGING // if not debug mode then return
	return ;
#endif
	// Close both files.
	
	if (CloseHandle(hAppend) != 0)
		printf(" file handle closed successfully!\n");

}
