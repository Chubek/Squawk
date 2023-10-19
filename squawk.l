%{

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistr.h>
#include <gc.h>

static inline uint8_t* gc_strndup(uint8_t* str, size_t len) {
	uint8_t* nstr = (uint8_t*)GC_MALLOC(len + 1);
	u8_strncat(nstr, str, len);
	return nstr;
}

#define DUP() gc_strndup(&yytext[0], yyleng)

%}

%s STR
%s INT
%s HEX
%s OCT
%s BIN
%s REG
%s FUN
%s ASN
%s BLT
%s CAL
%s FUR
%s IFF
%s INN
%s PRN
%s PRF
%s WHL

Int	[+-][1-9][0-9]*
Flt	[+-][0-9]*[eE\.][0-9]+
Hex	[0xX][a-fA-F0-9]+
Oct	[0oO][0-7]+
Bin	[0bB][0-1]+

%%

"BEGIN"			return BEG;
"END"			return END;
"break"			return BREAK;
"continue"		return CONTINUE;
"delete"		return DELETE;
"do"			return DO;
"else"			return ELSE;
"exit"			return EXIT;
"for"			{  BEGIN FUR; return FOR; 	}
"function"		{  BEGIN FUN; return FUNCTION;  }
"if"			{  BEGIN IFF; return IF;	}
"in"			{  BEGIN INN; return IN; 	}
"next"			return NEXT;
"print"			{  BEGIN PRN; return PRINT;	}
"printf"		{  BEGIN PRF; return PRINTF;	}
"while"			{  BEGIN WHL; return WHILE;	}
"return"		return RETURN;

"atan2" 
| "cos"
| "sin"
| "exp"
| "log"
| "sqrt"
| "int"
| "rand"
| "srand"
| "gsub"
| "index"
| "length"
| "match"
| "split"
| "sprintf"
| "sub"
| "substr"
| "tolower"
| "toupper"
| "close"
| "system"	     { BEGIN BTN; return BUILTIN;	}


[\/]		     { BEGIN REG; }
['] | ["]	     { BEGIN STR; }
{Int}		     { yylval.int = strtoll(yytext, NULL, 10); 	}
{Bin}		     { yylval.int = strtoll(yytext, NULL, 2);	}
{Hex}		     { yylval.int = strtoll(yytext, NULL, 16);  }
{Oct}		     { yylval.int = strtoll(yytext, NULL, 8);   }








