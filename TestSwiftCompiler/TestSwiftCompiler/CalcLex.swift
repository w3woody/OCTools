//
//  CalcLex.swift
//  TestSwiftCompiler
//
//  Created by William Woody on 4/8/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

import Cocoa

class CalcLex: OCLexInput {
	/*
	 *	Internal generated state
	 */

	let MAXSTATES: UInt16 = 5
	let MAXCHARCLASS: UInt16 = 3
	let MAXACTIONS: UInt16 = 3

	/*  CharClass
	 *
	 *      Maps 8-bit character to character class
	 */

	static let CharClass: [UInt16] = [
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   1,   1,   1,   1,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  1,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2,
		  2,   2,   2,   2,   2,   2,   2,   2
	]

	/*  StateActions
	 *
	 *      Maps states to actions. MAXACTION if this is not a terminal
	 */

	static let StateActions: [UInt16] = [
		  3,   0,   1,   2,   0
	]

	/*  StateMachineIA, StateMachineJA, StateMachineA
	 *
	 *      Lex state machine in compressed sparce row storage format. We do this
	 *  in order to compact the resulting sparse matrix state machine so we don't
	 *  consume as much space. Decoding the new state becomes an O(log(N)) process
	 *  on the input character class as we use a binary search on the JA array.
	 *
	 *      See the article below for more information:
	 *
	 *      https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_.28CSR.2C_CRS_or_Yale_format.29
	 */

	static let StateMachineIA: [UInt16] = [
		  0,   3,   4,   4,   4,   5
	];

	static let StateMachineJA: [UInt16] = [
		  0,   1,   2,   0,   0
	];

	static let StateMachineA: [UInt16] = [
		  1,   2,   3,   4,   4
	];


	/*
	 *	File status
	 */

	public var line: Int
	public var column: Int
	public var filename: String?
	public var text: String
	public var abort: String?
	public var value: AnyObject?

	/*
	 *	State
	 */

	private var file: OCFileInput


	/*
	 *	Internal status
	 */

	private var curLine: Int
	private var curColumn: Int

	// Mark location support
	private var markLine: Int
	private var markColumn: Int

	// Mark buffer storage
	private var isMarked: Bool
	private var markBuffer: NSMutableString

	// Read cache
	private var readPos: Int
	private var readBuffer: NSMutableString

	// Text read buffer
	private var textMarkSize: Int
	private var textBuffer: NSMutableString

	// State flags
	private var states: UInt64

	/*
	 *	Initialize our parser with the input buffer
	 */

	public init(stream f: OCFileInput)
	{
		file = f

		// Zero public values
		line = 0
		column = 0
		filename = nil
		text = String()
		abort = nil
		value = nil

		// Zero private
		isMarked = false
		markBuffer = NSMutableString()

		readPos = 0
		readBuffer = NSMutableString()

		textMarkSize = 0
		textBuffer = NSMutableString()

		curLine = 0
		curColumn = 0
		markLine = 0
		markColumn = 0

		states = 0

		// %init
	}

	deinit {
		// %finish
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
	 *		Mark: note that we should cache characters being read so we can rewind
	 *	to this location in the future
	 */

	private func mark()
	{
		markLine = curLine
		markColumn = curColumn
		textMarkSize = textBuffer.length

		isMarked = true
		markBuffer.deleteCharacters(in: NSMakeRange(0, markBuffer.length))
	}

	/*
	 *	Reset: reset the buffer positions
	 */

	private func reset()
	{
		if !isMarked {
			// not marked, nothing to do.
			return
		}

		/*
		 *	Clear the part that was already read
		 */

		if (readPos < readBuffer.length) {
			readBuffer.deleteCharacters(in: NSMakeRange(0,readPos))
		} else {
			readBuffer.deleteCharacters(in: NSMakeRange(0,readBuffer.length))
		}

		/*
		 *	Now moved the marked stuff
		 */

		if (markBuffer.length > 0) {
			readBuffer.insert(markBuffer as String, at: 0)
		}

		/*
		 *	Clear the mark, reset positions
		 */

		readPos = 0
		isMarked = false
		markBuffer.deleteCharacters(in: NSMakeRange(0, markBuffer.length))

		/*
		 *	Update the file position
		 */

		curLine = markLine
		curColumn = markColumn

		/*
		 *	Remove the read text back to the marked location
		 */

		if textMarkSize < textBuffer.length {
			textBuffer.deleteCharacters(in: NSMakeRange(textMarkSize,textBuffer.length - textMarkSize))
		}
	}

	/*
	 *	Read a single character or -1 if at EOF
	 */

	private func input() -> Int
	{
		var ch: Int

		/*
		 *	Read from marked buffer
		 */

		if readPos < readBuffer.length {
			ch = Int(readBuffer.character(at: readPos))
			readPos += 1
		} else {
			ch = file.readByte()
			if ch == -1 {
				return -1
			}
		}

		/*
		 *	Update file position
		 */

		if ch == 0x0A {			// '\n'
			curColumn = 0
			curLine += 1
		} else {
			curColumn += 1
		}

		/*
		 *	Handle mark
		 */

		if isMarked {
			markBuffer.appendFormat("%c", ch)
		}

		/*
		 *	Done.
		 */

		return ch
	}

	/*
	 *	EOL testing
	 */

	private func atEOL() -> Bool
	{
		var ch: Int

		if readPos < readBuffer.length {
			ch = Int(readBuffer.character(at: readPos))
		} else {
			ch = file.peekByte()
		}

		if (ch == -1) || (ch == 0x0A) {
			return true
		} else {
			return false
		}
	}

	private func atSOL() -> Bool
	{
		return column == 0
	}

	/*
	 *	Read the state for the class/state combination. Decodes the sparce
	 *	matrix that is compressed in StateMachineIA/JA/A above. If the
	 *	entry is not found, returns MAXSTATES. This is the same as the lookup
	 *	StateMachine[class][state] if the StateMachine sparse array was
	 *	unrolled
	 */

	private func stateForClass(_ charClass: UInt16, state: UInt16) -> UInt16
	{
		var min, max, mid: UInt16

		/* Find range */
		min = CalcLex.StateMachineIA[Int(state)]
		max = CalcLex.StateMachineIA[Int(state)+1]

		while min < max {
			mid = (min + max) / 2;
			let j = CalcLex.StateMachineJA[Int(mid)]
			if charClass == j {
				return CalcLex.StateMachineA[Int(mid)]
			} else if charClass < j {
				max = mid
			} else {
				min = mid + 1
			}
		}

		return MAXSTATES
	}

	public func set(file: String?, line l: Int)
	{
		if file != nil {
			filename = file
		}
		line = l
	}

	public func set(line l: Int)
	{
		line = l
	}

	/*
	 *	Internal methods declared within the Lex file
	 */

	// %{..%}

	/*
	 *  For conditional states this takes an end DFA state and
	 *  determines the proper end rule given the current start
	 *  conditionals.
	 */

	private func conditionalAction(_ state: UInt16) -> UInt16
	{
		switch state {
			default:
				return MAXACTIONS
		}
	}



	public func lex() -> Int
	{
		var state: UInt16
		var action: UInt16 = MAXACTIONS

		abort = nil
		value = nil
		text = ""

		/*
		 *	Run until we hit EOF or a production rule triggers a return
		 */

		while true {
			/*
			 *	Start running the DFA
			 */

			state = 0
			textBuffer.deleteCharacters(in: NSMakeRange(0, textBuffer.length))
			line = curLine
			column = curColumn

			while true {
				let ch = input()
				if ch == -1 {
					/*
					 *	We've hit EOF. If there is no stored text, we assume
					 *	we're at the file EOF, so return EOF. Otherwise treat as
					 *	illegal state transition.
					 */

					if textBuffer.length == 0 {
						return -1
					}
					break
				}

				/*
				 *	Attempt to translate to the next state
				 */

				let charClass = CalcLex.CharClass[Int(ch)]
				let newState = stateForClass(charClass, state: state)
				if newState >= MAXSTATES {
					/* Illegal state translation */
					break
				}

				/*
				 *	Store the character we read, and transition to the next
				 *	state
				 */

				state = newState
				textBuffer.appendFormat("%c", ch)

				/*
				 *	Note the current action if we have one
				 */

				var newAction = CalcLex.StateActions[Int(state)]
				if newAction > MAXACTIONS {
					newAction = conditionalAction(newAction)
				}
				if newAction != MAXACTIONS {
					action = newAction
					mark()
				}
			}

			/*
			 *	If no action is set, we simply abort after setting an internal
			 *	error state. This should never happen in a well designed lexer
			 */

			if action == MAXACTIONS {
				abort = "Illegal character sequence"
				return -1
			}

			/*
			 *	Action is set, so we rewind.
			 */

			reset()
			if textBuffer.length == 0 {
				abort = "No characters read in sequence"
				return -1
			}

			/*
			 *	Convert text sequence into string
			 */

			text = textBuffer as String
			value = textBuffer

			/*
			 *	Execute action
			 */

			switch action {
				case 0:
					value = NSNumber.init(value:Int(text) ?? 0)
					return CalcParser.NUMBER		// CalcLex.NUMBER???

				case 2:
					return Int(text.unicodeScalars.first?.value ?? 0)

				default:
					break;
			}
		}
	}
}
