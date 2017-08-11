//
//  OCYaccLR1.cpp
//  ocyacc
//
//  Created by William Woody on 8/10/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCYaccLR1.h"

/************************************************************************/
/*																		*/
/*	Construction														*/
/*																		*/
/************************************************************************/

/*	OCYaccLR1::OCYaccLR1
 *
 *		Construct
 */

OCYaccLR1::OCYaccLR1()
{
}

/*	OCYaccLR1::~OCYaccLR1
 *
 *		Destruct
 */

OCYaccLR1::~OCYaccLR1()
{
}


/************************************************************************/
/*																		*/
/*	Construction Entry Point											*/
/*																		*/
/************************************************************************/

/*	OCYaccLR1::Construct
 *
 *		Construct the grammar state machine and eventually build the
 *	various state tables
 */

bool OCYaccLR1::Construct(OCYaccParser &p)
{
	/*
	 *	Reset
	 */

	grammar.clear();

	/*
	 *	Convert the tokens and productions in our grammar to integer values
	 *	for faster processing. This also allows us to write our rules and
	 *	statements out quickly.
	 *
	 *	We also predefine the tokens $end and error.
	 *
	 *	Note: we assume 8-byte characters. We would need to change this to
	 *	start at 0x110000 if we want to support Unicode, as Unicode has
	 *	0x10FFFF code points. (This would allow us to write a language which
	 *	used a smily face as a token.) This would also require OCLex to be
	 *	rewritten...
	 */

	uint32_t index = 256;			// Assumption: 8-byte characters.

	/*
	 *	Insert our reserved tokens $end and error. Note we do not define
	 *	an '$undefined' token, since we use sparse compression, thus big
	 *	gaps in the token ID don't matter.
	 */

	tokenList.push_back("$end");
	tokens["$end"] = index++;
	tokenList.push_back("error");
	tokens["error"] = index++;


	/*
	 *	Scan all the parsed rules for all of the tokens. Anything written
	 *	as 'x' is a character, and uses the value of the character as the
	 *	token value.
	 */



	return false;
}
