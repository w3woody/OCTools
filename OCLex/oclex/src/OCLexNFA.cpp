//
//  OCLexNFA.cpp
//  oclex
//
//  Created by William Woody on 7/26/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCLexNFA.h"

/****

Lex-style regular expressions definition from 
http://dinosaur.compilertools.net/lex/index.html

               x        the character "x"
               "x"      an "x", even if x is an operator.
               \x       an "x", even if x is an operator.
               [xy]     the character x or y.
               [x-z]    the characters x, y or z.
               [^x]     any character but x.
               .        any character but newline.
               ^x       an x at the beginning of a line.
               <y>x     an x when Lex is in start condition y.
               x$       an x at the end of a line.
               x?       an optional x.
               x*       0,1,2, ... instances of x.
               x+       1,2,3, ... instances of x.
               x|y      an x or a y.
               (x)      an x.
 ns            x/y      an x but only if followed by y.
               {xx}     the translation of xx from the
                        definitions section.
 ns            x{m,n}   m through n occurrences of x

****/

/************************************************************************/
/*																		*/
/*	Construct NFA from regular expression								*/
/*																		*/
/************************************************************************/

static int ToHex(char ch)
{
	if ((ch >= '0') && (ch <= '9')) return ch - '0';
	if ((ch >= 'a') && (ch <= 'f')) return ch - 'a' + 10;
	if ((ch >= 'A') && (ch <= 'F')) return ch - 'A' + 10;
	return 0;
}

/**
 *	Grab the character after a '\', and translate into the appropriate
 *	escaped character. regex should point to the character after the '\'.
 */

char OCLexNFA::EscapeCharacter(const char * &regex)
{
	char ch = *regex++;

	if (ch == 'a') return 0x07;
	else if (ch == 'b') return 0x08;
	else if (ch == 'f') return 0x0C;
	else if (ch == 'n') return 0x0A;
	else if (ch == 'r') return 0x0D;
	else if (ch == 't') return 0x09;
	else if (ch == 'v') return 0x0B;
	else if (ch == 'e') return 0x1B;
	else if ((ch == 'x') || (ch == 'X')) {
		char ret = 0;
		while (isxdigit(ch = *regex)) {
			ret = (ret << 4) | ToHex(ch);
			++regex;
		}
		return ret;
	} else if ((ch >= '0') && (ch <= '7')) {
		char ret = ch - '0';
		while ((ch >= '0') && (ch <= '7')) {
			ret = (ret << 3) | (ch - '0');
			++regex;
		}
		return ret;
	} else {
		return ch;
	}
}

/*
 *	Alloc new state, return index
 */

uint32_t OCLexNFA::NewState()
{
	uint32_t index = (uint32_t)states.size();

	OCLexNFAState state;
	states.push_back(state);

	return index;
}

/*
 *	Grab the contents of a string
 */

OCLexNFAReturn OCLexNFA::ConstructString(const char * &regex)
{
	OCLexNFAReturn ret;

	ret.start = NewState();
	ret.end = ret.start;

	++regex;		// skip opening '"'
	for (;;) {
		if ((*regex == 0) || (*regex == '"')) break;

		char ch;
		if (*regex == '\\') {
			++regex;

			ch = EscapeCharacter(regex);
		} else {
			ch = *regex++;
		}

		/*
		 *	Add transition to new end state to existing end
		 */

		uint32_t e = NewState();
		OCLexNFATransition t;

		t.set.SetCharacter(ch);
		t.state = e;
		states[ret.end].list.push_back(t);
		ret.end = e;
	}

	if (*regex == '"') ++regex;
	return ret;
}

/*
 *	Construct a transition representing the character set
 */

OCLexNFAReturn OCLexNFA::ConstructCharSet(const char * &regex)
{
	bool atStart = true;
	bool invert = false;
	char lastChar = 0;

	/*
	 *	This is a little different. Scan forward and set the characters
	 *	in the character sets, returning a transition
	 */

	OCLexNFATransition t;

	++regex;
	for (;;) {
		if (*regex == 0) break;
		if (!atStart && (*regex == ']')) break;

		if (atStart && (*regex == '^')) {
			invert = true;
			++regex;
		} else if (lastChar && (*regex == '-')) {
			// Range from last char to next char
			char nextChar;

			nextChar = *++regex;

			// Fringe case: -], we count the ] as a close. If you want
			// to run the range to ], insert \] instead.
			if (nextChar == ']') {
				atStart = false;
				lastChar = '-';
				t.set.SetCharacter(lastChar);
				continue;
			}

			if (nextChar == '\\') {
				++regex;
				nextChar = EscapeCharacter(regex);
			}
			if (nextChar == 0) break;

			unsigned char startIter = (unsigned char)lastChar;
			unsigned char endIter = (unsigned char)nextChar;
			if (startIter > endIter) {
				startIter = (unsigned char)nextChar;
				endIter = (unsigned char)lastChar;
			}

			for (unsigned char i = startIter; i <= endIter; ++i) {
				t.set.SetCharacter(i);
			}

			++regex;
			lastChar = 0;

		} else if (*regex == '\\') {
			atStart = false;
			++regex;
			lastChar = EscapeCharacter(regex);
			t.set.SetCharacter(lastChar);

		} else {
			// Set character

			atStart = false;
			lastChar = *regex++;
			t.set.SetCharacter(lastChar);
		}
	}
	if (*regex == ']') ++regex;

	if (invert) {
		t.set.Invert();
	}

	/*
	 *	Construct transition
	 */

	OCLexNFAReturn ret;

	ret.start = NewState();
	ret.end = NewState();
	t.state = ret.end;
	states[ret.start].list.push_back(t);

	return ret;
}

OCLexNFAReturn OCLexNFA::ConstructDefinition(const char * &regex)
{
	std::string defname;

	++regex;			// skip '{'
	for (;;) {
		if ((*regex == 0) || (*regex == '}')) break;
		defname.push_back(*regex++);
	}

	if (*regex == '}') ++regex;

	/*
	 *	Now look up the regular expression for our definition
	 */

	if (definitions.end() == definitions.find(defname)) {
		fprintf(stderr,"Definition %s in regular expression not defined\n",defname.c_str());

		/*
		 *	Return empty sequence
		 */

		OCLexNFAReturn ret;
		OCLexNFATransition t;

		ret.start = NewState();
		ret.end = NewState();
		t.state = ret.end;
		t.e = true;
		states[ret.start].list.push_back(t);

		return ret;
	} else {
		/*
		 *	Determine if we are already in this definition. If we are, we
		 *	are in a loop
		 */

		if (inDefinition.find(defname) != inDefinition.end()) {
			fprintf(stderr,"Definition %s creates an infinite loop of definitions",defname.c_str());
			fprintf(stderr,"Fatal. Halting");
			exit(1);
		}


		/*
		 *	Recursively parse the embedded regular expression, tracking those
		 *	definitions we've already hit.
		 */

		inDefinition.insert(defname);

		const char *str = definitions[defname].c_str();
		OCLexNFAReturn ret = Construct(str);

		inDefinition.erase(defname);

		return ret;
	}
}

/*	OCConstruct
 *
 *		Internal construct. This takes a pointer to the regex string; this
 *	allows me to scan forwards
 */

OCLexNFAReturn OCLexNFA::Construct(const char * &regex)
{
	OCLexNFAReturn ret;
	OCLexNFAReturn last;
	bool hasLast = false;

	ret.start = NewState();
	ret.end = ret.start;

	if ((*regex == ')') || (*regex == 0)) {
		/*
		 *	Degenerate case; is empty. Return empty transition. This returns
		 *
		 *		START ---e---> END
		 */

		OCLexNFATransition t;

		ret.end = NewState();
		t.e = true;
		t.state = ret.end;
		states[ret.start].list.push_back(t);

	} else for (;;) {
		/*
		 *	Start reading the string left to right. Parse special
		 *	characters
		 */

		if ((*regex == ')') || (*regex == 0)) break;

		/*
		 *	Handle *, ?, + postfix
		 */

		if (hasLast && (*regex == '*')) {
			/*
			 *	0..N of the last expression. Connect for skip and repeat
			 */

			OCLexNFATransition t;
			t.e = true;
			t.state = last.end;
			states[last.start].list.push_back(t);

			t.state = last.start;
			states[last.end].list.push_back(t);

			++regex;
			continue;

		} else if (hasLast && (*regex == '+')) {
			/*
			 *	1..N of the last expression. Connect for skip
			 */

			OCLexNFATransition t;
			t.e = true;
			t.state = last.start;
			states[last.end].list.push_back(t);

			++regex;
			continue;
		} else if (hasLast && (*regex == '?')) {
			/*
			 *	0..1 of the last expression. Connect for skip
			 */

			OCLexNFATransition t;
			t.e = true;
			t.state = last.end;
			states[last.start].list.push_back(t);

			++regex;
			continue;
		}

		if (*regex == '|') {
			/*
			 *	Handle | operator. This recurses the right side, then
			 *	connects the two sides. Note that this will collapse
			 *	all postfix operators, so we only need to glue the
			 *	two sides together.
			 *
			 *	Also note that this will complete this subexpression, so
			 *	after we handle this, we stop parsing for this level
			 */

			++regex;
			if (ret.start == ret.end) {
				/*
				 *	Degenerate case: naked '|'. Turn the following into
				 *	the same as (xxx)?
				 */

				ret = Construct(regex);

				/*
				 *	Construct empty skip state
				 */

				OCLexNFATransition t;
				t.e = true;
				t.state = ret.end;
				states[ret.start].list.push_back(t);

			} else {
				/*
				 *	Parse RHS
				 */

				OCLexNFATransition t;
				OCLexNFAReturn lhs = ret;
				last = Construct(regex);

				/*
				 *	Now construct pre/post states
				 */

				ret.start = NewState();
				ret.end = NewState();

				t.e = true;
				t.state = lhs.start;
				states[ret.start].list.push_back(t);

				t.e = true;
				t.state = last.start;
				states[ret.start].list.push_back(t);

				t.e = true;
				t.state = ret.end;
				states[lhs.end].list.push_back(t);

				t.e = true;
				t.state = ret.end;
				states[last.end].list.push_back(t);
			}
			break;
		}

		/*
		 *	Handle subexpression
		 */

		if (*regex == '(') {
			// Recurse and concatenate the return value
			++regex;
			last = Construct(regex);
			hasLast = true;
			if (*regex == ')') ++regex;	// skip end

		} else if (*regex == '\"') {
			// Handle string.
			last = ConstructString(regex);
			hasLast = true;

		} else if (*regex == '[') {
			last = ConstructCharSet(regex);
			hasLast = true;

		} else if (*regex == '{') {
			last = ConstructDefinition(regex);
			hasLast = true;

		} else {
			/*
			 *	At this point we're simply adding the following character
			 *	to the list of states
			 */

			bool any = false;
			char ch;
			if (*regex == '\\') {
				++regex;			// skip and escape next character
				ch = EscapeCharacter(regex);
			} else if (*regex == '.') {
				ch = 0;		// special marker
				any = true;
				++regex;
			} else {
				ch = *regex++;
			}


			uint32_t e = NewState();
			OCLexNFATransition t;

			if (any) {
				t.set.Invert();
			} else {
				t.set.SetCharacter(ch);
			}

			/*
			 *	Add transition to new end state to existing end
			 */

			last.start = ret.end;	// Track last object transition for
			last.end = e;			// singleton operators
			hasLast = true;

			t.state = e;
			states[ret.end].list.push_back(t);
			ret.end = e;

			continue;
		}

		/*
		 *	If we get here, we have a last object we need to glue to the
		 *	current ret object. We glue together with an empty transition
		 *	rather than try to sort out the new start state we're concatenating
		 */

		OCLexNFATransition t;
		t.e = true;
		t.state = last.start;
		states[ret.end].list.push_back(t);
		ret.end = last.end;
	}

	return ret;
}

