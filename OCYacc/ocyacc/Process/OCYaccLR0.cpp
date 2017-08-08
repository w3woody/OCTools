//
//  OCYaccLR0.cpp
//  ocyacc
//
//  Created by William Woody on 8/5/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCYaccLR0.h"

/************************************************************************/
/*																		*/
/*	Construction														*/
/*																		*/
/************************************************************************/

/*	OCYaccLR0::OCYaccLR0
 *
 *		Construct
 */

OCYaccLR0::OCYaccLR0()
{
}

/*	OCYaccLR0::~OCYaccLR0
 *
 *		Destruct
 */

OCYaccLR0::~OCYaccLR0()
{
}

/************************************************************************/
/*																		*/
/*	Support																*/
/*																		*/
/************************************************************************/

/*	OCYaccLR0::Closure
 *
 *		Close the item set. Given an item set, this closes the set by
 *	finding all the production rules that are triggered by this
 *	transition
 */

void OCYaccLR0::Closure(ItemSet &set) const
{
	std::vector<std::string> queue;
	std::set<std::string> inside;

	/*
	 *	Scan the set of items in set, and populate q with the list of
	 *	productions that may be added.
	 */

	std::set<Item>::iterator iter;
	for (iter = set.items.begin(); iter != set.items.end(); ++iter) {
		const Rule &r = grammar[iter->rule];
		if (iter->pos < r.tokenlist.size()) {
			std::string prod = r.tokenlist[iter->pos];

			if (productions.find(prod) != productions.end()) {
				/*
				 *	This is a production rule, so iterate the entire list and
				 *	add the appropriate items
				 */

				queue.push_back(prod);
				inside.insert(prod);
			}
		}
	}

	/*
	 *	Now run through the queue, adding items
	 */

	while (!queue.empty()) {
		std::string production = queue.back();
		queue.pop_back();

		/*
		 *	Add the rules that are in this production
		 */

		size_t i,len = grammar.size();
		for (i = 0; i < len; ++i) {
			const Rule &r = grammar[i];
			if (production == r.production) {
				/*
				 *	Construct the new item
				 */

				Item item;
				item.rule = i;
				item.pos = 0;
				set.items.insert(item);

				/*
				 *	Grab the first production of this rule and insert
				 */

				if (r.tokenlist.size() > 0) {
					std::string newprod = r.tokenlist[0];

					if (productions.find(newprod) != productions.end()) {
						// newprod is a production.

						if (inside.find(newprod) == inside.end()) {
							// We haven't scanned this yet, add to queue
							queue.push_back(newprod);
							inside.insert(newprod);
						}
					}
				}
			}
		}
	}
}

/*	OCYaccLR0::BuildItemSets
 *
 *		Build the item sets, as well as the translation table.
 */

void OCYaccLR0::BuildItemSets()
{
	std::vector<ItemSet> queue;
	std::set<ItemSet> inside;
	size_t index = 0;

	/*
	 *	Build i0
	 */

	Item item;
	item.rule = 0;
	item.pos = 0;
	ItemSet iset;
	iset.index = index++;
	iset.items.insert(item);
	Closure(iset);
	itemSets.push_back(iset);

	/*
	 *	Add i0 to the queue
	 */

	queue.push_back(iset);
	inside.insert(iset);

	/*
	 *	Now run through the items in the queue until empty
	 */

	while (!queue.empty()) {
		iset = queue.back();
		queue.pop_back();

		/*
		 *	Iterate through all of the production rules, building a map from
		 *	the transition token and the item sets which increment through
		 *	each rule set.
		 */

		std::map<std::string,ItemSet> newSets;
		std::set<Item>::iterator iter;

		for (iter = iset.items.begin(); iter != iset.items.end(); ++iter) {
			Rule &r = grammar[iter->rule];
			if (iter->pos < r.tokenlist.size()) {
				/*
				 *	Get the token we're transitioning through
				 */

				std::string token = r.tokenlist[iter->pos];

				/*
				 *	Generate the item representing the current item passed
				 *	through the token
				 */

				Item item;
				item.rule = iter->rule;
				item.pos = iter->pos+1;

				newSets[token].items.insert(item);
			}
		}

		/*
		 *	Iterate through all of the constructed items and close them.
		 *	Then insert them into the queue if they haven't been reached
		 *	yet.
		 */

		std::map<std::string,ItemSet>::iterator m;
		for (m = newSets.begin(); m != newSets.end(); ++m) {
			Closure(m->second);
			size_t destIndex;

			/*
			 *	If this set is not in the queue, number and add
			 */

			std::set<ItemSet>::iterator iter = inside.find(m->second);
			if (iter == inside.end()) {
				// Insert the transition destination into the queue
				m->second.index = index++;
				queue.push_back(m->second);
				itemSets.push_back(m->second);
				inside.insert(m->second);

				destIndex = m->second.index;
			} else {
				// Already have this. Grab the index of the item set
				// inserted into our list of item sets
				destIndex = iter->index;
			}

			/*
			 *	Insert the transition
			 */

			Transition t;
			t.accept = false;
			t.shift = true;
			t.value = destIndex;
			trans[iset.index][m->first] = t;
		}
	}
}

/*	OCYaccLR0::DebugPrintItemSet
 *
 *		Debug item; prints the item set
 */

void OCYaccLR0::DebugPrintItemSet(const ItemSet &set) const
{
	/*
	 *	Run through and print the rules with dot notation
	 */

	printf("Item Set %zu\n",set.index);
	std::set<Item>::const_iterator iter;
	for (iter = set.items.cbegin(); iter != set.items.cend(); ++iter) {
		const Rule &r = grammar[iter->rule];

		printf("  %s :",r.production.c_str());
		size_t i,len = r.tokenlist.size();
		for (i = 0; i < len; ++i) {
			if (i == iter->pos) {
				printf(" .");
			}
			printf(" %s",r.tokenlist[i].c_str());
		}
		if (iter->pos == len) {
			printf(" .");
		}
		printf("\n");
	}
}

/************************************************************************/
/*																		*/
/*	Construction Entry Point											*/
/*																		*/
/************************************************************************/

/*	OCYaccLR0::ConstructLALR
 *
 *		Given the parser contents, construct the LALR parse table. If there
 *	is a problem this returns false.
 *
 *		Much of the algorithm is pulled from the Dragon Book, with a side
 *	venture into the following web site:
 *
 *		https://web.cs.dal.ca/~sjackson/lalr1.html
 *
 *		We also borrow some hints from Bison:
 *
 *		https://www.cs.uic.edu/~spopuri/cparser.html
 */

bool OCYaccLR0::Construct(OCYaccParser &p)
{
	/*
	 *	Reset
	 */

	grammar.clear();
	productions.clear();

	/*
	 *	Step 0: Unroll the compressed format for the production rules, and
	 *	insert an artifical production rule $accept: startrule. The top
	 *	rule is necessary as part of the rule production process
	 *
	 *	We also borrow a play from the Bison playbook and insert a new
	 *	start rule $accept: startrule $end
	 *
	 *	The start rule required by LALR into our grammar is inserted by
	 *	using an empty string.
	 */

	/*
	 *	Rule 0: . -> $accept
	 */

	Rule r;
//	r.production = "";
//	r.tokenlist.push_back("$accept");
//	r.code = "";
//
//	grammar.push_back(r);

	/*
	 *	Rule 0: $accept -> startsymbol $end
	 */

	r.production = "$accept";
	r.tokenlist.clear();
	r.tokenlist.push_back(p.startSymbol);
	r.tokenlist.push_back("$end");		// $end is an artificial terminal

	grammar.push_back(r);

	productions.insert("$accept");

	std::map<std::string,OCYaccParser::SymbolDecl>::iterator miter;
	for (miter = p.symbols.begin(); miter != p.symbols.end(); ++miter) {
		productions.insert(miter->first);

		/*
		 *	Iterate the declaration
		 */

		std::vector<OCYaccParser::SymbolInstance>::iterator viter;
		for (viter = miter->second.declarations.begin(); viter != miter->second.declarations.end(); ++viter) {
			r.production = miter->first;
			r.tokenlist = viter->tokenlist;
			r.code = viter->code;

			grammar.push_back(r);
		}
	}

	/*	DEBUG: Print productions */
	printf("Rules\n");
	size_t i,len = grammar.size();
	for (i = 0; i < len; ++i) {
		Rule &r = grammar[i];
		printf("%zu. %s :",i,r.production.c_str());
		size_t j,jlen = r.tokenlist.size();
		for (j = 0; j < jlen; ++j) {
			printf(" %s",r.tokenlist[j].c_str());
		}
		printf("\n");
	}
	printf("\n");
	/* END DEBUG */

	/*
	 *	Step 1: Build the item sets and translation tables. This is basically
	 *	the first step in building the LR(0) grammar.
	 */

	BuildItemSets();

	/*	DEBUG: Print productions */
	/*
	 *		Dump the kernel and translation table
	 */

	len = itemSets.size();
	for (i = 0; i < len; ++i) {
		ItemSet &s = itemSets[i];
		DebugPrintItemSet(s);
	}

	printf("\nTranslation Table\n");
	std::set<std::string> states;
	std::map<size_t,std::map<std::string,Transition>>::iterator titer;
	std::map<std::string,Transition>::iterator siter;
	size_t max = 0;
	for (titer = trans.begin(); titer != trans.end(); ++titer) {
		if (titer->first > max) max = titer->first;
		for (siter = titer->second.begin(); siter != titer->second.end(); ++siter) {
			states.insert(siter->first);
		}
	}
	std::set<std::string>::iterator iiter;
	printf("-\t");
	for (iiter = states.begin(); iiter != states.end(); ++iiter) {
		printf("%s\t",iiter->c_str());
	}
	printf("\n");
	for (i = 0; i <= max; ++i) {
		printf("%zu\t",i);
		std::map<std::string,Transition> &t = trans[i];
		for (iiter = states.begin(); iiter != states.end(); ++iiter) {
			if (t.find(*iiter) != t.end()) {
				Transition &tra = t[*iiter];
				printf("%zu\t",tra.value);
			} else {
				printf(".\t");
			}
		}
		printf("\n");
	}
	printf("\n");
	/* END DEBUG */

	/*
	 *	Step 2: Build the extended grammar.
	 */


	return false;
}
