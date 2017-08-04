//
//  OCYaccParser.cpp
//  ocyacc
//
//  Created by William Woody on 7/25/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCYaccParser.h"

/************************************************************************/
/*																		*/
/*	Parser Definitions													*/
/*																		*/
/************************************************************************/

/*	OCYaccParser::OCYaccParser
 *
 *		Construct this
 */

OCYaccParser::OCYaccParser()
{
}

/*	OCYaccParser::~OCYaccParser
 *
 *		Tear down
 */

OCYaccParser::~OCYaccParser()
{
}

/************************************************************************/
/*																		*/
/*	Parse file															*/
/*																		*/
/************************************************************************/

/*	OCYaccParser::ParseFile
 *
 *		parse the file into this parser, and construct an AST with this
 *	object as the root. If there was an error, this prints error information
 *	to the standard error stream and bails
 */

bool OCYaccParser::ParseFile(OCLexer &lex)
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

/************************************************************************/
/*																		*/
/*	Parse declarations													*/
/*																		*/
/************************************************************************/

/*	SkipToNextDeclaration
 *
 *		Used to skip to the next legal token in a declaration headdr
 */

void OCYaccParser::SkipToNextDeclaration(OCLexer &lex)
{
	int32_t sym;
	do {
		sym = lex.ReadToken();
	} while ((sym != OCTOKEN_SECTION) &&
			 (sym != '%') &&
			 (sym != OCTOKEN_OPENCODE) &&
			 (sym != -1));
	lex.PushBackToken();
}

/*	OCYaccParser::ParseDeclarations
 *
 *		Parse the declarations list. 
 */

bool OCYaccParser::ParseDeclarations(OCLexer &lex)
{
	for (;;) {
		int32_t sym = lex.ReadToken();

		if (sym == OCTOKEN_SECTION) return true;

		if (sym == '%') {

			/*
			 *	Parse the declaration.
			 */

			sym = lex.ReadToken();
			if (sym != OCTOKEN_TOKEN) {
				fprintf(stderr,"%s:%d Syntax error in grammar declarations section\n",lex.fFileName.c_str(),lex.fTokenLine);
				SkipToNextDeclaration(lex);
			} else {
				if (lex.fToken == "type") {
					/*
					 *	Parse the type declaration
					 */

					std::string typeName;
					sym = lex.ReadToken();
					if (sym != '<') {
						fprintf(stderr,"%s:%d Expect < after %%type\n",lex.fFileName.c_str(),lex.fTokenLine);

						SkipToNextDeclaration(lex);
						continue;
					}
					sym = lex.ReadToken();
					if (sym != OCTOKEN_TOKEN) {
						fprintf(stderr,"%s:%d Expect type declaration after %%type\n",lex.fFileName.c_str(),lex.fTokenLine);

						SkipToNextDeclaration(lex);
						continue;
					}
					sym = lex.ReadToken();
					if (sym != '>') {
						fprintf(stderr,"%s:%d Unclosed type, missing '>' after %%type\n",lex.fFileName.c_str(),lex.fTokenLine);

						SkipToNextDeclaration(lex);
						continue;
					}

					/*
					 *	Now grab tokens while we can
					 */

					for (;;) {
						sym = lex.ReadToken();
						if (sym == OCTOKEN_TOKEN) {
							// Store declaration
							symbolType[lex.fToken] = typeName;
						} else {
							lex.PushBackToken();
							break;
						}
					}

				} else if ((lex.fToken == "left") || (lex.fToken == "right") || (lex.fToken == "nonassoc") || (lex.fToken == "token")) {
					/*
					 *	Parse the association
					 */

					Assoc a;
					if (lex.fToken == "left") {
						a = Left;
					} else if (lex.fToken == "right") {
						a = Right;
					} else if (lex.fToken == "nonassoc") {
						a = NonAssoc;
					} else {
						a = None;
					}

					/*
					 *	Now grab tokens while we can
					 */

					for (;;) {
						sym = lex.ReadToken();
						if (sym == OCTOKEN_TOKEN) {
							// Store declaration
							terminalSymbol[lex.fToken] = a;
						} else {
							lex.PushBackToken();
							break;
						}
					}
				} else if (lex.fToken == "start") {
					/*
					 *	Parse the start token. The following is a single
					 *	token which we stash away
					 */

					sym = lex.ReadToken();
					if (sym != OCTOKEN_TOKEN) {
						fprintf(stderr,"%s:%d %%start must be followed by start symbol\n",lex.fFileName.c_str(),lex.fTokenLine);

						SkipToNextDeclaration(lex);
					} else {
						startSymbol = lex.fToken;
					}

				} else if ((lex.fToken == "global") || (lex.fToken == "local") || (lex.fToken == "header")) {
					/*
					 *	Parse the stream of tokens for code storage
					 */

					std::string codeType = lex.fToken;
					int sym = lex.ReadToken();
					if (sym == '{') {
						std::string code;

						/*
						 *	Parse the stream of tokens, copying them over.
						 */

						int cdepth = 1;

						for (;;) {
							sym = lex.ReadToken(false);
							if (sym == -1) {
								fprintf(stderr,"%s:%d Unexpected end of file reached in declarations section\n",lex.fFileName.c_str(),lex.fTokenLine);
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

						if (codeType == "header") {
							classHeader = code;
						} else if (codeType == "local") {
							classLocal = code;
						} else {
							classGlobal = code;
						}
					} else {
						fprintf(stderr,"%s:%d Syntax error in declarations\n",lex.fFileName.c_str(),lex.fTokenLine);
					}
				} else {
					fprintf(stderr,"%s:%d Unknown declaration %s in grammar declarations section\n",lex.fFileName.c_str(),lex.fTokenLine,lex.fToken.c_str());

					SkipToNextDeclaration(lex);
				}
			}
		} else if (sym == OCTOKEN_OPENCODE) {
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

		} else if (sym == -1) {
			fprintf(stderr,"%s:%d Unexpected end of file reached in grammar declarations section\n",lex.fFileName.c_str(),lex.fTokenLine);
			return false;

		} else {
			fprintf(stderr,"%s:%d Syntax error in grammar declarations section\n",lex.fFileName.c_str(),lex.fTokenLine);
			SkipToNextDeclaration(lex);
		}
	}
}

/************************************************************************/
/*																		*/
/*	Parse yacc-style tokens												*/
/*																		*/
/************************************************************************/

/*	SkipToNextRule
 *
 *		Scan until we hit a ';'
 */

void OCYaccParser::SkipToNextRule(OCLexer &lex)
{
	int32_t sym;
	do {
		sym = lex.ReadToken();
	} while ((sym != OCTOKEN_SECTION) &&
			 (sym != ';') &&
			 (sym != -1));
	if (sym != ';') {
		lex.PushBackToken();
	}
}

/*	PCParser::ParseCCDeclarations
 *
 *		Parse the compiler declarations. The format of each object is:
 *
 *		LINE: SYMBOL ':' SYMBOL_DECL_LIST ;
 *
 *		SYMBOL_DECL_LIST: SYMBOL_DECL ';'
 *						| SYMBOL_DECL_LIST '|' SYMBOL_DECL ';'
 *						;
 *
 *		SYMBOL_DECL: SYMBOL_LIST
 *				   | SYMBOL_LIST '{' code '}'
 *				   ;
 *
 *		SYMBOL_LIST: SYMBOL_LIST SYMBOL ;
 *
 *		SYMBOL : TOKEN
 *			   | STRING
 *			   ;
 */

bool OCYaccParser::ParseRules(OCLexer &lex)
{
	for (;;) {
		int32_t sym = lex.ReadToken();

		if (sym == OCTOKEN_SECTION) return true;
		if (sym == -1) return true;

		if (sym != OCTOKEN_TOKEN) {
			fprintf(stderr,"%s:%d Syntax error in declaration section\n",lex.fFileName.c_str(),lex.fTokenLine);
			SkipToNextRule(lex);
			continue;
		}

		/*
		 *	Store the symbol we're reading the declaration of
		 */

		std::string symName = lex.fToken;

		/*
		 *	Parse requred ':'
		 */

		if (':' != lex.ReadToken()) {
			fprintf(stderr,"%s:%d Expected ':' after symbol in declaration\n",lex.fFileName.c_str(),lex.fTokenLine);
			SkipToNextRule(lex);
			continue;
		}

		/*
		 *	Parse the full declaration
		 */

		SymbolDecl decl;
		for (;;) {
			SymbolInstance inst;

			for (;;) {
				/*
				 *	We're parsing a list of tokens.
				 */

				sym = lex.ReadToken();
				if (sym == OCTOKEN_TOKEN) {
					inst.tokenlist.push_back(lex.fToken);
				} else if (sym == OCTOKEN_CHAR) {
					inst.tokenlist.push_back(lex.fToken);
				} else {
					break;
				}
			}

			/*
			 *	See if we have some code
			 */

			if (sym == '{') {
				/*
				 *	Parse the stream of tokens, copying them over. We do no
				 *	token substitution here; that is handled later.
				 */

				int cdepth = 1;

				sym = lex.ReadToken(true);
				for (;;) {
					if (sym == -1) {
						fprintf(stderr,"%s:%d Unexpected end of file reached in grammar declarations section\n",lex.fFileName.c_str(),lex.fTokenLine);
						return false;
					} else if (sym == '{') {
						++cdepth;
						inst.code += lex.fToken;
					} else if (sym == '}') {
						--cdepth;
						if (cdepth <= 0) break;
						inst.code += lex.fToken;
					} else {
						inst.code += lex.fToken;
					}
					sym = lex.ReadToken(false);
				}

				// Skip forward after {...} code block
				sym = lex.ReadToken();
			}

			decl.declarations.push_back(inst);

			/*
			 *	Now bounce to next token
			 */

			if (sym == ';') {
				break;
			} else if (sym == -1) {
				fprintf(stderr,"%s:%d Unexpected end of file reached in grammar declarations section\n",lex.fFileName.c_str(),lex.fTokenLine);
				return false;
			} else if (sym != '|') {
				fprintf(stderr,"%s:%d Syntax error: expected ';' or '|'\n",lex.fFileName.c_str(),lex.fTokenLine);
				SkipToNextRule(lex);
				break;
			}
		}

		/*
		 *	Store declaration
		 */

		symbols[symName] = decl;
	}
}

