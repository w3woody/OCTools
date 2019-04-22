//
//  OCLexSwiftGenerator.h
//  oclex
//
//  Created by William Woody on 4/19/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#ifndef OCLexSwiftGenerator_h
#define OCLexSwiftGenerator_h

#include <stdio.h>
#include "OCLexDFA.h"

/************************************************************************/
/*																		*/
/*	Parser Definitions													*/
/*																		*/
/************************************************************************/

/*	OCLexSwiftGenerator
 *
 *		Inherits from the DFA and contains the code which writes the
 *	resulting Objective C output file
 */

class OCLexSwiftGenerator : public OCLexDFA
{
	public:
		OCLexSwiftGenerator(std::map<std::string,std::string> &defn): OCLexDFA(defn)
			{
			}

		~OCLexSwiftGenerator()
			{
			}

		/*
		 *	Write the Objective C file, after the DFA is generated.
		 */

		void WriteOCFileInput(FILE *f);
		void WriteOCLexInput(FILE *f);
		void WriteOCFile(const char *className, const char *outputName, FILE *f);

	private:
		void WriteStarts(FILE *f, const char *className);
		void WriteArray(FILE *f, uint32_t *list, size_t len);
		void WriteActions(FILE *f);

		void WriteStates(FILE *f);
};

#endif /* OCLexSwiftGenerator_h */
