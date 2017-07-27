//
//  OCLexer.cpp
//  OCCommon
//
//  Created by William Woody on 7/22/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCLexer.h"

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	OCLexer::OCLexer
 *
 *		Open the lexical analysis class
 */

OCLexer::OCLexer()
{
	fFile = NULL;
}

/*	OCLexer::~OCLexer
 *
 *		Shut down
 */

OCLexer::~OCLexer()
{
	if (fFile) fclose(fFile);
}

/************************************************************************/
/*																		*/
/*	File																*/
/*																		*/
/************************************************************************/

/*	OCLexer::OpenFile
 *
 *		Open file. Return false if unable to open
 */

bool OCLexer::OpenFile(std::string filename)
{
	if (fFile) {
		fclose(fFile);
		fFile = NULL;
	}

	fToken = "";
	fTokenID = 0;
	fFileName = filename;
	fLine = 1;
	fColumn = 0;

	fPushBackChar = 0;
	fPushToken = false;

	fFile = fopen(filename.c_str(),"r");
	return (fFile != NULL);
}

/************************************************************************/
/*																		*/
/*	Internal Read														*/
/*																		*/
/************************************************************************/

/*	OCLexer::ReadRawChar
 *
 *		Read next character from the input stream, managing the pushback
 *	stack
 */

int32_t OCLexer::ReadRawChar()
{
	if (fPushBackChar) {
		++fColumn;
		return fPushBack[--fPushBackChar];
	}

	int ch = fgetc(fFile);
	if (ch == -1) {
		if (fColumn != 0) {
			fColumn = 0;
			++fLine;
			return '\n';
		}
	} else if (ch == '\n') {
		fColumn = 0;
		++fLine;
	} else {
		++fColumn;
	}

	return ch;
}

/*	OCLexer::PushChar
 *
 *		Push back character on push stack
 */

void OCLexer::PushChar(int32_t ch)
{
	--fColumn;
	fPushBack[fPushBackChar++] = ch;
}

/*	OCLexer::ReadChar
 *
 *		read next character, stripping comments
 */

int32_t OCLexer::ReadChar()
{
	int32_t ch = ReadRawChar();
	if (ch == '/') {
		int d = ReadRawChar();
		if (d == '/') {
			for (;;) {
				d = ReadRawChar();
				if ((d == '\n') || (d == -1)) break;
			}
			return '\n';
		} else if (d == '*') {
			for (;;) {
				d = ReadRawChar();
				if (d == '*') {
					d = ReadRawChar();
					if (d == '/') break;
					PushChar(d);
				}
			}
			return ' ';
		} else {
			PushChar(d);
			return ch;
		}
	}
	return ch;
}

/************************************************************************/
/*																		*/
/*	Tokenizer															*/
/*																		*/
/************************************************************************/

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

/*	OCLexer::ReadRegEx
 *
 *		Read regular expression. This bypasses the tokenizer and reads the
 *	entire regular expression string, which is the largest block of non-escaped
 *	spaces. This makes no attempt at interpreting the regular expression.
 */

std::string OCLexer::ReadRegEx()
{
	std::string ret;
	bool inString = false;
	bool inChar = false;
	bool inEscape = false;

	int32_t ch;

	while (isspace(ch = ReadRawChar())) ;	// skip white space

	fTokenLine = fLine;
	fTokenColumn = fColumn;
	for (;;) {								// greedy parse the RegEx
		if (ch == -1) break;

		if (inEscape) {
			ret.push_back((char)ch);
			inEscape = false;
		} else if (inString) {
			ret.push_back((char)ch);
			if (ch == '"') inString = false;
		} else if (inChar) {
			ret.push_back((char)ch);
			if (ch == ']') {
				if ((ret != "[]") && (ret != "[^]")) {
					inChar = false;
				}
			}
		} else if (ch == '"') {
			ret.push_back((char)ch);
			inString = true;
		} else if (ch == '[') {
			ret.push_back((char)ch);
			inChar = true;
		} else if (ch == '\\') {
			ret.push_back((char)ch);
			inEscape = true;
		} else if (isspace(ch)) {
			PushChar(ch);
			break;
		} else {
			ret.push_back((char)ch);
		}
		ch = ReadRawChar();
	}

	return ret;
}

/*	OCLexer::ReadStringBody
 *
 *		Read the body of the string or character. Note that character
 *	escapes are translated in the resulting string token that is returned
 *	here. Note that we do not translate string escapes; that is handled
 *	later in the translation process.
 */

void OCLexer::ReadStringBody(int squote)
{
	bool esc = false;
	bool done = false;

	fToken.push_back((char)squote);
	do {
		int c = ReadRawChar();

		if (c == '\n') {
			if (squote == '\'') {
				fprintf(stderr,"%s:%d Character constant not closed\n",fFileName.c_str(),fTokenLine);
			} else {
				fprintf(stderr,"%s:%d String not closed\n",fFileName.c_str(),fTokenLine);
			}
			PushChar(c);
			fToken.push_back((char)squote);
			return;
		}
		if (c == -1) {
			fprintf(stderr,"%s:%d Unexpected end of file\n",fFileName.c_str(),fTokenLine);
			PushChar(c);
			fToken.push_back((char)squote);
			return;
		}

		if (esc) {
			esc = false;
		} else if (c == squote) {
			done = true;
		} else if (c == '\\') {
			esc = true;
		}

		fToken.push_back((char)c);
	} while (!done);
}

/*	OCLexer::ReadNumberBody
 *
 *		Read the body of digits
 */

void OCLexer::ReadNumberBody()
{
	int ch;
	bool lastE = false;

	for (;;) {
		ch = ReadChar();
		if ((ch == '.') || isalnum(ch) || (lastE && ((ch == '+') || (ch == '-')))) {
			fToken.push_back((char)ch);
			if ((ch == 'e') || (ch == 'E')) {
				lastE = true;
			} else {
				lastE = false;
			}
		} else {
			PushChar(ch);
			break;
		}
	}
}

/*	OCLexer::ReadToken
 *
 *		Read the next token in the list. If skip spaces is false, also
 *	returns whitespace
 */

int32_t OCLexer::ReadToken(bool skipSpaces)
{
	int32_t ch;

	if (fPushToken) {
		fPushToken = false;
		return fTokenID;
	}

	/*
	 *	Reset
	 */

	fToken.clear();

	/*
	 *	Skip or parse whitespace
	 */

	if (skipSpaces) {
		while (isspace(ch = ReadChar())) ;
		fTokenLine = fLine;
		fTokenColumn = fColumn;
	} else {
		fTokenLine = fLine;
		fTokenColumn = fColumn;

		ch = ReadChar();
		if (isspace(ch)) {
			fToken.push_back((char)ch);
			fTokenID = ch;
			return ch;
		}
	}

	/*
	 *	Process strings, tokens, numbers, etc.
	 */

	if ((ch == '\'') || (ch == '"')) {
		ReadStringBody(ch);
		fTokenID = (ch == '\'') ? OCTOKEN_CHAR : OCTOKEN_STRING;
		return fTokenID;
	}

	/*
	 *	Process token
	 */

	if ((ch == '_') || isalpha(ch)) {
		while ((ch == '_') || isalnum(ch)) {
			fToken.push_back((char)ch);
			ch = ReadChar();
		}
		PushChar(ch);
		fTokenID = OCTOKEN_TOKEN;
		return fTokenID;
	}

	/*
	 *	Process number
	 */

	if (ch == '.') {
		int d = ReadChar();
		if (isdigit(d)) {
			fToken.push_back((char)ch);
			fToken.push_back((char)d);
			ReadNumberBody();
			fTokenID = OCTOKEN_NUMBER;
			return fTokenID;
		} else {
			/* Not a .N number token. Put things back */
			PushChar(d);
		}
	}

	if (isdigit(ch)) {
		fToken.push_back((char)ch);
		ReadNumberBody();
		fTokenID = OCTOKEN_NUMBER;
		return fTokenID;
	}

	/*
	 *	Process various character patterns
	 */

	fToken.push_back((char)ch);
	if (ch == '%') {
		int d = ReadChar();
		if (d == '%') {
			fToken.push_back((char)d);
			return fTokenID = OCTOKEN_SECTION;
		} else if (d == '{') {
			fToken.push_back((char)d);
			return fTokenID = OCTOKEN_OPENCODE;
		} else if (d == '}') {
			fToken.push_back((char)d);
			return fTokenID = OCTOKEN_CLOSECODE;
		} else {
			PushChar(d);
			return fTokenID = ch;
		}
	}
	if (ch == '$') {
		int d = ReadChar();
		if (d == '$') {
			fToken.push_back((char)d);
			return fTokenID = OCTOKEN_CURPARAM;
		} else if (isdigit(d)) {
			do {
				fToken.push_back((char)d);
			} while (isdigit(d = ReadChar()));
			PushChar(d);

			return fTokenID = OCTOKEN_PARAMETER;
		} else {
			PushChar(d);
			return fTokenID = ch;
		}
	}

	return fTokenID = ch;
}

