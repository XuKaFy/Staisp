#include "node.h"
#include "evaluate.h"
#include "chunk.h"
#include "regs.h"

#define OP2(a, b, c) Node::operation_2(a, b, c )
#define INT32(x) Node::int32(x)
#define INT64(x) Node::int64(x)

int main()
{
    pNode root = newOprNode(
        { newOprNode( { newImmNode(1), newImmNode(2) }, OPR_MUL),
          newOprNode( { newImmNode(3), newImmNode(4) }, OPR_MUL) } ,OPR_ADD);
    Chunk c;
    Regs regs;
    Reg res = evaluate(c, root, regs);
    printf("  last reg: %s\n", res.name);
    return 0;
}

