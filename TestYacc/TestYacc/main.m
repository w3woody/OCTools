//
//  main.m
//  TestYacc
//
//  Created by William Woody on 8/3/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OCYaccTest.h"

/************************************************************************/
/*                                                                      */
/*  Test Lex environment												*/
/*                                                                      */
/************************************************************************/


static int GData[] = {
	TOKEN, '=', NUMBER, '*', NUMBER, '+', NUMBER, ';'
};
static NSString *GTokens[] = {
	@"A", @"=", @"3", @"*", @"4", @"+", @"5", @";"
};

/*
 *	OCLexTest
 *
 *		test lexer
 */

@interface OCLexTest : NSObject <OCLexInput>
{
	NSInteger index;
}

- (NSInteger)lex;

@property (assign) NSInteger line;
@property (assign) NSInteger column;
@property (copy) NSString *filename;
@property (copy) NSString *text;
@property (copy) NSString *abort;
@property (strong) id<NSObject> value;

@end

@implementation OCLexTest
- (instancetype)init
{
	if (nil != (self = [super init])) {
		index = 0;
	}
	return self;
}

- (NSInteger)lex
{
	if (index >= (sizeof(GData)/sizeof(int))) return -1;
	self.text = GTokens[index];
	return GData[index++];
}
@end

/************************************************************************/
/*                                                                      */
/*  Test Error handler													*/
/*                                                                      */
/************************************************************************/

@interface OCError: NSObject<OCYaccTestError>
@end

@implementation OCError
- (void)errorFrom:(OCYaccTest *)yacc line:(NSInteger)line column:(NSInteger)column
		filename:(NSString *)fname errorMessage:(NSString *)error
{
	NSLog(@"%@",error);
}
@end


/*
 *	Test
 */
int main(int argc, const char * argv[])
{
	@autoreleasepool {
		OCYaccTest *y = [[OCYaccTest alloc] initWithLexer:[[OCLexTest alloc] init]];

		[y parse];

	    NSLog(@"Done.");
	}
	return 0;
}
