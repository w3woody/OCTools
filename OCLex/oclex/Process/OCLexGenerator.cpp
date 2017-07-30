//
//  OCLexGenerator.cpp
//  oclex
//
//  Created by William Woody on 7/30/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include "OCLexGenerator.h"

/************************************************************************/
/*																		*/
/*	State writer														*/
/*																		*/
/************************************************************************/

/*	OCLexGenerator::WriteArray
 *
 *		Write an array. This simply writes the list of items to an array
 */

void OCLexGenerator::WriteArray(FILE *f, uint16_t *list, size_t len)
{
	size_t i = 0;

	for (i = 0; i < len; ++i) {
		if (i) {
			fprintf(f,", ");
		}
		if ((i % 8) == 0) {
			if (i) fprintf(f,"\n");
			fprintf(f,"    ");
		}
		fprintf(f,"%3d",list[i]);
	}
	fprintf(f,"\n");
}

/*	OCLexGenerator::WriteStates
 *
 *		Write the states
 */

void OCLexGenerator::WriteStates(FILE *f)
{
	/*
	 *	Print the state sizes
	 */

	fprintf(f,"/************************************************************************/\n");
	fprintf(f,"/*                                                                      */\n");
	fprintf(f,"/*  Lex Transition State Tables                                         */\n");
	fprintf(f,"/*                                                                      */\n");
	fprintf(f,"/************************************************************************/\n");

	/*
	 *	Constants
	 */

	fprintf(f,"\n");
	fprintf(f,"/*\n");
	fprintf(f," *  Constants\n");
	fprintf(f," */\n\n");

	fprintf(f,"#define MAXSTATES       %lu\n",dfaStates.size());
	fprintf(f,"#define MAXCHARCLASS    %lu\n",charClasses.size());
	fprintf(f,"#define MAXACTIONS      %lu\n\n",codeRules.size());

	/*
	 *	Generate the character class list.
	 */

	uint16_t carray[256];
	size_t i,len = charClasses.size();
	for (i = 0; i < 256; ++i) carray[i] = (uint16_t)len;
	for (i = 0; i < len; ++i) {
		const OCCharSet &set = charClasses[i];
		for (int j = 0; j < 256; ++j) {
			if (set.TestCharacter((unsigned char)j)) {
				carray[j] = i;
			}
		}
	}

	fprintf(f,"\n");
	fprintf(f,"//  Character classes used during parsing:\n");
	fprintf(f,"//\n");
	for (i = 0; i < len; ++i) {
		const OCCharSet &cset = charClasses[i];
		fprintf(f,"//  %zu: %s\n",i,cset.ToString().c_str());
	}
	fprintf(f,"\n");

	fprintf(f,"/*  CharClass\n");
	fprintf(f," *\n");
	fprintf(f," *      Maps 8-bit character to character class\n");
	fprintf(f," */\n\n");
	fprintf(f,"static uint16_t CharClass[256] = {\n");
	WriteArray(f, carray, 256);
	fprintf(f,"};\n\n");

	/*
	 *	Generate state actions. This is a table which maps from a state
	 *	index to an action index
	 */

	fprintf(f,"/*  StateActions\n");
	fprintf(f," *\n");
	fprintf(f," *      Maps states to actions. MAXACTION if this is not a terminal\n");
	fprintf(f," */\n\n");

	size_t alen = codeRules.size();
	len = dfaStates.size();
	uint16_t *scratch = (uint16_t *)malloc(len * sizeof(uint16_t));
	for (i = 0; i < len; ++i) {
		OCLexDFAState &state = dfaStates[i];
		scratch[i] = state.end ? state.endRule : alen;
	}

	fprintf(f,"static uint16_t StateActions[%zu] = {\n",len);
	WriteArray(f,scratch,len);
	fprintf(f,"};\n\n");

	/*
	 *	Generate the DFA state transitions
	 */

	size_t clen = charClasses.size();
	size_t tlen = len * clen;
	scratch = (uint16_t *)malloc(tlen * sizeof(uint16_t));

	size_t ptr = 0;
	for (i = 0; i < len; ++i) {
		OCLexDFAState &state = dfaStates[i];
		for (size_t j = 0; j < clen; ++j) {
			OCCharSet &cset = charClasses[j];

			/*
			 *	Find the transition which intersects
			 */

			uint16_t newState = (uint16_t)len;

			std::vector<OCLexDFATransition>::iterator t;
			for (t = state.list.begin(); t != state.list.end(); ++t) {
				if (t->set.Contains(cset)) {
					// This transition contains our cset. Set and move on
					newState = (uint16_t)t->state;
					break;
				}
			}

			scratch[ptr++] = newState;
		}
	}

	fprintf(f,"/*  StateMachine\n");
	fprintf(f," *\n");
	fprintf(f," *      Lex state machine. Each item indicates the transition from\n");
	fprintf(f," *  a character set and the state to a new state. The new state is\n");
	fprintf(f," *  MAXSTATES if the transition is illegal.\n");
	fprintf(f," *\n");
	fprintf(f," *      The index is charClass + state * MAXCHARCLASS\n");
	fprintf(f," */\n\n");
	fprintf(f,"static uint16_t StateMachine[%zu] = {\n",tlen);
	WriteArray(f,scratch,tlen);
	fprintf(f,"};\n\n");

	free(scratch);
}

/*	OCLexGenerator::WriteActions
 *
 *		Write the actions state machine. This only writes the contents of
 *	the switch state, not the switch state itself.
 */

void OCLexGenerator::WriteActions(FILE *f)
{
	size_t i,len = codeRules.size();
	for (i = 0; i < len; ++i) {
		fprintf(f,"            case %zu:\n",i);
		fprintf(f,"                %s\n",codeRules[i].c_str());
		fprintf(f,"                break;\n\n");
	}
}

/************************************************************************/
/*																		*/
/*	Parser Definitions													*/
/*																		*/
/************************************************************************/

/*	OCLexGenerator::WriteOCFile
 *
 *		This generates the various tables and writes the Objective C
 *	lex file
 */

void OCLexGenerator::WriteOCFile(FILE *f)
{
	WriteStates(f);

	WriteActions(f);
}
