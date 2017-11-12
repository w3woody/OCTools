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
 *		To handle conditional rules, I want to create a new NFA transition
 *	for the "start state" which signifies the start state transition. That
 *	is, I want to create a new transition state S:
 *
 *		S is transitioned only if the set of start conditions are correct.
 *
 *		There are two sorts of start transitions. First is the list of
 *	defined start rules {A}. Second is the condition if we are at the start
 *	of the line or not.
 *
 *		Behind each transition we then note the transitions forward only to
 *	those states that are permitted by that transition. So, for example, if
 *	we have the following rules:
 *
 *		A: ^...
 *		B: {R}...
 *		C: ...
 *		D: ...
 *
 *		Then we need to create initial NFA transitions representing the
 *	transition through ^ and through {R}.
 *
 *		The DFA transition rules must then be able to differentiate between
 *	transitioning through ^ and not ^, {R} and not {R}. These transition rules
 *	should be handled differently from rules which read characters from the
 *	input stream.
 *
 *	QUESTION: Do we need to track in each NFA constructed the list of valid
 *	states that this NFA operates in? Otherwise, the DFA process may collapse
 *	the rules:
 *
 *		{A}[A-C]+
 *		[A-C]+
 *
 *	since once we move beyond the initial conditional of the {A}, the two
 *	rules look identical. This may also imply we need to track the start
 *	set for the DFA rules being constructed as well.
 *
 *	NOTE: We may need to include a mask to indicate if a DFA can be taken if
 *	start conditional is true, false, or either. This is because the set of
 *	start rules change for two states A and B if A is true, B is true or both
 *	are true. And that implies a new data structure OCStartSet which tracks
 *	all of these possible combinations.
 *
 *	It also implies that transition states in the DFA may need to be picked
 *	depending on the current state and not just the letter read. (This is
 *	because parsing the two states:
 *
 *		{A}"[^"]*"
 *		"(\\.|[^\\"])*"
 *
 *	will diverge with the character sequence "\" if condition A is true.
 *	(It is a legal match to the first rule but not a legal match with the
 *	second.) And that implies for each state in the DFA we need some sort
 *	of indicator if the start state must be validated against during parsing.
 *
 *	NOTE: This raises an interesting problem: if we have a list of start
 *	states as a conditional, ideally the best thing to do is to allow the
 *	start state to permeate down while constructing the DFA in the same way
 *	we handle characters, but with multiple conditionals possibly being
 *	true. This creates the potential of complex AND/OR conditionals during
 *	construction.
 *
 *	I may need to add the start state object and experiment with how it
 *	is transformed during DFA conversion of an NFA state. That's because,
 *	for example,
 *
 *		{A}^...
 *
 *	implies start conditions A AND ^ are both true. This also implies during
 *	DFA construction we have a second set of states, followed if
 *	(NOT A) OR (NOT ^).
 *
 *	This gets more complicated if we have {A,B}^..., which implies
 *
 *		(A OR B) AND ^
 *
 *	and the fork is taken on
 *
 *		(NOT A AND NOT B) OR NOT ^
 *
 *	Because these are start states, I don't think they percolate down
 *	any farther than this. (And I wonder at the feasibility of joining
 *	disjointed states; I suspect that doesn't work.)
 *
 *	NOTE: It could be at the bottom of the stack we wind up generating
 *	a method which contains all the conditional compares to determine the
 *	start state in the final generated code. Despite this we do get a
 *	savings in the final size of the state machine. For example, if we have
 *	a rule:
 *
 *		{A}"[^"]*"
 *		"(\\.|[^\\"])*"
 *
 *	This implies in both state machines the collection of rules followed for
 *	any sequence not starting with a " will be the same.
 *
 *	(How do we guarantee this?)
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
