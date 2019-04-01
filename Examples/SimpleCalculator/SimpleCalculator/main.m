//
//  main.m
//  SimpleCalculator
//
//  Created by William Woody on 3/31/19.
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
	    CalcStream *stream = [[CalcStream alloc] initWithString:@"1 + 2 * 3"];

	    // Next, create the lexer to tokenize the input stream into tokens
	    CalcLex *lex = [[CalcLex alloc] initWithStream:stream];

		// Third, create our parser to parse the token stream.
	    CalcParser *parser = [[CalcParser alloc] initWithLexer:lex];

	    // Fourth, create a delegate to handle errors during parsing
		CalcErrorDelegate *delegate = [[CalcErrorDelegate alloc] init];
		parser.errorDelegate = delegate;

		// Fifth, actually run the parser.
		if ([parser parse]) {
			// YES indicates success. Get our results and present them.
			NSNumber *n = parser.result;
			NSLog(@"Answer: %d",n.intValue);

		} else {
			// NO indicate an error.
			NSLog(@"Error.");
		}
	    
	}
	return 0;
}
