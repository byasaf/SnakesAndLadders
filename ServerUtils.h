/* ServerUtils.h
 * ===========================
 * Exercise 4
 * ===========================
 * Name: Assaf Ben Yishay
 * =========================== */
#pragma once
#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H
#include "SharedUtils.h"

#define WAIT_FOR_CLIENTS_TIMEOUT		60//timeout in sec.
#define WAIT_WORKING_THREADS_TIMEOUT	1//timeout in sec.

#define BROADCAST_MUTEX_NAME		L"BroadcastMutex"

#define MESSAGE_CMD_LOG_FORMAT		"Private message sent from %s to %s: %s\n"
#define BROADCAST_CMD_LOG_FORMAT	"Broadcast message from user %s: %s\n"
#define WELCOME_LOG_FORMAT			"Players' game pieces' selection broadcasted to all users.\n"
#define PLAYERS_CMD_LOG_FORMAT		"Players' game pieces' selection sent to %s.\n"
#define NO_CLIENTS_LOG_FORMAT		"No players connected, exiting...\n"

typedef struct{
	SOCKET		socket;
	HANDLE		thread_handle;
	HANDLE		done_turn_event;
	int			client_num;
	char		username[MAX_USERNAME_LENGTH];
	char		symbol;
}CLIENT;

/*
 *   Description: HandshakeServerSide - manage the handshake proccess with a single client- works with HandshakeClientSide in the client
 *	 gets username from client than checks if username already exists, sends approval (or refuse) message with player's symbol to client
 *	-------------------------------------------------------
 *   CLIENT* new_client	- struct holds the data of the new_client
 *	 CLIENT* clients - pointer to array of CLIENT objects to add the new clients sockets 
 *	-------------------------------------------------------
 *   returns: exit code -	CLIENT_REFUSED - if username already exists
 *							SUCCESS - no errors occurred 
 */
EXIT_CODE HandshakeServerSide(CLIENT* new_client, CLIENT* clients);

/*
 *   Description: BroadcastMessage - send a message to all the connected clients
 *	-------------------------------------------------------
 *   const char* buffer	- string to send
 *	 int sender - the client number of the sender - this client won't get the message use -10 to send to all 
 *	 CLIENT* clients - pointer to array of CLIENT objects hold the data of all connected clients 
 *	-------------------------------------------------------
 *   returns: exit code -	SEND_ERROR - if send to one or more clients failed
 *							SUCCESS - no errors occurred 
 */
EXIT_CODE BroadcastMessage(const char* buffer, int sender, CLIENT* clients);

/*
 *   Description: UnicastMessage - send a message to a specific client
 *	-------------------------------------------------------
 *   const char* buffer	- string to send
 *	 int dest - the destination client number
 *	 CLIENT* clients - pointer to array of CLIENT objects hold the data of all connected clients 
 *	-------------------------------------------------------
 *   returns: exit code -	SEND_ERROR - if send failed
 *							SUCCESS - no errors occurred 
 */
EXIT_CODE UnicastMessage(const char* buffer, int dest, CLIENT* clients);

/*
 *   Description: FindUsername - find client number (index) using username
 *	-------------------------------------------------------
 *   const char* username - string holds the username
 *	 CLIENT* clients - pointer to array of CLIENT objects hold the data of all connected clients 
 *	-------------------------------------------------------
 *   returns: int - index of this username or -1 if user not exits
 */
int FindUsername(const char* username, CLIENT* clients);

/*
 *   Description: PlayersRoutine - been used when a "players" command arrives from one of the clients.
 *	 This routine send in response the list of players and thier symbols
 *	-------------------------------------------------------
 *   char* message - the message content
 *	 CLIENT* sender - the data of the requesting client
 *	 CLIENT* clients - pointer to array of CLIENT objects hold the data of all connected clients
 *	-------------------------------------------------------
 *   returns: exit code -	SEND_ERROR - if send failed
 *							SUCCESS - no errors occurred 
 */
EXIT_CODE PlayersRoutine(char* message, CLIENT* sender, CLIENT* clients);

/*
 *   Description: MessageRoutine - been used when a "message" command arrives from one of the clients.
 *	 This routine forwarding the message to the right destination
 *	-------------------------------------------------------
 *   char* message - the message content
 *	 CLIENT* sender - the data of the requesting client
 *	 CLIENT* clients - pointer to array of CLIENT objects hold the data of all connected clients
 *	-------------------------------------------------------
 *   returns: exit code -	SEND_ERROR - if send failed
 *							SUCCESS - no errors occurred 
 */
EXIT_CODE MessageRoutine(char* message, CLIENT* sender, CLIENT* clients);

/*
 *   Description: BroadcastRoutine - been used when a "broadcast" command arrives from one of the clients.
 *	 This routine forwarding the message to all of the clients
 *	-------------------------------------------------------
 *   char* message - the message content
 *	 CLIENT* sender - the data of the requesting client
 *	 CLIENT* clients - pointer to array of CLIENT objects hold the data of all connected clients
 *	-------------------------------------------------------
 *   returns: exit code -	SEND_ERROR - if send to one or more clients failed
 *							SUCCESS - no errors occurred  
 */
EXIT_CODE BroadcastRoutine(char* message, CLIENT* sender, CLIENT* clients);

/*
 *   Description: TurnAckRoutine - been used when a "turn done" acknoldgement arrives from one of the clients.
 *	 Atfer each move on the board, the player sends the turn result (player won or not), this routine gets this ack and notify the other players if player won
 *	-------------------------------------------------------
 *   char* message - the message content
 *	 CLIENT* sender - the data of the requesting client
 *	 CLIENT* clients - pointer to array of CLIENT objects hold the data of all connected clients
 *	-------------------------------------------------------
 *   returns: exit code -	PLAYER_WON - if last move resulted in victory
 *							PLAYER_NOT_WON - if last move was not resulted in victory  
 */
EXIT_CODE TurnAckRoutine(char* message, CLIENT* sender, CLIENT* clients);

/*
 *   Description: CleanupWorkerThreads - Terminate all working threads and close their sockets
 *	-------------------------------------------------------
 *	 CLIENT* clients - pointer to array of CLIENT objects hold the data of all connected clients
 *	 int num_of_active_clients - number of active clients to clean
 *	-------------------------------------------------------
 *   returns: exit code - none
 */
void CleanupWorkerThreads(CLIENT* clients, int num_of_active_clients);

#endif