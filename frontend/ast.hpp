#include "../include/ast_node.h"
#include "../include/convert_ast_to_ir.h"
#include "type.h"

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

inline TypedSym toTypedSym(std::string* id, pType ty) {
    TypedSym result(to_symbol(*id), ty);
    delete id;
    return result;
}

inline TypedSym toTypedSym(std::string* id, TYPE type) {
    TypedSym result(to_symbol(*id), toPTYPE(type));
    delete id;
    return result;
}

struct DefAST {
    std::string id;
    Vector<pNode> indexes;
    pNode initVal { nullptr };

    pNode create(bool is_const, TYPE type) {
        auto innerTy = toPTYPE(type);
        if(indexes.empty()) {
            return pNode(new Ast::VarDefNode(NULL, 
                TypedSym(to_symbol(id), innerTy), initVal, is_const));
        }
        for(auto &&i : indexes) {
            auto immValue = AstToIr::Convertor::constant_eval(i);
            if(is_imm_integer(immValue.ty)) {
                innerTy = make_array_type(innerTy, immValue.val.uval);
            } else {
                puts("index must be integer");
                exit(0);
            }
        }
        return pNode(new Ast::VarDefNode(NULL, 
            TypedSym(to_symbol(id), innerTy), initVal, is_const));
    }
};