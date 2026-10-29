# Encoding: C/C++ → VIR → Viper

Developer reference for the encoder (plan.md: "Design VIR and document it in `encoding.md`"; it grows each
milestone). The decisions behind it are 0010 (integer semantics), 0011 (driver), 0012 (scalar encoding), 0014
(control flow), 0015 (forward `goto`, functions that do not return, `assert`), 0017 (named constants and case values).
This file says *how*; those records say *what* and *why*. Status: M3 (scalar code), M4.1 (control flow), M4.2a and
M4.2b1; pointers inside one function since M4.5b1a (decisions 0028, 0032). Sections marked *(M3-n)* are
filled in at that checkpoint.

## 1. Pipeline and layering

```
EDG IL ──(src/backend/encode.cpp: IL → VIR, all C/C++ knowledge, allow-lists, rejections)──▶ VIR
VIR ──(src/vir/check.cpp: typing rules; fails closed)──▶ checked VIR
VIR ──(src/vir/lower.cpp: integer semantics → obligations, conversions)──▶ lowered VIR
lowered VIR ──(src/viper/emit.cpp + prelude.cpp + posmap.cpp: text only, no semantics)──▶ .vpr + .vpr.map
.vpr ──(Silicon, src/driver/silicon.cpp)──▶ errors ──(position map, src/driver/report.cpp)──▶ C/C++ lines
```

- `src/vir` and `src/viper` do not include EDG headers and do not link `edg_config`. Only `src/backend` sees the IL.
- All C/C++ meaning lives in `encode.cpp` (what the program says) and `lower.cpp` (what the operations mean,
  0010). The emitter prints; it decides nothing. The soundness auditor reviews those two files hardest.

## 2. VIR (`src/vir/vir.h`)

Front-end agnostic. The encoder builds a `Program` with a `Builder`; `check_program` (`check.h`) type-checks it;
`lower_program` (`lower.h`) lowers it to the Viper-level form (`lowered.h`); `src/viper` prints that.

- **Ownership.** `Program` is an arena (functions, integer types, expression and statement nodes). Builder calls
  return pointers valid for the life of the Program; expression nodes are immutable and may be shared.
- `SrcLoc { file, line, column }`: the C/C++ position by the rule of 0006 item 10. Every node has one; an
  obligation of a node is reported at the node's `loc` (the operator token for an operator, the `(` of a call,
  the conversion's own position, or the enclosing initializer/argument/`return` for a generated conversion).
- `IntType { name, bits, is_signed }`, filled from EDG's target configuration (0010 item 2). **Types are
  interned and compared by `(bits, is_signed)`** (`unsigned int` and `char32_t` are one VIR type): the
  spelling the user wrote lives in `Expr::c_type_name` / `Var::c_type_name` / `Param::c_type_name` and is used
  only in messages (`Builder::spelled`). `Program::set_c_int_bits` fixes the width of `int` (default 32).
  Since M4.2b2 (decision 0018) an **enumeration** is an `IntType` with `is_enum`, made by `Program::enum_type`:
  never interned, equal only to itself, with the size and signedness of its underlying (C++) or compatible (C)
  type and its own value range `[lo, hi]`, which `dec::range_of` returns, so every range rule below (parameters,
  results, automatic loop invariants, constants) uses the value range.
- `Type = Void | Bool | Int(IntType) | Ptr(pointee)` (`Ptr` since M4.5b1a, decision 0032: interned by
  `Program::pointer_to`, equal when the pointees are; a pointee is `Bool` or `Int`, enumerations included). `check.cpp`
  accepts `Ptr` only for locals that are not address-taken and for expressions: never for parameters or results, never
  for an address-taken local (`T**` cells are M4.5b1c).
- `Expr` (pure, typed): `Const(decimal | true/false)`, `Local(var)`, `Formal(k)`, `Result`, `Old(e)`,
  `Unary{Neg, Plus, Not, BitNot}`, `Binary{Add, Sub, Mul, Div, Rem, Shl, Shr, Lt, Le, Gt, Ge, Eq, Ne, LAnd, LOr, BitAnd,
  BitOr, BitXor}` (the bitwise operators appended in M4.4b1, decision 0023),
  `Cond(c, a, b)`, `Convert(to, e, explicit)` (Int to Int), `ToBool(e)` (`e != 0`), `BoolToInt(to, e)`,
  `Call(callee, args)`, `ReportAt(e, loc)` (obligations inside `e` are reported at `loc`, with a note at their
  own position: the obligations of a default argument are reported at the call). Since M4.3a (decision 0020) two
  expressions have side effects: `AssignE(var, a, b, yields_old)` (an assignment, compound assignment, `++` or `--`
  used as a value: `a` is the value stored, already converted to `var`'s type by `Builder::assign_expr`,
  `compound_assign_expr`, `inc_dec_expr`; for `x++`/`x--`, `b` is the one `Local x` node that `a` also uses, the old
  value, and the value of the node is `b`) and `Comma(a, b)` (only where its value is used; `a` may be a `void` call).
  A top-level assignment is still the statement `Assign`, and a discarded comma is the statements of its operands.
- `Stmt`: `Decl(var, init)` (since M4.2c, decision 0019, `init` may be null: a local declared without an
  initializer), `Assign(var, e)`, `ExprStmt(e)` (a call, a discarded value; only its obligations
  and effects count), `If(c, then, else)`, `While(cond, invariants[], measure[], body, step?, form, jump_id)`,
  `Return(e?)`, `Check(e)`, `Block(stmts)`, and since M4.1 (decision 0014) `Switch(scrutinee, body Block, jump_id)`,
  `Case(value | default)` (a marker directly in a switch body), `Break(id)`, `Continue(id)`. A loop's `form` is
  `While` (the test before the body; `step` is a `for` loop's increment, an `Assign` or `ExprStmt`), `DoWhile` (the
  test after the body) or `DoOnce` (`do … while (0)`: the condition evaluates to zero, no measure needed). Jump ids
  are unique in the Program (`Builder::jump_id()`); a `Break` names the innermost enclosing loop or switch, a
  `Continue` the innermost enclosing loop, so every jump is forward and none enters a loop. Since M4.2a (decision
  0015) `Goto(label id)` and `Label(id, name)`: `vir::goto_problems` (used by `check_program` and by the encoder for
  its diagnostics) accepts a `Goto` only if its `Label` is a direct statement of a `Block` enclosing the `Goto`, after
  the `Goto`'s ancestor there, with no `Decl` with an initializer of that `Block` between them ("outward and
  forward"; a `Decl` without one may be jumped past since M4.2c); so user jumps are
  forward too and never enter a block or a loop. `Check(e, from_assert)`: an `assert` from `<assert.h>` is a `Check`
  whose `e` is a code expression (calls allowed, as in an `if` condition). `Function::noreturn` marks a function
  declared not to return. There is no statement for falling off the end: lowering appends the epilogue of the function
  (`missing-return` for a non-`void` function, the `ensures` for a `void` one). `main`'s generated `return 0`
  is an ordinary `Return`.
- **Memory (M4.5b1a, decision 0032).** `Var::address_taken` (set by `Function::mark_address_taken` from the IL's
  `address_taken` flag, P51/P57) makes a local a *cell* (lowering, §3). Expressions `Null` (a pointer-typed null
  constant; in `==`/`!=` and as a `?:` arm it may carry another pointer type, which lowering ignores), `AddrOf(var)`
  (`&x` of an address-taken, non-parameter local) and `Deref(a)` (a read of `*a`; its type is the pointee). The
  statement `Store(addr, expr)` writes `*addr`; `Builder::store`, `compound_store` and `inc_dec_store` build `*p = e`,
  `*p op= e` and `++*p`/`(*p)--`, the compound forms reading a `Deref` that shares the `addr` node, so the pointer is
  evaluated once. `Expr::c_text` is the source spelling messages use (`p`, `*p`, `&x`; empty when unknown).
  `Stmt::end_loc` is a `Block`'s closing brace (required on a block that declares a cell: its scope exit is reported
  there). Assign and Decl of a cell keep their kinds; lowering dispatches on the flag. `check.cpp`: `==`/`!=` of two
  pointers of one type (a `Null` takes the other's), `ToBool` of a pointer, `Cond` with pointer arms; no arithmetic,
  ordering, bitwise or `Convert` on a pointer; no pointer node in contracts, invariants, measures, named constants or
  case values; no `AssignE` on a cell; and **`vir::memory_problems(Function)`** (`check.h`), used by the encoder for its
  diagnostics and by `check_program` as the backstop, finds decision 0032 item 10's shapes: a loop (any form) holding a
  pointer-typed node, a cell, `AddrOf`, `Deref` or `Store`; a `Return` in a loop while a cell of an enclosing block is
  live; a `Break`/`Continue`/`Goto` leaving a block that declares a cell; a cell-declaring block without `end_loc`.
  `sequence_problems` treats `Store(addr, value)` as two unsequenced operands.
- **Contracts over memory (M4.5b1b, decision 0037).** Pointer `Param`s and a pointer `Function::ret` (P58: no
  separate copy in the IL; `Param::is_restrict` from the parameter variable's type, C only). `Expr::Kind::Atom`
  (`AtomKind` `Owns`/`Reads`/`Live`, `a` = the pointer `Formal`, `amount` = the index of the `requires` atom, 0..N-1 in
  evaluation order; an `ensures` atom carries its `requires` partner's `amount`, same kind and parameter) appears only in
  `requires`/`ensures`; `contract_atoms(f)` lists the atoms with their guards (`AtomGuard {cond, holds}`: the left of an
  `||` with `holds == false`, a `?:` condition either way; `&&` adds none), `requires_atom_count(f)` gives N,
  `reads_heap(e)` tells whether an expression holds a `Deref` or an `Atom`. Heap reads in a contract are `Deref(Formal
  p)` (pre-state in a `requires`, post-state in an `ensures`) and `Old(Deref(Formal p))`. `check.cpp` splits `no_ptr_`:
  contracts may hold pointer `Formal`/`Result`, `Null`, pointer `Cond`, `==`/`!=`, `ToBool`, `Deref(Formal)`, atoms
  and `Old`; invariants, measures, named constants and case values still hold none. Backstops: atom positions, numbering
  and partners, duplicates, `restrict_backed(f, k)` (0037 item 2), and `placement_problems(f)` (0037 item 8: a call of
  a function with an atom beside another cell access or a second such call). `&param`: `address_taken` on a scalar
  parameter's copy (a pointer-typed one stays refused, b1c).

- `Function { id, c_name, params[], ret, requires[], ensures[], body?, trusted_reason, loc, end_loc }`; a null
  body is a trusted contract (`trusted_reason` required). Each parameter has a *local copy* (`param_var(k)`)
  that the body reads and assigns; contracts read the formal (`Formal(k)`, the entry value).
- `Program { functions, named_consts }`. Since M4.2b1 (decision 0017) a **`NamedConst { id, c_name, type, loc, init?, value }`**
  is a `const` global of the unit (`Program::add_named_const`) or a `const` local that a case value names
  (`Function::add_local_const`): `init` is the initializer as written (null for C's tentative definition), `value` is the
  front end's value (a decimal literal of `type`, `0`/`1` for `bool`). A use of a named constant is an ordinary `Const`
  of that value; there is no `Expr` kind for it. `Case` also carries **`folded`**, the front end's value of the label
  (`Builder::case_label(value, folded, loc)`), a decimal literal of the scrutinee's type; empty only for `default`.
- **Compound assignment, `++`, `--` are expanded by the Builder** (`compound_assign`, `inc_dec`), not by the
  encoder, so that the promotion rules exist in one place (`promoted`, `usual_arithmetic` in `vir.cpp`): the IL
  does not give the computation type (0010 item 6). `x op= e` is `x = (T)((C)x op (C)e)`; shifts convert only
  the left operand (`C` = the promoted left type); `++x` is `x += 1`. The conversion back to `T` is a checked
  conversion.

`check.cpp` typing rules (fail closed; a violation is a `Diag{loc, reason}` the encoder reports as
`unsupported_expression`):
- arithmetic operands and result have the same `IntType` (EDG's casts make promotions explicit, 0010 item 6);
- shift result type = left operand type (the count is typed on its own);
- comparisons: operands of one type; result `Bool`. In C the encoder wraps with `BoolToInt` (and reads an
  integer condition or logical operand with `ToBool`, 0010 item 6, C's truth values);
- `!`, `&&`, `||`, the conditions of `if`, `while`, `?:`, contract clauses, `check` and invariants: `Bool`;
- `Convert` is Int to Int; `ToBool` and `BoolToInt` are the only bridges;
- assignments, initializers, returns and call arguments have exactly the target's type;
- constants lie in their type's range; every node has a source position;
- `Formal`, `Result`, `Old` only in contracts (`Result`, `Old` only in `ensures`; `Old` not of the result);
  `Local` not in contracts; a local is declared before use, in scope, once;
- **calls** (0012 item 6, 0021): not in contracts, `check`, invariants, measures, case values, named-constant
  initializers or a `DoOnce` condition (`Ctx::Annot`); since M4.3b they may stand in a conditionally evaluated operand
  (the right of `&&`/`||`, a branch of `?:`), in a loop condition (`Ctx::LoopCond`; the loop then has a jump id), and
  anywhere in a full expression: whether the calls of a full expression can be ordered is decided by lowering (the
  dependency-chain rule, §3, reported through `LowerResult::rejections`); a `void` call is the whole of an `ExprStmt` or
  the left operand of a `Comma`;
- **side effects** (0020 item 9): `AssignE` and `Comma` only in code (statement expressions, initializers, returns,
  `if` and `switch` conditions, arguments, and loop conditions, `Ctx::LoopCond`, which accepts calls since M4.3b); never in
  contracts, `check` or `assert` conditions (`Ctx::Assert`), invariants, measures, case values, named-constant
  initializers or a `DoOnce` condition; an `AssignE` node is used once in a function, its `b` is a `Local` of its
  variable used in `a`; a loop whose condition has a side effect has a jump id; a `for` step may be a `Block` of
  `Assign`/`ExprStmt`; and `vir::sequence_problems` (0020 item 4: no local written by one operand and read or written
  by an operand not sequenced with it) finds nothing, which the encoder reports first as `unsequenced_modification`;
- a function without a body has a `trusted_reason`;
- **named constants** (0017): `init`, when present, is call-free, mentions no `Local`, `Formal`, `Result` or `Old`, and
  has exactly the constant's type; `value` is a literal of that type; a constant without `init` has value `0`; a
  function without a body has no local constants;
- **control flow** (0014): jump ids are unique; a `Break`/`Continue` names its innermost target (anything else is an
  internal inconsistency); a `Case` stands directly in its switch's body, which starts with a `Case` (or with `Decl`s
  without an initializer, M4.2c), has no `Decl` with an initializer before a later `Case` and at most one default; a case value is a call-free, local-free Int expression of exactly
  the scrutinee's type; every non-`default` `Case` has `folded`, a literal in the scrutinee type's range, and the
  `folded` values are distinct; the scrutinee is Int and may contain calls (evaluated once, as a
  `Decl` initializer); `While`/`DoWhile` need a measure, a `DoOnce` none; a `DoOnce`'s condition is rejected if it reads
  a local, calls a function or folds cleanly to a non-zero value, and accepted when the checker cannot fold it (its
  obligations and lowering's internal `assert !c` decide); no calls in a `DoOnce` condition (other loop conditions may
  call, 0021); `step` only on a `While`-form loop.

## 3. Lowering (`src/vir/lower.cpp`)

Each expression becomes a pure Viper value and obligations `{guard, cond, category, loc, message}`. A guard is
the condition under which C/C++ evaluates the operand (0012 item 5). Per operation (0010; `T` is the operation's
type, `lo`/`hi` its range, `w` its width):

| Operation | Value | Obligations (category) |
|---|---|---|
| `a + b`, `a - b`, `a * b` (signed or unsigned, 0022) | `a op b` | `lo <= a op b <= hi` (`overflow`) |
| signed `-a` | `-a` | `a != lo` (`overflow`) |
| unsigned `-a` (0022) | `-a` | `a == 0` (`overflow`) |
| `+a`, `!a` | `a`, `!a` | none |
| signed `a / b`, `a % b` | `c_div(a, b)`, `c_rem(a, b)` | `b != 0` (`division-by-zero`); `!(a == lo && b == -1)` (`overflow`) |
| unsigned `a / b`, `a % b` | `c_udiv(a, b)`, `c_urem(a, b)` | `b != 0` (`division-by-zero`) |
| `a << n` signed | `a * c_pow2(n)` | `0 <= n < w`; `a >= 0`; `a * c_pow2(n) <= hi` (`shift`, three obligations) |
| `a << n` unsigned (0022) | `a * c_pow2(n)` | `0 <= n < w`; `a * c_pow2(n) <= hi` (`shift`, two obligations) |
| `a >> n` | `c_shr(a, n)` (floor division by `2^n`) | `0 <= n < w` (`shift`) |
| `a & b`, `a \| b`, `a ^ b` (0023) | `c_and_N(a, b)`, `c_or_N`, `c_xor_N` (signed `T`), `c_uand_N`, `c_uor_N`, `c_uxor_N` (unsigned `T`), `N` = `w` | none |
| `a & m`, `m & a`, `m` a constant mask (0026 item 7) | `m = 2^k − 1`: `(a % 2^k)`; `m = ~(2^k − 1)`: `(2^k * (a \ 2^k))` (Viper's `%` and `\`, SMT-LIB's `mod` and `div`; `k ≤ w` unsigned, `k ≤ w − 1` signed; `m` is the operand as lowered, evaluated exactly; built only by `low::mod_pow2`/`div_pow2`, linted) | none |
| `a \| m`, `a ^ m` (and `m` on the left, `\|=`, `^=`), `m` a constant mask (0027 item 1) | with `H = a \ 2^k`, `L = a % 2^k`: `a \| (2^k − 1)` = `2^k * H + (2^k − 1)`; `a ^ (2^k − 1)` = `2^k * H + ((2^k − 1) − L)`; `a \| ~(2^k − 1)` = `L − 2^k` signed, `L + (2^w − 2^k)` unsigned; `a ^ ~(2^k − 1)` = `(L − 2^k) − 2^k * H` signed, `(L + (2^w − 2^k)) − 2^k * H` unsigned. The `\|` forms are inline and name `a` once; the `^` forms are calls of prelude functions `c_[u]xor_mask_w_k(a)` and `c_[u]xor_cmask_w_k(a)`, emitted when used, whose body is the form and whose postcondition is `in_range(a) ==> in_range(result)` (0027 item 3: a nested chain then gets each level's range from the postcondition) | none |
| `~a` (0023) | `-1 - a` (signed), `hi - a` (unsigned) | none |
| `Convert` to signed `T` | `e` | `lo <= e <= hi` (`conversion`); none when the source range is inside `T`'s |
| `Convert` to unsigned `T` (0022) | `e` | `lo <= e <= hi` (`conversion`); none when the source range is inside, or when `e` is a constant in range (0022 item 4) |
| `Convert` to an enumeration `T` (0018) | `e` | `lo <= e <= hi` over the value range (`conversion`), never a wrap; none when the source range is inside |
| `ToBool(e)`, `BoolToInt(e)` | `e != 0`, `e ? 1 : 0` | none |
| `a && b`, `a \|\| b`, `c ? x : y` | the Viper operators | the obligations of `b` are guarded by `a` (`!a`), of `x` by `c`, of `y` by `!c` |

An obligation is asserted as ONE atomic condition (a range is `!(e < lo || hi < e)`, not a conjunction):
Silicon reports each conjunct of an asserted `&&` as its own error.

**Calls and the placement of obligations** (0012 item 6, 0021). The calls of a full expression are hoisted into
temporaries `t__k := f(args)`, each preceded by `assert`s of the callee's `requires` over the argument values
(category `precondition`, note at the clause). Every item has the **set** of fixed points (calls, and branches that
contain a call) C must have completed before it evaluates the item: those whose values it uses, its guard's, the comma
floor's, and those of the earlier accesses to the locals it touches. **Two steps** (0026): the *exact* sets give a
call's precondition PRE(c) (everything its arguments use or contain, and the floor) and a comma's value its floor; the
*base* sets (0021's) give a precondition only the argument values its clauses read and a comma's value no floor.
Every set is computed with both halves in one pass; `flush` uses the exact family when it is a chain, else the base
family when that is (today's Viper exactly), else rejects with the base family's conflict. Each full expression,
and each branch's own, decides on its own. **The dependency-chain rule** (0021 item 3, 0025 item 2): the sets of a full expression (every
obligation and call-free branch; PRE ∪ {B} for each branch with a call; PRE of each explicit termination check) must
form a chain under ⊆, or the expression is rejected (`unsequenced_calls`, at the later of two witnessing calls); the
calls are then emitted in the chain's order (the members of the smallest set first, ties in creation order) and each
item right after the calls of its set and before every other, so a callee that never returns cannot hide a check C may
evaluate before it. Writes and snapshots, which cannot fail, are not in the family (M4.4c): each goes right after the
member of its set that comes last in that order (first when its set is empty), items in one place keeping creation
order; fail closed, every member of its set is emitted before it and every later item whose set contains its set after
it. For every expression accepted before M4.3b (calls nested or ordered by `,`) the base sets are intervals
of creation order and the base family emits that version's Viper; the exact family moves a precondition after the calls
in its arguments, and an item using a comma's value after the comma's left operand (0026 item 5). **Termination at a call** (0021 item 4): a call in the caller's
recursion cycle (Viper's strongly connected components: an edge for every call node, dead code included) that is
emitted after a call outside its PRE gets an explicit `assert` of Silicon's own decrease relation (the callee's
measure over the arguments against the caller's over its parameters, unequal lengths included), category
`termination`, placed by its PRE; Silicon's check at the call stays as a backstop.

**Side effects** (M4.3a, decision 0020 item 7). A full expression's `Full` holds its obligations and an ordered list
of **effects**: a call (as above), a write (`v := value; d := true`, the flag only for a flagged local), a snapshot
(`t := v`) and a branch (`if (c) { … } else { … }` assigning a temporary). Calls, and since M4.3b branches that
contain a call, are the fixed points, in the order of the chain above; every other item (obligation, write, snapshot,
call-free branch) goes right after the fixed points of its set (the temporaries it mentions, its guard's, the floor,
and for a snapshot of `x` every earlier write of `x`), or before the first fixed point when its set is empty — the
call rule above (writes and snapshots by the last member of their set, 0025 item 2) — and items in the same place keep
evaluation order (left to right for unsequenced operands). The `uninitialized` check of a snapshot of a flagged local
counts as an access of the local with the check's own set (guard and floor included), so a later write of the local
follows it (0025 item 1). A branch with a call holds its own full expression, placed by the same rule inside
the `if`. Every read of a local that the full expression writes is a snapshot
at its point (its `uninitialized` obligation follows the snapshot), so a printed value never names a variable the
expression writes; the value of an `AssignE` is its lowered `a` (or the snapshot of `b`). A right operand of `&&`/`||`
or an arm of `?:` with a side effect is a branch, whose effects and obligations are emitted inside the `if`; an
operand without one keeps the guard of the table above. After the left operand of `,` every later item depends at
least on the left operand's last call (the floor), so nothing of the right operand moves before it; with the exact
sets the comma's value carries the floor too (0026). After `flush`, a
fail-closed check confirms that the snapshots and writes of each variable kept their evaluation order. A full expression without
`AssignE` or `Comma` lowers exactly as before.

**Loops.** `while (c) { body }` becomes a Viper `while` whose invariant clauses are, in order: the automatic
range invariant of every local that the loop assigns and did not declare (category `internal`); for each user
invariant, the obligations of its expression (guarded by the earlier user invariants), then the invariant
(`invariant-entry` / `invariant-preserved`); then the obligations of the loop condition (guarded by all user
invariants). An obligation as an invariant clause is checked exactly where 0012 item 8 asks (before the first
evaluation of the condition, and before each next one) and is known inside the body.

**Jumps** (M4.1, decision 0014). A `While` loop is `while (c) inv dec { body; label l_cont__N; step } label
l_brk__N`, each label emitted only when a `Continue`/`Break` with that id exists (so code without jumps lowers as in
M3); `Continue` is `goto l_cont__N`, `Break` is `goto l_brk__N`. A continue path reaches the end of the body, so the
loop's own invariant, condition-obligation and measure clauses are checked there; no new map entries. A `DoWhile` is
`while (true) inv dec { body; label l_cont__N; assert <obligations of c>; if (!c) { goto l_brk__N } } label l_brk__N`.
A `While` whose condition has a side effect (M4.3a, 0020 item 7) is lowered with the test first, as the `do` shape
moved to the top: `while (true) inv dec { <the condition's effects and obligations>; if (!c) { goto l_brk__N } body;
label l_cont__N; step } label l_brk__N`. The locals a loop assigns (automatic range invariants, `ds_` snapshots)
include those written in any expression of the loop, its own condition and nested loops' conditions included.
A `DoOnce` is no loop: its invariants (with their obligations) are asserted at entry as `invariant-entry` and its
measure's obligations there too, then `body; label l_cont__N; assert <obligations of c>; assert !c` (category
`internal`) `; label l_brk__N`. A `Switch` lowers its scrutinee like any expression into a temporary (obligations
placed by the call rule), dispatches with
`if (t == folded) { goto l_case__N_i }` in source order (the front end's value, nothing asserted: the case values are in
the method's prologue, below) and then `goto l_default__N` or `goto l_brk__N`, and places a
label at each `Case` marker. The automatic range invariants of a loop include the locals assigned in its `step` and
in switch bodies. `lint_viper.py` rejects a backward `goto`, a label carrying invariants and a duplicate label.
A user `Goto` is `goto l_user_<name>__N` and its `Label` is `label l_user_<name>__N`, emitted only when a `goto`
targets it. Since M4.2a `lint_viper.py` checks every `goto` (generated or user) at token level: its label must come
later, inside braces that also enclose the `goto` (so no jump into a loop body or an `if` branch).

**The prologue** (M4.2b1, decision 0017 items 4 and 5). Every method that has a body, each function's method and
each `__wf`, begins, right after the copies of the parameters (at the start of the body in `__wf`), with:
(1) for each `NamedConst` of the program in definition order: `init` lowered like any expression, its obligations asserted
at their own positions, then an `internal` guard `assert <init> == <value>` at the constant's `loc` (message "internal
check: the value of `X` as written differs from the front end's"); the guard is omitted when `init` is a bare `Const`
equal to `value`, and a constant without `init` has neither; (2) the function's own local constants, the same way;
(3) each `Case` of the function's body in source pre-order (a nested switch's labels
included), its value's obligations asserted re-positioned at the label, with the note "in the value of this case label"
when an obligation's own position is on another line (decision 0014 item 6), then an `internal` guard `assert <value> ==
<folded>` at the label (message "internal check: case value as written differs from the front end's"), omitted when the
value is a bare `Const` equal to `folded`. Each case's guard follows that case's own obligations. Items (2) and (3) are
emitted in the function's own method **and in its `__wf` method** (which exists exactly when the function has a user
`requires`, `ensures` or `decreases` and assumes only the type ranges), with identical metas, so the driver's removal of
identical reports leaves one error; this is how a case value of a function whose `requires` is unsatisfiable (`requires
x > 0 && x < 0`, which makes its own method vacuous) is still checked. A prologue expression is closed: if lowering one
finds a call, a `Formal`, `Old`, `Result` or `Local`, that is an internal lowering error (`LowerResult::errors`), never a
dropped call or an out-of-range index. The obligations and guards are closed formulas (no variable), so Silicon, which assumes a failed assert's condition, stops the path at the
first failure and reports nothing after it; a function whose switches have only literal case values, and a program without
named constants, lower exactly as before.

**Static assertions in the prologue** (M4.4b2, decision 0024 item 2). A static assertion is a `NamedConst` of kind
`StaticAssertion` (`Program::add_static_assertion` for a file- or namespace-scope one, in source order with the named
constants and enumerators; `Function::add_local_static_assertion` for a block-scope one, among the local constants,
before the case values): type `Bool`, `init` the condition as written taken as a truth value, `value` `1` (`0` under
`perturb_folded`). It is lowered like a constant: the condition's obligations at their positions, then the `internal`
guard (message "internal check: static assertion condition as written differs from the front end's (it holds)"),
omitted for a bare `true`. `check_program` requires a closed `Bool` condition and a value of `0` or `1`.

**Locals without an initializer** (M4.2c, decision 0019 items 4–5). A function whose body has a `Decl` without an
initializer gets, for each such local `v_x__N`, a ghost `Bool` local `d_v_x__N` (its flag) and, per loop that assigns
it and does not declare it, a snapshot `ds_v_x__N__K` (`K` counts the function's loops that are not `DoOnce`). The flag
is reset (`d := false; v := 0`, `false` for `bool`, an enumeration's low end only if 0 were outside its range) at the
`Decl`, at the entry of the `Block` that directly holds the `Decl`, and, for a `Decl` directly in a switch body, after
the scrutinee and before the dispatch. Every `Assign` of a flagged local (and a `Decl` with an initializer) sets
`d := true` after the value. Every `Local` read of a flagged local is an obligation `d` (category `uninitialized`,
"`x` may be read here before it is given a value") at the read, under the current guard, like any other obligation.
A loop that assigns a flagged local declared outside it assigns the snapshot before the loop (after a `for` init) and
carries `invariant ds ==> d` (`internal`), after the automatic range invariants and before the user's clauses. A
function without such a local lowers exactly as before.

**Returns.** `return e` assigns `res`, asserts each `ensures` clause (`postcondition`, at the `return`, note at
the clause), and jumps to `l_end`. The end of the body appends `assert false` (`missing-return`) for a
non-`void` function and the `ensures` asserts for a `void` one, then `label l_end`. A function declared not to return
(M4.2a) asserts `false` with category `noreturn` at each `return` and at the end of its body instead, and its method
has `ensures false` (implied by those asserts; a bodyless one, from the C library table or declared here and defined
elsewhere, carries the trusted tag, `libc: …` or `declared noreturn here, defined in another unit`). A call of such a function is an
ordinary call: its `ensures false` ends the path.

**Well-formedness.** Each function with a contract also gets `f__wf`, a method with only the type ranges as
preconditions, that asserts the obligations of each `requires` clause under the clauses before it, and of each
`ensures` under all `requires` and the earlier `ensures`.

**The prelude** (`src/viper/prelude.cpp`) has total functions with bodies only (0022 removed `c_wrap`): `c_pow2(n)` (a
table, at least 1 for every `n`), `c_shr(a, n)`, `c_div`/`c_rem` for signed operands (four explicit cases) and
`c_udiv`/`c_urem` for unsigned ones (Z3 proves the range of these only in that form), and the bitwise functions of 0023
item 2: `c_<op>_N`/`c_u<op>_N` for `op` in `and`, `or`, `xor` and `N` in 32, 64 (flat: a floor-division bit chain
per operand and a weighted sum of per-bit tests) and 128 (two 64-bit halves). These are the only prelude functions with
a postcondition, the range of their type (`ensures lo <= result && result <= hi`), which Silicon proves against the
body and nothing assumes (owner, 2026-10-24; the lint accepts exactly that clause there). Claims that need symbolic bit
identities are not decided by Z3 (the run ends at `--timeout`, exit 3; 0023 item 2).

**Memory (M4.5b1a, decision 0032).** A *cell* (an address-taken local) is a Viper `Ref` local `v_<name>__<id>`
holding the address of a one-slot block; `find_flagged` skips cells, whose 0019 flag is the field `d_<t>`. Each
`Block` that declares cells (and each switch, before its dispatch) allocates them on entry, `v_x := alloc_<t>()`; a
`Decl` with an initializer writes it. Checks, in order, each one atomic (`!(a || b)`), under the operation's guard:

| Operation | Emitted |
|---|---|
| read `x` (a cell) | `perm(x.v_<t>) > none` (`internal`), the flag (`uninitialized`); value `x.v_<t>` |
| write `x` (statement) | `!(perm(x.v_<t>) != write \|\| perm(x.d_<t>) != write)` (`internal`); `x.v_<t> := e`, `x.d_<t> := true` |
| read `*p` | `p != null` (`null-dereference`), `perm(p.v_<t>) > none` (`permission`), the flag (`uninitialized`); value `p.v_<t>` |
| `Store` | the address and the value in one full expression; after `flush` a non-variable address is copied to a `Ref` temporary; `!= null` (skipped after a compound store's own read), write permission on both fields (`permission`), then the field writes |
| use of a pointer value (`==`, `!=`, truth test, copy) | `p != null ==> perm(lvloc(block_of(p)).lv) > none` (`lifetime`), skipped for `&x`, a null constant and a `?:` of only those; after `==`/`!=` the one-past-the-end row (`internal`), skipped when an operand is null |
| scope exit (fall-through at the block's `end_loc`, `return` after its value and before the postconditions) | per cell, in reverse order: `!(perm(v) != write \|\| perm(d) != write \|\| perm(lv) != write)` (`lifetime`), `exhale acc(v) && acc(d) && acc(lvloc(block_of(x)).lv)` |

A pointer-typed `?:` is lowered as an `if` assigning a `Ref` temporary, never a Viper `ite`: Silicon does not accept an
`ite` term as a field receiver (it reports insufficient permission even after `perm == write` asserts). Heap reads are
plain values with obligations: in b1a no call reaches a cell and a cell is written only at statement level (0032 items
9–10), so no snapshot is needed.

**Contracts over memory (M4.5b1b, decision 0037).** Contract lowering has four modes: the method's own clauses, the
`f__wf` method, a call (the callee's clauses over the arguments) and a return (the function's `ensures`). An atom is
`acc` with its ghost amount `amt_<k>` (a `Perm` formal per `requires` atom, bounded `none < amt_k && amt_k < write`
before the user's clauses); `owns(*p)` adds `p != null`, `WF(p)`, `offset_of(p) < len(block_of(p))`, full `acc` of
`v_<t>`/`d_<t>`, the flag and the value range, and the liveness cell at the amount; `reads` the same at the amount;
`live` the liveness cell. `left || atom` becomes `!left ==> atom`. Pointer parameters get `requires WF(p)`, pointer
results `ensures WF(res)`. Each heap read of a contract is guarded by itself: the chain `(D_1 ? p.v : (D_2 ? p.v :
… : 0))` over its framing disjuncts (omitted when trivially framed); `Old(Deref)` is Viper `old(...)` inside it.

`f__wf`: the contract's heap reads are ghost value formals `hv_<param>_pre`/`_post` with their ranges; each read is
asserted framed (`permission`, "the contract of `f` reads `*p` without owning it") under the earlier clauses plus the
added facts (two `owns` of one field in one clause list differ; aliasing places of one state have equal values; a
`reads` place's post value is its pre value); no flag, liveness or pointer-comparison obligations there.

Entry: `lvloc` case splits per atom pair, guarded `owns`/`owns` separation (`internal`), and case splits of each
pointer parameter without an atom against those with one. A call (all items under the call's exact set): each callee
atom part by part under its guard (`permission`), pairwise separation of `owns`/`owns` and `owns`/`reads` (`permission`),
`lvloc` and cell case splits, the amounts `(g_k ? m_k : write) / (N + 1)` inline as call arguments (`m_k` the liveness
cell's permission, for `reads` the minimum with the value and flag cells), the sum checks, the value preconditions
(`precondition`), the call; after a call returning a pointer, case splits of the result against the cells in scope and
the pointer parameters with atoms. A return: separation of the `ensures`' atoms, each atom part by part with the sums
(`permission`), `WF`/`offset < len`/`WF(res)` (`internal`), then the value `ensures` (`postcondition`). `&param`: the
copy is allocated at entry, stored from the formal, and leaves at the body's scope exits. A cell read or written by
name is checked as `permission` (a callee may have kept it); the one-past-the-end row is `pointer-comparison`, in code
only (in a contract `==` is the model's identity).

## 4. Viper shape

```
function c_pow2(n: Int): Int { ... }                      // only the prelude functions the program uses

// C/C++ function `f`
method f_f__0(p_a__0: Int, p_b__1: Int) returns (res: Int)
  requires (-2147483648 <= p_a__0) && (p_a__0 <= 2147483647)     // type ranges, then the user's requires
  ensures  (-2147483648 <= res) && (res <= 2147483647)             // the result's range, then the user's ensures
{
  var v_a__0: Int                                          // every local and temporary, at the top
  var t__0: Int
  v_a__0 := p_a__0                                         // the parameters' local copies (0012 item 2)
  assert <closed obligation>                               // the prologue: named constants, case values, their guards (0017)
  assert <guard ==> obligation>                            // one check per line
  t__0 := f_g__1(args)                                     // a hoisted call
  res := <value>
  assert <an ensures clause>                               // at a return
  goto l_end
  assert false                                             // missing-return
  label l_end
}
// cerastes:trusted-contract <reason>                      // above a bodyless method
method f_g__1(...) returns (res: Int) requires ... ensures ...
method f_f__0__wf(p_a__0: Int, ..., res: Int)              // well-formedness of the contract
```

**Memory prelude** (M4.5b1a; only when the program has a pointer type or a cell, so every other program keeps its
Viper): `field v_<t>` and `field d_<t>: Bool` per cell type, sorted, and `field lv: Bool`; `// cerastes:domain-ok 0028`
and `domain Block` with `loc`, `len`, `block_of`, `offset_of`, `member`, `parent_of`, `index_of`, `lvloc` and the four
axioms, each after `// cerastes:axiom-ok 0033`…`0036` (`lint_viper.py` pins the domain, the names and the texts);
then one trusted method per cell type, `alloc_<t>() returns (r: Ref)`, whose `ensures` are 0032 item 4's (its map
rows at the first cell of that type). `<t>` is `bool`, `schar`/`uchar`, `short`/`ushort`, `int`/`uint`, `long`/`ulong`,
`i128`/`u128` (else `i<bits>`/`u<bits>`), or `enum_<n>_<name>` for an enumeration (`_e` appended when the name would end
in `__<digits>`). One pair per VIR type: `long` and `long long` share one (0032 items 3 and 5 say why that is sound).

Names: every identifier has a prefix that says what it is: functions `f_<name>__<id>` (and `...__wf`), formals
`p_<name>__<k>`, locals `v_<name>__<id>`, temporaries `t__<k>`, the result `res`, labels `l_end`, prelude
functions `c_*`. A character that is not alphanumeric is written `_x<hex>`, `$` as `_D_`.

Silicon assumes an assertion's condition after reporting that it may fail, so later checks on the same path
cannot fail because of the same cause. Tests therefore put one failure on each path. Silicon also reports a
failing `&&` assertion once per failing conjunct (see section 3: obligations are atomic).

## 5. The position map (`<file>.vpr.map`)

One line per check, tab-separated, UTF-8, `\n`-terminated:

```
<vpr line> <TAB> <viper error id> <TAB> <category> <TAB> <file> <TAB> <line> <TAB> <column> <TAB> <message> [<TAB> <note file>:<note line>:<note text>]
```

- The Viper line carries `assert` (id `assert.failed`), a call of a method (`call.precondition`), an
  `invariant` clause (`invariant.not.established` and `invariant.not.preserved`: two entries on one line), or a
  method `requires`/`ensures` (`not.wellformed`; an `ensures` also `postcondition.violated`).
- `<category>` is a driver category (0011 item 10) for a check the user can cause, and `internal` for what only an
  encoder bug can make fail (Silicon's own check of a precondition at a call or of a postcondition, the
  well-formedness of a clause, an automatic range invariant): the driver exits with status 3.
- The two guards of M4.2b1 are `internal` entries on an `assert` line: "the value of `X` as written differs from the
  front end's" (at the constant's declaration) and "case value as written differs from the front end's" (at the case
  label). A failing guard means EDG's folding and cerastes's reading of the same expression disagree: exit 3.
- The driver matches each Silicon error by its `.vpr` line and id; no match is exit 3 (0011 item 11).
- The driver's classifier of Silicon's messages and the harness's (`_VIPER_ERRORS`) are two copies of one
  table pinned to the Silicon version; a self-test runs both on the same recorded outputs and requires
  agreement.

## 6. Linting (`scripts/lint_viper.py`) *(M3-7)*

Fails on: `inhale`/`assume` without `// cerastes:inhale-ok <reason>` (invariant 4); a bodyless method without
`// cerastes:trusted-contract <reason>` (0012 item 1); any `domain`, `axiom` or bodyless `function`; an `assert`,
`requires`, `ensures`, `invariant` or method-call line with no position-map entry; an empty input set.

## 7. Matching IL to VIR *(M3-1, M3-4)*

The IL facts each rule relies on (node kinds, fields, conversions) are recorded with evidence in
`internal/il-inventory.md`, section "M3: scalar code" (probes P1–P12). The ones the encoder depends on most:
- `int s = 0;` is a `stmk_decl` followed by a `stmk_init`: encode the initializer once, at the `stmk_init`.
- `static_assert` in a body is a `stmk_decl` of a static-assertion entry (not encoded).
- `break`/`continue` are compiler-generated `stmk_goto` + `stmk_label`; the label carries `break_label`,
  `continue_label` and, for a switch, `switch_break_label`. No field links a label to its construct: the break label
  is the next sibling of the loop or switch, the continue label the last statement of the loop body (M4.1, P13).
  A user `goto` is a `stmk_goto` without `compiler_generated`, naming its `a_label`; a user label is a `stmk_label`
  covering only `L:`, the labeled statement is its next sibling, and the label's attributes are on the label entity
  (M4.2a, P19). A computed `goto *p` is `stmk_assigned_goto`.
- `assert(e)` on macOS is `eok_question[void](__builtin_expect((long)!e, 0), __assert_rtn(…), (void)0)`, with a
  `bool` conversion around the condition in C++; every node is positioned at the macro invocation (M4.2a, P22).
- `do` is `stmk_end_test_while` (`expr`, `loop_statement`); `switch` is `stmk_switch`, its case labels flat
  siblings (`stmk_switch_case`) in the body block, each with `case_value` (already converted to the promoted
  scrutinee's type) and `range_end` (a GNU range); a C++ init-statement or condition declaration is an
  `enk_condition` (M4.1, P14–P16).
- A `for` init is a compiler-generated block; `if (int x = …)` has an `enk_condition` node (rejected).
- Reaching the closing brace of a non-`void` function is a compiler-generated `stmk_return` without an
  expression (`main`: with the constant 0).
- Calls: operand 1 is the resolved `enk_routine`; each argument is wrapped in a compiler-generated conversion
  to its parameter type; default arguments are copies whose top node has `generated_default_arg` and whose
  positions are in the callee's declaration.
- Parameters: contracts use `enk_param_ref` (`{k,0}` in `requires`, `{k,1}` in `ensures`, the result binder
  `{1,0}`); bodies and statement annotations use `enk_variable`, linked to parameter `k` by
  `variable.assoc_param_type`.
- Named constants (M4.2b1, P23–P24): a constant-initialized file-scope variable is `initk_static` with its value in
  `initializer.constant`; every use of it, in both modes, is a folded constant whose `expr` chain ends at the
  `enk_variable`; C's tentative `static const int z;` is not folded (its uses are variable reads); a local
  `const int k = 5;` is `initk_dynamic` with its value in the `stmk_init`'s `dik_constant`; a case value naming a
  constant is a constant of the promoted scrutinee type whose `expr` chain reaches the variable; weak constants and
  functions carry `is_weak`/`is_weakref` (`#pragma weak` leaves no attribute entry).
- Enumerations (M4.2b2, P25–P27): an enumeration is a `tk_integer` with `enum_type`, one type entry per enumeration;
  its enumerators are the `constant_list` (unscoped) or the `constants` of `assoc_scope` (scoped), each flagged
  `is_named_constant_definition`, and every reference is an `enk_constant` naming that very entry (also inside a folded
  constant's `expr` chain). An enumerator's definition constant has an `expr` only for a non-literal initializer;
  `enum_value_range` tells a literal initializer from an implicit value. In C an enumerator of an enumeration without a
  fixed type has type `int` (its own type entry, `affiliated_type`), elsewhere the enumeration. C++ promotes every
  enumeration operand with a compiler-generated cast (to `int` for an unfixed one); C converts the operands of `==`,
  `<` and arithmetic to the compatible type but leaves the switch scrutinee, conditions, the operands of unary `-`,
  `~` and shifts, and the left operand of `++`/`--`/compound assignment unconverted. The front end makes unflagged
  copies of an enumerator's value without `expr` in a few places (C++ `color d = GREEN;`, a scoped case value).
  `==`/`!=` between a value of an enumeration compared in an unsigned type in C (no negative values; unfixed, or a fixed
  unsigned base of rank `int` or above) and a signed non-enumeration operand that is not a constant ≥ 0 is rejected in both
  modes, like the compilers' `-Wsign-compare`; so is `==`/`!=` between two different enumerations (`-Wenum-compare`). A truth
  value (a comparison, `!`, `&&`, `||`, a `bool` node, a `?:` of truth values, a comma whose right operand is one; never
  an explicit cast or an `int` constant) counts as non-negative (0022 item 5, P39; 0023 item 7). In C, a `?:` whose arms
  are enumerators of one enumeration, as an operand of `==`/`!=`, is rejected as C++ rejects the same `?:` (0023 item 7).

**Positions of obligations.** An operator's obligation is reported at the operator node's `position` (the
operator token); a call's at its `position` (the `(`); a conversion's at the conversion node, or at the
enclosing initializer, assignment, argument or `return` when the conversion is compiler-generated; an
obligation that comes from a default argument at the call, with a note at the default argument. Inside a
`CERASTES_*` argument the original position (`orig_seq`/`orig_column`) is used (0006 item 10).

## 8. The encoder (`src/backend/encode.cpp`, M3-4..M3-6)

`verify_encode()` runs from `back_end()` in verify mode when the annotation checks found nothing, in this order:
(1) `#line`/linemarker rejection; (2) the declarations of verified files; (3) every routine of a verified file;
(4) the bodies; (5) `check_program`, `lower_program`, `emit`. Only when nothing was rejected does a program exist.

**What is verified.** A file is verified unless its canonical physical path is under a system directory or is exactly
`include/cerastes.h` (0011 item 4). A file that cannot be placed is verified (`in_verified_file`, `is_system_file`):
fail closed. Positions of nodes are mapped through `diag_position` (0006 item 10).

**Declarations (rejected with `unsupported_declaration`, at the declaration's first line).** In a verified file: a
class, struct or union (an enumeration: below; a lambda's closure class is reported at the lambda expression instead); a global
variable that is not a named constant (below); a weak function (`is_weak`/`is_weakref`, 0017 item 2); a file-scope
`asm`; a template; a `constexpr` or `consteval`
function; a variadic function; a function without a prototype; a `static`, unnamed-namespace or `inline` function
that is declared (with a contract, say) and not defined in the file (no other unit can define it, so its contract
would be trusted for nothing); locals that are `static`, `extern` or `thread_local`, arrays and VLAs, a `const` local
without an initializer, a local used in its own initializer. Since M4.2c a local without an initializer
(`initk_none`, no `stmk_init`) is a `Decl` without one (decision 0019). **A body written in a verified file that this encoder does not
encode is rejected, never skipped** (M3 pre-commit audit): the body of a lambda, of a compiler-generated function,
of an explicit specialization or other body for a template declared elsewhere, or of an out-of-line member of a class
declared elsewhere (`template <> int std::gcd<int, int>(...) { ... }`, `std::exception::what()`); a template or a
class written in the file is rejected at its declaration instead. Attributes written on a type of a verified file
(a typedef, the unnamed type of `int __attribute__((x)) f(int)`) are held to the allow-list, wherever the type is
recorded; `vector_size` and its relatives are exempt there (the vector type they make is rejected where it is used). `unsupported_type` (a pointer, reference, float,
`volatile`, `_Atomic`, `_BitInt`, vector or system enumeration type) is reported at the declaration that mentions the type
(the function for a parameter or result, the variable for a local). A call of `main` is rejected in both modes.

**Enumerations (decision 0018).** `enum_info` makes one VIR type per enumeration type entry (`Program::enum_type`:
bits and signedness of the IL type, the value range of 0018 item 3) and rejects at the declaration (at the enumerator
for a problem of one enumerator) an enumeration in a block, a class or a template, a `bool` base, an enumerator
outside `int`, an implicit enumerator value that is not the previous one plus one, an incomplete enumeration without
a fixed type, and attributes outside the allow-list; a system enumeration is rejected at its use. Enumerators with a
written initializer join the named constants of 0017 (`encode_named_enumerator`, in source order with the variables),
so their initializers are checked in every method's prologue. A use of an enumerator (`is_enumerator`) is a literal of
its IL type with the front end's value, read before the constant's `expr`, and only if its enumeration passed
`enum_info`; an `enk_constant` node whose type differs from its enumerator's adds the implicit conversion in C++
(definition-time types) and is rejected in C, and a C enumerator's initializer may use `sizeof`/`_Alignof` only of a
plain arithmetic type and no `typeof` (`plain_arithmetic_type`, `typeof_or_sizeof_of_expression`: EDG types an earlier
enumerator the C++ way there). A named constant of an enumeration whose
front-end value is outside the value range (its initializer's conversion fails in the prologue) is used as the
conversion of a literal of the underlying type (`value_literal`), and `check_program` checks such front-end values
against the underlying type (`front_end_value_fits`). A C switch over an enumeration of rank at most `int` without a
promotion node is encoded in the promoted type (`enum_case_value`): the scrutinee converted, each case constant
converted to the promoted type (C23 6.8.5.3), not into the enumeration. `conversion()` checks every
conversion into an enumeration (the lowering's `conversion` obligation over the value range), rejects one from `bool`,
and accepts an implicit conversion out of an enumeration only on a node its parent marked (`allow_enum_conversion`:
the operands of `==`/`!=`, the switch scrutinee, a whole initializer, the right side of `=`, an argument, a returned
value) or whose operand is an enumerator. Arithmetic, shifts and unary operators on a raw enumeration operand (C's
unconverted operands), a comparison of an enumeration with another type, an ordering of an unscoped enumeration, a
`?:` of enumeration type, and compound assignment and `++`/`--` on or with an enumeration are rejected at the
operation. `names_object` stops at an enumerator. A
rejected local is remembered: its later uses
are not diagnosed again, and after a VLA the compiler's own cleanup statements are not either (A6). One diagnostic is
reported per (tag, line).

**Routines.** Every routine of a verified file is checked for attributes and signature (a variadic function, a
function without a prototype, a `main` declared not to return, a parameter or result of a type with no VIR meaning;
a routine whose type says it does not return is `Function::noreturn`, M4.2a); every one
with a body becomes a VIR function with a body, created before any body is encoded so that calls and recursion find it.
Routines that are templates, instances of templates, members of classes, compiler-generated or lambda bodies are not
encoded (the template or class declaration is what is rejected); a call of a template instance is rejected. A callee without one is
a trusted contract when a declaration carries a `requires` or `ensures` or the function is declared not to return
(M4.2a), or when it is one of the C library functions of 0015 item 5 (`abort`, `exit`, `_Exit`: name, system file,
C linkage, signature and the "does not return" flag all checked), else `call_without_contract`; its reason is
"declared here, defined in another unit" or "trusted, never verified: declared in a system file". `predicate`/`pure`
routines and the spec functions are skipped (their bodies are not encoded); a call of one from code is
`spec_call_in_code`. `requires` on `main` is `contract_on_main`. Attributes are held to the allow-list of 0011 item 7
on routines, parameters, locals and statements of verified code (`unsupported_attribute`, `unrecognized_attribute`).

**Named constants** (0017). `encode_named_constants` runs after the declaration checks and classifies each file-scope
variable of a verified file, in source order: `const` (or `constexpr`), a value type, not `thread_local`, not weak,
and `initk_static` with an integer constant (or, in C, `static` with no initializer: value 0) makes a named constant,
whose initializer is encoded through the folded-constant path (original expression, `const_depth`, calls rejected)
and added with `Program::add_named_const` (EDG's value as `value`); anything else is rejected at its declaration
(mutable, `extern` without a definition, a C tentative definition with external linkage, dynamic initialization,
`thread_local`, weak, a static data member, an initializer of another type, a constant named in its own initializer;
`volatile` and other types through `vtype`). `variable()` looks the variable up in that table first, in every mode
and context (contracts included), and gives a literal of EDG's value; a file-scope variable of a system file is
rejected at its use. While a C++ case value is encoded, a `const`/`constexpr` local whose declaration recorded a
constant initializer becomes a function local constant once (`Function::add_local_const`), its initializer encoded
the same way; the case label carries EDG's `case_value` as `folded`. Verify mode passes `--diag_error=1901` in C,
so a C constant expression that names an object never reaches the encoder (its own C case-value rejection is an
unreached backstop). In both modes it passes `--diag_error=1422,1654,26,3255` (0023 item 8): a multi-character
character literal, or one whose value EDG truncates, refuses the file before the encoder sees its value.

**Contracts.** A `requires` or `ensures` argument is encoded in contract mode: `enk_param_ref` is `Formal(k)` (or the
result binder), a named constant is its value, any other variable reference is rejected, a call is `call_in_annotation` except `cerastes::old`, which the IL
shows as a dereference of a call whose argument is a reference to a parameter reference and is encoded as `Old`. The
same mode applies to the arguments of `check` and `invariant` (code mode, but calls are `call_in_annotation`).

**Positions.** In code a node is reported at its own position: for a token of a user macro's body that is the line of the
expansion (the programmer's line), not the `#define`; inside a `requires`, `ensures`, `check`, `invariant` or `decreases`
the original position of the argument token of the `CERASTES_*` macro is used (0006 item 10). The rejection of a loop
that has attributes in C++ (whose statement position starts at them) is reported at the loop's keyword, found in the
source text.

**Constants** (0010 item 12). An `enk_constant` of kind integer is encoded by its original expression
(`expr_node_from_constant`) when it has one: operations and conversions as written, a reference to a local `const`
variable as that local, a named constant as its value, `sizeof`/`alignof` by value; a call in it (a compile-time
evaluated `constexpr` call), another global and an enumerator are rejected; a constant with `orig_type` and no
recorded expression is rejected. A plain literal is
its exact decimal value (`decimal_str_for_integer_constant`).

**Expressions.** `+ - * / %` and shifts map to the VIR operators; comparisons and `&& || !` produce `Bool` and are
wrapped with `bool_to_int` when the node's type is `int` (C); an integer in a boolean position is read as `e != 0`;
casts map to `convert`, `to_bool` or `bool_to_int`; `?:` to `cond`; a call to `call`, a default argument wrapped in
`report_at` at the call. After building each operation its VIR type is compared with the node's EDG type, so a
conversion the IL left implicit fails closed. Since M4.3a (0020) an assignment, compound assignment, `++` or `--`
used as an operand becomes `assign_expr`/`compound_assign_expr`/`inc_dec_expr` (the same checks as at statement level:
a local target, the `bool` and enumeration rules, the own-initializer rule) and a comma whose value is used `comma`;
an IL node with a side effect is never encoded twice (fail closed); `vir::sequence_problems` is reported as
`unsequenced_modification` at the write; and the warning rules of 0020 item 5 (`assignment_as_condition`,
`comma_without_effect`, `value_not_used`, `side_effect_not_evaluated`) are IL checks, and so (M4.3b, 0021 item 8) is
`self_assignment`: an `eok_assign` (and, M4.4b1, an `&=` or `|=`) whose operands, after compiler-generated casts, are the
same variable, with no node from a macro expansion (`orig_seq`/`orig_column` equal to `seq`/`column`), as clang's
`-Wself-assign`. The bitwise warning rules of 0023 item 5 (`bitwise_operand_parentheses`, `logical_not_bitwise`,
`complement_of_truth_value`, `bitwise_condition_parentheses`, `bitwise_instead_of_logical`,
`tautological_bitwise_compare`, `xor_as_power`) run where their operator is encoded, also on a folded constant's `expr`
and in contracts, and `assignment_as_condition` also matches an `|=` condition. The other `-Wall` shapes of 0025 item 6
(`logical_operand_parentheses`, `shift_operand_parentheses`, `self_comparison`, `logical_not_comparison`,
`chained_comparison`, `truth_value_compare`, `int_in_truth_context`, `constant_logical_operand`,
`constant_out_of_range_compare`, `overlapping_comparisons` (code only); two enumerations in arithmetic or `?:` as
`unsupported_expression`) run the same way, and `dangling_else` and `empty_body` on the `if`/`while`/`for`/`switch`
statements (written versus compiler-generated blocks, P50b): in `logical`, `binary_arith`, `comparison`, the `?:` arm, and
`truth_context` (every truth context: conditions, `!`, `&&`/`||` operands, conversions to `bool`, annotation, `assert`
and static-assertion conditions). A `void` `?:` in statement position (an expression statement, a discarded comma
operand, a `for` step) is encoded as an `If` whose arms are expression statements in blocks (0025 item 3). Lowering's rejections
(`LowerResult::rejections`, 0021 item 3) are reported as `unsequenced_calls`. Everything else is
`unsupported_expression` with a description of the operator (pointer and member operations).

**Statements.** Allow-list: blocks (compiler-generated blocks are flattened, so a `for` initialization stays in
scope), declarations with their `stmk_init`, expression statements (assignment, compound assignment and `++`/`--` of
a local, calls, discarded expressions, `(void)e`; a discarded comma, also in a `for` init-statement or increment,
becomes the statements of its operands in order, 0020 item 6), `if`, `while`, `do` and `for` (the increment is the loop's
`step`, run after the body and after a `continue`) with their invariants read from the loop's attributes, `break` and
`continue` (the compiler-generated `goto`s, mapped to the innermost enclosing loop or switch, with the label's shape
checked, decision 0014), `switch` (case labels directly in its body), `return`, `static_assert`, null statements
(`[[fallthrough]]` among them) and `check`. A `do` whose condition is a constant zero (EDG's folded value) that names
no variable or parameter, following every constant's `expr` (`names_object`, 0017 item 7), is the loop form `DoOnce`. A compiler-generated `return` without a value ends a function and is not encoded (lowering
appends the epilogue); the generated `return 0` of `main` is encoded at the closing brace. A user `goto` and label
become `Goto`/`Label` (one id per `a_label`); the label entity's attributes go through the allow-list; after the
function is built, each `vir::goto_problems` entry is reported as `unsupported_statement` at the `goto` (M4.2a). The
`assert` expansion is matched by its exact shape and becomes `Check(e, from_assert)` (0015 item 6). A computed `goto`,
`try`, `asm`, range-based `for`, `if constexpr`/`consteval`, conditions with declarations, and the forms of `switch`
that 0014 item 1 rejects (nested labels, GNU ranges, a statement before the first label, a declaration a later label
jumps over, an init-statement) are `unsupported_statement`; verify mode suppresses EDG's warning #546 for the jump
over a declaration, which cerastes reports itself.

**Termination** (decision 0013). `decreases` is read as a contract of the function (`Function::decreases_`, the
measure over the formals) and before a loop (`Stmt::measure`); each component must be an integer expression. A loop
without a measure is `missing_decreases`, at its keyword. The encoder keeps the call graph of the verified functions
(an edge for every call of a function with a body in a verified file) and rejects a function that can reach itself
without a measure (`missing_decreases`, at its definition, once per function of the cycle). VIR requires a measure on
every loop (`check_program`). Lowering: a function's measure is the method's `decreases e1, ...` over its formals
(empty for a function with none; `decreases _` on a bodyless method, which the trusted-contract tag explains); its own
obligations are asserted in the `__wf` method under the `requires`. A loop's measure is the loop's one-line `decreases`
clause, placed after its invariants; its obligations are invariant clauses at the loop head, after the user invariants
and before the condition's. A call of a function that has a measure carries a `termination.failed` map entry (category
`termination`, a note at the callee's measure); a loop's `decreases` line has one (at the measure's line). Every method
and every loop has a `decreases` line (Silicon checks termination only where one is present); since M4.3b a call that
another unsequenced call may precede also gets cerastes's own `termination` assert (§3, 0021 item 4); a method's non-empty
`decreases` line also has an entry, which `lint_viper.py` requires. Silicon's semantics, measured on the pinned build: the
measure must decrease and be bounded (new < old and old >= 0 at the first component that changes) at every iteration
that is followed by another one; the iteration after which the loop exits may have a negative measure.

**Counting and output.** `n` of the verdict line is the number of verified function bodies (0011 item 9; trusted
contracts are not counted); with none, the driver says "nothing to verify" and exits 2. The map entries
and the Viper come from `emit`. A map entry without a C/C++ position, an internal error of `lower_program`, or the
encoder's "encoded twice" guard is reported as `internal_inconsistency` ("an internal inconsistency: …"), and the run
exits 3 (0022 item 12): it is an encoder bug, a tool failure, never a verdict, and never silently clamped or dropped.
A program that `check_program` refuses is a rejection (exit 2), since a program can reach it.

**Self-test hooks** (0011 item 13): `drop_assert[=N]`, `weaken_invariant`, `constant_without_expr` (every constant with
an original expression is treated as having none: rejected), `perturb_folded` (every named constant's value and every
case label's `folded` moved to a neighbouring value, so the prologue's internal guards fail: exit 3), `silicon_timeout`,
`abort`, `internal_error`, `perturb_fixed_order` (lowering's call order perturbed, so its fail-closed check reports an
internal error: exit 3, 0022 item 12), `drop_block_static_assertion` (block-scope static assertions ignored by the
encoder, so the cross-check of each encoded body's region rejects them: exit 2, 0025 item 5), `force_base_sets` (every
full expression takes step 2 of 0026, today's sets: the Viper of the previous version, and its false alarms). The VIR
self-test has its own lowering hooks, among them `perturb_write_slot` (0025 item 2) and `force_base_sets`.

**Values the front end computes (decision 0024).** *Static assertions:* file-, namespace-, `extern "C"`- and
class-scope ones come from the file scope's source sequence list (`collect_entities`, `IlEntities::static_assertions`;
`GENERATE_SOURCE_SEQUENCE_LISTS` is on), filtered by `static_assertion_in_verified_file` (the keyword's or the
condition's position; none counts as verified), and join `encode_named_constants`'s items by `seq`
(`encode_file_static_assertion` → `Program::add_static_assertion`); block-scope ones replace the old skip of the
`stmk_decl` entity (`encode_block_static_assertion`, the case-value path: `case_value` flag, the C `named_var` and
`variable_read_by` rejections, `constant_initializer`, `clause_bool` → `Function::add_local_static_assertion`). A lone
literal or enumerator adds nothing; a NULL condition is rejected; a `ck_template_param` one is left alone; any other
static assertion of a verified file that the walk reaches is rejected (the cross-check). Since M4.4c (0025 item 5) the
walk of each routine region also lists its block-scope assertions (`IlEntities::region_static_assertions`); after the
bodies are encoded, and only when nothing was rejected, each one in a verified file must have reached
`encode_block_static_assertion` (`check_block_static_assertions`), except in the region of a `predicate` or `pure`
routine. Before encoding, `scan_for_skip`
looks for Q39's triggers (a call of a system or position-less routine, `enk_builtin_operation`, `enk_concept_id`, a
system variable or enumerator), not entering `sizeof`/`_Alignof`/`__datasizeof`/`noexcept` operands, and applies the
type rule to every type operand and template argument it meets, skipped or not. *Wrapped character literals:*
`constant_value`'s literal arm sends a negative constant without `expr` or `orig_type` typed plain `int` (C) or
`char`/`wchar_t` (C++) to `wrapped_character_literal` (`convert(char|wchar_t, (unsigned N-bit) v + 2ᴺ)`, then to `int`
in C); `encode_named_enumerator` does the same for a written initializer without `expr`, the character type from
`character_kind`. *Types:* `computed_type_reason` (arrays, `_BitInt`, vectors, class instances with a `tak_nontype`
argument, through typerefs, pointers and routine types) runs on `sizeof`/`_Alignof`/`__datasizeof` operands
(`reject_computed_sizeof_operand`), on the C++ top-level `sizeof` enumerator, and on array typedefs of verified files
(`declaration_level_checks`); `vtype` rejects a typeref chain through an alias-template instance or a member typedef of a
class instance with a non-type argument (`typeref_chain_reason`).

**Known over-rejection.** A call written inside an annotation's own `sizeof` operand (`CERASTES_REQUIRES(sizeof(int[p(2)]) > 0)`)
is rejected as a call in an annotation, although the operand is never evaluated: the encoder reads the folded constant's
original expression, which contains the call. This is the safe direction (M8 inventory).
