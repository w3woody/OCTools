%token c d

%start G

%%

G : C C
  ;
C : c C
  | d
  ;
