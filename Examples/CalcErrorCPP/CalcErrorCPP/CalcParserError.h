//
//  CalcParserError.h
//  CalcErrorCPP
//
//  Created by William Woody on 4/2/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#ifndef CalcParserError_h
#define CalcParserError_h

#include "CalcParser.h"

/*
 *	Use class inheritance rather than delegation to obtain errors. In the
 *	future it may make sense to use delegation...
 */

class CalcParserError : public CalcParser
{
	public:
		CalcParserError(CalcLex *lex) : CalcParser(lex)
			{
			}

		void error(int32_t line, int32_t col, std::string fname, int32_t errCode, std::map<std::string,std::string> &map);
};

#endif /* CalcParserError_h */
