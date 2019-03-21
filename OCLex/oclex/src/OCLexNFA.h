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
