#include "ir_module.h"
#include "ast_node.h"
#include "convert_ast_to_ir.h"
#include "staisp_parser.h"

#include <fstream>

int main(int argc, char *argv[])
{
    if(argc != 2) {
        puts("Usage: staisp [in_file]");
        return 0;
    }
    std::ifstream in;
    in.open(argv[1], std::fstream::in);
    if(!in.is_open()) {
        printf("Error: cannot open file \"%s\".", argv[1]);
        return 1;
    }
    String code((std::istreambuf_iterator<char>(in)),  
                 std::istreambuf_iterator<char>()); 
    pCode p_code = make_code(code, argv[1]);
    try {
        AstProg root = Staisp::Parser().parse(p_code);
        Ir::pModule mod = AstToIr::Convertor().generate(root);
        std::ofstream out;
        out.open(String(argv[1]) + ".ll", std::fstream::out);
        out << mod->print_module();
        out.close();
    } catch (Exception) {
        puts("Exception Catched.");
    }
    return 0;
}
