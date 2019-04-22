//
//  CalcParser.swift
//  TestSwiftCompiler
//
//  Created by William Woody on 4/9/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

import Cocoa

/*
 *	%header/%{...%}
 */

protocol CalcParserError {
	func error(parser: CalcParser, line: Int, column: Int, filename: String?, errorCode: Int, data: [String: AnyObject]?)
}

class CalcParser {
	/*
	 *	Yacc Constants
	 */

	public static let NUMBER = 0x00110002

	/*
	 *	%global / %local
	 */

	public var result: NSNumber? = nil

	/*
	 *	Internal constants
	 */

	private static let K_ACCEPTSTATE: UInt16 = 13          // Final accept state
	private static let K_EOFTOKEN: Int       = 0x110000    // EOF token ID
	private static let K_ERRORTOKEN: Int     = 0x110001    // Error token ID
	private static let K_FIRSTTOKEN: Int     = 0x110002    // Error token ID
	private static let K_MAXSYMBOL: Int      = 0x110006    // Max ID for all symbols
	private static let K_STARTSTATE: UInt16  = 0           // Start state is always 0

	/*
	 *	%errors
	 */

	/*  TokenArray
	 *
	 *      Array of non-Unicode token values for error reporting
	 */

	private static let TokenArray: [String] = [
		"NUMBER"
	]


	/*  RuleLength
	 *
	 *      The number of tokens a reduce action removes from the stack
	 */

	private static let RuleLength: [UInt8] = [
	     1,  1,  1,  3,  3,  3,  3,  3
	]

	/*  RuleProduction
	 *
	 *      The prodution ID we reduce to
	 */

	private static let RuleProduction: [UInt32] = [
	    0x110003, 0x110004, 0x110005, 0x110005, 0x110005, 0x110005, 0x110005, 0x110005
	]

	/*
	 *  The following tables are compressed using CSR format.
	 *  See https://en.wikipedia.org/wiki/Sparse_matrix
	 */

	/*  ActionI, J, A
	 *
	 *      Compressed action index table.
	 */

	private static let ActionI:[UInt32] = [
			 0,      2,      4,      9,     10,     15,     17,     19,
			21,     23,     26,     31,     36,     39,     40,     42,
			47,     52,     57,     59,     61,     63,     65,     68,
			73,     78,     81,     86,     91
	]

	private static let ActionJ:[UInt32] = [
			0x28, 0x110002,     0x28, 0x110002,     0x2A,     0x2B,     0x2D,     0x2F,
		0x110000, 0x110000,     0x2A,     0x2B,     0x2D,     0x2F, 0x110000,     0x28,
		0x110002,     0x28, 0x110002,     0x28, 0x110002,     0x28, 0x110002,     0x2B,
			0x2D, 0x110000,     0x2A,     0x2B,     0x2D,     0x2F, 0x110000,     0x2A,
			0x2B,     0x2D,     0x2F, 0x110000,     0x2B,     0x2D, 0x110000, 0x110000,
			0x28, 0x110002,     0x29,     0x2A,     0x2B,     0x2D,     0x2F,     0x29,
			0x2A,     0x2B,     0x2D,     0x2F,     0x2A,     0x2B,     0x2D,     0x2F,
		0x110000,     0x28, 0x110002,     0x28, 0x110002,     0x28, 0x110002,     0x28,
		0x110002,     0x29,     0x2B,     0x2D,     0x29,     0x2A,     0x2B,     0x2D,
			0x2F,     0x29,     0x2A,     0x2B,     0x2D,     0x2F,     0x29,     0x2B,
			0x2D,     0x29,     0x2A,     0x2B,     0x2D,     0x2F,     0x29,     0x2A,
			0x2B,     0x2D,     0x2F
	]

	// Note: < 0 -> reduce (rule = -a-1), >= 0 -> shift (state).
	private static let ActionA:[Int16] = [
			 1,      2,     14,     15,     -3,     -3,     -3,     -3,
			-3,     13,      5,      6,      7,      8,     -2,      1,
			 2,      1,      2,      1,      2,      1,      2,     -7,
			-7,     -7,      5,      6,      7,      8,     -5,      5,
			 6,      7,      8,     -4,     -6,     -6,     -6,     -1,
			14,     15,     -3,     -3,     -3,     -3,     -3,     17,
			18,     19,     20,     21,     -8,     -8,     -8,     -8,
			-8,     14,     15,     14,     15,     14,     15,     14,
			15,     -7,     -7,     -7,     -5,     18,     19,     20,
			21,     -4,     18,     19,     20,     21,     -6,     -6,
			-6,     27,     18,     19,     20,     21,     -8,     -8,
			-8,     -8,     -8
	]

	/*  GotoI, J, A
	 *
	 *      Compressed goto table.
	 */

	private static let GotoI:[UInt32] = [
	         0,      2,      3,      3,      3,      3,      4,      5,
	         6,      7,      7,      7,      7,      7,      7,      8,
	         8,      8,      8,      9,     10,     11,     12,     12,
	        12,     12,     12,     12,     12
	]

	private static let GotoJ:[UInt32] = [
		0x110004, 0x110005, 0x110005, 0x110005, 0x110005, 0x110005, 0x110005, 0x110005,
		0x110005, 0x110005, 0x110005, 0x110005
	]

	private static let GotoA:[Int16] = [
			 3,      4,     16,     12,     11,     10,      9,     26,
			25,     24,     23,     22
	]


	/*
	 *	Error Values
	 */

	public static let ERRORMASK_WARNING = 0x8000
	public static let ERROR_SYNTAX = 0x0001
	public static let ERROR_MISSINGTOKEN = 0x0002
	public static let ERROR_MISSINGTOKENS = 0x0003
	public static let ERROR_STARTERRORID = 0x0100

	/*
	 *	Stack
	 */

	private struct Stack {
		var state: UInt16
		var line: Int
		var column: Int
		var filename: String?
		var value: AnyObject?

		init()
		{
			state = 0
			line = 0
			column = 0
			filename = nil
			value = nil
		}
	}

	/*
	 *	Internal values
	 */

	private var stack: [Stack]
	private var lex: OCLexInput

	// Error support
	private var success: Bool
	private var errorCount: Int

	private var hasError: Bool
	private var errorLine: Int
	private var errorColumn: Int
	private var errorFileName: String?

	// Delegate
	public var errorDelegate: CalcParserError?


	/*
	 *	Startup
	 */

	public init(lexer: OCLexInput)
	{
		lex = lexer

		stack = Array()
		success = true
		errorCount = 0;
		hasError = false
		errorLine = 0
		errorColumn = 0
		errorFileName = nil
		errorDelegate = nil

		/*
		 *	%init
		 */
	}

	deinit {
		/*
		 *	%finish
		 */
	}

	/*
	 *	Process production rule. This processes the production rule and creates
	 *	a new stack state with the rule reduction.
	 */

	private func processReduction(_ rule: Int16) -> CalcParser.Stack
	{
		// Get production len, for translating $1...$N into something useful
		let pos = stack.count - Int(CalcParser.RuleLength[Int(rule)])

		// Set up initial state.
		var s = CalcParser.Stack()

		// File position of reduced rule is first token of the symbols reduced
		let fs = stack[pos]
		s.filename = fs.filename;
		s.line = fs.line;
		s.column = fs.column;

		do {
			try reduction(rule: rule, at: pos, with: &s)
		} catch {
			// Ignore errors. Can happen when the stack is in an error state
		}

		return s
	}

	private func reduction(rule: Int16, at pos: Int, with s: inout CalcParser.Stack) throws
	{
		// Now process production.
		//
		// Note that $$ translated into (s.value), and
		// $n translates into ((<type> *)(self.stack[pos+(n-1)])), where <type>
		// is the declared type of the token or production rule.

		// Note: value is AnyObject, but we use as! cast to coerce, and assume
		// the exception is caught and ignored if it all goes south.
		switch rule {
			case 1:
				result = (stack[pos].value as! NSNumber)
			case 2:
				s.value = stack[pos].value
			case 3:
				s.value = NSNumber.init(value: (stack[pos].value as! NSNumber).intValue + (stack[pos+2].value as! NSNumber).intValue)
			case 4:
				s.value = NSNumber.init(value: (stack[pos].value as! NSNumber).intValue - (stack[pos+2].value as! NSNumber).intValue)
			case 5:
				s.value = NSNumber.init(value: (stack[pos].value as! NSNumber).intValue * (stack[pos+2].value as! NSNumber).intValue)
			case 6:
				s.value = NSNumber.init(value: (stack[pos].value as! NSNumber).intValue / (stack[pos+2].value as! NSNumber).intValue)
			case 7:
				s.value = (stack[pos+1].value as! NSNumber)
			default:
				break
		}
	}

	private func actionForState(_ state: UInt16, token t: Int) -> Int
	{
		var min,max,mid: Int

		let token = (t == -1) ? CalcParser.K_EOFTOKEN : t

		/* Find range */
		min = Int(CalcParser.ActionI[Int(state)])
		max = Int(CalcParser.ActionI[Int(state)+1])

		/* Binary search for value in ja */
		while min < max {
			mid = (min + max)/2
			let j = CalcParser.ActionJ[mid]
			if token == j {
				return Int(CalcParser.ActionA[mid])
			} else if token < j {
				max = mid
			} else {
				min = mid + 1
			}
		}
		return Int.max
	}

	/*
	 *	Look up the goto value for the state and token (production rule) provided.
	 *	Operates the same as actionForState, except we look in the goto table.
	 *	In teory I guess we could combine these two tables (as productions and
	 *	tokens do not overlap).
	 */

	private func goto(state: UInt16, production token: UInt32) -> Int
	{
		var min,max,mid: Int

		/* Find range */
		min = Int(CalcParser.GotoI[Int(state)])
		max = Int(CalcParser.GotoI[Int(state)+1])

		/* Binary search for value in ja */
		while min < max {
			mid = (min + max)/2
			let j = CalcParser.GotoJ[mid]
			if token == j {
				return Int(CalcParser.GotoA[mid])
			} else if token < j {
				max = mid
			} else {
				min = mid + 1
			}
		}
		return Int.max
	}

	/*
	 *	Errors. This formats and prints the specified error
	 */

	private func error(code: Int, data: [String: AnyObject])
	{
		if errorDelegate == nil {	// No error handling, ignore.
			return
		}
		if errorCount > 0 {			// skip until synced on 3 shifts
			return
		}

		// Call delegate with current token position
		// Token position is the topmost symbol

		if hasError {
			errorDelegate?.error(parser: self, line: errorLine, column: errorColumn, filename: errorFileName, errorCode: code, data: data)
		} else {
			if let top = stack.last {
				errorDelegate?.error(parser: self, line: top.line, column: top.column, filename: top.filename, errorCode: code, data: data)
			}
		}

		if 0 == (code & CalcParser.ERRORMASK_WARNING) {
			success = false
			errorCount = 3
		}
	}

	private func error(code: Int)
	{
		error(code: code, data: [:])
	}

	private func errorOK()
	{
		errorCount = 0
	}

	private func tokenToString(_ token: UInt32) -> String
	{
		if token >= CalcParser.K_FIRSTTOKEN {
			return CalcParser.TokenArray[Int(token) - CalcParser.K_FIRSTTOKEN]
		} else {
			return String.init(Unicode.Scalar(token) ?? Unicode.Scalar.init(UInt8(32)))
		}
	}

	private func reduce(action: Int16) -> Bool
	{
		// Determine the new state we're transitioning to.
		let production = CalcParser.RuleProduction[Int(action)]
		let length = CalcParser.RuleLength[Int(action)]

		// We pull the state we'd have after popping the stack.
		let statep = stack[stack.count - Int(length) - 1].state

		// Calculate the state we should transition to
		let newState = goto(state: statep, production: production)
		if newState == Int.max {
			return false
		}

		// Process reduction rule
		var state = processReduction(action)

		// Update state
		state.state = UInt16(newState)

		// Pop the stack
		stack.removeSubrange(stack.count - Int(length) ..< stack.count)

		// Push new state
		stack.append(state)

		// Clear error marker
		hasError = false

		// Return success
		return true
	}


//%%

	/*
	 *	Parser engine. Returns NO if there was an error during processing. Note
	 *	that as we uncover errors we call our delegate for error handling. This
	 *	implements the algorithm described in the Dragon Book, Algorithm 4.7.
	 */

	public func parse() -> Bool
	{
		var s: CalcParser.Stack
		var a: Int

		/*
		 *	Step 1: reset and push the empty state.
		 */

		success = true
		stack = []

		hasError = false
		errorCount = 0

		s = CalcParser.Stack()
		s.state = CalcParser.K_STARTSTATE
		s.filename = lex.filename
		s.line = lex.line
		s.column = lex.column

		stack.append(s)

		/*
		 *	Now repeat forever:
		 */

		a = lex.lex()
		while true {
			s = stack.last!

			/*
			 *	Determine if this is the end state. If so, then we immediately
			 *	quit. We assume the user has set the production rule at the
			 *	top, so we can simply drop the stack
			 */

			if s.state == CalcParser.K_ACCEPTSTATE {
				stack.removeAll()
				return success
			}

			/*
			 *	Now determine the action and shift, reduce or handle error as
			 *	appropriate
			 */

			var action = actionForState(s.state, token: a)
			var foundError = false

			if action == Int.max {
				/*
				 *	Handle error. First, note we have an error, and note the
				 *	symbol on which our error took place.
				 */

				success = false
				errorFileName = lex.filename
				errorLine = lex.line
				errorColumn = lex.column
				hasError = true

				/*
				 *	First, scan backwards from the current state, looking for one
				 *	which has an 'error' symbol.
				 */

				var ix = stack.count
				while (ix > 0) && !foundError {
					ix -= 1
					let si = stack[ix]
					action = actionForState(si.state, token: CalcParser.K_ERRORTOKEN)
					if (action >= 0) && (action != Int.max) {

						/*
						 *	Encountered error state. If the user has defined an
						 *	error token, we ultimately will want to (a) unwind
						 *	the stack until we find a state which handles the
						 *	error transition. We then .
						 */

						if ix+1 < stack.count {
							stack.removeSubrange(ix+1..<stack.count)
						}

						/*
						 *	At this point we perform a shift to our new error
						 *	state.
						 */

						var s = CalcParser.Stack()
						s.state = UInt16(action)
						s.value = lex.value

						s.filename = lex.filename
						s.line = lex.line
						s.column = lex.column

						stack.append(s)

						/*
						 *	Second, we start pulling symbols until we find a symbol
						 *	that shifts, or until we hit the end of file symbol.
						 *	This becomes our current token for parsing
						 */

						while (true) {
							a = lex.lex()
							action = actionForState(s.state, token: a)
							if (action >= 0) && (action != Int.max) {
								/*
								 *	Valid shift. This becomes our current token,
								 *	and we resume processing.
								 */

								foundError = true
								break
							} else if (a == CalcParser.K_EOFTOKEN) || (a == -1) {
								/*
								 *	We ran out of tokens. At this point all
								 *	we can do is print an error and force quit
								 */

								error(code: CalcParser.ERROR_SYNTAX)
								stack.removeAll()
								return false
							}
						}
					}
				}
				if foundError {
					continue
				}

				/*
				 *	If we reach this point, there is no error we can recover to.
				 *	So figure this out on our own.
				 *
				 *	First, we see if the state we're in has a limited number of
				 *	choices. For example, in C, the 'for' keyword will always be
				 *	followed by a '(' token, so we can offer to automatically
				 *	insert that token.
				 */

				let actionMin: Int = Int(CalcParser.ActionI[Int(s.state)])
				let actionMax: Int = Int(CalcParser.ActionI[Int(s.state) + 1])
				var actionVal: Int = actionMin
				var actionState: Int16 = -1

				for ix in actionMin..<actionMax {
					let act = CalcParser.ActionA[ix]
					if actionState == -1 {
						if act >= 0 {
							actionState = act;
							actionVal = ix
						}
					} else {
						actionState = -1
						break
					}
				}

				if actionState != -1 {
					/*
					 *	We can accomplish this transition with one token. Print
					 *	an error, and do a shift on the state with an empty value.
					 */

					let tokenStr = tokenToString(CalcParser.ActionJ[actionVal])
					error(code: CalcParser.ERROR_MISSINGTOKEN, data: ["token": tokenStr as AnyObject])


					/*
					 *	Perform a shift but do not pull a new token
					 */

					let top = stack.last!

					var s = CalcParser.Stack()

					s.state = UInt16(actionState)
					s.value = lex.value

					s.filename = top.filename
					s.line = top.line
					s.column = top.column

					stack.append(s)
					continue
				}

				/*
				 *	See if we have a limited choice in reductions. If this can
				 *	only reduce to a single state, try that reduction.
				 */

				actionState = 0
				for ix in actionMin..<actionMax {
					let act = CalcParser.ActionA[ix]
					if actionState == 0 {
						if (act < 0) && (actionState != act) {
							actionState = act;
						}
					} else {
						actionState = 0
						break
					}
				}

				if actionState != 0 {
					/*
					 *	We have one possible reduction. Try that. Note that this
					 *	will trigger a syntax error since we're reducing down
					 *	without the follow token. My hope is that the state we
					 *	transition to has a limited set of next tokens to follow.
					 */

					_ = reduce(action: Int16(action))
					continue;
				}

				/*
				 *	If we have a limited number of tokens which can follow,
				 *	print a list of them. Then shift by the first one we
				 *	find. We don't do this if the number of shifts is greater
				 *	than five.
				 */

				if actionMax - actionMin <= 5 {
					var list: [String] = []
					for ix in actionMin..<actionMax {
						list.append(tokenToString(CalcParser.ActionJ[ix]))
					}

					error(code: CalcParser.ERROR_MISSINGTOKENS, data: ["tokens": list as AnyObject])

					/*
					 *	Now we artificially insert the first of the list of
					 *	tokens as our action and continue.
					 */

					a = Int(CalcParser.ActionJ[actionMin])
					continue;
				}

				/*
				 *	If we get here, things just went too far south. So we
				 *	skip a token, print syntax error and move on
				 */

				error(code:CalcParser.ERROR_SYNTAX)
				a = lex.lex()
				if a == -1 {
					return false
				}

			} else if action >= 0 {
				/*
				 *	Shift operation.
				 */

				// Shift
				var s = CalcParser.Stack()

				s.state = UInt16(action)
				s.value = lex.value

				s.filename = lex.filename
				s.line = lex.line
				s.column = lex.column

				stack.append(s)

				// Advance to next token.
				a = lex.lex()

				// Decrement our error count. If this is non-zero we're in an
				// error state, and we don't pass spurrous errors upwards
				if errorCount > 0 {
					errorCount -= 1
				}
			} else {
				/*
				 *	Reduce action. (Reduce is < 0, and the production to reduce
				 *	by is given below
				 */

				action = -action-1

				if !reduce(action: Int16(action)) {

					// If there is an error, this handles the error.
					// (This should not happen in practice).
					error(code:CalcParser.ERROR_SYNTAX)

					// Advance to next token.
					a = lex.lex()
				}
			}
		}
	}

}
