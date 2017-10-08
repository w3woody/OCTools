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
	 *		%class { ... }
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
		} else if (sym == '%') {
			/*
			 *	Class declarations
			 */

			sym = lex.ReadToken();
			if ((sym == OCTOKEN_TOKEN) && (lex.fToken == "start")) {
				/*
				 *	Parse state list
				 */

				for (;;) {
					sym = lex.ReadToken();
					if (sym == -1) {
						fprintf(stderr,"%s:%d Unexpected end of file reached in grammar declarations section\n",lex.fFileName.c_str(),lex.fTokenLine);
						return false;
					} else if (sym == OCTOKEN_TOKEN) {
						ruleStates.push_back(lex.fToken);
					} else {
						lex.PushBackToken();
						break;
					}
				}
			} else if ((sym != OCTOKEN_TOKEN) ||
					((lex.fToken != "global") && (lex.fToken != "local") &&
					(lex.fToken != "header") && (lex.fToken != "init"))) {
				fprintf(stderr,"%s:%d Syntax error in declarations\n",lex.fFileName.c_str(),lex.fTokenLine);

			} else {
				/*
				 *	Class definitions
				 */

				std::string codeType = lex.fToken;

				int sym = lex.ReadToken();
				if (sym == '{') {
					inError = false;
					std::string code;

					/*
					 *	Parse the stream of tokens, copying them over.
					 */

					int cdepth = 1;

					sym = lex.ReadToken(true);
					for (;;) {
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
						sym = lex.ReadToken(false);
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
					inError = true;
					fprintf(stderr,"%s:%d Syntax error in declarations\n",lex.fFileName.c_str(),lex.fTokenLine);
				}
			}
		} else {
			if (!inError) {
				inError = true;
				fprintf(stderr,"%s:%d Syntax error in declarations\n",lex.fFileName.c_str(),lex.fTokenLine);
			}
		}
	}
}

static bool RuleStartsLine(std::string const &r)
{
	const char *str = r.c_str();
	if (*str == '^') return true;
	return false;
}

static bool RuleEndsLine(std::string const &r)
{
	const char *str = r.c_str();
	if (str[r.size()-1] != '$') return false;

	// Determine if '$' is not escaped. This requires scanning the string from
	// the front.
	bool endFlag = false;
	const char *ptr = str;
	while (*ptr != 0) {
		if ((*ptr == '$') && (ptr[1] == 0)) {
			endFlag = true;
			break;
		}
		if (*ptr == '\\') {
			++ptr;
			if (*ptr) ++ptr;
		} else {
			++ptr;
		}
	}

	return endFlag;
}

static std::string RuleStartState(std::string &r)
{
	const char *str = r.c_str();
	const char *ptr = str;

	/*
	 *	Run forward, only if we have a token in <> brackets
	 */

	if (*ptr++ != '<') return "";
	if (!isalpha(*ptr++)) return "";
	for (;;) {
		if (*ptr == '>') break;
		if (!isalnum(*ptr++)) return "";
	}

	std::string retval(str,1,ptr-str-1);	// token between < and >
	r.erase(0,ptr-str+1);					// erase <token>

	return retval;
}

/*	OCLexParser::ParseRules
 *
 *		parse rules. Each rule is of the form regex code.
 */

bool OCLexParser::ParseRules(OCLexer &lex)
{
	bool inError = false;
	bool atStart,atEnd;

	for (;;) {
		std::string regex = lex.ReadRegEx();
		if ((regex == "%%") || (regex == "")) return true;  // empty string: EOF

		// See if it has a <SYMBOL> prefix. If so, track prefix.
		std::string state = RuleStartState(regex);
		if (!state.empty()) {
			// Verify symbol defined
			bool found = false;
			std::list<std::string>::iterator riter;
			for (riter = ruleStates.begin(); riter != ruleStates.end(); riter++) {
				if (*riter == state) {
					found = true;
					break;
				}
			}
			if (!found) {
				fprintf(stderr,"%s:%d state %s not defined with %%state declaration\n",lex.fFileName.c_str(),lex.fTokenLine,state.c_str());
			}
		}

		// Examine regex to determine if it starts with a '^' or end
		// with a '$'. If so, mark the rule appropriately
		atStart = RuleStartsLine(regex);
		if (atStart) regex.erase(0,1);	// delete '^' from start

		atEnd = RuleEndsLine(regex);
		if (atEnd) regex.erase(regex.size()-1,1);

		int sym = lex.ReadToken();
		if (sym == '{') {
			inError = false;
			std::string code;

			/*
			 *	Parse the stream of tokens, copying them over.
			 */

			int cdepth = 1;

			sym = lex.ReadToken(true);
			for (;;) {
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
				} else if (lex.fToken == "BEGIN") {
					/*
					 *	BEGIN state; or BEGIN 0;
					 *
					 *		Translate into executable code appropriate for this
					 *	state transition
					 */

					sym = lex.ReadToken();
					if (sym == -1) {
						fprintf(stderr,"%s:%d Unexpected end of file reached in grammar declarations section\n",lex.fFileName.c_str(),lex.fTokenLine);
						return false;
					} else {
						if (lex.fToken == "0") {
							// insert code to clear state
							code += "states = 0;";
						} else {
							// Find bitmask
							uint32_t index = 1;
							bool found = false;
							std::list<std::string>::iterator riter;
							for (riter = ruleStates.begin(); riter != ruleStates.end(); riter++) {
								if (*riter == lex.fToken) {
									found = true;
									break;
								}
								index <<= 1;
							}

							if (found) {
								char buffer[64];
								sprintf(buffer,"states |= %u;",index);
								code += buffer;
							} else {
								fprintf(stderr,"%s:%d Illegal state after BEGIN declaration\n",lex.fFileName.c_str(),lex.fTokenLine);
							}
						}
						if (';' != lex.ReadToken()) {
							fprintf(stderr,"%s:%d Expect ';' after BEGIN declaration\n",lex.fFileName.c_str(),lex.fTokenLine);
						}
					}
				} else if (lex.fToken == "END") {
					/*
					 *	END state;
					 *
					 *		Translate into executable code appropriate for this
					 *	state transition
					 */

					sym = lex.ReadToken();
					if (sym == -1) {
						fprintf(stderr,"%s:%d Unexpected end of file reached in grammar declarations section\n",lex.fFileName.c_str(),lex.fTokenLine);
						return false;
					} else {
						// Find bitmask
						uint32_t index = 1;
						bool found = false;
						std::list<std::string>::iterator riter;
						for (riter = ruleStates.begin(); riter != ruleStates.end(); riter++) {
							if (*riter == lex.fToken) {
								found = true;
								break;
							}
							index <<= 1;
						}

						if (found) {
							char buffer[64];
							sprintf(buffer,"states &= ~%u;",index);
							code += buffer;
						} else {
							fprintf(stderr,"%s:%d Illegal state after END declaration\n",lex.fFileName.c_str(),lex.fTokenLine);
						}
						if (';' != lex.ReadToken()) {
							fprintf(stderr,"%s:%d Expect ';' after END declaration\n",lex.fFileName.c_str(),lex.fTokenLine);
						}
					}
				} else {
					code += lex.fToken;
				}
				sym = lex.ReadToken(false);
			}

			OCLexParser::Rule r;
			r.regex = regex;
			r.code = code;
			r.atStart = atStart;
			r.atEnd = atEnd;
			r.state = state;
			rules.push_back(r);

		} else {
			if (!inError) {
				inError = true;
				fprintf(stderr,"%s:%d Syntax error in declarations\n",lex.fFileName.c_str(),lex.fTokenLine);
			}
		}
	}
}

