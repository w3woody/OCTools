/*	simple.y
 *
 *		This is the first version of the Simple programming language; one that
 *	operates like Basic but uses C style expression syntax.
 */

%{
#import "ExpressionNode.h"
#import "ValueNode.h"
#import "FunctionNode.h"
#import "UnaryNode.h"
#import "BinaryNode.h"
#import "ConditionalNode.h"
#import "TypeNameNode.h"
#import "TypecastNode.h"
#import "InitDeclarationNode.h"
#import "StatementNode.h"
#import "DeclarationNode.h"
#import "SimpleStatement.h"
#import "ExpressionStatement.h"
#import "LabelStatement.h"
#import "CaseStatement.h"
%}

/*
 *	Token declarations
 */

%token <NSString> IDENTIFIER CONSTANT STRING_LITERAL INTEGER_CONSTANT

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

%type <NSMutableArray<ExpressionNode *>> argument_expression_list

%type <TypeNameNode> type_name

%type <InitDeclarationNode> init_declarator
%type <NSMutableArray<InitDeclarationNode *>> init_declarator_list
%type <DeclarationNode> declaration

%type <StatementNode> statement expression_statement labeled_statement
		selection_statement iteration_statement jump_statement
		switch_label

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
				$$ = [[ValueNode alloc] initWithIdentifier:$1];
			}
	| CONSTANT
			{
				$$ = [[ValueNode alloc] initWithConstant:$1];
			}
	| INTEGER_CONSTANT
			{
				$$ = [[ValueNode alloc] initWithIntegerConstant:$1];
			}
	| STRING_LITERAL
			{
				$$ = [[ValueNode alloc] initWithStringLiteral:$1];
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
				$$ = [[FunctionNode alloc] initWithFunction:$1 expression:nil];
			}
	| IDENTIFIER '(' argument_expression_list ')'
			{
				$$ = [[FunctionNode alloc] initWithFunction:$1 expression:$3];
			}
	| postfix_expression INC_OP
			{
				$$ = [[UnaryNode alloc] initWithUnaryOp:OP_POSTINC expression:$1];
			}
	| postfix_expression DEC_OP
			{
				$$ = [[UnaryNode alloc] initWithUnaryOp:OP_POSTDEC expression:$1];
			}
	;

argument_expression_list
	: assignment_expression
			{
				$$ = [NSMutableArray arrayWithObject:$1];
			}
	| argument_expression_list ',' assignment_expression
			{
				$$ = $1;
				[$$ addObject:$3];
			}
	;

unary_expression
	: postfix_expression
			{
				$$ = $1;
			}
	| INC_OP unary_expression
			{
				$$ = [[UnaryNode alloc] initWithUnaryOp:OP_PREINC expression:$2];
			}
	| DEC_OP unary_expression
			{
				$$ = [[UnaryNode alloc] initWithUnaryOp:OP_PREDEC expression:$2];
			}
	| '+' cast_expression
			{
				$$ = [[UnaryNode alloc] initWithUnaryOp:'+' expression:$2];
			}
	| '-' cast_expression
			{
				$$ = [[UnaryNode alloc] initWithUnaryOp:'-' expression:$2];
			}
	| '~' cast_expression
			{
				$$ = [[UnaryNode alloc] initWithUnaryOp:'~' expression:$2];
			}
	| '!' cast_expression
			{
				$$ = [[UnaryNode alloc] initWithUnaryOp:'!' expression:$2];
			}
	;

cast_expression
	: unary_expression
			{
				$$ = $1;
			}
	| '(' type_name ')' cast_expression
			{
				$$ = [[TypecastNode alloc] initWithType:$2 expression:$4];
			}
	;

binary_expression
	: cast_expression
			{
				$$ = $1;
			}
	| binary_expression '*' binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:'*' left:$1 right:$3];
			}
	| binary_expression '/' binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:'/' left:$1 right:$3];
			}
	| binary_expression '%' binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:'%' left:$1 right:$3];
			}
	| binary_expression '+' binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:'+' left:$1 right:$3];
			}
	| binary_expression '-' binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:'-' left:$1 right:$3];
			}
	| binary_expression LEFT_OP binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_LEFTSHIFT left:$1 right:$3];
			}
	| binary_expression RIGHT_OP binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_RIGHTSHIFT left:$1 right:$3];
			}
	| binary_expression '<' binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:'<' left:$1 right:$3];
			}
	| binary_expression '>' binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:'>' left:$1 right:$3];
			}
	| binary_expression LE_OP binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_LEQ left:$1 right:$3];
			}
	| binary_expression GE_OP binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_GEQ left:$1 right:$3];
			}
	| binary_expression EQ_OP binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_EQ left:$1 right:$3];
			}
	| binary_expression NE_OP binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_NEQ left:$1 right:$3];
			}
	| binary_expression '&' binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:'&' left:$1 right:$3];
			}
	| binary_expression '^' binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:'^' left:$1 right:$3];
			}
	| binary_expression '|' binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:'|' left:$1 right:$3];
			}
	| binary_expression AND_OP binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_AND left:$1 right:$3];
			}
	| binary_expression OR_OP binary_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_OR left:$1 right:$3];
			}
	;

conditional_expression
	: binary_expression
			{
				$$ = $1;
			}
	| binary_expression '?' expression ':' conditional_expression
			{
				$$ = [[ConditionalNode alloc] initWithConditional:$1 left:$3 right:$5];
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
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_ASSIGN left:$1 right:$3];
			}
	| unary_expression MUL_ASSIGN assignment_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_MULEQ left:$1 right:$3];
			}
	| unary_expression DIV_ASSIGN assignment_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_DIVEQ left:$1 right:$3];
			}
	| unary_expression MOD_ASSIGN assignment_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_MODEQ left:$1 right:$3];
			}
	| unary_expression ADD_ASSIGN assignment_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_ADDEQ left:$1 right:$3];
			}
	| unary_expression SUB_ASSIGN assignment_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_SUBEQ left:$1 right:$3];
			}
	| unary_expression LEFT_ASSIGN assignment_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_LEFTEQ left:$1 right:$3];
			}
	| unary_expression RIGHT_ASSIGN assignment_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_RIGHTEQ left:$1 right:$3];
			}
	| unary_expression AND_ASSIGN assignment_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_ANDEQ left:$1 right:$3];
			}
	| unary_expression XOR_ASSIGN assignment_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_XOREQ left:$1 right:$3];
			}
	| unary_expression OR_ASSIGN assignment_expression
			{
				$$ = [[BinaryNode alloc] initWithBinaryOp:OP_OREQ left:$1 right:$3];
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
				$$ = [[BinaryNode alloc] initWithBinaryOp:',' left:$1 right:$3];
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
			{
				$$ = $1;
			}
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
		{
			$$ = [[TypeNameNode alloc] initWithType:TYPENAME_CHAR];
		}
	| SHORT
		{
			$$ = [[TypeNameNode alloc] initWithType:TYPENAME_SHORT];
		}
	| INT
		{
			$$ = [[TypeNameNode alloc] initWithType:TYPENAME_INT];
		}
	| LONG
		{
			$$ = [[TypeNameNode alloc] initWithType:TYPENAME_LONG];
		}
	| FLOAT
		{
			$$ = [[TypeNameNode alloc] initWithType:TYPENAME_FLOAT];
		}
	| DOUBLE
		{
			$$ = [[TypeNameNode alloc] initWithType:TYPENAME_DOUBLE];
		}
	| STRING
		{
			$$ = [[TypeNameNode alloc] initWithType:TYPENAME_STRING];
		}
	;

init_declarator
	: IDENTIFIER
		{
			$$ = [[InitDeclarationNode alloc] initWithName:$1 expression:nil];
		}
	| IDENTIFIER '=' assignment_expression
		{
			$$ = [[InitDeclarationNode alloc] initWithName:$1 expression:$3];
		}
	;

init_declarator_list
	: init_declarator
		{
			$$ = [NSMutableArray arrayWithObject:$1];
		}
	| init_declarator_list ',' init_declarator
		{
			$$ = $1;
			[$1 addObject:$3];
		}
	;

declaration
	: type_name init_declarator_list ';'
		{
			$$ = [[DeclarationNode alloc] initWithType:$1 declarations:$2];
		}
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
		{
			$$ = [[ExpressionStatement alloc] initWithExpression:$1];
		}
	| ';'
		{
			$$ = [[ExpressionStatement alloc] initWithExpression:nil];
		}
	;

labeled_statement				// Note: we call out case/default elsewhere
	: IDENTIFIER ':'
		{
			$$ = [[LabelStatement alloc] initWithType:TYPE_LABEL label:$1];
		}
	;

switch_label
	: CASE constant_expression ':'
		{
			$$ = [[CaseStatement alloc] initWithExpression:$2];
			if (!$2.isValid) {
				[self errorWithFormat:@"case statement constant must be an integer"];
			}
		}
	| DEFAULT ':'
		{
			$$ = [[SimpleStatement alloc] initWithType:TYPE_DEFAULT];
		}
	;

switch_statement
	: switch_label
	| statement
	;

switch_statements
	: switch_statements switch_statement
	| switch_statement
	;

selection_statement
	: IF '(' expression ')' statement %prec IFSTATEMENT
	| IF '(' expression ')' statement ELSE statement
	| SWITCH '(' expression ')' '{' switch_label switch_statements '}'
	;

iteration_statement
	: WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	;

jump_statement
	: GOTO IDENTIFIER ';'
		{
			$$ = [[LabelStatement alloc] initWithType:TYPE_GOTO label:$2];
		}
	| GOSUB IDENTIFIER ';'		// new gosub; return resumes execution
		{
			$$ = [[LabelStatement alloc] initWithType:TYPE_GOSUB label:$2];
		}
	| CONTINUE ';'
		{
			$$ = [[SimpleStatement alloc] initWithType:TYPE_CONTINUE];
		}
	| BREAK ';'
		{
			$$ = [[SimpleStatement alloc] initWithType:TYPE_BREAK];
		}
	| RETURN ';'
		{
			$$ = [[SimpleStatement alloc] initWithType:TYPE_RETURN];
		}
	;

statement
	: declaration				// allow embedded declarations
		{
			$$ = $1;
		}
	| labeled_statement
		{
			$$ = $1;
		}
	| expression_statement
		{
			$$ = $1;
		}
	| selection_statement
		{
			$$ = $1;
		}
	| iteration_statement
		{
			$$ = $1;
		}
	| jump_statement
		{
			$$ = $1;
		}
	;


/*
 *	Our language is flat; we don't have functions. (Our function calls are
 *	all to pre-defined built-ins beyond the scope of this grammar.)
 */

statements : statements statement
		   | statement
		   ;

