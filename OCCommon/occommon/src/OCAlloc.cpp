//
//  OCAlloc.cpp
//  occommon
//
//  Created by William Woody on 7/26/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCUtilities.h"
#include <new>
#include <string.h>

/************************************************************************/
/*																		*/
/*	Constants															*/
/*																		*/
/************************************************************************/

#define MAXALLOC		(32768-16)
#define MAXSIZE			512

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	OCAlloc::OCAlloc
 *
 *		Construct allocator
 */

OCAlloc::OCAlloc()
{
	fSize = MAXALLOC;
	fPool = NULL;
	fAlloc = NULL;
}

/*	OCAlloc::~OCAlloc
 *
 *		Release
 */

OCAlloc::~OCAlloc()
{
	while (fAlloc) {
		void *next = *((void **)fAlloc);
		free(fAlloc);
		fAlloc = next;
	}
}

/*	OCAlloc::Alloc
 *
 *		Alloc memory
 */

void *OCAlloc::Alloc(size_t size)
{
	size = (size + 7) & ~7;

	if (size >= MAXSIZE) {
		void *ptr = malloc(size + sizeof(void *));
		if (ptr == NULL) throw std::bad_alloc();
		memset(ptr,0,size + sizeof(void *));
		*((void **)ptr) = fAlloc;
		fAlloc = ptr;

		return (void *)(1 + (void **)ptr);
	} else {
		if (size + fSize >= MAXALLOC) {
			fPool = malloc(MAXALLOC);
			if (fPool == NULL) throw std::bad_alloc();
			memset(fPool,0,MAXALLOC);
			*((void **)fPool) = fAlloc;
			fAlloc = fPool;

			fSize = sizeof(void *);
		}

		void *ret = (void *)(fSize + (unsigned char *)fPool);
		fSize += size;
		return ret;
	}
}
