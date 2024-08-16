#include <ast_node.h>

#include "bkd_module.h"
#include "common_node.h"
#include "convert_ast_to_ir.h"
#include "opt.h"
#include "convert_ir_to_asm.h"

#include <fstream>

extern void setFileName(const char *name);
extern void yyparse();
extern Vector<pNode> root;
bool flag_O1 = false;
bool flag_no_backend;

namespace Backend {
extern bool flag_O1;
}


int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Usage: %s -S [-o out_file] [in_file] [-o1]\n", argv[0]);
        return 0;
    }

    bool flag_S = false;
    bool in_file_set = false;
    String out_file_o;
    String out_file_ll;
    for (int i=1; i<argc; ++i) {
        if (strcmp(argv[i], "-S") == 0) {
            flag_S = true;
            continue;
        }
        if (strcmp(argv[i], "-o1") == 0 ||
            strcmp(argv[i], "-O1") == 0) {
            Backend::flag_O1 = flag_O1 = true;
            continue;
        }
        if (strcmp(argv[i], "-o") == 0) {
            ++i;
            if (i >= argc) {
                puts("Error: no specified file of \"-o\".");
            }
            out_file_o = argv[i];
            continue;
        }
        if (strcmp(argv[i], "-ll") == 0) {
            ++i;
            if (i >= argc) {
                puts("Error: no specified file of \"-ll\".");
            }
            out_file_ll = argv[i];
            continue;
        }
        setFileName(argv[i]);    
        in_file_set = true;
    }
    if (!flag_S) {
        printf("%s: only run in -S mode.\n", argv[0]);
        return 1;
    }
    if (!in_file_set) {
        puts("Error: no specified input.");
        return 1;
    }
    if (out_file_ll.empty() && out_file_o.empty()) {
        puts("Error: no specified output.");
        return 1;
    }
    
    if (flag_O1) {
        puts("Run in Optimization 1...");
    }
    
    flag_no_backend = out_file_o.empty();

    yyparse();

    AstProg ast_root(root.begin(), root.end());

#ifdef CATCHING_EXCEPTION
    try {
#endif
        std::ofstream out;
        AstToIr::Convertor convertor;
        Ir::pModule mod = convertor.generate(ast_root);

        Optimize::optimize(mod);

        if (!out_file_ll.empty()) {
            out.open(out_file_ll, std::fstream::out);
            out << mod->print_module();
            out.close();
        }

        if (!out_file_o.empty()) {
            Backend::Convertor bkd_convertor;
            Backend::Module bkd_mod = bkd_convertor.convert(mod);

            out.open(out_file_o, std::fstream::out);
            out << bkd_mod.print_module();
            out.close();
        }
#ifdef CATCHING_EXCEPTION
    } catch (Exception e) {
        printf("Exception Catched: [%s] error %lu: %s\n", e.object.c_str(),
               e.id, e.message.c_str());
        return 1;
    }
#endif
    return 0;
}