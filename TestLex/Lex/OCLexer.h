//
//  OCLexer.h
//  TestLex
//
//  Created by William Woody on 7/30/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#import <Foundation/Foundation.h>

/*	OCFileInput
 *
 *		The input file stream must correspond to this interface to read
 *	the contents of a file or data object. The byte returned is from 0 to 255,
 *	and EOF is marked with -1.
 */

#ifndef OCFileInputProtocol
#define OCFileInputProtocol

@protocol OCFileInput <NSObject>
- (int)readByte;
@end

#endif

/*
 *	This is a sample hand-built lexer, used to help figure out the code to
 *	generate the the OCLex/OCYacc code
 */

@interface OCLexer : NSObject

/*
 *	External interfaces
 */

- (instancetype)initWithStream:(id<OCFileInput>)file;

/*
 *	Current reader state
 */

@property (assign) NSInteger line;		// line of last read token
@property (assign) NSInteger column;	// column of last read token
@property (copy)   NSString *filename;	// marked filename (if provided)
@property (copy)   NSString *text;		// string of last read token
@property (assign) NSString *abort;		// Set to abort string if problem

- (NSInteger)lex;						// Method to read next token

@end
