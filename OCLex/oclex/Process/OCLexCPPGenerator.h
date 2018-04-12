//
//  OCLexCPPGenerator.hpp
//  oclex
//
//  Created by William Woody on 4/12/18.
//  Copyright © 2018 Glenview Software. All rights reserved.
//

#ifndef OCLexCPPGenerator_h
#define OCLexCPPGenerator_h

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

class OCLexCPPGenerator : public OCLexDFA
{
	public:
		OCLexCPPGenerator(std::map<std::string,std::string> &defn): OCLexDFA(defn)
			{
			}

		~OCLexCPPGenerator()
			{
			}

		/*
		 *	Write the Objective C file, after the DFA is generated.
		 */

		void WriteOCHeader(const char *className, const char *outputName, FILE *f);
		void WriteOCFile(const char *className, const char *outputName, FILE *f);

	private:
		void WriteStarts(FILE *f, const char *className);
		void WriteArray(FILE *f, uint32_t *list, size_t len);
		void WriteActions(FILE *f);

		void WriteStates(FILE *f);
};

#endif /* OCLexCPPGenerator_hpp */
