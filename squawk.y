%{

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <gc.h>
#include <unistr.h>

#define BB_BOUNDARY (last_compiled = NULL, block_insert(vmcodep))

Label 	*vm_prim;
Inst 	*vmcodep;
FILE 	*vm_out;
int 	vm_debug;

void 	yyerror(char *s);
int  	yylex(void);

%}
