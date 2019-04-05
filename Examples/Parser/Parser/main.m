//
//  main.m
//  Parser
//
//  Created by William Woody on 4/3/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CalcParser.h"
#import "CalcLex.h"

int main(int argc, const char * argv[]) {
	@autoreleasepool {

	    // Next, create the lexer to tokenize the input stream into tokens
	    CalcLex *lex = [[CalcLex alloc] init];

		// Third, create our parser to parse the token stream.
	    CalcParser *parser = [[CalcParser alloc] initWithLexer:lex];

		// Fourth, actually run the parser.
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
