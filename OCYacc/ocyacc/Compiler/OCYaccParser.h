//
//  OCYaccParser.h
//  ocyacc
//
//  Created by William Woody on 7/25/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCYaccParser_h
#define OCYaccParser_h

#include <OCLexer.h>

#include <stdio.h>
#include <map>
#include <list>
#include <vector>
#include <utility>
#include <string>

/************************************************************************/
/*																		*/
/*	Parser Definitions													*/
/*																		*/
/************************************************************************/

/*	OCYaccParser
 *
 *		Input stream parser; converts language definition into an abstract
 *	syntax tree for processing
 */

class OCYaccParser
{
	public:
		OCYaccParser();
		~OCYaccParser();

		bool ParseFile(OCLexer &lex);

		/*
		 *	Definitions
		 */

		// Declaration code
		std::string declCode;		// code in the declaration section

		// End code block
		std::string endCode;

		/*
		 *	Associativity, if declared for terminal symbols
		 */

		enum Assoc {
			None, Left, Right, NonAssoc
		};

		/*
		 *	SymbolInstance
		 *
		 *		Represents a single instance of a token declaration in
		 *	the CC section.
		 */

		struct SymbolInstance {
			std::vector<std::string> tokenlist;
			std::string code;
		};

		/*
		 *	SymbolDecl
		 *
		 *		Contains a list of all the declarations for a given CC
		 *	symbol
		 */

		struct SymbolDecl {
			std::vector<SymbolInstance> declarations;
		};

		/*
		 *	Parser results
		 */

		// associativity list. Shows %left, %right, %nonassoc mapping if
		// declared
		std::map<std::string,Assoc> terminalSymbol;

		std::map<std::string,std::string> symbolType;

		// union definition
		std::string unionDecl;

		// Code declaration section
		std::map<std::string,SymbolDecl> symbols;

		// Start symbol
		std::string startSymbol;

	private:
		void SkipToNextDeclaration(OCLexer &lex);
		bool ParseDeclarations(OCLexer &lex);

		void SkipToNextRule(OCLexer &lex);
		bool ParseRules(OCLexer &lex);
};


#endif /* OCYaccParser_h */
