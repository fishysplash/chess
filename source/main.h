/* -------------------------------
   main.h
   
   header file for main module
   
   (C) 2008 David Barone
    
   -------------------------------*/
#include "chessstate.h"   

#ifndef MAIN_H
	#define MAIN_H


	// gets column (0-7) for a square
	#define COL(square) (square % 8)				
	
	// gets row (0-7) for a square
	#define ROW(square) ((int)square / 8)			

	// gets the square number for a square in alg notation
	int get_square_number(char *algebraic_notation);	
	
	// gets the algebraic chess notation for a square
	char *get_square_text(int square_number);			

	typedef struct
	{
		CHESS_STATE_STR chess_state_str;
		char move[20];
		unsigned int time_limit;	// time limit in seconds
		unsigned int flg_xml : 1;
		unsigned int flg_new : 1;
		unsigned int flg_debug : 1;
	} ARGS_IN;

	typedef struct
	{
		int move_count;
		int quiescence_move_count;
		int deepest_search_depth;
		char *available_moves;
		char *best_move_path;
	} DEBUG_STATISTICS;
		
	typedef struct
	{
		int code;			// status code
							// 0 = OK
							// 1 = invalid move
		char status[20];	// status text
		char move_text[20];	// move text
		char move_ui[500];	// move (ui) text
		CHESS_STATE_STR chess_state_str;
	} ARGS_OUT;
	
	// computes the best move and returns the 'out' chess state
	ARGS_OUT compute(ARGS_IN in);		
	ARGS_IN set_arguments(int, char* []);
	void display_help();
	void display_output(int is_xml, ARGS_OUT out);
	ARGS_OUT new_game();

#endif
