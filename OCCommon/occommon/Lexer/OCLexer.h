//
//  OCLexer.h
//  OCCommon
//
//  Created by William Woody on 7/22/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCLexer_h
#define OCLexer_h

#include <stdio.h>
#include <string>

/************************************************************************/
/*																		*/
/*	Lex Tokenizer														*/
/*																		*/
/************************************************************************/

#define PCTOKEN_TOKEN		0x10000
#define PCTOKEN_STRING		0x10001
#define PCTOKEN_NUMBER		0x10002
#define PCTOKEN_CHAR		0x10003

#define PCTOKEN_SECTION		0x10004		/* %% */
#define PCTOKEN_OPENCODE	0x10005		/* %{ */
#define PCTOKEN_CLOSECODE	0x10006		/* }% */
#define PCTOKEN_PARAMETER	0x10007		/* $nnn, n in [0,9] */
#define PCTOKEN_CURPARAM	0x10008		/* $$ */

/*	OCLexer
 *
 *		Lexical analysis of the input grammar. This contains code for
 *	parsing tokens as well as code for parsing REGEX expressions which
 *	are not easily handled with the standard tokenizer
 */

class OCLexer
{
	public:
		OCLexer();
		~OCLexer();

		/*
		 *	Open file
		 */

		bool OpenFile(std::string filename);

		/*
		 *	Tokenizer
		 */

		int32_t ReadToken(bool skipSpaces = true);
		void PushBackToken()
			{
				fPushToken = true;
			}

		/*
		 *	Regex parser
		 */

		std::string ReadRegEx();

		/*
		 *	File token locations
		 */

		std::string fToken;
		int32_t fTokenID;

		std::string fFileName;
		uint32_t fTokenLine;
		uint32_t fTokenColumn;

	private:
		FILE *fFile;
		uint32_t fLine;
		uint32_t fColumn;

		// Phase 1
		int32_t fPushBackChar;
		int32_t fPushBack[8];

		int32_t ReadRawChar();
		void PushChar(int32_t ch);

		// Phase 2: comments
		int32_t ReadChar();

		// Phase 3: Tokenizer
		bool fPushToken;

		void ReadNumberBody(void);
		void ReadStringBody(int squote);
};


#endif /* OCLexer_hpp */
