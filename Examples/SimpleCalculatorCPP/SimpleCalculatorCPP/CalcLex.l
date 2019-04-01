/*	CalcLex.l
 *
 *		Defines a very simple lexical analyzer for our calculator language
 */

/* Load our CalcParser.h header for our token definitions */
%{
	#include "CalcParser.h"
%}

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
