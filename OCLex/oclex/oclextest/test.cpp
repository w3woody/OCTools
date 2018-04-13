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
#include "OCLexDFA.h"
#include "OCLexGenerator.h"
#include <set>

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
					printf("%u -- %s --> %u\n",state,t.set.ToString().c_str(),t.state);
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

//void PrintDFA(const OCLexDFA &dfa)
//{
//	int i,len = (int)dfa.dfaStates.size();
//	for (i = 0; i < len; ++i) {
//		const OCLexDFAState &s = dfa.dfaStates[i];
//
//		int t,tlen = (int)s.list.size();
//		for (t = 0; t < tlen; ++t) {
//			const OCLexDFATransition &trans = s.list[t];
//
//			printf("%d -- %s --> %u\n",i,trans.set.ToString().c_str(),trans.state);
//		}
//
//		if (s.end) {
//			printf("%u endrule %s\n",i,dfa.codeRules[s.endRule].code.c_str());
//		}
//	}
//}
//
//void Test4()
//{
//	std::map<std::string,std::string> definitions;
//	definitions["D"] = "[0-9]";
//
//	OCLexGenerator dfa(definitions);
//
//	dfa.AddRuleSet("{D}+", "return 0x10001;",false,false);
//	dfa.AddRuleSet("[A-Za-z][A-Za-z0-9]*", "return 0x10002;",false,false);
//	dfa.AddRuleSet("0x[A-Fa-f0-9]+", "return 0x10003;",false,true);
//	dfa.AddRuleSet(".","return textBuffer[0];",false,false);
//
//	dfa.GenerateDFA();
//
//	PrintDFA(dfa);
//
//	printf("\n\n\n");
//	dfa.WriteOCFile("OCTest","OCTest.m",stdout);
//}

int main(int argc, const char * argv[])
{
//	Test1();
//	Test2();
//	Test3();
//	Test4();

	printf("Done.\n");
	return 0;
}
