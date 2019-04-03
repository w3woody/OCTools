/*	CalcLex.l
 *
 *		Defines a very simple lexical analyzer for our calculator language
 */

%header {
#define NUMBER		0x10001
}

%union {
	int integer;
}

/* Simplify our definitions */

D		[0-9]

%%

{D}+		{
				/* Number */
				value.integer = atoi(text.c_str());
				return NUMBER;
			}

[ \t\v\n\f]	{
				/* Skip whitespace */
			}

.			{
				/* All else, return character value as token ID. */
				return text[0];
			}
