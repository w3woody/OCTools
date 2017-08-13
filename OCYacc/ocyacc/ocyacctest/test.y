/*
 *	Test language, contains four operators and parenthesis, separated by
 *	semicolons
 */

%token NUMBER TOKEN

%left '*' '/'
%left '+' '-'
%right '='

%start statements

%%

statements : statement
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
