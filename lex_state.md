# Branch read-me

There is a fundamental bug in the way OCLex handles states. The purpose of this branch is to fix the bug by pushing state information into the NFA state machine, so that initial state information can be pushed into the final DFA. (My theory is that by adding state to the NFA, state will permeate into the DFA, allowing the correct subbranch of states to be selected for those rules with a conditional, such as an initial '^' tag or an explicit state declaration.)
