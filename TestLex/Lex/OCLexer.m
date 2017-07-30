//
//  OCLexer.m
//  TestLex
//
//  Created by William Woody on 7/30/17.
//  Copyright © 2017 Glenview Software. All rights reserved.
//

#import "OCLexer.h"

/* ### INSERT HEADERS HERE */


/* ### INSERT HERE */

/************************************************************************/
/*                                                                      */
/*  Lex Transition State Tables                                         */
/*                                                                      */
/************************************************************************/

/*
 *  Constants
 */

#define MAXSTATES       9
#define MAXCHARCLASS    6
#define MAXACTIONS      4


//  Character classes used during parsing:
//
//  0: x
//  1: [G-Zg-wyz]
//  2: 0
//  3: [1-9]
//  4: [A-Fa-f]
//  5: [\000-/:-@\[-`{-\377]

/*  CharClass
 *
 *      Maps 8-bit character to character class
 */

static uint16_t CharClass[256] = {
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      2,   3,   3,   3,   3,   3,   3,   3, 
      3,   3,   5,   5,   5,   5,   5,   5, 
      5,   4,   4,   4,   4,   4,   4,   1, 
      1,   1,   1,   1,   1,   1,   1,   1, 
      1,   1,   1,   1,   1,   1,   1,   1, 
      1,   1,   1,   5,   5,   5,   5,   5, 
      5,   4,   4,   4,   4,   4,   4,   1, 
      1,   1,   1,   1,   1,   1,   1,   1, 
      1,   1,   1,   1,   1,   1,   1,   1, 
      0,   1,   1,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5, 
      5,   5,   5,   5,   5,   5,   5,   5
};

/*  StateActions
 *
 *      Maps states to actions. MAXACTION if this is not a terminal
 */

static uint16_t StateActions[9] = {
      4,   0,   1,   0,   3,   0,   1,   4, 
      2
};

/*  StateMachine
 *
 *      Lex state machine. Each item indicates the transition from
 *  a character set and the state to a new state. The new state is
 *  MAXSTATES if the transition is illegal.
 *
 *      The index is charClass + state * MAXCHARCLASS
 */

static uint16_t StateMachine[54] = {
      2,   2,   3,   1,   2,   4,   9,   9, 
      5,   5,   9,   9,   6,   6,   6,   6, 
      6,   9,   7,   9,   5,   5,   9,   9, 
      9,   9,   9,   9,   9,   9,   9,   9, 
      5,   5,   9,   9,   6,   6,   6,   6, 
      6,   9,   9,   9,   8,   8,   8,   9, 
      9,   9,   8,   8,   8,   9
};

/*
 *	Internal storage
 */

@interface OCLexer ()
{
	// Read position support
	NSInteger curLine;
	NSInteger curColumn;

	// Mark location support
	NSInteger markLine;
	NSInteger markColumn;

	// Mark buffer storage
	BOOL isMarked;							// yes if we have mark set
	unsigned char *markBuffer;				// mark buffer
	NSInteger markSize;						// bytes stored in buffer
	NSInteger markAlloc;					// capacity of buffer

	// Read cache
	unsigned char *readBuffer;				// read cache buffer
	NSInteger readPos;						// Read position
	NSInteger readSize;						// size of data in read buffer
	NSInteger readAlloc;					// Capacity of read cache

	// Text read buffer
	unsigned char *textBuffer;				// text cache for reading buffer
	NSInteger textMarkSize;
	NSInteger textSize;
	NSInteger textAlloc;
}

@property (strong) id<OCFileInput> file;

@end

/*
 *	Class lexer
 */

@implementation OCLexer

/*
 *	Instantiate parser.
 */

- (instancetype)initWithStream:(id<OCFileInput>)file
{
	if (nil != (self = [super init])) {
		self.file = file;

		isMarked = NO;
		markSize = 0;
		markAlloc = 256;
		markBuffer = (unsigned char *)malloc(markAlloc);

		readPos = 0;
		readSize = 0;
		readAlloc = 256;
		readBuffer = (unsigned char *)malloc(readAlloc);

		textMarkSize = 0;
		textSize = 0;
		textAlloc = 256;
		textBuffer = (unsigned char *)malloc(textAlloc);
	}
	return self;
}

/*
 *	Free internal storage
 */

- (void)dealloc
{
	if (markBuffer) free(markBuffer);
	if (readBuffer) free(readBuffer);
}

/*
 *	Internal read backtrack support. We implement the internal methods
 *	for mark, unmark and reset to note the current file position. This is
 *	used by the lexer to unwind the read queue if we overread the number
 *	of characters during parsing
 *
 *	In essence, as we execute the DFA while we read characters, when we
 *	find an end state, we mark the stream. Then we continue to read until
 *	we reach an error--at which point we rewind back to the mark location.
 */

/*
 *	mark: note that we should cache characters being read so we can rewind
 *	to this location in the future
 */

- (void)mark
{
	/*
	 *	Store the current file location and enable marking. This has the side
	 *	effect of flushing any previous mark buffer
	 */

	markLine = curLine;
	markColumn = curColumn;
	textMarkSize = textSize;

	isMarked = YES;
	markSize = 0;
}

/*
 *	Reset: reset the buffer positions
 */

- (void)reset
{
	if (!isMarked) return;	// not marked, nothing to do.

	/*
	 *	Reset to the marked position. We maintain two mutable data objects,
	 *	one which stores the data we're reading from (if any), and one which
	 *	stores the marked data.
	 *
	 *	Reset does two things: it prepends the read buffer with the data we
	 *	stored during the mark phase, and we clear the mark flag.
	 */

	if (markSize + readSize - readPos > readAlloc) {
		/*
		 *	Resize to fit
		 */

		NSInteger toFit = markSize + readSize - readPos;
		toFit = (toFit + 255) & ~255;	// align to 256 byte boundary
		if (toFit < 0) toFit = 256;

		unsigned char *ptr = (unsigned char *)realloc(readBuffer, toFit);
		if (ptr == NULL) {
			[NSException raise:NSMallocException format:@"Out of memory"];
		}

		readBuffer = ptr;
		readAlloc = toFit;
	}

	/*
	 *	Move the contents of the read buffer to a location above where the
	 *	mark queue will go
	 */

	if (readPos < readSize) {
		memmove(markSize + readBuffer, readPos + readBuffer, readSize - readPos);
	}

	/*
	 *	Move the marked data
	 */

	if (markSize > 0) {
		memmove(readBuffer, markBuffer, markSize);
	}

	/*
	 *	Update the read buffer settings
	 */

	readSize = markSize + readSize - readPos;
	readPos = 0;

	/*
	 *	Clear the mark
	 */

	isMarked = NO;
	markSize = 0;

	/*
	 *	Update the file position
	 */

	curLine = markLine;
	curColumn = markColumn;
	textSize = textMarkSize;
}

/*
 *	Read input stream.
 */

- (int)input
{
	int ch;

	/*
	 *	Low level: read, if from marked buffer
	 */

	if (readPos < readSize) {
		ch = readBuffer[readPos++];
	} else {
		ch = [self.file readByte];

		if (ch == -1) return -1;		// At EOF; immediate return.
	}

	/*
	 *	Update file position
	 */

	if (ch == '\n') {
		curColumn = 0;
		++curLine;
	} else {
		++curColumn;
	}

	/*
	 *	Handle mark
	 */

	if (isMarked) {
		if (markSize >= markAlloc) {
			NSInteger toFit = (markAlloc + 256) & ~255;
			if (toFit < 256) toFit = 256;
			unsigned char *ptr = (unsigned char *)realloc(markBuffer, toFit);
			if (ptr == NULL) {
				[NSException raise:NSMallocException format:@"Out of memory"];
			}

			markBuffer = ptr;
			markAlloc = toFit;
		}
		markBuffer[markSize++] = (unsigned char)ch;
	}

	/*
	 *	Return read character.
	 */

	return ch;
}

/*
 *	Internal methods declared within the Lex file
 */

/* ### INSERT HERE */

/*
 *	Lex interpreter. This runs the state machine until we find something
 */

- (NSInteger)lex
{
	uint16_t state;
	uint16_t action = MAXACTIONS;

	self.abort = NULL;

	/*
	 *	Run until we hit EOF
	 */

	for (;;) {
		/*
		 *	Start running the DFA
		 */

		state = 0;
		textSize = 0;

		for (;;) {
			int ch = [self input];
			if (ch == -1) {
				/*
				 *	We've hit EOF. If there is no stored text, we assume
				 *	we're at the file EOF, so return EOF. Otherwise treat as
				 *	illegal state transition.
				 */

				if (textSize == 0) return -1;
				break;
			}

			/*
			 *	Attempt to transition to the next state
			 */

			uint16_t charClass = CharClass[ch];
			uint16_t newState = StateMachine[charClass + MAXCHARCLASS * state];
			if (newState >= MAXSTATES) {
				/* Illegal state transition */
				break;
			}

			/*
			 *	Store the character we read, and transition to the next
			 *	state
			 */

			state = newState;

			if (textSize >= textAlloc) {
				NSInteger toFit = (textAlloc + 256) & ~255;
				if (toFit < 256) toFit = 256;
				unsigned char *ptr = (unsigned char *)realloc(textBuffer, toFit);
				if (ptr == NULL) {
					[NSException raise:NSMallocException format:@"Out of memory"];
				}

				textBuffer = ptr;
				textAlloc = toFit;
			}
			textBuffer[textSize++] = (char)ch;

			/*
			 *	Note the current action if we have one
			 */

			uint16_t newAction = StateActions[state];
			if (newAction != MAXACTIONS) {
				action = newAction;			/* Note action */
				[self mark];				/* Mark location for rewind */
			}
		}

		/*
		 *	When we reach here we read a character that is illegal.
		 *	This means we rewind to the last successfully read character,
		 *	and execute the
		 */

		/*
		 *	If no action is set, we simply abort after setting an internal
		 *	error state. This should never happen in a well designed lexer
		 */

		if (action == MAXACTIONS) {
			self.abort = @"Illegal character sequence";
			return -1;
		}

		/*
		 *	Action is set, so we rewind.
		 */

		[self reset];
		if (textSize == 0) {
			self.abort = @"No characters read in sequence";
			return -1;
		}

		/*
		 *	Convert text sequence into string
		 */

		self.text = [[NSString alloc] initWithBytes:textBuffer length:textSize encoding:NSUTF8StringEncoding];

		/*
		 *	Execute action
		 */

		switch (action) {
			/* ### INSERT HERE */
            case 0:
                return 0x10001;
                break;

            case 1:
                return 0x10002;
                break;

            case 2:
                return 0x10003;
                break;

            case 3:
                return textBuffer[0];
                break;

			default:
				break;
		}
	}
}

@end
