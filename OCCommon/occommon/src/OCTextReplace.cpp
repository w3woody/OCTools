//
//  OCTextReplace.cpp
//  occommon
//
//  Created by William Woody on 4/21/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#include <stdio.h>
#include "OCUtilities.h"

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	OCTextReplace::OCTextReplace
 *
 *		Construct
 */

OCTextReplace::OCTextReplace()
{
}

/*	OCTextReplace::~OCTextReplace
 *
 *		Destruction
 */

OCTextReplace::~OCTextReplace()
{
}

/************************************************************************/
/*																		*/
/*	File writing.														*/
/*																		*/
/************************************************************************/

/*	OCTextReplace::write
 *
 *		Write the file pointed to by the text pointer. This also scans for
 *	sequences in the text: $(XXX) and replace with the key in the map.
 */

void OCTextReplace::write(const char *text, FILE *f)
{
	const char *ptr = text;
	int index;

	while (*ptr) {
		if (*ptr == '$') {
			if (ptr[1] == '(') {
				/*
				 *	Scan forward, capturing the name
				 */

				std::string key;
				index = 2;
				while ((ptr[index] != ')') && (ptr[index])) {
					key.push_back(ptr[index++]);
				}
				if (ptr[index]) {
					ptr += index + 1;
					std::string value = map[key];
					fputs(value.c_str(), f);
					continue;
				}
			}
		}

		fputc(*ptr++, f);
	}
}
