//
//  OCYaccLR1.cpp
//  ocyacc
//
//  Created by William Woody on 8/10/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCYaccLR1.h"

/************************************************************************/
/*																		*/
/*	Constants															*/
/*																		*/
/************************************************************************/

#define FIRSTTOKEN		0x110000		/* Beyond last Unicode code point */

/************************************************************************/
/*																		*/
/*	Construction														*/
/*																		*/
/************************************************************************/

/*	OCYaccLR1::OCYaccLR1
 *
 *		Construct
 */

OCYaccLR1::OCYaccLR1()
{
}

/*	OCYaccLR1::~OCYaccLR1
 *
 *		Destruct
 */

OCYaccLR1::~OCYaccLR1()
{
}


/************************************************************************/
/*																		*/
/*	Support																*/
/*																		*/
/************************************************************************/

/*	ToHex
 *
 *		Convert character to hex
 */

static int ToHex(char ch)
{
	if ((ch >= '0') && (ch <= '9')) return ch - '0';
	if ((ch >= 'a') && (ch <= 'f')) return ch - 'a' + 10;
	if ((ch >= 'A') && (ch <= 'F')) return ch - 'A' + 10;
	return 0;
}

/*	OCYaccLR1::TokenForChar
 *
 *		Convert the character in the string to a token. This handles
 *	UTF-8 character sequences as well as escapes
 */

uint32_t OCYaccLR1::TokenForChar(std::string str)
{
	uint32_t ch = 0;

	// Delete the single quotes surrounding our character.
	str = str.substr(1,str.size()-2);
	const uint8_t *s = (uint8_t *)str.c_str();

	// Parse the character. We don't do as much error checking as we could.
	if (*s == '\\') {
		++s;
		if (*s == 'a') return 0x07;		// \a
		if (*s == 'b') return 0x08;		// \b
		if (*s == 'f') return 0x0C;
		if (*s == 'n') return 0x0A;
		if (*s == 'r') return 0x0D;
		if (*s == 't') return 0x09;
		if (*s == 'v') return 0x0B;
		if (*s == 'e') return 0x1B;		// escape

		if ((*s == 'x') || (*s == 'u') || (*s == 'U')) {
			// hex
			++s;
			while (*s) {
				ch = (ch << 4) | ToHex(*s);
			}
			return ch;
		} else if ((*s >= '0') && (*s <= '7')) {
			// octal
			while (*s) {
				ch = (ch << 3) | ToHex(*s);
			}
			return ch;
		} else {
			return *s;
		}
	} else {
		/*
		 *	Parse as UTF-8. Note the spec defines code points from 
		 *	0x10000 to 0x10FFFF in two different ways; we try to handle
		 *	both encodings. (This is where emoticons live.)
		 *
		 *	Note that we don't do error checking. The result is that we
		 *	could theoretically crash, or at least produce an invalid
		 *	sequence.
		 */

		if (*s < 0x80) return *s;

		// 0x80-0xBF is illegal. We punt and pass through
		if (*s < 0xC0) return *s;

		// Handle 2 byte encodings
		if (*s < 0xE0) {
			ch = 0x1F & *s++;
			ch = (ch << 6) | (0x3F & *s++);
			return ch;
		}

		// Three byte encodings.
		if (*s < 0xF0) {
			ch = 0x0F & *s++;
			ch = (ch << 6) | (0x3F & *s++);
			ch = (ch << 6) | (0x3F & *s++);

			if ((ch >= 0xD800) && (ch < 0xE000)) {
				// Pull second surrigate and treat as two UTF-16 characters
				// for a final Unicode encoding
				uint32_t ch2 = 0x0F & *s++;
				ch2 = (ch2 << 6) | (0x3F & *s++);
				ch2 = (ch2 << 6) | (0x3F & *s++);

				if (ch < ch2) {
					ch = ((ch & 0x03FF) << 10) | (ch2 & 0x03FF);
				} else {
					ch = ((ch2 & 0x03FF) << 10) | (ch & 0x03FF);
				}
			}
			return ch;
		}

		// Four byte encoding
		ch = 0x07 & *s++;
		ch = (ch << 6) | (0x3F & *s++);
		ch = (ch << 6) | (0x3F & *s++);
		ch = (ch << 6) | (0x3F & *s++);

		return ch;
	}
}

/*	OCYaccLR1::BuildGrammar
 *
 *		Construct the grammar state machine and eventually build the
 *	various state tables
 */

bool OCYaccLR1::BuildGrammar(OCYaccParser &p)
{
	bool valid = true;

	// grammarMap maps a token or production string to its integer value.
	std::map<std::string,uint32_t>	grammarMap;

	// prods tracks which grammar symbols are productions
	std::set<std::string> prods;

	// Undefined terminals
	std::set<std::string> undefined;

	/*
	 *	Reset
	 */

	grammar.clear();

	/*
	 *	Convert the tokens and productions in our grammar to integer values
	 *	for faster processing. This also allows us to write our rules and
	 *	statements out quickly.
	 *
	 *	We also predefine the tokens $end and error.
	 *
	 *	We use sparse matrix techniques for reducing the size of our tables, 
	 *	so the only effect this has is requiring us to use 32-bit values for 
	 *	our tokens.
	 *
	 *	Note that OCLex can (sorta) deal with Unicode characters. Or rather,
	 *	we deal with everything as UTF-8, which means our lexer handles a
	 *	thing like a smily face as a match of a bunch of 8-bit bytes. This
	 *	means we can ask for a smily face as an input symbol, but we cannot
	 *	ask for a range of emoticons. Eventually this will be fixed...
	 */

	uint32_t index = FIRSTTOKEN;			// Beyond last Unicode code point.

	/*
	 *	Construct a list of productions. Anything that is not a production is
	 *	assumed to be a token
	 */

	std::map<std::string,OCYaccParser::SymbolDecl>::iterator miter;
	for (miter = p.symbols.begin(); miter != p.symbols.end(); ++miter) {
		prods.insert(miter->first);

		if (p.terminalSymbol.find(miter->first) != p.terminalSymbol.end()) {
			printf("Production %s is defined as a token\n",miter->first.c_str());
			valid = false;
		}
	}

	/*
	 *	Insert our reserved tokens $end and error. Note we do not define
	 *	an '$undefined' token, since we use sparse compression, thus big
	 *	gaps in the token ID don't matter.
	 *
	 *	Also note that the order in which we construct things matters, as
	 *	it is used to test token values and productions below.
	 */

	tokenList.push_back("$end");
	grammarMap["$end"] = index++;
	tokenList.push_back("error");
	grammarMap["error"] = index++;

	/*
	 *	Iterate all the rules, extracting tokens. Note that single quote
	 *	characters are treated differently
	 */

	for (miter = p.symbols.begin(); miter != p.symbols.end(); ++miter) {

		/*
		 *	Iterate the declaration
		 */

		std::vector<OCYaccParser::SymbolInstance>::iterator viter;
		for (viter = miter->second.declarations.begin(); viter != miter->second.declarations.end(); ++viter) {

			/*
			 *	Iterate the symbols
			 */

			std::vector<std::string>::iterator i;
			for (i = viter->tokenlist.begin(); i != viter->tokenlist.end(); ++i) {
				/*
				 *	Skip productions.
				 */

				if (prods.find(*i) == prods.end()) {
					/*
					 *	Now determine if we have already processed the found
					 *	token. If not, add to our map.
					 */

					if (grammarMap.end() == grammarMap.find(*i)) {
						if ((*i)[0] == '\'') {
							/* A character is its unicode value */
							/* We store in our map but not in our array */
							grammarMap[*i] = TokenForChar(*i);
						} else {
							/* We create a new entry for our token */
							tokenList.push_back(*i);
							grammarMap[*i] = index;

							/* Insert token information */
							TokenConstant tc;
							tc.used = true;
							tc.value = index;
							tc.token = *i;
							tokens.push_back(tc);

							++index;

							/*
							 *	Determine if this item was defined in our lexer.
							 *	If not, print a warning
							 */

							if (p.terminalSymbol.find(*i) == p.terminalSymbol.end()) {
								if (undefined.find(*i) == undefined.end()) {
									fprintf(stderr,"Found undefined symbol %s in grammar; assuming token.\n",i->c_str());
									undefined.insert(*i);
								}
							}
						}
					}
				}
			}
		}
	}

	/*
	 *	Now iterate through all our declared tokens; that is, tokens
	 *	declared in our parser in %token, %left, %right or %nonassoc. Note
	 *	that because we scan our entire grammar, we assume any tokens
	 *	not declared as a %token or a production is a token.
	 */

	std::map<std::string,OCYaccParser::Precedence>::iterator liter;
	for (liter = p.terminalSymbol.begin(); liter != p.terminalSymbol.end(); ++liter) {
		if (liter->first[0] == '\'') continue;	// skip characters

		if (grammarMap.find(liter->first) == grammarMap.end()) {
			// We have a token that was not defined
			tokenList.push_back(liter->first);
			grammarMap[liter->first] = index;

			TokenConstant tc;
			tc.used = false;
			tc.value = index;
			tc.token = liter->first;
			tokens.push_back(tc);

			++index;
		}
	}

	maxToken = index;

	/*
	 *	Now set the max token value and iterate to assign values to our
	 *	productions. Note that our productions are not duplicated by 
	 *	construction.
	 *
	 *	We also add an "$accept" production before the rest.
	 */

	productionList.push_back("$accept");
	grammarMap["$accept"] = index++;

	for (miter = p.symbols.begin(); miter != p.symbols.end(); ++miter) {
		const std::string &p = miter->first;
		grammarMap[p] = index++;
	}
	maxProduction = index;

	/*
	 *	Now iterate and build our rules using the found indexes so we can
	 *	run our LR(1) state machine algorithm. First, we insert our 
	 *	artifical start rule.
	 */

	if (prods.find(p.startSymbol) == prods.end()) {
		fprintf(stderr,"%%start symbol %s not found in rules. Halting\n",p.startSymbol.c_str());
		return false;
	}

	Rule r;
	r.production = maxToken;			// $accept; maxToken == first production
	r.tokenlist.push_back(grammarMap[p.startSymbol]);
	r.tokenlist.push_back(FIRSTTOKEN);	// FIRSTTOKEN == $end by definition
	grammar.push_back(r);

	/*
	 *	Iterate the rest of our symbols and declarations
	 */

	for (miter = p.symbols.begin(); miter != p.symbols.end(); ++miter) {

		/*
		 *	Iterate the declaration and create our rules
		 */

		std::vector<OCYaccParser::SymbolInstance>::iterator viter;
		for (viter = miter->second.declarations.begin(); viter != miter->second.declarations.end(); ++viter) {
			std::vector<std::string>::iterator i;

			r.production = grammarMap[miter->first];
			r.code = viter->code;
			r.precedence = viter->precedence;

			r.tokenlist.clear();
			for (i = viter->tokenlist.begin(); i != viter->tokenlist.end(); ++i) {
				r.tokenlist.push_back(grammarMap[*i]);
			}

			grammar.push_back(r);
		}
	}

	return valid;
}

/************************************************************************/
/*																		*/
/*	State Machine Construction											*/
/*																		*/
/************************************************************************/

/*
 *	Note: by the way we construct our grammar symbol map, a few relationships
 *	hold true and are used in the code below.
 *
 *		$end == FIRSTTOKEN
 *		error == FIRSTTOKEN + 1
 *
 *		A grammar symbol g is a token if g < maxToken
 *		A grammar symbol g is a production if g >= maxToken
 */

/*	OCYaccLR1::First
 *
 *		Execute the first() algorithm on our list of grammar symbols
 */

std::set<uint32_t> OCYaccLR1::First(const std::vector<uint32_t> &gl) const
{
	std::set<uint32_t> ret;

	if (gl.size() == 0) {
		ret.insert(FIRSTTOKEN);		// $end token == FIRSTTOKEN
		return ret;
	}
	if (gl[0] < maxToken) {
		// front of gl is a token.
		ret.insert(gl[0]);
		return ret;
	}

	std::set<uint32_t> v;
	std::vector<uint32_t> q;
	v.insert(gl[0]);
	q.push_back(gl[0]);

	/*
	 *	Iterate through all found production rules and find their initial
	 *	tokens. We assume none of our rules are empty.
	 */

	while (!q.empty()) {
		uint32_t production = q.back();
		q.pop_back();

		size_t i,len = grammar.size();
		for (i = 0; i < len; ++i) {
			const Rule &r = grammar[i];
			if (production == r.production) {
				uint32_t g = r.tokenlist[0];
				if (g >= maxToken) {
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

/*	OCYaccLR1::Closure
 *
 *		Close the item set. Given an item set, this closes the set by
 *	finding all the production rules that are triggered by this
 *	transition, tracking the terminal which follows each item set as we go.
 */

void OCYaccLR1::Closure(ItemSet &set) const
{
	std::vector<Item> queue;
	std::set<Item> inside;

	/*
	 *	Add our current set of items to the queue and to inside.
	 */

	std::set<Item>::iterator iter;
	for (iter = set.items.begin(); iter != set.items.end(); ++iter) {
		queue.push_back(*iter);
		inside.insert(*iter);
	}

	/*
	 *	Now run through the queue, adding items
	 */

	while (!queue.empty()) {
		Item item = queue.back();
		queue.pop_back();

		/*
		 *	Determine if the item's next token is a production
		 */

		const Rule &r = grammar[item.rule];
		if (item.pos >= r.tokenlist.size()) continue;
		uint32_t p = r.tokenlist[item.pos];
		if (p >= maxToken) {
			/*
			 *	Next item is a production. First, find First()
			 */

			std::vector<uint32_t> gl;
			for (size_t i = item.pos+1; i < r.tokenlist.size(); ++i) {
				gl.push_back(r.tokenlist[i]);
			}
			gl.push_back(item.follow);

			std::set<uint32_t> f = First(gl);

			/*
			 *	Now construct the new item sets for each token
			 */

			std::set<uint32_t>::iterator fiter;
			for (fiter = f.begin(); fiter != f.end(); ++fiter) {

				/*
				 *	Iterate all matching rules
				 */

				size_t i,len = grammar.size();
				for (i = 0; i < len; ++i) {
					const Rule &pr = grammar[i];
					if (pr.production == p) {
						/*
						 *	Construct new item with position at start
						 */

						Item newItem;

						newItem.rule = i;
						newItem.pos = 0;
						newItem.follow = *fiter;

						set.items.insert(newItem);

						if (inside.insert(newItem).second) {
							// second is true if new item was inserted;
							// that is, if it wasn't there before.
							queue.push_back(newItem);
						}
//						if (inside.find(newItem) == inside.end()) {
//							/* Not inside. */
//							inside.insert(newItem);
//							queue.push_back(newItem);
//						}
					}
				}
			}
		}
	}
}

/*	OCYaccLR1::BuildStateMachine
 *
 *		Build the state machine using Knuth's LR(1) algorithm.
 */

void OCYaccLR1::BuildStateMachine()
{
	std::vector<ItemSet> queue;
	std::set<ItemSet> inside;
	size_t index = 0;

	/*
	 *	Build i0. This is derived from the first rule (rule 0), and is
	 *	followed by the end symbol
	 */

	Item item;
	item.rule = 0;
	item.pos = 0;
	item.follow = FIRSTTOKEN;		// $end == FIRSTTOKEN
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

		std::map<uint32_t,ItemSet> newSets;
		std::set<Item>::iterator iter;

		for (iter = iset.items.begin(); iter != iset.items.end(); ++iter) {
			Rule &r = grammar[iter->rule];
			if (iter->pos < r.tokenlist.size()) {
				/*
				 *	Get the token we're transitioning through
				 */

				uint32_t grammarSymbol = r.tokenlist[iter->pos];

				/*
				 *	Generate the item representing the current item passed
				 *	through the token
				 */

				Item item;
				item.rule = iter->rule;
				item.pos = iter->pos+1;
				item.follow = iter->follow;

				newSets[grammarSymbol].items.insert(item);
			}
		}

		/*
		 *	Iterate through all of the constructed items and close them.
		 *	Then insert them into the queue if they haven't been reached
		 *	yet.
		 */

		std::map<uint32_t,ItemSet>::iterator m;
		for (m = newSets.begin(); m != newSets.end(); ++m) {
			size_t destIndex;

			Closure(m->second);

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

			trans[iset.index][m->first] = destIndex;
		}
	}
}

/************************************************************************/
/*																		*/
/*	Goto table construction												*/
/*																		*/
/************************************************************************/

/*	OCYaccLR1::BuildGotoTable
 *
 *		Build the goto table. The goto table is a sparse table with the
 *	colunns the productions in our state machine, and the rows the
 *	states. Each entry represents a transition from one state to another
 *	as we migrate through productions. We use the Compressed sparse row
 *	format to compress the table; see:
 *
 *		https://en.wikipedia.org/wiki/Sparse_matrix
 */

void OCYaccLR1::BuildGotoTable()
{
	std::map<size_t,std::map<uint32_t,size_t>>::const_iterator i;
	size_t s = 0;
	size_t curState = 0;

	gotoA.clear();
	gotoI.clear();
	gotoJ.clear();

	/*
	 *	Start populating IA. The first index is always zero. Note that our
	 *	trans map may have gaps in states (this is typical), so we need
	 *	to iterate through curState until we find the state we've
	 *	iterated to
	 */

	gotoI.push_back(s);
	for (i = trans.cbegin(); i != trans.cend(); ++i) {
		while (curState++ < i->first) {
			gotoI.push_back(s);
		}

		// Now process this row.
		const std::map<uint32_t,size_t> &m = i->second;
		std::map<uint32_t,size_t>::const_iterator j;
		for (j = m.cbegin(); j != m.cend(); ++j) {
			if (j->first >= maxToken) {
				// j->first is a production.
				gotoJ.push_back(j->first);
				gotoA.push_back(j->second);
				++s;
			}
		}
		gotoI.push_back(s);
	}

	size_t len = itemSets.size();
	while (curState++ < len) {
		gotoI.push_back(s);
	}
}

/*	OCYaccLR1::BuildActionTable
 *
 *		Build the action table. This is done like the goto table, but also
 *	the reduction rules will be added.
 */

bool OCYaccLR1::BuildActionTable()
{
	/*
	 *	We iterate through all the states, since all of them will have either
	 *	a goto or a reduce or both.
	 */

	size_t i, len = itemSets.size();
	for (i = 0; i < len; ++i) {
		std::map<uint32_t,size_t> row;

		/*
		 *	Insert goto transitions
		 */

		if (trans.find(i) != trans.end()) {
			const std::map<uint32_t,size_t> &m = trans[i];
			std::map<uint32_t,size_t>::const_iterator iter;
			for (iter = m.cbegin(); iter != m.cend(); ++iter) {
				if (iter->first < maxToken) {
					row[iter->first] = iter->second;
				}
			}
		}

		/*
		 *	Now walk the items in this item set looking for the
		 *	reduction rules
		 */

		const ItemSet &iset = itemSets[i];
		std::set<Item>::const_iterator iter;
		for (iter = iset.items.cbegin(); iter != iset.items.cend(); ++iter) {
			const Rule &r = grammar[iter->rule];

#warning FINISH ME (determine reductions, warn shift/reduce and reduce/reduce conflicts that cannot be resolved via precedence.)
		}


		/*
		 *	Get the goto table transitions
		 */


	}
	return false;
}

/************************************************************************/
/*																		*/
/*	Construction Entry Point											*/
/*																		*/
/************************************************************************/

/*	OCYaccLR1::Construct
 *
 *		Construct the grammar state machine and eventually build the
 *	various state tables
 */

bool OCYaccLR1::Construct(OCYaccParser &p)
{
	/*
	 *	Step 1: build token list, productions and rule set
	 */

	if (!BuildGrammar(p)) return false;

	/*
	 *	Step 2: Build the state machine
	 */

	BuildStateMachine();

	/*
	 *	Step 3: Construct compressed goto table
	 */

	BuildGotoTable();

	/*
	 *	Step 4: Construct action table
	 */

	if (!BuildActionTable()) return false;


	// TODO
	return false;
}
