//
//  main.m
//  Tokenizer
//
//  Created by William Woody on 4/3/19.
//  Copyright © 2019 William Woody. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "CalcLex.h"
#include "CalcStream.h"

int main(int argc, const char * argv[]) {
	@autoreleasepool {
		// First, create an input stream to parse our equation.
	    CalcStream *stream = [[CalcStream alloc] initWithString:@"11 + 2 * 3 - 4 / (1 + 1)"];

	    // Next, create the lexer to tokenize the input stream into tokens
	    CalcLex *lex = [[CalcLex alloc] initWithStream:stream];
		
	    // Now pull tokens until we reach the end of file marker
	    NSInteger tokenID;
	    while (-1 != (tokenID = [lex lex])) {
	    	printf("%ld:%ld Token %ld: '%s', value %d\n",
				lex.line,lex.column,tokenID,
				lex.text.UTF8String,((NSNumber *)lex.value).intValue);
		}
	}
	return 0;
}
