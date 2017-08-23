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
	 *		http://dinosaur.compilertools.net/yacc/index.html
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

/*	OCYaccParser::ReadType
 *
 *		Read the next type in the list. Assumes we've already read the '<'
 */

std::string OCYaccParser::ReadType(OCLexer &lex)
{
	int32_t token;
	std::string ret;
	int count = 1;

	for (;;) {
		token = lex.ReadToken(false);
		if (token == -1) return ret;
		else if (token == '<') {
			++count;
			ret += lex.fToken;
		} else if (token == '>') {
			--count;
			if (count == 0) return ret;
			ret += lex.fToken;
		} else {
			ret += lex.fToken;
		}
	}
}

/*	OCYaccParser::ParseDeclarations
 *
 *		Parse the declarations list. 
 */

bool OCYaccParser::ParseDeclarations(OCLexer &lex)
{
	bool retVal = true;
	uint16_t precIndex = 0;

	for (;;) {
		int32_t sym = lex.ReadToken();

		if (sym == OCTOKEN_SECTION) return retVal;

		if (sym == '%') {

			/*
			 *	Parse the declaration.
			 */

			sym = lex.ReadToken();
			if (sym != OCTOKEN_TOKEN) {
				fprintf(stderr,"%s:%d Syntax error in grammar declarations section; %% not followed by operation name\n",lex.fFileName.c_str(),lex.fTokenLine);
				SkipToNextDeclaration(lex);
				retVal = false;
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
						retVal = false;
						continue;
					}

					typeName = ReadType(lex);

					/*
					 *	Now grab tokens while we can
					 */

					for (;;) {
						sym = lex.ReadToken();
						if (sym == OCTOKEN_TOKEN) {
							// Store declaration
							if (symbolType.find(lex.fToken) != symbolType.end()) {
								fprintf(stderr,"%s:%d Symbol type for %s already defined\n",lex.fFileName.c_str(),lex.fTokenLine,lex.fToken.c_str());
								retVal = false;
							}

							symbolType[lex.fToken] = typeName;
						} else {
							lex.PushBackToken();
							break;
						}
					}

				} else if (lex.fToken == "token") {
					/*
					 *	Define the token. First, get the type.
					 */

					sym = lex.ReadToken();
					std::string type;
					bool hasType = false;

					if (sym == '<') {
						hasType = true;
						type = ReadType(lex);
					} else {
						lex.PushBackToken();
					}

					/*
					 *	Now define the tokens, updating the symbol name if
					 *	the token is present
					 */

					for (;;) {
						sym = lex.ReadToken();
						if (sym == OCTOKEN_TOKEN) {
							tokens.insert(lex.fToken);
							if (hasType) {
								if (symbolType.find(lex.fToken) != symbolType.end()) {
									fprintf(stderr,"%s:%d Symbol type for %s already defined\n",lex.fFileName.c_str(),lex.fTokenLine,lex.fToken.c_str());
									retVal = false;
								}
								symbolType[lex.fToken] = type;
							}
						} else {
							lex.PushBackToken();
							break;
						}
					}

				} else if ((lex.fToken == "left") || (lex.fToken == "right") || (lex.fToken == "nonassoc")) {
					/*
					 *	Parse the association and track the precedence.
					 *	Note that two or more tokens on the same %left or
					 *	%right have the same precedence. Note that we
					 *	treat %token differently than YACC; %left implicitly
					 *	defines %token, though it does not permit a type.
					 */

					Precedence a;
					if (lex.fToken == "left") {
						a.assoc = Left;
						a.prec = ++precIndex;
					} else if (lex.fToken == "right") {
						a.assoc = Right;
						a.prec = ++precIndex;
					} else {
						a.assoc = NonAssoc;
						a.prec = ++precIndex;
					}

					/*
					 *	Grab the optional type. Only valid on %token
					 */

					std::string type;
					bool hasType = false;

					sym = lex.ReadToken();
					if (sym == '<') {
						hasType = true;
						type = ReadType(lex);
					} else {
						lex.PushBackToken();
					}

					/*
					 *	Now grab tokens while we can
					 */

					for (;;) {
						sym = lex.ReadToken();
						if ((sym == OCTOKEN_TOKEN) || (sym == OCTOKEN_CHAR)) {
							// Store declaration
							if (sym == OCTOKEN_TOKEN) {
								tokens.insert(lex.fToken);
							}

							// Store symbol type
							if (hasType) {
								if (symbolType.find(lex.fToken) != symbolType.end()) {
									fprintf(stderr,"%s:%d Symbol type for %s already defined\n",lex.fFileName.c_str(),lex.fTokenLine,lex.fToken.c_str());
									retVal = false;
								}
								symbolType[lex.fToken] = type;
							}

							// Store precedence
							std::map<std::string,Precedence>::iterator m;
							m = precedence.find(lex.fToken);
							if (m == precedence.end()) {
								precedence[lex.fToken] = a;
							} else {
								fprintf(stderr,"%s:%d token %s precedence already set\n",lex.fFileName.c_str(),lex.fTokenLine,lex.fToken.c_str());
								retVal = false;
							}
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

						retVal = false;
						SkipToNextDeclaration(lex);
					} else {
						startSymbol = lex.fToken;
					}

				} else if ((lex.fToken == "global") || (lex.fToken == "local") || (lex.fToken == "header") || (lex.fToken == "init")) {
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
						} else if (codeType == "init") {
							classInit = code;
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
					retVal = false;
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
			fprintf(stderr,"%s:%d Syntax error in grammar declarations section: unknown token %s\n",lex.fFileName.c_str(),lex.fTokenLine,lex.fToken.c_str());
			SkipToNextDeclaration(lex);
			retVal = false;
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
	bool retVal = true;

	for (;;) {
		int32_t sym = lex.ReadToken();

		if (sym == OCTOKEN_SECTION) return retVal;
		if (sym == -1) return retVal;

		if (sym != OCTOKEN_TOKEN) {
			fprintf(stderr,"%s:%d Syntax error in declaration section\n",lex.fFileName.c_str(),lex.fTokenLine);
			SkipToNextRule(lex);
			continue;
		}

		/*
		 *	Store the symbol we're reading the declaration of
		 */

		std::string symName = lex.fToken;
		if (symName == "error") {
			fprintf(stderr,"%s:%d error is a reserved word and cannot be used as a production rule name\n",lex.fFileName.c_str(),lex.fTokenLine);
			retVal = false;
		}
		if (symbols.find(symName) != symbols.end()) {
			fprintf(stderr,"%s:%d Duplicate rule name %s\n",lex.fFileName.c_str(),lex.fTokenLine,symName.c_str());
			retVal = false;
		}

		/*
		 *	Note the position of this rule
		 */

		FilePos pos;
		pos.line = lex.fTokenLine;
		pos.col = lex.fTokenColumn;
		pos.file = lex.fFileName;

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
		decl.pos = pos;

		for (;;) {
			SymbolInstance inst;
			bool hasPrec = false;
			bool hasFilePos = false;

			/*
			 *	Clear rule precedence
			 */
			
			inst.precedence.prec = 0;

			for (;;) {
				/*
				 *	We're parsing a list of tokens.
				 */

				std::string t;
				sym = lex.ReadToken();
				if ((sym == OCTOKEN_TOKEN) || (sym == OCTOKEN_CHAR)) {
					t = lex.fToken;
				} else {
					break;
				}

				inst.tokenlist.push_back(t);

				/*
				 *	Track the position of the first symbol in this rule instance
				 */

				if (!hasFilePos) {
					hasFilePos = true;

					pos.line = lex.fTokenLine;
					pos.col = lex.fTokenColumn;
					pos.file = lex.fFileName;

					inst.pos = pos;
				}

				/*
				 *	Precedence is actually associated with rules. If we have
				 *	rules with different precedence levels, warn the user
				 */

				if (precedence.find(t) != precedence.end()) {
					Precedence p = precedence[t];

					if (hasPrec && ((inst.precedence.prec != p.prec) || (inst.precedence.assoc != p.assoc))) {
						fprintf(stderr,"%s:%d Rule declaration has multiple tokens with different precedence levels\n",lex.fFileName.c_str(),lex.fTokenLine);
						retVal = false;
					} else {
						hasPrec = true;
						inst.precedence = p;
					}
				}
			}

			if (inst.tokenlist.size() == 0) {
				fprintf(stderr,"%s:%d Symbol %s is empty. OCYacc does not support empty symbols\n",lex.fFileName.c_str(),lex.fTokenLine,symName.c_str());
			}

			/*
			 *	See if we have a precedence assigned to this rule
			 */

			if (sym == '%') {
				sym = lex.ReadToken();
				if ((sym != OCTOKEN_TOKEN) || (lex.fToken != "prec")) {
					fprintf(stderr,"%s:%d %% not followed by 'prec'\n",lex.fFileName.c_str(),lex.fTokenLine);
					SkipToNextRule(lex);
					return false;
				}

				sym = lex.ReadToken();
				if ((sym != OCTOKEN_TOKEN) && (sym != OCTOKEN_CHAR)) {
					fprintf(stderr,"%s:%d token or character expected after %%prec",lex.fFileName.c_str(),lex.fTokenLine);
					SkipToNextRule(lex);
					return false;
				}

				if (precedence.find(lex.fToken) == precedence.end()) {
					fprintf(stderr,"%s:%d %s does not have precedence defined",lex.fFileName.c_str(),lex.fTokenLine,lex.fToken.c_str());
					SkipToNextRule(lex);
					return false;
				}

				/*
				 *	This overrides precedence
				 */

				inst.precedence = precedence[lex.fToken];
				sym = lex.ReadToken();
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
				retVal = false;
				break;
			}
		}

		/*
		 *	Store declaration.
		 */

		symbols[symName] = decl;
	}
}

