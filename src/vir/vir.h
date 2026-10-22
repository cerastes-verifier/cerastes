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
// An enumeration (decision 0018 item 3) is the same type only as itself, and its values are [lo, hi], not the
// range of its bits and signedness (which are those of its underlying or compatible type).
struct IntType {
  std::string name;  // "int", "unsigned long", "char", ... (for messages and Viper comments)
  int bits = 0;
  bool is_signed = false;
  bool is_enum = false;    // an enumeration type (Program::enum_type)
  bool is_scoped = false;  // a C++ scoped enumeration
  std::string lo, hi;      // an enumeration's value range, decimal (dec::range_of)
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
  // Same VIR type (Int types: same bits and signedness; an enumeration only itself).
  bool operator==(const Type& o) const {
    if (kind != o.kind) return false;
    if (kind != Kind::Int) return true;
    if (int_type->is_enum || o.int_type->is_enum) return int_type == o.int_type;
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

// The shape of a loop statement (decision 0014 items 4 and 5).
//  While:   `while (c) { body; step }`; the invariants hold wherever C evaluates `c`.
//  DoWhile: `do { body } while (c)`; the invariants hold at the start of every iteration; `c` is evaluated after the body
//           (and after each `continue`), is encoded for its obligations and may contain no call.
//  DoOnce:  `do { body } while (c)` where `c` is a constant zero: the body runs once, no iteration follows another, and no
//           measure is required; lowered as straight-line code.
enum class LoopForm { While, DoWhile, DoOnce };

struct Stmt {
  enum class Kind {
    Decl,      // `var` = `expr` (a local always has an initializer; the type of `expr` is the type of `var`)
    Assign,    // `var` = `expr` (same type)
    ExprStmt,  // evaluate `expr` (a call, a discarded value, a `(void)e`): only its obligations and effects count
    If,        // if (`expr`) then_body [else else_body]; either body may be null
    While,     // a loop (see `form`): while (`expr`) invariants body [step]; `expr` has no calls (check.h)
    Return,    // return [`expr`]: expr's type is the function's return type, absent iff the function is void
    Check,     // check(`expr`): a Bool, asserted (category `check`)
    Block,     // `stmts`
    Switch,    // switch (`expr`) body; `expr` is the (promoted) controlling Int expression, `body` a Block; `jump_id`
    Case,      // a `case value:` / `default:` marker inside a Switch's own body Block: `expr` is the value (an Int Const of
               // the scrutinee's type) or null for `default`
    Break,     // break: leaves the loop or switch whose `jump_id` is `jump_id`
    Continue,  // continue: goes to the end of the iteration of the loop whose `jump_id` is `jump_id`
    Goto,      // goto: a user `goto` to the Label whose `jump_id` is `jump_id` (decision 0015 item 2; goto_problems)
    Label      // a user label `label_name:` with id `jump_id` (a direct statement of a Block)
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
  LoopForm form = LoopForm::While;          // While
  const Stmt* step = nullptr;               // While: a `for` loop's increment (an Assign or ExprStmt), run after the body and
                                            // after each `continue`; may be null
  int jump_id = 0;                          // While, Switch: this construct's id (> 0 when a Break or Continue may name it;
                                            // unique within a Function, Builder::jump_id()); Break, Continue, Goto: the target's id;
                                            // Label: its own id
  std::string label_name;                   // Label: the user's name
  bool from_assert = false;                 // Check: written as C's `assert(e)`: `expr` is typed like code (calls allowed) and a
                                            // failure says `assert` (decision 0015 item 6)
  std::string folded;                       // Case (not default): the front end's value of the label, a decimal Int literal of the
                                            // scrutinee's type; the dispatch compares with it and a prologue guard proves `expr` equals
                                            // it (decision 0017 item 5).  Empty for `default`.
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

// A named constant (decision 0017): a file-scope `const` object (global), or a `const` local of a function that a case
// value names.  Its uses are literals holding `value`; `init` is the initializer as written, checked in the prologue of
// every method (obligations, then an internal guard that it equals `value`).
struct NamedConst {
  int id = 0;                // unique within its list
  std::string c_name;        // the C/C++ name (messages and the Viper comment)
  Type type;                 // Bool or Int
  SrcLoc loc;                // the declaration (the guard's position)
  const Expr* init = nullptr;  // the initializer as written: call-free, no Local/Result/Formal/Old, exactly `type`;
                               // null for C's tentative definition (value "0"): nothing is checked
  std::string value;         // the front end's value, a decimal literal of `type` ("0"/"1" for Bool)
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
  // Declared not to return (`[[noreturn]]`, `_Noreturn`; decision 0015 item 4): with a body, every reachable `return` and the
  // closing brace are `noreturn` errors and the method ensures `false`; without one it is a trusted contract that ensures `false`
  // (`trusted_reason` required).  It has no user `ensures`.
  bool noreturn = false;

  bool verified() const { return body != nullptr; }

  // The local copy of parameter `k` that the body reads and assigns (created by add_param).
  const Var* param_var(int k) const { return locals_[param_vars_[static_cast<size_t>(k)]].get(); }

  // The `const` locals a case value of the body names (C++ only; decision 0017 item 6), checked like the program's named
  // constants, after them, in this function's method and in its `__wf` method.  Definition order = call order.
  NamedConst* add_local_const(std::string c_name, Type type, SrcLoc loc, const Expr* init, std::string value);
  const std::deque<NamedConst>& local_consts() const { return local_consts_; }

  // Declare a parameter; returns its index.  Also creates its local copy (a `Var` named like the parameter).
  int add_param(std::string name, Type type, SrcLoc loc);
  // Declare a local.  The pointer stays valid for the life of the Function.
  Var* new_local(std::string name, Type type, SrcLoc loc);
  const std::vector<std::unique_ptr<Var>>& locals() const { return locals_; }

 private:
  std::vector<std::unique_ptr<Var>> locals_;  // all Vars (parameter copies and locals)
  std::vector<size_t> param_vars_;            // index into locals_ of each parameter's copy
  std::deque<NamedConst> local_consts_;
};

class Program {
 public:
  Program() = default;
  Program(const Program&) = delete;
  Program& operator=(const Program&) = delete;

  // A fresh jump id (decision 0014): unique within the Program, so also within every Function.
  int fresh_jump_id() { return next_jump_id_++; }

  // Integer types are interned by (bits, is_signed); the first name given wins.
  const IntType* int_type(const std::string& name, int bits, bool is_signed);
  // The interned type of that shape; a type nobody named is created with a synthetic name ("int32", "uint64").
  const IntType* int_type_of(int bits, bool is_signed);
  // A new enumeration type (decision 0018 item 3), never interned: each call makes a distinct type.  `bits` and
  // `is_signed` are the underlying (C++) or compatible (C) type's; [lo, hi] is the value range, within that type.
  const IntType* enum_type(const std::string& name, int bits, bool is_signed, bool is_scoped, const std::string& lo,
                           const std::string& hi);

  // The width of C's `int` on this target (EDG's `sizeof(int) * CHAR_BIT`); integer promotion is relative to
  // it.  Set it once, before building; the default is 32.
  void set_c_int_bits(int bits) { c_int_bits_ = bits; }
  int c_int_bits() const { return c_int_bits_; }

  // New functions are appended in definition order; the order is the order of the emitted methods.  The
  // callee of a Call must be a function of this Program, added before the call is built.
  Function* add_function(std::string c_name, Type ret, SrcLoc loc, SrcLoc end_loc);

  const std::deque<Function>& functions() const { return functions_; }
  std::deque<Function>& functions() { return functions_; }

  // The unit's named constants (decision 0017), in definition order; every method with a body checks them in its prologue.
  NamedConst* add_named_const(std::string c_name, Type type, SrcLoc loc, const Expr* init, std::string value);
  const std::deque<NamedConst>& named_consts() const { return named_consts_; }

  // Arena (used by the Builder).
  Expr* new_expr() { return &exprs_.emplace_back(); }
  Stmt* new_stmt() { return &stmts_.emplace_back(); }

 private:
  int c_int_bits_ = 32;
  int next_jump_id_ = 1;
  std::deque<std::unique_ptr<IntType>> int_types_;
  std::deque<std::unique_ptr<IntType>> enum_types_;  // apart from int_types_, so int_type() never returns one
  std::deque<Function> functions_;
  std::deque<NamedConst> named_consts_;
  std::deque<Expr> exprs_;
  std::deque<Stmt> stmts_;
};

// ---- user gotos (decision 0015 item 2) -------------------------------------------------------------------------

// Why a Goto is not accepted.  A Goto with label id L is accepted iff the Label with that id is a direct statement of a
// Block that is an ancestor of the Goto, at an index after the one holding the Goto, with no Decl of that Block between
// the two.  Otherwise: `no_label` (no such Label in the function), `skips_declaration` (`variable` names the variable),
// `backward` (the Label comes before the Goto in a pre-order walk) or `enters` (anything else).
struct GotoProblem {
  const Stmt* go = nullptr;
  enum class Kind { backward, enters, skips_declaration, no_label } kind = Kind::no_label;
  std::string label;     // the user's name of the label (empty for no_label)
  std::string variable;  // skips_declaration: the variable
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
  // A loop in any form (decision 0014).  `measure` is required for While and DoWhile and may be empty for DoOnce; `c` is the
  // condition (for DoOnce it is still encoded, for its obligations); `step` may be null; `jump_id` is from jump_id() (0: no
  // Break or Continue names this loop).
  const Stmt* loop(const Expr* c, std::vector<Clause> invariants, std::vector<Clause> measure, const Stmt* body, const Stmt* step,
                   LoopForm form, int jump_id, SrcLoc loc);
  const Stmt* switch_stmt(const Expr* scrutinee, const Stmt* body, int jump_id, SrcLoc loc);
  const Stmt* case_label(const Expr* value_or_null, SrcLoc loc);  // null: `default`; a value needs the folded form below
  // `value` is the label as written (obligations checked in the prologue); `folded` is the front end's value (decision 0017 item 5).
  const Stmt* case_label(const Expr* value, std::string folded, SrcLoc loc);
  const Stmt* break_stmt(int target_jump_id, SrcLoc loc);
  const Stmt* continue_stmt(int target_jump_id, SrcLoc loc);
  int jump_id() { return p_.fresh_jump_id(); }
  const Stmt* return_stmt(const Expr* value_or_null, SrcLoc loc);
  const Stmt* check(const Expr* e, SrcLoc loc, bool from_assert = false);
  const Stmt* goto_stmt(int label_id, SrcLoc loc);
  const Stmt* label_stmt(int label_id, std::string name, SrcLoc loc);
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
