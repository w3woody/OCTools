//  OCFileInput.swift
//
//  OCTools support source file
//
//  (C) 2019 William Woody, All Rights Reserved

import Foundation

/*	OCFileInput
 *
 *		The input file stream must correspond to this protocol to read
 *	the contents of a file or data object. The byte returned is from 0 to 255,
 *	and EOF is marked with -1.
 */

protocol OCFileInput {
	func readByte() -> Int
	func peekByte() -> Int
}

