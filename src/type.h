#pragma once

#include "def.h"
#include "imm.h"

enum TypeType {
    TYPE_BASIC_TYPE,
    TYPE_COMPOUND_TYPE,
};

struct Type {
    Type(bool is_const = false)
        : is_const(is_const) { }

    virtual TypeType type_type() const = 0;
    virtual Symbol type_name() const = 0;
    bool is_const;
};

typedef Pointer<Type> pType;

struct TypedSym
{
    TypedSym(Symbol sym, pType tr)
        : sym(sym), tr(tr) { }

    Symbol sym;
    pType tr;
};

struct BasicType : public Type
{
    BasicType(ImmType ty, bool is_const)
        : Type(is_const), ty(ty) { }

    virtual TypeType type_type() const override { return TYPE_BASIC_TYPE; }
    virtual Symbol type_name() const override;
    ImmType ty;
};

pType make_basic_type(ImmType ty, bool is_const);

enum CompoundTypeType {
    COMPOUND_TYPE_POINTER,
    COMPOUND_TYPE_STRUCT,
    COMPOUND_TYPE_ARRAY,
};

struct CompoundType : public Type {
    CompoundType(size_t len, bool is_const)
        : Type(is_const), len(len) { }

    virtual TypeType type_type() const override { return TYPE_COMPOUND_TYPE; }
    virtual CompoundTypeType compound_type_type() const = 0;
    size_t len;
};

struct StructType : public CompoundType {
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_STRUCT; }
    size_t len;
    Vector<TypedSym> elems;
};

struct PointerType : public CompoundType {
    PointerType(pType ty, bool is_const)
        : CompoundType(ARCH_BYTES, is_const), pointed_type(ty) { }

    virtual Symbol type_name() const override;
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_POINTER; }
    pType pointed_type;
};

pType make_pointer_type(pType ty, bool is_const);

struct ArrayType : public CompoundType {
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_ARRAY; }
    virtual Symbol type_name() const override;
    
    size_t elem_count;
    pType elem_type;
};

bool is_signed_imm_type(pType tr);
bool is_float(pType tr);
int bits_of_type(pType tr);
