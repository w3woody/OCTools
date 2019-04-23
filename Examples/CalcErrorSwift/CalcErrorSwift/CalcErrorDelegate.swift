//
//  CalcErrorDelegate.swift
//  CalcErrorSwift
//
//  Created by William Woody on 4/22/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

import Cocoa

class CalcErrorDelegate: CalcParserError {
	func error(parser: CalcParser, line: Int, column: Int, filename: String?, errorCode: Int, data: [String : AnyObject]?) {

		// Report error
		print("Error: ", errorCode, " at ", line, ":", column)
	}
}

