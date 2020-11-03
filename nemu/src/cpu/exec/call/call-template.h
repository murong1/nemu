#include "cpu/exec/template-start.h"

#define instr call

static void do_execute() {
	OPERAND_W(op_dest, op_src->val);
	print_asm_template2();
}

make_instr_helper(i2r)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)

make_helper(concat(call_i_, SUFFIX)) {
	int len = concat(decode_i_,SUFFIX)(cpu.eip + 1);

    reg_l(R_ESP) -= DATA_BYTE;
    swaddr_write(reg_l(R_ESP),4,cpu.eip + len + 1);
    DATA_TYPE_S imm = op_src -> val;
    print_asm("call\t%x",cpu.eip + 1 +len + imm);
    cpu.eip+=imm;
    return len + 1;
}


#include  "cpu/exec/template-end.h"