#include "../include/ast_node.h"

enum TYPE {
    TYPE_INT, TYPE_FLOAT, TYPE_VOID
};

inline pType toPTYPE(TYPE type) {
    switch (type) {
        case TYPE_VOID:
            return make_void_type();
        case TYPE_INT:
            return make_basic_type(IMM_I32);
        case TYPE_FLOAT:
            return make_basic_type(IMM_F32);
    }
    return make_void_type();
}

inline TypedSym toTypedSym(std::string* id, TYPE type) {
    TypedSym result(to_symbol(*id), toPTYPE(type));
    delete id;
    return result;
}

struct DefAST {
    std::string id;
    Vector<pNode> indexes;
    pNode initVal;

    pNode create(bool is_const, TYPE type) {
        return pNode(new Ast::VarDefNode(NULL, TypedSym(to_symbol(id), toPTYPE(type)), initVal, is_const));
    }
};