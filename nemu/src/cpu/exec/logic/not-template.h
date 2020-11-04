#include "cpu/exec/template-start.h"

#define instr not

static void do_execute() {
	DATA_TYPE result = ~op_src->val;
    	cpu.ZF=!result;
    	OPERAND_W(op_src, result);
	print_asm_template1();
}

make_instr_helper(rm)

#include "cpu/exec/template-end.h"
