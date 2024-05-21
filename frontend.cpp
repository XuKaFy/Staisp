#include "convert_ast_to_ir.h"
#include "ir_module.h"
#include "staisp_parser.h"

#include "opt.h"

#include <fstream>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        puts("Usage: staisp [in_file]");
        return 0;
    }
    std::ifstream in;
    in.open(argv[1], std::fstream::in);
    if (!in.is_open()) {
        printf("Error: cannot open file \"%s\".\n", argv[1]);
        return 1;
    }
    String code((std::istreambuf_iterator<char>(in)),
                std::istreambuf_iterator<char>());
    pCode p_code = make_code(code, argv[1]);
    try {
        AstProg root = Staisp::Parser().parse(p_code);
        Ir::pModule mod = AstToIr::Convertor().generate(root);

        std::ofstream out;
        // out.open(String(argv[1]) + ".ll", std::fstream::out);
        // out << mod->print_module();
        // out.close();

        // optimize
        Optimize::optimize(mod);

        out.open(String(argv[1]) + ".ll", std::fstream::out);
        out << mod->print_module();
        out.close();
    } catch (Exception e) {
        printf("Exception Catched: [%s] error %lu: %s\n", e.object.c_str(),
               e.id, e.message.c_str());
        return 1;
    }
    return 0;
}
