#include "convert_ast_to_ir.h"

#include "ast_node.h"
#include "common_node.h"
#include "def.h"
#include "env.h"
#include "ir_call_instr.h"
#include "ir_cast_instr.h"
#include "ir_cmp_instr.h"
#include "ir_constant.h"
#include "ir_control_instr.h"
#include "ir_func.h"
#include "ir_func_defined.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_opr_instr.h"
#include "ir_ptr_instr.h"

#include "imm.h"
#include "ir_val.h"
#include "type.h"
#include "value.h"
#include <cstddef>
#include <memory>

#include <functional>
#include <utility>

#define BUILTIN_INITIALIZER "__buildin_initializer"

namespace AstToIr {

void Convertor::throw_error(const pNode &root, int id, const String &msg) {
    root->throw_error(id, "Convertor", msg);
}

Ir::pModule Convertor::module() const { return _mod; }

bool Convertor::current_block_end() const {
    return _cur_ctx.body.back()->is_terminator();
}

Ir::pInstr Convertor::add_instr(Ir::pInstr instr) {
    _cur_ctx.body.push_back(instr);
    return instr;
}

Ir::pVal Convertor::find_left_value(const pNode &root, const String &sym,
                                    bool request_not_const) {
    Ir::pVal found_instr;
    if (_env.env()->count(sym)) { // local
        MaybeConstInstr found = _env.env()->find(sym);
        // const value is error
        if (!is_pointer(found.instr->ty) ||
            (request_not_const && found.is_const)) {
            throw_error(root, 10, "assignment to a local const value");
        }
        // no-const value should be loaded
        found_instr = found.instr;
    } else { // global
        for (const auto &i : module()->globs) {
            if (i->name() == "@" + sym) {
                if (request_not_const && i->is_const) {
                    throw_error(root, 11, "assignment to a global const value");
                }
                found_instr = i;
                break;
            }
        }
    }
    if (!found_instr) {
        printf("Warning: left-value \"%s\" not found.\n", sym.c_str());
        throw_error(root, 1, "left value cannot be found");
    }
    // printf("found %s %s\n", found_instr->ty->type_name(), sym);
    return found_instr;
}

Ir::pVal Convertor::cast_to_type(const pNode &root, Ir::pVal val,
                                 const pType &ty) {
    if (is_same_type(val->ty, ty)) {
        return val;
    }
    // decay
    if (is_pointer(ty) && is_pointer(val->ty) &&
        is_same_type(to_pointed_type(ty),
                     to_elem_type(to_pointed_type(val->ty)))) {
        // printf("found %s decay from %s to %s\n", val->name(),
        // val->ty->type_name(), ty->type_name());
        auto imm = _cur_ctx.cpool.add(ImmValue(0));
        auto r = Ir::make_item_instr(val, {imm});
        if (!is_same_type(ty, r->ty)) {
            throw_error(root, -1, "decay error?");
        }
        add_instr(r);
        return r;
    }
    if (!is_castable(val->ty, ty)) {
        printf("Message: not castable from %s to %s\n",
               val->ty->type_name().c_str(), ty->type_name().c_str());
        throw_error(root, 13, "[Convertor] error 13: not castable");
    }
    // 第零种转换：转化为 bool
    // 这个转换因为不是 CastInstr 而是 CmpInstr，所以必须放到这里来判断
    if (is_basic_type(val->ty) && is_basic_type(ty) &&
        (to_basic_type(ty)->ty == IMM_I1 || to_basic_type(ty)->ty == IMM_U1)) {
        auto imm =_cur_ctx.cpool.add(ImmValue(to_basic_type(val->ty)->ty));
        bool is_flt = is_float(val->ty);
        return add_instr(Ir::make_cmp_instr(is_flt ? Ir::CMP_UNE: Ir::CMP_NE, val, imm));
    }
    auto r = Ir::make_cast_instr(ty, val);
    add_instr(r);
    return r;
}

pType Convertor::analyze_type(const pNode &root) {
    switch (root->type) {
    case NODE_ARRAY_TYPE: {
        auto r = std::dynamic_pointer_cast<Ast::ArrayTypeNode>(root);
        return make_array_type(analyze_type(r->n),
                               constant_eval(r->index).val.uval);
    }
    case NODE_BASIC_TYPE:
        return std::dynamic_pointer_cast<Ast::BasicTypeNode>(root)->ty;
    case NODE_POINTER_TYPE:
        return make_pointer_type(analyze_type(
            std::dynamic_pointer_cast<Ast::PointerTypeNode>(root)->n));
    default:
        break;
    }
    throw_error(root, 28, "not a type");
    return {};
}

Ir::pVal Convertor::generate_and(const pNode &A, const pNode &B) {
    Ir::pVal a = analyze_value(A);
    Ir::pVal b = analyze_value(B);
    if (is_same_type(a->ty, b->ty) && is_integer(a->ty)) {
        return add_instr(Ir::make_binary_instr(Ir::INSTR_AND, a, b));
    }
    Ir::pVal xa = cast_to_type(A, a, make_basic_type(IMM_I1));
    Ir::pVal xb = cast_to_type(B, b, make_basic_type(IMM_I1));
    return add_instr(Ir::make_binary_instr(Ir::INSTR_AND, a, b));
}

Ir::pVal Convertor::generate_shortcut_and(const pNode &A, const pNode &B) {
    /* A and B that has shortcut
        %1 = alloca i1
        %2 = icmp ne A, 0
        br i1 %2, L2, L1
        L1:
            store %1, 0
            br L3
        L2:
            %3 = icmp ne B, 0
            store %1, %3
            br L3
        L3:
    */
    auto a0 = Ir::make_alloc_instr(make_basic_type(IMM_I1));
    auto L1 = Ir::make_label_instr();
    auto L2 = Ir::make_label_instr();
    auto L3 = Ir::make_label_instr();
    auto a1 = cast_to_type(A, analyze_value(A), make_basic_type(IMM_I1));
    
    auto constant = _cur_ctx.cpool.add(ImmValue(0ll, IMM_I1));

    add_instr(a0);
    add_instr(Ir::make_br_cond_instr(a1, L2, L1));
    add_instr(L1);
    add_instr(Ir::make_store_instr(a0, constant));
    add_instr(Ir::make_br_instr(L3));
    add_instr(L2);
    
    auto a2 = analyze_value(B);
    add_instr(Ir::make_store_instr(
        a0, cast_to_type(B, a2, make_basic_type(IMM_I1))));
    add_instr(Ir::make_br_instr(L3));
    add_instr(L3);

    return add_instr(Ir::make_load_instr(a0));
}

Ir::pVal Convertor::generate_or(const pNode &A, const pNode &B) {
    Ir::pVal a = analyze_value(A);
    Ir::pVal b = analyze_value(B);
    if (is_same_type(a->ty, b->ty) && is_integer(a->ty)) {
        return add_instr(Ir::make_binary_instr(Ir::INSTR_OR, a, b));
    }
    Ir::pVal xa = cast_to_type(A, a, make_basic_type(IMM_I1));
    Ir::pVal xb = cast_to_type(B, b, make_basic_type(IMM_I1));
    return add_instr(Ir::make_binary_instr(Ir::INSTR_OR, a, b));
}

Ir::pVal Convertor::generate_shortcut_or(const pNode &A, const pNode &B) {
    /* A or B that has shortcut
        %1 = alloca i1
        %2 = icmp ne A, 0
        br i1 %2, L1, L2
        L1:
            store %1, 1
            br L3
        L2:
            %3 = icmp ne B, 0
            store %1, %3
            br L3
        L3:
    */
    auto a0 = Ir::make_alloc_instr(make_basic_type(IMM_I1));
    auto L1 = Ir::make_label_instr();
    auto L2 = Ir::make_label_instr();
    auto L3 = Ir::make_label_instr();
    auto a1 = cast_to_type(A, analyze_value(A), make_basic_type(IMM_I1));
    
    auto constant = _cur_ctx.cpool.add(ImmValue(1LL, IMM_I1));
    
    add_instr(a0);
    add_instr(Ir::make_br_cond_instr(a1, L1, L2));
    add_instr(L1);
    add_instr(Ir::make_store_instr(a0, constant));
    add_instr(Ir::make_br_instr(L3));
    add_instr(L2);
    auto a2 = analyze_value(B);
    add_instr(Ir::make_store_instr(
        a0, cast_to_type(B, a2, make_basic_type(IMM_I1))));
    add_instr(Ir::make_br_instr(L3));
    add_instr(L3);

    return add_instr(Ir::make_load_instr(a0));
}

Ir::pVal Convertor::analyze_opr(const Pointer<Ast::BinaryNode> &root) {
    switch (root->type) {
    case OPR_ADD:
    case OPR_SUB:
    case OPR_MUL:
    case OPR_DIV:
    case OPR_REM: {
        auto a1 = analyze_value(root->lhs);
        auto a2 = analyze_value(root->rhs);
        auto joined_type = join_type(a1->ty, a2->ty);
        if (!joined_type) {
            printf("Warning: try to join %s and %s.\n",
                   a1->ty->type_name().c_str(), a2->ty->type_name().c_str());
            throw_error(root, 18, "type has no joined type");
        }
        auto ir =
            Ir::make_binary_instr(fromBinaryOpr(root, joined_type),
                                  cast_to_type(root->lhs, a1, joined_type),
                                  cast_to_type(root->rhs, a2, joined_type));
        add_instr(ir);
        return ir;
    }
    case OPR_AND:
        if (root->rhs->type == NODE_SYM) {
            return generate_and(root->lhs, root->rhs);
        }
        return generate_shortcut_and(root->lhs, root->rhs);
    case OPR_OR:
        if (root->rhs->type == NODE_SYM) {
            return generate_or(root->lhs, root->rhs);
        }
        return generate_shortcut_or(root->lhs, root->rhs);
    case OPR_EQ:
    case OPR_NE:
    case OPR_GT:
    case OPR_GE:
    case OPR_LT:
    case OPR_LE: {
        auto a1 = analyze_value(root->lhs);
        auto a2 = analyze_value(root->rhs);
        auto ty = join_type(a1->ty, a2->ty);
        if (!ty) {
            throw_error(root, 18, "type has no joined type");
        }
        auto ir = Ir::make_cmp_instr(fromCmpOpr(root, ty),
                                     cast_to_type(root->lhs, a1, ty),
                                     cast_to_type(root->rhs, a2, ty));
        add_instr(ir);
        return ir;
    }
    default:
        throw_error(root, 2, "operation not implemented");
        return Ir::make_empty_instr();
    }
}

ImmValue Convertor::find_const_value(const pNode &root, const String &sym) {
    if (!_const_env.env()->count(sym)) {
        printf("Warning: couldn't find symbol %s\n", sym.c_str());
        throw_error(root, 29, "not a constant");
    }
    return _const_env.env()->find(sym);
}

ImmValue Convertor::constant_eval(const pNode &node) {
    switch (node->type) {
    case NODE_SYM: {
        auto name = std::dynamic_pointer_cast<Ast::SymNode>(node)->sym;
        return find_const_value(node, name);
    }
    case NODE_IMM:
        return std::dynamic_pointer_cast<Ast::ImmNode>(node)->imm;
    case NODE_UNARY: {
        auto r = std::dynamic_pointer_cast<Ast::UnaryNode>(node);
        auto imm = constant_eval(r->ch);
        switch (r->type) {
        case OPR_NOT:
            return !imm;
        case OPR_NEG:
            return -imm;
        case OPR_POS:
            break;
        }
        return imm;
    }
    case NODE_BINARY: {
        auto r = std::dynamic_pointer_cast<Ast::BinaryNode>(node);
        auto lc = constant_eval(r->lhs);
        auto rc = constant_eval(r->rhs);
        switch (r->type) {
        case OPR_ADD:
            return lc + rc;
        case OPR_DIV:
            return lc / rc;
        case OPR_EQ:
            return lc == rc;
        case OPR_GE:
            return lc >= rc;
        case OPR_GT:
            return lc > rc;
        case OPR_LE:
            return lc <= rc;
        case OPR_LT:
            return lc < rc;
        case OPR_MUL:
            return lc * rc;
        case OPR_NE:
            return lc != rc;
        case OPR_REM:
            return lc % rc;
        case OPR_SUB:
            return lc - rc;
        case OPR_AND:
            return lc && rc;
        case OPR_OR:
            return lc || rc;
        }
    }
    default:
        break;
    }
    throw_error(node, 1, "not implemented");
    return 0;
}

Ir::pVal Convertor::analyze_left_value(const pNode &root,
                                       bool request_not_const) {
    switch (root->type) {
    case NODE_ITEM: {
        /*
        int fun(int a[]) { // i32* %a
            // %0 = alloca i32* 0           | %0 -> int**
            // store i32* %a, i32** %0
            int b[2] = {0};
            // %1 = alloca [2 x i32]        | %1 -> [2 x i32]*
            a[1]:   %2 = load i32*, i32** %0
                    %3 = gep i32, i32* %1, i64 1
            b[1]:   %2 = gep [2 x i32], [2 x i32]* %1, i64 0, i64 1
        }
        */
        auto r = std::dynamic_pointer_cast<Ast::ItemNode>(root);
        auto array = analyze_value(r->v, request_not_const);
        if (!is_pointer(array->ty)) {
            printf("Message: type is %s\n", array->ty->type_name().c_str());
            throw_error(r->v, 25, "not an array");
        }
        // printf("Array Type: %s\n", array->ty->type_name());
        Vector<Ir::pVal> indexs;
        for (const auto &i : r->index) {
            indexs.push_back(analyze_value(i));
            if (!is_integer(indexs.back()->ty)) {
                throw_error(i, 14, "type of index should be integer");
            }
        }
        auto itemptr = Ir::make_item_instr(array, indexs);
        add_instr(itemptr);
        return itemptr;
    }
    case NODE_SYM: {
        auto r = std::dynamic_pointer_cast<Ast::SymNode>(root);
        return find_left_value(r, r->sym, request_not_const);
    }
    case NODE_DEREF: {
        /*
            int x;          | x: int*
            int *y = &x;    | y: int**
            *y = 1;         | %1 = load i32** y
                            | store i32 1, i32* %1
        */
        auto r = std::dynamic_pointer_cast<Ast::DerefNode>(root);
        return add_instr(Ir::make_load_instr(analyze_left_value(r->val)));
    }
    default:
        throw_error(root, 22, "left value needed");
        break;
    }
    return Ir::make_empty_instr();
}

Ir::pVal Convertor::analyze_value(const pNode &root, bool request_not_const) {
    switch (root->type) {
    case NODE_DEREF:
    case NODE_ITEM:
    case NODE_SYM: {
        auto lv = analyze_left_value(root, request_not_const);
        // printf("analyze value: from lv %s\n", lv->ty->type_name());
        if (is_array(to_pointed_type(lv->ty))) { // 数组不能解引用传递
            return lv;
        }
        return add_instr(Ir::make_load_instr(lv));
    }
    case NODE_REF: {
        auto r = std::dynamic_pointer_cast<Ast::RefNode>(root);
        return analyze_left_value(r->v, true);
    }
    case NODE_CAST: {
        auto r = std::dynamic_pointer_cast<Ast::CastNode>(root);
        auto res =
            Ir::make_cast_instr(analyze_type(r->ty), analyze_value(r->val));
        add_instr(res);
        return res;
    }
    case NODE_BINARY: {
        auto ir = analyze_opr(std::dynamic_pointer_cast<Ast::BinaryNode>(root));
        return ir;
    }
    case NODE_UNARY: {
        auto r = std::dynamic_pointer_cast<Ast::UnaryNode>(root);
        auto ch = r->ch;
        auto val = analyze_value(ch);
        switch (r->type) {
        case OPR_POS: {
            return val;
        }
        case OPR_NOT: {
            auto imm_ty = to_basic_type(val->ty)->ty;
            auto imm = _cur_ctx.cpool.add(ImmValue(0).cast_to(imm_ty));
            return add_instr(Ir::make_cmp_instr(
                is_imm_integer(imm_ty) ? Ir::CMP_EQ : Ir::CMP_OEQ, val, imm));
        }
        case OPR_NEG: {
            if (is_float(val->ty)) {
                return add_instr(Ir::make_unary_instr(val));
            }
            if (is_integer(val->ty)) {
                auto imm = _cur_ctx.cpool.add(ImmValue(0));
                auto ty = join_type(val->ty, imm->ty);
                return add_instr(Ir::make_binary_instr(
                    Ir::INSTR_SUB, cast_to_type(root, imm, ty),
                    cast_to_type(root, val, ty)));
            }
            throw_error(root, 21, "neg an non-number");
        }
        }
    }
    case NODE_IMM: {
        auto r = std::dynamic_pointer_cast<Ast::ImmNode>(root);
        return _cur_ctx.cpool.add(r->imm);
    }
    case NODE_CALL: {
        auto r = std::dynamic_pointer_cast<Ast::CallNode>(root);
        if (!func_count(r->name)) {
            printf("Warning: function %s not found\n", r->name.c_str());
            throw_error(r, 20, "function not found");
        }
        auto func = find_func(r->name);
        Vector<Ir::pVal> args;
        // assert size equal
        size_t length = func->function_type()->arg_type.size();
        if (!func->variant_length && length != r->ch.size()) {
            throw_error(root, 8, "wrong count of arguments");
        }
        for (size_t i = 0; i < r->ch.size(); ++i) {
            Ir::pVal cur_arg = analyze_value(r->ch[i]);
            if (i < length) {
                args.push_back(cast_to_type(
                    r->ch[i], cur_arg, func->function_type()->arg_type[i]));
            } else {
                args.push_back(cur_arg);
            }
        }
        return add_instr(Ir::make_call_instr(func, args));
    }
    default:
        throw_error(root, 3, "node not calculatable");
    }
    return Ir::make_empty_instr();
}

Vector<pNode> flatten(InitializerVisitor &list,
                      const Pointer<ArrayType> &type) {
    Vector<pNode> ans;
    size_t size = type->elem_count;
    pType elem_ty = type->elem_type;

    if (elem_ty->type_type() == TYPE_BASIC_TYPE) {
        for (size_t i = 0; i < size; ++i) {
            auto elem = list.get();
            if (!elem && list.is_root) {
                break;
            }
            ans.push_back(elem); // possibly nullptr
        }
        return ans;
    }

    for (size_t i = 0; i < size; ++i) {
        auto elem = list.peek();
        if (!elem && list.is_root) {
            break;
        }
        if (elem && elem->type == NODE_ARRAY_VAL) {
            elem = list.get();
            InitializerVisitor new_v(
                std::dynamic_pointer_cast<Ast::ArrayDefNode>(elem));
            Vector<pNode> inner =
                flatten(new_v, std::dynamic_pointer_cast<ArrayType>(elem_ty));
            ans.insert(ans.end(), inner.begin(), inner.end());
        } else {
            Vector<pNode> inner =
                flatten(list, std::dynamic_pointer_cast<ArrayType>(elem_ty));
            ans.insert(ans.end(), inner.begin(), inner.end());
        }
    }

    return ans;
}

Vector<pNode> flatten(const Pointer<Ast::ArrayDefNode> &initializer,
                      const Pointer<ArrayType> &type) {
    InitializerVisitor new_v(initializer, true);
    return flatten(new_v, type);
}

void Convertor::copy_to_array(pNode root, Ir::pVal addr,
                              const Pointer<ArrayType> &t,
                              const Pointer<Ast::ArrayDefNode> &n,
                              bool constant) {
    if (!is_pointer(addr->ty)) {
        throw_error(root, 16, "list should initialize type that is pointed");
    }
    Vector<pNode> flat = flatten(n, t);
    Vector<Ir::pVal> indexes;
    auto begin = flat.begin();
    auto end = flat.end();
    std::function<void(pType t)> fn = [&](const pType &t) -> void {
        if (begin == end) {
            return;
        }
        if (is_basic_type(t)) {
            if (constant) {
                auto cur = *(begin++);
                if (!cur) {
                    return;
                }
                auto val = constant_eval(cur);
                if (!val) {
                    return;
                }
                auto item = Ir::make_item_instr(addr, indexes);
                auto imm = _cur_ctx.cpool.add(val);
                add_instr(item);
                auto store =
                    Ir::make_store_instr(item, cast_to_type(root, imm, t));
                add_instr(store);
                return;
            }
            auto cur = *(begin++);
            if (!cur) {
                return;
            }
            auto val = analyze_value(cur);
            auto item = Ir::make_item_instr(addr, indexes);
            add_instr(item);
            auto store = Ir::make_store_instr(item, cast_to_type(root, val, t));
            add_instr(store);
            return;
        }
        size_t length = to_array_type(t)->elem_count;
        pType next_ty = to_array_type(t)->elem_type;
        for (size_t i = 0; i < length; ++i) {
            auto index = _cur_ctx.cpool.add(
                ImmValue(static_cast<unsigned long long>(i), IMM_I32));
            indexes.push_back(index);
            fn(next_ty);
            indexes.pop_back();
        }
    };
    fn(t);
}

void Convertor::analyze_simple_if(const pNode &cond, const Ir::pVal &to, bool use_eq)
{
    Ir::pVal val = analyze_value(cond);
    if (use_eq) {
        Ir::pVal cast_i1 = cast_to_type(cond, val, make_basic_type(IMM_I1));
        Ir::pVal cast_ty = cast_to_type(cond, cast_i1, to_pointed_type(to->ty));
        add_instr(Ir::make_store_instr(to, cast_ty));
    } else {
        ImmType ty = to_basic_type(to->ty)->ty;
        Ir::pVal zero = _cur_ctx.cpool.add(ImmValue(ty));
        Ir::pVal cast_i1 = Ir::make_cmp_instr(is_imm_float(ty) ? Ir::CMP_UNE : Ir::CMP_NE,
                                         val,
                                         zero);
        Ir::pVal cast_ty = cast_to_type(cond, cast_i1, to_pointed_type(to->ty));
        add_instr(Ir::make_store_instr(to, cast_ty));
    }
}

Pointer<Ast::AssignNode> try_get_lv(const pNode &p)
{
    if (p->type == NODE_ASSIGN) {
        return std::dynamic_pointer_cast<Ast::AssignNode>(p);
    }
    if (p->type == NODE_BLOCK) {
        AstProg &body = std::dynamic_pointer_cast<Ast::BlockNode>(p)->body;
        if (body.size() == 1) {
            return try_get_lv(body.front());
        }
    }
    return nullptr;
}

int use_which(Pointer<Ast::AssignNode> mA, Pointer<Ast::AssignNode> mB) {
    if (!mA || !mB)
        return -1;
    if (mA->lv->type != NODE_SYM || mB->lv->type != NODE_SYM ||
        std::dynamic_pointer_cast<Ast::SymNode>(mA->lv)->sym
         != std::dynamic_pointer_cast<Ast::SymNode>(mB->lv)->sym)
        return -1;
    if (mA->val->type != NODE_IMM || mB->val->type != NODE_IMM)
        return -1;
    ImmValue imma = std::dynamic_pointer_cast<Ast::ImmNode>(mA->val)->imm;
    ImmValue immb = std::dynamic_pointer_cast<Ast::ImmNode>(mB->val)->imm;
    if (!is_imm_integer(imma.ty) || !is_imm_integer(immb.ty))
        return -1;
    if (imma.val.ival == 0 && immb.val.ival == 1) {
        return 0;
    }
    if (imma.val.ival == 1 && immb.val.ival == 0) {
        return 1;
    }
    return -1;
}

bool Convertor::analyze_if(const Pointer<Ast::IfNode> &r) {
    // judge whether it is a simple if
    // if (...) { SOME = 1; } else { SOME = 0; }
    // if (...) { SOME = 0; } else { SOME = 1; }
    if (r->elsed) {
        Pointer<Ast::AssignNode> mA, mB;

        mA = try_get_lv(r->body);
        mB = try_get_lv(r->elsed);

        int res = use_which(mA, mB);

        if (res != -1) {
            analyze_simple_if(r->cond, analyze_left_value(mA->lv), res);
            return false;
        }
    }

    // it is not a simple if
    bool is_end = false;
    if (r->elsed) {
        /*
            br xxx IF_BEGIN, ELSE_BEGIN
            IF_BEGIN:
                xxx
                GOTO ELSE_END:
            IF_END/ELSE_BEGIN:
                YYY
                GOTO ELSE_END:
            ELSE_END:
        */

        auto if_begin = Ir::make_label_instr();
        auto if_end = Ir::make_label_instr();
        auto if_else_end = Ir::make_label_instr();
        bool need_label = false;
        add_instr(Ir::make_br_cond_instr(
            cast_to_type(r, analyze_value(r->cond), make_basic_type(IMM_I1)),
            if_begin, if_end));
        add_instr(if_begin);
        analyze_statement_node(r->body);
        if ((static_cast<unsigned int>(!_cur_ctx.body.empty()) != 0U) &&
            !current_block_end()) {
            add_instr(Ir::make_br_instr(if_else_end));
            need_label = true;
        }
        add_instr(if_end);
        analyze_statement_node(r->elsed);
        if ((static_cast<unsigned int>(!_cur_ctx.body.empty()) != 0U) &&
            !current_block_end()) {
            add_instr(Ir::make_br_instr(if_else_end));
            need_label = true;
        }
        if (need_label) {
            add_instr(if_else_end);
        } else {
            is_end = true;
        }
    } else {
        /*
            br xxx IF_BEGIN, IF_END
            IF_BEGIN:
                xxx
                GOTO IF_END:
            IF_END:
        */
        auto if_begin = Ir::make_label_instr();
        auto if_end = Ir::make_label_instr();
        add_instr(Ir::make_br_cond_instr(
            cast_to_type(r, analyze_value(r->cond), make_basic_type(IMM_I1)),
            if_begin, if_end));
        add_instr(if_begin);
        analyze_statement_node(r->body);
        if ((static_cast<unsigned int>(!_cur_ctx.body.empty()) != 0U) &&
            !current_block_end()) {
            add_instr(Ir::make_br_instr(if_end));
        }
        add_instr(if_end);
    }
    return is_end;
}

bool Convertor::analyze_statement_node(const pNode &root) {
    bool is_end = false;
    switch (root->type) {
    case NODE_ARRAY_TYPE:
    case NODE_BASIC_TYPE:
    case NODE_POINTER_TYPE: {
        throw_error(root, 27, "single type");
    }
    case NODE_ASSIGN: {
        /* reject const:
            1.
                const int a = 10;
                a = 3; // reject
            2.
                const int a = 10;
                int *b = &a; // reject
        */
        auto r = std::dynamic_pointer_cast<Ast::AssignNode>(root);
        auto lv = analyze_left_value(r->lv, true);
        if (is_array(to_pointed_type(lv->ty))) {
            throw_error(r->lv, 23, "array cannot be left value");
        }
        auto rv = analyze_value(r->val);
        add_instr(Ir::make_store_instr(
            lv, cast_to_type(r->val, rv, to_pointed_type(lv->ty))));
        break;
    }
    case NODE_DEF_VAR: {
        auto r = std::dynamic_pointer_cast<Ast::VarDefNode>(root);
        Ir::pInstr tmp;
        auto ty = analyze_type(r->var.n);
        if (is_array(ty)) {
            add_instr(tmp = Ir::make_alloc_instr(ty));
            _env.env()->set(r->var.name, {tmp, r->is_const});
            if (r->val) {
                if (r->val->type != NODE_ARRAY_VAL) {
                    throw_error(r->val, 17,
                                "array should be initialized by a list");
                }
                auto rrr = std::dynamic_pointer_cast<Ast::ArrayDefNode>(r->val);
                auto len = to_array_type(ty)->length();
                assert(len % 4 == 0);
                auto imm = _cur_ctx.cpool.add(ImmValue(static_cast<unsigned long long>(len / 4), IMM_I32));
                add_instr(Ir::make_call_instr(
                    find_func("__builtin_fill_zero"),
                    {cast_to_type(r, tmp,
                                  make_pointer_type(make_basic_type(IMM_I8))),
                      imm}));
                copy_to_array(r, tmp, to_array_type(ty), rrr, false);
            }
            break;
        }
        add_instr(tmp = Ir::make_alloc_instr(ty));
        if (r->val) {
            if (r->is_const) {
                _const_env.env()->set(
                    r->var.name,
                    constant_eval(r->val).cast_to(to_basic_type(ty)->ty));
                // printf("[1] Set Const Value: %s\n",
                // _const_env.env()->find(r->var.name).print());
            }
            add_instr(Ir::make_store_instr(
                tmp, cast_to_type(r->val, analyze_value(r->val), ty)));
        }
        _env.env()->set(r->var.name, {tmp, r->is_const});
        break;
    }
    case NODE_IF: {
        return analyze_if(std::dynamic_pointer_cast<Ast::IfNode>(root));
    }
    case NODE_WHILE: {
        auto r = std::dynamic_pointer_cast<Ast::WhileNode>(root);
        /*
            br while_cond
            while_cond:
                br ... LABEL while_begin, LABEL while_end
            while_begin:
                ...
                br while_cond
            while_end:
        */
        auto while_cond = Ir::make_label_instr();
        auto while_begin = Ir::make_label_instr();
        auto while_end = Ir::make_label_instr();
        add_instr(Ir::make_br_instr(while_cond));
        add_instr(while_cond);
        auto cond =
            cast_to_type(r, analyze_value(r->cond), make_basic_type(IMM_I1));
        add_instr(Ir::make_br_cond_instr(cond, while_begin, while_end));
        add_instr(while_begin);
        push_loop_env(while_cond, while_end);
        analyze_statement_node(r->body);
        // add_instr(Ir::make_br_instr(while_cond));
        if ((static_cast<unsigned int>(!_cur_ctx.body.empty()) != 0U) &&
            !current_block_end()) {
            add_instr(Ir::make_br_instr(
                while_cond)); // who will write "while(xxx) return 0"?
        }
        add_instr(while_end);
        end_loop_env();
        break;
    }
    case NODE_FOR: {
        /*
                some_init...
                br for_cond
            for_cond:
                some_program...
                br ... LABEL for_body, LABEL for_end
            for_body:
                some_program...
                br for_exec
            for_exec:
                some_exec...
                br for_cond
            for_end:
        */
        auto r = std::dynamic_pointer_cast<Ast::ForNode>(root);
        auto for_cond = Ir::make_label_instr();
        auto for_body = Ir::make_label_instr();
        auto for_exec = Ir::make_label_instr();
        auto for_end = Ir::make_label_instr();
        _env.push_env();
        _const_env.push_env();
        push_loop_env(for_exec, for_end); // continue start from for_exec
        analyze_statement_node(r->init);
        add_instr(Ir::make_br_instr(for_cond));
        add_instr(for_cond);
        auto cond = analyze_value(r->cond);
        add_instr(Ir::make_br_cond_instr(cond, for_body, for_end));
        add_instr(for_body);
        analyze_statement_node(r->body);
        add_instr(Ir::make_br_instr(for_exec));
        add_instr(for_exec);
        analyze_statement_node(r->exec);
        add_instr(Ir::make_br_instr(for_cond));
        add_instr(for_end);
        end_loop_env();
        _const_env.end_env();
        _env.end_env();
        break;
    }
    case NODE_BREAK:
        if (!has_loop_env()) {
            throw_error(root, 9, "no outer loops");
        }
        add_instr(Ir::make_br_instr(loop_env()->loop_end));
        add_instr(Ir::make_label_instr());
        break;
    case NODE_CONTINUE:
        if (!has_loop_env()) {
            throw_error(root, 9, "no outer loops");
        }
        add_instr(Ir::make_br_instr(loop_env()->loop_begin));
        add_instr(Ir::make_label_instr());
        break;
    case NODE_RETURN: {
        auto r = std::dynamic_pointer_cast<Ast::ReturnNode>(root);
        if (r->ret) {
            auto res = analyze_value(r->ret);
            add_instr(Ir::make_ret_instr(
                cast_to_type(r, res, _cur_ctx.func_type->ret_type)));
        } else {
            add_instr(Ir::make_ret_instr());
        }
        is_end = true;
        break;
    }
    case NODE_BLOCK: {
        auto r = std::dynamic_pointer_cast<Ast::BlockNode>(root);
        _env.push_env();
        _const_env.push_env();
        for (const auto &i : r->body) {
            bool ret = analyze_statement_node(i);
            if (ret) {
                is_end = true;
                break;
            }
        }
        _const_env.end_env();
        _env.end_env();
        break;
    }
    case NODE_BINARY:
    case NODE_UNARY:
    case NODE_ITEM:
    case NODE_CAST:
    case NODE_IMM:
    case NODE_SYM:
    case NODE_REF:
    case NODE_DEREF:
    case NODE_ARRAY_VAL:
    case NODE_CALL:
        analyze_value(root);
        break;
    case NODE_DEF_FUNC:
        // impossible
        throw_error(root, -1,
                    "statement calculation not implemented - impossible");
    }
    return is_end;
}

Ir::pFuncDefined Convertor::generate_inline_function(const Pointer<Ast::FuncDefNode> &root) {
    my_assert(root, "?");
    _env.push_env();
    _const_env.push_env();
    Ir::pFuncDefined func;
    {
        Vector<pType> types;
        Vector<String> syms;
        for (const auto &i : root->args) {
            auto ty = analyze_type(i.n);
            if (is_array(ty)) {
                printf("Warning: %s is array.\n", i.name.c_str());
                throw_error(root, 24, "array cannot be argument");
            }
            types.push_back(ty);
            syms.push_back(i.name);
        }
        TypedSym root_var(root->var.name, analyze_type(root->var.n));
        func = Ir::make_func_defined(root_var, types, syms);
        func->ast_root = root;
        _cur_ctx.init(func);
        for (size_t i = 0; i < root->args.size(); ++i) {
            /*
                由于 sysy 语法中函数参数没有 const
                所以这里一定是 false
            */
            if (_env.env()->count(root->args[i].name)) {
                printf("Warning: %s repeated.\n", root->args[i].name.c_str());
                throw_error(root, 26, "repeated argument name");
            }
            _env.env()->set(root->args[i].name, {_cur_ctx.args[i], false});
        }
    }
    
    analyze_statement_node(root->body);
    
    func->end_function(_cur_ctx);
    _const_env.end_env();
    _env.end_env();

    return func;
}

void Convertor::generate_function(const Pointer<Ast::FuncDefNode> &root) {
    _env.push_env();
    _const_env.push_env();
    Ir::pFuncDefined func;
    {
        Vector<pType> types;
        Vector<String> syms;
        for (const auto &i : root->args) {
            auto ty = analyze_type(i.n);
            if (is_array(ty)) {
                printf("Warning: %s is array.\n", i.name.c_str());
                throw_error(root, 24, "array cannot be argument");
            }
            types.push_back(ty);
            syms.push_back(i.name);
        }
        TypedSym root_var(root->var.name, analyze_type(root->var.n));
        func = Ir::make_func_defined(root_var, types, syms);
        func->ast_root = root;

        _cur_ctx.init(func);

        for (size_t i = 0; i < root->args.size(); ++i) {
            /*
                由于 sysy 语法中函数参数没有 const
                所以这里一定是 false
            */
            if (_env.env()->count(root->args[i].name)) {
                printf("Warning: %s repeated.\n", root->args[i].name.c_str());
                throw_error(root, 26, "repeated argument name");
            }
            _env.env()->set(root->args[i].name, { _cur_ctx.args[i], false});
        }
        set_func(func->name(), func);
    }
    if (root->var.name == "main") {
        analyze_statement_node(
            Ast::new_call_node(nullptr, BUILTIN_INITIALIZER, {}));
    }

    analyze_statement_node(root->body);

    if (root->var.name == "main") {
        analyze_statement_node(
            Ast::new_return_node(nullptr, Ast::new_imm_node(nullptr, 0)));
    }


    func->end_function(_cur_ctx);
    _const_env.end_env();
    _env.end_env();

    module()->add_func(func);
}

Value Convertor::value_flatten(InitializerVisitor &list,
                      const Pointer<ArrayType> &type) {
    ArrayValue ans;
    ans.ty = type;
    size_t size = type->elem_count;
    pType elem_ty = type->elem_type;

    if (elem_ty->type_type() == TYPE_BASIC_TYPE) {
        for (size_t i = 0; i < size; ++i) {
            auto elem = list.get();
            if (!elem) {
                break;
            }
            ans.values.push_back(make_value(constant_eval(elem)));
        }
        return ans;
    }

    for (size_t i = 0; i < size; ++i) {
        auto elem = list.peek();
        if (!elem) {
            break;
        }
        if (elem && elem->type == NODE_ARRAY_VAL) {
            elem = list.get();
            InitializerVisitor new_v(
                std::dynamic_pointer_cast<Ast::ArrayDefNode>(elem));
            Value inner =
                value_flatten(new_v, std::dynamic_pointer_cast<ArrayType>(elem_ty));
            ans.values.push_back(make_value(inner));
        } else {
            Value inner =
                value_flatten(list, std::dynamic_pointer_cast<ArrayType>(elem_ty));
            ans.values.push_back(make_value(inner));
        }
    }

    return ans;
}

Value Convertor::from_array_def(const Pointer<Ast::ArrayDefNode> &n,
                                const Pointer<ArrayType> &t) {
    InitializerVisitor new_v(n, true);
    return value_flatten(new_v, t);
}

void Convertor::generate_global_var(const Pointer<Ast::VarDefNode> &root) {
    TypedSym root_var = TypedSym(root->var.name, analyze_type(root->var.n));
    if (is_basic_type(root_var.ty)) {
        if (root->val) {
            auto imm = constant_eval(root->val).cast_to(
                to_basic_type(root_var.ty)->ty);
            module()->add_global(
                Ir::make_global(root_var, Value(imm), root->is_const));
            if (root->is_const) {
                _const_env.env()->set(root_var.sym, imm);
                // printf("[2] Set Const Value: %s\n", imm.print());
            }
        } else {
            module()->add_global(
                Ir::make_global(root_var, Value(0), root->is_const));
            if (root->is_const) {
                _const_env.env()->set(root_var.sym, ImmValue(0));
                // printf("[3] Set Const Value: %s\n",
                // _const_env.env()->find(root_var.sym).print());
            }
        }
    } else {
        if (root->val) {
            auto n = std::dynamic_pointer_cast<Ast::ArrayDefNode>(root->val);
            module()->add_global(Ir::make_global(
                root_var,
                from_array_def(n, to_array_type(root_var.ty)), root->is_const));

            add_initialization(
                root_var,
                std::dynamic_pointer_cast<Ast::ArrayDefNode>(root->val));
        } else {
            module()->add_global(Ir::make_global(
                root_var,
                // Value(from_array_def(nullptr, to_array_type(root_var.ty))),
                Value(ArrayValue {{}, root_var.ty}), root->is_const));
        }
    }
}

void Convertor::generate_single(const pNode &root) {
    switch (root->type) {
    case NODE_DEF_FUNC:
        generate_function(std::dynamic_pointer_cast<Ast::FuncDefNode>(root));
        break;
    case NODE_DEF_VAR:
        generate_global_var(std::dynamic_pointer_cast<Ast::VarDefNode>(root));
        break;
    default:
        throw_error(root, 5, "global operation has type that not implemented");
    }
}

Ir::pModule Convertor::generate(const AstProg &asts) {
    _env.clear_env();
    _const_env.clear_env();
    _const_env.push_env();
    clear_loop_env();
    _prog = asts;
    _mod = std::make_shared<Ir::Module>();

    auto i1 = make_basic_type(IMM_I1);
    auto i8 = make_basic_type(IMM_I8);
    auto i8s = make_pointer_type(i8);
    auto i32 = make_basic_type(IMM_I32);
    auto i64 = make_basic_type(IMM_I64);
    auto f32 = make_basic_type(IMM_F32);
    auto vd = make_void_type();

    _initializer_func =
        Ir::make_func_defined(TypedSym(BUILTIN_INITIALIZER, vd), {}, {});
    _mod->add_func(_initializer_func);
    set_func(BUILTIN_INITIALIZER, _initializer_func);
    _init_ctx.init(_initializer_func);

    _mod->add_func_declaration(
        Ir::make_func(TypedSym("_sysy_starttime", vd), {i32}));
    _mod->add_func_declaration(
        Ir::make_func(TypedSym("_sysy_stoptime", vd), {i32}));

    _mod->add_func_declaration(
        Ir::make_func(TypedSym("__builtin_fill_zero", vd), {i8s, i32}));

    _mod->add_func_declaration(Ir::make_func(TypedSym("getint", i32), {}));
    _mod->add_func_declaration(Ir::make_func(TypedSym("getch", i32), {}));
    _mod->add_func_declaration(
        Ir::make_func(TypedSym("getarray", i32), {make_pointer_type(i32)}));
    _mod->add_func_declaration(Ir::make_func(TypedSym("getfloat", f32), {}));
    _mod->add_func_declaration(
        Ir::make_func(TypedSym("getfarray", i32), {make_pointer_type(f32)}));

    _mod->add_func_declaration(Ir::make_func(TypedSym("putint", vd), {i32}));
    _mod->add_func_declaration(Ir::make_func(TypedSym("putch", vd), {i32}));
    _mod->add_func_declaration(
        Ir::make_func(TypedSym("putarray", vd), {i32, make_pointer_type(i32)}));
    _mod->add_func_declaration(Ir::make_func(TypedSym("putfloat", vd), {f32}));
    _mod->add_func_declaration(Ir::make_func(TypedSym("putfarray", vd),
                                             {i32, make_pointer_type(f32)}));

    _mod->add_func_declaration(Ir::make_func(
        TypedSym("putf", vd), {make_pointer_type(i8)}, true)); // 可变

    for (auto &&i : _mod->funsDeclared) {
        set_func(i->name(), i);
    }

    for (const auto &i : asts) {
        generate_single(i);
    }

    _initializer_func->end_function(_init_ctx);

    // _const_env.end_env();
    return _mod;
}

void Convertor::add_initialization(const TypedSym &var,
                                   const Pointer<Ast::ArrayDefNode> &node) {
    EnvWrapper<MaybeConstInstr> env = _env;

    Ir::FunctionContext ctx = _cur_ctx;
    _cur_ctx = _init_ctx;

    _env = EnvWrapper<MaybeConstInstr>();
    _env.push_env();

    auto arr = find_left_value(node, var.sym);
    copy_to_array(node, arr, to_array_type(to_pointed_type(arr->ty)), node,
                  true);

    _env.end_env();
    _env = env;

    _init_ctx = _cur_ctx;
    _cur_ctx = ctx;
}

Ir::BinInstrType Convertor::fromBinaryOpr(const Pointer<Ast::BinaryNode> &root,
                                          const pType &ty) {
    bool is_signed = is_signed_type(ty);
    bool is_flt = is_float(ty);
    switch (root->type) {
    case OPR_ADD:
        return is_flt ? Ir::INSTR_FADD : Ir::INSTR_ADD;
    case OPR_SUB:
        return is_flt ? Ir::INSTR_FSUB : Ir::INSTR_SUB;
    case OPR_MUL:
        return is_flt ? Ir::INSTR_FMUL : Ir::INSTR_MUL;
    case OPR_DIV:
        return is_flt      ? Ir::INSTR_FDIV
               : is_signed ? Ir::INSTR_SDIV
                           : Ir::INSTR_UDIV;
    case OPR_REM:
        return is_flt      ? Ir::INSTR_FREM
               : is_signed ? Ir::INSTR_SREM
                           : Ir::INSTR_UREM;
    default:
        throw_error(
            root, -1,
            "binary operation conversion from ast to ir not implemented");
    }
    return Ir::INSTR_ADD;
}

Ir::CmpType Convertor::fromCmpOpr(const Pointer<Ast::BinaryNode> &root,
                                  const pType &ty) {
    bool is_signed = is_signed_type(ty);
    bool is_flt = is_float(ty);
    switch (root->type) {
    case OPR_EQ: {
        return is_flt ? Ir::CMP_OEQ : Ir::CMP_EQ;
    }
    case OPR_NE: {
        return is_flt ? Ir::CMP_UNE : Ir::CMP_NE;
    }
    case OPR_LE: {
        return is_flt ? Ir::CMP_OLE : is_signed ? Ir::CMP_SLE : Ir::CMP_ULE;
    }
    case OPR_GE: {
        return is_flt ? Ir::CMP_OGE : is_signed ? Ir::CMP_SGE : Ir::CMP_UGE;
    }
    case OPR_LT: {
        return is_flt ? Ir::CMP_OLT : is_signed ? Ir::CMP_SLT : Ir::CMP_ULT;
    }
    case OPR_GT: {
        return is_flt ? Ir::CMP_OGT : is_signed ? Ir::CMP_SGT : Ir::CMP_UGT;
    }
    default:
        throw_error(
            root, 7,
            "comparasion operation conversion from ast to ir not implemented");
    }
    return Ir::CMP_EQ;
}

pLoopEnv Convertor::loop_env() { return _loop_env_stack.top(); }

void Convertor::push_loop_env(Ir::pInstr begin, Ir::pInstr end) {
    _loop_env_stack.push(
        std::make_shared<LoopEnv>(LoopEnv{std::move(begin), std::move(end)}));
}

bool Convertor::has_loop_env() const { return !_loop_env_stack.empty(); }

void Convertor::end_loop_env() { _loop_env_stack.pop(); }

void Convertor::clear_loop_env() {
    while (!_loop_env_stack.empty()) {
        _loop_env_stack.pop();
    }
}

void Convertor::set_func(const String &sym, Ir::pFunc fun) {
    _func_map[sym] = std::move(fun);
}

bool Convertor::func_count(const String &sym) {
    return _func_map.count(sym) != 0U;
}

Ir::pFunc Convertor::find_func(const String &sym) { return _func_map[sym]; }

} // namespace AstToIr
