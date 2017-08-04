/*	OCYacc.h
 *
 *		This file was automatically generated by OCYacc, part of the OCTools
 *	suite available at:
 *
 *		https://github.com/w3woody/OCTools
 */

#import <Foundation/Foundation.h>

/*
 *	Class forwards
 */

@class OCYacc;

/*
 *	Yacc constants
 */

/* ### INSERT HERE */
#define YACCTOKEN_ID			0x10000

/*	OCLexInput
 *
 *		The protocol for our lex reader file that the lex stream must
 *	provide. This is the same as the protocol generated as part of the OCYacc
 *	output, and allows us to glue the Lexer and Parser together.
 */

#ifndef OCLexInputProtocol
#define OCLexInputProtocol

@protocol OCLexInput <NSObject>
- (NSInteger)line;
- (NSInteger)column;
- (NSString *)filename;
- (NSString *)text;
- (NSString *)abort;

- (NSInteger)lex;

- (id<NSObject>)value;
@end

#endif

/*	OCYaccError		-- Name varies with class
 *
 *		The protocol for our parser for handling errors. As errors take place,
 *	we invoke the method so the error can be recorded and displayed to the
 *	user.
 */

@protocol OCYaccError <NSObject>
- (void)errorFrom:(OCYacc *)yacc line:(NSInteger)line column:(NSInteger)column
		filename:(NSString *)fname token:(NSString *)text
		errorMessage:(NSString *)error;
@end


/*	OCYacc
 *
 *		The generated parser
 */

@interface OCYacc : NSObject

/*
 *	Error processing
 */

@property (weak) id<OCYaccError> errorDelegate;

/*
 *	External interfaces
 */

- (instancetype)initWithLexer:(id<OCLexInput>)lexer;

/*
 *	Run parser. If any errors are recorded during parsing, this returns
 *	NO.
 */

- (BOOL)parse;

/*	### Insert other header declarations here */

@end
