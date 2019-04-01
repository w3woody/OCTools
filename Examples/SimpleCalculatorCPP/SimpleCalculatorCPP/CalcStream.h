//
//  CalcStream.h
//  SimpleCalculatorCPP
//
//  Created by William Woody on 4/1/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#ifndef CalcStream_h
#define CalcStream_h

#include <stdio.h>
#include "CalcLex.h"

/*
class OCFileInput
{
	public:
		virtual int readByte() = 0;
		virtual int peekByte() = 0;
};
*/

class CalcStream : public OCFileInput
{
	public:
		CalcStream(const char *str)
			{
				string = str;
				ptr = string.c_str();
			}

		virtual int readByte()
			{
				if (!*ptr) return -1;
				return *ptr++;
			}
		virtual int peekByte()
			{
				if (!*ptr) return -1;
				return *ptr;
			}

	private:
		std::string string;
		const char *ptr;
};

#endif /* CalcStream_h */
