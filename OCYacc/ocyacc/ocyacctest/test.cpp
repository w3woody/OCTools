//
//  main.cpp
//  ocyacctest
//
//  Created by William Woody on 7/25/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include <iostream>
#include "OCYaccParser.h"

int main(int argc, const char * argv[])
{
	OCLexer lexer;
	lexer.OpenFile("test.yacc");

	OCYaccParser parser;
	parser.ParseFile(lexer);

	printf("Done.\n");
	return 0;
}
