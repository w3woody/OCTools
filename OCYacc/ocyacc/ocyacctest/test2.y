%token n

%start E

%%

E : A
  ;

A : M
  | A '+' M
  ;

M : n
  | M '*' n
  ;
