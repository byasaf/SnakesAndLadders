/* Clients.c
 * ===========================
 * Exercise 4
 * ===========================
 * Name: Assaf Ben Yishay
 * =========================== */
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
/* =========================== */
#include "Errors.h"
#include "Client.h"

EXIT_CODE MainClient(char* logfile, u_short server_port, char* username)
{
	return  EngineThread(logfile, server_port, username);	 
}

EXIT_CODE EngineThread(char* logfile, u_short server_port, char* username)
{
	FILE*						fp=NULL;
	HANDLE						threads_handles[2];			//handles for 0-ClientCommunicationThread and 1-UIThread
	HANDLE						semaphores[2];				//semaphore for new event used by:  0-ClientCommunicationThread and 1-UIThread
	HANDLE						done_semaphores[2];			//semaphore for done event handling used by: 0-ClientCommunicationThread and 1-UIThread
	HANDLE						players_turn_event;
	THREAD_ARGS					args[2];					//args for 0-ClientCommunicationThread and 1-UIThread	
	CLIENT_DATA					client_data;
	BOOL						done=FALSE;					//game loop flag
	DWORD						wait_result;
	DWORD						comm_exitcode;
	char						comm_buff[MESSAGE_LENGTH];	//buffer for string from server
	char						ui_buff[MESSAGE_LENGTH];	//buffer for string from UI
	char						cmd[MESSAGE_LENGTH];
	int							num_of_active_clients;
	MSG_TYPE					ui_msg_type;				//message from UI's type
	MSG_TYPE					comm_msg_type;				//message from server's type
	EXIT_CODE					ui_check_result;
	EXIT_CODE					play_result;
	EXIT_CODE					retval;
	
	g_logfile=logfile;
	retval = CreateLogFile(g_logfile,&fp);
	if(SUCCESS != retval)
	{
		return retval;
	}

	semaphores[0]=CreateSemaphore(NULL,0,1,NULL);
	semaphores[1]=CreateSemaphore(NULL,0,1,NULL);
	done_semaphores[0]=CreateSemaphore(NULL,1,1,NULL);
	done_semaphores[1]=CreateSemaphore(NULL,1,1,NULL);
	if( NULL == semaphores[0] || NULL == semaphores[1] || 
		NULL == done_semaphores[0] || NULL == done_semaphores[1])
	{
		SafelyOutput(g_logfile,NULL,TRUE,TRUE,
				"CreateSemaphore failed with error: %ld\n", GetLastError());
		retval = SEMAPHORE_ERROR; 
		goto client_cleanup_1;
		//AddHandler
	}
	players_turn_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if( NULL == players_turn_event )
	{
		SafelyOutput(g_logfile,NULL,TRUE,TRUE,
				"CreateEvent failed with error: %ld\n", GetLastError());
		retval = EVENT_ERROR; 
		goto client_cleanup_2;
		//AddHandler
	}

	strcpy(client_data.username,username);
	client_data.server_port =	server_port;

	//connect to server
	retval = InitialClientSocket(&client_data);
	if(SUCCESS != retval)	//connect to server
	{
		SafelyOutput(g_logfile,NULL,TRUE,TRUE,CONNECT_ERROR_FORMAT,server_port); //print and log
		goto client_cleanup_3;
		//AddHandler
	}
	SafelyOutput(g_logfile,NULL,TRUE,TRUE,CONNECT_SUCCESS_FORMAT,server_port); //print and log
	
	//handshake with server using username
	retval = HandshakeClientSide(&client_data);
	if(SUCCESS != retval)	
	{
		goto client_cleanup_4;
	}

	//successfuly connected, go to play
	args[0].semaphore =			&semaphores[0];
	args[0].done_semaphore =	&done_semaphores[0];
	args[0].client_data=		&client_data;
	args[0].message=			comm_buff;
	threads_handles[0]=CreateThread(
							NULL,
							0,
							(LPTHREAD_START_ROUTINE)ClientCommunicationThread,
							&args[0],
							0,
							NULL);

	args[1].semaphore =			&semaphores[1];
	args[1].done_semaphore =	&done_semaphores[1];
	args[1].client_data=		&client_data;
	args[1].message=			ui_buff;
	threads_handles[1]=CreateThread(
							NULL,
							0,
							(LPTHREAD_START_ROUTINE)UIThread,
							&args[1],
							0,
							NULL);
	retval = SUCCESS;
	//==============================Main client loop============================
	while(!done)
	{
		GetExitCodeThread(threads_handles[0],&comm_exitcode);
		if (RECV_ERROR == comm_exitcode) 
		{
			SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,"Lost connection to server. Exiting.\n");
			retval = RECV_ERROR;
			goto client_cleanup_5;
		}
		wait_result = WaitForMultipleObjects(2,semaphores,FALSE,INFINITE); 
		//===============message has been received from server=================
		if(WAIT_OBJECT_0 + 0 == wait_result)
		{
			//log message from server
			SafelyOutput(g_logfile,NULL,FALSE,TRUE,FROM_SERVER_LOG_FORMAT,comm_buff);	
			//get message type and luanch the right routine to handle it
			comm_msg_type = GetMessageType(comm_buff);
			switch(comm_msg_type)
			{
			case ORDER_T:
				printf("%s",comm_buff);
				OrderRoutine(&client_data,&num_of_active_clients);
				break;
			case TURN_T:
				printf("%s",comm_buff);
				SetEvent(players_turn_event);
				break;
			case TOSS_T:
				printf("%s",comm_buff);
				if(PLAYER_WON == PlayAnotherPlayerRoutine(&client_data,num_of_active_clients,comm_buff))
				{
					TerminateThread(threads_handles[0],PLAYER_WON); //stop receiving from server
					done=TRUE; //end loop in case another player won
				}
				break;
			default:
				printf("%s",comm_buff);
				break;
			}
			ReleaseSemaphore(done_semaphores[0],1,NULL);	//start again ClientCommunicationThread's loop
		}

		//===================user has finished typing a line=====================
		if(WAIT_OBJECT_0 + 1 == wait_result) 
		{
			//check message's command and arguments, if valid - send it, else print the right error
			ui_check_result = CheckUIMessage(ui_buff,&ui_msg_type,cmd);
			if( SUCCESS == ui_check_result)
			{
				if( PLAY_T == ui_msg_type) //if user tried to play, go to PlayRoutine before sending message to server
				{
					play_result = PlayRoutine(&client_data,num_of_active_clients,&players_turn_event,ui_buff);
					if(NOT_IN_TURN ==play_result)
					{
						SafelyOutput(g_logfile,NULL,FALSE,TRUE,NOT_IN_TURN_MSG_FORMAT);
						printf(NOT_IN_TURN_MSG_FORMAT);
					}
					else
					{
						SendBuffer(ui_buff,MESSAGE_LENGTH,client_data.socket);					//send the toss result
						SafelyOutput(g_logfile,NULL,FALSE,TRUE,TO_SERVER_LOG_FORMAT,ui_buff);	//log toss result
						// check if won or not
						if(PLAYER_NOT_WON == play_result)
						{
							//send "not win" acknowledgment to server 
							sprintf(ui_buff,TURN_ACK,FALSE);
							SendBuffer(ui_buff,MESSAGE_LENGTH,client_data.socket);
						}
						else if(PLAYER_WON == play_result)
						{
							//send "win" acknowledgment to server 
							sprintf(ui_buff,TURN_ACK,TRUE);
							SendBuffer(ui_buff,MESSAGE_LENGTH,client_data.socket);
							done=TRUE; //end loop	
						}
					}//if( PLAY_T == ui_msg_type)
				}//if( SUCCESS == ui_check_result)
				else
				{
					//log message from UI
					SafelyOutput(g_logfile,NULL,FALSE,TRUE,TO_SERVER_LOG_FORMAT,ui_buff);
					SendBuffer(ui_buff,MESSAGE_LENGTH,client_data.socket); //for verified commands, just send to server
				}
			}
			ReleaseSemaphore(done_semaphores[1],1,NULL);	//start again UIThread's loop
		}
	}
	
	//wait for winning confirmation from server
	retval = PLAYER_WON;
	ReceiveBuffer(comm_buff,MESSAGE_LENGTH,args->client_data->socket);
	printf("%s",comm_buff);
	SafelyOutput(g_logfile,NULL,FALSE,TRUE,FROM_SERVER_LOG_FORMAT,comm_buff); //log winning announcement

client_cleanup_5:
	CloseHandle(threads_handles[0]);
	CloseHandle(threads_handles[1]);

client_cleanup_4:
	closesocket(client_data.socket);
	WSACleanup();

client_cleanup_3:
	CloseHandle(players_turn_event);

client_cleanup_2:
	CloseHandle(semaphores[0]);
	CloseHandle(semaphores[1]);
	CloseHandle(done_semaphores[0]);
	CloseHandle(done_semaphores[1]);

client_cleanup_1:
	return retval;
}
EXIT_CODE UIThread(THREAD_ARGS* args)
{
	BOOL		done=FALSE;
	int			len;
	char		buff[MESSAGE_LENGTH];
	Sleep(THREAD_DEFUALT_SLEEP_TIME);


	while (!done) 
	{
		WaitForSingleObject(*args->done_semaphore,INFINITE);
		gets(buff); //Reading a string from the keyboard
		strcpy(args->message,buff);
		len=strlen(args->message);
		args->message[len]='\n';
		args->message[len+1]='\0';
		ReleaseSemaphore(*args->semaphore,1,NULL);
	}
	return SUCCESS;
}
EXIT_CODE ClientCommunicationThread(THREAD_ARGS* args)
{
	BOOL		done=FALSE;
	char		buff[MESSAGE_LENGTH];
	Sleep(THREAD_DEFUALT_SLEEP_TIME);

	while(!done)
	{	
		WaitForSingleObject(*args->done_semaphore,INFINITE);
		if(TRNS_SUCCEEDED == ReceiveBuffer(buff,MESSAGE_LENGTH,args->client_data->socket))
		{
			strcpy(args->message,buff);
		}
		else
		{
			return RECV_ERROR;
		}
		ReleaseSemaphore(*args->semaphore,1,NULL);
	}
	return SUCCESS;
}
