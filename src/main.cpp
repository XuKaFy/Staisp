#include "node.h"
#include "evaluate.h"
#include "chunk.h"
#include "regs.h"

#define OP2(a, b, c) Node::operation_2(a, b, c )
#define INT32(x) Node::int32(x)
#define INT64(x) Node::int64(x)

int main()
{
    Pointer<Node> node (OP2(NODE_ADD,
        OP2(NODE_MUL, INT64(1), INT64(2)), 
        OP2(NODE_MUL, INT64(3), INT64(4))));
    Chunk c;
    Regs regs;
    Reg res = evaluate(c, node, regs);
    c.print();
    printf("  last reg: ");
    res.print();
    puts("");
    return 0;
}

