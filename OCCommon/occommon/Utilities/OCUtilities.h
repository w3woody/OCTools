//
//  OCUtilities.h
//  occommon
//
//  Created by William Woody on 7/26/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCUtilities_h
#define OCUtilities_h

#include <stdlib.h>

/************************************************************************/
/*																		*/
/*	Pooled allocator													*/
/*																		*/
/************************************************************************/

/*	OCAlloc
 *
 *		Alloc memory as part of a pool which can be released all at once.
 *	Used to store intermediate state
 */

class OCAlloc
{
	public:
		OCAlloc();
		~OCAlloc();

		void *Alloc(size_t size);

	private:
		void *fPool;
		void *fAlloc;
		size_t fSize;
};


#endif /* OCUtilities_h */
