D			[0-9]
E			[Ee][+-]?{D}+

%local {
@property (strong) NSMutableArray<NSNumber *> *stack;
}

%%

{D}*({D}|\.{D}*)({E})?	{ [self pushNumber]; }
[+\-*/]					{ [self doOperator]; }
\n						{ return [self stackValue]; }
.						{ /* Ignore everything else */ }

%%

- (void)pushNumber
{
	if (self.stack == nil) self.stack = [[NSMutableArray alloc] init];
	[self.stack addObject:@( self.text.doubleValue )];
}

- (void)doOperator
{
	if (self.stack.count < 2) {
		NSLog(@"Error in operation");
	} else {
		NSNumber *b = self.stack.lastObject;
		[self.stack removeLastObject];
		NSNumber *a = self.stack.lastObject;
		[self.stack removeLastObject];

		if ([self.text isEqualToString:@"+"]) {
			a = @( a.doubleValue + b.doubleValue );
		} else if ([self.text isEqualToString:@"-"]) {
			a = @( a.doubleValue - b.doubleValue );
		} else if ([self.text isEqualToString:@"*"]) {
			a = @( a.doubleValue * b.doubleValue );
		} else if ([self.text isEqualToString:@"/"]) {
			a = @( a.doubleValue / b.doubleValue );
		}

		[self.stack addObject:a];
	}
}

- (NSInteger)stackValue
{
	if (self.stack.count != 1) {
		NSLog(@"Error in operation");
		return 0;
	}

	return self.stack[0].integerValue;
}

