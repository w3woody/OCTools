//
//  CalcLex.cpp
//  ParserCPP
//
//  Created by William Woody on 4/3/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#include "CalcLex.h"
#include "CalcParser.h"

int32_t CalcLex::lex()
{
	value.integer = 0;
	line = 0;

	switch (ix++) {
		default:
			return -1;

		case 1:
			column = 0;
			text = "11";
			value.integer = 11;
			return NUMBER;

		case 2:
			column = 3;
			text = "+";
			return '+';

		case 3:
			column = 5;
			text = "2";
			value.integer = 2;
			return NUMBER;

		case 4:
			column = 7;
			text = "*";
			return '*';

		case 5:
			column = 9;
			text = "3";
			value.integer = 3;
			return NUMBER;

		case 6:
			column = 11;
			text = "-";
			return '-';

		case 7:
			column = 13;
			text = "4";
			value.integer = 4;
			return NUMBER;

		case 8:
			column = 15;
			text = "/";
			return '/';

		case 9:
			column = 17;
			text = "(";
			return '(';

		case 10:
			column = 18;
			text = "1";
			value.integer = 1;
			return NUMBER;

		case 11:
			column = 20;
			text = "+";
			return '+';

		case 12:
			column = 22;
			text = "1";
			value.integer = 1;
			return NUMBER;

		case 13:
			column = 23;
			text = ")";
			return ')';
	}
}
