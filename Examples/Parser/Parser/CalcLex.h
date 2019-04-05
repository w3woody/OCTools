//
//  CalcLex.h
//  Parser
//
//  Created by William Woody on 4/3/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "CalcParser.h"

@interface CalcLex : NSObject <OCLexInput>

@property (assign) NSInteger line;
@property (assign) NSInteger column;
@property (copy) NSString *filename;
@property (copy) NSString *text;
@property (copy) NSString *abort;

- (NSInteger)lex;
- (id<NSObject>)value;

@end
