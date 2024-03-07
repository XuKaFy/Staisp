#pragma once

#include "def.h"
#include "imm.h"

enum TypeType {
    TYPE_BASIC_TYPE,
    TYPE_COMPOUND_TYPE,
};

struct Type {
    virtual TypeType type_type() const = 0;
};

typedef Pointer<Type> pType;

struct BasicType : public Type
{
    virtual TypeType type_type() const override { return TYPE_BASIC_TYPE; }
    ImmType ty;
};

enum CompoundTypeType {
    COMPOUND_TYPE_POINTER,
    COMPOUND_TYPE_ARRAY,
};

struct CompoundType : public Type {
    virtual TypeType type_type() const override { return TYPE_COMPOUND_TYPE; }
    virtual CompoundTypeType compound_type_type() const = 0;
    size_t len;
    pType pointed_type;
};

struct PointerType : public CompoundType {
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_POINTER; }
    size_t len;
};

struct ArrayType : public CompoundType {
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_POINTER; }
    size_t len;
};
