/*   ____   _   ____   _       _    _  ____   ______  __      _____   _____  _
    / ___/ /_/ / ___/ / /     / / / / / ___/ / __  / / /     / __  / / ___/ / /
   / /__  _   / /__  / /__   / /_/ / / /__  / /_/ / / /     / /_/ / / /__  / /__ 
  / ___/ / / /__  / / __  / /___  / /__  / / ____/ / /     / __  / /__  / / __  /
 / /    / / ___/ / / / / / ____/ / ___/ / / /     / /___  / /   / ___/ / / / / /
/_/    /_/ /____/ /_/ /_/ /_____/ /____/ /_/     /_____/ /_/  _/ /____/ /_/ /_/
		 ______   __  __  ______  _____   ______
        /  ___/  / / / / / ____/ / ___/  / ____/
       /  /     / /_/ / / /___  / /___  / /___
      /  /     / __  / / ____/ /___  / /___  / 
     /  /___  / / / / / /____ ____/ / ____/ /
    /______/ /_/ /_/ /______//_____/ /_____/
                                                       .::.
                                            _()_       _::_
                                  _O      _/____\_   _/____\_
           _  _  _     ^^__      / //\    \      /   \      /
          | || || |   /  - \_   {     }    \____/     \____/
          |_______| <|    __<    \___/     (____)     (____)
    _     \__ ___ / <|    \      (___)      |  |       |  |
   (_)     |___|_|  <|     \      |_|       |__|       |__|
  (___)    |_|___|  <|______\    /   \     /    \     /    \
  _|_|_    |___|_|   _|____|_   (_____)   (______)   (______)
 (_____)  (_______) (________) (_______) (________) (________)
 /_____\  /_______\ /________\ /_______\ /________\ /________\ 
    
   A CHESS SERVER
  
  (c) fishysplash.com (2008)

*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "chessstate.h"
#include "main.h"

/* -----------------------------------------------------------------------
	int get_square_number(char *)
	
	purpose: gets the square number for a square represented in algebraic
			 chess notation
	
	returns 0~63 for valid squares
	returns -1 if not a valid algebraic notation
   ----------------------------------------------------------------------- */
int get_square_number(char *algebraic_notation)
{
	int col, row;
	if (algebraic_notation[0]==0 || algebraic_notation[1]==0)
		return -1;
	
	if (strchr("12345678",algebraic_notation[1])!=NULL)
		row = algebraic_notation[1]-'1';
	else
		return -1;
		
	if (strchr("abcdefgh",algebraic_notation[0])!=NULL)
		col=algebraic_notation[0]-'a';
	else
		return -1;
	
	return (row*8) + col;
}

/* ----------------------------------------------------------------------
	char *get_square_text(int square_number)
	
	purpose: gets the square number for a square represented in algebraic
			 chess notation
	
	returns 0~63 for valid squares
	returns -1 if not a valid algebraic notation
   ---------------------------------------------------------------------- */
char *get_square_text(int square_number)
{
	char* ret = malloc(3 * sizeof (char));
	ret[0] = '\0';
	
	if (square_number > 63 || square_number < 0)
		return ret;

	ret[0] = 'a' + (square_number % 8);
	ret[1] = '1' + (square_number / 8);
	ret[2] = '\0';
	return ret;
}

/* ---------------------------------------------------------------------------------------
	char *safe_str_copy(char *target, int targetsize, char *source)
	
	purpose: copies a string to a target buffer, ensuring no buffer overruns
	         can occur.
   --------------------------------------------------------------------------------------- */
char *safe_str_copy(char *target, int targetsize, char *source)
{	
	(void)strncpy(target, source, targetsize - 1);
	target[targetsize - 1] = '\0';
	return target;
}

/* ---------------------------------------------------------------------------------------
	ARGS_IN set_arguments(int argc, char *argv[])
	
	purpose: takes the command line arguments, and copies them to global variables
	         for use by the program.
   --------------------------------------------------------------------------------------- */
ARGS_IN set_arguments(int argc, char *argv[])
{
	char currvar[255];
	
	ARGS_IN in={0};
	
	// defaults
	strcpy(in.chess_state_str.active_color,"w");
	strcpy(in.chess_state_str.full_move_number,"1");
	strcpy(in.chess_state_str.half_move_clock,"0");
	strcpy(in.chess_state_str.castling_availability,"KQkq");
	in.time_limit = 1;
	in.move[0] = 0;
	
	// argc[0] = exe path
	if (argc==1)
	{
		// no arguments other than exe path specified
		fprintf(stdout,"No arguments supplied. -h for help.\n");
		exit(0);
	}
	
	// process arguments
	int t;
	for (t=1; t<argc; t++)
	{
		safe_str_copy(currvar, sizeof(currvar), argv[t]);

		char prefix;
		if (strlen(currvar)>=2 && currvar[0] == '-')
		{
			prefix = currvar[1];
			switch(prefix)
			{
				case 'h':
				{
					display_help();
					exit(0);
				}
				case 'p':	// piece placement
					safe_str_copy(in.chess_state_str.piece_placement,
						sizeof(in.chess_state_str.piece_placement),
						&currvar[2]);
					break;
				case 'c':	// castling availability
					safe_str_copy(in.chess_state_str.castling_availability,
						sizeof(in.chess_state_str.castling_availability),
						&currvar[2]);
					break;
				case 'e':	// enpassant
					safe_str_copy(in.chess_state_str.enpassant_square,
						sizeof(in.chess_state_str.enpassant_square),
						&currvar[2]);
					break;
				case 'a':	// active colour
					safe_str_copy(in.chess_state_str.active_color,
						sizeof(in.chess_state_str.active_color),
						&currvar[2]);
					break;
				case 'l':	// half move clock
					safe_str_copy(in.chess_state_str.half_move_clock,
						sizeof(in.chess_state_str.half_move_clock),
						&currvar[2]);
					break;
				case 'f':	// full move number
					safe_str_copy(in.chess_state_str.full_move_number,
						sizeof(in.chess_state_str.full_move_number),
						&currvar[2]);
					break;
				case 'm':
					safe_str_copy(in.move,
						sizeof(in.move),
						&currvar[2]);
					break;
				case 't':
					in.time_limit = atoi(&currvar[2]);
					break;
				case 'x':
					in.flg_xml = 1;
					break;
				case 'd' :
					in.flg_debug = 1;
					break;
				case 'n':
					in.flg_new = 1;
					break;
			}
		}
	}
	return in;
}

/* ---------------------------------------------------------------------------------------
	int main(int argc, char *argv[])
	
	purpose: entry point for program.
   --------------------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
	// run tests
	#ifdef DEBUG
		test_main();
		test_bitboard();
		test_chessstate();
	#endif
	
	// set arguments
	ARGS_OUT out;
	ARGS_IN in = set_arguments(argc, argv);		

	// calculate
	if (in.flg_new!=0)
		out = new_game();
	else
	{
		out = compute(in);
	}

	// print output
	display_output(in.flg_xml,out);
	
	return 0;
}

/* ---------------------------------------------------------------------------------------
	ARGS_OUT compute(ARGS_IN in)
	
	purpose: main controlling function.
   --------------------------------------------------------------------------------------- */
ARGS_OUT compute(ARGS_IN in)
{
	CHESS_STATE cs = get_chess_state(in.chess_state_str);

	// find best moves using iterative deepening algorithm
	ITERATIVE_DEEPENING_INFO id_info[MAX_ITERATIVE_DEEPENING_DEPTH] = {0};
	
	CHESS_STATE *return_state = NULL;
	int id_count;

	if (strlen(in.move)>0)
	{
		// check move is legal - only need to go 1 level deep
		iterative_deepening(&cs, 0, &id_count, id_info, in.flg_debug);
		// check if move is in legal moves
		int i;
		generate_moves(&cs);
		for (i=0; i<id_info[1].legal_move_count; i++)
		{
			// the move passed in may be just the first/main UI move.
			// therefore, rather than check exact match,
			// just check the move passed in matches the start of the move 
			
			CHESS_STATE *legal_move = check_move(&cs, id_info[1].legal_moves[i]);
			
			// check move to algebraic chess notation
			if (strcmp(get_move(legal_move,0), in.move)==0)
			{
				return_state = legal_move;
				break;
			}

			// check move matches move UI text (up to end of in.move)
			char move_ui_buffer[20] = {0};
			strcpy(move_ui_buffer, in.move);
			strcat(move_ui_buffer, " ");	// add delimiter to ensure match is valid
			int move_ui_length;
			move_ui_length = strlen(move_ui_buffer);
			
			char actual_move_ui[20] = {0};
			strncpy(actual_move_ui,get_move(legal_move,1), move_ui_length-1);
			strcat(actual_move_ui," ");

			if (strcmp(actual_move_ui,move_ui_buffer)==0)
			{
				return_state = legal_move;
				break;
			}
		}
	}
	else
	{
		// get best move
		iterative_deepening(&cs, in.time_limit, &id_count, id_info, in.flg_debug);
		generate_moves(&cs);
		return_state = check_move(&cs, id_info[id_count].best_move);
	}
	 
	ARGS_OUT out = {0};

	if 	(return_state == NULL)
	{
		strcpy(out.move_text, "-");
		strcpy(out.move_ui, "-");
		strcpy(out.status, "Invalid move");
		out.code = 1;
		out.chess_state_str = get_chess_state_str(cs);	
	}
	else
	{
		strcpy(out.move_text, get_move(return_state,0));
		strcpy(out.move_ui, get_move(return_state,1));
		strcpy(out.status,"OK");
		out.code = 0;
		out.chess_state_str = get_chess_state_str(*return_state);		
	}	
	
	return out;
}

/* ---------------------------------------------------------------------------------------
	ARGS_OUT new_game()
	
	purpose: sets globals for new game.
   --------------------------------------------------------------------------------------- */
ARGS_OUT new_game()
{
	ARGS_OUT out;
	
	out.code=0;
	safe_str_copy(out.status, sizeof(out.status), "OK");
	safe_str_copy(out.move_text, sizeof(out.move_text), "-");
	
	safe_str_copy(out.move_ui, sizeof(out.move_ui), 
	"CLEAR:PIECES r:a8 n:b8 b:c8 q:d8 k:e8 b:f8 n:g8 r:h8 p:a7 p:b7 p:c7 p:d7 p:e7 p:f7 p:g7 p:h7 R:a1 N:b1 B:c1 Q:d1 K:e1 B:f1 N:g1 R:h1 P:a2 P:b2 P:c2 P:d2 P:e2 P:f2 P:g2 P:h2");
	
	safe_str_copy(out.chess_state_str.piece_placement,
		sizeof(out.chess_state_str.piece_placement),
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
	
	safe_str_copy(out.chess_state_str.castling_availability,
		sizeof(out.chess_state_str.castling_availability), "KQkq");

	safe_str_copy(out.chess_state_str.enpassant_square,
		sizeof(out.chess_state_str.enpassant_square),
		"-");
	
	safe_str_copy(out.chess_state_str.active_color,
		sizeof(out.chess_state_str.active_color),
		"w");
			
	safe_str_copy(out.chess_state_str.half_move_clock,
		sizeof(out.chess_state_str.half_move_clock),
		"0");
	safe_str_copy(out.chess_state_str.full_move_number,
		sizeof(out.chess_state_str.full_move_number),
		"1");

	return out;
}

/* ---------------------------------------------------------------------------------------
	void display_output(int is_xml, ARGS_OUT out)
	
	purpose: displays the final output of the program.
   --------------------------------------------------------------------------------------- */
void display_output(int is_xml, ARGS_OUT out)
{
	if (is_xml!=0)
	{
		fprintf(stdout,"<chess>\n");
		fprintf(stdout,"\t<code>%d</code>\n", out.code);
		fprintf(stdout,"\t<status>%s</status>\n", out.status);
		fprintf(stdout,"\t<move>%s</move>\n", out.move_text);
		fprintf(stdout,"\t<move_ui>%s</move_ui>\n", out.move_ui);
		fprintf(stdout,"\t<piece_placement>%s</piece_placement>\n",
			out.chess_state_str.piece_placement);
		
		fprintf(stdout,"\t<castle_availability>%s</castle_availability>\n",
			out.chess_state_str.castling_availability);
		
		fprintf(stdout,"\t<enpassant>%s</enpassant>\n",
			out.chess_state_str.enpassant_square);

		fprintf(stdout,"\t<active_color>%s</active_color>\n",
			out.chess_state_str.active_color);

		fprintf(stdout,"\t<half_move_clock>%s</half_move_clock>\n",
			out.chess_state_str.half_move_clock);
			
		fprintf(stdout,"\t<full_move_number>%s</full_move_number>\n",
			out.chess_state_str.full_move_number);
		
		fprintf(stdout,"</chess>\n");
	}
	else
	{
		fprintf(stdout,"Code:\t%d\n", out.code);
		fprintf(stdout,"Status:\t%s\n", out.status);
		fprintf(stdout,"Move:\t%s\n", out.move_text);
		fprintf(stdout,"Move UI:\t%s\n", out.move_ui);
		fprintf(stdout,"Placement:\t%s\n", out.chess_state_str.piece_placement);
		fprintf(stdout,"Castle:\t%s\n", out.chess_state_str.castling_availability);
		fprintf(stdout,"EnPassant\t%s\n", out.chess_state_str.enpassant_square);
		fprintf(stdout,"Color:\t%s\n", out.chess_state_str.active_color);
		fprintf(stdout,"HalfMove:\t%s\n", out.chess_state_str.half_move_clock);
		fprintf(stdout,"FullMove:\t%s\n", out.chess_state_str.full_move_number);
	}
}

/* ---------------------------------------------------------------------------------------
	void display_help()
	
	purpose: displays the help text to standard output.
   --------------------------------------------------------------------------------------- */
void display_help()
{

	fprintf(stdout, "A simple chess engine.\n");
	fprintf(stdout, "Copyright David Barone 2008\n\n");
	fprintf(stdout, "Usage: chess [OPTIONS]\n\n");
	fprintf(stdout, "Arguments as follows:\n");
	fprintf(stdout, "  -h              displays this help\n");
	fprintf(stdout, "  -p              piece placement - (this is mandatory)\n");
	fprintf(stdout, "  -n              new game\n");
	fprintf(stdout, "  -c              castling availability string (default='-')\n");
	fprintf(stdout, "  -e              enpassant square (default='-')\n");
	fprintf(stdout, "  -a              active color (next move) (default='w')\n");
	fprintf(stdout, "  -l              half move clock number (default=0)\n");
	fprintf(stdout, "  -f              full move number (default=0)\n");
	fprintf(stdout, "  -t              time limit (seconds) for move (default=1)\n");
	fprintf(stdout, "  -m              move (in either move ui text or algebraic chess notation)\n");
	fprintf(stdout, "  -x              xml output\n");
	fprintf(stdout, "  -d              debug mode (only for non-xml output)\n\n");
	fprintf(stdout, "The value returned is generally the next 'best' move calculated. \n");
	fprintf(stdout, "Exit status is 0 if OK, 1 if minor problems, 2 if serious trouble.\n\n");
	fprintf(stdout, "Example 1 - to get computer to give white's opening move:\n");
    fprintf(stdout, "./chess -prnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR\n\n");
	fprintf(stdout, "Example 2 - to verify white's opening move:\n");
    fprintf(stdout, "./chess -prnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR -md4\n\n");
	fprintf(stdout, "Report bugs to <david@fishysplash.com>.\n");
}
