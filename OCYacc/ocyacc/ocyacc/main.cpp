//
//  main.cpp
//  ocyacc
//
//  Created by William Woody on 7/24/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include <iostream>
#include <OCLexer.h>

int main(int argc, const char * argv[])
{
	OCLexer lexer;

	lexer.OpenFile("test.c");
	
	// insert code here...
	std::cout << "Hello, World!\n";
	return 0;
}
