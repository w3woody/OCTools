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
#include "CalcParserError.h"

int main(int argc, const char * argv[])
{
	// First, create an input stream to parse our equation.
	CalcStream stream("1+2; 3+4; a+c; 5+4; 7+$; 9");

	// Next, create the lexer to tokenize the input stream into tokens
	CalcLex lex(&stream);

	// Third, create our parser to parse the token stream.
	CalcParserError parser(&lex);

	// Fourth, actually run the parser.
	if (parser.parse()) {
		int i,len = (int)parser.result.size();
		for (i = 0; i < len; ++i) {
			printf("%d\n",parser.result[i]);
		}
	} else {
		printf("Error.\n");
	}

	return 0;
}
