//
//  CalcParserError.cpp
//  CalcErrorCPP
//
//  Created by William Woody on 4/2/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#include "CalcParserError.h"

/*	CalcParserError::error
 *
 *		Present our error
 */

void CalcParserError::error(int32_t line, int32_t col, std::string fname, int32_t errCode, std::map<std::string, std::string> &data)
{
	std::string err;

	switch (errCode) {
		case ERROR_SYNTAX:
			err = "Syntax Error";
			break;
		case ERROR_MISSINGTOKEN:
			err = "Missing token (";
			err += data["token"];
			err += ")";
			break;
		case ERROR_MISSINGTOKENS:
			err = "Missing tokens (";
			err += data["tokens"];
			err += ")";
			break;
		case ERROR_UNIMPLEMENTED:
			err = "Variables are unimplemented";
			break;
		default:
			err = "Unknown error";
			break;
	}

	printf("%s at %d:%d\n",err.c_str(),line,col);
}
