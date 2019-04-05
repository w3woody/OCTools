//
//  CalcLex.m
//  Parser
//
//  Created by William Woody on 4/3/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#import "CalcLex.h"

@interface CalcLex ()
{
	NSInteger ix;
	NSInteger value;
}
@end

@implementation CalcLex

- (instancetype)init
{
	if (nil != (self = [super init])) {
		ix = 1;
	}
	return self;
}

- (NSInteger)lex
{
	value = 0;
	self.line = 0;
	self.filename = nil;
	self.abort = nil;

	switch (ix++) {
		default:
			return -1;

		case 1:
			self.column = 0;
			self.text = @"11";
			value = 11;
			return NUMBER;

		case 2:
			self.column = 3;
			self.text = @"+";
			return '+';

		case 3:
			self.column = 5;
			self.text = @"2";
			value = 2;
			return NUMBER;

		case 4:
			self.column = 7;
			self.text = @"*";
			return '*';

		case 5:
			self.column = 9;
			self.text = @"3";
			value = 3;
			return NUMBER;

		case 6:
			self.column = 11;
			self.text = @"-";
			return '-';

		case 7:
			self.column = 13;
			self.text = @"4";
			value = 4;
			return NUMBER;

		case 8:
			self.column = 15;
			self.text = @"/";
			return '/';

		case 9:
			self.column = 17;
			self.text = @"(";
			return '(';

		case 10:
			self.column = 18;
			self.text = @"1";
			value = 1;
			return NUMBER;

		case 11:
			self.column = 20;
			self.text = @"+";
			return '+';

		case 12:
			self.column = 22;
			self.text = @"1";
			value = 1;
			return NUMBER;

		case 13:
			self.column = 23;
			self.text = @")";
			return ')';
	}
}

- (id<NSObject>)value
{
	return @( value );
}

@end
