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
	bool						end;
	uint32_t					endRule;	// Rule ID that this ends
};

/************************************************************************/
/*																		*/
/*	Routines															*/
/*																		*/
/************************************************************************/

/*	OCConstructRule
 *
 *		Given a single regular expression, construct the NFA state
 *	in the alloc pool provided
 */


// ### TODO: Code to return start and end states.

extern OCLexNFAState *OCConstructRule(OCAlloc &alloc, const char *regex);

#endif /* OCLexNFA_h */
