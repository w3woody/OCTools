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

Unlike many other implementations, for the most part regular expressions are parsed by examining the characters in the regular expression from left to right, recursing downwards only when seeing a (...) pair.

The final NFA is constructed as a set of states S, with a single start state S<sub>0</sub>, and end state S<sub>e</sub>, and a set of transitions f(S<sub>i</sub>,T<sub>s</sub>) -> S<sub>j</sub>. T is the set of symbols in our alphabet (generally ASCII), along with the empty (ε) transitions.

## Creating the DFA from the NFA.

In the traditional Regular Expression algorithm, a single NFA is reduced to a [Deterministic finite automaton](https://en.wikipedia.org/wiki/Deterministic_finite_automaton) using an algorithm like the [Powerset Construction algorithm](https://en.wikipedia.org/wiki/Powerset_construction). 

This algorithm works by representing each state in the DFA as a non-zero set of states in the NFA. A transition from a DFA state to another DFA state is computed by finding the set of all NFA states a transition passes to, and constructing a new DFA state with the set of represented NFA states.

In the end this will give us a DFA as a set of states S', with each S'<sub>i</sub> a subset of the NFA states S, and a set of transitions f(S'<sub>i</sub>,T)->S'<sub>j</sub>.

So, given an NFA, we construct the DFA by doing the following:

> Given NFA states S, and transitions f defined above
>
> * Set Q = {}, an empty queue of states to process
> * Set S' = {} and f' as an empty set of DFA states and an empty set of transitions.
> 
> The algorithm:
>
> 1. Calculate S'<sub>0</sub> = e-closure({S<sub>0</sub>})
> 2. Add S'<sub>0</sub> onto queue Q.
> 3. If Q is empty, return DFA = S', f'
> 4. Remove an item S'<sub>i</sub> off queue Q.
> 5. For every symbol T<sub>j</sub> in T, calculate S'<sub>j</sub> = buildState(S'<sub>i</sub>,T<sub>j</sub>). If S'<sub>j</sub> is not the empty set, add transition (S'<sub>i</sub>,T'<sub>j</sub>) -> S'<sub>j</sub> to f'.
> 7. If S'<sub>j</sub> is not in S', add S'<sub>j</sub> to S' and to Q.
> 8. Go to 4.

The algorithm basically loops through all the constructed states S'<sub>i</sub> and constructs new states S'<sub>j</sub> which can be reached via a transition T<sub>j</sub>.

(This can be optimized by only looping through the known transitions T<sub>j</sub> we know transition out of the NFA states inside S'<sub>i</sub>. This can be further optimized if we know multiple transitions pass to the same state by grouping those transitions together.)

This is done through the *buildState* algorithm:

> Given a DFA state S'<sub>i</sub> and a transition symbol T<sub>j</sub>, as well as the NFA states S and transitions f:
>
> * Set S'<sub>j</sub> = {}, an empty set.
> 
> The algorithm:
>
> 1. For every S<sub>i</sub> in S'<sub>i</sub> (remember: S'<sub>i</sub> is a unique set of states S<sub>i</sub> in the NFA), find the state S<sub>j</sub> = f(S<sub>i</sub>,T<sub>j</sub>), and add to S'<sub>j</sub>.
> 2. Return the set e-closure(S'<sub>j</sub>).

Note each set is constructed as to contain no duplicates, so if during construction a duplicate symbol is returned by f, it is not added to the set.

The ε-closure represents the total list of all states, including the states that can be reached via an empty transition from the existing set of states. This can be constructed as follows:

> Given a DFA state S'<sub>i</sub> and the NFA states S and transitions f:
>
> * Set Q = the set of states S'.
>
> The algorithm:
>
> 1. If Q is empty, return S'<sub>i</sub>.
> 2. Remove S<sub>i</sub> from Q
> 3. If a new state S<sub>j</sub> exists for e-transition f(S<sub>i</sub>,e), add S<sub>j</sub> to S'<sub>i</sub>.
> 4. Go to 1.

However, for the OCLex algorithm, we add two twists to the traditional tale.

**First,** we are reducing multiple NFAs representing multiple regular expressions at the same time. Our intent is to read a stream of symbols until we reach the maximum length symbol which matches one of our regular expressions, then execute the rule associated with that expression. 

So when we construct the initial state S'<sub>0</sub> as the ε-closure of all the start NFA states for all the regular expressions we've parsed.

**Second,** as we scan, we will eventually reach an NFA state inside one of our final DFA states which represents the completion of a rule.

When this happens, we note the DFA state which contains an NFA final state, and the rule associated with that DFA state during construction. If multiple NFA final states are associated with a particular DFA state, we favor the earlier regular expression rule associated with the NFA state.

(That is, if with DFA state 13 which contains NFA states 12, 14, 15 and 18, if states 12 and 15 are final rules associated with rule set 1 and 2, we associated DFA state 13 with rule set 1.)

When, during parsing, we reach a DFA state with an associated rule, we then mark the input stream and record the associated rule. The [mark](https://docs.oracle.com/javase/7/docs/api/java/io/InputStream.html#mark\(int\)) semantics we use are similar to that in Java: we note where in the input stream we are at, so a subsequent call to reset winds the input stream back to the mark location.

We continue reading until we hit an error, and when we do, we perform a reset operation, and execute the rule associated with the last mark and last rule recorded.

You can see an example of scanning for this rule in the method *FindEndRule* in the class **OCLexDFA**.

## Handling '\^' and '$' for start and end of line matching

In order to handle an end of line match ('$'), we add an additional flag to each rule indicating if the rule matches only if we are at the end of the line. This flag is stored as part of the associated rule.

The start of line matching is more interesting. Because we need to determine if the rules should be reduced before reading any tokens, we actually generate two state machines using the algorithms above; one which includes rules that start with the start of line matching token, and one which does not. We then select the appropriate state machine depending if we are at the start of the line or not.

## Generating the output files

There are a number of optimizations that are performed during processing. First, instead of building a transition table with 256 columns, the Lex parser determines groups of common characters which are shared in certain transitions. For example, if your regular expressions treat the characters '0' through '9' in the same way, the lex scanner creates a new character class.

Characters during parsing are then converted to their respective character class. This achieves a significant size reduction of the transition tables.

The other tables that are generated include the transition tables themselves (which represent the f' function transitioning from DFA states as integers to new DFA states), and the action rules associated with each DFA state, if any.

We also use an optimization to reduce the size of the final transition table f'. By compressing the transition table using a [Compressed Sparse Row](https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_.28CSR.2C_CRS_or_Yale_format.29) format, we trade an O(log(N)) lookup on the symbol class with a significant reduction in the resulting matrix size. See the generated code for more information.