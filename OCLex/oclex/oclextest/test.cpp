//
//  main.cpp
//  oclextest
//
//  Created by William Woody on 7/25/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include <iostream>

#include <OCLexer.h>
#include <OCUtilities.h>
#include "OCLexParser.h"
#include "OCLexNFA.h"
#include <set>

static void PrintCharacter(OCLexNFATransition *t)
{
	int i;
	uint32_t *p = t->set.a;

	for (i = 0; i < 8; ++i) {
		if (p[i] != (uint32_t)-1) break;
	}
	if (i >= 8) {
		printf(".");
		return;
	}

	for (i = 32; i < 128; ++i) {
		if (t->set.TestCharacter(i)) {
			printf("%c",(char)i);
		}
	}
}

static void DumpNFADebug(OCLexNFA state)
{
	uint32_t index = 1;

	/* Print information */
	std::set<OCLexNFAState *> states;
	std::list<OCLexNFAState *> queue;
	queue.push_back(state.start);

	state.start->endRule = 1;
	states.insert(state.start);

	while (!queue.empty()) {
		OCLexNFAState *state = queue.front();
		queue.pop_front();

		if (state->list == NULL) {
			printf("%u terminal\n",state->endRule);
		} else {
			for (OCLexNFATransition *t = state->list; t; t=t->next) {
				uint32_t outState;

				if (states.find(t->state) != states.end()) {
					// Already visited.
					outState = t->state->endRule;
				} else {
					// Not visited yet
					states.insert(t->state);
					queue.push_back(t->state);
					outState = ++index;
					t->state->endRule = outState;
				}

				if (t->e) {
					printf("%u --> %u\n",state->endRule,outState);
				} else {
					printf("%u -- ",state->endRule);
					PrintCharacter(t);
					printf(" --> %u\n",outState);
				}
			}
		}
	}

}

void Test1()
{
	OCLexer lexer;
	lexer.OpenFile("test.l");

	OCLexParser parser;
	parser.ParseFile(lexer);
}

void Test2()
{
	OCAlloc pool;
	std::map<std::string,std::string> definitions;

	definitions["D"] = "[0-9]";

	OCLexNFA ret = OCConstructRule(pool, definitions, "{D}\"AC\\\"\"(ABC|(DE)+F[A-C]?)");
	DumpNFADebug(ret);
}

int main(int argc, const char * argv[])
{
	Test1();
	Test2();

	printf("Done.\n");
	return 0;
}
