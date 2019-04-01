//
//  CalcErrorDelegate.m
//  Calculator
//
//  Created by William Woody on 4/1/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#import "CalcErrorDelegate.h"

@implementation CalcErrorDelegate

- (void)errorFrom:(CalcParser *)yacc line:(NSInteger)line
		column:(NSInteger)column filename:(NSString *)fname
		errorCode:(NSInteger)errorCode
		data:(NSDictionary<NSString *, id<NSObject>> *)data
{
	NSString *err;

	switch (errorCode) {
		case ERROR_SYNTAX:
			err = @"Syntax Error";
			break;
		case ERROR_MISSINGTOKEN:
			err = [NSString stringWithFormat:@"Missing token (%@)",data[@"token"]];
			break;
		case ERROR_MISSINGTOKENS:
			err = [NSString stringWithFormat:@"Missing tokens (%@)",[data[@"tokens"] description]];
			break;
		default:
			err = @"Unknown error";
			break;
	}

	NSLog(@"%@ at %d:%d",err,(int)line,(int)column);
}

@end
