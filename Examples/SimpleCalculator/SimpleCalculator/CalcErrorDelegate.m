//
//  CalcErrorDelegate.m
//  SimpleCalculator
//
//  Created by William Woody on 3/31/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#import "CalcErrorDelegate.h"

@implementation CalcErrorDelegate

- (void)errorFrom:(CalcParser *)yacc line:(NSInteger)line
		column:(NSInteger)column filename:(NSString *)fname
		errorCode:(NSInteger)errorCode
		data:(NSDictionary<NSString *, id<NSObject>> *)data
{
	/* Just print that we have an error. */
	NSLog(@"Error");
}

@end
