//
//  main.m
//  TestLex
//
//  Created by William Woody on 7/30/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OCLexTest.h"

@interface TestReader : NSObject <OCFileInput>
@property (assign) NSInteger pos;
@property (copy) NSString *test;
@end

@implementation TestReader
- (int)readByte
{
	if (self.pos >= self.test.length) return -1;
	return [self.test characterAtIndex:_pos++];
}
@end


int main(int argc, const char * argv[]) {
	@autoreleasepool {
	    // insert code here...

		TestReader *r = [[TestReader alloc] init];
		r.test = @"132 + 3.14 * 5";

		OCLexTest *lexer = [[OCLexTest alloc] initWithStream:r];
		NSInteger ident;

		while (-1 != (ident = [lexer lex])) {
			NSLog(@"-> %ld %@\n",(long)ident,lexer.text);
		}
	}
	return 0;
}
