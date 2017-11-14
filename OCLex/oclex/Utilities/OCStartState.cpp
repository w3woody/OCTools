//
//  OCStartState.cpp
//  oclex
//
//  Created by William Woody on 11/12/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCStartState.h"

/**
 *	Convert to bitmask
 */

uint32_t OCStartState::stateFlags(const std::list<std::string> &rstates) const
{
	uint32_t ret = 0;
	uint32_t mask = 1;

	std::list<std::string>::const_iterator riter;
	for (riter = rstates.begin(); riter != rstates.end(); ++riter) {
		std::list<std::string>::const_iterator iter;
		for (iter = fStates.begin(); iter != fStates.end(); ++iter) {
			if (*iter == *riter) {
				ret |= mask;
				break;
			}
		}

		mask <<= 1;
	}

	return ret;
}

