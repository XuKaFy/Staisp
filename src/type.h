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

struct TypedSym
{
    TypedSym(Symbol sym, pType tr, bool is_const = false);

    Symbol sym;
    pType tr;
    bool is_const;
};

struct BasicType : public Type
{
    virtual TypeType type_type() const override { return TYPE_BASIC_TYPE; }
    ImmType ty;
};

enum CompoundTypeType {
    COMPOUND_TYPE_POINTER,
    COMPOUND_TYPE_STRUCT,
    COMPOUND_TYPE_ARRAY,
};

struct CompoundType : public Type {
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
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_POINTER; }
    size_t len;
    pType pointed_type;
};

struct ArrayType : public CompoundType {
    virtual CompoundTypeType compound_type_type() const { return COMPOUND_TYPE_ARRAY; }
    size_t len;
    pType elem_type;
};

typedef Pointer<int8_t> RawMemory;

struct Memory {
    Memory(size_t len)
        : len(len), mem(RawMemory(new int8_t[len])) {}
    size_t len;
    RawMemory mem;
};

struct MemoryRef
{
    MemoryRef(Memory mem)
        : begin(0), len(mem.len), mem(mem.mem) { }
    size_t begin;
    size_t len;
    RawMemory mem;
};
