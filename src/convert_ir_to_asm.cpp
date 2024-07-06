#include "convert_ir_to_asm.h"

#include <cstddef>
#include <ir_call_instr.h>
#include <ir_cast_instr.h>
#include <ir_cmp_instr.h>
#include <ir_ptr_instr.h>

#include "bkd_global.h"
#include "bkd_ir_instr.h"
#include "bkd_module.h"
#include "bkd_reg.h"
#include "def.h"
#include "imm.h"
#include "ir_instr.h"
#include "ir_opr_instr.h"
#include "type.h"
#include "value.h"
#include <iterator>
#include <memory>
#include <string>
#include <utility>

namespace IrToAsm {

[[noreturn]] void unreachable() {
    my_assert(false, "unreachable");
}

Backend::pModule Convertor::convert(const Ir::pModule &mod)
{
    auto bkd_mod = std::make_shared<Backend::Module>();

    for (auto && global : mod->globs) {
        bkd_mod->globs.push_back(convert(global));
    }
    
    for (auto && func : mod->funsDefined) {
        FunctionConvertor convertor(func);
        bkd_mod->funcs.push_back(convertor.convert());
    }
    
    return bkd_mod;
}

Backend::Global Convertor::convert(const Ir::pGlobal &glob)
{
    Value& con = glob->con.v;
    switch (con.type()) {
    case VALUE_IMM:
        // all imm should be regard as int32
        return Backend::Global(glob->name().substr(1), con.imm_value().val.ival % 2147483648);
    case VALUE_ARRAY:
        return Backend::Global(glob->name().substr(1), con.array_value());
    }
    unreachable();
}

Backend::Func FunctionConvertor::convert()
{
    bkd_func.body.resize(func->p.size() + 2);
    int index = 0;
    for (auto && block : func->p) {
        bkd_func.body[++index] = convert(block);
    }
    bkd_func.body.front() = generate_prolog();
    bkd_func.body.back() = generate_epilog();
    return bkd_func;
}

Backend::Block FunctionConvertor::convert(const Ir::pBlock &block)
{
    Backend::Block bkd_block(func->name() + "_" + block->label()->name());

    for (auto && instr : *block) {
        auto res = convert(instr);
        bkd_block.body.insert(bkd_block.body.end(),
            std::move_iterator(res.begin()), std::move_iterator(res.end()));
    }

    return bkd_block;
}

struct ConvertBulk {
    Backend::MachineInstrs bulk;
    Ir::pFuncDefined func;
    int& allocate_register;
    explicit ConvertBulk(Ir::pFuncDefined func, int& allocate_register)
        : func(std::move(func)), allocate_register(allocate_register) {}

    void add(Backend::MachineInstr const& value) {
        bulk.push_back(value);
    }

    template<typename T>
    T selector(ImmType ty,
        T i32_use,
        T i64_use) {
        switch (ty) {
            case IMM_I1:
            case IMM_I8:
            case IMM_I16:
            case IMM_I32:
            case IMM_U1:
            case IMM_U8:
            case IMM_U16:
            case IMM_U32:
                return i32_use;
            case IMM_I64:
            case IMM_U64:
                return i64_use;
            default:
                unreachable();
        }
    }

    static constexpr Backend::Reg NO_HINT = (Backend::Reg) 32;
    static constexpr Backend::FReg NO_HINT_F = (Backend::FReg) 32;

    Backend::Reg toReg(Ir::Val* val, Backend::Reg tmp_hint = NO_HINT) {
        auto name = val->name();
        // if it is a register, use it
        if (name[0] == '%') {
            // %0 => -1
            return (Backend::Reg) ~std::stoi(name.substr(1));
        }
        if (tmp_hint == NO_HINT) tmp_hint = (Backend::Reg) ++allocate_register;
        // if it is a immediate, load it
        add({Backend::ImmInstr {
            Backend::ImmInstrType::LI, tmp_hint, std::stoi(name)}
        });
        return tmp_hint;
    }

    Backend::FReg toFReg(Ir::Val* val, Backend::FReg tmp_hint = NO_HINT_F) {
        auto name = val->name();
        // if it is a register, use it
        if (name[0] == '%') {
            // %0 => -1
            return (Backend::FReg) ~std::stoi(name.substr(1));
        }
        if (tmp_hint == NO_HINT_F) tmp_hint = (Backend::FReg) ++allocate_register;
        auto tmp = (Backend::Reg) ++allocate_register;
        // if it is a immediate, load it
        // lLVM store float constant into hexadecimal integral representation of corresponding double value
        union {
            unsigned long long ull;
            double d;
        };
        ull = std::stoull(name, nullptr, 16);
        union {
            float f;
            int x;
        };
        f = d;
        add({Backend::ImmInstr {
            Backend::ImmInstrType::LI, tmp, x}
        });
        add({ Backend::FRegRegInstr {
            Backend::FRegRegInstrType::FMV_W_X, .fr = tmp_hint, .r = tmp
        } });
        return tmp_hint;
    }

    Backend::RegRegInstrType selectRRType(ImmType ty, Ir::BinInstrType bin) {
        switch (bin) {
            case Ir::INSTR_ADD:
                return selector(ty,
                    Backend::RegRegInstrType::ADD,
                    Backend::RegRegInstrType::ADDW);
            case Ir::INSTR_SUB:
                return selector(ty,
                    Backend::RegRegInstrType::SUB,
                    Backend::RegRegInstrType::SUBW);
            case Ir::INSTR_MUL:
                return selector(ty,
                    Backend::RegRegInstrType::MUL,
                    Backend::RegRegInstrType::MULW);
            case Ir::INSTR_SDIV:
            case Ir::INSTR_UDIV:
                return selector(ty,
                    Backend::RegRegInstrType::DIV,
                    Backend::RegRegInstrType::DIVW);
            case Ir::INSTR_SREM:
            case Ir::INSTR_UREM:
                return selector(ty,
                    Backend::RegRegInstrType::REM,
                    Backend::RegRegInstrType::REMW);
            case Ir::INSTR_AND:
                return Backend::RegRegInstrType::AND;
            case Ir::INSTR_OR:
                return Backend::RegRegInstrType::OR;
            case Ir::INSTR_ASHR:
                return selector(ty,
                    Backend::RegRegInstrType::SRA,
                    Backend::RegRegInstrType::SRAW);
                case Ir::INSTR_LSHR:
                return selector(ty,
                    Backend::RegRegInstrType::SRL,
                    Backend::RegRegInstrType::SRLW);
            case Ir::INSTR_SHL:
                return selector(ty,
                    Backend::RegRegInstrType::SLL,
                    Backend::RegRegInstrType::SLLW);
            default:
                unreachable();
        }
    }


    Backend::FRegFRegInstrType selectFRFRType(ImmType ty, Ir::BinInstrType bin) {
        switch (bin) {
            case Ir::INSTR_FADD:
                return Backend::FRegFRegInstrType::FADD_S;
            case Ir::INSTR_FSUB:
                return Backend::FRegFRegInstrType::FSUB_S;
            case Ir::INSTR_FMUL:
                return Backend::FRegFRegInstrType::FMUL_S;
            case Ir::INSTR_FDIV:
                return Backend::FRegFRegInstrType::FDIV_S;
            default:
                unreachable();
        }
    }

    void convert_unary_instr(const Pointer<Ir::UnaryInstr> &instr) {
        auto rd = toFReg(instr.get(), {});
        auto rs = toFReg(instr->operand(0)->usee, rd);
        add({ Backend::FRegInstr {
                Backend::FRegInstrType::FNEG_S, rd, rs,
        } });
    }

    void convert_binary_instr(const Pointer<Ir::BinInstr> &instr) {
        if (is_float(instr->ty)) {
            auto rd = toFReg(instr.get());
            auto rs1 = toFReg(instr->operand(0)->usee, rd);
            auto rs2 = toFReg(instr->operand(1)->usee, rd);
            add({ Backend::FRegFRegInstr {
                selectFRFRType(to_basic_type(instr->ty)->ty, instr->binType), rd, rs1, rs2,
            } });
        } else {
            auto rd = toReg(instr.get());
            auto rs1 = toReg(instr->operand(0)->usee, rd);
            auto rs2 = toReg(instr->operand(1)->usee, rd);
            add({ Backend::RegRegInstr {
                selectRRType(to_basic_type(instr->ty)->ty, instr->binType), rd, rs1, rs2,
            } });
        }
    }

    void convert_return_instr(const Pointer<Ir::RetInstr> &instr) {
        auto ret = std::static_pointer_cast<Ir::RetInstr>(instr);
        if (ret->operand_size() > 0) {
            if (is_float(instr->ty)) {
                auto fa0 = Backend::FReg::FA0;
                add({ Backend::FRegInstr{
                    Backend::FRegInstrType::FMV_S,
                    fa0,
                    toFReg(ret->operand(0)->usee, fa0)
                } });
            } else {
                auto a0 = Backend::Reg::A0;
                add({ Backend::RegInstr{
                    Backend::RegInstrType::MV,
                    a0,
                    toReg(ret->operand(0)->usee, a0)
                } });
            }
        }
        add({ Backend::JInstr { func->name() + "_epilog" } });
    }

    void convert_cast_instr(const Pointer<Ir::CastInstr> &instr) {
        switch (instr->method()) {
            case Ir::CAST_PTRTOINT:
            case Ir::CAST_INTTOPTR:
            case Ir::CAST_TRUNC:
            case Ir::CAST_FPEXT:
            case Ir::CAST_FPTRUNC:
                unreachable();
            case Ir::CAST_BITCAST:
            case Ir::CAST_SEXT:
            case Ir::CAST_ZEXT: {
                auto rs = toReg(instr->operand(0)->usee);
                auto rd = toReg(instr.get());
                add({ Backend::RegInstr{
                    Backend::RegInstrType::MV,
                    rd,
                    rs
                } });
                break;
            }
            case Ir::CAST_SITOFP:
            case Ir::CAST_UITOFP: {
                auto rs = toReg(instr->operand(0)->usee);
                auto rd = toFReg(instr.get());
                add({ Backend::FRegRegInstr{
                    Backend::FRegRegInstrType::FCVT_S_W,
                    rd, rs
                } });
                break;
            }
            case Ir::CAST_FPTOSI:
            case Ir::CAST_FPTOUI: {
                auto rs = toFReg(instr->operand(0)->usee);
                auto rd = toReg(instr.get());
                add({ Backend::FRegRegInstr{
                    Backend::FRegRegInstrType::FCVT_W_S,
                    rs, rd
                } });
                break;
            }
        }

    }


    void convert_cmp_instr(const Pointer<Ir::CmpInstr> &instr) {
        auto ty = instr->operand(0)->usee->ty;
        if (is_float(ty)) {
            auto rd = toReg(instr.get());
            auto rs1 = toFReg(instr->operand(0)->usee);
            auto rs2 = toFReg(instr->operand(1)->usee);
            switch (instr->cmp_type) {
                case Ir::CMP_OEQ:
                    add({ Backend::FCmpInstr {
                        Backend::FCmpInstrType::FEQ_S, rd, rs1, rs2,
                    } });
                    break;
                case Ir::CMP_OGT:
                    add({ Backend::FCmpInstr {
                        Backend::FCmpInstrType::FLT_S, rd, rs2, rs1,
                    } });
                    break;
                case Ir::CMP_OGE:
                    add({ Backend::FCmpInstr {
                        Backend::FCmpInstrType::FLE_S, rd, rs2, rs1,
                    } });
                    break;
                case Ir::CMP_OLT:
                    add({ Backend::FCmpInstr {
                        Backend::FCmpInstrType::FLT_S, rd, rs1, rs2,
                    } });
                    break;
                case Ir::CMP_OLE:
                    add({ Backend::FCmpInstr {
                        Backend::FCmpInstrType::FLE_S, rd, rs1, rs2,
                    } });
                    break;
                case Ir::CMP_UNE:
                    add({ Backend::FCmpInstr {
                        Backend::FCmpInstrType::FEQ_S, rd, rs1, rs2,
                    } });
                    add({ Backend::RegInstr {
                        Backend::RegInstrType::SEQZ, rd, rd
                    } });
                    break;
                default:
                    unreachable();
            }
        } else {
            auto rd = toReg(instr.get());
            auto rs1 = toReg(instr->operand(0)->usee, rd);
            auto rs2 = toReg(instr->operand(1)->usee, rd);
            switch (instr->cmp_type) {
                case Ir::CMP_EQ:
                    add({ Backend::RegRegInstr {
                        Backend::RegRegInstrType::SUB, rd, rs1, rs2
                    } });
                    add({ Backend::RegInstr {
                        Backend::RegInstrType::SEQZ, rd, rd
                    } });
                    break;
                case Ir::CMP_NE:
                    add({ Backend::RegRegInstr {
                        Backend::RegRegInstrType::SUB, rd, rs1, rs2
                    } });
                    add({ Backend::RegInstr {
                        Backend::RegInstrType::SNEZ, rd, rd
                    } });
                    break;
                case Ir::CMP_ULT:
                case Ir::CMP_SLT:
                    add({ Backend::RegRegInstr {
                        Backend::RegRegInstrType::SUB, rd, rs1, rs2
                    } });
                    add({ Backend::RegInstr {
                        Backend::RegInstrType::SLTZ, rd, rd
                    } });
                    break;
                case Ir::CMP_UGE:
                case Ir::CMP_SGE:
                    add({ Backend::RegRegInstr {
                        Backend::RegRegInstrType::SUB, rd, rs1, rs2
                    } });
                    add({ Backend::RegInstr {
                        Backend::RegInstrType::SLTZ, rd, rd
                    } });
                    add({ Backend::RegInstr {
                        Backend::RegInstrType::SEQZ, rd, rd
                    } });
                    break;
                case Ir::CMP_UGT:
                case Ir::CMP_SGT:
                    add({ Backend::RegRegInstr {
                        Backend::RegRegInstrType::SUB, rd, rs1, rs2
                    } });
                    add({ Backend::RegInstr {
                        Backend::RegInstrType::SGTZ, rd, rd
                    } });
                    break;
                case Ir::CMP_ULE:
                case Ir::CMP_SLE:
                    add({ Backend::RegRegInstr {
                        Backend::RegRegInstrType::SUB, rd, rs1, rs2
                    } });
                    add({ Backend::RegInstr {
                        Backend::RegInstrType::SGTZ, rd, rd
                    } });
                    add({ Backend::RegInstr {
                        Backend::RegInstrType::SEQZ, rd, rd
                    } });
                    break;
                default:
                    unreachable();
            }

        }
    }

    void convert_branch_instr(const Pointer<Ir::BrCondInstr> &instr) {
        auto rs = toReg(instr->operand(0)->usee);
        auto label1 = func->name() + "_" + instr->operand(1)->usee->name();
        auto label2 = func->name() + "_" + instr->operand(2)->usee->name();
        // else branch: beqz rs label2, tend to be remote
        add({ Backend::RegLabelInstr{Backend::RegLabelInstrType::BEQZ, rs,  label2 } });
        // then branch: tend to follow current basic block
        add({ Backend::JInstr{ label1 } });
    }

    int convert_call_instr(const Pointer<Ir::CallInstr> &instr) {
        Backend::Reg arg = Backend::Reg::A0;
        Backend::FReg farg = Backend::FReg::FA0;
        int sp = 0;
        for (size_t i = 1; i < instr->operand_size(); ++i) {
            auto param = instr->operand(i)->usee;
            if (is_float(param->ty)) {
                auto rd = farg;
                auto rs = toFReg(param);
                if (rd <= Backend::FReg::FA7) {
                    add({  Backend::FRegInstr{
                        Backend::FRegInstrType::FMV_S,
                        rd,
                        rs
                    } });
                    farg = (Backend::FReg)((int)farg + 1);
                } else {
                    add({ Backend::FRegImmRegInstr {
                        Backend::FRegImmRegInstrType::FSW, rs,  sp, Backend::Reg::SP,
                    } });
                    sp += 8;
                }
            } else {
                auto rd = arg;
                auto rs = toReg(param);
                if (rd <= Backend::Reg::A7) {
                    add({  Backend::RegInstr{
                        Backend::RegInstrType::MV,
                        rd,
                        rs
                    } });
                    arg = (Backend::Reg)((int)arg + 1);
                } else {
                    add({ Backend::RegImmRegInstr {
                        Backend::RegImmRegInstrType::SD, rs,  sp, Backend::Reg::SP,
                    } });
                    sp += 8;
                }
            }
        }
        add({ Backend::CallInstr{instr->operand(0)->usee->name()} });
        auto rt = instr->func_ty->ret_type;
        if (is_float(rt)) {
            auto rd = toFReg(instr.get());
            auto fa0 = Backend::FReg::FA0;
            add({ Backend::FRegInstr{
                Backend::FRegInstrType::FMV_S,
                rd,
                fa0
            } });
        } else if (is_integer(rt)) {
            auto rd = toReg(instr.get());
            auto a0 = Backend::Reg::A0;
            add({ Backend::RegInstr{
                Backend::RegInstrType::MV,
                rd,
                a0
            } });
        }
        return sp;
    }

    void convert_store_instr(const Pointer<Ir::StoreInstr> &instr) {

    }

    void convert_load_instr(const Pointer<Ir::LoadInstr> &instr) {

    }

    void convert_gep_instr(const Pointer<Ir::ItemInstr> &instr) {

    }

};

Backend::MachineInstrs FunctionConvertor::convert(const Ir::pInstr &instr)
{
    ConvertBulk bulk(func, allocate_register);
    switch (instr->instr_type()) {
        case Ir::INSTR_RET:
            bulk.convert_return_instr(std::dynamic_pointer_cast<Ir::RetInstr>(instr));
            break;
        case Ir::INSTR_BR:
            bulk.add({ Backend::JInstr{ func->name() + "_" + instr->operand(0)->usee->name() } });
            break;
        case Ir::INSTR_BR_COND:
            bulk.convert_branch_instr(std::static_pointer_cast<Ir::BrCondInstr>(instr));
            break;
        case Ir::INSTR_CALL:
            excess_arguments = std::max(excess_arguments, bulk.convert_call_instr(std::static_pointer_cast<Ir::CallInstr>(instr)));
            break;
        case Ir::INSTR_UNARY:
            bulk.convert_unary_instr(std::static_pointer_cast<Ir::UnaryInstr>(instr));
            break;
        case Ir::INSTR_BINARY:
            bulk.convert_binary_instr(std::static_pointer_cast<Ir::BinInstr>(instr));
            break;
        case Ir::INSTR_CAST:
            bulk.convert_cast_instr(std::static_pointer_cast<Ir::CastInstr>(instr));
            break;
        case Ir::INSTR_CMP:
            bulk.convert_cmp_instr(std::static_pointer_cast<Ir::CmpInstr>(instr));
            break;
        case Ir::INSTR_STORE:
            bulk.convert_store_instr(std::static_pointer_cast<Ir::StoreInstr>(instr));
            break;
        case Ir::INSTR_LOAD:
            bulk.convert_load_instr(std::static_pointer_cast<Ir::LoadInstr>(instr));
            break;
        case Ir::INSTR_ITEM:
            bulk.convert_gep_instr(std::static_pointer_cast<Ir::ItemInstr>(instr));
            break;
        case Ir::INSTR_ALLOCA: {
            auto type = to_pointed_type(instr->ty);
            auto size = type->length();
            if (size == 1) size = 4; // for bool
            assert(size % 4 == 0);
            local_variables += size;
            break;
        }
        case Ir::INSTR_FUNC:
        case Ir::INSTR_LABEL:
        case Ir::INSTR_PHI:
        case Ir::INSTR_SYM:
        case Ir::INSTR_UNREACHABLE:
            // just ignore all these ir
            break;
    }
    return bulk.bulk;
}

Backend::Block FunctionConvertor::generate_prolog() {
    Backend::Block bkd_block(func->name() + "_prolog");
    auto add = [&bkd_block](Backend::MachineInstr const& value) {
        bkd_block.body.push_back(value);
    };
    {
        int sp = local_variables + excess_arguments;
        add({ Backend::RegImmInstr {
            Backend::RegImmInstrType::ADDI, Backend::Reg::SP, Backend::Reg::SP, -sp
        } });
        add({ Backend::RegImmRegInstr {
            Backend::RegImmRegInstrType::SD, Backend::Reg::RA,  sp - 8, Backend::Reg::SP,
        } });
        add({ Backend::RegImmRegInstr {
            Backend::RegImmRegInstrType::SD, Backend::Reg::S0,  sp - 16, Backend::Reg::SP,
        } });
        add({ Backend::RegImmInstr {
            Backend::RegImmInstrType::ADDI, Backend::Reg::S0, Backend::Reg::SP, sp
        } });
    }
    Backend::Reg arg = Backend::Reg::A0;
    Backend::FReg farg = Backend::FReg::FA0;
    int sp = 0;
    int reg = 0;
    auto ft = func->function_type();
    for (auto&& at : ft->arg_type) {
        if (is_float(at)) {
            auto rd = (Backend::FReg) ~reg++;
            auto rs = farg;
            if (rs <= Backend::FReg::FA7) {
                add({  Backend::FRegInstr{
                    Backend::FRegInstrType::FMV_S,
                    rd,
                    rs
                } });
                farg = (Backend::FReg)((int)farg + 1);
            } else {
                add({ Backend::FRegImmRegInstr {
                    Backend::FRegImmRegInstrType::FLW, rd,  sp, Backend::Reg::SP,
                } });
                sp += 8;
            }
        } else {
            auto rd = (Backend::Reg) ~reg++;
            auto rs = arg;
            if (rs <= Backend::Reg::A7) {
                add({  Backend::RegInstr{
                    Backend::RegInstrType::MV,
                    rd,
                    rs
                } });
                arg = (Backend::Reg)((int)arg + 1);
            } else {
                add({ Backend::RegImmRegInstr {
                    Backend::RegImmRegInstrType::LD, rd,  sp, Backend::Reg::SP,
                } });
                sp += 8;
            }
        }
    }
    return bkd_block;
}

Backend::Block FunctionConvertor::generate_epilog() {
    Backend::Block bkd_block(func->name() + "_epilog");
    auto add = [&bkd_block](Backend::MachineInstr const& value) {
        bkd_block.body.push_back(value);
    };
    {
        int sp = local_variables + excess_arguments;
        add({ Backend::RegImmRegInstr {
            Backend::RegImmRegInstrType::LD, Backend::Reg::RA,  8, Backend::Reg::SP,
        } });
        add({ Backend::RegImmRegInstr {
            Backend::RegImmRegInstrType::LD, Backend::Reg::S0,  0, Backend::Reg::SP,
        } });
        add({ Backend::RegImmInstr {
            Backend::RegImmInstrType::ADDI, Backend::Reg::SP, Backend::Reg::SP, sp
        } });
        add({ Backend::ReturnInstr{} });
    }
    return bkd_block;
}

} // namespace BackendConvertor

