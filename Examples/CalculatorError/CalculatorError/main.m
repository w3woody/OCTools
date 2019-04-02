//
//  main.m
//  Calculator
//
//  Created by William Woody on 4/1/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "CalcStream.h"
#import "CalcLex.h"
#import "CalcParser.h"
#import "CalcErrorDelegate.h"

int main(int argc, const char * argv[]) {
	@autoreleasepool {

		// First, create an input stream to parse our equation.
	    CalcStream *stream = [[CalcStream alloc] initWithString:@"1+2; 3+4; a+c; 5+4;"];

	    // Next, create the lexer to tokenize the input stream into tokens
	    CalcLex *lex = [[CalcLex alloc] initWithStream:stream];

		// Third, create our parser to parse the token stream.
	    CalcParser *parser = [[CalcParser alloc] initWithLexer:lex];

	    // Fourth, add delegate for error handling
	    CalcErrorDelegate *del = [[CalcErrorDelegate alloc] init];
	    parser.errorDelegate = del;

		// Fifth, actually run the parser.
		if ([parser parse]) {
			// YES indicates success. Get our results and present them.
			for (NSNumber *n in parser.result) {
				NSLog(@"%d",n.intValue);
			}

		} else {
			// NO indicate an error.
			NSLog(@"Error.");
		}

	}
	return 0;
}
