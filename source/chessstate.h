/* -------------------------------
   _CHESS_STATE.h
   
   CHESS_STATE library
      
   (C) 2008 David Barone
   
   -------------------------------*/
#ifndef CHESS_STATE_H

	#define CHESS_STATE_H

	#include <stdio.h>
	#include <string.h>
	#include "bitboard.h"

	#define PAWN_VALUE 100

	// bitboard enums
	typedef enum {
		BB_BLACK = 0,
		BB_WHITE = 6
	} COLOR_ENUM;

	typedef enum {
		BB_PAWN = 0,
		BB_ROOK,
		BB_KNIGHT,
		BB_BISHOP,
		BB_QUEEN,
		BB_KING,
	} PIECE_ENUM;

	#define BB_COUNT 12	// number of bitboards in state
    #define PIECE_TYPES 6 // number of distinct pieces of each color
    
	/* represents a chess state */
	struct _CHESS_STATE
	{
		bitboard boards[12];
		int half_move_clock;
		int full_move_number;

		struct _CHESS_STATE *parent;		// parent state
		struct _CHESS_STATE *child_head;	// head of child state linked  list
		struct _CHESS_STATE *next;			// next sibling state

		int board_value;					// white pieces = +, black pieces = -
		
		// bit flags
		unsigned int flg_white_king_castle : 1;
		unsigned int flg_white_queen_castle : 1;
		unsigned int flg_black_king_castle : 1;
		unsigned int flg_black_queen_castle : 1;
		unsigned int flg_is_white_move : 1;	
		unsigned int flg_is_enpassant : 1;
		unsigned int flg_enpassant_file : 3;
		unsigned int flg_is_illegal : 1;	// set on a state if a child state can capture the king
		unsigned int flg_is_noisy : 1;		// noisy move defined for quiescence search
	};
	
typedef struct _CHESS_STATE CHESS_STATE;
	
	struct _CHESS_STATE_STR
	{
		char piece_placement[100];
		char castling_availability[5];
		char active_color[2];
		char enpassant_square[3];
		char half_move_clock[4];
		char full_move_number[4];
	};
	typedef struct _CHESS_STATE_STR CHESS_STATE_STR;	
	
	struct _ITERATIVE_DEEPENING_INFO
	{
		int depth;							// depth of iteration
		char *best_move;						// the best move path at this depth
		int value;							// estimated MINIMAX value of this node
		int moves_evaluated;				// count of moves evaluated using alpha-beta pruning
		int legal_move_count;				// for level 1 only
		char *legal_moves[100];					// for level 1 only
	};

	typedef struct _ITERATIVE_DEEPENING_INFO ITERATIVE_DEEPENING_INFO;
	
	#define MAX_ITERATIVE_DEEPENING_DEPTH 20
	
	// converts set of strings to a CHESS_STATE
	CHESS_STATE get_chess_state(CHESS_STATE_STR chess_state_str);
						
	// converts CHESS_STATE to a set of strings
	CHESS_STATE_STR get_chess_state_str(CHESS_STATE chess_state);

	// set the piece placement
	CHESS_STATE set_piece_placement(char *piece_placement_string);

	// gets string from CHESS_STATE
	char *get_piece_placement(CHESS_STATE chess_state);

	CHESS_STATE *insert_child(
					CHESS_STATE *current_state
					, PIECE_ENUM piece_index
					, bitboard move_mask
					, bitboard delete_mask);

	//CHESS_STATE *insert_child(CHESS_STATE *current_state); 	// creates a new child chess state
	void generate_moves(CHESS_STATE *current_state);
	char *get_move(CHESS_STATE *current_state, int ui_flag);// gets the move string for a state
	int child_count(CHESS_STATE *current_state);			// counts child states
	CHESS_STATE *check_move(CHESS_STATE *parent_state, char *move);

	void iterative_deepening(CHESS_STATE *cs,
				 int time_limit,
				 int *id_count,
				 ITERATIVE_DEEPENING_INFO *id_info,
				 int debug_mode);


	// use symmetry: -(beta) becomes subsequently pruned alpha.
	#define ALPHA_BETA_MAX(alpha, beta) ((alpha>beta) ? (alpha) : (beta))
	
#endif
