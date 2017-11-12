/* main.c
 * ===========================
 * Exercise 4
 * ===========================
 * Name: Assaf Ben Yishay
 * =========================== */
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
/* =========================== */
#include "main.h"
#include "Errors.h"
#include "Server.h"
#include "Client.h"

EXIT_CODE main(int argc, char **argv)
{

	if( 0==strcmp(argv[1],SERVER_ARG_STR) && (4 == argc) )			
	{
		//server command
		return MainServer(argv[2],atoi(argv[3]));
	}
	else if( 0==strcmp(argv[1],CLIENT_ARG_STR) && (5 == argc) )	
	{
		//client command
		return MainClient(argv[2],atoi(argv[3]),argv[4]);
	}
	else
	{
		printf("Unknown arguments. Exiting");
		return UNKNOWN_ARGUMENTS;
	}
}
