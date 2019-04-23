/*	CalcParser.y
 *
 *		A very simple parser to parse calculator inputs. Works hand in hand
 *	with CalcLex.l
 */

%token <NSNumber> NUMBER
%token IDENTIFIER

%type <NSNumber> expression

/* Establish precedence of our operators */
%left '+' '-'
%left '*' '/'

/* Establish our return value */
%global {
	public var result:[NSNumber] = []
}

%errors {
	static let ERROR_UNIMPLEMENTED = (CalcParser.ERROR_STARTERRORID+1)
}

%start equations

%%

expression: NUMBER							{ $$ = $1 }
		  | expression '+' expression		{ $$ = NSNumber(value: $1.intValue + $3.intValue) }
		  | expression '-' expression		{ $$ = NSNumber(value: $1.intValue - $3.intValue) }
		  | expression '*' expression		{ $$ = NSNumber(value: $1.intValue * $3.intValue) }
		  | expression '/' expression		{ $$ = NSNumber(value: $1.intValue / $3.intValue) }
		  | '(' expression ')'				{ $$ = $2 }
		  | IDENTIFIER						{
												error(code: CalcParser.ERROR_UNIMPLEMENTED)
												$$ = NSNumber(0)
		  									}
		  ;

equation: expression ';'					{ result.append($1) }
		| error ';'							{ error(code: CalcParser.ERROR_SYNTAX) }
		;

equations: equation
         | equation equations
         ;

