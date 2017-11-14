//
//  OCLexNFA.h
//  oclex
//
//  Created by William Woody on 7/26/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCLexNFA_h
#define OCLexNFA_h

#include <OCUtilities.h>

#include "OCCharSet.h"

#include <stdio.h>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

/************************************************************************/
/*																		*/
/*	Internal Structures													*/
/*																		*/
/************************************************************************/

/*
 *	TODO: (Some musings as I figure this out)
 *
 *		We need to handle the following case:
 *
 *	<AA>magic         printf("first");
 *	<BB>magic         printf("second");
 *	<CC>magic         printf("third");
 *
 *		where AA, BB and CC can be in any state true/false.
 *
 *	As it turns out this cannot be easily handled by building the appropriate
 *	DFA state machine, unless we are willing to handle all 8 cases. That's
 *	because we cannot know apriori when we're starting the state transitions
 *	if we have something like:
 *
 *	<AA>[Mm]agic
 *	<BB>magic
 *	<CC>Magic
 *	[Mm]agic
 *
 *	The problem being that a set of rules may have multiple possible resulting
 *	states, but only one reulting state is valid once we've parsed that
 *	combination of letters.
 *
 *	And ultimately the start state determines which state we find in the
 *	DFA. Meaning the original top case is identical to:
 *
 *	magic				{ if AA then first, if BB then second, if CC then thrid }
 *
 *	and it may make more sense to handle it in this way: for the final result
 *	state to drop into a piece of code to determine which is the state we're
 *	to handle.
 *
 *	----
 *
 *	Handling ^
 *
 *	The problem with our current code is that:
 *
 *	^[A-Z]+
 *	[A-Z]+
 *
 *	will **fail** and not parse the second rule if the first rule is not true.
 *	Meaning right now it converts to:
 *
 *	[A-Z]*				{ if ^ then first }
 *
 *	and if ^ is not true, then the pattern [A-Z]* is ignored.
 *
 *	So there are two ways to handle it; either wrap it in the same sort of
 *	conditional code as handled above, or create an extra transition that
 *	can optionally happen if we are at the start of the line.
 *
 *	So perhaps the right answer here is:
 *
 *		(1) Replace DFA state with a list of states and conditions.
 *		(2) Remove existing state flag material. (StateFlag, StateCond and
 *	the like. Update StateActions to use a negative number if the rule
 *	is actually a conditional, then use a switch statement to jump into code
 *	which evaluates the conditional.
 */


/*	OCLexNFATransition
 *
 *		Represents an outbound transition from an old state to a new state.
 *	This represents the character set for the outbound state, and a pointer
 *	to the new state
 */

struct OCLexNFATransition
{
	OCCharSet	set;
	bool		e;			// true if this is an empty transition
	uint32_t	state;

	OCLexNFATransition()
		{
			e = false;
			state = 0;
		}
};

/*	OCLexNFAState
 *
 *		A state object; this represents a state, which is a list of
 *	transition objects to other states, and a marker to determine if this
 *	is an end state (and whose end-state this is)
 */

struct OCLexNFAState
{
	std::list<OCLexNFATransition> list;

	bool						end;		// End marker of rule?
	uint32_t					endRule;	// Rule ID that this ends

	OCLexNFAState()
		{
			end = false;
			endRule = 0;
		}
};

/*	OCLexNFA
 *
 *		Non-deterministic finite automata storage; holds references to
 *	the front and the end states
 */

struct OCLexNFAReturn
{
	uint32_t start;
	uint32_t end;
};

/************************************************************************/
/*																		*/
/*	Routines															*/
/*																		*/
/************************************************************************/

/*	OCLexNFA
 *
 *		NFA construction class. This contains all the state necessary to
 *	construct the NFA rules, including array accessors to look up the states
 *	by state index
 */

class OCLexNFA
{
	public:
		OCLexNFA(std::map<std::string,std::string> &defn) : definitions(defn)
			{
				stateIndex = 0;
			}

		~OCLexNFA()
			{
			}

		OCLexNFAReturn AddRule(const char *regex)
			{
				return Construct(regex);
			}

		std::vector<OCLexNFAState> states;
		std::list<std::string> ruleStates;

	private:
		uint32_t stateIndex;
		std::map<std::string,std::string> &definitions;
		std::set<std::string> inDefinition;

		OCLexNFAReturn Construct(const char *&regex);
		char EscapeCharacter(const char *&regex);

		OCLexNFAReturn ConstructString(const char *&regex);
		OCLexNFAReturn ConstructCharSet(const char *&regex);
		OCLexNFAReturn ConstructDefinition(const char *&regex);

		uint32_t NewState();
};


#endif /* OCLexNFA_h */
