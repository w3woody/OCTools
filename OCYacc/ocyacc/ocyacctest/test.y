/*
 *	Test language, contains four operators and parenthesis, separated by
 *	semicolons
 */

%token <NSNumber> NUMBER
%token <NSString> TOKEN

%left '*' '/'
%left '+' '-'
%right '='

%start statements

%type <NSNumber> statement expression assignment
%type <NSArray<NSNumber *>> statements

%%

statements : statement					{ $$ = $1; }
		   | statements statement		{ [$$ addObject:$1]; }
		   ;

statement  : assignment ';'				{ $$ = $1; }
		   | error ';'					{ [self errorWithFormat:@"?"]; }
		   ;

assignment : TOKEN '=' expression		{ $$ = $1; }
		   | TOKEN '=' assignment		{ $$ = $1; }
		   ;

expression : '(' expression ')'			{ $$ = $2; }
		   | expression '+' expression	{ $$ = $1 + $3; }
		   | expression '-' expression	{ $$ = $1 - $3; }
		   | expression '*' expression	{ $$ = $1 * $3; }
		   | expression '/' expression	{ $$ = $1 / $3; }
		   | NUMBER						{ $$ = $1; }
		   | TOKEN						{ $$ = $1; }
		   ;
