#pragma once

#include <utility>

#include "def.h"
#include "imm.h"
#include "type.h"

#include "common_node.h"
#include "common_token.h"

namespace Ast {

struct ImmNode : public Node {
    ImmNode(pToken t, ImmValue imm) : Node(std::move(t), NODE_IMM), imm(imm) {}

    void print(size_t tabs = 0) override { printf("%s", imm.print().c_str()); }

    ImmValue imm;
};

struct SymNode : public Node {
    SymNode(pToken t, String sym)
        : Node(std::move(t), NODE_SYM), sym(std::move(std::move(sym))) {}

    void print(size_t tabs = 0) override { printf("%s", sym.c_str()); }

    String sym;
};

struct BinaryNode : public Node {
    BinaryNode(pToken t, BinaryType type, pNode lhs, pNode rhs)
        : Node(std::move(t), NODE_BINARY), type(type),
          lhs(std::move(std::move(lhs))), rhs(std::move(std::move(rhs))) {}

    void print(size_t tabs = 0) override {
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
        : Node(std::move(t), NODE_UNARY), type(type),
          ch(std::move(std::move(ch))) {}

    void print(size_t tabs = 0) override {
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
    CallNode(pToken t, String name, Vector<pNode> ch)
        : Node(std::move(t), NODE_CALL), name(std::move(std::move(name))),
          ch(std::move(std::move(ch))) {}

    void print(size_t tabs = 0) override {
        printf("%s(", name.c_str());
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

    String name;
    Vector<pNode> ch;
};

struct AssignNode : public Node {
    AssignNode(pToken t, pNode lv, pNode val)
        : Node(std::move(t), NODE_ASSIGN), lv(std::move(std::move(lv))),
          val(std::move(std::move(val))) {}

    void print(size_t tabs = 0) override {
        lv->print(tabs);
        printf(" = ");
        val->print(tabs);
    }

    pNode lv;
    pNode val;
};

struct BasicTypeNode : public Node {
    BasicTypeNode(pToken t, pType ty)
        : Node(std::move(t), NODE_BASIC_TYPE), ty(std::move(std::move(ty))) {}

    void print(size_t tabs = 0) override {
        printf("%s", ty->type_name().c_str());
    }

    pType ty;
};

struct PointerTypeNode : public Node {
    PointerTypeNode(pToken t, pNode n)
        : Node(std::move(t), NODE_POINTER_TYPE), n(std::move(std::move(n))) {}

    void print(size_t tabs = 0) override { printf("(*)"); }

    pNode n;
};

struct ArrayTypeNode : public Node {
    ArrayTypeNode(pToken t, pNode n, pNode index)
        : Node(std::move(t), NODE_ARRAY_TYPE), n(std::move(std::move(n))),
          index(std::move(std::move(index))) {}

    void print(size_t tabs = 0) override {
        printf("[");
        n->print();
        printf("]");
    }

    pNode n;
    pNode index;
};

struct VarDefNode : public Node {
    VarDefNode(pToken t, TypedNodeSym var, pNode val, bool is_const = false)
        : Node(std::move(t), NODE_DEF_VAR), var(std::move(std::move(var))),
          val(std::move(std::move(val))), is_const(is_const) {}

    void print(size_t tabs = 0) override {
        if (is_const) {
            printf("const ");
        }
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
        : Node(std::move(t), NODE_DEF_FUNC), var(std::move(std::move(var))),
          args(std::move(std::move(args))), body(std::move(std::move(body))) {}

    void print(size_t tabs = 0) override {
        var.n->print(tabs);
        printf(" %s(", var.name.c_str());
        bool first = true;
        for (auto &&i : args) {
            if (first) {
                first = false;
            } else {
                printf(", ");
            }
            i.n->print(tabs);
            printf(" %s", i.name.c_str());
        }
        printf(") ");
        body->print(tabs);
    }

    TypedNodeSym var;
    Vector<TypedNodeSym> args;
    pNode body;
};

struct BlockNode : public Node {
    BlockNode(pToken t, AstProg body)
        : Node(std::move(t), NODE_BLOCK), body(std::move(std::move(body))) {}

    void print(size_t tabs = 0) override {
        printf("{\n");
        ++tabs;
        for (auto &&i : body) {
            for (size_t j = 0; j < tabs; ++j) {
                printf("    ");
            }
            i->print(tabs);
            if (i->type != NODE_BLOCK) {
                putchar(';');
            }
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
        : Node(std::move(t), NODE_IF), cond(std::move(std::move(cond))),
          body(std::move(std::move(body))), elsed(std::move(std::move(elsed))) {
    }

    void print(size_t tabs = 0) override {
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
        : Node(std::move(t), NODE_WHILE), cond(std::move(std::move(cond))),
          body(std::move(std::move(body))) {}

    void print(size_t tabs = 0) override {
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
        : Node(std::move(t), NODE_FOR), init(std::move(std::move(init))),
          cond(std::move(std::move(cond))), exec(std::move(std::move(exec))),
          body(std::move(std::move(body))) {}

    void print(size_t tabs = 0) override {
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
    BreakNode(pToken t) : Node(std::move(t), NODE_BREAK) {}

    void print(size_t tabs = 0) override { printf("break"); }
};

struct ContinueNode : public Node {
    ContinueNode(pToken t) : Node(std::move(t), NODE_CONTINUE) {}
    void print(size_t tabs = 0) override { printf("continue"); }
};

struct ReturnNode : public Node {
    ReturnNode(pToken t, pNode ret = {})
        : Node(std::move(t), NODE_RETURN), ret(std::move(std::move(ret))) {}

    void print(size_t tabs = 0) override {
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
        : Node(std::move(t), NODE_ARRAY_VAL), nums(std::move(std::move(nums))) {
    }

    void print(size_t tabs = 0) override {
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
        : Node(std::move(t), NODE_CAST), ty(std::move(std::move(ty))),
          val(std::move(std::move(val))) {}

    void print(size_t tabs = 0) override {
        printf("(");
        ty->print(tabs);
        printf(")");
        val->print(tabs);
    }

    pNode ty;
    pNode val;
};

struct RefNode : public Node {
    RefNode(pToken t, pNode v)
        : Node(std::move(t), NODE_REF), v(std::move(std::move(v))) {}

    void print(size_t tabs = 0) override { printf("&"); }

    pNode v;
};

struct DerefNode : public Node {
    DerefNode(pToken t, pNode val)
        : Node(std::move(t), NODE_DEREF), val(std::move(std::move(val))) {}

    void print(size_t tabs = 0) override { printf("*"); }

    pNode val;
};

struct ItemNode : public Node {
    ItemNode(pToken t, pNode v, Vector<pNode> index)
        : Node(std::move(t), NODE_ITEM), v(std::move(std::move(v))),
          index(std::move(std::move(index))) {}

    void print(size_t tabs = 0) override {
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
pNode new_sym_node(pToken t, String str);

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
pNode new_call_node(pToken t, String name, Vector<pNode> ch);

} // namespace Ast
