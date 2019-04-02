/*	CalcParser.y
 *
 *		A very simple parser to parse calculator inputs. Works hand in hand
 *	with CalcLex.l
 */

// Tell C++ generated class about lexer class
%lex CalcLex

// Establish our types
%union {
	int integer;
}

%token <integer> NUMBER
%token IDENTIFIER
%type <integer> expression

/* Establish precedence of our operators */
%left '+' '-'
%left '*' '/'

/* Establish our return value */
%global {
	std::vector<int> result;
}

%errors {
#define ERROR_UNIMPLEMENTED     (ERROR_STARTERRORID+1)
}

%start equations

%%

expression: NUMBER							{ $$ = $1; }
		  | expression '+' expression		{ $$ = $1 + $3; }
		  | expression '-' expression		{ $$ = $1 - $3; }
		  | expression '*' expression		{ $$ = $1 * $3; }
		  | expression '/' expression		{ $$ = $1 / $3; }
		  | '(' expression ')'				{ $$ = $2; }
		  | IDENTIFIER						{
		  										errorWithCode(ERROR_UNIMPLEMENTED);
												$$ = 0;
		  									}
		  ;

equation: expression ';'					{ result.push_back($1); }
		| error ';'							{ errorWithCode(ERROR_SYNTAX); }
		;

equations: equation
         | equation equations
         ;

