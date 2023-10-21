%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <gc.h>
#include <unistr.h>

#include "machine.h"

#define BB_BOUNDARY (last_compiled = NULL, block_insert(vmcodep))

extern uint8_t* gc_strndup(uint8_t* str, size_t len);                      extern uint8_t* gc_strncat(uint8_t* str, uint8_t* cat, size_t len);

Label 	*vm_prim;
Inst 	*vmcodep;
int 	vm_debug;

#include "squawk-gen.i"

extern void 	yyerror(char *s);
extern int  	yylex(void);

int locals 	= 0;
int nonparams 	= 0;
%}

%union {
	uint8_t*	str;
	uint8_t*	regex;
	uint8_t*	ident;
	int64_t		intg;
	long double	fltn;
	int		fieldn;
	Inst*		instp;
}

%token Begin   End
%token Break   Continue   Delete   Do   Else
%token Exit   For   Function   If   In
%token Next   Print   Printf   Return   While

%token BUILTIN_FUNC_NAME

%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN POW_ASSIGN

%token STRING REGEX 
%token INTEGER FLOATNUM
%token FIELD

%token OR   AND  NO_MATCH   EQ   LE   GE   NE   INCR  DECR  APPEND

%token LBRACE RBRACE LPAREN RPAREN LBRACK RBRACK ',' ';' NEWLINE
%token '+' '-' '*' '%' '^' '!' '>' '<' '|' '?' ':' '˜' '$' '='

%type <str> 	IDENT
%type <str> 	REGEX
%type <intg>	INTEGER
%type <fltn>	FLOATNUM
%type <fieldn>	FIELD

%start awkprog
%%
