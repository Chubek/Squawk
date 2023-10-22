#include <stdio.h>

#include "machine.h"

void genarg_i(Inst **vmcodepp, Cell i) {
  *((Cell *) *vmcodepp) = i;
  (*vmcodepp)++;
}

void genarg_target(Inst **vmcodepp, Inst *target) {
  *((Inst **) *vmcodepp) = target;
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

