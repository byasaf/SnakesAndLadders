/* Game.h
 * ===========================
 * Exercise 4
 * ===========================
 * Name: Assaf Ben Yishay
 * =========================== */
#pragma once
#ifndef GAME_H
#define GAME_H
#include "SharedUtils.h"

#define	BOARD_WIDTH				10
#define BOARD_SIZE				BOARD_WIDTH*BOARD_WIDTH+1
#define CELL_TEXT_LEN			7

#define EMPTY_CELL_FORMAT		"  %0.2d  "
#define SNAKE_TOP_FORMAT		"  %0.2dv "
#define SNAKE_BOTTOM_FORMAT		"  %0.2d_ "
#define LADDER_TOP_FORMAT		" =%0.2d  "
#define LADDER_BOTTOM_FORMAT	" ^%0.2d  "
#define LAST_CELL_FORMAT		"  100 "

#define PLAYERS_1_FORMAT		"   %c  "
#define PLAYERS_2_FORMAT		"  %c%c  "
#define PLAYERS_3_FORMAT		"  %c%c%c "
#define PLAYERS_4_FORMAT		" %c%c%c%c "

typedef enum { EMPTY_CELL, SNAKE_TOP, SNAKE_BOTTOM, LADDER_TOP, LADDER_BOTTOM } CELL_TYPE;
typedef struct {
	int			num;
	int			partner;
	BOOL		players[NUM_OF_CLIENTS]; //flags to represent which player stands on this cell
	CELL_TYPE	type;
	char		text[CELL_TEXT_LEN];	//text field to print to console
} CELL;


//struct to hold the game data, each client owns one object of the struct
typedef struct {
	CELL	board[BOARD_SIZE];
	int		num_of_players;
	int		players_positions[NUM_OF_CLIENTS];
	char	symbols[NUM_OF_CLIENTS+1];
}GAME_DATA;

/*
 *   Description: InitializeGameData - Initializing GAME_DATA object for a new game
 *	 Create board using BuildGameBoard and sets all payers postions to 0;
 *	-------------------------------------------------------
 *   GAME_DATA* game_data - GAME_DATA to initialize, all client has one.	
 *	 char symbols[]	- string holds the symbols of the players						
 *	 int num_of_players	- the number of active clients							
 *	-------------------------------------------------------
 *   returns: none						
 */
void InitializeGameData(GAME_DATA* game_data, char symbols[], int num_of_players);

/*
 *   Description: BuildGameBoard - Create game board from CELL objects array.
 *	 Builds the snakes and the ladder using CreateSnakeOrLadder calls
 *	-------------------------------------------------------
 *   CELL board[BOARD_SIZE] - array of CELLs represents the whole board.							
 *	-------------------------------------------------------
 *   returns: none						
 */
void BuildGameBoard(CELL board[BOARD_SIZE]);

/*
 *   Description: CreateSnakeOrLadder - Builds snake or ladder on the gameboard
 *	-------------------------------------------------------
 *   CELL board[BOARD_SIZE] - array of CELLs represents the whole board.
 *	 BOOL snake - TRUE - create snake, FALSE - create ladder
 *	 int top - the number of the CELL where the top edge of the new snake/ladder should be
 *	 int bottom - the number of the CELL where the bottom edge of the new snake/ladder should be
 *	-------------------------------------------------------
 *   returns: none						
 */
void CreateSnakeOrLadder(CELL board[BOARD_SIZE], BOOL snake, int top, int bottom);

/*
 *   Description: DrawGameBoard - draw a gameboard on the console screen
 *	-------------------------------------------------------
 *   CELL board[BOARD_SIZE] - array of CELLs represents the whole board to draw	
 *	 char symbols[]	- string holds the symbols of the players						
 *	 int num_of_players	- the number of active clients	
 *	-------------------------------------------------------
 *   returns: none						
 */
void DrawGameBoard(CELL board[BOARD_SIZE], char* symbols, int num_of_players);

/*
 *   Description: UpdateCellText - update the CELL.text field of a given CELL
 *	 accordin to the other fields of this CELL object.
 *	-------------------------------------------------------
 *   CELL* cell - a pointer to a CELL object to update
 *	 char symbols[]	- string holds the symbols of the players						
 *	 int num_of_players	- the number of active clients	
 *	-------------------------------------------------------
 *   returns: none						
 */
void UpdateCellText(CELL* cell, char* symbols, int num_of_players);

/*
 *   Description: toss - draw a toss.
 *	-------------------------------------------------------
 *	 no arguments
 *	-------------------------------------------------------
 *   returns: int - a random number between 1 to 6;				
 */
int toss();

/*
 *   Description: MovePlayer - draw a gameboard on the console screen
 *	-------------------------------------------------------
 *   CELL board[BOARD_SIZE] - array of CELLs represents the game board
 *	 int player_num - the players' number to move
 *	 int players_positions[] - array holds the current player's postions on the board						
 *	 int toss - number of steps to move the player forward.	
 *	-------------------------------------------------------
 *   returns: none						
 */
BOOL MovePlayer(CELL board[BOARD_SIZE], int player_num, int players_positions[], int toss);


#endif