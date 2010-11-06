#include "bitboard.h"
#include <assert.h>

#ifdef DEBUG

	void test_bit_count()
	{
		/* bitcount() */
		assert(bit_count(PAWN_START_WHITE)==8);	
		assert(bit_count(FULL_BOARD)==64);	
		assert(bit_count(EMPTY_BOARD)==0);	
	}

	void test_get_lowest_bit()
	{
		/* get_lowest_bit */
		bitboard bb = 15;
		bitboard lb = get_lowest_bit(&bb);
		assert(bb==14);
		assert(lb==1);

		bb = 0;
		lb = get_lowest_bit(&bb);
		assert(bb==0);
		assert(lb==0);
	}

	void test_floor_log2()
	{
		/* floorlog2() */
		assert(floor_log2(16)==4);
		assert(floor_log2(1)==0);
		assert(floor_log2(((bitboard)1<<63)==64));
	}

	void test_simple_move_mask()
	{
		/* simple move mask */
		bitboard knight_start = B1;

		/* Check that knight moves at B1 should be A3, C3, and D2 */
		assert(simple_move_mask(B1,
								KNIGHT_BASE_SQUARE,
								KNIGHT_BASE_MASK,
								LEFT_2_FILES,
								RIGHT_2_FILES,
								TOP_2_RANKS,
								BOTTOM_2_RANKS) == A3+C3+D2);
	}

	void test_move()
	{
		/* bitboard MOVE macros */
		assert(MOVE_UP_COUNT(PAWN_START_WHITE,5)==PAWN_START_BLACK);	
		assert(MOVE_UP_COUNT(PAWN_START_WHITE,7)==EMPTY_BOARD);
		assert(MOVE_DOWN_COUNT(PAWN_START_BLACK,5)==PAWN_START_WHITE);	
		assert(MOVE_LEFT_COUNT(RIGHT_FILE,7)==LEFT_FILE);	
		assert(MOVE_RIGHT_COUNT(LEFT_FILE,7)==RIGHT_FILE);
	}

	void test_bit_mask()
	{
		/* BIT_MASK */
		assert(BIT_MASK(0) == 0);
		assert(BIT_MASK(1) == FULL_BOARD);
		assert(BIT_MASK(-234) == FULL_BOARD);
		assert(BIT_MASK(EMPTY_BOARD & 123) == EMPTY_BOARD);
	}		

	void test_cached_king_move_mask()
	{
		/* Bitboard masks for move generation */
		assert(cached_king_move_mask(E8)==D8|D7|E7|F7|F8);
		assert(cached_king_move_mask(E1)==D1|D2|E2|F2|F1);
		assert(cached_king_move_mask(A1)==A2|B2|B1);
		assert(cached_king_move_mask(H1)==G8|G7|H7);
	}

	void test_cached_knight_move_mask()
	{
		assert(cached_knight_move_mask(E8)==F6|G7|D6|C7);
		assert(cached_knight_move_mask(E1)==F3|G2|D3|C2);
		assert(cached_knight_move_mask(A1)==B3|C2);
		assert(cached_knight_move_mask(H1)==G6|F7);
	}

	void test_cached_rook_move_mask()
	{
		assert(cached_rook_move_mask(C4,EMPTY_BOARD)==(A4|B4|D4|E4|F4|G4|H4|C1|C2|C3|C5|C6|C7|C8));
		assert(cached_rook_move_mask(A1,EMPTY_BOARD)==(B1|C1|D1|E1|F1|G1|H1|A2|A3|A4|A5|A6|A7|A8));
		assert(cached_rook_move_mask(A1,(A4|A6|A8|C1|D1|E1|H1))==(A2|A3|A4|B1|C1));
		assert(cached_rook_move_mask(H8,(H6|H2|H1|G8))==(H7|H6|G8));
		assert(cached_rook_move_mask(A2,A2)==(A1|A3|A4|A5|A6|A7|A8|B2|C2|D2|E2|F2|G2|H2));
	}

	void test_cached_bishop_move_mask()
	{
		assert(cached_bishop_move_mask(A1,EMPTY_BOARD)==(B2|C3|D4|E5|F6|G7|H8));
		assert(cached_bishop_move_mask(D2,FULL_BOARD)==(C1|E1|C3|E3));
		assert(cached_bishop_move_mask(D2,(G5|B4))==(E3|F4|G5|E1|C1|C3|B4));
	}
	
	void test_bitboard()
	{
		test_bit_count();
		test_get_lowest_bit();
		test_floor_log2();
		test_simple_move_mask();
		test_move();
		test_bit_mask();		
		test_cached_king_move_mask();
		test_cached_knight_move_mask();
		test_cached_rook_move_mask();
		test_cached_bishop_move_mask();
	}

#endif
