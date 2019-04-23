/*	CalcLex.l
 *
 *		Defines a very simple lexical analyzer for our calculator language
 */

/* Simplify our definitions */

D		[0-9]

%%

{D}+		{
				/* Number */
				value = NSNumber.init(value:Int(text) ?? 0)
				return CalcParser.NUMBER		// CalcLex.NUMBER???
			}

[A-Za-z_][A-Za-z0-9_]* {
				return CalcParser.IDENTIFIER;
			}

[ \t\v\n\f]	{
				/* Skip whitespace */
			}

.			{
				/* All else, return character value as token ID. */
				return Int(text.unicodeScalars.first?.value ?? 0)
			}
