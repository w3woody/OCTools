//
//  main.cpp
//  ocyacc
//
//  Created by William Woody on 7/24/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "OCYaccParser.h"
#include "OCYaccLR1.h"
#include "OCYaccGenerator.h"
#include "OCYaccCPPGenerator.h"
#include "OCYaccSwiftGenerator.h"

static const char *GHelp =
	"ocyacc\n"                                                                \
	"\n"                                                                      \
	"    A parser generator for generating code in Objective-C to perform fast\n" \
	"pattern matching on text. This tool takes an input grammar which is \n"  \
	"similar to yacc or bison, but generates a re-entrant Objective-C class.\n" \
	"\n"                                                                      \
	"Usage: ocyacc [-h] [-l [oc|c++]] [-o filename] [-c classname] inputfile\n" \
	"\n"                                                                      \
	"-h  Prints this help file. This help file will also be printed if any illegal\n" \
	"    parameters are provided. \n"                                         \
	"\n"                                                                      \
	"-l  Select language. Arguments are oc for Objective-C and c++ for C++. If\n" \
	"    not provided, uses Objective-C as default.\n"                        \
	"\n"                                                                      \
	"-o  Uses the file name as the base name for the output files. (By default this \n" \
	"    uses the input file name as the base name for the output files.) \n" \
	"\n"                                                                      \
	"-c  Uses the specified class name for the generated class. (By default this \n" \
	"    uses the input file name as the class name.)\n"                      \
	"\n"                                                                      \
	"This program takes an input file MyFile.y, and will generate two output files\n" \
	"MyFile.m and MyFile.h, with the class MyFile which parses an input file, unless\n" \
	"otherwise specified.\n"                                                  \
	"\n"                                                                      \
	"For more details please visit http://github.com/w3woody/OCTools\n"       \
	"\n"                                                                      \
	"Copyright (C) 2017-2019 William Woody and Glenview Software, all rights \n" \
	"reserved.\n";

/************************************************************************/
/*																		*/
/*	Declarations														*/
/*																		*/
/************************************************************************/

typedef enum LanguageEnum
{
	KLanguageOP,
	KLanguageCPP,
	KLanguageSwift
} LanguageEnum;


/************************************************************************/
/*																		*/
/*	Parser Definitions													*/
/*																		*/
/************************************************************************/

static char GOutputFile[FILENAME_MAX];
static char GOutputFileName[FILENAME_MAX];
static char GInputFile[FILENAME_MAX];
static char GClassName[FILENAME_MAX];
static LanguageEnum GLanguage = KLanguageOP;

/*	PrintHelp
 *
 *		Print a help message
 */

static void PrintHelp()
{
	printf("%s\n",GHelp);
	exit(0);
}

/*	PrintError
 *
 *		print the command line invocation, an error message, and help
 */

static void PrintError(int argc, const char *argv[])
{
	printf("Command line arguments contains an error:\n");
	for (int i = 0; i < argc; ++i) {
		printf("%s ",argv[i]);
	}
	printf("\n\n");
	PrintHelp();
	exit(1);
}

/*	ParseArgs
 *
 *		Parse the input arguments, pulling the parameters for invoking this
 *	app, and storing away the contents
 *
 *		The parameters we accept are:
 *
 *		-h			Help
 *		-c class	Output class name (default is input file name)
 *		-o file		Output file name (default is input file name)
 *		input		Input file name
 */

static void ParseArgs(int argc, const char *argv[])
{
	const char *ptr;

	GOutputFile[0] = 0;
	GInputFile[0] = 0;
	GClassName[0] = 0;

	for (int i = 1; i < argc; ) {
		ptr = argv[i++];
		if (*ptr == '-') {
			if (!strcmp(ptr,"-h")) {
				PrintHelp();
			} else if (!strcmp(ptr,"-l")) {
				if (i >= argc) {
					PrintError(argc,argv);
				}
				ptr = argv[i++];
				if (!strcmp(ptr,"oc")) {
					GLanguage = KLanguageOP;
				} else if (!strcmp(ptr,"cpp")) {
					GLanguage = KLanguageCPP;
				} else if (!strcmp(ptr,"swift")) {
					GLanguage = KLanguageSwift;
				} else {
					PrintError(argc,argv);
				}
			} else if (!strcmp(ptr,"-c")) {
				if (i >= argc) {
					PrintError(argc,argv);
				}
				if (GClassName[0]) {
					PrintError(argc,argv);
				}
				strncpy(GClassName,argv[i++],sizeof(GClassName)-1);
			} else if (!strcmp(ptr,"-o")) {
				if (i >= argc) {
					PrintError(argc,argv);
				}
				if (GOutputFile[0]) {
					PrintError(argc,argv);
				}
				strncpy(GOutputFile,argv[i++],sizeof(GOutputFile)-1);
			} else {
				PrintError(argc,argv);
			}
		} else {
			if (GInputFile[0]) {
				PrintError(argc,argv);
			}
			strncpy(GInputFile,ptr,sizeof(GInputFile)-1);
		}
	}

	if (GInputFile[0] == 0) {
		printf("Missing input file\n\n");
		PrintHelp();
	}
}

/*	main
 *
 *		Kick off the parser
 */

int main(int argc, const char * argv[])
{
	char *x, *y, *w;
	size_t s;
	char scratch[FILENAME_MAX];

	/*
	 *	Parse the arguments
	 */

	ParseArgs(argc,argv);

	/*
	 *	Now calculate the output file and class names from the input file,
	 *	if one is not provided.
	 */

	if (GOutputFile[0] == 0) {
		/*
		 *	Trim '.'
		 */

		strncpy(GOutputFile,GInputFile,sizeof(GOutputFile)-1);
		y = NULL;
		for (x = GOutputFile; *x; ++x) {
			if (*x == '/') y = NULL;
			if (*x == '.') y = x;
		}

		if (y) *y = 0;		// trim final .extension from file name
	}

	/*
	 *	Find the output file name. This is just the name of the file, and
	 *	is used when constructing the #include
	 */

	y = NULL;
	w = GOutputFile;

	for (x = GInputFile; *x; ++x) {
		if (*x == '/') {
			w = x+1;
			y = NULL;
		}
		if (*x == '.') y = x;
	}

	if (y == NULL) {
		strncpy(GOutputFileName,w,sizeof(GOutputFileName)-1);
	} else {
		s = y - w;
		if (s > sizeof(GOutputFileName)-1) s = sizeof(GOutputFileName)-1;
		memmove(GOutputFileName, w, s);
		GOutputFileName[s] = 0;
	}

	/*
	 *	Calculate the class name
	 */

	if (GClassName[0] == 0) {
		strncpy(GClassName,GOutputFileName,sizeof(GClassName)-1);
	}

	/*
	 *	Now load the lexer and parser to parse the input file
	 */

	OCLexer lexer;
	if (!lexer.OpenFile(GInputFile)) {
		printf("Input file %s not found\n\n",GInputFile);
		PrintError(argc, argv);
	}

	OCYaccParser parser;
	if (!parser.ParseFile(lexer)) {
		return -1;
	}

	/*
	 *	Compile the state machine
	 */

	OCYaccLR1 stateMachine;
	if (!stateMachine.Construct(parser)) {
		// Error during state machine construction
		return -1;
	}

	/*
	 *	Now generate the output file
	 */

	if (GLanguage == KLanguageOP) {
		OCYaccGenerator generator(parser,stateMachine);

		// Now write the final output files
		strncpy(scratch,GOutputFile,sizeof(scratch)-1);
		strncat(scratch,".h",sizeof(scratch) - strlen(scratch) - 1);
		FILE *out = fopen(scratch,"w");
		if (out == NULL) {
			printf("Unable to write to file %s\n\n",scratch);
			PrintError(argc, argv);
		}
		generator.WriteOCHeader(GClassName, GOutputFileName, out);
		fclose(out);

		strncpy(scratch,GOutputFile,sizeof(scratch));
		strncat(scratch,".m",sizeof(scratch) - strlen(scratch) - 1);
		out = fopen(scratch,"w");
		if (out == NULL) {
			printf("Unable to write to file %s\n\n",scratch);
			PrintError(argc, argv);
		}
		generator.WriteOCFile(GClassName, GOutputFileName, out);
		fclose(out);
	} else if (GLanguage == KLanguageCPP) {
		OCYaccCPPGenerator generator(parser,stateMachine);

		// Now write the final output files
		strncpy(scratch,GOutputFile,sizeof(scratch)-1);
		strncat(scratch,".h",sizeof(scratch) - strlen(scratch) - 1);
		FILE *out = fopen(scratch,"w");
		if (out == NULL) {
			printf("Unable to write to file %s\n\n",scratch);
			PrintError(argc, argv);
		}
		generator.WriteOCHeader(GClassName, GOutputFileName, out);
		fclose(out);

		strncpy(scratch,GOutputFile,sizeof(scratch));
		strncat(scratch,".cpp",sizeof(scratch) - strlen(scratch) - 1);
		out = fopen(scratch,"w");
		if (out == NULL) {
			printf("Unable to write to file %s\n\n",scratch);
			PrintError(argc, argv);
		}
		generator.WriteOCFile(GClassName, GOutputFileName, out);
		fclose(out);
	} else if (GLanguage == KLanguageSwift) {
		OCYaccSwiftGenerator generator(parser,stateMachine);

		// Now write the final output files
		strncpy(scratch,GOutputFile,sizeof(scratch));
		strncat(scratch,".swift",sizeof(scratch) - strlen(scratch) - 1);
		FILE *out = fopen(scratch,"w");
		if (out == NULL) {
			printf("Unable to write to file %s\n\n",scratch);
			PrintError(argc, argv);
		}
		generator.WriteOCFile(GClassName, GOutputFileName, out);
		fclose(out);
	}

	/*
	 *	Done.
	 */
	
	return 0;
}
