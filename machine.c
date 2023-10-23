#include <stdio.h>

#include "machine.h"
#include "squawk.h"


void genarg_i(Inst **vmcodepp, long i) {
   vm_i2Cell(i, *((Cell *) *vmcodepp));
   (*vmcodepp)++;
}

void genarg_target(Inst **vmcodepp, Inst* target) {
   vm_target2Cell(target, *((Cell *) *vmcodepp));
   (*vmcodepp)++;
}

void genarg_target(Inst **vmcodepp, Inst* target) {

   vm_target2Cell(target, *((Cell *) *vmcodepp));
   (*vmcodepp)++;
}

void genarg_f(Inst **vmcodepp, long double f) {
   vm_f2Cell(target, *((Cell *) *vmcodepp));
   (*vmcodepp)++;
}

void genarg_s(Inst **vmcodepp, unsigned char* s) {
   vm_s2Cell(target, *((Cell *) *vmcodepp));
   (*vmcodepp)++;
}

void printarg_i(Cell i) {
  fprintf(vm_out, "%ld ", i);
}

void printarg_target(Inst *target) {
  fprintf(vm_out, "%p ", target);
}

void printarg_a(char *a) {
  fprintf(vm_out, "%p ", a);
}


static inline Inst* func_addr(uint8_t* id) {
	Inst* start;
	Inst* end;
	int   nonparams;
	int   params;

	if (sym_func_get(id, &start, &end, &nonparams, &params) < 0) {
		fputs(ERR_SYM_FUNC, stderr);
		exit(EXIT_FAILURE);
	}

	return start;
}

static inline int func_calladjust(uint8_t* id) {
	Inst* start;
	Inst* end;
	int   nonparams;
	int   params;

	if (sym_func_get(id, &start, &end, &nonparams, &params) < 0) {
		fputs(ERR_SYM_FUNC, stderr);
		exit(EXIT_FAILURE);
	}

	return nonparams;
}

static inline int get_sym_offset(uint8_t* id) {
	int offs;
	if ((offs = sym_index(id) < 0)) {
		fputs(ERR_SYM_VAR, stderr);
		exit(EXIT_FAILURE);
	}
	return (locals - offs + 2) * sizeof(Cell);
}

