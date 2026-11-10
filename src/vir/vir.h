// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// VIR: the small, front-end-agnostic verification IR (internal/encoding.md section 2;
// decisions 0010 integer semantics, 0012 scalar encoding).
//
// The IL-to-VIR encoder (src/backend/encode.cpp) builds a `Program` with the `Builder`; `check_program`
// (check.h) type-checks it and fails closed; `lower_program` (lower.h) gives it integer semantics and
// produces the Viper-level program that src/viper prints.  Nothing here knows about EDG.
//
// Conventions
//  * Everything is owned by the `Program` (arena style): builder functions return pointers that stay valid
//    until the Program is destroyed.  Expression nodes are immutable and may be shared (a DAG is fine).
//  * Every node carries the C/C++ position it came from (`SrcLoc`).  An obligation of a node (overflow,
//    division by zero, ...) is reported at that node's `loc`: put the operator token's position on an
//    operator node, the `(` on a call, the conversion's position on a conversion (internal/encoding.md
//    section 7).  Inside a `CERASTES_*` macro argument use the original position (0006 item 10).
//  * A VIR value is a mathematical integer (`Int`) in [T_MIN, T_MAX] of its IntType, or a `Bool`.  The
//    VIR is C/C++-neutral: comparisons and logical operators yield `Bool`.  C's `int` truth values are
//    the encoder's job and are spelled with `to_bool` / `bool_to_int` (0010 item 6).
//  * VIR performs no implicit conversion.  Every operand type must be exactly what the operation requires
//    (check.h), so a conversion the IL left implicit fails closed instead of being computed in the wrong type.
#pragma once

#include <deque>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace cerastes::vir {

// ---- positions -----------------------------------------------------------------------------------------

// A C/C++ source position.  `line == 0` means "unknown" (check.h rejects an unknown position on any node
// that can produce an obligation).
struct SrcLoc
{
    std::string file;
    int line {0};
    int column {0};
    bool known() const
    {
        return line > 0;
    }
};

// ---- types ---------------------------------------------------------------------------------------------

// An integer type of the target, filled from EDG's configuration at run time (0010 item 2).  Two IntTypes
// are the same type for VIR iff they have the same `bits` and `is_signed` (`name` only labels messages):
// `unsigned int` and `char32_t` are one type, `char` and `signed char` are one type.  `bits` is at most 128.
// An enumeration (decision 0018 item 3) is the same type only as itself, and its values are [lo, hi], not the
// range of its bits and signedness (which are those of its underlying or compatible type).
struct IntType
{
    std::string name; // "int", "unsigned long", "char", ... (for messages and Viper comments)
    int bits {0};
    bool is_signed {false};
    bool is_enum {false};   // an enumeration type (Program::enum_type)
    bool is_scoped {false}; // a C++ scoped enumeration
    // an enumeration's value range, decimal (dec::range_of)
    std::string lo;
    std::string hi;
};

struct StructType;

struct Type
{
    // Ptr (decisions 0032 item 2, 0037 items 2-3, 0040 item 2): a pointer to `pointee`.  Accepted by check_program for a local variable
    // (address-taken or not), for an expression, for a parameter and for a result, when the pointee is a Bool or Int (an enumeration included)
    // or itself an accepted pointer, at any depth (`int **`, `int ***`: a pointer variable whose address is taken is a cell of a pointer
    // type).  Every other pointer type (a pointee that is void, at any level) is built as is and rejected there.
    // Struct (decision 0043 item 3; appended): a C structure of the Program (Program::add_struct), the type of a struct local (always a
    // cell, address-taken) and the pointee of a pointer.  A struct VALUE (decision 0044 item 2: a tuple of the struct's leaves, in struct_leaves
    // order) exists only between a few producers and consumers, and check_program rejects it in every other position: producers are a `Call` of a
    // struct-returning function, an `Aggregate`, a struct-typed `Field`, and a `Formal` or the `Result` of a contract directly under a `Field`
    // (a `Formal` may also sit in an `Old` under a `Field`); consumers are a call argument matching a struct parameter, a `Return` matching a
    // struct result, a struct `Store` whose value is a `Call`, an `Aggregate` or a `Field` chain over one of them, an `ExprStmt` of a `Call`,
    // and a `Field`.  A struct local, a copy and an initializer stay expanded by the encoder into leaf stores (decision 0043).
    // Pointers to structs are accepted at any depth like the other pointers (`struct S *`, `struct S **`).
    enum class Kind
    {
        Void,
        Bool,
        Int,
        Ptr,
        Struct
    };
    Kind kind {Kind::Void};
    const IntType* int_type {nullptr};       // Kind::Int only; owned by the Program
    const Type* pointee {nullptr};           // Kind::Ptr only; interned by Program::pointer_to, owned by the Program
    const StructType* struct_type {nullptr}; // Kind::Struct only; owned by the Program, compared by identity

    static Type void_type()
    {
        return Type {Kind::Void, nullptr, nullptr, nullptr};
    }
    static Type bool_type()
    {
        return Type {Kind::Bool, nullptr, nullptr, nullptr};
    }
    static Type integer(const IntType* t)
    {
        return Type {Kind::Int, t, nullptr, nullptr};
    }
    static Type structure(const StructType* s)
    {
        return Type {Kind::Struct, nullptr, nullptr, s};
    }

    bool is_void() const
    {
        return kind == Kind::Void;
    }
    bool is_bool() const
    {
        return kind == Kind::Bool;
    }
    bool is_int() const
    {
        return kind == Kind::Int;
    }
    bool is_ptr() const
    {
        return kind == Kind::Ptr;
    }
    bool is_struct() const
    {
        return kind == Kind::Struct;
    }
    // Same VIR type (Int types: same bits and signedness; an enumeration only itself; pointers: same pointee type, so `long*` and
    // `long long*` are one type, as `long` and `long long` are; structs: the same StructType object).
    bool operator==(const Type& o) const
    {
        if (kind != o.kind)
        {
            return false;
        }
        if (kind == Kind::Ptr)
        {
            return pointee != nullptr && o.pointee != nullptr && *pointee == *o.pointee;
        }
        if (kind == Kind::Struct)
        {
            return struct_type == o.struct_type;
        }
        if (kind != Kind::Int)
        {
            return true;
        }
        if (int_type->is_enum || o.int_type->is_enum)
        {
            return int_type == o.int_type;
        }
        return int_type->bits == o.int_type->bits && int_type->is_signed == o.int_type->is_signed;
    }
    bool operator!=(const Type& o) const
    {
        return !(*this == o);
    }
    // "bool", "void", the int type's name, the struct's C name ("struct S"), or the pointee's followed by "*" ("int*",
    // "bool**")
    std::string str() const;
};

// A member of a C structure, in declaration order (decision 0043 item 2): `type` is a Bool or Int (an enumeration included), an accepted
// pointer (the innermost pointee may be a struct, also the struct itself: `struct node *next`) or a nested accepted struct.
struct StructMember
{
    std::string name;
    Type type;
    SrcLoc loc;
    // `[[cerastes::owning]]` on the field (decision 0055 item 2): a pointer to an object type that owns the block it points to.  The encoder sets it after
    // add_member; no consumer reads it before the closure (item 3).  The owning-field graph is acyclic (check_program).
    bool owning {false};
};

// A C structure type (decision 0043 items 2 and 3), interned in the Program by Program::add_struct (one object per C type: two structs are
// the same VIR type iff they are the same object).  Its layout in Viper: an object is a slot, member k is the block `member(slot, k)` of
// length 1 and a scalar or pointer member the leaf cell `loc(member(slot, k), 0)`; a nested struct member is itself a slot there.  A struct has
// no value of its own, only leaves.
struct StructType
{
    int id {0}; // the index in Program::structs(); part of the Viper name
    // the C tag (`S` of `struct S`); may be empty for an untagged struct (named by its typedef: the encoder sets `tag`
    // then)
    std::string tag;
    // the unique display and cell name, written like an enumeration's `enum_<n>_<name>`: `struct_<id>_<tag>` with every
    // character
    std::string name;
    // that is not a letter, digit or underscore of the tag written `_`.  Always a valid Viper identifier part, unique in the Program
    std::string c_name; // the C type name for messages ("struct S"; the encoder sets what the program spells)
    SrcLoc loc;         // the definition
    std::vector<StructMember> members; // in declaration order; add with Program::add_member
};

// One leaf of a struct type: a scalar or pointer member reached through nested struct members (decision 0043 item 3).  `path` is the
// member indices from the root (the last one the leaf itself), `type` the leaf's Bool, Int or Ptr type, `text` its source spelling.
struct StructLeaf
{
    std::vector<int> path;
    Type type;
    std::string text;
    // the member is `[[cerastes::owning]]` (decision 0055 item 2): a store to this leaf takes item 4's value rule
    bool owning {false};
};
// The leaves of `s` in declaration order, depth first (a nested struct's leaves in place).  `text` is `root_text` followed by `.name` for each
// member on the way (`->name` for the first when `arrow`): struct_leaves(s, "t", false) gives "t.x", "t.u.y", struct_leaves(s, "p", true) "p->x", "p->u.y".
// An empty `root_text` gives "x", "u.y".
std::vector<StructLeaf> struct_leaves(const StructType& s, const std::string& root_text = "", bool arrow = false);
// The member paths at which `a` holds a slot of struct type `b`: `{}` (the empty path) when `a` is `b` itself, then each nested member that is `b`
// (and `b`s nested deeper), in declaration order, depth first.  Empty vector: `a` does not contain `b` (decision 0043 item 9: two struct atoms
// overlap iff one root is a nested slot of the other).
std::vector<std::vector<int>> struct_slots_of(const StructType& a, const StructType& b);
inline bool struct_contains(const StructType& a, const StructType& b)
{
    return !struct_slots_of(a, b).empty();
}
// The type of the member reached from `s` by `path` (the empty path: `s` itself); a Void type when the path is invalid.
Type struct_member_type(const StructType& s, const std::vector<int>& path);

// ---- expressions -----------------------------------------------------------------------------------------

struct Function;
struct Var;
struct Global;

// Neg, Plus: Int -> that IntType (Neg checks the result's range).  Not: Bool -> Bool.
// BitNot (decision 0023): Int -> that IntType, typed like Neg; `~a` is `-1 - a` (signed) or `MAX - a` (unsigned), always in range,
// no obligation.
enum class UnaryOp
{
    Neg,
    Plus,
    Not,
    BitNot
};
enum class BinaryOp
{
    Add,
    Sub,
    Mul,
    Div,
    Rem, // Int x Int (same IntType) -> that IntType
    Shl,
    Shr, // Int x Int (count: any IntType) -> the left operand's IntType
    Lt,
    Le,
    Gt,
    Ge, // Int x Int (same IntType) -> Bool
    Eq,
    Ne, // Int x Int (same IntType) or Bool x Bool -> Bool
    LAnd,
    LOr, // Bool x Bool -> Bool; the right operand is evaluated conditionally
    BitAnd,
    BitOr,
    BitXor // Int x Int (same IntType) -> that IntType; two's complement on the mathematical values, never leave
           // the type, no obligation (decision 0023).  The operands' type is the computation type; the front end
           // (the encoder) inserts the usual arithmetic conversions
};

// The permission an `Atom` stands for (decision 0037 item 6).  `Owns` and `Reads` take a place `*p` (p a pointer parameter) and `Live` the
// pointer `p`.  `OwnsBlock` (decision 0047 item 4; appended) takes the pointer `p` (a pointer `Formal`, or in an `ensures` the `Result`) and stands
// for the whole heap block `p` starts, with write permission: offset 0, the free token, the liveness cell, the length and every leaf cell.  It has no
// ghost amount (`amount` stays -1, requires_atom_count and contract_atoms skip it) and no partner rule: in a `requires` alone the function takes
// the block, in an `ensures` alone (on the `Result`) it hands the block to the caller, in both on one `Formal` it borrows and returns it.
enum class AtomKind
{
    Owns,
    Reads,
    Live,
    OwnsBlock
};

struct Expr
{
    enum class Kind
    {
        Const,     // `value`: a decimal integer (Int type; must lie in the type's range) or "true"/"false" (Bool)
        Local,     // `var`: a local, a parameter's local copy, or (read by `invariant`/`check`) the same
        Formal,    // `index`: parameter `index` as passed in (its ENTRY value); contracts only
        Result,    // the function's result; `ensures` only
        Old,       // `a`: the value of `a` on entry; `ensures` only; `a` must not mention Result or Local
        Unary,     // `uop`, `a`
        Binary,    // `bop`, `a`, `b`
        Cond,      // `a` ? `b` : `c` (a: Bool; b, c: same type); b and c are evaluated conditionally
        Convert,   // `a` (Int) converted to Int type `type`; `explicit_cast` says whether the program wrote it
        ToBool,    // `a` (Int) != 0, a Bool
        BoolToInt, // `a` (Bool) as 0 or 1 of Int type `type`
        Call,      // `callee`(`args`...): type = callee's return type
        ReportAt,  // `a`, but obligations inside it are reported at `loc` with a note at their own position
        // an assignment used as a value (decision 0020): `var` := `a` (already of `var`'s type); the value of the node
        // is `a` (the new value, `type` = `var`'s type), or, when `yields_old`, the old value `b` (x++, x--).  `b` is
        // null or the one `Local var` node that `a` also uses for the old value: the lowering reads it once.  Allowed
        // in code and in a loop condition, never in an annotation; reachable once per function; see
        // `sequence_problems`.  `var` may be a cell (an address-taken local; decision 0039 item 2): the write is then a
        // write effect of the full expression with the permission and flag obligations of a cell write, and a read of
        // `var` is a place read (E3 for memory).
        AssignE,
        // `a`, then `b` (decision 0020): `type` = `b`'s type, never void; `a` may be a void call; a sequence point
        // between them
        Comma,
        // ---- pointers (decision 0032; appended, so every earlier kind keeps its place) ----
        // the null pointer constant: `type` is a Ptr.  In `==`/`!=` and as an arm of `?:` it takes the other operand's
        // type: check_program accepts a Null of any pointer type there (decision 0032 item 7), and the value is `null`
        // whatever the type.
        Null,
        // `&var`: type = Ptr(var's type); `var` must be a local of this function with `address_taken` (a cell, decision
        // 0032 item 4; check_program refuses anything else).  A pointer value: no obligation of its own.
        AddrOf,
        // `*a` used as a READ (a: a Ptr; type = the pointee).  Obligations: the null check, the permission check and
        // the flag (decision 0032 item 6).  A write through a pointer is the statement Stmt::Store, never a Deref. In a
        // CONTRACT (decision 0037 item 5; Ctx::Requires/Ensures): `a` is a `Formal` of pointer type or, in an
        // `ensures`, the `Result` of a function returning a pointer (decision 0040 item 4: `*result` as a value), or a
        // GlobalAddr of a file-scope global (decision 0041 item 7: the read of `g`), or (decision 0045 item 2) a place
        // reached through loads and pointer leaves from one of those (`**pp`: Deref(Deref(Formal pp)); `*gp`:
        // Deref(GlobalAddr gp); `p->q->x`; `*s.q`: Deref(Field(Formal s, q)); `*r.q`), and nothing else (a local, `Old`
        // inside the place).  In a `requires` it reads the cell's value on entry, in an `ensures` its value on return,
        // and `Old(Deref(Formal p))` its value on entry; `Old(Deref(Result))` is a check_program error (the result has
        // no value on entry).  Lowering frames each read per path (item 7), keyed by the place `p` or `result`.  The
        // value of `*pp` of a `T **` parameter is a pointer (a Ptr-typed Deref).
        Deref,
        // ---- permissions in contracts (decision 0037 item 5; appended) ----
        // `atom` of the pointer `a` (a Ptr `Formal`: the parameter whose pointee is meant; in an `ensures` its entry
        // value), a Bool; in a contract, never inside `Old`; and, from decision 0040 item 5, in the `invariant` clauses
        // of a `While` or `DoWhile` loop, where `a` is a `Local` of pointer type (a pointer variable, read at each
        // iteration's start; see loop_atoms).  A measure, a `check`, code and the invariant of a `DoOnce` loop hold
        // none (check_program errors). Owns: `owns(*p)`, Reads: `reads(*p)` (a pointee that is a Bool, an Int or a
        // pointer), Live: `live(p)`.  `amount` names its ghost permission amount (item 6): the atoms of the `requires`,
        // in evaluation order (contract_atoms), are numbered 0, 1, ...; an atom of an `ensures` carries the number of
        // the `requires` atom it returns (the same kind and the same parameter, each partner at most once).  An
        // invariant atom's `amount` is unused (-1; lowering numbers its own). Positions (item 5): contract_atoms and
        // loop_atoms collect exactly the allowed ones; any other Atom is a check_program error. Decision 0041: in a
        // contract `a` may also be a GlobalAddr (`owns(g)`, `reads(g)`; the Owns of a `static` local's implicit atom
        // has `implicit_atom`).  An atom pairs with its `requires` partner by place (atom_place_key), not by the
        // parameter's index.
        Atom,
        // ---- a write through a pointer used as a value (decision 0039 item 2; appended) ----
        // `*c := a` as a value: `c` is the address (a Ptr), `a` the stored value (already of the pointee's type); the
        // value of the node is `a` (the new value, `type` = the pointee's type), or, when `yields_old`, the old value
        // `b` (`(*p)++`, `(*p)--`). `b` is null or the one `Deref` node of `c` that `a` also reads for the old value,
        // so the pointer is lowered once, as for Stmt::Store (Builder::compound_store_expr, inc_dec_store_expr share
        // the `c` node).  Allowed wherever AssignE is (code and a loop condition), never in an annotation; reachable
        // once per function; see `sequence_problems`.  A write effect of the full expression: the null, permission and
        // flag obligations of a write through `c`.  The spelling of the target for messages is "*" + c->c_text.
        StoreE,
        // ---- globals (decision 0041; appended) ----
        // `&g` of the mutable global or `static` local `global` (Program::globals()): type = Ptr(global->type); a
        // pointer value with no obligation (a global's block is allocated for the whole run: no allocation, no scope
        // exit).  A read of `g` is `Deref(GlobalAddr)`, a write `Store`/`StoreE` with a GlobalAddr address, an atom
        // `Atom(owns/reads, GlobalAddr)`; `*&g` is the same tree.  In CODE it stands where AddrOf does (a pointer
        // value, the address of a Deref/Store/StoreE); a `static` local's GlobalAddr only in its owner function.  In a
        // CONTRACT (Ctx::Requires/Ensures) it is a pointer value like any other (`p == &g`, `r == &g`, decision 0041
        // item 7), the operand of a Deref (the read of `g`) or of an Atom (`owns(g)`, `reads(g)`); a static local's
        // address stays out of contracts except as its implicit atom (Owns with `implicit_atom`).  In a loop's
        // invariant or measure and a function's `decreases` it is accepted like any pointer node (decision 0038 item 6:
        // `g >= 0`, `gp == &g` read the loop's places, framed by lowering); in a named constant, a global's initializer
        // or a case value it is a check_program error (no_ptr_). `Atom(Live, GlobalAddr)` and a GlobalAddr atom in a
        // loop's invariant are check_program errors (the encoder reports them first).
        GlobalAddr,
        // ---- structures (decision 0043; appended) ----
        // `&a->m`: `a` is a pointer to a struct, `index` the member k of that struct (Type::struct_type->members[k]);
        // type = Ptr(the member's type) (a nested struct member gives a pointer to that struct: `&p->t` is the slot of
        // `t`; `p->t.x` is MemberAddr(MemberAddr(p, t), x)). Lowering: `loc(member(a, k), 0)`.  c_text: "&" + the
        // member's place spelling (Builder::member_addr: `&p->x`, `&s.x`, `&s.t.x`); Builder::deref of it spells "p->x"
        // (without the "&").  A struct place is always reached through this node. PLACE versus VALUE (a convention, no
        // flag; lowering reads the context): a MemberAddr that is the operand `a` of a Deref, the `addr` of a
        // Stmt::Store, the `c` of a StoreE or the operand `a` of another MemberAddr that is itself in one of those
        // positions is a PLACE ACCESS (`p->x`, `s.t.x`, a read or write of a leaf): it checks `a != null`
        // (null-dereference) and the leaf's permission and flag, and nothing about the liveness of `a`'s pointer
        // variable (decision 0032 item 6: a member access is not a use of `p`'s value).  Any other MemberAddr is an
        // address used as a VALUE (`&p->x` as an argument, an initializer, a comparison operand): arithmetic on `a`
        // (decision 0028 item 3a), checking `a != null` and `a`'s liveness.  In a CONTRACT only as a place (the operand
        // of a Deref or an Atom, through further MemberAddrs) rooted at a Formal, or at the Result in an `ensures`
        // read; atoms and values of struct places are written `Atom(owns, MemberAddr(Formal p, k))` and
        // `Deref(MemberAddr(Formal p, k))`.  No obligation of its own.
        MemberAddr,
        // ---- struct values (decision 0044 item 2; appended) ----
        // member `index` of the struct VALUE `a` (`a.m`): type = the member's type (a nested struct member is a struct
        // value again: `a.t.y` is Field(Field(a, t), y)).  `a` is a struct-typed producer: a Call, an Aggregate,
        // another Field or, in a contract, a Formal or the Result (an `Old` of a Formal or of a Field chain over one,
        // in an `ensures`) directly under the Field.  c_text: `a`'s followed by `.m` when `a` has one. Lowering reads
        // the member's leaf (or leaves, for a nested struct) of the value's tuple; the node adds no obligation of its
        // own.
        Field,
        // a struct value built from its leaves: `type` is a Struct; `args` holds one expression per leaf of the struct
        // in struct_leaves order (depth first), each exactly the leaf's VIR type (a pointer leaf's Null has that
        // pointer type).  Allowed in code only (never in a contract). The arguments are unsequenced siblings, like the
        // arguments of a call but without a sequence point after them.
        Aggregate
    };
    Kind kind {Kind::Const};
    Type type; // the type of the value (Void only for a Call of a void function)
    SrcLoc loc;
    std::string value;            // Const
    const Var* var {nullptr};     // Local
    int index {0};                // Formal
    UnaryOp uop {UnaryOp::Neg};   // Unary
    BinaryOp bop {BinaryOp::Add}; // Binary
    const Expr* a {nullptr};
    const Expr* b {nullptr};
    const Expr* c {nullptr};
    bool explicit_cast {false}; // Convert
    // How the program spells this node's type (a C/C++ type name such as "signed char" or "unsigned short"),
    // for messages only; empty means "use type.str()".  IntTypes are interned by shape, so the spelling the
    // user wrote lives here: set it on Convert nodes and on constants where it helps (Builder::spelled).
    std::string c_type_name;
    // The source spelling of this node for messages (decision 0032 item 6): `p` on a pointer Local, `&x` on an AddrOf, `*p` on a Deref
    // (Builder::deref derives it from the operand's when that has one).  Optional: empty means "unknown" and the lowering words the
    // message without the name.  It never affects the verification.  Set it with Builder::text.
    std::string c_text;
    // MemberAddr only (decision 0045 item 12): `c_text` ("&" + the place) is the user's own spelling of a place reached through a load or a pointer leaf
    // (`&p->q->x`), and `place_text`/`member_place` return it instead of rebuilding `(p->q)->x`.  Set by the encoder for the new places only: every load-free place
    // and every place of code keeps member_place's text (existing position maps must not change).
    bool user_spelling {false};
    const Function* callee {nullptr}; // Call
    std::vector<const Expr*> args;    // Call: already converted to the parameter types
    AtomKind atom {AtomKind::Owns};   // Atom
    int amount {-1};                  // Atom: the number of the atom's ghost amount (see Kind::Atom)
    bool yields_old {false};          // AssignE, StoreE: the value is the old value `b` (x++, x--), not the new one
    const Global* global {nullptr};   // GlobalAddr (decision 0041): the global; owned by the Program
    // Atom: added by the encoder for a `static` local (decision 0041 item 12), not written by the user: an Owns of a
    // GlobalAddr whose global has an `owner`; messages say "the `static` local `s` of `f`"; or (decision 0052 item 2,
    // question 159) the merged borrow of an
    // `owning` pointer parameter: an Owns of the parameter's Formal in the `requires` and the `ensures`
    bool implicit_atom {false};
    // OwnsBlock: spelled `cerastes_owns_vlibc_block` (decision 0052 item 7): the block is vlibc's (its token's value is
    // `false`, decision 0050 item 3), not the platform's.
    bool vlibc_block {false};
};

// ---- statements ------------------------------------------------------------------------------------------------

// A contract or invariant clause: a Bool expression and the position of the clause (the position a
// postcondition or invariant failure is reported at, and the note of a precondition failure).
struct Clause
{
    const Expr* expr {nullptr};
    SrcLoc loc;
};

// The shape of a loop statement (decision 0014 items 4 and 5).
//  While:   `while (c) { body; step }`; the invariants hold wherever C evaluates `c`.
//  DoWhile: `do { body } while (c)`; the invariants hold at the start of every iteration; `c` is evaluated after the body
//           (and after each `continue`), is encoded for its obligations and may contain no call.
//  DoOnce:  `do { body } while (c)` where `c` is a constant zero: the body runs once, no iteration follows another, and no
//           measure is required; lowered as straight-line code.
enum class LoopForm
{
    While,
    DoWhile,
    DoOnce
};

struct Stmt
{
    enum class Kind
    {
        // `var` = `expr` (the type of `expr` is the type of `var`); `expr == nullptr`: declared WITHOUT an initializer,
        // so `var` holds no value until an Assign gives it one (decision 0019: reading it before is an `uninitialized`
        // error)
        Decl,
        Assign,   // `var` = `expr` (same type)
        ExprStmt, // evaluate `expr` (a call, a discarded value, a `(void)e`): only its obligations and effects count
        If,       // if (`expr`) then_body [else else_body]; either body may be null
        // a loop (see `form`): while (`expr`) invariants body [step]; `expr` may have calls and side effects (decisions
        // 0020, 0021), except a `DoOnce` condition (check.h)
        While,
        Return, // return [`expr`]: expr's type is the function's return type, absent iff the function is void
        Check,  // check(`expr`): a Bool, asserted (category `check`)
        Block,  // `stmts`
        Switch, // switch (`expr`) body; `expr` is the (promoted) controlling Int expression, `body` a Block; `jump_id`
        // a `case value:` / `default:` marker inside a Switch's own body Block: `expr` is the value (an Int Const of
        // the scrutinee's type) or null for `default`
        Case,
        Break,    // break: leaves the loop or switch whose `jump_id` is `jump_id`
        Continue, // continue: goes to the end of the iteration of the loop whose `jump_id` is `jump_id`
        Goto,     // goto: a user `goto` to the Label whose `jump_id` is `jump_id` (decision 0015 item 2; goto_problems)
        Label,    // a user label `label_name:` with id `jump_id` (a direct statement of a Block)
        // `*addr = expr` (decision 0032): a write through a pointer.  `addr` is a Ptr expression, `expr` has the
        // pointee's type.  The two are unsequenced operands (sequence_problems).  `*p op= e`, `++*p` and `(*p)--` are
        // Stores whose `expr` reads a Deref that SHARES the `addr` node (Builder::compound_store, inc_dec_store): the
        // lowering evaluates the pointer once.  The target's spelling for messages is "*" + addr->c_text.
        Store
    };
    Kind kind {Kind::Block};
    SrcLoc loc;
    const Var* var {nullptr};   // Decl, Assign
    const Expr* expr {nullptr}; // see Kind
    const Expr* addr {nullptr}; // Store: the pointer written through
    SrcLoc end_loc; // Block: the closing brace (decision 0032 item 8: where leaving the block's scope is reported);
                    // required (known) by check_program on a Block that declares a cell; unknown elsewhere is fine
    const Stmt* then_body {nullptr}; // If
    const Stmt* else_body {nullptr}; // If
    std::vector<Clause> invariants;  // While: the user's `invariant` clauses, in order
    std::vector<Clause> measure;     // While: the termination measure (`decreases`, decision 0013): one or more Int
                                     // expressions, compared lexicographically; required (check.h)
    const Stmt* body {nullptr};      // While
    std::vector<const Stmt*> stmts;  // Block
    LoopForm form {LoopForm::While}; // While
    // While: a `for` loop's increment (an Assign or ExprStmt, or a Block of them: a discarded comma `i++, j--`), run
    // after the body and after each `continue`; may be null
    const Stmt* step {nullptr};
    int jump_id {0};          // While, Switch: this construct's id (> 0 when a Break or Continue may name it;
                              // unique within a Function, Builder::jump_id()); Break, Continue, Goto: the target's id;
                              // Label: its own id
    std::string label_name;   // Label: the user's name
    bool from_assert {false}; // Check: written as C's `assert(e)`: `expr` is typed like code (calls allowed) and a
                              // failure says `assert` (decision 0015 item 6)
    std::string folded;       // Case (not default): the front end's value of the label, a decimal Int literal of the
    // scrutinee's type; the dispatch compares with it and a prologue guard proves `expr` equals
    // it (decision 0017 item 5).  Empty for `default`.
};

// ---- functions and programs ----------------------------------------------------------------------------------

struct Var
{
    int id {0};       // unique within its function
    std::string name; // the C/C++ name (for messages and generated identifiers)
    // Bool, Int, a pointer or (decision 0043 item 4) a struct: a struct local is ALWAYS a cell (`address_taken`, set by
    // the
    Type type;
    // encoder; check_program refuses a struct Var without it), declared without an initializer (`Decl` with a null `expr`: its
    // leaves are written by Stores, initializers and copies being expanded by the encoder); `AddrOf` of it is the slot
    SrcLoc loc;                 // the declaration
    std::string c_type_name;    // the C/C++ spelling of the type, for messages (empty: use type.str())
    bool is_param_copy {false}; // the local copy of a parameter (0012 item 2)
    // The hidden object of a compound literal `&(T){...}` (decision 0044 item 7): a cell the encoder declares at the start of the block that holds the literal and
    // writes with a StoreE inside the expression `Comma(StoreE(&cl, value), &cl)`.  `name` is how messages spell the literal (`(struct S){...}`), never an internal
    // name.  check_program admits a struct StoreE only at such a variable, with an Aggregate, as the discarded left operand of a Comma.
    bool compound_literal {false};
    int param_index {-1}; // for a parameter copy
    // The program takes this variable's address (EDG's `address_taken`, decision 0032 item 4): the variable is a CELL, an object in a
    // one-slot block that a pointer may reach.  Set with Function::mark_address_taken.  Assign, Decl and AssignE of it keep their kinds (the
    // lowering dispatches on the flag; an AssignE of a cell is accepted from decision 0039 item 2).  A pointer variable may be a cell too
    // (decision 0040 item 2: its field is the pointer-typed `v_ptr_...`).  On a PARAMETER COPY (decision 0037 item 4) the flag is accepted
    // (a scalar or a pointer parameter): the copy is allocated as a one-slot block at entry and holds the formal's value; the body's scope exit
    // releases it; contracts keep reading the Formal.
    bool address_taken {false};
    // `[[cerastes::owning]]` on an automatic pointer local (decision 0052 item 3): the variable owns the block it
    // points to (the value rule, the scope-exit and reassignment rows, the loop place on its current value).  The
    // encoder sets it, on the Var that new_local returns, only on a pointer variable that is neither address-taken nor
    // a parameter copy nor a compound literal, of a pointer-to-object type that is not `void *` and not a pointer to a
    // `const` or `volatile` type; check_program refuses any other Var with it.
    bool owning {false};
};

// A named constant (decision 0017): a file-scope `const` object (global), or a `const` local of a function that a case
// value names.  Its uses are literals holding `value`; `init` is the initializer as written, checked in the prologue of
// every method (obligations, then an internal guard that it equals `value`).
//
// A static assertion (decision 0024 item 2) is the same record with `kind` StaticAssertion: `type` is Bool, `init` is the
// condition as written as a closed Bool expression, `value` is "1" (EDG accepted the assertion, so its condition is true; the
// encoder's perturbation hook may set it to "0" so that the guard fails, 0017 item 9), and the
// prologue checks the condition's obligations at their own positions, then an internal guard that it holds.  Add one with
// Program::add_static_assertion / Function::add_local_static_assertion; the list order is the order of the prologue.
struct NamedConst
{
    enum class Kind
    {
        Constant,        // a `const` object (the default: every existing construction)
        StaticAssertion, // a `static_assert` / `_Static_assert` (decision 0024)
    };
    int id {0};                 // unique within its list
    std::string c_name;         // the C/C++ name (messages and the Viper comment)
    Type type;                  // Bool or Int
    SrcLoc loc;                 // the declaration (the guard's position)
    const Expr* init {nullptr}; // the initializer as written: call-free, no Local/Result/Formal/Old, exactly `type`;
                                // null for C's tentative definition (value "0"): nothing is checked
    std::string value;          // the front end's value, a decimal literal of `type` ("0"/"1" for Bool)
    Kind kind {Kind::Constant}; // appended with a default: every existing construction is unchanged
};

// A mutable global variable or a `static` local (decision 0041 items 2-5, 12): one block of the program, named by a `unique` 0-ary Viper function
// `GB_...(): Block`, its cell `loc(GB_g(), 0)`.  It is not a `Var` (a Var's id is unique per function only); it is reached through
// `Expr::Kind::GlobalAddr`.  Created with Program::add_global, in declaration order (the order of `program_start`'s rows and of the prelude); the
// initial-value fields are filled by the encoder after every global exists (an `&h` initializer may name a later global).
enum class GlobalInit
{
    Value,
    Null,
    Addr,
    Unknown
};

// The initial value of one leaf of a struct global (decision 0044 item 8): the same four kinds as a scalar global's, with the fields each names.  `kind`
// and the fields it names are read as Global's own are (Global::Init); `loc` is the leaf's position (the initializer element, or the global when
// the leaf has none), unknown means the global's.
struct GlobalLeaf
{
    GlobalInit kind {GlobalInit::Value};
    // Value, Unknown: the element as written (closed, call-free, exactly the leaf's type), or null
    const Expr* init {nullptr};
    std::string value {"0"};        // Value: EDG's value, a decimal literal of the leaf's type ("0"/"1" for bool)
    const Global* target {nullptr}; // Addr: another Global of the program
    SrcLoc loc;
};

struct Global
{
    // The value the global has when the program starts (decision 0041 item 3), for the trusted `program_start`.
    //  Value:   a bool or an integer (also an enumeration): `value`, EDG's value (a decimal literal of `type`, "0"/"1" for bool; "0" for a tentative
    //           definition and for a definition with no initializer), and `init`, the initializer as written (checked in every method's prologue
    //           like a named constant's, 0017 item 4: closed, call-free, exactly `type`; null: nothing to check).
    //  Null:    a pointer whose value is null (no initializer, or a null constant).
    //  Addr:    a pointer whose value is `&target` (`target`: another Global of the program, `type` = Ptr(target->type)).
    //  Unknown: initialized and in range (`WF` for a pointer) but not known: external linkage in C++, the transitive escape, a C tentative definition
    //           (question 84).  `init` may still hold the initializer as written (a bool, integer or enumeration): the prologue checks its obligations
    //           like a Value global's, with no guard against a value; `program_start` states the range only.
    using Init = GlobalInit;
    int id {0};         // the index in Program::globals(); unique in the Program; part of the Viper name
    std::string c_name; // the C/C++ name as messages say it (`g`, `count`, `ns::g`)
    // Bool, Int (an enumeration included), an accepted pointer or (decision 0044 item 8) a struct of the Program
    Type type;
    SrcLoc loc; // the defining declaration (the position of its `program_start` row)
    // a `static` local: the function whose body names it (decision 0041 item 12); null for a file/namespace-scope
    // global
    const Function* owner {nullptr};
    Init init_kind {Init::Value};
    const Expr* init {nullptr};     // Init::Value: the initializer as written, or null
    std::string value {"0"};        // Init::Value: EDG's value
    const Global* target {nullptr}; // Init::Addr
    std::string c_type_name;        // the C/C++ spelling of the type, for messages (empty: use type.str())
    // A STRUCT global (decision 0044 item 8): one entry per leaf of the struct in struct_leaves order (depth first), each with its own initial value; the
    // scalar fields above (`init_kind`, `init`, `value`, `target`) are then unused (check_program: `init` and `target` null).  Empty for every other global.
    std::vector<GlobalLeaf> leaves;
    // `[[cerastes::owning]]` and `[[cerastes::ref_to_uninit]]` on this pointer global or `static` local (decision 0054 item 5; the encoder sets them, on a
    // mutable pointer-to-object variable that is not `void *`, a pointer to a function or to a `const`/`volatile` type, whose every declaration carries
    // the attribute and whose initializer is absent or null).  `owning`: the variable owns the heap block it points to, so every `owns(g)` of it in a
    // contract is followed by the block site `g == NULL || owns_block(g)` (the deep atom, which the encoder adds), `main` holds it from `program_start`
    // with no block, and `main`'s leak row exempts it.  `ref_to_uninit`: the plain mode of the deep atom becomes the marked mode (flags unknown); under
    // `--strict-init` the encoder checks its stores.
    bool owning {false};
    bool ref_to_uninit {false};

    bool is_static_local() const
    {
        return owner != nullptr;
    }
    // The Viper name of the block constant: `GB_<name>__<id>` for a global, `GB_<function>__<name>__<id>` for a `static` local, with every character
    // that is not a letter, digit or underscore of `c_name` and the function's name written `_` (the id keeps the names distinct).  Always a valid
    // Viper identifier and unique in the Program.
    std::string viper_name() const;
};

// P4222's initialization markers on a pointer parameter (decision 0042 item 4).  `RefToUninit`: the pointee need not have a value on entry
// (`[[cerastes::ref_to_uninit]]`); `MustInit`: the same, and it has one on return (`[[cerastes::must_init]]`).  Either needs an `owns(*p)` atom
// in the `requires`, and MustInit one in the `ensures` as well (check_program repeats the rule as a backstop; the encoder reports it first).
enum class InitMarker
{
    None,
    RefToUninit,
    MustInit
};

// Which allocator a heap view belongs to (decision 0050 item 3, M4.5d2v): the platform's (`malloc`, `calloc`, `free`: the standard names) or vlibc's own
// (`vlibc_malloc` and the rest: a `vlibc_` name).  Set by the encoder from the routine's name class, on the typed views of allocations and deallocations only.
enum class Allocator
{
    Platform,
    Vlibc
};

struct Param
{
    std::string name; // may be empty (unnamed parameter)
    // Bool, Int, a pointer to one of them (decision 0037 item 2) or, by value, a struct of the Program (decision 0044
    // item 3)
    Type type;
    SrcLoc loc;
    std::string c_type_name; // the C/C++ spelling of the type, for messages (empty: use type.str())
    // `T *restrict p` (decision 0037 item 2): accepted only when the `requires` holds `owns(*p)` as a top-level conjunct or as `p == NULL ||
    // owns(*p)` and the `ensures` returns the same atom; check_program repeats the rule as a backstop (the encoder reports it first).
    bool is_restrict {false};
    // `[[cerastes::ref_to_uninit]]` / `[[cerastes::must_init]]` on a pointer parameter (decision 0042 item 4).  Set by the encoder before the
    // contract is read, so a written read of the pointee is rejected there.
    InitMarker init {InitMarker::None};
};

struct Function
{
    int id {0};         // unique within the Program
    std::string c_name; // for messages and generated identifiers
    SrcLoc loc;         // the declaration (what a precondition note points at when a clause has no loc)
    SrcLoc end_loc;     // the closing brace of the body (missing-return, void postconditions)
    // Void, Bool, Int, a pointer to Bool/Int (decision 0037 item 3: `ensures WF(result)` from lowering) or a struct by
    // value (0044 item 4)
    Type ret {Type::void_type()};
    std::vector<Param> params;
    std::vector<Clause> requires_; // the user's `requires`, in order; mention Formal parameters only
    std::vector<Clause> ensures_;  // the user's `ensures`, in order; Formal, Result and Old
    // The termination measure (`decreases`, decision 0013): Int expressions over the formals (the entry values), compared
    // lexicographically.  Empty for a function that is not part of a recursion cycle (then lowered as an empty `decreases`);
    // ignored for a function without a body (a trusted contract is assumed to terminate, 0013 item 5).
    std::vector<Clause> decreases_;
    const Stmt* body {nullptr}; // null: a function used only through its contract (a trusted contract)
    // For a function without a body: why its contract is trusted ("declared here, defined in another unit" or
    // "trusted, never verified" for a function of a system file).  Required then; emitted as a comment tag.
    std::string trusted_reason;
    // Declared not to return (`[[noreturn]]`, `_Noreturn`; decision 0015 item 4): with a body, every reachable `return` and the
    // closing brace are `noreturn` errors and the method ensures `false`; without one it is a trusted contract that ensures `false`
    // (`trusted_reason` required).  It has no user `ensures`.
    bool noreturn {false};
    // Decision 0047 items 2 and 5: P4222's `ref_to_uninit` on the RESULT (a pointer): the block the result owns (the `ensures` holds an `OwnsBlock` atom on
    // `Result`) has no value on return (its flags are unknown, `uninitialized` on a read before a write).  `None` (every function until M4.5d2a) or
    // `RefToUninit`; `MustInit` is refused (the encoder never sets it).  check_program: a `RefToUninit` needs a pointer result and an `ensures` `OwnsBlock`
    // on the `Result`.
    InitMarker result_init {InitMarker::None};
    // Decision 0047 item 2 (D): the function ends the allocation its one pointer parameter starts (the typed view of `free`): the `requires` holds an
    // `OwnsBlock` atom on parameter 0 (guarded `p == NULL ||`) that the `ensures` does not return, and the lowering adds the free checks (`invalid-free`).
    // check_program: a void function of exactly one pointer parameter, a `requires` `OwnsBlock` on it, no `ensures` `OwnsBlock` on it.
    bool deallocates {false};
    // Decision 0050 item 3: the allocator of a heap view (an allocation, or a deallocation with `deallocates`).  `Platform` for every other function.
    // check_program accepts `Vlibc` only on a bodiless, trusted allocation or deallocation view.
    Allocator allocator {Allocator::Platform};

    bool verified() const
    {
        return body != nullptr;
    }

    // The local copy of parameter `k` that the body reads and assigns (created by add_param).
    const Var* param_var(int k) const
    {
        return locals_[param_vars_[static_cast<size_t>(k)]].get();
    }

    // The `const` locals a case value of the body names (C++ only; decision 0017 item 6), checked like the program's named
    // constants, after them, in this function's method and in its `__wf` method.  Definition order = call order.
    NamedConst* add_local_const(std::string c_name, Type type, SrcLoc loc, const Expr* init, std::string value);
    // A block-scope static assertion of the body (decision 0024 item 2): `condition` is a closed Bool expression (as written),
    // `loc` the assertion.  It joins local_consts(), in call order with the local constants, and is checked the same way.
    NamedConst* add_local_static_assertion(SrcLoc loc, const Expr* condition);
    const std::deque<NamedConst>& local_consts() const
    {
        return local_consts_;
    }

    // Declare a parameter; returns its index.  Also creates its local copy (a `Var` named like the parameter).
    int add_param(std::string name, Type type, SrcLoc loc);
    // Declare a local.  The pointer stays valid for the life of the Function.
    Var* new_local(std::string name, Type type, SrcLoc loc);
    const std::vector<std::unique_ptr<Var>>& locals() const
    {
        return locals_;
    }
    // Mark `v` (a local of this function) as address-taken: a cell (decision 0032 item 4).  A foreign pointer is ignored; check_program
    // reports the unmarked `&v` of a foreign variable.
    void mark_address_taken(const Var* v);

private:
    std::vector<std::unique_ptr<Var>> locals_; // all Vars (parameter copies and locals)
    std::vector<size_t> param_vars_;           // index into locals_ of each parameter's copy
    std::deque<NamedConst> local_consts_;
};

class Program
{
public:
    Program() = default;
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;

    // A fresh jump id (decision 0014): unique within the Program, so also within every Function.
    int fresh_jump_id()
    {
        return next_jump_id_++;
    }

    // Integer types are interned by (bits, is_signed); the first name given wins.
    const IntType* int_type(const std::string& name, int bits, bool is_signed);
    // The interned type of that shape; a type nobody named is created with a synthetic name ("int32", "uint64").
    const IntType* int_type_of(int bits, bool is_signed);
    // A new enumeration type (decision 0018 item 3), never interned: each call makes a distinct type.  `bits` and
    // `is_signed` are the underlying (C++) or compatible (C) type's; [lo, hi] is the value range, within that type.
    const IntType* enum_type(const std::string& name, int bits, bool is_signed, bool is_scoped, const std::string& lo,
                             const std::string& hi);

    // The pointer type to `pointee`, interned (two pointer types with equal pointees share one `Type` object for the pointee).  The pointee
    // is copied into the Program, so a temporary is fine.
    Type pointer_to(const Type& pointee);

    // A new C structure type (decision 0043), never interned by shape: each call makes a distinct type.  `tag` is the C tag (may be empty),
    // `c_name` the spelling for messages ("struct S").  The members are added afterwards with add_member, so that a member may be a pointer to
    // the struct itself (`pointer_to(Type::structure(s))`).  `name` is `struct_<id>_<tag>`, sanitized (see StructType::name).
    StructType* add_struct(const std::string& tag, const std::string& c_name, SrcLoc loc);
    // Append member `name` of `type` (the next index, declaration order); returns its index.
    int add_member(StructType* s, std::string name, Type type, SrcLoc loc);
    const std::deque<StructType>& structs() const
    {
        return structs_;
    }

    // The width of C's `int` on this target (EDG's `sizeof(int) * CHAR_BIT`); integer promotion is relative to
    // it.  Set it once, before building; the default is 32.
    void set_c_int_bits(int bits)
    {
        c_int_bits_ = bits;
    }
    int c_int_bits() const
    {
        return c_int_bits_;
    }

    // New functions are appended in definition order; the order is the order of the emitted methods.  The
    // callee of a Call must be a function of this Program, added before the call is built.
    Function* add_function(std::string c_name, Type ret, SrcLoc loc, SrcLoc end_loc);

    const std::deque<Function>& functions() const
    {
        return functions_;
    }
    std::deque<Function>& functions()
    {
        return functions_;
    }

    // The unit's named constants (decision 0017), in definition order; every method with a body checks them in its prologue.
    NamedConst* add_named_const(std::string c_name, Type type, SrcLoc loc, const Expr* init, std::string value);
    // A file-scope or namespace-scope static assertion (decision 0024 item 2): `condition` is a closed Bool expression (as
    // written), `loc` the assertion.  It joins named_consts(), so the prologue checks it in call order with the named constants
    // and enumerators.
    NamedConst* add_static_assertion(SrcLoc loc, const Expr* condition);
    const std::deque<NamedConst>& named_consts() const
    {
        return named_consts_;
    }

    // The mutable globals and `static` locals (decision 0041), in declaration order; `id` is the index.  `owner` (a `static` local's function) may be
    // null; the pointer stays valid for the life of the Program.  Set the initial value (Global::init_kind and the fields it names) on the result.
    Global* add_global(std::string c_name, Type type, SrcLoc loc, const Function* owner = nullptr);
    const std::deque<Global>& globals() const
    {
        return globals_;
    }
    // The global-namespace `main` (decision 0041 item 4), or null: the function whose method calls `program_start()` first.  Set by the encoder; it
    // must be a function of this Program (check_program).
    void set_entry(const Function* f)
    {
        entry_ = f;
    }
    const Function* entry() const
    {
        return entry_;
    }

    // Arena (used by the Builder).
    Expr* new_expr()
    {
        return &exprs_.emplace_back();
    }
    Stmt* new_stmt()
    {
        return &stmts_.emplace_back();
    }

private:
    int c_int_bits_ {32};
    int next_jump_id_ {1};
    std::deque<std::unique_ptr<IntType>> int_types_;
    std::deque<Type> pointees_;                       // the interned pointee types of pointer_to
    std::deque<std::unique_ptr<IntType>> enum_types_; // apart from int_types_, so int_type() never returns one
    std::deque<Function> functions_;
    std::deque<NamedConst> named_consts_;
    std::deque<Global> globals_;
    std::deque<StructType> structs_;
    const Function* entry_ {nullptr};
    std::deque<Expr> exprs_;
    std::deque<Stmt> stmts_;
};

// ---- sequencing (decision 0020) -----------------------------------------------------------------------------------

// A modification of an object that is unsequenced with another access to it in one full expression (C23 6.5.1p3, C++ before C++17;
// cerastes uses C's rules in both modes).  `at` is the AssignE or StoreE whose write conflicts (report there); `variable` is the
// object's source name: a local (a cell by name counts as its variable, and `*&x` is `x`), a global or `static` local (decision 0041 item 11: one object
// per Global, named by its `c_name`, and `*&g` is `g`) or, from decision 0039 item 3, `*v` for a
// pointer variable `v` as written (one object per `v`, written by a Store or StoreE through `v`, read by a Deref of `v`, whether or not
// the full expression also writes `v`; any other address is the path-precise check of lowering instead).  The rule
// (decision 0020 item 4), with W the objects an operand writes (AssignE and StoreE targets) and R the objects it reads:
//  * the operands of every Binary other than LAnd/LOr, and the arguments of a Call among themselves, are unsequenced: a conflict
//    if W(p) meets W(q) or R(q) (or the other way round);
//  * a Cond's condition is sequenced before its arms, a Comma's left before its right, LAnd/LOr's left before their right (no
//    check); the two arms of a Cond are not checked against each other, but the Cond as a whole counts as W/R = their union;
//  * a plain store (an AssignE, a StoreE, an Assign, a Store or a Decl) conflicts with a write of its own target inside the stored expression unless a
//    sequence point separates them: the write lies inside the left operand of a Comma, the first operand of an LAnd/LOr, the
//    condition of a Cond or an argument of a Call.  (A compound assignment needs no extra rule: its expansion reads the target as
//    an operand of a Binary.)  A read of the target is no conflict; neither is the read of an AssignE's own target in its own `a`.
//  * a Store (decision 0032 item 9) and a StoreE are two unsequenced operands, their address and their value, like the operands of a Binary:
//    the value writing a local that the address reads (`*p = (p = q, 1)`) is a conflict.
//  * a compound store's value reads a Deref that shares the store's address node (decision 0039 item 5): the address counts once, so
//    `*(q = p) += 1` is no conflict.
struct SequenceProblem
{
    const Expr* at {nullptr}; // the AssignE or StoreE whose write conflicts
    std::string variable;     // the local's source name
};
std::vector<SequenceProblem> sequence_problems(const Function& f);

// The locals a statement or an expression assigns, and the locals a statement declares (the targets of Assign statements and AssignE nodes;
// a Decl's `expr` counts for its AssignEs).  Every expression counts, a nested loop's condition and a `do` condition included (decision 0020).
// Every Stmt kind that carries an expression is scanned, a Store's `addr` and `expr` included (`*(p = q) = 1`); the loop rule of decision 0038 item 3
// and `--strict-init`'s "passed unchanged" (decision 0042 item 8) use this one function.
void scan_assigned_expr(const Expr* e, std::set<int>& assigned);
void scan_assigned(const Stmt& s, std::set<int>& assigned, std::set<int>& declared);
// The indices of `f`'s pointer parameters that the body assigns (their formal's place is not automatic in a loop, decision 0038 item 3;
// `--strict-init`'s "passed on unchanged", decision 0042 item 7).  Empty for a function without a body.
std::set<int> assigned_params(const Function& f);

// ---- permissions in contracts (decision 0037 items 5-7) ---------------------------------------------------------------

// One `Atom` of a contract with the conditions under which it holds.  `guards` are the conditions of the enclosing `||` and `?:` (in
// evaluation order, outermost first): the atom is asserted where every `cond == holds`: for `a || atom` the guard is (`a`, false) (lowered
// `!a ==> atom`: Viper refuses `acc` under `||`), for `c ? atom : x` it is (`c`, true), for `c ? x : atom` it is (`c`, false).  Each `cond`
// holds no Atom, no call and no side effect; from decision 0045 item 9 (owner question 109) it may read memory (`*pp == NULL`, `p->q == NULL`: Deref
// nodes over the place grammar), which the lowering frames per path like any contract read.  The atoms of an enclosing `&&` add no guard.
struct AtomGuard
{
    const Expr* cond {nullptr};
    bool holds {true};
};
// The place an atom names (decision 0043 item 9): a root plus a member path.  The root is as atom_place_key (a parameter's index, `kGlobalSiteBase
// + id` for a global, `kNoPlaceKey` for anything else, an invariant's `Local` included); the path is the member indices from the root's pointee
// (empty for `owns(*p)`; `owns(p->t)` is {t}, `owns(p->t.x)` is {t, x}).  Two atoms name one place iff their roots are equal and valid and their
// paths are equal; they OVERLAP (share cells) by slots, which the lowering decides from the struct types (struct_slots_of), never by this key.
struct AtomSite
{
    const Expr* atom {nullptr}; // the Atom node
    bool in_ensures {false};
    int clause {0}; // the index in Function::requires_ or ensures_
    std::vector<AtomGuard> guards;
    // the member path of the atom's place (decision 0043; empty for `*p`, a global, a Local): atom_place(*atom).path
    std::vector<int> path;
    // Decision 0045 item 2 (places reached through loads): `via` is the steps from the root to the BASE pointer (atom_place(*atom).via; empty for every place of
    // decisions 0037-0044), `base` the base pointer expression itself, `member_root(atom->a)`: the Formal, the GlobalAddr or the Result for a place without
    // steps, otherwise a Deref or a Field node (`**pp`: base `*pp`, via {load}; `p->q->x`: base `p->q` as a loaded pointer, via {q, load}, path {x}).  Lowering
    // lowers `base`, then applies `path`.
    std::vector<int> via;
    const Expr* base {nullptr};
};
// The atoms of `f`'s contract in allowed positions, the `requires` first and then the `ensures`, each clause in evaluation order (left
// operand first; the arms of `?:` then-arm first).  An Atom in a position the record forbids (under `!`, inside `==`, as an argument, on
// the left of `||`, under `Old`, ...) is not listed: check_program reports it.  The k-th `requires` site's `amount` is k.
std::vector<AtomSite> contract_atoms(const Function& f);
// The `OwnsBlock` atoms of `f`'s contract (decision 0047 item 4), the `requires` first and then the `ensures`, in evaluation order, with the same guards
// and positions as contract_atoms (an OwnsBlock in a position the record forbids is not listed: check_program reports it).  contract_atoms never lists
// them, so every loop over its sites, the numbering of the ghost amounts and every `nreq` stay as they were.  `AtomSite::clause`/`in_ensures` say where
// the atom is written, `atom->a` is the pointer it names (a `Formal` or the `Result` today), `base` is `atom->a`, `path` is empty and `via` the steps of `atom->a` from its root
// (decision 0054 item 2; empty today, since only a `Formal` or the `Result` is accepted): so `atom_place(*atom)` of a block site is the place of the pointer it names.
std::vector<AtomSite> contract_block_atoms(const Function& f);
// The atoms of the `invariant` clauses of loop `s` (a `While` statement) in the positions of decision 0037 item 5 (decision 0040 item 5): in
// clause order, each clause in evaluation order, with the same guards (their `cond`s hold no atom, call or side effect, and may read memory
// like a contract's, decision 0045 item 9).  `AtomSite::clause` is the index in `s.invariants`, `in_ensures` is false, `atom->amount` is -1.  An atom of an invariant elsewhere
// (under `!`, as an argument, ...) is not listed: check_program reports it.  Empty for a loop of form DoOnce (check_program refuses its atoms).  From decision 0054 item 4
// the `OwnsBlock` atoms of the invariant are listed too (a consumer of the permission atoms skips them: loop_block_atoms is the list of those alone).
std::vector<AtomSite> loop_atoms(const Stmt& s);
// The `OwnsBlock` atoms of the `invariant` clauses of loop `s` (decision 0054 item 4), in the same positions and with the same guards as loop_atoms' sites; `atom->a` is the
// pointer the invariant reads (a `Local` pointer variable, or a place reached from one or from a global through loads and members), `atom->amount` is -1.
std::vector<AtomSite> loop_block_atoms(const Stmt& s);
// The shape of a site's guards for pointer parameter `param`: none (the atom is a top-level conjunct, or an arm of `&&`), exactly `param == NULL` being false
// (`param == NULL || atom`, with C's `bool_to_int`/`to_bool` wrapping of the comparison accepted), or anything else.  restrict_backed and the encoder's
// merge of `[[cerastes::owning]]` with an explicit atom (decision 0040 item 3) read it.
enum class GuardShape
{
    None,
    NullCheck,
    Other
};
GuardShape atom_guard_shape(const AtomSite& s, int param);
// The number of ghost amounts of `f` (the atoms of its `requires`).
int requires_atom_count(const Function& f);
// Whether `f`'s contract backs `restrict` on parameter `param` (decision 0037 item 2): an `Owns` atom of that parameter in the `requires`, the `Owns`
// atom with the same `amount` in the `ensures`, and, for EVERY OTHER pointer parameter of the same pointee type, an `Owns` or `Reads` atom of its own
// in the `requires` (permissions follow cells, not provenance: without it a second pointer to the same cell could be written through).  Each atom is
// unguarded or guarded by exactly `q == NULL` being false (`q == NULL || owns(*q)`).  The atoms must already be numbered.  When it is false,
// `*offender` (if given) is the other parameter that has no atom, or -1 when `param`'s own `owns` is the missing part.
bool restrict_backed(const Function& f, int param, int* offender = nullptr);
// Why `restrict` on pointer parameter `param` is refused whatever its contract owns (decision 0043 item 10, audit B2): empty when nothing is in the way.
// The pointee of `param` is a struct (M8), or another pointer parameter's object or a global that an atom of the contract names can hold a pointer to the same
// type (`int **`, a struct with an `int *` leaf or struct-pointer leaves that reach one, an `int *` global): a pointer loaded from there is not based on
// `param`, so a write through it could alias `*param`.  A by-value struct parameter whose leaves can hold such a pointer (an `int *` leaf, or a `struct U *` leaf whose `U`
// has an `int` leaf, at any depth) is one of those places too (decision 0044 item 11).  restrict_backed is false when this is not empty (the encoder reports it
// first).  The message names the offender in source terms.  Decision 0045 item 10: an atom whose place is reached through loads or pointer leaves (`*gp`, `**pp`,
// `*s.q`) is judged by its innermost root (place_root): a global by its type, the result by the function's result type, a Formal by the loop over the parameters;
// any other root of such a place fails closed.  restrict_backed never counts such an atom (it requires the atom's operand to BE the Formal).
std::string restrict_hazard(const Function& f, int param);
// Whether the expression reads the heap: it contains a `Deref` or an `Atom` (an `Old` of one included).
bool reads_heap(const Expr& e);
// The place a permission atom names (decision 0041 item 8: the sites of 0037 are keyed by place, not by the parameter's index): the parameter's index
// (>= 0) for an atom on a `Formal`, `kGlobalSiteBase + global->id` for an atom on a `GlobalAddr`, `kNoPlaceKey` for anything else (an invariant's
// `Local`).  Two atoms name one place iff their keys are equal and not `kNoPlaceKey`; `kResultSiteKey` is reserved for lowering's result site.
constexpr int kGlobalSiteBase {1 << 20U};
// The ranges of site keys are disjoint (decision 0045 audit S7): a Formal's index in [0, kGlobalSiteBase), a global's `kGlobalSiteBase + id` in
// [kGlobalSiteBase, kLocalSiteBase), an invariant's pointer variable or cell `kLocalSiteBase + var->id` in [kLocalSiteBase, kDerivedKeyBase), and the
// lowering's derived keys (one per distinct root and steps, decision 0045 item 2) from kDerivedKeyBase on.  `kResultSiteKey` and `kNoPlaceKey` are negative.
// Test a key with these helpers, never with a one-sided `>=`.
constexpr int kLocalSiteBase {1 << 24U};
constexpr int kDerivedKeyBase {1 << 28U};
inline bool is_global_site_key(int k)
{
    return k >= kGlobalSiteBase && k < kLocalSiteBase;
}
inline bool is_local_site_key(int k)
{
    return k >= kLocalSiteBase && k < kDerivedKeyBase;
}
inline bool is_derived_site_key(int k)
{
    return k >= kDerivedKeyBase;
}
constexpr int kResultSiteKey {-1};
constexpr int kNoPlaceKey {-2};
// The root key of an atom whose place has an EMPTY member path and no steps (`via`); `kNoPlaceKey` for any other place (use `atom_place`).  So a caller that
// predates structs never treats `owns(p->x)` as `owns(*p)`, and one that predates decision 0045 never treats `owns(**pp)` as `owns(*pp)`.
int atom_place_key(const Expr& atom);
// The steps of a place (decision 0045 item 2).  A place is a root and the steps from it to the BASE pointer (the pointer whose pointee the member tail starts
// from), then the member tail `path`.  A step is a member index k >= 0 (a MemberAddr: the address of member k of the struct the pointer so far names), kLoadStep
// (a Deref: the pointer so far is a pointer to a pointer and the step reads the pointer it holds: a LOAD) or `field_step(k)` (a Field of a by-value struct
// parameter's value or of the result's: the pointer leaf `s.q`, `r.q`).  Examples: `**pp` = (pp, {load}, {}); `p->q->x` = (p, {q, load}, {x}); `*s.q` =
// (s, {field q}, {}); `*gp` = (global gp, {load}, {}); `(*ps)->data` = (ps, {load}, {data}); `owns(*p)` and `owns(p->t.x)` have no steps.
constexpr int kLoadStep {-1};
constexpr int field_step(int member)
{
    return -2 - member;
}
constexpr bool is_field_step(int step)
{
    return step <= -2;
}
constexpr int field_of_step(int step)
{
    return -2 - step;
}
// Whether the steps contain a load (the place is reached through a heap read; a `Field` step alone reads no heap).
bool steps_have_load(const std::vector<int>& via);
// The root keys of the places an invariant reaches (a pointer variable or a cell, by the variable's id): `kLocalSiteBase + var->id`.  A plain `Local` pointer
// variable with no steps and no member path (`owns(*q)`, every invariant atom of decisions 0040-0044) keeps `kNoPlaceKey`: it is compared by variable.
// The full place of an atom (decisions 0043 item 9, 0045 item 2): the root key of atom_place_key's rule, however long the steps and the path are, the steps
// and the member path.  The root is a Formal's index, `kGlobalSiteBase + global->id`, `kResultSiteKey` for the Result (an `ensures`), or in an invariant
// `kLocalSiteBase + var->id`; the atom's `a` is the root under any number of MemberAddr, Deref and Field nodes (an `Old` of a Formal under a Field counts as
// the Formal when it stands directly under a Field: in an `ensures` a Formal is its entry value either way); `valid()` is false for anything else.  Two atoms name one place iff roots, steps and
// paths are equal; the key is SYNTACTIC (it decides duplicates and `requires`/`ensures` partners only): overlap, separation and framing are decided on
// `Ref`s in the evaluated state, never by this key.
struct PlaceKey
{
    int root {kNoPlaceKey};
    // the steps from the root to the base pointer (see above); empty for every place without a load or a pointer leaf
    std::vector<int> via;
    std::vector<int> path; // the member indices from the base pointer's pointee; >= 0 only
    bool valid() const
    {
        return root != kNoPlaceKey;
    }
    bool operator==(const PlaceKey& o) const
    {
        return root == o.root && via == o.via && path == o.path;
    }
    bool operator!=(const PlaceKey& o) const
    {
        return !(*this == o);
    }
};
PlaceKey atom_place(const Expr& atom);
// The root of a place expression (decision 0045 item 2): `e` under any number of MemberAddr, Deref and Field nodes (and an `Old` of a Formal directly under a Field), the
// first node that is none of them (a Formal, Result, GlobalAddr, Local, AddrOf, or anything else, which a caller refuses).  `*steps` (if given) receives the
// steps from the root outward, members included (so the steps of `member_root`'s path come last).  A read of a cell by name, a `Local` whose variable is
// `address_taken` (the root, kept as the Local node), counts as a load from the cell's address: its steps start with kLoadStep, as those of `Deref(AddrOf v)`
// do (root AddrOf), so the two spellings of one read have one key.
const Expr* place_root(const Expr* e, std::vector<int>* steps = nullptr);
// Strips MemberAddr nodes from a place expression: the root pointer expression (a Formal, Result, GlobalAddr, Local, AddrOf, ...) and, in `*path`
// (if given), the member indices from the root outward.
const Expr* member_root(const Expr* e, std::vector<int>* path = nullptr);
// The spelling of the place a Store/StoreE/Deref through `addr` writes or reads, for messages: the global's name for a `GlobalAddr` (`g`), "*" + `c_text`
// otherwise (empty when the operand has no spelling), the member place (`p->x`, `s.t.x`; decision 0043) for a MemberAddr.
std::string place_text(const Expr& addr);

// ---- user gotos (decision 0015 item 2) -------------------------------------------------------------------------

// Why a Goto is not accepted.  A Goto with label id L is accepted iff the Label with that id is a direct statement of a
// Block that is an ancestor of the Goto, at an index after the one holding the Goto, with no Decl WITH an initializer of that
// Block between the two (a Decl without one may be jumped over, decision 0019 item 6).  Otherwise: `no_label` (no such Label in the function), `skips_declaration` (`variable` names the variable),
// `backward` (the Label comes before the Goto in a pre-order walk) or `enters` (anything else).
struct GotoProblem
{
    const Stmt* go {nullptr};
    enum class Kind
    {
        backward,
        enters,
        skips_declaration,
        no_label
    } kind {Kind::no_label};
    std::string label;    // the user's name of the label (empty for no_label)
    std::string variable; // skips_declaration: the variable
};
// The problems of the Gotos of `f`'s body (none for a function without a body).  check_program reports each one as an
// internal error: the encoder rejects them first, with a diagnostic at the C/C++ line.
std::vector<GotoProblem> goto_problems(const Function& f);

// ---- the standards' conversion rules (the one place they are implemented) -----------------------------------------

// Integer promotion of an integer type (C23 6.3.1.1, C++ [conv.prom]): a type with fewer bits than `int`
// becomes `int` (all its values fit); every other type is unchanged.  (`bool` is not an IntType: the encoder
// converts it with `bool_to_int` first.)
const IntType* promoted(Program& p, const IntType* t);

// The usual arithmetic conversions of two ALREADY PROMOTED integer types (C23 6.3.1.8, C++ [expr.arith.conv]),
// by (bits, signedness): equal signedness gives the wider type; otherwise the unsigned operand wins when it
// is at least as wide as the signed one, else the signed one (it can represent every value of the narrower
// unsigned type).  Types of equal width and different signedness give the unsigned type; this agrees with the
// standards' rank rule (`long long` + `unsigned long` is `unsigned long long`: same shape).
const IntType* usual_arithmetic(Program& p, const IntType* a, const IntType* b);

// ---- the builder -------------------------------------------------------------------------------------------------

// Builds nodes in a Program.  It computes each node's `type` from its operands without checking them (an
// ill-typed tree is built as is, with the type of its first operand, and rejected by check_program).
class Builder
{
public:
    explicit Builder(Program& p) : p_(p) {}
    Program& program()
    {
        return p_;
    }

    // Expressions.
    const Expr* int_const(Type int_type, std::string decimal, SrcLoc loc);
    const Expr* bool_const(bool value, SrcLoc loc);
    const Expr* local(const Var* v, SrcLoc loc);                        // a local or a parameter's copy
    const Expr* formal(const Function& f, int param_index, SrcLoc loc); // the entry value (contracts only)
    const Expr* result(const Function& f, SrcLoc loc);                  // `ensures` only
    const Expr* old(const Expr* e, SrcLoc loc);                         // `ensures` only
    const Expr* unary(UnaryOp op, const Expr* a, SrcLoc loc);
    const Expr* binary(BinaryOp op, const Expr* a, const Expr* b, SrcLoc loc);
    const Expr* cond(const Expr* c, const Expr* a, const Expr* b, SrcLoc loc);
    const Expr* convert(Type to_int_type, const Expr* a, bool explicit_cast, SrcLoc loc);
    const Expr* to_bool(const Expr* a, SrcLoc loc);                       // a != 0
    const Expr* bool_to_int(Type to_int_type, const Expr* a, SrcLoc loc); // a ? 1 : 0
    const Expr* call(const Function* callee, std::vector<const Expr*> args, SrcLoc loc);
    // `note` says what the obligation's own position is ("in the default argument", "in the macro"); empty: the default.
    const Expr* report_at(const Expr* a, SrcLoc loc, std::string note = "");
    // Record how the program spells the node's type ("signed char"); returns `e`.  For messages only.
    const Expr* spelled(const Expr* e, std::string c_type_name);
    // Record the node's source spelling (`Expr::c_text`: "p", "*p", "&x"); returns `e`.  For messages only.
    const Expr* text(const Expr* e, std::string c_text);
    // An atom of a contract (decision 0037 item 5): `ptr` is a `Formal` of pointer type (c_text "p" when the encoder set it), `amount` the
    // number of the ghost amount (see Expr::Kind::Atom).  Type Bool.  c_text: "owns(*p)", "reads(*p)" or "live(p)" when `ptr` has a spelling.
    const Expr* atom(AtomKind kind, const Expr* ptr, int amount, SrcLoc loc);
    // An atom on a global (decision 0041): `ptr` is a GlobalAddr; c_text "owns(g)" / "reads(g)".  `implicit` marks the Owns the encoder adds for a `static`
    // local (item 12).  Same as atom() otherwise (the atom's `a` is the GlobalAddr).
    const Expr* global_atom(AtomKind kind, const Expr* global_addr, int amount, SrcLoc loc, bool implicit = false);

    // Pointers (decision 0032).  `ptr_type` is a Ptr type from Program::pointer_to.
    const Expr* null_const(Type ptr_type, SrcLoc loc);
    // `&v`: type Ptr(v->type); c_text "&" + v->name.  `v` must be a cell (Function::mark_address_taken) for check_program to accept it.
    const Expr* addr_of(const Var* v, SrcLoc loc);
    // `&g` of a global or `static` local (decision 0041): type Ptr(g->type); c_text "&" + g->c_name.  `deref` of it has c_text `g`.
    const Expr* global_addr(const Global* g, SrcLoc loc);
    // `*addr` as a read: type = addr's pointee (an ill-typed `addr` is built with its own type and rejected by check_program); c_text
    // "*" + addr->c_text when the operand has one.
    const Expr* deref(const Expr* addr, SrcLoc loc);
    // `&a->m` (decision 0043): `a` a pointer to a struct, `index` the member's index; type Ptr(the member's type) (an ill-typed `a` or an index out of range
    // is built with `a`'s type and rejected by check_program).  c_text "&" + the place spelling (see Expr::Kind::MemberAddr); `deref` of it is `p->m`.
    const Expr* member_addr(const Expr* a, int index, SrcLoc loc);
    // `a.m` (decision 0044 item 2): member `index` of the struct value `a`; type = the member's type (an ill-typed `a` or an index out of range is built with
    // `a`'s type and rejected by check_program).  c_text: `a`'s followed by `.m` when `a` has one.
    const Expr* field(const Expr* a, int index, SrcLoc loc);
    // A struct value of `struct_type` (a Struct type) built from one expression per leaf, in struct_leaves order (see Expr::Kind::Aggregate).
    const Expr* aggregate(Type struct_type, std::vector<const Expr*> args, SrcLoc loc);

    // Statements.
    // `init` null: a local declared without an initializer (decision 0019).
    const Stmt* decl(const Var* v, const Expr* init, SrcLoc loc);
    const Stmt* assign(const Var* v, const Expr* value, SrcLoc loc);
    const Stmt* expr_stmt(const Expr* e, SrcLoc loc);
    const Stmt* if_stmt(const Expr* c, const Stmt* then_body, const Stmt* else_body, SrcLoc loc);
    const Stmt* while_stmt(const Expr* c, std::vector<Clause> invariants, std::vector<Clause> measure, const Stmt* body,
                           SrcLoc loc);
    // A loop in any form (decision 0014).  `measure` is required for While and DoWhile and may be empty for DoOnce; `c` is the
    // condition (for DoOnce it is still encoded, for its obligations); `step` may be null; `jump_id` is from jump_id() (0: no
    // Break or Continue names this loop).
    const Stmt* loop(const Expr* c, std::vector<Clause> invariants, std::vector<Clause> measure, const Stmt* body,
                     const Stmt* step, LoopForm form, int jump_id, SrcLoc loc);
    const Stmt* switch_stmt(const Expr* scrutinee, const Stmt* body, int jump_id, SrcLoc loc);
    // null: `default`; a value needs the folded form below
    const Stmt* case_label(const Expr* value_or_null, SrcLoc loc);
    // `value` is the label as written (obligations checked in the prologue); `folded` is the front end's value (decision 0017 item 5).
    const Stmt* case_label(const Expr* value, std::string folded, SrcLoc loc);
    const Stmt* break_stmt(int target_jump_id, SrcLoc loc);
    const Stmt* continue_stmt(int target_jump_id, SrcLoc loc);
    int jump_id()
    {
        return p_.fresh_jump_id();
    }
    const Stmt* return_stmt(const Expr* value_or_null, SrcLoc loc);
    const Stmt* check(const Expr* e, SrcLoc loc, bool from_assert = false);
    const Stmt* goto_stmt(int label_id, SrcLoc loc);
    const Stmt* label_stmt(int label_id, std::string name, SrcLoc loc);
    const Stmt* block(std::vector<const Stmt*> stmts, SrcLoc loc);
    // A Block with the position of its closing brace, where leaving the scope of its cells is reported (decision 0032 item 8).
    const Stmt* block(std::vector<const Stmt*> stmts, SrcLoc loc, SrcLoc end_loc);
    // `*addr = value` (decision 0032): `value` has the pointee's type.
    const Stmt* store(const Expr* addr, const Expr* value, SrcLoc loc);
    // `*addr op= rhs`, `++*addr` and `(*addr)--` as Stores, with the computation and the conversions of compound_assign (below): the
    // stored value is `(T)((C)(*addr) op (C)rhs)` where `*addr` is a Deref sharing the `addr` node.  The pointee must be an Int (anything
    // else is built ill-typed and rejected by check_program).
    const Stmt* compound_store(const Expr* addr, BinaryOp op, const Expr* rhs, SrcLoc loc);
    const Stmt* inc_dec_store(const Expr* addr, bool increment, SrcLoc loc);

    // Effects used as values (decision 0020).  The statement forms below stay as they are: a top-level assignment is an `Assign`.
    // `value` has `v`'s type (the encoder converts it as for `assign`).  The node's type is `v`'s; its value is `value`.
    const Expr* assign_expr(const Var* v, const Expr* value, SrcLoc loc);
    // `x op= rhs` as a value: the same computation type and conversions as compound_assign; the value is the new value of `x`.
    const Expr* compound_assign_expr(const Var* x, BinaryOp op, const Expr* rhs, SrcLoc loc);
    // `++x` (post false: the new value), `x++` (post true: the old value, yields_old); `--` likewise.  The old value is one shared
    // `Local x` node (`b`), also used inside the stored expression.
    const Expr* inc_dec_expr(const Var* x, bool increment, bool post, SrcLoc loc);
    // `*addr = value` as a value (decision 0039 item 2): a StoreE of type `value`'s type (the pointee's); `value` has the pointee's type.
    const Expr* store_expr(const Expr* addr, const Expr* value, SrcLoc loc);
    // `*addr op= rhs` as a value (the new value), with compound_store's computation: the stored value reads a Deref sharing `addr`.
    const Expr* compound_store_expr(const Expr* addr, BinaryOp op, const Expr* rhs, SrcLoc loc);
    // `++*addr` (post false: the new value), `(*addr)++` (post true: the old value, yields_old with `b` the shared Deref); `--` likewise.
    const Expr* inc_dec_store_expr(const Expr* addr, bool increment, bool post, SrcLoc loc);
    // `a, b`: the value of `b` after `a`.  `a` may be a void call; `b` must not be void.
    const Expr* comma(const Expr* a, const Expr* b, SrcLoc loc);

    // `x op= rhs` and `++x`, `x++`, `--x`, `x--` as statements (0010 item 6).  The IL does not give the
    // computation type (P2), so these compute it with the rules above and expand to
    //     x = (T_x)( (C)x  op  (C)rhs )                      op in Add Sub Mul Div Rem BitAnd BitOr BitXor, C = usual arithmetic
    //                                                          conversions of promoted(T_x) and promoted(type of rhs)
    //     x = (T_x)( (C)x  op  rhs )                         op in Shl Shr, C = promoted(T_x); the count keeps its type
    //     x = (T_x)( (C)x +/- (C)1 )                         C = usual arithmetic conversions of promoted(T_x) and int
    // The conversion back to T_x is a checked conversion (a narrowing to a signed type is an error, 0010 item 5).
    // `x` must be an Int variable and `rhs` an Int expression (the encoder converts a bool with bool_to_int, and
    // rejects ++/-- on bool); anything else is built as an ill-typed Assign that check_program rejects.  Every
    // generated node has position `loc`, the operator's.
    const Stmt* compound_assign(const Var* x, BinaryOp op, const Expr* rhs, SrcLoc loc);
    const Stmt* inc_dec(const Var* x, bool increment, SrcLoc loc);

private:
    // `(T_x)((C)xval op (C)rhs)`: the stored value of a compound assignment (see compound_assign); `xval` is the read of the object, of
    // type `xt`.
    const Expr* compound_value(const Type& xt, const Expr* xval, BinaryOp op, const Expr* rhs, SrcLoc loc);
    Program& p_;
};

} // namespace cerastes::vir
