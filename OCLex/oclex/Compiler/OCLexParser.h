//
//  OCLexParser.h
//  oclex
//
//  Created by William Woody on 7/24/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCLexParser_h
#define OCLexParser_h

#include <OCLexer.h>

#include <stdio.h>
#include <map>
#include <list>
#include <vector>
#include <string>

/************************************************************************/
/*																		*/
/*	Parser Definitions													*/
/*																		*/
/************************************************************************/

/*	OCLexParser
 *
 *		Input stream parser; converts language definition into an abstract
 *	syntax tree for processing
 */

class OCLexParser
{
	public:
		OCLexParser();
		~OCLexParser();

		bool ParseFile(OCLexer &lex);


	private:
};


#endif /* OCLexParser_h */
