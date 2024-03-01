CC = g++
CFLAGS = -Wall -g

EXECUTABLE = compiler

GLOBAL_DEPS = def.h
GLOBAL_OBJ = def.o

AST_DEPS = ast_node.h
AST_OBJ = ast_node.o

IR_DEPS = ir_block.h ir_constant.h ir_instr.h ir_val.h
IR_OBJ = ir_block.o ir_constant.o ir_instr.o ir_val.o

ASM_OBJ = asm_instr.o asm_regs.o
ASM_DEPS = asm_instr.h asm_regs.h

DEPS = $(AST_DEPS) $(IR_DEPS) $(ASM_DEPS) $(GLOBAL_DEPS)
OBJ = $(AST_OBJ) $(IR_OBJ) $(ASM_OBJ) $(GLOBAL_OBJ) $(EXECUTABLE).o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

compiler: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

run: $(EXECUTABLE).exe
	./$(EXECUTABLE).exe

.PHONY: clean

clean:
	rm -f *.o *~ compiler
