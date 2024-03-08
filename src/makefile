CC = g++
CFLAGS = -Wall -g

EXECUTABLE = compiler

GLOBAL_DEPS = def.h common_node.h common_token.h type.h imm.h env.h
GLOBAL_OBJ =  def.o common_node.o common_token.o type.o imm.o

AST_DEPS = ast_node.h ast_exec.h
AST_OBJ =  ast_node.o ast_exec.o

IR_DEPS = ir_block.h ir_constant.h ir_instr.h ir_val.h ir_opr_instr.h ir_func.h ir_func_defined.h \
		  ir_call_instr.h ir_sym.h ir_global.h ir_module.h ir_mem_instr.h ir_control_instr.h ir_cmp_instr.h ir_cast_instr.h ir_ptr_instr.h
IR_OBJ =  ir_block.o ir_constant.o ir_instr.o ir_val.o ir_opr_instr.o ir_func.o ir_func_defined.o \
	      ir_call_instr.o ir_sym.o ir_global.o ir_module.o ir_mem_instr.o ir_control_instr.o ir_cmp_instr.o ir_cast_instr.o ir_ptr_instr.o

ASM_DEPS = asm_instr.h asm_regs.h
ASM_OBJ =  asm_instr.o asm_regs.o

CONVERT_AST_TO_IR_DEPS = convert_ast_to_ir.h
CONVERT_AST_TO_IR_OBJ  = convert_ast_to_ir.o

STAISP_DEPS = staisp_parser.h staisp_lexer.h
STAISP_OBJ  = staisp_parser.o staisp_lexer.o

DEPS = $(AST_DEPS) $(IR_DEPS) $(ASM_DEPS) $(GLOBAL_DEPS) $(CONVERT_AST_TO_IR_DEPS) $(STAISP_DEPS)
OBJ = $(AST_OBJ) $(IR_OBJ) $(ASM_OBJ) $(GLOBAL_OBJ) $(EXECUTABLE).o $(CONVERT_AST_TO_IR_OBJ) $(STAISP_OBJ)

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

run: $(EXECUTABLE).exe
	./$(EXECUTABLE).exe

$(EXECUTABLE).exe: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ compiler
 