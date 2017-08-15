/*
 *	Test language, contains four operators and parenthesis, separated by
 *	semicolons
 */

%token NUMBER TOKEN

%left '*' '/'
%left '+' '-'
%right '='

%start statements

%type <NSNumber> statement

%%

statements : statement					{ $$ = $1; }
		   | statements statement
		   ;

statement  : assignment ';'
		   | error ';'
		   ;

assignment : TOKEN '=' expression
		   | TOKEN '=' assignment
		   ;

expression : '(' expression ')'
		   | expression '+' expression
		   | expression '-' expression
		   | expression '*' expression
		   | expression '/' expression
		   | NUMBER
		   | TOKEN
		   ;
