//
//  OCLexGenerator.h
//  oclex
//
//  Created by William Woody on 7/30/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCLexGenerator_h
#define OCLexGenerator_h

#include <stdio.h>
#include "OCLexDFA.h"

/************************************************************************/
/*																		*/
/*	Parser Definitions													*/
/*																		*/
/************************************************************************/

/*	OCLexGenerator
 *
 *		Inherits from the DFA and contains the code which writes the
 *	resulting Objective C output file
 */

class OCLexGenerator : public OCLexDFA
{
	public:
		OCLexGenerator(std::map<std::string,std::string> &defn): OCLexDFA(defn)
			{
			}

		~OCLexGenerator()
			{
			}

		/*
		 *	Write the Objective C file, after the DFA is generated.
		 */

		void WriteOCHeader(const char *className, const char *outputName, FILE *f);
		void WriteOCFile(const char *className, const char *outputName, FILE *f);

	private:
		void WriteArray(FILE *f, uint32_t *list, size_t len);
		void WriteActions(FILE *f);

		void WriteStates(FILE *f);
};

#endif /* OCLexGenerator_h */
