//
//  OCLexParser.cpp
//  oclex
//
//  Created by William Woody on 7/24/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCLexParser.h"

/************************************************************************/
/*																		*/
/*	Parser Definitions													*/
/*																		*/
/************************************************************************/

/*	OCLexParser::OCLexParser
 *
 *		Construct this
 */

OCLexParser::OCLexParser()
{
}

/*	OCLexParser::~OCLexParser
 *
 *		Tear down
 */

OCLexParser::~OCLexParser()
{
}

/************************************************************************/
/*																		*/
/*	Parse file															*/
/*																		*/
/************************************************************************/

/*	OCLexParser::ParseFile
 *
 *		parse the file into this parser, and construct an AST with this
 *	object as the root. If there was an error, this prints error information
 *	to the standard error stream and bails
 */

bool OCLexParser::ParseFile(OCLexer &lex)
{
	/*
	 *	Parse the lex file format. The format is the same as used by flex
	 *	as outlined in the following document:
	 *
	 *		http://dinosaur.compilertools.net/flex/flex_6.html
	 *
	 *	In summary the document is
	 *
	 *		definitions
	 *		%%
	 *		rules
	 *		%%
	 *		user code
	 *
	 *	definitions has the format:
	 *
	 *		definition regex
	 *		or %{...%}
	 *
	 *	rules:
	 *
	 *		pattern { action }
	 *
	 *	user code
	 */

	return false;
}

