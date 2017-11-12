/* Server.c
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
#include "Server.h"

EXIT_CODE MainServer(char* logfile, u_short server_port)
{ 
	return ServerGameManagementThread(logfile,server_port);
}

EXIT_CODE ServerGameManagementThread(char* logfile, u_short server_port)
{
	FILE*						fp=NULL;
	WSADATA						wsa_data;
	CLIENT						clients[NUM_OF_CLIENTS];
	HANDLE						broadcast_mutex;
	HANDLE						wait_events_array[2];
	int							num_of_active_clients=0;
	int							i;
	int							current_turn;
	char						send_message[MESSAGE_LENGTH];
	char						usernames_list[MAX_USERNAME_LENGTH*NUM_OF_CLIENTS]="";	
	EXIT_CODE					retval=SUCCESS;
	
	//global memory initialize
	g_logfile=logfile;
	
	retval = CreateLogFile(logfile,&fp);
	if(SUCCESS != retval)
	{
		return retval;
	}

	AllThreadsMustEnd = CreateEvent(NULL,TRUE,FALSE,NULL);
	if(NULL == AllThreadsMustEnd)
	{
		SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,
			"CreateEvent failed with error: %ld\n", GetLastError());  
		retval = EVENT_ERROR; 
		goto server_cleanup_1;
	}

	broadcast_mutex = CreateMutex(NULL,FALSE,BROADCAST_MUTEX_NAME);
	logfile_mutex = CreateMutex(NULL,FALSE,NULL);
	if(NULL == broadcast_mutex)
	{
		SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,
			"CreateMutex failed with error: %ld\n", GetLastError());
		retval = MUTEX_ERROR; 
		goto server_cleanup_2;
	}

    if ( WSAStartup( MAKEWORD( 2, 2 ), &wsa_data ) != NO_ERROR )
	{
		SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,
			"error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		retval = WSA_STARTUP_ERROR;
		goto server_cleanup_2;
	}

	//========================WAIT FOR CLIENTS==========================
	retval = AcceptClientsWithTimeout(clients,server_port,&num_of_active_clients);	
	if(SUCCESS != retval)
	{
		goto server_cleanup_3;
	}

	//========================GAME START HERE==========================
	for(i=0 ; i<num_of_active_clients ; i++) //building the players' order
	{
		strcat(usernames_list,clients[i].username);
		if(i == num_of_active_clients-1) //last username
			{ strcat(usernames_list,".\n");}
		else
			{ strcat(usernames_list,", "); }
	}
	sprintf(send_message,ORDER_MSG_FORMAT,usernames_list);
	WaitForSingleObject(broadcast_mutex, INFINITE);
	//==============Critical Section Start==============//
		BroadcastMessage(send_message,-10,clients); //send players' order list
		for(i=0 ; i<num_of_active_clients ; i++)	//sending the symbols - Players command's result
		{
			PlayersRoutine(NULL,&clients[i],clients);
		}
		SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,WELCOME_LOG_FORMAT); //log this message
	//===============Critical Section End===============//
	ReleaseMutex(broadcast_mutex);


	//waiting for all ServerCommunicationThread's to be ready before game
	for(i=0 ; i<num_of_active_clients ; i++) 
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(clients[i].done_turn_event,INFINITE))
		{
			ResetEvent(clients[i].done_turn_event);
		}
		else i--;	//if a thread not ready wait for it again
	}
	
	//===============================Main game loop======================================
	retval = SUCCESS;
	current_turn=0;
	while(!CheckMustEnd()) 
	{	
		sprintf(send_message,ANOTHERS_TURN_MSG_FORMAT,clients[current_turn].username);
		SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,send_message); //log "whos turn" message
		if(	SEND_ERROR == UnicastMessage(TURN_MSG_FORMAT,current_turn,clients) ||
			SEND_ERROR == BroadcastMessage(send_message,current_turn,clients)) //send turn messages
		{
			SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,
				"Send to client error %ld, closing server.\n", WSAGetLastError());
			retval=SEND_ERROR;
			SetMustEnd();
			goto server_cleanup_3;
			//AddHandler (Close all workers threads)
		}
		ResetEvent(clients[current_turn].done_turn_event);

		//build array for WaitForMultipleObjects
		wait_events_array[0]=clients[current_turn].done_turn_event; //player done turn event 
		wait_events_array[1]=AllThreadsMustEnd;						//connection error (or any else fatal error)

		WaitForMultipleObjects(2,wait_events_array,FALSE,INFINITE); //wait for player to play or connection error (receive error);

		current_turn++;
		current_turn=current_turn%num_of_active_clients; //turns in loop
	}


server_cleanup_3:
	CleanupWorkerThreads(clients,num_of_active_clients);
	WSACleanup();

server_cleanup_2:	
	CloseHandle(broadcast_mutex);
	CloseHandle(logfile_mutex);

server_cleanup_1:
	return retval;
}
EXIT_CODE ServerCommunicationThread(CLIENT* client)
{
	char					accept_message[MESSAGE_LENGTH];
	MSG_TYPE				msg_type;
	HANDLE					broadcast_mutex;
	BOOL					done=FALSE;
	CLIENT*					clients=NULL;	
	EXIT_CODE				play_result;
	EXIT_CODE				retval=SUCCESS;

	Sleep(THREAD_DEFUALT_SLEEP_TIME);

	clients=&client[-client->client_num]; //pointer to the beginning of the clients array

	broadcast_mutex = OpenMutex(SYNCHRONIZE ,TRUE, BROADCAST_MUTEX_NAME);
	if(NULL == broadcast_mutex)
	{
		SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,
			"OpenMutex failed with error: %ld\n",GetLastError());
		closesocket(client->socket);
		return MUTEX_ERROR; 
	}

	client->done_turn_event= CreateEvent(NULL,TRUE,TRUE,NULL);
	if(NULL == client->done_turn_event)
	{
		SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,
			"CreateEvent failed with error: %ld\n", GetLastError());
		closesocket(client->socket);
		return EVENT_ERROR; 
	}

	while(!done && !CheckMustEnd())
	{
		if(TRNS_SUCCEEDED == ReceiveBuffer(accept_message,MESSAGE_LENGTH,client->socket))
		{
			msg_type = GetMessageType(accept_message);
			switch(msg_type)
			{
			case PLAYERS_T:
				WaitForSingleObject(broadcast_mutex, INFINITE);
				//==============Critical Section Start==============//
					PlayersRoutine(accept_message,client,clients);
				//===============Critical Section End===============//
				ReleaseMutex(broadcast_mutex);
				break;
			case MESSAGE_T:
				WaitForSingleObject(broadcast_mutex, INFINITE);
				//==============Critical Section Start==============//
					MessageRoutine(accept_message,client,clients);
				//===============Critical Section End===============//
				ReleaseMutex(broadcast_mutex);
				break;
			case BROADCAST_T:
				WaitForSingleObject(broadcast_mutex, INFINITE);
				//==============Critical Section Start==============//
					BroadcastRoutine(accept_message,client,clients);
				 //===============Critical Section End===============//
				ReleaseMutex(broadcast_mutex);
				break;
			case TOSS_T:
				WaitForSingleObject(broadcast_mutex, INFINITE);
				//==============Critical Section Start==============//
					BroadcastMessage(accept_message,client->client_num,clients);
					SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,accept_message);
				 //===============Critical Section End===============//
				ReleaseMutex(broadcast_mutex);
				break;
			case TACK_T:
				WaitForSingleObject(broadcast_mutex, INFINITE);
				//==============Critical Section Start==============//
					play_result = TurnAckRoutine(accept_message,client,clients);
					if( PLAYER_WON == play_result) //turn was resulted in winning
					{
						SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,accept_message);
						SetMustEnd();				//stop main game loop (ServerGameManagementThread)
						done=TRUE;					//stop this thread loop
						retval=PLAYER_WON;
						SetEvent(client->done_turn_event);
					}
					else							//turn was not resulted in winning
					{
						SetEvent(client->done_turn_event);
					}
				 //===============Critical Section End===============//
				ReleaseMutex(broadcast_mutex);
				break;
			}
		}
		else //if receive failed
		{
			SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,
				"Lost connection to client %d. Exiting.\n",client->client_num);
			retval = RECV_ERROR;
			SetMustEnd();
		}
	}
	closesocket(client->socket);
	return retval;
}


EXIT_CODE AcceptClientsWithTimeout(CLIENT* clients, u_short server_port,int* num_of_active_clients)
{
	SOCKET				listen_socket;	
	HANDLE				AcceptClientsThreadHandle;
    AcceptClientsArgs	args;
	int					i;
	
	for(i = 0; i<NUM_OF_CLIENTS; i++)
	{
		clients[i].client_num=-1;			//mark as unused CLIENT object
	}

	//initial arguments for AcceptClientsThread
	args.listen_socket=				&listen_socket;
	args.clients=					clients;
	args.server_port=				&server_port;
	args.num_of_active_clients=		num_of_active_clients;

	//run AcceptClientsThread as thread and wait for it for "WAIT_FOR_CLIENTS_TIMEOUT" secs
	AcceptClientsThreadHandle = CreateThread(
											NULL,
											0,
											(LPTHREAD_START_ROUTINE)AcceptClientsThread,
											&args,
											0,
											NULL);	
	if(WAIT_TIMEOUT == WaitForSingleObject(AcceptClientsThreadHandle,WAIT_FOR_CLIENTS_TIMEOUT*1000))
	{
		TerminateThread(AcceptClientsThreadHandle,WAIT_TIMEOUT);
	}
	closesocket(listen_socket);
	CloseHandle(AcceptClientsThreadHandle);
	if(0 == *num_of_active_clients)
	{
		SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,NO_CLIENTS_LOG_FORMAT); //log no clients have connected
		return NO_PLAYERS_CONNECTED;
	}
	return SUCCESS;
}
EXIT_CODE AcceptClientsThread(AcceptClientsArgs* args)
{
	int					i=0;
	SOCKADDR_IN			service;
	SOCKET				new_client_socket;
	char				new_player_message[MESSAGE_LENGTH];
	HANDLE				broadcast_mutex;
	Sleep(THREAD_DEFUALT_SLEEP_TIME);

    *args->listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*args->listen_socket == INVALID_SOCKET)
	{
		SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,
			"listen_socket failed with error: %ld\n", WSAGetLastError());
		return CREATE_SOCKET_FAILED;
		//AddHandler
    }

    service.sin_family=			AF_INET;
    service.sin_addr.s_addr=	inet_addr(SERVER_ADDRESS_STR);
    service.sin_port=			htons(*args->server_port);

    if (SOCKET_ERROR == bind(*args->listen_socket,(SOCKADDR*)&service, sizeof (service))) 
	{
		SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,
			"bind listen_socket failed with error: %ld\n", WSAGetLastError());
		closesocket(*args->listen_socket);
		return BIND_SOCKET_ERROR;
		//AddHandler
    }
    if (listen(*args->listen_socket, SOMAXCONN) == SOCKET_ERROR) 
	{
		SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,
			"listen to listen_socket failed with error: %ld\n", WSAGetLastError());
		closesocket(*args->listen_socket);
		return LISTEN_ERROR;
		//AddHandler	
    }

	broadcast_mutex = OpenMutex(SYNCHRONIZE ,TRUE, BROADCAST_MUTEX_NAME);
	if(NULL == broadcast_mutex)
	{
		SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,
			"OpenMutex broadcast_mutex failed with error: %ld\n", GetLastError());
		closesocket(*args->listen_socket);
		return MUTEX_ERROR; 
		//AddHandler
	}
	//printf("Waiting for clients to connect...\n");
	for(i=0;i<NUM_OF_CLIENTS;i++)
	{
		// Accept the connection.
		new_client_socket = accept(*args->listen_socket, NULL, NULL );

		if ( new_client_socket == INVALID_SOCKET) 
		{
			SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,
				"accept failed with error: %ld\n", WSAGetLastError());
			i--;	//accept again to this place in array
			continue;
		} 

	WaitForSingleObject(broadcast_mutex, INFINITE);
	//==============Critical Section Start==============// accepting new client
		args->clients[i].socket=new_client_socket;
		args->clients[i].client_num=i;

		//printf("New client connect -> Handshake result: ");			

		//handshake with username
		if(CLIENT_REFUSED == HandshakeServerSide(&args->clients[i],args->clients))
		{
			//printf("%s refused. (%d active clients)\n",args->clients[i].username,*args->num_of_active_clients);
			closesocket(args->clients[i].socket);
			i--;
		}
		else
		{
			//run ServerCommunicationThread for each player who connected
			args->clients[i].thread_handle = CreateThread(
												NULL,
												0,
												(LPTHREAD_START_ROUTINE)ServerCommunicationThread,
												&(args->clients[i]),
												0,
												NULL);	
			*args->num_of_active_clients=i+1;
			//printf("%s(%c) approved. (%d active clients)\n",args->clients[i].username,args->clients[i].symbol,*args->num_of_active_clients);
			
			//notify the other players about the new player
			sprintf(new_player_message,NEW_PLAYER_MSG_FORMAT,args->clients[i].username,args->clients[i].symbol);
			SafelyOutput(g_logfile,&logfile_mutex,TRUE,TRUE,new_player_message);
			BroadcastMessage(new_player_message,i,args->clients); 
		}
	//===============Critical Section End===============//
	ReleaseMutex(broadcast_mutex);
	}
	CloseHandle(broadcast_mutex);
	return SUCCESS;
}

