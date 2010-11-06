#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "bitboard.h"
#include "chessstate.h"
#include "main.h"

#ifdef DEBUG

	void test_chess_state_basic()
	{
		/* -------------------------------------
		   CHESSSTATE FUNCTIONS
		   -------------------------------------*/

		CHESS_STATE_STR csa = {0};
		
		strcpy(csa.piece_placement, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

		// set_piece_placement
		CHESS_STATE cs = set_piece_placement(csa.piece_placement);
		
		assert(bit_count(cs.boards[BB_BLACK+BB_PAWN])==8);
		assert(bit_count(cs.boards[BB_WHITE+BB_PAWN])==8);
		assert(bit_count(cs.boards[BB_BLACK+BB_KING])==1);
		assert(bit_count(cs.boards[BB_WHITE+BB_KING])==1);

		assert(cs.board_value==0);	 // with 'start FEN', board value should be zero")
				
		// get_piece_placement
		CHESS_STATE_STR css = {0};
		strcpy(css.piece_placement, get_piece_placement(cs));
		assert(strcmp(csa.piece_placement,css.piece_placement)==0);

		strcpy(css.active_color,"w");
		strcpy(css.castling_availability,"KQkq");
		strcpy(css.enpassant_square,"-");
		strcpy(css.half_move_clock,"43");
		strcpy(css.full_move_number,"54");

		// get_chess_state
		cs = get_chess_state(css);

		assert(cs.half_move_clock==43);
		assert(cs.full_move_number==54);
		assert(cs.boards[BB_WHITE+BB_PAWN]==PAWN_START_WHITE);
		assert(cs.boards[BB_BLACK+BB_PAWN]==PAWN_START_BLACK);
		assert(cs.boards[BB_WHITE+BB_KING]==16);
		assert(cs.flg_is_white_move==1);
		assert(cs.flg_black_king_castle==1);
		assert(cs.flg_white_king_castle==1);
		assert(cs.flg_black_queen_castle==1);
		assert(cs.flg_white_queen_castle==1);
		assert(cs.flg_is_enpassant==0);

		csa = get_chess_state_str(cs);

		assert(strcmp(csa.active_color,"w")==0);
		assert(strlen(csa.castling_availability)==4);
		assert(strcmp(csa.half_move_clock,"43")==0);
		assert(strcmp(csa.full_move_number,"54")==0);
		
		// get_chess_state with some extra settings
		strcpy(csa.enpassant_square,"d1");
		strcpy(csa.active_color, "b");
		strcpy(csa.castling_availability, "-");
		strcpy(csa.castling_availability,"-");
		
		cs = get_chess_state(csa);
			
		assert(cs.flg_is_white_move!=1);
		assert(cs.flg_white_queen_castle==0);
		assert(cs.flg_black_king_castle==0);
		assert(cs.flg_is_enpassant==1);
		assert(cs.flg_enpassant_file==3);

		// chess state copying...
		CHESS_STATE cs1 = {0};
		cs1.flg_is_white_move=1;
		cs1.boards[4] = 12345678ULL;
		
		CHESS_STATE cs2 = cs1;
		assert(cs2.flg_is_white_move=1);
		assert(cs2.boards[4] = 12345678ULL);
	}

	void test_generate_moves_default()
	{
		// generate_moves
		CHESS_STATE_STR css = {0};
		strcpy(css.piece_placement, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
		CHESS_STATE cs = get_chess_state(css);
		generate_moves(&cs);

		// check addnextstate has set default values correctly
		CHESS_STATE ns = cs.child_head[0];
		assert(ns.full_move_number==cs.full_move_number+1);
		assert(ns.half_move_clock==cs.half_move_clock+1);
		assert(ns.flg_is_white_move!=cs.flg_is_white_move);
		assert(ns.parent==&cs);
	}

	/* -----------------------------------------------------
		This gets all the child moves
		and compares them to a string of moves
		
		ALL the moves must match
		----------------------------------------------------*/
	void check_contains_moves(CHESS_STATE *current_state, char *expected_moves)
	{
		CHESS_STATE *child_states;

		char buffer[100] = {0};
		
		strcpy(buffer,expected_moves);
		generate_moves(current_state);
		
		char *next_expected_move = strtok(buffer, ",");
		
		while(next_expected_move != NULL)
		{
			char *move_text;
			int found = 0;
			
			for (child_states = current_state->child_head;
				child_states != NULL;
				child_states = child_states->next)
			{
				move_text = get_move(child_states,0);
			
				if (strcmp(move_text, next_expected_move)==0)
				{
					found=1;
					break;
				}
			}
			if (found==0)
			{
				printf("%s not found!\n",next_expected_move);
				for (child_states = current_state->child_head;
						child_states!=NULL;
						child_states = child_states->next)
						{
							move_text=get_move(child_states,0);
							printf("%s ",move_text);
						}
				exit(1);
			}
			
			next_expected_move = strtok(NULL,",");
		}
		delete_nodes(current_state);
	}

	CHESS_STATE test_search(char *test,
						char *piece_placement,
						char *en_passant_square,
						char *castling_availability,
						int expected_move_count,
						char *active_color,
						char *expected_moves)
	{
		CHESS_STATE_STR css = {0};
		strcpy(css.piece_placement, piece_placement);
		strcpy(css.active_color,active_color);
		strcpy(css.enpassant_square,en_passant_square);
		strcpy(css.castling_availability,castling_availability);
		CHESS_STATE cs = get_chess_state(css);
		
		int id_count;
		ITERATIVE_DEEPENING_INFO id_info[MAX_ITERATIVE_DEEPENING_DEPTH] = {0};

		iterative_deepening(&cs, 0, &id_count, id_info, 0);

		if (id_info[1].legal_move_count!=expected_move_count)
			printf("Test: %s\nExpected moves: %d, Actual moves: %d",test, expected_move_count,id_info[1].legal_move_count);
			
		assert(id_info[1].legal_move_count==expected_move_count);
		
		if (strlen(expected_moves)>0)
			check_contains_moves(&cs, expected_moves);
	}

	void test_search_moves()
	{

		test_search("white en passant","8/1ppppppp/8/pP6/8/8/8/8","a6","-",2,"w","bxa6e.p.,b6");
		test_search("en pass set, but not available","8/8/8/3p4/P7/8/8/8/","d6","-",1,"w","a5");

		// pawn moves
		test_search("Should find 8 pawn moves","8/8/8/8/8/PPPPPPPP/8/8","-","-",8,"w","");
		test_search("Should find 8 pawn moves","8/8/pppppppp/8/8/8/8/8","-","-",8,"b","");
		test_search("Pawn blocked","8/8/p6p/P6P/8/8/8/8","-","-",0,"b","");

		// pawn capture moves
		test_search("Pawn capture (b)","8/8/p7/1P5P/8/8/8/8","-","-",2,"b","a5,xb5");
		test_search("Pawn capture (w)","8/8/1p5p/P7/8/8/8/8","-","-",2,"w","a6,xb6");

		//  pawn double moves
		test_search("Pawn Double","8/8/8/8/8/8/PPPPPPPP/8","-","-",16,"w","");
		test_search("Pawn Double #2","8/8/8/8/p7/8/PPPPPPPP/8","-","-",15,"w","");
		test_search("Pawn Double #3","8/pppppppp/8/8/8/8/8/8","-","-",16,"b","");
		test_search("Pawn Double #4","8/pppppppp/8/P7/8/8/8/8","-","-",15,"b","");

		// king moves
		test_search("King","8/8/8/8/8/8/3K4/8","-","-",8,"w","Kc1,Kd1,Ke1,Kc2,Ke2,Kc3,Kd3,Ke3");
		test_search("King","4k3/8/8/8/8/8/8/8","-","-",5,"b","kd8,kf8,kd7,ke7,kf7");

		// note one below has only 6 moves - 2 leave king exposed to check.
		// note for this to work, black must have 1 other piece still on board
		// or the alpha-beta search cuts off and stops the flg_is_illegal being set
		// correctly.
		test_search("King + capture","p7/8/8/8/8/3p4/3K4/8","-","-",6,"w","Kc1,Kd1,Ke1,Kc3,Kxd3,Ke3");
		
		// knight moves
		test_search("Knight","8/8/8/8/8/8/3N4/8","-","-",6,"w","Nb1,Nb3,Nc4,Ne4,Nf3,Nf1");
		test_search("Knight + capture","3k4/8/8/8/8/8/3N4/5p2","-","-",6,"w","Nb1,Nb3,Nc4,Ne4,Nf3,Nxf1");
		
		// rook moves
		test_search("Rook","4k3/8/8/8/8/8/R7/8","-","-",14,"w","Rb2,Rc2,Rd2,Re2,Rf2,Rg2,Rh2,Ra1,Ra3,Ra4,Ra5,Ra6,Ra7,Ra8");
		test_search("Black rook 1","7r/8/7B/8/8/8/PPPPPPPP/RNBQKBNR","-","-",9,"b","ra8,rb8,rc8,rd8,re8,rf8,rg8,rh7,rxh6");
		test_search("black rook with capture","8/8/8/8/2p5/1prp4/1P1P4/2R3K1","-","-",2,"b","rc2,rxc1");		

		// bishop moves
		test_search("Bishop","8/8/3b4/8/8/8/8/4K3","-","-",11,"b","be7,bf8,bc5,bb4,ba3,be5,bf4,bg3,bh2,bc7,bb8");

		// test check_validation
		test_search("Check validation","8/8/8/8/8/1q6/1B6/1K6","-","-",2,"w","Ka1,Kc1");
		
		// pawn promotion
		test_search("pawn promotion","8/P7/8/8/3k4/8/8/8","-","-",1,"w","a8Q");
		test_search("pawn promotion with capture","1b6/P7/8/8/8/4k3/8/8","-","-",2,"w","a8Q,xb8Q");

		// disallow moves ending in check
		test_search("test in check","p7/8/8/8/8/4q3/4K3/8","-","-",3,"w","Kd1,Kf1,Kxe3");
	}

	void test_chessstate()
	{
		test_chess_state_basic();
		test_generate_moves_default();
		test_search_moves();
		printf("Tests completed OK!\n");
	}

#endif
