#include "main.h"
#include <assert.h>
#include <string.h>

#define NULL_STRING ""

#ifdef DEBUG

	void test_col()
	{
		assert(COL(0)==0);
		assert(COL(7)==7);
		assert(COL(8)==0);
		assert(COL(63)==7);
	}
	
	void test_row()
	{
		assert(ROW(0)==0);
		assert(ROW(63)==7);
		assert(ROW(16)==2);
	}

	void test_get_square_number()
	{
		assert(get_square_number("a1")==0);
		assert(get_square_number("h8")==63);
		assert(get_square_number("h1")==7);
		assert(get_square_number("banana") == -1);
		assert(get_square_number("") == -1);
	}

	void test_get_square_test()
	{
		assert(strcmp(get_square_text(0),"a1")==0);
		assert(strcmp(get_square_text(7),"h1")==0);
		assert(strcmp(get_square_text(63),"h8")==0);
		assert(strcmp(get_square_text(8),"a2")==0);
		assert(strcmp(get_square_text(-50),NULL_STRING)==0);
	}

	void test_main()
	{
		test_col();
		test_row();
		test_get_square_number();
		test_get_square_test();
	}
#endif
