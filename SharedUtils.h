/* SharedUtils.h
 * ===========================
 * Exercise 4
 * ===========================
 * Name: Assaf Ben Yishay
 * =========================== */
#pragma once
#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#define SERVER_ADDRESS_STR				"127.0.0.1"
#define THREAD_DEFUALT_SLEEP_TIME		10
#define MESSAGE_LENGTH					150
#define MAX_USERNAME_LENGTH				30
#define NUM_OF_CLIENTS					4
#define PLAYERS_SYMBOLS					"@#%*"
#define REFUSE_SYMBOL					'~'

//Messages formats
#define USERNAME_MSG_FORMAT				"username=%s\n"
#define SYMBOL_MSG_FORMAT				"%s your game piece is %c\n"
#define NEW_PLAYER_MSG_FORMAT			"New player joined the game: %s %c.\n"
#define REFUSE_MSG_FORMAT				"Connection to server refused. Exiting.\n"
#define ORDER_MSG_FORMAT				"The order of the players' in the game is %s"
	#define ORDER_MSG_DECIDE_LEN		40 //num of first chars of the MSG_FORMAT to decide if the received message is indeed this message
#define TURN_MSG_FORMAT					"Your turn to play.\n"
#define ANOTHERS_TURN_MSG_FORMAT		"It is now %s's turn to play.\n"
#define PRIVATE_MSG_FORMAT				"Private message from %s: %s\n"
#define USER_NOT_EXISTS_MSG_FORMAT		"User %s doesn't exist in the game.\n"
#define BROADCAT_MSG_FORMAT				"Broadcast from %s: %s\n"
#define PLAYERS_MSG_FORMAT				"%s-%c,"
#define TOSS_MSG_FORMAT					"Player %c (%s) drew a %d.\n"
#define TOSS_MSG_SCAN_FORMAT			"%*s %c %*s %*s %*c %d.\n"
#define WIN_MSG_FORMAT					"Player %s won the game. Congratulations.\n"
#define NOT_WIN_MSG_FORMAT				"Player %s didn't won the game. Congratulations.\n"
	//the next three defines used to decide if a recieved message is WIN or TOSS message or none of them
	#define TOSS_N_WIN_PREFIX			"Player"
	#define	TOSS_IDENTIFY_WORD			"drew"
	#define	WIN_IDENTIFY_WORD			"Congratulations"
	#define	NOT_WIN_IDENTIFY_WORD		"didn't"
#define TOSS_RESULT_FORMAT				"You (%c) drew a %d.\n"
#define NOT_IN_TURN_MSG_FORMAT			"Sorry, this is not your turn.\n"

#define TURN_ACK						"TACK %d"
	#define TURN_ACK_DECIDE_LEN			4 //num of first chars to compare
//UI commands strings
#define PLAYERS_STR						"players"
#define MESSAGE_STR						"message"
#define BROADCAST_STR					"broadcast"
#define PLAY_STR						"play"

typedef enum {	UNKNOWN_T, PLAYERS_T, MESSAGE_T, BROADCAST_T, PLAY_T, TOSS_T, ORDER_T, TURN_T, TACK_T } MSG_TYPE;

typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;

extern	char*	g_logfile;
extern	HANDLE	logfile_mutex;
extern	HANDLE	AllThreadsMustEnd;

/**
 * SendBuffer() uses a socket to send a buffer.
 *
 * Accepts:
 * -------
 * Buffer - the buffer containing the data to be sent.
 * BytesToSend - the number of bytes from the Buffer to send.
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if sending succeeded
 * TRNS_FAILED - otherwise
 */
TransferResult_t SendBuffer( const char* Buffer, int BytesToSend, SOCKET sd );

/**
 * Accepts:
 * -------
 * ReceiveBuffer() uses a socket to receive a buffer.
 * OutputBuffer - pointer to a buffer into which data will be written
 * OutputBufferSize - size in bytes of Output Buffer
 * BytesReceivedPtr - output parameter. if function returns TRNS_SUCCEEDED, then this 
 *					  will point at an int containing the number of bytes received.
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if receiving succeeded
 * TRNS_DISCONNECTED - if the socket was disconnected
 * TRNS_FAILED - otherwise
 */ 
TransferResult_t ReceiveBuffer( char* OutputBuffer, int RemainingBytesToReceive, SOCKET sd );

/*
 *   Description: CreateLogFile - Create new empty output file
 *	-------------------------------------------------------
 *   char* file_name - string contains the name of the output file 
 *   FILE** file_handle -pointer to FILE*
 *	-------------------------------------------------------
 *   returns: exit code-	OPEN_FILE_ERROR - if create new report file failed
 *							SUCCESS - no error occurred
 */
EXIT_CODE CreateLogFile(char* file_name, FILE** file_pointer);

/*
 *   Description: Safely print to file using the given mutex to avoid collision between threads
 *	 Also can use to print to console when 
 *	-------------------------------------------------------
 *   FILE* fp:				string contains the name of the output file 
 *   HANDLE* mutex:			pointer to a mutex, can be NULL if not needed.
 *   BOOL console:			TRUE - to print to console.
 *   BOOL log:				TRUE - to print to log.
 *   const char *format:	format of the line as defined in printf, fprintf functions
 *   ...					other args
 *	-------------------------------------------------------
 *   returns: vfprintf return values
 */
int SafelyOutput(char* file, HANDLE* mutex, BOOL console, BOOL log, const char *format, ...);

/*
 *   Description: GetMessageType - use to determined MSG_TYPE of a given message
 *	(from server or from client) 
 *	-------------------------------------------------------
 *   char* message - string contains the message to examine 
 *	-------------------------------------------------------
 *   returns: vfprintf return values
 */
MSG_TYPE GetMessageType(char* message);

/*
 *   Description: Check if thread must end since error occurred in another thread
 *	 using Event
 *	-------------------------------------------------------
 *	 no arguments
 *	-------------------------------------------------------
 *   returns: none
 */
BOOL CheckMustEnd();

/*
 *   Description: Set event for all threads to end
 *	-------------------------------------------------------
 *	 no arguments
 *	-------------------------------------------------------
 *   returns: none
 */
void SetMustEnd();

#endif