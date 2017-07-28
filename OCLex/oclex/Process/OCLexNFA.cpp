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
 ns            ^x       an x at the beginning of a line.
 ns            <y>x     an x when Lex is in start condition y.
 ns            x$       an x at the end of a line.
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
/*	Forwards															*/
/*																		*/
/************************************************************************/

static OCLexNFA OCConstruct(OCAlloc &alloc, std::map<std::string,std::string> &definitions, const char * &regex);

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
 *	escaped character. regex should point ot the character after the '\'.
 */

static char OCEscapeCharacter(const char * &regex)
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
 *	Grab the contents of a string
 */

static OCLexNFA OCConstructString(OCAlloc &alloc, std::map<std::string,std::string> &definitions, const char * &regex)
{
	OCLexNFA ret;

	ret.start = (OCLexNFAState *)alloc.Alloc(sizeof(OCLexNFAState));
	ret.end = ret.start;		// for bookkeeping purposes

	++regex;		// skip opening '"'
	for (;;) {
		if ((*regex == 0) || (*regex == '"')) break;

		char ch;
		if (*regex == '\\') {
			++regex;

			ch = OCEscapeCharacter(regex);
		} else {
			ch = *regex++;
		}

		/*
		 *	Add transition to new end state to existing end
		 */

		OCLexNFAState *e = (OCLexNFAState *)alloc.Alloc(sizeof(OCLexNFAState));

		OCLexNFATransition *t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));

		t->set.SetCharacter(ch);

		t->state = e;
		t->next = ret.end->list;
		ret.end->list = t;
		ret.end = e;
	}

	if (*regex == '"') ++regex;
	return ret;
}

/*
 *	Construct a transition representing the character set
 */

static OCLexNFA OCConstructCharSet(OCAlloc &alloc, std::map<std::string,std::string> &definitions, const char * &regex)
{
	bool atStart = true;
	bool invert = false;
	char lastChar = 0;

	/*
	 *	This is a little different. Scan forward and set the characters
	 *	in the character sets, returning a transition
	 */

	OCLexNFATransition *t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));

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
			if (nextChar == '\\') {
				++regex;
				nextChar = OCEscapeCharacter(regex);
			}
			if (nextChar == 0) break;

			unsigned char startIter = (unsigned char)lastChar;
			unsigned char endIter = (unsigned char)nextChar;
			if (startIter > endIter) {
				startIter = (unsigned char)nextChar;
				endIter = (unsigned char)lastChar;
			}

			for (unsigned char i = startIter; i <= endIter; ++i) {
				t->set.SetCharacter(i);
			}

			++regex;
			lastChar = 0;

		} else if (*regex == '\\') {
			atStart = false;
			++regex;
			lastChar = OCEscapeCharacter(regex);
			t->set.SetCharacter(lastChar);

		} else {
			// Set character

			atStart = false;
			lastChar = *regex++;
		}
	}
	if (*regex == ']') ++regex;

	if (invert) {
		t->set.Invert();
	}

	/*
	 *	Construct transition
	 */

	OCLexNFA ret;
	ret.start = (OCLexNFAState *)alloc.Alloc(sizeof(OCLexNFAState));
	ret.end = (OCLexNFAState *)alloc.Alloc(sizeof(OCLexNFAState));
	t->state = ret.end;
	t->next = ret.start->list;
	ret.start->list = t;
	return ret;
}

static OCLexNFA OCConstructDefinition(OCAlloc &alloc, std::map<std::string,std::string> &definitions, const char * &regex)
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

		OCLexNFA ret;
		ret.start = (OCLexNFAState *)alloc.Alloc(sizeof(OCLexNFAState));
		ret.end = (OCLexNFAState *)alloc.Alloc(sizeof(OCLexNFAState));
		OCLexNFATransition *t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));
		t->e = true;
		t->state = ret.end;
		t->next = ret.start->list;
		ret.start->list = t;
		return ret;
	} else {
		/*
		 *	Recursively parse the embedded regular expression
		 */

		const char *str = definitions[defname].c_str();
		return OCConstruct(alloc,definitions,str);
	}
}

/*	OCConstruct
 *
 *		Internal construct. This takes a pointer to the regex string; this
 *	allows me to scan forwards
 */

static OCLexNFA OCConstruct(OCAlloc &alloc, std::map<std::string,std::string> &definitions, const char * &regex)
{
	OCLexNFA ret;
	OCLexNFA last;
	bool hasLast = false;

	ret.start = (OCLexNFAState *)alloc.Alloc(sizeof(OCLexNFAState));
	ret.end = ret.start;		// for bookkeeping purposes

	if ((*regex == ')') || (*regex == 0)) {
		/*
		 *	Degenerate case; is empty. Return empty transition. This returns
		 *
		 *		START ---e---> END
		 */

		ret.end = (OCLexNFAState *)alloc.Alloc(sizeof(OCLexNFAState));
		OCLexNFATransition *t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));
		t->e = true;
		t->state = ret.end;
		t->next = ret.start->list;
		ret.start->list = t;

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

			OCLexNFATransition *t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));
			t->e = true;
			t->state = last.end;
			t->next = last.start->list;
			last.start->list = t;

			t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));
			t->e = true;
			t->state = last.start;
			t->next = last.end->list;
			last.end->list = t;

			++regex;
			continue;

		} else if (hasLast && (*regex == '+')) {
			/*
			 *	1..N of the last expression. Connect for skip
			 */

			OCLexNFATransition *t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));
			t->e = true;
			t->state = last.start;
			t->next = last.end->list;
			last.end->list = t;

			++regex;
			continue;
		} else if (hasLast && (*regex == '?')) {
			/*
			 *	0..1 of the last expression. Connect for skip
			 */

			OCLexNFATransition *t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));
			t->e = true;
			t->state = last.end;
			t->next = last.start->list;
			last.start->list = t;

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

				ret = OCConstruct(alloc, definitions, regex);

				/*
				 *	Construct empty skip state
				 */

				OCLexNFATransition *t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));
				t->e = true;
				t->state = ret.end;
				t->next = ret.start->list;
				ret.start->list = t;
			} else {
				/*
				 *	Parse RHS
				 */

				OCLexNFATransition *t;
				OCLexNFA lhs = ret;
				last = OCConstruct(alloc, definitions, regex);

				/*
				 *	Now construct pre/post states
				 */

				ret.start = (OCLexNFAState *)alloc.Alloc(sizeof(OCLexNFAState));
				ret.end = (OCLexNFAState *)alloc.Alloc(sizeof(OCLexNFAState));

				t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));
				t->e = true;
				t->state = lhs.start;
				t->next = ret.start->list;
				ret.start->list = t;

				t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));
				t->e = true;
				t->state = last.start;
				t->next = ret.start->list;
				ret.start->list = t;

				t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));
				t->e = true;
				t->state = ret.end;
				t->next = lhs.end->list;
				lhs.end->list = t;

				t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));
				t->e = true;
				t->state = ret.end;
				t->next = last.end->list;
				last.end->list = t;
			}
			break;
		}

		/*
		 *	Handle subexpression
		 */

		if (*regex == '(') {
			// Recurse and concatenate the return value
			++regex;
			last = OCConstruct(alloc, definitions, regex);
			hasLast = true;
			if (*regex == ')') ++regex;	// skip end

		} else if (*regex == '\"') {
			// Handle string.
			last = OCConstructString(alloc, definitions, regex);
			hasLast = true;

		} else if (*regex == '[') {
			last = OCConstructCharSet(alloc, definitions, regex);
			hasLast = true;

		} else if (*regex == '{') {
			last = OCConstructDefinition(alloc, definitions, regex);
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
				ch = OCEscapeCharacter(regex);
			} else if (*regex == '.') {
				ch = 0;		// special marker
				any = true;
			} else {
				ch = *regex++;
			}

			OCLexNFAState *e = (OCLexNFAState *)alloc.Alloc(sizeof(OCLexNFAState));

			OCLexNFATransition *t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));

			if (any) {
			} else {
				t->set.SetCharacter(ch);
			}

			/*
			 *	Add transition to new end state to existing end
			 */

			last.start = ret.end;	// Track last object transition for
			last.end = e;			// singleton operators
			hasLast = true;

			t->state = e;
			t->next = ret.end->list;
			ret.end->list = t;
			ret.end = e;

			continue;
		}

		/*
		 *	If we get here, we have a last object we need to glue to the
		 *	current ret object. We glue together with an empty transition
		 *	rather than try to sort out the new start state we're concatenating
		 */

		OCLexNFATransition *t = (OCLexNFATransition *)alloc.Alloc(sizeof(OCLexNFATransition));
		t->e = true;
		t->state = last.start;
		t->next = ret.end->list;
		ret.end->list = t;
		ret.end = last.end;
	}

	return ret;
}

/*	OCConstructRule
 *
 *		Construct NFA rule from regular expression using Thompson's 
 *	Construction, https://en.wikipedia.org/wiki/Thompson%27s_construction
 */

OCLexNFA OCConstructRule(OCAlloc &alloc, std::map<std::string,std::string> &definitions, const char *regex)
{
	return OCConstruct(alloc, definitions, regex);
}
