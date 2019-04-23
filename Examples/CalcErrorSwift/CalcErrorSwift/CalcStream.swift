//
//  CalcStream.swift
//  CalcErrorSwift
//
//  Created by William Woody on 4/22/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

import Cocoa

class CalcStream : OCFileInput
{
	/*
	 *	Dear Apple: Really? You had what, a dozen different random
	 *	representations of a string?
	 *
	 *	So we pick UTF-8 because that's what our lexer is capable
	 *	of dealing with. In the future I may extend this to handle
	 *	full unicode, but that would require altering the CharClass
	 *	array to be a searchable system.
	 */

	private var string: [String.UTF8View.Element]
	private var pos: Int

	init(string s: String)
	{
		pos = 0
		string = Array(s.utf8)
	}

	func readByte() -> Int
	{
		if pos >= string.count {
			return -1
		} else {
			let ret = Int(string[pos])
			pos += 1
			return ret
		}
	}

	func peekByte() -> Int
	{
		if pos >= string.count {
			return -1
		} else {
			return Int(string[pos])
		}
	}
}
