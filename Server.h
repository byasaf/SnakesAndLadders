/* Server.h
 * ===========================
 * Exercise 4
 * ===========================
 * Name: Assaf Ben Yishay
 * =========================== */
#pragma once
#ifndef SERVER_H
#define SERVER_H
#include "ServerUtils.h"

typedef struct{
	SOCKET*		listen_socket;
	CLIENT*		clients;
	u_short*	server_port;
	int*		num_of_active_clients;
}AcceptClientsArgs;

/*
 *   Description: Main Server - only runs the server mode, starts the ServerGameManagementThread
 *	-------------------------------------------------------
 *   char* logfile: string holds log's file name
 *   u_short server_port: listening port of the server
 *	-------------------------------------------------------
 *   returns: exit code - same as ServerGameManagementThread list since only mirroring its exit codes						
 */
EXIT_CODE MainServer(char* logfile, u_short server_port);

/*
 *   Description: ServerGameManagementThread - Main thread, manage the connect and handshake proccess with the all clients,
 *	 than entering to the main loop which manages the game (set turns and send notifications to clients)
 *	-------------------------------------------------------
 *   char* logfile: string holds log's file name
 *   u_short server_port: listening port of the server
 *	-------------------------------------------------------
 *   returns: exit code-	EVENT_ERROR - if create event failed						
 *							MUTEX_ERROR - if create mutex failed
 *							OPEN_FILE_ERROR - if create a new log file failed
 *							CREATE_SOCKET_FAILED, CONNECT_TO_SERVER_FAILED ,
 *							CONNECT_TO_SERVER_FAILED , WSA_STARTUP_ERROR 
 *									- if any of these errors occured while creating sockets with clients
 *							RECV_ERROR - if connection to a client lost after established connection
 *							SEND_ERROR - if connection to a client lost after established connection
 *							SUCCESS - no errors occurred
 */
EXIT_CODE ServerGameManagementThread(char* logfile, u_short server_port);

/*
 *   Description: ServerCommunicationThread - manage the connection between the server and a single client (using AcceptClientsWithTimeout)
 *	 Wait in a loop for transsmition from client than runs the right routine to handle it
 *	-------------------------------------------------------
 *   CLIENT* client: struct holds the data of the client which this thread handles
 *	-------------------------------------------------------
 *   returns: exit code-	RECV_ERROR - if connection to the client lost after established connection					
 *							SUCCESS - no errors occurred
 */
EXIT_CODE ServerCommunicationThread(CLIENT* client);

/*
 *   Description: AcceptClientsWithTimeout - open listening socket and then runs the AcceptClientsThread as thread
 *	 which accepts sockets with the clients. AcceptClientsWithTimeout terminate AcceptClientsThread when the timeout
 *	 set by WAIT_FOR_CLIENTS_TIMEOUT passed. 
 *	-------------------------------------------------------
 *   CLIENT* clients:				pointer to array of CLIENT objects to hold the new clients sockets
 *	 u_short server_port:			port number of listen socket
 *	 int* num_of_active_clients:	pointer to int to holds the final number of active clients		
 *	-------------------------------------------------------
 *   returns: exit code-	NO_PLAYERS_CONNECTED - if no clients have connected after the timeout passed					
 *							SUCCESS - no errors occurred
 */
EXIT_CODE AcceptClientsWithTimeout(CLIENT* clients, u_short server_port,int* num_of_active_clients);

/*
 *   Description: AcceptClientsThread - a thread activated by AcceptClientsWithTimeout to accept clients connections
 *	-------------------------------------------------------
 *   AcceptClientsArgs* args:	arguments for this thread:	
 *						args->	SOCKET*		listen_socket	-pointer to a socket for listening. 
 *															 Allows AcceptClientsWithTimeout to close this socket from outsid after this thread been terminated
 *								CLIENT*		clients;		-pointer to array of CLIENT objects to hold the new clients sockets
 *								u_short server_port:		-port number of listen socket
 *								int* num_of_active_clients:	-pointer to int to holds the final number of active clients	
 *	-------------------------------------------------------
 *   returns: exit code-	NO_PLAYERS_CONNECTED - if no clients have connected after the timeout passed					
 *							SUCCESS - no errors occurred
 */
EXIT_CODE AcceptClientsThread(AcceptClientsArgs* args);


#endif