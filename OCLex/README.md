# OCLex inner workings

This document describes the inner workings of the OCLex source kit. This gives
an overview of the algorithms used, hopefully in sufficient detail so that
someone else could implement a similar application.

I'm writing this so I can remember what I did.

## Parsing Lex input files.

When invoked, OCLex starts by reading in the contents of the input file. This
is handled by a hand-rolled recursive-descent parser. First, the input file
is tokenized using the code in the class **OCLexer**, and then the stream of
tokens is parsed and checked for errors using the **OCLexParser** class. The
methods are relatively self-explanatory.

## Creating the NFA from the input rules.

The [non-deterministic finite automaton](https://en.wikipedia.org/wiki/Nondeterministic_finite_automaton) which represents the regular expressions
of the various rules is constructed using a variation of [Thompson's construction](https://en.wikipedia.org/wiki/Thompson%27s_construction).

The primary starting point for this parsing operation is in the class **OCLexNFA**, in the method *Construct(const char *&regex)*.

## Creating the DFA from the NFA.

