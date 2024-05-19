#include "../include/ast_node.h"
#include "type.h"
#include <iterator>

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

inline TypedNodeSym toTypedSym(std::string* id, pType ty) {
    TypedNodeSym result(to_symbol(*id), Ast::new_basic_type_node(NULL, ty));
    delete id;
    return result;
}

inline TypedNodeSym toTypedSym(std::string* id, TYPE type) {
    return toTypedSym(id, toPTYPE(type));
}

inline TypedNodeSym toTypedSym(std::string* id, pNode type) {
    return TypedNodeSym(to_symbol(*id), type);
}

struct DefAST {
    std::string id;
    Vector<pNode> indexes;
    pNode initVal { nullptr };

    pNode create(bool is_const, TYPE type) {
        auto innerTy = Ast::new_basic_type_node(NULL, toPTYPE(type));
        if(indexes.empty()) {
            return pNode(new Ast::VarDefNode(NULL, 
                TypedNodeSym(to_symbol(id), innerTy), initVal, is_const));
        }
        for(auto i = std::rbegin(indexes); i!=std::rend(indexes); ++i) {
            innerTy = Ast::new_array_type_node(NULL, innerTy, *i);
        }
        return pNode(new Ast::VarDefNode(NULL, 
            TypedNodeSym(to_symbol(id), innerTy), initVal, is_const));
    }
};