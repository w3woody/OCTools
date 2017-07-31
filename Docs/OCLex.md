# OCLex

A sorta drop-in replacement for Lex which generates a text lexical parser in
Objective C.

## Introduction

So this project started when I had the problem of wanting to build a simplified
subset of the C language--but wanted to use Lex and YACC in 
[Xcode.](https://developer.apple.com/xcode/) 

Note that it kinda works. But I wanted to generate a re-entrant Objective C
class which could parse my language, and I wanted a way to parse multiple
languages in the same source kit. And that's where I hit a brick wall.

Rather than continue to bang away at the existing tools, I decided (like many
programmers) to roll my own tools. After all, why spend a week when I can spend
a month? :-)

Thus, OCLex was born.

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

## Usage

Usage: oclex [-h] [-o filename] [-c classname] inputfile

* -h  
    Prints this help file. This help file will also be printed if any illegal
    parameters are provided. 

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

    %header {
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

Note that all four sections are optional in the OCLex file, and may be
omitted. 

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
substring; there is no implicit rule to ignore illegal characters. If you wish
to emulate this behavior, you must provide the following as the last rule in
your rule set:

    .            { /* Ignore illegal characters */ }

Also note that code executed when a rule is matched **must be included in curly
braces.** Thus, the following, legal in Lex or Flex, is illegal in oclex:

    {DIGIT}+     return INTEGER;

Instead, this must be written:

    {DIGIT}+     { return INTEGER; }

(Note: when the code segment is written to the Objective C file, the curly
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

#### Code

The final code segment is optional and may be omitted. If provided, any code
in the code segment is written as part of the generated Objective C class, and
is inserted in the code **before** the lexical parser method. This allows those
methods to be invoked from within rule actions.

## An overview of the output class

The generated Objective C class includes a header file and a source file.
By default they (and the class name itself) is the same as the name of the
input source file (with the extension stripped).

The generated Objective C class header takes an input file (specified as a
class which inherits from the generated protocol) as the initializer of
the class, and provides a single method to parse the contents of the input
file "MyInput.l":

    @interface MyInput : NSObject
    
    - (instancetype)initWithStream:(id<OCFileInput>)file;
    
    @property (assign) NSInteger line;     // line of last read token
    @property (assign) NSInteger column;   // column of last read token
    @property (copy)   NSString *filename; // marked filename (if provided)
    @property (copy)   NSString *text;     // string of last read token
    @property (assign) NSString *abort;    // Set to abort string if problem
    
    - (NSInteger)lex;                      // Method to read next token
    
    // (Optional %global declarations go here)
    
    @end

The primary lexical analysis method is the method **lex**; subsequent calls to
the **lex** method will provide the next token in the input stream, or -1 if
the end of the file is reached.

The fields provided are:

* **line**: 
    The current line number, starting with 1, of the input stream.

* **column**:
    The current column number (or character inset) on the line, starting at 0.
    Note that no allowances for tabs is taken; tabs are treated by the column
    count as 1 character wide. Also, no allowances for UTF-8 is taken.

* **filename**:
    The current file name. This parameter is not used by the lexical parser,
    but can be used in a more complex scheme when a lexical parser may want
    to handle C-style #line declarations.

* **text**:
    The text of the last read token.

* **abort**:
    If the **lex** method returns -1 because of an error, the **abort** field
    will contain a text string explaining the error. On and end of file
    condition, **abort** will be nil.

### The OCFileInput protocol

Each generated header contains a protocol declaration, protected by #define
declarations (so the protocol is only defined once):

    @protocol OCFileInput <NSObject>
    - (int)readByte;
    @end

Any input file must correspond to this protocol. The *readByte* method reads
the next byte character from the input file. (Note that the generated lex
file does not parse Unicode files, but treats input files as 8-bit characters.
This implies if you are clever, you can create input rules which match
UTF-8 unicode streams, though with limitations.)

### The overall header file.

The overall header file that is written by oclex looks like the following:

    #import <Foundation/Foundation.h>
    // (Optional %header declarations go here)
    
    #ifndef OCFileInputProtocol
    #define OCFileInputProtocol
    
    @protocol OCFileInput <NSObject>
    - (int)readByte;
    @end
    
    #endif
    
    @interface MyInput : NSObject
    
    /*
     *	External interfaces
     */
    
    - (instancetype)initWithStream:(id<OCFileInput>)file;
    
    /*
     *	Current reader state
     */
    
    @property (assign) NSInteger line;		// line of last read token
    @property (assign) NSInteger column;	// column of last read token
    @property (copy)   NSString *filename;	// marked filename (if provided)
    @property (copy)   NSString *text;		// string of last read token
    @property (assign) NSString *abort;		// Set to abort string if problem
    
    - (NSInteger)lex;						// Method to read next token
    // (Optional %global declarations go here)
    @end


### The output source file.

The output class itself has the following structure:

    /*	MyInput.m
     *
     *		This file was automatically generated by OCLex, part of the OCTools
     *	suite available at:
     *
     *		https://github.com/w3woody/OCTools
     */

    #import "MyInput.h"
    
    // Optional %{...%} header declarations goes here
    
    /************************************************************************/
    /*                                                                      */
    /*  Lex Transition State Tables                                         */
    /*                                                                      */
    /************************************************************************/

    ... Lex state tables and declarations go here

The headers are added after the "MyInput.h" file is included. This is
important to keep in mind, as it implies if your generated lexical class

Following the lex state tables, the following private interface declaration
is written for your class:

    @interface MyInput ()
    {
        // Read position support
        NSInteger curLine;
        NSInteger curColumn;

        // Mark location support
        NSInteger markLine;
        NSInteger markColumn;
    
        // Mark buffer storage
        BOOL isMarked;                // yes if we have mark set
        unsigned char *markBuffer;    // mark buffer
        NSInteger markSize;           // bytes stored in buffer
        NSInteger markAlloc;          // capacity of buffer
    
        // Read cache
        unsigned char *readBuffer;    // read cache buffer
        NSInteger readPos;            // Read position
        NSInteger readSize;           // size of data in read buffer
        NSInteger readAlloc;          // Capacity of read cache
    
        // Text read buffer
        unsigned char *textBuffer;    // text cache for reading buffer
        NSInteger textMarkSize;
        NSInteger textSize;
        NSInteger textAlloc;
    }
    
    @property (strong) id<OCFileInput> file;

    // Optional local declarations go here
    
    @end

Notice that a number of internal state variables are declared. Behavior of
the parser is illegal if you attempt to replace or override any of these
internal variables. It is also not recommended that you manipulate or access
any of these internal state variables. The only exceptions are:

- **textBuffer**:
    This contains a buffer which is used to store the token as it is being
    built. 

If for some reason you wish to write a simple rule which returns the 
character read as a token, you can write the following rule:

    .               { return *textBuffer; }

Note that before the rule action is executed, the text buffer is turned into
a string in **self.text**, so you could also write this rule as:

    .               { return [self.text characterAtIndex:0]; }

Following this is the class code itself. You can read the generated comments
if you are interested in how the class works.

The class declaration itself has the following format:

    @implementation MyInput
    
    /*
     *  Instantiate parser.
     */
    
    - (instancetype)initWithStream:(id<OCFileInput>)file
    {
    ... initializer
    }
    
    /*
     *  Free internal storage
     */
    
    - (void)dealloc
    {
    ... free used memory
    }
    
    - (void)mark
    {
    ... internally used method
    }
    
    - (void)reset
    {
    ... internally used method
    }
    
    - (int)input
    {
    ... returns next character in input stream
    }
    
    // Optional code section is inserted here.
    
    - (NSInteger)lex
    {
    ... lexical analysis engine ...
            /*
             *  Convert text sequence into string
             */
    
            self.text = [[NSString alloc] initWithBytes:textBuffer length:textSize encoding:NSUTF8StringEncoding];
    
            /*
             *  Execute action
             */
    
            switch (action) {
                case 0:
                    ... action for rule 0 ... 
                    break;
                case 1:
                    ... action for rule 1 ... 
                    break;
                ...
                default:
                    break;
            }
        }
    }
    
    @end

Note there are several reserved method names: **mark**, **reset**, **input** 
and **lex**. The one method provided which you may be interested in is the 
**input** method; this returns the next byte in the input stream, or -1 if 
the end of the file has been reached.

This allows you to, for example, parse the input stream for closing a 
C-style comment in the following way:

    - (void)closeComment
    {
        int d = [self input];
        while (d != -1) {
            if (d == '*') {
                d = [self input];
                if (d == '/') return;
            } else {
                d = [self input];
            }
        }
    }
    
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
    x?       an optional x.
    x*       0,1,2, ... instances of x.
    x+       1,2,3, ... instances of x.
    x|y      an x or a y.
    (x)      an x.
    {xx}     the translation of xx from the definitions section.

## A trivial example

The following file gives a trivial example showing a minimum grammar. This
grammar will parse the integers in the input text file, and return the value
of those integers. Any other characters are ignored:

    %%
    [0-9]+   { return self.text.integerValue; }
    .        { /* Ignore all others */ }

Note because the declaration section is optional, we skip the declarations
by starting our file with a '%%'. And since the trailing code is optional,
we simply omit the following code section.

## A non-trivial example

The following grammar acts as a simple postfix calculator. The input is
scanned for floating point values and operators, and the intermediate value
is stored on a stack of numbers. The parser then returns the value of the
last operation on reaching a newline.

    D           [0-9]
    E           [Ee][+-]?{D}+
    
    %local {
    @property (strong) NSMutableArray<NSNumber *> *stack;
    }
    
    %%
    
    {D}*({D}|\.{D}*)({E})?  { [self pushNumber]; }
    [+\-*/]                 { [self doOperator]; }
    \n                      { return [self stackValue]; }
    .                       { /* Ignore everything else */ }
    
    %%
    
    - (void)pushNumber
    {
        if (self.stack == nil) self.stack = [[NSMutableArray alloc] init];
        [self.stack addObject:@( self.text.doubleValue )];
    }
    
    - (void)doOperator
    {
        if (self.stack.count < 2) {
            NSLog(@"Error in operation");
        } else {
            NSNumber *b = self.stack.lastObject;
            [self.stack removeLastObject];
            NSNumber *a = self.stack.lastObject;
            [self.stack removeLastObject];
    
            if ([self.text isEqualToString:@"+"]) {
                a = @( a.doubleValue + b.doubleValue );
            } else if ([self.text isEqualToString:@"-"]) {
                a = @( a.doubleValue - b.doubleValue );
            } else if ([self.text isEqualToString:@"*"]) {
                a = @( a.doubleValue * b.doubleValue );
            } else if ([self.text isEqualToString:@"/"]) {
                a = @( a.doubleValue / b.doubleValue );
            }
    
            [self.stack addObject:a];
        }
    }
    
    - (NSInteger)stackValue
    {
        if (self.stack.count != 1) {
            NSLog(@"Error in operation");
            return 0;
        }
    
        return self.stack[0].integerValue;
    }

The above example could be cleaned up and better error handling could be added
by creating a publicly available method to determine the nature of the error.

But it does show a non-trivial example using many of the features of the oclex
tool.
