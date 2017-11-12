/* ClientsUtils.c
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
#include "ClientUtils.h"


EXIT_CODE InitialClientSocket(CLIENT_DATA* client_data)
{
	WSADATA			wsa_data;
	SOCKADDR_IN		client_service;

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR)
	{                            
		return WSA_STARTUP_ERROR;
    }

    // Create a SOCKET for connecting to server    
    client_data->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_data->socket == INVALID_SOCKET) 
	{
		WSACleanup();
		return CREATE_SOCKET_FAILED;
    }
 
	client_service.sin_family=		AF_INET;
    client_service.sin_addr.s_addr=	inet_addr(SERVER_ADDRESS_STR);
	client_service.sin_port=		htons(client_data->server_port);
    // Connect to server. 
    if (connect(client_data->socket, (SOCKADDR *) & client_service, sizeof (client_service)) == SOCKET_ERROR)
	{
        closesocket(client_data->socket);
		return CONNECT_TO_SERVER_FAILED;
    }
	return SUCCESS;
}
EXIT_CODE HandshakeClientSide(CLIENT_DATA* client_data)
{	
	char username_message[MESSAGE_LENGTH];
	char symbol_message[MESSAGE_LENGTH];

	sprintf(username_message,USERNAME_MSG_FORMAT,client_data->username);
	//sends player name
	SafelyOutput(g_logfile,NULL,FALSE,TRUE,TO_SERVER_LOG_FORMAT,username_message);	//log
	SendBuffer(username_message,MESSAGE_LENGTH,client_data->socket);	

	//receive player's symbol from server, REFUSE_SYMBOL if user already exists
	ReceiveBuffer(symbol_message,MESSAGE_LENGTH,client_data->socket);				
	sscanf(symbol_message,SYMBOL_MSG_FORMAT,client_data->username,&client_data->symbol); //extract symbol
	if(REFUSE_SYMBOL == client_data->symbol)
	{
		SafelyOutput(g_logfile,NULL,TRUE,TRUE,REFUSE_MSG_FORMAT);	//log and print refuse
		return CLIENT_REFUSED;
	}
	printf("%s",symbol_message);
	SafelyOutput(g_logfile,NULL,FALSE,TRUE,FROM_SERVER_LOG_FORMAT,symbol_message);	//log
	return SUCCESS;
	
}
EXIT_CODE CheckUIMessage(char* message, MSG_TYPE* type, char* cmd)
{
	int		num_of_args=0;
	char	arg1[MESSAGE_LENGTH];
	char	arg2[MESSAGE_LENGTH];
	
	cmd[0]='\0';
	*type =	GetMessageType(message);
	num_of_args=sscanf(message,"%s %s %s",cmd,arg1,arg2);
	switch(*type)
	{
	case PLAYERS_T:
		if(1 != num_of_args)
		{
			SafelyOutput(g_logfile,NULL,TRUE,TRUE,"Illegal argument for players. Command format is \"players\".\n");
			return ILEGAL_MSG_ARGS;
		}
		break;
	case MESSAGE_T:
		if(3 > num_of_args) 
		{
			SafelyOutput(g_logfile,NULL,TRUE,TRUE,"Illegal argument for command message. Command format is \"message <user> <message>\".\n");
			return ILEGAL_MSG_ARGS;
		}
		break;
	case BROADCAST_T:
		if(2 > num_of_args)
		{
			SafelyOutput(g_logfile,NULL,TRUE,TRUE,"Illegal argument for command broadcast. Command format is \"broadcast <message>\".\n");	
			return ILEGAL_MSG_ARGS;
		}
		break;
	case PLAY_T:
		if(1 != num_of_args) 
		{
			SafelyOutput(g_logfile,NULL,TRUE,TRUE,"Illegal argument for command play. Command format is \"play\".\n");	
			return ILEGAL_MSG_ARGS;
		}
		break;
	default:
		SafelyOutput(g_logfile,NULL,TRUE,TRUE,"Command \"%s\" is not recognized. Possible commands are: players, message, broadcast and play.\n",cmd);
		return ILEGAL_MSG_CMD;
		break;
	}
	return SUCCESS;
}
int SymbolToNum(char symbol)
{
	char all_symbols[]=PLAYERS_SYMBOLS;
	int i;
	for(i=0;i<strlen(PLAYERS_SYMBOLS);i++)
	{
		if(all_symbols[i] == symbol) return i;
	}
	return -1;
}

EXIT_CODE OrderRoutine(CLIENT_DATA* client_data, int* num_of_clients)
{
	char	message[MESSAGE_LENGTH];
	char*	charp=message;
	int		i;
	ReceiveBuffer(message,MESSAGE_LENGTH,client_data->socket); //get the users list
	printf("%s",message);
	SafelyOutput(g_logfile,NULL,FALSE,TRUE,FROM_SERVER_LOG_FORMAT,message); //log
	for (i=0; charp[i]!='.'; charp[i]==',' ? (char*)i++ : charp++); //count users by couinting the ","
	*num_of_clients=i+1;
	
	InitializeGameData(&client_data->game_data,PLAYERS_SYMBOLS,*num_of_clients);
	DrawGameBoard(client_data->game_data.board,PLAYERS_SYMBOLS,*num_of_clients);

	return SUCCESS;
}
EXIT_CODE PlayRoutine(CLIENT_DATA* client_data, int num_of_clients, HANDLE* players_turn_event, char* buff)
{
	int			toss_result;
	BOOL		retval;
	if(WAIT_OBJECT_0 == WaitForSingleObject(*players_turn_event,0)) //check if it is his turn
	{
		toss_result = toss();	//get random number between 1-6
		SafelyOutput(g_logfile,NULL,TRUE,TRUE,TOSS_RESULT_FORMAT,client_data->symbol,toss_result);	//print and log toss result

		//build the message to send to other players
		sprintf(buff,TOSS_MSG_FORMAT,client_data->symbol,client_data->username,toss_result); 
		
		//moving the player on the board
		retval = MovePlayer(client_data->game_data.board,SymbolToNum(client_data->symbol), client_data->game_data.players_positions, toss_result);
		
		//drawing the new board's state
		DrawGameBoard(client_data->game_data.board,PLAYERS_SYMBOLS,num_of_clients);
		
		//lock the event to prevent playing not in turn
		ResetEvent(*players_turn_event); 
		return retval? PLAYER_WON : PLAYER_NOT_WON;
	}
	else
	{
		return NOT_IN_TURN;
	}
}
EXIT_CODE PlayAnotherPlayerRoutine(CLIENT_DATA* client_data, int num_of_clients, char* buff)
{
	char		player_symbol;
	int			toss_result;
	BOOL		retval;

	//extract the message containing the other player's toss result
	sscanf(buff,TOSS_MSG_SCAN_FORMAT,&player_symbol,&toss_result);
	//moving the other player on the board
	retval = MovePlayer(client_data->game_data.board,SymbolToNum(player_symbol), client_data->game_data.players_positions, toss_result);
	//drawing the new board's state
	DrawGameBoard(client_data->game_data.board,PLAYERS_SYMBOLS,num_of_clients);
	return retval? PLAYER_WON : PLAYER_NOT_WON;
}