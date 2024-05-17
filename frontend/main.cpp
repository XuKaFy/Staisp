#include <ast_node.h>


extern void setFileName(const char *name);
extern void yyparse();
extern Vector<pNode> root;


int main() {
    setFileName("test.sysy");
    yyparse();
    printf("root elements: %zu\n", root.size());
    for(auto &&i : root) {
        i->print();
        puts("");
    }
}