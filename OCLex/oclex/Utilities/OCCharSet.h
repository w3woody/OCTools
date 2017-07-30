//
//  OCCharSet.h
//  oclex
//
//  Created by William Woody on 7/26/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCCharSet_h
#define OCCharSet_h

#include <stdint.h>
#include <string.h>
#include <string>

/************************************************************************/
/*																		*/
/*	Character set														*/
/*																		*/
/************************************************************************/

/*	OCCharSet
 *
 *		Character set manipulation. Note that this assumes 256-byte
 *	characters; we don't support wide characters.
 */

class OCCharSet
{
	public:
		OCCharSet()
			{
				memset(a,0,sizeof(a));
			}

		~OCCharSet()
			{
			}

		void Clear();
		void Invert();
		void ClearCharacter(unsigned char ch);
		void SetCharacter(unsigned char ch);
		bool TestCharacter(unsigned char ch) const;

		OCCharSet &operator &= (const OCCharSet &cset);
		OCCharSet &operator |= (const OCCharSet &cset);
		OCCharSet &operator -= (const OCCharSet &cset);

		bool operator == (const OCCharSet &set) const;
		bool operator < (const OCCharSet &set) const;	// for use in map

		bool Contains(const OCCharSet &set) const;
		bool IsEmpty() const;

		OCCharSet operator & (const OCCharSet &b) const;
		OCCharSet operator | (const OCCharSet &b) const;
		OCCharSet operator ^ (const OCCharSet &b) const;
		OCCharSet operator ~ () const;

		std::string ToString() const;

	private:
		uint32_t a[8];
};


#endif /* OCCharSet_h */
