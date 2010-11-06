 #include <stdio.h>
 #include "bitboard.h"
 #include "main.h"
 #include "math.h"
 #include <stdlib.h>

/* ------------------------------------------------------------------------
	int bit_count(bitboard board)
	
	purpose: gets and number of set bits on a bitboard.
   ------------------------------------------------------------------------ */
int bit_count(bitboard board)
{
	int n;
	for (n = 0; board != 0; n++, board &= (board - 1)) ;
	return n;
}

/* ------------------------------------------------------------------------
	bitboard get_lowest_bit(bitboard *board)
	
	purpose: gets and removes the lowest bit from a bit board.
	Note:    the bit IS REMOVED from the original bitboard.
   ------------------------------------------------------------------------ */
bitboard get_lowest_bit(bitboard *board)
{
	int count = bit_count(*board);
	bitboard lowest_bit=0;
	
	if (count>0)
	{
		lowest_bit = (*board & ((~*board)+1));
		*board = *board & ~lowest_bit;	
	}

	return lowest_bit;
}

/* ------------------------------------------------------------------------
	int floor_log2 (bitboard *board)
	
	purpose: gets the bitnumber of a single-bit board using Log2 maths.
   ------------------------------------------------------------------------ */
int floor_log2(bitboard board)
{
	int pos = 0;
	if (board >= (bitboard)1<<32) { board >>= 32; pos += 32; }  
	if (board >= (bitboard)1<<16) { board >>= 16; pos += 16; }
	if (board >= (bitboard)1<< 8) { board >>=  8; pos +=  8; }
	if (board >= (bitboard)1<< 4) { board >>=  4; pos +=  4; }
	if (board >= (bitboard)1<< 2) { board >>=  2; pos +=  2; }
	if (board >= (bitboard)1<< 1) {               pos +=  1; }

	return ((board == 0) ? (-1) : pos);
}

/* ------------------------------------------------------------------------
	simple_move_mask()
	
	purpose: routine to get a bitboard of available moves for pieces that
	        have a 'fixed' move mask. This is basically king and knight
	        moves - both these pieces have up to 8 available moves.
	         
			The routine returns a bitmask for the available moves
			for the given 'piecetomove' bitboard. This routine works by
    		starting with available moves at d4 and then shifting the mask
    		to required position, whilst removing off-edge moves.

	For example, for knight moves: 
	
	8  0  0  0  0  0  0  0  0
    7  0  0  0  0  0  0  0  0           A magic number (passed in as 'd4moves'
    6  0  0  X  0  X  0  0  0           is defined based on moves available
    5  0  X  0  0  0  X  0  0           from d4 (see left). This magic number
    4  0  0  0  K  0  0  0  0          	is then shifted to whatever square the
    3  0  X  0  0  0  X  0  0 			actual knight is on, and then bit that
    2  0  0  X  0  X  0  0  0           'wrap' around to other side of board
    1  0  0  0  0  0  0  0  0			are removed, thus leaving only legal
       a  b  c  d  e  f  g  h			moves !
   ------------------------------------------------------------------------ */
bitboard simple_move_mask(bitboard piece_to_move,
							bitboard base_square,
							bitboard base_mask,
							bitboard left_edge_mask,
							bitboard right_edge_mask,
							bitboard top_edge_mask,
							bitboard bottom_edge_mask)
{
	int square = floor_log2(piece_to_move);	// square number for piece
	
	int base_square_number = floor_log2(base_square);

	// shift the magic mask relative to square.
	base_mask = MOVE_DOWN_COUNT(base_mask, ROW(base_square_number)-ROW(square));
	base_mask = MOVE_LEFT_COUNT(base_mask, COL(base_square_number)-COL(square));

	// Now remove off-board pieces.
	// This done by assuming that if initial piece within x squares of an 
	// edge, then any calculated moves within x squares of the opposite edge
	// are illegal where x is the 'maximum' length a piece can move in a
	// given direction (2 for knights, 1 for kings)
	bitboard is_right_edge = BIT_MASK(right_edge_mask & piece_to_move);
	bitboard is_left_edge = BIT_MASK(left_edge_mask & piece_to_move);
	bitboard is_top_edge = BIT_MASK(top_edge_mask & piece_to_move);
	bitboard is_bottom_edge = BIT_MASK(bottom_edge_mask & piece_to_move);
	
	bitboard remove_off_board_left = (~is_right_edge) | (~left_edge_mask);
	bitboard remove_off_board_right = (~is_left_edge) | (~right_edge_mask);
	bitboard remove_off_board_bottom = (~is_top_edge) | (~bottom_edge_mask);
	bitboard remove_off_board_top = (~is_bottom_edge) | (~top_edge_mask);

	return base_mask &
			remove_off_board_left &
			remove_off_board_right &
			remove_off_board_bottom &
			remove_off_board_top;	
}

/* ------------------------------------------------------------------------
	bitboard cached_knight_move_mask(bitboard piece_to_move)
	
	Purpose: Implementation of simple_move_mask for knight moves
	 		 featuring caching as well.
	------------------------------------------------------------------------ */
bitboard cached_knight_move_mask(bitboard piece_to_move)
{
	static bitboard cache[64] = {0};
	static int is_cached = 0;
	
	if (is_cached==0)
	{
		is_cached=1;
		int i = 0;
		for (i=0; i<64; i++)
		{
			cache[i] = simple_move_mask((bitboard)1<<i,
										KNIGHT_BASE_SQUARE,
										KNIGHT_BASE_MASK,
										LEFT_2_FILES,
										RIGHT_2_FILES,
										TOP_2_RANKS,
										BOTTOM_2_RANKS);
		}
	}				
	
	return cache[floor_log2(piece_to_move)];	
}

/* ------------------------------------------------------------------------
	bitboard cached_king_move_mask(bitboard piece_to_move)
	
	Purpose: Implementation of simple_move_mask for king moves
	 		 featuring caching as well.
	------------------------------------------------------------------------ */
bitboard cached_king_move_mask(bitboard piece_to_move)
{
	static bitboard cache[64] = {0};
	static int is_cached = 0;
	
	if (is_cached==0)
	{
		is_cached=1;
		int i = 0;
		for (i=0; i<64; i++)
		{
			cache[i] = simple_move_mask((bitboard)1<<i,
										KING_BASE_SQUARE,
										KING_BASE_MASK,
										LEFT_FILE,
										RIGHT_FILE,
										TOP_RANK,
										BOTTOM_RANK);
		}
	}				
	
	return cache[floor_log2(piece_to_move)];	
}

/* ------------------------------------------------------------------------
	void sliding_move_mask(SLIDING_MOVE_HASH_ENTRY *cached_attacks[8][257], int dx, int dy, bitboard ray_mask)
	
	Purpose: Caches sliding moves based on square (0~7) and occupancy (0~255).
	         Occupancy represents distribution of pieces (both friendly and
	         opponent). It is up to the actual implementor of the cached
	         moved table to remove friendly piece captures.
	
	Further improvement: cound reduce array from 256 to 64, since dont need
	         starting square nor final square in ray.
   ------------------------------------------------------------------------ */
void sliding_move_mask(SLIDING_MOVE_HASH_ENTRY *cached_attacks[8][257], int dx, int dy, bitboard ray_mask)
{
	bitboard ray_bits[8] = {0};	// individual bits of ray mask
	bitboard occupancy_values[256]; // set bit indicates piece (either own or opponent)
	int i;
	bitboard ray_mask_copy = ray_mask;
	
	// preliminaries...
	for (i=0; i<8; i++)
	{
		ray_bits[i]=get_lowest_bit(&ray_mask_copy);
	}

	int j;
	for (j=0; j<256; j++)
	{
		bitboard temp = j;
		occupancy_values[j] = 0;
		bitboard next_bit;
		while ((next_bit = get_lowest_bit(&temp)) != 0)
		{
			occupancy_values[j] |= ray_bits[floor_log2(next_bit)];
		}
	}

	// now calculate values of all occupancies for each of the 8 squares in the ray.
	for (i=0; i<8; i++)
	{
		for (j=0; j<256; j++)
		{
			// hash value and add entry to linked list.
			int hash = (int)(occupancy_values[j] % 257);
			SLIDING_MOVE_HASH_ENTRY *new_entry = (SLIDING_MOVE_HASH_ENTRY*)malloc(sizeof(SLIDING_MOVE_HASH_ENTRY));
			new_entry->next=cached_attacks[i][hash];
			new_entry->occupancy_key = occupancy_values[j];
			new_entry->attacks = 0;
			cached_attacks[i][hash] = new_entry;

			int direction;
			for (direction=-1; direction<=1; direction+=2)
			{
				bitboard tmp_square = ray_bits[i];
				
				while (1)
				{
					// move as long as no occupancy on square, or tmp_square falls
					// off ray mask
					if ((tmp_square & ray_mask) == 0)
						break;
					
					new_entry->attacks |= tmp_square;

					if (tmp_square!=ray_bits[i] && (tmp_square & occupancy_values[j]) != 0)
						break;
						
					tmp_square = MOVE_RIGHT_COUNT(tmp_square,dx * direction);
					tmp_square = MOVE_UP_COUNT(tmp_square,dy * direction);
				}
				new_entry->attacks ^= ray_bits[i];	// remove starting square
			}
		}
	}
}

/* ------------------------------------------------------------------------
	bitboard get_attacks(SLIDING_MOVE_HASH_ENTRY *attack_table[8][257], int square_index, bitboard occupancy_key) 
	
	Purpose: Gets the attack bitboard from linked list hash table of attack moves.
			 This is the method of obtaining sliding moves with high parallelism.
			 This implementation attempts to find sliding moves via 'Magic Bitboard'
			 (enter URL HERE), rather than by rotated bitboards, which create a lot of overhead.
			 
			 NOTE: square_index is index into 1st array dimension - LSB is at position 0
	------------------------------------------------------------------------ */
bitboard get_attacks(SLIDING_MOVE_HASH_ENTRY *attack_table[8][257], int square_index, bitboard occupancy_key) 
{
	int hash_bucket = occupancy_key % 257;
	
	SLIDING_MOVE_HASH_ENTRY *move = 
		attack_table[square_index][hash_bucket];
		
	while (move->occupancy_key != occupancy_key)
	{ 
		if (move->next==NULL)
			abort();
			
		move = move->next;
	}
	return move->attacks;
}

/* ------------------------------------------------------------------------
	bitboard cached_king_move_mask(bitboard piece_to_move)
	
	Purpose: Implementation of simple_move_mask for king moves
	 		 featuring caching as well.
	------------------------------------------------------------------------ */
bitboard cached_rook_move_mask(bitboard start_square, bitboard occupancy)
{
	static SLIDING_MOVE_HASH_ENTRY *rank_attacks[8][257];
	static SLIDING_MOVE_HASH_ENTRY *file_attacks[8][257];
	
	static int is_cached;
	
	if (is_cached==0)
	{
		is_cached=1;
		sliding_move_mask(rank_attacks,1,0,BOTTOM_RANK);
		sliding_move_mask(file_attacks,0,1,LEFT_FILE);
	}
	
	int square_number = floor_log2(start_square);
	int row = ROW(square_number);
	int col = COL(square_number);
	
	bitboard rank_occupancy = MOVE_DOWN_COUNT(occupancy,row) & BOTTOM_RANK;
	bitboard file_occupancy = MOVE_LEFT_COUNT(occupancy,col) & LEFT_FILE;
		
	bitboard rank_attack_mask = MOVE_UP_COUNT(get_attacks(rank_attacks,col,rank_occupancy),row);
	bitboard file_attack_mask = MOVE_RIGHT_COUNT(get_attacks(file_attacks,row,file_occupancy),col);

	return (rank_attack_mask|file_attack_mask);
}

/* ------------------------------------------------------------------------
	bitboard cached_king_move_mask(bitboard piece_to_move)
	
	Purpose: Implementation of simple_move_mask for king moves
	 		 featuring caching as well.
	------------------------------------------------------------------------ */
bitboard cached_bishop_move_mask(bitboard start_square, bitboard occupancy)
{
	static SLIDING_MOVE_HASH_ENTRY *a1h8_diag_attacks[8][257];
	static SLIDING_MOVE_HASH_ENTRY *a8h1_diag_attacks[8][257];
	
	static int is_cached;
	
	if (is_cached==0)
	{
		is_cached=1;
		sliding_move_mask(a1h8_diag_attacks,1,1,A1H8_DIAG);
		sliding_move_mask(a8h1_diag_attacks,-1,1,A8H1_DIAG);
	}
	
	int square_number = floor_log2(start_square);
	int row = ROW(square_number);
	int col = COL(square_number);
	
	bitboard a1h8_diag_occupancy = MOVE_DOWN_COUNT(occupancy,row-col) & A1H8_DIAG;
	bitboard a8h1_diag_occupancy = MOVE_DOWN_COUNT(occupancy,col-7+row) & A8H1_DIAG;

	bitboard a1h8_diag_attack_mask = MOVE_UP_COUNT(get_attacks(a1h8_diag_attacks,col,a1h8_diag_occupancy),row-col);
	bitboard a8h1_diag_attack_mask = MOVE_UP_COUNT(get_attacks(a8h1_diag_attacks,7-col,a8h1_diag_occupancy),col-7+row);

	return (a1h8_diag_attack_mask|a8h1_diag_attack_mask);
}

/* ------------------------------------------------------------------------
	char *debug_bitboard(bitboard board)
	
	purpose: pretty-prints a bitboard
   ------------------------------------------------------------------------ */
void debug_bitboard(bitboard board)
{
	char *output = malloc(65*sizeof(char));
	printf(" ---------- ");
	int row;
	int col;
	for (row=7;row>=0; row--)
	{
		printf("\n|%d ", row+1);
		for (col=0; col<8; col++)
		{
			if ((((bitboard)1)<<((row*8+col)) & board) == (bitboard)1<<(row*8+col))
				putchar('X');
			else
				putchar(' ');
		}
		printf("|");
	}
	printf("\n|  abcdefgh|\n");
	printf(" ---------- \n");
}

