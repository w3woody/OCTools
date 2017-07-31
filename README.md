# OCTools

A suite of tools which serve as a plug-in replacement for yacc and lex. The goal
is to provide a (roughly) source compatible tool which can convert yacc and lex
grammars into Objective C output for building parsers that run on MacOS and iOS.

The suite of tools include:

*   [OCLex,](Docs/OCLex.md) a lexical analysis tool for generating text lexical
    parsers in Objective C.
*   [OCYacc,](Docs/OCYacc.md) a parser generator which generates a parser in 
    Objective C.

---

Each of these tools produce a re-entrant* Objective C class which can be used to
parse an input text file using a specification similar to the input files for 
[Lex and YACC.](http://dinosaur.compilertools.net)

(Re-entrant: instances of these classes can be used in a multi-threaded
environment to simultaneously parse multiple files. The individual classes are 
not thread safe, meaning one instance of a class cannot be accessed by multiple
threads at the same time.)

## Using with Xcode

(TODO: Insert instructions on how to build and incorporate into Xcode.)

## License

Licensed under the open-source BSD license:

Copyright 2017 William Woody and Glenview Software

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
