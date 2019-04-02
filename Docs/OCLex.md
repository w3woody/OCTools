# OCLex

A sorta drop-in replacement for Lex which generates a text lexical parser in
Objective C.

Full documentation (in HTML) may be found here: [Using OCTools](http://htmlpreview.github.io/?https://github.com/w3woody/OCTools/blob/master/Docs/UsingOCTools.html).


## Usage

Usage: oclex [-h] [-l [oc|cpp]] [-o filename] [-c classname] inputfile

* -h  

    Prints this help file. This help file will also be printed if any illegal
    parameters are provided. 
    
* -l [oc|cpp] 

    Choose language. Currently supports Objective-C by default. Can generate
    a re-entrant C++ file instead. (This feature is currently experimental.)

* -o  

    Uses the file name as the base name for the output files. (By default 
    this uses the input file name as the base name for the output files.) 

* -c  

    Uses the specified class name for the generated class. (By default this 
    uses the input file name as the class name.)

This program takes an input file MyFile.l, and will generate two output files
MyFile.m and MyFile.h, with the class MyFile which performs lexical analysis on
an input file, unless otherwise specified.

## An overview of the input lex file.

The input lexical file is similar to that used by 
[Lex or Flex](http://dinosaur.compilertools.net/flex/manpage.html), but with
some very important differences.

First, because the output source file being generated is a re-entrant
Objective C file, the code snippets you add can be written in Objective C.
Further, an additional declaration section is provided so if you need to
create new variables for class state, you can do so by providing @protocol
declarations.

Further, local class variables have different names than the ones used by Lex.

### The basic input file

Like Lex, the OCLex input file has the following format:

    Declarations
    %%
    Rules
    %%
    Code

#### Declarations

OCLex accepts five different types of declarations.

First are simple **name definitions** to simplify the scanner specification. 
A name definition has the following format:

    name pattern

For example:

    DIGIT    [0-9]
    LETTER   [A-Za-z]
    IDENT    [A-Za-z_][A-Za-z0-9_]*

This defines three different name definitions: a digit (which can be any digit
from 0 to 9), a letter (which can be any upper- or lower-case letter from
'a' to 'z'), and IDENT, which can be any acceptable C-style lexical token.

Subsequent usage of definitions in the rules section look like {name}. So,
for example, an integer rule may look like:

    {DIGIT}+          { return INTEGER; }

Second are header declarations. Any text contained between %{ and %} are 
included as part of the header declarations for our class. Thus, if your
class requires the "MySymbols.h" header, you can write the following as part
of your declaration:

    %{
    #include "MySymbols.h"
    %}

This can be used to include any predefined symbols that may be generated as
part of the [OCYacc](OCYacc.ml) code generator.

Third are header declarations. Any code in this section is included as part
of the class header. This allows you to specify class forwards or include
header files in your generated parser's header file. Any text inside the 
curly braces of the declaration %header { ... } is written as part of the 
class declaration in the header file.

For example, if you wish to include a file and declare a class that is used 
by your parser class file, you can write:

    %header {
        #include "MyOtherClass.h"
        
        @class CustomClass;
    }

Fourth are global class declarations. These are written as part of the class
declaration in your header file, and become part of the declared class. This
allows you to add methods or properties to your parser class. Any code inside
the curly braces of the declaration %global {...} is written as part of the
class declaration in the header file.

For example, if you wish to include a new method in your class declaration,
you can write:

    %global {
        - (NSInteger)intValue;
    }

Fifth are private class header declarations. These are written as part of
a private class declaration inside the .m file. This allows you to provide
internal method and properties as part of the class declaration. Any text
inside the curly braces of the declaration %local {...} is written as part
of the class declaration in the source file.

For example, if you wish to create an internal integer property you can
write:

    %local {
        @property (assign) NSInteger intProperty;
    }

Sixth is initializer code that can be inserted into the initializer of your class. This is code that can be used to set up any internal state. For example, if you need to set your intProperty to 1, you can write: 

    %init {
        intProperty = 1;
    }

Seventh is code run when the class is torn down. Normally in Objective-C you probably don't need to provide this, but sometimes you may wish to if, for example, you need to insert some shutdown code to release internal state (such as unregistering your class from receiving notifications), you can write: 

    %finish {
        [[NSNotificationCenter defaultCenter] removeObserver:self];
    }

This would insert the line of code in the %finish bracket into the Objective-C's `dealloc` method.

Note that all seven sections are optional in the OCLex file, and may be
omitted. 

##### Optional Rule Support

Rules may be optionally triggered by a start flag. You declare start flags by using the %start declaration:

    %start TOKEN TOKEN ...

#### Rules

The rules section contains one or more rules of the form:

    pattern  { code }

Like lex or flex, the code inside the curly braces following a pattern is
executed as soon as the pattern is successfully matched. Generally on matching
a pattern you can either return an integer value representing the matched
token, or you can execute some internal code.

Patterns are then matched until we reach the end of the file.

When the generated scanner is run, it scans the input looking for the longest
substring which match the provided patterns. If multiple patterns match, the
first rule in the list of rules is used.

Note that because this scans for the longest input pattern, certain rules may
not behave as expected. For example, the following rule to eliminate C
comments will not behave as expected:

    "/*".*"*/"     { /* Skip comment */ }

This is because the rule will continue to scan until it finds the very last
close comment in the file. That is, the entire example below will be ignored
by your rule:

    /* Hello.c */
    
    #include <stdio.h>
    int main()
    {
        printf("Hello world.\n"):
        return 0;
    }
    /* Done. */

That's because the scanner will find the longest substring which matches the
rule--and the longest substring matches all the way from the very first "/*"
of the first comment all the way down to the last "*/" of the "Done" comment.

**Unlike Lex or Flex**, you must provide rules for matching all possible
substrings; there is no implicit rule to ignore illegal characters. If you wish
to emulate this behavior, you must provide the following as the last rule in
your rule set:

    .            { /* Ignore illegal characters */ }

Also note that code executed when a rule is matched **must be included in curly
braces.** Thus, the following, legal in Lex or Flex, is illegal in oclex:

    {DIGIT}+     return INTEGER;

Instead, this must be written:

    {DIGIT}+     { return INTEGER; }

When the code segment is written to the Objective C file, the curly
braces are stripped. So if you need to declare a local variable in your action
code segment, you will need to write:

    {DIGIT}+     { 
                     { 
                         int tmp;
                         ... do something with tmp;
                         return INTEGER;
                     }
                 }

Note that actions may span multiple lines.

##### Optional rules

A rule may be made optional by adding the \<START\> prefix, with START as one of the states previously declared in the %start declaration section. If <START> is added, then the rule is only executed if the START state is true. 

A rule state may be activated by adding the code `BEGIN START;`--this is translated to code which sets the state associated with *START*. If the statement `BEGIN 0;` is found, all states are cleared. The code `END START;` is used to clear the specific state associated with *START*.

##### Token Values associated with Rules

OCLex uses a slightly different mechanism for passing values to OCYacc than
used by Lex and Yacc. To review, Yacc uses the %union directive to specify
a union structure in C to represent the potential values of symbols and tokens.
This structure is exposed to Lex via the **yylval** global, which is a union
object declared with the contents of the %union declaration.

Thus, if you wish to pass back an integer value associated with a token (such
as returning the integer value of a string of digits), you can declare in Yacc
a union like:

    %union {
        int tokenValue;
    }
    %token <tokenValue> INTEGER

And in your Lex production rule, write:

    [0-9]+    { yylval.tokenValue = atoi(yytext); return INTEGER; }

Objective-C does not provide unions, so OCLex and OCYacc uses a different
mechanism. Each token in OCYacc may have an associated type; the type represents
a class that stores the value. No %union{...} declaration is required.

Thus, in OCYacc, you'd replace the above with:

    %token <NSNumber> INTEGER

This indicates our integer value is returned in an NSNumber object. You would
then set the value **self.value** to the value in your Lex production rule:

    [0-9]+    { self.value = @( self.text.intValue ); return INTEGER; }

See the [OCYacc](OCYacc.ml) documentation for more information.

#### Code

The final code segment is optional and may be omitted. If provided, any code
in the code segment is written as part of the generated Objective C class, and
is inserted in the code **before** the lexical parser method. This allows those
methods to be invoked from within rule actions.


## Regular expressions

The oclex file handles the following regular expression types. Note this is
only a subset of what is processed by lex or flex. In the future this list may
be expanded.

    x        the character "x"
    "x"      an "x", even if x is an operator.
    \x       an "x", even if x is an operator.
    [xy]     the character x or y.
    [x-z]    the characters x, y or z.
    [^x]     any character but x.
    .        any character but newline.
    ^        (if used at the start) matches the start of the line
    $        (if used at the end) matched the end of the line
    x?       an optional x.
    x*       0,1,2, ... instances of x.
    x+       1,2,3, ... instances of x.
    x|y      an x or a y.
    (x)      an x.
    {xx}     the translation of xx from the definitions section.
    <y>x     rule x only if OCLex is in start condition y. (This must be at 
             the start of a regular expression.)


## License

Licensed under the open-source BSD license:

Copyright 2017-2019 William Woody and Glenview Software

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
