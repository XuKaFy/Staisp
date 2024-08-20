#include "trans_wrapper.h"
#include "def.h"
#include "ir_call_instr.h"
#include "ir_func.h"
#include "ir_func_defined.h"
#include "ir_global.h"
#include "ir_instr.h"
#include "ir_mem_instr.h"
#include "ir_val.h"
#include "type.h"

namespace Optimize {

bool memoi_wrapper::is_purely_recursive(Ir::FuncDefined *func) {
    if (func->function_type()->ret_type->type_type() == TYPE_VOID_TYPE ||
        func->function_type()->arg_type.empty())
        return false;
    for (auto &&arg_type : func->function_type()->arg_type) {
        if (!is_basic_type(arg_type))
            return false;
    }
    bool recursive = false;
    for (auto &&block : func->p) {
        for (auto &&instr : *block) {
            switch (instr->instr_type()) {
            case Ir::INSTR_CALL: {
                auto call = static_cast<Ir::CallInstr *>(instr.get());
                auto callee = dynamic_cast<Ir::FuncDefined *>(call->operand(0)->usee);
                if (!callee) return false;
                if (callee != func) { // recursion do not cancel purity
                    if (!is_purely_recursive(callee)) {
                        return false;
                    }
                } else {
                    recursive = true;
                }
                break;
            }
            case Ir::INSTR_LOAD:
            case Ir::INSTR_ITEM:
            case Ir::INSTR_STORE: {
                if (instr->operand(0)->usee->name()[0] == '@') {
                    return false;
                }
                break;
            }
            }
        }
    }
    return recursive;
}

void memoi_wrapper::ap() {

    for (auto &&funcDef : module->funsDefined) {
        auto func = funcDef.get();
        if (!is_purely_recursive(func)) {
            continue;
        }
        if (func->function_type()->arg_type.size() > 3) continue;

        auto func_args = func->arg_name;
        auto func_ty = func->function_type();

        auto cur_cache =
            Ir::make_func(TypedSym(func->name() + "_cached", func_ty->ret_type),
                  func_ty->arg_type);
        auto users = func->users();
        for (auto &&func_uses : users) {
            auto user = func_uses->user;
            auto call_instr_func = dynamic_cast<Ir::CallInstr *>(user);
            call_instr_func->change_operand(0, cur_cache.get());
            my_assert(call_instr_func, "must be call ");
        }
        module->add_func_declaration(cur_cache);
        module->funsCache.insert(funcDef);
    }

    // printf("can be recursive: %s\n", func->name().c_str());
}

String construct(size_t id, Ir::FuncDefined* func) {
    std::string name = func->name();
    auto type = func->function_type();
    std::string function_label = name + "_cached";
    std::string miss_label = name + "_cached_miss";
    std::string epilog_label = name + "_cached_epilog";
    std::string directive = R"(.text
.globl )" + function_label;
    std::string prolog = "\n" + function_label;
    prolog += R"(:
    addi    sp, sp, -48
    sd      ra, 40(sp)
    sd      s0, 32(sp)
    sd      s1, 24(sp)
    sd      s2, 16(sp)
    sd      s3, 8(sp))";
    std::string epilog = "\n" + epilog_label;
    epilog += R"(:
    ld      ra, 40(sp)
    ld      s0, 32(sp)
    ld      s1, 24(sp)
    ld      s2, 16(sp)
    ld      s3, 8(sp)
    addi    sp, sp, 48
    ret
)";
    std::string store_args;
    std::string load_args;
    int reg = 0;
    int freg = 0;
    for (size_t i = 0; i < 3; ++i) {
    std::string target = "s" + std::to_string(i + 1);
        if (i >= type->arg_type.size()) {
            store_args += "\n    mv      " + target + ", zero";
        } else if (is_float(type->arg_type[i])) {
            std::string source = "fa" + std::to_string(freg++);
            store_args += "\n    fmv.x.s " + target + ", " + source;
            load_args  += "\n    fmv.s.x " + source + ", " + target;
        } else {
            std::string source = "a" + std::to_string(reg++);
            store_args += "\n    mv      " + target + ", " + source;
            load_args  += "\n    mv      " + source + ", " + target;
        }
    }
    std::string mv_args;
    mv_args += "\n    li      a0, " + std::to_string(id);
    mv_args += R"(
    mv      a1, s1
    mv      a2, s2
    mv      a3, s3)";
    std::string if_hit = R"(
    call    __hit
    beqz    a0, )";
    if_hit += miss_label;
    std::string call_get = "\n    call    __get";
    std::string call_put = "\n    mv      a4, s0\n    call    __put";
    std::string call_func = "\n    call    " + name;
    std::string store_return, load_return;
    if (is_float(type->ret_type)) {
        load_return  = "\n    fmv.s.x fa0, s0";
        store_return = "\n    fmv.x.s s0, fa0";
    } else {
        load_return  = "\n    mv      a0, s0";
        store_return = "\n    mv      s0, a0";
    }
    std::string ret = "\n    ret\n";
    std::string j_epilog = "\n    j " + epilog_label;

    std::string miss_block = "\n" + miss_label + ":";

    return
    directive
    + prolog
    + store_args
    + mv_args
    + if_hit
    + mv_args
    + call_get
    + "\n    mv      s0, a0" // get always return int
    + load_return
    + j_epilog
    + miss_block
    + load_args
    + call_func
    + store_return
    + mv_args
    + call_put
    + load_return
    + epilog;
}

void memoi_wrapper::bk_fill(Ir::Module* mod, String &res) {
    const char* tools = R"(
.text
.globl __hash
__hash:
    slli    a0, a0, 2
    slli    a1, a1, 1
    xor     a0, a0, a1
    xor     a0, a0, a2
    slli    a0, a0, 1
    xor     a0, a0, a3
    andi    a0, a0, 1023
    ret

.globl __hit
__hit:
    slli    a4, a0, 2
    slli    a5, a1, 1
    xor     a4, a4, a5
    xor     a4, a4, a2
    slli    a4, a4, 1
    xor     a4, a4, a3
    andi    a4, a4, 1023
    la      a5, VALID
    add     a5, a5, a4
    lbu     a5, 0(a5)
    beqz    a5, __hit_miss
    slli    a4, a4, 4
    la      a5, KEY
    add     a4, a4, a5
    lw      a5, 0(a4)
    bne     a5, a0, __hit_miss
    lw      a0, 4(a4)
    bne     a0, a1, __hit_miss
    lw      a0, 8(a4)
    bne     a0, a2, __hit_miss
    lw      a0, 12(a4)
    xor     a0, a0, a3
    seqz    a0, a0
    ret
__hit_miss:
    li      a0, 0
    ret

.globl __get
__get:
    slli    a0, a0, 2
    slli    a1, a1, 1
    xor     a0, a0, a1
    xor     a0, a0, a2
    slli    a0, a0, 1
    xor     a0, a0, a3
    andi    a0, a0, 1023
    slli    a0, a0, 2
    la      a1, VALUE
    add     a0, a0, a1
    lw      a0, 0(a0)
    ret

.globl __put
__put:
    slli    a6, a0, 2
    slli    a5, a1, 1
    xor     a5, a5, a6
    xor     a5, a5, a2
    slli    a5, a5, 1
    xor     a5, a5, a3
    andi    a7, a5, 1023
    slli    a6, a7, 4
    la      a5, KEY
    add     a5, a5, a6
    sw      a0, 0(a5)
    sw      a1, 4(a5)
    sw      a2, 8(a5)
    sw      a3, 12(a5)
    slli    a0, a7, 2
    la      a1, VALUE
    add     a0, a0, a1
    sw      a4, 0(a0)
    la      a0, VALID
    add     a0, a0, a7
    li      a1, 1
    sb      a1, 0(a0)
    ret
.data

.globl VALID
VALID:
    .zero   1024

.globl KEY
KEY:
    .zero   16384

.globl VALUE
VALUE:
    .zero   4096
)";

    res += tools;
    int id = 0;
    for (auto&& function : mod->funsCache) {
        res += construct(id++, function.get());
    }
}

} // namespace Optimize