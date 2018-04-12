D						[0-9]
L						[a-zA-Z_]
H						[a-fA-F0-9]
E						[Ee][+-]?{D}+
FS						(f|F|l|L)
IS						(u|U|l|L)*

%{
/* header */
#define ABSTRACT                                      0x00110002
#define ADD_ASSIGN                                    0x00110003
#define AND_ASSIGN                                    0x00110004
#define AND_OP                                        0x00110005
#define BREAK                                         0x00110006
#define BYTE                                          0x00110007
#define CASE                                          0x00110008
#define CHAR_LITERAL                                  0x00110009
#define CLASS                                         0x0011000a
#define CONTINUE                                      0x0011000b
#define DEC_OP                                        0x0011000c
#define DEFAULT                                       0x0011000d
#define DELETE                                        0x0011000e
#define DIV_ASSIGN                                    0x0011000f
#define DO                                            0x00110010
#define DOUBLE                                        0x00110011
#define ELSE                                          0x00110012
#define EQ_OP                                         0x00110013
#define FLOAT                                         0x00110014
#define FOR                                           0x00110015
#define FUNCCALL                                      0x00110016
#define GE_OP                                         0x00110017
#define GOSUB                                         0x00110018
#define GOTO                                          0x00110019
#define IDENTIFIER                                    0x0011001a
#define IF                                            0x0011001b
#define IFSTATEMENT                                   0x0011001c
#define INC_OP                                        0x0011001d
#define INT                                           0x0011001e
#define INTEGER_CONSTANT                              0x0011001f
#define LEFT_ASSIGN                                   0x00110020
#define LEFT_OP                                       0x00110021
#define LE_OP                                         0x00110022
#define LONG                                          0x00110023
#define MOD_ASSIGN                                    0x00110024
#define MUL_ASSIGN                                    0x00110025
#define NEW                                           0x00110026
#define NE_OP                                         0x00110027
#define OR_ASSIGN                                     0x00110028
#define OR_OP                                         0x00110029
#define POSTDEC                                       0x0011002a
#define POSTINC                                       0x0011002b
#define PREDEC                                        0x0011002c
#define PREFIXOP                                      0x0011002d
#define PREINC                                        0x0011002e
#define PTR_OP                                        0x0011002f
#define REAL_CONSTANT                                 0x00110030
#define RETURN                                        0x00110031
#define RIGHT_ASSIGN                                  0x00110032
#define RIGHT_OP                                      0x00110033
#define SHORT                                         0x00110034
#define SIZEOF                                        0x00110035
#define STRING_LITERAL                                0x00110036
#define STRUCT                                        0x00110037
#define SUBSCRIPT                                     0x00110038
#define SUB_ASSIGN                                    0x00110039
#define SWITCH                                        0x0011003a
#define TERNOP                                        0x0011003b
#define TYPE_NAME                                     0x0011003c
#define UNSIGNED                                      0x0011003d
#define VOID                                          0x0011003e
#define WHILE                                         0x0011003f
#define XOR_ASSIGN                                    0x00110040
%}

%header {
	/* %header */
}

%global {
	/* %global */
}

%local {
	/* %local */
	int checkType();
	void skipComment();
}

%init {
	/* %init */
}

%%
"/*"					{ skipComment(); }
"//"[^\n]*\n			{ /* Skip C++ style comment */ }

"abstract"				{ return ABSTRACT; }
"break"					{ return BREAK; }
"byte"					{ return BYTE; }
"case"					{ return CASE; }
"class"					{ return CLASS; }
"continue"				{ return CONTINUE; }
"default"				{ return DEFAULT; }
"delete"				{ return DELETE; }
"do"					{ return DO; }
"double"				{ return DOUBLE; }
"else"					{ return ELSE; }
"float"					{ return FLOAT; }
"for"					{ return FOR; }
"goto"					{ return GOTO; }
"gosub"					{ return GOSUB; }
"if"					{ return IF; }
"int"					{ return INT; }
"new"					{ return NEW; }
"long"					{ return LONG; }
"return"				{ return RETURN; }
"short"					{ return SHORT; }
"sizeof"				{ return SIZEOF; }
"struct"				{ return STRUCT; }
"switch"				{ return SWITCH; }
"unsigned"				{ return UNSIGNED; }
"void"					{ return VOID; }
"while"					{ return WHILE; }

{L}({L}|{D})*			{ return checkType(); }

0[xX]{H}+{IS}?			{ return INTEGER_CONSTANT; }
0{D}+{IS}?				{ return INTEGER_CONSTANT; }
{D}+{IS}?				{ return INTEGER_CONSTANT; }
'(\\.|[^\\'])+'			{ return CHAR_LITERAL; }

{D}+{E}{FS}?			{ return REAL_CONSTANT; }
{D}*"."{D}+({E})?{FS}?	{ return REAL_CONSTANT; }
{D}+"."{D}*({E})?{FS}?	{ return REAL_CONSTANT; }

\"(\\.|[^\\"])*\"		{ return STRING_LITERAL; }

">>="					{ return RIGHT_ASSIGN; }
"<<="					{ return LEFT_ASSIGN; }
"+="					{ return ADD_ASSIGN; }
"-="					{ return SUB_ASSIGN; }
"*="					{ return MUL_ASSIGN; }
"/="					{ return DIV_ASSIGN; }
"%="					{ return MOD_ASSIGN; }
"&="					{ return AND_ASSIGN; }
"^="					{ return XOR_ASSIGN; }
"|="					{ return OR_ASSIGN; }
">>"					{ return RIGHT_OP; }
"<<"					{ return LEFT_OP; }
"++"					{ return INC_OP; }
"--"					{ return DEC_OP; }
"->"					{ return PTR_OP; }
"&&"					{ return AND_OP; }
"||"					{ return OR_OP; }
"<="					{ return LE_OP; }
">="					{ return GE_OP; }
"=="					{ return EQ_OP; }
"!="					{ return NE_OP; }

[ \t\v\n\f]				{ /* Skip whitespace */ }
.						{ return fText[0]; }

%%

/* code */

int OCLexTest::checkType()
{
	// Determine if this is a type
	return IDENTIFIER;
}

void OCLexTest::skipComment()
{
	int ch;

	while (-1 != (ch = input())) {
		if (ch == '*') {
			mark();
			ch = input();
			if (ch == '/') return;
			reset();
		}
	}
}

