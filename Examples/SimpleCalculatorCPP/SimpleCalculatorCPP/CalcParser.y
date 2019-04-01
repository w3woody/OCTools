/*	CalcParser.y
 *
 *		A very simple parser to parse calculator inputs. Works hand in hand
 *	with CalcLex.l
 */

// Tell C++ generated class about lexer class
%lex CalcLex

// Establish our types
%union {
	int value;
}

%token <value> NUMBER
%type <value> expression

/* Establish precedence of our operators */
%left '+' '-'
%left '*' '/'

/* Establish our return value */
%global {
	int result;
}

%start equation

%%

expression: NUMBER							{ $$ = $1; }
		  | expression '+' expression		{ $$ = $1 + $3; }
		  | expression '-' expression		{ $$ = $1 - $3; }
		  | expression '*' expression		{ $$ = $1 * $3; }
		  | expression '/' expression		{ $$ = $1 / $3; }
		  | '(' expression ')'				{ $$ = $2; }
		  ;

equation: expression						{ result = $1; }
		;
