#include "convert_ir_to_asm.h"

#include <cstddef>
#include <ir_call_instr.h>
#include <ir_cast_instr.h>
#include <ir_cmp_instr.h>

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

namespace IrToAsm {

Backend::pModule Convertor::convert(const Ir::pModule &mod)
{
    auto bkd_mod = std::make_shared<Backend::Module>();

    for (auto &&i : mod->globs) {
        bkd_mod->globs.push_back(convert(i));
    }
    
    for (auto &&i : mod->funsDefined) {
        bkd_mod->funcs.push_back(convert(i));
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
    my_assert(false, "?");
    return Backend::Global("", 0);
}

Backend::Func Convertor::convert(const Ir::pFuncDefined &func)
{
    Backend::Func bkd_func(func->name());

    for (auto &&i : func->p) {
        bkd_func.body.push_back(convert(func, i));
    }

    return bkd_func;
}

Backend::Block Convertor::convert(const Ir::pFuncDefined &func, const Ir::pBlock &block)
{
    Backend::Block bkd_block(func->name() + "_" + block->label()->name());

    for (auto &&i : *block) {
        auto res = convert(func, i);
        bkd_block.body.insert(bkd_block.body.end(),
            std::move_iterator(res.begin()), std::move_iterator(res.end()));
        res.clear();
    }

    return bkd_block;
}

struct ConvertBulk {
    Backend::MachineInstrs bulk;
    int allocate_register = 32;

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
                break;
        }
        my_assert(false, "?");
    }

    static constexpr Backend::Reg NO_HINT = (Backend::Reg) 32;
    static constexpr Backend::FReg NO_HINT_F = (Backend::FReg) 32;

    Backend::Reg toReg(Ir::Val* val, Backend::Reg tmp_hint = NO_HINT) {
        auto name = val->name();
        // if it is a register, use it
        if (name[0] == '%') {
            return (Backend::Reg) -std::stoi(name.substr(1));
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
            return (Backend::FReg) -std::stoi(name.substr(1));
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
        }
        my_assert(false, "unreachable");
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
        }
        my_assert(false, "unreachable");
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
        add({ Backend::ReturnInstr{} });
    }

    void convert_cast_instr(const Pointer<Ir::CastInstr> &instr) {
        switch (instr->method()) {
            case Ir::CAST_PTRTOINT:
            case Ir::CAST_INTTOPTR:
            case Ir::CAST_TRUNC:
            case Ir::CAST_FPEXT:
            case Ir::CAST_FPTRUNC:
                my_assert(false, "unreachable");
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
            }

        }
    }

    void convert_branch_instr(const Ir::pFuncDefined &func, const Pointer<Ir::BrCondInstr> &instr) {
        auto rs = toReg(instr->operand(0)->usee);
        auto label1 = func->name() + "_" + instr->operand(1)->usee->name();
        auto label2 = func->name() + "_" + instr->operand(2)->usee->name();
        add({ Backend::BranchInstr{Backend::BranchInstrType::BNE, rs, Backend::Reg::ZERO, label1 } });
        add({ Backend::JInstr{ label2 } });
    }

    void convert_call_instr(const Pointer<Ir::CallInstr> &instr) {

        // arguments are not yet proceeded
        add({ Backend::CallInstr{instr->operand(0)->usee->name()} });
    }

};

Backend::MachineInstrs Convertor::convert(const Ir::pFuncDefined &func, const Ir::pInstr &instr)
{
    ConvertBulk bulk;
    switch (instr->instr_type()) {
        case Ir::INSTR_RET:
            bulk.convert_return_instr(std::dynamic_pointer_cast<Ir::RetInstr>(instr));
            break;
        case Ir::INSTR_BR:
            bulk.add({ Backend::JInstr{ func->name() + "_" + instr->operand(0)->usee->name() } });
            break;
        case Ir::INSTR_BR_COND:
            bulk.convert_branch_instr(func, std::static_pointer_cast<Ir::BrCondInstr>(instr));
            break;
        case Ir::INSTR_CALL:
            bulk.convert_call_instr(std::static_pointer_cast<Ir::CallInstr>(instr));
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
        case Ir::INSTR_LOAD:
        case Ir::INSTR_ITEM:
            break;
        case Ir::INSTR_FUNC:
        case Ir::INSTR_ALLOCA:
        case Ir::INSTR_LABEL:
        case Ir::INSTR_PHI:
        case Ir::INSTR_SYM:
        case Ir::INSTR_UNREACHABLE:
            // just ignore all these ir
            break;
    }
    return bulk.bulk;
}

} // namespace BackendConvertor

