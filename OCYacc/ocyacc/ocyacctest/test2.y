%token n e

%start P

%%

P : E P
  | E
  ;

E : A ';'
  | e ';'
  ;

A : n '+' A
  | n
  ;
