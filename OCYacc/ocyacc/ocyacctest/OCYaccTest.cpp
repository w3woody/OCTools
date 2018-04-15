/*	OCYaccTest.cpp
 *
 *		This file was automatically generated by OCYacc, part of the OCTools
 *	suite available at:
 *
 *		https://github.com/w3woody/OCTools
 */

#include "OCYaccTest.h"
#include "OCLexTest.h"


/*
 *	Note: this is a baseline implementation for an LR parser as described in
 *	Aho, Sethi, Ullman: "Compilers: Principles, Techniques and Tools", (the
 *	Dragon Book), pages 216-220. Unlike the example there we don't bother 
 *	storing the tokens on the token stack. We also take some liberties to
 *	compress our tables, and do a few optimizations similar to Bison.
 *
 *	Table compression scheme comes from
 *
 *	https://en.wikipedia.org/wiki/Sparse_matrix
 *
 *	A discussion of the Bison internals comes from
 *
 *	https://www.cs.uic.edu/~spopuri/cparser.html
 */

/************************************************************************/
/*                                                                      */
/*  State Tables and Constants                                          */
/*                                                                      */
/************************************************************************/

// Various constants
#define K_ACCEPTSTATE       6            // Final accept state
#define K_EOFTOKEN          0x110000     // EOF token ID
#define K_ERRORTOKEN        0x110001     // Error token ID
#define K_FIRSTTOKEN        0x110002     // Error token ID
#define K_MAXSYMBOL         0x110009     // Max ID for all symbols
#define K_STARTSTATE		0            // Start state is always 0

/*  TokenArray
 *
 *      Array of non-Unicode token values for error reporting
 */

static const char *TokenArray[] = {
    "NUMBER",
    "TOKEN"
};

/*  RuleLength
 *
 *      The number of tokens a reduce action removes from the stack
 */

static const uint8_t RuleLength[13] = {
     1,  3,  3,  3,  3,  3,  3,  3, 
     1,  2,  2,  1,  2
};

/*  RuleProduction
 *
 *      The prodution ID we reduce to
 */

static const uint32_t RuleProduction[13] = {
    0x110004, 0x110005, 0x110005, 0x110006, 0x110006, 0x110006, 0x110006, 0x110006, 
    0x110006, 0x110007, 0x110007, 0x110008, 0x110008
};

/*
 *  The following tables are compressed using CSR format.
 *  See https://en.wikipedia.org/wiki/Sparse_matrix
 */

/*  ActionI, J, A
 *
 *      Compressed action index table.
 */

static const uint32_t ActionI[38] = {
         0,      2,      3,      4,      5,      8,     11,     12, 
        15,     18,     21,     23,     28,     29,     34,     36, 
        38,     40,     42,     47,     50,     53,     58,     60, 
        65,     70,     75,     77,     79,     81,     83,     88, 
        91,     94,     99,    104,    109,    112
};

static const uint32_t ActionJ[112] = {
    0x110001, 0x110003,      ';',      '=',      ';', 0x110000, 0x110001, 0x110003, 
    0x110000, 0x110001, 0x110003, 0x110000, 0x110000, 0x110001, 0x110003, 0x110000, 
    0x110001, 0x110003,      '(', 0x110002, 0x110003,      '(', 0x110002,      '*', 
         '+',      '-',      '/',      ';',      ';',      '*',      '+',      '-', 
         '/',      ';',      '(', 0x110002,      '(', 0x110002,      '(', 0x110002, 
         '(', 0x110002,      '*',      '+',      '-',      '/',      ';',      '*', 
         '/',      ';',      '*',      '/',      ';',      '*',      '+',      '-', 
         '/',      ';',      '(', 0x110002,      ')',      '*',      '+',      '-', 
         '/',      ')',      '*',      '+',      '-',      '/',      '*',      '+', 
         '-',      '/',      ';',      '(', 0x110002,      '(', 0x110002,      '(', 
    0x110002,      '(', 0x110002,      ')',      '*',      '+',      '-',      '/', 
         ')',      '*',      '/',      ')',      '*',      '/',      ')',      '*', 
         '+',      '-',      '/',      ')',      '*',      '+',      '-',      '/', 
         ')',      '*',      '+',      '-',      '/', 0x110000, 0x110001, 0x110003
};

// Note: < 0 -> reduce (rule = -a-1), >= 0 -> shift (state).
static const int16_t ActionA[112] = {
         1,      2,     36,      9,      8,    -12,    -12,    -12, 
         6,      1,      2,     -1,    -13,    -13,    -13,    -10, 
       -10,    -10,     10,     11,      2,     22,     23,     -9, 
        -9,     -9,     -9,     -9,     -3,     14,     15,     16, 
        17,     -2,     10,     11,     10,     11,     10,     11, 
        10,     11,     14,     15,     16,     17,     -8,     -6, 
        -6,     -6,     -5,     -5,     -5,     14,     15,     16, 
        17,     -7,     22,     23,     -9,     -9,     -9,     -9, 
        -9,     25,     26,     27,     28,     29,     -4,     -4, 
        -4,     -4,     -4,     22,     23,     22,     23,     22, 
        23,     22,     23,     -8,     26,     27,     28,     29, 
        -6,     -6,     -6,     -5,     -5,     -5,     -7,     26, 
        27,     28,     29,     35,     26,     27,     28,     29, 
        -4,     -4,     -4,     -4,     -4,    -11,    -11,    -11
};

/*  GotoI, J, A
 *
 *      Compressed goto table.
 */

static const uint32_t GotoI[38] = {
         0,      3,      3,      3,      3,      3,      5,      5, 
         5,      5,      7,      8,      8,      8,      8,      9, 
        10,     11,     12,     12,     12,     12,     12,     13, 
        13,     13,     13,     14,     15,     16,     17,     17, 
        17,     17,     17,     17,     17,     17
};

static const uint32_t GotoJ[17] = {
    0x110005, 0x110007, 0x110008, 0x110005, 0x110007, 0x110005, 0x110006, 0x110006, 
    0x110006, 0x110006, 0x110006, 0x110006, 0x110006, 0x110006, 0x110006, 0x110006, 
    0x110006
};

static const int16_t GotoA[17] = {
         3,      4,      5,      3,      7,     12,     13,     24, 
        21,     20,     19,     18,     34,     33,     32,     31, 
        30
};

/************************************************************************/
/*                                                                      */
/*  Parser Code															*/
/*                                                                      */
/************************************************************************/

/*
 *	Construction
 */

OCYaccTest::OCYaccTest(OCLexTest *l)
{
	lex = l;

}

/*
 *	Destruction
 */

OCYaccTest::~OCYaccTest()
{

}



/*
 *	Process production rule. This processes the production rule and creates
 *	a new stack state with the rule reduction.
 */

OCYaccTestStack OCYaccTest::processReduction(int16_t rule)
{
	// Get production len, for translating $1...$N into something useful
	int32_t pos = (int32_t)stack.size() - RuleLength[rule];

	// Set up initial state.
	OCYaccTestStack s;

	// File position of reduced rule is first token of the symbols reduced
	OCYaccTestStack &fs = stack[pos];
	s.filename = fs.filename;
	s.line = fs.line;
	s.column = fs.column;

	// Now process production.
	//
	// Note that $$ translated into (s.value), and
	// $n translates into ((<type> *)(stack[pos+(n-1)])), where <type>
	// is the declared type of the token or production rule.

	try {
		switch (rule) {

        // Production rules
            // (110004) $accept : statements $end
            case 0:
                break;

            // (110005) assignment : TOKEN '=' expression 
            case 1:
                break;

            // (110005) assignment : TOKEN '=' assignment 
            case 2:
                break;

            // (110006) expression : '(' expression ')' 
            case 3:
                break;

            // (110006) expression : expression '+' expression 
            case 4:
                break;

            // (110006) expression : expression '-' expression 
            case 5:
                break;

            // (110006) expression : expression '*' expression 
            case 6:
                break;

            // (110006) expression : expression '/' expression 
            case 7:
                break;

            // (110006) expression : NUMBER 
            case 8:
                break;

            // (110007) statement : assignment ';' 
            case 9:
                break;

            // (110007) statement : error ';' 
            case 10:
                break;

            // (110008) statements : statement 
            case 11:
                s.value.a = (stack[pos].value.a); 
                break;

            // (110008) statements : statements statement 
            case 12:
                break;


			default:
				break;
		}
	}
	catch (...) {
	}
	return s;
}
/*
 *	Look up the action value for the state and token values provided. This
 *	parses through the ActionI, ActionJ and ActionA lists to find the correct
 *	entry. This returns NSIntegerMax if a value was not found, and this is an
 *	error entry.
 */

int32_t OCYaccTest::actionForState(int32_t state, int32_t token)
{
	size_t min,max,mid;

	/* Deal with EOF case */
	if (token == -1) token = K_EOFTOKEN;

	/* Find range */
	min = ActionI[state];
	max = ActionI[state+1];

	/* Binary search for value in ja */
	while (min < max) {
		mid = (min + max)/2;
		uint32_t j = ActionJ[mid];
		if (token == j) {
			return ActionA[mid];
		} else if (token < j) {
			max = mid;
		} else {
			min = mid+1;
		}
	}
	return INT_MAX;
}

/*
 *	Look up the goto value for the state and token (production rule) provided.
 *	Operates the same as actionForState, except we look in the goto table.
 *	In teory I guess we could combine these two tables (as productions and
 *	tokens do not overlap).
 */

int32_t OCYaccTest::gotoForState(int32_t state, int32_t token)
{
	size_t min,max,mid;

	/* Find range */
	min = GotoI[state];
	max = GotoI[state+1];

	/* Binary search for value in ja */
	while (min < max) {
		mid = (min + max)/2;
		int32_t j = GotoJ[mid];
		if (token == j) {
			return GotoA[mid];
		} else if (token < j) {
			max = mid;
		} else {
			min = mid+1;
		}
	}
	return INT_MAX;
}

/*	OCYaccTest::error
 *
 *		Override this if you want error reporting
 */

void OCYaccTest::error(int32_t line, int32_t col, std::string fname, int32_t errCode, std::map<std::string,std::string> &map)
{
	// By default this does nothing.
}

/*
 *	Errors. This formats and prints the specified error
 */

void OCYaccTest::errorWithCode(int32_t code, std::map<std::string,std::string> &data)
{
	if (errorCount > 0) return;		// skip until synced on 3 shifts

	// Call delegate with current token position
	// Token position is the topmost symbol
	OCYaccTestStack &top = stack.back();

	error(top.line,top.column,top.filename,code,data);

	// And now skip the next 3 token shifts so we don't spew garbage.
	if (0 == (code & ERRORMASK_WARNING)) {
		success = false;
		errorCount = 3;
	}
}

void OCYaccTest::errorWithCode(int32_t code)
{
	std::map<std::string,std::string> empty;
	errorWithCode(code, empty);
}

void OCYaccTest::errorOK()
{
	errorCount = 0;
}

std::string OCYaccTest::tokenToString(uint32_t token)
{
	if (token >= K_FIRSTTOKEN) {
		return TokenArray[token - K_FIRSTTOKEN];
	} else {
		/*
		 *	Convert token to UTF-8
		 */

		std::string ret;
		if (token < 0x80) {
			ret += (char)token;
		} else if (token < 0x800) {
			ret += (char)(0xC0 | (0x1F & (token >> 6)));
			ret += (char)(0x80 | (0x3F & (token)));
		} else if (token < 0x10000) {
			ret += (char)(0xE0 | (0x0F & (token >> 12)));
			ret += (char)(0x80 | (0x3F & (token >> 6)));
			ret += (char)(0x80 | (0x3F & (token)));
		} else {
			ret += (char)(0xF0 | (0x07 & (token >> 18)));
			ret += (char)(0x80 | (0x3F & (token >> 12)));
			ret += (char)(0x80 | (0x3F & (token >> 6)));
			ret += (char)(0x80 | (0x3F & (token)));
		}
		return ret;
	}
}

bool OCYaccTest::reduceByAction(int16_t action)
{
	// Determine the new state we're transitioning to.
	uint32_t production = RuleProduction[action];
	uint8_t length = RuleLength[action];

	// We pull the state we'd have after popping the stack.
	uint16_t statep = stack[stack.size() - length - 1].state;

	// Now calculate the state we should transition to
	int32_t newState = gotoForState(statep,production);
	if (newState == INT_MAX) return false;

	// Process production rule, which generates the new state
	OCYaccTestStack state = processReduction(action);

	// Update state
	state.state = newState;

	// Pop the stack
	stack.erase(stack.begin() + (stack.size() - length),stack.end());

	// Push new state
	stack.push_back(state);

	// Done.
	return true;
}

/*
 *	Parser engine. Returns NO if there was an error during processing. Note
 *	that as we uncover errors we call our delegate for error handling. This
 *	implements the algorithm described in the Dragon Book, Algorithm 4.7.
 */

bool OCYaccTest::parse()
{
	int32_t a;				// lex symbol

	/*
	 *	Step 1: reset and push the empty state.
	 */

	success = true;
	stack.clear();

	OCYaccTestStack initStack;
	initStack.state = K_STARTSTATE;
	initStack.filename = lex->filename;
	initStack.line = lex->line;
	initStack.column = lex->column;
	stack.push_back(initStack);

	/*
	 *	Now repeat forever:
	 */

	a = lex->lex();

	for (;;) {
		OCYaccTestStack &s = stack.back();

		/*
		 *	Determine if this is the end state. If so, then we immediately
		 *	quit. We assume the user has set the production rule at the
		 *	top, so we can simply drop the stack
		 */

		if (s.state == K_ACCEPTSTATE) {
			stack.clear();
			return success;
		}

		/*
		 *	Now determine the action and shift, reduce or handle error as
		 *	appropriate
		 */

		int32_t action = actionForState(s.state, a);

		if (action == INT_MAX) {
			/*
			 *	Handle error. First, note we have an error, and note the
			 *	symbol on which our error took place.
			 */

			success = false;		// regardless, we will always fail.

			/*
			 *	First, scan backwards from the current state, looking for one
			 *	which has an 'error' symbol.
			 */

			size_t ix = stack.size();
			while (ix > 0) {
				OCYaccTestStack &si = stack[--ix];
				action = actionForState(si.state, K_ERRORTOKEN);
				if ((action >= 0) && (action != INT_MAX)) {
					/*
					 *	Encountered error state. If the user has defined an
					 *	error token, we ultimately will want to (a) unwind
					 *	the stack until we find a state which handles the
					 *	error transition. We then .
					 */

					if (ix+1 < stack.size()) {
						stack.erase(stack.begin() + (ix + 1),stack.end());
					}

					/*
					 *	At this point we perform a shift to our new error
					 *	state.
					 */

					OCYaccTestStack stmp;
					stmp.state = action;
					stmp.filename = lex->filename;
					stmp.line = lex->line;
					stmp.column = lex->column;
					stack.push_back(stmp);

					/*
					 *	Second, we start pulling symbols until we find a symbol
					 *	that shifts, or until we hit the end of file symbol.
					 *	This becomes our current token for parsing
					 */

					for (;;) {
						a = lex->lex();
						action = actionForState(s.state, a);
						if ((action >= 0) && (action != INT_MAX)) {
							/*
							 *	Valid shift. This becomes our current token,
							 *	and we resume processing.
							 */

							continue;

						} else if (action == K_EOFTOKEN) {
							/*
							 *	We ran out of tokens. At this point all
							 *	we can do is print an error and force quit.
							 */

							errorWithCode(ERROR_SYNTAX);
							stack.clear();

							return false;
						}
					}
				}
			}

			/*
			 *	If we reach this point, there is no error we can recover to.
			 *	So figure this out on our own.
			 *
			 *	First, we see if the state we're in has a limited number of
			 *	choices. For example, in C, the 'for' keyword will always be
			 *	followed by a '(' token, so we can offer to automatically
			 *	insert that token.
			 */

			int32_t actionMin = ActionI[s.state];
			int32_t actionMax = ActionI[s.state + 1];
			int32_t actionVal = actionMin;
			int16_t actionState = -1;
			for (int32_t ix = actionMin; ix < actionMax; ++ix) {
				int16_t act = ActionA[ix];
				if (actionState == -1) {
					if (act >= 0) {
						actionState = act;
						actionVal = ix;
					}
				} else {
					actionState = -1;
					break;
				}
			}

			if (actionState != -1) {
				/*
				 *	We can accomplish this transition with one token. Print
				 *	an error, and do a shift on the state with an empty value.
				 */

				std::string tokenStr = tokenToString(ActionJ[actionVal]);
				std::map<std::string,std::string> map;
				map["token"] = tokenStr;
				errorWithCode(ERROR_MISSINGTOKEN, map);

				/*
				 *	Perform a shift but do not pull a new token
				 */

				OCYaccTestStack &top = stack.back();

				OCYaccTestStack stmp;
				stmp.state = actionState;
				stmp.filename = top.filename;
				stmp.line = top.line;
				stmp.column = top.column;

				stack.push_back(stmp);
				continue;
			}

			/*
			 *	See if we have a limited choice in reductions. If this can
			 *	only reduce to a single state, try that reduction.
			 */

			actionState = 0;
			for (int32_t ix = actionMin; ix < actionMax; ++ix) {
				int16_t act = ActionA[ix];
				if (actionState == 0) {
					if ((act < 0) && (actionState != act)) {
						actionState = act;
					}
				} else {
					actionState = 0;
					break;
				}
			}

			if (actionState != 0) {
				/*
				 *	We have one possible reduction. Try that. Note that this
				 *	will trigger a syntax error since we're reducing down
				 *	without the follow token. My hope is that the state we
				 *	transition to has a limited set of next tokens to follow.
				 */

				reduceByAction(action);
				continue;
			}

			/*
			 *	If we have a limited number of tokens which can follow,
			 *	print a list of them. Then shift by the first one we
			 *	find. We don't do this if the number of shifts is greater
			 *	than five.
			 */

			if (actionMax - actionMin <= 5) {
				std::string tlist;
				for (int32_t ix = actionMin; ix < actionMax; ++ix) {
					if (ix > actionMin) tlist += ", ";
					tlist += tokenToString(ActionJ[ix]);
				}

				std::map<std::string,std::string> map;
				map["token"] = tlist;
				errorWithCode(ERROR_MISSINGTOKENS, map);

				/*
				 *	Now we artificially insert the first of the list of
				 *	tokens as our action and continue.
				 */

				a = ActionJ[actionMin];
				continue;
			}

			/*
			 *	If we get here, things just went too far south. So we
			 *	skip a token, print syntax error and move on
			 */

			errorWithCode(ERROR_SYNTAX);
			a = lex->lex();
			if (a == -1) return false;

		} else if (action >= 0) {
			/*
			 *	Shift operation.
			 */

			// Shift
			OCYaccTestStack stmp;
			stmp.state = action;
			stmp.filename = lex->filename;
			stmp.line = lex->line;
			stmp.column = lex->column;

			stack.push_back(stmp);

			// Advance to next token.
			a = lex->lex();

			// Decrement our error count. If this is non-zero we're in an
			// error state, and we don't pass spurrous errors upwards
			if (errorCount) --errorCount;

		} else {
			/*
			 *	Reduce action. (Reduce is < 0, and the production to reduce
			 *	by is given below
			 */

			action = -action-1;

			if (!reduceByAction(action)) {
				// If there is an error, this handles the error.
				// (This should not happen in practice).
				errorWithCode(ERROR_SYNTAX);

				// Advance to next token.
				a = lex->lex();
			}
		}
	}
}
