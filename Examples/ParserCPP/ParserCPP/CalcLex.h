//
//  CalcLex.h
//  ParserCPP
//
//  Created by William Woody on 4/3/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#ifndef CalcLex_h
#define CalcLex_h

#include "CalcParser.h"

class CalcLex : public CalcParserInput
{
	public:
		CalcLex()
			{
				ix = 1;
			}

		virtual int32_t lex();

	private:
		int ix;
};

#endif /* CalcLex_h */
