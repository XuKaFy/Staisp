#include "ir_module.h"
#include "ast_node.h"
#include "convert_ast_to_ir.h"

#define IF(x, y) newOprNode(OPR_IF, { y } )
#define OR(x, y) newOprNode(OPR_OR, {x, y} )
#define EQ(x, y) newOprNode(OPR_EQ, {x, y} )
#define ADD(x, y) newOprNode(OPR_ADD, {x, y} )
#define SUB(x, y) newOprNode(OPR_SUB, {x, y} )
#define SYM(x) newSymNode(#x)
#define RET(x) newOprNode(OPR_RET, {x} )
#define I32NUM(x) newImmNode(x, IMM_I32)
#define CALL(x, y) newOprNode(OPR_CALL, y)
#define DEF_I32VAR(x, y) newVarDefNode(#x, y, IMM_I32)
#define DECL(x, y) (TypedSym { #y, x })
#define ASSIGN(x, y) newAssignNode(#x, y)

Ast::AstProg test()
{
    using namespace Ast;
    AstProg prog;
/*
    int n = 50;

    int fib(int n)
    {
        if(n == 1 || n == 2)
            return 1;
        return fib(n-1) + fib(n-2);
    }

    int main() {
        int ans = fib(n);
        return ans;
    }
*/
    prog.push_back(DEF_I32VAR(n, 50));
    prog.push_back(newFuncDefNode("fib", IMM_I32, { DECL(IMM_I32, n) }, {
        IF(OR(EQ(SYM(n), I32NUM(1)), EQ(SYM(n), I32NUM(2))),
            RET(I32NUM(1))
        ),
        RET(ADD(CALL(fib, { SUB(SYM(n), I32NUM(1)) }), CALL(fib, { SUB(SYM(n), I32NUM(2)) })))
    }));
    prog.push_back(newFuncDefNode("main", IMM_I32, { }, {
        DEF_I32VAR(ans, 0),
        ASSIGN(ans, CALL(fib, { SYM(n) } )),
        RET(SYM(ans))
    }));
    return prog;
}

int main()
{
    Ir::pModule mod = AstToIr::generate(test());
    printf("%s\n", mod->print_module());
    return 0;
}
