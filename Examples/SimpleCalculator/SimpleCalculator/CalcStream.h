//
//  CalcStream.h
//  SimpleCalculator
//
//  Created by William Woody on 3/31/19.
//  Copyright © 2019 Glenview Software. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CalcLex.h"

@interface CalcStream : NSObject <OCFileInput>

- (instancetype)initWithString:(NSString *)str;

@end
