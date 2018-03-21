/*	simple.y
 *
 *		This is the first version of the Simple programming language; one that
 *	operates like Basic but uses C style expression syntax.
 */

%{
%}

/*
 *	Class declarations
 *
 *		SymbolPool is used during parsing to track which variables have been
 *	defined, so we can determine if something is undefined, or if it is
 *	a duplicate definition. (We structure the reduce for our compound statement,
 *	structure and class definitions in order to support the push/pop scope in
 *	the correct order.)
 */

/*
 *	Token declarations
 */

%token <NSString> IDENTIFIER REAL_CONSTANT STRING_LITERAL CHAR_LITERAL
		INTEGER_CONSTANT TYPE_NAME

%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME SIZEOF

%token BYTE SHORT INT LONG UNSIGNED FLOAT DOUBLE VOID STRUCT CLASS ABSTRACT

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

%nonassoc GOSUB

/*
 *	Start: statements list
 */

%start translation_unit

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
			$$ = $1_file;
		}
	| REAL_CONSTANT
	| INTEGER_CONSTANT
	| CHAR_LITERAL
	| STRING_LITERAL
	| '(' expression ')'
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'
	| postfix_expression '(' ')'
	| postfix_expression '(' argument_expression_list ')'
	| postfix_expression '.' IDENTIFIER
	| postfix_expression PTR_OP IDENTIFIER
	| postfix_expression INC_OP
	| postfix_expression DEC_OP
	;

argument_expression_list
	: assignment_expression
	| argument_expression_list ',' assignment_expression
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression
	| DEC_OP unary_expression
	| '+' cast_expression
	| '-' cast_expression
	| '~' cast_expression
	| '!' cast_expression
	| SIZEOF unary_expression
	| SIZEOF '(' abstract_declaration ')'
	;

cast_expression
	: unary_expression
	| '(' abstract_declaration ')' cast_expression
	;

binary_expression
	: cast_expression
	| binary_expression '*' binary_expression
	| binary_expression '/' binary_expression
	| binary_expression '%' binary_expression
	| binary_expression '+' binary_expression
	| binary_expression '-' binary_expression
	| binary_expression LEFT_OP binary_expression
	| binary_expression RIGHT_OP binary_expression
	| binary_expression '<' binary_expression
	| binary_expression '>' binary_expression
	| binary_expression LE_OP binary_expression
	| binary_expression GE_OP binary_expression
	| binary_expression EQ_OP binary_expression
	| binary_expression NE_OP binary_expression
	| binary_expression '&' binary_expression
	| binary_expression '^' binary_expression
	| binary_expression '|' binary_expression
	| binary_expression AND_OP binary_expression
	| binary_expression OR_OP binary_expression
	;

conditional_expression
	: binary_expression
	| binary_expression '?' expression ':' conditional_expression
	;

/*
 *	Assignment expression is used as part of the initializer
 */

assignment_expression
	: conditional_expression
	| unary_expression '=' assignment_expression
	| unary_expression MUL_ASSIGN assignment_expression
	| unary_expression DIV_ASSIGN assignment_expression
	| unary_expression MOD_ASSIGN assignment_expression
	| unary_expression ADD_ASSIGN assignment_expression
	| unary_expression SUB_ASSIGN assignment_expression
	| unary_expression LEFT_ASSIGN assignment_expression
	| unary_expression RIGHT_ASSIGN assignment_expression
	| unary_expression AND_ASSIGN assignment_expression
	| unary_expression XOR_ASSIGN assignment_expression
	| unary_expression OR_ASSIGN assignment_expression
	;

/*
 *	Expression is the catch-all for where we use expressions
 */

expression
	: assignment_expression
	| expression ',' assignment_expression
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
 *	Abstract declarations
 */

abstract_declaration
	: type_part abstract_declarator
	| type_part
	;

abstract_declarator
	: '*'
	| '*' abstract_declarator
	| direct_abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' constant_expression ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' constant_expression ']'
	;

/*
 *	Structure declarations. Note we separate out structure_name as a separate
 *	rule to force a reduction prior to parsing the contents of the structure
 *	list. This allows us to push the variable pool state with the appropriate
 *	scope.
 */

structure_name
	: STRUCT IDENTIFIER
	;

structure_declaration
	: structure_name '{' structure_declaration_list '}'
	;

structure_declaration_list
	: structure_item_declaration
	| structure_declaration_list structure_item_declaration
	;

structure_item_declaration
	: type_part struct_item_declaration_list ';'
	;

struct_item_declaration_list
	: declarator
	| struct_item_declaration_list ',' declarator
	;

/*
 *	Declaration specifiers.
 *
 *		Use simplified declarations. See documentation for overview
 */

declarator
	: '*' declarator
	| direct_declarator
	;

direct_declarator
	: IDENTIFIER
	| '(' declarator ')'
	| direct_declarator '[' constant_expression ']'
	| direct_declarator '[' ']'
	;

initializer_list
	: initializer_list ',' initializer
	| initializer
	;

initializer
	: assignment_expression
	| '{' initializer_list '}'
	;

init_declarator
	: declarator
	| declarator '=' initializer
	;

init_declarator_list
	: init_declarator_list ',' init_declarator
	| init_declarator
	;

type_part
	: UNSIGNED BYTE
	| BYTE
	| UNSIGNED SHORT
	| SHORT
	| UNSIGNED INT
	| INT
	| UNSIGNED LONG
	| LONG
	| FLOAT
	| DOUBLE
	| VOID
	| TYPE_NAME					// name of structure or class
	;

declaration
	: type_part init_declarator_list ';'
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
	: IDENTIFIER ':'
	;

switch_statement
	: CASE constant_expression ':'
	| DEFAULT ':'
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
	| compound_statement		// compound statements
	| labeled_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

compound_start			// Force push of state for compound statements
	: '{'
	;

compound_statement
	: '{' '}'
	| compound_start statement_list '}'		// Modification: inline declarations
	;


/*
 *	Our language is flat; we don't have functions. (Our function calls are
 *	all to pre-defined built-ins beyond the scope of this grammar.)
 */

statement_list
	: statement_list statement
	| statement
	;

/*
 *	The translation unit. Our language is flat; we are simply a list of 
 *	statements without founctions. (Our function calls are to predefined
 *	built-ins beyond the scope of this parser)
 */

translation_unit
	: statement_list
	;
