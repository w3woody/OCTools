//
//  OCYaccTestLALR.h
//  ocyacc
//
//  Created by William Woody on 8/5/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCYaccTestLALR_h
#define OCYaccTestLALR_h

#include <stdio.h>
#include <set>
#include <vector>
#include <string>

#include "OCYaccParser.h"

/************************************************************************/
/*																		*/
/*	Class Declarations													*/
/*																		*/
/************************************************************************/

/*	OCYaccTestLALR
 *
 *		Construct the LALR state tables from the input grammar
 */

class OCYaccTestLALR
{
	public:
		OCYaccTestLALR();
		~OCYaccTestLALR();

		// Construct LALR tables; return false if error.
		bool Construct(OCYaccParser &p);


	private:
		/*
		 *	Rule: represents a single production rule
		 *	A -> B C D ... code
		 */

		struct Rule {
			std::string production;
			std::vector<std::string> tokenlist;
			std::string code;
		};

		/*	Item
		 *
		 *		An item is a rule (represented by index) and an index into the
		 *	set of tokens that represent the rule.
		 */

		struct Item
		{
			size_t rule;
			size_t pos;
			std::set<std::string> follow;

			// For storing in map and set
			bool operator == (const Item &set) const
				{
					return (rule == set.rule) && (pos == set.pos);
				}
			bool operator != (const Item &set) const
				{
					return (rule != set.rule) || (pos != set.pos);
				}
			bool operator < (const Item &set) const
				{
					if (rule < set.rule) return true;
					if (rule > set.rule) return false;
					if (pos < set.pos) return true;
					return false;
				}
		};

		/*	ItemSet
		 *
		 *		An item set is a set of items.
		 */

		struct ItemSet
		{
			bool followMark;
			size_t index;
			std::set<Item> items;

			// For storing in map and set
			bool operator == (const ItemSet &set) const
				{
					if (items.size() != set.items.size()) return false;

					std::set<Item>::const_iterator aiter,biter;
					aiter = items.cbegin();
					biter = set.items.cbegin();

					while ((aiter != items.cend()) && (biter != set.items.cend())) {
						if (*aiter != *biter) return false;
						++aiter;
						++biter;
					}
					return true;
				}
			bool operator < (const ItemSet &set) const
				{
					std::set<Item>::const_iterator aiter,biter;

					aiter = items.cbegin();
					biter = set.items.cbegin();

					while ((aiter != items.cend()) && (biter != set.items.cend())) {
						if (*aiter < *biter) return true;
						if (*aiter != *biter) return false;
						++aiter;
						++biter;
					}
					if ((aiter == items.cend()) && (biter != set.items.cend())) {
						/* If first is empty and second is not, then less */
						return true;
					}
					return false;
				}
		};

		/*	Transition
		 *
		 *		Transition structure
		 */

		struct Transition
		{
			bool accept;			// accept
			bool shift;				// true if shift
			size_t value;			// shift new state, or reduction rule
		};


		std::vector<Rule> grammar;
		std::set<std::string> productions;	// Productions
		std::vector<ItemSet> itemSets;		// Item sets
		std::map<size_t,std::map<std::string,Transition>> trans; // src: term->dst

		std::set<std::string> First(std::vector<std::string> gl) const;
		void Closure(ItemSet &set) const;
		void BuildItemSets();
		void PropagateFollow();

		void DebugPrintItemSet(const ItemSet &set) const;
};

#endif /* OCYaccTestLALR_h */
