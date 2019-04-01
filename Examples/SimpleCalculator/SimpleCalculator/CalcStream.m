//
//  CalcStream.m
//  SimpleCalculator
//
//  Created by William Woody on 3/31/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#import "CalcStream.h"

@interface CalcStream ()
{
	NSString *input;
	NSInteger pos;
}
@end

@implementation CalcStream

- (instancetype)initWithString:(NSString *)str
{
	if (nil != (self = [super init])) {
		input = str;
		pos = 0;
	}
	return self;
}

- (int)peekByte
{
	if (pos >= input.length) return -1;
	return [input characterAtIndex:pos];
}

- (int)readByte
{
	if (pos >= input.length) return -1;
	return [input characterAtIndex:pos++];
}

@end
