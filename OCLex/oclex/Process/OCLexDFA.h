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
#include "OCStartState.h"

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

/*	OCLexDFAEnd
 *
 *		Represents an end state. End states are listed in order in which
 *	they appear along with the rules which indicate their appearance.
 */

struct OCLexDFAEnd
{
	uint32_t endRule;
	OCStartState startState;
};

/*	OCLexDFAState
 *
 *		DFA state
 */

struct OCLexDFAState
{
	std::vector<OCLexDFATransition> list;
	std::vector<OCLexDFAEnd> endList;		// all potential end rules

	OCLexDFAState()
	{
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
			OCStartState start;
		};

		/*
		 *	Add rules with rule states and code
		 */

		void AddRuleSet(std::string regex, std::string code, const OCStartState &start);

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
		std::string classFinish;		// finish code to insert in init
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
