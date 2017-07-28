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
#include <string>

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
	OCCharSet					set;
	bool						e;			// true if this is an empty transition
	struct OCLexNFATransition	*next;
	struct OCLexNFAState		*state;
};

/*	OCLexNFAState
 *
 *		A state object; this represents a state, which is a list of
 *	transition objects to other states, and a marker to determine if this
 *	is an end state (and whose end-state this is)
 */

struct OCLexNFAState
{
	OCLexNFATransition			*list;
	uint32_t					state;		// State number
	bool						end;
	uint32_t					endRule;	// Rule ID that this ends
};

/*	OCLexNFA
 *
 *		Non-deterministic finite automata storage; holds references to
 *	the front and the end states
 */

struct OCLexNFA
{
	OCLexNFAState				*start;
	OCLexNFAState				*end;
};

/************************************************************************/
/*																		*/
/*	Routines															*/
/*																		*/
/************************************************************************/

extern uint32_t GStateIndex;

/*	OCStartRuleConstructor
 *
 *		Reset the internal counter for a group of NFAs
 */

extern void OCStartRuleConstructor();

/*	OCConstructRule
 *
 *		Given a single regular expression, construct the NFA state
 *	in the alloc pool provided
 */

extern OCLexNFA OCConstructRule(OCAlloc &alloc, std::map<std::string,std::string> &definitions, const char *regex);

#endif /* OCLexNFA_h */
