//
//  OCYaccSwiftGenerator.cpp
//  ocyacc
//
//  Created by William Woody on 4/22/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#include "OCYaccSwiftGenerator.h"
#include <OCUtilities.h>


/************************************************************************/
/*																		*/
/*	Source Constants													*/
/*																		*/
/************************************************************************/

static const char *GParser1 =
	"//  $(outname)\n"                                                        \
	"\n"                                                                      \
	"import Cocoa\n"                                                          \
	"\n"                                                                      \
	"$(header)\n"                                                             \
	"$(%)\n"                                                                  \
	"\n"                                                                      \
	"protocol $(classname)Error {\n"                                          \
	"\tfunc error(parser: $(classname), line: Int, column: Int, filename: String\?, errorCode: Int, data: [String: AnyObject]\?)\n" \
	"}\n"                                                                     \
	"\n"                                                                      \
	"class $(classname) {\n"                                                  \
	"\t/*\n"                                                                  \
	"\t *\tYacc Constants\n"                                                  \
	"\t */\n"                                                                 \
	"\n";

static const char *GParser2 =
	"\n"                                                                      \
	"\t$(global)\n"                                                           \
	"\t$(local)\n"                                                            \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tError Values\n"                                                    \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tpublic static let ERRORMASK_WARNING = 0x8000\n"                        \
	"\tpublic static let ERROR_SYNTAX = 0x0001\n"                             \
	"\tpublic static let ERROR_MISSINGTOKEN = 0x0002\n"                       \
	"\tpublic static let ERROR_MISSINGTOKENS = 0x0003\n"                      \
	"\tpublic static let ERROR_STARTERRORID = 0x0100\n"                       \
	"\n"                                                                      \
	"    $(errors)\n"                                                         \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tStack\n"                                                           \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tprivate struct Stack {\n"                                              \
	"\t\tvar state: UInt16\n"                                                 \
	"\t\tvar line: Int\n"                                                     \
	"\t\tvar column: Int\n"                                                   \
	"\t\tvar filename: String\?\n"                                            \
	"\t\tvar value: AnyObject\?\n"                                            \
	"\n"                                                                      \
	"\t\tinit()\n"                                                            \
	"\t\t{\n"                                                                 \
	"\t\t\tstate = 0\n"                                                       \
	"\t\t\tline = 0\n"                                                        \
	"\t\t\tcolumn = 0\n"                                                      \
	"\t\t\tfilename = nil\n"                                                  \
	"\t\t\tvalue = nil\n"                                                     \
	"\t\t}\n"                                                                 \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tInternal values\n"                                                 \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tprivate var stack: [Stack]\n"                                          \
	"\tprivate var lex: OCLexInput\n"                                         \
	"\n"                                                                      \
	"\t// Error support\n"                                                    \
	"\tprivate var success: Bool\n"                                           \
	"\tprivate var errorCount: Int\n"                                         \
	"\n"                                                                      \
	"\tprivate var hasError: Bool\n"                                          \
	"\tprivate var errorLine: Int\n"                                          \
	"\tprivate var errorColumn: Int\n"                                        \
	"\tprivate var errorFileName: String\?\n"                                 \
	"\n"                                                                      \
	"\t// Delegate\n"                                                         \
	"\tpublic var errorDelegate: $(classname)Error\?\n"                       \
	"\n"                                                                      \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tStartup\n"                                                         \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tpublic init(lexer: OCLexInput)\n"                                      \
	"\t{\n"                                                                   \
	"\t\tlex = lexer\n"                                                       \
	"\n"                                                                      \
	"\t\tstack = Array()\n"                                                   \
	"\t\tsuccess = true\n"                                                    \
	"\t\terrorCount = 0;\n"                                                   \
	"\t\thasError = false\n"                                                  \
	"\t\terrorLine = 0\n"                                                     \
	"\t\terrorColumn = 0\n"                                                   \
	"\t\terrorFileName = nil\n"                                               \
	"\t\terrorDelegate = nil\n"                                               \
	"\n"                                                                      \
	"        $(init)\n"                                                       \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\tdeinit {\n"                                                            \
	"\t    $(finish)\n"                                                       \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tProcess production rule. This processes the production rule and creates\n" \
	"\t *\ta new stack state with the rule reduction.\n"                      \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tprivate func processReduction(_ rule: Int16) -> $(classname).Stack\n"  \
	"\t{\n"                                                                   \
	"\t\t// Get production len, for translating $1...$N into something useful\n" \
	"\t\tlet pos = stack.count - Int($(classname).RuleLength[Int(rule)])\n"   \
	"\n"                                                                      \
	"\t\t// Set up initial state.\n"                                          \
	"\t\tvar s = $(classname).Stack()\n"                                      \
	"\n"                                                                      \
	"\t\t// File position of reduced rule is first token of the symbols reduced\n" \
	"\t\tlet fs = stack[pos]\n"                                               \
	"\t\ts.filename = fs.filename;\n"                                         \
	"\t\ts.line = fs.line;\n"                                                 \
	"\t\ts.column = fs.column;\n"                                             \
	"\n"                                                                      \
	"\t\tdo {\n"                                                              \
	"\t\t\ttry reduction(rule: rule, at: pos, with: &s)\n"                    \
	"\t\t} catch {\n"                                                         \
	"\t\t\t// Ignore errors. Can happen when the stack is in an error state\n" \
	"\t\t}\n"                                                                 \
	"\n"                                                                      \
	"\t\treturn s\n"                                                          \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\tprivate func reduction(rule: Int16, at pos: Int, with s: inout $(classname).Stack) throws\n" \
	"\t{\n"                                                                   \
	"\t\t// Now process production.\n"                                        \
	"\t\t//\n"                                                                \
	"\t\t// Note that $$ translated into (s.value), and\n"                    \
	"\t\t// $n translates into ((<type> *)(self.stack[pos+(n-1)])), where <type>\n" \
	"\t\t// is the declared type of the token or production rule.\n"          \
	"\n"                                                                      \
	"\t\t// Note: value is AnyObject, but we use as! cast to coerce, and assume\n" \
	"\t\t// the exception is caught and ignored if it all goes south.\n"      \
	"\t\tswitch rule {\n";

static const char *GParser3 =
	"\t\t\tdefault:\n"                                                        \
	"\t\t\t\tbreak\n"                                                         \
	"\t\t}\n"                                                                 \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\tprivate func actionForState(_ state: UInt16, token t: Int) -> Int\n"   \
	"\t{\n"                                                                   \
	"\t\tvar min,max,mid: Int\n"                                              \
	"\n"                                                                      \
	"\t\tlet token = (t == -1) \? $(classname).K_EOFTOKEN : t\n"              \
	"\n"                                                                      \
	"\t\t/* Find range */\n"                                                  \
	"\t\tmin = Int($(classname).ActionI[Int(state)])\n"                       \
	"\t\tmax = Int($(classname).ActionI[Int(state)+1])\n"                     \
	"\n"                                                                      \
	"\t\t/* Binary search for value in ja */\n"                               \
	"\t\twhile min < max {\n"                                                 \
	"\t\t\tmid = (min + max)/2\n"                                             \
	"\t\t\tlet j = $(classname).ActionJ[mid]\n"                               \
	"\t\t\tif token == j {\n"                                                 \
	"\t\t\t\treturn Int($(classname).ActionA[mid])\n"                         \
	"\t\t\t} else if token < j {\n"                                           \
	"\t\t\t\tmax = mid\n"                                                     \
	"\t\t\t} else {\n"                                                        \
	"\t\t\t\tmin = mid + 1\n"                                                 \
	"\t\t\t}\n"                                                               \
	"\t\t}\n"                                                                 \
	"\t\treturn Int.max\n"                                                    \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tLook up the goto value for the state and token (production rule) provided.\n" \
	"\t *\tOperates the same as actionForState, except we look in the goto table.\n" \
	"\t *\tIn teory I guess we could combine these two tables (as productions and\n" \
	"\t *\ttokens do not overlap).\n"                                         \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tprivate func goto(state: UInt16, production token: UInt32) -> Int\n"   \
	"\t{\n"                                                                   \
	"\t\tvar min,max,mid: Int\n"                                              \
	"\n"                                                                      \
	"\t\t/* Find range */\n"                                                  \
	"\t\tmin = Int($(classname).GotoI[Int(state)])\n"                         \
	"\t\tmax = Int($(classname).GotoI[Int(state)+1])\n"                       \
	"\n"                                                                      \
	"\t\t/* Binary search for value in ja */\n"                               \
	"\t\twhile min < max {\n"                                                 \
	"\t\t\tmid = (min + max)/2\n"                                             \
	"\t\t\tlet j = $(classname).GotoJ[mid]\n"                                 \
	"\t\t\tif token == j {\n"                                                 \
	"\t\t\t\treturn Int($(classname).GotoA[mid])\n"                           \
	"\t\t\t} else if token < j {\n"                                           \
	"\t\t\t\tmax = mid\n"                                                     \
	"\t\t\t} else {\n"                                                        \
	"\t\t\t\tmin = mid + 1\n"                                                 \
	"\t\t\t}\n"                                                               \
	"\t\t}\n"                                                                 \
	"\t\treturn Int.max\n"                                                    \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tErrors. This formats and prints the specified error\n"             \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tprivate func error(code: Int, data: [String: AnyObject])\n"            \
	"\t{\n"                                                                   \
	"\t\tif errorDelegate == nil {\t// No error handling, ignore.\n"          \
	"\t\t\treturn\n"                                                          \
	"\t\t}\n"                                                                 \
	"\t\tif errorCount > 0 {\t\t\t// skip until synced on 3 shifts\n"         \
	"\t\t\treturn\n"                                                          \
	"\t\t}\n"                                                                 \
	"\n"                                                                      \
	"\t\t// Call delegate with current token position\n"                      \
	"\t\t// Token position is the topmost symbol\n"                           \
	"\n"                                                                      \
	"\t\tif hasError {\n"                                                     \
	"\t\t\terrorDelegate\?.error(parser: self, line: errorLine, column: errorColumn, filename: errorFileName, errorCode: code, data: data)\n" \
	"\t\t} else {\n"                                                          \
	"\t\t\tif let top = stack.last {\n"                                       \
	"\t\t\t\terrorDelegate\?.error(parser: self, line: top.line, column: top.column, filename: top.filename, errorCode: code, data: data)\n" \
	"\t\t\t}\n"                                                               \
	"\t\t}\n"                                                                 \
	"\n"                                                                      \
	"\t\tif 0 == (code & $(classname).ERRORMASK_WARNING) {\n"                 \
	"\t\t\tsuccess = false\n"                                                 \
	"\t\t\terrorCount = 3\n"                                                  \
	"\t\t}\n"                                                                 \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\tprivate func error(code: Int)\n"                                       \
	"\t{\n"                                                                   \
	"\t\terror(code: code, data: [:])\n"                                      \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\tprivate func errorOK()\n"                                              \
	"\t{\n"                                                                   \
	"\t\terrorCount = 0\n"                                                    \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\tprivate func tokenToString(_ token: UInt32) -> String\n"               \
	"\t{\n"                                                                   \
	"\t\tif token >= $(classname).K_FIRSTTOKEN {\n"                           \
	"\t\t\treturn $(classname).TokenArray[Int(token) - $(classname).K_FIRSTTOKEN]\n" \
	"\t\t} else {\n"                                                          \
	"\t\t\treturn String.init(Unicode.Scalar(token) \?\? Unicode.Scalar.init(UInt8(32)))\n" \
	"\t\t}\n"                                                                 \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"\tprivate func reduce(action: Int16) -> Bool\n"                          \
	"\t{\n"                                                                   \
	"\t\t// Determine the new state we\'re transitioning to.\n"               \
	"\t\tlet production = $(classname).RuleProduction[Int(action)]\n"         \
	"\t\tlet length = $(classname).RuleLength[Int(action)]\n"                 \
	"\n"                                                                      \
	"\t\t// We pull the state we\'d have after popping the stack.\n"          \
	"\t\tlet statep = stack[stack.count - Int(length) - 1].state\n"           \
	"\n"                                                                      \
	"\t\t// Calculate the state we should transition to\n"                    \
	"\t\tlet newState = goto(state: statep, production: production)\n"        \
	"\t\tif newState == Int.max {\n"                                          \
	"\t\t\treturn false\n"                                                    \
	"\t\t}\n"                                                                 \
	"\n"                                                                      \
	"\t\t// Process reduction rule\n"                                         \
	"\t\tvar state = processReduction(action)\n"                              \
	"\n"                                                                      \
	"\t\t// Update state\n"                                                   \
	"\t\tstate.state = UInt16(newState)\n"                                    \
	"\n"                                                                      \
	"\t\t// Pop the stack\n"                                                  \
	"\t\tstack.removeSubrange(stack.count - Int(length) ..< stack.count)\n"   \
	"\n"                                                                      \
	"\t\t// Push new state\n"                                                 \
	"\t\tstack.append(state)\n"                                               \
	"\n"                                                                      \
	"\t\t// Clear error marker\n"                                             \
	"\t\thasError = false\n"                                                  \
	"\n"                                                                      \
	"\t\t// Return success\n"                                                 \
	"\t\treturn true\n"                                                       \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"$(code)\n"                                                               \
	"\n"                                                                      \
	"\t/*\n"                                                                  \
	"\t *\tParser engine. Returns NO if there was an error during processing. Note\n" \
	"\t *\tthat as we uncover errors we call our delegate for error handling. This\n" \
	"\t *\timplements the algorithm described in the Dragon Book, Algorithm 4.7.\n" \
	"\t */\n"                                                                 \
	"\n"                                                                      \
	"\tpublic func parse() -> Bool\n"                                         \
	"\t{\n"                                                                   \
	"\t\tvar s: $(classname).Stack\n"                                         \
	"\t\tvar a: Int\n"                                                        \
	"\n"                                                                      \
	"\t\t/*\n"                                                                \
	"\t\t *\tStep 1: reset and push the empty state.\n"                       \
	"\t\t */\n"                                                               \
	"\n"                                                                      \
	"\t\tsuccess = true\n"                                                    \
	"\t\tstack = []\n"                                                        \
	"\n"                                                                      \
	"\t\thasError = false\n"                                                  \
	"\t\terrorCount = 0\n"                                                    \
	"\n"                                                                      \
	"\t\ts = $(classname).Stack()\n"                                          \
	"\t\ts.state = $(classname).K_STARTSTATE\n"                               \
	"\t\ts.filename = lex.filename\n"                                         \
	"\t\ts.line = lex.line\n"                                                 \
	"\t\ts.column = lex.column\n"                                             \
	"\n"                                                                      \
	"\t\tstack.append(s)\n"                                                   \
	"\n"                                                                      \
	"\t\t/*\n"                                                                \
	"\t\t *\tNow repeat forever:\n"                                           \
	"\t\t */\n"                                                               \
	"\n"                                                                      \
	"\t\ta = lex.lex()\n"                                                     \
	"\t\twhile true {\n"                                                      \
	"\t\t\ts = stack.last!\n"                                                 \
	"\n"                                                                      \
	"\t\t\t/*\n"                                                              \
	"\t\t\t *\tDetermine if this is the end state. If so, then we immediately\n" \
	"\t\t\t *\tquit. We assume the user has set the production rule at the\n" \
	"\t\t\t *\ttop, so we can simply drop the stack\n"                        \
	"\t\t\t */\n"                                                             \
	"\n"                                                                      \
	"\t\t\tif s.state == $(classname).K_ACCEPTSTATE {\n"                      \
	"\t\t\t\tstack.removeAll()\n"                                             \
	"\t\t\t\treturn success\n"                                                \
	"\t\t\t}\n"                                                               \
	"\n"                                                                      \
	"\t\t\t/*\n"                                                              \
	"\t\t\t *\tNow determine the action and shift, reduce or handle error as\n" \
	"\t\t\t *\tappropriate\n"                                                 \
	"\t\t\t */\n"                                                             \
	"\n"                                                                      \
	"\t\t\tvar action = actionForState(s.state, token: a)\n"                  \
	"\t\t\tvar foundError = false\n"                                          \
	"\n"                                                                      \
	"\t\t\tif action == Int.max {\n"                                          \
	"\t\t\t\t/*\n"                                                            \
	"\t\t\t\t *\tHandle error. First, note we have an error, and note the\n"  \
	"\t\t\t\t *\tsymbol on which our error took place.\n"                     \
	"\t\t\t\t */\n"                                                           \
	"\n"                                                                      \
	"\t\t\t\tsuccess = false\n"                                               \
	"\t\t\t\terrorFileName = lex.filename\n"                                  \
	"\t\t\t\terrorLine = lex.line\n"                                          \
	"\t\t\t\terrorColumn = lex.column\n"                                      \
	"\t\t\t\thasError = true\n"                                               \
	"\n"                                                                      \
	"\t\t\t\t/*\n"                                                            \
	"\t\t\t\t *\tFirst, scan backwards from the current state, looking for one\n" \
	"\t\t\t\t *\twhich has an \'error\' symbol.\n"                            \
	"\t\t\t\t */\n"                                                           \
	"\n"                                                                      \
	"\t\t\t\tvar ix = stack.count\n"                                          \
	"\t\t\t\twhile (ix > 0) && !foundError {\n"                               \
	"\t\t\t\t\tix -= 1\n"                                                     \
	"\t\t\t\t\tlet si = stack[ix]\n"                                          \
	"\t\t\t\t\taction = actionForState(si.state, token: $(classname).K_ERRORTOKEN)\n" \
	"\t\t\t\t\tif (action >= 0) && (action != Int.max) {\n"                   \
	"\n"                                                                      \
	"\t\t\t\t\t\t/*\n"                                                        \
	"\t\t\t\t\t\t *\tEncountered error state. If the user has defined an\n"   \
	"\t\t\t\t\t\t *\terror token, we ultimately will want to (a) unwind\n"    \
	"\t\t\t\t\t\t *\tthe stack until we find a state which handles the\n"     \
	"\t\t\t\t\t\t *\terror transition. We then .\n"                           \
	"\t\t\t\t\t\t */\n"                                                       \
	"\n"                                                                      \
	"\t\t\t\t\t\tif ix+1 < stack.count {\n"                                   \
	"\t\t\t\t\t\t\tstack.removeSubrange(ix+1..<stack.count)\n"                \
	"\t\t\t\t\t\t}\n"                                                         \
	"\n"                                                                      \
	"\t\t\t\t\t\t/*\n"                                                        \
	"\t\t\t\t\t\t *\tAt this point we perform a shift to our new error\n"     \
	"\t\t\t\t\t\t *\tstate.\n"                                                \
	"\t\t\t\t\t\t */\n"                                                       \
	"\n"                                                                      \
	"\t\t\t\t\t\tvar s = $(classname).Stack()\n"                              \
	"\t\t\t\t\t\ts.state = UInt16(action)\n"                                  \
	"\t\t\t\t\t\ts.value = lex.value\n"                                       \
	"\n"                                                                      \
	"\t\t\t\t\t\ts.filename = lex.filename\n"                                 \
	"\t\t\t\t\t\ts.line = lex.line\n"                                         \
	"\t\t\t\t\t\ts.column = lex.column\n"                                     \
	"\n"                                                                      \
	"\t\t\t\t\t\tstack.append(s)\n"                                           \
	"\n"                                                                      \
	"\t\t\t\t\t\t/*\n"                                                        \
	"\t\t\t\t\t\t *\tSecond, we start pulling symbols until we find a symbol\n" \
	"\t\t\t\t\t\t *\tthat shifts, or until we hit the end of file symbol.\n"  \
	"\t\t\t\t\t\t *\tThis becomes our current token for parsing\n"            \
	"\t\t\t\t\t\t */\n"                                                       \
	"\n"                                                                      \
	"\t\t\t\t\t\twhile (true) {\n"                                            \
	"\t\t\t\t\t\t\ta = lex.lex()\n"                                           \
	"\t\t\t\t\t\t\taction = actionForState(s.state, token: a)\n"              \
	"\t\t\t\t\t\t\tif (action >= 0) && (action != Int.max) {\n"               \
	"\t\t\t\t\t\t\t\t/*\n"                                                    \
	"\t\t\t\t\t\t\t\t *\tValid shift. This becomes our current token,\n"      \
	"\t\t\t\t\t\t\t\t *\tand we resume processing.\n"                         \
	"\t\t\t\t\t\t\t\t */\n"                                                   \
	"\n"                                                                      \
	"\t\t\t\t\t\t\t\tfoundError = true\n"                                     \
	"\t\t\t\t\t\t\t\tbreak\n"                                                 \
	"\t\t\t\t\t\t\t} else if (a == $(classname).K_EOFTOKEN) || (a == -1) {\n" \
	"\t\t\t\t\t\t\t\t/*\n"                                                    \
	"\t\t\t\t\t\t\t\t *\tWe ran out of tokens. At this point all\n"           \
	"\t\t\t\t\t\t\t\t *\twe can do is print an error and force quit\n"        \
	"\t\t\t\t\t\t\t\t */\n"                                                   \
	"\n"                                                                      \
	"\t\t\t\t\t\t\t\terror(code: $(classname).ERROR_SYNTAX)\n"                \
	"\t\t\t\t\t\t\t\tstack.removeAll()\n"                                     \
	"\t\t\t\t\t\t\t\treturn false\n"                                          \
	"\t\t\t\t\t\t\t}\n"                                                       \
	"\t\t\t\t\t\t}\n"                                                         \
	"\t\t\t\t\t}\n"                                                           \
	"\t\t\t\t}\n"                                                             \
	"\t\t\t\tif foundError {\n"                                               \
	"\t\t\t\t\tcontinue\n"                                                    \
	"\t\t\t\t}\n"                                                             \
	"\n"                                                                      \
	"\t\t\t\t/*\n"                                                            \
	"\t\t\t\t *\tIf we reach this point, there is no error we can recover to.\n" \
	"\t\t\t\t *\tSo figure this out on our own.\n"                            \
	"\t\t\t\t *\n"                                                            \
	"\t\t\t\t *\tFirst, we see if the state we\'re in has a limited number of\n" \
	"\t\t\t\t *\tchoices. For example, in C, the \'for\' keyword will always be\n" \
	"\t\t\t\t *\tfollowed by a \'(\' token, so we can offer to automatically\n" \
	"\t\t\t\t *\tinsert that token.\n"                                        \
	"\t\t\t\t */\n"                                                           \
	"\n"                                                                      \
	"\t\t\t\tlet actionMin: Int = Int($(classname).ActionI[Int(s.state)])\n"  \
	"\t\t\t\tlet actionMax: Int = Int($(classname).ActionI[Int(s.state) + 1])\n" \
	"\t\t\t\tvar actionVal: Int = actionMin\n"                                \
	"\t\t\t\tvar actionState: Int16 = -1\n"                                   \
	"\n"                                                                      \
	"\t\t\t\tfor ix in actionMin..<actionMax {\n"                             \
	"\t\t\t\t\tlet act = $(classname).ActionA[ix]\n"                          \
	"\t\t\t\t\tif actionState == -1 {\n"                                      \
	"\t\t\t\t\t\tif act >= 0 {\n"                                             \
	"\t\t\t\t\t\t\tactionState = act;\n"                                      \
	"\t\t\t\t\t\t\tactionVal = ix\n"                                          \
	"\t\t\t\t\t\t}\n"                                                         \
	"\t\t\t\t\t} else {\n"                                                    \
	"\t\t\t\t\t\tactionState = -1\n"                                          \
	"\t\t\t\t\t\tbreak\n"                                                     \
	"\t\t\t\t\t}\n"                                                           \
	"\t\t\t\t}\n"                                                             \
	"\n"                                                                      \
	"\t\t\t\tif actionState != -1 {\n"                                        \
	"\t\t\t\t\t/*\n"                                                          \
	"\t\t\t\t\t *\tWe can accomplish this transition with one token. Print\n" \
	"\t\t\t\t\t *\tan error, and do a shift on the state with an empty value.\n" \
	"\t\t\t\t\t */\n"                                                         \
	"\n"                                                                      \
	"\t\t\t\t\tlet tokenStr = tokenToString($(classname).ActionJ[actionVal])\n" \
	"\t\t\t\t\terror(code: $(classname).ERROR_MISSINGTOKEN, data: [\"token\": tokenStr as AnyObject])\n" \
	"\n"                                                                      \
	"\n"                                                                      \
	"\t\t\t\t\t/*\n"                                                          \
	"\t\t\t\t\t *\tPerform a shift but do not pull a new token\n"             \
	"\t\t\t\t\t */\n"                                                         \
	"\n"                                                                      \
	"\t\t\t\t\tlet top = stack.last!\n"                                       \
	"\n"                                                                      \
	"\t\t\t\t\tvar s = $(classname).Stack()\n"                                \
	"\n"                                                                      \
	"\t\t\t\t\ts.state = UInt16(actionState)\n"                               \
	"\t\t\t\t\ts.value = lex.value\n"                                         \
	"\n"                                                                      \
	"\t\t\t\t\ts.filename = top.filename\n"                                   \
	"\t\t\t\t\ts.line = top.line\n"                                           \
	"\t\t\t\t\ts.column = top.column\n"                                       \
	"\n"                                                                      \
	"\t\t\t\t\tstack.append(s)\n"                                             \
	"\t\t\t\t\tcontinue\n"                                                    \
	"\t\t\t\t}\n"                                                             \
	"\n"                                                                      \
	"\t\t\t\t/*\n"                                                            \
	"\t\t\t\t *\tSee if we have a limited choice in reductions. If this can\n" \
	"\t\t\t\t *\tonly reduce to a single state, try that reduction.\n"        \
	"\t\t\t\t */\n"                                                           \
	"\n"                                                                      \
	"\t\t\t\tactionState = 0\n"                                               \
	"\t\t\t\tfor ix in actionMin..<actionMax {\n"                             \
	"\t\t\t\t\tlet act = $(classname).ActionA[ix]\n"                          \
	"\t\t\t\t\tif actionState == 0 {\n"                                       \
	"\t\t\t\t\t\tif (act < 0) && (actionState != act) {\n"                    \
	"\t\t\t\t\t\t\tactionState = act;\n"                                      \
	"\t\t\t\t\t\t}\n"                                                         \
	"\t\t\t\t\t} else {\n"                                                    \
	"\t\t\t\t\t\tactionState = 0\n"                                           \
	"\t\t\t\t\t\tbreak\n"                                                     \
	"\t\t\t\t\t}\n"                                                           \
	"\t\t\t\t}\n"                                                             \
	"\n"                                                                      \
	"\t\t\t\tif actionState != 0 {\n"                                         \
	"\t\t\t\t\t/*\n"                                                          \
	"\t\t\t\t\t *\tWe have one possible reduction. Try that. Note that this\n" \
	"\t\t\t\t\t *\twill trigger a syntax error since we\'re reducing down\n"  \
	"\t\t\t\t\t *\twithout the follow token. My hope is that the state we\n"  \
	"\t\t\t\t\t *\ttransition to has a limited set of next tokens to follow.\n" \
	"\t\t\t\t\t */\n"                                                         \
	"\n"                                                                      \
	"\t\t\t\t\t_ = reduce(action: Int16(action))\n"                           \
	"\t\t\t\t\tcontinue;\n"                                                   \
	"\t\t\t\t}\n"                                                             \
	"\n"                                                                      \
	"\t\t\t\t/*\n"                                                            \
	"\t\t\t\t *\tIf we have a limited number of tokens which can follow,\n"   \
	"\t\t\t\t *\tprint a list of them. Then shift by the first one we\n"      \
	"\t\t\t\t *\tfind. We don\'t do this if the number of shifts is greater\n" \
	"\t\t\t\t *\tthan five.\n"                                                \
	"\t\t\t\t */\n"                                                           \
	"\n"                                                                      \
	"\t\t\t\tif actionMax - actionMin <= 5 {\n"                               \
	"\t\t\t\t\tvar list: [String] = []\n"                                     \
	"\t\t\t\t\tfor ix in actionMin..<actionMax {\n"                           \
	"\t\t\t\t\t\tlist.append(tokenToString($(classname).ActionJ[ix]))\n"      \
	"\t\t\t\t\t}\n"                                                           \
	"\n"                                                                      \
	"\t\t\t\t\terror(code: $(classname).ERROR_MISSINGTOKENS, data: [\"tokens\": list as AnyObject])\n" \
	"\n"                                                                      \
	"\t\t\t\t\t/*\n"                                                          \
	"\t\t\t\t\t *\tNow we artificially insert the first of the list of\n"     \
	"\t\t\t\t\t *\ttokens as our action and continue.\n"                      \
	"\t\t\t\t\t */\n"                                                         \
	"\n"                                                                      \
	"\t\t\t\t\ta = Int($(classname).ActionJ[actionMin])\n"                    \
	"\t\t\t\t\tcontinue;\n"                                                   \
	"\t\t\t\t}\n"                                                             \
	"\n"                                                                      \
	"\t\t\t\t/*\n"                                                            \
	"\t\t\t\t *\tIf we get here, things just went too far south. So we\n"     \
	"\t\t\t\t *\tskip a token, print syntax error and move on\n"              \
	"\t\t\t\t */\n"                                                           \
	"\n"                                                                      \
	"\t\t\t\terror(code:$(classname).ERROR_SYNTAX)\n"                         \
	"\t\t\t\ta = lex.lex()\n"                                                 \
	"\t\t\t\tif a == -1 {\n"                                                  \
	"\t\t\t\t\treturn false\n"                                                \
	"\t\t\t\t}\n"                                                             \
	"\n"                                                                      \
	"\t\t\t} else if action >= 0 {\n"                                         \
	"\t\t\t\t/*\n"                                                            \
	"\t\t\t\t *\tShift operation.\n"                                          \
	"\t\t\t\t */\n"                                                           \
	"\n"                                                                      \
	"\t\t\t\t// Shift\n"                                                      \
	"\t\t\t\tvar s = $(classname).Stack()\n"                                  \
	"\n"                                                                      \
	"\t\t\t\ts.state = UInt16(action)\n"                                      \
	"\t\t\t\ts.value = lex.value\n"                                           \
	"\n"                                                                      \
	"\t\t\t\ts.filename = lex.filename\n"                                     \
	"\t\t\t\ts.line = lex.line\n"                                             \
	"\t\t\t\ts.column = lex.column\n"                                         \
	"\n"                                                                      \
	"\t\t\t\tstack.append(s)\n"                                               \
	"\n"                                                                      \
	"\t\t\t\t// Advance to next token.\n"                                     \
	"\t\t\t\ta = lex.lex()\n"                                                 \
	"\n"                                                                      \
	"\t\t\t\t// Decrement our error count. If this is non-zero we\'re in an\n" \
	"\t\t\t\t// error state, and we don\'t pass spurrous errors upwards\n"    \
	"\t\t\t\tif errorCount > 0 {\n"                                           \
	"\t\t\t\t\terrorCount -= 1\n"                                             \
	"\t\t\t\t}\n"                                                             \
	"\t\t\t} else {\n"                                                        \
	"\t\t\t\t/*\n"                                                            \
	"\t\t\t\t *\tReduce action. (Reduce is < 0, and the production to reduce\n" \
	"\t\t\t\t *\tby is given below\n"                                         \
	"\t\t\t\t */\n"                                                           \
	"\n"                                                                      \
	"\t\t\t\taction = -action-1\n"                                            \
	"\n"                                                                      \
	"\t\t\t\tif !reduce(action: Int16(action)) {\n"                           \
	"\n"                                                                      \
	"\t\t\t\t\t// If there is an error, this handles the error.\n"            \
	"\t\t\t\t\t// (This should not happen in practice).\n"                    \
	"\t\t\t\t\terror(code:$(classname).ERROR_SYNTAX)\n"                       \
	"\n"                                                                      \
	"\t\t\t\t\t// Advance to next token.\n"                                   \
	"\t\t\t\t\ta = lex.lex()\n"                                               \
	"\t\t\t\t}\n"                                                             \
	"\t\t\t}\n"                                                               \
	"\t\t}\n"                                                                 \
	"\t}\n"                                                                   \
	"\n"                                                                      \
	"}\n";

/************************************************************************/
/*																		*/
/*	Construction/Destruction											*/
/*																		*/
/************************************************************************/

/*	OCYaccSwiftGenerator::OCYaccSwiftGenerator
 *
 *		Construt this class
 */

OCYaccSwiftGenerator::OCYaccSwiftGenerator(const OCYaccParser &p, const OCYaccLR1 &y) : parser(p), state(y)
{
}

/*	OCYaccSwiftGenerator::~OCYaccSwiftGenerator
 *
 *		Destructor
 */

OCYaccSwiftGenerator::~OCYaccSwiftGenerator()
{
}

/************************************************************************/
/*																		*/
/*	Support																*/
/*																		*/
/************************************************************************/

/*	OCYaccSwiftGenerator::WriteRule
 *
 *		Write rule, translating $$ and $n, unless it's within a string.
 *	We detect strings by scanning for double quotes
 */

void OCYaccSwiftGenerator::WriteRule(FILE *f, const OCYaccLR1::Reduction &rule)
{
	char q = 0;
	char buffer[256];
	std::string ret;

	const char *ptr = rule.code.c_str();

	while (*ptr) {
		// Skip anything following a backslash.
		char c = *ptr++;

		if (c == '\\') {
			ret.push_back(c);
			if (*ptr) {
				ret.push_back(*ptr++);
			}
			continue;
		}

		if ((c == '"') || (c == '\'')) {
			if (q == c) q = 0;
			else q = c;
		}

		// q is set if we're inside a string
		if (q) {
			ret.push_back(c);
			continue;
		}

		// We're not in a string. Scan for $
		if (c == '$') {
			if (*ptr == '$') {
				// $$; get type and prepend if not part of assignment
				++ptr;

				const char *x = ptr;
				while (isspace(*x)) ++x;
				bool isAssign = (*x == '=');

				if (isAssign) {
					ret += "s.value";
				} else {
					if (rule.prodType.length() == 0) {
						fprintf(stderr,"Warning: Production for rule %s has no type\n",rule.prodDebug.c_str());
						ret += "(s.value)";
					} else {
						ret += " (s.value as! ";
						ret += rule.prodType;
						ret += ")";
					}
				}
			} else if (isdigit(*ptr)) {
				// $n; extract number value and construct proper substitution
				size_t value = 0;
				while (isdigit(*ptr)) {
					value = (value * 10) + *ptr++ - '0';
				}

				if ((value < 1) || (value > rule.types.size())) {
					value = 1;

					fprintf(stderr,"Warning: Rule %s has illegal '$' specifier\n",rule.prodDebug.c_str());
					fprintf(stderr,"Code: %s\n",rule.code.c_str());
				}

				/*
				 *	Determine if we have one of the special fields, _file, _line,
				 *	_col. This allows us to extract the current token location of
				 *	a symbol
				 */

				if (*ptr == '_') {
					std::string param;
					++ptr;
					while (isalnum(*ptr)) {
						param.push_back(*ptr++);
					}

					if (value == 1) {
						ret += "(stack[pos].";
					} else {
						sprintf(buffer,"(stack[pos + %zu].",value-1);
						ret += buffer;
					}

					if (param == "file") {
						ret += "filename)";
					} else if (param == "line") {
						ret += "line)";
					} else if (param == "col") {
						ret += "column)";
					} else {
						ret += "state)";
						fprintf(stderr,"Warning: Unknown synthetic type $%zu_%s\n",value,param.c_str());
					}

				} else {
					// Insert (type *) if type is defined
					std::string valueType = rule.types[value-1];

					if (value == 1) {
						ret += "(stack[pos].value";
					} else {
						sprintf(buffer,"(stack[pos + %zu].value",value-1);
						ret += buffer;
					}

					if (valueType.size() > 0) {
						ret += " as! ";
						ret += valueType;
					} else {
						fprintf(stderr,"Warning: Rule %s, $%zu has no type\n",rule.prodDebug.c_str(),value);
					}

					ret.push_back(')');
				}

			} else {
				// ???
				ret.push_back(c);
			}
		} else {
			ret.push_back(c);
		}
	}

	/*
	 *	Print rule with 12 space prefix
	 */

	if (ret.length()) {
		fprintf(f,"                %s\n",ret.c_str());
	}
}

/*	OCYaccSwiftGenerator::WriteYTables
 *
 *		Write the files
 */

void OCYaccSwiftGenerator::WriteYTables(FILE *f)
{
	size_t i,len;
	char buffer[64];

	// Constants to print
	fprintf(f,"\t// Various constants\n");
	fprintf(f,"\tprivate static let K_ACCEPTSTATE: UInt16 = %-8zu    // Final accept state\n",state.accept);
	fprintf(f,"\tprivate static let K_EOFTOKEN: Int       = 0x%-8x   // EOF token ID\n",state.eofTokenID);
	fprintf(f,"\tprivate static let K_ERRORTOKEN: Int     = 0x%-8x   // Error token ID\n",state.errorTokenID);
	fprintf(f,"\tprivate static let K_FIRSTTOKEN: Int     = 0x%-8x   // Error token ID\n",state.firstTokenID);
	fprintf(f,"\tprivate static let K_MAXSYMBOL:Int       = 0x%-8x   // Max ID for all symbols\n",state.maxSymbolID);
	fprintf(f,"\tprivate static let K_STARTSTATE: UInt16  = 0        // Start state is always 0\n\n");

	// Non-Unicode tokens
	fprintf(f,"\t/*  TokenArray\n");
	fprintf(f,"\t *\n");
	fprintf(f,"\t *      Array of non-Unicode token values for error reporting\n");
	fprintf(f,"\t */\n\n");

	bool first = true;
	fprintf(f,"\tprivate static let TokenArray: [String] = [\n");
	std::vector<OCYaccLR1::TokenConstant>::const_iterator tokIter;
	for (tokIter = state.tokens.cbegin(); tokIter != state.tokens.cend(); ++tokIter) {
		if (first) {
			first = false;
			fprintf(f,"\t\t");
		} else {
			fprintf(f,",\n\t\t");
		}
		fprintf(f,"\"%s\"",tokIter->token.c_str());
	}
	fprintf(f,"\n\t]\n\n");

	// Reduction table
	len = state.reductions.size();
	fprintf(f,"\t/*  RuleLength\n\t *\n\t *      The number of tokens a reduce action removes from the stack\n\t */\n\n");
	fprintf(f,"\tprivate static let RuleLength: [UInt8] = [\n");
	for (i = 0; i < len; ++i) {
		if (i) {
			fprintf(f,", ");
		}
		if ((i % 8) == 0) {
			if (i) fprintf(f,"\n");
			fprintf(f,"\t\t");
		}
		fprintf(f,"%2zu",state.reductions[i].reduce);
	}
	fprintf(f,"\n\t]\n");

	len = state.reductions.size();
	fprintf(f,"\n\t/*  RuleProduction\n\t *\n\t *      The prodution ID we reduce to\n\t */\n\n");
	fprintf(f,"\tprivate static let RuleProduction: [UInt32] = [\n");
	for (i = 0; i < len; ++i) {
		if (i) {
			fprintf(f,", ");
		}
		if ((i % 8) == 0) {
			if (i) fprintf(f,"\n");
			fprintf(f,"\t\t");
		}
		sprintf(buffer,"0x%x",state.reductions[i].production);
		fprintf(f,"%8s",buffer);
	}
	fprintf(f,"\n\t]\n\n");

	// Action tables
	fprintf(f,"\t/*\n\t *  The following tables are compressed using CSR format.\n");
	fprintf(f,"\t *  See https://en.wikipedia.org/wiki/Sparse_matrix\n");
	fprintf(f,"\t */\n\n");

	fprintf(f,"\t/*  ActionI, J, A\n\t *\n\t *      Compressed action index table.\n\t */\n\n");
	len = state.actionI.size();
	fprintf(f,"\tprivate static let ActionI:[UInt32] = [\n");
	for (i = 0; i < len; ++i) {
		if (i) {
			fprintf(f,", ");
		}
		if ((i % 8) == 0) {
			if (i) fprintf(f,"\n");
			fprintf(f,"\t\t");
		}
		fprintf(f,"%6zu",state.actionI[i]);
	}
	fprintf(f,"\n\t]\n\n");

	len = state.actionJ.size();
	fprintf(f,"\tprivate static let ActionJ:[UInt32] = [\n");
	for (i = 0; i < len; ++i) {
		if (i) {
			fprintf(f,", ");
		}
		if ((i % 8) == 0) {
			if (i) fprintf(f,"\n");
			fprintf(f,"\t\t");
		}
		uint32_t token = state.actionJ[i];
		sprintf(buffer,"0x%x",token);
		fprintf(f,"%8s",buffer);
	}
	fprintf(f,"\n\t]\n\n");

	len = state.actionA.size();
	fprintf(f,"\t// Note: < 0 -> reduce (rule = -a-1), >= 0 -> shift (state).\n");
	fprintf(f,"\tprivate static let ActionA:[Int16] = [\n");
	for (i = 0; i < len; ++i) {
		if (i) {
			fprintf(f,", ");
		}
		if ((i % 8) == 0) {
			if (i) fprintf(f,"\n");
			fprintf(f,"\t\t");
		}
		int32_t val = (int32_t)(state.actionA[i].value);
		if (state.actionA[i].reduce) {
			val = -val-1;
		}

		fprintf(f,"%6d",val);
	}
	fprintf(f,"\n\t]\n\n");

	// Goto table
	fprintf(f,"\t/*  GotoI, J, A\n\t *\n\t *      Compressed goto table.\n\t */\n\n");
	len = state.actionI.size();
	fprintf(f,"\tprivate static let GotoI:[UInt32] = [\n");
	for (i = 0; i < len; ++i) {
		if (i) {
			fprintf(f,", ");
		}
		if ((i % 8) == 0) {
			if (i) fprintf(f,"\n");
			fprintf(f,"\t\t");
		}
		fprintf(f,"%6zu",state.gotoI[i]);
	}
	fprintf(f,"\n\t]\n\n");

	len = state.gotoJ.size();
	fprintf(f,"\tprivate static let GotoJ:[UInt32] = [\n");
	for (i = 0; i < len; ++i) {
		if (i) {
			fprintf(f,", ");
		}
		if ((i % 8) == 0) {
			if (i) fprintf(f,"\n");
			fprintf(f,"\t\t");
		}
		sprintf(buffer,"0x%x",state.gotoJ[i]);
		fprintf(f,"%8s",buffer);
	}
	fprintf(f,"\n\t]\n\n");

	len = state.gotoA.size();
	fprintf(f,"\tprivate static let GotoA:[Int16] = [\n");
	for (i = 0; i < len; ++i) {
		if (i) {
			fprintf(f,", ");
		}
		if ((i % 8) == 0) {
			if (i) fprintf(f,"\n");
			fprintf(f,"\t\t");
		}
		int32_t val = (int32_t)(state.gotoA[i]);

		fprintf(f,"%6d",val);
	}
	fprintf(f,"\n\t]\n\n");
}

/************************************************************************/
/*																		*/
/*	Write Files															*/
/*																		*/
/************************************************************************/

/*	OCYaccSwiftGenerator::WriteOCFile
 *
 *		Write the header
 */

void OCYaccSwiftGenerator::WriteOCFile(const char *classname, const char *outputName, FILE *f)
{
	OCTextReplace replace;

	replace.map["classname"] = classname;
	replace.map["outname"] = outputName;

	replace.map["%"] = parser.declCode;
	replace.map["header"] = parser.classHeader;
	replace.map["global"] = parser.classGlobal;
	replace.map["local"] = parser.classLocal;
	replace.map["init"] = parser.classInit;
	replace.map["finish"] = parser.classFinish;
	replace.map["code"] = parser.endCode;
	replace.map["errors"] = parser.classErrors;

	// Write opening
	replace.write(GParser1,f);

	// Write YACC constants table
	std::vector<OCYaccLR1::TokenConstant>::const_iterator tokIter;
	for (tokIter = state.tokens.cbegin(); tokIter != state.tokens.cend(); ++tokIter) {
		fprintf(f,"\tpublic static let %-45s = 0x%08x\n",tokIter->token.c_str(),tokIter->value);
//		if (!tokIter->used) {
//			fprintf(f,"   /* Unused */\n");
//		} else {
//			fprintf(f,"\n");
//		}
	}
	fprintf(f,"\n");

	// Write internal constants
	WriteYTables(f);

	// Write error to the rules section
	replace.write(GParser2,f);

	// Write rules section
	fprintf(f,"\n        // Production rules\n");
	size_t i,len = state.reductions.size();
	for (i = 0; i < len; ++i) {
		fprintf(f,"            // (%x) %s\n",state.reductions[i].production,state.reductions[i].prodDebug.c_str());
		if (state.reductions[i].code.length() > 0) {
			fprintf(f,"            case %zu:\n",i);
			WriteRule(f, state.reductions[i]);
		}
	}
	fprintf(f,"\n");

	// Write the rest
	replace.write(GParser3,f);
}
