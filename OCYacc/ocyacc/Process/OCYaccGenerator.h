//
//  OCYaccGenerator.h
//  ocyacc
//
//  Created by William Woody on 8/13/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCYaccGenerator_h
#define OCYaccGenerator_h

#include <stdio.h>
#include "OCYaccLR1.h"

/************************************************************************/
/*																		*/
/*	Class Declarations													*/
/*																		*/
/************************************************************************/

/*	OCYaccGenerator
 *
 *		Code to write header file
 */

class OCYaccGenerator
{
	public:
		OCYaccGenerator(const OCYaccParser &p, const OCYaccLR1 &y);
		~OCYaccGenerator();

		void WriteOCHeader(const char *classname, const char *outputName, FILE *f);
		void WriteOCFile(const char *classname, const char *outputName, FILE *f);

	private:
		const OCYaccParser &parser;
		const OCYaccLR1 &state;

		void WriteYTables(FILE *f);
		void WriteRule(FILE *f,  const OCYaccLR1::Reduction &rule);
};

#endif /* OCYaccGenerator_h */
