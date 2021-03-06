# OCTools

A suite of tools which serve as a plug-in replacement for yacc and lex. The goal
is to provide a (roughly) source compatible tool which can convert yacc and lex
grammars into Objective C, C++ or Swift for building parsers that run on MacOS and iOS.

The suite of tools include:

*   [OCLex,](Docs/OCLex.md) a lexical analysis tool for generating text lexical
    parsers in Objective C, C++ or Swift.
*   [OCYacc,](Docs/OCYacc.md) a parser generator which generates a parser in 
    Objective C, C++ or Swift.

Full documentation, including examples of using OCLex and OCYacc together in Objective C, C++ and Swift, can be found here: [Using OCTools](http://htmlpreview.github.io/?https://github.com/w3woody/OCTools/blob/master/Docs/UsingOCTools.html)

---

This source kit also contains a detailed document on how YACC-style LR parser-generators work, and hopefully at a sufficient degree of detail to allow a beginning Computer Science student to understand how it works. This document includes a description of LR(0), SLR and LALR grammars. 

[The paper can be downloaded from here.](OCYacc/Docs/OCYacc%20Building%20LR1%20GLR.pdf)

Unlike the rest of the source kit, this paper is freely available and reproducible for non-commercial purposes only, such as use in a classroom setting. For reproduction for the purpose of commercial gain (such as including in a larger book that is sold), please contact me for permission.

---

Each of these tools produce a re-entrant* class which can be used to
parse an input text file using a specification similar to the input files for 
[Lex and YACC.](http://dinosaur.compilertools.net)

(Re-entrant: instances of these classes can be used in a multi-threaded
environment to simultaneously parse multiple files. The individual classes are 
not thread safe, meaning one instance of a class cannot be accessed by multiple
threads at the same time.)

## Using with Xcode

To use the OCTools in Xcode, you need to install the files on your computer and update the build rules of your Xcode project to use them.

### Step 1: Install the tools.

Download the [installer](https://github.com/w3woody/OCTools/raw/master/octools.pkg) from GitHub. This file is a MacOS X file which will install two files to /usr/local/bin: the `ocyacc` command-line tool and the `oclex` command line tool.

**NOTE:** to uninstall, you can simply use the MacOS Terminal to delete these two tools: with the terminal open type `sudo rm /usr/local/bin/oclex` and `sudo rm /usr/local/bin/ocyacc`. The installer does not install any other files.

### Step 2: Update your $PATH settings. (Optional)

This step is strictly not necessary but comes in handy if you wish to run the oclex/ocyacc tool from the command line.

Open Terminal. Make sure your home directory is the current directory. (If not, enter the command `cd ~`.) Determine if the file .bash_profile exists; if not, create the file using the command `touch .bash_profile`. Edit the file (`open .bash_profile`).

In the file, add the following line:

    export PATH=/usr/local/bin:$PATH

Save and close the file. The next terminal window that is opened will have the $PATH variable set appropriately. You can test this by opening a new terminal window and entering the command `ocyacc` or `oclex`; this will print version and copyright information for each of the tools.

### Step 3: Include support into your project.

For Xcode to use the tools you must update the *Build Rules* setting under the target for the project you are building.

First, open the Xcode project, and select the project file in the Project navigator on the far left. This will show the project settings.

Second, select the target which you wish to modify. You will need to change the build rules for each target in your project.

Third, select "Build Rules" from the tab bar along the top.

Fourth, select the "Add" button, the plus sign in the upper-left of the build rules page. This will add a new build rule at the top. 

For **OCLex** and **OCYacc**, the build rule uses the built-in lex and yacc source files. The settings should be as follows: for OCLex--

    Process      Lex source files
    Using        Custom script:
    
        /usr/local/bin/oclex -o "$DERIVED_SOURCES_DIR/$INPUT_FILE_BASE" "$INPUT_FILE_PATH"
    
    Output Files:
        $DERIVED_SOURCES_DIR/$(INPUT_FILE_BASE).m
        $DERIVED_SOURCES_DIR/$(INPUT_FILE_BASE).h

For OCYacc:

    Process      Yacc source files
    Using        Custom script:
    
        /usr/local/bin/ocyacc -o "$DERIVED_SOURCES_DIR/$INPUT_FILE_BASE" "$INPUT_FILE_PATH"
    
    Output Files:
        $DERIVED_SOURCES_DIR/$(INPUT_FILE_BASE).m
        $DERIVED_SOURCES_DIR/$(INPUT_FILE_BASE).h

After this is done, yacc and lex files will be compiled to .h and .m files of the same name.

**NOTE:** Because the output .h and .m files are derived from the input file of the .y and .l files, they should not have the same name. For example, if creating a language using a lexer and a yacc file, you should name them something like:

    MyLangLex.l
    MyLangParser.y

This would create two Objective-C classes with names MyLangLex and MyLangParser. (If you give them the same name, you'll create two classes with the same name, and this is the path to ruin. Or at least confusion.)

**NOTE:** If you wish to generate C++, you would add the `-l cpp` argument to the command-line, and change the output to `.cpp` from `.m`. Thus, for OCYacc, you'd enter:

    Process      Yacc source files
    Using        Custom script:
    
        /usr/local/bin/ocyacc -l cpp -o "$DERIVED_SOURCES_DIR/$INPUT_FILE_BASE" "$INPUT_FILE_PATH"
    
    Output Files:
        $DERIVED_SOURCES_DIR/$(INPUT_FILE_BASE).cpp
        $DERIVED_SOURCES_DIR/$(INPUT_FILE_BASE).h

For Swift, you'd add the `-l swift` declaration, and change the output files to `$(INPUT_FILE_BASE).swift`.

## New Features

### March-April 2019

* Added MacOS installer and code to build an installer.
  * To build the installer yourself, download the source kit, open the OCTools workspace, and build the "OCTools" aggregate target. This will run the scripts necessary to build the installer package.
* Started complete reworking of source kit and documentation set to make this more useful. Clean up some minor issues with C++ code generation. Started adding code examples.
* Added new `%errors` declaration to OCYacc and improved error handling and error reporting. Fixed several error processing bugs.
* Added support for generating Swift.

### April 15, 2018

* Added %finish declaration token, for inserting code into the generated class's destructor/dealloc method.
* Added C++ generator.

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
