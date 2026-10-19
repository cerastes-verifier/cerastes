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
#include <string>
#include <utility>
#include <vector>

namespace cerastes::vir {

// ---- positions -----------------------------------------------------------------------------------------

// A C/C++ source position.  `line == 0` means "unknown" (check.h rejects an unknown position on any node
// that can produce an obligation).
struct SrcLoc {
  std::string file;
  int line = 0;
  int column = 0;
  bool known() const { return line > 0; }
};

// ---- types ---------------------------------------------------------------------------------------------

// An integer type of the target, filled from EDG's configuration at run time (0010 item 2).  Two IntTypes
// are the same type for VIR iff they have the same `bits` and `is_signed` (`name` only labels messages):
// `unsigned int` and `char32_t` are one type, `char` and `signed char` are one type.  `bits` is at most 128.
struct IntType {
  std::string name;  // "int", "unsigned long", "char", ... (for messages and Viper comments)
  int bits = 0;
  bool is_signed = false;
};

struct Type {
  enum class Kind { Void, Bool, Int };
  Kind kind = Kind::Void;
  const IntType* int_type = nullptr;  // Kind::Int only; owned by the Program

  static Type void_type() { return Type{Kind::Void, nullptr}; }
  static Type bool_type() { return Type{Kind::Bool, nullptr}; }
  static Type integer(const IntType* t) { return Type{Kind::Int, t}; }

  bool is_void() const { return kind == Kind::Void; }
  bool is_bool() const { return kind == Kind::Bool; }
  bool is_int() const { return kind == Kind::Int; }
  // Same VIR type (Int types: same bits and signedness).
  bool operator==(const Type& o) const {
    if (kind != o.kind) return false;
    if (kind != Kind::Int) return true;
    return int_type->bits == o.int_type->bits && int_type->is_signed == o.int_type->is_signed;
  }
  bool operator!=(const Type& o) const { return !(*this == o); }
  std::string str() const;  // "bool", "void", or the int type's name
};

// ---- expressions -----------------------------------------------------------------------------------------

struct Function;
struct Var;

enum class UnaryOp { Neg, Plus, Not };
enum class BinaryOp {
  Add, Sub, Mul, Div, Rem,  // Int x Int (same IntType) -> that IntType
  Shl, Shr,                 // Int x Int (count: any IntType) -> the left operand's IntType
  Lt, Le, Gt, Ge,           // Int x Int (same IntType) -> Bool
  Eq, Ne,                   // Int x Int (same IntType) or Bool x Bool -> Bool
  LAnd, LOr                 // Bool x Bool -> Bool; the right operand is evaluated conditionally
};

struct Expr {
  enum class Kind {
    Const,       // `value`: a decimal integer (Int type; must lie in the type's range) or "true"/"false" (Bool)
    Local,       // `var`: a local, a parameter's local copy, or (read by `invariant`/`check`) the same
    Formal,      // `index`: parameter `index` as passed in (its ENTRY value); contracts only
    Result,      // the function's result; `ensures` only
    Old,         // `a`: the value of `a` on entry; `ensures` only; `a` must not mention Result or Local
    Unary,       // `uop`, `a`
    Binary,      // `bop`, `a`, `b`
    Cond,        // `a` ? `b` : `c` (a: Bool; b, c: same type); b and c are evaluated conditionally
    Convert,     // `a` (Int) converted to Int type `type`; `explicit_cast` says whether the program wrote it
    ToBool,      // `a` (Int) != 0, a Bool
    BoolToInt,   // `a` (Bool) as 0 or 1 of Int type `type`
    Call,        // `callee`(`args`...): type = callee's return type
    ReportAt     // `a`, but obligations inside it are reported at `loc` with a note at their own position
  };
  Kind kind = Kind::Const;
  Type type;     // the type of the value (Void only for a Call of a void function)
  SrcLoc loc;
  std::string value;                       // Const
  const Var* var = nullptr;                // Local
  int index = 0;                           // Formal
  UnaryOp uop = UnaryOp::Neg;              // Unary
  BinaryOp bop = BinaryOp::Add;            // Binary
  const Expr* a = nullptr;
  const Expr* b = nullptr;
  const Expr* c = nullptr;
  bool explicit_cast = false;              // Convert
  // How the program spells this node's type (a C/C++ type name such as "signed char" or "unsigned short"),
  // for messages only; empty means "use type.str()".  IntTypes are interned by shape, so the spelling the
  // user wrote lives here: set it on Convert nodes and on constants where it helps (Builder::spelled).
  std::string c_type_name;
  const Function* callee = nullptr;        // Call
  std::vector<const Expr*> args;           // Call: already converted to the parameter types
};

// ---- statements ------------------------------------------------------------------------------------------------

// A contract or invariant clause: a Bool expression and the position of the clause (the position a
// postcondition or invariant failure is reported at, and the note of a precondition failure).
struct Clause {
  const Expr* expr = nullptr;
  SrcLoc loc;
};

struct Stmt {
  enum class Kind {
    Decl,      // `var` = `expr` (a local always has an initializer; the type of `expr` is the type of `var`)
    Assign,    // `var` = `expr` (same type)
    ExprStmt,  // evaluate `expr` (a call, a discarded value, a `(void)e`): only its obligations and effects count
    If,        // if (`expr`) then_body [else else_body]; either body may be null
    While,     // while (`expr`) invariants body; `expr` has no calls (check.h)
    Return,    // return [`expr`]: expr's type is the function's return type, absent iff the function is void
    Check,     // check(`expr`): a Bool, asserted (category `check`)
    Block      // `stmts`
  };
  Kind kind = Kind::Block;
  SrcLoc loc;
  const Var* var = nullptr;                 // Decl, Assign
  const Expr* expr = nullptr;               // see Kind
  const Stmt* then_body = nullptr;          // If
  const Stmt* else_body = nullptr;          // If
  std::vector<Clause> invariants;           // While: the user's `invariant` clauses, in order
  std::vector<Clause> measure;              // While: the termination measure (`decreases`, decision 0013): one or more Int
                                            // expressions, compared lexicographically; required (check.h)
  const Stmt* body = nullptr;               // While
  std::vector<const Stmt*> stmts;           // Block
};

// ---- functions and programs ----------------------------------------------------------------------------------

struct Var {
  int id = 0;                // unique within its function
  std::string name;          // the C/C++ name (for messages and generated identifiers)
  Type type;                 // Bool or Int
  SrcLoc loc;                // the declaration
  std::string c_type_name;   // the C/C++ spelling of the type, for messages (empty: use type.str())
  bool is_param_copy = false;  // the local copy of a parameter (0012 item 2)
  int param_index = -1;        // for a parameter copy
};

struct Param {
  std::string name;  // may be empty (unnamed parameter)
  Type type;         // Bool or Int
  SrcLoc loc;
  std::string c_type_name;  // the C/C++ spelling of the type, for messages (empty: use type.str())
};

struct Function {
  int id = 0;                        // unique within the Program
  std::string c_name;                // for messages and generated identifiers
  SrcLoc loc;                        // the declaration (what a precondition note points at when a clause has no loc)
  SrcLoc end_loc;                    // the closing brace of the body (missing-return, void postconditions)
  Type ret = Type::void_type();      // Void, Bool or Int
  std::vector<Param> params;
  std::vector<Clause> requires_;     // the user's `requires`, in order; mention Formal parameters only
  std::vector<Clause> ensures_;      // the user's `ensures`, in order; Formal, Result and Old
  // The termination measure (`decreases`, decision 0013): Int expressions over the formals (the entry values), compared
  // lexicographically.  Empty for a function that is not part of a recursion cycle (then lowered as an empty `decreases`);
  // ignored for a function without a body (a trusted contract is assumed to terminate, 0013 item 5).
  std::vector<Clause> decreases_;
  const Stmt* body = nullptr;        // null: a function used only through its contract (a trusted contract)
  // For a function without a body: why its contract is trusted ("declared here, defined in another unit" or
  // "trusted, never verified" for a function of a system file).  Required then; emitted as a comment tag.
  std::string trusted_reason;

  bool verified() const { return body != nullptr; }

  // The local copy of parameter `k` that the body reads and assigns (created by add_param).
  const Var* param_var(int k) const { return locals_[param_vars_[static_cast<size_t>(k)]].get(); }

  // Declare a parameter; returns its index.  Also creates its local copy (a `Var` named like the parameter).
  int add_param(std::string name, Type type, SrcLoc loc);
  // Declare a local.  The pointer stays valid for the life of the Function.
  Var* new_local(std::string name, Type type, SrcLoc loc);
  const std::vector<std::unique_ptr<Var>>& locals() const { return locals_; }

 private:
  std::vector<std::unique_ptr<Var>> locals_;  // all Vars (parameter copies and locals)
  std::vector<size_t> param_vars_;            // index into locals_ of each parameter's copy
};

class Program {
 public:
  Program() = default;
  Program(const Program&) = delete;
  Program& operator=(const Program&) = delete;

  // Integer types are interned by (bits, is_signed); the first name given wins.
  const IntType* int_type(const std::string& name, int bits, bool is_signed);
  // The interned type of that shape; a type nobody named is created with a synthetic name ("int32", "uint64").
  const IntType* int_type_of(int bits, bool is_signed);

  // The width of C's `int` on this target (EDG's `sizeof(int) * CHAR_BIT`); integer promotion is relative to
  // it.  Set it once, before building; the default is 32.
  void set_c_int_bits(int bits) { c_int_bits_ = bits; }
  int c_int_bits() const { return c_int_bits_; }

  // New functions are appended in definition order; the order is the order of the emitted methods.  The
  // callee of a Call must be a function of this Program, added before the call is built.
  Function* add_function(std::string c_name, Type ret, SrcLoc loc, SrcLoc end_loc);

  const std::deque<Function>& functions() const { return functions_; }
  std::deque<Function>& functions() { return functions_; }

  // Arena (used by the Builder).
  Expr* new_expr() { return &exprs_.emplace_back(); }
  Stmt* new_stmt() { return &stmts_.emplace_back(); }

 private:
  int c_int_bits_ = 32;
  std::deque<std::unique_ptr<IntType>> int_types_;
  std::deque<Function> functions_;
  std::deque<Expr> exprs_;
  std::deque<Stmt> stmts_;
};

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
class Builder {
 public:
  explicit Builder(Program& p) : p_(p) {}
  Program& program() { return p_; }

  // Expressions.
  const Expr* int_const(Type int_type, std::string decimal, SrcLoc loc);
  const Expr* bool_const(bool value, SrcLoc loc);
  const Expr* local(const Var* v, SrcLoc loc);                     // a local or a parameter's copy
  const Expr* formal(const Function& f, int param_index, SrcLoc loc);  // the entry value (contracts only)
  const Expr* result(const Function& f, SrcLoc loc);               // `ensures` only
  const Expr* old(const Expr* e, SrcLoc loc);                      // `ensures` only
  const Expr* unary(UnaryOp op, const Expr* a, SrcLoc loc);
  const Expr* binary(BinaryOp op, const Expr* a, const Expr* b, SrcLoc loc);
  const Expr* cond(const Expr* c, const Expr* a, const Expr* b, SrcLoc loc);
  const Expr* convert(Type to_int_type, const Expr* a, bool explicit_cast, SrcLoc loc);
  const Expr* to_bool(const Expr* a, SrcLoc loc);                  // a != 0
  const Expr* bool_to_int(Type to_int_type, const Expr* a, SrcLoc loc);  // a ? 1 : 0
  const Expr* call(const Function* callee, std::vector<const Expr*> args, SrcLoc loc);
  // `note` says what the obligation's own position is ("in the default argument", "in the macro"); empty: the default.
  const Expr* report_at(const Expr* a, SrcLoc loc, std::string note = "");
  // Record how the program spells the node's type ("signed char"); returns `e`.  For messages only.
  const Expr* spelled(const Expr* e, std::string c_type_name);

  // Statements.
  const Stmt* decl(const Var* v, const Expr* init, SrcLoc loc);
  const Stmt* assign(const Var* v, const Expr* value, SrcLoc loc);
  const Stmt* expr_stmt(const Expr* e, SrcLoc loc);
  const Stmt* if_stmt(const Expr* c, const Stmt* then_body, const Stmt* else_body, SrcLoc loc);
  const Stmt* while_stmt(const Expr* c, std::vector<Clause> invariants, std::vector<Clause> measure, const Stmt* body, SrcLoc loc);
  const Stmt* return_stmt(const Expr* value_or_null, SrcLoc loc);
  const Stmt* check(const Expr* e, SrcLoc loc);
  const Stmt* block(std::vector<const Stmt*> stmts, SrcLoc loc);

  // `x op= rhs` and `++x`, `x++`, `--x`, `x--` as statements (0010 item 6).  The IL does not give the
  // computation type (P2), so these compute it with the rules above and expand to
  //     x = (T_x)( (C)x  op  (C)rhs )                      op in Add Sub Mul Div Rem, C = usual arithmetic
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
  Program& p_;
};

}  // namespace cerastes::vir
