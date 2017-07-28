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
#include <string>

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

/************************************************************************/
/*																		*/
/*	Integer Set															*/
/*																		*/
/************************************************************************/

/*	OCIntegerSet
 *
 *		Represents a set of 32-bit integers. Used when building NFAs
 */

class OCIntegerSet
{
	public:
		OCIntegerSet();
		OCIntegerSet(const OCIntegerSet &set);
		~OCIntegerSet();

		OCIntegerSet &operator = (const OCIntegerSet &set);

		bool operator == (const OCIntegerSet &set) const;
		bool operator < (const OCIntegerSet &set) const;	// for use in map

		/*
		 *	Modify the set
		 */

		void Add(uint32_t val);
		void AddSet(const OCIntegerSet &set);

		bool Contains(uint32_t val);

		void RemoveAll()
			{
				size = 0;
			}
		void Remove(uint32_t val);
		void RemoveSet(const OCIntegerSet &set);

		size_t Size()
			{
				return size;
			}

		std::string ToString();

	private:
		size_t alloc;
		size_t size;
		uint32_t *list;		// Sorted list of integer items
};


#endif /* OCUtilities_h */
