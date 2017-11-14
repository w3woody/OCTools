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
#include <list>

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
				fStart = false;
				fEnd = false;
			}

		~OCStartState()
			{
			}

		/*
		 *	Building the start condition. In general the start condition
		 *	is (startLine) && (state || state || state...)
		 */

		void addStartCondition(std::string state)
			{
				fStates.push_back(state);
			}
		void setStartConditions(std::list<std::string> state)
			{
				fStates = state;
			}
		void addStartLine()
			{
				fStart = true;
			}
		void addEndLine()
			{
				fEnd = true;
			}

		/*
		 *	Return true if there are no start conditions
		 */

		bool unconditional() const
			{
				return !fStart && !fEnd && (fStates.size() == 0);
			}

		/*
		 *	Get the start, end states
		 */

		bool startFlag() const
			{
				return fStart;
			}
		bool endFlag() const
			{
				return fEnd;
			}

		uint32_t stateFlags(const std::list<std::string> &rstates) const;

	private:
		bool fStart;
		bool fEnd;
		std::list<std::string> fStates;
};

#endif /* OCStartSet_hpp */
