/* SharedUtils.c
 * ===========================
 * Exercise 4
 * ===========================
 * Name: Assaf Ben Yishay
 * =========================== */
#include <winsock2.h>
#include <stdio.h>
#include <string.h>
/* =========================== */
#include "Errors.h"
#include "SharedUtils.h"

//Global memory used by all modules
char*		g_logfile=NULL;
HANDLE		logfile_mutex;
HANDLE		AllThreadsMustEnd;

TransferResult_t SendBuffer( const char* Buffer, int BytesToSend, SOCKET sd )
{
	/*
	const char* CurPlacePtr = Buffer;
	int BytesTransferred;
	int RemainingBytesToSend = BytesToSend;
	
	while ( RemainingBytesToSend > 0 )  
	{
		// send does not guarantee that the entire message is sent
		BytesTransferred = send (sd, CurPlacePtr, RemainingBytesToSend, 0);
		if ( BytesTransferred == SOCKET_ERROR ) 
		{
			printf("send() failed, error %d\n", WSAGetLastError() );
			return TRNS_FAILED;
		}
		
		RemainingBytesToSend -= BytesTransferred;
		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
	}
	*/

	if (SOCKET_ERROR != send(sd,Buffer,BytesToSend,0))
		return TRNS_SUCCEEDED;
	else return TRNS_FAILED;
}
TransferResult_t ReceiveBuffer( char* OutputBuffer, int BytesToReceive, SOCKET sd )
{
	/*
	char* CurPlacePtr = OutputBuffer;
	int BytesJustTransferred;
	int RemainingBytesToReceive = BytesToReceive;
	
	while ( RemainingBytesToReceive > 0 )  
	{
		// recv does not guarantee that the entire message is sent 
		BytesJustTransferred = recv(sd, CurPlacePtr, RemainingBytesToReceive, 0);
		if ( BytesJustTransferred == SOCKET_ERROR ) 
		{
			printf("recv() failed, error %d\n", WSAGetLastError() );
			return TRNS_FAILED;
		}		
		else if ( BytesJustTransferred == 0 )
			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.

		RemainingBytesToReceive -= BytesJustTransferred;
		CurPlacePtr += BytesJustTransferred; // <ISP> pointer arithmetic
	}*/
	
	if (SOCKET_ERROR != recv(sd,OutputBuffer,BytesToReceive,0))
		return TRNS_SUCCEEDED;
	else return TRNS_FAILED;
}
EXIT_CODE CreateLogFile(char* file_name, FILE** file_pointer)
{
	*file_pointer=fopen(file_name,"w");
	if(NULL==*file_pointer)
	{
		printf("Could not create file: %s\n",file_name);
		return OPEN_FILE_ERROR;
	}
	fclose(*file_pointer);
	return SUCCESS;
}
int SafelyOutput(char* file, HANDLE* mutex, BOOL console, BOOL log, const char *format, ...)
{
	int		ret;
    va_list args;
	FILE* fp=fopen(file,"a");
	if(NULL == fp && TRUE==log) return 1;
	va_start(args, format);
    if(NULL != mutex) WaitForSingleObject(*mutex, INFINITE);
	//==============Critical Section Start==============// 
	if (log) ret = vfprintf(fp,format,args);
	if (console) vprintf(format,args);
	//===============Critical Section End===============//
    if(NULL != mutex) ReleaseMutex(*mutex);
    va_end(args);
	fclose(fp);
	return ret;
}
MSG_TYPE GetMessageType(char* message)
{
	char	word1[MESSAGE_LENGTH];

	if(0 == strncmp(message,TURN_MSG_FORMAT,strlen(TURN_MSG_FORMAT)-1))		return TURN_T;
	if(0 == strncmp(message,ORDER_MSG_FORMAT,ORDER_MSG_DECIDE_LEN))			return ORDER_T;
	if(0 == strncmp(message,TURN_ACK,TURN_ACK_DECIDE_LEN))					return TACK_T;
	sscanf(message,"%s %*s",word1);
	if(0 == strcmp(word1,PLAYERS_STR))										return PLAYERS_T;
	if(0 == strcmp(word1,MESSAGE_STR))										return MESSAGE_T;
	if(0 == strcmp(word1,BROADCAST_STR))									return BROADCAST_T;
	if(0 == strcmp(word1,PLAY_STR))											return PLAY_T;
	if(0 == strcmp(word1,TOSS_N_WIN_PREFIX))
	{
		if(NULL != strstr(message,TOSS_IDENTIFY_WORD))						return TOSS_T;
	}

	return UNKNOWN_T;
}

BOOL CheckMustEnd()
{
	DWORD exit_code = WaitForSingleObject(AllThreadsMustEnd,0);
	if(WAIT_OBJECT_0==exit_code)
		return TRUE;
	else return FALSE;
}
void SetMustEnd()
{
	SetEvent(AllThreadsMustEnd);
}