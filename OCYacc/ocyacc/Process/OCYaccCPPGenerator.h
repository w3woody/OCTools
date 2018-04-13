//
//  OCYaccCPPGenerator.h
//  ocyacc
//
//  Created by William Woody on 4/12/18.
//  Copyright © 2018 Glenview Software. All rights reserved.
//

#ifndef OCYaccCPPGenerator_h
#define OCYaccCPPGenerator_h

#include <stdio.h>
#include "OCYaccLR1.h"

/************************************************************************/
/*																		*/
/*	Class Declarations													*/
/*																		*/
/************************************************************************/

/*	OCYaccCPPGenerator
 *
 *		Code to write header file
 */

class OCYaccCPPGenerator
{
	public:
		OCYaccCPPGenerator(const OCYaccParser &p, const OCYaccLR1 &y);
		~OCYaccCPPGenerator();

		void WriteOCHeader(const char *classname, const char *outputName, FILE *f);
		void WriteOCFile(const char *classname, const char *outputName, FILE *f);

	private:
		const OCYaccParser &parser;
		const OCYaccLR1 &state;

		void WriteYTables(FILE *f);
		void WriteRule(FILE *f,  const OCYaccLR1::Reduction &rule);
};

#endif /* OCYaccCPPGenerator_h */
