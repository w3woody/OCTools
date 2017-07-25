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

	if (!ParseDeclarations(lex)) return false;
	if (!ParseRules(lex)) return false;

	/*
	 *	The bottom is simply code; grab until we hit EOF
	 */

	while (-1 != lex.ReadToken(false)) {
		endCode += lex.fToken;
	}

	return true;
}

/*	OCLexParser::ParseDeclarations
 *
 *		parse declarations
 */

bool OCLexParser::ParseDeclarations(OCLexer &lex)
{
	bool inError = false;

	for (;;) {
		int32_t sym = lex.ReadToken();

		if (sym == OCTOKEN_SECTION) return true;

		if (sym == OCTOKEN_OPENCODE) {
			inError = false;

			/*
			 *	Parse the code
			 */

			std::string codeStr;

			for (;;) {
				sym = lex.ReadToken(false);
				if (sym == -1) {
					fprintf(stderr,"%s:%d Unexpected end of file reached in grammar declarations section\n",lex.fFileName.c_str(),lex.fTokenLine);
					return false;
				} else if (sym == OCTOKEN_CLOSECODE) {
					break;
				} else {
					codeStr += lex.fToken;
				}
			}

			declCode += codeStr;
		} else if (sym == OCTOKEN_TOKEN) {
			/*
			 *	Declaration
			 */

			std::string def = lex.fToken;
			std::string rule = lex.ReadRegEx();

			if (definitions.end() != definitions.find(def)) {
				fprintf(stderr,"%s:%d %s already defined\n",lex.fFileName.c_str(),lex.fTokenLine,def.c_str());
			} else {
				inError = false;
				definitions[def] = rule;
			}
		} else {
			if (!inError) {
				inError = true;
				fprintf(stderr,"%s:%d Syntax error in declarations\n",lex.fFileName.c_str(),lex.fTokenLine);
			}
		}
	}
}



/*	OCLexParser::ParseRules
 *
 *		parse rules. Each rule is of the form regex code.
 */

bool OCLexParser::ParseRules(OCLexer &lex)
{
	bool inError = false;

	for (;;) {
		std::string regex = lex.ReadRegEx();
		if (regex == "%%") return true;

		int sym = lex.ReadToken();
		if (sym == '{') {
			inError = false;
			std::string code;

			/*
			 *	Parse the stream of tokens, copying them over.
			 */

			int cdepth = 1;

			for (;;) {
				sym = lex.ReadToken(false);
				if (sym == -1) {
					fprintf(stderr,"%s:%d Unexpected end of file reached in grammar declarations section\n",lex.fFileName.c_str(),lex.fTokenLine);
					return false;
				} else if (sym == '{') {
					++cdepth;
					code += lex.fToken;
				} else if (sym == '}') {
					--cdepth;
					if (cdepth <= 0) break;
					code += lex.fToken;
				} else {
					code += lex.fToken;
				}
			}

			std::pair<std::string,std::string> p(regex,code);
			rules.push_back(p);

		} else {
			if (!inError) {
				inError = true;
				fprintf(stderr,"%s:%d Syntax error in declarations\n",lex.fFileName.c_str(),lex.fTokenLine);
			}
		}
	}
}

