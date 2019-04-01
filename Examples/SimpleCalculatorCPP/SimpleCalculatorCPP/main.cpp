//
//  main.cpp
//  SimpleCalculatorCPP
//
//  Created by William Woody on 4/1/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#include <iostream>
#include "CalcStream.h"
#include "CalcLex.h"
#include "CalcParser.h"

int main(int argc, const char * argv[])
{
	// First, create an input stream to parse our equation.
	CalcStream stream("11 + 2 * 3 - 4 / (1 + 1)");

	// Next, create the lexer to tokenize the input stream into tokens
	CalcLex lex(&stream);

	// Third, create our parser to parse the token stream.
	CalcParser parser(&lex);

	// Fourth, actually run the parser.
	if (parser.parse()) {
		printf("Answer: %d\n",parser.result);
	} else {
		printf("Error.");
	}

	return 0;
}
