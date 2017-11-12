/* Errors.h
 * ===========================
 * Exercise 4
 * ===========================
 * Name: Assaf Ben Yishay
 * =========================== */
#pragma once
#ifndef ERRORS_H
#define ERRORS_H

//exit and error codes
typedef enum{
			SUCCESS,
			UNKNOWN_ARGUMENTS,
			WSA_STARTUP_ERROR,
			MEMORY_ALLOCATION_FAILED,
			WAIT_THREAD_TIMEOUT,
			OPEN_FILE_ERROR,
			MUTEX_ERROR,
			EVENT_ERROR,
			SEMAPHORE_ERROR,
			CREATE_SOCKET_FAILED,
			BIND_SOCKET_ERROR,
			LISTEN_ERROR,
			NO_PLAYERS_CONNECTED,
			CONNECT_TO_SERVER_FAILED,
			CLIENT_REFUSED,
			SEND_ERROR,
			RECV_ERROR,
			USER_NOT_EXISTS,
			ILEGAL_MSG_CMD,
			ILEGAL_MSG_ARGS,
			NOT_IN_TURN,
			PLAYER_WON,
			PLAYER_NOT_WON
}EXIT_CODE;

#endif