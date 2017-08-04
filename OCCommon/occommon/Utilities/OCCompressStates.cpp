//
//  OCCompressStates.cpp
//  occommon
//
//  Created by William Woody on 8/3/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include <stdio.h>
#include "OCUtilities.h"
#include <stdlib.h>

/************************************************************************/
/*																		*/
/*	State Machine Compression											*/
/*																		*/
/************************************************************************/

/*	OCCompressStates
 *
 *		Compress according to the CSR format
 */

OCCompressStates::OCCompressStates(size_t width, size_t height, uint32_t *states, uint32_t illegal)
{
	/*
	 *	Allocate memory
	 */

	iwidth = height + 1;
	ia = (uint32_t *)malloc(sizeof(uint32_t) * iwidth);

	asize = 0;
	size_t tsize = width * height;
	for (size_t i = 0; i < tsize; ++i) {
		if (states[i] != illegal) ++asize;
	}
	size_t alloc = asize ? asize : 1;

	a = (uint32_t *)malloc(sizeof(uint32_t) * alloc);
	ja = (uint32_t *)malloc(sizeof(uint32_t) * alloc);

	/*
	 *	Now run the compression 
	 */

	size_t pos = 0;
	for (size_t y = 0; y < height; ++y) {
		ia[y] = (uint32_t)pos;
		for (size_t x = 0; x < width; ++x) {
			uint32_t val = states[x + y * width];
			if (val != illegal) {
				ja[pos] = (uint32_t)x;
				a[pos] = val;
				++pos;
			}
		}
	}
	ia[height] = (uint32_t)pos;
}

/*	OCCompressStates::~OCCompressStates
 *
 *		Delete allocated memory
 */

OCCompressStates::~OCCompressStates()
{
	free(ia);
	free(a);
	free(ja);
}
