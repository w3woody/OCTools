//
//  OCIntegerSet.cpp
//  occommon
//
//  Created by William Woody on 7/28/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include <stdio.h>
#include "OCUtilities.h"
#include <new>
#include <string.h>

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	OCIntegerSet::OCIntegerSet
 *
 *		Construct an empty integer set
 */

OCIntegerSet::OCIntegerSet()
{
	size = 0;
	alloc = 16;
	list = (uint32_t *)malloc(sizeof(uint32_t) * alloc);
	if (list == NULL) throw std::bad_alloc();
}

/*	OCIntegerSet::~OCIntegerSet
 *
 *		Delete our integer set
 */

OCIntegerSet::~OCIntegerSet()
{
	if (list) free(list);
}

/*	OCIntegerSet::OCIntegerSet
 *
 *		Copy constructor. Unlike the empty constructor we size the array
 *	to exactly fit the items we're manipulating
 */

OCIntegerSet::OCIntegerSet(const OCIntegerSet &set)
{
	size = set.size;
	alloc = set.size;
	if (alloc < 1) alloc = 1;

	list = (uint32_t *)malloc(sizeof(uint32_t) * alloc);
	if (list == NULL) throw std::bad_alloc();
	memmove(list,set.list,sizeof(uint32_t) * size);
}

/*	OCIntegerSet::operator =
 *
 *		Copy operator. If we have to resize, we only resize to fit
 */

OCIntegerSet &OCIntegerSet::operator =(const OCIntegerSet &set)
{
	if (alloc < set.size) {
		uint32_t *tmp = (uint32_t *)realloc(list,sizeof(uint32_t) * set.size);
		if (tmp == NULL) throw std::bad_alloc();

		alloc = set.size;
		list = tmp;
	}

	size = set.size;
	memmove(list,set.list,sizeof(uint32_t) * size);

	return *this;
}

/************************************************************************/
/*																		*/
/*	Compare Operators													*/
/*																		*/
/************************************************************************/

/*	OCIntegerSet::operator ==
 *
 *		Return true if two items are equal
 */

bool OCIntegerSet::operator == (const OCIntegerSet &set) const
{
	if (size != set.size) return false;

	for (size_t i = 0; i < size; ++i) {
		if (list[i] != set.list[i]) return false;
	}

	return true;
}

/*	OCIntegerSet::operator <
 *
 *		Return true if the first list is less than the second. Does this
 *	using a string-like compare. The operator is not really meaningful except
 *	for providing a strictly ordered set for std::map
 */

bool OCIntegerSet::operator < (const OCIntegerSet &set) const
{
	size_t i;

	for (i = 0; (i < size) && (i < set.size); ++i) {
		if (list[i] < set.list[i]) return true;
		if (list[i] > set.list[i]) return false;
	}

	/* If we get here the prefixes are the same. Return true if the first
	 * list is shorter than the second
	 */

	return size < set.size;
}

/************************************************************************/
/*																		*/
/*	Manipulation														*/
/*																		*/
/************************************************************************/

/*	OCIntegerSet::Contains
 *
 *		Determine if val is contained in our list. We use a binary search
 *	to minimize the time it takes to find our item
 */

bool OCIntegerSet::Contains(uint32_t val)
{
	size_t min = 0;
	size_t max = size;
	size_t mid;

	while (min < max) {
		mid = (min + max)/2;

		uint32_t item = list[mid];
		if (item == val) return true;
		if (item < val) {
			min = mid+1;
		} else {
			max = mid;
		}
	}
	return false;
}

/*	OCIntegerSet::Add
 *
 *		Add a single item.
 */

void OCIntegerSet::Add(uint32_t val)
{
	/*
	 *	Is this already contained? If so, skip the rest. We do a containment
	 *	test first, as that is O(log(N)), and we'd have to run a scan of
	 *	the table twice at O(N) in order to implicitly determine if the item
	 *	was contained.
	 */

	if (Contains(val)) return;

	/*
	 *	First, make sure we can add by resizing.
	 */

	if (alloc == size) {
		/*
		 *	Grow our memory pool
		 */

		size_t newAlloc = (alloc + 16) & ~15;	// next 16 object boundary
		uint32_t *tmp = (uint32_t *)realloc(list,sizeof(uint32_t) * newAlloc);
		if (tmp == NULL) throw std::bad_alloc();

		alloc = newAlloc;
		list = tmp;
	}

	/*
	 *	Now run the list from the top to insert
	 */

	bool insert = false;
	for (size_t off = size; off > 0; --off) {
		uint32_t tmp = list[off-1];
		if (tmp > val) {
			list[off] = tmp;
		} else {
			list[off] = val;
			insert = true;
			break;
		}
	}
	if (!insert) {
		list[0] = val;
	}
	++size;
}

/*	OCIntegerSet::AddSet
 *
 *		Add set. This folds two lists together in linear time
 */

void OCIntegerSet::AddSet(const OCIntegerSet &set)
{
	/*
	 *	First, make sure we have enough for the worst case scenario of
	 *	two lists
	 */

	size_t reqSize = set.size + size;
	if (reqSize > alloc) {
		/*
		 *	Grow our memory pool
		 */

		reqSize = (reqSize + 16) & ~15;		// nearest 16 byte boundary
		uint32_t *tmp = (uint32_t *)realloc(list,sizeof(uint32_t) * reqSize);
		if (tmp == NULL) throw std::bad_alloc();

		alloc = reqSize;
		list = tmp;
	}

	/*
	 *	Now we start folding the two lists together from the top down.
	 */

	size_t d = alloc;
	size_t a = size;
	size_t b = set.size;

	while ((a > 0) && (b > 0)) {
		uint32_t av = list[a-1];
		uint32_t bv = set.list[b-1];

		if (av == bv) {
			list[--d] = av;
			--a;
			--b;
		} else if (av < bv) {
			list[--d] = bv;
			--b;
		} else {
			list[--d] = av;
			--a;
		}
	}
	while (a > 0) {
		list[--d] = list[--a];
	}

	size = alloc - d;
	if (d > 0) {
		memmove(list, list + d, size * sizeof(uint32_t));
	}
}

/*	OCIntegerSet::Remove
 *
 *		Remove a single item
 */

void OCIntegerSet::Remove(uint32_t val)
{
	if (!Contains(val)) return;

	// Find the index of the item to remove
	size_t a = 0;
	while (a < size) {
		if (list[a] == val) break;
		++a;
	}

	// Compress the rest of the list
	while (a < size) {
		list[a] = list[a+1];
		++a;
	}
	--size;
}

/*	OCIntegerSet::RemoveSet
 *
 *		Remove the set
 */

void OCIntegerSet::RemoveSet(const OCIntegerSet &set)
{
	size_t a = 0;
	size_t b = 0;
	size_t p = 0;

	/*
	 *	Copy, but drop items that are in both.
	 */

	while ((a < size) && (b < set.size)) {
		uint32_t av = list[a];
		uint32_t bv = set.list[b];

		if (av == bv) {
			++a;
			++b;
		} else if (av < bv) {
			list[p++] = list[a++];
		} else {
			++b;
		}
	}

	/*
	 *	Copy the rest
	 */

	while (a < size) {
		list[p++] = list[a++];
	}

	size = p;
}


/************************************************************************/
/*																		*/
/*	Debug Support														*/
/*																		*/
/************************************************************************/

/*	OCIntegerSet::ToString
 *
 *		Format as a string
 */

std::string OCIntegerSet::ToString()
{
	std::string str;

	str.push_back('[');
	for (size_t i = 0; i < size; ++i) {
		if (i) str.push_back(',');

		char buffer[256];
		sprintf(buffer,"%u",list[i]);
		str.append(buffer);
	}
	str.push_back(']');

	return str;
}

