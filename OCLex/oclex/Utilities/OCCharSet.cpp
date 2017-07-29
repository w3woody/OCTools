//
//  OCCharSet.cpp
//  oclex
//
//  Created by William Woody on 7/26/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCCharSet.h"
#include <string.h>

/************************************************************************/
/*																		*/
/*	Manipulation														*/
/*																		*/
/************************************************************************/

/*	OCCharSet::Clear
 *
 *		Clear
 */

void OCCharSet::Clear()
{
	memset(a,0,sizeof(a));
}

/*	OCCharSet::Invert
 *
 *		Negate bit array
 */

void OCCharSet::Invert()
{
	for (int i = 0; i < 8; ++i) {
		a[i] = ~a[i];
	}
}

/*	OCCharSet::ClearCharacter
 *
 *		Clear character
 */

void OCCharSet::ClearCharacter(unsigned char ch)
{
	a[0x07 & (ch >> 5)] &= ~(1UL << (0x1F & ch));
}

/*	OCCharSet::SetCharacter
 *
 *		Set character
 */

void OCCharSet::SetCharacter(unsigned char ch)
{
	a[0x07 & (ch >> 5)] |= 1UL << (0x1F & ch);
}

/*	OCCharSet::TestCharacter
 *
 *		Teset if character is set
 */

bool OCCharSet::TestCharacter(unsigned char ch)
{
	return	(a[0x07 & (ch >> 5)] & 1UL << (0x1F & ch)) ? true : false;
}

/************************************************************************/
/*																		*/
/*	Containment															*/
/*																		*/
/************************************************************************/

/*	OCCharSet::Contains
 *
 *		Returns true if the item passed in is equal to, or the items are
 *	contained in, the current item.
 *
 *		In essence this does the following operation:
 *
 *		self	set		value	-->  ~(~self & set)
 *		0		0		1
 *		0		1		0
 *		1		0		1
 *		1		1		1
 */

bool OCCharSet::Contains(const OCCharSet &set) const
{
	for (int i = 0; i < 8; ++i) {
		if (~a[i] & set.a[i]) return false;
	}
	return true;
}

/*	OCCharSet::IsEmpty
 *
 *		Is this empty?
 */

bool OCCharSet::IsEmpty() const
{
	for (int i = 0; i < 8; ++i) {
		if (a[i]) return false;
	}
	return true;
}

/************************************************************************/
/*																		*/
/*	Compare																*/
/*																		*/
/************************************************************************/

/*	OCCharSet::operator ==
 *
 *		Are the two equal?
 */

bool OCCharSet::operator == (const OCCharSet &set) const
{
	for (int i = 0; i < 8; ++i) {
		if (a[i] != set.a[i]) return false;
	}
	return true;
}

/*	OCCharSet::operator <
 *
 *		Compare for using in map?
 */

bool OCCharSet::operator < (const OCCharSet &set) const
{
	for (int i = 0; i < 8; ++i) {
		if (a[i] < set.a[i]) return true;
		if (a[i] > set.a[i]) return false;
	}
	return false;
}

/************************************************************************/
/*																		*/
/*	Math																*/
/*																		*/
/************************************************************************/

/*	OCCharSet::operator &=
 *
 *		And and return result
 */

OCCharSet &OCCharSet::operator &= (const OCCharSet &cset)
{
	for (int i = 0; i < 8; ++i) {
		a[i] &= cset.a[i];
	}

	return *this;
}

/*	OCCharSet::operator |=
 *
 *		And and return result
 */

OCCharSet &OCCharSet::operator |= (const OCCharSet &cset)
{
	for (int i = 0; i < 8; ++i) {
		a[i] |= cset.a[i];
	}

	return *this;
}

/*	OCCharSet::operator -=
 *
 *		Return the set of characters in the source with the characters in
 *	the destination removed.
 *
 *		Equal to
 *
 *			a = a & ~b
 */

OCCharSet &OCCharSet::operator -= (const OCCharSet &cset)
{
	for (int i = 0; i < 8; ++i) {
		a[i] &= ~cset.a[i];
	}

	return *this;
}


/************************************************************************/
/*																		*/
/*	Binary operators													*/
/*																		*/
/************************************************************************/

OCCharSet OCCharSet::operator & (const OCCharSet &b) const
{
	OCCharSet s;

	for (int i = 0; i < 8; ++i) {
		s.a[i] = a[i] & b.a[i];
	}
	return s;
}

OCCharSet OCCharSet::operator | (const OCCharSet &b) const
{
	OCCharSet s;

	for (int i = 0; i < 8; ++i) {
		s.a[i] = a[i] | b.a[i];
	}
	return s;
}

OCCharSet OCCharSet::operator ^ (const OCCharSet &b) const
{
	OCCharSet s;

	for (int i = 0; i < 8; ++i) {
		s.a[i] = a[i] ^ b.a[i];
	}
	return s;
}

OCCharSet OCCharSet::operator ~ () const
{
	OCCharSet s = *this;
	s.Invert();
	return s;
}

