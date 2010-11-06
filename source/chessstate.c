#include <stdlib.h>
#include <time.h>
#include "chessstate.h"
#include "main.h"

char piece_chr[] = {'p','r','n','b','q','k','P','R','N','B','Q','K'};
int piece_values[] = {-PAWN_VALUE,-500,-300,-325,-900,-20000,PAWN_VALUE,500,300,325,900,20000};

// function pointers typedef to knight and king move masks
typedef bitboard(*fp_cached_move_mask)(bitboard);

//function pointers typedef to rook, bishop and queen masks
typedef bitboard(*fp_cached_sliding_move_mask)(bitboard, bitboard);

// prototypes
inline void pawn_moves(CHESS_STATE *current_state, bitboard empty_squares);
inline void pawn_double_moves(CHESS_STATE *current_state, bitboard empty_squares);
inline void pawn_capture_moves(CHESS_STATE *current_state, 
							bitboard empty_squares,
							bitboard opponents_squares);
inline void en_passant_moves(CHESS_STATE *current_state);
inline void simple_move_mask_moves(CHESS_STATE *current_state, 
									PIECE_ENUM piece,
									fp_cached_move_mask move_mask_function,
									bitboard empty_squares,
									bitboard opponents_squares);
inline int remove_opponents_piece(CHESS_STATE *current_state, bitboard piece);
inline void sliding_moves(CHESS_STATE *current_state,
				 bitboard empty_squares,
				 bitboard opponents_squares,
				 PIECE_ENUM piece,
				 fp_cached_sliding_move_mask sliding_move_mask_function);

/* ----------------------------------------------------------
   CHESS_STATE get_chess_state(CHESS_STATE_STR chess_state_str)
   
   purpose: gets a chess state from individual chess state
            component strings.
   ---------------------------------------------------------- */
CHESS_STATE get_chess_state(CHESS_STATE_STR chess_state_str)
{
		CHESS_STATE cs = {0};
		
		// parse the piece placement string
		cs = set_piece_placement(chess_state_str.piece_placement);

		// next move
		if (strcmp(chess_state_str.active_color,"w")==0 || strcmp(chess_state_str.active_color,"W")==0)
			cs.flg_is_white_move=1;
		else
			cs.flg_is_white_move=0;

		// castling
		cs.flg_black_queen_castle = (strstr(chess_state_str.castling_availability,"q")==NULL) ? 0 : 1;
		cs.flg_white_queen_castle = (strstr(chess_state_str.castling_availability,"Q")==NULL) ? 0 : 1;
		cs.flg_black_king_castle = (strstr(chess_state_str.castling_availability,"k")==NULL) ? 0 : 1;
		cs.flg_white_king_castle = (strstr(chess_state_str.castling_availability,"K")==NULL) ? 0 : 1;

		// enpassant
		if (get_square_number(chess_state_str.enpassant_square)<0)
			cs.flg_is_enpassant=0;
		else
		{
			cs.flg_enpassant_file=COL(get_square_number(chess_state_str.enpassant_square));
			if (cs.flg_enpassant_file>=0 && cs.flg_enpassant_file<8)
				cs.flg_is_enpassant=1;
		}

		// halfmove clock
		cs.half_move_clock = atoi(chess_state_str.half_move_clock);
		
		// fullmove number
		cs.full_move_number = atoi(chess_state_str.full_move_number);
		
		cs.parent=NULL;
		cs.child_head=NULL;
		cs.next=NULL;
		return cs;
}

/* ----------------------------------------------------------
   CHESS_STATE_STR get_chess_state_str(CHESS_STATE chess_state)
   
   purpose: takes an existing CHESS_STATE and extracts the
   			string components making up the chess state.
   			Used to return CHESS_STATE information back
   			to a client.
   ---------------------------------------------------------- */
CHESS_STATE_STR get_chess_state_str(CHESS_STATE chess_state)
{
	CHESS_STATE_STR csa = {0};
	
	char squares[63];
	char *castle_black_king = "k";
	char *castle_black_queen = "q";
	char *castle_white_king = "K";
	char *castle_white_queen = "Q";

	// piece placement
	strcpy(csa.piece_placement, get_piece_placement(chess_state));
	
	// castle availability
	if (chess_state.flg_black_king_castle)
		strcat(csa.castling_availability, castle_black_king);

	if (chess_state.flg_black_queen_castle)
		strcat(csa.castling_availability, castle_black_queen);

	if (chess_state.flg_white_king_castle)
		strcat(csa.castling_availability, castle_white_king);

	if (chess_state.flg_white_queen_castle)
		strcat(csa.castling_availability, castle_white_queen);

	// en passant (to do)			
	if (chess_state.flg_is_enpassant==0)
		strcpy(csa.enpassant_square,"-");
	else
	{
		int en_passant_row = (csa.active_color=="w" ? 5 : 2);
		strcpy(csa.enpassant_square,
			get_square_text(en_passant_row*8+chess_state.flg_enpassant_file));
	}

	// half move clock
	sprintf(csa.half_move_clock, "%d", chess_state.half_move_clock);
	
	// full move number
	sprintf(csa.full_move_number, "%d", chess_state.full_move_number);	

	// active color
	if (chess_state.flg_is_white_move)
		strcpy(csa.active_color,"w");
	else
		strcpy(csa.active_color,"b");
		
	return csa;	
}

/* ----------------------------------------------------------
   char *get_piece_placement(CHESS_STATE chess_state)
   
   purpose: gets the piece placement string from a CHESS_STATE.
   ---------------------------------------------------------- */
char *get_piece_placement(CHESS_STATE chess_state)
{
	char squares[64] = {0};
	int index=0;
	int blanks=0;
	char last_square = '\0';
	char* piece_placement_string = malloc(100 * sizeof (char));

	int i;
	
	for (i=0; i<BB_COUNT; i++)
	{
		bitboard next_board = chess_state.boards[i];
		bitboard next_bit;
	
		while((next_bit=get_lowest_bit(&next_board))!=0)
			squares[floor_log2(next_bit)]=piece_chr[i];
	}	
		
	int row_start = floor_log2(A8);	// starting point 
	int col = 0;

	for (row_start = 56; row_start >= 0; row_start-=8)
	{
		for (col = 0; col < 8; col++)
		{
			if ((squares[row_start+col] != last_square) && (blanks > 0))
			{
				piece_placement_string[index++] = '0' + blanks;	// number of blank squares
				blanks = 0;
			}

			switch(squares[row_start+col])
			{
				case '\0':	// blank square
					++blanks;
					break;
				default:	// square has piece on.
					piece_placement_string[index++] = squares[row_start+col];
					blanks = 0;	// reset blanks
			}
			last_square=squares[row_start+col];
		}
		// end of row
		if (blanks > 0)
		{
			piece_placement_string[index++] = '0' + blanks;	// number of blank squares
			blanks = 0;
		}			
		
		if (row_start != 0)
			piece_placement_string[index++] = '/';	// new line
			
		blanks = 0;
		last_square = '\0';
	}	

	piece_placement_string[index] = '\0';	// null terminator
	
	return piece_placement_string;
}

/* ----------------------------------------------------------
   CHESS_STATE set_piece_placement(char *piece_placement_string)
   
   purpose: sets the chess state piece_placement (bitboards) from
            the piece placement string.
   ---------------------------------------------------------- */
CHESS_STATE set_piece_placement(char *piece_placement_string)
{
	char *delim_slash = "/";	// delimiter
	char *next_row = NULL;		// holds value of each row
	int row_start;
	row_start = floor_log2(A8);

	char* pps = malloc(100 * sizeof (char));

	// copy the string passed in, to an internal field
	// just in case the function argument is a literal,
	// as strtok can't work on literals.
	strcpy(pps, piece_placement_string);
	
	CHESS_STATE cs = {0};
	
	next_row = strtok(pps, delim_slash);
	while(next_row != NULL)
	{
		int col=0;
		int row=0;
		char next_char;
		
		while((next_char = next_row[row++]) != '\0')
		{
			switch(next_char)
			{
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8': case '9':
					col+=(next_char-'0');

				case 'p': case 'r': case 'n': case 'b': case 'q': case 'k':
				case 'P': case 'R': case 'N': case 'B': case 'Q': case 'K':
					{
						int i;
						for (i=0; i<BB_COUNT; i++)
						{
							if (piece_chr[i] == next_char)
							{
								cs.boards[i] |= ((bitboard)1<<row_start+col); ++col;
								cs.board_value += piece_values[i];
								break;
							}
						}
					}
			}
		}

    	if (col!=8) {exit(1);}	// each row should add up to 8
    	row_start=row_start-8 ;	// drop down one line
    	row=0;
 		next_row = strtok(NULL, delim_slash);
	}

	if (row_start!= -8)
	{ 
		printf("Error in set_piece_placement");
		exit(1);
	}
	
	free(pps);
	return cs;
}

/* ---------------------------------------------------------------------------------------
	CHESS_STATE *insert_child(CHESS_STATE *current_state)
	
	purpose: creates a child state and returns pointer to it.
   --------------------------------------------------------------------------------------- */
CHESS_STATE *insert_child(CHESS_STATE *current_state, PIECE_ENUM piece_index, bitboard move_mask, bitboard delete_mask)
{
		CHESS_STATE *child = (CHESS_STATE*)malloc(sizeof(CHESS_STATE));

		// copy existing state into new state
		child = (CHESS_STATE*)memcpy(child,current_state,sizeof(CHESS_STATE));
	
		child->flg_is_white_move = ~child->flg_is_white_move;
		child->parent = current_state;
		child->half_move_clock++;
		child->full_move_number++;
		child->child_head=NULL;
		child->board_value = current_state->board_value; // default value of board to value of parent board.	
		child->flg_is_enpassant=0;
		child->flg_is_noisy=0;
		
		COLOR_ENUM color = current_state->flg_is_white_move ? BB_WHITE : BB_BLACK;
	
		// do actual move
		child->boards[color + piece_index] ^= move_mask; 

		// delete opponents piece.
		int capture_value = 0;
		if (delete_mask != EMPTY_BOARD)
		{
			capture_value = remove_opponents_piece(child,delete_mask);
			child->board_value -= capture_value;
			// noisy move defined as major piece capture.
			child->flg_is_noisy = abs(capture_value)>PAWN_VALUE ? 1 : 0; 
		}

		// special case - check for pawn promotion
		// for moment - automatically make queen.
		if (piece_index == BB_PAWN)
		{
			bitboard pawn_promotion = child->boards[color + piece_index] &
					(color==BB_WHITE ? TOP_RANK : BOTTOM_RANK);

			if (pawn_promotion != EMPTY_BOARD)
			{
				child->boards[color + piece_index] &= ~pawn_promotion;
				child->boards[color + BB_QUEEN] |= pawn_promotion;
				child->board_value+=piece_values[color+BB_QUEEN]-piece_values[color+BB_PAWN];
			}				
		}

		// insert child at head of linked list)
		child->next = current_state->child_head;
		current_state->child_head=child;

		return child;
}

/* ---------------------------------------------------------------------------------------
	void generate_moves(CHESS_STATE *current_state)
	
	purpose: the search tree function - gets all child states for a given state.
	NOTE: this function simply gets all moves for an immediate chess state using
	bitboard maths etc. It _may_ include illegal moves (eg if king is left exposed.
	Such illegal moves are filtered out using iterative deepening function (see later
	in source), using flg_is_illegal flag on node.
   --------------------------------------------------------------------------------------- */
void generate_moves(CHESS_STATE *current_state)
{
	bitboard pieces=0;		// bitboard of pieces under investigation
	bitboard piece;			// bitboard of single piece

	COLOR_ENUM color = (current_state->flg_is_white_move) ? BB_WHITE : BB_BLACK;
	COLOR_ENUM opponent_color = (current_state->flg_is_white_move) ? BB_BLACK : BB_WHITE;

	// calculated bitboards
	int i;

	// empty and opponents squares cached for performance
	bitboard empty_squares = EMPTY_BOARD;
	bitboard opponents_squares = EMPTY_BOARD;
		
	for (i=0; i< BB_COUNT; i++)
	{
		empty_squares |= current_state->boards[i];
	}
	empty_squares = ~empty_squares;
	
	for (i=0; i<6; i++)
	{
		opponents_squares |= current_state->boards[opponent_color+i];
	}
		
	// pawns
	pawn_moves(current_state,empty_squares);
	pawn_double_moves(current_state,empty_squares);
	en_passant_moves(current_state);
	pawn_capture_moves(current_state,empty_squares, opponents_squares);

	//knight
	fp_cached_move_mask knight_func = &cached_knight_move_mask;
	simple_move_mask_moves(current_state, BB_KNIGHT, knight_func, empty_squares, opponents_squares);

	//king
	fp_cached_move_mask king_func = &cached_king_move_mask;
	simple_move_mask_moves(current_state, BB_KING, king_func, empty_squares, opponents_squares);

	// rook
	fp_cached_sliding_move_mask rook_func = &cached_rook_move_mask;
	sliding_moves(current_state,empty_squares,opponents_squares,BB_ROOK,rook_func);
		
	// bishop
	fp_cached_sliding_move_mask bishop_func = &cached_bishop_move_mask;
	sliding_moves(current_state,empty_squares,opponents_squares,BB_BISHOP,bishop_func);
	
	// queen
	sliding_moves(current_state,empty_squares,opponents_squares,BB_QUEEN,rook_func);
	sliding_moves(current_state,empty_squares,opponents_squares,BB_QUEEN,bishop_func);
	
}

/* ---------------------------------------------------------------------------------------
	void pawn_moves(CHESS_STATE *current_state)
	
	purpose: adds any pawn moves
   --------------------------------------------------------------------------------------- */
inline void pawn_moves(CHESS_STATE *current_state, bitboard empty_squares)
{
	/* Pawn Single Moves */
	COLOR_ENUM color = (current_state->flg_is_white_move ? BB_WHITE : BB_BLACK);
	int pawn_direction = (color==BB_WHITE) ? 1 : -1;
	
	bitboard pawn_moves = MOVE_UP_COUNT(current_state->boards[color + BB_PAWN],pawn_direction) & empty_squares;
	bitboard pawn;
	while((pawn = get_lowest_bit(&pawn_moves))!=0)
	{
		CHESS_STATE *new_state = insert_child(
									current_state,
									BB_PAWN,
									MOVE_DOWN_COUNT(pawn,pawn_direction) | pawn,
									EMPTY_BOARD);
	}
}

/* ---------------------------------------------------------------------------------------
	void pawn_double_moves(CHESS_STATE *current_state)
	
	purpose: adds any pawn double moves
   --------------------------------------------------------------------------------------- */
inline void pawn_double_moves(CHESS_STATE *current_state, bitboard empty_squares)
{
	/* Pawn Single Moves */
	COLOR_ENUM color = (current_state->flg_is_white_move ? BB_WHITE : BB_BLACK);
	int pawn_direction = (color==BB_WHITE) ? 1 : -1;
	bitboard start_mask = (color==BB_WHITE) ? PAWN_START_WHITE : PAWN_START_BLACK;
	
	bitboard pawn_moves = current_state->boards[color + BB_PAWN] & start_mask;
	pawn_moves = MOVE_UP_COUNT(pawn_moves,pawn_direction) & empty_squares;
	pawn_moves = MOVE_UP_COUNT(pawn_moves,pawn_direction) & empty_squares;
	
	bitboard pawn;
	while((pawn = get_lowest_bit(&pawn_moves))!=0)
	{
		CHESS_STATE *new_state = insert_child(
									current_state,
									BB_PAWN,
									MOVE_DOWN_COUNT(pawn,2*pawn_direction) | pawn,
									EMPTY_BOARD);

		// when pawn moves double - exposes to possible en passant
		if (new_state != NULL)
		{
			new_state->flg_is_enpassant = 1;
			new_state->flg_enpassant_file = COL(floor_log2(pawn));
		}
	}
}

/* ---------------------------------------------------------------------------------------
	void simple_move_mask_moves(CHESS_STATE *current_state)
	
	purpose: adds any moves that can be done by simple mask
	         basically, king and knight moves.
   --------------------------------------------------------------------------------------- */
inline void simple_move_mask_moves(CHESS_STATE *current_state, 
									PIECE_ENUM piece,
									fp_cached_move_mask move_mask_function,
									bitboard empty_squares,
									bitboard opponents_squares)
{
	COLOR_ENUM color = current_state->flg_is_white_move ? BB_WHITE : BB_BLACK;
	bitboard pieces = current_state->boards[color + piece];
	bitboard next_piece = 0;
	bitboard next_move;
	bitboard available_squares = empty_squares | opponents_squares;
	
	while ((next_piece=get_lowest_bit(&pieces))!=0)
	{
		bitboard move_mask = move_mask_function(next_piece) & available_squares;
		
		while ((next_move=get_lowest_bit(&move_mask))!=0)
		{
			// add move
			CHESS_STATE *new_state = insert_child(
										current_state,
										piece,
										next_piece | next_move,
										next_move & opponents_squares);
		}
	}
}

/* ---------------------------------------------------------------------------------------
	void pawn_capture_moves(CHESS_STATE *current_state)
	
	purpose: adds any pawn double moves
   --------------------------------------------------------------------------------------- */
inline void pawn_capture_moves(CHESS_STATE *current_state, bitboard empty_squares, bitboard opponents_squares)
{
	/* Pawn Single Moves */
	COLOR_ENUM color = current_state->flg_is_white_move ? BB_WHITE : BB_BLACK;
	int pawn_direction = (color==BB_WHITE) ? 1 : -1;

	int horizontal_direction = 0;
	for (horizontal_direction=-1; horizontal_direction <= 1; horizontal_direction+=2)
	{ 
		bitboard pawn_moves =
			MOVE_UP_COUNT(current_state->boards[color + BB_PAWN],pawn_direction) &
			(horizontal_direction==-1 ? ~LEFT_FILE : ~RIGHT_FILE);
	
		pawn_moves = MOVE_RIGHT_COUNT(pawn_moves,horizontal_direction) &
			opponents_squares;
	
		bitboard pawn_move;
	
		while((pawn_move = get_lowest_bit(&pawn_moves))!=0)
		{
			CHESS_STATE *new_state = insert_child(current_state,
				BB_PAWN,
				MOVE_DOWN_COUNT(MOVE_LEFT_COUNT(pawn_move,horizontal_direction),pawn_direction) | pawn_move,
				pawn_move);
		}
	}
}

/* ---------------------------------------------------------------------------------------
	void pawn_capture_moves(CHESS_STATE *current_state)
	
	purpose: adds any pawn double moves
   --------------------------------------------------------------------------------------- */
inline void en_passant_moves(CHESS_STATE *current_state)
{
	static bitboard white_enpassant_mask = B5 | H4;
	static bitboard black_enpassant_mask = B4 | H3;
	static bitboard en_passant_square_white = A5;
	static bitboard en_passant_square_black = A4;

	if (current_state->flg_is_enpassant==1)
	{
		COLOR_ENUM color = current_state->flg_is_white_move ? BB_WHITE : BB_BLACK;
		int pawn_direction = (color==BB_WHITE) ? 1 : -1;
		bitboard attacking_pawns = (color==BB_WHITE) ? white_enpassant_mask :
			black_enpassant_mask;
		
		attacking_pawns = (color == BB_WHITE ? EN_PASSANT_WHITE : EN_PASSANT_BLACK) & 
			MOVE_RIGHT_COUNT(attacking_pawns,current_state->flg_enpassant_file) &
			current_state->boards[color+BB_PAWN];

		bitboard next_pawn;
		while ((next_pawn=get_lowest_bit(&attacking_pawns))!=0)
		{
			// en passant moving to square behind where attacked
			// pawn currently resides.
			bitboard attacked_pawn = MOVE_RIGHT_COUNT(
				(color==BB_WHITE ? en_passant_square_white : en_passant_square_black),
				current_state->flg_enpassant_file);
			bitboard move_to_square = MOVE_UP_COUNT(attacked_pawn,(color==BB_WHITE ? 1 : -1));

			CHESS_STATE *new_state = insert_child(current_state,
							BB_PAWN,
							next_pawn | move_to_square,
							attacked_pawn);
		}
	}
}


/* ---------------------------------------------------------------------------------------
	void pawn_double_moves(CHESS_STATE *current_state)
	
	purpose: adds any pawn double moves
   --------------------------------------------------------------------------------------- */
inline void sliding_moves(CHESS_STATE *current_state,
				 bitboard empty_squares,
				 bitboard opponents_squares,
				 PIECE_ENUM piece,
				 fp_cached_sliding_move_mask sliding_move_mask_function)
{
	COLOR_ENUM color = current_state->flg_is_white_move ? BB_WHITE : BB_BLACK;
	bitboard pieces = current_state->boards[color + piece];
	bitboard next_piece = 0;
	bitboard next_move;
	bitboard friendly_squares = ~(empty_squares | opponents_squares);
	bitboard occupancy = friendly_squares | opponents_squares;
	
	while ((next_piece=get_lowest_bit(&pieces))!=0)
	{
		// Get sliding move mask and remove friendly captures.
		bitboard move_mask = sliding_move_mask_function(next_piece,occupancy)
								& (~friendly_squares);
		
		while ((next_move=get_lowest_bit(&move_mask))!=0)
		{
			// add move
			CHESS_STATE *new_state = insert_child(current_state,
							piece,
							next_piece | next_move,
							next_move & opponents_squares);
		}
	}
}


/* ---------------------------------------------------------------------------------------
	inline void remove_opponents_piece(CHESS_STATE *current_state, bitboard piece)
	
	purpose: removes any opponents pieces that may be on a particular
	         square - generally called after a move is made to ensure
	         only one piece is on a square.

	Notes:   this is always called within the child move - i.e the 'opponent'
	         is whatever the active color is.
	
	Returns: the value of the piece (if any) removed.
   --------------------------------------------------------------------------------------- */
inline int remove_opponents_piece(CHESS_STATE *current_state, bitboard piece)
{
	int i;
	
	COLOR_ENUM color = (current_state->flg_is_white_move ? BB_WHITE : BB_BLACK);
	for (i=0; i<PIECE_TYPES; i++)
	{
		if ((current_state->boards[color + i] & piece) != EMPTY_BOARD)
		{
			current_state->boards[color + i] = current_state->boards[color + i] ^ piece;
			if (i==BB_KING)
			{
				current_state->parent->flg_is_illegal=1;
			}
				
			return piece_values[color + i];
		}
	}
	
	// should neve get here as function only called when there is something to 
	// delete.
	printf("No piece to delete !\n");	
	abort();
}

/* ---------------------------------------------------------------------------------------
	char *get_move(CHESS_STATE *current_state, int ui_flag)
	
	purpose: gets the movetext for a chess state
	
	Notes: if ui_flag!=0, the MoveTextUI string is returned.   
		   This routine checks all bitboards.
		   Move is valid if:
		   1. Maximum of 1 piece moves
		   2. Exception to #1 above is castling
		   3. Maximum of 1 piece removed

	The full move is either normal text or UI text
	normal text, eg xe3 (pawn captures piece at e3)
	UI text , eg d2:e3 e3:P (black pawn captures white pawn at e3)
	
	when ui_text==0, the standard algebraic chess notation
	for the move is returned. This is the abbreviated form
	UNLESS an explicit representation is required, in order
	of:
	1. specific rank qualifier
	2. specific row qualifier
	3. specific exact starting square
	
	Special treatment required for:
	1. en passant moves
		   	
--------------------------------------------------------------------------------------- */
char *get_move(CHESS_STATE *current_state, int ui_flag)
{
	// full move
	char *move = malloc(20 * sizeof (char));
	memset(move,0,20);

	char from_str[3] = {0};
	char to_str[3] = {0};
	char captured_str[2] = {0};
	char enpassant_suffix[5]={0};
		
	int number_moved = 0;		// number of pieces moved - normally = 1
	int number_captured = 0;	// number of pieces captured - <=1
	int number_friendly_captured = 0;	// pawn promotion
	int number_new = 0;			// number of new pieces (pawn promotion)
	int captured_piece = 0;		// captured piece index
	int friendly_captured_piece = 0; // the pawn 'removed'
	int moved_piece=0;			// moving piece index
	int new_piece=0;			// the new piece (normally queen for promoted pawn)

	int explicit_row = 0;	// flags to denote explicit move text
	int explicit_col = 0;	// flags to denote explicit move text
	
	CHESS_STATE* parent = current_state->parent;
	
	// the color of the moving piece is the 'parent' move.
	// the current state is the 'position' after the move, where
	// the other piece has become the active color.
	COLOR_ENUM bb_color = (parent->flg_is_white_move) ? BB_WHITE : BB_BLACK;

	int i;
	for (i=0; i<BB_COUNT; i++)
	{
		if (parent->boards[i] != current_state->boards[i])
		{
			bitboard delta = (parent->boards[i])^
				(current_state->boards[i]);
			bitboard from = parent->boards[i] & delta;
			bitboard to = current_state->boards[i] & delta;
			
			if (bit_count(to) == bit_count(from))
			{
				// moved
				number_moved += bit_count(to);
				if (number_moved > 1)
				{
					printf("Too may pieces Moved !");
					abort();
				}
				
				moved_piece=i;
				
				// at this point, see if any other moves from same
				// parent state involve the same piece type moving
				// to the same finishing square
				CHESS_STATE *other_states;
				for (other_states = current_state->parent->child_head;
					other_states!=NULL;
					other_states=other_states->next)
				{
					if (other_states!=current_state)
					{
						//see if other state shares similar move:
						bitboard other_delta = (parent->boards[i])^
							(other_states->boards[i]);
						bitboard other_from = parent->boards[i] & other_delta;
						bitboard other_to = current_state->boards[i] & other_delta;

						if (other_to==to && other_from != from)	// other move cannot start from same square !
						{
							// another move DOES share the same finishing square
							explicit_row += 
								((ROW(floor_log2(from))==(ROW(floor_log2(other_from)))));
							explicit_col +=
								((COL(floor_log2(from))==(COL(floor_log2(other_from)))));
						}														
					}
				}				

				// special treatment #1 (en passant)
				if (moved_piece-bb_color==BB_PAWN &&
					parent->flg_is_enpassant==1 &&
					to == (parent->flg_enpassant_file + (bb_color==BB_WHITE?A6:A3)))
				{
					explicit_col += 1;
					strcpy(enpassant_suffix,"e.p.");
				}
								
				strcpy(from_str,get_square_text(floor_log2(from)));
				strcpy(to_str,get_square_text(floor_log2(to)));
			}
			else if (bit_count(from)>0 && i != (bb_color+BB_PAWN))
			{
				// captured (opponents only) - excluding pawn promotion
				number_captured +=  bit_count(from);
				if (number_captured > 1)
				{
					printf("more than one piece captured");					
					exit(1);
				}
				captured_piece=i;
				strcpy(captured_str,"x");
			}
			else if (bit_count(from) > 0)
			{
				// friendly pawn removed for pawn promotion
				number_friendly_captured +=  bit_count(from);
				if (number_friendly_captured > 1)
				{
					printf("more than one friendly piece captured");					
					exit(1);
				}
				friendly_captured_piece = i;
				strcpy(from_str,get_square_text(floor_log2(from)));
			}
			
			else if (bit_count(to) > 0)
			{
				// pawn_promotion
				number_new += bit_count(to);
				if (number_new > 1)
				{
					printf("more than one new piece");					
					exit(1);
				}
				new_piece = i;
				strcpy(to_str,get_square_text(floor_log2(to)));
			}
		}
	}

	// 1. piece moving (normal text)
	if (ui_flag==0 && moved_piece % 6 != BB_PAWN)
		move[0] = piece_chr[moved_piece];
	
	// 2. from square
	if (ui_flag!=0)
		strcat(move,from_str);
	else
	{
		if (explicit_col!=0)
			strncat(move,&from_str[0],1);
	
		if (explicit_row!=0)
			strncat(move,&from_str[1],1);
	}

	// 3. capture string or ':'
	if (ui_flag==0)
		if (number_captured>0)
			strcat(move, captured_str);
		else {}
	else
	{
		strcat(move, ":");
	}

	// 4. to square
	strcat(move,to_str);
	
	// 5. en passant suffix
	if (ui_flag==0)
		strcat(move,enpassant_suffix);

	// 6. piece to delete (UI text only)
	if (ui_flag!=0 && number_captured>0)
	{
		strcat(move," ");
		strcat(move,to_str);
		strcat(move,":");
		move[strlen(move)] = piece_chr[captured_piece];
	}		

	// 7. pawn promotion
	if (ui_flag==0 && number_new > 0)
	{
		move[strlen(move)] = piece_chr[new_piece];
	}

	if (ui_flag!=0 && number_new > 0)
	{
		// pawn
		strcat(move," ");
		strcat(move,from_str);
		strcat(move,":");
		move[strlen(move)] = piece_chr[friendly_captured_piece];
		
		//queen
		strcat(move," ");
		strcat(move,to_str);
		strcat(move,":");
		move[strlen(move)] = piece_chr[new_piece];
	}
		
	return move;
}

/* ---------------------------------------------------------------------------------------
	int child_count(CHESS_STATE *current_state)
	
	purpose: counts the _LEGAL_ children of a state
   --------------------------------------------------------------------------------------- */
int child_count(CHESS_STATE *current_state)
{
	
	CHESS_STATE *child;
	int c=0;

	for (child = current_state->child_head; child!=NULL; child=child->next)
		c += child->flg_is_illegal ? 0 : 1;
	
	return c;
}

/* ---------------------------------------------------------------------------------------
	CHESS_STATE *check_move(CHESS_STATE *parent_state, CHAR *move)
	
	purpose: scans the child moves of a parent state, and if the 
	         supplied movetext (which can be either algebraic chess
	         notation, or move ui text) matches the move text of the 
	         child state, the child state is returned. else null is returned.
   --------------------------------------------------------------------------------------- */
CHESS_STATE *check_move(CHESS_STATE *parent_state, char *move)
{
	CHESS_STATE *child = parent_state->child_head;

	while (child !=NULL)
	{
		if (strcmp(get_move(child,0),move)==0)
			return child;
		else
		{
			char move_text_ui_buffer[20] = {0};
			strcpy(move_text_ui_buffer,get_move(child,1));
			// get the first move of the move text ui
			// this is the 'main move' and is what is 
			// checked with user's IN move.
			char *main_ui_move = strtok(move_text_ui_buffer, " ");	
			if (strcmp(main_ui_move, move)==0)
				return child;
		}
		child = child->next;
	}
	return NULL;
}

/* --------------------------------------------------------------------------------------
    void delete_state(CHESS_STATE *cs)
    
    purpose: deletes the children from a chess state (and all children recursively).
   ---------------------------------------------------------------------------------------*/
void delete_nodes(CHESS_STATE *cs)
{
	CHESS_STATE *p;
	CHESS_STATE *q;

	for (p = cs->child_head; p != NULL; p = q)
	{
		q = p->next;
		delete_nodes(p);
		free(p);
		p=NULL;
	}
	cs->child_head=NULL;
}

/* --------------------------------------------------------------------------------------
	void swap_nodes(CHESS_STATE *node_1, CHESS_STATE *node_2)

	Purpose: swaps a node with the head node in a linked list
	Used to put the 'best' move at each level first in the list
	so as to improve the performance of the alpha beta search.
   ---------------------------------------------------------------------------------------*/
void swap_nodes(CHESS_STATE *node_1, CHESS_STATE *node_2)
{
	CHESS_STATE tmp = *node_1;
	
	// get node before node_2
	CHESS_STATE *iterator = node_2->parent;
	CHESS_STATE *prev = NULL;
	
	while (iterator != NULL)
	{
		if (iterator->next==node_2)
		{
			prev=iterator;
			break;
		}
		iterator = iterator->next;
	}
	
	// if node 2 not already first in list, then swap with first
	if (prev != NULL)
	{
		node_2->parent->child_head = node_2;
		node_2->next = node_1->next;
		prev->next = node_1;
	}
	
}
/* --------------------------------------------------------------------------------------
	int alpha_beta(CHESS_STATE *node, int depth, int alpha, int beta)
    
    purpose: alpha-beta pruning evaluation of minimax algorithm
			 the algorithm always returns an absolute number
			 regardless of player's color.
			 + = good
			 - = bad

			beta represents previous player's best choice. Doesn't want it
			if alpha would worsen it.
   ---------------------------------------------------------------------------------------*/
int alpha_beta(CHESS_STATE *node, int current_depth, int fixed_depth_horizon, int alpha, int beta, ITERATIVE_DEEPENING_INFO *id_info)
{
	// leaf node test (include quiescence search)
	// note that we extend the current depth to go 1 PAST
	// the fixed depth horizon, to allow for check validation
	// at the leaf node. The quiescence search will automatically
	// cater for check validation
	if (current_depth>fixed_depth_horizon && !node->flg_is_noisy)
	{
		id_info[current_depth-1].moves_evaluated++;
		return node->board_value;
	}
	
	// otherwise, get opponents moves
	generate_moves(node);
	current_depth++;

	CHESS_STATE *p;
	CHESS_STATE *q;

	for (p = node->child_head; p != NULL; p = q) 
	{
		q = p->next;

		if (node->flg_is_white_move)
		{
			// maximiser
			if (alpha>=beta)
			{
				break;		// cutoff
			}
			int ab = alpha_beta(p, current_depth, fixed_depth_horizon, alpha, beta, id_info);
			if (ab > alpha)
			{
				alpha=ab;
				int d;
				
				if (current_depth==1)
					id_info[fixed_depth_horizon].best_move = get_move(p,0);
			}
		}
		else
		{
			// minimiser
			if (alpha>=beta)
			{
				break;		// cutoff
			}
			int ab = alpha_beta(p, current_depth, fixed_depth_horizon, alpha, beta, id_info);
			if (ab < beta)//
			{
				beta=ab;
				int d;
				if (current_depth==1)
					id_info[fixed_depth_horizon].best_move = get_move(p,0);
			}
		}
	}

	// legal moves (only required for depth 1)
	if (current_depth==1)
	{
		CHESS_STATE *child;

		int i=0;
		for (child=node->child_head; child!=NULL; child=child->next)
		{
			if (child->flg_is_illegal==0)
			{
				id_info[1].legal_moves[i] = get_move(child,0);
				i++;
			}
		}
		id_info[1].legal_move_count=i;
	}
	
	delete_nodes(node);
	
	if (node->flg_is_white_move)
	{
		// maximier
		return alpha;
	}
	else
	{
		return beta;
	}
}

/* ---------------------------------------------------------------------------
	void iterative_deepening(CHESS_STATE *cs, int max_depth)
   
   Purpose: An iterative deepening function
            The idea is to repeat search at each depth from 1 to n.
            At start of each search > 1, the best result from the previous
            search is performed first. This improves the performance of
            alpha beta.
   Returns: 3 structures are returned by reference as output parameters:
   			1. ITERATIVE_DEEPENING_INFO structure
   			2. Legal Move Count
   			3. Legal Moves 
   --------------------------------------------------------------------------- */
void iterative_deepening(CHESS_STATE *cs,
				 int time_limit,
				 int *id_count,
				 ITERATIVE_DEEPENING_INFO *id_info,
				 int debug_mode)
{
	int depth;
	depth=0;
	
	if (debug_mode==1)
	{
		printf("\nDepth\tValue\tCount\tPlay\tTime\tRate\n");
		printf("-----\t-----\t-----\t----\t----\t----\n");
	}
	
	while (depth < MAX_ITERATIVE_DEEPENING_DEPTH)
	{
		depth++;
		id_info[depth].depth = depth;
		
		// start stopwatch
		clock_t start, end;
		start = clock();
		id_info[depth].value = alpha_beta(cs, 0, depth, -999999, 999999, id_info);
		end = clock();
		
		double elapsed;
		elapsed = ((double)(end-start)) / CLOCKS_PER_SEC;
		int rate;
		rate = (elapsed==0) ? 0 : (float)id_info[depth].moves_evaluated / elapsed;
		
		// display best values
		if (debug_mode==1)
			printf("%d\t%d\t%d\t%s\t%.2f\t%d\n",id_info[depth].depth,id_info[depth].value, id_info[depth].moves_evaluated, id_info[depth].best_move, elapsed, rate);
	
		// check time limit elapsed
		if (time_limit==0 || id_info[1].legal_move_count * elapsed > (double)time_limit)
		{
			*id_count = depth;
			break;
		}
	}
	
	// print legal moves
	if (debug_mode==1)
	{
		printf("\nLegal move count: %d\nMoves: ", id_info[1].legal_move_count);
		int i;
		for (i=0; i<id_info[1].legal_move_count; i++)
			printf("%s ", id_info[1].legal_moves[i]);
	
		printf("\n\n");
	}
}


