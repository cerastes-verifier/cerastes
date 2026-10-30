// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The lowered program: Viper-level, with the integer semantics of decision 0010 already applied.  `lower.cpp`
// produces it from VIR; `src/viper/emit.cpp` prints it and decides nothing (internal/encoding.md section 1).
// Names are final Viper identifiers; every check that can fail carries what a user must be told about it.
#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "vir.h"

namespace cerastes::low {

using vir::SrcLoc;

// ---- pure expressions (Viper expressions over Int and Bool) -------------------------------------------------

struct LExpr;
using E = std::shared_ptr<const LExpr>;

enum class UnOp { Neg, Not };
// PermDiv (decision 0037 item 6): a Perm divided by an Int literal (Viper's `/` on a permission); the only way to build it is `perm_div`.
enum class BinOp { Add, Sub, Mul, Lt, Le, Gt, Ge, Eq, Ne, And, Or, Implies, PermDiv };

struct LExpr {
  // Null, Field, PermOf, Write, NoPerm, Acc and Dom are the memory model's (decision 0032 item 5; decision 0028 item 2); they appear only in
  // a program that has a cell or a pointer, and `LProgram::memory` says so.  Old (decision 0037 items 6-7) is Viper's `old(a)`, in an
  // `ensures` and in a method body (the state at entry).  Acc has an optional amount in `b` (a Perm expression; null: full permission), and
  // Ite, PermOf, Write, NoPerm and PermDiv are the Perm-valued expressions (`LType::Perm` variables hold them).
  enum class Kind { IntLit, BoolLit, Var, Un, Bin, Ite, Fn, ModPow2, DivPow2, Null, Field, PermOf, Write, NoPerm, Acc, Dom, Old } kind = Kind::IntLit;
  std::string text;          // IntLit: a decimal (possibly negative); Var: the name; Fn: the prelude function; ModPow2, DivPow2: the divisor, 2^k, a decimal;
                             // Field, PermOf, Acc: the Viper field (`v_int`, `d_int`, `lv`); Dom: the domain function (`loc`, `len`, ...)
  bool bool_value = false;   // BoolLit
  UnOp un = UnOp::Neg;
  BinOp bin = BinOp::Add;
  E a, b, c;                 // operands; Ite: condition, then, else; Field, PermOf, Acc: the receiver (a Ref expression in `a`)
  std::vector<E> args;       // Fn, Dom
};

E int_lit(std::string decimal);
E bool_lit(bool v);
E var(std::string name);
E neg(E a);
E not_(E a);
E bin(BinOp op, E a, E b);
E ite(E c, E a, E b);
E fn(std::string prelude_name, std::vector<E> args);
// The memory model (decision 0032 item 5).
E null_();                                  // Viper's `null`
E field(E recv, std::string field_name);    // `recv.field` (a heap read: the lowering has checked the permission before)
E perm_of(E recv, std::string field_name);  // `perm(recv.field)`
E write_perm();                             // `write`
E no_perm();                                // `none`
E acc(E recv, std::string field_name);      // `acc(recv.field)`, in an `exhale` or a contract
E acc(E recv, std::string field_name, E amount);  // `acc(recv.field, amount)`, a contract clause (decision 0037 item 6)
E old_(E a);                                // `old(a)`: the value in the state at entry (an `ensures`, or the body of a method)
E perm_div(E a, int divisor);               // `a / divisor`, a Perm over a positive Int literal (decision 0037 item 6)
// An application of one of the Block domain's functions (`loc`, `len`, `block_of`, `offset_of`, `member`, `parent_of`, `index_of`,
// `lvloc`) or a global's block constant (`GB_<...>`, no arguments, decision 0041); any other name is refused (std::logic_error), so lowering
// cannot spell a domain function the prelude does not declare.
E dom(const std::string& domain_function, std::vector<E> args);
bool is_domain_function(const std::string& name);
// a % 2^k, Viper's `%` (the non-negative remainder for a positive divisor).  The only way to build a `%`: the divisor is a power of two
// (k >= 0), which is what decision 0026 item 7 needs for a constant mask, so it cannot stand for a C `%` (C's `%` goes through the
// prelude, decision 0010).
E mod_pow2(E a, int k);
// a \ 2^k, Viper's integer division (the floor quotient for a positive divisor), so that 2^k * (a \ 2^k) is a with its low k bits cleared
// (a - a % 2^k) without naming a twice (decision 0026 item 7).  Like mod_pow2 it is the only way to build a `\`, with a power-of-two divisor.
E div_pow2(E a, int k);
E and_(E a, E b);                 // null operands are "true"
E implies(E guard_or_null, E c);  // `c` when the guard is null
// lo <= e && e <= hi  (for contracts and invariants)
E in_range(E e, const std::string& lo, const std::string& hi);
// !(e < lo || hi < e): the same condition as ONE atomic assertion.  Silicon reports each conjunct of an
// asserted `&&` as its own error; an obligation must produce one error.
E in_range_atomic(E e, const std::string& lo, const std::string& hi);

// ---- what a user is told about a failed check -----------------------------------------------------------------

struct Meta {
  // One of the driver's categories (0011 item 10), or "internal" for a check whose failure is a bug of the
  // encoder (a backstop, an automatic range invariant): the driver exits with status 3.
  std::string category;
  SrcLoc loc;
  std::string message;
  bool has_note = false;
  SrcLoc note_loc;
  std::string note_text;
};

// A check the emitter writes on its own line and records in the position map: the Viper error id Silicon's
// message maps to (the table of scripts/run_tests.py) and the meaning of a failure.
struct Check {
  std::string viper_id;  // "assert.failed", "call.precondition", "invariant.not.established", ...
  Meta meta;
};

// ---- statements ---------------------------------------------------------------------------------------------------

struct LStmt;

struct LInvariant {
  E expr;
  std::vector<Check> checks;  // usually both invariant.not.established and invariant.not.preserved
};

struct LStmt {
  // FieldAssign and Exhale are the memory model's (decision 0032): `name.field := expr` (name: a Ref variable) and `exhale expr`.
  enum class Kind { Assign, Assert, Call, If, While, Goto, Label, Block, Comment, FieldAssign, Exhale, CaseSplit } kind = Kind::Block;
  // CaseSplit (decision 0037 items 8-9): the no-op `if (<expr>) { }`, one line, that makes Silicon branch on whether two terms are equal.
  std::string name;            // Assign: target; Call: the method; Goto, Label: the label; Comment: the text; FieldAssign: the receiver variable
  std::string field;           // FieldAssign: the field written
  E expr;                      // Assign: value; Assert: the condition; If, While: the condition
  Check check;                 // Assert; Call (its call.precondition backstop)
  bool has_termination = false;  // Call: the callee has a termination measure, so Silicon may report termination.failed here
  Check termination;           // Call: that check (decision 0013 item 8)
  std::vector<E> args;         // Call
  std::string target;          // Call: the result variable, or empty
  std::vector<LStmt> then_body, else_body;  // If; While uses then_body as the loop body
  std::vector<LInvariant> invariants;       // While
  std::vector<E> decreases;                 // While: the termination measure, one line `decreases e1, e2` (always at least one)
  Check decreases_check;                    // While: what a termination.failed at that line means
  std::vector<LStmt> stmts;    // Block
};

// ---- methods and the program ------------------------------------------------------------------------------------

// The Viper type of a variable: Int, Bool, Ref (a pointer; an address-taken local, which holds its cell) or Perm (a ghost amount).
// Perm: a ghost permission amount (decision 0037 item 6): a formal `none < a && a < write`, or a local that holds one.
enum class LType { Int, Bool, Ref, Perm };

struct LVar {
  std::string name;
  LType type = LType::Int;
};

struct LClause {
  E expr;
  std::vector<Check> checks;  // the failures Silicon can report at this clause's line
};

struct LMethod {
  std::string name;
  std::string comment;                   // a `//` line above the method (the C/C++ function it stands for)
  std::vector<LVar> formals;
  std::vector<LVar> results;             // zero or one
  std::vector<LClause> requires_;
  std::vector<LClause> ensures_;
  // `decreases e1, ...` (decision 0013): printed on every method; empty for a function outside any recursion cycle and
  // for the well-formedness methods; `decreases _` (assumed) only for a bodyless method with a trusted contract.
  std::vector<E> decreases_;
  bool decreases_assumed = false;
  Check decreases_check;                 // the map entry of a non-empty `decreases` line (termination.failed)
  bool has_body = false;
  std::vector<LVar> locals;              // declared at the top of the body
  std::vector<LStmt> body;
  std::string trusted_reason;            // bodyless methods: emitted as `// cerastes:trusted-contract <reason>`
};

struct LProgram {
  std::vector<LMethod> methods;
  std::set<std::string> prelude_used;    // the prelude functions the methods call (emit.cpp adds their dependencies)
  // The memory prelude (decision 0032 item 5), emitted only when `memory` is set: the fields `v_<t>: <type>` and `d_<t>: Bool` of each cell
  // type `<t>` (the key; the value is the type of `v_<t>`), `field lv: Bool`, and the Block domain with its four axioms.
  bool memory = false;
  std::map<std::string, LType> cell_types;
  // The Viper names of the globals' block constants, `GB_<name>__<id>` (decision 0041 item 4), in declaration order: declared `unique function
  // GB_...(): Block` inside the Block domain (only with `memory`).
  std::vector<std::string> globals;
};

}  // namespace cerastes::low
