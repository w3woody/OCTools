//
//  OCYaccLALR.cpp
//  ocyacc
//
//  Created by William Woody on 8/5/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCYaccLALR.h"

/************************************************************************/
/*																		*/
/*	Internal Strutures													*/
/*																		*/
/************************************************************************/


/************************************************************************/
/*																		*/
/*	Construction														*/
/*																		*/
/************************************************************************/

/*	OCYaccLALR::OCYaccLALR
 *
 *		Construct
 */

OCYaccLALR::OCYaccLALR()
{
}

/*	OCYaccLALR::~OCYaccLALR
 *
 *		Destruct
 */

OCYaccLALR::~OCYaccLALR()
{
}

/************************************************************************/
/*																		*/
/*	Support																*/
/*																		*/
/************************************************************************/


/*	OCYaccLALR::First
 *
 *		Implement the First() algorithm to find the first tokens in the
 *	grammar list gl.
 */

std::set<std::string> OCYaccLALR::First(std::vector<std::string> gl) const
{
	std::set<std::string> ret;

	if (gl.size() == 0) {
		ret.insert("$end");
		return ret;
	}
	if (productions.find(gl[0]) == productions.end()) {
		// front of gl is a token.
		ret.insert(gl[0]);
		return ret;
	}

	std::set<std::string> v;
	std::vector<std::string> q;
	v.insert(gl[0]);
	q.push_back(gl[0]);

	/*
	 *	Iterate through all found production rules and find their initial
	 *	tokens. We assume none of our rules are empty.
	 */

	while (!q.empty()) {
		std::string production = q.back();
		q.pop_back();

		size_t i,len = grammar.size();
		for (i = 0; i < len; ++i) {
			const Rule &r = grammar[i];
			if (production == r.production) {
				std::string g = r.tokenlist[0];
				if (productions.find(g) != productions.end()) {
					// g is a production rule.
					if (v.find(g) == v.end()) {
						q.push_back(g);
						v.insert(g);
					}
				} else {
					// g is a token.
					ret.insert(g);
				}
			}
		}
	}

	return ret;
}

/*	OCYaccLALR::Closure
 *
 *		Close the item set. Given an item set, this closes the set by
 *	finding all the production rules that are triggered by this
 *	transition, tracking the terminal which follows each item set as we go.
 *
 *		Rather than be tricky we simply do the LR(1) closure using a structure
 *	that looks like the LR(1) item set, but then we roll it back up into an
 *	LALR-style item set.
 */

void OCYaccLALR::Closure(ItemSet &set) const
{
	/*
	 *	Private structure.
	 */

	struct ItemP {
		size_t rule;
		size_t pos;
		std::string follow;

		// For storing in map and set
		bool operator == (const ItemP &set) const
			{
				return (rule == set.rule) && (pos == set.pos) && (follow == set.follow);
			}
		bool operator != (const ItemP &set) const
			{
				return (rule != set.rule) || (pos != set.pos) || (follow != set.follow);
			}
		bool operator < (const ItemP &set) const
			{
				if (rule < set.rule) return true;
				if (rule > set.rule) return false;
				if (pos < set.pos) return true;
				if (pos > set.pos) return false;
				if (follow < set.follow) return true;
				if (follow > set.follow) return false;
				return false;
			}
	};

	std::vector<ItemP> queue;
	std::set<ItemP> inside;		// Our "visited" doubles as our item set proxy

	/*
	 *	Runroll into our LR(0) style format
	 */

	std::set<Item>::iterator setiter;
	for (setiter = set.items.begin(); setiter != set.items.end(); ++setiter) {
		std::set<std::string>::iterator fiter;
		for (fiter = setiter->follow.begin(); fiter != setiter->follow.end(); ++fiter) {
			ItemP itemp;
			itemp.rule = setiter->rule;
			itemp.pos = setiter->pos;
			itemp.follow = *fiter;

			queue.push_back(itemp);
			inside.insert(itemp);
		}
	}

	/*
	 *	Now run through the queue, adding items
	 */

	while (!queue.empty()) {
		ItemP item = queue.back();
		queue.pop_back();

		/*
		 *	Determine if the item's next token is a production
		 */

		const Rule &r = grammar[item.rule];
		if (item.pos >= r.tokenlist.size()) continue;
		std::string p = r.tokenlist[item.pos];
		if (productions.find(p) != productions.end()) {
			/*
			 *	Next item is a production. First, find First()
			 */

			std::vector<std::string> gl;
			for (size_t i = item.pos+1; i < r.tokenlist.size(); ++i) {
				gl.push_back(r.tokenlist[i]);
			}
			gl.push_back(item.follow);

			std::set<std::string> f = First(gl);

			/*
			 *	Now construct the new item sets for each token
			 */

			std::set<std::string>::iterator fiter;
			for (fiter = f.begin(); fiter != f.end(); ++fiter) {

				/*
				 *	Iterate all matching rules
				 */

				size_t i,len = grammar.size();
				for (i = 0; i < len; ++i) {
					const Rule &pr = grammar[i];
					if (pr.production == p) {
						/*
						 *	Construct new item
						 */

						ItemP newItem;

						newItem.rule = i;
						newItem.pos = 0;
						newItem.follow = *fiter;

						if (inside.find(newItem) == inside.end()) {
							/* Not inside. */
							inside.insert(newItem);
							queue.push_back(newItem);
						}
					}
				}
			}
		}
	}

	/*
	 *	At this point we've calculated the LR(1) style closure. Roll up
	 *	into our LALR-style structure. This really is a kludge.
	 */

	set.items.clear();
	std::map<Item,std::set<std::string>> m;
	std::set<ItemP>::iterator siter;
	for (siter = inside.begin(); siter != inside.end(); ++siter) {
		Item item;
		item.rule = siter->rule;
		item.pos = siter->pos;
		m[item].insert(siter->follow);
	}
	std::map<Item,std::set<std::string>>::iterator mi;
	for (mi = m.begin(); mi != m.end(); ++mi) {
		Item item;
		item.rule = mi->first.rule;
		item.pos = mi->first.pos;
		item.follow = mi->second;
		set.items.insert(item);
	}
}

/*	OCYaccLALR::BuildItemSets
 *
 *		Build the item sets, as well as the translation table.
 */

void OCYaccLALR::BuildItemSets()
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
	item.follow.insert("$end");
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
				item.follow = iter->follow;

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

/*	OCYaccLALR::PropagateFollow
 *
 *		Propagate the follow lists
 */

void OCYaccLALR::PropagateFollow()
{
	// Set all item sets as needing processing
	size_t i,len = itemSets.size();
	bool change = true;

	for (i = 0; i < len; ++i) {
		itemSets[i].followMark = true;
	}

	// Repeat until our list is empty
	while (change) {
		change = false;

		for (i = 0; i < len; ++i) {
			ItemSet &iset = itemSets[i];
			if (!iset.followMark) continue;
			iset.followMark = false;

			// The item set at i has been updated, so push the follow sets
			// using the propagate follow algorithm
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
					item.follow = iter->follow;

					newSets[token].items.insert(item);
				}
			}

			// At this point we have a list of grammar symbols which map
			// to a list of item sets. Now find the transitions that were
			// noted in our prior build of the state machine, and update the
			// follow lists.

			std::map<std::string,ItemSet>::iterator niter;
			for (niter = newSets.begin(); niter != newSets.end(); ++niter) {
				if (niter->second.items.empty()) continue;

				ItemSet nset = niter->second;
				Closure(nset);

				/*
				 *	Run through the items in our item set, and merge the
				 *	follow lists.. Note we are guaranteed that the transition
				 *	exists (by construction), and the transition is a shift
				 *	transition to the state t.value.
				 *
				 *	To make this easy we use a map from items to the set of
				 *	terminals.
				 */

				Transition t = trans[i][niter->first];
				if (t.value == i) continue;		// Don't merge into myself.

				ItemSet merge = itemSets[t.value];
				std::map<Item,std::set<std::string>> itemFollowMap;

				std::set<Item>::iterator ni;
				for (ni = merge.items.begin(); ni != merge.items.end(); ++ni) {
					itemFollowMap[*ni] = ni->follow;
				}

				for (ni = nset.items.begin(); ni != nset.items.end(); ++ni) {
					/*
					 *	Merge the items in ni into our follow map.
					 */

					std::set<std::string>::iterator giter;
					for (giter = ni->follow.begin(); giter != ni->follow.end(); ++giter) {
						itemFollowMap[*ni].insert(*giter);
					}
				}

				/*
				 *	Now determine if anything in my item set changed. If so,
				 *	mark as changed, update the follow list in my item set
				 */

				bool updateMerge = false;
				std::map<Item,std::set<std::string>>::iterator ifmi;
				for (ifmi = itemFollowMap.begin(); ifmi != itemFollowMap.end(); ++ifmi) {
					// By construction each key in our follow map contains
					// the old list. See if the size of the new list is
					// bigger.
					if (ifmi->first.follow.size() < ifmi->second.size()) {
						change = true;
						updateMerge = true;

						// Update merge
						Item newItem;
						newItem.rule = ifmi->first.rule;
						newItem.pos = ifmi->first.pos;
						newItem.follow = ifmi->second;

						ni = merge.items.find(ifmi->first);
						merge.items.erase(ni);
						merge.items.insert(newItem);
					}
				}

				/*
				 *	Update our destination item set if we need to
				 */

				if (updateMerge) {
					merge.followMark = true;
					itemSets[t.value] = merge;		// replace with new object
				}
			}
		}
	}
}


/*	OCYaccLALR::DebugPrintItemSet
 *
 *		Debug item; prints the item set
 */

void OCYaccLALR::DebugPrintItemSet(const ItemSet &set) const
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

		printf("\t\t{ ");
		std::set<std::string>::iterator siter;
		for (siter = iter->follow.begin(); siter != iter->follow.end(); ++siter) {
			printf("%s ",siter->c_str());
		}
		printf("}\n");
	}
}

/************************************************************************/
/*																		*/
/*	Construction Entry Point											*/
/*																		*/
/************************************************************************/

/*	OCYaccLALR::Construct
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

bool OCYaccLALR::Construct(OCYaccParser &p)
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

	printf("\nIntermediate state machine\n");
	len = itemSets.size();
	for (i = 0; i < len; ++i) {
		ItemSet &s = itemSets[i];
		DebugPrintItemSet(s);
	}

	/*
	 *	Step 2: Finish propagating the following token maps
	 */

	PropagateFollow();

	/*	DEBUG: Print productions */
	/*
	 *		Dump the kernel and translation table
	 */

	printf("\nFinal State Machine\n");
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
	 *	Step 2: Forward the .
	 */


	return false;
}
