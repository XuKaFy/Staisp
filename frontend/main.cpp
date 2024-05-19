#include <ast_node.h>

#include "common_node.h"
#include "convert_ast_to_ir.h"
#include "opt.h"

#include <fstream>

extern void setFileName(const char *name);
extern void yyparse();
extern Vector<pNode> root;

int main(int argc, char *argv[])
{
    if(argc != 2) {
        puts("Usage: SysYFrontend [in_file]");
        return 0;
    }
    setFileName(argv[1]);
    yyparse();

    AstProg ast_root(root.begin(), root.end());
    
    /*printf("root elements: %zu\n", root.size());
    for(auto &&i : root) {
        i->print();
        puts("");
    }*/
    
    try {
        Ir::pModule mod = AstToIr::Convertor().generate(ast_root);

        std::ofstream out;
        out.open(String(argv[1]) + ".ll", std::fstream::out);
        out << mod->print_module();
        out.close();
        
        // optimize
        Optimize::optimize(mod);

        out.open(String(argv[1]) + ".opt.ll", std::fstream::out);
        out << mod->print_module();
        out.close();
    } catch (Exception e) {
        printf("Exception Catched: [%s] error %lu: %s\n", e.object, e.id, e.message);
        return 1;
    }
    return 0;
}