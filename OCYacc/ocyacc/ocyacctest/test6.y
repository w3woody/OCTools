%token n

%start E

%%

E : L '=' R
  | R
  ;

L : '*' R
  | n
  ;

R : L
  ;

