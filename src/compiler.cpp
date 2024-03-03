#include "ir_module.h"
#include "ast_node.h"
#include "convert_ast_to_ir.h"
#include "staisp_parser.h"

#include <fstream>

int main(int argc, char *argv[])
{
    if(argc != 2) {
        puts("Usage: staisp [in_file]");
        return 1;
    }
    std::ifstream in;
    in.open(argv[1], std::fstream::in);
    if(!in.is_open()) {
        printf("Error: cannot open file \"%s\".", argv[1]);
        return 1;
    }
    String code((std::istreambuf_iterator<char>(in)),  
                 std::istreambuf_iterator<char>()); 
    Ast::AstProg root = Staisp::Parser().parser(code);
    Ir::pModule mod = AstToIr::Convertor().generate(root);
    printf("%s\n", mod->print_module());
    return 0;
}
