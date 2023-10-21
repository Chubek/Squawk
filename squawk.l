%{

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#include <unistr.h>
#include <gc.h>

#include "escape-map.h"
#include "yy.tab.h"

uint8_t* gc_strndup(uint8_t* str, size_t len) {
	uint8_t* nstr = (uint8_t*)GC_MALLOC(len + 1);
	u8_strncat(nstr, str, len);
	return nstr;
}

uint8_t* gc_strncat(uint8_t* str, uint8_t* cat, size_t len) {
	str = (uint8_t*)GC_REALLOC(u8_strlen(str) + len);
	u8_strncat(&str[0], &cat[0], len);
	return str;
}

#define DUP() gc_strndup(&yytext[0], yyleng
#define CAT() gc_strncat(yylval.str, yytext, yyleng)
#define MAP() gc_strncat(yylval.str, ESCAPE_MAP[yytext[0]], 1)
#define FLD() strtoll(&yytext[1], NULL, 10)

extern YYSTYPE yylval;

extern size_t lineno;
extern size_t bracecnt;
extern size_t brackcnt;
extern size_t parencnt;

%}

%x STR
%x REG
%x ESC
%s COM

Num	[0-9]+
Int	[+-][1-9][0-9]*
Flt	[+-][0-9]*[eE\.][0-9]+
Hex	[0xX][a-fA-F0-9]+
Oct	[0oO][0-7]+
Bin	[0bB][0-1]+
Ident	[a-zA-Z\_][a-zA-Z\_0-9]*
Esc	[abefnrtv'"\[\]\\]

%%

"BEGIN"			return BEG;
"END"			return END;
"break"			return BREAK;
"continue"		return CONTINUE;
"delete"		return DELETE;
"do"			return DO;
"else"			return ELSE;
"exit"			return EXIT;
"for"			return FOR;
"function"		return FUNCTION;
"if"			return IF;
"in"			return IN;
"next"			return NEXT;
"print"			return PRINT;
"printf"		return PRINTF;
"while"			return WHILE;
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
| "system"	     return BUILTIN;


[\/]		       BEGIN REG;
['] | ["]	       BEGIN STR;
{Int}		     { yylval.int = strtoll(yytext, NULL, 10); 	}
{Bin}		     { yylval.int = strtoll(yytext, NULL, 2);	}
{Hex}		     { yylval.int = strtoll(yytext, NULL, 16);  }
{Oct}		     { yylval.int = strtoll(yytext, NULL, 8);   }
{Ident}		     { yylval.ident = DUP(); return IDENT; 	}

"$"{Num}	     { yylval.fld = FLD(); return FIELD;	}

<STR> "\\"	        BEGIN ESC;
<STR> [^"]* | [^']*  { yylval.str = DUP(); return STRING; 	}
<STR> \" | \'	        BEGIN INITIAL;

<REG> [^/]+	     { yylval.regex = DUP(); return REGEX;	}
<REG> '\\'	        BEGIN ESC;
<REG> "/"	        BEGIN INITIAL;

<REG, STR> "\n"	     { yyerror(NEWLINE_ERR); BEGIN INITIAL;     }
<ESC, STR> {Esc}     { yylval.str = MAP(); BEGIN STR;           }
<ESC, REG> {Esc}     { yylval.str = MAP(); BEGIN REG;           }

"+="		      return ADD_ASSIGN;
"-="		      return SUB_ASSIGN;
"*="		      return MUL_ASSIGN;
"^="		      return POW_ASSIGN;
"/="		      return DIV_ASSIGN;
"%="		      return REM_ASSIGN;

"||"		      return OR;
"&&"		      return AND;
"!~"		      return NO_MATCH;
"=="		      return EQ;
"<="		      return LE;
">="		      return GE;
"!="		      return NE;
"++"		      return INCR;
"--"		      return DECR;
">>"		      return APPEND;

"{"		      { bracecnt++; return LBRACE; }
"}"		      { bracecnt <= 0 
			? yyerror(BRACE_ERR) 
			: bracecnt--; return RBRACE; }
"["		      { brackcnt++; return LBRACK; }
"]"		      { brackcnt <= 0 
			? yyerror(BRACK_ERR) 
			: brackcnt--; return RBRACK; }
"("		      { parencnt++; return LPAREN; }
"}"		      { parencnt <= 0 
			? yyerror(PAREN_ERR) 
			: parencnt--; return RPAREN; }
"\n" | "\r\n"	      { lineno++; return NEWLINE; }

"/*" | "//"	        BEGIN COM;
<COM> .*		REJECT();
<COM> "*/" | 
"\n" 	   |
"\r\n"			BEGIN INITIAL;


