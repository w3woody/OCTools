/*
 *	Test language, contains four operators and parenthesis, separated by
 *	semicolons
 */

%left '*' '/'
%left '+' '-'
%right '='

%union {
	int a;
	long b;
}

%token <a> NUMBER
%token <b> TOKEN

%type <a> statements statement
%type <b> assignment

%lex OCLexTest

%start statements

%%

statements : statement					{ $$ = $1; }
		   | statements statement		{ }
		   ;

statement  : assignment ';'				{ }
		   | error ';'					{ }
		   ;

assignment : TOKEN '=' expression		{ }
		   | TOKEN '=' assignment		{ }
		   ;

expression : '(' expression ')'			{ }
		   | expression '+' expression	{ }
		   | expression '-' expression	{ }
		   | expression '*' expression	{ }
		   | expression '/' expression	{ }
		   | NUMBER						{ }
		   ;
