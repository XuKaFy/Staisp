#pragma once

#include "def.h"
#include "imm.h"
#include "type.h"

#include "common_node.h"
#include "common_token.h"

namespace Ast {

struct ImmNode : public Node {
    ImmNode(pToken t, ImmValue imm) : Node(t, NODE_IMM), imm(imm) {}

    virtual void print(size_t tabs = 0) override { printf("%s", imm.print()); }

    ImmValue imm;
};

struct SymNode : public Node {
    SymNode(pToken t, Symbol sym) : Node(t, NODE_SYM), sym(sym) {}

    virtual void print(size_t tabs = 0) override { printf("%s", sym); }

    Symbol sym;
};

struct BinaryNode : public Node {
    BinaryNode(pToken t, BinaryType type, pNode lhs, pNode rhs)
        : Node(t, NODE_BINARY), type(type), lhs(lhs), rhs(rhs) {}

    virtual void print(size_t tabs = 0) override {
        printf("(");
        lhs->print(tabs);
        switch (type) {
        case BinaryType::OPR_ADD:
            printf(" + ");
            break;
        case BinaryType::OPR_AND:
            printf(" && ");
            break;
        case BinaryType::OPR_DIV:
            printf(" / ");
            break;
        case BinaryType::OPR_EQ:
            printf(" == ");
            break;
        case BinaryType::OPR_GE:
            printf(" >= ");
            break;
        case BinaryType::OPR_GT:
            printf(" > ");
            break;
        case BinaryType::OPR_LE:
            printf(" <= ");
            break;
        case BinaryType::OPR_LT:
            printf(" < ");
            break;
        case BinaryType::OPR_MUL:
            printf(" * ");
            break;
        case BinaryType::OPR_NE:
            printf(" != ");
            break;
        case BinaryType::OPR_OR:
            printf(" || ");
            break;
        case BinaryType::OPR_REM:
            printf(" %% ");
            break;
        case BinaryType::OPR_SUB:
            printf(" - ");
            break;
        }
        rhs->print(tabs);
        printf(")");
    }

    BinaryType type;
    pNode lhs, rhs;
};

struct UnaryNode : Node {

    UnaryNode(pToken t, UnaryType type, pNode ch)
        : Node(t, NODE_UNARY), type(type), ch(ch) {}

    virtual void print(size_t tabs = 0) override {
        printf("(");
        switch (type) {
        case UnaryType::OPR_NEG:
            printf("-");
            break;
        case UnaryType::OPR_NOT:
            printf("!");
            break;
        case UnaryType::OPR_POS:
            break;
        }
        ch->print(tabs);
        printf(")");
    }

    UnaryType type;
    pNode ch;
};

struct CallNode : public Node {
    CallNode(pToken t, Symbol name, Vector<pNode> ch)
        : Node(t, NODE_CALL), name(name), ch(ch) {}

    virtual void print(size_t tabs = 0) override {
        printf("%s(", name);
        bool first = true;
        for (auto &&i : ch) {
            if (first) {
                first = false;
            } else {
                printf(", ");
            }
            i->print(tabs);
        }
        printf(")");
    }

    Symbol name;
    Vector<pNode> ch;
};

struct AssignNode : public Node {
    AssignNode(pToken t, pNode lv, pNode val)
        : Node(t, NODE_ASSIGN), lv(lv), val(val) {}

    virtual void print(size_t tabs = 0) override {
        lv->print(tabs);
        printf(" = ");
        val->print(tabs);
    }

    pNode lv;
    pNode val;
};

struct BasicTypeNode : public Node {
    BasicTypeNode(pToken t, pType ty) : Node(t, NODE_BASIC_TYPE), ty(ty) {}

    virtual void print(size_t tabs = 0) override {
        printf("%s", ty->type_name());
    }

    pType ty;
};

struct PointerTypeNode : public Node {
    PointerTypeNode(pToken t, pNode n) : Node(t, NODE_POINTER_TYPE), n(n) {}

    virtual void print(size_t tabs = 0) override { printf("(*)"); }

    pNode n;
};

struct ArrayTypeNode : public Node {
    ArrayTypeNode(pToken t, pNode n, pNode index)
        : Node(t, NODE_ARRAY_TYPE), n(n), index(index) {}

    virtual void print(size_t tabs = 0) override {
        printf("[");
        n->print();
        printf("]");
    }

    pNode n;
    pNode index;
};

struct VarDefNode : public Node {
    VarDefNode(pToken t, TypedNodeSym var, pNode val, bool is_const = false)
        : Node(t, NODE_DEF_VAR), var(var), val(val), is_const(is_const) {}

    virtual void print(size_t tabs = 0) override {
        if (is_const)
            printf("const ");
        var.print();
        if (val) {
            printf(" = ");
            val->print(tabs);
        }
    }

    TypedNodeSym var;
    pNode val;
    bool is_const;
};

struct FuncDefNode : public Node {
    FuncDefNode(pToken t, TypedNodeSym var, Vector<TypedNodeSym> args,
                pNode body)
        : Node(t, NODE_DEF_FUNC), var(var), args(args), body(body) {}

    virtual void print(size_t tabs = 0) override {
        var.n->print(tabs);
        printf(" %s(", var.name);
        bool first = true;
        for (auto &&i : args) {
            if (first) {
                first = false;
            } else {
                printf(", ");
            }
            i.n->print(tabs);
            printf(" %s", i.name);
        }
        printf(") ");
        body->print(tabs);
    }

    TypedNodeSym var;
    Vector<TypedNodeSym> args;
    pNode body;
};

struct BlockNode : public Node {
    BlockNode(pToken t, AstProg body) : Node(t, NODE_BLOCK), body(body) {}

    virtual void print(size_t tabs = 0) override {
        printf("{\n");
        ++tabs;
        for (auto &&i : body) {
            for (size_t j = 0; j < tabs; ++j) {
                printf("    ");
            }
            i->print(tabs);
            if (i->type != NODE_BLOCK)
                putchar(';');
            printf("\n");
        }
        --tabs;
        for (size_t j = 0; j < tabs; ++j) {
            printf("    ");
        }
        printf("}");
    }

    AstProg body;
};

struct IfNode : public Node {
    IfNode(pToken t, pNode cond, pNode body, pNode elsed = {})
        : Node(t, NODE_IF), cond(cond), body(body), elsed(elsed) {}

    virtual void print(size_t tabs = 0) override {
        printf("if (");
        cond->print(tabs);
        printf(") ");
        body->print(tabs);
        if (elsed) {
            printf(" else ");
            elsed->print(tabs);
        }
    }

    pNode cond;
    pNode body;
    pNode elsed;
};

struct WhileNode : public Node {
    WhileNode(pToken t, pNode cond, pNode body)
        : Node(t, NODE_WHILE), cond(cond), body(body) {}

    virtual void print(size_t tabs = 0) override {
        printf("while (");
        cond->print(tabs);
        printf(") ");
        body->print(tabs);
    }

    pNode cond;
    pNode body;
};

struct ForNode : public Node {
    ForNode(pToken t, pNode init, pNode cond, pNode exec, pNode body)
        : Node(t, NODE_FOR), init(init), cond(cond), exec(exec), body(body) {}

    virtual void print(size_t tabs = 0) override {
        printf("for (");
        init->print(tabs);
        printf("; ");
        cond->print(tabs);
        printf("; ");
        exec->print(tabs);
        printf(") ");
        body->print(tabs);
    }

    pNode init;
    pNode cond;
    pNode exec;
    pNode body;
};

struct BreakNode : public Node {
    BreakNode(pToken t) : Node(t, NODE_BREAK) {}

    virtual void print(size_t tabs = 0) override { printf("break"); }
};

struct ContinueNode : public Node {
    ContinueNode(pToken t) : Node(t, NODE_CONTINUE) {}
    virtual void print(size_t tabs = 0) override { printf("continue"); }
};

struct ReturnNode : public Node {
    ReturnNode(pToken t, pNode ret = {}) : Node(t, NODE_RETURN), ret(ret) {}

    virtual void print(size_t tabs = 0) override {
        printf("return");
        if (ret) {
            printf(" ");
            ret->print(tabs);
        }
    }

    pNode ret;
};

struct ArrayDefNode : public Node {
    ArrayDefNode(pToken t, Vector<pNode> nums)
        : Node(t, NODE_ARRAY_VAL), nums(nums) {}

    virtual void print(size_t tabs = 0) override {
        bool first = true;
        printf("{");
        for (auto &&i : nums) {
            if (first) {
                first = false;
            } else {
                printf(", ");
            }
            i->print(tabs);
        }
        printf("}");
    }

    Vector<pNode> nums;
};

struct CastNode : public Node {
    CastNode(pToken t, pNode ty, pNode val)
        : Node(t, NODE_CAST), ty(ty), val(val) {}

    virtual void print(size_t tabs = 0) override {
        printf("(");
        ty->print(tabs);
        printf(")");
        val->print(tabs);
    }

    pNode ty;
    pNode val;
};

struct RefNode : public Node {
    RefNode(pToken t, pNode v) : Node(t, NODE_REF), v(v) {}

    virtual void print(size_t tabs = 0) override { printf("&"); }

    pNode v;
};

struct DerefNode : public Node {
    DerefNode(pToken t, pNode val) : Node(t, NODE_DEREF), val(val) {}

    virtual void print(size_t tabs = 0) override { printf("*"); }

    pNode val;
};

struct ItemNode : public Node {
    ItemNode(pToken t, pNode v, Vector<pNode> index)
        : Node(t, NODE_ITEM), v(v), index(index) {}

    virtual void print(size_t tabs = 0) override {
        if (v->type == NODE_SYM) {
            v->print(tabs);
        } else {
            printf("(");
            v->print(tabs);
            printf(")");
        }
        for (auto &&i : index) {
            printf("[");
            i->print(tabs);
            printf("]");
        }
    }

    pNode v;
    Vector<pNode> index;
};

pNode new_imm_node(pToken t, ImmValue imm);
pNode new_sym_node(pToken t, Symbol symbol);

pNode new_block_node(pToken t, AstProg body);

pNode new_binary_node(pToken t, BinaryType type, pNode lhs, pNode rhs);
pNode new_unary_node(pToken t, UnaryType type, pNode rhs);
pNode new_cast_node(pToken t, pNode ty, pNode val);
pNode new_ref_node(pToken t, pNode v);
pNode new_deref_node(pToken t, pNode val);
pNode new_item_node(pToken t, pNode val, Vector<pNode> index);

pNode new_basic_type_node(pToken t, pType ty);
pNode new_pointer_type_node(pToken t, pNode n);
pNode new_array_type_node(pToken t, pNode n, pNode index);

pNode new_var_def_node(pToken t, TypedNodeSym var, pNode val,
                       bool is_const = false);
pNode new_func_def_node(pToken t, TypedNodeSym var, Vector<TypedNodeSym> args,
                        pNode body);
pNode new_array_def_node(pToken t, Vector<pNode> nums);

pNode new_assign_node(pToken t, pNode lv, pNode val);
pNode new_if_node(pToken t, pNode cond, pNode body, pNode elsed = {});
pNode new_while_node(pToken t, pNode cond, pNode body);
pNode new_for_node(pToken t, pNode init, pNode cond, pNode exec, pNode body);
pNode new_break_node(pToken t);
pNode new_return_node(pToken t, pNode ret = {});
pNode new_continue_node(pToken t);
pNode new_call_node(pToken t, Symbol name, Vector<pNode> ch);

} // namespace Ast
