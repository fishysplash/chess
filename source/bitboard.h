/* -------------------------------
   bitboad.h
   
   header file for bitboard functions
   
   Notes: The bitboards are 'flipped' vertically.
   the LSB is col h and the MSB is col a. Note the
   direction of shifting of MOVE_LEFT and MOVE_RIGHT
   macros.
   
   (C) 2008 David Barone
   
   -------------------------------*/
   
#ifndef BITBOARD_H
	#define BITBOARD_H

	// 64-bit structure to handle bitboard	
	typedef unsigned long long int bitboard;

	/* Constant definitions */
	#define PAWN_START_WHITE	0x000000000000FF00ULL
	#define PAWN_START_BLACK	0x00FF000000000000ULL
	#define RIGHT_FILE			0x8080808080808080ULL
	#define LEFT_FILE			0x0101010101010101ULL
	#define RIGHT_2_FILES		0xc0c0c0c0c0c0c0c0ULL
	#define LEFT_2_FILES		0x0303030303030303ULL
	#define	TOP_2_RANKS			0xffff000000000000ULL
	#define BOTTOM_2_RANKS		0x000000000000ffffULL
	#define	TOP_RANK			0xFF00000000000000ULL
	#define	BOTTOM_RANK			0x00000000000000FFULL
	#define TOP_BOTTOM_RANKS	0xFF000000000000FFULL
	#define LEFT_RIGHT_FILES	0x8181818181818181ULL
	#define	CASTLE_BLACK_QUEEN	0xf800000000000000ULL
	#define	CASTLE_BLACK_KING	0x0f00000000000000ULL
	#define	CASTLE_WHITE_QUEEN	0x00000000000000f8ULL
	#define	CASTLE_WHITE_KING	0x000000000000000fULL
	#define	EMPTY_BOARD			0ULL
	#define	FULL_BOARD			0xFFFFFFFFFFFFFFFFULL
	#define EN_PASSANT_WHITE	0x000000FF00000000ULL
	#define EN_PASSANT_BLACK	0x00000000FF000000ULL
	
	/* square values */
	#define A1 0x0000000000000001ULL
	#define B1 0x0000000000000002ULL
	#define C1 0x0000000000000004ULL
	#define D1 0x0000000000000008ULL
	#define E1 0x0000000000000010ULL
	#define F1 0x0000000000000020ULL
	#define G1 0x0000000000000040ULL
	#define H1 0x0000000000000080ULL
	#define A2 0x0000000000000100ULL
	#define B2 0x0000000000000200ULL
	#define C2 0x0000000000000400ULL
	#define D2 0x0000000000000800ULL
	#define E2 0x0000000000001000ULL
	#define F2 0x0000000000002000ULL
	#define G2 0x0000000000004000ULL
	#define H2 0x0000000000008000ULL
	#define A3 0x0000000000010000ULL
	#define B3 0x0000000000020000ULL
	#define C3 0x0000000000040000ULL
	#define D3 0x0000000000080000ULL
	#define E3 0x0000000000100000ULL
	#define F3 0x0000000000200000ULL
	#define G3 0x0000000000400000ULL
	#define H3 0x0000000000800000ULL
	#define A4 0x0000000001000000ULL
	#define B4 0x0000000002000000ULL
	#define C4 0x0000000004000000ULL
	#define D4 0x0000000008000000ULL
	#define E4 0x0000000010000000ULL
	#define F4 0x0000000020000000ULL
	#define G4 0x0000000040000000ULL
	#define H4 0x0000000080000000ULL
	#define A5 0x0000000100000000ULL			
	#define B5 0x0000000200000000ULL			
	#define C5 0x0000000400000000ULL			
	#define D5 0x0000000800000000ULL			
	#define E5 0x0000001000000000ULL			
	#define F5 0x0000002000000000ULL			
	#define G5 0x0000004000000000ULL			
	#define H5 0x0000008000000000ULL			
	#define A6 0x0000010000000000ULL			
	#define B6 0x0000020000000000ULL			
	#define C6 0x0000040000000000ULL			
	#define D6 0x0000080000000000ULL			
	#define E6 0x0000100000000000ULL			
	#define F6 0x0000200000000000ULL			
	#define G6 0x0000400000000000ULL			
	#define H6 0x0000800000000000ULL			
	#define A7 0x0001000000000000ULL			
	#define B7 0x0002000000000000ULL			
	#define C7 0x0004000000000000ULL			
	#define D7 0x0008000000000000ULL			
	#define E7 0x0010000000000000ULL			
	#define F7 0x0020000000000000ULL			
	#define G7 0x0040000000000000ULL			
	#define H7 0x0080000000000000ULL			
	#define A8 0x0100000000000000ULL			
	#define B8 0x0200000000000000ULL			
	#define C8 0x0400000000000000ULL			
	#define D8 0x0800000000000000ULL			
	#define E8 0x1000000000000000ULL			
	#define F8 0x2000000000000000ULL			
	#define G8 0x4000000000000000ULL			
	#define H8 0x8000000000000000ULL			

	#define KNIGHT_BASE_SQUARE D4
	#define KNIGHT_BASE_MASK (C2+E2+B3+F3+B5+F5+C6+E6)
	
	#define KING_BASE_SQUARE D4
	#define KING_BASE_MASK (C3+D3+E3+C4+E4+C5+D5+E5)

	#define A1H8_DIAG (A1|B2|C3|D4|E5|F6|G7|H8)
	#define A8H1_DIAG (A8|B7|C6|D5|E4|F3|G2|H1)
	
	struct _sliding_move_hash_entry {
		struct _sliding_move_hash_entry *next;
		bitboard occupancy_key;
		bitboard attacks;
	};
	typedef struct _sliding_move_hash_entry SLIDING_MOVE_HASH_ENTRY;
	
	/* prototypes */
	int bit_count(bitboard board);
	
	// returns lowest bit, and removed from *board.
	bitboard get_lowest_bit(bitboard *board);			
	
	// converts the bitboard to a square number (0-63)
	int floor_log2(bitboard board);					
	
	// calculates bitboards for pieces with fixed move masks
	// like kings and knights	
	inline bitboard simple_move_mask(bitboard piece_to_move,
							bitboard base_square,
							bitboard base_mask,
							bitboard left_edge_mask,
							bitboard right_edge_mask,
							bitboard top_edge_mask,
							bitboard bottom_edge_mask);
	
	// cached impl of simple_move_mask
	bitboard cached_king_move_mask(bitboard piece_to_move);
	bitboard cached_knight_move_mask(bitboard piece_to_move);

	//bitboard sliding_move_mask(bitboard piece_to_move, bitboard available_squares);
	bitboard get_attacks(SLIDING_MOVE_HASH_ENTRY *attack_table[8][257], int square_index, bitboard occupancy_key);
	void sliding_move_mask(SLIDING_MOVE_HASH_ENTRY *cached_attacks[8][257],	int dx, int dy, bitboard ray_mask);
	bitboard cached_rook_move_mask(bitboard start_square, bitboard occupancy);
	bitboard cached_bishop_move_mask(bitboard start_square, bitboard occupancy);

	void debug_bitboard(bitboard board);


	/* --------------------*/
	/* macros              */
	/* --------------------*/
	#define MOVE_UP(board) ((bitboard)board << 8)
	#define MOVE_UP_COUNT(board, count)(((count)>0)?(bitboard)board<<(8*(count)):(bitboard)board>>(8*(-(count))))	
	#define MOVE_DOWN(board) ((bitboard)board >> 8)
	#define MOVE_DOWN_COUNT(board, count)(((count)>0)?(bitboard)board>>(8*(count)):(bitboard)board << (8*(-(count))))	
	#define MOVE_LEFT(board) ((bitboard)board >> 1)
	#define MOVE_LEFT_COUNT(board, count)(((count)>0)?(bitboard)board>>(1*(count)):(bitboard)board << (1*(-(count))))
	#define MOVE_RIGHT(board) ((bitboard)board << 1)
	#define MOVE_RIGHT_COUNT(board, count)(((count)>0)?(bitboard)board<<(1*(count)):(bitboard)board>>(1*(-(count))))
	// converts a board into a bit mask
	#define BIT_MASK(board)(((board)==0) ? EMPTY_BOARD : FULL_BOARD)
	
#endif /* BITBOARD_H */
