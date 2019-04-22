//
//  OCYaccSwiftGenerator.h
//  ocyacc
//
//  Created by William Woody on 4/22/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#ifndef OCYaccSwiftGenerator_h
#define OCYaccSwiftGenerator_h

#include <stdio.h>
#include "OCYaccLR1.h"

/************************************************************************/
/*																		*/
/*	Class Declarations													*/
/*																		*/
/************************************************************************/

/*	OCYaccSwiftGenerator
 *
 *		Code to write header file
 */

class OCYaccSwiftGenerator
{
	public:
		OCYaccSwiftGenerator(const OCYaccParser &p, const OCYaccLR1 &y);
		~OCYaccSwiftGenerator();

		void WriteOCLexInput(FILE *f);
		void WriteOCFile(const char *classname, const char *outputName, FILE *f);

	private:
		const OCYaccParser &parser;
		const OCYaccLR1 &state;

		void WriteYTables(FILE *f);
		void WriteRule(FILE *f,  const OCYaccLR1::Reduction &rule);
};

#endif /* OCYaccSwiftGenerator_h */
