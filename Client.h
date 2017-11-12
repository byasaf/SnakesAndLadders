/* Client.h
 * ===========================
 * Exercise 4
 * ===========================
 * Name: Assaf Ben Yishay
 * =========================== */
#pragma once
#ifndef CLIENT_H
#define CLIENT_H
#include "ClientUtils.h"

typedef struct{
	CLIENT_DATA*	client_data;
	HANDLE*			semaphore;
	HANDLE*			done_semaphore;
	char*			message;
}THREAD_ARGS;

/*
 *   Description: Main Client - only runs the client mode, starts the EngineThread
 *	-------------------------------------------------------
 *   char* logfile: string holds log's file name
 *   u_short server_port: listening port of the server
 *   char* username: string holds the username
 *	-------------------------------------------------------
 *   returns: exit code - same as EngineThread list since only mirroring its exit codes						
 */
EXIT_CODE MainClient(char* logfile, u_short server_port, char* username);

/*
 *   Description: EngineThread - manage the connect and handshake proccess with the server,
 *	 than waiting in loop to UIThread or ClientCommunicationThread and proccess the information arrived from them
 *	-------------------------------------------------------
 *   char* logfile: string holds log's file name
 *   u_short server_port: listening port of the server
 *   char* username: string holds the username
 *	-------------------------------------------------------
 *   returns: exit code-	EVENT_ERROR - if create event failed						
 *							SEMAPHORE_ERROR - if create semaphore failed
 *							OPEN_FILE_ERROR - if create a new log file failed
 *							CLIENT_REFUSED - if username already exists in server's database
 *							CREATE_SOCKET_FAILED, CONNECT_TO_SERVER_FAILED ,
 *							CONNECT_TO_SERVER_FAILED , WSA_STARTUP_ERROR 
 *									- if any of these errors occured while connecting to server
 *							RECV_ERROR - if connection to server lost after established connection
 *							PLAYER_WON - if this client's player won the game
 *							SUCCESS - no errors occurred
 */
EXIT_CODE EngineThread(char* logfile, u_short server_port, char* username);

/*
 *   Description: UIThread - wait to user to enter a string using the keyboard 
 *	 than pass control to EngineThread for proccesing the information. This proccess happaned in a loop.
 *	-------------------------------------------------------
 *   THREAD_ARGS* args:		args->	CLIENT_DATA*	client_data		-holds the client's data (socket, username etc)
 *									HANDLE*			semaphore		-become signaled when new data arrives
 *									HANDLE*			done_semaphore	-signaled when EngineThread done proccessing the previous data
 *									char*			message			-holds the string arrived from user
 *	-------------------------------------------------------
 *   returns: exit code- alwaye return SUCCESS						
 */
EXIT_CODE UIThread(THREAD_ARGS* args);

/*
 *   Description: ClientCommunicationThread - receive data from server string using client's socket 
 *	 than pass control to EngineThread for proccesing the information. This proccess happaned in a loop.
 *	-------------------------------------------------------
 *   THREAD_ARGS* args:		args->	CLIENT_DATA*	client_data		-holds the client's data (socked, username etc)
 *									HANDLE*			semaphore		-become signaled when new data arrives
 *									HANDLE*			done_semaphore	-signaled when EngineThread done proccessing the previous data
 *									char*			message			-holds the string arrived from server
 *	-------------------------------------------------------
 *   returns: exit code-	RECV_ERROR - if lost connection to server
 *							SUCCESS - no errors occurred
 */
EXIT_CODE ClientCommunicationThread(THREAD_ARGS* args);


#endif