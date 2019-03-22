%start Test

%token <value> NUMBER
%type <value> add

%union {
    int value;
}

/* Test placement */
%global {
    %global
}
%local {
    %local
}
%header {
    %header
}
%init {
    %init
}
%finish {
    %finish
}
%{
    %%
%}

%start add

%%

add: NUMBER '+' NUMBER 
        { 
            $$ = $1 + $3; 
        } 
   ;

%%
CODE
