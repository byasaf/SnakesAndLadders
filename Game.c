/* Game.c
 * ===========================
 * Exercise 4
 * ===========================
 * Name: Assaf Ben Yishay
 * =========================== */
#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
/* =========================== */
#include "Errors.h"
#include "Game.h"

void InitializeGameData(GAME_DATA* game_data, char symbols[], int num_of_players)
{
	int i,j;
	game_data->num_of_players=num_of_players;
	strcpy(game_data->symbols,symbols);
	BuildGameBoard(game_data->board);
	for( i=0 ; i<game_data->num_of_players; i++) //set players' postions to 0
	{
		game_data->players_positions[i] = 0;
	}
	
	for( j=1 ; j<BOARD_SIZE ; j++)	//set all CELL's players' flags on borad to FALSE
	{
		for( i=0 ; i<game_data->num_of_players; i++)
		{
			game_data->board[j].players[i]=FALSE;
		}
	}
}
void BuildGameBoard(CELL board[BOARD_SIZE])
{
	int i;
	for(i=1 ; i<BOARD_SIZE ; i++)
	{
		board[i].num=i;
		board[i].type=EMPTY_CELL;
		board[i].partner=0;
	}
	
	//create snakes
	CreateSnakeOrLadder(board,TRUE,17,7);
	CreateSnakeOrLadder(board,TRUE,54,34);
	CreateSnakeOrLadder(board,TRUE,62,19);
	CreateSnakeOrLadder(board,TRUE,64,60);
	CreateSnakeOrLadder(board,TRUE,87,24);
	CreateSnakeOrLadder(board,TRUE,93,73);
	CreateSnakeOrLadder(board,TRUE,95,75);
	CreateSnakeOrLadder(board,TRUE,99,78);
	
	//create ladders
	CreateSnakeOrLadder(board,FALSE,14,4);
	CreateSnakeOrLadder(board,FALSE,31,9);
	CreateSnakeOrLadder(board,FALSE,38,20);
	CreateSnakeOrLadder(board,FALSE,84,28);
	CreateSnakeOrLadder(board,FALSE,59,40);
	CreateSnakeOrLadder(board,FALSE,67,51);
	CreateSnakeOrLadder(board,FALSE,81,63);
	CreateSnakeOrLadder(board,FALSE,91,71);

//easy game
	/*
	CreateSnakeOrLadder(board,FALSE,95,4);
	CreateSnakeOrLadder(board,FALSE,95,9);
	CreateSnakeOrLadder(board,FALSE,95,20);
	CreateSnakeOrLadder(board,FALSE,95,28);
	CreateSnakeOrLadder(board,FALSE,95,40);
	CreateSnakeOrLadder(board,FALSE,95,51);
	CreateSnakeOrLadder(board,FALSE,95,63);
	CreateSnakeOrLadder(board,FALSE,95,71);
	*/
}
void CreateSnakeOrLadder(CELL board[BOARD_SIZE], BOOL snake, int top, int bottom)
{
	switch(snake)
	{
	case TRUE: //snake
		board[top].type=SNAKE_TOP;
		board[bottom].type=SNAKE_BOTTOM;
		break;
	case FALSE: //ladder
		board[top].type=LADDER_TOP;
		board[bottom].type=LADDER_BOTTOM;
		break;
	
	}
		board[top].partner=bottom;
		board[bottom].partner=top;
}
void DrawGameBoard(CELL board[BOARD_SIZE], char* symbols, int num_of_players)
{
	int i,j,k,index,row,col;
	for(j=0;j<BOARD_WIDTH*CELL_TEXT_LEN+1;j++, printf("-")); //seperation line
	printf("\n");
	for(i=0 ; i<BOARD_WIDTH ; i++)
	{
		for(j=0 ; j<BOARD_WIDTH ; j++)
		{
							row=i;
			if (0 == i%2)	col=j;
			else			col=BOARD_WIDTH-j-1;
			index= BOARD_SIZE-1 - (row * BOARD_WIDTH + col);
			UpdateCellText(&board[index],symbols,num_of_players);
			printf("|%s",board[index].text);
		}
		printf("|\n");
		for(k=0;k<BOARD_WIDTH*CELL_TEXT_LEN+1;k++, printf("-")); //seperation line
		printf("\n");
	}
}
void UpdateCellText(CELL* cell, char* symbols, int num_of_players)
{
	char	temp[CELL_TEXT_LEN];
	int		i,j=0;

	for(i=0;i<num_of_players;i++)
	{
		if(TRUE == cell->players[i])
		{
			temp[j]=symbols[i]; //building the symbols string of this specific cell
			j++;
		}
	}
	if(0 != j) //at least one player is here
	{
		switch(j)
		{
		case 1:
			sprintf(cell->text,PLAYERS_1_FORMAT,temp[0]);
			break;
		case 2:
			sprintf(cell->text,PLAYERS_2_FORMAT,temp[0],temp[1]);
			break;
		case 3:
			sprintf(cell->text,PLAYERS_3_FORMAT,temp[0],temp[1],temp[2]);
			break;
		case 4:
			sprintf(cell->text,PLAYERS_4_FORMAT,temp[0],temp[1],temp[2],temp[3]);
			break;
		}
		return;
	}
	//there are no players in here:
	if( BOARD_SIZE-1 == cell->num)
	{
		sprintf(cell->text,LAST_CELL_FORMAT); //this is the 100 cell and there are no players in here
		return;
	}
	switch(cell->type)
	{
		case EMPTY_CELL:
			sprintf(cell->text,EMPTY_CELL_FORMAT,cell->num);
			break;
		case SNAKE_TOP:
			sprintf(cell->text,SNAKE_TOP_FORMAT,cell->num);
			break;
		case SNAKE_BOTTOM:
			sprintf(cell->text,SNAKE_BOTTOM_FORMAT,cell->num);
			break;
		case LADDER_TOP:
			sprintf(cell->text,LADDER_TOP_FORMAT,cell->num);
			break;
		case LADDER_BOTTOM:
			sprintf(cell->text,LADDER_BOTTOM_FORMAT,cell->num);
			break;
	}
	
}
int toss()
{
	time_t t;
	srand((unsigned) time(&t));
	return rand()%6+1;
}
BOOL MovePlayer(CELL board[BOARD_SIZE], int player_num, int players_positions[], int toss)
{
	board[players_positions[player_num]].players[player_num]=FALSE;	//set that the cell is no longer holds this player
	players_positions[player_num]+=toss;							//calc new player position
	if(BOARD_SIZE-1 <= players_positions[player_num])				//check if won
	{
		return TRUE;
	}
	switch(board[players_positions[player_num]].type)				//if not won - check new postion cell type
	{
		case EMPTY_CELL:
		case SNAKE_BOTTOM:
		case LADDER_TOP:
			board[players_positions[player_num]].players[player_num] = TRUE;	//save new location and return		
			break;
		case SNAKE_TOP:												//if snake head or ladder bottom
		case LADDER_BOTTOM:											//go to linked cell, save new position and return
			players_positions[player_num]=board[players_positions[player_num]].partner; 
			board[players_positions[player_num]].players[player_num] = TRUE;
			break;
	}
	return FALSE;
}