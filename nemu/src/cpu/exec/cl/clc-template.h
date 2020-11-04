#include "cpu/exec/template-start.h"


make_helper(clc) {
	cpu.CF = 0;
	print_asm("clc");
	return 1;
}

#include "cpu/exec/template-end.h"
