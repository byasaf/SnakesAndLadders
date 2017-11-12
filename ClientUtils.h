/* ClientUtils.h
 * ===========================
 * Exercise 4
 * ===========================
 * Name: Assaf Ben Yishay
 * =========================== */
#pragma once
#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H
#include "SharedUtils.h"
#include "Game.h"


#define TO_SERVER_LOG_FORMAT	"Sent to server: %s"
#define FROM_SERVER_LOG_FORMAT	"Received from server: %s"
#define CONNECT_SUCCESS_FORMAT	"Connected to server on port %hu.\n"
#define CONNECT_ERROR_FORMAT	"Failed connecting to server on port %hu.\n"

typedef struct{
	u_short		server_port;
	SOCKET		socket;
	char		username[MAX_USERNAME_LENGTH];
	char		symbol;
	GAME_DATA	game_data;
}CLIENT_DATA;

/*
 *   Description: InitialClientSocket - creating socket and connectig to server
 *	-------------------------------------------------------
 *   CLIENT_DATA* client_data	-pointer to a struct holds the client's data (socket, username etc)
 *	-------------------------------------------------------
 *   returns: exit code -	CREATE_SOCKET_FAILED, CONNECT_TO_SERVER_FAILED ,
 *							CONNECT_TO_SERVER_FAILED , WSA_STARTUP_ERROR 
 *							- if any of these errors occured while connecting to server	
 *							SUCCESS - no errors occurred 
 */
EXIT_CODE InitialClientSocket(CLIENT_DATA* client_data);

/*
 *   Description: HandshakeClientSide - manage the handshake proccess with the server-
 *	 sends username to server than wait for respones from server (HandshakeServerSide)
 *	 a game piece symbol or refuse in case of duplicate username.
 *	-------------------------------------------------------
 *   CLIENT_DATA* client_data	-pointer to a struct holds the client's data (socket, username etc)
 *	-------------------------------------------------------
 *   returns: exit code -	CLIENT_REFUSED - if username already exists
 *							SUCCESS - no errors occurred 
 */
EXIT_CODE HandshakeClientSide(CLIENT_DATA* client_data);

/*
 *   Description: CheckUIMessage - Checks string arrive from user if valid according to first word.
 *	 In case of unknown type or illegal arguments, output error to logfile
 *	-------------------------------------------------------
 *   char* message - string from user keyboard
 *	 MSG_TYPE* type - use to return the detemined MSG_TYPE to the caller
 *	 char* cmd - use to return the first word of the message to the caller
 *	-------------------------------------------------------
 *   returns: exit code -	ILEGAL_MSG_ARGS - if arguments are illegal
 *							ILEGAL_MSG_CMD - if could not determine the command type
 *							SUCCESS - no errors occurred 
 */
EXIT_CODE CheckUIMessage(char* message, MSG_TYPE* type, char* cmd);

/*
 *   Description: SymbolToNum - Converts players' game piece symbol to its client_num
 *	-------------------------------------------------------
 *   char* symbol - game piece symbol
 *	-------------------------------------------------------
 *   returns: int -	the client_num who the symbol belongs to
 */
int SymbolToNum(char symbol);

/*
 *   Description: OrderRoutine - been used when a message containing the gaming order
 *	 from the server. Counts the number of active players in the game and gets thier symbols.
 *	-------------------------------------------------------
 *   CLIENT_DATA* client_data - pointer to a struct holds the client's data (socket, username etc)
 *	 int* num_of_clients - number of active players
 *	-------------------------------------------------------
 *   returns: exit code - always return SUCCESS
 */
EXIT_CODE OrderRoutine(CLIENT_DATA* client_data, int* num_of_clients);

/*
 *   Description: PlayRoutine - been used when "Your turn to play" message arrives from server.
 *	 This routing draw a toss than move the player' game piece (using MovePlayer)
 *	 and draw the board in its new state (using DrawGameBoard),
 *	-------------------------------------------------------
 *   CLIENT_DATA* client_data - pointer to a struct holds the client's data (socket, username etc)
 *	 int* num_of_clients - number of active players
 *	 HANDLE* players_turn_event - pointer to events which been singaled when it is this clien't turn to play.
 *	 char* buff - string to hold the toss result message to be sent to other players
 *	-------------------------------------------------------
 *   returns: exit code -	NOT_IN_TURN - if tried to play not in turn (players_turn_event is not singaled)
 *							PLAYER_WON - if this move resulted in plyer's victory
 *							PLAYER_NOT_WON -if this move was not reulted in player's victory
 *							
 */
EXIT_CODE PlayRoutine(CLIENT_DATA* client_data, int num_of_clients, HANDLE* players_turn_event, char* buff);

/*
 *   Description: PlayAnotherPlayerRoutine - been used when a toss result of another player arrives from server.
 *	 This routing extract the toss result and the players symbole from the message than move the player' game piece (using MovePlayer)
 *	 and draw the board in its new state (using DrawGameBoard),
 *	-------------------------------------------------------
 *   CLIENT_DATA* client_data - pointer to a struct holds the client's data (socket, username etc)
 *	 int* num_of_clients - number of active players
 *	 HANDLE* players_turn_event - pointer to events which been singaled when it is this clien't turn to play.
 *	 char* buff - string to hold the toss result message to be sent to other players
 *	-------------------------------------------------------
 *   returns: exit code -	PLAYER_WON - if this move resulted in plyer's victory
 *							PLAYER_NOT_WON -if this move was not reulted in player's victory
 *							
 */
EXIT_CODE PlayAnotherPlayerRoutine(CLIENT_DATA* client_data, int num_of_clients, char* buff);

#endif