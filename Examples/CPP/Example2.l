%{
    %%
%}

N       [0-9]

/*
%union {
    int value;
}
*/

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

%start Test


%%

N   { return 1; }
.   {}

%%
CODE
