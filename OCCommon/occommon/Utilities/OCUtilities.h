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

		size_t Size() const
			{
				return size;
			}
		uint32_t Item(size_t index) const
			{
				return list[index];
			}

		std::string ToString();

	private:
		size_t alloc;
		size_t size;
		uint32_t *list;		// Sorted list of integer items
};


/************************************************************************/
/*																		*/
/*	State Machine Compression											*/
/*																		*/
/************************************************************************/

/*	OCCompressStates
 *
 *		Given an input array and the dimensions of the array, this generates
 *	a compressed state machine using the CSR sparce compression format
 *	documented here:
 *
 *	https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_.28CSR.2C_CRS_or_Yale_format.29
 */

class OCCompressStates
{
	public:
		// States are in row major format
		OCCompressStates(size_t width, size_t height, uint32_t *states, uint32_t illegal);
		~OCCompressStates();

		/*
		 *	Values calculated on construction
		 */

		size_t iwidth;
		uint32_t *ia;

		size_t asize;
		uint32_t *ja;
		uint32_t *a;
};

#endif /* OCUtilities_h */
