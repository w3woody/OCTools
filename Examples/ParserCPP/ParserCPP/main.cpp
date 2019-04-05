//
//  main.cpp
//  ParserCPP
//
//  Created by William Woody on 4/3/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#include <iostream>
#include "CalcLex.h"
#include "CalcParser.h"

int main(int argc, const char * argv[])
{
	// Create our custom lexer
	CalcLex lex;

	// Construct our parser
	CalcParser parser(&lex);

	// Parse
	if (parser.parse()) {
		printf("Answer: %d\n",parser.result);
	} else {
		printf("Error.\n");
	}
	return 0;
}
