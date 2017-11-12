/* ServerUtils.c
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
#include "ServerUtils.h"

EXIT_CODE HandshakeServerSide(CLIENT* new_client, CLIENT* clients)
{
	char	username_message[MESSAGE_LENGTH];
	char	symbol_message[MESSAGE_LENGTH];
	char	symbols[]=PLAYERS_SYMBOLS;
	int		i;
	
	//receive username from client
	ReceiveBuffer(username_message,MESSAGE_LENGTH,new_client->socket);
	sscanf(username_message,USERNAME_MSG_FORMAT,new_client->username);

	//check if username already exists
	for(i=new_client->client_num-1; i>=0 ;i--)
	{
		if(0 == strcmp(new_client->username,clients[i].username))
		{
			sprintf(symbol_message,SYMBOL_MSG_FORMAT,new_client->username,REFUSE_SYMBOL);
			SendBuffer(symbol_message,MESSAGE_LENGTH,new_client->socket); //notify client about refuse
			new_client->client_num=-1;
			return CLIENT_REFUSED;
		}
	}
	
	//send symbol to client
	new_client->symbol=symbols[new_client->client_num]; //choose client's symbol
	sprintf(symbol_message,SYMBOL_MSG_FORMAT,new_client->username,new_client->symbol);
	SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,symbol_message); //log this message
	SendBuffer(symbol_message,MESSAGE_LENGTH,new_client->socket);

	return SUCCESS;
}
EXIT_CODE BroadcastMessage(const char* buffer, int sender, CLIENT* clients)
{
	int			i=0;
	EXIT_CODE	retval=SUCCESS;

	for(i=0;clients[i].client_num!=-1 && i<NUM_OF_CLIENTS;i++)
	{
		if(clients[i].client_num != sender && clients[i].client_num != -1)
		{
			if(TRNS_FAILED == SendBuffer(buffer,MESSAGE_LENGTH,clients[i].socket)) retval=SEND_ERROR;
		}
	}
	return retval;
}
EXIT_CODE UnicastMessage(const char* buffer, int dest, CLIENT* clients)
{
	if(TRNS_FAILED == SendBuffer(buffer,MESSAGE_LENGTH,clients[dest].socket)) return SEND_ERROR;
	else return SUCCESS;
}
int FindUsername(const char* username, CLIENT* clients)
{
	int			i;
	for(i=0;clients[i].client_num!=-1 && i<NUM_OF_CLIENTS;i++)
	{
		if(0 == strcmp(clients[i].username,username))
		{
			return i;
		}
	}
	return -1;
}

EXIT_CODE PlayersRoutine(char* message, CLIENT* sender, CLIENT* clients)
{
	char	send_str[MESSAGE_LENGTH]="";
	char	temp_str[MESSAGE_LENGTH];
	int		i;
	for(i=0;clients[i].client_num!=-1 && i<NUM_OF_CLIENTS;i++)
	{
		sprintf(temp_str,PLAYERS_MSG_FORMAT,clients[i].username,clients[i].symbol);
		strcat(send_str,temp_str);
	}
	send_str[strlen(send_str)-1]='.';
	send_str[strlen(send_str)]='\n';
	SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,PLAYERS_CMD_LOG_FORMAT,sender->username); //log this message
	return UnicastMessage(send_str,sender->client_num,clients);
}
EXIT_CODE MessageRoutine(char* message, CLIENT* sender, CLIENT* clients)
{
	char	dest[MESSAGE_LENGTH];
	char	body[MESSAGE_LENGTH];
	char	send_str[MESSAGE_LENGTH];
	int		dest_index;

	sscanf(message,"%*s %s %[^\t\n]",dest,body);
	dest_index = FindUsername(dest,clients);
	if ( -1 == dest_index) 
	{
		sprintf(send_str,USER_NOT_EXISTS_MSG_FORMAT,dest);
		UnicastMessage(send_str,sender->client_num,clients);
		return USER_NOT_EXISTS;
	}
	sprintf(send_str,PRIVATE_MSG_FORMAT,sender->username,body);
	SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,MESSAGE_CMD_LOG_FORMAT,
				sender->username,clients[dest_index].username,body); //log
	return UnicastMessage(send_str,dest_index,clients);
}
EXIT_CODE BroadcastRoutine(char* message, CLIENT* sender, CLIENT* clients)
{
	char	body[MESSAGE_LENGTH];
	char	send_str[MESSAGE_LENGTH];

	sscanf(message,"%*s %[^\t\n]",body);
	sprintf(send_str,BROADCAT_MSG_FORMAT,sender->username,body);
	SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,BROADCAST_CMD_LOG_FORMAT,
				sender->username,body); //log
	return BroadcastMessage(send_str,sender->client_num,clients);
}
EXIT_CODE TurnAckRoutine(char* message, CLIENT* sender, CLIENT* clients)
{
	int result=0;
	sscanf(message,TURN_ACK,&result);
	if(TRUE == result)
	{
		sprintf(message,WIN_MSG_FORMAT,sender->username);
		BroadcastMessage(message,-10,clients); //notify everybody
	}
	return result? PLAYER_WON : PLAYER_NOT_WON;
}

void CleanupWorkerThreads(CLIENT* clients, int num_of_active_clients)
{
	int i;
	for(i =0 ; i<num_of_active_clients; i++)
	{
		if(NULL != clients[i].thread_handle)
		{
			if(WAIT_OBJECT_0 != WaitForSingleObject(clients[i].thread_handle,WAIT_WORKING_THREADS_TIMEOUT*1000))
			{
				closesocket(clients[i].socket);
				TerminateThread(clients[i].thread_handle,WAIT_THREAD_TIMEOUT);
			}
			CloseHandle(clients[i].thread_handle);
		}
	}
}
