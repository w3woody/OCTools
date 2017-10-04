//
//  OCLexDFA.h
//  oclex
//
//  Created by William Woody on 7/29/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCLexDFA_h
#define OCLexDFA_h

#include "OCLexNFA.h"

#include <stdio.h>

/************************************************************************/
/*																		*/
/*	Structures															*/
/*																		*/
/************************************************************************/

/*	OCLexDFATransition
 *
 *		Transition for a DFA
 */

struct OCLexDFATransition
{
	OCCharSet	set;
	uint32_t	state;
};

/*	OCLexDFAState
 *
 *		DFA state
 */

struct OCLexDFAState
{
	std::vector<OCLexDFATransition> list;
	bool end;				// if this is an end state
	uint32_t endRule;		// index to the code rule representing this state

	OCLexDFAState()
	{
		end = false;
		endRule = 0;
	}
};

/************************************************************************/
/*																		*/
/*	DFA constructor; uses NFA as a base									*/
/*																		*/
/************************************************************************/

/*	OCLexDFA
 *
 *		Helper to construct the deterministic finite state automaton which
 *	can parse our Lexical statements
 */

class OCLexDFA: public OCLexNFA
{
	public:
		OCLexDFA(std::map<std::string,std::string> &defn) : OCLexNFA(defn)
			{
			}

		~OCLexDFA()
			{
			}

		struct CodeRule {
			std::string code;
			bool atEnd;
		};

		/*
		 *	Add rules with rule states and code
		 */

		void AddRuleSet(std::string regex, std::string code, bool atStart, bool atEnd);

		/*
		 *	Generate DFA
		 */

		bool GenerateDFA();

		/*
		 *	Final DFA structure
		 */

		std::string declCode;			// include section of .m
		std::string classHeader;		// class declaration of .h
		std::string classLocal;			// class declaration of .m
		std::string classInit;			// init code to insert in init
		std::string classGlobal;		// class declaration of .h
		std::string endCode;			// code section of .m

		std::vector<OCCharSet> charClasses;
		std::vector<CodeRule> codeRules;
		std::vector<OCLexDFAState> dfaStates;


	private:
		/*
		 *	Store the start states and the list of code rules
		 *	which are melded into the underlying NFA
		 */

		std::vector<uint32_t> startStates;

		/*
		 *	Support for mapping the power set to a state index
		 */

		std::map<OCIntegerSet,uint32_t> setMap;

		void EClosure(OCIntegerSet &set);
		void SplitCharSet(std::vector<OCCharSet> &set);
		void FindEndRule(OCLexDFAState &state, const OCIntegerSet &set);
};

#endif /* OCLexDFA_h */
