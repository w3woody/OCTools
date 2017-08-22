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
%type <NSMutableArray<NSNumber *>> statements

%%

statements : statement					{ $$ = [@[ $1 ] mutableCopy]; }
		   | statements statement		{ [$1 addObject:$2]; $$ = $1; }
		   ;

statement  : assignment ';'				{ $$ = $1; }
		   | error ';'					{ $$ = @0; [self errorWithCode:ERROR_SYNTAX]; }
		   ;

assignment : TOKEN '=' expression		{ $$ = $3; }
		   | TOKEN '=' assignment		{ $$ = $3; }
		   ;

expression : '(' expression ')'			{ $$ = $2; }
		   | expression '+' expression	{ $$ = @($1.doubleValue + $3.doubleValue); }
		   | expression '-' expression	{ $$ = @($1.doubleValue - $3.doubleValue); }
		   | expression '*' expression	{ $$ = @($1.doubleValue * $3.doubleValue); }
		   | expression '/' expression	{ $$ = @($1.doubleValue / $3.doubleValue); }
		   | NUMBER						{ $$ = $1; }
		   ;
