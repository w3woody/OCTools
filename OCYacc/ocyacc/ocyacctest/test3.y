%token n

%start E

%%

E : E '+' E
  | E '*' E
  | n
  ;
