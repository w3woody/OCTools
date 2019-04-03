//
//  main.cpp
//  TokenizerCPP
//
//  Created by William Woody on 4/3/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#include <iostream>
#include "CalcStream.h"
#include "CalcLex.h"

int main(int argc, const char * argv[])
{
	// First, create an input stream to parse our equation.
	CalcStream stream("11 + 2 * 3 - 4 / (1 + 1)");

	// Next, create the lexer to tokenize the input stream into tokens
	CalcLex lex(&stream);

	// Now read all the tokens.
	int32_t tokenID;
	while (-1 != (tokenID = lex.lex())) {
		printf("%d:%d Token %d: '%s', value %d\n",
			lex.line,lex.column,tokenID,
			lex.text.c_str(),lex.value.integer);
	}
}
