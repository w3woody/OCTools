//
//  OCYaccLR1.h
//  ocyacc
//
//  Created by William Woody on 8/10/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#ifndef OCYaccLR1_h
#define OCYaccLR1_h

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

/*	OCYaccLR1
 *
 *		Construct the LR1 state tables from the input grammar. This builds
 *	the accept table, the goto table, the final state, and the error
 *	state tables for our LR1 style parser.
 */

class OCYaccLR1
{
	public:
		OCYaccLR1();
		~OCYaccLR1();

		/*
		 *	Verbose information
		 */

		enum Verbose {
			None,
			Warning,
			Information
		};

		void SetVerboseLevel(Verbose level)
			{
				verboseLevel = level;
			}

		/*
		 *	Construct LR1 tables and values below from our input 
		 *	Returns false if there was an error.
		 */

		bool Construct(OCYaccParser &p);

		/*
		 *	Action encoding for our action table
		 */

		struct Action {
			bool reduce;					// true if reduce, false if shift
			size_t value;					// shift: state, reduce: rule
		};

		/*
		 *	Reduction encoding
		 */

		struct Reduction {
			size_t reduce;					// Number of states to pop
			uint32_t production;			// Production code we reduce to
			std::string prodDebug;			// For debugging
			std::string code;				// Code to execute (with $$,$n)
		};

		/*
		 *	This is the publicly available values after our construction
		 *	process runs. These are the values we will eventually write out
		 *	to our Objective-C file.
		 */

		// Compressed Action Table
		std::vector<size_t> actionI;		// State index -> offset in J
		std::vector<uint32_t> actionJ;		// token list to A
		std::vector<Action> actionA;		// shift or reduce action

		// Compressed GOTO table
		std::vector<size_t> gotoI;			// State index -> offset in J
		std::vector<uint32_t> gotoJ;		// production to A
		std::vector<size_t> gotoA;			// new state in I

		// Accept state
		size_t accept;						// Accept state.

		// Special token values
		uint32_t eofTokenID;				// -1 turns into this for errors
		uint32_t errorTokenID;				// This is for errors
		uint32_t firstTokenID;				// First non-Unicode printable token
		uint32_t maxSymbolID;				// All tokens, prods < this value

		// Rule reductions
		std::vector<Reduction> reductions;	// How to reduce by rule N

		/*
		 *	Token constants
		 */

		struct TokenConstant
		{
			bool used;						// set if used in grammar
			uint32_t value;
			std::string token;
		};
		std::vector<TokenConstant> tokens;	// Write these as #defines.

	private:
		Verbose verboseLevel = None;

		/*
		 *	Token/production map
		 *
		 *		This maps tokens and productions to and from their integer
		 *	values. We use integers to represent tokens and productions for
		 *	efficiency purposes.
		 */

		uint32_t	maxToken;		// tokens are < this value; productions >=

		std::map<uint32_t,std::string> tokenMap;

		uint32_t	TokenForChar(std::string str);

		/*
		 *	Rules: represents a single production rule A -> B C D ... code
		 */

		struct Rule {
			uint32_t production;
			std::vector<uint32_t> tokenlist;
			std::string prodName;					// for error reporting
			OCYaccParser::FilePos filePos;			// for error reporting
			OCYaccParser::Precedence precedence;
		};

		/*
		 *	Translated grammar
		 */

		std::vector<Rule> grammar;

		bool		BuildGrammar(OCYaccParser &p);

		/*
		 *	State Machine Representation
		 */

		/*	Item
		 *
		 *		An item is a rule (represented by index) and an index into the
		 *	set of tokens that represent the rule.
		 */

		struct Item
		{
			size_t rule;
			size_t pos;
			uint32_t follow;		// follow grammar symbol

			// For storing in map and set
			bool operator == (const Item &set) const
				{
					return (rule == set.rule) && (pos == set.pos) && (follow == set.follow);
				}
			bool operator != (const Item &set) const
				{
					return (rule != set.rule) || (pos != set.pos) || (follow != set.follow);
				}
			bool operator < (const Item &set) const
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

		/*	ItemSet
		 *
		 *		An item set is a set of items.
		 */

		struct ItemSet
		{
			size_t index;			// == index in itemSets.
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

		/*
		 *	itemSets: the states S of our LR(1) state machine.
		 */

		std::vector<ItemSet> itemSets;		// Item sets

		/*
		 *	trans: the transitions T of our LR(1) state machine.
		 *
		 *	Note: trans[state_index][grammar symbol] -> new state index
		 *
		 *	Also note that our action and goto table construction algorithms
		 *	rely on the fact that std::map sorts the order of the keys in
		 *	ascending order.
		 */

		std::map<size_t,std::map<uint32_t,size_t>> trans;

		/*
		 *	State machine construction support
		 */

		std::set<uint32_t> First(const std::vector<uint32_t> &gl) const;
		void Closure(ItemSet &set) const;
		void BuildStateMachine();

		/*
		 *	Goto/action table construction
		 */

		void BuildGotoTable();
		bool BuildActionTable(const OCYaccParser &parser);
		void FindAcceptState();
};

#endif /* OCYaccLR1_h */
