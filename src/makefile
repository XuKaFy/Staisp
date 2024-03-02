CC = g++
CFLAGS = -Wall -g

EXECUTABLE = compiler

GLOBAL_DEPS = def.h
GLOBAL_OBJ = def.o

AST_DEPS = ast_node.h
AST_OBJ =  ast_node.o

IR_DEPS = ir_block.h ir_constant.h ir_instr.h ir_val.h ir_opr_instr.h ir_func.h ir_func_defined.h ir_call_instr.h ir_sym.h ir_global.h ir_module.h ir_mem_instr.h ir_control_instr.h
IR_OBJ =  ir_block.o ir_constant.o ir_instr.o ir_val.o ir_opr_instr.o ir_func.o ir_func_defined.o ir_call_instr.o ir_sym.o ir_global.o ir_module.o ir_mem_instr.o ir_control_instr.o

ASM_DEPS = asm_instr.h asm_regs.h
ASM_OBJ =  asm_instr.o asm_regs.o

DEPS = $(AST_DEPS) $(IR_DEPS) $(ASM_DEPS) $(GLOBAL_DEPS)
OBJ = $(AST_OBJ) $(IR_OBJ) $(ASM_OBJ) $(GLOBAL_OBJ) $(EXECUTABLE).o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

run: $(EXECUTABLE).exe
	./$(EXECUTABLE).exe

$(EXECUTABLE).exe: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ compiler
 