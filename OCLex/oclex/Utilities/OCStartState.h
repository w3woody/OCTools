//
//  OCStartState.h
//  oclex
//
//  Created by William Woody on 11/12/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCStartState_h
#define OCStartState_h

#include <stdint.h>
#include <string.h>
#include <string>


/************************************************************************/
/*                                                                      */
/*  Start State Set														*/
/*																		*/
/************************************************************************/

/*	OCStartState
 *
 *		The set of start states. Basically represents a set of OR
 *	conditionals of the form (expr1 | expr2 ...) where expr is
 *	either "state = true" or "state = false".
 *
 *		Note that this is constructed in the context of a finite number
 *	of states. So negate (state1 = true) | (state2 = true)
 */

class OCStartState
{
	public:
		OCStartState()
			{
			}

		~OCStartState()
			{
			}

};

#endif /* OCStartSet_hpp */
