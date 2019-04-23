//  OCLexInput.swift
//
//  OCTools support source file
//
//  (C) 2019 William Woody, All Rights Reserved

import Foundation

/*	OCLexInput
 *
 *		The protocol for our lex reader file that the lex stream must
 *	provide. This is the same as the protocol generated as part of the OCYacc
 *	output, and allows us to glue the Lexer and Parser together.
 */

protocol OCLexInput {
	var line: Int { get }
	var column: Int { get }
	var filename: String? { get }
	var text: String { get }
	var abort: String? { get }
	var value: AnyObject? { get }

	func lex() -> Int
}


