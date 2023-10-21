#ifndef MACHINE_H_
#define MACHINE_H_

#include <stdio.h>
#include <stdint.h>

#include "squawk.h"

#ifdef __GNUC__
typedef void* Label;
#else
typedef long* Label;
#endif

typedef union Cell {
	long long		i;
	Cell*			target;
	Label			inst;
	char*			a;
	unsigned char*		s;
	void*			v;
	unsigned char**		p;
	void**			j;
	unsigned long long	u;
	unsigned double		f;
} Cell, Inst;

#define vm_Cell2i(_cell,_x)     	((_x)=(_cell).i)
#define vm_i2Cell(_x,_cell)     	((_cell).i=(_x))        
#define vm_Cell2target(_cell,_x) 	((_x)=(_cell).target)
#define vm_target2Cell(_x,_cell) 	((_cell).target=(_x))  
#define vm_Cell2a(_cell,_x)     	((_x)=(_cell).a)
#define vm_a2Cell(_x,_cell)     	((_cell).a=(_x))        
#define vm_Cell2Cell(_x,_y) 		((_y)=(_x))

#define symfn_res2nparams(res)		(res & MAX_UINT32)
#define symfn_res2nonparams(res)	(res & (MAX_UINT32 << UINT32_WIDTH))

extern Label *vm_prim;
extern int locals;
extern struct Peeptable_entry **peeptable;
extern int vm_debug;
extern FILE *yyin;
extern int yylineno;
extern char *program_name;
extern FILE *vm_out;
extern Inst *vmcodep;
extern Inst *last_compiled;
extern Inst *vmcode_end;
extern int use_super;


void gen_inst(Inst **vmcodepp, Label i);
void init_peeptable(void);
void vm_disassemble(Inst *ip, Inst *endp, Label prim[]);
void vm_count_block(Inst *ip);
struct block_count *block_insert(Inst *ip);
void vm_print_profile(FILE *file);

long engine(Inst *ip0, Cell *sp, char *fp);
long engine_debug(Inst *ip0, Cell *sp, char *fp);

#endif
