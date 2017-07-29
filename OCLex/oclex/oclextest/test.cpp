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

static void PrintCharacter(OCLexNFATransition &t)
{
	int i;

	for (i = 32; i < 128; ++i) {
		if (t.set.TestCharacter(i)) {
			printf("%c",(char)i);
		}
	}
}

static void DumpNFADebug(OCLexNFA &c, OCLexNFAReturn ret)
{
	/* Print information */
	std::set<uint32_t> states;
	std::list<uint32_t> queue;

	queue.push_back(ret.start);
	states.insert(ret.start);

	while (!queue.empty()) {
		uint32_t state = queue.front();
		queue.pop_front();

		OCLexNFAState &s = c.states[state];

		if (s.list.empty()) {
			printf("%u terminal\n",state);
		} else {
			std::list<OCLexNFATransition>::iterator iter;

			for (iter = s.list.begin(); iter != s.list.end(); ++iter) {
				OCLexNFATransition &t = *iter;
				if (states.find(t.state) == states.end()) {
					// Not visited yet
					states.insert(t.state);
					queue.push_back(t.state);
				}

				if (t.e) {
					printf("%u --> %u\n",state,t.state);
				} else {
					printf("%u -- ",state);
					PrintCharacter(t);
					printf(" --> %u\n",t.state);
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
	std::map<std::string,std::string> definitions;
	OCLexNFA construct(definitions);

	definitions["D"] = "[0-9]";

	OCLexNFAReturn ret = construct.AddRule("{D}\"AC\\\"\"(ABC|(DE)+F[A-C]?)");
	DumpNFADebug(construct,ret);
}

void Test3()
{
	OCIntegerSet set;

	set.Add(5);
	set.Add(2);
	set.Add(4);
	set.Add(1);
	set.Remove(2);
	printf("%s\n",set.ToString().c_str());

	OCIntegerSet set2;
	set2.Add(2);
	set2.Add(3);
	set2.Add(7);
	set.AddSet(set2);
	printf("%s\n",set.ToString().c_str());

	set2.RemoveAll();
	set2.Add(2);
	set2.Add(4);
	set.RemoveSet(set2);
	printf("%s\n",set.ToString().c_str());
}

int main(int argc, const char * argv[])
{
	Test1();
	Test2();
	Test3();

	printf("Done.\n");
	return 0;
}
