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
#include "OCYaccLR1.h"

void test()
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
}

void test2()
{
	OCLexer lexer;
	if (!lexer.OpenFile("/Users/woody/Development/OCTools/OCYacc/ocyacc/ocyacctest/test.y")) {
		printf("Unable to open\n");
		exit(-1);
	}

	OCYaccParser parser;
	parser.ParseFile(lexer);

	OCYaccLR1 lalr;
	lalr.Construct(parser);

	printf("Done.\n");
}

int main(int argc, const char * argv[])
{
	test2();
	return 0;
}
