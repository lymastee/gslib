%{
#include <rathen\config.h>
#include <rathen\rpsr.h>

static rpscontext*  __rps_context = 0;
//extern int yylex();
//extern int rathen_yy_read(char* buff, int* bytes, int cap);
%}

%token TYPE TOKEN CHAIN DIVIDER KEYWORD OPERATOR_P OPERATOR_C STRING INTEGER FLOATPOINT EOSENTENCE

%%

NUMBER:             INTERGER            { rathen_create_constant(__rps_context, 0, rvt_integer, $1); }
                    | FLOATPOINT        { rathen_create_constant(__rps_context, 0, rvt_float, $1); }
                    ;
CONSTANT:           NUMBER
                    | STRING            { rathen_create_constant(__rps_context, 0, rvt_string, $1); }
                    ;
VALUE:              TOKEN               { /*rathen_create_*/ }
                    | CHAIN
                    | CONSTANT
                    | CALLING
                    | EXPRESSION
                    ;
ARGUMENT_LIST:      VALUE
                    | ARGUMENT_LIST ',' VALUE
                    ;
VALUE_TABLE:        '(' ')'
                    | '(' ARGUMENT_LIST ')'
                    ;
CALLING:            TYPE TOKEN VALUE_TABLE
                    ;
EXPRESSION:         VALUE OPERATOR_C VALUE
                    | OPERATOR_P VALUE
                    ;
ARGUMENT_DECL:      TYPE TOKEN
                    | TYPE& TOKEN
                    | ARGUMENT_DECL ',' TYPE TOKEN
                    | ARGUMENT_DECL ',' TYPE& TOKEN
                    ;
DECL_TABLE:         '(' ')'
                    | '(' ARGUMENT_DECL ')'
                    ;
STATEMENT:          DECLARATION
                    | IF_STATEMENT
                    | LOOP_STATEMENT
                    | EXIT_STATEMENT
                    | "go"
                    | "stop"
                    ;
BLOCK_CONTENT:      STATEMENT
                    | BLOCK_CONTENT STATEMENT
                    ;
BLOCK:              '{' '}'
                    | '{' BLOCK_CONTENT '}'
                    ;
DECLARATION:        TYPE TOKEN
                    | TYPE& TOKEN = VALUE
                    | DECLARATION ',' TOKEN
                    ;
IF_STATEMENT:       "if" '(' VALUE ')' BLOCK
                    | IF_STATEMENT "elif" '(' VALUE ')' BLOCK
                    | IF_STATEMENT "else" BLOCK
                    ;
LOOP_STATEMENT:     "loop" '(' ')' BLOCK
                    | "loop" '(' VALUE ')' BLOCK
                    | "loop" '(' INTERGER ')' BLOCK
                    ;
EXIT_STATEMENT:     "exit"
                    | "exit" '(' VALUE ')'
                    ;
FUNCTION:           TYPE TOKEN DECL_TABLE BLOCK
                    ;
SENTENCE:           EOSENTENCE
                    ;

%%
