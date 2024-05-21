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
#include "type.h"
#include "value.h"
#include <cstddef>
#include <memory>

#include <functional>

#define BUILTIN_INITIALIZER "__buildin_initializer"
#define MEMSET "llvm.memset.p0i8.i64"

namespace AstToIr {

void Convertor::throw_error(pNode root, int id, String msg) {
    root->throw_error(id, "Convertor", msg);
}

Ir::pModule Convertor::module() const { return _mod; }

bool Convertor::current_block_end() const {
    return _cur_func->body.back()->is_end_of_block();
}

Ir::pInstr Convertor::add_instr(Ir::pInstr instr) {
    _cur_func->add_body(instr);
    return instr;
}

Ir::pVal Convertor::find_left_value(pNode root, String sym,
                                    bool request_not_const) {
    Ir::pInstr found_instr;
    if (_env.env()->count(sym)) { // local
        MaybeConstInstr found = _env.env()->find(sym);
        // const value is error
        if (!is_pointer(found.instr->ty) ||
            (request_not_const && found.is_const))
            throw_error(root, 10, "assignment to a local const value");
        // no-const value should be loaded
        found_instr = found.instr;
    } else { // global
        for (auto i : module()->globs) {
            if (i->name() == "@" + sym) {
                if (request_not_const && i->is_const)
                    throw_error(root, 11, "assignment to a global const value");
                auto sym_node = Ir::make_sym_instr(TypedSym("@" + sym, i->ty));
                _cur_func->add_imm(sym_node);
                found_instr = sym_node;
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

Ir::pVal Convertor::cast_to_type(pNode root, Ir::pVal val, pType ty) {
    if (is_same_type(val->ty, ty))
        return val;
    // decay
    if (is_pointer(ty) && is_pointer(val->ty) &&
        is_same_type(to_pointed_type(ty),
                     to_elem_type(to_pointed_type(val->ty)))) {
        // printf("found %s decay from %s to %s\n", val->name(),
        // val->ty->type_name(), ty->type_name());
        auto imm = Ir::make_constant(ImmValue(0));
        auto r = Ir::make_item_instr(val, {imm});
        if (!is_same_type(ty, r->ty)) {
            throw_error(root, -1, "decay error?");
        }
        add_instr(r);
        _cur_func->add_imm(imm);
        return r;
    }
    if (!is_castable(val->ty, ty)) {
        printf("Message: not castable from %s to %s\n",
               val->ty->type_name().c_str(), ty->type_name().c_str());
        throw_error(root, 13, "[Convertor] error 13: not castable");
    }
    auto r = Ir::make_cast_instr(ty, val);
    add_instr(r);
    return r;
}

pType Convertor::analyze_type(pNode root) {
    switch (root->type) {
    case NODE_ARRAY_TYPE: {
        auto r = std::static_pointer_cast<Ast::ArrayTypeNode>(root);
        return make_array_type(analyze_type(r->n),
                               constant_eval(r->index).val.uval);
    }
    case NODE_BASIC_TYPE:
        return std::static_pointer_cast<Ast::BasicTypeNode>(root)->ty;
    case NODE_POINTER_TYPE:
        return make_pointer_type(analyze_type(
            std::static_pointer_cast<Ast::PointerTypeNode>(root)->n));
    default:
        break;
    }
    throw_error(root, 28, "not a type");
    return {};
}

Ir::pVal Convertor::analyze_opr(Pointer<Ast::BinaryNode> root) {
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
    case OPR_OR: { // shortcut evaluation
        /*
            A and B
            %1 = alloca i32
            %2 = A
            br i32 %2, L2, L1
            L1:
                store %1, 0
                br L3
            L2:
                store %1, B
                br L3
            L3:

            A or B
            %1 = alloca i32
            %2 = A
            br i32 %2, L1, L2
            L1:
                store %1, 1
                br L3
            L2:
                %3 = cast B
                store %1, %3
                br L3
            L3:
        */
        auto a0 = Ir::make_alloc_instr(make_basic_type(IMM_I1));
        auto L1 = Ir::make_label_instr();
        auto L2 = Ir::make_label_instr();
        auto L3 = Ir::make_label_instr();

        add_instr(a0);
        auto a1 = cast_to_type(root->lhs, analyze_value(root->lhs),
                               make_basic_type(IMM_I1));
        if (root->type == OPR_AND) {
            add_instr(Ir::make_br_cond_instr(a1, L2, L1));
        } else {
            add_instr(Ir::make_br_cond_instr(a1, L1, L2));
        }
        add_instr(L1);
        bool num = root->type != OPR_AND;
        auto constant = Ir::make_constant(ImmValue(num));
        add_instr(Ir::make_store_instr(a0, constant));
        _cur_func->add_imm(constant);
        add_instr(Ir::make_br_instr(L3));
        add_instr(L2);
        auto a2 = analyze_value(root->rhs);
        add_instr(Ir::make_store_instr(
            a0, cast_to_type(root->rhs, a2, make_basic_type(IMM_I1))));
        add_instr(Ir::make_br_instr(L3));
        add_instr(L3);

        return add_instr(Ir::make_load_instr(a0));
    }
    case OPR_EQ:
    case OPR_NE:
    case OPR_GT:
    case OPR_GE:
    case OPR_LT:
    case OPR_LE: {
        auto a1 = analyze_value(root->lhs);
        auto a2 = analyze_value(root->rhs);
        auto ty = join_type(a1->ty, a2->ty);
        if (!ty)
            throw_error(root, 18, "type has no joined type");
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

ImmValue Convertor::find_const_value(pNode root, String sym) {
    if (!_const_env.env()->count(sym)) {
        throw_error(root, 29, "not a constant");
    }
    return _const_env.env()->find(sym);
}

ImmValue Convertor::constant_eval(pNode node) {
    switch (node->type) {
    case NODE_SYM: {
        auto name = std::static_pointer_cast<Ast::SymNode>(node)->sym;
        return find_const_value(node, name);
    }
    case NODE_IMM:
        return std::static_pointer_cast<Ast::ImmNode>(node)->imm;
    case NODE_UNARY: {
        auto r = std::static_pointer_cast<Ast::UnaryNode>(node);
        auto imm = constant_eval(r->ch);
        switch (r->type) {
        case OPR_NOT:
            return !imm;
        case OPR_NEG:
            return imm.neg();
        case OPR_POS:
            break;
        }
        return imm;
    }
    case NODE_BINARY: {
        auto r = std::static_pointer_cast<Ast::BinaryNode>(node);
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

Ir::pVal Convertor::analyze_left_value(pNode root, bool request_not_const) {
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
        auto r = std::static_pointer_cast<Ast::ItemNode>(root);
        auto array = analyze_value(r->v, request_not_const);
        if (!is_pointer(array->ty)) {
            printf("Message: type is %s\n", array->ty->type_name().c_str());
            throw_error(r->v, 25, "not an array");
        }
        // printf("Array Type: %s\n", array->ty->type_name());
        Vector<Ir::pVal> indexs;
        for (auto i : r->index) {
            indexs.push_back(analyze_value(i));
            if (!is_integer(indexs.back()->ty))
                throw_error(i, 14, "type of index should be integer");
        }
        auto itemptr = Ir::make_item_instr(array, indexs);
        add_instr(itemptr);
        return itemptr;
    }
    case NODE_SYM: {
        auto r = std::static_pointer_cast<Ast::SymNode>(root);
        return find_left_value(r, r->sym, request_not_const);
    }
    case NODE_DEREF: {
        /*
            int x;          | x: int*
            int *y = &x;    | y: int**
            *y = 1;         | %1 = load i32** y
                            | store i32 1, i32* %1
        */
        auto r = std::static_pointer_cast<Ast::DerefNode>(root);
        return add_instr(Ir::make_load_instr(analyze_left_value(r->val)));
    }
    default:
        throw_error(root, 22, "left value needed");
        break;
    }
    return Ir::make_empty_instr();
}

Ir::pVal Convertor::analyze_value(pNode root, bool request_not_const) {
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
        auto r = std::static_pointer_cast<Ast::RefNode>(root);
        return analyze_left_value(r->v, true);
    }
    case NODE_CAST: {
        auto r = std::static_pointer_cast<Ast::CastNode>(root);
        auto res =
            Ir::make_cast_instr(analyze_type(r->ty), analyze_value(r->val));
        add_instr(res);
        return res;
    }
    case NODE_BINARY: {
        auto ir = analyze_opr(std::static_pointer_cast<Ast::BinaryNode>(root));
        return ir;
    }
    case NODE_UNARY: {
        auto r = std::static_pointer_cast<Ast::UnaryNode>(root);
        auto ch = r->ch;
        auto val = analyze_value(ch);
        switch (r->type) {
        case OPR_POS: {
            return val;
        }
        case OPR_NOT: {
            auto imm_ty = to_basic_type(val->ty)->ty;
            auto imm = Ir::make_constant(ImmValue(0).cast_to(imm_ty));
            _cur_func->add_imm(imm);
            return add_instr(Ir::make_cmp_instr(is_imm_integer(imm_ty) ? Ir::CMP_EQ : Ir::CMP_OEQ, val, imm));
        }
        case OPR_NEG: {
            if (is_float(val->ty)) {
                return add_instr(Ir::make_unary_instr(val));
            } else if (is_integer(val->ty)) {
                auto imm = Ir::make_constant(ImmValue(0));
                auto ty = join_type(val->ty, imm->ty);
                _cur_func->add_imm(imm);
                return add_instr(Ir::make_binary_instr(
                    Ir::INSTR_SUB, cast_to_type(root, imm, ty),
                    cast_to_type(root, val, ty)));
            }
            throw_error(root, 21, "neg an non-number");
        }
        }
    }
    case NODE_IMM: {
        auto r = std::static_pointer_cast<Ast::ImmNode>(root);
        auto res = Ir::make_constant(r->imm);
        _cur_func->add_imm(res);
        return res;
    }
    case NODE_CALL: {
        auto r = std::static_pointer_cast<Ast::CallNode>(root);
        if (!func_count(r->name)) {
            printf("Warning: function %s not found\n", r->name.c_str());
            throw_error(r, 20, "function not found");
        }
        auto func = find_func(r->name);
        Vector<Ir::pVal> args;
        // assert size equal
        size_t length = func->functon_type()->arg_type.size();
        if (!func->variant_length && length != r->ch.size())
            throw_error(root, 8, "wrong count of arguments");
        for (size_t i = 0; i < r->ch.size(); ++i) {
            Ir::pVal cur_arg = analyze_value(r->ch[i]);
            if (i < length) {
                args.push_back(cast_to_type(r->ch[i], cur_arg,
                                            func->functon_type()->arg_type[i]));
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

struct InitializerVisitor {
    InitializerVisitor(Pointer<Ast::ArrayDefNode> vst, bool is_root = false)
        : vst(vst), is_root(is_root) {
        if (vst) {
            begin = vst->nums.begin();
            end = vst->nums.end();
            is_empty = false;
        } else {
            is_empty = true;
        }
    }

    pNode peek() {
        if (begin == end) {
            return nullptr;
        }
        if (is_empty)
            return nullptr;
        return *begin;
    }

    pNode get() {
        if (begin == end) {
            return nullptr;
        }
        if (is_empty)
            return nullptr;
        return *(begin++);
    }

    Pointer<Ast::ArrayDefNode> vst;
    Vector<pNode>::iterator begin;
    Vector<pNode>::iterator end;

    bool is_empty;
    bool is_root;
};

Vector<pNode> flatten(InitializerVisitor &list, Pointer<ArrayType> type) {
    Vector<pNode> ans;
    size_t size = type->elem_count;
    pType elem_ty = type->elem_type;

    if (elem_ty->type_type() == TYPE_BASIC_TYPE) {
        for (size_t i = 0; i < size; ++i) {
            auto elem = list.get();
            if (!elem && list.is_root)
                break;
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
                flatten(new_v, std::static_pointer_cast<ArrayType>(elem_ty));
            ans.insert(ans.end(), inner.begin(), inner.end());
        } else {
            Vector<pNode> inner =
                flatten(list, std::static_pointer_cast<ArrayType>(elem_ty));
            ans.insert(ans.end(), inner.begin(), inner.end());
        }
    }

    return ans;
}

Vector<pNode> flatten(Pointer<Ast::ArrayDefNode> initializer,
                      Pointer<ArrayType> type) {
    InitializerVisitor new_v(initializer, true);
    return flatten(new_v, type);
}

void Convertor::copy_to_array(pNode root, Ir::pVal addr, Pointer<ArrayType> t,
                              Pointer<Ast::ArrayDefNode> n, bool constant) {
    if (!is_pointer(addr->ty)) {
        throw_error(root, 16, "list should initialize type that is pointed");
    }
    Vector<pNode> flat = flatten(n, t);
    Vector<Ir::pVal> indexes;
    auto begin = flat.begin();
    auto end = flat.end();
    std::function<void(pType t)> fn = [&](pType t) -> void {
        if (begin == end)
            return;
        if (is_basic_type(t)) {
            if(constant) {
                auto cur = *(begin++);
                if(!cur)
                    return;
                auto val = constant_eval(cur);
                if(!val)
                    return;
                auto item = Ir::make_item_instr(addr, indexes);
                auto imm = Ir::make_constant(val);
                add_instr(item);
                auto store = Ir::make_store_instr(item, cast_to_type(root, imm, t));
                add_instr(store);
                _cur_func->add_imm(imm);
                return ;
            }
            auto cur = *(begin++);
            if(!cur)
                return;
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
            auto index =
                Ir::make_constant(ImmValue((unsigned long long)i, IMM_I32));
            _cur_func->add_imm(index);
            indexes.push_back(index);
            fn(next_ty);
            indexes.pop_back();
        }
    };
    fn(t);
}

bool Convertor::analyze_statement_node(pNode root) {
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
        auto r = std::static_pointer_cast<Ast::AssignNode>(root);
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
        auto r = std::static_pointer_cast<Ast::VarDefNode>(root);
        Ir::pInstr tmp;
        auto ty = analyze_type(r->var.n);
        if (is_array(ty)) {
            add_instr(tmp = Ir::make_alloc_instr(ty));
            _env.env()->set(r->var.name, {tmp, r->is_const});
            if (r->val) {
                if (r->val->type != NODE_ARRAY_VAL)
                    throw_error(r->val, 17,
                                "array should be initialized by a list");
                auto rrr = std::static_pointer_cast<Ast::ArrayDefNode>(r->val);
                auto imm1 = Ir::make_constant(ImmValue(0ll, IMM_I8));
                auto imm2 = Ir::make_constant(ImmValue((unsigned long long)to_array_type(ty)->length(), IMM_U64));
                auto imm3 = Ir::make_constant(ImmValue(0ll, IMM_I1));
                _cur_func->add_imm(imm1);
                _cur_func->add_imm(imm2);
                _cur_func->add_imm(imm3);
                add_instr(Ir::make_call_instr(find_func(MEMSET), {
                    cast_to_type(r, tmp, make_pointer_type(make_basic_type(IMM_I8))),
                    imm1, imm2, imm3
                }));
                copy_to_array(r, tmp, to_array_type(ty), rrr, false);
            }
            break;
        }
        add_instr(tmp = Ir::make_alloc_instr(ty));
        if (r->val) {
            if (r->is_const) {
                _const_env.env()->set(r->var.name, constant_eval(r->val).cast_to(to_basic_type(ty)->ty));
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
        auto r = std::static_pointer_cast<Ast::IfNode>(root);
        auto if_begin = Ir::make_label_instr();
        auto if_end = Ir::make_label_instr();
        auto if_else_end = Ir::make_label_instr();
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
            bool need_label = false;
            add_instr(
                Ir::make_br_cond_instr(cast_to_type(r, analyze_value(r->cond),
                                                    make_basic_type(IMM_I1)),
                                       if_begin, if_end));
            add_instr(if_begin);
            analyze_statement_node(r->body);
            if (_cur_func->body.size() && !current_block_end()) {
                add_instr(Ir::make_br_instr(if_else_end));
                need_label = true;
            }
            add_instr(if_end);
            analyze_statement_node(r->elsed);
            if (_cur_func->body.size() && !current_block_end()) {
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
            add_instr(
                Ir::make_br_cond_instr(cast_to_type(r, analyze_value(r->cond),
                                                    make_basic_type(IMM_I1)),
                                       if_begin, if_end));
            add_instr(if_begin);
            analyze_statement_node(r->body);
            if (_cur_func->body.size() && !current_block_end()) {
                add_instr(Ir::make_br_instr(if_end));
            }
            add_instr(if_end);
        }
        break;
    }
    case NODE_WHILE: {
        auto r = std::static_pointer_cast<Ast::WhileNode>(root);
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
        if (_cur_func->body.size() && !current_block_end()) {
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
        auto r = std::static_pointer_cast<Ast::ForNode>(root);
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
        auto r = std::static_pointer_cast<Ast::ReturnNode>(root);
        if (r->ret) {
            auto res = analyze_value(r->ret);
            add_instr(Ir::make_ret_instr(
                cast_to_type(r, res, _cur_func->functon_type()->ret_type)));
        } else {
            add_instr(Ir::make_ret_instr());
        }
        is_end = true;
        break;
    }
    case NODE_BLOCK: {
        auto r = std::static_pointer_cast<Ast::BlockNode>(root);
        _env.push_env();
        _const_env.push_env();
        for (auto i : r->body) {
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

void Convertor::generate_function(Pointer<Ast::FuncDefNode> root) {
    _env.push_env();
    _const_env.push_env();
    Ir::pFuncDefined func;
    {
        Vector<pType> types;
        Vector<String> syms;
        for (auto i : root->args) {
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
        for (size_t i = 0; i < root->args.size(); ++i) {
            /*
                由于 sysy 语法中函数参数没有 const
                所以这里一定是 false
            */
            if (_env.env()->count(root->args[i].name)) {
                printf("Warning: %s repeated.\n", root->args[i].name.c_str());
                throw_error(root, 26, "repeated argument name");
            }
            _env.env()->set(root->args[i].name, {func->args[i], false});
        }
        set_func(func->name(), func);
    }
    _cur_func = func;
    if(root->var.name == "main") {
        analyze_statement_node(Ast::new_call_node(NULL, BUILTIN_INITIALIZER, {}));
    }
    analyze_statement_node(root->body);
    func->end_function();
    module()->add_func(func);
    _const_env.end_env();
    _env.end_env();
}

Value Convertor::from_array_def(Pointer<Ast::ArrayDefNode> n,
                                Pointer<ArrayType> t) {
    Vector<pNode> flat = flatten(n, t);
    auto begin = flat.begin();
    std::function<Value(pType t)> fn = [&](pType t) -> Value {
        if (is_basic_type(t)) {
            return constant_eval(*(begin++)).cast_to(to_basic_type(t)->ty);
        }
        size_t length = to_array_type(t)->elem_count;
        pType next_ty = to_array_type(t)->elem_type;
        ArrayValue v;
        v.ty = next_ty;
        for (size_t i = 0; i < length; ++i) {
            v.arr.push_back(make_value(fn(next_ty)));
        }
        return v;
    };
    return fn(t);
}

void Convertor::generate_global_var(Pointer<Ast::VarDefNode> root) {
    TypedSym root_var = TypedSym(root->var.name, analyze_type(root->var.n));
    if (is_basic_type(root_var.ty)) {
        if (root->val) {
            auto imm = constant_eval(root->val).cast_to(to_basic_type(root_var.ty)->ty);
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
            module()->add_global(Ir::make_global(
                root_var,
                /*
                Value(from_array_def(
                    std::static_pointer_cast<Ast::ArrayDefNode>(root->val),
                    to_array_type(root_var.ty))),
                */
                Value(),
                root->is_const));

                add_initialization(root_var, std::static_pointer_cast<Ast::ArrayDefNode>(root->val));
        } else {
            module()->add_global(Ir::make_global(
                root_var,
                // Value(from_array_def(nullptr, to_array_type(root_var.ty))),
                Value(),
                root->is_const));
        }
    }
}

void Convertor::generate_single(pNode root) {
    switch (root->type) {
    case NODE_DEF_FUNC:
        generate_function(std::static_pointer_cast<Ast::FuncDefNode>(root));
        break;
    case NODE_DEF_VAR:
        generate_global_var(std::static_pointer_cast<Ast::VarDefNode>(root));
        break;
    default:
        throw_error(root, 5, "global operation has type that not implemented");
    }
}

Ir::pModule Convertor::generate(AstProg asts) {
    _env.clear_env();
    _const_env.clear_env();
    _const_env.push_env();
    clear_loop_env();
    _prog = asts;
    _mod = Ir::pModule(new Ir::Module());

    auto i1 = make_basic_type(IMM_I1);
    auto i8 = make_basic_type(IMM_I8);
    auto i8s = make_pointer_type(i8);
    auto i32 = make_basic_type(IMM_I32);
    auto i64 = make_basic_type(IMM_I64);
    auto f32 = make_basic_type(IMM_F32);
    auto vd = make_void_type();

    _initializer_func = Ir::make_func_defined(TypedSym(BUILTIN_INITIALIZER, vd), {}, {});
    _mod->add_func(_initializer_func);
    set_func(BUILTIN_INITIALIZER, _initializer_func);

    _mod->add_func_declaration(Ir::make_func(TypedSym("_sysy_starttime", vd), { i32 }));
    _mod->add_func_declaration(Ir::make_func(TypedSym("_sysy_stoptime", vd), { i32 }));

    _mod->add_func_declaration(Ir::make_func(TypedSym(MEMSET, vd), { i8s, i8, i64, i1 } ));

    _mod->add_func_declaration(Ir::make_func(TypedSym("getint", i32), {}));
    _mod->add_func_declaration(Ir::make_func(TypedSym("getch", i8), {}));
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

    for (auto i : asts) {
        generate_single(i);
    }

    _initializer_func->end_function();

    _const_env.end_env();
    return _mod;
}

void Convertor::add_initialization(TypedSym var, Pointer<Ast::ArrayDefNode> node)
{
    EnvWrapper<MaybeConstInstr> env = _env;
    auto cur_func = _cur_func;
    
    _cur_func = _initializer_func;
    _env = EnvWrapper<MaybeConstInstr>();
    _env.push_env();
    
    auto arr = find_left_value(node, var.sym);
    copy_to_array(node, arr, to_array_type(to_pointed_type(arr->ty)), node, true);
    
    _cur_func = cur_func;
    _env.end_env();
    _env = env;
}

Ir::BinInstrType Convertor::fromBinaryOpr(Pointer<Ast::BinaryNode> root,
                                          pType ty) {
    bool is_signed = is_signed_type(ty);
#define SELECT(x)                                                              \
    case OPR_##x:                                                              \
        return Ir::INSTR_##x;
    switch (root->type) {
        SELECT(ADD)
        SELECT(SUB)
        SELECT(MUL)
        SELECT(DIV)
        //    SELECT(AND)
        //    SELECT(OR)
    case OPR_REM: {
        if (is_signed)
            return Ir::INSTR_SREM;
        return Ir::INSTR_UREM;
    }
    default:
        throw_error(
            root, -1,
            "binary operation conversion from ast to ir not implemented");
    }
#undef SELECT
    return Ir::INSTR_ADD;
}

Ir::CmpType Convertor::fromCmpOpr(Pointer<Ast::BinaryNode> root, pType ty) {
    bool is_signed = is_signed_type(ty);
    bool is_flt = is_float(ty);
    switch (root->type) {
        case OPR_EQ: {
            if (is_flt) {
                return Ir::CMP_OEQ;
            }
            return Ir::CMP_EQ;
        }
        case OPR_NE: {
            if (is_flt) {
                return Ir::CMP_UNE;
            }
            return Ir::CMP_NE;
        }
#undef SELECT
#define SELECT_US(x)                                                           \
    case OPR_##x: {                                                            \
        if (is_flt) {                                                          \
            return Ir::CMP_O##x;                                               \
        } else if (is_signed) {                                                \
            return Ir::CMP_S##x;                                               \
        } else {                                                               \
            return Ir::CMP_U##x;                                               \
        }                                                                      \
        break;                                                                 \
    }
        SELECT_US(LE)
        SELECT_US(GE)
        SELECT_US(LT)
        SELECT_US(GT)
#undef SELECT
    default:
        throw_error(
            root, 7,
            "comparasion operation conversion from ast to ir not implemented");
    }
    return Ir::CMP_EQ;
}

pLoopEnv Convertor::loop_env() { return _loop_env_stack.top(); }

void Convertor::push_loop_env(Ir::pInstr begin, Ir::pInstr end) {
    _loop_env_stack.push(pLoopEnv(new LoopEnv{begin, end}));
}

bool Convertor::has_loop_env() const { return !_loop_env_stack.empty(); }

void Convertor::end_loop_env() { _loop_env_stack.pop(); }

void Convertor::clear_loop_env() {
    while (!_loop_env_stack.empty())
        _loop_env_stack.pop();
}

void Convertor::set_func(String sym, Ir::pFunc fun) { _func_map[sym] = fun; }

bool Convertor::func_count(String sym) { return _func_map.count(sym); }

Ir::pFunc Convertor::find_func(String sym) { return _func_map[sym]; }

} // namespace AstToIr
