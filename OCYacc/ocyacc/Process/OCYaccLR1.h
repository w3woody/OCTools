//
//  OCYaccLR1.h
//  ocyacc
//
//  Created by William Woody on 8/10/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCYaccLR1_h
#define OCYaccLR1_h

#include <stdio.h>
#include <set>
#include <vector>
#include <string>

#include "OCYaccParser.h"

/************************************************************************/
/*																		*/
/*	Class Declarations													*/
/*																		*/
/************************************************************************/

/*	OCYaccLR1
 *
 *		Construct the LR1 state tables from the input grammar. This builds
 *	the accept table, the goto table, the final state, and the error
 *	state tables for our LR1 style parser.
 */

class OCYaccLR1
{
	public:
		OCYaccLR1();
		~OCYaccLR1();

		// Construct LR1 tables; return false if error.
		bool Construct(OCYaccParser &p);


	private:
		/*
		 *	Token/production map
		 *
		 *		This maps tokens and productions to and from their integer
		 *	values. We use integers to represent tokens and productions for
		 *	efficiency purposes.
		 */

		uint32_t	maxToken;		// tokens are < this value
		uint32_t	maxProduction;	// maxToken <= productions are < this value

		std::map<std::string,uint32_t>	tokens;			// terminal token to ID
		std::map<std::string,uint32_t>	productions;	// production to ID
		std::vector<std::string> tokenList;
		std::vector<std::string> productionList;

		/*
		 *	Rules: represents a single production rule A -> B C D ... code
		 */

		struct Rule {
			uint32_t production;
			std::vector<uint32_t> tokenlist;
			std::string code;
		};

		/*
		 *	Translated grammar
		 */

		std::vector<Rule> grammar;
};

#endif /* OCYaccLR1_h */
