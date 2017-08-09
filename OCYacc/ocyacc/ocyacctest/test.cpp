//
//  main.cpp
//  ocyacctest
//
//  Created by William Woody on 7/25/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include <iostream>
#include "OCYaccParser.h"
#include "OCYaccLR0.h"
#include "OCYaccSLR.h"
#include "OCYaccLR1.h"

int main(int argc, const char * argv[])
{
	OCLexer lexer;
	lexer.OpenFile("test5.y");

	OCYaccParser parser;
	parser.ParseFile(lexer);

//	OCYaccLALR lalr;
//	lalr.ConstructLALR(parser);

//	OCYaccLR1 lalr;
//	lalr.ConstructLR1(parser);

	OCYaccSLR lalr;
	lalr.Construct(parser);

	printf("Done.\n");
	return 0;
}
