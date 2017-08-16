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
#include <set>
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

		std::string declCode;			// include section of .m
		std::string classHeader;		// class declaration of .h
		std::string classLocal;			// class declaration of .m
		std::string classGlobal;		// class declaration of .h
		std::string endCode;			// code section of .m

		/*
		 *	FilePos gives the position of certain declarations for 
		 *	error reporting
		 */

		struct FilePos {
			uint32_t line;
			uint32_t col;
			std::string file;
		};

		/*
		 *	Associativity, if declared for terminal symbols
		 */

		enum Assoc {
			Left, Right, NonAssoc
		};

		struct Precedence {
			uint16_t prec;
			Assoc assoc;
//			std::string type;
		};

		/*
		 *	SymbolInstance
		 *
		 *		Represents a single instance of a token declaration in
		 *	the CC section.
		 */

		struct SymbolInstance {
			std::vector<std::string> tokenlist;
			Precedence precedence;		// Precedence level of this fule
			std::string code;
			FilePos pos;			// position of first token in rule
		};

		/*
		 *	SymbolDecl
		 *
		 *		Contains a list of all the declarations for a given CC
		 *	symbol
		 */

		struct SymbolDecl {
			FilePos pos;			// position of first token in rule
			std::vector<SymbolInstance> declarations;
		};

		/*
		 *	Parser results
		 */

		// associativity list. Shows %left, %right, %nonassoc mapping if
		// declared
		std::map<std::string,Precedence> precedence;

		// List of tokens
		std::set<std::string> tokens;

		// Type name for production or token
		std::map<std::string,std::string> symbolType;

		// Code declaration section
		std::map<std::string,SymbolDecl> symbols;

		// Start symbol
		std::string startSymbol;

	private:
		std::string ReadType(OCLexer &lex);	// call after seeing '<'
		void SkipToNextDeclaration(OCLexer &lex);
		bool ParseDeclarations(OCLexer &lex);

		void SkipToNextRule(OCLexer &lex);
		bool ParseRules(OCLexer &lex);
};


#endif /* OCYaccParser_h */
