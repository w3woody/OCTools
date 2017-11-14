//
//  OCLexParser.h
//  oclex
//
//  Created by William Woody on 7/24/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCLexParser_h
#define OCLexParser_h

#include <OCLexer.h>

#include <stdio.h>
#include <map>
#include <list>
#include <vector>
#include <utility>
#include <string>

#include "OCStartState.h"

/************************************************************************/
/*																		*/
/*	Parser Definitions													*/
/*																		*/
/************************************************************************/

/*	OCLexParser
 *
 *		Input stream parser; converts language definition into an abstract
 *	syntax tree for processing
 */

class OCLexParser
{
	public:
		OCLexParser();
		~OCLexParser();

		bool ParseFile(OCLexer &lex);

		/*
		 *	Definitions
		 */

		struct Rule {
			std::string regex;
			std::string code;

			OCStartState start;		// Start state marker
		};

		// Declaration code
		std::string declCode;		// code in the declaration section

		// Class parameters
		std::string classHeader;	// code in .h header section
		std::string classLocal;		// code in .m code declarations
		std::string classGlobal;	// code in .h code declaratiokns
		std::string classInit;		// code in .m init... code

		// definitions
		std::map<std::string,std::string> definitions;

		// states declaration
		std::list<std::string> ruleStates;

		// Rules: contains the rules to match against
		std::list<Rule> rules;

		// End code block
		std::string endCode;

	private:
		bool ParseDeclarations(OCLexer &lex);
		bool ParseRules(OCLexer &lex);

		std::list<std::string> RuleStartState(std::string &r, OCLexer &lex);
};


#endif /* OCLexParser_h */
