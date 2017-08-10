//
//  main.cpp
//  ocyacctest
//
//  Created by William Woody on 7/25/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include <iostream>
#include "OCYaccParser.h"
#include "OCYaccTestLR0.h"
#include "OCYaccTestSLR.h"
#include "OCYaccTestLR1.h"
#include "OCYaccTestLALR.h"

int main(int argc, const char * argv[])
{
	OCLexer lexer;
	if (!lexer.OpenFile("test2.y")) {
		printf("Unable to open\n");
		exit(-1);
	}

	OCYaccParser parser;
	parser.ParseFile(lexer);

//	OCYaccTestLALR lalr;
//	lalr.Construct(parser);

	OCYaccTestLR1 lalr;
	lalr.Construct(parser);

//	OCYaccTestSLR lalr;
//	lalr.Construct(parser);

	printf("Done.\n");
	return 0;
}
