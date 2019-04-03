/*	CalcLex.l
 *
 *		Defines a very simple lexical analyzer for our calculator language
 */

/* Load our CalcParser.h header for our token definitions */
%header {
#define NUMBER		0x10001
}

/* Simplify our definitions */

D		[0-9]

%%

{D}+		{
				/* Number */
				self.value = @( self.text.intValue );
				return NUMBER;
			}

[ \t\v\n\f]	{
				/* Skip whitespace */
			}

.			{
				/* All else, return character value as token ID. */
				return [self.text characterAtIndex:0];
			}
