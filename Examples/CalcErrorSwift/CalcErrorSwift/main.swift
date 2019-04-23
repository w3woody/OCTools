//
//  main.swift
//  CalcErrorSwift
//
//  Created by William Woody on 4/22/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

import Foundation

let stream = CalcStream.init(string: "11 + 2 * 3 - 4 / (1 + 1);")
let lex = CalcLex.init(stream: stream)
let parser = CalcParser.init(lexer: lex)
let error = CalcErrorDelegate.init()
parser.errorDelegate = error

if (parser.parse()) {
	print("Answer: ", parser.result)
} else {
	print("Error.")
}

