/*	simple.y
 *
 *		This is the first version of the Simple programming language; one that
 *	operates like Basic but uses C style expression syntax.
 */

%{
#import "ExpressionNode.h"
%}

/*
 *	Token declarations
 */

%token <NSString> IDENTIFIER CONSTANT STRING_LITERAL

%token SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID STRING
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO GOSUB CONTINUE BREAK RETURN

/*
 *	Operator precedence.
 *
 *		http://en.cppreference.com/w/cpp/language/operator_precedence
 *
 *	Note that the structure of our declarations makes some of these obsolete,
 *	but they're listed here for completeness.
 */

%left ','
%right TERNOP '=' LEFT_ASSIGN RIGHT_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN
	   DIV_ASSIGN MOD_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN
%left OR_OP
%left AND_OP
%left '|'
%left '^'
%left '&'
%left EQ_OP NE_OP
%left '<' '>' LE_OP GE_OP			// Compare operators
%left LEFT_OP RIGHT_OP
%left '+' '-'
%left '*' '/' '%'
%right PREINC PREDEC PREFIXOP
%left POSTINC POSTDEC FUNCCALL SUBSCRIPT

/*
 *	To handle the if/else conflict
 */

%left IFSTATEMENT
%left ELSE

/*
 *	Statement values
 */

%type <ExpressionNode> primary_expression postfix_expression
		unary_expression cast_expression binary_expression
		conditional_expression assignment_expression expression
		constant_expression

%type <NSMutableArray<ExpressionNode *>> argument_expression_list;

/*
 *	Start: statements list
 */

%start statements

%%

/*
 *	A lot of this is based on the Jeff Lee Yacc grammar, found here:
 *
 *		http://www.lysator.liu.se/c/ANSI-C-grammar-y.html
 *
 *	We collapse some of the statements to reduce the number of states.
 *
 *	The following gives the expression syntax for expressions in our language.
 */

primary_expression
	: IDENTIFIER
			{
				$$ = [[ExpressionNode alloc] initWithIdentifier:$1];
			}
	| CONSTANT
			{
				$$ = [[ExpressionNode alloc] initWithConstant:$1];
			}
	| STRING_LITERAL
			{
				$$ = [[ExpressionNode alloc] initWithStringLiteral:$1];
			}
	| '(' expression ')'
			{
				$$ = $2;
			}
	;

postfix_expression
	: primary_expression
			{
				$$ = $1;
			}
	| IDENTIFIER '(' ')'
			{
				$$ = [[ExpressionNode alloc] initWithFunction:$1 expression:nil];
			}
	| IDENTIFIER '(' argument_expression_list ')'
			{
				$$ = [[ExpressionNode alloc] initWithFunction:$1 expression:$3];
			}
	| postfix_expression INC_OP
			{
				$$ = [[ExpressionNode alloc] initWithUnaryOp:OP_POSTINC expression:$1];
			}
	| postfix_expression DEC_OP
			{
				$$ = [[ExpressionNode alloc] initWithUnaryOp:OP_POSTDEC expression:$1];
			}
	;

argument_expression_list
	: assignment_expression
			{
				$$ = $1;
			}
	| argument_expression_list ',' assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:',' left:$1 right:$3];
			}
	;

unary_expression
	: postfix_expression
			{
				$$ = $1;
			}
	| INC_OP unary_expression
			{
				$$ = [[ExpressionNode alloc] initWithUnaryOp:OP_PREINC expression:$2];
			}
	| DEC_OP unary_expression
			{
				$$ = [[ExpressionNode alloc] initWithUnaryOp:OP_PREDEC expression:$2];
			}
	| '+' cast_expression
			{
				$$ = [[ExpressionNode alloc] initWithUnaryOp:'+' expression:$2];
			}
	| '-' cast_expression
			{
				$$ = [[ExpressionNode alloc] initWithUnaryOp:'-' expression:$2];
			}
	| '~' cast_expression
			{
				$$ = [[ExpressionNode alloc] initWithUnaryOp:'~' expression:$2];
			}
	| '!' cast_expression
			{
				$$ = [[ExpressionNode alloc] initWithUnaryOp:'!' expression:$2];
			}
	;

cast_expression
	: unary_expression
			{
				$$ = $1;
			}
	| '(' type_name ')' cast_expression
			{
				$$ = $4;		// ### TODO: Hook up casting of expressions
			}
	;

binary_expression
	: cast_expression
			{
				$$ = $1;
			}
	| binary_expression '*' binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:'*' left:$1 right:$3];
			}
	| binary_expression '/' binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:'/' left:$1 right:$3];
			}
	| binary_expression '%' binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:'%' left:$1 right:$3];
			}
	| binary_expression '+' binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:'+' left:$1 right:$3];
			}
	| binary_expression '-' binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:'-' left:$1 right:$3];
			}
	| binary_expression LEFT_OP binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_LEFTSHIFT left:$1 right:$3];
			}
	| binary_expression RIGHT_OP binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_RIGHTSHIFT left:$1 right:$3];
			}
	| binary_expression '<' binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:'<' left:$1 right:$3];
			}
	| binary_expression '>' binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:'>' left:$1 right:$3];
			}
	| binary_expression LE_OP binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_LEQ left:$1 right:$3];
			}
	| binary_expression GE_OP binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_GEQ left:$1 right:$3];
			}
	| binary_expression EQ_OP binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_EQ left:$1 right:$3];
			}
	| binary_expression NE_OP binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_NEQ left:$1 right:$3];
			}
	| binary_expression '&' binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:'&' left:$1 right:$3];
			}
	| binary_expression '^' binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:'^' left:$1 right:$3];
			}
	| binary_expression '|' binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:'|' left:$1 right:$3];
			}
	| binary_expression AND_OP binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_AND left:$1 right:$3];
			}
	| binary_expression OR_OP binary_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OR_OP left:$1 right:$3];
			}
	;

conditional_expression
	: binary_expression
			{
				$$ = $1;
			}
	| binary_expression '?' expression ':' conditional_expression
			{
				$$ = [[ExpressionNode alloc] initWithConditional:$1 left:$3 right:$5];
			}
	;

/*
 *	Assignment expression is used as part of the initializer
 */

assignment_expression
	: conditional_expression
			{
				$$ = $1;
			}
	| unary_expression '=' assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_ASSIGN left:$1 right:$3];
			}
	| unary_expression MUL_ASSIGN assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_MULEQ left:$1 right:$3];
			}
	| unary_expression DIV_ASSIGN assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_DIVEQ left:$1 right:$3];
			}
	| unary_expression MOD_ASSIGN assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_MODEQ left:$1 right:$3];
			}
	| unary_expression ADD_ASSIGN assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_ADDEQ left:$1 right:$3];
			}
	| unary_expression SUB_ASSIGN assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_SUBEQ left:$1 right:$3];
			}
	| unary_expression LEFT_ASSIGN assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_LEFTEQ left:$1 right:$3];
			}
	| unary_expression RIGHT_ASSIGN assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_RIGHTEQ left:$1 right:$3];
			}
	| unary_expression AND_ASSIGN assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_ANDEQ left:$1 right:$3];
			}
	| unary_expression XOR_ASSIGN assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_XOREQ left:$1 right:$3];
			}
	| unary_expression OR_ASSIGN assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:OP_OREQ left:$1 right:$3];
			}
	;

/*
 *	Expression is the catch-all for where we use expressions
 */

expression
	: assignment_expression
			{
				$$ = $1;
			}
	| expression ',' assignment_expression
			{
				$$ = [[ExpressionNode alloc] initWithBinaryOp:',' left:$1 right:$3];
			}
	;

/*
 *	Constant expressions are used where we need constants, and must be
 *	evaluable at compile time. Semantically they're similar to conditional
 *	expressions, but we do extra processing here to make sure we can reduce
 *	the value to a constant value.
 *
 *	Note that use of these constants may impose greater restrictions, such
 *	as requiring positive integers.
 */

constant_expression
	: conditional_expression
	;

/*
 *	Declaration specifiers.
 *
 *		Unlike C we allow declarations inside a block of statements. This
 *	is a subset of declarations allowed in C. Note our langauge does not
 *	permit anything but basic types, such as integers, floats and strings.
 */

type_name
	: CHAR
	| SHORT
	| INT
	| LONG
	| FLOAT
	| DOUBLE
	| STRING
	;

init_declarator
	: IDENTIFIER
	| IDENTIFIER '=' assignment_expression
	;

init_declarator_list
	: init_declarator
	| init_declarator_list ',' init_declarator
	;

declaration
	: type_name init_declarator_list ';'
	;


/*
 *	Language specification.
 *
 *		For the Simple language, we restrict our flow control statements
 *	to the various iteration statements, jump statements and introduce a
 *	new gosub statement with a corresponding goto statement.
 *
 *		All of these are extracted from the C specification so we follow
 *	similar syntax.
 */

expression_statement
	: expression ';'
	| ';'
	;

labeled_statement				// Note: we call out case/default elsewhere
	: IDENTIFIER ':' statement
	;

switch_statement
	: CASE constant_expression ':' statement
	| DEFAULT ':' statement
	| statement
	;

switch_statements
	: switch_statements switch_statement
	| switch_statement
	;

selection_statement
	: IF '(' expression ')' statement %prec IFSTATEMENT
	| IF '(' expression ')' statement ELSE statement
	| SWITCH '(' expression ')' '{' switch_statements '}'
	;

iteration_statement
	: WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	;

jump_statement
	: GOTO IDENTIFIER ';'
	| GOSUB IDENTIFIER ';'		// new gosub; return resumes execution
	| CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	;

statement
	: declaration				// allow embedded declarations
	| labeled_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;


/*
 *	Our language is flat; we don't have functions. (Our function calls are
 *	all to pre-defined built-ins beyond the scope of this grammar.)
 */

statements : statements statement
		   | statement
		   ;

