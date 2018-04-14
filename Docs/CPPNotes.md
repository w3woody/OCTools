# C++ Notes

One of the principle goals of the OCTool set is to provide a way to build a Lex-like and Yacc-like tool which generates re-entrant classes that can be used on the iPhone or on the Mac.

When building a complete parser solution using both the Lex and Yacc tools, the flow of information is as follows:

                   -----------                     ------------
    input_file --> |  OCLex  |  - (token/value) -> |  OCYacc  |
                   -----------                     ------------

For the Objective-C version of the tools, the value object can be a generic reference to an Objective-C object. We do this in OCYacc: we define the %type parameter as the name of the Objective-C object passed in the value.

However, for C++, this doesn't work quite so well. Ideally we should be able to pass any sort of value, rather than simply passing a `void *` parameter around. To that end for the C++ generator we reintroduce the `%union` declaration.

Now in the traditional yacc and lex tools, a `%union` declaration allows you to declare the different types that can be passed up as a value, which is a global declaration.

However, for our purposes (reentrancy), this doesn't work so well. 

We thus introduce the '%lex' token, which allows us to define the lexer for which the '%union' declaration applies. We also introduce the '%union' declaration on the lexer side if we intend to only use the OCLex generated lexer class without an associated yacc tool.

----

In practice, this means the following when generating a re-entrant C++ lexer and yacc system.

1. If only using OCLex, if you wish to return a value, add a `%union` definition.
    * If the %union definition is not provided, the value field is not defined.
    * To populate the value you must include code in the lex rules.

2. If you only use OCYacc, then an abstract class `OCLexInput` class is generated as part of the header.
    * This is used to build your custom lexer class.
    * You may optionally provide a `%union` to define the value field in the lexer, which will be used by the yacc tool. The `%union` is also used to define the %type fields. Note the `%type` values are the field names in the union, not the class names of the type being passed (as they are in Objective-C). Also note there is no mechanism to dealloc values, so if they hold a pointer to an object, you must allocate the object from a memory pool that can be released later, or live with leaks.

3. If you are using OCLex and OCYacc together:
    * Define the `%union` in the yacc definition.
    * Include the header to the yacc class in the OCLex %{...%} header.
    * Define the lex class name in the yacc definition using the new `%lex` value. The `%lex` declaration takes two fields, one mandatory (the name of the lexer class), and one optional (a string with the name of the header file holding the lexer class). If the second parameter is not provided, the header file is assumed to be the lexer class name followed by a '.h'.
