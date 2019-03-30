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

This source kit also contains a detailed document on how YACC-style LR parser-generators work, and hopefully at a sufficient degree of detail to allow a beginning Computer Science student to understand how it works. This document includes a description of LR(0), SLR and LALR grammars. 

[The paper can be downloaded from here.](OCYacc/OCYacc%20Building%20LR1%20GLR.pdf)

Unlike the rest of the source kit, this paper is freely available and reproducible for non-commercial purposes only, such as use in a classroom setting. For reproduction for the purpose of commercial gain (such as including in a larger book that is sold), please contact me for permission.

---

_Update March 29, 2019:_ I'm currently working on an [updated document](http://htmlpreview.github.io/?https://github.com/w3woody/OCTools/blob/master/Docs/UsingOCTools.html) which gives more up-to-date instructions for downloading, installing and using OCTools. This is a work in progress, but it is more up to date than the documentation that follows.

Over the next few weeks all the documentation will be revised, and hopefully will make it easier to integrate and use OCTools.

---

Each of these tools produce a re-entrant* Objective C class which can be used to
parse an input text file using a specification similar to the input files for 
[Lex and YACC.](http://dinosaur.compilertools.net)

(Re-entrant: instances of these classes can be used in a multi-threaded
environment to simultaneously parse multiple files. The individual classes are 
not thread safe, meaning one instance of a class cannot be accessed by multiple
threads at the same time.)

## Using with Xcode

To build and incorporate the OCTools into Xcode, you need to build the OCYacc tool and the OCLex tool as command-line tools, copy into a known location (such as `/usr/local/bin`), add the path to your shell $PATH setting (if needed), and modify the settings for each Xcode project where you use the tools.

### Step 1: Build the tools

This can be done by opening the OCTools workspace, selecting "oclex" and running "Archive", then selecting "ocyacc" and running Archive. In both cases this will produce an archive with the individual tools oclex and ocyacc as command-line tools. 

(After each archive process is complete, the "Archives" window will open, showing the archive of the tool. Right-click and select "Show In Finder" to show the *xcarchive* bundle. Right-click on the bundle and select "Show Package Contents." The command line tool will be in the directory Products/usr/local/bin.)

### Step 2: Copy to a known path location.

I usually copy the tools to `/usr/local/bin`; you may choose somewhere else if you wish.

Open Terminal. Create `/usr/local/bin` if necessary; this can be done by executing commands like:

    cd /usr
    sudo mkdir local
    cd local
    sudo mkdir bin

(After each sudo command you may need to enter your password.)

Now with the current path of the Terminal at `/usr/local/bin`, execute:

    open .

This will open a finder window at the location `/usr/local/bin`. You can then drag and drop the tool located in the *xcarchive* bundle using Finder. (Finder may ask for a password before copying the tool into place.)

### Step 3: Update your $PATH settings. (Optional)

This step is strictly not necessary but comes in handy if you wish to run the oclex/ocyacc tool from the command line.

Open Terminal. Make sure your home directory is the current directory. (If not, enter the command `cd ~`.) Determine if the file .bash_profile exists; if not, create the file using the command `touch .bash_profile`. Edit the file (`open .bash_profile`).

In the file, add the following line:

    export PATH=/usr/local/bin:$PATH

Save and close the file. The next terminal window that is opened will have the $PATH variable set appropriately. You can test this by opening a new terminal window and entering the command `ocyacc` or `oclex`; this will print version and copyright information for each of the tools.

### Step 4: Include support into your project.

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
        $(INPUT_FILE_BASE).m
        $(INPUT_FILE_BASE).h

For OCYacc:

    Process      Yacc source files
    Using        Custom script:
    
        /usr/local/bin/ocyacc -o "$DERIVED_SOURCES_DIR/$INPUT_FILE_BASE" "$INPUT_FILE_PATH"
    
    Output Files:
        $(INPUT_FILE_BASE).m
        $(INPUT_FILE_BASE).h

After this is done, yacc and lex files will be compiled to .h and .m files of the same name.

**NOTE:** Because the output .h and .m files are derived from the input file of the .y and .l files, they should not have the same name. For example, if creating a language using a lexer and a yacc file, you should name them something like:

    MyLangLex.l
    MyLangParser.y

This would create two Objective-C classes with names MyLangLex and MyLangParser. (If you give them the same name, you'll create two classes with the same name, and this is the path to ruin. Or at least confusion.)

## New Features

### April 15, 2018

* Added %finish declaration token, for inserting code into the generated class's destructor/dealloc method.
* Added C++ generator. This is an untested experimental feature and is subject to change. [See the notes here for more information.](Docs/CPPNotes.md)

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
