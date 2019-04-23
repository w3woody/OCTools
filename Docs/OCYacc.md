# OCYacc Quick Reference

A sorta drop-in replacement for Yacc which generates a text lexical parser in
Objective C.

Full documentation (in HTML) may be found here: [Using OCTools](http://htmlpreview.github.io/?https://github.com/w3woody/OCTools/blob/master/Docs/UsingOCTools.html).

## Usage

Usage: ocyacc [-h] [-l [oc|cpp|swift]] [-o filename] [-c classname] inputfile

* -h  

    Prints this help file. This help file will also be printed if any illegal
    parameters are provided. 
    
* -l [oc|cpp|swift] 

    Choose language. Currently supports Objective-C by default. Can generate
    a re-entrant C++ file or a re-entrant Swift file instead.

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

**%type** *\<type\> symbols*

Defines the type of a given set of production rule names. This defines the type of the '$$' or '$n' symbols that represent the value of the rule.

##### Value Types

**%union** { ... }

For C++, defines the union used to define token and type values. Objective-C assumes all types are given as class declarations (descendant from `id<NSObject>`), and thus %union is ignored.

##### Precedence

**%left** *symbols* or **%left** *\<type\> symbols*

**%right** *symbols* or **%left** *\<type\> symbols*

**%nonassoc** *symbols* or **%left** *\<type\> symbols*

Sets the precedence and associativity of a group of tokens. Tokens declared on the same line have the same precedence, and are treated with left associativity, right associativity, or are made non-associative if they show up in the same statement. Later declarations have higher precedence than earlier declarations.

##### Start Symbol

**%start** symbol

The start symbol is the first rule in the grammar that defines your language. Unlike *Yacc* or *Bison,* this parameter is required.

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

Defines code that is inserted into the initializer of the generated class. This can be used to initialize any internal global or local state in your code.

**%finish** { ... }

Defines code that is inserted into the destructor/dealloc method of the generated class.

**%errors** { ... }

Defines code inserted right after the standard error definitions in the header of a generated class file. This can be used to define your own custom errors.

#### Rules

Production rules in OCYacc are declared similarly as in Yacc.

A production rule has the format:

    result : components... { code };

Where *result* is the production symbol this rule describes, and *components* are other tokens and productions that describe this rule. The *code* section is optional and gives the code that will be executed when the rule is reduced.

Each token or production rule may have a value. The value for the production rule is represented as **$$**, and the components of the rule have the values **$1..$n**, for the first through n'th component in the rule. Using a variable for a rule or token whose value is undefined is illegal.

**Note:** OCYacc does not permit actions to be embedded in the components; an action in the form of Objective C code must be given at the end of the components.

##### %prec *token*

Each list of components in a rule may optionally be followed by the **%prec** declaration with a token defined in the list of **%left**, **%right** or **%nonassoc** precedence rules above. This defines the precedence that should be taken when evaluating the given rule.


## Error Handling

You can obtain a list of errors as the parser discovers them by implementing the class protocol of the generated parser class and setting the field `errorDelegate` (Objective C), or by inheriting and overriding the `error` method (C++).

Examples of this can be found in the Examples folder.

### The `error` token

A rule may be written with the special `error` token. This token matches any syntax error that is discovered while parsing the specified rule, and allows you to handle the error yourself, and (hopefully) resolving the error so later code can also be parsed and further errors detected.

Without the `error` token, OCYacc makes a best-faith effort to resynchronize the parser to the input stream of tokens.

### Posting errors

Two methods are provided in C++ and Objective C for your rules to invoke error handling.

For Objective-C, the method signatures are:

    - (void)errorWithCode:(NSInteger)code;
    - (void)errorWithCode:(NSInteger)code data:(NSDictionary<NSString *, id<NSObject>> *)data

For C++, they are:

    void errorWithCode(int32_t code, std::map<std::string,std::string> &data);
    void errorWithCode(int32_t code);

Error constants can be defined in the `%errors` declaration above. Predefined symbols that can be used by your error declarations and for handling errors are:

Built-in Errors     | Definition
------------------- | -------------
ERROR_SYNTAX        | A general syntax error was detected
ERROR_MISSINGTOKEN  | An error was encountered in a rule that had only one possible token that should follow. (The missing token is returned in data[@"token"].)
ERROR_MISSINGTOKENS | An error was encountered in a rule that had a limited number of possible tokens that could follow. (The array of possible tokens is returned in data[@"tokens"].)

Other constants defined for your use are:

Constants           | Definition
------------------- | -------------
ERRORMASK_WARNING   | A bitmask constant that should be OR'ed into your own error code to indicate that this is a warning. Warnings do not cause the parser to return failure or cause the parser to prematurely terminate.
ERROR_STARTERRORID  | The first legal value you may use for your own errors. (You should eclare your errors `(ERROR_STARTERRORID+0)`, `(ERROR_STARTERRORID+1)`, etc.)


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

