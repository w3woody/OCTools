/*	CalcParser.y
 *
 *		A very simple parser to parse calculator inputs. Works hand in hand
 *	with CalcLex.l
 */

%token <NSNumber> NUMBER
%type <NSNumber> expression

/* Establish precedence of our operators */
%left '+' '-'
%left '*' '/'

/* Establish our return value */
%global {
	@property (strong) NSMutableArray<NSNumber *> *result;
}

%init {
	self.result = [[NSMutableArray alloc] init];
}

%start equations

%%

expression: NUMBER							{ $$ = $1; }
		  | expression '+' expression		{ $$ = @( $1.intValue + $3.intValue ); }
		  | expression '-' expression		{ $$ = @( $1.intValue - $3.intValue ); }
		  | expression '*' expression		{ $$ = @( $1.intValue * $3.intValue ); }
		  | expression '/' expression		{ $$ = @( $1.intValue / $3.intValue ); }
		  | '(' expression ')'				{ $$ = $2; }
		  ;

equation: expression ';'					{ [self.result addObject:$1]; }
		| error ';'							{ [self errorWithCode:ERROR_SYNTAX]; }
		;

equations: equation
         | equation equations
         ;

