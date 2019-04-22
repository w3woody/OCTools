//
//  OCLexSwiftGenerator.cpp
//  oclex
//
//  Created by William Woody on 4/12/18.
//  Copyright © 2018 Glenview Software. All rights reserved.
//

#include "OCLexSwiftGenerator.h"
#include <OCUtilities.h>

/************************************************************************/
/*																		*/
/*	Standard File Components											*/
/*																		*/
/************************************************************************/

static const char *GSource1 =
	"//\t$(outname)\n"                                                        \
	"\n"                                                                      \
	"import Cocoa\n"                                                          \
	"\n"                                                                      \
	"$(header)\n"                                                             \
	"$(%)\n"                                                                  \
	"\n"                                                                      \
	"class $(classname): OCLexInput {\n"                                      \
	"\t$(global)\n"                                                           \
	"\t$(local)\n"                                                            \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tInternal generated state\n"                                        \
	"\t */\n"                                                                 \
	"\n";

static const char *GSource2 =
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tFile status\n"                                                     \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tpublic var line: Int\n"                                                \
	"\tpublic var column: Int\n"                                              \
	"\tpublic var filename: String\?\n"                                       \
	"\tpublic var text: String\n"                                             \
	"\tpublic var abort: String\?\n"                                          \
	"\tpublic var value: AnyObject\?\n"                                       \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tState\n"                                                           \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tprivate var file: OCFileInput\n"                                       \
	"\n"                                                                      \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tInternal status\n"                                                 \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tprivate var curLine: Int\n"                                            \
	"\tprivate var curColumn: Int\n"                                          \
	"\n"                                                                      \
	"\t// Mark location support\n"                                            \
	"\tprivate var markLine: Int\n"                                           \
	"\tprivate var markColumn: Int\n"                                         \
	"\n"                                                                      \
	"\t// Mark buffer storage\n"                                              \
	"\tprivate var isMarked: Bool\n"                                          \
	"\tprivate var markBuffer: NSMutableString\n"                             \
	"\n"                                                                      \
	"\t// Read cache\n"                                                       \
	"\tprivate var readPos: Int\n"                                            \
	"\tprivate var readBuffer: NSMutableString\n"                             \
	"\n"                                                                      \
	"\t// Text read buffer\n"                                                 \
	"\tprivate var textMarkSize: Int\n"                                       \
	"\tprivate var textBuffer: NSMutableString\n"                             \
	"\n"                                                                      \
	"\t// State flags\n"                                                      \
	"\tprivate var states: UInt64\n"                                          \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tInitialize our parser with the input buffer\n"                     \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tpublic init(stream f: OCFileInput)\n"                                  \
	"\t{\n"                                                                   \
	"\t\tfile = f\n"                                                          \
	"\n"                                                                      \
	"\t\t// Zero public values\n"                                             \
	"\t\tline = 0\n"                                                          \
	"\t\tcolumn = 0\n"                                                        \
	"\t\tfilename = nil\n"                                                    \
	"\t\ttext = String()\n"                                                   \
	"\t\tabort = nil\n"                                                       \
	"\t\tvalue = nil\n"                                                       \
	"\n"                                                                      \
	"\t\t// Zero private\n"                                                   \
	"\t\tisMarked = false\n"                                                  \
	"\t\tmarkBuffer = NSMutableString()\n"                                    \
	"\n"                                                                      \
	"\t\treadPos = 0\n"                                                       \
	"\t\treadBuffer = NSMutableString()\n"                                    \
	"\n"                                                                      \
	"\t\ttextMarkSize = 0\n"                                                  \
	"\t\ttextBuffer = NSMutableString()\n"                                    \
	"\n"                                                                      \
	"\t\tcurLine = 0\n"                                                       \
	"\t\tcurColumn = 0\n"                                                     \
	"\t\tmarkLine = 0\n"                                                      \
	"\t\tmarkColumn = 0\n"                                                    \
	"\n"                                                                      \
	"\t\tstates = 0\n"                                                        \
	"\n"                                                                      \
	"\t\t$(init)\n"                                                           \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\tdeinit {\n"                                                            \
	"\t\t$(finish)\n"                                                         \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tInternal read backtrack support. We implement the internal methods\n" \
	"\t *\tfor mark, unmark and reset to note the current file position. This is\n" \
	"\t *\tused by the lexer to unwind the read queue if we overread the number\n" \
	"\t *\tof characters during parsing\n"                                    \
	"\t *\n"                                                                  \
	"\t *\tIn essence, as we execute the DFA while we read characters, when we\n" \
	"\t *\tfind an end state, we mark the stream. Then we continue to read until\n" \
	"\t *\twe reach an error--at which point we rewind back to the mark location.\n" \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\t\tMark: note that we should cache characters being read so we can rewind\n" \
	"\t *\tto this location in the future\n"                                  \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tprivate func mark()\n"                                                 \
	"\t{\n"                                                                   \
	"\t\tmarkLine = curLine\n"                                                \
	"\t\tmarkColumn = curColumn\n"                                            \
	"\t\ttextMarkSize = textBuffer.length\n"                                  \
	"\n"                                                                      \
	"\t\tisMarked = true\n"                                                   \
	"\t\tmarkBuffer.deleteCharacters(in: NSMakeRange(0, markBuffer.length))\n" \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tReset: reset the buffer positions\n"                               \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tprivate func reset()\n"                                                \
	"\t{\n"                                                                   \
	"\t\tif !isMarked {\n"                                                    \
	"\t\t\t// not marked, nothing to do.\n"                                   \
	"\t\t\treturn\n"                                                          \
	"\t\t}\n"                                                                 \
	"\n"                                                                      \
	"\t\t/*\n"                                                                \
	"\t\t *\tClear the part that was already read\n"                          \
	"\t\t */\n"                                                               \
	"\n"                                                                      \
	"\t\tif (readPos < readBuffer.length) {\n"                                \
	"\t\t\treadBuffer.deleteCharacters(in: NSMakeRange(0,readPos))\n"         \
	"\t\t} else {\n"                                                          \
	"\t\t\treadBuffer.deleteCharacters(in: NSMakeRange(0,readBuffer.length))\n" \
	"\t\t}\n"                                                                 \
	"\n"                                                                      \
	"\t\t/*\n"                                                                \
	"\t\t *\tNow moved the marked stuff\n"                                    \
	"\t\t */\n"                                                               \
	"\n"                                                                      \
	"\t\tif (markBuffer.length > 0) {\n"                                      \
	"\t\t\treadBuffer.insert(markBuffer as String, at: 0)\n"                  \
	"\t\t}\n"                                                                 \
	"\n"                                                                      \
	"\t\t/*\n"                                                                \
	"\t\t *\tClear the mark, reset positions\n"                               \
	"\t\t */\n"                                                               \
	"\n"                                                                      \
	"\t\treadPos = 0\n"                                                       \
	"\t\tisMarked = false\n"                                                  \
	"\t\tmarkBuffer.deleteCharacters(in: NSMakeRange(0, markBuffer.length))\n" \
	"\n"                                                                      \
	"\t\t/*\n"                                                                \
	"\t\t *\tUpdate the file position\n"                                      \
	"\t\t */\n"                                                               \
	"\n"                                                                      \
	"\t\tcurLine = markLine\n"                                                \
	"\t\tcurColumn = markColumn\n"                                            \
	"\n"                                                                      \
	"\t\t/*\n"                                                                \
	"\t\t *\tRemove the read text back to the marked location\n"              \
	"\t\t */\n"                                                               \
	"\n"                                                                      \
	"\t\tif textMarkSize < textBuffer.length {\n"                             \
	"\t\t\ttextBuffer.deleteCharacters(in: NSMakeRange(textMarkSize,textBuffer.length - textMarkSize))\n" \
	"\t\t}\n"                                                                 \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tRead a single character or -1 if at EOF\n"                         \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tprivate func input() -> Int\n"                                         \
	"\t{\n"                                                                   \
	"\t\tvar ch: Int\n"                                                       \
	"\n"                                                                      \
	"\t\t/*\n"                                                                \
	"\t\t *\tRead from marked buffer\n"                                       \
	"\t\t */\n"                                                               \
	"\n"                                                                      \
	"\t\tif readPos < readBuffer.length {\n"                                  \
	"\t\t\tch = Int(readBuffer.character(at: readPos))\n"                     \
	"\t\t\treadPos += 1\n"                                                    \
	"\t\t} else {\n"                                                          \
	"\t\t\tch = file.readByte()\n"                                            \
	"\t\t\tif ch == -1 {\n"                                                   \
	"\t\t\t\treturn -1\n"                                                     \
	"\t\t\t}\n"                                                               \
	"\t\t}\n"                                                                 \
	"\n"                                                                      \
	"\t\t/*\n"                                                                \
	"\t\t *\tUpdate file position\n"                                          \
	"\t\t */\n"                                                               \
	"\n"                                                                      \
	"\t\tif ch == 0x0A {\t\t\t// \'\\n\'\n"                                   \
	"\t\t\tcurColumn = 0\n"                                                   \
	"\t\t\tcurLine += 1\n"                                                    \
	"\t\t} else {\n"                                                          \
	"\t\t\tcurColumn += 1\n"                                                  \
	"\t\t}\n"                                                                 \
	"\n"                                                                      \
	"\t\t/*\n"                                                                \
	"\t\t *\tHandle mark\n"                                                   \
	"\t\t */\n"                                                               \
	"\n"                                                                      \
	"\t\tif isMarked {\n"                                                     \
	"\t\t\tmarkBuffer.appendFormat(\"%c\", ch)\n"                             \
	"\t\t}\n"                                                                 \
	"\n"                                                                      \
	"\t\t/*\n"                                                                \
	"\t\t *\tDone.\n"                                                         \
	"\t\t */\n"                                                               \
	"\n"                                                                      \
	"\t\treturn ch\n"                                                         \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tEOL testing\n"                                                     \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tprivate func atEOL() -> Bool\n"                                        \
	"\t{\n"                                                                   \
	"\t\tvar ch: Int\n"                                                       \
	"\n"                                                                      \
	"\t\tif readPos < readBuffer.length {\n"                                  \
	"\t\t\tch = Int(readBuffer.character(at: readPos))\n"                     \
	"\t\t} else {\n"                                                          \
	"\t\t\tch = file.peekByte()\n"                                            \
	"\t\t}\n"                                                                 \
	"\n"                                                                      \
	"\t\tif (ch == -1) || (ch == 0x0A) {\n"                                   \
	"\t\t\treturn true\n"                                                     \
	"\t\t} else {\n"                                                          \
	"\t\t\treturn false\n"                                                    \
	"\t\t}\n"                                                                 \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\tprivate func atSOL() -> Bool\n"                                        \
	"\t{\n"                                                                   \
	"\t\treturn column == 0\n"                                                \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tRead the state for the class/state combination. Decodes the sparce\n" \
	"\t *\tmatrix that is compressed in StateMachineIA/JA/A above. If the\n"  \
	"\t *\tentry is not found, returns MAXSTATES. This is the same as the lookup\n" \
	"\t *\tStateMachine[class][state] if the StateMachine sparse array was\n" \
	"\t *\tunrolled\n"                                                        \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tprivate func stateForClass(_ charClass: UInt16, state: UInt16) -> UInt16\n" \
	"\t{\n"                                                                   \
	"\t\tvar min, max, mid: UInt16\n"                                         \
	"\n"                                                                      \
	"\t\t/* Find range */\n"                                                  \
	"\t\tmin = $(classname).StateMachineIA[Int(state)]\n"                     \
	"\t\tmax = $(classname).StateMachineIA[Int(state)+1]\n"                   \
	"\n"                                                                      \
	"\t\twhile min < max {\n"                                                 \
	"\t\t\tmid = (min + max) / 2;\n"                                          \
	"\t\t\tlet j = $(classname).StateMachineJA[Int(mid)]\n"                   \
	"\t\t\tif charClass == j {\n"                                             \
	"\t\t\t\treturn $(classname).StateMachineA[Int(mid)]\n"                   \
	"\t\t\t} else if charClass < j {\n"                                       \
	"\t\t\t\tmax = mid\n"                                                     \
	"\t\t\t} else {\n"                                                        \
	"\t\t\t\tmin = mid + 1\n"                                                 \
	"\t\t\t}\n"                                                               \
	"\t\t}\n"                                                                 \
	"\n"                                                                      \
	"\t\treturn $(classname).MAXSTATES\n"                                     \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\tpublic func set(file: String\?, line l: Int)\n"                        \
	"\t{\n"                                                                   \
	"\t\tif file != nil {\n"                                                  \
	"\t\t\tfilename = file\n"                                                 \
	"\t\t}\n"                                                                 \
	"\t\tline = l\n"                                                          \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\tpublic func set(line l: Int)\n"                                        \
	"\t{\n"                                                                   \
	"\t\tline = l\n"                                                          \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tInternal methods declared within the Lex file\n"                   \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"$(code)\n"                                                               \
	"\n";

static const char *GSource3 =
	"\n"                                                                      \
	"\tpublic func lex() -> Int\n"                                            \
	"\t{\n"                                                                   \
	"\t\tvar state: UInt16\n"                                                 \
	"\t\tvar action: UInt16 = $(classname).MAXACTIONS\n"                      \
	"\n"                                                                      \
	"\t\tabort = nil\n"                                                       \
	"\t\tvalue = nil\n"                                                       \
	"\t\ttext = \"\"\n"                                                       \
	"\n"                                                                      \
	"\t\t/*\n"                                                                \
	"\t\t *\tRun until we hit EOF or a production rule triggers a return\n"   \
	"\t\t */\n"                                                               \
	"\n"                                                                      \
	"\t\twhile true {\n"                                                      \
	"\t\t\t/*\n"                                                              \
	"\t\t\t *\tStart running the DFA\n"                                       \
	"\t\t\t */\n"                                                             \
	"\n"                                                                      \
	"\t\t\tstate = 0\n"                                                       \
	"\t\t\ttextBuffer.deleteCharacters(in: NSMakeRange(0, textBuffer.length))\n" \
	"\t\t\tline = curLine\n"                                                  \
	"\t\t\tcolumn = curColumn\n"                                              \
	"\n"                                                                      \
	"\t\t\twhile true {\n"                                                    \
	"\t\t\t\tlet ch = input()\n"                                              \
	"\t\t\t\tif ch == -1 {\n"                                                 \
	"\t\t\t\t\t/*\n"                                                          \
	"\t\t\t\t\t *\tWe\'ve hit EOF. If there is no stored text, we assume\n"   \
	"\t\t\t\t\t *\twe\'re at the file EOF, so return EOF. Otherwise treat as\n" \
	"\t\t\t\t\t *\tillegal state transition.\n"                               \
	"\t\t\t\t\t */\n"                                                         \
	"\n"                                                                      \
	"\t\t\t\t\tif textBuffer.length == 0 {\n"                                 \
	"\t\t\t\t\t\treturn -1\n"                                                 \
	"\t\t\t\t\t}\n"                                                           \
	"\t\t\t\t\tbreak\n"                                                       \
	"\t\t\t\t}\n"                                                             \
	"\n"                                                                      \
	"\t\t\t\t/*\n"                                                            \
	"\t\t\t\t *\tAttempt to translate to the next state\n"                    \
	"\t\t\t\t */\n"                                                           \
	"\n"                                                                      \
	"\t\t\t\tlet charClass = $(classname).CharClass[Int(ch)]\n"               \
	"\t\t\t\tlet newState = stateForClass(charClass, state: state)\n"         \
	"\t\t\t\tif newState >= $(classname).MAXSTATES {\n"                       \
	"\t\t\t\t\t/* Illegal state translation */\n"                             \
	"\t\t\t\t\tbreak\n"                                                       \
	"\t\t\t\t}\n"                                                             \
	"\n"                                                                      \
	"\t\t\t\t/*\n"                                                            \
	"\t\t\t\t *\tStore the character we read, and transition to the next\n"   \
	"\t\t\t\t *\tstate\n"                                                     \
	"\t\t\t\t */\n"                                                           \
	"\n"                                                                      \
	"\t\t\t\tstate = newState\n"                                              \
	"\t\t\t\ttextBuffer.appendFormat(\"%c\", ch)\n"                           \
	"\n"                                                                      \
	"\t\t\t\t/*\n"                                                            \
	"\t\t\t\t *\tNote the current action if we have one\n"                    \
	"\t\t\t\t */\n"                                                           \
	"\n"                                                                      \
	"\t\t\t\tvar newAction = $(classname).StateActions[Int(state)]\n"         \
	"\t\t\t\tif newAction > $(classname).MAXACTIONS {\n"                      \
	"\t\t\t\t\tnewAction = conditionalAction(newAction)\n"                    \
	"\t\t\t\t}\n"                                                             \
	"\t\t\t\tif newAction != $(classname).MAXACTIONS {\n"                     \
	"\t\t\t\t\taction = newAction\n"                                          \
	"\t\t\t\t\tmark()\n"                                                      \
	"\t\t\t\t}\n"                                                             \
	"\t\t\t}\n"                                                               \
	"\n"                                                                      \
	"\t\t\t/*\n"                                                              \
	"\t\t\t *\tIf no action is set, we simply abort after setting an internal\n" \
	"\t\t\t *\terror state. This should never happen in a well designed lexer\n" \
	"\t\t\t */\n"                                                             \
	"\n"                                                                      \
	"\t\t\tif action == $(classname).MAXACTIONS {\n"                          \
	"\t\t\t\tabort = \"Illegal character sequence\"\n"                        \
	"\t\t\t\treturn -1\n"                                                     \
	"\t\t\t}\n"                                                               \
	"\n"                                                                      \
	"\t\t\t/*\n"                                                              \
	"\t\t\t *\tAction is set, so we rewind.\n"                                \
	"\t\t\t */\n"                                                             \
	"\n"                                                                      \
	"\t\t\treset()\n"                                                         \
	"\t\t\tif textBuffer.length == 0 {\n"                                     \
	"\t\t\t\tabort = \"No characters read in sequence\"\n"                    \
	"\t\t\t\treturn -1\n"                                                     \
	"\t\t\t}\n"                                                               \
	"\n"                                                                      \
	"\t\t\t/*\n"                                                              \
	"\t\t\t *\tConvert text sequence into string\n"                           \
	"\t\t\t */\n"                                                             \
	"\n"                                                                      \
	"\t\t\ttext = textBuffer as String\n"                                     \
	"\t\t\tvalue = textBuffer\n"                                              \
	"\n"                                                                      \
	"\t\t\t/*\n"                                                              \
	"\t\t\t *\tExecute action\n"                                              \
	"\t\t\t */\n"                                                             \
	"\n"                                                                      \
	"\t\t\tswitch action {\n";

static const char *GSource4 =
	"\t\t\t\tdefault:\n"                                                      \
	"\t\t\t\t\tbreak;\n"                                                      \
	"\t\t\t}\n"                                                               \
	"\t\t}\n"                                                                 \
	"\t}\n"                                                                   \
	"}\n";

/************************************************************************/
/*																		*/
/*	State writer														*/
/*																		*/
/************************************************************************/

/*	OCLexSwiftGenerator::WriteArray
 *
 *		Write an array. This simply writes the list of items to an array
 */

void OCLexSwiftGenerator::WriteArray(FILE *f, uint32_t *list, size_t len)
{
	size_t i = 0;

	for (i = 0; i < len; ++i) {
		if (i) {
			fprintf(f,", ");
		}
		if ((i % 8) == 0) {
			if (i) fprintf(f,"\n");
			fprintf(f,"\t\t");
		}
		fprintf(f,"%3d",list[i]);
	}
	fprintf(f,"\n");
}

/*	OCLexSwiftGenerator::WriteStates
 *
 *		Write the states
 */

void OCLexSwiftGenerator::WriteStates(FILE *f)
{
	/*
	 *	Print the state sizes
	 */

	fprintf(f,"\tstatic let MAXSTATES: UInt16 =       %lu\n",dfaStates.size());
	fprintf(f,"\tstatic let MAXCHARCLASS: UInt16 =    %lu\n",charClasses.size());
	fprintf(f,"\tstatic let MAXACTIONS: UInt16 =      %lu\n\n",codeRules.size());

	/*
	 *	Generate the character class list.
	 */

	uint32_t carray[256];
	size_t i,len = charClasses.size();
	for (i = 0; i < 256; ++i) carray[i] = (uint16_t)len;
	for (i = 0; i < len; ++i) {
		const OCCharSet &set = charClasses[i];
		for (int j = 0; j < 256; ++j) {
			if (set.TestCharacter((unsigned char)j)) {
				carray[j] = (uint32_t)i;
			}
		}
	}

	fprintf(f,"\n");
	fprintf(f,"\t//  Character classes used during parsing:\n");
	fprintf(f,"\t//\n");
	for (i = 0; i < len; ++i) {
		const OCCharSet &cset = charClasses[i];
		fprintf(f,"\t//  %zu: %s\n",i,cset.ToString().c_str());
	}
	fprintf(f,"\n");

	fprintf(f,"\t/*  CharClass\n");
	fprintf(f,"\t *\n");
	fprintf(f,"\t *      Maps 8-bit character to character class\n");
	fprintf(f,"\t */\n\n");
	fprintf(f,"\tstatic let CharClass: [UInt16] = [\n");
	WriteArray(f, carray, 256);
	fprintf(f,"\t]\n\n");

	/*
	 *	Generate state actions. This is a table which maps from a state
	 *	index to an action index. Note if this DFA state has no rule, we
	 *	land on MAXACTION. If the DFA has a conditional rule, we go to
	 *	an index greater than MAXACTION.
	 */

	fprintf(f,"\t/*  StateActions\n");
	fprintf(f,"\t *\n");
	fprintf(f,"\t *      Maps states to actions. MAXACTION if this is not a terminal\n");
	fprintf(f,"\t */\n\n");

	size_t alen = codeRules.size();
	uint32_t swindex = (uint32_t)alen;
	len = dfaStates.size();
	uint32_t *scratch = (uint32_t *)malloc(len * sizeof(uint32_t));
	for (i = 0; i < len; ++i) {
		OCLexDFAState &state = dfaStates[i];

		if (state.endList.size() == 0) {
			scratch[i] = (uint32_t)alen;
		} else if ((state.endList.size() == 1) &&
				   (state.endList[0].startState.unconditional())) {
			scratch[i] = state.endList[0].endRule;
		} else {
			scratch[i] = ++swindex;
		}
	}

	fprintf(f,"\tstatic let StateActions: [UInt16] = [\n");
	WriteArray(f,scratch,len);
	fprintf(f,"\t]\n\n");
	free(scratch);

	/*
	 *	Generate the DFA state transitions
	 */

	size_t clen = charClasses.size();
	size_t tlen = len * clen;
	scratch = (uint32_t *)malloc(tlen * sizeof(uint32_t));

	size_t ptr = 0;
	for (i = 0; i < len; ++i) {
		OCLexDFAState &state = dfaStates[i];
		for (size_t j = 0; j < clen; ++j) {
			OCCharSet &cset = charClasses[j];

			/*
			 *	Find the transition which intersects
			 */

			uint32_t newState = (uint32_t)len;

			std::vector<OCLexDFATransition>::iterator t;
			for (t = state.list.begin(); t != state.list.end(); ++t) {
				if (t->set.Contains(cset)) {
					// This transition contains our cset. Set and move on
					newState = (uint32_t)t->state;
					break;
				}
			}

			scratch[ptr++] = newState;
		}
	}

	// New format
	fprintf(f,"\t/*  StateMachineIA, StateMachineJA, StateMachineA\n");
	fprintf(f,"\t *\n");
	        //   01234567890123456789012345678901234567890123456789012345678901234567890123456789
	fprintf(f,"\t *      Lex state machine in compressed sparce row storage format. We do this\n");
	fprintf(f,"\t *  in order to compact the resulting sparse matrix state machine so we don't\n");
	fprintf(f,"\t *  consume as much space. Decoding the new state becomes an O(log(N)) process\n");
	fprintf(f,"\t *  on the input character class as we use a binary search on the JA array.\n");
	fprintf(f,"\t *\n");
	fprintf(f,"\t *      See the article below for more information:\n");
	fprintf(f,"\t *\n");
	fprintf(f,"\t *      https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_.28CSR.2C_CRS_or_Yale_format.29\n");
	fprintf(f,"\t */\n\n");

	OCCompressStates comp(clen,len,scratch,(uint32_t)len);

	fprintf(f,"\tstatic let StateMachineIA: [UInt16] = [\n");
	WriteArray(f,comp.ia,comp.iwidth);
	fprintf(f,"\t]\n\n");

	fprintf(f,"\tstatic let StateMachineJA: [UInt16] = [\n");
	WriteArray(f,comp.ja,comp.asize);
	fprintf(f,"\t]\n\n");

	fprintf(f,"\tstatic let StateMachineA: [UInt16] = [\n");
	WriteArray(f,comp.a,comp.asize);
	fprintf(f,"\t]\n\n");

	free(scratch);
}

/*	OCLexSwiftGenerator::WriteActions
 *
 *		Write the actions state machine. This only writes the contents of
 *	the switch state, not the switch state itself.
 */

void OCLexSwiftGenerator::WriteActions(FILE *f)
{
	size_t i,len = codeRules.size();
	for (i = 0; i < len; ++i) {
		if (codeRules[i].code.length() > 0) {
			fprintf(f,"            case %zu:\n",i);
			fprintf(f,"                %s\n",codeRules[i].code.c_str());
		}
	}
}

/************************************************************************/
/*																		*/
/*	Write start states													*/
/*																		*/
/************************************************************************/

void OCLexSwiftGenerator::WriteStarts(FILE *f, const char *className)
{
	uint32_t index = (uint32_t)codeRules.size();

	fprintf(f,"\t/*\n");
	fprintf(f,"\t *  For conditional states this takes an end DFA state and\n");
	fprintf(f,"\t *  determines the proper end rule given the current start\n");
	fprintf(f,"\t *  conditionals.\n");
	fprintf(f,"\t */\n\n");
	fprintf(f,"\tprivate func conditionalAction(_ state: UInt16) -> UInt16\n");
	fprintf(f,"{\n");
	fprintf(f,"    switch state {\n");
	fprintf(f,"        default:\n");
	fprintf(f,"            return %s.MAXACTIONS;\n",className);

	uint32_t i,len;
	len = (uint32_t)dfaStates.size();
	for (i = 0; i < len; ++i) {
		OCLexDFAState &state = dfaStates[i];

		if ((state.endList.size() > 1) ||
			((state.endList.size() == 1) &&
				!state.endList[0].startState.unconditional())) {

			/*
			 *	This has at least one conditional rule or multiple rules.
			 *	Write them out
			 */

			bool atEnd = false;
			fprintf(f,"        case %d:\n",++index);

			uint32_t e,elen = (uint32_t)state.endList.size();
			for (e = 0; e < elen; ++e) {
				const OCLexDFAEnd &d = state.endList[e];

				if (d.startState.unconditional()) {
					fprintf(f,"            return %d;\n",d.endRule);
					atEnd = true;
					break;
				}

				bool flag = false;
				fprintf(f,"            if (");
				if (d.startState.startFlag()) {
					fprintf(f,"atSOL()");
					flag = true;
				}
				if (d.startState.endFlag()) {
					if (flag) {
						fprintf(f," && ");
					} else {
						flag = true;
					}
					fprintf(f,"atEOL()");
				}
				uint32_t m = d.startState.stateFlags(ruleStates);
				if (m) {
					if (flag) {
						fprintf(f," && ");
					}
					fprintf(f,"((%d & states) != 0)",m);
				}

				fprintf(f,") return %d;\n",d.endRule);
			}

			if (!atEnd) {
				fprintf(f,"            return %s.MAXACTIONS;\n",className);
			}
		}
	}

	fprintf(f,"    }\n");
	fprintf(f,"}\n\n");
}

/************************************************************************/
/*																		*/
/*	Fixed Files															*/
/*																		*/
/************************************************************************/

static const char *GFileInput =
	"//\tOCFileInput.swift\n"                                                 \
	"\n"                                                                      \
	"import Foundation\n"                                                     \
	"\n"                                                                      \
	"/*\tOCFileInput\n"                                                       \
	" *\n"                                                                    \
	" *\t\tThe input file stream must correspond to this protocol to read\n"  \
	" *\tthe contents of a file or data object. The byte returned is from 0 to 255,\n" \
	" *\tand EOF is marked with -1.\n"                                        \
	" */\n"                                                                   \
	"\n"                                                                      \
	"protocol OCFileInput {\n"                                                \
	"\tfunc readByte() -> Int\n"                                              \
	"\tfunc peekByte() -> Int\n"                                              \
	"}\n"                                                                     \
	"\n";

static const char *GLexInput =
	"//\tOCLexInput.swift\n"                                                  \
	"\n"                                                                      \
	"import Foundation\n"                                                     \
	"\n"                                                                      \
	"/*\tOCLexInput\n"                                                        \
	" *\n"                                                                    \
	" *\t\tThe protocol for our lex reader file that the lex stream must\n"   \
	" *\tprovide. This is the same as the protocol generated as part of the OCYacc\n" \
	" *\toutput, and allows us to glue the Lexer and Parser together.\n"      \
	" */\n"                                                                   \
	"\n"                                                                      \
	"protocol OCLexInput {\n"                                                 \
	"\tvar line: Int { get }\n"                                               \
	"\tvar column: Int { get }\n"                                             \
	"\tvar filename: String\? { get }\n"                                      \
	"\tvar text: String { get }\n"                                            \
	"\tvar abort: String\? { get }\n"                                         \
	"\tvar value: AnyObject\? { get }\n"                                      \
	"\n"                                                                      \
	"\tfunc lex() -> Int\n"                                                   \
	"}\n";


/*	OCLexSwiftGenerator::WriteOCFileInput
 *
 *		This generates the fixed OCFileInput file
 */

void OCLexSwiftGenerator::WriteOCFileInput(FILE *f)
{
	fprintf(f,"%s",GFileInput);
}

/*	OCLexSwiftGenerator::WriteOCLexInput
 *
 *		This generates the fixed OCLexInput file
 */

void OCLexSwiftGenerator::WriteOCLexInput(FILE *f)
{
	fprintf(f,"%s",GLexInput);
}

/************************************************************************/
/*																		*/
/*	Parser Definitions													*/
/*																		*/
/************************************************************************/

/*	OCLexSwiftGenerator::WriteOCFile
 *
 *		This generates the various tables and writes the Objective C
 *	lex file
 */

void OCLexSwiftGenerator::WriteOCFile(const char *className, const char *outName, FILE *f)
{
	OCTextReplace replace;

	replace.map["classname"] = className;
	replace.map["outname"] = outName;

	replace.map["%"] = declCode;
	replace.map["header"] = classHeader;
	replace.map["global"] = classGlobal;
	replace.map["local"] = classLocal;
	replace.map["init"] = classInit;
	replace.map["finish"] = classFinish;
	replace.map["code"] = endCode;

	// Write opening declarations
	replace.write(GSource1,f);

	// State declarations
	WriteStates(f);

	// Write body
	replace.write(GSource2,f);

	// Conditional class table
	WriteStarts(f,className);

	// Lexer engine
	replace.write(GSource3,f);

	// Action states
	WriteActions(f);

	// And the rest of the stuff
	replace.write(GSource4,f);
}
