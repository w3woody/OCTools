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
	@property (strong) NSMutableArray<NSNumber *> *result;
}

%init {
	self.result = [[NSMutableArray alloc] init];
}

%errors {
#define ERROR_UNIMPLEMENTED     (ERROR_STARTERRORID+1)
}

%start equations

%%

expression: NUMBER							{ $$ = $1; }
		  | expression '+' expression		{ $$ = @( $1.intValue + $3.intValue ); }
		  | expression '-' expression		{ $$ = @( $1.intValue - $3.intValue ); }
		  | expression '*' expression		{ $$ = @( $1.intValue * $3.intValue ); }
		  | expression '/' expression		{ $$ = @( $1.intValue / $3.intValue ); }
		  | '(' expression ')'				{ $$ = $2; }
		  | IDENTIFIER						{
												[self errorWithCode:ERROR_UNIMPLEMENTED];
												$$ = @0;
		  									}
		  ;

equation: expression ';'					{ [self.result addObject:$1]; }
		| error ';'							{ [self errorWithCode:ERROR_SYNTAX]; }
		;

equations: equation
         | equation equations
         ;

