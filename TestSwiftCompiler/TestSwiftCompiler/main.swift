//
//  main.swift
//  TestSwiftCompiler
//
//  Created by William Woody on 4/8/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

import Foundation

let stream = CalcStream.init(string: "11 + 2 * 3 - 4 / (1 + 1)")
let lex = CalcLex.init(stream: stream)
let parser = CalcParser.init(lexer: lex)

if (parser.parse()) {
	print("Answer: ", parser.result ?? 0)
} else {
	print("Error.")
}
