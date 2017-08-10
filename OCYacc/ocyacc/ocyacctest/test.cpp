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
#include "OCYaccLALR.h"

int main(int argc, const char * argv[])
{
	OCLexer lexer;
	if (!lexer.OpenFile("test2.y")) {
		printf("Unable to open\n");
		exit(-1);
	}

	OCYaccParser parser;
	parser.ParseFile(lexer);

//	OCYaccLALR lalr;
//	lalr.Construct(parser);

	OCYaccLR1 lalr;
	lalr.Construct(parser);

//	OCYaccSLR lalr;
//	lalr.Construct(parser);

	printf("Done.\n");
	return 0;
}
