//
//  OCLexDFA.cpp
//  oclex
//
//  Created by William Woody on 7/29/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCLexDFA.h"

#include <set>

/************************************************************************/
/*																		*/
/*	Add rule set														*/
/*																		*/
/************************************************************************/

/*	OCLexDFA::AddRuleSet
 *
 *		Add rule set
 */

void OCLexDFA::AddRuleSet(std::string regex, std::string code, bool atStart, bool atEnd)
{
	/*
	 *	Get the current code segment and the index to the code rule
	 */

	CodeRule c;
	c.code = code;
	c.atEnd = atEnd;

	uint32_t ruleIndex = (uint32_t)codeRules.size();
	codeRules.push_back(c);

	/*
	 *	Generate the NFA for this regular expression
	 */

	OCLexNFAReturn ret = AddRule(regex.c_str());

	/*
	 *	Now update the end state to mark it as translating to our code
	 *	segment. We use this during DFA generation
	 */

	OCLexNFAState &state = states[ret.end];
	state.end = true;
	state.endRule = ruleIndex;

	/*
	 *	And store the start state
	 */

	startStates.push_back(ret.start);
}

/************************************************************************/
/*																		*/
/*	DFA Support															*/
/*																		*/
/************************************************************************/

/*	OCLexDFA::EClosure
 *
 *		Calculate e-closure of the state set provided by walking through
 *	all the possible states reachable by an e move from the list of
 *	input states
 */

void OCLexDFA::EClosure(OCIntegerSet &set)
{
	std::set<uint32_t> visited;

	/*
	 *	This repeats until we've discovered the set of all states that
	 *	can be reached via e closure from the input state.
	 */

	for (;;) {
		/*
		 *	Find the first unvisited state in set
		 */

		uint32_t state = 0;
		bool found = false;
		size_t i,len = set.Size();
		for (i = 0; i < len; ++i) {
			state = set.Item(i);
			if (visited.end() == visited.find(state)) {
				// Item not found, so bounce
				found = true;
				break;
			}
		}

		if (!found) return;		// Finished closure of set.
		visited.insert(state);

		/*
		 *	Now visit all the e-reachable items for this state
		 */

		OCLexNFAState &s = states[state];
		std::list<OCLexNFATransition>::iterator iter;
		for (iter = s.list.begin(); iter != s.list.end(); ++iter) {
			if (iter->e) {
				/*
				 *	Add the state we can reach via an e transition
				 */

				set.Add(iter->state);
			}
		}
	}
}

/*	OCLexDFA::SplitCharSet
 *
 *		Split the character set. This looks at the list of character
 *	sets and splits everything apart until each character set does not
 *	contain the others, and the union of all character sets include all
 *	the characters of the supplied list
 */

void OCLexDFA::SplitCharSet(std::vector<OCCharSet> &set)
{
	if (set.size() <= 1) return;		// Nothing to split

	bool split;
	do {
		split = false;
		size_t len = set.size();
		for (size_t i = 0; (i < len) && !split; ++i) {
			for (size_t j = i+1; j < len; ++j) {
				/*
				 *	See if the item at i & j is non-zero.
				 */

				OCCharSet &a = set[i];
				OCCharSet &b = set[j];
				OCCharSet c = a & b;

				if (!c.IsEmpty()) {
					/*
					 *	a and b overlap. There can be two or three
					 *	valid non-overlapping items; if two, then one
					 *	of the following operations will hit zero.
					 */

					a -= c;
					b -= c;
					if (a.IsEmpty()) {
						set[i] = c;
						if (b.IsEmpty()) {
							// Degenerate and interesting case. We truncate
							// b, update a, and continue
							set.erase(set.begin() + j);
						}
					} else if (b.IsEmpty()) {
						set[j] = c;
					} else {
						set.push_back(c);
					}

					/*
					 *	We've now split a and b into non-overlapping
					 *	elements, so mark as split, escape to start over.
					 */

					split = true;
					break;
				}
			}
		}
	} while (split);
}

/************************************************************************/
/*																		*/
/*	DFA Construction													*/
/*																		*/
/************************************************************************/

/*	OCLexDFA::FindEndRule
 *
 *		Given the OCIntegerSet representing the NFA states that comprise
 *	this DFA state, this determines the DFA end rule (if one exists) and
 *	sets it. If an end-rule does not exist, no action is taken.
 *
 *		Note that this process is how we find the rule set which ties to
 *	the code to execute if we have a match, and this also implements the
 *	rule that the first found lex rule is the one we collapse to if there
 *	is a conflict.
 */

void OCLexDFA::FindEndRule(OCLexDFAState &state, const OCIntegerSet &set)
{
	bool found = false;
	uint32_t ruleIndex = 0;

	size_t i,len = set.Size();
	for (i = 0; i < len; ++i) {
		uint32_t nfaState = set.Item(i);

		const OCLexNFAState &s = states[nfaState];
		if (s.end) {
			if (!found) {
				ruleIndex = s.endRule;
				found = true;
			} else {
				if (ruleIndex > s.endRule) {
					ruleIndex = s.endRule;
				}
			}
		}
	}

	if (found) {
		state.end = true;
		state.endRule = ruleIndex;
	}
}

/*	OCLexDFA::GenerateDFA
 *
 *		Generate the DFA from the NFA using a powerset construction
 *	algorithm at:
 *
 *		https://en.wikipedia.org/wiki/Powerset_construction
 */

bool OCLexDFA::GenerateDFA()
{
	/*
	 *	Reset internal state
	 */

	dfaStates.clear();
	setMap.clear();

	/*
	 *	Set up the state queue
	 */

	std::list<OCIntegerSet> stateQueue;

	/*
	 *	Start: construct the set of start states that represent our start.
	 *	This is the list of start states we accumulated as we were building
	 *	the rules in AddRuleSet above
	 */

	OCIntegerSet start;
	size_t i,len = startStates.size();
	for (i = 0; i < len; ++i) {
		start.Add(startStates[i]);
	}
	EClosure(start);

	// Construct state representation and add to queue
	uint32_t stateID = (uint32_t)dfaStates.size();

	OCLexDFAState dfa;
	FindEndRule(dfa,start);
	dfaStates.push_back(dfa);

	if (dfa.end) {
		fprintf(stderr,"Warning: Lex rules contains a potentially empty regular expression");
	}

	setMap[start] = stateID;
	stateQueue.push_back(start);

	/*
	 *	Now run through all the states we encounter. At each point we 
	 *	find the list of possible transitions from this state set, then
	 *	calculate for each possible unique transition which states we
	 *	arrive at, enqueing them as needed.
	 */

	while (!stateQueue.empty()) {
		/*
		 *	Get the list of possible transitions from this state. This is
		 *	the list of all possible transitions from all the states within
		 *	the state object representation
		 */

		OCIntegerSet state = stateQueue.front();
		stateQueue.pop_front();

		stateID = setMap[state];

		/*
		 *	Build total list of transitions
		 */

		std::vector<OCCharSet> cset;
		std::vector<OCLexNFATransition> transitions;

		size_t i,len = state.Size();
		for (i = 0; i < len; ++i) {
			uint32_t nfaState = state.Item(i);
			std::list<OCLexNFATransition>::iterator iter;

			OCLexNFAState &nfa = states[nfaState];
			for (iter = nfa.list.begin(); iter != nfa.list.end(); ++iter) {
				if (!iter->e) {
					transitions.push_back(*iter);
					cset.push_back(iter->set);
				}
			}
		}

		SplitCharSet(cset);

		/*
		 *	At this point cset contains a list of all the possible unique
		 *	ways we can transition, and the transitions array contain all
		 *	the NFA transitions. Now we run through all the transition
		 *	states possible in cset, and derive the states we are
		 *	transitioning to
		 *
		 *	The intent is that if we have a character c, this will be
		 *	represented by a unique state OCLexDFAState.
		 */

		std::vector<OCCharSet>::iterator csetIter;
		for (csetIter = cset.begin(); csetIter != cset.end(); ++csetIter) {
			/*
			 *	Now run the list of transitions we found and stored in the
			 *	transition list. We're accumulating all NFA states that
			 *	cset moves to
			 */

			OCIntegerSet newState;

			std::vector<OCLexNFATransition>::iterator titer;
			for (titer = transitions.begin(); titer != transitions.end(); ++titer) {
				if (titer->set.Contains(*csetIter)) {
					newState.Add(titer->state);
				}
			}

			EClosure(newState);

			/*
			 *	Determine if this state we're transitioning to exists, and
			 *	if it does not, add it.
			 */

			int newStateID;
			if (setMap.end() == setMap.find(newState)) {
				newStateID = (uint32_t)dfaStates.size();

				OCLexDFAState dfa;
				FindEndRule(dfa,newState);
				dfaStates.push_back(dfa);

				setMap[newState] = newStateID;
				stateQueue.push_back(newState);
			} else {
				newStateID = setMap[newState];
			}

			/*
			 *	Now construct the DFA state transition to the new DFA
			 *	state in stateID.
			 */

			OCLexDFAState &dfaState = dfaStates[stateID];

			OCLexDFATransition t;
			t.state = newStateID;	// state we're going to
			t.set = *csetIter;		// transition which triggers new state
			dfaState.list.push_back(t);
		}
	}

	/*
	 *	At this point we've calculated the DFA. The last step, used when
	 *	generating the tables, is to build the character class set so
	 *	we can reduce the size of our output tables
	 */

	// Accumulate a unique list of all possible character transitions
	std::set<OCCharSet> charSet;
	std::vector<OCLexDFAState>::iterator dfaIter;
	for (dfaIter = dfaStates.begin(); dfaIter != dfaStates.end(); ++dfaIter) {
		std::vector<OCLexDFATransition>::iterator titer;
		for (titer = dfaIter->list.begin(); titer != dfaIter->list.end(); ++titer) {
			charSet.insert(titer->set);
		}
	}

	// Uniquely identify the sets
	charClasses.clear();
	std::set<OCCharSet>::iterator setIter;
	for (setIter = charSet.begin(); setIter != charSet.end(); ++setIter) {
		charClasses.push_back(*setIter);
	}

	SplitCharSet(charClasses);

	return true;
}
