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
- **Pure functions (M4.5e1d, record 0062).** `Function::pure` marks a `pure` function the encoder read; it has no statement `body` and no `trusted_reason`, so it is neither
  a verified method nor a trusted contract (`verified()` is `!pure && body != nullptr`). `Function::pure_body` is its value `E`: one Int or Bool expression over `Formal`s, a pre-state
  contract expression, with the front end's generated conversion at the `return` as a `Convert` node (its obligation is the `conversion` row of the function's `__wf`);
  `requires_`, `ensures_` and `decreases_` are its contract and measure. `Expr::Kind::Apply` is an **application** of such a function: `callee`, the encoded `args` (already of the parameter
  types, each a site expression, evaluated once in the site's state) and `c_text`, the site's spelling (`sum_to(a, n)`). It is not a `Call` (the code call lowering hoists as a statement): it is an
  expression of a contract, a `check` or a `pure` body. **`reads_heap` is true for it** whatever its callee reads, so every rule that asks for "reads no memory" (an atom's count or guard, an
  element's index, a `forall`'s trailing argument) refuses an application by itself; `guard_ok` is false for it; `same_pure_expr` never calls two applications equal; `expr_touches` is true.
  `Builder::apply(callee, args, text, loc)` builds one. `check.cpp` (`apply_node`, `pure_body_rules`, `pure_contract_rules`, `application_graph`) admits an `Apply` in a `check`, in the body,
  `requires` and `ensures` of a `pure` function and (Part C) in an ordinary function's `requires` and `ensures`; never in a measure, under `Old` or under a `Forall`'s binder (a loop invariant admits it since Part D). It checks the callee (`pure` with a value expression), arity, parameter and result types, that a pointer an atom of the callee names is a `Formal` (a contract, a
  `pure` body) or a pointer `Local` that is not address-taken or the `AddrOf` of a named array (a `check`), that a `pure_body` holds no `Old`, atom, quantifier, write or call, that the atoms of a
  `pure` function's `requires` are `reads`/`reads_range` of a `Formal` as top-level conjuncts, and that **the graph of applications over bodies and contracts has no cycle but a function's own
  application of itself in its body** (lowering has the same backstop over the emitted functions, as `check_cycles` has for methods).
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
- **Arrays (M4.5e1a, decision 0059).** `Type::Kind::Array`, `Expr::Kind::ElemAddr`, `Var::zero_filled` and `Stmt::keyword_loc` (§3, "Arrays"): the
  array local, the element address and the zero fill, and a loop's keyword position for the rows that must report one line in both modes.
- **Ranges (M4.5e1b, decision 0060).** `AtomKind::OwnsRange`/`ReadsRange` (count in `Expr::b`), `Builder::range_atom`, `same_pure_expr`, `atom_partners` and the contract read
  `Deref(ElemAddr(Formal, index))` (§3, "Ranges in contracts").
- **Quantifiers (M4.5e1c, record 0061).** `Expr::Kind::Forall`, `Bound` and `ForallArg` (`bound_id`, `bound_type`, `arg_index`, `forall_name`; `Builder::forall`, `bound`,
  `forall_arg`; `LExpr::Forall` with several binders and trigger sets, §3 "Quantifiers"). The spec function is C's `cerastes_forall(f, args...)`, declared in
  `<cerastes.h>` in both modes (C23 variadic; `bool` and `noexcept` inside `extern "C"` in C++) and named in the name lists (`SpecFn::forall`, `il_query.cpp`,
  `annotation_checks.cpp`, `edg_args.cpp`); **cut 3:** `cerastes::forall`, with a lambda or a function, is `unsupported_annotation` ("write `cerastes_forall`", M7) in every
  position of verify mode, because P80 (b) found that an rvalue trailing argument reaches the IL as an `enk_temp_init` the annotation reader refuses, so no single rule reads
  every argument of the namespace spelling. `Forall::a` is the body (Bool); `args` are the encoded trailing arguments in the parameter order of `f` after the bound variable;
  the body reads a parameter of `f` as `Bound` (the first) or `ForallArg(k)`.
- **Contracts over memory (M4.5b1b, decision 0037).** Pointer `Param`s and a pointer `Function::ret` (P58: no
  separate copy in the IL; `Param::is_restrict` from the parameter variable's type, C only; set only so that `check.cpp` can refuse it: every `restrict` parameter is rejected, record 0064 item 19). `Expr::Kind::Atom`
  (`AtomKind` `Owns`/`Reads`/`Live`, `a` = the pointer `Formal`, `amount` = the index of the `requires` atom, 0..N-1 in
  evaluation order; an `ensures` atom carries its `requires` partner's `amount`, same kind and parameter) appears only in
  `requires`/`ensures`; `contract_atoms(f)` lists the atoms with their guards (`AtomGuard {cond, holds}`: the left of an
  `||` with `holds == false`, a `?:` condition either way; `&&` adds none), `requires_atom_count(f)` gives N,
  `reads_heap(e)` tells whether an expression holds a `Deref` or an `Atom`. Heap reads in a contract are `Deref(Formal
  p)` (pre-state in a `requires`, post-state in an `ensures`) and `Old(Deref(Formal p))`. `check.cpp` splits `no_ptr_`:
  contracts may hold pointer `Formal`/`Result`, `Null`, pointer `Cond`, `==`/`!=`, `ToBool`, `Deref(Formal)`, atoms
  and `Old`; invariants, measures, named constants and case values still hold none. Backstops: atom positions, numbering
  and partners, duplicates, a `restrict` parameter (refused outright, 0064 item 19), and `placement_problems(f)` (0037 item 8: a call of
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
- **Arrays in structures and in static storage (M4.5e1e1, decision 0063).** `Type::Kind::Array` is now also the type of a `Global` and of a `StructMember`; an array still has no value:
  its address (`AddrOf` of a local, `GlobalAddr` of a global array, `MemberAddr` of an array member, a `Ptr(Array)`) is the base of an `ElemAddr` and the operand of a whole-array
  atom, and nowhere else. **Three views of a structure's leaves** (`vir.h`): `struct_object_leaves` (a cell leaf for each scalar member, one *array leaf* for each array member:
  `StructLeaf::array`, `element`, `count`), `struct_value_leaves` (every element of an array member is a leaf of its own, `StructLeaf::elem` the index, `text` `s.a[2]`; the tuple of a
  structure *value*) and `struct_leaves` (the old view: an array member is one leaf of array type with `array` false; **the poison**: lowering's guard `leaves_of` and every walk that
  still treats a leaf as a cell report an internal error on it, so a site not yet given a view stops instead of mistaking an array for a cell). For a structure without an array member
  the three lists are equal, so no existing program's Viper changed (`vprdiff` proves it after every part). `Global::elems`/`rest` and `GlobalLeaf::elems`/`rest` (`ArrayRest`: Zero or
  Unknown) carry an array's initial values (the listed prefix and the rest). New statements and expressions, built by the Builder and checked by `check.cpp`: `Stmt::Kind::ArrayZero`
  (C's zero fill of the elements a list does not write, the array member's address), `Stmt::Kind::ArrayCopy` (the copy of one array member into another of the same type: both
  operands are `MemberAddr`s of array type) and `Expr::Kind::ElemValue` (`a.m[b]`: element `b` of array member `index` of a structure *value* `a`, a call, an aggregate, a `Field`
  chain, or in a contract a `Formal`/`Result`; `b` any Int expression, the bound an obligation of lowering). An `Aggregate` of a structure with an array member has one argument per
  value-view leaf. `check.cpp` refuses a structure value with more than `kMaxByValueArrayElements` array elements (`vir.h`; the encoder rejects it first, question 295, which measured Silicon: a larger tuple does not verify in time).
- **Leaf paths and the poison rule for aggregate elements (M4.5e1e2 Part A, decision 0064 item 4; behaviour-neutral: no existing program's Viper or map changed, `vprdiff` shows it).**
  `Type::Array`'s element may be a `Struct` or an `Array` in VIR (no gate lifted by Part A: `check.cpp` still refuses it everywhere and the encoder rejects it first). **`object_leaf_paths(type,
  root_text, arrow)`** (`vir.h`) is the general view that the three views above are special cases of: a `LeafPath` is the *steps* to one leaf (`LeafStep::Kind::Member` with the member's index, or
  `Kind::Index` with the array's `count`), the leaf's cell `type` (Bool, Int or Ptr), its `text` (an index step followed by more steps is written `[]`, the last one is not: `v[].a`, `m[]`)
  and `owning`/`ref_to_uninit`; **one path per leaf whatever the counts**, and `cells()` (the product of the index counts), `indexes()` and `members()`. For a type 0063 accepted it is
  `struct_object_leaves`' list (a cell: member steps only; an array leaf: member steps and one index step), which `check_leaf_paths` (self-test) compares. **The three views stay as 0063 made
  them and gain one flag:** a member that is an array of structures or of arrays is ONE leaf of the array's type with `StructLeaf::aggregate_element` set (the object view also keeps `array`
  and `element`; the value view no longer expands it), so no consumer can read an aggregate element as a cell. **The poison rule's guards** (every one an internal error, exit 3, with a raw-lowering
  case in `selftest_cases35.inc`): lowering's `checked_object_leaves` and `checked_value_leaves` (every former view call) report the flag; `elem_cell_suffix` and `element_suffix` report an
  aggregate element at every site that names one cell field per element (the loop's reach tests, `arr_disjoint`, `site_fields`, `array_alloc_method`); `element_range` and `elem_fkey` report a range over
  structures or rows (and `fkey` of an array, a former null read); `check_flat_elem_base` refuses an element address whose base is the address of a row (an `ElemAddr`, or any pointer to an array
  that is not an array local, a global array or an array member) in `elem_addr`, `inv_elem` and `lb_chain`; `elem_base_kind_ok` refuses an element-read base of a contract that is none of `Formal`,
  `GlobalAddr`, `MemberAddr` in `elem_base_key`, `same_elem_base` and `wf_method`'s tie. `leaf_count`, `leaf_span`, `has_pointer_leaf`, `value_array_elems` and `may_hold_pointer_to`
  descend (the product of the counts; a pointer anywhere inside counts). `check.cpp`'s nest walkers (`contains_by_value`, `owning_reaches`, `struct_has_marked`) descend through arrays, and its
  backstop gained three refusals (a partnerless `ensures` atom on a structure with an array member or on an array member, a written `owns`/`reads(*q)` of such a structure in a loop invariant,
  `owns(*p->q)` through a heap read). Places are spelled as written: `member_place` over an `ElemAddr` gives `v[i].x` and `p[i].x`, `elem_place` over a row `m[i][j]`. A part that gives a
  site its rows removes that site's guard and keeps its case as a positive one. The encoder's helper predicates for the part's rejections (`vir_aggregate_has_marked`,
  `vir_element_has_array_member`, `edg_index_depth`, `declared_array_dimensions`) have no caller until Part B.

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
has `ensures false` (implied by those asserts; a bodyless one declared here and defined elsewhere carries the trusted
tag `declared noreturn here, defined in another unit`). The C library's `abort`, `exit` and `_Exit` are not
`Function::noreturn` (M4.5d1, 0046 item 5): their `ensures false` is vlibc's own `ensures(false)` clause, and their tag
is vlibc's (`vlibc: abort never returns (C23 7.24.4.1)`). A call of such a function is an
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

**Loops and jumps over memory (M4.5b1c1, decision 0038).** `loop()` computes the loop's places and writes (`LoopWrites`:
the cell fields a store by name, a store through a pointer, or a call whose `requires` owns a pointer of that field
reaches, over the condition, body, step and every nested statement). The places are the `requires` atoms' places of
the pointer parameters the function never assigns (value and flag cells for `owns`/`reads`, one liveness cell per
parameter, through the formal) and every cell of `fn_.scopes`, `&param` copies included. When the loop touches memory,
or the function holds a place and the body has a `return` or a `goto` out of the loop, lowering writes right before the
Viper `while`: case splits of each parameter place against every other parameter with an atom of its field (value and
liveness cell), then the ghost snapshots `kv_<n>`, `kd_<n>`, `kl_<n>` (`perm(...)`), `ds_<n>` (under `kd > none`) and
`vs_<n>` (under `kv > none`), then the entry framing asserts. The automatic invariants (all `internal` rows) come first:
`acc(v, kv) && acc(d, kd)` and the liveness `acc` under the de-duplication guards (parameter places of one cell suffix
by `Ref`; liveness cells over all parameters with atoms), then per place the value (`kv > none ==> v == vs` when no write
reaches its field, else `none < kv && kv < write ==> v == vs` and the flag-guarded range), then the flag (`kd > none &&
ds ==> d`). A condition that touches memory takes the `top_test` shape. The body starts with the case splits `if (p ==
X) { }` of every pointer variable the loop assigns against the places of its field (repeated after the loop); it ends,
after the `continue` label, the step and a `do` loop's exit test, with one `permission` assert per place (`perm(v) >= kv
&& perm(d) >= kd && perm(lv) >= kl` in De Morgan form) at the body's closing brace, then the framing asserts. Invariants
and loop measures are lowered in `CMode::Inv`: `==` is identity, no liveness or flag obligation, a cell read by name is
`(kv > none ? x.v : 0)` and `*q` a chain over the places of its field; each read's framing `prior && guard ==> OR_i (q ==
place_i && kv_i > none)` is asserted after the entry snapshots and at the back edge (`permission`). A loop that touches
no memory, and has no `return`/outward `goto` while the function holds a place, lowers exactly as before.

Jumps: `leave_to()` writes the scope exits of every block a `break`, `continue` or `goto` leaves, innermost first, at the
jump; `Fn::jump_depth` records the scope depth at each loop (a `continue` through a `switch` also leaves the switch
body's block) and inside each switch body (a switch `break` lands before the body's own exit), and `label_depths()`
the depth of each user label. A `return` exits every open scope as before.

**Sequencing and placement over memory (M4.5b1c2, decision 0039).** A cell written inside an expression is an
`AssignE` on an address-taken `Var` or a `StoreE` (`*c := a`; with `yields_old` its value is the `Deref` `b` of the shared
address `c`); `cell_write` lowers either as a write effect: the null check (`StoreE`), `perm == write` on both fields
(`permission`), then `r.v_T := value; r.d_T := true`, after every fixed point of its operands in both halves of its set.
Each access has a key (`key_of_cell`, `key_of_addr`: the cell's variable, `*v` per pointer variable, `*&x` as `x`, a key
of its own for any other address); `check_variable_order` runs per key. `mem_scan` collects the fields a full expression
writes (writes inside it, never the statement's own store) and the fields of its memory calls (`memory_fields`: a callee
whose `requires` owns or reads a pointer of that field); `needs_snapshot` then makes `mem_read` emit a guarded snapshot
`t := g ? r.v_T : 0` whose set holds its guard's, or leaves the read inline as before. `register_access` records each
access and call with its tree path; `order_rel` decides from two paths whether C orders them (a sequence point, an
operand before its operator, a call's arguments before the call, a store after its operands) and, when it does not,
lowering emits the path-precise `unsequenced` obligation (`g_A && g_B ==> place_A != place_B`, or against a callee
atom's argument; inside the E5 branch that computes a place, or after it with D(B)). `check_memory_order` then checks,
from the paths, that every cell write and snapshot is emitted on C's side of each memory call C orders with it (exit 3;
hooks `perturb_memory_order`, `drop_snapshot_guard_set`). A call whose arguments write a cell takes PRE(c) in both
halves for its permission and heap-reading checks. In `f__wf`, `overlap_obligations` asserts `prior ==> !(g_o && g_r)`
for an `owns`/`reads` pair on one parameter in one clause list, and each `ensures` atom asserts `prior ==> (g_ens ==>
g_partner)` (nothing when the partner is unconditional or has the same guard); a function's measure is lowered in
`CMode::Wf` over the pre-state ghosts with its framing assert; the method's `decreases` is plain and chain-guarded;
`explicit_termination` reads the callee's measure in the current heap and the caller's under `old(…)`. A full
expression with none of these lowers exactly as before.

**Pointers to pointers, `*result` and written atoms (M4.5b1c3, decision 0040).** A cell's field suffix recurses through
pointer types (`cell_suffix`: `ptr_int`, `ptr_ptr_int`; `fkey`, `use_cell_type` and `ltype` alike), so an address-taken
pointer variable is a cell with `field v_ptr_<t>: Ref`, its flag and an `alloc_ptr_<t>` method; `WF` takes the place of
the range wherever a cell's range is emitted (`cell_range`, `cell_range_inv`, an atom's value rows, the `__wf` ghost
`requires`), and the dummies of chains and snapshots are `null`. A store of a pointer into a cell asserts `WF` of the
value (`internal`) after the value's liveness check. Every pointer loaded from a cell is a snapshot temporary followed
by `result_splits`' case splits against the cells in scope, the parameter places and the enclosing loops' written-atom
places of its pointee's field (0040 item 2, audit B1). `contract_read` accepts the place `Result` (ghost key `-1`,
`hv_r_post`), and `wf_method` adds its `WF` or range, its equalities with the parameter ghosts of its field, and `g_r &&
g_e && result == q ==> h_result == h_q_pre` for a `reads` place returned by its partner. Written atoms in a loop's
invariants come from `vir::loop_atoms` as `WAtom`s with their own ghost amounts: the rows of 0040 item 5 follow the
automatic conjuncts (`NV`, `NL` with the automatic places' guards and the guarded de-duplication; positivity; flag and
range for `reads` too; value rows when the loop assigns neither `q` nor anything reaching the field); entry and
back-edge checks, the separation of every pair with an `owns`, and case splits before the snapshots, at the top of the
body, after the loop and after `l_brk`. Each atom is also a framing `Place` of `inv_places_` with guard `g && q == r &&
a_v > none`, and an `Atom` node in `CMode::Inv` lowers to `true` (its meaning is in those rows). 0037 item 8's per-call
splits also run against the enclosing loops' written `reads` places.

**Globals and `static` locals (M4.5b2a, decision 0041).** A mutable global or `static` local is a `vir::Global` in
`Program` (not a `Var`: `Var::id` is unique per function), and `&g` is the expression `GlobalAddr`; a read is
`Deref(GlobalAddr)`, a write a `Store`/`StoreE` through it, an atom `Atom(a = GlobalAddr)`. Atom sites are keyed by
`site_key` (a formal's index, `-1` the result, `kGlobalSiteBase + id` a global) with the receiver `site_ref`
(`loc(GB_g__<id>(), 0)` for a global, which has no argument at a call). Every method of a unit with globals except the
entry's takes the facts `len(GB_g()) == 1 && lvloc(GB_g()) == loc(GB_g(), -2)` as its first `requires`; the entry
(`Program::entry`, the file-scope `main`) calls `program_start()` first. A global's initializer as written is checked in
every prologue, as a named constant's is (no value guard for an unknown value). A use of a pointer gains the liveness
disjunct `lvloc(block_of(p)) == lvloc(GB_g())` per global (an operand that is `GlobalAddr` itself has no liveness
check). In loops a held global (from written and implicit `requires` atoms; every global in the entry) is an automatic
place `kl_GB_g__<id>__<loop>`; a write by name reaches only that global, a write through a pointer every place of the
field. No separation check, case split or `__wf` fact is emitted between two globals (`unique` blocks); a global and a
`*p` atom of one field are split and guarded as two parameter places are. A `static` local's declaration emits
nothing; the encoder appends the implicit `owns(s)` (`implicit_atom`) to its function and, by reverse reachability over
the call edges, to every caller except the entry. `__wf` ghosts of a global read are `hv_g<id>_pre`/`_post`.

**Initialization through pointers (M4.5b2b, decision 0042).** `vir::Param::init` (`None`, `RefToUninit`, `MustInit`)
marks a pointer parameter; `site_marker` gives the marker of an `owns` site on a parameter (never a global or implicit
site). A marked `requires` atom is `acc(p.v_T) && acc(p.d_T) && (p.d_T ==> range)` with no flag (no range row for a
`bool` cell, `WF` for a pointer cell); a `must_init` `ensures` atom is the plain one; a `ref_to_uninit` `ensures` atom
has `((g_r && old(p.d_T)) ==> p.d_T) && (p.d_T ==> range)`, `g_r` the `requires` partner's guard. `contract_read` never
frames a read by a marked site (pre-state always, post-state for `ref_to_uninit`). In `call_atoms` a marked atom has no
flag row and the range row `p.d_T ==> range`; every other cell atom's flag row is `uninitialized` (`uninit_text`). In
`atom_returns` `must_init`'s flag row is `postcondition` and `ref_to_uninit`'s is the `internal` row
`(g_r && old(p.d_T)) ==> p.d_T`, each in the place of the flag row it replaces. A function with a marked parameter or a
call of one (`Fn::init_range_rows`, `uses_init_markers`) gives every loop place `held && flag ==> range`, reached or not.
The assigned-parameter scan is `vir::assigned_params` (shared with the encoder's strict check; it counts stores too).

**Struct objects (M4.5c1, decision 0043).** A `vir::StructType` (interned in `Program`, members in declaration order;
`struct_leaves` gives each leaf's member path, type and source text, `struct_slots_of`/`struct_contains` the nested
slots) has no value field; a struct pointer's cell suffix is `ptr_` + `StructType::name` (`ptr_struct_<n>_<tag>`). A
struct `Var` is always a cell. `MemberAddr(a, k)` lowers to `loc(member(a, k), 0)`; in place position (under a `Deref`,
`Store`, `StoreE` or another place `MemberAddr`, recorded in `place_nodes_`) the chain's root pointer gets one
`null-dereference` row, reported at the consumer, and the leaf the permission and flag rows; as a value (`&p->x`) one
`null-dereference` row `p != null && offset_of(p) < len(block_of(p))` and then `p`'s liveness (`lifetime`). A member
leaf is written through a `Ref` temporary, so `lint_flags` is unchanged. Each struct type used by a local gets one
trusted `alloc_struct_<n>_<tag>()` (`alloc_struct_method`): the root facts, then one clause per ancestor/descendant
slot pair `X != Y`, then per member block in declaration order and depth first `len(member(X, k)) == 1` (and for a
leaf `acc(L.v_t) && acc(L.d_t) && !L.d_t`), then the liveness clause; the scope exit asserts every leaf and the liveness
cell at `write` (`lifetime`, naming the leaf) and exhales them. `WF_S(p)` (`WF(p)` and, under `p != null &&
offset_of(p) < len(block_of(p))`, the member lengths and slot facts) is written wherever a struct pointer's `WF` is.
Atoms carry a `vir::PlaceKey` (root, member path): `owns(*p)`/`reads(*p)` expand to every leaf with one ghost amount and
one liveness fraction, member atoms to one leaf or a nested object's leaves; every atom on a struct root gives `p !=
null`, `WF_S(p)`, `offset_of(p) < len(block_of(p))`. **Overlap is per leaf cell and VIR field**: `exclusive_pair` means
"a leaf field in common", `overlap_pairs` asserts the cells of one field differ (two atoms on one struct type compare
their slots); sums at calls and returns are per leaf; `split_candidates` gives the case splits (a struct target against
nested slots, a scalar target against the leaves of its field); framing has one disjunct per covering leaf; `f__wf`
ghosts are keyed by root, path and state. A struct atom makes its call a memory call on every leaf field (0039 item 4);
loops take every leaf as a place, the liveness row on the first leaf of each root under `lvloc(block_of(a)) !=
lvloc(block_of(b))` against earlier roots; havoc reaches every leaf field a struct atom owns. Copies and initializers
reach lowering as leaf `Deref`s into temporaries and leaf `Store`s (no VIR of their own). The markers apply per leaf
(`must_init`'s return row names the missing member).

**Struct values and struct storage (M4.5c2a, decision 0044).** A struct *value* is a tuple of leaves that exists in VIR
only between producers (`Call` of a struct-returning function, `Aggregate(T, args)` with one argument per leaf,
`Field(a, k)`, and in contracts `Formal`/`Result`/`Old(Formal)` under a `Field`) and consumers (a call argument, `Return`,
a struct `Store`, an expression statement of a `Call`, a `Field`; `check_program` admits nothing else, and lowering's
poison `LV` makes any other use an internal error). Lowering carries a struct value as `LV::leaves`; a by-value parameter
is one formal per leaf (`p_s__k_m<i>`, one `_m<i>` per path element) with each leaf's range or `WF`/`WF_S` as a
`requires`, its copy a struct block allocated at entry and filled from the formals through `Ref` temporaries; a struct
result is one result per leaf (`res_m<i>`) and a call of it a multi-target call (`t__n_m0, t__n_m1 := f(…)`,
`CallItem::targets`, `LStmt::targets`), with `result_splits` per pointer leaf. `ret()` assigns the result leaves before
the scope exits; a struct `Store` evaluates its value's leaves into temporaries, then checks the base once and writes
every destination leaf (permission, value, flag), and `writes_in` counts it as a write of every leaf field. An object
argument or `return s;` is an `Aggregate` of leaf `Deref`s (question 97: every member read). C++ `S{…}`/`S()` and C
compound literals used as values are `Aggregate`s; `&(T){…}` in C is a hidden address-taken local declared at the start
of the literal's block, its evaluation `Comma(StoreE(AddrOf(cl), Aggregate(…)), AddrOf(cl))` (the elements unsequenced,
then the stores). A struct global's `Global::leaves` give one initial value per leaf; the global facts gain its member
lengths and slot facts (`struct_lens`; S34: no `parent_of` fact is needed to tell two globals' blocks apart);
`program_start` has one row per struct global (every leaf's permissions, flag and value, then one `lv`); in `main` its
leaves are loop places; a `static` struct local is such a global with an implicit struct `owns`.

**Places reached through a heap read (M4.5c2b, decision 0045).** A contract place is a root, the steps to its base pointer
and a member tail (`vir::PlaceKey{root, via, path}`; `via` holds members, `kLoadStep` and by-value `field_step(k)`).
Lowering gives each distinct (root, via) with a step a per-function derived key (`kDerivedKeyBase + n`, built eagerly over
`requires`, `ensures` and `decreases`) and lowers its base in the current mode: chain-guarded over the atoms before it in the
method and at calls (Call mode over the arguments), ghost values `hv_<N>_pre/_post` with `WF`/`WF_S` in `f__wf`, the
current heap at returns; every use of a `requires` atom's guard, place or framing disjuncts outside the entry state is one
`old(...)` over the whole chain. Load-free places keep their int keys and Viper. In the body a loading `requires` place is
`eb_<N>` (assigned at method entry from the `requires`-framed chain), an automatic place of every loop, with a loop-entry
`lb_<N>__<L>` where the body may write a loaded field; returns run pass 0 (the inner cells held) before the separation rows;
an `ensures` atom through a load returns `ite(old(g_p) && q_post == q_ppre, a_p, ite(old(g_i) && …, a_i, a_p))` (amounts
follow objects), and a function without a body must tie it to its partner's place in `f__wf`. Guards may read the heap
(framed like any read). A memory call with a load-rooted contract snapshots each such place (`t__k`) before the call for
0039 item 4's checks. Written atoms on places through loads read their place through the automatic places' chains, with a
per-iteration ghost `aw_q<j>__<L>` for the body's splits; struct written atoms are 0040 item 5's rows per leaf with one
liveness amount and `WF_S`.

**The heap (M4.5d2a, decision 0047).** vlibc's `malloc`, `calloc` and `free` are trusted from vlibc by their contract's
*shape*, not their name: (A) a `void*` result with `ensures(B == NULL || owns_block(B, S) [&& zeroed(B)])` and exactly one
of `ref_to_uninit` and `zeroed`, (D) one `void*` parameter marked `ref_to_uninit` with `requires(P == NULL ||
owns_block(P))`. The encoder diverts such routines before `check_signature` and turns each call into a cached **view**:
an (A) call converted at once to `T*` (a cast, or C's implicit conversion) whose constant size equals `sizeof(T)` becomes a
bodiless, parameterless `Function` with result `Ptr(T)` and `ensures r == NULL || owns_block(r)` (calloc: one zero value
clause per leaf; malloc: `result_init = RefToUninit`); a (D) call over a `T*` argument becomes a `(Ptr(T)) -> void` view
with `deallocates`. The arguments are still encoded for their checks (a `Comma` before the call); VIR never holds
`void*`. `owns_block` is `AtomKind::OwnsBlock`, listed apart (`contract_block_atoms`), with no ghost amount: the block
part (offset 0, the token `loc(b, -1).tok`, `lvloc(b) == loc(b, -2)`, full `lv`, `len` and member lengths) plus, per
leaf, what `owns` gives except its liveness fraction. Calls and returns check it one row per conjunct, with `lvloc`
separation rows first; a (D) call's rows are `invalid-free` and replace the argument's liveness use check. In a heap
program (any view or block site) every method ends, after `label l_end`, with `assert forperm t: Ref [t.tok] :: E`
(`leak`, at the closing brace; `E` the `ensures` block sites' tokens, or `false`), and every loop back edge with a row
exempting the loop's token places. A block site counts as a full-permission atom on every leaf for 0039 item 4's
sequencing rules; consuming sites and (D) views register a write of the liveness cell (pseudo-field `lv`). In a heap
program a loop's automatic places also hold, for each pointer variable declared before it, the object it designates at
entry (the variable, or a ghost `hb := v` when the loop assigns it): leaf cells, liveness cell and the free token (`kt`
snapshot, back-edge `perm(tok) >= kt`), which the back-edge leak row exempts. Silicon is made to branch on the
guards (0047 item 10, amended at M4.5d2v): an empty `if (k > none) { }` after each heap snapshot, `block_of` splits
between token places, and `block_of`/`lvloc`/receiver (member-cell, for a structure) splits against each pointer
variable declared before the loop that it does not use and each block-site formal; the copy of an unassigned parameter
with an `owns`/`reads`/`live` atom is not a heap place; every pointer local a loop assigns (declared outside it) gets an
automatic `internal` invariant `flag ==> WF(v)` (`WF(v)` without a flag), so the free and call rows' `WF` check is
`internal`.
A heap place's back-edge permission, liveness and token rows are ending rows (`if (!cond) { assert false }`, so a failed
row ends its path), and its flag, value and range invariants have `permission` back-edge rows of their own. In a heap program
a call's pointer result (not a view's) is split against the call's own pointer arguments (`if (r == a) { }`), so a
free of a result that may be one of its arguments' blocks is resolved. **vlibc's
allocator** (record 0050): a block's allocator is its token's value, `loc(b, -1).tok` true for the platform's and false
for vlibc's; a `vlibc_` view ensures `!tok` (`Function::allocator`, from the generated name list's pairing field), a
user's `owns_block` keeps `tok`; in a program whose VIR holds any `vlibc_` view the token row is split (the permission,
then the guarded value naming the heap) and each loop token place carries its value (`ktv`), the internal invariant
`tok == ktv` replacing `tok`; a program that calls no `vlibc_` routine has none of this (its Viper is 0047's, with
the ending and mirror rows above in its loops).

**Results that give a place, owning locals and marked locals (M4.5d2b1, decision 0052).** The exact Viper is in
`build/m4.5d2b/shapes.md` (the VIR self-test cases `po_*`, `ol_*`, `bg_*`, `vs_*`, `fu_*`, `er_*`).

*Atoms without a partner (item 2).* An `ensures` atom with no `requires` partner of its kind and place is numbered
`nreq + k` in clause order (the encoder's `finish_atoms`; `add_static_local_atoms` shifts the numbers when it adds
`requires` atoms and puts the implicit `ensures` atoms first in such a method, audit B5). Only IMPLICIT atoms cover (a
`static` local's `owns(s)` and the atoms an `owning` parameter stands for, both marked `Expr::implicit_atom`; question
159): `cov_k` is the disjunction of `g_j && place_j == place_k` over them in the post-state (the value cell for `owns` and
`reads`, the liveness cell for `live`). Where `g_k && !cov_k` the atom GIVES an output amount, a ghost `Perm` result of
the method `out_<k>`, with `ensures none < out && out <= write` before every user clause; before the return rows `out_k :=
(cov_k || !g_k) ? write : (perm(L_k) - R_k) / M_k` (`R_k` the amounts returned for partnered atoms on that cell, `M_k` the
given atoms on it; for `reads` the least over value, flag and liveness cells). The ending row (`if (!(g ==> (cov || (out
> none && cells held)))) { assert false }`, `permission`: "returns `*r` without holding it") reports an atom the function
does not hold once; the existing per-cell rows follow with `out_k` in the sums. The postcondition of the atom is `pure &&
(!cov ==> (none < out && perms)) && facts`; a caller receives the outputs in ghost `Perm` temps of a multi-target call.
`atom_place` gives a result-rooted place the key `kResultSiteKey`. A bodiless contract (item 2a, `finish_atoms`) may carry
one only on its result and only if no clause equates the result, or a pointer read through it, with a non-null value
(`contract_equates_result`); a formal's partnerless atom, a block site with no partner on a formal and `owns_block(r) && r
== p` are rejected (`unsupported_annotation`, the contract manifest, M6). An atom nested with a partnered `ensures` atom on
one root is rejected before lowering (audit N7), and the lint pins the `out` rows (`lint_viper.py`).

*Owning locals (item 3).* `Var::owning` (set by the encoder only on an automatic, non-address-taken pointer local of
object type). Each owner `v` has two method-level ghosts: `held_v` (false at method entry and at the declaration) and
`tv_v` (the token's value). The value rule at every `Decl` with an initializer and every `Assign` (`owning_write`) checks
`new != null ==> offset_of(new) == 0 && perm(tok) == write` (`permission`), then `held_v := new != null`, `tv_v :=
tok(block_of(new))`. After every call and free `held_o := held_o && o != null && perm(tok(block_of(o))) > none` for each
owner in scope (a consumed block holds nothing, audit S5). Owners at an exit: the owners of the outer scopes and the
earlier ones of the scope, and at a `return`/the function's last statement the entry value of each formal with an
`ensures` block site (a parked borrow, S11) and the returned value when the function has a result block site. The ENDING
ROW (`if (!(held_v ==> owners)) { assert false }`, `leak`, once per block) is emitted at a block's end, `break`, `continue`,
`goto` out of the scope and `return` (`Fn::Scope::owners`, `owners_exit`), and before a reassignment with the new value
counted among the owners. The end-of-function `forperm` stays. A loop carries each owner declared outside it that it uses or assigns (`loop_owners`) by its CURRENT value, not by
0047 item 10's entry-keyed place: the invariant `held_v == (v != null)` with the whole block (token at `write` with the
value `tv_v`, full liveness, every cell) under `v != null`, explicit entry and back-edge ENDING rows `v != null ==>
held_v` and whole-block permission rows (`permission`, "set it to `NULL` after freeing it or handing it on") ahead of the
invariant, and the back-edge leak exemption `v != null && t == loc(block_of(v), -1)`. An owner the loop neither uses nor
assigns but whose body calls a function carries only `held_v` and its token. In a vlibc heap program the token's value is
the invariant's constant where every assignment to the owner in the loop comes from one allocator's views or is `NULL`
(`assigned_class`), with a `permission` row at entry for an owner that enters with the other allocator's block, and the
loop-entry snapshot otherwise. A non-owning alias of the owner's block keeps its 0047 item 10 place, with guards that
exclude the owner's block. A user invariant or measure that reads `*v` of a carried owner is framed by the owner's cells,
and a written atom on them is a `permission` at the atom ("which the loop already owns through its owning local"). Programs
with no owner keep their Viper. Owners on globals, `static` locals and address-taken locals: M4.5d2b2a (0054 items 5 and 6); fields: M4.5d2b2b (0055).

*Marked locals (item 4).* `ref_to_uninit` on an automatic pointer local changes no VIR; the encoder records it
(`Ctx::ref_marked`). Under `--strict-init` a call whose result is `ref_to_uninit` is accepted only where its node is in
`Ctx::strict_result_ok` (a marked local's initializer or assignment, a `ref_to_uninit` argument, a marked function's
return, filled before the value is encoded); a marked local receives only a null pointer, a `ref_to_uninit` call, `&x` of
an `uninit` local or another marked pointer; a marked function returns a marked local, a `ref_to_uninit` call or a null
pointer (question 154).

*Block-site guards and vlibc blocks (items 6 (b), 7).* A block site's guard that reads the heap, a global or `old` is
lowered like an atom guard (0045 item 11): `site_guard` frames it by the atoms before the site (`atoms_before_`), in the
contract, at the call and return rows and in the leak exemption (an `ensures` guard read at the end label). A site with
`Expr::vlibc_block` (`cerastes_owns_vlibc_block`, C only) states `!tok`, its rows read the token's value (`want ==
Vlibc`), and a contract that names one makes the program a vlibc heap program. A pointer argument's liveness use is registered at the argument's own place, so `two((free(p), 0), p)`
is `unsequenced` (6 (e)). Cut to M4.5d2b2 at the time: a written `owns_block` in a loop invariant (6 (a)), built by M4.5d2b2a (0054 item 4).

**Block sites through memory, owners in memory, block carriers in loops (M4.5d2b2a, decision 0054).** The Viper
shapes are pinned by `tests/viper-smoke/memory/s39_*` (spike S39) and the VIR self-test cases `hbl_*`, `olp_*`, `owg_*`.

*Block sites through memory (item 2).*
- **Keys.** `atom_place` no longer special-cases `OwnsBlock`: a block site's place is the place of its pointer value,
  `*pp` → `(pp, [load], [])` (the key of `owns(**pp)`), `g` → `(global, [load], [])`. `block_key` is gone. Consumers use
  `block_formal(s)` (a formal's index only when the pointer is the parameter itself), `block_is_result(s)`, `site_key`,
  `same_place` and `site_root`. `block_ptr` lowers through `site_root` in every mode. In the return state,
  `block_ptr_ret` serves `leak_exemption` and `returned_owners`. `block_arg_text` replaces `e.args[key]` in call
  messages.
- **Duplicates and "beside".** `finish_atoms` and `check_program`'s `block_atoms` compare whole keys, so
  `owns(*pp) && owns_block(*pp)` is a cell and its block, while `owns(**pp) && owns_block(*pp)` is "beside".
- **Snapshots.** `entry_snapshots` takes `eb_N` for `requires` sites through loads. The sequencing registration takes
  0045's pre-call snapshot. Loops treat such a site as an automatic place with a token place.
- **Bodiless contracts.** `f__wf` asserts the tie `prior && clause ==> (g_ens ==> (g_partner && E_post ==
  E_partner_pre))` for each `ensures` site that loads or rests on a global. The at-atom partner row is skipped for
  sites with `via`, whose guards read two states. The pairwise distinctness assert is skipped for pairs tied to distinct
  `requires` partners.

*Loop carriers (item 3).* `LoopOwner` carries:
- `pairs` (`OwnerPair{snap, kind}`), `written`, `share`, `unmentioned`, `hs`, `rank`, `ev`, `vals`, `valset` and
  `merged`;
- `carrier_guard`, which is `!cur` (current state) for owner–owner pairs a loop writes, `!snap && !cur` for a heap place
  paired with a written owner, and `!snap` otherwise;
- the exclusivity clause `snap || !cur` with its back-edge row, which replaces 0052's pair row, except for pairs with a
  syntactic sharing write (`owner_values`);
- value sets `o == ev_o || o == null || o == x…`, emitted only in a loop that has another carrier or place;
- unmentioned owners, carried under `held_v == hs_v` with the row `hs_v ==> held_v`;
- the flag clause on `hb_w` (`owner_flag_rows`).

*Written and synthetic block sites (items 4, 5, 6).*
- **Written sites.** `loop_atoms` lists written `owns_block` sites, and `loop_block_atoms` lists them alone.
  `WBlock`/`resolve_blocks` build a whole-block carrier keyed on the current value, plain mode (flags claimed). Its
  entry and back-edge rows come from `block_rows`, and pair rows separate it from sites, owners and places (no NOTAUTO
  guards).
- **Merges and takeovers.** A site on an owner merges with the owner's carrier. A site through a load on a parameter
  copy takes over the derived `requires` place (`wsite_keys_`, `wsite_vars_`).
- **The back-edge `forperm`.** It is emitted when `loop_may_allocate(s) || !wbs.empty()`. The site pointers and guards
  are hoisted into temps before it.
- **Synthetic sites.** `owner_loop_sites` adds a synthetic site `x == NULL || owns_block(x)` for each held owning
  global or `static` local the loop names (or whose `owns(g)` a callee in it takes), and for each owning address-taken
  local the loop names or may reach through a store or load of its type. Each comes with `held_x == (x != NULL)`. Its
  flags are kept by `wef_*` ghosts: `known && wef ==> flag`, with an `uninitialized` back-edge row.

*Owners in memory (items 5, 6).*
- **Owning globals.** `Global::owning` and `Global::ref_to_uninit` are set by the encoder's `read_variable_markers`
  (per-declaration lists from `il_query`'s `variable_secondary_attrs`). `apply_owning_globals` joins `owns(g)` with
  `g == NULL || owns_block(g)` as one conjunct, and has an internal-error fallback. `add_static_local_atoms` appends the
  deep clause after a `static` local's implicit atom.
- **Ghosts.** `Fn::gowners` declares `held_GB_x` in every method of a program with an owning global; there is no `tv_`
  ghost for owners in memory (only owning automatic locals keep `tv_`, which the loop code reads). `main` starts with
  `held` false. Another function starts with `held := cell != null` under a deep `requires` site. `is_heap_program`
  counts owning globals. An owner that a loop does not carry is framed out of it (`framed_ghosts_`, `FramedScope`):
  `held_updates` skips its ghost in the body. A synthetic site's `wef` also starts true when a callee in the loop
  takes the owner's atom (`callee_takes_owner`). Written-block case splits (`bws`) stand after the place snapshots
  (`snap_mark`).
- **Rows at stores.** `global_owner_rows` writes the leak row and the value rule at stores by name (`Store`, `StoreE`).
  There are no rows for stores through aliases.
- **After calls.** `held := held && perm(cell) > none && cell != null && perm(tok) > none`, or `held := cell != null`
  when the callee's `ensures` has `owns(g)`.
- **`main` and `program_start`.** `main`'s leak exemption gains `held_GB && t == loc(block_of(cell), -1)`.
  `program_start` prints `// cerastes:owning-global GB_x` (`LMethod::tags`), which the lint requires for every
  exemption term.
- **Address-taken owners.** These read `owner_value` from the cell, through `owning_rows`/`owning_after` at
  `write_cell`, `cell_write` and owner writes, with guarded `if (recv == v)` rows at stores through a `T**`. After a
  callee that hands a block back through `*pp`, `held_v := held_v || (v != null && offset_of(v) == 0 && perm(tok) ==
  write)`.

*The leaf cover (item 7).* `cov_of` covers an atom on a scalar leaf of a `static` struct through its implicit `owns(s)`,
over the leaves of the atom's pointee type: `cov := res == leaf_0 || …`. Value facts and permissions stand under `!cov`
only for leaf covers. A contract read of `*res` after a leaf-covered atom is routed through the covering leaf when two
or more leaves of the type exist.

**Owning fields (M4.5d2b2b, decision 0055).** `vir::StructMember::owning` (and `StructLeaf::owning`) marks a pointer
member; the encoder reads it in `struct_info` from the field's own attribute list and rejects the shapes of item 2
there (`owning_field`, `owning_reaches` for cycles; `check.cpp` has a backstop). The Viper shapes of the named leaves
are pinned by `tests/viper-smoke/memory/s40_*` (spike S40) and the VIR self-test cases `owning_field_*`,
`owning_leaf_*`, `owning_cover_*`, `owning_main_*` and `owning_global_leaf_*`.

*The closure (item 3).* `apply_owning_closure` runs after `apply_owning` and `apply_owning_globals`, before
`finish_atoms`: `close_source` replaces each `Owns` atom (and each `OwnsBlock` atom on a pointer to such a structure)
that covers owning leaves by `atom && (L == NULL || (owns_block(L) && <the closure of L's own leaves>))`, wherever the
atom sits in its clause, so the clause stands under the atom's guards. Only clauses are added, never atoms. A user
site of the same shape (or the vlibc spelling) is merged; another block site on `L`, or a `reads`/`live` atom on its
block, beside the covering atom is rejected. Markers (`closure_marked`) and the heap views (built by `heap_view`,
never through the pass) carry none. In a bodiless contract a partnerless closure is accepted only for an `owns(*r)`
result that gives exactly one block. `deep_expr` closes the deep atom of an owning `struct S *` global, and
`add_static_local_atoms` appends `global_closure(g)` after a `static` struct local's implicit atom.
`apply_invariant_closure` does the same for written loop invariants. `check.cpp` admits a contract read of a `static`
struct local's owning leaf (`owning_leaf_place`) and an invariant `owns_block(s.q)` of a struct local's leaf.

*The value rule (item 4).* `owning_leaf_of(addr)` finds an owning member at a store; `leaf_value_rule` asserts `v !=
null ==> offset_of(v) == 0 && perm(tok slot of v) == write` (`permission`) at `store`, `cell_write`, `struct_store`
and `struct_cell_write`, before `owner_alias_rows`. No dropped-block row. `is_heap_program` is true when any struct
type has an owning member.

*Owners at named leaves (item 5).* The owner key is `OwnerRef {var, global, path}` (a neutral re-keying first:
`held_name`, `owner_value`, `tv_name`, `owners_in_scope`, `LoopOwner::owner`). Leaf owners come from
`global_owners_of(g)` (struct globals and `static` struct locals, in `fn_.gowners`) and `Fn::Scope::leaves`
(struct locals, a by-value parameter's copy in the function scope). Ghosts `held_<GB>_<k…>` and `held_v_<var>__<n>_<k…>`.
- **Entry:** true where a `requires` site names the leaf (`site_leaf_owner`, `param_leaf_sites_at_entry`), through a
  temp `t := <guard> ? <cell> : null; held := t != null` for a guarded site; false in `main` and at a local's block
  entry.
- **After calls and frees** (`held_updates`): the framed `held := (held && perm(<cell>) > none) && (L != null &&
  perm(tok(block_of L)) > none)`; after a callee whose `ensures` has an `owns` atom covering the leaf and that is
  given exactly `&s` (`gives_param_leaf`, `gives_global`), `t := perm(<cell>) > none ? <cell> : null; held := t !=
  null`.
- **Stores by name** go through `global_owner_rows` generalised to `OwnerRef` (the reassignment `leak` row and the
  value rule); whole-struct stores find each leaf's owner with `tracked_owner_at`. `returned_owners` skips a leaf
  site of a struct global.
- **Lifetime end:** `owners_exit` has a leaf loop ("owning field `s.data` still holds its block"); `scope_end` runs
  owners' exits before the cells go when a scope has leaves; a void function's fall-through runs the parameter
  scope's rows.
- **`main`:** the exemption term `held_GB_g__0_0 && t == loc(block_of(<leaf cell>.v_ptr_T), -1)` with the
  `program_start` tag `// cerastes:owning-global GB_g__0/0` (member indices after `/`). Deeper owners (a chain under a
  static owner, `gp->q` of an owning `struct S *` global) use Bool locals assigned before the leak row, since Silicon
  refuses `perm` inside `forperm`: `exm_0 := (held_GB_g__0_0 && <leaf> != null) && (perm(<next cell>) > none)`,
  `exm_1 := (exm_0 && <next> != null) && (perm(<cell>) > none)`, and the disjuncts `exm_n && t ==
  loc(block_of(<chain>), -1)`.
- **The cover** (audit S6, then round 2's S3 and the re-audit's B1): `closure_cover()` marks the closure site of an
  output `owns` atom that `cov_of` can cover; such a site is never given (`block_atom` gives `true`), never returned
  (`block_returns`' guard `false`) and never exempt (`leak_exemption` skips it, `closure_cover_ret`).

*Loops (item 7).* `owner_loop_sites` builds a synthetic site `L == NULL || owns_block(L)` for each leaf owner the loop
reaches (it names the structure, stores through a pointer of its type, or calls a function whose contract names cells
of that type; a global leaf reached only by type is carried outside `main` and only where the function holds it at
entry), unless a written site names it; `loop_leaf_owners` carries its ghost with `leaf_held_agrees`: an entry snapshot
`hp_<held>__<loop>` and the invariant `hp ==> (held == (L != null))`, `!hp ==> !held`, with entry and back-edge rows
(`permission`). Unreached leaves join `framed_owner_ghosts`. Reads of a leaf owner's value that rest on `held` add
`perm(cell) > none` (`held_equals`). An owning local `struct S *v`'s loop carries `make_closure`'s site over its
pointee's owning fields (for a struct local's leaf one level until M4.5d2b2c, every depth since: decision 0056 item 7), standing only where the cell it is read
from has a value (`synth_closure_atoms_`, `wfc_<i>__<loop>`), and not built when the loop may replace the owner
(`may_replace_local`, `call_takes_address_of`).

**Loop limits of owning fields (M4.5d2b2c, decision 0056).** Every change only adds a checked invariant, a mapped row
or a guarded update; the VIR self-test cases are `define_loop_limits_l1` and `define_loop_limits_l2`
(`src/viper/selftest_cases25.inc`), the Viper spikes `tests/viper-smoke/memory/s41_*`.
- **Flags of a site the loop cannot fill (item 2).** `flags_kept_entry`'s `wef_<i>_<j>__<loop>` starts
  `stores_type || callee_takes_owner || !loop_may_fill(loop, pt)`; `loop_may_fill` (`expr_may_fill`, `stmt_may_fill`)
  sees a non-null store of the pointer type through memory or a whole-structure store, a write of an address-taken
  variable of that type, and a callee whose `ensures` has a block site with a load step and that type. Every synthetic
  site (question 204). `callee_takes_owner` is true for a leaf site when a callee's `requires` has such a block site
  (item 4(b)).
- **The value set (item 3).** `others` also counts an owner with a synthetic closure site (`has_closure_site`).
- **A leaf's ghost through an alias (item 4(a)).** `held_updates` receives the call's arguments; for a plain local
  pointer argument `a` whose parameter points to the root's struct type and `gives_param_leaf`: `t := (a == <root> &&
  perm(<cell>) > none) ? <cell> : null; held := (a == <root>) ? (t != null) : held`, after the framed update; `<root>`
  is the struct local's block or `loc(GB_g(), 0)`.
- **In `main`, a global leaf reached by type (item 5).** The synthetic site stands under `hsl_<held>__<loop> := held
  && perm(<cell>) > none` (taken before the entry rows, conjoined to `WBlock::g`); the invariants `hsl ==> (held == (L
  != null))` and `!hsl ==> !held` with entry and back-edge rows replace the `hp` pair for that leaf; the loop gets its
  back-edge `forperm` row.
- **Site cells as places (item 6).** `resolve_blocks` gives each resolved site whose pointer the loop cannot write its
  block cells as invariant places (not keyed places; none for a site merged with an owner's carrier), with the value
  rule over entry ghosts `eob ==> cell == vsb`, `eob ==> on && p == epb` and mapped rows. `owner_loop_sites` builds
  `make_closure` for an owning `struct S *` global and for struct-global leaves that already have a site, with a
  replace test (a store to the global by name, inside an expression or through a `struct S **`, a callee covering its
  cell by name or by type).
- **Chains (item 7).** `closure_deep` is gone; each intermediate flag joins the deeper site's guard; the pre-read of a
  site read through an earlier site's cells is emitted after that site's rows (struct-leaf and owning-global closures;
  question 206); `wblock_splits` leaves out the pair splits between such a site and that earlier site (no-op case
  splits, for Silicon's paths only).

**The invalidation profile (M4.5d2b2d, decision 0058): lowering.** Every change below is lowering's half of an encoder rule (§8, "The invalidation
profile in the encoder"); the VIR self-test cases are `define_not_invalidating`, `define_not_invalidating_rejections`
(`src/viper/selftest_cases26.inc`) and `define_marked_fields` (`src/viper/selftest_cases27.inc`); the Viper spikes are
`tests/viper-smoke/memory/s42_*` (spike S42, with its addendum in `internal/il-inventory.md`).

*`not_invalidating`: the new `f__wf` row (item 2).* `vir::Param::not_invalidating` marks a pointer parameter. In
`wf_method`, in `go`'s `if (ens)` part after the existing partner rows and the bodiless tie rows, each `ensures` atom
site (partner `fa.sites[atom->amount]`) and each `ensures` block site (partner `block_partner`) of the clause whose root
(`atom_place(*atom).root`) is a marked parameter gets `ANT ==> (GP ==> GE)`, `permission`, at the `ensures` atom:
"the contract of `f` takes `<text>` on a path where it does not return it, but `p` is `cerastes::not_invalidating`".
`ANT` is `prior && v.e`, the clauses before **and the whole clause** of the site, so the equalities the encoder spliced
into it are in the antecedent (a guard that reads a loaded pointer needs them); `GP` is the partner's guard over the entry
ghosts (`hv_<k>_m<j>_pre`, or the formals), `GE` the site's over the return ghosts (`hv_<k>_m<j>_post`). Nothing is
asserted when the `ensures` site has no guard, or when both guards lower to one expression (`same_expr`: a guard that
reads the heap never does, its two states are two ghosts); with no `requires` guard the consequent is `GE` alone. It is the
converse of the partner row `ANT ==> (GE ==> GP)` (the same two guards swapped; a lint can pair them), it is in `f__wf`
with or without a body (a body may drop a cell or a block the contract names on a path the guard excludes, which no return
row sees), and it is only for marked parameters (an unmarked pair keeps today's rows). Pinned by `ni_row_cells_*`,
`ni_row_blocks_*` and the fixtures `s42_1a_*`, `s42_1b_*`, `s42_3a_B_tie_ok`, `s42_3b_B_chain_ok`, `s42_3d_N_chain_spliced_ok`
(the last is the tool's own output for a spliced chain).

*N1: the bodiless tie above the partner row.* In the bodiless block-site loop the tie assert ("... without saying it is the
one its `requires` took ...") now precedes the partner-guard assert ("... on a path where its `requires` did not take
it") for the same site: Silicon goes on after a failed assert as if it held, so an untied borrow gets one error, the
tie's, which names the fix. Eleven existing programs (22 maps) swapped two adjacent lines and nothing else changed; the VIR
case `owning_global_leaf_guarded_ensures` swapped too (`s42_3c_B_unmarked_second_n1_fail` pins the one-error result).

*The postcondition message (item 2).* `Clause::added` holds the text of the equalities the encoder spliced into a clause
(`p->q == old(p->q)`, joined by " and "), `Clause::added_params` the indexes of the marked parameters they are for.
`postconditions()` words a failing clause "the postcondition of `f` may not hold (with `<added>`, which
`cerastes::not_invalidating` on `p` adds)" (several parameters joined by "` and `"; the first marked one when the list is
empty). Cases `ni_added_holds`, `ni_added_message`, `ni_added_message_two`.

*The backstop (`check.cpp`, `not_invalidating_markers`).* A marked parameter is a pointer, not beside an initialization
marker, and on a function that does not return no atom or site is rooted at it; every `requires` atom or block site rooted at
it has an `ensures` partner and every `ensures` one a `requires` partner (a block site by place, a permission atom by kind
and amount, as the encoder's `check_not_invalidating` does). An internal error otherwise (`ni_backstop_*`).

*Marked closures (item 4).* The closure of an owning field is a block site on a load (`Deref(MemberAddr(base, k))`).
`block_marker` reads `StructMember::ref_to_uninit` of the last member step (`marked_field_site`, through
`owning_leaf_of`), and the deep atom of an owning and `ref_to_uninit` global (0054 item 2) is the other
`marked_through_load` site; every marker of a formal's site is as before. `block_mode` then gives, for such a site: a
`requires` site `Uninit` (no flag claimed); a given `ensures` site (no `requires` partner, audit S2) `Uninit`; a borrowed
one `Kept` only where it is **tied** (`site_tied`, by position, audit round 2 S1: an `E == old(E)` conjunct that is
unconditional at the top of any `ensures` clause (`conjunct_ties`), or a conjunct of an `&&` chain on the path from the root
of the site's own clause down to its atom, the other operand of each `&&` above it (`tie_on_path`, which descends `&&`
operands, the right operand of `||` and the arms of `?:`, as `contract_atoms` finds sites): so the equality
`not_invalidating` splices under an `owning` guard counts, and so does a guarded written tie under the site's own guards; a tie under
another guard, or in another `||` arm or `?:` arm, is never an operand of an `&&` on that path and does not count) and
`Uninit` otherwise. *Why tied only (audit B1; the table is in the addendum of S42 in `il-inventory.md`):* a
body that frees and replaces the block compares the old block's flag with the new block's, and the model does not prove
a fresh block differs from the freed one (`free` consumes the token and the liveness cell, `malloc` inhales another's, and
Silicon's distinctness assumption covers only locations it still holds), so no sound row verifies an untied replacing
body (it exited 3 before, on the owning and `ref_to_uninit` global too); an untied site claims no flag, which is what a
given one does. *The row and one error:* a `Kept` site's row, `(old(L != null) && old(E.d)) ==> E.d` per cell (`kept_row`,
factored out of `block_conj`), is the site's part of the method's `ensures` (Silicon's own check) AND, in `Ret` mode,
`block_atom` returns it instead of `true`, so `postconditions()` asserts the whole clause (the tie, the closure's row)
as one atomic `!(!(CLAUSE))` (`ret_kept_row_`; Silicon reports each conjunct of an asserted `&&` as its own error). A body
that replaces a tied block gets ONE user `postcondition` when the tie is in the site's own clause, and Silicon's own check of
the contract passes after it (no exit 3); a tie that counts by the top-level rule in ANOTHER `ensures` clause is a separate
return assertion, so a replacing body gets two (the site's clause and the tie's), still no exit 3 (re-audit N2).
An unmarked owning field keeps the plain closure, byte-identical; a marked field that does not own changes no Viper.
The caller side (the partner-guard rows at calls and returns) is not touched: an untied callee that replaces an
initialized buffer leaves the caller `uninitialized` at its read (`mf_untied_replaces_caller_reads`,
`s42_4b_untied_replaces_caller_reads_fail`). Pinned by `mf_*` (the filler, the read first, the plain controls, the kept
flag, the replacing bodies) and the fixtures `s42_4b_*` (tied, untied, replacing; the two globals).

*Return rows in order (item 4, audit round 2 B1, question 220).* `block_returns` emits each `ensures` block site's OWN rows
(`block_rows`, per site in `fb.sites` order) before the separation rows (the pairwise `live_cell` distinctness of two block
sites, and of a block site and each `ensures` atom site: "... may be the same object"), as the call rows already did and as
a written loop invariant's back-edge rows did: a separation row reads the pointers of nested sites (`a->b->x` through
`a->b`'s block), which are readable only where the function still holds that block, and the site's own row is the one that
ends the path where it does not (Silicon goes on after a failed assert as if it held). A correct program exited 3 on the
base through a separation row's read (`q7_alias.c`). The separation rows themselves are unchanged. The order changes the Viper of 227 existing programs (row order alone) and drops one duplicate `permission` at the
same line in three pinned tests (the failed own row is assumed, which proves the blocks distinct).

*A marked owner closes nothing (item 4, question 202).* `owner_marked(OwnerRef)`: a local or global with an empty path is its
own `ref_to_uninit` (`Var::ref_to_uninit`, `Global::ref_to_uninit`); with a path, the last member's (`StructLeaf::
ref_to_uninit`, filled by `struct_leaves`). The three `make_closure` sites of the loops (owning locals, struct leaves,
owning globals) skip a marked owner; its own block site is still carried. A WRITTEN invariant site over a marked field's block
is a plain site and claims the flags: an unwritten buffer is `uninitialized` at the loop entry (a false alarm, as for an
unmarked field; the way out is to write the buffer first). Cases `mv_*`.

**Arrays (M4.5e1a, decision 0059).** An automatic local `T a[N]` (one dimension, `T` a `bool`, an integer, an enumeration or a pointer to an
object type, `N >= 1`) is one block of `len` N whose element `k` is `loc(b, k)`, in the value and flag fields of `T`'s cell type (the same
`v_<t>`/`d_<t>` as a scalar cell: `v_ptr_<t>` for a pointer element). Cases: `selftest_cases28.inc` (rejections), `selftest_cases29.inc` (lowering,
`arr_*` for Part B and `arrl_*` for the loops); the Viper spikes are `tests/viper-smoke/memory/s43_*` (S43 and its addendum in
`internal/il-inventory.md`, chapter M4.5e1a, with P78's IL facts).

- **VIR.** `Type::Kind::Array` (`element`, `count`; `Program::array_of` interns the element like `pointer_to`); `Expr::Kind::ElemAddr(a, b)`, the
  address of the element `b` slots after the address `a` (its type is `a`'s, or `Ptr(T)` when `a` is `AddrOf` of an array local, whose own type is
  `Ptr(Array(T, N))`); `Var::zero_filled` (the declaration fills the block with C's static-storage zero before the written elements, which follow as
  ordinary `Store`s); `Stmt::keyword_loc` (a loop's keyword, the same line in both modes: C++ puts a statement with attributes at its first attribute,
  `loc`). `a[i]` is `Deref(ElemAddr(AddrOf(a), i))`, a write a `Store` to it, `&a[i]` the node as a value; `p[i]` is `Deref(ElemAddr(p, i))`; the decay of
  a local array is `ElemAddr(AddrOf(a), 0)` (a pointer to the element type, so `*a` is the place of `a[0]`). `check.cpp` accepts an array only as an
  address-taken local of an accepted element type (never owning, a parameter copy or a compound literal), `AddrOf` of it only as the base of an `ElemAddr`,
  and an `ElemAddr` base that is that address or any accepted pointer to a non-struct (a non-enumeration `Int` index of any `IntType`; never in a contract);
  every other array type (parameter, result, member, global, pointee, array of arrays) is an error. `place_text` spells `a[i]`/`p[i]` (the index by its
  `c_text`, a constant's value, or `+ - *` of those).
- **Allocation.** One trusted method per element cell type, `// cerastes:trusted-contract`: `alloc_arr_<t>(n: Int) returns (r: Ref)` (`requires n >= 1`;
  `r != null`, `r == loc(block_of(r), 0)`, `len(block_of(r)) == n`, `lvloc(block_of(r)) == loc(block_of(r), -2)` with full permission to its `lv`, and, as
  two quantifiers over the absolute slot with the trigger `{ loc(block_of(r), k) }`, full permission to `v_<t>` and `d_<t>` of every slot in `[0, n)` with
  every flag false) and `zero_arr_<t>(r, n)` (takes and returns the same permissions, ensures every element zero, `false` or `null` and every flag
  true). Both are emitted only for a program that uses them (keys `arr_<t>` and `zero_arr_<t>` of `alloc_at_`, so `program_start`'s insertion index
  counts them). The block is allocated where its block is entered (`v_a := alloc_arr_int(N)`, 0032 item 4), and a zero-filled `Decl` calls
  `zero_arr_int(v_a, N)`; elements an initializer does not write are zero in C (6.7.11p11, p21), so a list that writes all N skips the fill (`{}` is a fill alone). An enumeration's zero fill is `v == 0` even when 0 is not an enumerator.
  Without an initializer the flags stay false: reading an unwritten element is `uninitialized`. A `Decl` a `switch` jumps past does not run its fill.
- **Element rows, in this order, each one atomic** (`elem_addr` emits the bound itself, through `obl`, so it precedes the consumer's rows): for a pointer
  base `null-dereference` and `lifetime` (`ptr_use`); `out-of-bounds`, place `!(off < 0 || len <= off)`, value `!(off < 0 || len < off)` with
  `off = offset_of(base) + i` (one past the end is formed, never accessed; the index's value, not a conversion, is the offset: `a[-1]` is outside); then
  the consumer's `permission` (`perm > none` for a read, `== write` for a write) and, for a read, `uninitialized`. A named array in scope needs no null or
  liveness row (`address_or_null`). Messages name the place as written ("reads `a[i]`, whose index may be outside `a`", "forms `&a[i]` beyond one
  past the end of `a`"). A store goes through a `Ref` temporary as for a cell. `*p` keeps today's rows: a `*p` one past the end is a `permission` error.
- **Scope exits** (`array_scope_exit`, 0032 item 8's order): per field one marked range row (below), category `lifetime`, "the address of an element of
  `a` may still be in use when its lifetime ends", then the liveness row and three `exhale`s (the two quantified permissions and the `lv` cell). After a
  failed marked row Silicon reports nothing else on that path (S43 G2), so the `exhale`s need no guard.
- **Marked range rows.** `assert forall k: Int :: { loc(B, k) } lo <= k && k < hi ==> acc(loc(B, k).f)`, one field per row; `low::Check::expected_reason`
  (`insufficient.permission`) puts `[insufficient.permission]` after the row's Viper id in the map (§5), and `LExpr::Kind::Forall` (`forall k: Int :: { t }
  body`, parenthesized unless it is the whole condition) and `old_(e, label)` (`old[lbl](e)`) are the lowered forms the loop conjuncts use.
- **Loops** (`loop`, 0038 items 3 and 4 over a block of N slots). Every array local of an open scope becomes an `ArrayPlace` (a block that declares the
  array allocates it at its entry, so a loop before the declaration carries it too); it is not a `Place`: `loop_places` skips array cells. Arrays are
  carried exactly when the loop carries its places (`stmt_touches`, or a `return`/outward `goto` while something is held). *Entry*, after the scalar
  snapshots: per array a marked row per field (`permission`, "this loop begins without the permission to every element of `a`", at
  `Stmt::keyword_loc` when the encoder set it, else `loc`), then `kl := perm(lvloc(..).lv)`; `label lh_<loop>` sits right before the Viper `while`.
  *Conjuncts* (`internal` rows, after the places' own, every `acc` first): `acc(lv, kl)`; the two quantified `acc`s; the flags kept (`forall k :: in range
  && old[lh](flag) ==> flag`); the type's range (`forall k :: in range && flag ==> lo <= v && v <= hi`; `WF` for a pointer element; none for `bool`); and,
  when no write reaches the array, the values kept (`forall k :: in range ==> v == old[lh](v)`). The flag and range conjuncts go on every array the loop
  holds, reached or not. *Reach*: a store by name to an element (`ElemAddr(AddrOf(a), i)`, `Store` or `StoreE`) goes into `LoopWrites::cells` under the
  array's id (it reaches that array and every pointer-reached place, no scalar cell and no other array); a store through a pointer or a callee owning a cell
  of the field reaches by field. *Back edge*: per array a marked row per field (`permission`, "an iteration of this loop ends without the permission to `a`
  that it began with") and the liveness row, at the body's closing brace (fallback the loop's keyword). *Invariants and measures*: `inv_deref` sends
  `Deref(ElemAddr)` to `inv_elem`, `ite(in bounds, a[e], dummy)` plus two obligation clauses (`out-of-bounds`, "this loop invariant reads `a[e]`, whose index
  may be outside `a`"; `uninitialized`, guarded by the bound); no framing chain, the `acc` conjuncts come first. A missed write cannot give a false proof:
  every conjunct is a checked invariant, so one that is not inductive fails as an `internal` `invariant.not.preserved` (exit 3). An array declared in a
  body needs nothing new (allocated per iteration, exited at the body's end and by `continue`/`break`/`return`). After a loop that fills an array declared
  without an initializer a read is `uninitialized` (e1c's quantified invariants say which elements were written; pinned).
- **Sequencing.** Lowering gives every element access a FRESH key (`key_of_addr` falls through for an address it does not know), so `register_access` never
  skips a pair as "the same object" and each pair of an element with another write of the field, or with a `*p`, gets the path-precise row `place_a !=
  place_b` (`unsequenced`; S43 (e): `loc(b, i) != loc(b, j)` holds exactly when `i != j`). `check.cpp`'s `object_of` has a separate key space used only
  for the static rejection: `a[const]` and `a[local]` name one element as written (array id, index), so `a[i] = a[i]++`, `a[i]++ + a[i]` and
  `a[0] = a[0]++` are `unsequenced_modification`, as GCC's `-Wsequence-point`; `a[i] = i++` is 0020 item 4's rule on `i`. Any other index form is left to the
  path-precise row.
- **Interior pointers: `p[i]`, the decay and the owners' `offset_of == 0` restatements (Part D).** `elem_addr` takes any pointer base. A named array
  (`AddrOf` of an array local, which includes the decay `&a[0]`) keeps the rows above. Any other pointer `p` gets, in order: `null-dereference` (place
  "dereferences `p`, which may be null"; value "forms `&p[i]`, but `p` may be null"), `lifetime` (`ptr_use`, at the consumer's position for a place), the bound
  against the pointer's own offset (place `0 <= offset_of(p) + i < len(block_of(p))`, value `<=`: "reads/writes `p[i]` outside the object `p` points
  into", "forms `&p[i]` beyond one past the end of the object `p` points into"), then the consumer's `permission` and `uninitialized`. The consumers (`deref`,
  `store`, `cell_write`) skip their own null row for any `ElemAddr` and use the array-specific permission message only for a named array. Keys are fresh, as
  for arrays. `*p` is untouched. **No case splits were added** for a pointer assigned `&a[i]`: one assigned inside a loop and read after the assignment
  verifies as it is (`array_ptr_walk`, `array_ptr_loop_kept`); a pointer the loop does not reassign keeps its loop place, and `pk[0]`
  verifies wherever `*pk` does (`array_ptr_loop_pk_cell.c`; audit round 1's predicted false alarm does not reproduce); one the loop reassigns already
  fails at `*pk` (`null-dereference`, `permission`), as for scalar cells, and `pk[i]` adds `lifetime` and `out-of-bounds` against the unknown
  `len(block_of(pk))`. On a pointer
  known through `owns(*p)` or `reads(*p)`, `p[0]` verifies (the atom carries `offset_of(p) < len(block_of(p))`); `p[1]` fails twice, `out-of-bounds`
  and `permission` (S43 N1). Cases: `define_array_pointers` (`arrp_*`).
  *The restatements (item 10), backstops that no encoder path reaches.* `holds_token(p)` is `p != null && offset_of(p) == 0 && perm(tok) > none`; the three
  `leak_exemption` terms (the `ensures` block site; an owning global or `static` local, `held && offset_of(v) == 0 && t == tok_slot(v)`; and
  `exempt_owned_fields`) gain `offset_of == 0`; 0054 item 3(e)'s `flag_follows` premise gains `offset_of(pl.lroot) == 0` (the place's pointer then starts the
  block, so the place's cell is the owner's cell). They guard against an owner holding an interior pointer (`&p[1]` of a one-object heap block), which
  would exempt the block at `main`'s end and miss the leak; the owning stores' own rows (`owning_rows`, `global_owner_rows`, `block_rows`, which
  already require `offset_of == 0`) end the path before any state with an interior owner exists, so no VIR program reaches the restated terms with
  `&p[1]`: there is no VIR twin, and the hole is shown only by the hand-written fixtures `tests/viper-smoke/memory/s43_f_*`. The change reaches every
  program with an owner: 1,080 existing `.vpr` files changed (no `.map`), every one the same edit (deleting each `offset_of(..) == 0` conjunct restores
  the baseline line; 566 `held_x := held_x && ...` lines, 192 leaf-token lines, 48 global ones, 32 flag-clause lines), with outcomes unchanged. What the
  encoder hands lowering: the decay as a value is `ElemAddr(AddrOf(a), 0)`, `p[i]`/`i[p]`/`&p[i]` over any pointer value (parameter, local, loaded
  pointer, call result) is `ElemAddr(<pointer>, i)`; a pointer to a structure, a pointer element in a loop invariant or measure, and any element in a
  contract stay rejected.
- **Front-end options** (`verify_main.cpp`, with the diagnostics' lists): `--diag_error=175` in both modes (`subscript out of range`, a constant index
  outside a named array, also in dead code: the strictest reading; `&a[3]`, `p[3]`, `sizeof(a[3])` and `0 && a[3]` raise none), `--diag_error=1162` in
  C (excess initializers are only a warning there and the IL drops them; C++ has the hard error #146) and `--diag_error=138` in C (`register` on an array,
  `&` of a `register` variable). New encoder diagnostics: `char_subscripts` (`-Wchar-subscripts`), `array_address_null_test` (`-Wpointer-bool-conversion`,
  `-Wtautological-pointer-compare`, `-Waddress`) and `c99_designator` (C++).

**Ranges in contracts (M4.5e1b, decision 0060).** `owns_range(p, n)` and `reads_range(p, n)` are atoms on the elements `p[0..n)` of a pointer
parameter, `p[e]` is readable in a contract, a call can pass a whole array or a sub-range, a loop carries the ranges of its function's parameters, a
marker (`ref_to_uninit`, `must_init`) can stand on a range, `owning(n)` marks an owned range, and `T a[]` / `T a[N]` are pointer parameters. Cases:
`selftest_cases30.inc` (`rng_*` the callee, `rngc_*` the caller, `rngd_*` the loops, `rngm_*` the markers; `range_rej_*`, `rngm_rej_*`
rejections); the Viper spikes are `tests/viper-smoke/memory/s44_*` (S44 and its addendum in `internal/il-inventory.md`, chapter M4.5e1b, with P79's IL
facts).

- **VIR (Part A, plumbing).** `AtomKind::OwnsRange`, `ReadsRange` (`is_range_atom`) on `Expr::Kind::Atom`: `a` is the pointer's `Formal` (in an `ensures` its entry value),
  **`b` the count**; `Builder::range_atom(kind, ptr, count, amount, loc)`; the atom's `c_text` is `owns_range(p, n)` from the two operands' spellings. `same_pure_expr`
  compares two pure expressions structurally (Const, Local, Formal, Unary, Binary, Cond, Convert, ToBool, BoolToInt, Old, with their types; anything else, or an
  `Old(Formal)` against a `Formal`, is "not equal", so the partner rule fails closed). `atom_place` of a range atom is valid: root = the parameter's index, no steps, no
  member path, the count NOT in the key (two atoms of one kind on one parameter are duplicates whatever their counts); `atom_partners(ensures, requires)` is the
  one partner test (same kind, same place and, for a range, `same_pure_expr` counts). `range_perm(ref, lo, hi, field, amount)` in lowering takes expression bounds and
  an optional amount (the old literal-bound form delegates to it).
- **`check.cpp` (Part B).** A range atom is accepted in `requires` and `ensures` only. `range_atom_operands`: the base a bare pointer `Formal` to a bool, an integer, an
  enumeration or a pointer (a structure pointee is refused), the count an integer (not a bool, an enumeration or a pointer) that reads no memory, names no `Result` and
  is not under `Old`. `range_sites` backstops the encoder: a guard that reads memory, a range atom beside a cell atom or `owns_block` of the same parameter, two range
  atoms of one parameter in one clause list, a partnerless or other-count `ensures` range atom, a marker on a parameter that holds one. `Deref(ElemAddr(Formal, index))` is
  accepted as a READ in `requires`, `ensures`, `decreases` and under `Old` (the index reads no memory and may name the result); `ElemAddr` as a value in a contract is
  refused. `sequence_problems` and the loop rules are untouched.
- **Encoder (Part B, `encode.cpp`).** `<cerastes.h>` declares `cerastes_owns_range(...)`, `cerastes_reads_range(...)` (C23 variadic, `noexcept` in `extern "C"`) and
  `cerastes::reads_range` (owns_range's signature); the names are in the six lists (`SpecFn`, `il_query.cpp`, `annotation_checks.cpp`, `edg_args.cpp`) and in
  `spec_fn`'s declared-in-the-header test. `spec_range_atom` reads a call of either: exactly ONE top-level generated conversion is peeled on the base (C++'s `const T*`)
  and on the count (the `unsigned long` of C++'s parameter, C's argument promotion), never one inside a written count (`n + 1u` keeps its conversion) and never a written
  cast (a written cast on the base is rejected; on the count it keeps its meaning, but a conversion to the same type vanishes in VIR). The base must be a parameter
  (`Formal`) of a bool, integer, enumeration or pointer pointee, not `const` for `owns_range`; the count's IL type an integer other than a bool, an enumeration, a
  float or a pointer, and its VIR an expression over `Const`, `Formal` and the pure operators (a memory read, the result, `old` and calls each have a message).
  `finish_atoms` numbers `requires` range atoms with the cell atoms, pairs an `ensures` range atom by `same_pure_expr` of the counts (a missing or another-count partner, the
  same kind twice, a guard that reads memory, and a range beside a cell atom, a block site or the other kind of range of its parameter, through loads included, are rejected,
  each with a message naming where it is lifted; when another clause of the contract was already rejected the partner message is not repeated). `spell_formals` gives the
  `Formal`s of a count and an index their parameter's name (and a `Result` its binder's), so the lowering's messages print `p[0..len + 1)` and `p[i - 1]`.
  *Element reads:* `contract_element_address` (from `subscript_address` in a contract, `old_call` and `old_c`) builds `ElemAddr(Formal, index)`; the read is its `Deref`,
  under `Old` for `cerastes_old(p[e])` (C: `address_of(subscript(p, e))`, never folded; C++: a reference bound to the subscript; P79 (d)); a marked parameter is refused as `*p` is;
  `&p[e]` and `p + i` in a contract stay rejected (M4.5e2). In code and a `check` an element is read as before; a loop invariant or measure reads one by the rules of the loop bullet below (Part D).
- **Lowering of the callee (Part B).** `range_atom_value`: in `f__wf` the pure line `p != null && WF(p) && 0 <= n && offset_of(p) + n <= len(block_of(p))` (the count's
  obligations in the same assert); in the method's contract that line, then per field one quantifier `forall k :: { loc(block_of(p), k) } offset_of(p) <= k && k <
  offset_of(p) + n ==> acc(loc(block_of(p), k).f[, amount])` (full for `owns_range`, the atom's ghost amount for `reads_range`), the flags, the type's range (`WF` for a
  pointer element, none for a bool) and the liveness at the amount; the guard stands outside each quantifier, one conjunct per line (`push_clauses`, `split_conjuncts`: nested
  guards conjoined, `?:` split into two implications). A contract element read has an `elem_key` (one per function, parameter and structurally equal index), ghost
  formals `hv_e<n>_pre|_post`, and in `f__wf` the framing assertion: the read's address lies in a cell atom's place or in a range atom's interval (`permission`, "the
  contract of `f` reads `p[i]` without owning it"), the address-equality facts (also between an element read and a cell read), and a `reads` place's post value equal to
  its pre value. The partners-by-path assertion of 0039 item 7 covers ranges (B1: "returns `p[0..n)` on a path where its `requires` did not take it"). Returns
  (`range_returns`): marked rows (value then flag, inside `if (g) { }`, in the sum form where two read atoms of one field may overlap), the shape row as `permission`,
  the flags, the type's range and the liveness sum; a `reads` cell's return sum gains `(g_r && x in range r ? amt_r : none)`.
- **The caller (Part C).** *The encoder's part.* A call's arguments are spelled as written for the messages: `spell_range_arguments` (in `call()`, for a callee whose contract holds a
  range atom) gives an integer argument the text of its expression (`n`, `n + 1`, `3`) and the decay of a local array its name (`a`); `&a[1]` and a pointer variable already carry
  theirs. The encoder classifies nothing about placement; it forwards lowering's `unsequenced_calls` rejections. The earlier staging pin S1 is gone.
  *Lowering (`call_atoms`, record items 6, 8 and 10; every row atomic, under the atom's guard).* Per range atom the base and the count are first **bound to locals** (`bind`:
  an item of the call's rows that assigns a fresh temp; the base argument the call passes for that parameter is the bound local, `bound_args`, so the callee's quantifiers are
  instantiated at a variable and no row's base, bounds, count or amount reads the heap). The rows, in the built order: per atom the **shape row** first (`range_shape`: not null, `WF`,
  `0 <= n`, `offset_of(p) + n <= len(block_of(p))`, `permission`: "calls `f`, which needs to own `a[0..3)`, without owning it"; "... to read ..." for `reads_range`), then a
  `reads_range`'s first-slot row (`!(n > 0 && (perm(F.v) == none || perm(F.d) == none))`), then the liveness row; no case split involves a range. **Separation** follows every atom's
  rows, only where one side owns and the two share a field: a range and a cell (`!(block_of(x) == block_of(p) && offset_of(p) <= offset_of(x) < offset_of(p) + n)`; audit round 3, S1: the row is also built against a global's cell and against every cell of a place reached through a load, from the same `recv` the cell rows read; `rngc_global_*` and `rngc_loaded_*`), two ranges
  (`range_disjoint`). **Amounts:** `owns_range` takes `perm(L)` (the liveness cell); `reads_range` takes `n > 0 ? min(min(perm(F.v), perm(F.d)), perm(L), <the caller's own ghost
  amounts of the field>) : perm(L)` with `F` the first slot; `amt = (g ? m : write) / (N + 1)` is bound to a `Perm` local (`perm` cannot stand under a quantifier) and the rows
  after it name the local. **Per atom after the amounts:** the two marked rows (value then flag) inside `if (g) { }`, then the flags (`uninitialized`: "passes `a` to `f`, whose
  parameter `p` needs every element of `p[0..n)` to have a value"), then the type's range under the flag. The liveness sums include range atoms, and a `reads` cell's sum gains
  `(g_r && x in range r ? amt_r : none)`; **the marked rows of a `reads_range` at a call carry the sum** (audit round 3, B1; `call_atoms` step 4b): `acc(loc(B, k).f, amt_i + Σ_j (g_j && in_j(k) ? amt_j : none))` over the OTHER `reads_range` atoms `j` of the same field in the same call, `in_j(k)` being `block_of(q_j) == block_of(p_i) && offset_of(q_j) <= k < offset_of(q_j) + n_j` over the row's binder, exactly the form `range_returns` uses; with no other read range of the field in the call the rows are the plain ones. Item 8's earlier argument that no range sum row is needed fails once a callee keeps a fraction of a slot and gives the liveness cell back (question 238's shape): each amount is then within the slot's holding but the sum of two is not, and Silicon's own precondition check would fail (an exit 3); the sum form reports a `permission` error at the first such row instead, the limit question 238 lists (`rngc_reads_meet_kept`, `rngc_reads_meet_kept_local`). The sum is never smaller than one amount, so a caller that verified keeps verifying. A count argument that is exactly a bare parameter of the callee passes the bound count local too, like the base (N2). Messages name the range as the call wrote it (`range_call_text`:
  `a[0..3)` for `&a[0]` and a count that is a bare parameter or a constant, `a[i..i + n)` for numbers, `(base)[0..n)` otherwise; a generic phrase when an argument or a count
  has no spelling). *Item 10:* a range atom is a **memory call on its element field** (`memory_fields`), registered as an access atom with a count; the pair rows (`place_disjoint`:
  cell-cell, cell-range, range-range) cover access against call and call against call (`unsequenced`, 0039 item 4); a callee's `owns_range` reaches its field in `LoopWrites`
  (`writes_in`), and a loop in a function with no range contract that calls a range callee carries its array and has its values havoced. No program without a range atom
  changes: a range call's Viper differs from a cell call's by its temps only. Cases: `rngc_*` in `selftest_cases30.inc` (59), each positive with a vacuity twin; the pin of
  question 238's unequal fractions is covered end to end by `range_call_reads_unequal_pin.c`, not by a VIR case.
- **Loops over a parameter's range (Part D, record item 11; `loop`, `RangePlace`).** The staging gate (S2) is gone. A loop that carries places gets, for each `requires` range atom of a
  parameter, a **range place** (`range_places`): base `p`, bounds `offset_of(p)` and `offset_of(p) + n`, the count and the guard read from the FORMALS (the entry values: a loop that assigns the
  count or the pointer keeps the entry bounds; the pin `range_loop_pin_assigned_param.c`), the amount `write` for `owns_range` and the function's own ghost amount for `reads_range`
  (`range_row_amount`). The place's liveness is the parameter's existing live-only `Place` (`held_keys` lists the range keys), so its snapshot, back-edge row, entry case splits (S44 F-g1) and
  array/cell guards need no second carrier. Rows, all under the atom's guard: the **marked entry rows** at the loop keyword and the **marked back-edge rows** at the end of the body (value then
  flag, `permission`; in the sum form over the other read places of the field, as calls and returns have it), a `lh_<n>` label when ranges exist, and the conjuncts in this order (before the user's
  invariants, so a progress clause that reads the range is well-formed): the permissions, the flags kept (`old[lh]`), the type's range, the values kept when no write reaches the field or the place
  is read-only. **Beside other places:** every value place of the field that is not a local cell (a parameter, a global, a loaded or heap place) gains `!(g && inside the range)` on its value
  conjunct, and the entry splits `g && inside` against every value place of the field and `block_of(p) == block_of(a)` against same-field arrays are made before the snapshots. **Reads in
  invariants and measures** (`inv_elem`, `inv_read_at`, `inv_range_chain`): `p[e]` of any pointer and `q == a` (the decay `ElemAddr(AddrOf(a), 0)`, `elem_addr` admits `CMode::Inv`), and the element's
  address as a value (`q == &a[1]`), are accepted; a read is framed by a chain of disjuncts over the cell places, the range places and, for a local array, `array_place_hit` (block and `0 <= offset <
  count`), for the value and for the flag, and the framing assertion covers all of them; an unframed read is `permission` ("this loop invariant reads `p[e]`, which this loop does not hold"). A range atom
  written in an invariant stays rejected (M4.5e2, question 237). The encoder's side (Part D): `subscript_address` accepts `p[e]` of any pointer value in a loop clause (the index reads no memory,
  `index_reads_memory`), `array_decay` accepts the decay there.
- **Markers over ranges (Part E, record item 12).** `check_marker_atoms` takes `owns_range(p, n)` as the atom a marker needs and refuses a `reads_range` beside a marker, and `reject_marked_read` (from
  `contract_element_address`) refuses a contract read `p[e]` of a marked parameter as it refuses `*p`; `--strict-init` accepts an `uninit` local array (the decay) passed to a marked parameter as it
  accepts `&x`. In VIR, `check.cpp`'s `init_markers` counts `owns_range` as the atom (and refuses `reads_range` beside the marker and a read of `p[e]`). In lowering `site_marker` gives a range atom
  its parameter's marker: in the method's contract a marked `requires` range has no flags conjunct and the type's range under the flag; the `ensures` of a `ref_to_uninit` parameter says `old(flag) ==> flag`
  under the quantifier (the partner's guard in the entry state) and the range under the flag; `must_init`'s `ensures` is the plain atom. At a return (`range_returns`): `must_init` has a flags row of
  category `postcondition` ("returns without initializing every element of `p[0..n)`, which `cerastes::must_init` on `p` promises"), `ref_to_uninit` an `internal` row `old(flag) ==> flag`, and the range
  row under the flag for both. At a call (`call_atoms`): no flags row for a marked callee atom, the range row stays under the flag; a marked range site frames no pre-state contract read. A loop has nothing
  to add: its place's flags are `old[lh](flag) ==> flag` with the range under the flag, so no flag is assumed for a marked range.
- **`owning(n)` on a parameter (Part E, item 13).** `<cerastes.h>`'s `CERASTES_OWNING_N(n)` is `[[cerastes::owning(n)]]` (empty for other compilers). The argument's parameter references are `enk_param_ref {k,
  levels_up 1}`: `interpret_param_ref` reads them as parameter `k` for `Annot::owning` (P79 F4), `Ctx::owning_count` makes `param_reference` apply that rule. `read_marker` keeps the count (`OwningParam::count`) and
  every declaration must give the same one (`same_count_node`: shape, parameter positions, constants; otherwise `annotation_mismatch`, also when only one declaration has a count). `check_attribute` scans the
  argument like an annotation expression. `apply_owning` desugars to `p == NULL || owns_range(p, n)` first in the `requires` and the `ensures` (`owning_count` reads `n` by the rules of item 3: an integer other
  than bool, an enumeration, a float or a pointer, over earlier parameters and constants, reading no memory; a structure pointee is the range over structures of Part C1, below) and merges an explicit `owns_range(p, n)` of the same count (unguarded or
  after `p == NULL ||`); `owns(*p)`, an `owns_block`, a `reads_range`, another count or another guard beside it is rejected, and a range atom is never `implicit_atom`. `owning(n)` on a function, a local, a global
  or a field stays rejected (M4.5e2).
- **`restrict` (record 0064 item 19, question 326; replaces Part E's item 14 and records 0037 item 2, 0043 item 10).** Every `restrict` pointer parameter is rejected: `function_of` reads the qualifier where it always did (`signature_vtype(..., allow_restrict)` sets `VType::restrict_q` from the definition's own parameter variables and the declaration's qualifiers; C only, C++'s `__restrict` is `vtype`'s `unsupported_type`) and, once the contract is read, gives the function ONE `unsupported_annotation` at the position every earlier `restrict` rejection used (`at`: the definition's head, or for a bodiless declaration the first call that asks for it), naming every `restrict` parameter and telling the user to drop the keyword. A `restrict` written only on a prototype of a function the unit defines without it stays accepted, as before. A `pure` function's and a quantifier function's `restrict` parameter keep their own rejections (`pure_function_rules`, `read_forall_function`, texts pointing at item 19). `Param::is_restrict` stays only as the flag `check.cpp` refuses (a backstop for hand-built VIR, pinned by the cases `e1e2r_backstop_*` in `selftest_cases36.inc`). `lower.cpp` never read the flag, so no program's Viper changed. The pair rule, `restrict_backed`, `restrict_hazard`, `restrict_body_reason` and the encoder's `report_restrict_bodies` and `recheck_restrict_after_static_atoms` are gone. *Which declaration counts* (audit round R, should-fix 1, question 341): for a routine WITHOUT a body `declared_restrict_params` reads every declaration's own written type (the primary's `r->declared_type` and each secondary declaration's `declared_type`; the front end's merged type of two declarations that differ in a parameter's top-level qualifiers carries none, and a declaration keeps `int p[restrict]` as the array type whose own `variant.array.qualifiers` holds the keyword), and a parameter is `restrict` if any declaration gives it the qualifier: in C `function_of` adds it to the rejected parameters (the same tag, message and place as above), in C++ `check_signature` rejects it as `vtype`'s `unsupported_type`, as for a `__restrict` on the first declaration. A routine with a body keeps the definition's own parameters, so a `restrict` prototype before or after a plain definition stays accepted.
- **Array parameters (Part E, item 15).** `T a[]` and `T a[N]` are pointer parameters (the adjusted type); the front end keeps the array type in the parameter's `declared_type` (P79 (b)). `check_signature`
  rejects a VLA parameter on every declaration, bodiless included (`int a[n]`, `int a[*]`: `unsupported_declaration`, question 241, M8) and an array of arrays (`int a[][3]`, naming M4.5e1e2); a written bound is checked
  as a local array's is (`check_array_parameter_bound`, per parameter in `encode_body`, sharing `check_array_bound_constant` with the local form; in C++ the bound is `unsigned long` under a generated cast over the
  written tree, P79 F2, and a literal there has nothing to check). Two new diagnostic tags, defined in `diagnostics.def`: **`sizeof_array_argument`** (`reject_computed_sizeof_operand`: an `enk_sizeof` over a
  parameter whose `declared_type` is an array, `sizeof(a)` and `sizeof(a) / sizeof(a[0])`) and **`array_parameter_mismatch`** (`check_array_parameter_declarations`: the routine's declared type and each
  secondary declaration's, P79 F3; an array against another bound, a bound against none, a bound against a pointer, in both modes; `int a[]` against `int *a` is accepted); both follow the compilers'
  default or `-Wall` warnings (the Step 0 matrix, question 240).

**Quantifiers (M4.5e1c, record 0061).** `cerastes_forall(f, e_1, ..., e_m)` says that the body `E` of the `pure` function `f` holds for every value of its first parameter
(the bound variable), the other parameters being the arguments. The Viper spikes are `tests/viper-smoke/memory/s45_*` (S45 and its addenda in `internal/il-inventory.md`,
chapter M4.5e1c, with P80's IL facts). No Viper function is emitted for `f`: its body is read at each site (question 226), so a call of `f` elsewhere stays rejected until e1d.

- **The reader (encode.cpp, §8).** `f` is `f` or `&f`, a `pure` function (not a `predicate`) with a verified body that is exactly one `return E;` (an extra block is read
  through; C23 wraps the return in a generated `eok_bool_cast`), result `bool`, no contract, not a member, a template, a lambda or variadic; the bound variable has an integer
  type other than `bool` and the enumerations; the other parameters are `bool`, integers, enumerations or pointers to them. In `E` the first parameter is `Bound(id, type)` and every
  other parameter `ForallArg(id, k)`, with the encoded arguments in `Forall::args`; each argument is **evaluated once, at the site, before the binder**: `forall_expr` lowers the
  `args` first, so an argument's own obligations (an overflow in `n + 1`, a division, a conversion) are rows of the site that exist whether or not `f` uses the parameter, and
  an argument reads no memory. `check.cpp` (`forall_node`, `forall_shape`) re-checks what the encoder refuses, so the backstop is never a user's message.
- **Read classes, binders, triggers.** A *read class* is a pair (pointer argument, constant `c`) of the reads `p[i + c]` of `E` (`fq_collect`; the index is exactly `i`, `i + c` or
  `i - c`, no conversion). The first class's binder is `q0`, the absolute slot, with `i = q0 - offset_of(base_0) - c_0`; a further class `j` gets a binder `q<j>` with
  `q<j> == offset_of(base_j) + i + c_j` in the antecedent (T2); the antecedent also holds the bound variable's type range. The quantifier has **one trigger set with one term per
  class**, `loc(block_of(X), q<j>)` with `X` a variable (T1, one binder with computed terms, does not finish on neighbours such as `a[i] <= a[i + 1]`, S45 (b)). A listed limit
  (question 253): a multi-class quantifier is instantiated only where the program holds a term of every class, so a correct program that uses it where one class's term occurs
  alone fails: a false alarm, never a proof (`s45_b_t2_oneterm`; M4.5e2).
- **Contracts.** A clause that holds a quantifier is split per top-level conjunct (`push_clauses`, `split_conjuncts`); a guard stands outside (`g ==> forall ...`; `g || forall(...)`
  is `!g ==> forall(...)`, a quantifier left of `||` is refused, M4.8). Each read under the binder is its own `ite` chain over the framing disjuncts of the atoms seen so far, each with
  its atom's guard (0037 item 7, 0060 item 5); a **marked** site (`must_init`, `ref_to_uninit`) frames no pre-state read, nor a post-state read where the parameter is
  `ref_to_uninit` (0042 item 6). `cerastes_old(p[e])` under the binder is the read in the entry state, in its own chain, and is accepted in an `ensures` only.
- **Calls and returns.** At a call, one `precondition` row **per conjunct** of a quantified clause, after the atom rows, in the caller's state; `call()` binds every pointer a callee's
  quantifier subscripts to a local first (the range atom's local, else a variable argument as is, else a new local), so the trigger term `loc(block_of(X), q)` has a variable `X`.
  At a return, one `postcondition` row per conjunct. The Viper call's own precondition stays the `internal` backstop.
- **`f__wf`, variant M (`wf_method`).** The method stays heap-free. A read under a binder is the ghost function of its base formal and state at the read's absolute slot,
  `hvq_<method>__<formal>_<pre|post>(s)`, typed `Int`, `Bool` or `Ref` after the element, in one domain `Hvq_<method>` per `__wf` method with no axiom (tagged
  `// cerastes:domain-ok 0061`; Viper names are program wide, hence the method prefix, `kHvqUniqueNames`, question 255). Assumed beside today's `facts_`, each with one trigger
  set per ghost application it relates: **(a)** the element type's range per state; **(b)** the scalar tie, ground (`hv_k_<st>` of `p[e]` equals `hvq_p_<st>(offset_of(p) + e)`);
  **(c)** two parameters on one block with the same cell field have equal ghosts; **(d)** a `reads_range` atom with an `ensures` partner keeps its value, under the conjunction of
  the two atoms' guards (the unguarded form is unsound, S45). (a) to (c) are emitted only for a formal that has an atom of that cell field (`has_atom_field`: a formal with no atom,
  such as `q` in `requires(q == p)`, gets no range fact and keeps its framing row; the lint reads the element field from the atoms). Each obligation of `E` is one row `prior ==>
  forall q<j> :: { hvq... } range(i) && path ==> obligation` over its own class's binder, the `prior` outside, reported at the site with the message "in `f`: ..." and a note at the
  operation in `f` (question 247). A class read in both states (`a[i]` and `old(a[i])`) has one single-application trigger set per state on its clause and rows (question 257), so a
  later obligation that names one state is instantiated.
- **Loops (`CMode::Inv`; `fq_loop_read`).** A loop invariant clause that holds a quantifier is split per top-level conjunct into invariant lines of their own, each keeping the clause's
  line and category; **in a loop invariant a quantifier is a top-level `&&` conjunct only** (question 262: under `||`, in a `?:` arm or inside a guarded conjunction `g || (A && forall)` it
  is refused, M4.8, because `split_invariant` splits only top-level `&&` and the other forms emitted Viper the lint refuses). For each read class, before the user's clause and in evaluation
  order: the framing `assert` against the loop's places (`permission`, "this loop invariant reads `a[i]`, which this loop does not hold", at entry and at the back edge, as
  `framing_condition` does for a single read; `inv_reads_` entries carry `fq_class`), then, for a named array, an invariant line for the bound (`out-of-bounds`) and one for the flag
  (`uninitialized`), the flag line's read taking the array's bound in its antecedent (N14). Every such line and framing assert is guarded by the earlier invariant clauses in evaluation
  order, quantified ones included (`obligation_clause`, question 258; the lint admits those conjuncts in that guard). A read not indexed by the binder has its own framing and flag rows. A
  pointer variable (not address-taken) or a named local array is the argument; for an array the argument is `AddrOf(a)` (its decay collapsed to the array, so `inv_elem` and the array rows
  match) and an element read is `ElemAddr(AddrOf(a), i)`. A loop whose invariant holds a quantified read carries its places (`expr_touches`). Where an element's flag and value are both
  unknown, a body that skips the write fails the flag line and `invariant-preserved`: two errors (question 254). **Several read classes** (question 261): the obligations of arithmetic are
  rows of the first class's binder, so arithmetic (a binary operator that is no comparison or connective, a unary operator, a conversion) over the read of any other class is refused
  in a loop invariant and a `check` (M4.5e2); comparisons of plain reads and arithmetic on the first class's reads stay accepted.
- **Checks (`CMode::None`; `fq_check_rows`).** A `check` holds a quantifier as a top-level `&&` conjunct only (question 262, as for a loop invariant), and the check is split per conjunct.
  Per read class the rows come in this order: for a pointer base the null row and the liveness row (`kq := perm(lvloc(...).lv)` bound first, then `forall q :: reach ==> kq > none`) (a
  named array has no null or liveness row), the bound of a named array, the marked rows `forall q :: { loc(B, q) } reach ==> acc(loc(B, q).v_T, wildcard)` for the value field and then the
  flag field (`permission`; `perm` may not stand under a quantifier in an `assert`, `acc(.., wildcard)` may), the flag row (`uninitialized`), then `E`'s own obligations over the binder
  with `E`'s path, the index's own obligation (`i + c`) among them, then the fact (`check`). A failed marked row ends its path. The question 261 limit for several read classes applies
  here too.
- **Listed limits.** Left to M4.8: a quantifier under `!`, as an operand of `==` or arithmetic, in a `?:` condition, an atom's guard, a measure, a `do ... while (0)` invariant or on the left
  of `||` (questions 249, 255); in a loop invariant or a `check`, a quantifier that is not a top-level `&&` conjunct (under `||`, in a `?:` arm: question 262). Left to M4.5e2: a body with no read indexed `i`, `i + c`, `i - c`, an index of another form or under a conversion; in a multi-class quantifier a guard that
  reads memory and arithmetic over two classes' reads (255); in a loop invariant or a `check`, arithmetic over the read of a class other than the first (261); `&a[1]`, `p + 1`, a loaded or address-taken pointer as the base; in a loop or a `check`, `*p` of an array argument and a pointer
  argument used as a value; in a `check`, a read the binder does not index and pointer elements (259); an argument that reads memory; `cerastes_old` in a loop invariant (cut 1, question 260)
  and in a `check`; structure members and global arrays under the binder (M4.5e1e2, Part F). A `pure` function with a contract or a `predicate` named by `cerastes_forall` is e1d's and M4.5f1's.

**Pure functions (M4.5e1d, record 0062).** A `pure` function that an annotation applies, or that carries a `requires`, an `ensures` or a `decreases`, is **read** (record item 2) and
lowered to a Viper function with its contract and a definition check; an application of it is a Viper application of that function. The Viper spikes are `tests/viper-smoke/memory/s46_*`
(S46 and P81 in `internal/il-inventory.md`, chapter M4.5e1d).

- **The Viper function (`pure_function`, `pf_lines`).** Named `pf_<name>__<id>` (`<id>` the VIR function id, digits only: question 274). Formals as a method's `p_<name>__<k>`, no ghost amount;
  `// cerastes:result <lo> <hi>` after the signature states the C result type's range (the lint checks fact (i) against it). `requires`: the parameters' type ranges and each pointer's shape
  line, then the user's clauses in order: a value clause as a method's contract has it; an atom in the **minimal shape** (S46 (a), form Min): its pure lines (`p != null`, the shape,
  `0 <= n`, `offset_of(p) + n <= len(block_of(p))`, one `requires` per conjunct) and the permission `forall k :: { loc(block_of(p), k) } in-range ==> acc(loc(block_of(p), k).v_<t>, wildcard)`
  (a cell atom: the three lines for one slot). No `acc` of the flag field or the liveness cell and no flag line, and no line that states the result's range (callers learn a bound from the
  function's `ensures`; `__wf` assumes the range). `ensures`: the user's clauses, one line each, `result` for the result. `decreases`: the user's measure, or an **empty `decreases` line** for a
  function that does not call itself; never `_` and never no line (with none Silicon checks nothing, S46 (a)). The body is `E`, lowered in the function's own contract form (one state, the entry
  state with every `requires` site in view); **every application of the function itself begins a line of its own** (`LFunction::body_calls`, the emitter breaks the line: the map is keyed by line).
  `pf_lines` is the one list that is the function's own `requires`, the guard `G` and the condition `V` of fact (ii): the guard can never be stronger than the rows assert.
- **The prelude** prints an empty `decreases` line on every prelude function **in a program that holds a `pf_` function, and only there** (S46 (a): an application of a prelude function from a
  function that has a `decreases` line needs one), so no existing program's Viper changes (`prelude_text(used, empty_decreases)`).
- **The guard `G` and the rows at an application** (item 7). A guard is the conjunction of (i) for each atom of the callee the disjunction, over the enclosing contract's unmarked sites of that cell
  field, of "the site's guard, the same block, the callee's interval inside the site's" (one site must hold the whole interval: a listed limit; a marked site frames no application) and (ii)
  every other line of the callee's precondition that reads no memory, at the arguments by substitution (`apply_conjuncts`, `apply_guard`, `apply_frame`). Per mode (`apply_node`):
  **Method** (a method's contract, a call, a return: the clause is lowered again over the arguments) and a `pure` body: `(G ? pf_g(args) : d)`, `d` the type's default, so Viper's own check of the
  contract or the function can never fail on it; the obligations are the `__wf` method's rows. **`Wf`**: the application is a ghost function `hva_<m>__<g>_<pre|post>` (below) and each conjunct of `G` is
  a row `prior ==> conjunct`: a **frame** conjunct is `permission` ("calls `g`, which needs to read `a[0..n)`, without the permission to read it"), an atom's pure line or a value clause is
  `precondition` with a note at the callee's clause, a range or a shape is `internal`. **`None` (a `check`)**: first the callee's parameter ranges and pointer shapes at the arguments (`internal`),
  then per callee atom its pure lines (`precondition`; question 278: first, so a count past an array is one error), then 0061 item 10's rows over its interval in the binder form (the null and liveness
  rows for a pointer base, the bound of a named array, the marked wildcard rows `v_T` and `d_T`, the flag row, then the element-range row, `internal`), then one `precondition` assert per value clause,
  then the fact with the bare application (a failed row is assumed afterwards, so Viper's own `application.precondition` is silent). A `cerastes_forall` in the callee's `requires` (cut 2) is, at an
  application, the quantifier it would be written at the site with the arguments in place of the parameters, lowered by 0061 in the site's mode, one `precondition` row and one more conjunct of `G`.
  **`Inv`** (loop invariants): the next bullet.
- **Loop invariants (`CMode::Inv`, `apply_inv`; Part D, owner questions 283 and 284).** An application in the invariant of a `while` or `do ... while` loop is lowered, before the user's clause and in
  evaluation order, as rows on the loop's own places: **the framing** of each callee atom, that its interval lies inside one place the loop holds (a range place or an array place; no quantifier), asserted at entry
  and at the back edge as a read's framing is (`permission`, a message of the contract's form with the interval, `a[0..i)`, named as the loop's own place); then one `obligation_clause` invariant line per
  obligation, in the order `apply_check` gives a `check`: **per callee atom its pure lines as `precondition` rows** (question 283: `p != null`, the shape, `0 <= n`, `offset_of(p) + n <= len(block_of(p))`, with the note
  at the callee's clause; the parameters' ranges and pointer shapes stay in the guard only), a named array's bound (`out-of-bounds`), the flags over the interval in 0061's `q0` form, guarded by the framing
  (`uninitialized`), and each value clause of the callee (`precondition`). The application inside the clause is **guarded** by the same inclusion and value clauses, `(G ? pf_g(args) : d)`, as `inv_elem` guards a read, so
  Viper's own check of the clause never meets an application outside the callee's precondition; but where the guard is false the clause is read with the default `d`, and a clause that the default breaks
  (`s == 0` while `s` grows) fails as `invariant-preserved` beside the framing row: the guard hides the native error, not the user-visible one. A loop whose invariant holds an application of a function with an atom carries its places (`expr_touches`). An unframed application is more than one error (S46 (d), question 277:
  the framing assert at entry and at the back edge, the callee's pure line, and where the default breaks the clause `invariant-preserved`); the tests expect every one. A quantified clause of the callee's `requires` (cut 2) is
  the quantifier it would be written at the site, as 0061 item 9 lowers a loop's quantifier (a top-level conjunct invariant line with its framing and flag rows). Two arrays with a pair of places need the entry case split
  `if (lvloc(block_of(b)) == lvloc(block_of(a))) { }` before the snapshots, as 0060 item 11 has it. A pointer argument that an atom names is a pointer variable that is not address-taken or a named local array, as in
  a `check`; a callee with an atom over pointer elements is refused in an invariant (R3-S2, M4.5e2). **The invariant of a `do ... while (0)` loop** (`do_once`, no loop, question 284): an application there is checked like
  a `check` (`CMode::None`: the same rows, then the fact asserted as `invariant-entry`); `check.cpp` treats that invariant as a check for an application.
- **`f__wf` for a read function and for contracts with applications** (items 8 and 9). Every read function has its definition-check method `pf_<name>__<id>__wf` (`wf_method`, whether or not it has a
  contract): the `requires` clauses, then **the body as one more clause of the entry state** (every obligation of `E`: the operations' ranges, each read's framing by the function's own atoms, each
  application's rows, the conversion at the `return`, reported in the function at the operation), then the `ensures` clauses' own obligations, then the measure's. Silicon's own check of the Viper function
  proves the `ensures` and the termination; those two are reported from it (the two native rows, §5), and anything else it reports is an internal error. An application in a contract is a **ghost
  function** of the method's domain `Hvq_<m>` (`hva_<m>__<g>_<pre|post>(args)`: one parameter per C parameter, `Int`, `Bool` or `Ref`, the callee's result type; `<m>` the `__wf` method without `__wf`,
  `<g>` the callee's full Viper name; `LGhostFn` has typed parameters), with no axiom, and the assumed facts are: **(i)** the result's type range (an integer result only, unguarded); **(ii)** the callee's
  `ensures` at the arguments **under `V`**, the callee's value `requires`, pure lines and quantified clauses at the arguments (the guard is what makes it sound: an `ensures` outside the callee's `requires`
  may be false, S46 (c) (2u)), for a clause that reads no memory and holds no quantifier or application; **(iii)** the keep fact (`wf_keep_facts`): for a post-state application whose arguments read no memory and
  whose every callee atom is on the same cell field with a bare-variable pointer argument, under both atoms' guards, the same block and the inclusion of the callee's interval in the `reads` atom's,
  `hva_..._post(args) == hva_..._pre(args)`. Listed limits (false alarms, fail closed; question 279, M4.8): fact (i) is not written for an enumeration result, fact (ii) not for a callee whose `requires` holds an
  application, and the definition of `g` is not known in `f__wf`.
- **Termination** (item 10): the encoder records the applications among read functions, from bodies and contracts; a self-application in a body needs a `decreases`, every other cycle is rejected
  (M4.5f2); Silicon's termination plugin checks the function against its measure; a failure is `termination` at the recursive call. An application is not a call of a method, so `compute_cycles` is unchanged.
- **The definition check's argument** (audit round 1 checks it): by induction on the measure, the Viper body computes in unbounded integers, the `__wf` rows show under the `requires`, with every inner
  application's `ensures`, that no operation of `E` overflows, divides by zero, converts out of range or reads what it may not; so the two values agree.

**Arrays of scalars in structures and static storage (M4.5e1e1, decision 0063; facts P82 and S47 in `internal/il-inventory.md`).** Global and `static` local arrays, array members
(in every storage a structure has), array compound literals in C, array types the front end computes, and structure values with an array member. The encoder's half is
`encode.cpp` (below, "Arrays in structures and static storage"); this is what lowering does with what it hands over.
- **Layout** (item 3). An array member is a block of its own, `member(X, k)` of length N, whose element `i` is `loc(member(X, k), i)`; it shares its root's liveness cell. A global array
  is the block `GB_g()` of length N. Both are hypotheses discharged by an allocation, by `program_start` or by a caller, never axioms. `alloc_struct_<n>_<tag>` ensures
  `len(member(X, k)) == N` and, per array member, three quantifiers over `[0, N)` (trigger `loc(member(X, k), j)`): the value permission, the flag permission, every flag false; it is
  emitted for every structure type with an array member, called or not, so the lint's count of array members has its pinned source. `struct_lens` (`WF_S`) states `len == N` for an
  array member where it states `== 1` for the others.
- **The bound local** (item 5, S47 (a)). Where a row ranges over an array's slots in code (scope exit, loop entry and back edge, a call's and a return's marked rows), the first slot is
  bound once to a Viper local and e1a's and e1b's rows run over that local unchanged: `ga_<name>__<id>` for a global array (the Viper name of the block without `GB_`, assigned
  `loc(GB_g(), 0)` in every method of a unit that has an array global, after `program_start` in `main`), a temporary `t__<n>` for a member (`t__n := loc(member(X, k), 0)`; a root that
  is not a Viper variable is bound to a temporary first). A marked row's block is thereby `block_of` of a variable. Where there is no local (a method's contract, `alloc_struct`'s
  `ensures`, `program_start`, the global facts, `struct_lens`) the rows are in the block-term form `loc(member(X, k), j)`, `loc(GB_g(), j)`, and the first slot's offset is folded to 0.
- **Elements.** `g[i]`, `s.a[i]`, `p->a[i]` are `ElemAddr` over the array's address. A global array and an array member are *named* bases: no null or liveness row of their own (for `p->a[i]`
  the root's `null-dereference` row once, and the access is no use of `p`'s value: the place mark passes through `MemberAddr`). Then 0059 item 4's bound (`out-of-bounds`), `permission`
  and, for a read, `uninitialized`.
- **Atoms and calls** (items 7 and 8). `owns(g)`, `reads(g)` (written or the implicit atom of a `static` local array) are e1b's range atoms over the whole array
  (`OwnsRange`/`ReadsRange`, base `loc(GB_g(), 0)`, count the literal N), so 0060's amount, shape, marked, flag and sum rules apply as written. `owns(*p)` of a structure with an
  array member is the cell leaves' lines plus, per array leaf, four lines over `loc(member(X, k), j)`; `owns(p->a)` is that leaf alone (a plain atom whose place is a `Ptr(Array)`).
  Overlap is by slots: an array leaf is a range of N slots at its first slot, for every rule that pairs two sites, and a pair of array leaves of one structure type at one index is
  separated by the first-slot inequality `t1 != t2`, **never a bare block comparison** (`member(p, 0) != member(q, 0)` is not decided by Silicon from quantified permissions: the audit's
  p10a). A `reads` site's amount at a call is the minimum over its cell leaves and, per array leaf, the first slot's (0060 item 6).
- **`program_start`** (item 9). The row group of an array global, in this order, each row its own `ensures`: the two quantified permissions over `[0, N)`; the flags, every one true; the
  values (known: one conjunct per listed element and one quantifier for the zero rest; unknown: the element range, or `WF`, as one quantifier; none for a `bool`); the liveness cell. A
  structure global's array leaf has the same group over `member(<chain>, k)` rooted at `loc(GB_g(), 0)` (no liveness cell of its own), each (receiver, field) pair given once.
- **Loops** (item 10). Array places of a loop with 0059 item 8's rows over the bound local: every global array in `main`, those a callee's `requires` names elsewhere, every array leaf of a
  structure object in scope, and each array part of a held structure place. A store by name to an element of a global array reaches that array only; a store to a member's element
  through a pointer, or a call that owns a cell of the field, reaches every array place of the element field (0043 item 11; the listed false alarm N9).
- **Zero fill and copy** (items 6 and 13). `ArrayZero` is `zero_arr_<t>(first_slot, N)` after binding; `ArrayCopy` emits the rows in S47 (g)'s order and then `copy_arr_<t>(d, s, N, rd)`:
  `rd` is a `Perm` local, the source's first-slot amount (the least of value and flag permission), `none < rd` (`permission`: "copies `s`, whose `s.a` the function may not own or may only
  partly own"), the source's marked rows (value, then flag) at `rd`, every source flag (`uninitialized`, "copies `s`, whose member `s.a` may hold an element without a value": question
  296), the destination's marked rows at `write`. Then the call: none when both roots are the same named object (`t = t`), unguarded between two named objects (`AddrOf` or `GlobalAddr`
  roots, which Silicon proves distinct; a guard `d != s` there would be undecided from quantified permissions and leave a branch that copies nothing), and under `if (d != s)` where
  either root is reached through a pointer (an exact self-copy does nothing). The `internal` row `block_of(d) != block_of(s)` precedes every call, so an omitted guard is a tool failure,
  never a proof. `copy_arr_<t>` is trusted (the loop `d[k] = s[k]` over two distinct blocks): it takes and returns full permission to the destination's value and flag fields over N slots and
  the amount `rd` of the source's, and ensures each destination value is the source's old value, each destination flag is set, each source value is unchanged and each source flag stays
  set. The listed false alarm: between two `owns` parameters whose leaves of the field are all array members the guard is not decided.
- **Structure values** (item 13). A structure value is the tuple of its value-view leaves: an array member contributes one formal, result or temporary per element, named
  `p_s__<k>_m<j>_e<i>` and `res_m<j>_e<i>` (`value_suffix`). A parameter's copy block is written from the formals at entry, an object argument and `return s;` read every element, an
  `Aggregate` carries one argument per element. `ElemValue` is an `ite` chain over the member's leaves (the last the default) with one `out-of-bounds` obligation `0 <= b < N` that comes
  before every use; it works in code (a call's result leaves) and in contracts (`Formal`, `Result`, `Old`). A struct store (`struct_store`, `struct_cell_write`) writes `loc(member(X, k), i)`
  per element, with no bound row (the element is inside by construction).
- **Rejected** (each a pin, fail closed). For a structure whose array members are arrays of SCALARS, Part D (M4.5e1e2c, decision 0064 item 15) lifted what this bullet listed since M4.5e1e1: heap storage (`malloc`, `calloc`, `free`, `owns_block`; `calloc` of such a structure with more than 24 zero values stays rejected: the cap, question 354), an array member beside an `owning` or marked member, a marker on a pointer to such a structure, a contract place through a heap read, a partnerless `ensures` atom, and a written `owns(*p)` or `owns(p->a)` atom in a loop invariant. Still rejected: a written `reads(*p)` or `reads(p->a)` of an array part in a loop invariant, and a written atom of an array part through a load, a global pointer, an address-taken pointer or a member of a by-value member, under a guard that reads memory, or `owns(p->a)` of a structure that also has an aggregate array member (M4.5e2, item 15): `invariant_atom` and `check_invariant_atoms` reject each at the invariant's C line with `unsupported_annotation` before `check.cpp` sees it (audit rounds D1 and D2); a partnerless `ensures` atom on a formal of a bodiless function (0052 item 2a, question 350); a `ref_to_uninit` parameter with no `owns` in the `requires` (0042 item 4). **A structure whose array member is an array of structures or of arrays (question 348, fail closed) stays rejected at each of item 15's six positions** (`malloc`, `calloc`, `free` and `owns_block` of it; an `owning` or `ref_to_uninit` member beside it; `ref_to_uninit`, `must_init` or `owning` on a pointer to it; a place of it reached through a heap read; an `ensures` atom on it with no partner; a written atom on it): the encoder rejects at the C line, `check.cpp` refuses and lowering stops with an internal error behind them (M4.5e2). Also rejected: a range or an atom on part of a global array or member (M4.5e2) and a structure value of more than `kMaxByValueArrayElements` array elements (M4.5f3).

- **The global facts as clauses (M4.5e1e2 Part C2, audit round C2b; decision 0064 items 3 and 9; `global_fact_clauses()`).** A unit with globals states the facts of its globals (liveness cells, block lengths, the slot facts of a structure global, `WF_S`) as one conjunction, and a length quantifier that `struct_lens` produces for a member that is an array of structures, or for an array global of structures, is NOT part of that conjunction: `global_facts(&quants)` returns the conjunction and appends each such quantifier to `quants`, and `global_fact_clauses()` returns the conjunction (when it is not null) followed by one clause per quantifier. Every place that states the facts (the `requires` and `ensures` of the methods that hold the globals, `program_start`'s postcondition) takes the list of clauses, so each quantifier is a clause of its own. The reason is the shape the Viper lint expects (a clause of its own, after the facts' clause; `scripts/lint_viper.py`); it has a measured cost that question 347 records: a function other than `main` with `requires reads(gt) && gt.v[1].x == 3` that calls a `reads(gt)` function and checks that leaf verified in 17 s with the quantifier inside the conjunction and gets no verdict in 200 s with clauses of their own (a timeout is exit 3, never a proof; the family was already fragile: the two-call chain over such a global had no verdict either way), and its programs are named in the handoff for whoever revisits the shape.
- **The binder of a length quantifier (decision 0064 item 3, "A length quantifier under another quantifier"; `length_binder(X)`).** A quantifier over the slots `loc(member(X, j), k)` of a member that is an array of structures names its binder `k` where the slot expression `X` does not mention `k` (so every program that needed no other name has the Viper it had), else the first of `k1`, `k2`, ... that `X` does not mention, decided by a structural walk of `X` for a free variable of that name (`lexpr_free_var`). `struct_lens`, the global facts and `alloc_struct_<T>`'s member-array block share the helper, so every caller of `struct_lens` has the rule through it. The outer binder keeps its name inside `X`; the new binder occurs only as the index of `loc(member(X, j), .)` and in its bounds. Without the rule the inner quantifier captured the outer binder (Silicon: "duplicate identifier", an exit 3, never a wrong proof).
- **Part D: the six positions for a structure whose array members are arrays of scalars (decision 0064 item 15; encoder half, `check.cpp` and lowering are the il-analyst's).** Question 348: item 15 and the plan's Part D table were written for `struct SA { int n; int a[4]; }`; since Part C2 a structure may hold a member that is an array of structures (and from Part E one that is an array of arrays), and each gate tested "has an array member" whatever the element. The encoder's gates now test **one predicate, aggregate array: a member, at any depth of the by-value nest, that is an array whose element (typedefs skipped) is a structure or an array** (`vir_type_is_aggregate_array`, `vir_struct_has_aggregate_array`, `vir_type_has_aggregate_array` over the VIR types of a pointee; `edg_type_has_aggregate_array` and `pointer_to_aggregate_array_struct` over the front end's types, one pointer level, for a structure that may still be provisional inside `struct_info`). The gates that took `vir_type_has_array`, `edg_type_has_array` or `pointer_to_array_struct` now call it, each keeping its tag and position, with a message that names decision 0064 item 15 and question 348 and says the general form is M4.5e2's: `read_marker` (`owning`, `ref_to_uninit`, `must_init` on a pointer parameter), `apply_function_markers` (`owning` and `ref_to_uninit` on a function's pointer result), `read_variable_markers` (a local, a global, a `static` local); `allocation_view` (`malloc`, `calloc`), `heap_call` (`free`), `spec_block_atom` and `invariant_block_atom` (`owns_block`), with `heap_view`'s internal error kept for an aggregate pointee only; `struct_info`'s three sites (an `owning` member and a `ref_to_uninit` member whose pointee is such a structure; `first_aggregate_array_at`, an aggregate array member beside an `owning` or `ref_to_uninit` member, at any depth); `reject_array_behind_load` (a place of a contract through a heap read whose path holds such a structure, `p->q->v[e].x` included); `finish_atoms` (an `ensures` atom on it with no `requires` partner); `invariant_atom` (a written atom). The predicate is applied only at these positions: everything Part C2 accepts for such a structure keeps its outcome. For an array member of scalars the encoder accepts every position and builds no new VIR except one: `heap_view` states `calloc`'s zeros through the object view (`struct_object_leaves`), a cell leaf as before and an array leaf element by element (`result->a[0] == 0 && result->a[1] == 0 ...`, `ElemAddr` over the member's address, the index a constant), because VIR has no quantifier over an array member's elements outside `forall` of a `pure` function; the permissions of the array leaf are the lowering's (`owns_block`). **The cap (question 354, settled with the advisor; fail closed).** A view with many zero clauses ends without a verdict: on the x86-64 box (lane edg at the audit snapshot, both modes) `struct S { int a[N]; }` with `calloc(1, sizeof(struct S))`, one element checked and `free`, verified for N = 8 to 50 (12 s at 16, 25 s at 32, 76 s at 48 with a cell beside the array) and stopped for N = 52 and above with `Silicon did not produce a verdict (exit status 1)` (a JVM stack overflow in the view's own `__wf` method: depth, not time); with cells beside the array the time grows faster (`int a[30]` and 60 cells: over Silicon's 300 s), and two arrays of 24 verify, of 32 do not, so the total of clauses is what counts (a structure of 100 cells and no array verified, in 153 s). `allocation_view` therefore rejects a `calloc` of a structure that has an array leaf and whose view would hold more than `kCallocClauseCap` = 24 value clauses (about half of the largest passing count, for the margin of a different stack depth on macOS), at the call, `unsupported_expression`, with a message that names decision 0064 item 15 and M4.5e2 and says to allocate with `malloc` and write each element before it is read. The count (`calloc_zero_clause_count`) is the clauses `heap_view` writes: one per cell leaf and one per ELEMENT of every array leaf of `struct_object_leaves`, by-value members included. A structure with no array leaf is not capped (its view is what 0047 built). `malloc` and `free` have no cap: their views hold quantified lines per leaf, no clause per element. The message promises no more than is true: a `malloc`ed structure's elements that only a LOOP wrote read as `uninitialized` today (the loop's automatic conjuncts do not say which flags it set: probe d/fill_read.c, a false alarm, fail closed), so the advice is to write each element that is read. Heap views of an array of `bool`, of an enumeration, of pointers and of two array members are accepted and verify (build/m4.5e1e2c/views/). `owning` on a parameter and on a local pointer to such a structure, `ref_to_uninit` on a local and `owning` on a global need no lowering rule beyond the existing `atom` (their Viper is `owns(*p)` and the liveness fraction); the encoder accepts them and the lowering half below carries them through loops.
- **Part D, the lowering half as landed (decision 0064 item 15; `src/vir/lower.cpp`, `check.cpp`; the il-analyst's, recorded here for the encoder's reader).** (1) **The heap block's array part as a loop place.** A scalar array member of a block that a loop touches is a `HeapArr` place: a bound local `loc(member(hb, j), 0)` (Silicon's trigger needs a variable), the first slot's permission amounts as ghost snapshots (S48 (k)), a guard (the pointer is non-null and the part is no slot of another place of its field), entry marked rows per part (a slot a callee kept is a `permission` at the loop), conjuncts for permission, flags, the type range and the values kept, and back-edge marked rows placed before the cells' rows. The place exists only when `array_block_possible` holds: some function of the program has an `owns_block` site, an implicit atom (an `owning` parameter's merged borrow) or an `owning` local, global or member whose pointee holds the structure at any depth; when it is false no pointer can point at a token-carrying block of such a structure, so the gate only ever loses a place (a false `permission`, never a proof), and every older program has the Viper it had (neutrality: every pre-session lowering case is byte-identical). A cell-less structure (`struct SO { int a[4]; }`) gets a value-less liveness-and-token place. (2) **The owner and marked-closure carriers in loops** (`owner_invariants`, `owner_rows`, `owner_flag_rows`, `flags_kept_*`, `wblock_*`) walk the object view: the array parts of an owner's block are `g ==> forall k :: acc(v)` and `acc(d)` at full permission under the owner's guard, the flags under the same guard, and the marked rows per field over a bound local. A written `owns_block` site read through loads names its parts by a ghost pointer `abp_<i>__<loop>`; when the loop may write the pointer (a reassigned owning global) the ghost is TAKEN AGAIN at the back edge, `if (g && p != null) { abp := p }`, after the site's rows and before the invariant is checked, so a block that the body ends with is asserted whole at its own pointer. The owner carriers state no flags for a part, as they state none for a cell, so a read after a loop that carries the owner is a false `uninitialized` for a structure with an array member and for one without (the guide `v1b_noarray.c`, `u1b_noarray.c`). (3) **Written atoms in a loop invariant:** `owns(*p)` with `p` a pointer variable that is not address-taken, and `owns(p->a)` with one member step, on a structure whose array members are scalars, at `write` (for `owns` the first slot's permission and `write` are one value once the entry row has asserted it, so no `Perm` local); each array leaf is a written part (`WArr`) with its entry and back-edge rows, pair rows ("owns A and B, which may overlap"), case splits against every other place of its field and per-part invariants (the values kept only when the pointer is unmoved and no write reaches the field). The `reads` forms, a load and an address-taken pointer, and an aggregate element are refused by `check.cpp` with texts of their own (and lowering stops behind them). **Limits (all fail closed):** a user invariant that reads an element of a moved written part or of a calloc'ed block is a false `permission` (no framing place); the owning-member reassignment shape (`r->q = calloc` in a loop) gives false `invalid-free`, `leak` and `permission` rows, as it did before this session for a member's closure; a loop over a pointer that a callee gave by a partnerless `ensures` atom has no place for it in a function that is not a heap function (a false `permission`, pre-existing for every structure); heap array parts have no splits against a holder's array (completeness only).

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

A read `pure` function (M4.5e1d) is a Viper `function` printed before the methods, with its own definition-check method:

```
// C/C++ function `tri` (a `pure` function, record 0062)
function pf_tri__0(p_n__0: Int): Int // cerastes:result -2147483648 2147483647
  requires <the parameters' ranges, the pointer shapes>
  requires <the user's requires, an atom in the minimal shape>
  ensures  <the user's ensures, `result`>
  decreases p_n__0                                         // or an empty `decreases` for a function that does not call itself
{
  <E: an application of itself begins a line of its own>
}
// well-formedness of the contract of C/C++ function `tri`
method pf_tri__0__wf(p_n__0: Int, res: Int)                // the body, the ensures and the measure as obligations (domain Hvq_pf_tri__0 above it, if it applies a function)
```

**Memory prelude** (M4.5b1a; only when the program has a pointer type or a cell, so every other program keeps its
Viper): `field v_<t>` and `field d_<t>: Bool` per cell type, sorted, and `field lv: Bool`; `// cerastes:domain-ok 0028`
and `domain Block` with `loc`, `len`, `block_of`, `offset_of`, `member`, `parent_of`, `index_of`, `lvloc` and the four
axioms, each after `// cerastes:axiom-ok 0033`…`0036` (`lint_viper.py` pins the domain, the names and the texts);
then one trusted method per cell type, `alloc_<t>() returns (r: Ref)`, whose `ensures` are 0032 item 4's (its map
rows at the first cell of that type). `<t>` is `bool`, `schar`/`uchar`, `short`/`ushort`, `int`/`uint`, `long`/`ulong`,
`i128`/`u128` (else `i<bits>`/`u<bits>`), or `enum_<n>_<name>` for an enumeration (`_e` appended when the name would end
in `__<digits>`). One pair per VIR type: `long` and `long long` share one (0032 items 3 and 5 say why that is sound).
A program with an array local (decision 0059) also has `alloc_arr_<t>(n: Int)` (and, for an initializer that leaves elements out, `zero_arr_<t>(r, n)`)
for each element type it uses: trusted, tagged like `alloc_<t>`, ensuring the block's `len == n` and two quantified permissions over `[0, n)` (§3,
"Arrays"); `lint_viper.py` pins their contracts beside `alloc_contract`'s `len == 1` rule.
A program with an array member also has `alloc_struct_<n>_<tag>()` (§3, "Arrays of scalars in structures and static storage"), and one that copies a structure with an array member has
`copy_arr_<t>(d: Ref, s: Ref, n: Int, rd: Perm)` per element type, trusted and pinned by the lint (8 `requires`, 8 `ensures`, `decreases _`).
With globals (0041 item 4), the domain also holds one `unique function GB_<name>__<id>(): Block` per global and
`static` local (the text is unchanged without them), and a trusted bodyless `program_start()` follows the `alloc_<t>`
methods in every unit with a global: its first `ensures` row is the global facts (an array global's `len(GB_g()) == N`), then one row per global, positioned at
its declaration (`acc` of the value and flag cells, the flag, the value or a range/`WF` row for an unknown one, `acc` of
the liveness cell; an array global's row group is the two quantified permissions over `[0, N)`, the flags, the values, the liveness cell, each its own `ensures`, and a structure global's
array leaf has the same group over its member block), ending in `decreases _`; `lint_viper.py`'s `lint_globals` pins it.
A heap program (decision 0047) adds `field tok: Bool` and its views, bodiless methods tagged with vlibc's reason
followed by `; typed view: one <T> (<n> bytes), decision 0047`.

Names: every identifier has a prefix that says what it is: functions `f_<name>__<id>` (and `...__wf`), the Viper functions of read `pure` functions `pf_<name>__<id>` (and `pf_<name>__<id>__wf`; `<id>` digits only), the ghost functions of an application `hva_<m>__<g>_<pre|post>`, formals
`p_<name>__<k>`, locals `v_<name>__<id>`, temporaries `t__<k>` (the bound local of an array member's first slot among them), the bound locals of global arrays `ga_<name>__<id>` (the Viper name of the block without `GB_`), the formals and results of a structure value's array elements `p_<name>__<k>_m<j>_e<i>` and `res_m<j>_e<i>`, the result `res`, labels `l_end`, prelude
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
- A **marked range row** (decision 0059 item 7: an `assert forall k: Int :: { loc(B, k) } lo <= k && k < hi ==> acc(loc(B, k).f)` over an array's slots)
  writes its id as `assert.failed[insufficient.permission]`: the second column is `<id>[<reason>]`, the reason the one Silicon gives when the
  permission is missing; every other row is written as before. `parse_position_map` reads the bracket and refuses any pair but exactly
  `assert.failed[insufficient.permission]` (a malformed id is a map problem, exit 3).
- **A Viper function** (M4.5e1d, record 0062 items 11 and 12): each `requires` line has an `internal` row (`not.wellformed`); each `ensures` line has the user row `postcondition.violated` -> `postcondition` (at
  the function's `return`, with a note at the clause: "`g` may not satisfy its `ensures`") and an `internal` `not.wellformed`; a non-empty `decreases` line has the `termination.failed` entry the rule
  for methods asks for (Silicon does not report there); **each body line that begins with an application of the function itself has the `termination.failed` row of that call** (`termination` at the
  recursive call, with a note at the measure). Both native rows go through the existing reasons (`assertion.false` and the three termination reasons), so `report.cpp` and the map's format do not change; every
  other row on a function is `internal`.
- **One accepted warning** (record 0062 item 11, question 275). Silicon prints, before its verdict, `Could not resolve expression <e> (<file>@<l>.<c>--<l>.<c>) during the axiomatisation of function <f>.
  This typically happens if the expression is on an infeasible path, i.e. is dead code. The unresolved expression will be replaced by a fresh symbol, i.e. an arbitrary value.` when a memory read or an application in
  a `pf_` function's body or `ensures` stands on a branch no path satisfying the `requires` reaches (it can stand beside "verified"). `src/driver/silicon_output.cpp` and `run_tests.py` accept exactly that line,
  **only before the verdict line**, when `<f>` matches `pf_[A-Za-z_][A-Za-z0-9_]*__[0-9]+`; the same words for a method, a prelude function or a `pf_` name without a digits-only id, a changed word, or the line after the verdict
  stay an unexpected line (exit 3). The argument that it is sound (the fresh symbol replaces the expression only inside the definitional axiom, which holds only where the `requires` holds) is record 0062 item 11.
- The driver matches each Silicon error by its `.vpr` line and id (the id before the bracket); no match is exit 3 (0011 item 11). A row's reason must be
  `assertion.false` (`tuple.false`, `tuple.condition.false` or `termination.condition.false` for `termination.failed`) as today; a **marked row accepts
  exactly its expected reason** instead, mapped to the row's category and message, so `assertion.false` on it, `qp.not.injective` anywhere, and
  `insufficient.permission` on an unmarked row are all exit 3.
- The driver's classifier of Silicon's messages and the harness's (`_VIPER_ERRORS`) are two copies of one
  table pinned to the Silicon version; a self-test runs both on the same recorded outputs and requires
  agreement.

## 6. Linting (`scripts/lint_viper.py`) *(M3-7)*

Fails on: `inhale`/`assume` without `// cerastes:inhale-ok <reason>` (invariant 4); a bodyless method without
`// cerastes:trusted-contract <reason>` (0012 item 1); any `domain`, `axiom` or bodyless `function`; an `assert`,
`requires`, `ensures`, `invariant` or method-call line with no position-map entry; an empty input set. From M4.5d2a
(`lint_heap_token`): `field tok: Bool` exactly when `tok` is used; `tok` only as `loc(block_of(…), -1).tok` in a
`requires`, `ensures`, `invariant` or `assert` line or a snapshot's `perm(…)`, never in an `exhale`, `inhale`, assignment,
condition or `var`; `forperm` only as `assert forperm t: Ref [t.tok] :: <E>` on a `leak` row.

**Pure functions (M4.5e1d, record 0062 item 12).** A top-level `function` is a prelude function (by name, its body under today's prelude rules, and an empty `decreases` line admitted where the file holds a `pf_` function
and only there), a `pf_<name>__<id>` function (digits only) or a function that applies only prelude functions (the lint's own fixtures; it can neither recurse nor reach a `pf_` function). A `pf_` function is bodied, has one
`decreases` line (a measure or empty, never `_` or `*`), `requires` lines in exactly the minimal shapes (`wildcard` is admitted there and on 0061 item 10's marked row, nowhere else), `ensures` lines with no `acc`, `perm`, `old`
or `inhale` and a body with no `acc`, `perm`, `old`, `forperm`, `inhale` or `assume`; it needs the map entries of §5 and its `pf_<name>__<id>__wf` method. A `pf_` application may stand in a `pf_` function, a method's
`requires` and `ensures`, an `invariant` and an `assert`, never in a `__wf` method (there it is a ghost function), never in a `decreases` line, and `HEAP_READ_RE` counts it as a heap read. The ghost domain of a `__wf` method may hold
`hva_<method>__<callee>_<pre|post>` functions of `Int`, `Bool` and `Ref` parameters; the assumed facts have exactly the shapes (i) to (iii) of §3 (fact (ii) under `V` with its quantified clauses, the keep fact under both atoms' guards and
the inclusion read from the callee's own atoms, fact (i)'s range checked against the `// cerastes:result` tag; each unguarded form is a must-fail self-test). A user quantifier of 0061's shape is admitted in a `pf_` function's `requires` and
`ensures` and as a conjunct of an application's guard.

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
`include/cerastes.h` (0011 item 4). A file under vlibc's `include/` is a library file (0046 item 3): verified, and not
system for `SrcPos::in_system` either, although EDG flags it (the driver passes vlibc's directories with
`--sys_include`, first). A file that cannot be placed is verified (`in_verified_file`, `is_system_file`):
fail closed. Positions of nodes are mapped through `diag_position` (0006 item 10).

**Declarations (rejected with `unsupported_declaration`, at the declaration's first line).** In a verified file: a
class, struct or union other than a verified structure (0043 item 2: `struct_info` classifies a class once, by P65's
fields, members at their own lines; since M4.5c1) (an enumeration: below; a lambda's closure class is reported at the lambda expression instead); a global
variable that is not a named constant (below); a weak function (`is_weak`/`is_weakref`, 0017 item 2); a file-scope
`asm`; a template; a `constexpr` or `consteval`
function; a variadic function; a function without a prototype; a `static`, unnamed-namespace or `inline` function
that is declared (with a contract, say) and not defined in the file (no other unit can define it, so its contract
would be trusted for nothing); locals that are `static`, `extern` or `thread_local`, an array other than an automatic local written as an array
declarator (since M4.5e1a an array local of a bool, integer, enumeration or object-pointer element with a constant bound is a block, §3 "Arrays"; a VLA, a
zero-length array, a bound naming a local constant, `const`/`volatile`/`_Atomic` elements, an array of arrays or of structures, one reached through
`typeof`, `decltype`, a typedef or an alias, an `int a[static N]` parameter, a string-literal initializer and an array compound literal are rejected with
the session that lifts each in the message), a `const` local without an initializer, a local used in its own initializer (an array's included: an element
that reads, writes, takes the address of or decays the array being declared). Since M4.2c a local without an initializer
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
(M4.2a), else `call_without_contract`; its reason is "declared here, defined in another unit" or "trusted, never
verified: declared in a system file", and for a contract written in a library file vlibc's own tag (0046 item 4: the
comment line above the declaration; 0015 item 5's table is gone). A name vlibc declares with a contract is reserved: a
contract on it outside a library file is `annotation_reserved_library` (0046 item 6). Attributes written in a system
file are not held to the allow-list (0046 item 7). `predicate`/`pure`
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
`unsupported_expression` with a description of the operator (pointer and member operations). Since M4.5b1c2 (0039) `assign_target` takes an address-taken
local or `*e` anywhere an assignment may stand: `assignment_value` builds an `AssignE` on the cell or a `StoreE`
(`store_expr`, `compound_store_expr`, `inc_dec_store_expr`, sharing the address node); `sequence_problems` counts a cell
(and `*&x`) by its variable and `*v` once per pointer variable `v`; `return_statement` rejects R14
`return_local_address` when the returned value, through parentheses, explicit pointer casts, either arm of `?:`, the
right operand of `,` and `&*`, is `&x` of an automatic local or a parameter; and a dereference in a function's
`decreases`, lvalue-flagged at the top of the measure (P60 C3), is read like a contract's.
Since M4.5b1c3 (0040) `vtype` recurses into pointer pointees (`void *`, struct, union and function pointees, and
`volatile`, `_Atomic` and `restrict` below the top, stay rejected), pointer variables and parameters may be
address-taken cells, and `pointer_conversion` accepts a conversion only when the target's pointee is the source's with
qualifiers added at its top level (`qualifiers_only_added` only chooses the message: GCC's `-Wincompatible-pointer-types`
in C, C++'s multi-level qualification conversion in C++); the driver makes #144 and #167 errors in both modes.
`contract_read` accepts `*r` of the result binder in an `ensures` (`old(*r)` rejected); `owns(**pp)`, `live(*pp)` and
`**pp` reads in contracts, invariants and measures were rejected naming M4.5c2 until M4.5c2b.
Since M4.5c1 (0043) `vtype` accepts verified structures as cell types and pointees at any depth; `eok_dot_field` and
`eok_points_to_field` build `MemberAddr` chains (`c_text` such as `p->t.x`); braced initializers expand to one store per
leaf (each element an assignment conversion read under EDG's fold; in C every element into a temporary first;
overrides, brace elision and out-of-order C designators rejected); struct copies (`struct_assignment_statement`, only
in statement position) read every source leaf into temporaries before storing the destination's; contract places and
atoms take member paths, overlapping atoms on one root are rejected; a `restrict` parameter is rejected
whole (0064 item 19; the earlier backing rules of 0043 item 10 are withdrawn). `spec_atom`/`invariant_atom` accept atoms
in a loop's invariants (not in a `do … while (0)` invariant or a measure) on a pointer variable that is not a cell, and
`check_invariant_atoms` checks their positions. `spec_range_atom`, `contract_element_address` and the range rules of `finish_atoms` (record 0060) are described in §3, "Ranges in contracts". `read_owning` collects `[[cerastes::owning]]` from every declaration's
parameter types (the definition's `assoc_param_type`, a typedef'd function type merged by the front end), matches the
declarations (`annotation_mismatch`) and rejects the other shapes; `apply_owning` inserts `p == NULL || owns(*p)` first
in `requires_` and `ensures_`, merged with an explicit `owns(*p)` or `p == NULL || owns(*p)`; an owning parameter counts as a contract in `callee_for_call`. `read_marker` does the same for
`ref_to_uninit` and `must_init` (decision 0042; `markers_`, then `Param::init` in `function_of` before the contract is
encoded, so `reject_marked_read` refuses `*p`, `old(*p)` and `decreases(*p)` where the flag is not promised;
`check_marker_atoms` after `apply_owning` and `finish_atoms`); `reject_markers_in_spec_function` and
`reject_markers_on_system_redeclaration` refuse the three parameter attributes where no signature is read. `uninit` is
read in `declaration()` (`uninit_initialized`; `Ctx::uninit_marked`). Under `VerifyConfig::strict_init`,
`declaration()` rejects an unmarked local without an initializer (`uninit_unmarked`) and `strict_init_arguments` in
`call()` accepts only `AddrOf` of a marked local or the caller's marked parameter's copy, settled after the body with
`vir::assigned_params` (`uninit_ref_initialized`). `Sequencer::object_of` keys each depth of `*…*v`
of one pointer variable as its own object (P62 A8).

Since M4.5c2b (0045) one builder, `place_pointer`, turns a contract place's IL into its VIR pointer by shape (never by the
lvalue flag; `(*p->q).x` and `p->q->x` are one place; the C promotion cast of a narrow place peeled), every load an ordinary
`contract_read` (so 0042 item 6's marked-read rule applies to each); `old_call`/`old_c` read `old_(X)` as the place `*X`
(or `Y` for `address_of(Y)`, P70 H1) under one `Old`; `spec_atom` and `invariant_atom` accept atoms through loads and on
struct places; the `const` rule is the place's own qualifier (below C++'s binding `lvalue_adjust`, `orig_lvalue_type` in C);
`finish_atoms` pairs and de-duplicates by the full `PlaceKey`; `check_marker_atoms` and `apply_owning` walk the path with
`place_root`; `AtomCollector`'s `guard_ok` admits guards that read the heap but hold no atom, call or side effect;
Still rejected: atoms on the result or its members
(M4.5d2), `old` inside a place's path (M8).

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
(`reject_computed_sizeof_operand`; since M4.5e1a an expression operand that names an array local written as a declarator is let through, its constant
keeping its `expr`: the bound is checked as written by `check_array_bound`, a local constant of the function whose prologue row is an `overflow` for a
wrapping bound such as `65536u * 65536u + 1`, and the value is EDG's `N * sizeof(T)`; a type operand, `typeof`, a typedef, a string or compound literal
stay rejected), on the C++ top-level `sizeof` enumerator, and on array typedefs of verified files
(`declaration_level_checks`); `vtype` rejects a typeref chain through an alias-template instance or a member typedef of a
class instance with a non-type argument (`typeref_chain_reason`).

**Arrays in the encoder (M4.5e1a, decision 0059).** *Types:* `declaration` sends a local whose type entry is `tk_array` (not a VLA) to `array_vtype`, which
alone makes an `Array` type (`vtype_object`/`vtype` still refuse arrays, so parameters, results, globals, members and pointees stay rejected, each message
naming the session that lifts it); only a type written as an array declarator counts (`written_as_array_declarator`: a typedef, alias, `typeof`, `decltype`
or `auto` in the typeref chain is `unsupported_declaration`). The bound is `number_of_elements`, and a bound that kept an `expr` (`2 * 3`, a macro sum,
`65536u * 65536u + 1`, which EDG folds to 1) is checked as written by `check_array_bound` (`constant_value`, then `add_local_const`: obligations in the
prologue, an `overflow` for the wrapping one); a literal, macro literal or enumerator bound and `int a[] = {…}` keep none. A bound naming a local constant
(`constant_bound_expr_in_local_expr_node_ref`), `[0]`, qualified elements, `static`/`extern`/thread storage, `owning` and `ref_to_uninit` on an array
(`read_variable_markers`) are rejected. The array is always a cell (`mark_address_taken`; `a[i]` sets EDG's `address_taken` but `sizeof(a)` and annotations do
not, P78 F4). *Initializers* (`array_initializer`, from `init_statement` or at the declaration): the list's elements through `collect_array_elems` (a
cursor; a nested list, a range designator and every array designator are rejected, `c99_designator` in C++), each value through `constant_initializer` or
`expr`, `Var::zero_filled` set when fewer than N distinct indexes are written; C evaluates every non-constant element into an `init_<i>` temporary before
the first store (`unsequenced_calls` for two with an effect), C++ stores each in order; the array being declared is in `cx_.declaring`/`pending`, so an
element that reads, writes, takes the address of or decays it is "a local variable used in its own initializer". *Expressions:* `eok_subscript`
(`subscript_address`: the pointer operand is the one of pointer type, so `i[a]` works; an `array_to_pointer(var a)` base is `AddrOf(a)`, any other pointer
value is the base as it is; a pointee that is a structure and a pointer element in a loop invariant or measure are rejected; a plain `char` index is
`char_subscripts`), `&a[i]` (`address_of`), writes (`assign_target`), the decay (`array_decay`, after the own-initializer test and the
`array_address_null_test` detection in `truth_context` and `comparison`), `return a;`/`return &a[i];` (`returned_local_address`), `a[i] == a[i]`
(`same_pure_expr`), an element inside a folded constant's original expression read as written (`0 && a[3]`), `eok_padd`/`psubtract`/`pdiff` and pointer
ordering rejected with M4.5e2 in the text. Elements are allowed in `check`, a loop invariant and a loop measure (`loop_clause`), not in a contract (e1b).
`loop_keyword_position` gives each `b_.loop` its `keyword_loc` (`Builder::with_keyword_loc`).

**Known over-rejection.** A call written inside an annotation's own `sizeof` operand (`CERASTES_REQUIRES(sizeof(int[p(2)]) > 0)`)
is rejected as a call in an annotation, although the operand is never evaluated: the encoder reads the folded constant's
original expression, which contains the call. This is the safe direction (M8 inventory).

**The invalidation profile in the encoder (M4.5d2b2d, decision 0058).** The lowering's half is in §3; the fixtures
and cases are named there.

*`cerastes::not_invalidating` (item 2).* Registration `{"not_invalidating", "", "p", RAF_NO_FLAGS}` (as `must_init`:
EDG's #1835 on a function, a field, a trailing marker after an array or a function pointer; P77); `Annot::not_invalidating`
must be in `classify`'s `kNames` in the commit that registers it (a registered name the reader does not list is
`Annot::none` and is silently ignored). `read_marker` files it in its own map, `not_invalidating_`, with `vir::Param::
not_invalidating` set in `function_of` before the contract is read; `marker_name`, `is_param_attribute` (the function-marker
rejection, `first_param_marker`, `reject_markers_in_spec_function`, `carries_marker`; `is_param_marker` is unchanged),
`reject_library_not_invalidating` (a marker in a library file) and the `check_signature` call make it never ignored. The
`const` branch of `read_marker` does not apply (question 203); `annotation_mismatch` is the existing machinery;
`check_not_invalidating_pairs` rejects it beside `ref_to_uninit` or `must_init`. *The check on the contract*
(`check_not_invalidating`, after `finish_atoms`, with or without a body): every `requires` atom or block site rooted at
the parameter has an `ensures` partner (a permission atom by kind and amount: an `ensures` atom with no partner carries an
output amount no `requires` atom has; a block site by place), an `ensures` one a `requires` partner, and a `noreturn`
function takes nothing through it. *The implicit `ensures`* (`tie_not_invalidating`, after that check): for each
`ensures` atom or block site rooted at a marked parameter whose place goes through loads, each loaded pointer `E` on the
way (root first, deduplicated by cell) gets `E == old(E)`, **spliced right after the `ensures` permission atom or block
site that frames the load of `E`**, a closure's block site included (`a->b->x` of a two-level chain is framed by the block
site of `a->b`, there is no `owns(*a->b)`), as `close_source` places a closure: it stands under that atom's guards by
position, never under the site's own null test (variant A, which a free-and-null body satisfies vacuously, is wrong:
S42), and no guard is copied. Each clause that receives one records its text in `Clause::added` and the marked parameter's
index in `Clause::added_params`. The lowering reads both (§3).

*`--strict-owner` (item 3).* The pattern is `--strict-init`'s: `Args::strict_owner`, `VerifyConfig::strict_owner`, two
tags in `diagnostics.def` (`owner_result_unowned` for p4, `owner_release_unowned` for p6, each "... (--strict-owner,
decision 0058 item 3)"), and nothing is computed without the option. Two function-level facts: `gives_result_block(f)`
(the `result_sites` test as a function: the `ensures` has a block site on the result) and `param_consumes_block(f, k)`
(the encoder's twin of lowering's `block_consumes`: the `requires` has a block site on parameter `k` with no `ensures` partner).
*p4:* `Ctx::owner_result_ok` (AST nodes, filled by the receiving context before the value is encoded, peeled through casts
and lvalue adjustments like `allow_strict_result`) is set by `initializer` (an `owning` local), `assignment_parts`
(an owning local, global or `static` local; an owning member through `owner_target`), `return_statement` (when
`gives_result_block`), the argument of a consuming parameter (`call()`'s argument loop, `heap_call`'s `free(make())`) and an
owning leaf in a struct initializer or compound literal (`init_element_value`); it is checked in `call()` (after the
callee is known, before `b_.call`) and in `allocation_view` (the view's result: `malloc`, `calloc`, vlibc's allocations;
heap routines leave `call()` early), by `reject_owner_result`. *p6:* `call()` (`strict_owner_arguments` over
`param_consumes_block`) and `heap_call` (the `free` operand), by `reject_release`; an owner by name (`owner_by_name`,
VIR-based so C and C++ read alike) is an owning local (an address-taken one as `Deref(AddrOf v)`), an owning global or `static`
local, an owning member by `.` or `->` at any depth (`owning_member_addr`, `owning_leaf_at`), a parameter of the enclosing
function that consumes its block, or a call whose callee `gives_result_block`; an `owning` parameter (a borrow, question
175) is not an owner. Verified bodies only; both strict options may be given together. What the static rule cannot see is
listed in the record and on the Verifying page.

*Marked fields (item 4).* Registration `ref_to_uninit` `"r|v|p|d"`; `vir::StructMember::ref_to_uninit`, read in
`struct_info` beside `owning` (`marker_nodes(..., Annot::ref_to_uninit)`), the union loop of `declaration_level_checks`
reports it too. Rejected, one `unsupported_annotation` each, at the attribute: a non-pointer, a reference, `void *`, a
pointer to a function, a pointer to `const` or `volatile` (nothing could initialize it), twice, a bit-field, a union
member; `must_init` on a field stays EDG's #1835. *`--strict-init`:* a store to a marked field (`assignment_parts`:
`marked_member_addr`) takes 0052 item 4's rule (`check_marked_field_source`: a null pointer, a `ref_to_uninit` call's
result, `&x` of an `uninit` local, another marked local, parameter or field); a marked field is a source for a marked local
(`strict_value_uninit_ok`) and an argument of a marked parameter (`strict_init_arguments`); per leaf of an initializer list
and compound literal (`check_marked_element`, in `init_element_value`); a global's static initializer: null accepted, an
address constant rejected (`read_struct_global_init`); the same store to an unmarked field stays rejected. *The closure:*
`owning_leaves` carries each leaf's mark (`leaf_is_marked`); `close_leaf(…, marked)` **stops at a marked leaf**: nothing
below a marked block is closed (audit S3: its cells have no known value, so its own owning fields own nothing yet), and
`closure_marked` makes a marked parameter's, a result's and an owning and marked global's block site close nothing
(`deep_expr` builds no `inner` for such a global), and, since audit round 2 S2, a WRITTEN block site on an owning and
`ref_to_uninit` member (`owns_block(p->q)`, a `Deref` of a `marked_member_addr` whose member is `owning`) too: `close_source`
treats every block site as a source, and without this case the written spelling closed below the field (`p->q->x`, plain) while
the automatic closure of the same field stops at the marked leaf, so the two spellings of one contract meant different things
(`permission: the contract of fill reads p->q->x without owning it` for the written one). The two now lower to the same contract
(the tie conjunct differs in position only). It is consulted for contracts, not for loop invariants (a written invariant
site over a marked field still closes below the field: a pinned false alarm, M4.8); a marked field that does not own is
unaffected (it changes no Viper). *Question 202:* the three rejections (`reject_marked_owning_pointees` for a
parameter and a result, the owning-field rejection of `read_variable_markers`) are gone, so `ref_to_uninit` on a pointer
(parameter, result, variable) to a structure with an owning field is accepted and the marked site closes nothing, on
either side; `Var::ref_to_uninit` is set from an owning and marked local. A callee that leaves a block in the field is
reported by its own end row, a caller whose structure held a block keeps it and reports it at its end if the callee
overwrote the field (`leak`: S42's `s42_5*`, which emulate an unmarked parameter, a stronger callee), and an initializer
is `must_init`. The marked closure's flag modes are lowering's (§3).

**Quantifiers in the encoder (M4.5e1c, record 0061).** *Names and tags:* `cerastes_forall` is `SpecFn::forall` (guarded by `declared_in_cerastes_header`, so a user function of that name is
not it) and `cerastes::forall` stays `SpecFn::unsupported`; `forall_namespace` refuses the namespace spelling in a contract, an invariant and a `check` (a lambda: "the lambda form: M7"; any
other first argument: "not read when verifying (M7)", cut 3); two tags in `diagnostics.def`, **`forall_function`** (the function named is not a `pure` function the reader accepts: its
message says which rule) and **`forall_argument_mismatch`** (arity, or a written argument type that is not the parameter's up to top-level qualifiers; `T *` for `const T *`). *The site:*
`forall_site` runs from `call()` for every annotation position; `mark_forall_positions` (run on each `requires`, `ensures`, invariant and `check` root) records the calls in a positive
position (in a contract: a conjunct, an operand of `||` after `collect_forall_calls` has refused a quantifier in its left operand, an arm of `?:`; in a loop invariant or a `check`: a top-level `&&` conjunct only, a quantifier under `||` or in a `?:` arm goes to `forall_not_conjunct_`, question 262); every other position, a `decreases`, `old`,
`owning(n)`, a nested quantifier, a measure and a `do ... while (0)` invariant are `unsupported_annotation` naming M4.8. *Reading `f`:* `read_forall_function` (a `predicate`, a contract on `f`
and a `restrict` parameter are `unsupported_annotation`; the parameter types are classified before `check_signature`, which then gives a VLA or `[static N]` parameter its usual tag), the body
of its one `return E;`, and `first_forall_effect` (a call, write or increment in `E` is refused before `E` is read: a body is encoded at every site that names `f`, and an effect is
encoded once per function). *Arguments:* one per parameter after the first; the written type after **one** generated cast (C's promotion; a written cast is never peeled) must match
(`forall_argument_type_ok`); a `&a[1]` pointer argument is refused where it is encoded, a global array too, a named local array in a loop or a `check` is collapsed to `AddrOf(a)`
(`array_variable`), and an argument that reads memory or names `cerastes_old` is refused (`index_reads_memory` catches a cell read by name). *The body:* `Ctx::forall` (a `ForallSite`: id, site, `slot`
map from the parameter variable, names, types, encoded arguments, `read_classes`) makes `variable()` return `Bound` / `ForallArg` (`forall_parameter`); `report_position` makes every report inside the
body come out at the site; `contract_element_address` (contracts) and `subscript_address` / `indirect_read` (loops, checks) accept a `ForallArg` base after `forall_base_ok` (a contract: a bare
`Formal`; a loop or a `check`: a `Local` pointer that is not address-taken, or `AddrOf` of a local array) and `forall_deref_ok` for `*p`; `bound_index_form` classifies each index (`forall_index_form`
mirrors `check.cpp`, so the user meets the encoder's message); `forall_body_ok` is the list of node kinds a body may hold (the same as `check.cpp`'s); `forall_loop_body_ok` and
`forall_multi_class_why` and `forall_later_class_arithmetic` mirror the narrowings `check.cpp` makes in loops, checks and multi-class quantifiers (questions 255, 259, 261), each as `unsupported_annotation` naming M4.5e2.
`reject_marked_read` maps a `ForallArg` to its argument (0042 item 6). `cerastes_old` under the binder is `old_c`'s in an `ensures` and `old_outside_ensures` at the site elsewhere. *Truth:* a loop
invariant or a `check` that holds a quantifier keeps its Bool tree through C's `&&` of type `int` (`holds_forall` beside `holds_atom` in `clause_bool`), as a contract does. The shape rules of
annotation expressions (the `-Wall` shapes, R5 and R9) reach `E` through `truth_context`, reported at the site.

**Pure functions in the encoder (M4.5e1d, record 0062).** *Which are read* (item 2): lazily and by walking the annotations, never from the IL's `called` flag (P81 (b): false for a function only annotations name).
`read_pure(r, at)` reads a function once (`pure_`, memoised; a function met again while it is being read returns its `vir::Function`, which is how self-recursion and a cycle through a contract are seen); its entry points
are the run loop (a `pure` function with a body that carries a `requires`, `ensures` or `decreases`, after `spec_routine_clean`, the checks `Encoder::run` always made on a spec routine: a function they reject is not read
further) and `application()` (an annotation that applies it). *The function* (`pure_function_rules`, tag **`pure_function`** in `diagnostics.def`, reported at the call, or at the function when only its contract made it read):
defined in a verified file (a call of one that is not: `unsupported_annotation`, M6), not a member, template, instance, lambda, variadic, `constexpr` or `consteval`, a result that is `bool`, an integer or an enumeration, parameters that are
those or pointers to them (to a pointer at any depth), no `restrict`, `check_signature`. *The body* (`read_pure_statements`): blocks, `static_assert`s (each entity of a `stmk_decl` that is a static assertion goes to
`encode_block_static_assertion` with the function's own context: it becomes a local constant of the function, checked by the prologue of its `__wf`; any other entity is a local declaration, `pure_function`) and exactly one trailing
`return E;`. `E` is encoded from a **copy** of the expression (`clone_pure_expr`, nodes in `pure_nodes_`) whose parameter variables are `enk_param_ref`s, so that it goes through the contract path unchanged (reads through the
parameters, framing, R1-R13, the conversion at the `return`) with `Ctx::pure_fn` and `Ctx::pure_body` set (`encode_pure_body`; an assignment or `++` in `E` is `annotation_side_effect`, found by `first_pure_write`). *The contract*
(`encode_contract` sets `Ctx::pure_fn`; `pure_clause_ok` checks each clause): value clauses that read no memory (`value_reads_memory`), `reads`/`reads_range` of a parameter as top-level conjuncts, no atom and no `old` in the `ensures`, no
memory in the measure, no read through a pointer loaded from memory (`loaded_read`). *Applications* (`application()`, from `call()` when the callee is `pure`): `unsupported_annotation` under `old`, in a `decreases`, in a
position that reads no memory (`Ctx::no_read` is raised around an atom's count, an index and a `cerastes_forall`'s trailing arguments), in the guard of an atom (`application_in_guard`); `call_in_annotation` under a quantifier's
binder; an application is accepted in a `check`, a loop invariant (the `do ... while (0)` one included), a contract and a `pure` function; otherwise `Builder::apply` after reading the callee. An argument is `expr()` of the site expression with the one generated qualifier conversion of a pointer peeled
(P81 (d)); a named local array in a `check` is `AddrOf(array local)`, as `cerastes_forall` takes it; a pointer a callee's atom or quantifier names must be a bare parameter (a contract, a `pure` body) or a pointer variable
that is not address-taken or a named local array (a `check`), `&a[1]` and `&x` are `unsupported_annotation`, and an argument for a parameter a callee's quantifier names reads no memory. *The graph* (`check_pure_cycles`, item 10):
`pure_edges_` holds the applications among read functions; a self-application without a `decreases` is `missing_decreases` at the definition, a cycle through two functions is `unsupported_annotation` at each definition, a cycle
through a contract at the application. The success line counts each read function (`verified_functions()`: question 281). `check_block_static_assertions` exempts the region of a `predicate` and of a `pure` function that was not
read, and no other.

**Arrays in structures and static storage in the encoder (M4.5e1e1, decision 0063).** *Types:* `array_vtype` is the one rule for which array is read (one dimension, a scalar element that is not `const`, `volatile` or `_Atomic`, a constant bound of at least 1); `global_vtype` tests the `const` rule first (M4.6), then `global_array_vtype`; `struct_info`'s `case tk_array` reads a member (the `const`, flexible, system-typedef, array-of-array or array-of-structure refusals, the member bound as a unit prologue constant "the bound of `S::a`" by `check_array_type_bound(..., unit=true)`, question 293's refusal of a structure that also has an `owning` or `ref_to_uninit` member). *Computed array types:* `array_type_reason` and `check_array_type_bound` (a bound read as written once: a function constant, or a unit prologue constant outside a function; in C++ read under the compiler-generated cast), `note_typedef_uses`, `file_array_typedef` (a file-scope typedef that is an array, a unit constant) and the pending block typedefs (EDG lists a block typedef in the function scope, not in a statement), `reject_computed_sizeof_operand` (the type operand of `sizeof` of such a type is accepted, its value EDG's literal, the bound's `internal` guard tying it). *Globals:* `make_global`, `read_array_global_init`/`read_array_element` (each element through `constant_initializer`, P38's shape rejected, the zero rest, an enumeration without 0 refused for the rest), `is_array_decay_constant`/`decayed_array_variable` (the decay of a global array is recognized by its `expr`, offset 0 and no subobject path: P82 (a)). *Accesses:* `subscript_address` takes a named array (`GlobalAddr`), a member (`place_address(member)`, named base, `in_own_initializer` through `ElemAddr`) or any pointer; `member_array_decay`; `contract_element_address` builds `ElemAddr` over a contract place; `&s.a` and a truth test of an array address are rejected. *Initializers:* `collect_init_elems` maps a nested list to `InitElem{path, elem}` (brace elision rejected, C's `{0}` and a one-member C++ `{0}` accepted: question 303), `struct_initializer` writes `ArrayZero` for each member the list does not write in full and then the element stores (C: every element evaluated first), `read_struct_global_init` the array leaf of a structure global; array compound literals (`array_literal_decay`, C only, a hidden array local zero-filled at its declaration and its elements stored in the expression; `reject_literal_object_writes` for a write inside a literal used as an object). *The rejections:* heap (`allocation_view`, `heap_call`, `spec_block_atom`, `invariant_block_atom`), markers (`read_marker`, result markers, `struct_info`), `reject_array_behind_load` (question 306), written atoms in loop invariants.

**Arrays of structures in code in the encoder (M4.5e1e2 Part B, decision 0064 items 2, 5, 6, 11, 12 and 16; facts P83 and S48 in `internal/il-inventory.md`).** *Types:* `array_vtype(type, aggregate_elements)` reads a structure element only for the automatic local array of `declaration` (a global, a `static` local, a member, a literal and every computed type keep the old rejection: Part C or E); the element structure is `vtype_object`'s, then its own rejections, none resting on `struct_info`'s gate: an `owning` or `ref_to_uninit` member at any depth (`vir_aggregate_has_marked`: `unsupported_type`, M4.8), an array member at any depth (`vir_element_has_array_member`: the declaration's tag, Part E, depth 2); `uninit` on such an array is `unsupported_annotation` (`declaration`), `owning` and `ref_to_uninit` on any array already were (`read_variable_markers`). *Accesses:* `subscript_address` already built `ElemAddr(AddrOf(v), i)` for a local array and now builds `ElemAddr(p, i)` for a pointer to a verified structure (`struct_element_pointee_reason` rejects a pointee with a marked or an array member, `unsupported_expression`; a quantifier's function rejects it, Part F; a contract reads `p[e].x` since Part C1, see the paragraph below); a loop invariant or measure that names a structure element through a pointer is read only where `struct_range_carried` holds (Part C1, below; the framing is per leaf), a `check` reads it like code. `place_address` of a subscript is that address (`v[i]`, `p[i]` as a structure place: `MemberAddr(ElemAddr(..), k)` is `v[i].x`, whose bound row the lowering takes strictly inside); a use of an array of structures whose declaration was rejected still draws its own row, as before. *Values and copies:* `struct_value_node` and `copy_struct` accept a subscript; `object_value` accepts an `ElemAddr` root over an array's address or a pointer local (the index has no effect: `first_effect`), `copy_struct`'s `stable` evaluates the element's pointer and index once each into temporaries (unless a constant, an array's address or a local that is no cell) so that every leaf's store reaches the same slot; a whole struct read or assigned anywhere else (`subscript_read`, `assign_target`) is rejected. *Initializers:* `aggregate_array_initializer` reads each element's braced list with `collect_init_elems` (a leaf per `InitElem`, the slot index beside it), sets `Var::zero_filled` when any leaf is unwritten (enumerations whose values exclude 0 rejected through `zero_leaf`), evaluates every leaf first in C and stores in list order in C++; an array designator is `reject_array_designator`'s; an element from a struct value is rejected. *Question 322 (item 5, widened in Part R):* `index_address` (was `struct_element_address`) recognizes every address formed by an index, whatever the element: `&a[i]`, `&p[i]`, `&t.a[i]`, `&p->a[i]`, `&m[i]`, `&v[i].x` (a chain of `.` over an element; a chain through `->` is another object's address and is not), and `detect_array_address_test` rejects its truth test and null comparison (`array_address_null_test`), `&p[0]` included (question 335). *A folded index address* (audit round R, should-fix 2): in C the front end folds `&ga[2]` on a global or `static` array or a global structure's array member into a `ck_address` constant that keeps the written `&E[i]` as its `expr`; when such a constant is an operand of a truth test (the condition of `if`, `while`, `for`, `do`, an operand of `&&`, `||`, a `?:` condition, a conversion to `bool`), `detect_array_address_test` reads through it (`expr_node_from_constant`) and rejects it with the same tag. A test the front end folds whole to an integer constant (`!&ga[1]`, `&ga[1] != NULL`, `return &ga[1] ? 1 : 0;`) shows the address to no operand (its original expression is read in a constant context, `const_depth > addr_const`, where the look-through is off) and keeps `unsupported_expression` with no second row, and so does all of C++ (the look-through is C only). In a contract the message names the indexed parameter, or the result's binder in an `ensures`, and no longer `"?"`. *Item 16 (questions 304 and 321):* `collect_init_elems(..., whole)`: in C an elided subaggregate (a nested structure or an array member) is accepted only in the lone `{0}` that is the whole initializer, at any depth of the chain (`whole`), which also fixes the frozen base's rejection of that `{0}` below an elided structure; in C++ an elided nested structure is rejected when its parent has two or more members, and an array member that elides its braces (the lone `{0}`) is read only when its parent is a structure of one member, at any depth of an elided chain (`B1 { A1 x; }` with `A1 { int a[4]; }` is accepted: the frozen base refused it by the elided recursion, both compilers are silent); an element of an array of structures that elides its braces is rejected in C++ always and in C unless it is the lone zero chain (`lone_zero_chain`). *The lone zero's spelling* (question 325, built by 339; C only): `lone_zero_spelling_ok` takes the innermost constant of the chain, peels the generated casts and accepts it only when it keeps no `expr`, carries `is_simple_zero` or `non_arithmetic`, is no named constant's definition and is not `bool`; it is called where the C++ predicate applies (the elided subaggregate of `collect_init_elems` whose parent has two or more members, an array member's lone zero under the same parent, an elided element of `aggregate_array_initializer`), so `{'\0'}`, an enumerator, `{1 - 1}`, `{false}`, `{NULL}`, `{nullptr}`, `{-0}`, `{+0}` and `{(int)0}` are `unsupported_expression` and `{0}`, `{0u}`, `{0L}`, `{0x0}`, `{00}`, `{0b0}`, `{0ULL}` and a macro of those are not; a parenthesis leaves no trace in the IL, so `{(0)}` stays accepted (M8). *Designators* change neither compiler's -Wmissing-braces answer: a braced designated value (`{.t = {0}}`) is a nested list (rejected in C where it elides, by the C++ predicate in C++); an unbraced designated element (`{.t = 0}`, `{.a = 0}`: `designated_elision`) is accepted in C when its value is zero (the lone-zero chain below it at any depth) and a non-zero value there is rejected in both modes (the list below it is read as elided); in C++ every unbraced designated element is rejected whatever its parent and value, because clang++ reports it without a flag (`-Wc99-designator`), 0020 item 5's union (one tag, `unsupported_expression`, for the struct and for the array member). *`restrict`* (item 19): every `restrict` parameter is rejected in `function_of` (see §3's `restrict` bullet).
*Structure values* (Part E, item 13): `copy_struct` reads the OBJECT view: each cell leaf through a temporary and then its store (as before), then one `ArrayCopy(dest MemberAddr, source MemberAddr)` per array leaf; **the encoder writes no guard**: a root that is `AddrOf` or `GlobalAddr` is a named object, any other (a pointer value, a `copy_base` temporary) is "through a pointer", and lowering decides the `d != s` guard from the roots. A copy has no size limit. `object_value` (an object passed or returned) is an `Aggregate` over the VALUE view, an element leaf `Deref(ElemAddr(MemberAddr chain, Const i : long))`; `aggregate_value` (a list or literal value) has one argument per value-view leaf, `zero_leaf` for the elements the list does not write, keyed by `(path, elem)`; `signature_vtype` rejects (`unsupported_type`, question 295) a structure whose value holds more than `vir::kMaxByValueArrayElements` array elements (`vir_value_over_limit`; `reject_array_value` is the same limit where a value is built); `elem_value_read` reads `f().a[i]`, `S{...}.a[i]` and `(struct S){...}.a[i]` in code (the base through `struct_value_node`) and `s.a[i]`, `r.a[i]` of a by-value parameter or the result in a contract (through `contract_field_chain`) as `ElemValue`, also under `old` and `cerastes_old`. What stays rejected: `mk().a` as a decay (a pointer to a temporary's member), a structure `?:`, comma or assignment value (0044 item 5), heap storage, and (C++) special members.

**Ranges over structures in contracts in the encoder (M4.5e1e2 Part C1, encoder half; decision 0064 items 7, 8 and 10; the lowering's half is the il-analyst's and `check.cpp` refuses whatever it has not lifted).** *The gates lifted:* `spec_range_atom` and `owning_count` accept a pointer parameter to a verified structure (`owns_range`, `reads_range`, `owning(n)`) and refuse an element structure that holds an `owning` or `ref_to_uninit` member at any depth (M4.8) or an array member at any depth (Part E, depth 2) through `range_element_reason` (`unsupported_annotation` at the atom, as the old gate); the stale "structures: M4.5e1e2" words of the other base rejections went. `contract_element_address` builds `ElemAddr(Formal p, e)` for a structure pointee (`struct_element_pointee_reason`'s rejections, now `unsupported_annotation`; a quantifier argument is Part F's) and `contract_place_address_raw` has a `subscript` case, taken only below a member step (`depth > 0`): `p[e].x`, `p[e].t.y` are `Deref(MemberAddr(ElemAddr(Formal p, e), k))`, the member steps of `dot_field` over it; an element as a place of its own (depth 0) and a base that is not a `Formal` stay `no_place` and `unsupported_annotation`. `contract_element_finish` already refused a read of a marked parameter's element (`reject_marked_read` with the `Formal`: `must_init`'s `ensures` may read it, every other contract position is `unsupported_annotation`), so `p[e].x` of a `ref_to_uninit` or `must_init` pointee is refused as `p[e]` was; `read_marker`, `check_marker_atoms` and `contract_field_chain` need no change (the marker rows over a range of structures are the lowering's, and a by-value parameter's `s.x` chain never meets a subscript). *What the lift newly opens, and what keeps it rejected:* `place_through_element` (a place whose nodes down to the root are members, loads and fields with an `ElemAddr` among them) rejects an atom on a member of an element (`owns(p[1].x)`, `reads(p[1].t)`, `live`, `owns_block` of `p[0].q`: `unsupported_annotation`, an atom on one element is M4.5e2's) in `spec_atom` and every pointer read from an element as the base of a place in `place_pointer` (`p[i].q->x`, `owns(*p[i].q)`); the whole element under `old` (`old(p[e])`) is `reject_old_element_struct`; `owns_range(p, n)` beside `owns(*p)` or `owns(p->x)` keeps its existing rejection. *Loops (item 10):* `subscript_address` accepts `p[e].x` in a loop invariant or measure only when `struct_range_carried` holds: the base is the parameter's own local (`Local` of a parameter copy that is not address-taken) and the function's `requires` (after `apply_owning`, so `owning(n)` counts, guards included) holds an `owns_range` or `reads_range` atom of that parameter (`vir::atom_place(...).root == param_index`, no `via`); with only an `owns` or `reads` atom of the pointee the message says to write `p->x`, with none (or a local copy of the pointer, a cell, an atom only in the `ensures`) it says the loop carries only its function's parameters' ranges; both are `unsupported_expression` at the invariant's line, as the pin was. The predicate does not test that `p` still holds its entry value: `p = q;` before the loop passes the encoder, and the lowering must frame the read by the slot of `p`'s current value (as `inv_elem` does for scalars), never by the entry value. A permission atom through an element in an invariant (`owns(p[1].x)`, `reads(p[1].y)`, `live(p[0].q)`, `owns(*p[0].q)`) is `invariant_atom`'s existing `unsupported_annotation` (a local or an element: M4.5e2), `&p[1].x` against a null is `array_address_null_test`; a pointer loaded from an element and then read (`p[0].q[0]`, `p[0].q->x`) reaches check.cpp and the lowering like the scalar `pp[0][0]`. A `check` reads an element like code does. *C++ own-initializer (Part C1, a message only):* the front end folds an array initializer that reads the array itself to a constant list of repeated values and runs that first (`dik_constant`, backing `enk_initializer`, il_def.h 13692), so the repetition read as a range designator (`int a[2] = {1, a[0]}`) or as elided braces (`{{1, 2}, {v[0].x, 3}}`); `folded_own_initializer` looks at the original list in the backing expression and, when a dynamic element names the variable (`expression_names_variable`), `init_statement` reports `a local variable used in its own initializer` at the declaration, as C does (the tag stays `unsupported_declaration` for an array of scalars; an array of structures gives `unsupported_declaration` where the elision message gave `unsupported_expression`).

**Arrays of structures in static storage and as members in the encoder (M4.5e1e2 Part C2, encoder half; decision 0064 items 9, 10 and 12; the lowering's half is the il-analyst's and `check.cpp` refuses whatever it has not lifted).** *Types:* `array_vtype(type, aggregate_elements)` is now passed `written_as_array_declarator(type)` by `global_array_vtype` (a global and a `static` local, through `global_vtype`) and by `struct_info` (a member), so a global, a `static` local and a member written with an array declarator may have a verified structure for element (the element rules are Part B's: an `owning` or `ref_to_uninit` member, M4.8; an array member, Part E); a typedef'd or `typeof` array of structures, an array compound literal's type and a type operand (`array_type_reason`) pass no flag and stay rejected (message re-pointed to Part E's typedef helpers). *Initial values* (`program_start`): a global or `static` local array of structures is read by `read_struct_array_global_init` (from `read_array_global_init`): the braced list is read by `collect_struct_array_slots` (the loop Part B's `aggregate_array_initializer` had, now shared: one nested list per element, `collect_init_elems`' rules, item 16's elision rule with `whole` true, a leaf written twice rejected) and set by `read_aggregate_elems` into `Global::elems`, **one entry per element up to the last the list writes, each with `leaves` = one `GlobalLeaf` per leaf of the element structure in `struct_object_leaves` order (a leaf the element's list leaves out is zero, Value `"0"` or Null), `rest` Zero** (so a list of three elements with only the first written has `elems.size() == 1`); every constant is read as an array element's is (`read_array_element`: a pointer leaf is null or the address of another global, entered in `addr_init_` for the escape rule; P38's refusal when the front end kept no original expression applies to every leaf of a list of 1,002 elements or more that sits under a conversion); an enumeration leaf whose values exclude 0 must be written in every element; a list longer than the array, an element initialized from a value, a leaf with no field and a duplicate are rejections. The values are known by 0041 item 3's linkage rule and 0063 item 9's two escape directions, as for scalars, and `make_unknown` sets every entry of `elems` and of each entry's `leaves` to Unknown and `rest` to Unknown (so `is_unknown` is `rest == Unknown` as for an array of scalars). A structure global or `static` local whose member is an array of structures (`read_struct_global_init`, then the member's elements) has `Global::leaves[k]` for the member's array leaf (the `aggregate_element` leaf of `struct_object_leaves`, as 0063 gave an array leaf) with `elems` and `rest` as above. *Initializers of a structure with such a member* (`collect_init_elems`, a branch before the array-of-scalars one): the member's braced list is read by `collect_struct_array_slots` with `whole` false and each leaf becomes an `InitElem` whose `path` is the structure's members down to the leaf (`{v, x}`), `elem` the element's index and **`index_after`** the number of path members before the index step (1 for `t.v[i].x`; the default `npos` is the array-of-scalars form, index last); a member whose braces are elided is accepted only as C's lone `{0}` that is the whole initializer (no leaf written: all zero) with question 325's literal-zero rule, in C++ never; an unbraced designated element (`{.v = 0}`) is accepted only in C and only as a lone zero (a lone-zero chain; no leaf written: item 16's designated rule), otherwise rejected; a braced member list inside an elided list is rejected. `init_target` builds the leaf's cell (`MemberAddr(ElemAddr(MemberAddr(root, v), i), x)`), `init_leaf_type` its type; `struct_initializer` zero-fills the member (`ArrayZero` of its address, item 4) unless the list writes every leaf of every element, then stores the written leaves; `aggregate_value` (a compound literal, a C++ temporary) matches an element's `(path, elem)` against the value view's leaves and rejects a list element no leaf takes (never read as zero). *The value view* (item 12): `vir_value_array_elements` counts an aggregate member's leaves (`count` times the element's) against `kMaxValueArrayElements`; `object_value` accepts an element root whose base is a local array's address, a pointer local, a global or `static` array's address or the address of an array member over a named object or a pointer local (`g[i]`, `t.v[i]`, `p->v[i]` passed or copied by value; a base read from memory is still rejected), and `object_value` and `aggregate_value` build a leaf's address with `value_leaf_address` (the element index after the first array-typed member on the path) and reject a value-view leaf that is the whole unexpanded aggregate member (`reject_unexpanded_leaf`: `unsupported_type`; the VIR layer's value view expands an array member of structures into one leaf per element leaf (`path` through the member, `elem` the index, `index_after`), so only a matrix member reaches it; run end to end: initializers, copies between locals and with a global, compound literals, by-value parameters and results); `copy_struct` copies such a member by leaf stores (a temporary per leaf of every element, read before any store, then the stores: no `copy_arr_<E>` exists) and rejects a structure whose aggregate members hold more than `kMaxValueArrayElements` leaves (`unsupported_type`, M4.5f3); an element of an array member of structures of a structure VALUE (`f().v[i]`, `r.v[i].x` in a contract) is rejected in `elem_value_read`. A contract read of such an element of a by-value parameter or of the result (`t.v[1].y`, `r.v[0].x` of a structure with an array member of structures) keeps `unsupported_annotation` at its line with the true text (`contract_element_address`, before the member place is read: "an element of an array member of structures of a structure passed or returned by value in a contract ... M4.5f3: state the property on the caller's object, or check it in the body"); a scalar element of a value's array member (`s.a[1]`) is the value view's and is read. In C++ a brace-initialized temporary or compound literal of a structure with an array member of structures, passed in a call that initializes a variable, reaches the encoder as a folded constant without its braces flags and draws the folded-temporary rejection of 0044 item 6 (`int r; r = f(T{...});` works). A copy's element base that is the address of an array member over a named object or a pointer local is evaluated again for each leaf (a temporary would be a pointer to an array); over a loaded pointer it is rejected. *Loops (item 10):* a loop invariant or measure may name an element of a global or `static` array of structures and of an array member of structures (`g[e].x`, `t.v[e].x`, `p->v[e].x`) as it names a local array's (`named_local` in `subscript_address`: the lowering frames each as the place the loop or the function holds). *Contract reads of a member of an element* (item 7; question 344): `contract_place_address_raw`'s subscript case (a member step over `ElemAddr`) accepts, through `contract_struct_element_base_ok`, exactly an element whose base is (a) a pointer parameter to verified structures (Part C1: `p[e].x`), (b) the `GlobalAddr` of a global or `static` array of structures (`g[e].x`, `g[e].t.y`), or (c) a member array of structures reached by member steps alone, with no load of a pointer, from a pointer parameter, a global or, in an `ensures`, the result (`p->v[e].x`, `gt.v[e].x`, `r->v[e].x`), the index reading no memory (`contract_element_finish`); in `requires`, `ensures`, `decreases` and under `old` of a member (`old(g[e].x)`). Everything else there stays rejected at its C line: a base reached through a load of a pointer (`p->q->v[e].x`: Part D, question 306), an element with an array member (`p[e].a[j]`: Part E), an atom on one element (`owns(g[1].x)`, `reads(g[1])`: M4.5e2, `place_through_element`), `&g[e]` in a contract, `old` of a whole element (`reject_old_element_struct`). *The lowering's two framing rules the encoder relies on* (il-analyst, audit round C1): `apply_frame` skips every site whose pointee is a structure (a structure site holds its fields in member blocks, so its frame disjunct would speak of another shape; an application beside it is unframed by that site and a `permission` error stands, a false alarm in the one case that its cells really are a member of the range); and a read under a binder (`fq_cls >= 0` in `contract_read`, `r.fq_class >= 0` in loop invariants) is not framed by a range over structures or by an aggregate array part (an array member of structures of a structure atom, a global array under `owns(g)`), so a `forall` whose read only such a site would frame is a `permission` error (fail closed, Part F). *What stays rejected* (each `unsupported_*` at its C line): `owns_range(g, n)` and `owns_range(p->v, n)` (M4.5e2); `v[i] = f();` (M4.8, the text re-pointed); an element structure with an array member (Part E) or an `owning` or marked member (M4.8); an array of arrays (Part E); heap storage of a structure with such a member (Part D: the encoder rejects it by `vir_type_has_aggregate_array`); a `const` array (M4.6); a struct value that holds an unexpanded aggregate member until the VIR layer expands it.
