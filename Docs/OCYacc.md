# OCYacc

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

Usage: ocyacc [-h] [-o filename] [-c classname] inputfile

* -h  
    Prints this help file. This help file will also be printed if any illegal
    parameters are provided. 

* -o  
    Uses the file name as the base name for the output files. (By default 
    this uses the input file name as the base name for the output files.) 

* -c  
    Uses the specified class name for the generated class. (By default this 
    uses the input file name as the class name.)

This program takes an input file MyFile.y, and will generate two output files
MyFile.m and MyFile.h, with the class MyFile which parses an input file, 
unless otherwise specified.

## An overview of the input yacc file.

The input lexical file is similar to that used by 
[Yacc or Bison](http://dinosaur.compilertools.net/bison/), but with
some very important differences.

First, because the output source file being generated is a re-entrant
Objective C file, the code snippets you add can be written in Objective C.
Further, an additional declaration section is provided so if you need to
create new variables for class state, you can do so by providing @protocol
declarations.

Further, local class variables and error processing methods have different
names than used by Yacc.


### The basic input file

Like Yacc, the OCYacc input file has the following format:

    Declarations
    %%
    Rules
    %%
    Code

#### Declarations

OCYacc accepts the following declarations in the declarations section:

##### Tokens and Types

**%token** *symbols* or **%token** *\<type\> symbols*

Declares a list of symbols as terminal tokens that will be returned by the lexer. The symbols are copied as #define statements in the generated header file, and can be used by the lexer as the integer value to return when those tokens are found.

The value of a token is set in the lexer in the value field; see the comments below for more information.

**%type** *\<type\> symbols*

Defines the type of a given set of production rule names. This defines the type of the '$$' or '$n' symbols that represent the value of the rule.

For both the token and type declarations, as well as the precedence symbols below, the *\<type\>* is an Objective C class name which gives the type of the value used in the rule or token.

Types are internally passed around in the generated Objective-C code as an *id\<NSObject\>* value. This is cast into a pointer to the class type that is given in the *type* field. Thus, for example, if you have a rule whose value is an array of strings, you would write:

    %type <NSMutableArray<NSString *>> rule

**Note that this implies there is no %union declaration.** As all types are interpreted as class declarations, this implies that during error handling excess memory will be automatically released. It also allows you to use certain built-in classes without additional declaration.

##### Precedence

**%left** *symbols* or **%left** *\<type\> symbols*

**%right** *symbols* or **%left** *\<type\> symbols*

**%nonassoc** *symbols* or **%left** *\<type\> symbols*

Sets the precedence and associativity of a group of tokens. Tokens declared on the same line have the same precedence, and are treated with left associativity, right associativity, or are made non-associative if they show up in the same statement. Later declarations have higher precedence than earlier declarations.

For example, if we set the associativity of basic math operators as:

    %left '+' '-'
    %left '*' '/'

The first two symbols (multiply and divide) are treated with higher precedence, so the following grammar rules will evaluate the statement 1 + 2 * 3 + 4 correctly:

    expression : expression '*' expression
               | expression '/' expression
               | expression '+' expression
               | expression '-' expression
               | NUMBER
               ;

The type is optional, and provides a way to specify the type of the symbol; see the discussion about tokens and types above.

##### Start Symbol

**%start** symbol

The start symbol is the first rule in the grammar that defines your language.

##### Embedded Code

**%{** ... **%}**

Defines the code which is copied to the header of the generated .m file. This would be the place to define #import declarations for your generated code.

**%header** { ... }

Defines the code which is copied to the header of the generated .h file. This would be the place to define #import declarations that are included when your class header is included.

**%global** { ... }

Defines the code that is added to the class declaration for the generated class in the .h header file. These allow you to extend the generated class to provide the interfaces specified. So, for example, if you want your parsing class to provide a new property, you could write:

    %global {
        @property (strong) NSArray<NSString *> *stuff;
    }

**%local** { ... }

Defines the code inserted into a private interface declared as part of the class declaration of your class in the generated .m file.

**%init** { ... }

Defines code that is inserted into the initializer of the generated lex class. This can be used to initialize any internal global or local state in your code.

###### Embedded Code Example

The following may help to visualize where each of these headers are placed. For Objective C, we generate a header and code. There are up to six different chunks of code which are copied into the class definition; these are labeled in red below.

Your embedded code is inserted as follows:

<pre>
 /* MyLex.h */
 #import &lt;Foundation/Foundation.h&gt;
 <font style="color:red">%header</font>
 
     <i>...</i>
 
 @interface MyLex : NSObject &lt;OCLexInput&gt;
     <i>...</i>
 - (NSInteger)lex;
 <font style="color:red">%global</font>
 @end
</pre>

For our source kit:

<pre>
 /* MyLex.h */
 #import &lt;Foundation/Foundation.h&gt;
 <font style="color:red">%{..%}</font>
 
     <i>...</i>
 
 @interface MyLex ()
     <i>... private definitions ...</i>
     <font style="color:red">%local</font>
 @end
 
 @implementation MyLex
 - (instancetype)initWithStream(id<OCLexInput>)file
 {
 	if (nil != (self = [super init])) {
 	    <i>...</i>
        <font style="color:red">%init</font>
 	}
 	return self;
 }
     <i>...internal lexer code...</i>
 
 <font style="color:red">code_block from bottom of lex file</font>

 - (NSInteger)lex
 {
 ...
 }
 @end
</pre>

#### Rules

Production rules in OCYacc are declared similarly as in Yacc.

A production rule has the format:

    result : components... ;

Where *result* is the production symbol this rule describes, and *components* are other tokens and productions that describe this rule.

Multiple rules may be written that give the same result by separating them with a vertical bar:

    result : rule1-components...
           | rule2-components...
           ;

Each rule may end with an optional *%prec \<token\>* which gives the precedence of the specific rule when deciding conflicts, as well as optional code which is executed when the rule is reduced.

Optional code is written in Objective C and is made part of the generated class. The code may contain variables of the form **$$** which defines the value of the production, or **$1**...**$n**, which represent the value of each component that makes the rule.

**Note:** OCYacc does not permit actions to be embedded in the components; an action in the form of Objective C code must be given at the end of the components.

##### %prec

The *%prec* declaration following a list of components gives the relative precedence of a rule and can be used to resolve the if/else problem.

In C, you can write rules for if/else as:

    if_statement : IF '(' expression ')' statement
                 | IF '(' expression ')' statement ELSE statement
                 ;

This, however, creates a shift/reduce conflict, as if the user were to write the statement:

    if (a) if (b) c else d

Does he mean:

    if (a)
        if (b) c
        else d

Or

    if (a)
        if (b) c
    else d

That is, does the 'else' tie to the first if? Or the second?

For C, where the else ties with the closest if (that is, the second if, making the first indention correct), this conflict can be resolved by using the *%prec* statement

    %nonassoc IFSTATEMENT
    %nonassoc ELSE
    %%
    if_statement : IF '(' expression ')' statement %prec IFSTATEMENT
	             | IF '(' expression ')' statement ELSE statement
	             ;

This associates the else statement with a higher precedence than the naked if statement. This will cause the reduction by the 'else' statement so that it is tied to the closest if statement.

#### Code

The code segment following the rules is optional. If provided, the code is inserted into the body of the Objective C class being generated in the .m file. 

## An overview of the output class

The generated Objective C class includes a header file and a source file.
By default they (and the class name itself) is the same as the name of the
input source file (with the extension stripped).

The generated Objective C class header takes an input lexer file which generates a stream of tokens. The lexer file is defined as a protocol that matches the lexer protocol generated by OCLex:

    @protocol OCLexInput <NSObject>
    - (NSInteger)line;
    - (NSInteger)column;
    - (NSString *)filename;
    - (NSString *)text;
    - (NSString *)abort;
    
    - (NSInteger)lex;
    
    - (id<NSObject>)value;
    @end

The generated lexer class then has a single delegate for reporting parser error messages, and a single entry point for parsing the input file. The overall parser declaration generated by OCYacc is:

    @interface MyClass : NSObject
    @property (weak) id<MyClassError> errorDelegate;
    
    - (instancetype)initWithLexer:(id<OCLexInput>)lexer;
    - (BOOL)parse;
    // Additional %global declarations here
    @end

The error delegate (whose name is based on the class name of the generated class) declares a single method, one called on each error reported by the parser:

    @protocol MyClassError <NSObject>
    - (void)errorFrom:(MyClass *)yacc 
                 line:(NSInteger)line 
               column:(NSInteger)column
		      filename:(NSString *)fname 
		  errorMessage:(NSString *)error;
    @end

The error message itself is given in the error parameter; the rest gives the location in the input lex stream of the token which caused the error.

The header file also has a list of constants which define the integer value of each token that should be returned by the Lexer; the header file can be included into the OCLex .l file to define the constants to return for each token.

This gives the overall header file as:

    /*	MyClass.h
     *
     *		This file was automatically generated by OCYacc, part of the OCTools
     *	suite available at:
     *
     *		https://github.com/w3woody/OCTools
     */
    
    #import <Foundation/Foundation.h>
    
    // %header declarations
    
    /*
     *	Class forwards
     */
    
    @class MyClass;
    
    /*
     *	Yacc constants
     */
    
    #define MUL_ASSIGN                                    0x00110002
    // Other constant definitions go here
    
    /*	OCLexInput
     *
     *		The protocol for our lex reader file that the lex stream must
     *	provide. This is the same as the protocol generated as part of the lex
     *	output, and allows us to glue the Lexer and Parser together.
     */
    
    #ifndef OCLexInputProtocol
    #define OCLexInputProtocol
    
    @protocol OCLexInput <NSObject>
    - (NSInteger)line;
    - (NSInteger)column;
    - (NSString *)filename;
    - (NSString *)text;
    - (NSString *)abort;
    
    - (NSInteger)lex;
    
    - (id<NSObject>)value;
    @end
    
    #endif
    
    /*	MyClassError
     *
     *		The protocol for our parser for handling errors. As errors take place,
     *	we invoke the method so the error can be recorded and displayed to the
     *	user.
     */
    
    @protocol MyClassError <NSObject>
    - (void)errorFrom:(MyClass *)yacc line:(NSInteger)line column:(NSInteger)column
    		filename:(NSString *)fname errorMessage:(NSString *)error;
    @end
    
    /*	MyClass
     *
     *		The generated parser
     */
    
    @interface MyClass : NSObject
    @property (weak) id<MyClassError> errorDelegate;
    
    - (instancetype)initWithLexer:(id<OCLexInput>)lexer;
    - (BOOL)parse;
    // %global declarations
    @end

### The Output Source File

The output source file has the following structure:

    /*  MyClass.m
     *
     *      This file was automatically generated by OCYacc, part of the OCTools
     *  suite available at:
     *
     *      https://github.com/w3woody/OCTools
     */
    
    #import "MyClass.h"

At this point the code declared in the %{ ... %} declaration section is inserted.
    
    /************************************************************************/
    /*                                                                      */
    /*  State Tables and Constants                                          */
    /*                                                                      */
    /************************************************************************/

State tables are inserted in here. Afterwards we declare the internal class
that is used to store the state stack and value stack.    
    
    /************************************************************************/
    /*                                                                      */
    /*  Internal Structures                                                 */
    /*                                                                      */
    /************************************************************************/
    
    /*
     *  Internal parser stack
     */
    
    @interface MyClassStack: NSObject
    @property (assign) uint16_t state;
    
    /* Represent the intermediate values for reduction rule values */
    @property (strong) id<NSObject> value;
    @end
    
    @implementation MyClassStack
    @end

The parser engine itself consists of some internally declared values, and the class itself.
    
    /************************************************************************/
    /*                                                                      */
    /*  Parser Code                                                         */
    /*                                                                      */
    /************************************************************************/
    
    /*
     *  Class internals
     */
    
    @interface MyClass ()
    @property (strong) NSMutableArray<MyClassStack *> *stack;
    @property (strong) id<OCLexInput> lex;
    
    // Error support
    @property (assign) NSInteger line;
    @property (assign) NSInteger column;
    @property (copy) NSString *filename;
    
    @property (assign) NSInteger errorCount;

At this point %local declarations are inserted. This is how you can define private class variables within the generated class that is used by your production rule code.
    
    @end
    
    /*
     *  Generated class
     */
    
    @implementation MyClass
    
We don't give the code but just the method names of the internally used methods below.

    - (instancetype)initWithLexer:(id<OCLexInput>)lexer;

Your custom code, following the last %% in the .y file, is inserted as methods after the init constructor is declared. This code is declared within the class and must be declared as internal methods.
    
    /*
     *  Process production rule. This processes the production rule and creates
     *  a new stack state with the rule reduction.
     */
    
    - (MyClassStack *)processReduction:(NSInteger)rule
    {
        NSInteger pos = self.stack.count - RuleLength[rule];
        MyClassStack *s = [[MyClassStack alloc] init];
    
        // Now process production.
        //
        // Note that $$ translated into (s.value), and
        // $n translates into ((<type> *)(self.stack[pos+(n-1)])), where <type>
        // is the declared type of the token or production rule.
    
        @try {
            switch (rule) {

The code associated with each rule is inserted as a switch statement here. 

            }
        }
        @catch (NSException *exception) {
            /* This can happen in the event we start seeing errors */
        }
    
        return s;
    }

Following the method to process productions are the following methods. The two significant methods you need to remember are -errorWithFormat:... and -errorOK.    
    
    - (NSInteger)actionForState:(NSInteger)state token:(NSInteger)token;
    - (NSInteger)gotoForState:(NSInteger)state production:(NSInteger)token;
    
    // Error reporting entry point
    - (void)errorWithFormat:(NSString *)format,...;
    - (void)errorOK;
    
    - (const NSString *)tokenToString:(uint32_t)token;
    - (BOOL)reduceByAction:(int16_t)action;

Following these internal methods is the parse engine, which is documented in better detail in the generated source kit.
    
    - (BOOL)parse
    {
        ...
    }
    
    @end

## Error Handling

There are two methods you can use for error handling in conjunction with the predefined **error** token.

Those methods are -errorWithFormat:... and -errorOK.

The first dispatches a callback through the error handling delegate with the provided error message, and errorOK resets the count which hides further error reporting until after 3 tokens are successfully read. 

For example, suppose we have a language where statements are separated by a semicolon. You can handle errors by writing:

    statement : stuff ';'
              | error ';' { [self errorWithFormat:@"Syntax error"]; }
              ;

If an error is seen while parsing *stuff*, the state stack is unwound and the **error** token will swallow tokens until we see a semicolon, and can resynchronize the token stream. The rule is called, an error is printed (with the location given as where the first offending token was found), and an internal counter is set: once we see 3 consecutive successfully read tokens, error reporting is permitted again.

If you wish to halt the error counting, you may invoke the errorOK method:

    statement : stuff ';'
              | error ';' 
                  { 
                      [self errorWithFormat:@"Syntax error"];
                      [self errorOK];
                  }
              ;

## Examples

Several examples have been checked into the source kit, and it is worth examining those.
