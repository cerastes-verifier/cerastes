#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Lint the Viper that `cerastes verify` emits (invariant 4; internal/encoding.md
section 6, decision 0012).

    python3 scripts/lint_viper.py [path ...]

A path is a directory (every `*.vpr` under it, and its `MANIFEST`) or a `.vpr`
file.  The default is `build/vpr`, where scripts/run_tests.py puts the
`--keep-vpr` output of the verify tests.  Each `<file>.vpr` is read with its
position map `<file>.vpr.map` (encoding.md section 5).  The lint fails (exit
status 1) on:

* `inhale` or `assume` without `// cerastes:inhale-ok <reason>` on its line
  (a reason is required);
* a bodyless `method` without `// cerastes:trusted-contract <reason>` on its
  `method` line or in the comment lines right above it (decision 0012 item 1:
  the contracts of functions defined in other units);
* any `domain` but `domain Block` preceded directly by `// cerastes:domain-ok 0028`, any `axiom` but the four of
  `BLOCK_AXIOMS` inside it (`loc_inv` 0033, `loc_nonnull` 0034, `member_inv` 0035, `member_lv` 0036: each once, with
  its exact text, whitespace-normalized, and `// cerastes:axiom-ok <its record>` directly above; the comment lines
  right above; the record has a file internal/decisions/NNNN-*.md: decision 0028 item 2, 0032 item 11), any method named `alloc_<t>`
  that is not the trusted allocation of decision 0032 item 4 (bodyless, exactly its header, two `ensures` lines and `decreases _`,
  as the emitter prints them), any bodyless
  top-level `function` (the prelude has bodies only, decision 0012 item 11; the functions inside the `Block`
  domain are not top-level), and Viper's `new(...)` anywhere (its results would not satisfy `WF`);
* an `assert`, `requires`, `ensures` or `invariant` line, or a call of a method
  declared in the file, with no entry in the position map for its `.vpr` line
  (every check is on its own line and has an entry, decision 0012 item 13).  The one exception
  (decision 0012 item 11 as amended for 0023, owner 2026-10-24): the range postcondition of a bitwise
  prelude function, `function c_[u]<and|or|xor>_<32|64|128>(a: Int, b: Int): Int` with exactly one
  header line `ensures <lo> <= result && result <= <hi>` (`(-2^(N-1)) ... 2^(N-1)-1` signed, `0 ...
  2^N-1` unsigned; Silicon proves it once per function, it is never assumed), which needs no map
  entry.  Any other `ensures`, a `requires` or a `decreases` in a prelude function, and an `ensures`
  of another shape in a bitwise one, still need an entry (so they are reported);
* a position map that is not in the format of encoding.md section 5, a map entry
  for a line that is not there (past the end, blank or a comment), the same
  Viper error id twice for one line, or a missing map;
* termination clauses (decision 0013 item 5): a method or `while` without a
  `decreases` clause, `decreases _` anywhere but on a bodyless method with the
  trusted-contract tag, `decreases *` (it assumes termination: invariant 4) and a
  conditional `decreases <measure> if <condition>`;
* jumps (decision 0014 item 9): a backward `goto` (a `goto L` whose `label L` is earlier in the
  same method: a cycle that Silicon does not check for termination), a `label` that carries
  `invariant` clauses (an unchecked loop) and a label defined twice in one method.  A forward
  `goto` and a label without invariants (`goto l_end` ... `label l_end`) are fine; and (decision 0015)
  a forward `goto` into a block: the `label` must sit in a block (a `{ ... }`: the method body, a
  `while` body, an `if` or `else` branch) that also encloses the `goto`, so a jump goes outward or
  stays in its block, never into a loop (which would skip the invariant check on entry) or a branch
  (which would skip its condition);
* the marked closure's flag rows and the row of `cerastes::not_invalidating` (decision 0058 items 2 and 4): a kept-flag row
  `((old(L != null) && old(E.d_T)) ==> E.d_T)` for a pointer `E` read through a load stands in a clause that also holds
  `E == old(E)`, and in a return assertion that is one atomic `assert !(!(CLAUSE))`; and the row that a map message says is
  the `not_invalidating` one (`... does not return it, but ... is cerastes::not_invalidating`) is an `assert ANT ==> (GP ==> GE)`
  of a `__wf` method, the converse of an earlier row `ANT' ==> (GE ==> GP)` whose antecedent it holds;
* the flags of locals without an initializer (decision 0019 item 5; `held_<v> := ...` and `tv_<v> := ...` lines of an owning local, decision 0052 item 3, may stand between a write and its flag: only a write sets a flag, and a
  declaration, a block entry or a switch dispatch resets it): in a method, every `d_<name> := true` must
  directly follow (the previous line that is neither blank nor a comment) an assignment `<name> := ...` of
  the variable it flags, so a flag cannot be set without the write; every `d_<name> := false` (a reset) must
  directly precede `<name> := <dummy>`, an integer literal or `true`/`false` (the emitter writes the flag
  first, then the value: `d_v := false` and then `v := 0`), so a variable cannot be reset to a value that
  is then taken for a determinate one; and a flag is assigned nothing but `true` and `false`.  The
  snapshot of a loop (`ds_<name> := d_<name>`) and the reads of a flag are not assignments to it.  A dummy
  may also be `null` (the dummy of a `Ref` local).  The flag of an address-taken cell is a field (decision 0032
  item 11): `<recv>.d_<t> := true` standing alone on its line and directly after `<recv>.v_<t> := ...` (the
  same receiver and `<t>`), nothing else is assigned to a field flag (the allocation gives `false`);
* a `%` (Viper's modulo: the Euclidean remainder, never C's `%`) or a `\\` (Viper's floor division, never C's `/`)
  that is not Viper's exact mask or bit-function form (decision 0026 item 7: `a & (2^k-1)` is `a % 2^k`,
  `a & ~(2^k-1)` is `2^k * (a \\ 2^k)`): outside the bodies of the prelude functions that use the operator (`%`:
  `c_rem`, `c_urem`; `\\`: `c_div`, `c_udiv`, `c_shr`; by name), the divisor of every `%` and `\\` must be a plain
  decimal literal that is a power of two, 1 or more (`x % 256`, `16 * (x \\ 16)`).  The lint checks the form of the
  divisor only: that no C `%` or `/` is emitted as Viper's is guaranteed by `low::mod_pow2` and `low::div_pow2`
  (src/vir/lowered.cpp) being the only constructors of them, not by this rule (a C `/ 16` emitted as `\\ 16` would
  pass it); a `%` or `\\` whose divisor the rule cannot read as such a literal is reported too;
* the ghost amounts of the memory model (decision 0037 items 6 and 14): every `Perm` formal of a method has exactly one
  `requires none < a && a < write` (parentheses and spaces ignored) and no other clause of the method mentions `a`
  before it (a clause that reads the amount before its bounds is not well-formed for Silicon, and an amount without
  the bounds could be `write` or `none`); `wildcard` (an amount Silicon chooses, which is an unchecked assumption) is
  refused anywhere.  The case split `if (<condition>) { }` is plain Viper and needs no rule;
* the ghost amount results of a method (decision 0052 item 5: the output amount of a partnerless `ensures` atom): a result of type
  `Perm` has exactly one `ensures none < o && o <= write` (parentheses and spaces ignored) and no clause of the method mentions
  it before that line (the postcondition that keeps the amount positive and at most `write` whatever the body, or a trusted
  contract, says); a call's `Perm` target (`t, o := f(...)`) is allowed only where `f` is a method of the file whose result at
  that position is such a `Perm` (otherwise the rule of the next paragraph flags it);
* the ghost amounts of loops (decision 0038 item 9, amending 0037 item 14): a `Perm` **local** (`var k: Perm`; the
  snapshots `kv := perm(x.v_int)` that a loop's invariant uses as the amount of an `acc`) is assigned only `perm(...)`,
  `none`, `write` or a conditional `c ? a : b` of those (never `wildcard`, never arithmetic, never another variable:
  the snapshot is what the method holds), and is never assigned inside a loop whose invariant mentions it (a
  havocked amount could exceed `write` and make the body vacuous); `acc(x.f, k)` with such an amount is allowed in
  an invariant, `perm(...)` in an invariant is refused (Viper refuses it; the snapshot is the way);
* a heap block's array part as a loop place (decision 0064 item 15, Part D): its first slot is bound to a local X and its permissions are snapshotted at
  the loop's entry in two `Perm` locals that the method assigns ONCE, `perm(X.v_t)` and `perm(X.d_t)` (`amount_snapshots`); under the part's guard
  `g ==> ` (required) the permission conjunct is `forall k :: { loc(block_of(X), k) } offset_of(X) <= k && k < offset_of(X) + N ==> acc(loc(block_of(X),
  k).f, K)` with K the snapshot of X and that same field f, the flag conjunct carries the antecedent `Kd > none`, the range conjunct `Kv > none && Kd > none`
  and the value conjunct `Kv > none` (and `Kv < write` where a write can reach the field), each K the snapshot of the same X and the field the row reads;
  the entry and back-edge marked rows at those amounts are `assert`s of the same range rows (a snapshot of another base, another field or a quantifier that
  does not run from X's own offset is refused).  A conjunct without the antecedents is the older shape (a range place of a local array) and stays clean.
  Not checked: that N is the array's length, that the liveness invariant's amount is the snapshot of its own cell (the value-less liveness place of a
  structure with no cell is the older shape), that the guard names the pointer the part was bound under;
* the heap's free token (decision 0047 items 6, 7 and 10): `field tok: Bool` is declared exactly in a program that uses it (a heap
  program, which has a view call or a block site: a file that declares it and never uses it, uses it undeclared or declares it
  with another type is a problem); `tok` appears only as `<slot>.tok` where the slot is `loc(block_of(...), -1)` (the token's slot
  of 0028 item 6), and only in a `requires`, `ensures`, `invariant` or `assert` line (the conjunction of a view or a block site, an
  explicit row, a loop place's invariant or its back-edge row) or inside the `perm(...)` of a snapshot `kt := perm(...)` (a loop
  place's amount); never in an `exhale`, an assignment to `<slot>.tok`, a condition, a `var`, a `fold` or any other statement
  (a token is created only by a view's or a function's postcondition and consumed only by a call whose `requires` has the block
  site: nothing in the body may make or take one); `forperm` appears only as the statement `assert forperm t: Ref [t.tok] :: <E>`
  (one binder, the field `tok`, no `tok` in `<E>`) whose position-map entries are all category `leak`: the one assertion that
  reads every token the method holds (the rest of the heap rule is not checked here: the exemption `<E>` is the encoder's);
* the owner ghosts of an owning local (decision 0052 item 3): `tv_<v> := (<x> != null) ? loc(block_of(<x>), -1).tok : tv_<v>` is the
  one assignment that reads a token outside a `perm(...)` snapshot (the same ghost on both sides, the same pointer in the guard and
  the slot); `held_<v> := ... perm(loc(block_of(<o>), -1).tok) > none` reads it inside `perm(...)`, and the ending rows
  `if (!(<row>)) { assert false }` read no token;
* the allocator ghost of an owning loop variable in a vlibc heap program (decision 0052 item 3, loops): a `tve_<...>` is assigned only
  `(<x> != null) ? tv_<y> : true` and never inside a loop whose invariant mentions it; the entry and back-edge rows of an owner
  (`if (!(<row>)) { assert false }`, which read the token under `perm(...)` and compare it with `tve_...`) and its invariants (`held_v
  == (v != null)`, `(v != null) ==> (acc(<slot>.tok) && <slot>.tok == tve_...)`) need no rule of their own: they are the ending rows
  and the conjuncts the token rules above already read;
* the typed views of the allocation functions (decision 0047 items 2, 4 and 7): a bodyless method whose trusted-contract tag is
  vlibc's `malloc`, `calloc` or `free` with `typed view:` in its reason is the trusted view of one allocation type, and its text is
  pinned as `alloc_<t>`'s is, built from the type's leaves: `method f_<routine>__<n>() returns (res: Ref)` (`(p_ptr__0: Ref)` for
  `free`), exactly two clauses and `decreases _`; the first clause the well-formedness of the pointer (and, for a structure, its member
  lengths under `offset_of < len`), the second `!(res == null) ==>` the block part (the pointer, its well-formedness, offset 0, the
  token `acc(loc(block_of(res), -1).tok) && ...tok`, the liveness equation and cell, `len == 1` with the structure's member blocks and
  slot facts in the grammar of decision 0043 item 8, then for each leaf, in member order, its value and flag permissions and the facts
  `owns` states under the flag: an integer's range of its field's type, a pointer's well-formedness (with the pointee's member blocks),
  nothing for a `bool`) and, for `calloc` only, the flags and then the values 0, `false` or `null` of every leaf, and for no view a
  clause more or fewer: a view with an extra clause would be an unchecked assumption;
* where a `forall` may stand (decision 0059 items 7 and 8, decision 0060 item 16; every quantifier is `forall k: Int :: { loc(block_of(X), k) }
  lo <= k && k < hi ==> ROW` with X a variable, and X, lo, hi and any amount reading no heap): a marked row (an unguarded `assert` of
  `acc(loc(block_of(X), k).f[, amount])` whose map entry carries `assert.failed[insufficient.permission]`, and no other row's does), the range
  `exhale`, item 8's four loop conjuncts (the permission with no amount or the method's own `Perm` formal, the flags kept, the type's range,
  the values kept), alone or under a guard `g ==> `, a range atom's rows in a `method`'s `requires` and `ensures` (range `offset_of(X) <= k &&
  k < offset_of(X) + n`; `acc(F.f)` or `acc(F.f, a)` with `a` a `Perm` formal of the method, the flag `F.d_t`, the standard range of an integer
  type or WF of a pointer element, `F.d_t ==> range`, and in an `ensures` `old(F.d_t) ==> F.d_t`; a guard is a plain condition) and the pure
  `assert` rows of the same four non-permission shapes (a guard there reads no heap); `old[label]` only in the loop conjuncts, for a label that
  an earlier `label` of the method defines.  A `Perm` local may be assigned the amount of a range atom's call row divided by a decimal
  literal of 2 or more: `perm(lvloc(block_of(B)).lv) / N`, with the guard `(g ? ... : write)` and, for `reads_range`, the minimum over the
  first slot `loc(block_of(B), offset_of(B))`, whose nested-conditional minimum may also hold the method's own `Perm` formals (decision 0060 item 6, audit
  round 2 S5);
* arrays in structures and in static storage (decision 0063 item 15, M4.5e1e1): where a line has no Viper local, a quantified row is `forall k: Int ::
  { loc(B, k) } 0 <= k && k < N ==> ROW` with B a block TERM, `GB_<g>()` of a declared array global or `member(<chain>, k)` over decision 0043 item 13's
  canonical chain rooted at a formal, a result, `r` or `loc(GB_<g>(), 0)`, the bounds literals and N the count the facts state (`len(GB_g()) == N` of
  `program_start`, a `len(member(...)) == N` of the method's own clauses); in a method's `requires` and `ensures` the rows are a range atom's (see
  above, with `0 <= k && k < N`), in `alloc_struct_<n>_<tag>` an array member is `len(member(X, k)) == N` and its three quantifiers (value permission,
  flag permission, `!flag`), and `program_start` holds an array global's row group (the two permissions and the flags over `[0, N)`, one conjunct per listed
  element then the zero rest or an unknown value's range, `acc(lvloc(GB_g()).lv)`) with the facts `len(GB_g()) == N` the same in `program_start`, every
  method's first `requires` and the views'; `struct_lens`' `len(member(...)) == N` of an unknown pointer's row equals the allocation's count;
  `copy_arr_<t>` is pinned by its clause set (source flags kept by its `ensures`) with `alloc_*` and `zero_arr_<t>`; all three are exempt from the global facts, and
  `copy_arr_<t>` from the ghost-amount bounds, by the clause set and not by the name;
* ranges over structures (decision 0064 items 6, 7 and 17, M4.5e1e2 Part C1): in a method's `requires` and `ensures` (and the pure `assert` rows of a call) a
  range atom over a pointer to structures is, per element `k` of `offset_of(X) <= k && k < offset_of(X) + n`, the length facts `forall k :: { loc(block_of(X), k) }
  ... ==> len(member(<chain>, j)) == 1` for each member block of the element (a chain rooted at the slot, a nested structure's own members inside its block, no
  block twice, the slot facts of decision 0043 item 4 exactly or none) and, per LEAF path, the rows of a scalar range atom with the leaf receiver `loc(member(
  <chain>, j), 0)` as trigger and cell (`acc` of the value and of the flag with an amount that is a `Perm` formal, the flag, the type's range, the flag implication, in
  an `ensures` `old(flag) ==> flag`); a leaf row needs the length-facts row of its range in the same clause kind and that row must state the member block of the leaf
  and of every nested structure above it (a permission over a member block of unknown length is not an atom).  A method with `v := alloc_arr_<E>(N)` of an
  aggregate element exhales at every exit exactly what the allocation gave: each run of `exhale forall` rows over `block_of(v)` is the allocation's set of (receiver,
  field) rows over `0 <= k && k < N`, directly preceded (a `perm(lvloc(block_of(v)).lv) == write` assert may stand between) by a run of marked `assert`s of the same
  set, and each `exhale acc(lvloc(block_of(v)).lv)` stands directly after the whole run of its exhales (a method's missing-return `assert false` hides nothing), a method that allocates and has no exit at all and no `assert false` is refused (the loops' back-edge rows are other runs).
  The amount a call row divides for a `reads_range` over structures is `n > 0 ? <minimum> : <liveness>` where the minimum (nested conditionals) takes both the value and the
  flag permission of each leaf cell of the range's first slot (`loc(member(<slot>, j), 0)`, nested for a nested leaf), the liveness cell and the method's own `Perm` formals, nothing else
  (decision 0064 item 7, Amounts: `_struct_range_minimum_ok`)
* arrays of structures in storage (decision 0064 items 3, 9 and 17, M4.5e1e2 Part C2): `program_start`'s group of an array global of structures is the length facts over the
  slots, then per leaf of the element its three rows with the leaf receiver `loc(member(<chain>, j), 0)` as trigger, the listed values element by element (leaves in
  declaration order), per leaf the quantifier of the rest (where the list ends) or of an unknown value's range, last `acc(lvloc(GB_g()).lv)` (`aggregate_global_group`; a
  structure global's array member of structures has the same group over `member(loc(GB_g(), 0), k)` without the liveness row); `alloc_struct_<n>_<tag>` of a structure
  whose member is such an array states `len(member(X, k)) == N`, the length facts of the slots, then the three rows per leaf (`aggregate_member_rows`); in a method's
  `requires` and `ensures` the same rows over a block term, each leaf row tied to a length-facts row of the same range and clause kind that states its member blocks
  (`parse_block_leaf_row`, `block_row_problem`, `lint_block_length_facts`)
* the user quantifiers of decision 0061 item 11 (M4.5e1c; `cerastes_forall`): `forall q0: Int[, q1: Int ...] :: { loc(block_of(X), q0)[, ...] } body`, the
  binders named `q<j>`, ONE trigger set holding one term per binder with X a variable, an antecedent that holds the type's range of the bound
  variable (`lo_T <= (q0 - offset_of(X)) && (q0 - offset_of(X)) <= hi_T`), a body with no `acc`, `perm`, `forperm`, `inhale` or `forall` and no
  computed `loc` over a binder (`loc(B, q0 + 1)`: T1's form); a clause of its own (under at most a guard `g ==> `) of a method's `requires`,
  `ensures`, `invariant` or `assert`; the one permission it may hold is item 10's marked row, an unguarded `assert forall q0 :: { loc(block_of(X),
  q0) } reach ==> acc(loc(block_of(X), q0).f, wildcard)` (reach heap-free) whose map entry carries `assert.failed[insufficient.permission]`:
  `wildcard` stands nowhere else.  In a `<f>__wf` method (variant M of item 7) the heap does not exist: a read under a binder is a ghost function
  `hvq_<method>__<formal>_pre|post` (result `Int`, `Bool` or `Ref`) of ONE domain per method tagged `// cerastes:domain-ok 0061` (bodiless `function f(a:
  Int|Block, ...): Int|Bool|Ref` and nothing else: no axiom, no body, no contract; used by that `__wf` method alone); its `requires` that mention
  a ghost function are exactly item 7's four assumed facts, each checked against the atoms of the method `f` the `__wf` belongs to (audit round 1,
  B2): (a) the type range of one application, the element type's (an integer type's bounds, a pointer's well-formedness, none for `Bool`; the
  ghost's result type is that type's Viper type), one trigger set; (b) the scalar tie `G(offset_of(p)[ + e]) == hv_<id>_<state>` of a formal ghost of
  the function's own state, `e` an expression over the method's formals; (c) two parameters on one block, `block_of(p) == block_of(q) ==> forall s ::
  { Gp(s) } { Gq(s) } Gp(s) == Gq(s)`, only between formals of the same cell field; (d) a reads place keeps its value, tied to a `reads_range(p, n)`
  `requires` atom with an `ensures` partner of the same field, amount and count: `forall s :: { post(s) } { pre(s) } offset_of(p) <= s && s <
  offset_of(p) + n ==> post(s) == pre(s)` under EXACTLY the conjunction of the two atoms' guards (none if neither has one), and for `reads(*p)` the
  ground form `post(offset_of(p)) == pre(offset_of(p))` under the same guards; one trigger set per application a fact relates.  Its quantified
  `assert` rows are `[PRIOR ==> ] (forall q<j> :: { G(q<j>), ... } ...)` over ghost applications with no heap read and no computed application, the
  prior's quantifiers conjuncts of their own (a guard `g ==> ` allowed) and none inside a body; the binders of the facts and of the prior's clauses
  are `s`, `s0`, ... (the ghost functions are named after their method and the Viper formal, `hvq_<method>__<formal>_<state>`: Viper names are program-wide);
* the Viper functions of the `pure` functions of decision 0062 item 12 (M4.5e1d): a top-level `function` whose name starts `pf_` is `pf_<name>__<id>`
  (the pattern is `PF_NAME`, one line), bodied, with exactly one `decreases` line (a measure, or empty for a function that does not apply itself;
  `_` and `*` stay refused by the termination rules); its `ensures` hold no `acc`, `perm`, `old`, `forperm`, `inhale` or `assume`, nor its body; every
  application of itself in its body begins a line (the position map is keyed by line); the only cycle among `pf_` functions is such a
  self-application (no mutual recursion, none through a `requires`, an `ensures` or a `decreases`); a user quantifier of item 11's shape stands in its
  `requires` and `ensures`; an application of a `pf_` function stands in a `pf_` function, a method's `requires`, `ensures`, `invariant` or `assert`
  (on the line of its keyword), never in a `__wf` method, a `decreases`, a prelude or other function or a method's statement, and its callee is a
  `pf_` function of the file; `HEAP_READ_RE` counts an application as a read of memory, so every position that must read none refuses it; the ghost
  functions `hva_<method>__<callee>_<pre|post>` of an `f__wf` method's domain have `Int`, `Bool` or `Ref` parameters and their callee's signature, and the
  `requires` over them are exactly the facts (i) the type range of an application (the callee's result tag `// cerastes:result <lo> <hi>` on its signature
  line, itself a standard C integer range; unguarded), (ii) the callee's `ensures` at the arguments under V, exactly the callee's `requires` that read
  no memory and its user quantifiers over the method's `hvq` ghosts at the arguments (a V without the quantified clause proves what the callee never
  promised), (iii) the keep fact under the guards of a `reads_range` atom and its `ensures` partner, the same block and the inclusion of every range atom
  of the callee at the arguments in that atom's interval (read from the callee's own atoms; none: refused); every `pf_` function has its `__wf` method;
  the prelude functions apply only prelude functions, in no cycle; the map rows of a function: each `requires` `not.wellformed` (`internal`), each
  `ensures` `postcondition.violated` (`postcondition`) and `not.wellformed` (`internal`), a `decreases` measure `termination.failed` (`termination`),
  each body line that begins an application of the function itself `termination.failed` (`termination`) and no other body line any entry.  What a
  `pf_` function's `requires` lines may be is decision 0062 item 6's form Min (`pf_requires_problems`); a non-`pf_`, non-prelude function applies only
  prelude functions (the owner's H');
* an empty input set, and a MANIFEST that names a file which is not there.  The
  manifest is the set of files scripts/run_tests.py expects: its first line is
  `# run-id: <id>` (printed by the run), then one file per line.  Fewer files
  than it lists is a failure, so is an empty manifest, one without a run id,
  and a `.vpr` file that the manifest does not list (a stale file, or one from
  a partial run: the directory is cleared at the start of every run that
  selects a verify test).  With no MANIFEST and no
  `.vpr` file at all, the lint passes (`no emitted Viper to lint`) only when no
  verify test of the tree expects Viper (a `verified` or `error` test in C or
  C++): once one exists, an absent or empty `build/vpr` is a failure.

A comment is `//` to the end of the line, or `/* ... */`; a tag in a block
comment does not count.  Exit status: 0 clean, 1 problems, 2 usage error.
"""

from __future__ import annotations

import argparse
import dataclasses
import re
import sys
from pathlib import Path
from typing import Callable

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_tests as rt  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DIR = Path("build") / "vpr"
MANIFEST = rt.VPR_MANIFEST
MAP_SUFFIX = ".map"

# The categories a map entry may carry: the driver's (decision 0011 item 10)
# and `internal`, for the checks whose failure is a bug of the encoder and exit
# status 3 (the automatic range invariants of decision 0012 item 8).
MAP_CATEGORIES = frozenset(rt.DRIVER_CATEGORIES | {"internal"})
# The Viper error ids Silicon's messages map to (the table of scripts/run_tests.py).
# `application.precondition` is not one of them: the prelude's functions have no preconditions
# (encoding.md section 5), so no emitted check can produce it.
MAP_ERROR_IDS = frozenset(viper_id for _text, viper_id, _cat in rt._VIPER_ERRORS
                          if viper_id != "application.precondition")
# The second column of a map line is `<id>`, or `<id>[<reason>]` on a marked range row (decision 0059 item 7): the only
# reason the driver accepts on a marked row is `insufficient.permission`, and only `assert.failed` is marked.
MAP_REASONS = frozenset({"insufficient.permission"})
MAP_MARKED_ID = "assert.failed"
MAP_ID_RE = re.compile(r"^(?P<id>[A-Za-z][\w.]*)(?:\[(?P<reason>[^\[\]]*)\])?$")

INHALE_TAG_RE = re.compile(r"//\s*cerastes:inhale-ok\b[ \t]*(?P<reason>\S.*)?$")
TRUSTED_TAG_RE = re.compile(r"//\s*cerastes:trusted-contract\b[ \t]*(?P<reason>\S.*)?$")
DOMAIN_TAG_RE = re.compile(r"//\s*cerastes:domain-ok\b[ \t]*(?P<record>\S+)?")
AXIOM_TAG_RE = re.compile(r"//\s*cerastes:axiom-ok\b[ \t]*(?P<record>\S+)?")
DECISIONS_DIR = ROOT / "internal" / "decisions"
BLOCK_DOMAIN_RE = re.compile(r"^\s*domain\s+Block\s*\{")
AXIOM_DECL_RE = re.compile(r"^\s*axiom\s+(?P<name>[A-Za-z_$][\w$']*)\s*\{")
NEW_RE = re.compile(r"(?<![\w$'.])new\s*\(")
INHALE_WORD_RE = re.compile(r"\b(inhale|assume)\b")
CHECK_LINE_RE = re.compile(r"^\s*(assert|requires|ensures|invariant|decreases)\b")
CHECK_WORD_RE = re.compile(r"\b(assert|requires|ensures|invariant|decreases)\b")
DECREASES_LINE_RE = re.compile(r"^\s*decreases\b(?P<measure>.*)$")
WHILE_WORD_RE = re.compile(r"(?<![\w$'])while\b")
# What ends a `decreases` clause that runs over several lines: a line that opens a block (`{`, `}`), starts
# another clause, a statement or a declaration (a line that starts with anything else continues the clause).
CLAUSE_END_RE = re.compile(r"^\s*(?:[{}]|(?:assert|requires|ensures|invariant|decreases|while|var|label|goto|inhale|"
                           r"exhale|assume|fold|unfold|apply|package|else|method|function|predicate|domain|field|"
                           r"define|import|axiom|adt|type)(?![\w$']))")
# The Viper error ids a position-map entry may have, by the kind of the line it is
# for (encoding.md section 5): an `assert` is `assert.failed`; a call of a method is
# `call.precondition`; an `invariant` clause is `invariant.not.established` and
# `invariant.not.preserved` (two entries on one line); a method `requires` is
# `not.wellformed` (a failed precondition is reported at the call, never at the clause);
# a method `ensures` is `not.wellformed` and `postcondition.violated`; a `decreases` line with a measure
# is `termination.failed` (a loop's measure is where Silicon reports a loop that does not terminate), and a
# call may also be `termination.failed` (a recursive call whose measure does not decrease, decision 0013).
KIND_IDS = {
    "assert": {"assert.failed"},
    "requires": {"not.wellformed"},
    "ensures": {"not.wellformed", "postcondition.violated"},
    "invariant": {"invariant.not.established", "invariant.not.preserved"},
    "call": {"call.precondition", "termination.failed"},
    "decreases": {"termination.failed"},
}
TOKEN_RE = re.compile(r"[A-Za-z_$][\w$']*|\d+|::|:=|\S")
# Decision 0062 item 12: the Viper function of a `pure` function is `pf_<name>__<id>`.  The pattern of that name is written here and
# nowhere else (a change of the `<id>` rule is this line); `PF_ANY_RE` is every application of a name that starts `pf_`.
PF_NAME = r"pf_[A-Za-z_$][\w$']*__[0-9]+"
PF_NAME_RE = re.compile(rf"^{PF_NAME}$")
PF_APP_RE = re.compile(rf"(?<![\w$'.])(?P<name>{PF_NAME})\s*\(")
PF_ANY_RE = re.compile(r"(?<![\w$'.])(?P<name>pf_[\w$']*)\s*\(")
TOP_KEYWORDS = ("method", "function", "predicate", "domain", "field", "define", "import")


# ---------------------------------------------------------------------------
# Reading Viper text
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class Source:
    code: list[str]  # per line: the line without comments
    comments: list[str]  # per line: its `//` comment text ("" if none), with the `//`


def split_source(text: str) -> Source:
    """Split each line into code and `//` comment; block comments are blanked
    from the code (their text never counts as a tag)."""
    code: list[str] = []
    comments: list[str] = []
    in_block = False
    for raw in text.split("\n"):
        out = []
        comment = ""
        i = 0
        while i < len(raw):
            if in_block:
                j = raw.find("*/", i)
                if j < 0:
                    i = len(raw)
                else:
                    in_block = False
                    out.append(" ")
                    i = j + 2
                continue
            if raw.startswith("//", i):
                comment = raw[i:]
                break
            if raw.startswith("/*", i):
                in_block = True
                i += 2
                continue
            out.append(raw[i])
            i += 1
        code.append("".join(out))
        comments.append(comment)
    return Source(code, comments)


@dataclasses.dataclass
class Decl:
    kind: str
    name: str
    line: int  # 1-based line of the keyword
    has_body: bool = False
    body_line: int = 0  # 1-based line of the `{` that opens the body (0: none)
    end_line: int = 0  # 1-based last line of the declaration (set by declarations())


def declarations(src: Source) -> list[Decl]:
    """The top-level declarations, with whether each has a body."""
    tokens: list[tuple[str, int]] = []
    for lineno, line in enumerate(src.code, start=1):
        tokens += [(m.group(), lineno) for m in TOKEN_RE.finditer(line)]
    decls: list[Decl] = []
    depth = 0  # braces
    paren = 0
    i = 0
    n = len(tokens)
    trigger_end = -2  # index of the `}` that closed the last trigger set
    while i < n:
        text, lineno = tokens[i]
        prev = tokens[i - 1][0] if i else ""
        if text == "{":
            # a quantifier trigger set (`:: { f(x) }`, and the sets that follow it:
            # `{ f(x) } { g(x) }`), not a body: skip it
            if prev == "::" or i - 1 == trigger_end:
                level = 0
                while i < n:
                    if tokens[i][0] == "{":
                        level += 1
                    elif tokens[i][0] == "}":
                        level -= 1
                        if level == 0:
                            break
                    i += 1
                trigger_end = i
            else:
                if depth == 0 and paren == 0 and decls and not decls[-1].has_body:
                    decls[-1].has_body = True
                    decls[-1].body_line = lineno
                depth += 1
        elif text == "}":
            depth = max(0, depth - 1)
        elif text == "(":
            paren += 1
        elif text == ")":
            paren = max(0, paren - 1)
        elif depth == 0 and paren == 0 and text in TOP_KEYWORDS and prev != "::":
            name = tokens[i + 1][0] if i + 1 < n else ""
            decls.append(Decl(text, name, lineno))
        i += 1
    nlines = len(src.code)
    for k, d in enumerate(decls):
        d.end_line = decls[k + 1].line - 1 if k + 1 < len(decls) else nlines
    return decls


# ---------------------------------------------------------------------------
# Position maps (encoding.md section 5)
# ---------------------------------------------------------------------------

@dataclasses.dataclass(frozen=True)
class MapEntry:
    vpr_line: int
    error_id: str
    category: str
    file: str
    line: int
    column: int
    message: str
    note: str | None
    map_line: int  # line of the map file, for messages
    # a marked range row (decision 0059 item 7): the Silicon reason that is also this row's failure, written
    # `<id>[<reason>]` in the second column; "" for every other row
    expected_reason: str = ""


def parse_map(text: str, name: str) -> tuple[list[MapEntry], list[str]]:
    """The entries of a position map and the problems with its format: one line
    per check, tab-separated `vpr line, viper error id, category, file, line,
    column, message [, note]`; the id of a marked range row is `assert.failed[insufficient.permission]`
    (decision 0059 item 7), read as the id and its expected reason."""
    entries: list[MapEntry] = []
    problems: list[str] = []
    for lineno, raw in enumerate(text.split("\n"), start=1):
        if not raw.strip():
            continue

        def bad(why: str) -> None:
            problems.append(f"{name}:{lineno}: {why}")
        fields = raw.split("\t")
        if len(fields) not in (7, 8):
            bad(f"a position map line has 7 or 8 tab-separated fields, found {len(fields)}")
            continue
        try:
            vpr_line, line, column = int(fields[0]), int(fields[4]), int(fields[5])
        except ValueError:
            bad("the Viper line, line and column of a map entry must be integers")
            continue
        if vpr_line < 1 or line < 1 or column < 1:
            bad("the Viper line, line and column of a map entry start at 1")
            continue
        idm = MAP_ID_RE.match(fields[1])
        if idm is None or idm.group("id") not in MAP_ERROR_IDS:
            bad(f"unknown Viper error id `{fields[1]}`")
            continue
        error_id, reason = idm.group("id"), idm.group("reason")
        if reason is not None:
            if reason not in MAP_REASONS:
                bad(f"unknown expected reason `[{reason}]` (known: " + ", ".join(sorted(MAP_REASONS)) + "; decision 0059 item 7)")
                continue
            if error_id != MAP_MARKED_ID:
                bad(f"an expected reason is only on `{MAP_MARKED_ID}`, found `{fields[1]}` (decision 0059 item 7)")
                continue
        if fields[2] not in MAP_CATEGORIES:
            bad(f"unknown category `{fields[2]}` (known: "
                + ", ".join(sorted(MAP_CATEGORIES)) + ")")
            continue
        if not fields[3].strip() or not fields[6].strip():
            bad("a map entry needs a file and a message")
            continue
        if len(fields) == 8 and not fields[7].strip():
            bad("an empty note field")
            continue
        entries.append(MapEntry(vpr_line, error_id, fields[2], fields[3], line, column,
                                fields[6], fields[7] if len(fields) == 8 else None, lineno, reason or ""))
    return entries, problems


# ---------------------------------------------------------------------------
# One .vpr file
# ---------------------------------------------------------------------------

class CallStatement:
    """`match(line)`: the line is exactly one call statement, `name(args)` or
    `targets := name(args)`, with the call's parentheses closing at the end of the
    line (so `p := h(); p := h()` is not one)."""

    PREFIX = re.compile(r"^\s*(?:[A-Za-z_$][\w$']*(?:\s*,\s*[A-Za-z_$][\w$']*)*\s*:=\s*)?"
                        r"(?P<name>[A-Za-z_$][\w$']*)\s*\(")

    def match(self, code: str):
        m = self.PREFIX.match(code)
        if m is None:
            return None
        depth = 1
        i = m.end()
        while i < len(code) and depth:
            depth += (code[i] == "(") - (code[i] == ")")
            i += 1
        return m if depth == 0 and code[i:].strip() in ("", ";") else None


def decreases_measures(src: Source) -> dict[int, str]:
    """The measure of every `decreases` clause, by the line of its keyword: the text after the keyword,
    and the lines that follow while they do not start a block, another clause, a statement or a
    declaration (`CLAUSE_END_RE`), so that a clause written over several lines (`decreases` on one line and
    `*` or `if c` on the next) is read whole.  Blank lines (and comments) inside a clause are skipped."""
    out: dict[int, str] = {}
    for lineno, code in enumerate(src.code, start=1):
        m = DECREASES_LINE_RE.match(code)
        if not m:
            continue
        parts = [m.group("measure")]
        for more in src.code[lineno:]:
            if more.strip() and CLAUSE_END_RE.match(more):
                break
            parts.append(more)
        out[lineno] = " ".join(" ".join(parts).split())
    return out


BIT_FUNCTION_RE = re.compile(r"^c_(?P<u>u?)(?P<op>and|or|xor)_(?P<width>32|64|128)$")


# The mask functions of `^` (decision 0027 item 3 as revised): `c_[u]xor_mask_N_k(a)` is a ^ (2^k - 1) and
# `c_[u]xor_cmask_N_k(a)` is a ^ ~(2^k - 1); their bodies are remainder forms, their one `ensures` is a range implication.
# Exactly what `parse_xor_mask_fn` (src/viper/prelude.cpp) reads, tightened: k is a decimal without leading zeros, with
# `0 <= k <= N` for the unsigned functions and `0 <= k <= N - 1` for the signed ones (a signed `2^N` is not a value of the type).
XOR_MASK_FUNCTION_RE = re.compile(r"^c_(?P<u>u?)xor_(?P<c>c?)mask_(?P<width>32|64|128)_(?P<k>0|[1-9][0-9]{0,2})$")


def xor_mask_function_ok(name: str) -> bool:
    m = XOR_MASK_FUNCTION_RE.match(name)
    return m is not None and int(m.group("k")) <= int(m.group("width")) - (0 if m.group("u") else 1)


def bit_range_ensures_lines(src: Source, decls: list[Decl]) -> set[int]:
    """The lines of the range postconditions that a bitwise prelude function may carry without a position map
    entry (decision 0012 item 11, amended for 0023): in `function c_[u]<op>_<N>(a: Int, b: Int): Int` the single
    header line `ensures <lo> <= result && result <= <hi>`, where `[lo, hi]` is the range of the computed type
    (`(-2^(N-1)) .. 2^(N-1)-1` for the signed functions, `0 .. 2^N-1` for the unsigned ones).  Any other
    `ensures` line, and a second one in the same function, is not exempt."""
    out: set[int] = set()
    for d in decls:
        if d.kind == "function" and d.has_body and xor_mask_function_ok(d.name):
            out |= xor_mask_range_ensures_line(src, d)
            continue
        m = BIT_FUNCTION_RE.match(d.name) if d.kind == "function" else None
        if m is None or not d.has_body:
            continue
        head = " ".join(src.code[d.line - 1].split())
        if not re.fullmatch(r"function " + re.escape(d.name) + r"\(a: Int, b: Int\): Int", head):
            continue
        width = int(m.group("width"))
        lo, hi = (0, 2 ** width - 1) if m.group("u") else (-2 ** (width - 1), 2 ** (width - 1) - 1)
        want = f"ensures {'(' + str(lo) + ')' if lo < 0 else lo} <= result && result <= {hi}"
        lines = [n for n in range(d.line + 1, d.body_line) if src.code[n - 1].strip().startswith("ensures")]
        if len(lines) == 1 and " ".join(src.code[lines[0] - 1].split()) == want:
            out.add(lines[0])
    return out


def xor_mask_range_ensures_line(src: Source, d: Decl) -> set[int]:
    """The single range postcondition of `function c_[u]xor_[c]mask_N_k(a: Int): Int`, exactly
    `ensures (LO <= a && a <= HI) ==> (LO <= result && result <= HI)` with `[LO, HI]` the range of the N-bit type
    (`LO` in parentheses when negative); empty when the header, the line or the count of `ensures` lines differs."""
    m = XOR_MASK_FUNCTION_RE.match(d.name)
    head = " ".join(src.code[d.line - 1].split())
    if m is None or not xor_mask_function_ok(d.name) or head != f"function {d.name}(a: Int): Int":
        return set()
    width = int(m.group("width"))
    lo, hi = (0, 2 ** width - 1) if m.group("u") else (-2 ** (width - 1), 2 ** (width - 1) - 1)
    lo_text = f"({lo})" if lo < 0 else str(lo)
    want = f"ensures ({lo_text} <= a && a <= {hi}) ==> ({lo_text} <= result && result <= {hi})"
    lines = [n for n in range(d.line + 1, d.body_line) if src.code[n - 1].strip().startswith("ensures")]
    if len(lines) == 1 and " ".join(src.code[lines[0] - 1].split()) == want:
        return {lines[0]}
    return set()


# The prelude functions whose bodies use Viper's `%` and `\` (src/viper/prelude.cpp), by operator.  `%`: `c_urem` guards
# `a >= 0 && b > 0` and `c_rem` calls it with the absolute values.  `\`: `c_div` and `c_udiv` divide by the variable `b`
# in the same guarded way, `c_shr` by `c_pow2(n)`.  The bitwise functions `c_[u]<and|or|xor>_<N>` also use `\`, but only
# with a power-of-two literal (`a \ 2`, `a \ 2^64`), so they need no exemption and are held to the rule.
DIVISION_PRELUDE = {"%": frozenset({"c_rem", "c_urem"}), "\\": frozenset({"c_div", "c_udiv", "c_shr"})}
DIVISOR_RE = re.compile(r"\s*(?P<lit>[0-9]+)(?![\w.$'])")
DIVISION_WHAT = {
    "%": "Viper's Euclidean remainder and is emitted only by a constant mask (`a % 2^k`)",
    "\\": "Viper's floor division and is emitted only by a bit function or a constant mask (`2^k * (a \\ 2^k)`)",
}


def lint_remainders(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0026 item 7: Viper's `%` and `\\` are the Euclidean remainder and the floor division (a remainder in
    `[0, divisor)` for a positive divisor), C's `%` and `/` truncate, and the two must never be confused: C's go
    through `c_rem`/`c_urem` and `c_div`/`c_udiv`.  The emitted ones outside those prelude bodies are the constant-mask
    rewrite (`a % 2^k`, `2^k * (a \\ 2^k)`) and the bit functions' `\\ 2^j`, whose divisor is a power of two.  So outside the
    bodies of the prelude functions that use the operator (`DIVISION_PRELUDE`, by name), every `%` and `\\` must be
    followed by a plain decimal literal 2^k (k >= 0, no sign, no parentheses, no leading zeros); anything the rule
    cannot read as that fails."""
    exempt: dict[str, set[int]] = {op: set() for op in DIVISION_PRELUDE}
    for d in decls:
        if d.kind == "function" and d.has_body:
            for op, names in DIVISION_PRELUDE.items():
                if d.name in names:
                    exempt[op].update(range(d.body_line, d.end_line + 1))
    for lineno, code in enumerate(src.code, start=1):
        for m in re.finditer(r"[%\\]", code):
            op = m.group()
            if lineno in exempt[op]:
                continue
            names = " and ".join(f"`{n}`" for n in sorted(DIVISION_PRELUDE[op]))
            d = DIVISOR_RE.match(code, m.end())
            if d is None:
                add(lineno, f"a `{op}` whose divisor is not a decimal literal: outside the prelude's {names}, "
                            f"`{op}` is {DIVISION_WHAT[op]}, with a power of two as divisor (decision 0026 item 7)")
                continue
            lit = d.group("lit")
            n = int(lit)
            if (len(lit) > 1 and lit[0] == "0") or n < 1 or n & (n - 1):
                add(lineno, f"a `{op}` with the divisor `{lit}`: outside the prelude's {names}, the divisor of "
                            f"`{op}` must be a power of two, 1 or more, written as a decimal literal "
                            "(decision 0026 item 7)")


def tag_above(src: Source, line: int, tag_re: re.Pattern):
    """The match of `tag_re` in the run of comment-only lines directly above `line` (1-based), or None.
    A blank line or a line with code ends the run."""
    i = line - 2
    while i >= 0 and not src.code[i].strip() and src.comments[i]:
        if m := tag_re.search(src.comments[i]):
            return m
        i -= 1
    return None


def record_exists(record: str | None) -> bool:
    """Whether `record` is a number NNNN with a decision record internal/decisions/NNNN-*.md."""
    if record is None or not re.fullmatch(r"[0-9]{4}", record):
        return False
    return any(DECISIONS_DIR.glob(f"{record}-*.md"))


def block_close_line(src: Source, d: Decl) -> int:
    """The line of the `}` that closes the body of `d` (the last line of the file if it never closes)."""
    depth = 0
    for lineno in range(d.body_line or d.line, len(src.code) + 1):
        for ch in src.code[lineno - 1]:
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return lineno
    return len(src.code)


# The memory model's domain and axioms, exactly (decision 0028 item 2, records 0033-0036): the domain is tagged
# `domain-ok 0028`, each of the four axioms has its own record and this text (whitespace-normalized).  Anything else
# in the domain's axioms is refused, a fifth axiom and an altered body included.
DOMAIN_RECORD = "0028"
BLOCK_AXIOMS: dict[str, tuple[str, str]] = {
    "loc_inv": ("0033", "axiom loc_inv { forall b: Block, i: Int :: { loc(b, i) } "
                        "block_of(loc(b, i)) == b && offset_of(loc(b, i)) == i }"),
    "loc_nonnull": ("0034", "axiom loc_nonnull { forall b: Block, i: Int :: { loc(b, i) } loc(b, i) != null }"),
    "member_inv": ("0035", "axiom member_inv { forall r: Ref, k: Int :: { member(r, k) } "
                           "parent_of(member(r, k)) == r && index_of(member(r, k)) == k }"),
    "member_lv": ("0036", "axiom member_lv { forall r: Ref, k: Int :: { lvloc(member(r, k)) } "
                          "lvloc(member(r, k)) == lvloc(block_of(r)) }"),
}


def lint_domain(src: Source, d: Decl, add: Callable[[int, str], None]) -> bool:
    """Whether the `domain` at `d` is the memory model's: named `Block`, with a body and with
    `// cerastes:domain-ok 0028` directly above (the record exists).  A tag that names another record
    is reported here (and the domain is then refused with it)."""
    if d.name != "Block" or not d.has_body or not BLOCK_DOMAIN_RE.match(src.code[d.line - 1]):
        return False
    tag = tag_above(src, d.line, DOMAIN_TAG_RE)
    if tag is None:
        return False
    if tag.group("record") != DOMAIN_RECORD or not record_exists(DOMAIN_RECORD):
        add(d.line, f"`// cerastes:domain-ok {tag.group('record') or ''}` must name decision {DOMAIN_RECORD} "
                    "(internal/decisions/0028-*.md must exist)")
        return False
    return True


def axiom_text(src: Source, lineno: int) -> str:
    """The text of the axiom that starts at `lineno`, up to its closing brace, whitespace-normalized."""
    depth = 0
    parts: list[str] = []
    for ln in range(lineno, len(src.code) + 1):
        parts.append(src.code[ln - 1])
        depth += src.code[ln - 1].count("{") - src.code[ln - 1].count("}")
        if depth <= 0:
            break
    return " ".join(" ".join(parts).split())


def tagged_axiom_lines(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> set[int]:
    """The lines of the axioms that are accepted: inside an accepted `domain Block`, exactly the four of
    `BLOCK_AXIOMS`, each once, each with its record in `// cerastes:axiom-ok <record>` directly above (the record
    exists) and its exact text.  A missing one is reported at the domain."""
    ok: set[int] = set()
    for d in decls:
        if d.kind != "domain" or not lint_domain(src, d, lambda _l, _w: None):
            continue
        seen: set[str] = set()
        for lineno in range(d.line + 1, block_close_line(src, d) + 1):
            m = AXIOM_DECL_RE.match(src.code[lineno - 1])
            if not m:
                continue
            name = m.group("name")
            if name not in BLOCK_AXIOMS:
                add(lineno, f"`axiom {name}` is not one of the memory model's four axioms "
                            f"({', '.join(BLOCK_AXIOMS)}; decision 0028 item 2)")
                continue
            record, text = BLOCK_AXIOMS[name]
            if name in seen:
                add(lineno, f"`axiom {name}` twice in the domain")
                continue
            seen.add(name)
            tag = tag_above(src, lineno, AXIOM_TAG_RE)
            if tag is None:
                continue  # the generic refusal of an untagged axiom reports it
            if tag.group("record") != record or not record_exists(record):
                add(lineno, f"`// cerastes:axiom-ok {tag.group('record') or ''}` must name decision {record} for "
                            f"`axiom {name}` (internal/decisions/{record}-*.md must exist)")
                continue
            if axiom_text(src, lineno) != text:
                add(lineno, f"`axiom {name}` is not the text of decision {record}: `{text}`")
                continue
            ok.add(lineno)
        for name in BLOCK_AXIOMS:
            if name not in seen:
                add(d.line, f"`domain Block` lacks `axiom {name}` (decision {BLOCK_AXIOMS[name][0]}): the emitter "
                            "writes all four")
    return ok


# The trusted allocation of an address-taken local of type `<t>` (decision 0032 item 4), exactly as the emitter prints it: any method named
# `alloc_<t>` has this header and these two postconditions (whitespace-normalized), `decreases _` and nothing else.
ALLOC_PREFIX = "alloc_"
ALLOC_SUFFIX_RE = re.compile(r"^[A-Za-z0-9_]+$")


def alloc_contract(suffix: str) -> list[str]:
    return [
        f"method alloc_{suffix}() returns (r: Ref)",
        "ensures ((r != null) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == 1)",
        f"ensures ((((lvloc(block_of(r)) == loc(block_of(r), (-2))) && acc(r.v_{suffix})) && acc(r.d_{suffix})) && "
        f"(!r.d_{suffix})) && acc(lvloc(block_of(r)).lv)",
        "decreases _",
    ]


STRUCT_ALLOC_RE = re.compile(r"^struct_[0-9]+_[A-Za-z0-9_]+$")
FIELD_ANY_RE = re.compile(r"^\s*field\s+(?P<kind>[vd])_(?P<suffix>[A-Za-z0-9_$']+)\s*:")
_STRUCT_INDEX = r"(?:0|[1-9][0-9]*)"
_CHAIN_LOC_RE = re.compile(rf"^loc\(member\((?P<inner>.*),(?P<k>{_STRUCT_INDEX})\),0\)$")
_MEMBER_RE = re.compile(rf"^member\((?P<inner>.*),(?P<k>{_STRUCT_INDEX})\)$")


def parse_struct_chain(text: str, root: str = "r") -> tuple[int, ...] | None:
    """`root` (`r` for an allocation, the pointer's value for a `WF_S`) is (); `loc(member(<chain>, k), 0)` is the chain's tuple plus k,
    with a non-negative literal k and nothing else inside (no `block_of`, `parent_of`, other offset or arithmetic): the grammar of
    decision 0043 item 13."""
    if text == root:
        return ()
    m = _CHAIN_LOC_RE.match(text)
    if not m:
        return None
    inner = parse_struct_chain(m.group("inner"), root)
    return None if inner is None else inner + (int(m.group("k")),)


def parse_struct_member(text: str, root: str = "r") -> tuple[int, ...] | None:
    """`member(<chain>, k)` as the tuple of the chain plus k."""
    m = _MEMBER_RE.match(text)
    if not m:
        return None
    inner = parse_struct_chain(m.group("inner"), root)
    return None if inner is None else inner + (int(m.group("k")),)


def struct_wf_lens_problem(parts: list[str], root: str, counts: dict[tuple[int, ...], int] | None = None) -> str | None:
    """The guarded conjunct of `WF_S(v)` in a `program_start` row (decision 0043 item 8): under `v != null && offset_of(v) < len(block_of(v))`
    only `len(member(<chain>, k)) == 1` with a literal k, each member block once, a member of a member only inside a nested struct member,
    and the slot facts `A != B` exactly for the pairs of the root or a nested slot A and a nested slot B below it, chains rooted at the value.
    An array member of the structure states its count instead (decision 0063 items 3 and 15): `len(member(<chain>, k)) == N` with the `N` of
    `counts` (the array members of the allocation `alloc_struct_<n>_<tag>` of that structure, by member key); a count other than 1 for a
    member the allocation does not state as an array is refused.  Returns the reason, or None."""
    members: list[tuple[int, ...]] = []
    facts: list[tuple[tuple[int, ...], tuple[int, ...]]] = []
    for part in parts:
        if "!=" in part:
            a, _, b = part.partition("!=")
            ca, cb = parse_struct_chain(a, root), parse_struct_chain(b, root)
            if ca is None or cb is None:
                return f"the slot fact `{part}` is not between canonical chains rooted at the value"
            facts.append((ca, cb))
            continue
        m = re.fullmatch(r"len\((?P<m>.*)\)==(?P<n>[0-9]+)", part)
        key = parse_struct_member(m.group("m"), root) if m else None
        if key is None or int(m.group("n")) != (counts or {}).get(key, 1):
            if key is not None and key in (counts or {}):
                return (f"`{part}`: the allocation of the structure states `{(counts or {})[key]}` elements for the array member {key} "
                        "(decision 0063 item 15: the count of the allocation)")
            return f"`{part}` is not `len(member(<chain>, <literal>)) == 1` or a slot fact"
        if key in members:
            return f"the member block {key} twice"
        members.append(key)
    mset = set(members)
    if not mset:
        return "no member block"
    for key in mset:
        if key[:-1] and key[:-1] not in mset:
            return f"the member block {key} is not inside a nested struct member"
    structs = {k for k in mset if any(o[:-1] == k for o in mset)}
    expected = {(a, b) for b in structs for a in {()} | structs if len(b) > len(a) and b[:len(a)] == a}
    if len(facts) != len(set(facts)):
        return "a slot fact twice"
    if set(facts) != expected:
        return "the slot facts are not exactly the pairs of the root or a nested slot and a nested slot below it"
    return None


def aggregate_member_rows(middle: list[str], at: int, q1: dict[str, str], key: tuple[int, ...], n: int, declared: dict[str, set[str]]):
    """The rows of an array member whose elements are structures in `alloc_struct_<n>_<tag>` (decision 0064 items 3 and 17), `q1` being the length-facts row
    already read (clause `at - 1`): over `member(X, k)`'s slots `0 <= j && j < n`, `len(member(<chain>, i)) == 1` for each member block of the element (a nested
    structure's own facts inside it); then, for each leaf of the element in declaration order, the quantified rows `acc(C.v_t)`, `acc(C.d_t)`, `!C.d_t` with
    `C` the leaf receiver `loc(member(<chain>, i), 0)` rooted at the slot as the trigger.  Returns (the next clause, [(leaf, field suffix)]) or the reason."""
    if parse_struct_member(q1["b"]) != key:
        return f"the array member {key}: the length facts of its elements are over another block than the one `len(...) == {n}` states"
    if q1["lo"] != "0" or q1["hi"] != str(n):
        return f"the array member {key}: the length facts of its elements are not over `0 <= j && j < {n}`"
    slot = f"loc({q1['b']},{q1['k']})"
    keys, why = range_lens_members(q1["cons"], slot)
    if why is not None or keys is None:
        return f"the array member {key}: the length facts of its elements: {why}"
    out: list[tuple[tuple[int, ...], str]] = []
    for leaf in element_leaves(keys):
        cell = leaf_receiver(slot, leaf)
        suffix = ""
        for off, (what, shape) in enumerate((("acc({c}.v_{t})", r"acc\({c}\.v_(?P<t>[\w$']+)\)"), ("acc({c}.d_{t})", r"acc\({c}\.d_(?P<t>[\w$']+)\)"),
                                             ("!{c}.d_{t}", r"!{c}\.d_(?P<t>[\w$']+)"))):
            if at >= len(middle):
                return f"the array member {key}: the leaf {leaf} of its elements is not followed by its three quantified permissions (value, flag, `!flag`)"
            q = parse_slot_row(middle[at][len("ensures "):])
            at += 1
            if q is None or q["trig"] != cell or q["lo"] != "0" or q["hi"] != str(n) or q["k"] != q1["k"]:
                return (f"the array member {key}: a quantified row of the leaf {leaf} is not `forall j :: {{ {cell} }} 0 <= j && j < {n} ==> "
                        f"{what.format(c=cell, t='T')}` (value permission, flag permission, `!flag`, in this order)")
            cm = re.fullmatch(shape.format(c=re.escape(cell)), q["cons"])
            if cm is None or (off and cm.group("t") != suffix):
                return f"the array member {key}: a quantified row of the leaf {leaf} is not `{what.format(c=cell, t='T')}` of one field suffix"
            suffix = cm.group("t")
        if suffix not in declared["v"] or suffix not in declared["d"]:
            return f"the field suffix `{suffix}` is not declared (`field v_{suffix}` and `field d_{suffix}`)"
        out.append((leaf, suffix))
    return at, out


def struct_allocation_problem(src: Source, d: Decl, suffix: str) -> str | None:
    """Decision 0043 items 4 and 13: the trusted allocation `alloc_struct_<n>_<tag>` is pinned by its shape: bodyless, the header
    and `decreases _`; first the root facts of `alloc_<t>`; then clauses in which each member block has `len(member(X, k)) == 1`,
    each leaf exactly `acc(L.v_t) && acc(L.d_t) && !L.d_t` with `L = loc(member(X, k), 0)` and one declared suffix `t`, a nested struct
    member nothing else (its slot is the X of the clauses that follow), and the slot facts `A != B` of item 4, exactly the pairs
    (A, B) with B a nested slot and A the root or a nested slot whose chain B's chain strictly extends; last the liveness clause
    with exactly one `lv` permission on the literal `lvloc(block_of(r))`; every chain in the canonical grammar rooted at `r`, every
    member block and every (receiver, field) once; nothing else.  Returns the reason, or None."""
    if d.has_body:
        return "it has a body"
    lines = [" ".join(src.code[n - 1].split()) for n in range(d.line, d.end_line + 1) if src.code[n - 1].strip()]
    if len(lines) < 5 or lines[0] != f"method alloc_{suffix}() returns (r: Ref)" or lines[-1] != "decreases _":
        return "its header, `decreases _` or the number of clauses is not the emitter's"
    body = lines[1:-1]
    if not all(l.startswith("ensures ") for l in body):
        return "it has a clause that is not an `ensures`"
    root_text = alloc_contract("x")[1]
    if norm_viper(body[0][len("ensures "):]) != norm_viper(root_text[len("ensures "):]):
        return "its first `ensures` is not the root facts of `alloc_<t>`"
    live = norm_viper("lvloc(block_of(r)) == loc(block_of(r), -2) && acc(lvloc(block_of(r)).lv)")
    if norm_viper(body[-1][len("ensures "):]) != live:
        return "its last `ensures` is not exactly the liveness token (one `lv` permission on `lvloc(block_of(r))`)"
    declared = {"v": set(), "d": set()}
    for line in src.code:
        m = FIELD_ANY_RE.match(line)
        if m:
            declared[m.group("kind")].add(m.group("suffix"))
    members: set[tuple[int, ...]] = set()
    structs: set[tuple[int, ...]] = set()  # member blocks with no leaf: their slot is the next chain
    leaves: set[tuple[tuple[int, ...], str]] = set()
    facts: list[tuple[tuple[int, ...], tuple[int, ...]]] = []
    middle = body[1:-1]
    if not any("len(" in c for c in middle):
        return "it states no member block"
    arrays: set[tuple[int, ...]] = set()  # member blocks that are array members (decision 0063 item 6)
    at = 0
    while at < len(middle):
        clause = middle[at]
        at += 1
        standalone = re.fullmatch(r"len\((?P<m>member\(.*\))\)==(?P<n>[0-9]+)", norm_viper(clause[len("ensures "):]))
        if standalone is not None and (int(standalone.group("n")) != 1 or (at < len(middle) and middle[at].startswith("ensures forall"))):
            # an array member (decision 0063 items 6 and 15): `len(member(X, k)) == N` and then its three quantifiers, in this order, each
            # over the block `member(X, k)` with the trigger `{ loc(member(X, k), j) }` and the range `0 <= j && j < N`
            key = parse_struct_member(standalone.group("m"))
            n = int(standalone.group("n"))
            if key is None:
                return f"`{clause}` is not `len(member(<chain>, <literal>)) == N`"
            if n < 1:
                return f"the array member {key} has the count {n}"
            if key in members:
                return f"the member block {key} twice"
            if key[:-1] and key[:-1] not in structs:
                return f"the member block {key} is not inside a nested struct member"
            if at + 3 > len(middle):
                return f"the array member {key} is not followed by its three quantified permissions (value, flag, `!flag`)"
            members.add(key)
            arrays.add(key)
            if at < len(middle) and (q1 := parse_block_row(middle[at][len("ensures "):])) is not None and "len(member(" in q1["cons"]:
                # an array member whose elements are structures (decision 0064 items 3 and 17): the length facts of the slots, then per leaf of the
                # element the three quantified rows over the leaf receiver, in this order, each leaf once
                at += 1
                why = aggregate_member_rows(middle, at, q1, key, n, declared)
                if isinstance(why, str):
                    return why
                at, agg_leaves = why
                for leaf, t in agg_leaves:
                    leaves.add((key + (-1,) + leaf, t))
                continue
            suffix_seen: set[str] = set()
            for want in ("acc(loc(B,j).v_T)", "acc(loc(B,j).d_T)", "!loc(B,j).d_T"):
                q = parse_block_row(middle[at][len("ensures "):])
                at += 1
                if q is None or q["lo"] != "0" or q["hi"] != str(n):
                    return f"the array member {key}: a quantified row is not `forall j :: {{ loc(member(X, k), j) }} 0 <= j && j < {n} ==> ...`"
                if parse_struct_member(q["b"]) != key:
                    return f"the array member {key}: a quantified row is over another block than the one `len(...) == {n}` states"
                shape = {"acc(loc(B,j).v_T)": rf"acc\(loc\({re.escape(q['b'])},{re.escape(q['k'])}\)\.v_(?P<t>[\w$']+)\)",
                         "acc(loc(B,j).d_T)": rf"acc\(loc\({re.escape(q['b'])},{re.escape(q['k'])}\)\.d_(?P<t>[\w$']+)\)",
                         "!loc(B,j).d_T": rf"!loc\({re.escape(q['b'])},{re.escape(q['k'])}\)\.d_(?P<t>[\w$']+)"}[want]
                cm = re.fullmatch(shape, q["cons"])
                if cm is None:
                    return f"the array member {key}: a quantified row is not `{want.replace('B', 'member(X, k)')}` (value permission, flag permission, `!flag`, in this order)"
                suffix_seen.add(cm.group("t"))
            if len(suffix_seen) != 1:
                return f"the array member {key}: its three quantified rows are over more than one field suffix"
            (t,) = suffix_seen
            if t not in declared["v"] or t not in declared["d"]:
                return f"the field suffix `{t}` is not declared (`field v_{t}` and `field d_{t}`)"
            if (key, t) in leaves:
                return f"the leaf {key} twice"
            leaves.add((key, t))
            continue
        parts = norm_viper(clause[len("ensures "):]).split("&&")
        lens: list[tuple[int, ...]] = []
        accs: dict[str, tuple[tuple[int, ...], str]] = {}
        nots: list[tuple[tuple[int, ...], str]] = []
        for part in parts:
            if "!=" in part:
                a, _, b = part.partition("!=")
                ca, cb = parse_struct_chain(a), parse_struct_chain(b)
                if ca is None or cb is None:
                    return f"the slot fact `{part}` is not between canonical chains"
                facts.append((ca, cb))
                continue
            m = re.fullmatch(r"len\((?P<m>.*)\)==1", part)
            if m:
                key = parse_struct_member(m.group("m"))
                if key is None:
                    return f"`{part}` is not `len(member(<chain>, <literal>)) == 1`"
                lens.append(key)
                continue
            m = re.fullmatch(r"acc\((?P<l>.*)\.(?P<f>[vd])_(?P<t>[A-Za-z0-9_$']+)\)", part)
            if m:
                lc = parse_struct_chain(m.group("l"))
                if lc is None or not lc:
                    return f"`{part}` is not a permission to a canonical leaf"
                if m.group("f") in accs:
                    return f"`{part}`: a second {m.group('f')} permission in one clause"
                accs[m.group("f")] = (lc, m.group("t"))
                continue
            m = re.fullmatch(r"!(?P<l>.*)\.d_(?P<t>[A-Za-z0-9_$']+)", part)
            if m:
                lc = parse_struct_chain(m.group("l"))
                if lc is None or not lc:
                    return f"`{part}` is not `!L.d_<t>` of a canonical leaf"
                nots.append((lc, m.group("t")))
                continue
            return f"`{part}` is not one of the clauses of decision 0043 item 13"
        if not lens:
            if accs or nots:
                return "a leaf's permissions without its member block's `len(...) == 1`"
            continue  # the slot facts alone
        if len(lens) != 1:
            return "two member blocks in one clause"
        key = lens[0]
        if key in members:
            return f"the member block {key} twice"
        if key[:-1] and key[:-1] not in structs:
            return f"the member block {key} is not inside a nested struct member"
        members.add(key)
        if not accs and not nots:
            structs.add(key)
            continue
        if set(accs) != {"v", "d"} or len(nots) != 1:
            return "a leaf is not exactly `acc(L.v_t) && acc(L.d_t) && !L.d_t`"
        (lv_, tv), (ld_, td), (ln_, tn) = accs["v"], accs["d"], nots[0]
        if not (lv_ == ld_ == ln_ == key and tv == td == tn):
            return "a leaf's value, flag and `!d` are not one cell `loc(member(X, k), 0)` of one field suffix"
        if tv not in declared["v"] or tv not in declared["d"]:
            return f"the field suffix `{tv}` is not declared (`field v_{tv}` and `field d_{tv}`)"
        if (key, tv) in leaves:
            return f"the leaf {key} twice"
        leaves.add((key, tv))
    expected = {(a, b) for b in structs for a in {()} | structs if len(b) > len(a) and b[:len(a)] == a}
    if len(facts) != len(set(facts)):
        return "a slot fact twice"
    if set(facts) != expected:
        return ("the slot facts `A != B` are not exactly the pairs of the root or a nested slot A and a nested slot B below it "
                "(decision 0043 item 4)")
    return None


def lint_allocations(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0032 item 4: a bodyless method `alloc_<t>` is trusted (the tag rule), so its contract is pinned: a different
    postcondition (a dropped `!r.d_<t>`, an extra fact, `ensures false`, another suffix) is an unsound assumption.  The struct
    allocations `alloc_struct_<n>_<tag>` have their own shape (decision 0043 item 13)."""
    for d in decls:
        if d.kind != "method" or not d.name.startswith(ALLOC_PREFIX):
            continue
        suffix = d.name[len(ALLOC_PREFIX):]
        if d.name.startswith(ARR_ALLOC_PREFIX):
            continue  # the array allocations have their own rule (lint_array_allocations, decision 0059 item 3)
        if STRUCT_ALLOC_RE.match(suffix):
            why = struct_allocation_problem(src, d, suffix)
            if why is not None:
                add(d.line, f"`method {d.name}` is not the trusted struct allocation of decision 0043 items 4 and 13: {why}")
            continue
        lines = [" ".join(src.code[n - 1].split()) for n in range(d.line, d.end_line + 1) if src.code[n - 1].strip()]
        if d.has_body or not ALLOC_SUFFIX_RE.match(suffix) or lines != alloc_contract(suffix or "_"):
            add(d.line, f"`method {d.name}` is not the trusted allocation of decision 0032 item 4: it must be bodyless with "
                        "exactly the header, the two `ensures` lines and `decreases _` that the emitter prints for "
                        f"`alloc_{suffix or '<t>'}` (`ensures ((r != null) && ...` with the fields `v_{suffix or '<t>'}`/`d_{suffix or '<t>'}`)")


# Globals and `static` locals (decision 0041 item 4): each is a `unique function GB_<ident>(): Block` of `domain Block`, and the trusted
# `program_start()` gives `main` their initial state.  That method is an assumption (invariant 4), so its contract is pinned row by row, as
# `alloc_<t>` is; the pure facts about the blocks (`len(GB_x()) == 1 && lvloc(GB_x()) == loc(GB_x(), -2)`, true in 0028 item 2's model) are
# a hypothesis that every other method takes as its first `requires` and its callers discharge, never an axiom.
UNIQUE_RE = re.compile(r"(?<![\w$'])unique(?![\w$'])")
GB_DECL_RE = re.compile(r"^\s*unique\s+function\s+(?P<name>GB_[A-Za-z0-9_$']+)\(\)\s*:\s*Block\s*$")
GB_FUNCTION_RE = re.compile(r"^\s*function\s+GB_")
FIELD_DECL_RE = re.compile(r"^\s*field\s+v_(?P<suffix>[A-Za-z0-9_$']+)\s*:\s*(?P<type>[A-Za-z]+)\s*$")
PROGRAM_START = "program_start"
PROGRAM_START_CALL_RE = re.compile(r"(?<![\w$'])program_start\s*\(")
INT_LITERAL_RE = r"-?[0-9]+"


def norm_viper(text: str) -> str:
    """`text` without whitespace and without grouping parentheses (a `(` that does not follow a name is a group; the
    parentheses of a call or of `GB_x()` stay), so that the emitter's full parenthesization and a plain spelling
    of the same `&&` chain compare equal."""
    out: list[str] = []
    stack: list[bool] = []  # True: a call's parenthesis, kept
    prev = ""
    for ch in text:
        if ch.isspace():
            continue
        if ch == "(":
            keep = bool(prev) and (prev.isalnum() or prev in "_$'")
            stack.append(keep)
            if keep:
                out.append(ch)
        elif ch == ")":
            if (stack.pop() if stack else True):
                out.append(ch)
        else:
            out.append(ch)
        prev = ch
    return "".join(out)


# Decision 0063 item 15: the quantified rows over an array's slots in the lines that have no Viper local (`alloc_struct_*`'s `ensures`, the rows of
# `program_start`, a method's contract), the block-term form `forall k: Int :: { loc(B, k) } lo <= k && k < hi ==> CONSEQUENT` with B a block TERM:
# `GB_<g>()` of a declared array global or `member(<chain>, k)` with the canonical chain of decision 0043 item 13 rooted at a formal, a result,
# `r` or `loc(GB_<g>(), 0)`; lo and hi are literals.  (A row in a method's body is over a local, `block_of(v)`: the older rules.)
BLOCK_ROW_RE = re.compile(r"^forall(?P<k>[A-Za-z_$][\w$']*):Int::\{loc\((?P<b>.+?),(?P=k)\)\}(?P<lo>[0-9]+)<=(?P=k)&&(?P=k)<(?P<hi>[0-9]+)==>(?P<cons>.+)$")
GB_TERM_RE = re.compile(r"^(GB_[\w$']+)\(\)$")


def parse_block_row(text: str) -> dict[str, str] | None:
    """The fields of a block-term row (`text` is what follows the keyword; whitespace and grouping parentheses do not matter): k, b (the
    block term), lo, hi (literals) and cons (the consequent), or None when the text is not that shape (extra antecedents, a computed bound)."""
    m = BLOCK_ROW_RE.match(norm_viper(text))
    if m is None:
        return None
    out = {key: m.group(key) for key in ("k", "b", "lo", "hi", "cons")}
    close = text.find("}")
    if close >= 0:  # the consequent as written, grouping kept (see `parse_slot_row`)
        pieces = _split_top(_strip_wrap(text[close + 1:].strip()), "==>")
        if len(pieces) >= 2:
            out["raw"] = "==>".join(pieces[1:]).strip()
    return out


LOAD_FIELD_RE = re.compile(r"\.v_ptr_struct_[0-9]+_[\w$']+")


def load_root(b: str, roots: set[str]) -> tuple[str, str] | None:
    """Decision 0064 item 15 (contract places through a heap read, scalars only): the one pointer LOAD a block term is rooted at.  `b` (normalized)
    holds exactly one `.v_ptr_struct_<n>_<tag>` read, of a cell `loc(member(<chain>, k), 0)` with at least one member step, the chain canonical and
    rooted at one of `roots` (a formal, a result, `loc(GB_g(), 0)`); returns (the load's text, `b` with that text replaced by `LOAD`), or None."""
    hits = list(LOAD_FIELD_RE.finditer(b))
    if len(hits) != 1:
        return None
    end = hits[0].end()
    close = hits[0].start() - 1
    if close < 0 or b[close] != ")":
        return None
    depth, i = 0, close
    while i >= 0:
        depth += (b[i] == ")") - (b[i] == "(")
        if depth == 0:
            break
        i -= 1
    start = i - len("loc") if i >= 3 and b[i - 3:i] == "loc" else -1
    if start < 0:
        return None
    cell = b[start:hits[0].start()]
    for root in roots:
        if parse_struct_chain(cell, root) is not None:  # the root cell itself (`loc(GB_g(), 0)`, a `T **` formal) or a member cell below it
            return b[start:end], b[:start] + "LOAD" + b[end:]
    return None


def block_term(b: str, gbs: set[str], roots: set[str], loads: bool = False) -> tuple[str, str, tuple[int, ...]] | None:
    """What a normalized block term is: ("gb", `GB_g`, ()) for `GB_g()` of a declared global, ("member", root, chain+k) for `member(<chain>, k)`
    over a canonical chain rooted at one of `roots` (formals and results of the method, `r`, `loc(GB_g(), 0)` of a declared global), or rooted at one
    pointer load of such a chain (`load_root`: ("member", the load's text, chain+k)) where `loads` is set (only the rows of a contract, `block_row_problem`
    and `block_clause_problem`: `bound_locals` roots chains at a Viper variable, never at an expression); None otherwise."""
    m = GB_TERM_RE.match(b)
    if m is not None:
        return ("gb", m.group(1), ()) if m.group(1) in gbs else None
    all_roots = roots | {f"loc({g}(),0)" for g in gbs}
    for root in sorted(all_roots):
        key = parse_struct_member(b, root)
        if key is not None:
            return "member", root, key
    lr = load_root(b, all_roots) if loads else None
    if lr is not None:
        key = parse_struct_member(lr[1], "LOAD")
        if key is not None:
            return "member", lr[0], key
    return None


ZERO_LITERAL_OF_KIND = {"Int": "0", "Bool": "false"}


def zero_literal(fields: dict[str, str], suffix: str) -> str | None:
    kind = fields.get(suffix)
    if kind == "Ref" and suffix.startswith("ptr_"):
        return "null"
    return ZERO_LITERAL_OF_KIND.get(kind or "")


def global_facts_atoms(names: list[str], lens: dict[str, int] | None = None) -> list[str]:
    atoms: list[str] = []
    for n in names:
        atoms += [norm_viper(f"len({n}()) == {(lens or {}).get(n, 1)}"), norm_viper(f"lvloc({n}()) == loc({n}(), -2)")]
    return sorted(atoms)


def clause_texts(src: Source, d: Decl) -> list[tuple[str, int, str]]:
    """(keyword, first line, text after the keyword) of each clause of the header of the method `d`; a clause that runs over
    several lines is joined."""
    out: list[tuple[str, int, str]] = []
    end = d.body_line - 1 if d.has_body else d.end_line
    for n in range(d.line, max(d.line, end) + 1):
        code = src.code[n - 1]
        m = CLAUSE_START_RE.match(code)
        if m:
            out.append((m.group(1), n, code[m.end():]))
        elif out and code.strip() and not code.strip().startswith("{"):
            k, ln, text = out[-1]
            out[-1] = (k, ln, text + " " + code)
    return out


def first_statement(src: Source, d: Decl) -> tuple[str, int]:
    """The text and line of the first statement of the body of `d`: after the `var` declarations."""
    for n in range(d.body_line + 1, d.end_line + 1):
        code = src.code[n - 1].strip()
        if code and not code.startswith("var ") and not code.startswith("var\t"):
            return code, n
    return "", 0


def leaf_value(fields: dict[str, str], suffix: str, c: str, mid: str,
               counts: dict[str, dict[tuple[int, ...], int]] | None = None) -> tuple[bool, str | None]:
    """The value part of a `program_start` row for the cell `c` (normalized text) of field suffix `suffix` (decision 0041 item 4):
    a literal of the type, `null`, the range of an unknown value, `WF` / `WF_S` of an unknown pointer, or `loc(GB_h(), 0)` (returned
    as the target).  Returns (good, target)."""
    kind, target, good = fields.get(suffix), None, False
    if kind == "Int":
        good = re.fullmatch(re.escape(c) + rf"\.v_{suffix}==" + INT_LITERAL_RE, mid) is not None
        if not good and (r := re.fullmatch(rf"({INT_LITERAL_RE})<={re.escape(c)}\.v_{suffix}&&{re.escape(c)}\.v_{suffix}<=({INT_LITERAL_RE})", mid)):
            good = int(r.group(1)) <= int(r.group(2))
    elif kind == "Bool":
        good = mid in ("", f"{c}.v_{suffix}==true", f"{c}.v_{suffix}==false")
    elif kind == "Ref" and suffix.startswith("ptr_"):
        v = f"{c}.v_{suffix}"
        wf = f"{v}==null||{v}==loc(block_of({v}),offset_of({v}))&&0<=offset_of({v})&&offset_of({v})<=len(block_of({v}))"
        guard = rf"{re.escape(v)}!=null&&offset_of\({re.escape(v)}\)<len\(block_of\({re.escape(v)}\)\)"
        if mid in (f"{v}==null", wf):
            good = True
        elif mid.startswith(wf + "&&") and (r := re.fullmatch(guard + r"==>(?P<lens>.*)", mid[len(wf) + 2:])):
            # a pointer to a struct: WF followed by the guarded WF_S conjunct (decision 0043 item 8)
            good = struct_wf_lens_problem(r.group("lens").split("&&"), v, (counts or {}).get(suffix[len("ptr_"):])) is None
        elif r := re.fullmatch(re.escape(v) + r"==loc\((GB_[\w$']+)\(\),0\)", mid):
            target, good = r.group(1), True
    return good, target


_LEAF_PERM_RE = re.compile(r"^(?P<l>loc\(member\(.*\),[0-9]+\),0\))\.v_(?P<t>[\w$']+)\)$")
_LEAF_FLAG_RE = re.compile(r"^(?P<l>loc\(member\(.*\),[0-9]+\),0\))\.d_(?P<t>[\w$']+)\)&&(?P=l)\.d_(?P=t)(?:&&(?P<value>.*))?$")


def struct_row(row: str, fields: dict[str, str],
               counts: dict[str, dict[tuple[int, ...], int]] | None = None) -> tuple[str, list[tuple[tuple[int, ...], str, str | None]]] | str:
    """The `program_start` row of a struct global (decision 0044 items 8 and 13), normalized: `acc(L.v_t) && acc(L.d_t) && L.d_t && <value>`
    for every leaf L, then exactly one `acc(lvloc(GB_g()).lv)`, nothing else.  Every L is a canonical chain `loc(member(<chain>, k), 0)` rooted at
    `loc(GB_g(), 0)` (no `block_of`, `parent_of`, arithmetic), each leaf once and no leaf's chain a prefix of another's.  Returns (the global,
    [(leaf chain, field suffix, `GB_h` of an address value or None)]) or the reason."""
    m = re.search(r"&&acc\(lvloc\((GB_[\w$']+)\(\)\)\.lv\)$", row)
    if m is None:
        return "it does not end with exactly one `acc(lvloc(GB_x()).lv)`"
    g = m.group(1)
    root = f"loc({g}(),0)"
    parts = row.split("&&acc(")
    if not parts[0].startswith("acc("):
        return "it does not start with a permission"
    parts[0] = parts[0][len("acc("):]
    if parts[-1] != f"lvloc({g}()).lv)":
        return f"it does not end with exactly one `acc(lvloc({g}()).lv)`"
    body = parts[:-1]
    if not body or len(body) % 2:
        return "a leaf is not exactly `acc(L.v_t) && acc(L.d_t) && L.d_t && <value>`"
    leaves: list[tuple[tuple[int, ...], str, str | None]] = []
    for i in range(0, len(body), 2):
        pm, fm = _LEAF_PERM_RE.match(body[i]), _LEAF_FLAG_RE.match(body[i + 1])
        if pm is None or fm is None or pm.group("l") != fm.group("l") or pm.group("t") != fm.group("t"):
            return "a leaf is not exactly `acc(L.v_t) && acc(L.d_t) && L.d_t && <value>` over one cell `loc(member(X, k), 0)` and one field suffix"
        chain = parse_struct_chain(pm.group("l"), root)
        if chain is None or not chain:
            return f"the leaf `{pm.group('l')}` is not a canonical chain `loc(member(<chain>, k), 0)` rooted at `loc({g}(), 0)`"
        suffix = pm.group("t")
        good, target = leaf_value(fields, suffix, pm.group("l"), fm.group("value") or "", counts)
        if not good:
            return (f"the value of the leaf `{pm.group('l')}` (field `v_{suffix}`{'' if suffix in fields else ', not declared'}) is not a literal of its "
                    "type, `null`, `loc(GB_h(), 0)` of a declared global, or an unknown value's range / `WF`")
        leaves.append((chain, suffix, target))
    chains = [c for c, _s, _t in leaves]
    for a in chains:
        for b in chains:
            if a != b and b[:len(a)] == a:
                return f"the leaf {a} is a prefix of the leaf {b}: a nested struct is its leaves, not a cell"
    if len(chains) != len(set(chains)):
        return "a leaf twice (two full permissions to one cell would make `main` vacuous)"
    return g, leaves


def slot_text(g: str, chain: tuple[int, ...]) -> str:
    out = f"loc({g}(),0)"
    for k in chain:
        out = f"loc(member({out},{k}),0)"
    return out


def struct_facts_atoms(g: str, leaves: list[tuple[tuple[int, ...], str, str | None]], arrays: dict[tuple[int, ...], int] | None = None) -> list[str]:
    """The member lengths `len(member(X, k)) == 1` of every member block of the row's struct (every prefix of a leaf chain) and its slot facts
    `X != Y` (Y a nested slot, X the root or a nested slot whose chain Y's chain strictly extends), normalized (decision 0044 item 8)."""
    chains = [c for c, _s, _t in leaves] + list(arrays or {})
    blocks = {c[:i] for c in chains for i in range(1, len(c) + 1)}
    slots = {c[:i] for c in chains for i in range(1, len(c))}
    atoms = [f"len(member({slot_text(g, b[:-1])},{b[-1]}))=={(arrays or {}).get(b, 1)}" for b in sorted(blocks)]
    atoms += [f"{slot_text(g, a)}!={slot_text(g, b)}" for b in sorted(slots) for a in sorted({()} | slots) if len(b) > len(a) and b[:len(a)] == a]
    return atoms


def global_facts_atoms_of(names: list[str], structs: dict[str, list[tuple[tuple[int, ...], str, str | None]]],
                          lens: dict[str, int] | None = None, sarrays: dict[str, dict[tuple[int, ...], int]] | None = None) -> list[str]:
    """The facts of the unit (sorted atoms): each global's scalar facts (`len(GB_g()) == N` for an array global of `lens`, decision 0063 item 15:
    the same N in the three places the facts are pinned), and each struct global's member lengths and slot facts."""
    atoms = global_facts_atoms(names, lens)
    for g, leaves in structs.items():
        atoms += struct_facts_atoms(g, leaves, (sarrays or {}).get(g))
    return sorted(atoms)


def struct_array_counts(src: Source, decls: list[Decl]) -> dict[str, dict[tuple[int, ...], int]]:
    """The array members of each structure allocation `alloc_struct_<n>_<tag>`: `struct_<n>_<tag>` -> {member key: N} (decision 0063 item 15:
    `struct_lens`' conjunct states the count of the allocation).  Read leniently (the allocation's own rule reports a wrong one)."""
    out: dict[str, dict[tuple[int, ...], int]] = {}
    for d in decls:
        if d.kind != "method" or not d.name.startswith(ALLOC_PREFIX) or not STRUCT_ALLOC_RE.match(d.name[len(ALLOC_PREFIX):]):
            continue
        clauses = [norm_viper(t) for k, _l, t in clause_texts(src, d) if k == "ensures"]
        for i, c in enumerate(clauses[:-1]):
            m = re.fullmatch(r"len\((?P<m>member\(.*\))\)==(?P<n>[0-9]+)", c)
            key = parse_struct_member(m.group("m")) if m else None
            if key is not None and clauses[i + 1].startswith("forall"):
                out.setdefault(d.name[len(ALLOC_PREFIX):], {})[key] = int(m.group("n"))
    return out


ARRAY_GROUP_FIRST_RE = re.compile(r"^forall[A-Za-z_$][\w$']*:Int::\{loc\(GB_")


def array_global_group(clauses: list[tuple[str, int, str]], i: int, end: int, fields: dict[str, str], gbs: dict[str, int],
                       counts: dict[str, dict[tuple[int, ...], int]], lens_q0: dict[str, str] | None = None):
    """The row group of an array global in `program_start` (decision 0063 items 9 and 15), starting at clause `i` (the value permission), in
    this order, each row its own `ensures`: the two quantified permissions and the flags over `[0, N)` (`acc(loc(GB_g(), k).v_t)`,
    `acc(... .d_t)`, `loc(GB_g(), k).d_t`); the values (known: one conjunct `loc(GB_g(), i).v_t == c` per listed element, i = 0, 1, ..., then
    one quantifier `m <= k && k < N ==> ... == 0` (`false`, `null`) for the rest when m < N; unknown: `0 <= k && k < N ==> <the type's range or
    WF>` alone; a `bool` array of unknown value states none); last `acc(lvloc(GB_g()).lv)`.  Returns (the next clause index, (g, N, suffix,
    [(line, target)]) or None, [(line, why)]); after a problem the rest of the group is skipped (one report)."""
    problems: list[tuple[int, str]] = []
    first_line = clauses[i][1]
    q0 = lens_q0 if lens_q0 is not None else parse_block_row(clauses[i][2])
    m0 = GB_TERM_RE.match(q0["b"]) if q0 else None
    g = m0.group(1) if m0 else ""
    key: tuple[int, ...] | None = None  # a structure global's array leaf: the member key; the group has no liveness row (the cell row has it)
    if q0 is not None and m0 is None:
        for cand in gbs:
            k0 = parse_struct_member(q0["b"], f"loc({cand}(),0)")
            if k0 is not None:
                g, key = cand, k0
                break

    def fail(ln: int, why: str, at: int):
        problems.append((ln, f"the row group of an array global in `program_start` (decision 0063 item 9): {why}"))
        j = at
        if key is not None:
            return end, None, problems  # one report: the rest of the rows are not read
        while j < end and norm_viper(clauses[j][2]) != f"acc(lvloc({g}()).lv)":
            j += 1
        return min(j + 1, end), None, problems

    if q0 is not None and (m0 is not None or key is not None) and g in gbs and "len(member(" in q0["cons"]:
        return aggregate_global_group(clauses, i, end, fields, counts, q0, g, key, fail, lens_q0 is not None)
    if q0 is None or (m0 is None and key is None) or g not in gbs:
        return fail(first_line, "the first row is not the value permission `forall k: Int :: { loc(GB_g(), k) } 0 <= k && k < N ==> acc(loc(GB_g(), k).v_t)` "
                                "of a declared `GB_g` (or of an array member `member(loc(GB_g(), 0), k)` of a structure global)", i + 1)
    n = int(q0["hi"])
    if q0["lo"] != "0" or n < 1:
        return fail(first_line, f"the range is `0 <= k && k < N` with a literal N >= 1, found `{q0['lo']} <= k && k < {q0['hi']}`", i + 1)
    k, b = q0["k"], q0["b"]
    cell = f"loc({b},{k})"
    mv = re.fullmatch(rf"acc\({re.escape(cell)}\.v_(?P<t>[\w$']+)\)", q0["cons"])
    if mv is None or mv.group("t") not in fields:
        return fail(first_line, "the first row is not `acc(loc(GB_g(), k).v_t)` of a declared field `v_t`", i + 1)
    t = mv.group("t")
    want = [(f"acc({cell}.d_{t})", "the flag permission `acc(loc(GB_g(), k).d_t)`"), (f"{cell}.d_{t}", "the flags `loc(GB_g(), k).d_t` (every one true)")]
    for off, (cons, what) in enumerate(want, start=1):
        if i + off >= end:
            return fail(first_line, f"the group ends before {what}", i + off)
        q = parse_block_row(clauses[i + off][2])
        if q is None or q["b"] != b or q["lo"] != "0" or q["hi"] != str(n) or q["cons"] != cons:
            return fail(clauses[i + off][1], f"the row after the value permission is not {what} over `0 <= k && k < {n}`", i + off + 1)
    j = i + 3
    listed = 0
    zero_rest = unknown = False
    targets: list[tuple[int, str | None]] = []
    c0 = lambda idx: f"loc({b},{idx})"  # noqa: E731
    while j < end:
        ln, text = clauses[j][1], norm_viper(clauses[j][2])
        if text == f"acc(lvloc({g}()).lv)" and key is None:
            break
        if zero_rest or unknown:
            if key is not None:
                break  # the next group of the structure global starts here
            return fail(ln, "a row after the quantifier of the values (the group ends with `acc(lvloc(GB_g()).lv)`)", j + 1)
        q = parse_block_row(clauses[j][2])
        if q is not None:
            if key is not None and q["b"] != b:
                break  # the next array member's group
            if q["b"] != b or q["hi"] != str(n):
                return fail(ln, f"the quantifier of the values is over `{{ loc({b}, k) }}` up to `k < {n}`", j + 1)
            zero = zero_literal(fields, t)
            if q["cons"] == f"{cell}.v_{t}=={zero}" and zero is not None:
                if int(q["lo"]) != listed:
                    return fail(ln, f"the zero rest starts at `{q['lo']}` but {listed} elements are listed (it starts where the list ends)", j + 1)
                if listed >= n:
                    return fail(ln, "a zero rest although every element is listed", j + 1)
                zero_rest = True
            else:
                good, target = leaf_value(fields, t, cell, q["cons"], counts)
                if not good or target is not None or re.fullmatch(rf"{re.escape(cell)}\.v_{re.escape(t)}==[^|&<]+", q["cons"]):
                    return fail(ln, f"the quantifier of the values is neither the zero rest (`{cell}.v_{t} == {zero}`) nor the type's range / `WF` of an unknown value", j + 1)
                if listed != 0 or q["lo"] != "0":
                    return fail(ln, "an unknown value's range quantifier stands alone over `0 <= k && k < N` (no element is listed)", j + 1)
                unknown = True
            j += 1
            continue
        em = re.fullmatch(rf"loc\({re.escape(b)},(?P<i>[0-9]+)\)\.v_{re.escape(t)}==(?P<v>.+)", text)
        if em is None and key is not None:
            break  # a structure global's group ends where the next row is no value of it
        if em is None:
            return fail(ln, f"`{text}` is not a value conjunct `loc({b}, i).v_{t} == c` of a listed element, the quantifier of the rest or "
                            f"`acc(lvloc({g}()).lv)`", j + 1)
        if int(em.group("i")) != listed:
            return fail(ln, f"the listed element {em.group('i')} is out of order: the next one is {listed} (the elements are listed from 0, once each, "
                            "and a slot at or past the list belongs to the rest)", j + 1)
        if listed >= n:
            return fail(ln, f"an element at or past the count {n}", j + 1)
        good, target = leaf_value(fields, t, c0(listed), text, counts)
        if not good or "||" in em.group("v") or "<=" in em.group("v"):
            return fail(ln, f"the listed value `{em.group('v')}` is not a literal of its type, `null` or `loc(GB_h(), 0)` of a declared global", j + 1)
        targets.append((ln, target))
        listed += 1
        j += 1
    if j >= end and key is None:
        return fail(clauses[min(i, end - 1)][1], f"the group has no final `acc(lvloc({g}()).lv)`", end)
    if listed < n and not zero_rest and not unknown and zero_literal(fields, t) is not None and fields.get(t) != "Bool":
        return fail(clauses[j][1], "no zero rest and no range of an unknown value: the elements from "
                                   f"{listed} on have no value row", j + 1)
    if listed < n and not zero_rest and not unknown and fields.get(t) == "Bool" and listed != 0:
        return fail(clauses[j][1], f"no zero rest for the elements from {listed} on", j + 1)
    return (j if key is not None else j + 1), (g, n, t, targets, key, None), problems


# Decision 0064 items 9 and 17 (M4.5e1e2 Part C2): an array of STRUCTURES (a global, a `static` local, a structure global's array member) has, per LEAF
# path of its element, the row group of an array of scalars, with the leaf receiver `loc(member(<chain>, j), 0)` rooted at the slot `loc(B, k)` as the
# trigger and the cell, after one row of length facts over the slots (`forall k :: { loc(B, k) } 0 <= k && k < N ==> len(member(<chain>, j)) == 1 && ...`,
# `range_lens_members`).  The only rows that name an element are the listed values, `loc(member(loc(B, i), j), 0).v_t == c`, one per leaf of each listed
# element (element by element, the leaves in declaration order), then, per leaf in the same order, the quantifier of the elements the list does not reach
# (`m <= k && k < N ==> <receiver>.v_t == 0`) or the type's range of an unknown value over `[0, N)`; last `acc(lvloc(GB_g()).lv)` for a global.
SLOT_ROW_RE = re.compile(r"^forall(?P<k>[A-Za-z_$][\w$']*):Int::\{(?P<trig>.+?)\}(?P<lo>[0-9]+)<=(?P=k)&&(?P=k)<(?P<hi>[0-9]+)==>(?P<cons>.+)$")


def parse_slot_row(text: str) -> dict[str, str] | None:
    """The fields of a quantified row whose trigger is any one term (`parse_block_row` wants `loc(B, k)`; a leaf row of an element has the leaf
    receiver): k, trig (the trigger), lo, hi (literals) and cons; None when the text is not that shape.  Whitespace and grouping parentheses do not matter."""
    m = SLOT_ROW_RE.match(norm_viper(text))
    if m is None:
        return None
    out = {key: m.group(key) for key in ("k", "trig", "lo", "hi", "cons")}
    # the consequent as written (grouping kept): `cons` has lost its parentheses, which a guarded `a || b && (c ==> d)` needs back (audit round C2, finding 1)
    close = text.find("}")
    if close >= 0:
        pieces = _split_top(_strip_wrap(text[close + 1:].strip()), "==>")
        if len(pieces) >= 2:
            out["raw"] = "==>".join(pieces[1:]).strip()
    return out


def parse_block_leaf_row(text: str) -> dict[str, str] | None:
    """A row of `parse_block_row`'s kind over a LEAF of the elements of an array of structures: the trigger is the leaf receiver `loc(member(<chain>,
    j), 0)` rooted at the slot `loc(B, k)` (B a block term), at least one member step.  The fields are those of `parse_block_row` (k, b, lo, hi, cons)
    and `recv`, the receiver; None otherwise (including a row of the older shape)."""
    q = parse_slot_row(text)
    if q is None:
        return None
    k, trig = q["k"], q["trig"]
    root = trig
    steps = 0
    while (m := _CHAIN_LOC_RE.match(root)) is not None:
        root, steps = m.group("inner"), steps + 1
    if steps == 0 or not root.startswith("loc(") or not root.endswith(f",{k})"):
        return None
    b = root[len("loc("):-len(f",{k})")]
    if parse_struct_chain(trig, f"loc({b},{k})") is None:
        return None
    out = {"k": k, "b": b, "lo": q["lo"], "hi": q["hi"], "cons": q["cons"], "recv": trig}
    if "raw" in q:
        out["raw"] = q["raw"]
    return out


def leaf_receiver(slot: str, leaf: tuple[int, ...]) -> str:
    """The receiver of the leaf `leaf` (member steps) of the element at `slot`: `loc(member(<...>, j), 0)`."""
    r = slot
    for j in leaf:
        r = f"loc(member({r},{j}),0)"
    return r


def element_leaves(keys: list[tuple[int, ...]]) -> list[tuple[int, ...]]:
    """The leaf paths of an element whose member blocks are `keys` (`range_lens_members`): the keys that are no prefix of another, in declaration order."""
    kset = set(keys)
    return sorted(k for k in kset if not any(o != k and o[:len(k)] == k for o in kset))


def aggregate_global_group(clauses: list[tuple[str, int, str]], i: int, end: int, fields: dict[str, str], counts, q0: dict[str, str], g: str,
                           key: tuple[int, ...] | None, fail, detached: bool = False):
    """The row group of an array of structures in `program_start` (decision 0064 item 9; see the comment above); `i` is the first clause (the length
    facts), `q0` its `parse_block_row`; `key` is the member key of a structure global's array member (no liveness row then); `detached`: the length facts were read from an earlier clause and `i` is the
    first row of the leaves.  Returns what
    `array_global_group` returns, with the field suffix of the first pointer leaf that holds an address (else of the first leaf)."""
    first = clauses[i][1]
    n, b, k = int(q0["hi"]), q0["b"], q0["k"]
    if q0["lo"] != "0" or n < 1:
        return fail(first, f"the length facts of the elements are over `0 <= k && k < N` with a literal N >= 1, found `{q0['lo']} <= k && k < {q0['hi']}`", i + 1)
    keys, why = range_lens_members(q0["cons"], f"loc({b},{k})")
    if why is not None or keys is None:
        return fail(first, f"the length facts of the elements of an array of structures: {why}", i + 1)
    leaves = element_leaves(keys)
    suffixes: list[str] = []
    j = i if detached else i + 1  # a structure global's length facts are a clause of their own, before the structure's cell row (`detached`)
    for leaf in leaves:
        cell = leaf_receiver(f"loc({b},{k})", leaf)
        t = ""
        for off, (what, shape) in enumerate((("the value permission", "acc({c}.v_{t})"), ("the flag permission", "acc({c}.d_{t})"), ("the flags (every one true)", "{c}.d_{t}"))):
            if j >= end:
                return fail(first, f"the group ends before {what} of the leaf {leaf}", j)
            q = parse_slot_row(clauses[j][2])
            if q is None or q["trig"] != cell or q["lo"] != "0" or q["hi"] != str(n) or q["k"] != k:
                return fail(clauses[j][1], f"the row after the length facts is not {what} of the leaf {leaf} over `0 <= k && k < {n}` with the receiver `{cell}` as trigger", j + 1)
            if off == 0:
                mv = re.fullmatch(rf"acc\({re.escape(cell)}\.v_(?P<t>[\w$']+)\)", q["cons"])
                if mv is None or mv.group("t") not in fields:
                    return fail(clauses[j][1], f"the first row of the leaf {leaf} is not `acc({cell}.v_t)` of a declared field", j + 1)
                t = mv.group("t")
            elif q["cons"] != shape.format(c=cell, t=t):
                return fail(clauses[j][1], f"the row after the value permission of the leaf {leaf} is not {what}", j + 1)
            j += 1
        suffixes.append(t)
    listed = li = 0  # the next listed conjunct is the leaf `li` of the element `listed`
    rest: list[int] = []  # the leaves whose quantifier of the rest or of an unknown value has been seen
    targets: list[tuple[int, str | None]] = []
    ptr_suffix: str | None = None
    rest_lo: int | None = None
    unknown = False
    while j < end:
        ln, text = clauses[j][1], norm_viper(clauses[j][2])
        if text == f"acc(lvloc({g}()).lv)" and key is None:
            break
        q = parse_slot_row(clauses[j][2])
        if q is not None:
            if key is not None and q["trig"] not in {leaf_receiver(f"loc({b},{k})", lf) for lf in leaves}:
                break  # the next array member's group
            if li != 0:
                return fail(ln, f"the element {listed} has only {li} of its {len(leaves)} leaves listed before the quantifier of the rest", j + 1)
            at = len(rest)
            if at >= len(leaves):
                return fail(ln, "a quantifier of the values after every leaf has one", j + 1)
            cell, t = leaf_receiver(f"loc({b},{k})", leaves[at]), suffixes[at]
            zero = zero_literal(fields, t)
            if q["trig"] != cell or q["hi"] != str(n) or q["k"] != k:
                return fail(ln, f"the quantifier of the values of the leaf {leaves[at]} is over `{{ {cell} }}` up to `k < {n}` (the leaves in declaration order, each once)", j + 1)
            if zero is not None and q["cons"] == f"{cell}.v_{t}=={zero}":
                if unknown or int(q["lo"]) != listed:
                    return fail(ln, f"the zero rest of the leaf {leaves[at]} starts at `{q['lo']}` but {listed} elements are listed (it starts where the list ends, for every leaf)", j + 1)
                if listed >= n:
                    return fail(ln, "a zero rest although every element is listed", j + 1)
                rest_lo = listed
            else:
                good, target = leaf_value(fields, t, cell, q["cons"], counts)
                if not good or target is not None or re.fullmatch(rf"{re.escape(cell)}\.v_{re.escape(t)}==[^|&<]+", q["cons"]):
                    return fail(ln, f"the quantifier of the values of the leaf {leaves[at]} is neither the zero rest (`{cell}.v_{t} == {zero}`) nor the type's range / `WF` of an unknown value", j + 1)
                if listed != 0 or q["lo"] != "0" or (rest_lo is not None):
                    return fail(ln, "an unknown value's range quantifier stands alone over `0 <= k && k < N` (no element is listed, no zero rest beside it)", j + 1)
                unknown = True
            rest.append(at)
            j += 1
            continue
        if rest:
            if key is not None:
                break  # the next group of the structure global starts here
            return fail(ln, "a row after the quantifiers of the values (the group ends with `acc(lvloc(GB_g()).lv)`)", j + 1)
        cell, t = leaf_receiver(f"loc({b},{listed})", leaves[li]), suffixes[li]
        em = re.fullmatch(rf"{re.escape(cell)}\.v_{re.escape(t)}==(?P<v>.+)", text)
        if em is None:
            if key is not None:
                break  # a structure global's group ends where the next row is no value of it
            return fail(ln, f"`{text}` is not the listed value `{cell}.v_{t} == c` (the elements are listed from 0, each leaf in declaration order, once), the "
                            f"quantifier of the rest or `acc(lvloc({g}()).lv)`", j + 1)
        if listed >= n:
            return fail(ln, f"an element at or past the count {n}", j + 1)
        good, target = leaf_value(fields, t, cell, text, counts)
        if not good or "||" in em.group("v") or "<=" in em.group("v"):
            return fail(ln, f"the listed value `{em.group('v')}` is not a literal of its type, `null` or `loc(GB_h(), 0)` of a declared global", j + 1)
        if target is not None:
            if ptr_suffix not in (None, t):
                return fail(ln, "addresses in pointer leaves of two types (the escape rule reads one pointer field of an array of structures)", j + 1)
            ptr_suffix = t
        targets.append((ln, target))
        li += 1
        if li == len(leaves):
            listed, li = listed + 1, 0
        j += 1
    if li != 0:
        return fail(clauses[min(j, end - 1)][1], f"the element {listed} has only {li} of its {len(leaves)} leaves listed", j)
    if j >= end and key is None:
        return fail(clauses[min(i, end - 1)][1], f"the group has no final `acc(lvloc({g}()).lv)`", end)
    if rest and len(rest) != len(leaves):
        return fail(clauses[min(j, end) - 1][1], f"the quantifier of the values is given for {len(rest)} of the {len(leaves)} leaves (every leaf, in declaration order)", j)
    for idx, t in enumerate(suffixes):
        if listed < n and not rest and zero_literal(fields, t) is not None and not (fields.get(t) == "Bool" and listed == 0):
            return fail(clauses[min(j, end - 1)][1], f"no zero rest and no range of an unknown value: the elements from {listed} on have no value row for the leaf {leaves[idx]}", j + 1)
    return (j if key is not None else j + 1), (g, n, ptr_suffix or suffixes[0], targets, key, frozenset(zip(leaves, suffixes))), []


def lint_globals(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> tuple[bool, list[str] | None]:
    ok_domains = [d for d in decls if d.kind == "domain" and lint_domain(src, d, lambda _l, _w: None)]
    in_domain: set[int] = set()
    for d in ok_domains:
        in_domain.update(range(d.line + 1, block_close_line(src, d) + 1))
    gbs: dict[str, int] = {}  # name -> line, in declaration order
    for lineno, code in enumerate(src.code, start=1):
        if not UNIQUE_RE.search(code):
            if GB_FUNCTION_RE.match(code) and lineno in in_domain:
                add(lineno, "a `GB_` function of `domain Block` must be `unique`: two distinct globals are two blocks "
                            "(decision 0041 item 4)")
            continue
        m = GB_DECL_RE.match(code)
        if m is None or lineno not in in_domain:
            add(lineno, "`unique`: admitted only as `unique function GB_<name>(): Block` inside `domain Block` "
                        "(decision 0041 item 4); distinctness anywhere else would be an assumption")
        elif m.group("name") in gbs:
            add(lineno, f"`unique function {m.group('name')}` declared twice")
        else:
            gbs[m.group("name")] = lineno
    ps = [d for d in decls if d.kind == "method" and d.name == PROGRAM_START]
    if not gbs:
        for d in ps:
            add(d.line, "`method program_start` in a program with no `unique function GB_...`: it exists only for globals "
                        "(decision 0041 item 4)")
        return False, None
    if len(ps) != 1:
        add(min(gbs.values()), "a program with globals (`unique function GB_...`) has exactly one `method program_start()` "
                               f"(decision 0041 item 4), found {len(ps)}")
    fields = {m.group("suffix"): m.group("type") for l in src.code if (m := FIELD_DECL_RE.match(l))}
    counts = struct_array_counts(src, decls)
    facts_atoms: list[str] | None = global_facts_atoms(list(gbs))  # replaced below by the unit's facts, with the struct globals' lengths and slot facts
    for d in ps:
        want_head = "method program_start()"
        header = " ".join(src.code[d.line - 1].split())
        clauses = clause_texts(src, d)
        problem = None
        # A row is `acc(<cell>.v_T) && acc(<cell>.d_T) && <cell>.d_T && <value> && acc(lvloc(GB_x()).lv)` of a scalar global, or the
        # rows of a struct global's leaves (decision 0044 items 8 and 13); first read every row, then the facts they imply.
        rows: dict[str, tuple[int, str, str | None]] = {}  # global -> (line, field suffix, target of a `loc(GB_h(), 0)` value)
        srows: dict[str, tuple[int, list[tuple[tuple[int, ...], str, str | None]]]] = {}  # struct global -> (line, leaves)
        arows: dict[str, tuple[int, str, int, list[tuple[int, str | None]]]] = {}  # array global -> (line, field suffix, N, [(line, target)])
        # a struct global's array leaves (decision 0063 item 9): global -> {member key: (line, field suffix, N, [(line, target)])}
        sarrs: dict[str, dict[tuple[int, ...], tuple[int, str, int, list[tuple[int, str | None]]]]] = {}
        row_problems: list[tuple[int, str]] = []
        ashapes: dict[str, frozenset] = {}  # an array global of structures -> its element's shape: the (leaf path, field suffix) pairs
        pending_lens: dict[tuple[str, tuple[int, ...]], tuple[int, dict[str, str]]] = {}  # (global, member key or ()) -> (line, parse_block_row) of a length facts clause not yet followed by its group
        if d.has_body or header != want_head:
            problem = f"it must be a bodyless `{want_head}`"
        elif trusted_tag(src, d.line) is None:
            problem = "it must carry `// cerastes:trusted-contract <reason>`"
        elif not clauses or clauses[-1][0] != "decreases" or norm_viper(clauses[-1][2]) != "_" \
                or any(k != "ensures" for k, _l, _t in clauses[:-1]):
            problem = "its clauses are `ensures` rows and a final `decreases _`, nothing else"
        else:
            at, stop = 1, len(clauses) - 1
            while at < stop:
                _k, ln, text = clauses[at]
                at += 1
                row = norm_viper(text)
                if row.startswith("forall"):
                    # a structure global's array member of structures: its length facts are a clause of their own right after the global facts, and the
                    # member's row group follows the structure's cell row (decision 0064 item 9, as built in Part C2)
                    ql = parse_block_row(text)
                    if ql is not None and "len(member(" in ql["cons"]:
                        gm = GB_TERM_RE.match(ql["b"])
                        mk = (gm.group(1), ()) if gm is not None and gm.group(1) in gbs else next(
                            ((cand, parse_struct_member(ql["b"], f"loc({cand}(),0)")) for cand in gbs
                             if parse_struct_member(ql["b"], f"loc({cand}(),0)") is not None), None)
                        if mk is not None and mk not in pending_lens:
                            pending_lens[mk] = (ln, ql)
                            continue
                    sq = parse_slot_row(text) if pending_lens else None
                    lens_hit = None
                    if sq is not None:
                        for pk, (_pl, pq) in pending_lens.items():
                            pkeys, pwhy = range_lens_members(pq["cons"], f"loc({pq['b']},{pq['k']})")
                            if pwhy is None and pkeys and sq["trig"] == leaf_receiver(f"loc({pq['b']},{pq['k']})", element_leaves(pkeys)[0]):
                                lens_hit = pk
                                break
                    # an array global's row group (decision 0063 items 9 and 15): several rows, one global
                    at, group, why = array_global_group(clauses, at - 1, stop, fields, gbs, counts,
                                                        pending_lens.pop(lens_hit)[1] if lens_hit is not None else None)
                    row_problems.extend(why)
                    if group is not None and group[4] is not None:
                        ag, an, asuffix, atargets, akey, _ashape = group
                        if ag not in srows:
                            row_problems.append((ln, f"the array member {akey} of the structure global `{ag}` has its row group before the structure's cell row, or the "
                                                     "structure has no cell leaf (the cell row ends with `acc(lvloc(GB_g()).lv)`; decision 0063 item 9)"))
                        elif akey in sarrs.get(ag, {}):
                            row_problems.append((ln, f"a second row group for the array member {akey} of `{ag}` (two full permissions would make `main` vacuous)"))
                        else:
                            sarrs.setdefault(ag, {})[akey] = (ln, asuffix, an, atargets)
                    elif group is not None:
                        ag, an, asuffix, atargets, _akey, ashape = group
                        if ag in rows or ag in srows or ag in arows:
                            row_problems.append((ln, f"a second `program_start` row for `{ag}` (two full permissions would make `main` vacuous)"))
                        else:
                            arows[ag] = (ln, asuffix, an, atargets)
                            if ashape is not None:
                                ashapes[ag] = ashape
                    continue
                if row.startswith("acc(loc(member("):
                    r = struct_row(row, fields, counts)
                    if isinstance(r, str):
                        row_problems.append((ln, f"a `program_start` struct row is wrong: {r}"))
                        continue
                    g, leaves = r
                    if g not in gbs:
                        row_problems.append((ln, f"a `program_start` struct row names `{g}`, which is not a declared `GB_` global"))
                    elif g in rows or g in srows or g in arows:
                        row_problems.append((ln, f"a second `program_start` row for `{g}` (two full permissions would make `main` vacuous)"))
                    else:
                        srows[g] = (ln, leaves)
                    continue
                lone = re.fullmatch(r"acc\(lvloc\((GB_[\w$']+)\(\)\)\.lv\)", row)
                if lone:
                    # a structure global with no cell leaf (decision 0063 item 9): its cell row is the liveness cell alone, then the array groups
                    lg = lone.group(1)
                    if lg not in gbs:
                        row_problems.append((ln, f"a `program_start` liveness row names `{lg}`, which is not a declared `GB_` global"))
                    elif lg in rows or lg in srows or lg in arows:
                        row_problems.append((ln, f"a second `program_start` row for `{lg}` (two full permissions would make `main` vacuous)"))
                    else:
                        srows[lg] = (ln, [])
                    continue
                m = re.match(r"acc\(loc\((?P<g>GB_[\w$']+)\(\),0\)\.v_(?P<s>[\w$']+)\)&&acc\(loc\((?P=g)\(\),0\)\.d_(?P=s)\)&&"
                             r"loc\((?P=g)\(\),0\)\.d_(?P=s)&&(?P<rest>.*)$", row)
                g = m.group("g") if m else ""
                tail = f"acc(lvloc({g}()).lv)"
                if m is None or g not in gbs or not (m.group("rest") == tail or m.group("rest").endswith("&&" + tail)):
                    row_problems.append((ln, "a `program_start` row is `acc(<cell>.v_T) && acc(<cell>.d_T) && <cell>.d_T && <value> && "
                                             "acc(lvloc(GB_x()).lv)` for a declared `GB_x`, <cell> being `loc(GB_x(), 0)`"))
                    continue
                suffix, c = m.group("s"), f"loc({g}(),0)"
                mid = m.group("rest")[:-len(tail)].rstrip("&")
                good, target = leaf_value(fields, suffix, c, mid, counts)
                if not good:
                    row_problems.append((ln, f"the value of the `program_start` row of `{g}` (field `v_{suffix}`{'' if suffix in fields else ', not declared'}) is not "
                                             "a literal of its type, `null`, `loc(GB_h(), 0)` of a declared global, or an unknown value's range / `WF`"))
                    continue
                if g in rows or g in srows or g in arows:
                    row_problems.append((ln, f"a second `program_start` row for `{g}` (two full permissions would make `main` vacuous)"))
                    continue
                rows[g] = (ln, suffix, target)
            for (pg, pkey), (pln, _pq) in pending_lens.items():
                row_problems.append((pln, f"the length facts of the array of structures `{pg}`{' (member ' + str(pkey) + ')' if pkey else ''} are not followed by its row group "
                                          "(decision 0064 item 9)"))
            for g, (lln, lleaves) in srows.items():
                if not lleaves and not sarrs.get(g):
                    row_problems.append((lln, f"the liveness row of `{g}` stands alone, and no array member of it has a row group: a structure global with no cell leaf "
                                              "is its liveness cell and the groups of its array members (decision 0063 item 9)"))
            # the facts follow the rows: with a row that is wrong or missing the expected facts are unknown, and the row is the problem
            missing = [g for g in gbs if g not in rows and g not in srows and g not in arows]
            facts_atoms = None if row_problems or missing else global_facts_atoms_of(
                list(gbs), {g: lv for g, (_l, lv) in srows.items()}, {g: n for g, (_l, _s, n, _t) in arows.items()},
                {g: {k: v[2] for k, v in arrs.items()} for g, arrs in sarrs.items()})
            if facts_atoms is not None and (len(clauses) < 2 or sorted(norm_viper(clauses[0][2]).split("&&")) != facts_atoms):
                problem = ("its first `ensures` row must be the global facts: `len(GB_x()) == 1 && lvloc(GB_x()) == loc(GB_x(), -2)` "
                           "for each declared `GB_x` (`len(GB_x()) == N` for an array global of N elements, decision 0063 item 15), each once, and "
                           "for each struct global the member lengths `len(member(X, k)) == 1` of every "
                           "slot of its row and its slot facts `X != Y` (decision 0044 item 8), nothing else")
        if problem:
            add(d.line, f"`method program_start` is not the trusted start of decision 0041 item 4: {problem}")
            continue
        for ln, why in row_problems:
            add(ln, why)
        shapes: dict[str, frozenset] = {}  # pointer field suffix -> the shape of the struct globals it points at
        # every address value `loc(GB_h(), 0)`, of a scalar global's row or of a struct global's leaf: (owner, line, field suffix, target)
        addrs = [(g, ln, suffix, target) for g, (ln, suffix, target) in rows.items() if target is not None]
        addrs += [(g, ln, suffix, target) for g, (ln, leaves) in srows.items() for _c, suffix, target in leaves if target is not None]
        addrs += [(g, eln, suffix, target) for g, (_l, suffix, _n, elems) in arows.items() for eln, target in elems if target is not None]
        addrs += [(g, eln, v[1], target) for g, arrs in sarrs.items() for v in arrs.values() for eln, target in v[3] if target is not None]
        for g, ln, suffix, target in addrs:
            pointee = suffix[len("ptr_"):]
            if target in rows:
                if rows[target][1] != pointee:
                    add(ln, f"the row of `{g}` points at `{target}`, which is not a declared global of the pointee's field "
                            f"`v_{pointee}` (decision 0041 item 4)")
            elif target in ashapes:
                # `struct P *pg = g;` is `loc(GB_g(), 0)`, the first element of an array global of structures (decision 0064 item 9): the pointee is a
                # structure of the element's shape, as for a pointer to a structure global
                if not pointee.startswith("struct_"):
                    add(ln, f"the row of `{g}` points at the array of structures `{target}` in a field `v_{suffix}` that is not a pointer to a struct "
                            "(decision 0064 item 9)")
                elif shapes.setdefault(suffix, ashapes[target]) != ashapes[target]:
                    add(ln, f"the field `v_{suffix}` points at globals of two different shapes (`{target}` is not of the pointee's type)")
            elif target in arows:
                if arows[target][1] != pointee:  # `loc(GB_h(), 0)`: the first element of an array global (the decay, decision 0063 item 9)
                    add(ln, f"the row of `{g}` points at the array `{target}`, whose elements are not of the pointee's field "
                            f"`v_{pointee}` (decision 0063 item 9)")
            elif target in srows:
                shape = frozenset((c, sfx) for c, sfx, _t in srows[target][1])
                if not pointee.startswith("struct_"):
                    add(ln, f"`{g}` has an address of the struct global `{target}` in a field `v_{suffix}` that is not a pointer to a struct "
                            "(decision 0044 item 13: the address of a struct global is the value of a `ptr_struct_<n>_<tag>` field)")
                elif shapes.setdefault(suffix, shape) != shape:
                    add(ln, f"the field `v_{suffix}` points at struct globals of two different shapes (`{target}` is not of the pointee's type)")
            else:
                add(ln, f"`{g}` points at `{target}`, which is not a declared global with a row (decision 0041 item 4)")
        for g in gbs:
            if g not in rows and g not in srows and g not in arows and not row_problems:
                add(d.line, f"`method program_start` has no row for `{g}` (decision 0041 item 4: one row per global)")
    # `program_start` is reached once, as the first statement of `main`; every other method takes the facts
    methods = [d for d in decls if d.kind == "method"]
    ps_lines = {d.line for d in ps}
    calls = [n for n, code in enumerate(src.code, start=1) if PROGRAM_START_CALL_RE.search(code) and n not in ps_lines]
    entries: list[Decl] = []
    for n in calls:
        owner = next((d for d in methods if d.line <= n <= d.end_line), None)
        if owner is not None and owner.has_body and first_statement(src, owner) == ("program_start()", n):
            entries.append(owner)
        else:
            add(n, "`program_start()` is called only as the first statement of `main`'s method (decision 0041 item 4)")
    if len(calls) > 1:
        add(calls[1], "`program_start()` is called once (a second call would hold every global's permission twice)")
    # The entry method is reached by the program's start only: a call of it (from another method or from itself, after `program_start()`) would
    # run `program_start()` again and inhale every global's permission twice, and it takes no `requires` (it holds the globals from `program_start`).
    decl_lines = {d.line for d in methods}
    for e in entries:
        if any(k == "requires" for k, _l, _t in clause_texts(src, e)):
            add(e.line, f"`method {e.name}` starts the program (its first statement is `program_start()`): it takes no `requires` "
                        "(decision 0041 item 4)")
        call = re.compile(r"(?<![\w$'.])" + re.escape(e.name) + r"\s*\(")
        for n, code in enumerate(src.code, start=1):
            if n not in decl_lines and call.search(code):
                add(n, f"a call of the entry method `{e.name}`: it runs `program_start()` once, as the program starts; a call would "
                       "inhale every global's permission again (decision 0041 item 4)")
    exempt = {PROGRAM_START} | {e.name for e in entries} | {e.name + "__wf" for e in entries}
    for d in methods:
        if d.name in exempt or d.name.startswith((ALLOC_PREFIX, ARR_ZERO_PREFIX)) or is_pinned_array_copy(src, d):
            continue
        reqs = [(ln, t) for k, ln, t in clause_texts(src, d) if k == "requires"]
        if facts_atoms is not None and (not reqs or sorted(norm_viper(reqs[0][1]).split("&&")) != facts_atoms):
            add(reqs[0][0] if reqs else d.line,
                f"`method {d.name}` of a program with globals must take the global facts as its first `requires` "
                "(`len(GB_x()) == 1 && lvloc(GB_x()) == loc(GB_x(), -2)` for each global, before the amounts' rows; "
                "decision 0041 item 4)")
    return True, facts_atoms


# The ghost amounts (decision 0037 items 6 and 14): a method's `Perm` formals are the amounts of the atoms of its contract.
WILDCARD_RE = re.compile(r"(?<![\w$'.])wildcard(?![\w$'])")
PERM_FORMAL_RE = re.compile(r"^\s*(?P<name>[A-Za-z_$][\w$']*)\s*:\s*Perm\s*$")
CLAUSE_START_RE = re.compile(r"^\s*(requires|ensures|invariant|decreases)\b")


def method_header_text(src: Source, d: Decl) -> str:
    """The header of a method: its lines up to the first clause or the opening brace, joined."""
    header = []
    for n in range(d.line, d.end_line + 1):
        line = src.code[n - 1]
        if CLAUSE_START_RE.match(line) or line.strip().startswith("{"):
            break
        header.append(line)
    return " ".join(header)


def parenthesized_list(text: str, start: int) -> list[str]:
    """The top-level comma-separated parts of the parenthesis that opens at `text[start]`."""
    depth, parts, cur = 0, [], []
    for ch in text[start:]:
        if ch == "(":
            depth += 1
            if depth == 1:
                continue
        elif ch == ")":
            depth -= 1
            if depth == 0:
                break
        if depth == 1 and ch == ",":
            parts.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
    parts.append("".join(cur))
    return [x for x in parts if x.strip()]


def method_formals(src: Source, d: Decl) -> list[str]:
    """The text of each formal of a method (the first parenthesis of its header, split at top-level commas)."""
    text = method_header_text(src, d)
    start = text.find("(", text.find(d.name) + len(d.name))
    if start < 0:
        return []
    return parenthesized_list(text, start)


RESULT_PART_RE = re.compile(r"^\s*(?P<name>[A-Za-z_$][\w$']*)\s*:\s*(?P<type>.+?)\s*$")


def method_results(src: Source, d: Decl) -> list[tuple[str, str]]:
    """The (name, type) of each result of a method: the parenthesis after `returns` in its header."""
    text = method_header_text(src, d)
    after = text.find(d.name) + len(d.name)
    m = re.search(r"\)\s*returns\s*\(", text[after:])
    if m is None:
        return []
    start = after + m.end() - 1
    return [(r.group("name"), r.group("type")) for part in parenthesized_list(text, start) if (r := RESULT_PART_RE.match(part))]


def perm_result_rows(src: Source, d: Decl, o: str) -> tuple[list[int], int]:
    """(the lines of the `ensures none < o && o <= write` of a ghost `Perm` result, the first other clause that mentions it)."""
    word = re.compile(r"(?<![\w$'.])" + re.escape(o) + r"(?![\w$'])")
    bounds = re.sub(r"[\s()]", "", f"none<{o}&&{o}<=write")
    rows, first_mention = [], 0
    for n in range(d.line + 1, d.end_line + 1):
        line = src.code[n - 1]
        if not CLAUSE_START_RE.match(line):
            continue
        if line.strip().startswith("ensures") and re.sub(r"[\s()]", "", line.strip()[len("ensures"):]) == bounds:
            rows.append(n)
        elif word.search(line) and not first_mention:
            first_mention = n
    return rows, first_mention


def perm_result_has_row(src: Source, d: Decl, o: str) -> bool:
    """Whether the ghost `Perm` result `o` has exactly one `ensures none < o && o <= write` and no clause before it mentions it."""
    rows, first_mention = perm_result_rows(src, d, o)
    return len(rows) == 1 and not (first_mention and first_mention < rows[0])


def lint_ghost_amounts(src: Source, decls: list[Decl], add: Callable[[int, str], None],
                       wildcard_rows: frozenset[int] = frozenset()) -> None:
    for lineno, code in enumerate(src.code, start=1):
        if WILDCARD_RE.search(code) and lineno not in wildcard_rows:
            add(lineno, "`wildcard`: an amount Silicon chooses is an assumption (decision 0037 item 14); ghost amounts "
                        "are `Perm` formals with `requires none < a && a < write`")
    for d in decls:
        if d.kind != "method" or is_pinned_array_copy(src, d):
            continue  # `copy_arr_<t>`'s amount `rd` has its own bounds, pinned with the clause set (decision 0063 item 13), not by the name
        amounts = [m.group("name") for f in method_formals(src, d) if (m := PERM_FORMAL_RE.match(f))]
        for a in amounts:
            word = re.compile(r"(?<![\w$'.])" + re.escape(a) + r"(?![\w$'])")
            bounds = re.sub(r"[\s()]", "", f"none<{a}&&{a}<write")
            bound_lines = []
            first_mention = 0
            for n in range(d.line + 1, d.end_line + 1):
                line = src.code[n - 1]
                if not CLAUSE_START_RE.match(line):
                    continue
                if re.sub(r"[\s()]", "", line.strip()[len("requires"):]) == bounds and line.strip().startswith("requires"):
                    bound_lines.append(n)
                elif word.search(line) and not first_mention:
                    first_mention = n
            if len(bound_lines) != 1:
                add(d.line, f"`method {d.name}`: the ghost amount `{a}` must have exactly one `requires none < {a} && {a} < write` "
                            f"(decision 0037 items 6 and 14), found {len(bound_lines)}")
            elif first_mention and first_mention < bound_lines[0]:
                add(first_mention, f"`method {d.name}`: a clause mentions the ghost amount `{a}` before its bounds "
                                   f"`requires none < {a} && {a} < write` (line {bound_lines[0]}; Silicon's well-formedness check "
                                   "needs the bounds first, decision 0037 item 6)")
        # Decision 0052 item 5: a ghost `Perm` *result* (the output amount of a partnerless `ensures` atom) has exactly one
        # `ensures none < o && o <= write`, before any clause that mentions it: the amount is positive and at most `write`
        # whatever the body or a trusted contract says, so a caller's `t, o := f(...)` receives a bounded amount.
        for o, typ in method_results(src, d):
            if typ != "Perm":
                continue
            rows, first_mention = perm_result_rows(src, d, o)
            if len(rows) != 1:
                add(d.line, f"`method {d.name}`: the ghost amount result `{o}` must have exactly one `ensures none < {o} && {o} <= write` "
                            f"(decision 0052 item 5), found {len(rows)}")
            elif first_mention and first_mention < rows[0]:
                add(first_mention, f"`method {d.name}`: a clause mentions the ghost amount result `{o}` before its bounds "
                                   f"`ensures none < {o} && {o} <= write` (line {rows[0]}; decision 0052 item 5)")


IDENT = r"[A-Za-z_$][\w$']*"
BLOCK_BASE_RE = re.compile(rf"^block_of\(({IDENT})\)$")
# What a guard `g ==> forall ...` may not hold (decision 0060 item 16): a guard is a plain condition, not a permission, a quantifier or a
# labelled `old`; in an `assert` it also reads no heap (an `assert`'s failure must be its own row's, never a read in its guard).
GUARD_BAD_RE = re.compile(r"(?<![\w$'])(?:forall|acc|perm|forperm)(?![\w$'])|(?<![\w$'])old\s*\[")



# The ghost amounts of loops (decision 0038 item 9): snapshots of `perm(...)` in `Perm` locals, used as amounts in invariants.
PERM_LOCAL_RE = re.compile(r"(?<![\w$'.])var\s+(?P<names>[A-Za-z_$][\w$']*(?:\s*,\s*[A-Za-z_$][\w$']*)*)\s*:\s*Perm\b"
                           r"(?P<rest>.*)$")
PERM_CALL_RE = re.compile(r"(?<![\w$'.])perm\s*\(")


def perm_local_names(src: Source, d: Decl) -> set[str]:
    """The `Perm` locals of a method with a body: its `var <name>: Perm` lines after the opening brace (the formals and
    the results of the header are not locals)."""
    names: set[str] = set()
    if not d.has_body:
        return names
    for n in range(d.body_line, d.end_line + 1):
        if m := PERM_LOCAL_RE.search(src.code[n - 1]):
            names |= {x.strip() for x in m.group("names").split(",")}
    return names


def _statement_rhs(code: str, start: int) -> str:
    """The text of an assignment's right-hand side that starts at `start`: up to a top-level `;` or `}` or the line's end."""
    depth = 0
    for i in range(start, len(code)):
        ch = code[i]
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif depth == 0 and ch in ";}":
            return code[start:i].strip()
    return code[start:].strip()


def _strip_outer_parens(text: str) -> str:
    text = text.strip()
    while text.startswith("(") and text.endswith(")"):
        depth = 0
        for i, ch in enumerate(text):
            depth += ch == "("
            depth -= ch == ")"
            if depth == 0 and i < len(text) - 1:
                return text
        text = text[1:-1].strip()
    return text


def _perm_divisor(text: str) -> tuple[str, int] | None:
    """(the dividend, the divisor) of `A / N` with a top-level `/` and a decimal literal N, or None."""
    depth, last = 0, -1
    for i, ch in enumerate(text):
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif ch == "/" and depth == 0:
            last = i
    if last < 0:
        return None
    m = re.fullmatch(r"\(*\s*(\d+)\s*\)*", text[last + 1:].strip())
    return (text[:last], int(m.group(1))) if m else None


def _split_conditional(text: str) -> tuple[str, str, str] | None:
    """(condition, then, else) of a top-level `c ? a : b`, or None."""
    text = _strip_outer_parens(text)
    depth, q = 0, -1
    for i, ch in enumerate(text):
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif ch == "?" and depth == 0:
            q = i
            break
    if q < 0:
        return None
    depth, nested = 0, 0
    for i in range(q + 1, len(text)):
        ch = text[i]
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif depth == 0 and ch == "?":
            nested += 1
        elif depth == 0 and ch == ":":
            if nested == 0:
                return text[:q], text[q + 1:i], text[i + 1:]
            nested -= 1
    return None


def _sq(text: str) -> str:
    return "".join(_strip_outer_parens(text).split())


RANGE_LIVENESS_RE = re.compile(rf"^perm\(lvloc\(block_of\(({IDENT})\)\)\.lv\)$")


def _min_leaves(e: str, formals: frozenset[str]) -> list[str] | None:
    """The leaves of a minimum written as Viper has it, `X < Y ? X : Y` nested (squashed text): `perm(...)` calls and names of the
    method's own `Perm` formals; None if `e` is anything else (a `Perm` local, a literal, another comparison)."""
    t = _sq(e)
    if t.startswith("perm(") and _group_end(t, 4) == len(t) - 1:
        return [t]
    if re.fullmatch(IDENT, t) and t in formals:
        return [t]
    cond = _split_conditional(e)
    if cond is None:
        return None
    c, a, b = cond
    sides = _split_top(_strip_outer_parens(c), "<")
    if len(sides) != 2 or _sq(sides[0]) != _sq(a) or _sq(sides[1]) != _sq(b):
        return None
    left, right = _min_leaves(a, formals), _min_leaves(b, formals)
    return None if left is None or right is None else left + right


PERM_LEAF_FIELD_RE = re.compile(rf"^perm\((?P<cell>.+)\.(?P<f>[vd])_(?P<t>{IDENT})\)$")
CELL_TERM_RE = re.compile(r"^loc\((?P<m>member\(.+\)),0\)$")


def _site_amount_inner(d: str, formals: frozenset[str] = frozenset(), slots: dict[str, str] | None = None) -> bool:
    """Decision 0063 item 8 (a `reads(*p)` or `reads(p->a)` site with array parts): the amount a structure site's call row divides is the minimum, in
    nested conditionals, over the permissions the site holds: for each CELL leaf `F = loc(member(<chain>, k), 0)` (chain canonical, rooted at the
    variable R) `perm(F.v_t)` and `perm(F.d_t)`, for each array part's first slot `S = loc(block_of(B), offset_of(B))` (B a variable, the part's
    bound local) `perm(S.v_t)` and `perm(S.d_t)`, the liveness `perm(lvloc(block_of(R)).lv)` of the root, and the method's own `Perm` formals.  The
    value and the flag of every cell and slot must both be in it (a missing one would let the amount exceed what the method holds), exactly one
    liveness leaf, no other `perm`, local or literal.  A slot's base B must be a local the method assigns once `loc(member(<chain>, k), 0)` with the
    chain rooted at the liveness variable R (`slots`: local -> the root of its chain), so a minimum over some other variable's slot is refused.
    Not checked: that every cell leaf of the site is present (a missing cell leaf only lowers nothing: the amount may then exceed what the cell
    holds, which the call's own `exhale` reports as a false alarm, never a proof).
    A plain scalar cell site (audit round 3 S2: a call with array parts binds every cell site's amount to a `Perm` local): the cell IS the
    liveness variable R, so the leaves are `perm(R.v_t)` and `perm(R.d_t)` (value and flag of one type, both required) and the liveness
    `perm(lvloc(block_of(R)).lv)` of the same R; a cell that is another variable than the liveness one is refused as before."""
    leaves = _min_leaves(d, formals)
    if leaves is None:
        return False
    live = [m.group(1) for l in leaves if (m := RANGE_LIVENESS_RE.match(l))]
    if len(set(live)) != 1:
        return False
    root = live[0]
    held: dict[tuple[str, str], set[str]] = {}  # (cell kind and text, suffix) -> {v, d}
    for leaf in leaves:
        if leaf in formals or RANGE_LIVENESS_RE.match(leaf):
            continue
        m = PERM_LEAF_FIELD_RE.match(leaf)
        if m is None:
            return False
        cell = m.group("cell")
        sm = re.fullmatch(rf"loc\(block_of\(({IDENT})\),offset_of\(\1\)\)", cell)
        if cell == root:
            kind = "root"
        elif sm:
            if (slots or {}).get(sm.group(1)) != root:
                return False
            kind = "slot"
        elif (cm := CELL_TERM_RE.match(cell)) is not None and parse_struct_member(cm.group("m"), root) is not None:
            kind = "cell"
        elif (base := _agg_slot_base(cell)) is not None and (slots or {}).get(base) == root:
            kind = "leaf"  # a leaf of the first slot of an array part whose elements are structures (decision 0064 items 7 and 17)
        else:
            return False
        held.setdefault((f"{kind}:{cell}", m.group("t")), set()).add(m.group("f"))
    if any(key.startswith("root:") for key, _ in held):
        # a plain scalar cell site is the root cell alone (one type); a structure site never holds a cell that is its root
        if len(held) != 1:
            return False
    return all(fs == {"v", "d"} for fs in held.values())


def _agg_slot_base(cell: str) -> str | None:
    """The base local B of a leaf cell of the first slot of an array part of structures: `loc(member(<S>, j), 0)` (one or more member steps) with
    `<S>` the slot `loc(block_of(B), offset_of(B))` of a variable B; None otherwise.  Squashed text."""
    steps = 0
    while (m := re.fullmatch(r"loc\(member\((.+),\d+\),0\)", cell)) is not None:
        cell, steps = m.group(1), steps + 1
    sm = re.fullmatch(rf"loc\(block_of\(({IDENT})\),offset_of\(\1\)\)", cell)
    return sm.group(1) if sm is not None and steps >= 1 else None


def _first_slot_leaf(cell: str, slot: str) -> bool:
    """Whether `cell` is a leaf of the first slot of a range over structures (decision 0064 item 7: the receiver of a leaf
    path of the element at the range's first slot): `loc(member(<S>, j), 0)` with `<S>` the slot `loc(block_of(B), offset_of(B))`
    itself or, for a leaf of a nested structure, another such cell, so `loc(member(loc(member(<slot>, 1), 0), 0), 0)`; at least one
    member step (the slot itself is a scalar range's cell).  Squashed text."""
    steps = 0
    while cell != slot:
        m = re.fullmatch(r"loc\(member\((.+),\d+\),0\)", cell)
        if m is None:
            return False
        cell, steps = m.group(1), steps + 1
    return steps >= 1


def _struct_range_minimum_ok(leaves: list[str], slot: str, big: str, formals: frozenset[str]) -> bool:
    """The minimum a `reads_range` over structures' call row divides (decision 0064 item 7, Amounts: the minimum over each leaf
    path's first slot, the liveness cell and the caller's own read amounts): for every leaf cell of the first slot both its value and
    its flag permission (of one type), the liveness cell `big`, and nothing else but the method's own `Perm` formals.  A leaf of
    another range's slot, a cell of another variable, a `Perm` local, a literal or a missing value or flag is refused.  Not checked:
    that every leaf of the element is in it (a missing leaf only leaves the amount above what that leaf holds, which the call's own
    exhale reports as a false alarm, never a proof: `_site_amount_inner`'s note)."""
    held: dict[tuple[str, str], set[str]] = {}
    for leaf in leaves:
        if leaf == big or leaf in formals:
            continue
        m = PERM_LEAF_FIELD_RE.match(leaf)
        if m is None or not _first_slot_leaf(m.group("cell"), slot):
            return False
        held.setdefault((m.group("cell"), m.group("t")), set()).add(m.group("f"))
    return big in leaves and all(fs == {"v", "d"} for fs in held.values())


def _range_amount_inner(d: str, formals: frozenset[str] = frozenset(), slots: dict[str, str] | None = None) -> bool:
    """The amount a range atom's call row divides (decision 0060 item 6, S44 (c), audit round 2 S5): the liveness
    `perm(lvloc(block_of(B)).lv)` of a variable B (an `owns_range`), or, for a `reads_range`, `n > 0 ? min(perm(F.v_t), perm(F.d_t),
    perm(L), a_1, ...) : perm(L)` over the first slot `F = loc(block_of(B), offset_of(B))`, the minimum written as nested conditionals
    (Viper has no `min`) and the `a_i` the method's own `Perm` formals (the caller's read amounts, which only lower the amount).  The
    three `perm`s of the first slot and the liveness cell must all be in it; no other `perm`, local or literal may be."""
    live = RANGE_LIVENESS_RE.match(_sq(d))
    if live:
        return True
    cond = _split_conditional(d)
    if cond is None:
        return False
    if _site_amount_inner(d, formals, slots):
        return True
    c, a, b = cond
    live = RANGE_LIVENESS_RE.match(_sq(b))
    n = re.fullmatch(r"(.+)>0", _sq(c))
    if live is None or n is None or not _heap_free(n.group(1)):
        return False
    bb, big = live.group(1), _sq(b)
    leaves = _min_leaves(a, formals)
    if leaves is None:
        return False
    slot = f"loc(block_of({bb}),offset_of({bb}))"
    if any((m := PERM_LEAF_FIELD_RE.match(leaf)) is not None and m.group("cell") != slot for leaf in leaves if leaf != big and leaf not in formals):
        return _struct_range_minimum_ok(leaves, slot, big, formals)
    types = {m.group(1) for leaf in leaves if (m := re.fullmatch(rf"perm\({re.escape(slot)}\.v_({IDENT})\)", leaf))}
    if len(types) != 1:
        return False
    t = next(iter(types))
    required = {f"perm({slot}.v_{t})", f"perm({slot}.d_{t})", big}
    return required <= set(leaves) and all(leaf in required or leaf in formals for leaf in leaves)


def range_amount_dividend_ok(d: str, formals: frozenset[str] = frozenset(), slots: dict[str, str] | None = None) -> bool:
    """`_range_amount_inner`, or `g ? <that> : write` for a guarded atom (`g` plain: no permission, quantifier or heap read)."""
    if _range_amount_inner(d, formals, slots):
        return True
    cond = _split_conditional(d)
    if cond is None or _sq(cond[2]) != "write":
        return False
    return (GUARD_BAD_RE.search(cond[0]) is None and _heap_free(cond[0]) and _range_amount_inner(cond[1], formals, slots)
            and not RANGE_LIVENESS_RE.match(_sq(cond[0])))


def perm_value_ok(text: str, formals: frozenset[str] = frozenset(), slots: dict[str, str] | None = None) -> bool:
    """Whether `text` is `perm(...)`, `none`, `write` or a conditional `c ? a : b` of those (parentheses anywhere), or the amount of
    a range atom's call row divided by a decimal literal of 2 or more: `perm(lvloc(block_of(B)).lv) / (N + 1)`, with the guard
    `(g ? perm(...) : write)` and, for `reads_range`, the minimum over the first slot (decision 0060 item 6: `range_amount_dividend_ok`).
    Any other quotient, a product or a sum could exceed what the method holds, and stays refused."""
    text = _strip_outer_parens(text)
    if text in ("none", "write"):
        return True
    div = _perm_divisor(text)
    if div is not None:
        return div[1] >= 2 and range_amount_dividend_ok(div[0], formals, slots)
    m = PERM_CALL_RE.match(text)
    if m:
        depth = 0
        for i in range(m.end() - 1, len(text)):
            depth += text[i] == "("
            depth -= text[i] == ")"
            if depth == 0:
                return i == len(text) - 1
        return False
    depth, q = 0, -1
    for i, ch in enumerate(text):
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif ch == "?" and depth == 0:
            q = i
            break
    if q < 0:
        return False
    depth, nested, colon = 0, 0, -1
    for i in range(q + 1, len(text)):
        ch = text[i]
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif depth == 0 and ch == "?":
            nested += 1
        elif depth == 0 and ch == ":":
            if nested == 0:
                colon = i
                break
            nested -= 1
    if colon < 0:
        return False
    return perm_value_ok(text[q + 1:colon], formals, slots) and perm_value_ok(text[colon + 1:], formals, slots)


def while_loops(src: Source, d: Decl):
    """[(line of `while`, the tokens of its `invariant` clauses, the tokens of its body)] of a method with a body; a token is
    (text, line).  The body is between the braces that open it and the matching `}` (a quantifier's trigger set `:: { f(x) }`
    is not a body)."""
    tokens: list[tuple[str, int]] = []
    for lineno in range(d.line, d.end_line + 1):
        tokens += [(m.group(), lineno) for m in TOKEN_RE.finditer(src.code[lineno - 1])]
    out = []
    n = len(tokens)
    for k, (text, lineno) in enumerate(tokens):
        if text != "while" or (k and tokens[k - 1][0] in ("::", "'")):
            continue
        i = k + 1
        if i < n and tokens[i][0] == "(":
            depth = 0
            while i < n:
                depth += tokens[i][0] == "("
                depth -= tokens[i][0] == ")"
                i += 1
                if depth == 0:
                    break
        inv: list[tuple[str, int]] = []
        clause = ""
        trigger_end = -2
        while i < n:
            t = tokens[i][0]
            prev = tokens[i - 1][0] if i else ""
            if t == "{":
                if prev == "::" or i - 1 == trigger_end:
                    level = 0
                    while i < n:
                        level += tokens[i][0] == "{"
                        level -= tokens[i][0] == "}"
                        if level == 0:
                            break
                        if clause == "invariant":
                            inv.append(tokens[i])
                        i += 1
                    trigger_end = i
                    i += 1
                    continue
                break
            if t in ("invariant", "decreases"):
                clause = t
            elif clause == "invariant":
                inv.append(tokens[i])
            i += 1
        body: list[tuple[str, int]] = []
        if i < n and tokens[i][0] == "{":
            level = 0
            j = i
            while j < n:
                level += tokens[j][0] == "{"
                level -= tokens[j][0] == "}"
                body.append(tokens[j])
                j += 1
                if level == 0:
                    break
        out.append((lineno, inv, body))
    return out


GHOST_REF_LOCAL_RE = re.compile(r"(?<![\w$'.])var\s+(?P<names>[A-Za-z_$][\w$']*(?:\s*,\s*[A-Za-z_$][\w$']*)*)\s*:\s*Ref\b")
EB_NAME_RE = re.compile(r"eb_\d+")
LB_NAME_RE = re.compile(r"lb_\d+__\d+")
AW_NAME_RE = re.compile(r"a[wb]_q\d+__\d+")
REF_FIELD_END_RE = re.compile(r"\.v_ptr_[\w$']+$")
LEAF_CELL_RE = re.compile(r"^loc\(member\((?P<base>.*), -?\d+\), 0\)$", re.S)


def ref_read_ok(text: str, conditional: bool) -> bool:
    """Whether `text` is `null` or a plain read of a pointer cell's field (`<place>.v_ptr_...`, a `?:` inside the place is
    part of the place), or, when `conditional` is set, a `c ? a : b` of those (the chain-guarded read of decision 0045 items 4
    and 6: `ite(<framing guards>, read, null)`)."""
    text = _strip_outer_parens(text)
    if text == "null":
        return True
    depth, q = 0, -1
    for i, ch in enumerate(text):
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif ch == "?" and depth == 0:
            q = i
            break
    if q < 0:
        return bool(REF_FIELD_END_RE.search(text))
    if not conditional:
        return False
    depth, nested, colon = 0, 0, -1
    for i in range(q + 1, len(text)):
        ch = text[i]
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif depth == 0 and ch == "?":
            nested += 1
        elif depth == 0 and ch == ":":
            if nested == 0:
                colon = i
                break
            nested -= 1
    if colon < 0:
        return False
    return ref_read_ok(text[q + 1:colon], True) and ref_read_ok(text[colon + 1:], True)


def lint_ghost_refs(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0045 item 14: the ghost `Ref` locals that snapshot the object a contract place designates.  `eb_<N>` (the
    method-entry snapshot of a load-rooted `requires` place) is assigned once, outside every loop, from `null` or a read of a
    pointer cell (or a conditional of those: the chain-guarded read); `lb_<N>__<L>` (the loop-entry snapshot) only from `null`
    or a plain read of a pointer cell, outside every loop body (the loop never assigns it, so the loop's automatic place keeps
    one object); `aw_q<j>__<L>` (a written atom's object at the top of an iteration) is assigned once, from such a read or a
    conditional of them, and only in the leading statements of a loop body, before anything else of the iteration.
    `ab_q<j>__<L>` (decision 0054 item 4: a written `owns_block` site's pointer at the top of an iteration) is the same ghost."""
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        names: set[str] = set()
        for n in range(d.body_line, d.end_line + 1):
            if m := GHOST_REF_LOCAL_RE.search(src.code[n - 1]):
                names |= {x.strip() for x in m.group("names").split(",")}
        ghosts = {x for x in names if EB_NAME_RE.fullmatch(x) or LB_NAME_RE.fullmatch(x) or AW_NAME_RE.fullmatch(x)}
        if not ghosts:
            continue
        loops = while_loops(src, d)
        bodies = [(b[0][1], b[-1][1], b) for _ln, _inv, b in loops if b]   # (line of `{`, line of `}`, tokens)
        assigns: dict[str, list[tuple[int, str]]] = {g: [] for g in ghosts}
        for n in range(d.body_line, d.end_line + 1):
            code = src.code[n - 1]
            for g in sorted(ghosts):
                for a in re.finditer(r"(?<![\w$'.])" + re.escape(g) + r"\s*:=", code):
                    assigns[g].append((n, _statement_rhs(code, a.end())))

        def in_loop(line: int) -> bool:
            return any(lo < line <= hi for lo, hi, _b in bodies)

        for g in sorted(ghosts):
            lst = assigns[g]
            if EB_NAME_RE.fullmatch(g):
                if len(lst) != 1:
                    add(d.line, f"ghost `Ref` local `{g}` is assigned {len(lst)} times: the method-entry snapshot is assigned once "
                                "(decision 0045 item 14)")
                for n, rhs in lst:
                    if in_loop(n):
                        add(n, f"ghost `Ref` local `{g}` is assigned inside a loop: the method-entry snapshot is assigned once at the "
                               "method's entry (decision 0045 item 14)")
                    elif not ref_read_ok(rhs, True):
                        add(n, f"ghost `Ref` local `{g}` is assigned `{rhs}`: a snapshot is `null`, a read of a pointer cell or a "
                               "conditional of those (decision 0045 item 14)")
            elif LB_NAME_RE.fullmatch(g):
                if not 1 <= len(lst) <= 2:
                    add(d.line, f"ghost `Ref` local `{g}` is assigned {len(lst)} times: `:= null`, then at most one guarded read, "
                                "right before its loop (decision 0045 item 14)")
                for k, (n, rhs) in enumerate(lst):
                    if in_loop(n):
                        add(n, f"ghost `Ref` local `{g}` is assigned inside a loop: the loop-entry snapshot is assigned only before "
                               "its loop (decision 0045 item 14)")
                    elif (k == 0 and rhs != "null") or (k > 0 and not ref_read_ok(rhs, False)):
                        add(n, f"ghost `Ref` local `{g}` is assigned `{rhs}`: `null` first, then a plain read of a pointer cell "
                               "(decision 0045 item 14)")
            else:
                if len(lst) != 1:
                    add(d.line, f"ghost `Ref` local `{g}` is assigned {len(lst)} times: a written atom's object is assigned once, "
                                "at the top of an iteration (decision 0045 item 14)")
                for n, rhs in lst:
                    host = [(lo, hi) for lo, hi, _b in bodies if lo < n <= hi]
                    if not host:
                        add(n, f"ghost `Ref` local `{g}` is assigned outside a loop body: it holds the written atom's object at the "
                               "top of each iteration (decision 0045 item 14)")
                        continue
                    lo, _hi = min(host, key=lambda h: h[1] - h[0])
                    # only ghost assignments between the loop's `{` and this one
                    for m in range(lo + 1, n):
                        if src.code[m - 1].strip() and not any(re.match(r"\s*" + re.escape(x) + r"\s*:=", src.code[m - 1]) for x in ghosts if AW_NAME_RE.fullmatch(x)):
                            add(n, f"ghost `Ref` local `{g}` is assigned after other statements of the loop body (line {m}): it is "
                                   "the first statement of an iteration (decision 0045 item 14)")
                            break
                    leaf = LEAF_CELL_RE.match(_strip_outer_parens(rhs))   # a struct atom's leaf cell: loc(member(<chain read>, k), 0)
                    if not ref_read_ok(leaf.group("base") if leaf else rhs, True):
                        add(n, f"ghost `Ref` local `{g}` is assigned `{rhs}`: a read of a pointer cell, `null`, a conditional of "
                               "those or, for a struct atom, a leaf cell `loc(member(<such a read>, k), 0)` (decision 0045 item 14)")



CALL_RHS_RE = re.compile(r"^(?P<callee>[A-Za-z_$][\w$']*)\s*\(.*\)$", re.S)


def perm_call_target_ok(src: Source, by_name: dict[str, Decl], code: str, start: int, end: int, rhs: str) -> bool:
    """Decision 0052 item 5 (amending 0038 item 9): a `Perm` local that is a target of a multi-target call `t, o := f(...)` is the
    output amount of a partnerless atom when the callee is a method of this file whose result at that position is a `Perm` with
    exactly one `ensures none < o && o <= write` before any clause that mentions it (the amount a caller receives is bounded
    whatever the callee does).  The target is `code[start:end]`; every other call target, and every other right-hand side, is
    the rule of decision 0038 item 9."""
    m = CALL_RHS_RE.match(rhs)
    callee = by_name.get(m.group("callee")) if m else None
    if callee is None:
        return False
    colon = code.index(":=", end)
    targets = re.search(r"((?:[A-Za-z_$][\w$']*\s*,\s*)*[A-Za-z_$][\w$']*)\s*$", code[:colon])
    if targets is None:
        return False
    names = [t.strip() for t in targets.group(1).split(",")]
    results = method_results(src, callee)
    position = len(re.findall(r"[A-Za-z_$][\w$']*", code[targets.start():start]))
    if len(results) != len(names) or position >= len(results):
        return False
    o, typ = results[position]
    return typ == "Perm" and perm_result_has_row(src, callee, o)


def member_bound_slots(src: Source, d: Decl) -> dict[str, str]:
    """The locals of method `d` that the body assigns exactly once `loc(member(<chain>, k), 0)` (a member's bound local, decision 0063 item 15), as
    {local: the variable the canonical chain is rooted at}."""
    assigns: dict[str, list[str]] = {}
    for n in range(d.line + 1, d.end_line + 1):
        line = src.code[n - 1]
        if (m := LOCAL_ASSIGN_RE.match(line)):
            for t in (x.strip() for x in m.group("lhs").split(",")):
                assigns.setdefault(t, []).append(norm_viper(m.group("rhs")) if "," not in m.group("lhs") else "<multi-target>")
        elif (m := LOCAL_VAR_INIT_RE.match(line)):
            assigns.setdefault(m.group("x"), []).append(norm_viper(m.group("rhs")))
    out: dict[str, str] = {}
    for x, rhss in assigns.items():
        bm = BOUND_RHS_RE.match(rhss[0]) if len(rhss) == 1 else None
        rm = re.match(rf"^(?:member\(loc\()*member\(({IDENT}),", bm.group("b")) if bm and bm.group("j") == "0" else None
        if rm and parse_struct_member(bm.group("b"), rm.group(1)) is not None:
            out[x] = rm.group(1)
    return out


def lint_perm_locals(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0038 item 9: the amounts of a loop's invariant are `Perm` locals that snapshot what the method holds.  A
    local is assigned only `perm(...)`, `none`, `write` or a conditional of those (an assignment of anything else, `wildcard`
    included, could give the invariant an amount that the method does not hold or above `write`); it is never assigned inside
    a loop whose invariant mentions it (the loop head havocs what the body assigns, so the amount could exceed `write`
    and make the body vacuous); and `perm(...)` is not written in an invariant (Viper refuses it: the snapshot is the way)."""
    by_name = {x.name: x for x in decls if x.kind == "method"}
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        names = perm_local_names(src, d)
        formals = frozenset(m.group("name") for f in method_formals(src, d) if (m := PERM_FORMAL_RE.match(f)))
        slots = member_bound_slots(src, d)
        for n in range(d.body_line, d.end_line + 1):
            code = src.code[n - 1]
            if (m := PERM_LOCAL_RE.search(code)) and ":=" in m.group("rest"):
                rhs = _statement_rhs(m.group("rest"), m.group("rest").index(":=") + 2)
                if not perm_value_ok(rhs, formals, slots) and not WILDCARD_RE.search(rhs):  # wildcard: reported by its own rule
                    add(n, f"`Perm` local `{m.group('names')}` is initialized with `{rhs}`: a snapshot is `perm(...)`, `none`, "
                           "`write` or a conditional of those (decision 0038 item 9)")
            for name in sorted(names):
                # `name` is a target when `:=` follows it directly or after more targets: `a, b := m(args)` assigns both (decision
                # 0044 item 13: a multi-target call is one assignment of every target; the first target must be seen too)
                for a in re.finditer(r"(?<![\w$'.])" + re.escape(name) + r"\s*(?=(?:,\s*[A-Za-z_$][\w$']*\s*)*:=)", code):
                    rhs = _statement_rhs(code, code.index(":=", a.end()) + 2)
                    if not perm_value_ok(rhs, formals, slots) and not WILDCARD_RE.search(rhs):  # wildcard: reported by its own rule
                        if perm_call_target_ok(src, by_name, code, a.start(), a.end(), rhs):
                            continue
                        add(n, f"`Perm` local `{name}` is assigned `{rhs}`: a snapshot is `perm(...)`, `none`, `write` or a "
                               "conditional of those, never arithmetic or another variable (decision 0038 item 9; `wildcard` has its own rule)")
        for line, inv, body in while_loops(src, d):
            used = {t for t, _ln in inv} & names
            for k, (t, ln) in enumerate(inv):
                if t == "perm" and k + 1 < len(inv) and inv[k + 1][0] == "(" and (k == 0 or inv[k - 1][0] != "."):
                    add(ln, "`perm(...)` in a loop invariant: Viper refuses it (decision 0038 item 9); the invariant uses a "
                            "`Perm` local snapshotted before the loop")
            for k, (t, ln) in enumerate(body):
                j = k + 1  # the targets after this one: `kv, other := m(args)` assigns kv as well
                while j + 1 < len(body) and body[j][0] == "," and re.fullmatch(r"[A-Za-z_$][\w$']*", body[j + 1][0]):
                    j += 2
                if t in used and j < len(body) and body[j][0] == ":=" and (k == 0 or body[k - 1][0] != "."):
                    add(ln, f"`Perm` local `{t}` is assigned inside the loop of line {line}, whose invariant uses it as an "
                            "amount: the loop head havocs what the body assigns (decision 0038 item 9)")


# The allocator ghost of an owning loop variable in a vlibc heap program (decision 0052 item 3 Loops, shapes L1-L3): `tve_<v>__<loop> :=
# (<x> != null) ? tv_<y> : true` is the token value the owner held at the loop's entry, a method-level `Bool` that the loop's invariant and
# its rows compare with the token.  It is assigned only that way (the value is the owner's own ghost, never a read of a token), and never
# inside a loop whose invariant mentions it (the loop head would havoc it and the invariant would no longer say which allocator's block
# the owner holds).
TVE_WORD_RE = re.compile(r"(?<![\w$'.])tve_[\w$']*")
TVE_ASSIGN_RE = re.compile(r"^\s*tve_[\w$']*\s*:=\s*\(\s*[A-Za-z_$][\w$']*\s*!=\s*null\s*\)\s*\?\s*tv_[\w$']*\s*:\s*true\s*$")


def lint_owner_loop_ghosts(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    for lineno, code in enumerate(src.code, start=1):
        m = re.match(r"^\s*(?P<name>tve_[\w$']*)\s*:=", code)
        if m and not TVE_ASSIGN_RE.match(code):
            add(lineno, f"`{m.group('name')}` is assigned something but `(<x> != null) ? tv_<y> : true`: the allocator ghost of an owning loop "
                        "variable is its own token value, never a read of a token or another expression (decision 0052 item 3)")
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        for line, inv, body in while_loops(src, d):
            used = {t for t, _ln in inv if TVE_WORD_RE.fullmatch(t)}
            for k, (t, ln) in enumerate(body):
                if t in used and k + 1 < len(body) and body[k + 1][0] == ":=" and (k == 0 or body[k - 1][0] != "."):
                    add(ln, f"`{t}` is assigned inside the loop of line {line}, whose invariant mentions it: the loop head havocs what "
                            "the body assigns (decision 0052 item 3)")


# The loop-entry ghosts of the loop carriers (decision 0054 items 3 and 4), each a local that the lowering assigns once before its loop
# and the loop never assigns, so the invariant that names it states a fact about the loop's entry and not an assumption:
#   `ev_<v>__<L>` (`Ref`): the owner `v`'s value at entry, `ev_<v>__<L> := <v>`; the value set `v == ev || v == null || v == x...` names it;
#   `hs_<v>__<L>` (`Bool`): `held_<v>` at entry, `hs_<v>__<L> := held_<v>`; the invariant is `held_<v> == hs_<v>__<L>`, its back-edge row
#       `if (!(hs_<v>__<L> ==> held_<v>)) {`, `assert false`, `}`;
#   `dup_...__<L>` and `ad_...__<L>` (`Bool`): whether two carriers shared a block at entry, `S := ((a != null) && (b != null)) &&
#       (block_of(a) == block_of(b))`; it is used negated (`!S`) as a carrier guard and, positively, only in the exclusivity clause
#       `invariant S || (!(<the same pair formula>))` and its back-edge row `if (!(S || (!(<formula>)))) {`.
# Each is assigned exactly once, before the `while` of its own loop (the number after the last `__`, in the order of the loops'
# `while`s; an inner loop's ghost may stand in the outer loop's body), from exactly that right-hand side.
LOOP_GHOST_LOCAL_RE = re.compile(r"(?<![\w$'.])var\s+(?P<names>[A-Za-z_$][\w$']*(?:\s*,\s*[A-Za-z_$][\w$']*)*)\s*:\s*(?P<type>[A-Za-z]+)\b")
EV_NAME_RE = re.compile(r"^ev_(?P<v>[A-Za-z_$][\w$']*)__(?P<l>\d+)$")
HS_NAME_RE = re.compile(r"^hs_(?P<v>[A-Za-z_$][\w$']*)__(?P<l>\d+)$")
SNAP_NAME_RE = re.compile(r"^(?:dup|ad)_[\w$']*__\d+$")
PAIR_FORMULA_RE = re.compile(r"^\(\((?P<a>[A-Za-z_$][\w$']*)!=null\)&&\((?P<b>[A-Za-z_$][\w$']*)!=null\)\)&&"
                             r"\(block_of\((?P=a)\)==block_of\((?P=b)\)\)$")
INV_LINE_RE = re.compile(r"^\s*invariant\b(?P<rest>.*)$")


def _nospace(text: str) -> str:
    return re.sub(r"\s+", "", text)


def lint_loop_entry_ghosts(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        types: dict[str, str] = {}
        for n in range(d.body_line, d.end_line + 1):
            if m := LOOP_GHOST_LOCAL_RE.search(src.code[n - 1]):
                for x in (x.strip() for x in m.group("names").split(",")):
                    types[x] = m.group("type")
        ghosts: dict[str, str] = {}
        for x, t in types.items():
            kind = "ev" if EV_NAME_RE.match(x) else "hs" if HS_NAME_RE.match(x) else "snap" if SNAP_NAME_RE.match(x) else None
            if kind is None:
                continue
            ghosts[x] = kind
            if t != ("Ref" if kind == "ev" else "Bool"):
                add(d.line, f"loop-entry ghost `{x}` is a `{t}` local: `ev_...` is a `Ref`, `hs_...`, `dup_...` and `ad_...` are `Bool` "
                            "(decision 0054 item 3)")
        if not ghosts:
            continue
        loops = while_loops(src, d)   # numbered in the order of their `while`, as the lowering numbers them
        assigns: dict[str, list[tuple[int, str]]] = {g: [] for g in ghosts}
        for n in range(d.body_line, d.end_line + 1):
            code = src.code[n - 1]
            for g in ghosts:
                for a in re.finditer(r"(?<![\w$'.])" + re.escape(g) + r"\s*:=", code):
                    assigns[g].append((n, _statement_rhs(code, a.end())))
        rhs_of: dict[str, str] = {}
        for g, kind in sorted(ghosts.items()):
            lst = assigns[g]
            loop_no = int(re.search(r"__(\d+)$", g).group(1))
            if len(lst) != 1:
                add(d.line, f"loop-entry ghost `{g}` is assigned {len(lst)} times: it is assigned once, right before its loop "
                            "(decision 0054 item 3)")
            if loop_no >= len(loops):
                add(d.line, f"loop-entry ghost `{g}` names loop {loop_no}, which the method does not have (decision 0054 item 3)")
            for n, rhs in lst:
                if loop_no < len(loops) and n >= loops[loop_no][0]:
                    add(n, f"loop-entry ghost `{g}` is assigned at or after the `while` of its own loop: the loop head would havoc it and "
                           "the invariant that names it would no longer say anything about the entry (decision 0054 item 3)")
                if kind == "ev":
                    want = EV_NAME_RE.match(g).group("v")
                    ok = _nospace(rhs) == want
                    what = f"`{want}`, the owner's own value"
                elif kind == "hs":
                    want = "held_" + HS_NAME_RE.match(g).group("v")
                    ok = _nospace(rhs) == want
                    what = f"`{want}`"
                else:
                    ok = PAIR_FORMULA_RE.match(_nospace(rhs)) is not None
                    what = "`((a != null) && (b != null)) && (block_of(a) == block_of(b))`"
                    rhs_of[g] = _nospace(rhs)
                if not ok:
                    add(n, f"loop-entry ghost `{g}` is assigned `{rhs}`: its right-hand side is exactly {what} (decision 0054 item 3)")
        alt = "|".join(re.escape(g) for g in sorted(ghosts, key=len, reverse=True))
        word_re = re.compile(r"(?<![\w$'.])(" + alt + r")(?![\w$'])")
        for n in range(d.body_line, d.end_line + 1):
            code = src.code[n - 1]
            if not word_re.search(code):
                continue
            if re.match(r"^\s*(?:var\s+(?:" + alt + r")\s*:|(?:" + alt + r")\s*:=)", code):
                continue
            norm = _nospace(code)
            for g in sorted({m.group(1) for m in word_re.finditer(code)}):
                kind = ghosts[g]
                if kind == "hs":
                    v = HS_NAME_RE.match(g).group("v")
                    if norm == f"invariantheld_{v}=={g}":
                        continue
                    if is_ending_row(src, n) and norm == f"if(!({g}==>held_{v})){{":
                        continue
                    add(n, f"`{g}` appears outside `invariant held_{v} == {g}` and its back-edge row `if (!({g} ==> held_{v})) {{`: it is "
                           "the loop-entry value of the owner's flag and nothing else (decision 0054 item 3)")
                elif kind == "ev":
                    v = EV_NAME_RE.match(g).group("v")
                    m = INV_LINE_RE.match(code)
                    terms = _nospace(m.group("rest")).replace("(", "").replace(")", "").split("||") if m else []
                    if terms and terms[0] == f"{v}=={g}" and all(re.fullmatch(re.escape(v) + r"==[A-Za-z_$][\w$']*", t) for t in terms[1:]):
                        continue
                    add(n, f"`{g}` appears outside the value set `invariant {v} == {g} || {v} == null || {v} == x...`: it is the "
                           "owner's entry value and only the value set names it (decision 0054 item 3)")
                else:
                    f = rhs_of.get(g)
                    if f is not None and PAIR_FORMULA_RE.match(f):
                        if norm == f"invariant{g}||(!({f}))":
                            continue
                        if is_ending_row(src, n) and norm == f"if(!({g}||(!({f})))){{":
                            continue
                    bare = [m for m in word_re.finditer(code) if m.group(1) == g and not code[:m.start()].rstrip().endswith("!")]
                    if bare:
                        add(n, f"`{g}` is used positively outside the exclusivity clause `invariant {g} || (!(<its pair formula>))` and its "
                               f"back-edge row: a carrier guard is `!{g}` (decision 0054 item 3)")


# The token clause of a loop carrier (decision 0054 items 3 and 4): an `invariant` that holds `acc(loc(block_of(P), -1).tok ...)` is an
# implication whose antecedent names `P`, so a carrier never holds a token for a pointer its guard does not mention (a written site's
# `P != null ==> ...`, an owner's `held_o && ... ==> ...`, a place's `... && P != null ==> ...`).
TOK_ACC = "acc(loc(block_of("


# The ghost pointer of a block site read through memory (decision 0064 item 15, step A and step 1): `abp_<i>__<loop>` (`Ref`) names the block whose array parts a
# loop carries, because the quantified permissions need a variable as their trigger.  It is assigned in two shapes only, each under the single-statement
# `if (C) {` ... `}` the emitter writes:
#   (a) `if ((!(E == null)) && (E != null)) { abp := E }` with E `kv_<x> > none ? loc(GB_<g>(), 0).v_ptr_<t> : null`, a held global's pointer, at the loop's entry;
#   (b) `if (G && (X != null)) { abp := X }` with G a `Bool` local and X a `Ref` local or formal, at the back edge of a loop that may write the pointer:
#       the ghost takes the pointer the body ends with, and only where it is a block.
ABP_ASSIGN_RE = re.compile(rf"^\s*(?P<n>abp_[\w$']*)\s*:=\s*(?P<rhs>.+?)\s*$")
ABP_ENTRY_RE = re.compile(r"^(?P<kv>kv_[\w$']+)>none\?(?P<cell>loc\((?P<gb>GB_[\w$']+)\(\),0\)\.v_ptr_[\w$']+):null$")


def _plain_assignments_at(src: Source, d: Decl, name: str) -> list[tuple[int, str]]:
    """(line, the squashed right-hand side) of the statements `name := rhs` of the method `d`."""
    out: list[tuple[int, str]] = []
    for n in range(d.body_line, d.end_line + 1):
        m = re.match(rf"^\s*{re.escape(name)}\s*:=\s*(?P<rhs>.+?)\s*$", src.code[n - 1])
        if m is not None:
            out.append((n, norm_viper(m.group("rhs"))))
    return out


def _plain_assignments(src: Source, d: Decl, name: str) -> list[str]:
    """The squashed right-hand sides of the statements `name := rhs` of the method `d`."""
    return [rhs for _, rhs in _plain_assignments_at(src, d, name)]


def _enclosing_loop_start(src: Source, d: Decl, lineno: int) -> int | None:
    """The line of the `{` that opens the body of the innermost `while` around line `lineno` of the method `d` (the emitter writes `while (c)`, its `invariant` and
    `decreases` lines, then a `{` of its own), None outside every loop."""
    stack: list[int | None] = []
    pending = False
    for n in range(max(d.body_line, 1) + 1, lineno):
        code = src.code[n - 1].strip()
        if code.startswith("while"):
            pending = True
        elif code.startswith("}"):
            if stack:
                stack.pop()
            if code.endswith("{"):
                stack.append(None)
        elif code.endswith("{") and not code.endswith("{ }"):
            stack.append(n if pending and code == "{" else None)
            pending = False
    inner = [x for x in stack if x is not None]
    return inner[-1] if inner else None


def _abp_tie_invariants(src: Source, d: Decl, name: str) -> set[str]:
    """The squashed `E` of the loop invariants `G ==> E == <name>` of the method `d`: the pointer the ghost `name` stands for (decision 0064 item 15)."""
    out: set[str] = set()
    for n in range(d.body_line, d.end_line + 1):
        m = INV_LINE_RE.match(src.code[n - 1])
        if m is None:
            continue
        parts = norm_viper(m.group("rest")).split("==>", 1)
        if len(parts) == 2 and parts[1].endswith(f"=={name}") and len(parts[1]) > len(name) + 2:
            out.add(parts[1][:-(len(name) + 2)])
    return out


def lint_abp_ghosts(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    gb_names = {m.group("name") for l in src.code if (m := GB_DECL_RE.match(l))}
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        refs = method_locals(src, d, REF_LOCAL_RE)
        bools = method_locals(src, d, BOOL_LOCAL_RE)
        perms = perm_local_names(src, d)
        pointers = refs | set(_formal_names(src, d))
        formals = set(_formal_names(src, d))
        for n in range(d.body_line, d.end_line + 1):
            m = ABP_ASSIGN_RE.match(src.code[n - 1])
            if m is None:
                continue
            name, rhs = m.group("n"), norm_viper(m.group("rhs"))
            prev = src.code[n - 2].strip() if n >= 2 else ""
            nxt = src.code[n].strip() if n < len(src.code) else ""
            im = re.match(r"^if\s*\((?P<c>.*)\)\s*\{$", prev)
            cond = norm_viper(im.group("c")) if im is not None and nxt == "}" else None
            if name not in refs:
                add(n, f"`{name}` is assigned but is no declared `Ref` local of the method")
            elif cond is None:
                add(n, f"`{name}` is assigned outside a single-statement `if (C) {{ ... }}`: a ghost block pointer takes its value only where its guard holds")
            elif (em := ABP_ENTRY_RE.match(rhs)) is not None:
                if cond != f"!{rhs}==null&&{rhs}!=null" or em.group("gb") not in gb_names or em.group("kv") not in perms:
                    add(n, f"`{name}` takes a held global's pointer under a guard that is not `(!(E == null)) && (E != null)` of that very E, or of a Perm local "
                           "and a declared global that it does not name")
                elif _plain_assignments(src, d, em.group("kv")) != [f"perm({em.group('cell')})"]:
                    add(n, f"`{name}` reads a global's pointer under `{em.group('kv')}`, which is not the one snapshot `{em.group('kv')} := perm(<that very cell>)` "
                           "of the method: the guard's amount and the pointer it guards are one cell's")
                elif rhs not in _abp_tie_invariants(src, d, name):
                    add(n, f"`{name}` takes `{m.group('rhs')}` but no loop invariant `G ==> <that pointer> == {name}` of the method ties the ghost to it")
            elif re.fullmatch(IDENT, rhs) and rhs in pointers:
                cm = re.fullmatch(rf"({IDENT})&&{re.escape(rhs)}!=null", cond)
                if cm is None or cm.group(1) not in bools:
                    add(n, f"`{name}` is retaken from `{rhs}` under a guard that is not `G && ({rhs} != null)` with G a `Bool` local")
                else:
                    ties = _abp_tie_invariants(src, d, name)
                    g = cm.group(1)
                    start = _enclosing_loop_start(src, d, n)
                    xs = _plain_assignments_at(src, d, rhs)
                    gs = _plain_assignments_at(src, d, g)
                    # the pointer and the guard the ghost is retaken from are the ones the body ENDS with: each is assigned once, from the invariant's E, inside this
                    # loop's body (a copy of E taken before the loop holds the entry's pointer, not the pointer the body leaves)
                    x_ok = (len(xs) == 1 and xs[0][1] in ties and start is not None and xs[0][0] > start) or (rhs in formals and rhs in ties)
                    g_ok = len(gs) == 1 and gs[0][1] in {f"!{e}==null" for e in ties} and start is not None and gs[0][0] > start
                    if not ties:
                        add(n, f"`{name}` is retaken from `{rhs}` but no loop invariant `G ==> <pointer> == {name}` of the method names the pointer it stands for")
                    elif not x_ok:
                        add(n, f"`{name}` is retaken from `{rhs}`, which is not (assigned once from) the pointer `E` of the invariant `G ==> E == {name}`")
                    elif not g_ok:
                        add(n, f"`{name}` is retaken under `{g}`, which is not the one local assigned `!(E == null)` for the pointer `E` of the invariant `G ==> E == {name}`")
            else:
                add(n, f"`{name}` is assigned `{m.group('rhs')}`: a ghost block pointer is a held global's pointer or a pointer local that is not null")


def lint_carrier_clauses(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        for n in range(d.body_line, d.end_line + 1):
            m = INV_LINE_RE.match(src.code[n - 1])
            if m is None:
                continue
            text = _nospace(m.group("rest"))
            i = text.find(TOK_ACC)
            while i >= 0:
                start = i + len("acc(loc(")
                end = _group_end(text, start + len("block_of"))
                arg = text[start + len("block_of("):end] if end >= 0 else ""
                tail = text[end + 1:] if end >= 0 else ""
                if end >= 0 and re.match(r",\(?-1\)?\)\.tok", tail):
                    depth, arrow = 0, -1
                    for k in range(len(text)):
                        c = text[k]
                        depth += c == "("
                        depth -= c == ")"
                        if depth == 0 and text.startswith("==>", k):
                            arrow = k
                            break
                    if arrow < 0 or arrow > i or arg not in text[:arrow]:
                        add(n, f"an `invariant` holds the token of `block_of({arg})` outside an implication whose antecedent names "
                               f"`{arg}`: a loop carrier is guarded by its pointer (decision 0054 items 3 and 4)")
                        break
                i = text.find(TOK_ACC, i + 1)


# Owning globals, `static` locals and address-taken owners (decision 0054 items 5, 6 and 8) and the owning leaves of struct objects
# (decision 0055 items 5 and 9).  The lowering gives each such owner the method-level ghost `held_<o>` (for a global or `static` local
# `<o>` is its block's name, `GB_<name>`; an owning local also has `tv_<o>`), and `main`'s leak row exempts the block of an owning global
# by a term `held_GB_x && t == loc(block_of(loc(GB_x(), 0).v_ptr_T), -1)`.  An owning leaf of a struct has the ghost `held_<root>_<k>...`
# (`<root>` the global's block `GB_x` or the struct local's variable, `k...` the member path) and the leaf's cell `loc(member(<chain>, k),
# 0).v_ptr_T` in place of the owner's own cell.  The lint reads only the Viper, so:
#   * `program_start` carries one tag line `// cerastes:owning-global GB_<name>` per owning pointer global, and
#     `// cerastes:owning-global GB_<name>/<k>[/<k>...]` per owning leaf of a struct global (right after its head, before its first
#     clause), each naming a declared `unique function GB_<name>()` once; a member path is pinned against the global's `program_start`
#     row (a pointer leaf of that struct, 0055 item 9), a struct global is tagged only by member paths; `held_GB_x...` exists only for a
#     tagged block or leaf, under the name the tag spells (`held_GB_x_<k>_<k>`);
#   * a `held_<o>` is assigned only: `false`; `<x> != null` (the owner received a value; its value row precedes) or `<leaf cell> != null`
#     (an owning leaf, the cell of that very leaf); `held_<o> && ((<x> != null) && (perm(tok(<x>)) > none))` (after a call or a free,
#     local); `(held_<o> && (perm(<C>) > none)) && ((<C> != null) && (perm(tok(<C>)) > none))` (after a call or a free, global or
#     address-taken owner, `<C>` the owner's own cell `loc(GB_x(), 0).v_ptr_T` or `<o>.v_ptr_T`, or the leaf's cell: the ghost's name must
#     be the one that cell spells); `held_<o> || (((<x> != null) && (offset_of(<x>) == 0)) && (perm(tok(<x>)) == write))` (an address-taken
#     owner that received a block): it never becomes `true` without a read of the token's permission; and, after a call that passes a
#     pointer variable `<a>` whose value may be the owner's own struct (decision 0056 item 4(a), the guarded update of 0054 item 6),
#     `(<a> == <R>) ? (<t> != null) : held_<o>` for an owning leaf, `<R>` the leaf's own root address (the struct local's variable, or
#     `loc(GB_x(), 0)` of the struct global) and `<t>` the temp that the nearest earlier assignment just read from that very leaf's cell
#     under the same guard, `<t> := ((<a> == <R>) && (perm(<cell>) > none)) ? <cell> : null` (see `guarded_update_problem`): the ghost
#     changes only where `<a>` is the owner and the cell's permission was read, and never to `true` on a value that was not read there;
#   * a `tv_<o>` (an owning local's allocator ghost) is assigned only `(<x> != null) ? tok(<x>) : tv_<o>`;
#   * an exemption term of a `forperm` row with a `held_GB_x` guard names exactly `t == loc(block_of(loc(GB_x(), 0).v_ptr_T), -1)`, with a
#     leaf's ghost exactly `t == loc(block_of(<that leaf's cell>), -1)` (a deeper term, 0055 S4, extends `exemption_term_problem`);
#   * a comparison of `held_GB_x...` with a value is `held_GB_x == (<its cell> != null)`, as the loop invariant and its rows say.
OWNING_GLOBAL_TAG_RE = re.compile(r"//\s*cerastes:owning-global\b[ \t]*(?P<name>\S+)?")
GB_CALL_RE = re.compile(r"(?<![\w$'])(?P<name>GB_[A-Za-z0-9_$']+)\s*\(\s*\)")
GHOST_ANY_RE = re.compile(r"(?<![\w$'.])(?P<ghost>held_GB_[\w$']*)")
LOCAL_GHOST_COMPARE_RE = re.compile(r"(?<![\w$'.])(?P<ghost>held_(?!GB_)[\w$']+)\s*==(?!>)")
OWNER_ASSIGN_RE = re.compile(r"(?<![\w$'.])(?P<g>(?:held|tv)_[\w$']+)\s*:=")
_ID = r"[A-Za-z_$][\w$']*"
_SLOT = r"loc\(block_of\((?P<{n}>.+?)\),\(?-1\)?\)\.tok"
_LEAF_CELL_RE = re.compile(r"^(?P<slot>loc\(member\(.*,[0-9]+\),0\))\.v_ptr_[\w$']+$")
_PATH_INDEX_RE = re.compile(r"0|[1-9][0-9]*")


def leaf_cell_owner(c: str) -> tuple[str, str, tuple[int, ...]] | None:
    """A leaf's cell `loc(member(<chain>, k), 0).v_ptr_T` (normalized text) as (`global` or `local`, the block `GB_x` or the variable, the
    member path), the chain rooted at `loc(GB_x(), 0)` or at the variable and nothing else inside (the grammar of decision 0043 item 13)."""
    m = _LEAF_CELL_RE.match(c)
    if m is None:
        return None
    slot = m.group("slot")
    if g := re.search(r"loc\((GB_[\w$']+)\(\),0\)", slot):
        path = parse_struct_chain(slot, g.group(0))
        return ("global", g.group(1), path) if path else None
    if v := re.search(rf"member\(({_ID}),", slot):
        path = parse_struct_chain(slot, v.group(1))
        return ("local", v.group(1), path) if path else None
    return None


def leaf_ghost_name(root: str, path: tuple[int, ...]) -> str:
    return "held_" + root + "".join(f"_{k}" for k in path)


def owner_assignment_ok(g: str, rhs: str) -> bool:
    r = _nospace(rhs)
    e = re.escape(g)
    if g.startswith("tv_"):
        return re.fullmatch(rf"\((?P<x>{_ID})!=null\)\?loc\(block_of\((?P=x)\),\(?-1\)?\)\.tok:{e}", r) is not None
    if r == "false" or re.fullmatch(rf"{_ID}!=null", r):
        return True
    if m := re.fullmatch(r"(?P<c>.+)!=null", r):
        lo = leaf_cell_owner(m.group("c"))
        if lo is not None:
            return g == leaf_ghost_name(lo[1], lo[2])
    # decision 0059 item 10: `holds_token(x)` is `x != null && offset_of(x) == 0 && perm(tok) > none` (a pointer into the middle of a block holds no token)
    if re.fullmatch(rf"{e}&&\(\(\((?P<x>{_ID})!=null\)&&\(offset_of\((?P=x)\)==0\)\)&&\(perm\(loc\(block_of\((?P=x)\),\(?-1\)?\)\.tok\)>none\)\)", r):
        return True
    if re.fullmatch(rf"{e}\|\|\(\(\((?P<x>{_ID})!=null\)&&\(offset_of\((?P=x)\)==0\)\)&&\(perm\(loc\(block_of\((?P=x)\),\(?-1\)?\)\.tok\)==write\)\)", r):
        return True
    m = re.fullmatch(rf"\({e}&&\(perm\((?P<c>.+?)\)>none\)\)&&\(\(\((?P=c)!=null\)&&\(offset_of\((?P=c)\)==0\)\)&&\(perm\(loc\(block_of\((?P=c)\),\(?-1\)?\)\.tok\)>none\)\)", r)
    if m:
        c = m.group("c")
        if lo := leaf_cell_owner(c):
            return g == leaf_ghost_name(lo[1], lo[2])
        if gm := re.fullmatch(r"loc\((?P<gb>GB_[\w$']+)\(\),0\)\.v_ptr_[\w$']+", c):
            return g == "held_" + gm.group("gb")
        return re.fullmatch(rf"{_ID}\.v_ptr_[\w$']+", c) is not None and c.split(".")[0] == g[len("held_"):]
    return False


def guarded_update_form(g: str, rhs: str) -> tuple[str, str, str] | None:
    """`(<a> == <R>) ? (<t> != null) : <g>` (decision 0056 item 4(a)) as (a, R, t), `<R>` an identifier or `loc(GB_x(), 0)`, for an
    owner ghost `held_...`; None for any other text (which `owner_assignment_ok` and its message then judge)."""
    if not g.startswith("held_"):
        return None
    m = re.fullmatch(rf"\((?P<a>{_ID})==(?P<r>{_ID}|loc\(GB_[\w$']+\(\),0\))\)\?\((?P<t>{_ID})!=null\):{re.escape(g)}", _nospace(rhs))
    return None if m is None else (m.group("a"), m.group("r"), m.group("t"))


def leaf_root_address(lo: tuple[str, str, tuple[int, ...]]) -> str:
    """The address of the struct a leaf belongs to, as the guard of a guarded update names it: a struct local's variable, a struct
    global's `loc(GB_x(), 0)` (normalized text)."""
    kind, root, _path = lo
    return root if kind == "local" else f"loc({root}(),0)"


def guarded_update_problem(src: Source, lineno: int, g: str, form: tuple[str, str, str]) -> str | None:
    """Why `held_<leaf> := (a == R) ? (t != null) : held_<leaf>` is not the guarded update of decision 0056 item 4(a), or None: the
    nearest earlier assignment to `t` (in the same method) is exactly `((a == R) && (perm(C) > none)) ? C : null` with the same `a`
    and `R`, `C` the cell of the very leaf the ghost's name spells, and `R` the address of the struct that leaf belongs to."""
    a, r, t = form
    rhs = None
    for n in range(lineno - 1, max(0, lineno - 40), -1):
        code = src.code[n - 1]
        if re.match(r"^(method|function)\b", code):
            break
        m = re.match(rf"^\s*{re.escape(t)}\s*:=", code)
        if m:
            rhs = _nospace(_statement_rhs(code, m.end()))
            break
    where = "(decision 0054 item 6, decision 0056 item 4(a))"
    if rhs is None:
        return f"`{g}` is updated under `{a} == {r}` but `{t}` is not read just before it {where}"
    m = re.fullmatch(rf"\(\({re.escape(a)}=={re.escape(r)}\)&&\(perm\((?P<c>.+)\)>none\)\)\?(?P=c):null", rhs)
    if m is None:
        return (f"`{g}` is updated from `{t}`, whose nearest earlier assignment `{rhs}` is not `(({a} == {r}) && (perm(<cell>) > none)) ? "
                f"<cell> : null`: the guard and the permission read belong to the update {where}")
    lo = leaf_cell_owner(m.group("c"))
    if lo is None or g != leaf_ghost_name(lo[1], lo[2]):
        return f"`{g}` is updated from the cell `{m.group('c')}`, which is not the leaf its name spells {where}"
    if r != leaf_root_address(lo):
        return f"`{g}` is updated under `{a} == {r}`, which is not the address of the struct its leaf belongs to ({leaf_root_address(lo)}) {where}"
    return None


def program_start_shapes(src: Source, start: Decl | None) -> dict[str, set[tuple[tuple[int, ...], str]]]:
    """The struct globals of `program_start`'s rows, each as the set of its leaves (member path, field suffix): what a member path of an
    owning-leaf tag is pinned against.  A row that does not parse is `lint_globals`' problem, not read here."""
    if start is None:
        return {}
    fields = {m.group("suffix"): m.group("type") for l in src.code if (m := FIELD_DECL_RE.match(l))}
    shapes: dict[str, set[tuple[tuple[int, ...], str]]] = {}
    for _k, _ln, text in clause_texts(src, start):
        row = norm_viper(text)
        if row.startswith("acc(loc(member("):
            r = struct_row(row, fields)
            if not isinstance(r, str):
                shapes[r[0]] = {(c, sfx) for c, sfx, _t in r[1]}
    return shapes


def exemption_term_problem(ghost: tuple[str, tuple[int, ...] | None], loc_text: str) -> bool:
    """Whether `t == <loc_text>` is the exemption term of the ghost `ghost` = (block, member path or None for a pointer global): exactly the
    block of that owner's own cell and no other.  A deeper term (a chain under a static owner, record 0055 S4) is added here."""
    gb, path = ghost
    cell = f"loc({gb}(),0)" if not path else slot_text(gb, path)
    want = rf"loc\(block_of\({re.escape(cell)}\.v_ptr_[\w$']+\),\(?-1\)?\)"
    return re.fullmatch(want, _nospace(loc_text)) is not None


def ghost_cell_text(ghost: tuple[str, tuple[int, ...] | None]) -> str:
    """The normalized block cell of an owner ghost `(block, member path)`: `loc(GB_x(),0)` or the leaf's slot; `.v_ptr_T` reads it."""
    gb, path = ghost
    return f"loc({gb}(),0)" if not path else slot_text(gb, path)


def temp_entry_problem(src: Source, lineno: int, t: str, g: str) -> str | None:
    """`held_<o> := t != null` of an owning leaf (decision 0055 items 5 and 6): when the temp `t` was just assigned a leaf's cell, bare
    (an unguarded entry) or as `<guard> ? <cell> : null` (an entry under a by-value site's guard, or the gives-back after a callee whose
    `ensures` covers the leaf, where the guard is exactly `perm(<cell>) > none`), the cell is the cell the ghost's name spells, the
    else branch is `null`, and a guard that reads a permission reads that cell's.  Any other `t` (a stored value) is not read here."""
    rhs = None
    for n in range(lineno - 1, max(0, lineno - 40), -1):
        code = src.code[n - 1]
        if re.match(r"^(method|function)\b", code):
            break
        m = re.match(rf"^\s*{re.escape(t)}\s*:=", code)
        if m:
            rhs = _nospace(_statement_rhs(code, m.end()))
            break
    if rhs is None:
        return None
    guard, cell, other = None, rhs, "null"
    top = _split_top(rhs, "?")
    if len(top) == 2:
        branches = _split_top(top[1], ":")
        if len(branches) == 2:
            guard, cell, other = top[0], branches[0], branches[1]
    lo = leaf_cell_owner(cell)
    if lo is None:
        return None
    if g != leaf_ghost_name(lo[1], lo[2]):
        return f"`{g}` is set from the leaf cell `{cell}`, which is another leaf's: a leaf's ghost reads its own cell"
    if other != "null":
        return f"`{g}` is set from `{rhs}`, whose else branch is not `null`"
    if guard is not None and "perm(" in guard and re.fullmatch(rf"\(?perm\({re.escape(cell)}\)>none\)?", guard) is None:
        return f"`{g}` is set from `{rhs}`, whose guard reads a permission that is not `perm({cell}) > none`"
    return None


HP_TOKEN_RE = re.compile(r"(?<![\w$'.])hp_[\w$']+")
HP_ASSIGN_RE = re.compile(r"(?<![\w$'.])(?P<hp>hp_[\w$']+)\s*:=")
HP_VAR_RE = re.compile(r"^\s*var\s+hp_[\w$']+\s*:\s*Bool\s*$")
_HP_NAME_RE = re.compile(r"^hp_(?P<held>held_[\w$']+)__(?P<n>[0-9]+)$")
_CARRY_INV_RE = re.compile(r"^invariant\((?P<hp>hp_[\w$']+)==>\((?P<g>held_[\w$']+)==\((?P<c>.+?)!=null\)\)\)&&\(\(!(?P=hp)\)==>\(!(?P=g)\)\)$")
_CARRY_ROW_RE = re.compile(r"^if\(!\(\(\(perm\((?P<c>.+?)\)>none\)==>\((?P<g>held_[\w$']+)==\((?P=c)!=null\)\)\)&&"
                           r"\(\(!\(perm\((?P=c)\)>none\)\)==>\(!(?P=g)\)\)\)\)\{$")


def operands_of_nonnull(n: str) -> list[str]:
    """The left operand of each `!= null` of the normalized text `n`: the term back to the unmatched `(` or the nearest operator."""
    out = []
    for m in re.finditer(r"!=null", n):
        k, depth = m.start() - 1, 0
        while k >= 0:
            ch = n[k]
            if ch == ")":
                depth += 1
            elif ch == "(":
                if depth == 0:
                    break
                depth -= 1
            elif depth == 0 and ch in ",&|=!<>":
                break
            k -= 1
        out.append(n[k + 1:m.start()])
    return out


def lint_leaf_carry(src: Source, decls: list[Decl], ghosts: dict[str, tuple[str, tuple[int, ...] | None]],
                    add: Callable[[int, str], None]) -> set[int]:
    """The loop carry of an owning leaf's ghost (decision 0055 item 7, `leaf_held_agrees`): at the loop's entry the snapshot `hp_<held>__<n> :=
    perm(<cell>) > none`, the invariant `(hp ==> (held == (<cell> != null))) && ((!hp) ==> (!held))` and its entry and back-edge rows
    `if (!(((perm(<cell>) > none) ==> (held == (<cell> != null))) && ((!(perm(<cell>) > none)) ==> (!held)))) {`.  `<cell>` is the leaf's own
    cell, the one the ghost's name spells (`leaf_ghost_name`; a global leaf's ghost is a tagged one), in all of them; the snapshot of a loop is
    assigned once, before the invariant that reads it, and is read nowhere else.  Returns the lines it found well formed, which the older
    comparison rules (`held == (<cell> != null)`) then leave alone."""
    carry: set[int] = set()
    methods = [d for d in decls if d.kind == "method"]
    snaps: dict[int, dict[str, int]] = {}

    def cell_ok(g: str, c: str) -> bool:
        lo = leaf_cell_owner(c)
        return lo is not None and g == leaf_ghost_name(lo[1], lo[2]) and (lo[0] == "local" or g in ghosts)

    for lineno, code in enumerate(src.code, start=1):
        d = next((m for m in methods if m.line <= lineno <= m.end_line), None)
        n = _nospace(code)
        if (a := HP_ASSIGN_RE.search(code)) is not None:
            hp = a.group("hp")
            mine = snaps.setdefault(d.line if d else -1, {})
            rhs = _nospace(_statement_rhs(code, a.end()))
            nm = _HP_NAME_RE.match(hp)
            pm = re.fullmatch(r"perm\((?P<c>.+)\)>none", rhs)
            if hp in mine:
                add(lineno, f"`{hp}` is assigned twice: a loop's snapshot is assigned once, at its entry (decision 0055 items 7 and 9)")
            elif nm is None or pm is None or not cell_ok(nm.group("held"), pm.group("c")):
                add(lineno, f"`{hp}` is assigned `{rhs}`: it is `perm(<cell>) > none` of the very cell its ghost's name spells, the leaf's own "
                            "(decision 0055 items 7 and 9)")
            else:
                mine[hp] = lineno
            continue
        if (m := _CARRY_ROW_RE.match(n)) is not None:
            if cell_ok(m.group("g"), m.group("c")):
                carry.add(lineno)
            continue
        if (m := _CARRY_INV_RE.match(n)) is not None:
            nm = _HP_NAME_RE.match(m.group("hp"))
            if (nm is not None and nm.group("held") == m.group("g") and cell_ok(m.group("g"), m.group("c"))
                    and snaps.get(d.line if d else -1, {}).get(m.group("hp"), lineno) < lineno):
                carry.add(lineno)
                continue
        if HP_TOKEN_RE.search(code) and not HP_VAR_RE.match(code):
            add(lineno, "an `hp_` snapshot is read only in the loop carry of its own leaf ghost, `(hp ==> (held == (<its cell> != null))) && "
                        "((!hp) ==> (!held))`, after its assignment (decision 0055 items 7 and 9)")
    return carry


HSL_TOKEN_RE = re.compile(r"(?<![\w$'.])hsl_[\w$']+")
HSL_ASSIGN_RE = re.compile(r"(?<![\w$'.])(?P<g>hsl_[\w$']+)\s*:=")
HSL_VAR_RE = re.compile(r"^\s*var\s+(?P<g>hsl_[\w$']+)\s*:\s*(?P<type>\w+)\s*$")
_HSL_NAME_RE = re.compile(r"^hsl_(?P<held>held_[\w$']+)__(?P<n>[0-9]+)$")


def lint_leaf_entry_snapshot(src: Source, decls: list[Decl], ghosts: dict[str, tuple[str, tuple[int, ...] | None]],
                             add: Callable[[int, str], None]) -> set[int]:
    """The entry snapshot `hsl_<held>__<n>` of a leaf `main` reaches only by type (decision 0056 item 5): a `Bool` local, assigned once,
    before the `while` of loop `<n>` of its method, as `<held> && (perm(<cell>) > none)` where `<held>` is the leaf's ghost and `<cell>` the
    very cell its name spells (the leaf's own, `leaf_ghost_name`; a global leaf's ghost is a tagged one).  It is read only in
      * the pair `invariant (hsl ==> (held == (<cell> != null))) && ((!hsl) ==> (!held))` and its entry and back-edge rows
        `if (!((hsl ==> (held == (<cell> != null))) && ((!hsl) ==> (!held)))) {` (`<held>` and `<cell>` the snapshot's own); and
      * as the first conjunct of a guard (`hsl && ...`, never negated or alone in another formula), the site's guard that `main` carries
        the leaf under.
    Returns the lines of the pair, which the older comparison rules (`held == (<cell> != null)`) then leave alone."""
    pair: set[int] = set()
    methods = [d for d in decls if d.kind == "method" and d.has_body]

    def cell_ok(g: str, c: str) -> bool:
        lo = leaf_cell_owner(c)
        return lo is not None and g == leaf_ghost_name(lo[1], lo[2]) and (lo[0] == "local" or g in ghosts)

    for d in methods:
        names: dict[str, int] = {}
        for n in range(d.body_line, d.end_line + 1):
            if (m := HSL_VAR_RE.match(src.code[n - 1])) is not None:
                names[m.group("g")] = n
                if m.group("type") != "Bool":
                    add(n, f"`{m.group('g')}` is a `{m.group('type')}` local: an entry snapshot is `Bool` (decision 0056 item 5)")
        if not names and not any(HSL_TOKEN_RE.search(src.code[n - 1]) for n in range(d.body_line, d.end_line + 1)):
            continue
        loops = while_loops(src, d)
        assigned: dict[str, int] = {}
        seen_assign: set[str] = set()
        for n in range(d.body_line, d.end_line + 1):
            code = src.code[n - 1]
            for a in HSL_ASSIGN_RE.finditer(code):
                g = a.group("g")
                seen_assign.add(g)
                rhs = _nospace(_statement_rhs(code, a.end()))
                nm = _HSL_NAME_RE.match(g)
                m = re.fullmatch(r"(?P<h>held_[\w$']+)&&\(perm\((?P<c>.+)\)>none\)", rhs)
                loop_no = int(nm.group("n")) if nm else -1
                if g in assigned:
                    add(n, f"`{g}` is assigned twice: an entry snapshot is assigned once, before its loop (decision 0056 item 5)")
                elif g not in names:
                    add(n, f"`{g}` is assigned but not declared as a `Bool` local of this method (decision 0056 item 5)")
                elif nm is None or m is None or m.group("h") != nm.group("held") or not cell_ok(m.group("h"), m.group("c")):
                    add(n, f"`{g}` is assigned `{rhs}`: it is `<held> && (perm(<cell>) > none)` of the leaf its name spells, `held` and the "
                           "cell that leaf's own (decision 0056 item 5)")
                elif loop_no >= len(loops) or n >= loops[loop_no][0]:
                    add(n, f"`{g}` is assigned at or after the `while` of its own loop (or names a loop the method does not have): the loop "
                           "head would havoc it, and it would no longer say anything about the entry (decision 0056 item 5)")
                else:
                    assigned[g] = n
        for g in names:
            if g not in seen_assign:
                add(names[g], f"`{g}` is declared but never assigned: an entry snapshot is assigned once, before its loop (decision 0056 item 5)")
        for n in range(d.body_line, d.end_line + 1):
            code = src.code[n - 1]
            toks = list(HSL_TOKEN_RE.finditer(code))
            if not toks or HSL_VAR_RE.match(code) or (HSL_ASSIGN_RE.match(code.strip()) and len(toks) == 1):
                continue
            norm = _nospace(code)
            for g in sorted({m.group() for m in toks}):
                nm = _HSL_NAME_RE.match(g)
                if nm is None:
                    add(n, f"`{g}` is not named `hsl_<held ghost>__<loop>` (decision 0056 item 5)")
                    continue
                held = nm.group("held")
                body = rf"\({re.escape(g)}==>\({re.escape(held)}==\((?P<c>.+?)!=null\)\)\)&&\(\(!{re.escape(g)}\)==>\(!{re.escape(held)}\)\)"
                shapes = (rf"invariant{body}", rf"if\(!\({body}\)\)\{{")
                hit = next((m for s in shapes if (m := re.fullmatch(s, norm)) is not None), None)
                if hit is not None:
                    if not cell_ok(held, hit.group("c")):
                        add(n, f"`{g}` is compared with `{hit.group('c')} != null`, which is not the cell of the leaf `{held}` spells "
                               "(decision 0056 item 5)")
                    elif g not in assigned or assigned[g] >= n:
                        add(n, f"`{g}` is read before its assignment (decision 0056 item 5)")
                    else:
                        pair.add(n)
                    continue
                # a guard: every occurrence is the first conjunct of a `&&`, after `(`, `:=` or the start of the formula
                for m in re.finditer(re.escape(g), norm):
                    before, after = norm[:m.start()], norm[m.end():]
                    if not (after.startswith("&&") and (before.endswith("(") or before.endswith(":=") or before.endswith("invariant")
                                                        or before.endswith("assert") or before == "")):
                        add(n, f"`{g}` appears other than as the first conjunct of a guard `{g} && ...` or in its pair `({g} ==> ({held} == "
                               f"(<cell> != null))) && ((!{g}) ==> (!{held}))` (decision 0056 item 5)")
                        break
    return pair


EXM_TOKEN_RE = re.compile(r"(?<![\w$'.])exm_(?P<n>[0-9]+)(?![\w$'])")
EXM_ASSIGN_RE = re.compile(r"(?<![\w$'.])exm_(?P<n>[0-9]+)\s*:=")
EXM_VAR_RE = re.compile(r"^\s*var\s+exm_[0-9]+\s*:\s*Bool\s*$")
_EXM_LEAF_RE = re.compile(r"^(?P<g>held_[\w$']+)&&\(perm\((?P<n>.+)\)>none\)$")
_EXM_SHAPE_RE = re.compile(r"^\((?P<g>[\w$']+)&&\((?P<x>.+?)!=null\)\)&&\(perm\((?P<n>.+)\)>none\)$")


def lint_exemption_ghosts(src: Source, decls: list[Decl], ghosts: dict[str, tuple[str, tuple[int, ...] | None]],
                          add: Callable[[int, str], None]) -> dict[int, dict[int, tuple[int, str]]]:
    """`main`'s deeper exemption (decision 0055 S4): the Bool locals `exm_<n>`, each assigned exactly once, in a method, as
    `(<g> && (<X> != null)) && (perm(<N>) > none)` where `<g>` is the `held_` ghost of a TAGGED owner and `<X>` that owner's own cell, or
    an earlier `exm_<m>` and `<X>` the cell `<N>` of that one, and `<N>` is `loc(member(<X>, k), 0).v_T`: the cell of a member of the block
    `<X>` points to.  Returns, per method (the line of its keyword), each `exm_<n>` as (its line, its cell `<N>`) for the `forperm` rows."""
    info: dict[int, dict[int, tuple[int, str]]] = {}
    methods = [d for d in decls if d.kind == "method"]

    def method_of(lineno: int) -> Decl | None:
        return next((d for d in methods if d.line <= lineno <= d.end_line), None)

    for lineno, code in enumerate(src.code, start=1):
        if not EXM_TOKEN_RE.search(code):
            continue
        d = method_of(lineno)
        a = EXM_ASSIGN_RE.search(code)
        if d is None:
            add(lineno, "an `exm_<n>` ghost stands only inside a method: it frames `main`'s exemption of a static owner's deeper blocks (decision 0055 item 9)")
        elif EXM_VAR_RE.match(code):
            continue
        elif a is not None:
            n = int(a.group("n"))
            mine = info.setdefault(d.line, {})
            rhs = _nospace(_statement_rhs(code, a.end()))
            if n in mine:
                add(lineno, f"`exm_{n}` is assigned twice: each exemption ghost is assigned exactly once (decision 0055 item 9)")
                continue
            m = _EXM_SHAPE_RE.match(rhs)
            problem = None
            nxt = ""
            if (lm := _EXM_LEAF_RE.match(rhs)) is not None:
                # a leaf's own block: `held_<leaf> && (perm(<that leaf's cell>) > none)`, the cell the next level is rooted at
                g, nxt = lm.group("g"), lm.group("n")
                if g not in ghosts or not ghosts[g][1]:
                    problem = f"`{g}` is not the ghost of an owning leaf tagged `// cerastes:owning-global GB_x/<k>...`"
                elif re.fullmatch(re.escape(ghost_cell_text(ghosts[g])) + r"\.v_ptr_[\w$']+", nxt) is None:
                    problem = f"`{g}` reads the permission to its own leaf's cell, `{ghost_cell_text(ghosts[g])}.v_ptr_T`, and no other"
            elif m is None:
                problem = "it is `(<held ghost or earlier exm> && (<cell> != null)) && (perm(<member cell>) > none)`"
            else:
                g, x, nxt = m.group("g"), m.group("x"), m.group("n")
                if g in ghosts:
                    if re.fullmatch(re.escape(ghost_cell_text(ghosts[g])) + r"\.v_ptr_[\w$']+", x) is None:
                        problem = f"`{g}` guards the cell of its own owner, `{ghost_cell_text(ghosts[g])}.v_ptr_T`, and no other"
                elif (gm := re.fullmatch(r"exm_([0-9]+)", g)) is not None:
                    prev = mine.get(int(gm.group(1)))
                    if prev is None:
                        problem = f"`{g}` is not assigned before it (rooting an exemption through an unassigned ghost)"
                    elif x != prev[1]:
                        problem = f"`{g}` guards the cell `{prev[1]}` that it read, and no other"
                else:
                    problem = f"`{g}` is not the ghost of an owner tagged `// cerastes:owning-global` nor an earlier `exm_` ghost"
                if problem is None and re.fullmatch(r"loc\(member\(" + re.escape(x) + r",[0-9]+\),0\)\.v_[\w$']+", nxt) is None:
                    problem = "the cell it reads is `loc(member(<X>, k), 0).v_T`, a member of the block `<X>` points to"
            if problem:
                add(lineno, f"`exm_{n}` is assigned `{rhs}`: {problem} (decision 0055 item 9)")
            mine[n] = (lineno, nxt)
        elif not FORPERM_LINE_RE.match(code):
            add(lineno, "an `exm_<n>` ghost is read only in the `forperm` row of the exemption it frames (decision 0055 item 9)")
    return info


def lint_owning_global_tags(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    declared: dict[str, int] = {}
    for code in src.code:
        if m := GB_DECL_RE.match(code):
            declared[m.group("name")] = declared.get(m.group("name"), 0) + 1
    start = next((d for d in decls if d.kind == "method" and d.name == PROGRAM_START), None)
    first_clause = last_clause = None   # `program_start` is a bodyless trusted method: its head, then its clauses
    if start is not None:
        clauses = [n for n in range(start.line + 1, start.end_line + 1) if re.match(r"^\s*(requires|ensures|decreases)\b", src.code[n - 1])]
        first_clause, last_clause = (clauses[0], clauses[-1]) if clauses else (None, start.line)
    shapes = program_start_shapes(src, start)
    tagged: set[str] = set()                              # owning pointer globals
    tagged_leaves: set[tuple[str, tuple[int, ...]]] = set()   # owning leaves of struct globals
    for lineno, comment in enumerate(src.comments, start=1):
        m = OWNING_GLOBAL_TAG_RE.search(comment) if comment else None
        if m is None:
            continue
        name = m.group("name")
        gb, *rest = name.split("/") if name else [None]
        path = tuple(int(k) for k in rest) if all(_PATH_INDEX_RE.fullmatch(k) for k in rest) else None
        if start is None or not start.line <= lineno <= last_clause:
            add(lineno, "`// cerastes:owning-global` stands only inside `method program_start`: it names an owning global whose block "
                        "`main`'s leak row exempts (decision 0054 item 8)")
        elif name is None or declared.get(gb, 0) != 1:
            add(lineno, f"`// cerastes:owning-global {name or ''}` names no single `unique function GB_...` declared in this file "
                        "(decision 0054 item 8)")
        elif path is None or (rest and not path):
            add(lineno, f"`// cerastes:owning-global {name}`: the member path is `/<k>[/<k>...]`, each `k` a decimal index (decision 0055 item 9)")
        elif path and not any(c == path and sfx.startswith("ptr_") for c, sfx in shapes.get(gb, ())):
            add(lineno, f"`// cerastes:owning-global {name}`: `{path}` is not a pointer leaf of the struct global `{gb}` in `program_start`'s "
                        "rows: the member path names the owning field's leaf (decision 0055 item 9)")
        elif not path and gb in shapes:
            add(lineno, f"`// cerastes:owning-global {name}` tags a struct global without a member path: write `{gb}/<k>` for each owning "
                        "leaf (decision 0055 item 9)")
        elif (gb, path) in tagged_leaves or (not path and gb in tagged):
            add(lineno, f"`{name}` is tagged `// cerastes:owning-global` twice (decision 0054 item 8)")
        elif first_clause is not None and lineno > first_clause:
            add(lineno, "`// cerastes:owning-global` stands right after the head of `method program_start`, before its first clause "
                        "(decision 0054 item 8)")
        elif path:
            tagged_leaves.add((gb, path))
        else:
            tagged.add(gb)
    ghosts: dict[str, tuple[str, tuple[int, ...] | None]] = {}
    for gb in tagged:
        ghosts["held_" + gb] = (gb, None)
    for gb, path in tagged_leaves:
        ghosts.setdefault(leaf_ghost_name(gb, path), (gb, path))
    tagged_blocks = tagged | {gb for gb, _p in tagged_leaves}
    carry_lines = lint_leaf_carry(src, decls, ghosts, add) | lint_leaf_entry_snapshot(src, decls, ghosts, add)
    for lineno, code in enumerate(src.code, start=1):
        for gm in sorted({m.group("ghost") for m in GHOST_ANY_RE.finditer(code)}):
            if gm not in ghosts:
                if gm[len("held_"):] in declared:
                    add(lineno, f"the ownership ghost `{gm}` of `{gm[len('held_'):]}` in a program whose `program_start` does not tag "
                                f"`// cerastes:owning-global {gm[len('held_'):]}`: only an owning global has one (decision 0054 item 5)")
                else:
                    add(lineno, f"the ownership ghost `{gm}` in a program whose `program_start` tags no `// cerastes:owning-global GB_x/<k>...` "
                                "that spells it: only a tagged owning global or an owning leaf of a tagged struct global has one "
                                "(decision 0055 item 9)")
                break
        for a in OWNER_ASSIGN_RE.finditer(code):
            rhs = _statement_rhs(code, a.end())
            tm = re.fullmatch(rf"({_ID})!=null", _nospace(rhs)) if a.group("g").startswith("held_") else None
            if tm and owner_assignment_ok(a.group("g"), rhs) and (why := temp_entry_problem(src, lineno, tm.group(1), a.group("g"))):
                add(lineno, f"{why} (decision 0055 items 5 and 6, item 9)")
            guarded = None if owner_assignment_ok(a.group("g"), rhs) else guarded_update_form(a.group("g"), rhs)
            if guarded is not None:
                if (why := guarded_update_problem(src, lineno, a.group("g"), guarded)):
                    add(lineno, why)
            elif not owner_assignment_ok(a.group("g"), rhs):
                add(lineno, f"`{a.group('g')}` is assigned `{rhs}`: an owner's ghost is assigned `false`, `<x> != null`, `held && ...` "
                            "(the update after a call or a free, which reads the token's permission), `held || ...` (a block received "
                            "through an address) or, for `tv_`, the conditional read of the token (decision 0054 items 5 and 6); an owning "
                            "leaf's ghost names its own leaf's cell (decision 0055 item 9)")
        for gm in sorted({m.group("ghost") for m in re.finditer(r"(?<![\w$'.])(?P<ghost>held_GB_[\w$']*)\s*==(?!>)", code)} if lineno not in carry_lines else set()):
            if gm not in ghosts:
                continue
            gb, path = ghosts[gm]
            cell_text = f"loc({gb}(),0)" if not path else slot_text(gb, path)
            cell = rf"{re.escape(cell_text)}\.v_ptr_[\w$']+"
            n = _nospace(code)
            if not (re.fullmatch(rf"invariant{re.escape(gm)}==\({cell}!=null\)", n)
                    or re.fullmatch(rf"if\(!\({re.escape(gm)}==\({cell}!=null\)\)\)\{{", n)):
                add(lineno, f"`{gm}` is compared with something but `({cell_text.replace(',', ', ')}.v_ptr_T != null)`, in the loop invariant "
                            "or its row (decision 0054 item 5)")
    ref_vars = {m.group(1) for l in src.code if (m := re.match(r"^\s*var\s+([\w$']+)\s*:\s*Ref\b", l))}
    exm_info = lint_exemption_ghosts(src, decls, ghosts, add)
    for lineno, code in enumerate(src.code, start=1):
        for gm in sorted({m.group("ghost") for m in LOCAL_GHOST_COMPARE_RE.finditer(code)} if lineno not in carry_lines else set()):
            n = _nospace(code)
            leafish = any(re.fullmatch(rf"held_{re.escape(v)}(?:_[0-9]+)+", gm) for v in ref_vars) or any(
                leaf_cell_owner(c) is not None for c in operands_of_nonnull(n))
            if not leafish:
                continue
            ok = False
            for shape in (rf"invariant{re.escape(gm)}==\((?P<c>.+)!=null\)", rf"if\(!\({re.escape(gm)}==\((?P<c>.+)!=null\)\)\)\{{"):
                if (m := re.fullmatch(shape, n)) is not None:
                    c = m.group("c")
                    lo = leaf_cell_owner(c)
                    ok = (re.fullmatch(_ID, c) is not None and gm == "held_" + c) or (lo is not None and gm == leaf_ghost_name(lo[1], lo[2]))
            if not ok:
                add(lineno, f"`{gm}` is compared with something but `(<its own cell> != null)`, in the loop invariant or its entry and back-edge "
                            "rows: an owning local's own value, or an owning leaf's own cell, the one its name spells (decision 0055 items 7 and 9)")
    for lineno, code in enumerate(src.code, start=1):
        m = FORPERM_LINE_RE.match(code)
        if m is None:
            continue
        body = _strip_wrap(m.group("body").strip())
        enclosing = next((d for d in decls if d.kind == "method" and d.line <= lineno <= d.end_line), None)
        for disjunct in _flatten_or(body):
            conjuncts = _and_leaves(disjunct)  # decision 0059 item 10 nests `held && offset_of(p) == 0` as one operand of the term
            helds = [c for c in conjuncts if re.fullmatch(r"held_[\w$']*", c)]
            for e in (c for c in conjuncts if re.fullmatch(r"exm_[0-9]+", c)):
                got = exm_info.get(enclosing.line if enclosing else -1, {}).get(int(e[len("exm_"):]))
                tt = next((re.match(r"^t\s*==\s*(?P<loc>loc\(.*\))$", c, re.S) for c in conjuncts if c.startswith("t")), None)
                if got is None or got[0] > lineno:
                    add(lineno, f"`{e}` is read by this exemption row but not assigned before it in this method (decision 0055 item 9)")
                elif tt is None or re.fullmatch(rf"loc\(block_of\({re.escape(got[1])}\),\(?-1\)?\)", _nospace(tt.group("loc"))) is None:
                    add(lineno, f"the exemption term guarded by `{e}` is exactly `t == loc(block_of({got[1]}), -1)`: the block of the cell `{e}` "
                                "read, one member further than its guard (decision 0055 item 9)")
                elif not has_offset_zero(conjuncts, got[1]):
                    add(lineno, f"the exemption term guarded by `{e}` holds `offset_of({got[1]}) == 0`: the cell `{e}` read must start its block "
                                "(decision 0059 item 10)")
            if not helds:
                continue
            for c in conjuncts:
                t = re.match(r"^t\s*==\s*(?P<loc>loc\(.*\))$", c, re.S)
                if t is None:
                    continue
                gbs = {g.group("name") for g in GB_CALL_RE.finditer(t.group("loc"))}
                for g in sorted(gbs - tagged_blocks):
                    add(lineno, f"an exemption term `held_... && t == loc(block_of(...{g}()...), -1)` names the cell of `{g}`, which "
                                f"`program_start` does not tag `// cerastes:owning-global {g}`: only an owning global's block is exempt "
                                "at `main`'s end (decision 0054 item 8)")
                pointer = token_term_pointer(t.group("loc"))
                if pointer is not None and not has_offset_zero(conjuncts, pointer):
                    add(lineno, f"the exemption term `held_... && t == loc(block_of({pointer}), -1)` holds `offset_of({pointer}) == 0`: a held owner "
                                "exempts the block its pointer starts, never one it points into (decision 0059 item 10)")
                for h in helds:
                    if h not in ghosts:
                        continue
                    if not exemption_term_problem(ghosts[h], t.group("loc")):
                        gb, path = ghosts[h]
                        own = f"loc({gb}(), 0)" if not path else slot_text(gb, path).replace(",", ", ")
                        add(lineno, f"the exemption term guarded by `{h}` is exactly `t == loc(block_of({own}.v_ptr_T), "
                                    "-1)`: a held owning global exempts its own block and no other (decision 0054 item 8)")


# The heap's free token (decision 0047 items 6, 7 and 10).  `field tok: Bool` is the ghost token of a heap block at slot -1: held at
# `write` and true by whoever owns the allocation.  It is created by a view's postcondition (a bodyless trusted method) or a
# function's `ensures`, consumed by a call whose `requires` has the block site (`free`), and read by the leak rows `forperm`.  Nothing
# in a method body may make or take one, so the lint refuses every statement form that could: an `exhale`, an assignment, an `inhale`,
# a condition.  The text of the view methods themselves is pinned by `lint_views` (above).
TOK_WORD_RE = re.compile(r"(?<![\w$'])tok(?![\w$'])")
FORPERM_WORD_RE = re.compile(r"(?<![\w$'])forperm(?![\w$'])")
FORPERM_LINE_RE = re.compile(r"^\s*assert\s+forperm\s+(?P<v>[A-Za-z_$][\w$']*)\s*:\s*Ref\s*\[\s*(?P=v)\.tok\s*\]\s*::\s*(?P<body>\S.*)$")
TOKEN_SLOT_RE = re.compile(r"loc\(\s*block_of\(.*\)\s*,\s*\(?\s*-\s*1\s*\)?\s*\)$", re.S)
TOK_FIELD_DECL = "field tok: Bool"
TOK_STATEMENT_RE = re.compile(r"^\s*(?P<kw>assert|requires|ensures|invariant)\b")
# The loop value snapshot of decision 0050 item 5: `ktv_<n> := loc(block_of(...), -1).tok`, taken when the loop's `kt_<n> > none`
# (one line `if (kt_<n> > none) { ktv_<n> := ... }`, or the assignment alone on a line right below `if (kt_<n> > none) {`).  It is the
# only read of a token's value that is stored: a local `Bool` that the loop's invariant and back-edge row compare with the cell.
KTV_ASSIGN = r"(?P<v>ktv_[\w$']*)\s*:=\s*(?P<slot>loc\(.*\))\.tok"
KTV_IF = r"if\s*\(\s*(?P<g>kt(?:_[\w$']*)?)\s*>\s*none\s*\)\s*\{"
KTV_ONE_LINE_RE = re.compile(rf"^\s*{KTV_IF}\s*{KTV_ASSIGN}\s*\}}\s*$", re.S)
KTV_ALONE_RE = re.compile(rf"^\s*{KTV_ASSIGN}\s*$", re.S)
KTV_IF_LINE_RE = re.compile(rf"^\s*{KTV_IF}\s*$")
# The ending rows of a heap place's back edge (decision 0047 item 10 as amended at M4.5d2v): `if (!(<row>)) {`, then `assert false`, then
# `}` stands for `assert <row>` whose failure ends the path (so the rows after it assume it).  The row is read like an assert's
# conjuncts (a `tok` in it only as `loc(block_of(...), -1).tok`); an `if` with anything else in its body, or a condition that is not
# the negation of one parenthesised row, is an ordinary `if` and may not read `tok`.
ENDING_IF_RE = re.compile(r"^\s*if\s*\(\s*!\s*(?P<row>\(.*\))\s*\)\s*\{\s*$", re.S)


# The owner's token value (decision 0052 item 3, 0050 item 5): `tv_<v> := (<x> != null) ? loc(block_of(<x>), -1).tok : tv_<v>`, taken
# at the initialization or assignment of an owning local right after its value row (which checks the whole block is held where `<x>`
# is not null).  It is the second read of a token's value that is stored: a method-level `Bool` that only the owner's ending rows and
# the loop's place compare with the cell.  Only this exact shape: the same ghost on both sides, the same pointer in the guard and the slot.
TV_ASSIGN_RE = re.compile(r"^\s*(?P<v>tv_[\w$']*)\s*:=\s*\(\s*(?P<x>[A-Za-z_$][\w$']*)\s*!=\s*null\s*\)\s*\?\s*"
                          r"loc\(\s*block_of\(\s*(?P=x)\s*\)\s*,\s*\(?\s*-\s*1\s*\)?\s*\)\.tok\s*:\s*(?P=v)\s*$")


def is_ending_row(src: Source, lineno: int) -> bool:
    """The line `lineno` (1-based) is `if (!(<row>)) {` and the next two lines are exactly `assert false` and `}`."""
    m = ENDING_IF_RE.match(src.code[lineno - 1])
    if m is None or _group_end(m.group("row"), 0) != len(m.group("row")) - 1:
        return False
    rest = [c.strip() for c in src.code[lineno:lineno + 2]]
    return rest == ["assert false", "}"]


def strip_perm_calls(text: str) -> str:
    """`text` without its `perm(...)` calls (balanced), for a snapshot's right-hand side."""
    out = []
    i = 0
    while i < len(text):
        m = PERM_CALL_RE.match(text, i)
        if m:
            depth = 1
            j = m.end()
            while j < len(text) and depth:
                depth += (text[j] == "(") - (text[j] == ")")
                j += 1
            out.append(" ")
            i = j
            continue
        out.append(text[i])
        i += 1
    return "".join(out)


# The typed views of the allocation functions (decision 0047 items 2, 4 and 7).  A view is a bodyless trusted method, so its contract is
# an assumption (invariant 4): it is pinned as `alloc_<t>` is, by rebuilding the expected clauses from the type's leaves, which are read
# from the text itself (a leaf is an `acc(C.v_t)` and `acc(C.d_t)` pair in member order; the member blocks and slot facts of a
# structure are checked with `struct_wf_lens_problem`).  Whatever is added to, missing from or reordered in what the rebuilt text says
# (a `true`-ed flag on a `malloc` cell, `ensures false`, a second token, a wider range) makes the two texts differ.
# Record 0050 item 9 (M4.5d2v): a view of vlibc's own `vlibc_malloc`, `vlibc_calloc` or `vlibc_free` has the same text with the
# token's value `false` (`!loc(block_of(...), -1).tok`): the block belongs to vlibc's heap (item 3), not the platform's.
VIEW_TAG_RE = re.compile(r"vlibc:\s+(?P<routine>(?:vlibc_)?(?:malloc|calloc|free))\b[^\n]*;\s*typed view:")
VIEW_NAME_RE = re.compile(r"^f_(?P<routine>(?:vlibc_)?(?:malloc|calloc|free))__[0-9]+$")
VIEW_INT_RANGES = {
    "schar": (-128, 127), "uchar": (0, 255), "short": (-32768, 32767), "ushort": (0, 65535),
    "int": (-2**31, 2**31 - 1), "uint": (0, 2**32 - 1), "long": (-2**63, 2**63 - 1), "ulong": (0, 2**64 - 1),
}
VIEW_STANDARD_RANGES = {(-(2**(n - 1)), 2**(n - 1) - 1) for n in (8, 16, 32, 64, 128)} | {(0, 2**n - 1) for n in (8, 16, 32, 64, 128)}


def _group_end(text: str, start: int) -> int:
    """The index of the `)` matching the `(` at `start`, or -1."""
    depth = 0
    for i in range(start, len(text)):
        if text[i] == "(":
            depth += 1
        elif text[i] == ")":
            depth -= 1
            if depth == 0:
                return i
    return -1


def _strip_wrap(text: str) -> str:
    text = text.strip()
    while text.startswith("(") and _group_end(text, 0) == len(text) - 1:
        text = text[1:-1].strip()
    return text


def _split_top(text: str, op: str) -> list[str]:
    parts, depth, start, i = [], 0, 0, 0
    while i < len(text):
        c = text[i]
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
        elif depth == 0 and text.startswith(op, i):
            parts.append(text[start:i])
            start = i + len(op)
            i += len(op)
            continue
        i += 1
    parts.append(text[start:])
    return parts


def _flatten_and(text: str) -> list[str]:
    """The conjuncts of a fully parenthesized `&&` chain, as raw text (an operand with `||` or `==>` stays whole)."""
    text = _strip_wrap(text)
    parts = _split_top(text, "&&")
    if len(parts) == 1:
        return [text]
    if len(_split_top(text, "||")) > 1 or len(_split_top(text, "==>")) > 1:
        raise ValueError("an unparenthesized mix of `&&` with `||` or `==>`")
    out: list[str] = []
    for part in parts:
        out += _flatten_and(part)
    return out


def _view_wf(x: str) -> str:
    return f"({x} == null) || ((({x} == loc(block_of({x}), offset_of({x}))) && (0 <= offset_of({x}))) && (offset_of({x}) <= len(block_of({x}))))"


def _chain_cell(chain: tuple[int, ...], root: str) -> str:
    cell = root
    for k in chain:
        cell = f"loc(member({cell}, {k}), 0)"
    return cell


def _lens_members(parts: list[str], root: str, counts: dict[tuple[int, ...], int] | None = None) -> tuple[list[tuple[int, ...]], str | None]:
    why = struct_wf_lens_problem(parts, root, counts)
    if why is not None:
        return [], why
    keys = []
    for part in parts:
        m = re.fullmatch(r"len\((?P<m>.*)\)==[0-9]+", part)
        if m:
            keys.append(parse_struct_member(m.group("m"), root))
    return keys, None


def _leaf_kind(t: str) -> str:
    if t == "bool":
        return "bool"
    if t.startswith("ptr_"):
        return "ptr"
    if t.startswith("enum_"):
        return "enum"
    return "int"


def view_problem(src: Source, d: Decl, routine: str, declared: dict[str, set[str]], has_globals: bool = False,
                 global_facts: list[str] | None = None, vlibc: bool = False) -> str | None:
    """The reason why the view method `d` is not the text decision 0047 gives its allocation type, or None.  In a program with
    globals every method takes the global facts as its first `requires` (decision 0041 item 4), views included: exactly one such
    leading `requires`, equal to the facts of the program (`global_facts`, from `program_start`), and no other.  `routine` is the
    base name (`malloc`, `calloc`, `free`); `vlibc` says the view is of the `vlibc_` routine, whose token conjunct is `!tok`
    (decision 0050 items 3 and 9)."""
    if d.has_body:
        return "it has a body"
    shown = ("vlibc_" if vlibc else "") + routine
    name_m = VIEW_NAME_RE.match(d.name)
    if name_m is None or name_m.group("routine") != shown:
        return f"its name is not `f_{shown}__<n>`"
    lines = [" ".join(src.code[n - 1].split()) for n in range(d.line, d.end_line + 1) if src.code[n - 1].strip()]
    root = "p_ptr__0" if routine == "free" else "res"
    keyword = "requires" if routine == "free" else "ensures"
    header = f"method {d.name}(p_ptr__0: Ref)" if routine == "free" else f"method {d.name}() returns (res: Ref)"
    if has_globals and len(lines) == 5 and lines[1].startswith("requires "):
        facts_line = lines[1][len("requires "):]
        if global_facts is not None and sorted(norm_viper(facts_line).split("&&")) != global_facts:
            return "its leading `requires` is not the global facts of the program (`len(GB_x()) == 1 && lvloc(GB_x()) == loc(GB_x(), -2)` for each global, as every method takes them)"
        lines = lines[:1] + lines[2:]
    elif has_globals:
        return "a view in a program with globals takes exactly one leading `requires`, the global facts of the program (decision 0041 item 4)"
    if len(lines) != 4 or lines[0] != header or lines[3] != "decreases _":
        return f"its header, its two `{keyword}` clauses and `decreases _` are not the emitter's"
    if not (lines[1].startswith(keyword + " ") and lines[2].startswith(keyword + " ")):
        return f"its two clauses are not both `{keyword}`"
    first, second = lines[1][len(keyword) + 1:], lines[2][len(keyword) + 1:]
    try:
        implies = _split_top(_strip_wrap(second), "==>")
        if len(implies) != 2 or norm_viper(implies[0]) != norm_viper(f"!({root} == null)"):
            return f"its second clause is not `!({root} == null) ==> <the block>`"
        raw = _flatten_and(implies[1])
    except ValueError as e:
        return str(e)
    atoms = [norm_viper(a) for a in raw]
    token_value = ("!" if vlibc else "") + f"loc(block_of({root}), (-1)).tok"
    prefix = [f"{root} != null", _view_wf(root), f"offset_of({root}) == 0", f"acc(loc(block_of({root}), (-1)).tok)",
              token_value, f"lvloc(block_of({root})) == loc(block_of({root}), (-2))",
              f"acc(lvloc(block_of({root})).lv)", f"len(block_of({root})) == 1"]
    if atoms[:len(prefix)] != [norm_viper(a) for a in prefix]:
        return ("the block part does not begin with the pointer, its well-formedness, offset 0, the token and its value "
                f"({'`!tok`: a block of vlibc\'s heap' if vlibc else '`tok`: a block of the platform\'s heap'}), the liveness equation "
                "and cell and `len == 1`")
    i = len(prefix)
    lens: list[str] = []
    while i < len(atoms) and not atoms[i].startswith(("acc(", "forall")):
        lens.append(atoms[i])
        i += 1
    members: list[tuple[int, ...]] = []
    counts: dict[tuple[int, ...], int] = {}
    if lens:
        # decision 0064 item 15 (heap storage): an array member states its count, `len(member(<chain>, k)) == N`; the rows of its leaf decide
        # whether the member is an array leaf (a cell has `len == 1` and `acc` lines)
        for part in lens:
            cm = re.fullmatch(r"len\((?P<m>.*)\)==(?P<n>[0-9]+)", part)
            ck = parse_struct_member(cm.group("m"), root) if cm else None
            if ck is not None and int(cm.group("n")) > 1:
                counts[ck] = int(cm.group("n"))
        members, why = _lens_members(lens, root, counts)
        if why is not None:
            return f"the structure's member blocks: {why}"
    structs = {k for k in members if any(o[:-1] == k for o in members)}
    for key in counts:
        if key in structs:
            return f"the member block {key} is a nested structure and states `len == {counts[key]}`: only an array member states a count other than 1"
    leaf_chains = sorted(k for k in members if k not in structs) if lens else [()]
    # the leaves, in the text: `acc(C.v_t)`, `acc(C.d_t)` and the facts under the flag
    leaves: list[tuple[tuple[int, ...], str, str]] = []  # chain, suffix, the rebuilt facts
    arrays: dict[tuple[int, ...], tuple[str, int, str, list[str]]] = {}  # array leaf: chain -> (suffix, N, the bound variable, its rows)
    # the leaves in the order `block_conj` writes them (decision 0064 item 15): the cell leaves in member order (value permission, flag
    # permission, the range under the flag), then (`calloc`: the cell flags), then the array leaves in member order (the three quantified rows
    # each), then (`calloc`: the array flags); an array leaf is one whose member states a count other than 1 or whose rows are quantified
    array_chains = [c for c in leaf_chains if lens and (c in counts or any(
        re.match(r"forall[A-Za-z_$][\w$']*:Int::\{loc\(" + re.escape(norm_viper(f"member({_chain_cell(c[:-1], root)}, {c[-1]})")) + ",", a) for a in atoms))]
    cell_chains = [c for c in leaf_chains if c not in array_chains]
    order = cell_chains + array_chains
    transitioned = False
    for chain in order:
        if chain in array_chains and not transitioned:
            transitioned = True
            if routine == "calloc":
                while i < len(atoms) and not atoms[i].startswith("forall"):
                    i += 1  # the cell flags: compared with the rebuilt text below
        if chain in array_chains:
            # an array leaf of scalars (decision 0064 item 15, question 292): three quantified rows over `[0, N)`, N the member's `len`, the
            # trigger the element's cell `loc(member(<parent>, k), j)`, one field suffix: the value permission, the flag permission and (but for
            # a `bool`) the type's range under the flag; each (receiver, field) pair once
            n = counts.get(chain, 1)
            blk = norm_viper(f"member({_chain_cell(chain[:-1], root)}, {chain[-1]})")
            rows_text: list[str] = []
            suffix = ""
            kvar = ""
            for off in range(3):
                if i + off >= len(atoms):
                    break
                am = re.fullmatch(r"forall(?P<k>[A-Za-z_$][\w$']*):Int::\{loc\(" + re.escape(blk) + r",(?P=k)\)\}0<=(?P=k)&&(?P=k)<(?P<hi>[0-9]+)==>(?P<cons>.+)",
                                  atoms[i + off])
                if am is None or am.group("hi") != str(n) or (off and am.group("k") != kvar):
                    return (f"array leaf {chain}: row {off + 1} is not `forall k: Int :: {{ loc({blk}, k) }} 0 <= k && k < {n} ==> ...` over the member's "
                            f"own block with the count `len({blk}) == {n}` the view states (value permission, flag permission, the type's range under the flag)")
                kvar = am.group("k")
                cell = f"loc({blk},{kvar})"
                if off == 0:
                    vm = re.fullmatch(r"acc\(" + re.escape(cell) + r"\.v_(?P<t>[A-Za-z0-9_$']+)\)", am.group("cons"))
                    if vm is None:
                        return f"array leaf {chain}: row 1 is not `acc({cell}.v_<t>)`"
                    suffix = vm.group("t")
                    if suffix not in declared["v"] or suffix not in declared["d"]:
                        return f"the field suffix `{suffix}` is not declared (`field v_{suffix}` and `field d_{suffix}`)"
                    if _leaf_kind(suffix) == "ptr" and suffix.startswith("ptr_struct_"):
                        return f"array leaf {chain}: an array of pointers to structures is not a pinned view shape"
                    rows_text.append(f"acc({cell}.v_{suffix})")
                elif off == 1:
                    if am.group("cons") != f"acc({cell}.d_{suffix})":
                        return f"array leaf {chain}: row 2 is not `acc({cell}.d_{suffix})`"
                    rows_text.append(f"acc({cell}.d_{suffix})")
                    if _leaf_kind(suffix) == "bool":
                        break
                elif _leaf_kind(suffix) == "ptr":
                    # an array of pointers to scalars (the three clauses of a cell's pointer leaf, over the elements): the well-formedness of the value under the flag
                    value, flag = f"{cell}.v_{suffix}", f"{cell}.d_{suffix}"
                    if norm_viper(am.group("cons")) != norm_viper(f"{flag} ==> ({_view_wf(value)})"):
                        return f"array leaf {chain}: row 3 is not `{flag} ==> WF({value})`"
                    rows_text.append(f"{flag} ==> ({_view_wf(value)})")
                else:
                    value, flag = f"{cell}.v_{suffix}", f"{cell}.d_{suffix}"
                    m2 = re.fullmatch(re.escape(flag) + r"==>(?P<lo>-?[0-9]+)<=" + re.escape(value) + "&&" + re.escape(value) + r"<=(?P<hi>-?[0-9]+)",
                                      am.group("cons"))
                    if m2 is None:
                        return f"array leaf {chain}: row 3 is not `{flag} ==> lo <= value && value <= hi`"
                    lo, hi = int(m2.group("lo")), int(m2.group("hi"))
                    kind = _leaf_kind(suffix)
                    ok = lo <= 0 <= hi if kind == "enum" else (lo, hi) == VIEW_INT_RANGES[suffix] if suffix in VIEW_INT_RANGES else (lo, hi) in VIEW_STANDARD_RANGES
                    if not ok:
                        return f"array leaf {chain}: the range [{lo}, {hi}] is not the range of its field `{suffix}` (decision 0018)"
                    rows_text.append(f"{flag} ==> (({lo}) <= {value} && {value} <= {hi})")
            need = 2 if _leaf_kind(suffix) == "bool" else 3
            if len(rows_text) != need:
                return f"array leaf {chain}: the rows of the leaf are cut short ({len(rows_text)} of {need})"
            i += need
            arrays[chain] = (suffix, n, kvar, [f"forall {kvar}: Int :: {{ loc({blk}, {kvar}) }} 0 <= {kvar} && {kvar} < {n} ==> ({t})" for t in rows_text])
            leaves.append((chain, suffix, "<array>"))
            continue
        cell = _chain_cell(chain, root)
        m = re.fullmatch(r"acc\((?P<c>.*)\.v_(?P<t>[A-Za-z0-9_$']+)\)", atoms[i]) if i < len(atoms) else None
        if m is None or m.group("c") != norm_viper(cell):
            return f"leaf {chain or 'res'}: `acc({cell}.v_<t>)` is expected here"
        t = m.group("t")
        if t not in declared["v"] or t not in declared["d"]:
            return f"the field suffix `{t}` is not declared (`field v_{t}` and `field d_{t}`)"
        if i + 1 >= len(atoms) or atoms[i + 1] != norm_viper(f"acc({cell}.d_{t})"):
            return f"leaf {chain or 'res'}: `acc({cell}.d_{t})` is expected after its value permission"
        i += 2
        kind = _leaf_kind(t)
        value, flag = f"{cell}.v_{t}", f"{cell}.d_{t}"
        if kind == "bool":
            facts = ""
        elif kind == "ptr":
            fact_raw = raw[i] if i < len(raw) else ""
            rhs = _split_top(_strip_wrap(fact_raw), "==>")
            if len(rhs) != 2 or norm_viper(rhs[0]) != norm_viper(flag):
                return f"leaf {chain or 'res'}: the pointer's facts are not `{flag} ==> ...`"
            sub = _flatten_and(rhs[1])
            if len(sub) == 1:
                facts = f"{flag} ==> ({_view_wf(value)})"
            elif len(sub) == 2:
                g = _split_top(_strip_wrap(sub[1]), "==>")
                if len(g) != 2:
                    return f"leaf {chain or 'res'}: the pointee's member blocks are not `guard ==> lens`"
                pointee = [norm_viper(a) for a in _flatten_and(g[1])]
                pcounts: dict[tuple[int, ...], int] = {}
                for part in pointee:  # the pointee's array member states its count (decision 0064 item 15), as the view's own do
                    pm = re.fullmatch(r"len\((?P<m>.*)\)==(?P<n>[0-9]+)", part)
                    pk = parse_struct_member(pm.group("m"), norm_viper(value)) if pm else None
                    if pk is not None and int(pm.group("n")) > 1:
                        pcounts[pk] = int(pm.group("n"))
                _members, why = _lens_members(pointee, norm_viper(value), pcounts)
                if why is not None:
                    return f"leaf {chain or 'res'}: the pointee's member blocks: {why}"
                facts = (f"{flag} ==> (({_view_wf(value)}) && (({value} != null && offset_of({value}) < len(block_of({value}))) "
                         f"==> ({' && '.join(pointee)})))")
            else:
                return f"leaf {chain or 'res'}: the pointer's facts are not its well-formedness and its pointee's member blocks"
            i += 1
        else:
            fact = atoms[i] if i < len(atoms) else ""
            m2 = re.fullmatch(re.escape(norm_viper(flag)) + r"==>(?P<lo>-?[0-9]+)<=" + re.escape(norm_viper(value)) + "&&"
                              + re.escape(norm_viper(value)) + r"<=(?P<hi>-?[0-9]+)", fact)
            if m2 is None:
                return f"leaf {chain or 'res'}: the facts under its flag are not `{flag} ==> lo <= value && value <= hi`"
            lo, hi = int(m2.group("lo")), int(m2.group("hi"))
            if kind == "enum":
                ok = lo <= 0 <= hi
            elif t in VIEW_INT_RANGES:
                ok = (lo, hi) == VIEW_INT_RANGES[t]
            else:
                ok = (lo, hi) in VIEW_STANDARD_RANGES
            if not ok:
                return f"leaf {chain or 'res'}: the range [{lo}, {hi}] is not the range of its field `{t}` (decision 0018)"
            facts = f"{flag} ==> (({lo}) <= {value} && {value} <= {hi})"
            i += 1
        leaves.append((chain, t, facts))
    # rebuild the whole text
    body = [*prefix, *lens]
    zero = {"bool": "false", "ptr": "null"}
    cell_leaves = [(c, t, f) for c, t, f in leaves if c not in arrays]
    for chain, t, facts in cell_leaves:
        cell = _chain_cell(chain, root)
        body += [f"acc({cell}.v_{t})", f"acc({cell}.d_{t})"] + ([facts] if facts else [])
    if routine == "calloc" and arrays:
        body += [f"{_chain_cell(c, root)}.d_{t}" for c, t, _ in cell_leaves]
    for chain in arrays:
        body += arrays[chain][3]
    tail: list[str] = []
    if routine == "calloc":
        if arrays:
            # the array leaves' flags, then the zero clauses: a cell's `C.v_t == 0`; an array leaf of scalars (decision 0064 item 15, question 351,
            # TO CONFIRM against the emitted text after the joined proof) one clause per ELEMENT, the contract read `root->a[j] == 0` as the
            # lowering frames it (the slot of each cell leaf of the array's own field suffix, in member order, then the array's own range: `contract_read`'s framing of an `owns_block`
            # site, cells before arrays whatever the member order), j a literal in `[0, N)`, each once; the zero clauses are
            # compared as a multiset (their order is the lowering's)
            for c in arrays:
                t, n, kv, _rows = arrays[c]
                blk = f"member({_chain_cell(c[:-1], root)}, {c[-1]})"
                body.append(f"forall {kv}: Int :: {{ loc({blk}, {kv}) }} 0 <= {kv} && {kv} < {n} ==> loc({blk}, {kv}).d_{t}")
                first_slot = f"loc({blk}, 0)"
                for j in range(n):
                    slot = f"loc({blk}, {j})"
                    expr = zero.get(_leaf_kind(t), "0")
                    tests = [f"(!({root} == null)) && ({_chain_cell(cc, root)} == {slot})" for cc, ct, _f0 in cell_leaves if ct == t]
                    # then the range of EVERY array leaf of the same field suffix, in member order (the slot may be one of another array's slots)
                    for c2, (t2, n2, _kv2, _rows2) in arrays.items():
                        if t2 == t:
                            blk2 = f"member({_chain_cell(c2[:-1], root)}, {c2[-1]})"
                            first2 = f"loc({blk2}, 0)"
                            tests.append(f"(!({root} == null)) && (((block_of({slot}) == block_of({first2})) && (offset_of({first2}) <= offset_of({slot}))) && "
                                         f"(offset_of({slot}) < (offset_of({first2}) + {n2})))")
                    for test in reversed(tests):
                        expr = f"(({test}) ? {slot}.v_{t} : {expr})"
                    tail.append(f"{expr} == {zero.get(_leaf_kind(t), '0')}")
            tail += [f"{_chain_cell(c, root)}.v_{t} == {zero.get(_leaf_kind(t), '0')}" for c, t, _ in cell_leaves]
        else:
            body += [f"{_chain_cell(c, root)}.d_{t}" for c, t, _ in cell_leaves]
            body += [f"{_chain_cell(c, root)}.v_{t} == {zero.get(_leaf_kind(t), '0')}" for c, t, _ in cell_leaves]
    if arrays:
        # the atoms are compared one by one (the text without grouping parentheses cannot tell a conjunct inside a quantifier from one beside it)
        got = atoms[len(prefix) + len(lens):]
        want_head = [norm_viper(b) for b in body[len(prefix) + len(lens):]]
        want_tail = sorted(norm_viper(b) for b in tail)
        if got[:len(want_head)] != want_head or sorted(got[len(want_head):]) != want_tail:
            return ("the block part is not exactly what its type's leaves give: the cells' lines, each array leaf's rows once (value permission, "
                    "flag permission, range under the flag), and for a `calloc` view the flags and one zero clause per element, each once, "
                    "and nothing else")
    body += tail
    expected_second = f"!({root} == null) ==> ({' && '.join(body)})"
    if norm_viper(second) != norm_viper(expected_second) and not arrays:
        return ("the block part is not exactly what its type's leaves give" +
                (" (a `calloc` view adds the flags and then the zero values of every leaf, and nothing else)" if routine == "calloc"
                 else " (a `malloc` view states no flag and no value, a `free` view only what `owns_block` takes)"))
    expected_first = _view_wf(root)
    if lens:
        expected_first += f" && (({root} != null && offset_of({root}) < len(block_of({root}))) ==> ({' && '.join(lens)}))"
    if norm_viper(first) != norm_viper(expected_first):
        return "its first clause is not the pointer's well-formedness (and the member block lengths of a structure under `offset_of < len`)"
    return None


TYPED_VIEW_WORD_RE = re.compile(r";\s*typed view:")
VIEW_ROUTINE_TAG_RE = re.compile(r"vlibc:\s+(?:vlibc_)?(?:malloc|calloc|free)\b")


def lint_views(src: Source, decls: list[Decl], add: Callable[[int, str], None], has_globals: bool = False,
               global_facts: list[str] | None = None, accepted: set[int] | None = None) -> None:
    """Decision 0047 items 2, 4 and 7, decision 0064 item 15: the pinned text of every typed view.  The lines of a view that passed (the pin is
    the only rule that reads a quantifier of a view, `lint_quantified_rows` skips exactly these lines) are added to `accepted`."""
    declared = {"v": set(), "d": set()}
    for line in src.code:
        m = FIELD_ANY_RE.match(line)
        if m:
            declared[m.group("kind")].add(m.group("suffix"))
    for d in decls:
        if d.kind != "method" or d.has_body and not VIEW_NAME_RE.match(d.name):
            continue
        tag = trusted_tag(src, d.line)
        if tag is None:
            continue
        vt = VIEW_TAG_RE.search(tag.group(0))
        if vt is None:
            if not d.has_body and VIEW_ROUTINE_TAG_RE.search(tag.group(0)):
                add(d.line, f"`method {d.name}` is tagged as vlibc's malloc, calloc or free but not as a typed view "
                            "(`; typed view:` in the reason): the allocation functions, and the `vlibc_` ones of decision 0050 item 3, are "
                            "trusted only as the views of decision 0047 items 2 and 4, whose text is pinned")
            elif not d.has_body and TYPED_VIEW_WORD_RE.search(tag.group(0)):
                # decision 0050 item 9: a method whose tag claims to be a typed view that no pattern above recognises was skipped
                add(d.line, f"`method {d.name}` is tagged `; typed view:` but is not a view of `malloc`, `calloc`, `free` or their "
                            "`vlibc_` forms (decision 0047 items 2 and 4, decision 0050 item 3): a bodyless method may not claim to "
                            "be a pinned view without being one")
            continue
        routine = vt.group("routine")
        vlibc = routine.startswith("vlibc_")
        why = view_problem(src, d, routine[len("vlibc_"):] if vlibc else routine, declared, has_globals, global_facts, vlibc)
        if why is not None:
            add(d.line, f"`method {d.name}` is not the trusted typed view of decision 0047 items 2 and 4: {why}")
        elif accepted is not None:
            accepted.update(range(d.line, d.end_line + 1))


def _flatten_or(text: str) -> list[str]:
    """The disjuncts of a possibly nested `||` chain (`((A || B) || C)`, as the lowering nests it, or `A || (B || C)`), as raw text."""
    text = _strip_wrap(text.strip())
    parts = _split_top(text, "||")
    if len(parts) == 1:
        return [text]
    out: list[str] = []
    for part in parts:
        out += _flatten_or(part)
    return out


def _and_leaves(text: str) -> list[str]:
    """The conjuncts of a nested `&&` chain at any grouping (`(A && B) && (C && D)` is A, B, C, D), as raw text."""
    text = _strip_wrap(text.strip())
    parts = _split_top(text, "&&")
    if len(parts) == 1:
        return [text]
    out: list[str] = []
    for part in parts:
        out += _and_leaves(part)
    return out


def token_term_pointer(loc_text: str) -> str | None:
    """`P` of `loc(block_of(P), -1)` (the token slot of the block of P), the text between `block_of(` and its closing parenthesis."""
    m = re.match(r"^loc\(\s*block_of\(", loc_text)
    if m is None:
        return None
    end = _group_end(loc_text, m.end() - 1)
    return None if end < 0 else loc_text[m.end():end]


def has_offset_zero(conjuncts: list[str], pointer: str) -> bool:
    """Whether the conjuncts hold `offset_of(<pointer>) == 0` (decision 0059 item 10: an exemption names a block's token only through a
    pointer to the start of the block)."""
    want = _nospace(f"offset_of({pointer}) == 0")
    return any(_nospace(c) == want for c in conjuncts)


# The loop-carried exemption terms of records 0050 and 0056 (record 0059 item 10 leaves them unrestated: they are reached only through owners,
# whose updates require `offset_of == 0`).  A term that lacks `offset_of(P) == 0` is accepted only when it is exactly one of these families,
# each over a pointer variable `P` (an identifier), so that a carried shape that is not one of them is refused:
#   temp      `T && t == loc(block_of(U), -1)` with `T` and `U` the loop's temporaries `t__<n>`;
#   kt        `P != null && kt_<...> > none && t == loc(block_of(P), -1)` (one kt snapshot compare), with alias guards: `block_of(P) != block_of(X)`,
#             `!ad_<...>`, `!((P != null && X != null) && block_of(P) == block_of(X))`;
#   replaced  `P != null && t == loc(block_of(P), -1)` (a replaced or written loop's owner), with the nullness guards `!(P == null)` or
#             `!(((P == null) ? n : n) != n)` of a block site read through a loop.
CARRY_ID = r"[A-Za-z_$][\w$']*"


def carried_term_family(leaves: list[str], term: str, pointer: str) -> str | None:
    """The loop-carry family of a term (`temp`, `kt` or `replaced`, see above), or None."""
    p = _nospace(pointer)
    if not re.fullmatch(CARRY_ID, p):
        return None
    others = [_nospace(l) for l in leaves if l != term]
    if len(others) == 1 and re.fullmatch(r"t__[0-9]+", others[0]) and re.fullmatch(r"t__[0-9]+", p):
        return "temp"
    nonnull = f"{p}!=null"
    if nonnull not in others:
        return None
    rest = list(others)
    rest.remove(nonnull)
    kts = [x for x in rest if re.fullmatch(r"kt_[\w$']*>none", x)]
    if len(kts) == 1:
        rest.remove(kts[0])
        e = re.escape(p)
        if all(re.fullmatch(rf"block_of\({e}\)!=block_of\({CARRY_ID}\)|!ad_[\w$']*|"
                            rf"!\(\(\({e}!=null\)&&\({CARRY_ID}!=null\)\)&&\(block_of\({e}\)==block_of\({CARRY_ID}\)\)\)", x)
               for x in rest):
            return "kt"
        return None
    e = re.escape(p)
    if all(re.fullmatch(rf"!\({e}==null\)|!\(\(\({e}==null\)\?[0-9]+:[0-9]+\)!=[0-9]+\)", x) for x in rest):
        return "replaced"
    return None


def forperm_exemption_problem(body: str, carried_ok: bool = False) -> str | None:
    """Decision 0054 item 8: the exemption of a `forperm` leak row is `false` or a disjunction of terms, each a conjunction of guards
    with exactly one `t == loc(block_of(<pointer>), -1)`; `t` appears nowhere else, so an exemption only names the token of a block
    by its location (a carried token), never `true`, `t != ...` or a negated term.  The loop-carry families of `carried_term_family` (terms
    without `offset_of(P) == 0`) are accepted only when `carried_ok`: in the row of a loop's back edge (`LOOP_LEAK_ROW_MESSAGE` in its map
    entry), where the lowering emits them; a function's `ensures` and end rows hold the conjunct, whatever their guard looks like."""
    body = _strip_wrap(body.strip())
    if body == "false":
        return None
    for disjunct in _flatten_or(body):
        conjuncts = _and_leaves(disjunct)
        terms = [c for c in conjuncts if (m := re.match(r"^t\s*==\s*(?P<loc>loc\(.*\))$", c, re.S)) and TOKEN_SLOT_RE.search(m.group("loc"))
                 and _group_end(m.group("loc"), 3) == len(m.group("loc")) - 1]
        if len(terms) == 1:
            pointer = token_term_pointer(re.match(r"^t\s*==\s*(?P<loc>loc\(.*\))$", terms[0], re.S).group("loc"))
            if (pointer is None or not has_offset_zero(conjuncts, pointer)) and (
                    pointer is None or not carried_ok or carried_term_family(conjuncts, terms[0], pointer) is None):
                return ("an exemption term of a `forperm` leak row holds `offset_of(<pointer>) == 0` for the pointer of its `t == loc(block_of(<pointer>), -1)`: "
                        "a pointer into the middle of a block carries no token, unless the row is a loop's back edge and the term exactly one of "
                        "the loop-carry families of decisions 0050 and 0056 (decision 0059 item 10)")
        if len(terms) != 1:
            return ("an exemption term of a `forperm` leak row is `<guards> && t == loc(block_of(<pointer>), -1)` with exactly one "
                    "such comparison: it names a carried token by its location (decision 0054 item 8)")
        for c in conjuncts:
            if c not in terms and re.search(r"(?<![\w$'.])t(?![\w$'])", c):
                return ("`t` appears in a `forperm` exemption outside the comparison `t == loc(block_of(<pointer>), -1)`: a guard "
                        "that tests the token itself would exempt more than a carried block (decision 0054 item 8)")
    return None


# The map message of the leak row at a loop's back edge (lower.cpp: "an iteration of this loop may end while it still owns an allocation that the
# loop does not carry to the next iteration"): the one row whose exemption may hold the loop-carry terms.
LOOP_LEAK_ROW_MESSAGE = "an iteration of this loop may end while it still owns an allocation"


def lint_heap_token(src: Source, decls: list[Decl], add: Callable[[int, str], None], loop_rows: frozenset[int] = frozenset()) -> set[int]:
    """The rules of the free token (see above).  Returns the lines of the `forperm` assertions, whose map entries the caller
    checks against the category `leak`."""
    forperm_lines: set[int] = set()
    decl_lines = {d.line for d in decls if d.kind == "field" and d.name == "tok"}
    used: list[int] = []
    for lineno, code in enumerate(src.code, start=1):
        if lineno in decl_lines:
            if " ".join(code.split()) != TOK_FIELD_DECL:
                add(lineno, f"`field tok` must be exactly `{TOK_FIELD_DECL}` (the free token of a heap block, decision 0047)")
            continue
        has_forperm = bool(FORPERM_WORD_RE.search(code))
        tok_hits = list(TOK_WORD_RE.finditer(code))
        if not has_forperm and not tok_hits:
            continue
        used.append(lineno)
        if has_forperm:
            m = FORPERM_LINE_RE.match(code)
            if m is None:
                add(lineno, "`forperm` is only the statement `assert forperm t: Ref [t.tok] :: <exemption>` of a `leak` row "
                            "(decision 0047 item 7)")
                continue
            if TOK_WORD_RE.search(m.group("body")):
                add(lineno, "the exemption of a `forperm` leak row does not mention `tok`: it names the places the method "
                            "hands back (decision 0047 item 7)")
            elif (why := forperm_exemption_problem(m.group("body"), lineno in loop_rows)) is not None:
                add(lineno, why)
            forperm_lines.add(lineno)
            continue
        for h in tok_hits:
            before = code[:h.start()]
            if not before.endswith(".") or not TOKEN_SLOT_RE.search(before[:-1]):
                add(lineno, "`tok` outside the token slot of a block: it is only `loc(block_of(...), -1).tok` (decision 0047 "
                            "item 4)")
                break
        else:
            stripped = code.strip()
            if re.match(r"^exhale\b", stripped):
                add(lineno, "`exhale` of a `tok` place: a token is consumed only by a call whose `requires` has the block "
                            "site (decision 0047 item 6)")
            elif is_ending_row(src, lineno):
                pass  # an ending back-edge row: the token rules above have read its `tok`s as an assert's
            elif (km := KTV_ONE_LINE_RE.match(code) or KTV_ALONE_RE.match(code)) is not None:
                above = next((c for c in reversed(src.code[:lineno - 1]) if c.strip()), "")
                guard = km.group("g") if "g" in km.groupdict() else None
                if guard is None and (gm := KTV_IF_LINE_RE.match(above)) is not None:
                    guard = gm.group("g")
                # (the slot itself is the loop above's check: any `.tok` not at `loc(block_of(...), -1)` is reported there)
                if guard is None:
                    add(lineno, f"`{km.group('v')} := ....tok` outside `if (kt_<n> > none) {{ ... }}`: the loop value snapshot is taken "
                                "only where the loop holds the token (decision 0050 item 5)")
                elif not guard.startswith("kt_") or km.group("v") != "ktv" + guard[2:]:
                    add(lineno, f"`{km.group('v')}` is taken under the guard of `{guard}`: the snapshot of a place's value is taken where "
                                "that same place's `kt_<n>` is held, so its name is `ktv_<n>` for the guard `kt_<n> > none` "
                                "(decision 0050 item 5)")
            elif TV_ASSIGN_RE.match(code):
                pass  # an owner's token value (decision 0052 item 3): the one conditional read of a token that is stored
            elif re.search(r"\.tok\s*:=", code):
                add(lineno, "assignment to `tok`: a token is created only by a postcondition (decision 0047 item 2)")
            elif TOK_STATEMENT_RE.match(code):
                pass  # a conjunct of a view, a block site, an explicit row or a loop place
            elif re.match(r"^\s*(?:var\s+)?[A-Za-z_$][\w$']*\s*(?::\s*Perm\s*)?:=", code):
                rhs = code.split(":=", 1)[1]
                if TOK_WORD_RE.search(strip_perm_calls(rhs)):
                    add(lineno, "`tok` in the right-hand side of an assignment beyond the `perm(...)` snapshot of a loop "
                                "place's amount (decision 0047 item 10)")
            else:
                add(lineno, f"`tok` in `{stripped.split()[0] if stripped else ''}`: it is only a conjunct of a `requires`, "
                            "`ensures`, `invariant` or `assert`, or inside the `perm(...)` of a snapshot (decision 0047 items "
                            "4, 7 and 10)")
    if decl_lines and not used:
        add(min(decl_lines), "`field tok: Bool` in a program that never uses a token: only a heap program (one with a view "
                             "call or a block site) declares it (decision 0047 item 7)")
    if used and not decl_lines:
        add(used[0], "`tok` is used but `field tok: Bool` is not declared")
    return forperm_lines



# The flag rows of a marked closure site and the row of `cerastes::not_invalidating` (decision 0058 items 2 and 4).
# A marked (`ref_to_uninit`) closure site read through a load keeps the flags of the entry cells only when its pointer is
# tied (`E == old(E)`): the row `(old(L != null) && old(E.d)) ==> E.d` compares the entry block's flag with the return
# block's, so without the equality (B1 of audit round 1) it compares two different blocks and cannot be proved.  A site
# through a load that is not tied claims no flags and has no such row.
KEPT_ROW_RE = re.compile(r"\(\(old\((?P<guard>.+?)\) && old\((?P<cell>[^&]+?\.d_\w+)\)\) ==> (?P=cell)\)")
FLAG_FIELD_RE = re.compile(r"\.d_\w+$")
MEMBER_CELL_RE = re.compile(r"loc\(member\((?P<base>.+), \d+\), 0\)")
NOT_INVALIDATING_ROW_MARK = "does not return it, but"


def _double_negation(body: str) -> bool:
    """Whether `body` is exactly one `!(!(CLAUSE))`: the outer negation's group ends at the end of the text, and so
    does the inner one's (a text that merely starts with `!(!(` is not)."""
    body = body.strip()
    if not body.startswith("!(") or _group_end(body, 1) != len(body) - 1:
        return False
    inner = body[2:-1].strip()
    return inner.startswith("!(") and _group_end(inner, 1) == len(inner) - 1


def lint_marked_kept_rows(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """A Kept row for a pointer read through a load (the text `.v_` in it: a formal, a result or a global's own cell
    cannot change, and their rows stand as in decision 0042 item 5) stands in a clause that also holds the equality
    `E == old(E)` (a written tie lands after the closure, a spliced one beside its framing atom: its place in the clause
    does not matter, its presence does: an untied site is never emitted as Kept); and where the row stands in an `assert`
    (the return assertion of the clause) that assertion is the one atomic `assert !(!(CLAUSE))`, so that Silicon's check
    of the contract passes after the one user error (decision 0058 item 4, B1)."""
    for lineno, code in enumerate(src.code, start=1):
        text = code.strip()
        in_ensures = text.startswith("ensures ")
        in_assert = text.startswith("assert ")
        if not (in_ensures or in_assert):
            continue
        for m in KEPT_ROW_RE.finditer(code):
            pointer = FLAG_FIELD_RE.sub("", m.group("cell"))
            # a cell of a block reached through a pointer (`loc(member(E, k), 0)`, a member of the pointee structure) is
            # kept under the tie of that pointer E (E is itself a loaded pointer, so one step reaches it)
            if (cell := MEMBER_CELL_RE.fullmatch(pointer)) is not None:
                pointer = cell.group("base")
            if ".v_" not in pointer:
                continue
            tie = f"{pointer} == old({pointer})"
            if tie not in code:
                add(lineno, f"a marked closure site's kept-flag row for `{pointer}` (a pointer read through a load) is in a "
                            f"clause without `{tie}`: without the tie the row compares the flag of two "
                            "different blocks and cannot be proved; an untied site claims no flags (decision 0058 item 4, "
                            "audit B1)")
            if in_assert and not _double_negation(text[len("assert "):]):
                add(lineno, f"the return assertion that holds a marked closure site's kept-flag row for `{pointer}` is not the "
                            "one atomic `assert !(!(CLAUSE))` (decision 0058 item 4, audit B1): the tie and the row would "
                            "be two checks, and a failed tie would leave the row to Silicon's own check of the contract")


def _guard_parts(consequent: str) -> list[str]:
    """The guards of a consequent: `(GP) ==> (GE)` as [GP, GE], a lone `GE` as [GE]."""
    inner = _strip_wrap(consequent)
    parts = _split_top(inner, "==>")
    return [_strip_wrap(x) for x in parts]


def _assert_row(code: str) -> tuple[str, list[str]] | None:
    """(antecedent, guards) of an `assert ANT ==> CONSEQUENT` line, or None."""
    text = code.strip()
    if not text.startswith("assert "):
        return None
    parts = _split_top(text[len("assert "):], "==>")
    if len(parts) != 2:
        return None
    return _strip_wrap(parts[0]), _guard_parts(parts[1])


def lint_not_invalidating_rows(src: Source, decls: list[Decl], entries: list["MapEntry"],
                               add: Callable[[int, str], None]) -> None:
    """The `f__wf` row of `cerastes::not_invalidating` (decision 0058 item 2, "taken implies returned on every path"),
    found by its map message: an `assert` of a `__wf` method of the form `ANT ==> (GP ==> GE)` (or `ANT ==> GE` for an
    unguarded requires site), whose guards are the converse of an earlier row of the same method, the partner row
    `ANT' ==> (GE ==> GP)` (so a mutant that restates the existing row, or drops the swap, is refused), and whose
    antecedent holds the antecedent of that row.  The partner row exists for a bodiless contract (its base method has no
    body) and is required there; for a function with a body only the form and the place are checked."""
    for e in entries:
        if NOT_INVALIDATING_ROW_MARK not in e.message or "cerastes::not_invalidating" not in e.message:
            continue
        line = e.vpr_line
        if line > len(src.code):
            continue
        owner = next((d for d in decls if d.kind == "method" and d.line <= line <= d.end_line), None)
        if owner is None or not owner.name.endswith("__wf"):
            add(line, "the row of `cerastes::not_invalidating` (taken implies returned) is not in a `__wf` method "
                      "(decision 0058 item 2)")
            continue
        row = _assert_row(src.code[line - 1])
        if row is None:
            add(line, "the row of `cerastes::not_invalidating` is not an `assert ANT ==> CONSEQUENT` line (decision 0058 item 2)")
            continue
        antecedent, guards = row
        if len(guards) == 1:
            continue
        if len(guards) != 2:
            add(line, "the row of `cerastes::not_invalidating` has the consequent `GP ==> GE` or `GE`, not "
                      "a longer chain (decision 0058 item 2)")
            continue
        gp, ge = guards
        partner = None
        for earlier in range(owner.line, line):
            other = _assert_row(src.code[earlier - 1])
            if other is not None and len(other[1]) == 2 and other[1] == [ge, gp]:
                partner = other
                break
        base = next((d for d in decls if d.kind == "method" and d.name == owner.name[:-len("__wf")]), None)
        if partner is None and base is not None and base.has_body:
            # a function with a body has no partner row (the existing row and the tie are asserted for a bodiless
            # contract, decision 0054 item 2): the converse is checked against it only where it exists
            continue
        if partner is None:
            add(line, "the row of `cerastes::not_invalidating` (`GP ==> GE`) has no earlier row in its `__wf` method "
                      "with the two guards swapped (the partner row `GE ==> GP`): the new row is the converse of "
                      "the existing one (decision 0058 item 2)")
        elif partner[0] not in antecedent:
            add(line, "the antecedent of the row of `cerastes::not_invalidating` does not hold the antecedent of its "
                      "partner row: it is the contract up to and including the site's clause (decision 0058 item 2)")



# ---------------------------------------------------------------------------
# Arrays (decision 0059): the trusted allocations, the quantified rows, `old[label]`
# ---------------------------------------------------------------------------

# The trusted allocation of an automatic array of `<t>` (decision 0059 item 3) and its zero fill: bodyless, tagged, and an
# assumption (invariant 4), so every line is pinned like `alloc_<t>`'s.  The quantifiers are one per field and each over the
# absolute slot with the trigger `{ loc(block_of(r), k) }`; `zero_arr_<t>` takes and returns the same two `acc`
# quantifiers over the same range, so it cannot conjure a permission (an `ensures` range wider than the `requires`, or an
# `ensures acc` with no `requires acc`, would).  The zero literal is the field's: `0` for an Int field, `false` for Bool, `null` for
# Ref (S43 evidences int only; the other two follow from the field's declared type).
ARR_ALLOC_PREFIX = "alloc_arr_"
ARR_ZERO_PREFIX = "zero_arr_"
ARR_SUFFIX_RE = re.compile(r"^[A-Za-z0-9_]+$")
ZERO_LITERAL = {"Int": "0", "Bool": "false", "Ref": "null"}


def _arr_quantified(body: str, r: str = "r") -> str:
    return f"forall k: Int :: {{ loc(block_of({r}), k) }} ((0 <= k) && (k < n)) ==> {body}"


def array_alloc_clauses(t: str) -> list[tuple[str, str]]:
    """The (keyword, text) clauses of `alloc_arr_<t>` as S43 pins them (whitespace and grouping are not compared)."""
    q = _arr_quantified
    return [
        ("requires", "1 <= n"),
        ("ensures", "((r != null) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == n)"),
        ("ensures", "(lvloc(block_of(r)) == loc(block_of(r), (-2))) && acc(lvloc(block_of(r)).lv)"),
        ("ensures", q(f"acc(loc(block_of(r), k).v_{t})")),
        ("ensures", q(f"acc(loc(block_of(r), k).d_{t})")),
        ("ensures", q(f"(!loc(block_of(r), k).d_{t})")),
    ]


def array_zero_clauses(t: str, zero: str) -> list[tuple[str, str]]:
    q = _arr_quantified
    return [
        ("requires", "((1 <= n) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == n)"),
        ("requires", q(f"acc(loc(block_of(r), k).v_{t})")),
        ("requires", q(f"acc(loc(block_of(r), k).d_{t})")),
        ("ensures", q(f"acc(loc(block_of(r), k).v_{t})")),
        ("ensures", q(f"acc(loc(block_of(r), k).d_{t})")),
        ("ensures", q(f"(loc(block_of(r), k).v_{t} == {zero})")),
        ("ensures", q(f"loc(block_of(r), k).d_{t}")),
    ]


ARR_COPY_PREFIX = "copy_arr_"


def array_copy_clauses(t: str) -> list[tuple[str, str]]:
    """The (keyword, text) clauses of `copy_arr_<t>` (decision 0063 item 13: the copy of n elements between two distinct blocks, as
    s47_g_copy_*.vpr write it): the destination's value and flag permissions over `[0, n)`, the source's at the amount `rd` (`none < rd <= write`), every
    source flag set, the blocks distinct and starting at slot 0 with n slots; back: the same permissions, each destination value the source's old value at the
    same index, each destination flag set, each source value unchanged and each source flag still set (S47 (g): without it the source's next read fails its
    `uninitialized` row)."""
    def q(b: str, body: str) -> str:
        return f"forall k: Int :: {{ loc(block_of({b}), k) }} ((0 <= k) && (k < n)) ==> {body}"
    d, s_ = "d", "s"
    cd, cs = "loc(block_of(d), k)", "loc(block_of(s), k)"
    return [
        ("requires", "(1 <= n) && (d == loc(block_of(d), 0)) && (len(block_of(d)) == n) && (s == loc(block_of(s), 0)) && (len(block_of(s)) == n)"),
        ("requires", "block_of(d) != block_of(s)"),
        ("requires", "(none < rd) && (rd <= write)"),
        ("requires", q(d, f"acc({cd}.v_{t})")), ("requires", q(d, f"acc({cd}.d_{t})")),
        ("requires", q(s_, f"acc({cs}.v_{t}, rd)")), ("requires", q(s_, f"acc({cs}.d_{t}, rd)")), ("requires", q(s_, f"{cs}.d_{t}")),
        ("ensures", q(d, f"acc({cd}.v_{t})")), ("ensures", q(d, f"acc({cd}.d_{t})")),
        ("ensures", q(s_, f"acc({cs}.v_{t}, rd)")), ("ensures", q(s_, f"acc({cs}.d_{t}, rd)")),
        ("ensures", q(d, f"{cd}.v_{t} == old({cs}.v_{t})")), ("ensures", q(d, f"{cd}.d_{t}")),
        ("ensures", q(s_, f"{cs}.v_{t} == old({cs}.v_{t})")), ("ensures", q(s_, f"{cs}.d_{t}")),
    ]


def _clause_split(code: str) -> tuple[str, str] | None:
    m = re.match(r"^\s*(requires|ensures)\s+(.*)$", code)
    return (m.group(1), norm_viper(m.group(2))) if m else None


def array_allocation_problem(src: Source, d: Decl, field_types: dict[str, str]) -> str | None:
    alloc = d.name.startswith(ARR_ALLOC_PREFIX)
    copy = d.name.startswith(ARR_COPY_PREFIX)
    t = d.name[len(ARR_ALLOC_PREFIX if alloc else ARR_COPY_PREFIX if copy else ARR_ZERO_PREFIX):]
    if not ARR_SUFFIX_RE.match(t):
        return f"the element suffix `{t}` is not an identifier"
    if not copy and any("member(" in src.code[n - 1] for n in range(d.line, d.end_line + 1)):
        # an element that is a structure (decision 0064 item 17): the clauses name the element's members, so the layout is read from them
        why = aggregate_array_problem(src, d, alloc, field_types)[0]
        return None if why is None else f"(an element that is a structure, decision 0064 items 3 and 6) {why}"
    if f"v_{t}" not in field_types or f"d_{t}" not in field_types:
        return f"the field suffix `{t}` is not declared (`field v_{t}` and `field d_{t}`)"
    lines = [(n, src.code[n - 1]) for n in range(d.line, d.end_line + 1) if src.code[n - 1].strip()]
    if d.has_body:
        return "it has a body (a trusted method is bodyless)"
    header = " ".join(lines[0][1].split())
    want_header = (f"method {d.name}(n: Int) returns (r: Ref)" if alloc else
                   f"method {d.name}(d: Ref, s: Ref, n: Int, rd: Perm)" if copy else f"method {d.name}(r: Ref, n: Int)")
    if header != want_header:
        return f"its header is not `{want_header}`"
    if " ".join(lines[-1][1].split()) != "decreases _":
        return "it does not end with `decreases _`"
    want_clauses = (array_alloc_clauses(t) if alloc else array_copy_clauses(t) if copy
                    else array_zero_clauses(t, ZERO_LITERAL.get(field_types[f"v_{t}"], "?")))
    want = sorted((kw, norm_viper(text)) for kw, text in want_clauses)
    got = []
    for _n, code in lines[1:-1]:
        c = _clause_split(code)
        if c is None:
            return f"a line that is not a `requires` or `ensures` clause: `{' '.join(code.split())}`"
        got.append(c)
    # `n >= 1` and `1 <= n` are one requirement (the record writes the first, S43's fixtures the second)
    got = [(kw, "1<=n" if text == "n>=1" else text) for kw, text in got]
    got.sort()
    if got != want:
        extra = [c for c in got if c not in want]
        missing = [c for c in want if c not in got]
        return ("its clauses are not the pinned ones"
                + (f"; unexpected `{extra[0][0]} {extra[0][1]}`" if extra else "")
                + (f"; missing `{missing[0][0]} {missing[0][1]}`" if missing else ""))
    return None


# The trusted allocation of an automatic array whose element is a structure (decision 0064 items 3, 6 and 17): `alloc_arr_<E>(n)` and its zero fill
# `zero_arr_<E>(r, n)`.  The element's layout is not in the name, so the clauses are read: the SLOT of element `k` is `loc(block_of(r), k)`; the length
# facts are ONE quantifier over `[0, n)` with that slot as its trigger whose consequent is what `struct_wf_lens_problem` accepts of a structure at `slot`
# (`len(member(<chain>, j)) == 1` for each member block, a nested structure's own members, the slot facts `A != B` of decision 0043 item 4); each LEAF (a
# member block with no member of its own) has, as the leaf receiver `loc(<chain>, 0)` of the canonical chain rooted at the slot, three quantifiers over
# `[0, n)` with that receiver as trigger (the value permission, the flag permission, `!flag`), each (receiver, field) once.  A leaf with an array member
# (the second binder) and a count other than 1 are Part E's: the rule refuses them (`struct_wf_lens_problem` gets no counts).
AGG_QUANT_RE = re.compile(r"^forall(?P<k>[A-Za-z_$][\w$']*):Int::\{(?P<trigger>[^{}]*)\}(?P<lo>[\w$']+)<=(?P=k)&&(?P=k)<(?P<hi>[\w$']+)==>(?P<cons>.+)$")
AGG_SLOT = "loc(block_of(r),k)"
AGG_ROOT_FACTS = norm_viper("((r != null) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == n)")
AGG_LIVE_FACTS = norm_viper("(lvloc(block_of(r)) == loc(block_of(r), (-2))) && acc(lvloc(block_of(r)).lv)")
AGG_ZERO_ROOT_FACTS = norm_viper("((1 <= n) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == n)")
_AGG_ACC_RE = re.compile(r"^acc\((?P<cell>.+)\.(?P<f>[vd])_(?P<t>[\w$']+)\)$")
_AGG_NOT_FLAG_RE = re.compile(r"^!(?P<cell>.+)\.d_(?P<t>[\w$']+)$")
_AGG_FLAG_RE = re.compile(r"^(?P<cell>.+)\.d_(?P<t>[\w$']+)$")
_AGG_ZERO_RE = re.compile(r"^(?P<cell>.+)\.v_(?P<t>[\w$']+)==(?P<z>.+)$")


def aggregate_array_problem(src: Source, d: Decl, alloc: bool, field_types: dict[str, str]) -> tuple[str | None, dict[tuple[int, ...], str]]:
    """(why the trusted method `d` (an `alloc_arr_<E>` when `alloc`, else a `zero_arr_<E>`) is not the allocation or zero fill of an array of
    structures, or None; the leaves it states: chain -> field suffix)."""
    none: dict[tuple[int, ...], str] = {}
    if d.has_body:
        return "it has a body (a trusted method is bodyless)", none
    lines = [(n, src.code[n - 1]) for n in range(d.line, d.end_line + 1) if src.code[n - 1].strip()]
    header = " ".join(lines[0][1].split())
    want_header = f"method {d.name}(n: Int) returns (r: Ref)" if alloc else f"method {d.name}(r: Ref, n: Int)"
    if header != want_header:
        return f"its header is not `{want_header}`", none
    if " ".join(lines[-1][1].split()) != "decreases _":
        return "it does not end with `decreases _`", none
    clauses: list[tuple[str, str]] = []
    for _n, code in lines[1:-1]:
        c = _clause_split(code)
        if c is None:
            return f"a line that is not a `requires` or `ensures` clause: `{' '.join(code.split())}`", none
        clauses.append(c)
    seen: dict[str, int] = {}

    def once(what: str) -> str | None:
        seen[what] = seen.get(what, 0) + 1
        return f"{what} twice" if seen[what] > 1 else None

    lens_parts: list[str] | None = None
    roles: dict[tuple[tuple[int, ...], str], dict[str, str]] = {}  # (chain, role) -> field suffix, one entry per quantified row
    for kw, text in clauses:
        if kw == "requires" and alloc:
            if text not in ("1<=n", "n>=1"):
                return f"the `requires {text}` is not `1 <= n`", none
            if (why := once("`requires 1 <= n`")) is not None:
                return why, none
            continue
        if text == (AGG_ROOT_FACTS if alloc else AGG_ZERO_ROOT_FACTS) and kw == ("ensures" if alloc else "requires"):
            if (why := once("the root facts")) is not None:
                return why, none
            continue
        if alloc and kw == "ensures" and text == AGG_LIVE_FACTS:
            if (why := once("the liveness token")) is not None:
                return why, none
            continue
        m = AGG_QUANT_RE.match(text)
        if m is None:
            return f"`{kw} {text}` is not one of the clauses of decision 0064 items 3 and 6", none
        if m.group("k") != "k" or m.group("lo") != "0" or m.group("hi") != "n":
            return f"`{kw} {text}`: a quantified clause is over `0 <= k && k < n` with the binder `k`", none
        trigger, cons = m.group("trigger"), m.group("cons")
        if trigger == AGG_SLOT:
            if not (alloc and kw == "ensures"):
                return "the length facts stand in the `ensures` of an allocation only", none
            if (why := once("the length facts")) is not None:
                return why, none
            lens_parts = cons.split("&&")
            continue
        chain = parse_struct_chain(trigger, AGG_SLOT)
        if not chain:
            return (f"`{kw} {text}`: the trigger is not the slot `loc(block_of(r), k)` (the length facts) or a leaf receiver `loc(<chain>, 0)` "
                    "of the canonical chain rooted at the slot (decision 0064 item 17)"), none
        role = None
        cm = _AGG_ACC_RE.match(cons)
        if cm is not None and cm.group("cell") == trigger:
            role = ("acc_" + cm.group("f"), cm.group("t"))
        elif (cm := _AGG_NOT_FLAG_RE.match(cons)) is not None and cm.group("cell") == trigger and alloc and kw == "ensures":
            role = ("not_d", cm.group("t"))
        elif (cm := _AGG_FLAG_RE.match(cons)) is not None and cm.group("cell") == trigger and not alloc and kw == "ensures":
            role = ("flag", cm.group("t"))
        elif (cm := _AGG_ZERO_RE.match(cons)) is not None and cm.group("cell") == trigger and not alloc and kw == "ensures":
            zero = ZERO_LITERAL.get(field_types.get(f"v_{cm.group('t')}", ""), "?")
            if cm.group("z") != zero:
                return f"`{kw} {text}`: the zero literal of the field `v_{cm.group('t')}` is `{zero}`", none
            role = ("zero", cm.group("t"))
        if role is None:
            return f"`{kw} {text}`: the consequent is not one of the leaf's rows (`acc` of the value or the flag, `!flag`, the zero or the flag set) of the receiver", none
        key = (chain, f"{kw} {role[0]}")
        if key in roles:
            return f"the leaf {chain}: `{kw} {role[0]}` twice (each (receiver, field) is given once)", none
        roles[key] = {"t": role[1]}
    if alloc:
        if not {"`requires 1 <= n`", "the root facts", "the liveness token", "the length facts"} <= set(seen):
            return "a clause is missing (`requires 1 <= n`, the root facts, the length facts or the liveness token)", none
        assert lens_parts is not None
        if (why := struct_wf_lens_problem(lens_parts, AGG_SLOT)) is not None:
            return f"the length facts: {why}", none
        members = {parse_struct_member(m.group("m"), AGG_SLOT) for p in lens_parts if (m := re.fullmatch(r"len\((?P<m>.*)\)==[0-9]+", p))}
        members.discard(None)
        want_leaves = {key for key in members if not any(o[:-1] == key and o != key for o in members)}
        need = ("ensures acc_v", "ensures acc_d", "ensures not_d")
    else:
        if "the root facts" not in seen:
            return "the `requires` of the root facts is missing", none
        want_leaves = {chain for chain, _role in roles}
        need = ("requires acc_v", "requires acc_d", "ensures acc_v", "ensures acc_d", "ensures zero", "ensures flag")
    have = {chain for chain, _role in roles}
    if have != want_leaves or not have:
        return (f"the leaves {sorted(have)} of its quantified clauses are not the leaves {sorted(want_leaves)} of the element's members"
                if alloc else "it states no leaf"), none
    leaves: dict[tuple[int, ...], str] = {}
    for chain in sorted(have):
        suffixes = set()
        for role in need:
            row = roles.get((chain, role))
            if row is None:
                return f"the leaf {chain} lacks its `{role}` row", none
            suffixes.add(row["t"])
        if len({role for (c, role) in roles if c == chain} - set(need)):
            return f"the leaf {chain} has a row that is not one of `{', '.join(need)}`", none
        if len(suffixes) != 1:
            return f"the leaf {chain}: its rows are over more than one field suffix", none
        (t,) = suffixes
        if f"v_{t}" not in field_types or f"d_{t}" not in field_types:
            return f"the field suffix `{t}` is not declared (`field v_{t}` and `field d_{t}`)", none
        leaves[chain] = t
    return None, leaves


def array_field_types(src: Source) -> dict[str, str]:
    field_types: dict[str, str] = {}
    for line in src.code:
        m = re.match(r"^\s*field\s+(?P<name>[vd]_[A-Za-z0-9_$']+)\s*:\s*(?P<type>[A-Za-z]+)\s*$", line)
        if m:
            field_types[m.group("name")] = m.group("type")
    return field_types


def is_pinned_array_copy(src: Source, d: Decl) -> bool:
    """Whether `d` is a `copy_arr_<t>` with exactly the pinned clause set: the exemptions of the trusted copy from the global facts and from the ghost-amount
    bounds are granted by the clauses, not by the name (a method that only carries the prefix is an ordinary method with those rules, and is refused as a copy)."""
    return (d.kind == "method" and d.name.startswith(ARR_COPY_PREFIX)
            and array_allocation_problem(src, d, array_field_types(src)) is None)


def lint_array_allocations(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0059 items 3 and 12: `alloc_arr_<t>` and `zero_arr_<t>` are trusted (the tag rule), so each has exactly the
    contract the record states, beside `alloc_<t>`'s `len == 1` rule (a different postcondition would be an unsound assumption)."""
    field_types = array_field_types(src)
    for d in decls:
        if d.kind != "method" or not d.name.startswith((ARR_ALLOC_PREFIX, ARR_ZERO_PREFIX, ARR_COPY_PREFIX)):
            continue
        why = array_allocation_problem(src, d, field_types)
        if why is None and d.name.startswith(ARR_ZERO_PREFIX) and any("member(" in src.code[n - 1] for n in range(d.line, d.end_line + 1)):
            # decision 0064 item 6: the zero fill of an element structure takes and gives the leaves its allocation gives
            alloc = next((a for a in decls if a.kind == "method" and a.name == ARR_ALLOC_PREFIX + d.name[len(ARR_ZERO_PREFIX):]), None)
            if alloc is not None and (alloc_why := aggregate_array_problem(src, alloc, True, field_types))[0] is None:
                if aggregate_array_problem(src, d, False, field_types)[1] != alloc_why[1]:
                    why = f"(an element that is a structure, decision 0064 items 3 and 6) its leaves are not the leaves of `{alloc.name}` (line {alloc.line})"
        if why is not None:
            if d.name.startswith(ARR_COPY_PREFIX):
                add(d.line, f"`method {d.name}` is not the trusted array copy of decision 0063 item 13: {why}")
                continue
            what = "trusted array allocation" if d.name.startswith(ARR_ALLOC_PREFIX) else "trusted zero fill"
            add(d.line, f"`method {d.name}` is not the {what} of decision 0059 item 3: {why}")


# Decision 0064 item 6 (audit round 2, issue 7): every way out of the scope of an array of structures exhales exactly what its allocation gave.  In a method
# with `X := alloc_arr_<E>(N)` (N a literal) of an aggregate element, each run of consecutive `exhale forall` rows over `block_of(X)` is the allocation's set of
# (receiver, field) rows over `0 <= k && k < N` (each leaf's value and flag permission, each once), and is directly preceded (a `perm(lvloc(block_of(X)).lv) ==
# write` assert may stand between) by a run of marked `assert forall` rows of the same set (the lifetime rows); a method that allocates and has no such exit
# and no `assert false` (a path that never leaves) is refused.  A row over another variable is the older rules'.
ALLOC_ARR_CALL_RE = re.compile(rf"^\s*(?P<v>{IDENT})\s*:=\s*(?P<alloc>alloc_arr_[A-Za-z0-9_$']+)\s*\((?P<n>[^()]*)\)\s*$")
LIVE_EXHALE_RE = re.compile(rf"^\s*exhale\s+acc\(lvloc\(block_of\((?P<v>{IDENT})\)\)\.lv\)\s*$")
END_LABEL_RE = re.compile(r"^\s*label\s+l_end\s*$")
GOTO_END_RE = re.compile(r"^\s*goto\s+l_end\s*$")
LIVE_WRITE_RE = re.compile(rf"^\s*assert\s+perm\(lvloc\(block_of\((?P<v>{IDENT})\)\)\.lv\)\s*==\s*write\b")


def _aggregate_leaf_rows(src: Source, d: Decl) -> list[tuple[int, str, str, tuple[int, ...], str, str, str]]:
    """(line, `assert` or `exhale`, X, leaf chain, field, lo, hi) of the unguarded quantified permission rows over a leaf of the elements of X in `d`."""
    rows = []
    for n in range(d.line, d.end_line + 1):
        m = QUANT_STMT_RE.match(src.code[n - 1])
        if m is None or m.group("kw") not in ("assert", "exhale"):
            continue
        guard, body = split_guard(m.group("rest"))
        q = parse_leaf_row(body) if body is not None and guard is None else None
        if q is None or q[2]:
            continue
        acc = _row_acc(q)
        xm = BLOCK_BASE_RE.match(_squash(q[1]))
        chain = parse_struct_chain(q[6], f"loc({_squash(q[1])},{q[0]})")
        if acc is None or acc[1] is not None or xm is None or not chain:
            continue
        rows.append((n, m.group("kw"), xm.group(1), chain, acc[0], q[4], q[5]))
    return rows


def lint_scope_exits(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0064 item 6 (the lint half of `Every way out exhales exactly what the allocation gave`): see the comment above."""
    field_types = array_field_types(src)
    by_name = {d.name: d for d in decls if d.kind == "method"}
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        allocs: dict[str, tuple[int, int | None, set[tuple[tuple[int, ...], str]]]] = {}
        for n in range(d.line, d.end_line + 1):
            m = ALLOC_ARR_CALL_RE.match(src.code[n - 1])
            ad = by_name.get(m.group("alloc")) if m else None
            if m is None or ad is None or not any("member(" in src.code[k - 1] for k in range(ad.line, ad.end_line + 1)):
                continue
            why, leaves = aggregate_array_problem(src, ad, True, field_types)
            if why is not None:
                continue  # `lint_array_allocations` reports it
            want = {(chain, f"{kind}_{t}") for chain, t in leaves.items() for kind in "vd"}
            count = int(m.group("n")) if re.fullmatch(r"[0-9]+", m.group("n").strip()) else None
            allocs[m.group("v")] = (n, count, want)
        if not allocs:
            continue
        rows = [r for r in _aggregate_leaf_rows(src, d) if r[2] in allocs]
        # runs of consecutive rows of one kind over one array
        runs: list[list[tuple[int, str, str, tuple[int, ...], str, str, str]]] = []
        for r in rows:
            if runs and runs[-1][-1][1] == r[1] and runs[-1][-1][2] == r[2] \
                    and all(not src.code[k - 1].strip() for k in range(runs[-1][-1][0] + 1, r[0])):
                runs[-1].append(r)
            else:
                runs.append([r])
        exits: dict[str, int] = {}
        lifetime: set[int] = set()  # the first line of each marked run that stands directly before an exit's exhales (the loops' back-edge rows are others)
        for run in runs:
            if run[0][1] != "exhale":
                continue
            prev = run[0][0] - 1
            while prev >= d.line and not src.code[prev - 1].strip():
                prev -= 1
            lm = LIVE_WRITE_RE.match(src.code[prev - 1]) if prev >= d.line else None
            if lm is not None and lm.group("v") == run[0][2]:
                prev -= 1
                while prev >= d.line and not src.code[prev - 1].strip():
                    prev -= 1
            for rr in runs:
                if rr[-1][0] == prev and rr[0][1] == "assert" and rr[0][2] == run[0][2]:
                    lifetime.add(rr[0][0])
        for i, run in enumerate(runs):
            kind, x = run[0][1], run[0][2]
            at, count, want = allocs[x]
            if kind == "assert" and run[0][0] not in lifetime:
                continue
            got = [(r[3], r[4]) for r in run]
            what = "the scope exit's `exhale`s" if kind == "exhale" else "the lifetime rows before a scope exit"
            if len(set(got)) != len(got):
                add(run[0][0], f"{what} over `{x}` give a (receiver, field) row twice (decision 0064 item 6)")
            elif set(got) != want:
                missing = sorted(want - set(got))
                extra = sorted(set(got) - want)
                add(run[0][0], f"{what} over `{x}` are not the rows of its allocation (line {at}): "
                    + (f"missing {missing[0]}" if missing else f"unexpected {extra[0]}") + " (decision 0064 item 6: every way out exhales exactly what the allocation gave)")
            bad = [r for r in run if r[5] != "0" or (count is not None and r[6] != str(count))]
            if bad:
                add(bad[0][0], f"{what} over `{x}` range over `{bad[0][5]} <= k && k < {bad[0][6]}`, not the allocation's `0 <= k && k < {count}` (line {at}; decision 0064 item 6)")
            if kind != "exhale":
                continue
            exits[x] = exits.get(x, 0) + 1
            prev = run[0][0] - 1
            while prev >= d.line and not src.code[prev - 1].strip():
                prev -= 1
            lm = LIVE_WRITE_RE.match(src.code[prev - 1]) if prev >= d.line else None
            if lm is not None and lm.group("v") == x:
                prev -= 1
                while prev >= d.line and not src.code[prev - 1].strip():
                    prev -= 1
            marked = next((r for r in rows if r[0] == prev), None)
            if marked is None or marked[1] != "assert" or marked[2] != x:
                add(run[0][0], f"the scope exit's `exhale`s over `{x}` are not directly preceded by its lifetime rows (marked `assert`s over the same rows, decision 0064 item 6)")
            else:
                chain_run = next((rr for rr in runs if rr[-1][0] == prev), None)
                if chain_run is None or {(r[3], r[4]) for r in chain_run} != set(got):
                    add(run[0][0], f"the lifetime rows before the exhale over `{x}` are not the same (receiver, field) rows as the exhale's (decision 0064 item 6)")
        # the audit's b01x: every `exhale forall` of the exits blanked.  A method with a missing-return `assert false` is no help: the liveness cell is
        # exhaled at each exit after the rows, so each `exhale acc(lvloc(block_of(v)).lv)` must stand directly after the whole run of its exhales
        run_ends = {run[-1][0]: run[0][2] for run in runs if run[0][1] == "exhale"}
        lv_exits: dict[str, int] = {}
        for n in range(d.line, d.end_line + 1):
            lm = LIVE_EXHALE_RE.match(src.code[n - 1])
            if lm is None or lm.group("v") not in allocs:
                continue
            x = lm.group("v")
            lv_exits[x] = lv_exits.get(x, 0) + 1
            prev = n - 1
            while prev >= d.line and not src.code[prev - 1].strip():
                prev -= 1
            if run_ends.get(prev) != x:
                add(n, f"the exit's `exhale` of the liveness cell of `{x}` is not directly preceded by the exhales of its rows (decision 0064 item 6: every way out exhales "
                       "exactly what the allocation gave)")
        end_label = next((n for n in range(d.end_line, d.line - 1, -1) if END_LABEL_RE.match(src.code[n - 1])), None)
        for x, (at, _count, _want) in allocs.items():
            if end_label is None:
                # a hand-written method (no `label l_end`): a path that never leaves has an `assert false` and no exit
                never = any("assert false" in src.code[k - 1] for k in range(d.line, d.end_line + 1))
                if exits.get(x, 0) == 0 and lv_exits.get(x, 0) == 0 and not never:
                    add(at, f"the array `{x}` allocated here is never exhaled at a scope exit (decision 0064 item 6: every way out exhales exactly what the allocation gave)")
                continue
            # an emitted method (audit round C1, finding 6): the missing-return `assert false` that ends every method with a result hides nothing.  Walking
            # from the allocation, the liveness cell's exhale (the last line of an exit) stands between the allocation, or the previous `goto l_end`, and
            # each `goto l_end`, and before the `label l_end` that ends the method; a path that leaves without it is refused.  An array of an inner block
            # has its exit at the block's end, which stands before a later `goto l_end` outside the block.
            have = False
            seen_goto = False
            for n in range(at + 1, end_label + 1):
                lm = LIVE_EXHALE_RE.match(src.code[n - 1])
                if lm is not None and lm.group("v") == x:
                    have = True
                    continue
                is_goto = GOTO_END_RE.match(src.code[n - 1]) is not None
                if not is_goto and n != end_label:
                    continue
                if n == end_label:
                    # after a `goto l_end` the missing-return `assert false` that ends a method with a result needs no exit of its own (the paths ended at
                    # the gotos); with no `goto l_end` at all the end is the only way out
                    prev = n - 1
                    while prev > at and not src.code[prev - 1].strip():
                        prev -= 1
                    if seen_goto and src.code[prev - 1].strip() == "assert false":
                        continue
                if not have:
                    where = "this `goto l_end`" if is_goto else "the end of the method"
                    add(n, f"the array `{x}` allocated at line {at} is not exhaled before {where}: the exit's exhale of its liveness cell is missing "
                           "(decision 0064 item 6: every way out exhales exactly what the allocation gave)")
                have = False
                seen_goto = seen_goto or is_goto


# Decision 0064 item 7: the length facts a range atom over structures states are tied to the leaves its permission rows name.  In a method's `requires` or
# `ensures`, the rows over one range of one pointer (the same clause kind, block and bounds): a row over a leaf receiver `loc(member(...), 0)` needs the
# length-facts row of that range to state the member block of the leaf and of every nested structure member above it, and a leaf with no length-facts row at
# all is refused (a permission row over a member block of unknown length is not a well-formed atom).
def lint_range_length_facts(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    for d in decls:
        if d.kind != "method" or d.name.startswith((ARR_ALLOC_PREFIX, ARR_ZERO_PREFIX, ARR_COPY_PREFIX, ALLOC_PREFIX)) or d.name == PROGRAM_START:
            continue
        lens: dict[tuple[str, str, str, str], tuple[int, set[tuple[int, ...]]]] = {}
        leaves: dict[tuple[str, str, str, str], list[tuple[int, tuple[int, ...]]]] = {}
        for n in range(d.line, (d.body_line or d.end_line + 1)):
            m = QUANT_STMT_RE.match(src.code[n - 1])
            if m is None or m.group("kw") not in ("requires", "ensures"):
                continue
            guard, body = split_guard(m.group("rest"))
            if body is None:
                continue
            q = parse_range_quantifier_full(body)
            if q is not None:
                cs = _squash(q[3])
                if not q[2] and "len(member(" in cs:
                    keys, why = range_lens_members(cs, f"loc({_squash(q[1])},{q[0]})")
                    if why is None and keys is not None:
                        xm = BLOCK_BASE_RE.match(_squash(q[1]))
                        lens[(m.group("kw"), xm.group(1) if xm else "", q[4], q[5])] = (n, set(keys))
                continue
            q = parse_leaf_row(body)
            if q is None:
                continue
            chain = parse_struct_chain(q[6], f"loc({_squash(q[1])},{q[0]})")
            xm = BLOCK_BASE_RE.match(_squash(q[1]))
            if chain and xm:
                leaves.setdefault((m.group("kw"), xm.group(1), q[4], q[5]), []).append((n, chain))
        for key, rows in leaves.items():
            have = lens.get(key)
            if have is None:
                add(rows[0][0], f"the rows over the leaves of the elements of `{key[1]}` in this `{key[0]}` have no length-facts row over the same range "
                                "(decision 0064 item 7: `len(member(<chain>, j)) == 1` for each member block of the element)")
                continue
            for line, chain in rows:
                need = [chain[:i] for i in range(1, len(chain) + 1)]
                missing = [c for c in need if c not in have[1]]
                if missing:
                    add(line, f"the length facts (line {have[0]}) of `{key[1]}` do not state the member block {missing[0]} of this leaf {chain} (decision 0064 item 7)")
                    break


ARR_COPY_CALL_RE = re.compile(rf"^\s*(?P<name>copy_arr_[A-Za-z0-9_$']+)\s*\((?P<args>.*)\)\s*$")


def array_copy_call_problem(src: Source, d: Decl, n: int, t: str, args: list[str]) -> str | None:
    """Decision 0063 item 13: a call `copy_arr_<t>(D, S, N, RD)` is preceded, in its method, by an explicit row for every `requires` of the trusted copy, so
    that no omitted row can let Silicon prove a copy it did not check: `RD` is a local assigned the minimum of the value and the flag permission of the
    source's first slot `S` (`(perm(S.v_t) < perm(S.d_t)) ? perm(S.v_t) : perm(S.d_t)`, the amount the caller holds), `assert none < RD`, the source's
    value and flag permissions at `RD` and its flags over `[0, N)` and the destination's value and flag permissions over `[0, N)` (marked `forall` rows
    over `block_of(S)` and `block_of(D)`), and, immediately before the call, `assert block_of(D) != block_of(S)` (the `internal` row).  The call is
    alone or the only statement of an `if (D != S) {` block (the guard of a copy that may be a self-copy)."""
    if len(args) != 4 or not all(re.fullmatch(r"[A-Za-z_$][\w$']*", a) for a in (args[0], args[1], args[3])) or not re.fullmatch(r"[1-9][0-9]*", args[2]):
        return "its arguments are not three variables and a literal count"
    dst, s_, cnt, rd = args
    body = [(k, norm_viper(src.code[k - 1])) for k in range(d.body_line, n) if src.code[k - 1].strip()]
    texts = {c for _k, c in body}

    def q(blk: str, tail: str) -> str:
        return norm_viper(f"assert forall k: Int :: {{ loc(block_of({blk}), k) }} ((0 <= k) && (k < {cnt})) ==> {tail}")
    cs, cd = f"loc(block_of({s_}), k)", f"loc(block_of({dst}), k)"
    first = norm_viper(f"{rd} := (perm({s_}.v_{t}) < perm({s_}.d_{t})) ? perm({s_}.v_{t}) : perm({s_}.d_{t})")
    want = {
        "the amount `RD` as the minimum over the source's first slot": first,
        "`assert none < RD`": norm_viper(f"assert none < {rd}"),
        "the source's value permission at `RD`": q(s_, f"acc({cs}.v_{t}, {rd})"),
        "the source's flag permission at `RD`": q(s_, f"acc({cs}.d_{t}, {rd})"),
        "the source's flags": q(s_, f"{cs}.d_{t}"),
        "the destination's value permission": q(dst, f"acc({cd}.v_{t})"),
        "the destination's flag permission": q(dst, f"acc({cd}.d_{t})"),
    }
    for what, text in want.items():
        if text not in texts:
            return f"no row for {what} before the call"
    prev = [(k, c) for k, c in body if k < n]
    if not prev or prev[-1][1] != norm_viper(f"assert block_of({dst}) != block_of({s_})"):
        return f"the line before the call is not `assert block_of({dst}) != block_of({s_})` (the internal row)"
    if len(prev) >= 2 and prev[-2][1].startswith("if("):
        if prev[-2][1] != norm_viper(f"if ({dst} != {s_}) {{"):
            return f"the call's guard is not `if ({dst} != {s_}) {{`"
        nxt = [c for k, c in [(k, norm_viper(src.code[k - 1])) for k in range(n + 1, d.end_line + 1) if src.code[k - 1].strip()]][:1]
        if nxt != ["}"]:
            return "the guarded block holds more than the call"
    return None


def lint_array_copy_calls(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """The call sites of the trusted `copy_arr_<t>` (decision 0063 item 13; `array_copy_call_problem`)."""
    pinned = {d.name for d in decls if is_pinned_array_copy(src, d)}
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        for n in range(d.body_line, d.end_line + 1):
            m = ARR_COPY_CALL_RE.match(src.code[n - 1])
            if m is None or m.group("name") not in pinned:
                continue
            args = [a.strip() for a in m.group("args").split(",")]
            why = array_copy_call_problem(src, d, n, m.group("name")[len(ARR_COPY_PREFIX):], args)
            if why is not None:
                add(n, f"`{m.group('name')}` is called without its rows (decision 0063 item 13): {why}")


# The quantified rows (decision 0059 items 7 and 8).  Viper refuses `perm` under a quantifier, so a permission check over a
# range is an `assert forall k :: { loc(B, k) } lo <= k && k < hi ==> acc(loc(B, k).f)` row with exactly one field, whose
# `insufficient.permission` is the row's own failure (the map's `assert.failed[insufficient.permission]`); B, lo and hi read no
# heap, so nothing else on the line can produce that reason.  `forall` appears nowhere else but the two trusted contracts
# above, the `exhale` of the same shape, `domain Block`'s axioms and item 8's four loop conjuncts; `old[label]` only in two of those.
# Decision 0060 item 16 adds a range atom's rows in a method's `requires` and `ensures` and their pure `assert` rows
# (`range_atom_row_problem`), a guard `g ==> ` on any of them but a marked row, and the method's `Perm` formal as the amount of a
# permission row or a loop's permission conjunct.
FORALL_RE = re.compile(r"(?<![\w$'])forall(?![\w$'])")
OLD_LABEL_RE = re.compile(r"(?<![\w$'])old\s*\[")
QUANT_STMT_RE = re.compile(r"^\s*(?P<kw>assert|exhale|invariant|requires|ensures)\s+(?P<rest>.*)$")
QUANT_HEAD_RE = re.compile(r"^forall\s+(?P<k>[A-Za-z_$][\w$']*)\s*:\s*Int\s*::\s*\{\s*(?P<trigger>[^{}]*?)\s*\}\s*(?P<body>.*)$", re.S)
HEAP_READ_BASE = r"\.\s*[A-Za-z_$]|(?<![\w$'])(?:perm|acc|old|forperm|forall)(?![\w$'])"
HEAP_READ_RE = re.compile(HEAP_READ_BASE + r"|(?<![\w$'.])pf_[\w$']*\s*\(")  # an application of a `pure` function counts as a read (decision 0062)
HEAP_READ_NO_APPLICATION_RE = re.compile(HEAP_READ_BASE)
INT_BOUND_RE = re.compile(r"^\(*\s*-?\s*\d+\s*\)*$")


def _squash(text: str) -> str:
    return "".join(text.split())


def _heap_free(text: str) -> bool:
    return HEAP_READ_RE.search(text) is None


def parse_range_quantifier_full(text: str):
    """`forall k: Int :: { loc(block_of(X), k) } lo <= k && k < hi ==> CONSEQUENT` (what follows `assert`, `exhale`, `invariant`,
    `requires` or `ensures`, after a guard): (k, B, antecedent extras, consequent, lo, hi), the extras being the conjuncts after
    the two range conjuncts; None if it is not that shape.  B must be `block_of(X)` for a variable X (decision 0060 item 8, S44
    F-b1: Silicon refuses a trigger `loc(block_of(e), k)` whose `e` holds an operator, so the base of a range is bound to a local
    first), and B, lo and hi read no heap.  The antecedent's `lo <= k && k < hi` are its first two conjuncts."""
    head = QUANT_HEAD_RE.match(text.strip())
    if head is None:
        return None
    k = head.group("k")
    trig = re.match(r"^loc\((?P<b>.+),\s*" + re.escape(k) + r"\s*\)$", head.group("trigger").strip(), re.S)
    if trig is None:
        return None
    b = trig.group("b").strip()
    if BLOCK_BASE_RE.match(_squash(b)) is None:
        return None
    parts = _split_top(head.group("body"), "==>")
    if len(parts) != 2:
        return None
    try:
        ante = [_squash(x) for x in _flatten_and(parts[0])]
    except ValueError:
        return None
    if len(ante) < 2:
        return None
    lo = re.match(r"^(?P<lo>.+)<=" + re.escape(k) + r"$", ante[0])
    hi = re.match(r"^" + re.escape(k) + r"<(?P<hi>.+)$", ante[1])
    if lo is None or hi is None or not (_heap_free(b) and _heap_free(lo.group("lo")) and _heap_free(hi.group("hi"))):
        return None
    return k, b, ante[2:], _strip_wrap(parts[1]), lo.group("lo"), hi.group("hi")


def parse_range_quantifier(text: str):
    """The first four fields of parse_range_quantifier_full: (k, B, antecedent extras, consequent), or None."""
    q = parse_range_quantifier_full(text)
    return None if q is None else q[:4]


def parse_leaf_row(text: str):
    """Decision 0064 item 17: a quantified row over a LEAF of the elements of an array of structures, `forall k: Int :: { R } lo <= k && k < hi ==>
    CONSEQUENT` with R the leaf receiver, the canonical chain `loc(member(<chain>, j), 0)` (decision 0043 item 13's grammar) rooted at the slot
    `loc(block_of(X), k)` of element `k` for a VARIABLE X, with at least one member step: (k, `block_of(X)`, antecedent extras, consequent, lo, hi, R).
    The same fields as `parse_range_quantifier_full` and the receiver as a seventh; the older parser never sees this shape (its callers rebuild the
    cell as `loc(b, k)`), so a leaf row is accepted only where a rule asks for it.  A second binder (a leaf that is an array member, or a row of a
    matrix) is not read: Part E's."""
    head = QUANT_HEAD_RE.match(text.strip())
    if head is None:
        return None
    k = head.group("k")
    trig = norm_viper(head.group("trigger"))
    xs = re.findall(r"(?<![\w$'])block_of\(([A-Za-z_$][\w$']*)\)", trig)
    if len(xs) != 1 or trig.count("block_of(") != 1:
        return None
    if not parse_struct_chain(trig, f"loc(block_of({xs[0]}),{k})"):
        return None
    parts = _split_top(head.group("body"), "==>")
    if len(parts) != 2:
        return None
    try:
        ante = [_squash(x) for x in _flatten_and(parts[0])]
    except ValueError:
        return None
    if len(ante) < 2:
        return None
    lo = re.match(r"^(?P<lo>.+)<=" + re.escape(k) + r"$", ante[0])
    hi = re.match(r"^" + re.escape(k) + r"<(?P<hi>.+)$", ante[1])
    if lo is None or hi is None or not (_heap_free(lo.group("lo")) and _heap_free(hi.group("hi"))):
        return None
    return k, f"block_of({xs[0]})", ante[2:], _strip_wrap(parts[1]), lo.group("lo"), hi.group("hi"), trig


def _row_acc(q) -> tuple[str, str | None] | None:
    """`_range_acc` of a parsed quantified row `q` (a range row, or a leaf row whose cell is its receiver)."""
    return _range_acc(q[3], q[0], q[1], q[6] if len(q) > 6 else None)


def split_guard(rest: str) -> tuple[str | None, str | None]:
    """(guard, quantifier text) of what follows the keyword of a clause or statement: `forall ...`, `(forall ...)`, `g ==> forall ...`
    or `(g) ==> (forall ...)`; the guard is None for an unguarded quantifier, and the quantifier text None when the text is not one
    (decision 0060 item 4: the guard stands outside the quantifier, one conjunct per line)."""
    text = rest.strip()
    inner = _strip_wrap(text)
    if FORALL_RE.match(inner):
        return None, inner
    parts = _split_top(text, "==>")
    if len(parts) < 2:
        return None, None
    body = _strip_wrap(text[len(parts[0]) + len("==>"):])
    if not FORALL_RE.match(body):
        return None, None
    return parts[0].strip(), body


def _std_type_range(cons: str, cell: str, t: str) -> bool:
    """Whether `cons` is the range of the value `cell.v_t`: the standard range of an integer type (8 to 128 bits, signed or
    unsigned: a postcondition of a trusted method is an assumption, so the bounds are not free) or, for a pointer element, WF of
    its value.  As a cell atom's range row is read (`view_problem`): an integer field whose type has a fixed range
    (`VIEW_INT_RANGES`) takes exactly that one, another integer field a standard range, an enumeration's field (`enum_<name>`)
    any `lo <= 0 <= hi` (its own range, decision 0018)."""
    try:
        raw = _flatten_and(cons)
        parts = [_squash(x) for x in raw]
    except ValueError:
        return False
    v = f"{_squash(cell)}.v_{t}"
    if t.startswith("ptr_"):
        if len(parts) == 1:
            return norm_viper(parts[0]) == norm_viper(_view_wf(v))
        # audit round C1, finding 4: the element of a range over `T **` or the leaf of a range over a structure with a `struct S *` member, S a structure:
        # WF followed by the guarded WF_S conjunct `v != null && offset_of(v) < len(block_of(v)) ==> len(member(<chain>, k)) == 1 && ...` (the leaf row of
        # `check_view_wf`, decision 0043 item 8); the guard and the length facts are the ones `struct_wf_lens_problem` accepts.  Audit round C2, finding 1:
        # when S has an array member of structures the facts hold its count and the quantifier over its elements, with a binder the slot does not
        # mention (`struct_lens_conjuncts_problem`, decision 0064 item 3)
        if len(parts) != 2 or norm_viper(parts[0]) != norm_viper(_view_wf(v)):
            return False
        sides = _split_top(_strip_wrap(raw[1]), "==>")
        if len(sides) != 2 or norm_viper(sides[0]) != norm_viper(f"{v} != null && offset_of({v}) < len(block_of({v}))"):
            return False
        try:
            lens = _flatten_and(sides[1])
        except ValueError:
            return False
        return bool(lens) and struct_lens_conjuncts_problem(lens, v) is None
    if len(parts) != 2:
        return False
    lo = re.match(r"^\(*(-?\d+)\)*<=" + re.escape(v) + r"$", parts[0])
    hi = re.match(r"^" + re.escape(v) + r"<=\(*(-?\d+)\)*$", parts[1])
    if lo is None or hi is None:
        return False
    low, high = int(lo.group(1)), int(hi.group(1))
    if t.startswith("enum_"):
        return low <= 0 <= high
    if t in VIEW_INT_RANGES:
        return (low, high) == VIEW_INT_RANGES[t]
    return (low, high) in VIEW_STANDARD_RANGES


# Decision 0064 items 3, 9 and 17 (Part C2): the length facts of an array of structures that a method's `requires` or `ensures` states over a block term
# (`GB_g()` of a global, `member(<chain>, k)` of a structure's array member) are tied to the leaves its permission rows name, as `lint_range_length_facts`
# ties a range over a pointer: a row over a leaf receiver needs the length-facts row of the same range and clause kind to state the member block of the
# leaf and of every nested structure above it, and a leaf with no length-facts row at all is refused.
def lint_block_length_facts(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    for d in decls:
        if d.kind != "method" or d.name.startswith((ARR_ALLOC_PREFIX, ARR_ZERO_PREFIX, ARR_COPY_PREFIX, ALLOC_PREFIX)) or d.name == PROGRAM_START:
            continue
        lens: dict[tuple[str, str, str, str], tuple[int, set[tuple[int, ...]]]] = {}
        leaves: dict[tuple[str, str, str, str], list[tuple[int, tuple[int, ...]]]] = {}
        for kw, n, text in clause_texts(src, d):
            if kw not in ("requires", "ensures"):
                continue
            guard, body = split_guard(text)
            if body is None:
                continue
            ql = parse_block_leaf_row(body)
            if ql is not None:
                chain = parse_struct_chain(ql["recv"], f"loc({ql['b']},{ql['k']})")
                if chain:
                    leaves.setdefault((kw, ql["b"], ql["lo"], ql["hi"]), []).append((n, chain))
                continue
            qb = parse_block_row(body)
            if qb is not None and "len(member(" in qb["cons"]:
                keys, why = range_lens_members(qb["cons"], f"loc({qb['b']},{qb['k']})")
                if why is None and keys is not None:
                    lens[(kw, qb["b"], qb["lo"], qb["hi"])] = (n, set(keys))
        for key, rows in leaves.items():
            have = lens.get(key)
            if have is None:
                add(rows[0][0], f"the rows over the leaves of the elements of `{key[1]}` in this `{key[0]}` have no length-facts row over the same range "
                                "(decision 0064 items 3 and 9: `len(member(<chain>, j)) == 1` for each member block of the element)")
                continue
            for line, chain in rows:
                need = [chain[:i] for i in range(1, len(chain) + 1)]
                missing = [c for c in need if c not in have[1]]
                if missing:
                    add(line, f"the length facts (line {have[0]}) of `{key[1]}` do not state the member block {missing[0]} of this leaf {chain} (decision 0064 items 3 and 9)")
                    break


def range_lens_members(cons: str, slot: str) -> tuple[list[tuple[int, ...]] | None, str | None]:
    """(the member blocks, why not) of the length-facts consequent `cons` (squashed) of a range atom over structures: `len(member(<chain>, j)) == 1`
    for each member block of the element at `slot` (a chain of decision 0043 item 13 rooted at the slot, a literal j each), a member of a member only
    inside a nested structure member, no block twice, and the slot facts `A != B` (decision 0043 item 4) either exactly the pairs of the root or a
    nested slot and a nested slot below it, or none (the range atom's rows state the lengths only: S48 (c2))."""
    try:
        parts = [_squash(x) for x in _flatten_and(cons)]
    except ValueError:
        return None, "the length facts are not a conjunction"
    members = [x for x in parts if "!=" not in x]
    facts = [x for x in parts if "!=" in x]
    keys: list[tuple[int, ...]] = []
    for part in members:
        m = re.fullmatch(r"len\((?P<m>.*)\)==(?P<n>[0-9]+)", part)
        key = parse_struct_member(m.group("m"), slot) if m else None
        if key is None or int(m.group("n")) != 1:
            return None, f"`{part}` is not `len(member(<chain>, <literal>)) == 1` with a chain rooted at the slot `{slot}`"
        if key in keys:
            return None, f"the member block {key} twice"
        keys.append(key)
    if not keys:
        return None, "no member block"
    kset = set(keys)
    for key in kset:
        if key[:-1] and key[:-1] not in kset:
            return None, f"the member block {key} is not inside a nested struct member"
    if facts:
        why = struct_wf_lens_problem(parts, slot)
        if why is not None:
            return None, why
    return keys, None


def range_lens_problem(cons: str, cell: str) -> str | None:
    """Why `cons` is not the length facts of a range atom over structures (`range_lens_members`), or None."""
    return range_lens_members(cons, cell)[1]


def _mentions_name(text: str, name: str) -> bool:
    """Whether the identifier `name` occurs in the normalized term `text` (a whole identifier: `k` is not in `k1`)."""
    return re.search(r"(?<![\w$'])" + re.escape(name) + r"(?![\w$'])", text) is not None


def struct_lens_conjuncts_problem(lens: list[str], v: str) -> str | None:
    """The conjuncts `lens` (unsquashed) of the guarded length facts of the pointer value `v` (a variable, or a slot's leaf `loc(block_of(X), k).v_ptr_S`
    of a range or of an element of an array): the structure's own `len(member(v, j)) == 1` and slot facts, and for each array member of structures its
    `len(member(v, j)) == N` and the quantifier over its elements `forall b :: { loc(member(v, j), b) } 0 <= b && b < N ==> <the element's length
    facts>` (decision 0064 items 3 and 17).  **The binder `b` must not occur in `member(v, j)`** (audit round C2, finding 1: `v` under a range's own
    `forall k` mentions `k`, and an inner `forall k` would capture it; the lowering takes the first of k1, k2, ... that the slot does not mention):
    a binder free in the slot expression is refused, and with it a reuse of the outer binder's name.  Returns None when the facts are exactly that,
    else the reason."""
    counts: dict[tuple[int, ...], int] = {}
    others: list[str] = []
    for part in lens:
        if not _strip_wrap(part).startswith("forall"):
            others.append(_squash(part))
            continue
        qb = parse_block_row(part)
        key = parse_struct_member(qb["b"], v) if qb is not None else None
        if qb is None or key is None or qb["lo"] != "0":
            return f"`{_squash(part)}` is not a quantifier over the elements of an array member of `{v}` from 0"
        if _mentions_name(qb["b"], qb["k"]):
            return (f"the binder `{qb['k']}` of the quantifier over the member {key} occurs in its slot expression `{qb['b']}`: an inner quantifier "
                    "that reuses a name its slot mentions captures it (decision 0064 item 3, a length quantifier under another quantifier)")
        keys, why = range_lens_members(qb["cons"], f"loc({qb['b']},{qb['k']})")
        if why is not None:
            return f"the length facts of the elements of the member {key}: {why}"
        if key in counts:
            return f"the quantifier of the member {key} twice"
        counts[key] = int(qb["hi"])
    why = struct_wf_lens_problem(others, v, counts)
    if why is not None:
        return why
    stated = {parse_struct_member(m.group(1), v) for x in others if (m := re.fullmatch(r"len\((.*)\)==[0-9]+", x))}
    for key, n in counts.items():
        if key not in stated:
            return f"the member {key} has a quantifier over its {n} elements and no `len(member(<chain>, <literal>)) == {n}`"
    return None


def block_clause_problem(kw: str, rest: str, gbs: set[str], arrays: dict[str, int], roots: set[str], lens: dict[str, set[int]],
                         perm_formals: frozenset[str]) -> str | None:
    """Decision 0064 item 15 (heap storage, owners, markers): the `requires` or `ensures` of a method that is one `owns_block` or `owns` site's whole
    part written as a conjunction (not as the rows of one array atom): `P != null && <WF> && offset_of(P) == 0 && acc(loc(block_of(P), -1).tok) && ...`
    under `!(P == null) ==>` (or `!(((P == null) ? 1 : 0) != 0) ==>`, as a parameter's site writes it); `P` a formal, a result, or (a global's cell
    atom `owns(g)`: `loc(GB_g(), 0) != null && ... && acc(loc(GB_g(), 0).v_ptr_<t>) ...`) the global's cell; and, as a conjunct of its own under the
    guard `!(L == null) ==>`, the same shape for the block of a pointer `L` the site's owner closure reaches (a canonical load).  Where a block holds an
    array leaf of scalars its quantified conjuncts are rows over the block TERM of a member of that block's pointer (`member(<chain>, k)` over a
    canonical chain rooted at the pointer, or at one load), each exactly a row `block_row_problem` accepts (`acc` of the value or the flag, the flags
    set, the type's range under the flag or alone), over `0 <= k && k < N` with the N the clause's own length facts state.  Returns None when `rest`
    is not such a clause (another rule looks at it), "" when every quantified conjunct is a row of that grammar, else the reason."""
    text = _strip_wrap(rest.strip())
    sides = _split_top(text, "==>")
    if len(sides) > 2:
        return None
    guard_p = None
    if len(sides) == 2:
        # `!(P == null)`, or as a parameter's site writes it, `!(((P == null) ? 1 : 0) != 0)`; the pointer may be a load
        gm = re.fullmatch(r"!(.+?)==null|!(?:\()?(.+?)==null\?1:0\)?!=0", norm_viper(sides[0]))
        if gm is None:
            return None
        guard_p = gm.group(1) or gm.group(2)
    body = sides[-1]
    try:
        conj = _flatten_and(body)
    except ValueError:
        return None
    pm = re.fullmatch(r"(.+)!=null", norm_viper(conj[0]))
    if pm is None:
        return None
    p = pm.group(1)
    all_roots = roots | {f"loc({g}(),0)" for g in gbs}
    is_load = load_root(p, all_roots) is not None and load_root(p, all_roots)[0] == p
    if p not in all_roots and not is_load:
        return None
    # applicable: a block with its token, or a global's cell (the atom `owns(g)`) holding a pointer
    has_tok = any(norm_viper(c) == f"acc(loc(block_of({p}),-1).tok)" for c in conj)
    has_cell = p.startswith("loc(GB_") and any(norm_viper(c).startswith(f"acc({p}.v_ptr_") for c in conj)
    # an object site `owns(*r)` of a structure with an owning member (decision 0064 item 15, question 293; il-analyst D3): the top-level pointer is no block
    # (no token, no global cell) and its clause holds, as conjuncts of their own, the block clauses of the pointers its closure reaches
    has_nested = any(FORALL_RE.search(c) is not None and len(_split_top(_strip_wrap(c.strip()), "==>")) == 2 for c in conj)
    if not (has_tok or has_cell or has_nested):
        return None
    if guard_p is not None and guard_p != p:
        return "the guard of an `owns_block` clause names another pointer than its block part"
    for c in conj:
        t = _strip_wrap(c).strip()
        if FORALL_RE.match(t):
            bq = parse_block_row(t)
            if bq is None:
                return ("a quantified conjunct of an `owns_block` clause is not a row `forall k: Int :: { loc(B, k) } 0 <= k && k < N ==> ...` over a block "
                        "term B with literal bounds (decision 0064 item 15)")
            bt = block_term(bq["b"], gbs, roots, loads=True)
            if bt is None or bt[0] != "member" or not (bt[1] == p or bt[1].startswith(f"loc(member({p},")):
                return f"the block `{bq['b']}` of a quantified conjunct is not a member block of the clause's own pointer `{p}`"
            why = block_row_problem(kw, None, bq, gbs, arrays, roots, lens, perm_formals)
            if why:
                return why
        elif FORALL_RE.search(t):
            nested = block_clause_problem(kw, t, gbs, arrays, roots, lens, perm_formals)
            if nested is None:
                return "a conjunct of an `owns_block` clause holds a quantifier that is neither a row nor the block clause of a pointer its closure reaches"
            if nested:
                return nested
    return ""


def struct_wf_member_array_row(rest: str, expect_v: str | None = None) -> str | None:
    """`WF(v) && (v != null && offset_of(v) < len(block_of(v)) ==> <lens>)`, the well-formedness row of a pointer to a structure with an array member
    whose elements are structures (decision 0064 items 3, 7 and 17, as built in Part C2): the length facts the guarded conjunct states are the structure's
    own (`len(member(v, j)) == 1`, `== N` for the array member of N elements) and, for each such array member, the quantifier over its elements
    `forall k :: { loc(member(v, j), k) } 0 <= k && k < N ==> <the element's length facts>` of the same N.  Returns None when `rest` is not that shape (the
    caller looks at it as another row), else "" when it is exactly it, else the reason."""
    try:
        parts = _flatten_and(rest)
    except ValueError:
        return None
    if len(parts) != 2:
        return None
    head = re.match(r"\(*([A-Za-z_$][\w$']*|loc\(GB_[\w$']+\(\),0\))==null", _squash(parts[0]))
    if head is None or norm_viper(parts[0]) != norm_viper(_view_wf(head.group(1))):
        return None
    v = head.group(1)
    sides = _split_top(_strip_wrap(parts[1]), "==>")
    if len(sides) != 2 or "forall" not in sides[1]:
        return None
    if expect_v is not None and v != expect_v:
        return f"the well-formedness row is about `{v}`, not `{expect_v}`"
    if norm_viper(sides[0]) != norm_viper(f"{v} != null && offset_of({v}) < len(block_of({v}))"):
        return "the guard of the structure's length facts is not `v != null && offset_of(v) < len(block_of(v))`"
    try:
        lens = _flatten_and(sides[1])
    except ValueError:
        return "the length facts are not a conjunction"
    why = struct_lens_conjuncts_problem(lens, v)
    return "" if why is None else why


def wf_store_assert_problem(rest: str) -> str | None:
    """`V != null ==> !(!(perm(lvloc(block_of(V)).lv) > none) || !(ROW))`, the check of a pointer value `V` a body stores (an initializer or a store of a
    pointer to a structure with an array member of structures, decision 0043 item 8 with decision 0064 item 3): ROW is the well-formedness row of the
    same V (`struct_wf_member_array_row`, its length quantifier under `struct_lens_conjuncts_problem`).  None when `rest` is not that shape or holds no
    such row, "" when it is exactly it, else the reason (audit round C2, finding 1: the kept Viper of a local array of `struct T *`)."""
    parts = _split_top(rest.strip(), "==>")
    if len(parts) < 2 or "forall" not in rest:
        return None
    g = re.fullmatch(r"([A-Za-z_$][\w$']*)!=null", norm_viper(parts[0]))
    if g is None:
        return None
    v = g.group(1)
    body = _strip_wrap("==>".join(parts[1:]).strip())
    m = re.fullmatch(r"!\((.*)\)", body, re.S)
    if m is None:
        return None
    ors = _split_top(m.group(1), "||")
    if len(ors) != 2 or norm_viper(ors[0]) != norm_viper(f"!(perm(lvloc(block_of({v})).lv) > none)"):
        return None
    neg = re.fullmatch(r"!\((.*)\)", _strip_wrap(ors[1].strip()), re.S)
    if neg is None:
        return None
    why = struct_wf_member_array_row(neg.group(1), v)
    return None if why is None else why


def _is_struct_lens_quantifier(text: str) -> bool:
    """`v != null && offset_of(v) < len(block_of(v)) ==> forall k :: { loc(member(<chain>, j), k) } 0 <= k && k < N ==> <the element's length facts>`: one
    conjunct of the well-formedness of a pointer to a structure with an array member of structures, as an `f__wf` prior holds it (decision 0064 items 3
    and 17); the same quantifier `struct_wf_member_array_row` reads inside the one row."""
    guard, body = split_guard(_strip_wrap(text))
    if guard is None or body is None:
        return False
    g = re.fullmatch(r"([A-Za-z_$][\w$']*|loc\(GB_[\w$']+\(\),0\))!=null&&offset_of\(\1\)<len\(block_of\(\1\)\)", norm_viper(guard))
    qb = parse_block_row(body)
    if g is None or qb is None or qb["lo"] != "0" or parse_struct_member(qb["b"], g.group(1)) is None:
        return False
    return range_lens_members(qb["cons"], f"loc({qb['b']},{qb['k']})")[1] is None


def _is_struct_wf_member_array_row(text: str) -> bool:
    return struct_wf_member_array_row(text) == ""


def struct_wf_prior_problem(rest: str) -> str | None:
    """An obligation `assert PRIOR ==> CONCLUSION` of an `f__wf` method whose PRIOR holds the well-formedness row of a pointer to a structure with an
    array member of structures (`struct_wf_member_array_row`) as a conjunct of its own (decision 0064 items 7 and 17, as built in Part C2): None when no
    conjunct of the prior is that row (the line is judged as before), "" when the other conjuncts and the conclusion are heap-free with no quantifier,
    else the reason."""
    text = rest.strip()
    parts = _split_top(text, "==>")
    if len(parts) < 2:
        return None
    prior = parts[0]
    try:
        conjuncts = [_strip_wrap(c) for c in _flatten_and_keeping_quantifiers(prior, _is_struct_wf_member_array_row)]
    except ValueError:
        return None
    if not any(_is_struct_wf_member_array_row(c) or _is_struct_lens_quantifier(c) for c in conjuncts):
        return None
    for c in conjuncts:
        if not (_is_struct_wf_member_array_row(c) or _is_struct_lens_quantifier(c)) and (FORALL_RE.search(c) or not _heap_free(c)):
            return "a conjunct of the prior besides the structure's well-formedness row is a quantifier or reads the heap"
    conclusion = text[len(prior) + len("==>"):]
    if FORALL_RE.search(conclusion) or not _heap_free(conclusion):
        return "the conclusion of an obligation under a structure's well-formedness row is a quantifier or reads the heap"
    return ""


LOAD_CELL_RE = re.compile(r"^(?P<cell>loc\(.+\))\.v_ptr_[\w$']+$")


def _canonical_load(text: str) -> bool:
    """Whether `text` is ONE load of a pointer from a canonical cell chain: `loc(GB_g(), 0).v_ptr_<t>` or `loc(member(<chain>, k), 0).v_ptr_<t>` with the
    chain rooted at one variable (decision 0043 item 13's grammar), and nothing else (no read inside the chain)."""
    c = norm_viper(text)
    m = LOAD_CELL_RE.match(c)
    if m is None or len(LOAD_FIELD_RE.findall(c)) != 1:
        return False
    cell = m.group("cell")
    gb = re.fullmatch(r"loc\((GB_[\w$']+)\(\),0\)", cell)
    if gb is not None:
        return True
    root = re.search(rf"member\((?:member\()*({IDENT})", cell)
    return root is not None and parse_struct_chain(cell, root.group(1)) is not None


def _canonical_null_test(conjunct: str) -> bool:
    """`!(L == null)`, `L != null`, or the owner member's `!((G ? L : null) == null)` with L a canonical load and G heap-free."""
    c = _strip_wrap(conjunct.strip())
    if c.startswith("!"):
        inner = _strip_wrap(c[1:].strip())
        sides = _split_top(inner, "==")
        if len(sides) != 2 or _sq(sides[1]) != "null":
            return False
        left = _strip_wrap(sides[0].strip())
        if _canonical_load(left):
            return True
        cond = _split_conditional(left)
        return cond is not None and _heap_free(cond[0]) and _canonical_load(_strip_wrap(cond[1])) and _sq(cond[2]) == "null"
    sides = _split_top(c, "!=")
    if len(sides) != 2 or _sq(sides[1]) != "null":
        return False
    left = _strip_wrap(sides[0].strip())
    if _canonical_load(left):
        return True
    cond = _split_conditional(left)
    return cond is not None and _heap_free(cond[0]) and _canonical_load(_strip_wrap(cond[1])) and _sq(cond[2]) == "null"


def _guard_has_only_null_tests(guard: str) -> bool:
    """Whether every conjunct of `guard` that reads the heap is a canonical null test of a canonical load (the rest are heap-free)."""
    try:
        parts = _flatten_and(guard)
    except ValueError:
        return False
    return all(_heap_free(c) or _canonical_null_test(c) for c in parts)


def range_atom_row_problem(kw: str, guard: str | None, q, perm_formals: set[str],
                           bound: Callable[[str], tuple[set[int], str] | None] | None = None) -> str | None:
    """Decision 0060 item 16: why the quantified `requires`, `ensures` or pure `assert` row `q` (under `guard`) is not one of the rows
    of a range atom, or None.  The rows (X a variable, F = `loc(block_of(X), k)`, the range `offset_of(X) <= k && k <
    offset_of(X) + n`): `acc(F.f[, a])` with `a` a `Perm` formal of the method (a `requires` or `ensures`; the marked `assert` rows are
    another rule), the flag `F.d_t`, the type's range of `F.v_t`, `F.d_t ==> range`, and `old(F.d_t) ==> F.d_t` (an `ensures` or an
    `assert`).  Nothing else holds a `forall` there."""
    k, b, extra, cons, lo, hi = q[:6]
    recv = q[6] if len(q) > 6 else None  # a leaf row's receiver (decision 0064 item 17), the cell its consequent is about
    x = BLOCK_BASE_RE.match(_squash(b)).group(1)
    if guard is not None:
        if GUARD_BAD_RE.search(guard):
            return "the guard holds a permission, a quantifier or a labelled `old`"
        # a guard of an `assert` row may hold, besides heap-free conjuncts, only the CANONICAL null test of a canonical load (the loaded pointer an owner
        # site's closure tests against null, as a cell atom's guard may: record 0045 item 3, decision 0064 item 15; audit round D1, finding 4): any other
        # use of a pointer read (inside `offset_of`, `block_of`, a comparison with anything but null, arithmetic), a scalar value or a flag stays refused
        if kw == "assert" and not _heap_free(guard) and not _guard_has_only_null_tests(guard):
            return "the guard of an `assert` row reads the heap"
        if not guard.strip():
            return "an empty guard"
    if extra:
        return "an antecedent beyond the range"
    # the emitter parenthesizes every sum under a comparison: `k < (offset_of(p) + n)`
    low, high = _squash(_strip_wrap(lo)), _squash(_strip_wrap(hi))
    info = bound(x) if bound is not None and low == "0" else None
    if info is not None:
        # decision 0063 item 15: the rows over a bound local (`t := loc(GB_g(), 0)`, `t := loc(member(X, k), 0)`) run over `[0, N)`, N the block's length fact
        counts, why = info
        if why:
            return why
        if not re.fullmatch(r"[0-9]+", high) or int(high) not in counts:
            return f"the count {high} of the rows over the bound local `{x}` is not the `len(...) == N` its block's facts state ({', '.join(map(str, sorted(counts)))})"
    elif low != f"offset_of({x})":
        return f"the range does not start at `offset_of({x})`"
    else:
        rem = high[len(f"offset_of({x})+"):] if high.startswith(f"offset_of({x})+") else ""
        if not rem or re.search(rf"(?<![\w$']){re.escape(k)}(?![\w$'])", rem):
            return f"the range does not end at `offset_of({x}) + <a count that does not name {k}>`"
    cell = recv if recv is not None else f"loc({_squash(b)},{k})"
    cells = re.escape(cell)
    acc = _range_acc(cons, k, b, recv)
    if acc is not None:
        if kw == "assert":
            return "a permission row (`acc`) is a marked row: unguarded, no wrapping, `assert` only with its own map entry"
        field, amount = acc
        if amount is not None and not (re.fullmatch(IDENT, amount) and amount in perm_formals):
            return "the amount of an `acc` is a `Perm` formal of the method"
        return None
    stripped = _strip_wrap(cons)
    sides = _split_top(stripped, "==>")
    cs = _squash(stripped)
    mp = re.search(cells + r"\.v_(ptr_" + IDENT + ")", cs) if len(sides) == 1 else None
    if mp and _std_type_range(stripped, cell, mp.group(1)):
        return None  # the well-formedness of a pointer element, with its pointee's length facts when it points to a structure (audit round C1, finding 4)
    if recv is None and len(sides) == 1 and "len(member(" in cs:
        # decision 0064 item 7: the length facts of the elements of a range of structures, one quantifier over the slots with the slot as its trigger
        return range_lens_problem(cs, cell)
    if len(sides) == 1:
        if re.fullmatch(cells + r"\.d_" + IDENT, cs):
            return None
        m = re.search(cells + r"\.v_(" + IDENT + ")", cs)
        if m and _std_type_range(stripped, cell, m.group(1)):
            return None
        return "the consequent is not `acc(F.f)`, the flag, the type's range or a flag implication"
    if len(sides) != 2:
        return "the consequent is not `acc(F.f)`, the flag, the type's range or a flag implication"
    ante, cons2 = _squash(_strip_wrap(sides[0])), _strip_wrap(sides[1])
    c2 = _squash(cons2)
    m = re.fullmatch(r"old\(" + cells + r"\.d_(" + IDENT + r")\)", ante)
    if m and c2 == f"{cell}.d_{m.group(1)}":
        return None if kw != "requires" else "`old` in a `requires`"
    m = re.fullmatch(cells + r"\.d_(" + IDENT + ")", ante)
    if m and _std_type_range(cons2, cell, m.group(1)):
        return None
    return "the consequent is not `acc(F.f)`, the flag, the type's range or a flag implication"


def array_global_counts(src: Source, decls: list[Decl]) -> dict[str, int]:
    """The array globals of the unit and their counts N, read from the first quantified row of each group of `program_start` (the group's own rule
    is `array_global_group`; decision 0063 item 9)."""
    out: dict[str, int] = {}
    for d in decls:
        if d.kind != "method" or d.name != PROGRAM_START:
            continue
        for kw, _ln, text in clause_texts(src, d):
            q = parse_block_row(text) if kw == "ensures" else None
            m = GB_TERM_RE.match(q["b"]) if q else None
            if q and m and q["lo"] == "0" and re.fullmatch(rf"acc\(loc\({re.escape(q['b'])},{re.escape(q['k'])}\)\.v_[\w$']+\)", q["cons"]):
                out.setdefault(m.group(1), int(q["hi"]))
            elif q and m and q["lo"] == "0" and "len(member(" in q["cons"]:
                out.setdefault(m.group(1), int(q["hi"]))  # an array of structures: its group starts with the length facts (decision 0064 item 9)
    return out


def method_len_facts(src: Source, d: Decl) -> dict[str, set[int]]:
    """The counts a method's clauses state: block term -> {N} for each `len(B) == N` that stands in a `requires` or an `ensures` (decision 0063
    item 15: a row over a member block's slots has the literal count the facts state)."""
    out: dict[str, set[int]] = {}
    for kw, _ln, text in clause_texts(src, d):
        if kw in ("requires", "ensures"):
            # the block term is balanced (a nested chain has parentheses; a guard `offset_of(p) < len(block_of(p)) ==> len(member(p, 0)) == 4` has an
            # earlier `len(` that is no count)
            for b, n in re.findall(r"len\(((?:[^()]|\((?:[^()]|\((?:[^()]|\([^()]*\))*\))*\))+)\)==([0-9]+)", norm_viper(text)):
                out.setdefault(b, set()).add(int(n))
    return out


LOCAL_ASSIGN_RE = re.compile(rf"^\s*(?P<lhs>{IDENT}(?:\s*,\s*{IDENT})*)\s*:=\s*(?P<rhs>.+?)\s*$")
LOCAL_VAR_INIT_RE = re.compile(rf"^\s*var\s+(?P<x>{IDENT})\s*:\s*[A-Za-z]+\s*:=\s*(?P<rhs>.+?)\s*$")
BOUND_RHS_RE = re.compile(r"^loc\((?P<b>.+),(?P<j>-?[0-9]+)\)$")
ALLOC_CALL_RE = re.compile(r"^(?P<name>alloc_struct_[0-9]+_[\w$']+)\(\)$")


def member_array_counts(counts: dict[str, dict[tuple[int, ...], int]]) -> dict[tuple[int, ...], set[int]]:
    """Every array member of every structure allocation `alloc_struct_<n>_<tag>`, by member key: key -> {N} (decision 0063 item 15: a member's bound
    local has the literal count of the member at that path and index in ANY allocation of a structure that has an array member there; lowering emits
    the allocation for every such structure, called or not, so the count always has its pinned source)."""
    out: dict[tuple[int, ...], set[int]] = {}
    for per in counts.values():
        for key, n in per.items():
            out.setdefault(key, set()).add(n)
    return out


def bound_locals(src: Source, d: Decl, decls: list[Decl], gbs: set[str], arrays: dict[str, int],
                 counts: dict[str, dict[tuple[int, ...], int]] | None = None) -> dict[str, tuple[set[int], str]]:
    """Decision 0063 item 15's bound local: the `Ref` locals of method `d` that some statement of its body assigns `loc(B, j)` with B a block term
    (`GB_<g>()` or `member(<chain>, k)`), as {local: (the counts the block's length fact states, why it is no bound local of the grammar or "")}.  A
    bound local is e1a's array local whose rows are `block_of(t)` over `0 <= k < N`: it is assigned exactly `loc(B, 0)` (every assignment the same text)
    with B in the block grammar, and N is, for `GB_g()`, the count of the global's row group in `program_start`, and for `member(<chain>, k)` the literal
    count of the member at that path and index in an `alloc_struct_*` of the unit (`member_array_counts`).  The chain is rooted at a Viper VARIABLE only
    (never an expression): a formal, a result, a local assigned an `alloc_struct_*` call, or any other local that the body assigns exactly once (a
    call-row temporary bound from the argument first, a pointer a call returned, a load for 0045's places), and `loc(GB_<g>(), 0)` of a declared
    global."""
    assigns: dict[str, list[str]] = {}
    for n in range(d.line + 1, d.end_line + 1):
        line = src.code[n - 1]
        m = LOCAL_ASSIGN_RE.match(line)
        if m:
            targets = [t.strip() for t in m.group("lhs").split(",")]
            for t in targets:
                # a multi-target call (`a, b := m(...)`) assigns each target something that is no `loc(B, 0)`
                assigns.setdefault(t, []).append(norm_viper(m.group("rhs")) if len(targets) == 1 else f"<multi-target {norm_viper(m.group('rhs'))}>")
        elif (m := LOCAL_VAR_INIT_RE.match(line)):
            assigns.setdefault(m.group("x"), []).append(norm_viper(m.group("rhs")))
    by_name = {e.name: e for e in decls if e.kind == "method"}
    formals = {*_formal_names(src, d), *(n for n, _t in method_results(src, d))}
    member_counts = member_array_counts(counts if counts is not None else struct_array_counts(src, decls))

    roots = set(formals)
    single = {x: rhss[0] for x, rhss in assigns.items() if len(rhss) == 1}
    for x, rhs in single.items():
        am = ALLOC_CALL_RE.match(rhs)
        if (am and am.group("name") in by_name) or not rhs.startswith("loc("):
            roots.add(x)  # a structure local from an allocation; a temporary bound from the argument, a call's result, a load
    pending = {x: BOUND_RHS_RE.match(rhs) for x, rhs in single.items() if x not in roots and rhs.startswith("loc(")}
    changed = True
    while changed:
        changed = False
        for x, m in pending.items():
            if x in roots or m is None:
                continue
            bt = block_term(m.group("b"), gbs, roots)
            # a pointer to a slot that is no array (`&s.t`, the address of a scalar or structure global) is a root for chains
            if bt is not None and (bt[1] not in arrays if bt[0] == "gb" else bt[2] not in member_counts):
                roots.add(x)
                changed = True
    out: dict[str, tuple[set[int], str]] = {}
    for x, rhss in assigns.items():
        if x in formals or not any(r.startswith("loc(") for r in rhss):
            continue
        ms = [BOUND_RHS_RE.match(r) for r in rhss]
        if any(m is None for m in ms) or len(set(rhss)) != 1:
            out[x] = (set(), f"the local `{x}` is not assigned one `loc(B, 0)` (it is assigned {', '.join(f'`{r}`' for r in sorted(set(rhss)))})")
            continue
        m = ms[0]
        b = m.group("b")
        bt = block_term(b, gbs, roots - {x})
        if m.group("j") != "0":
            out[x] = (set(), f"the local `{x}` is bound to slot {m.group('j')} of `{b}`, not the first slot of an array")
        elif bt is None:
            out[x] = (set(), f"the local `{x}` is bound to `loc({b}, 0)`, whose block is neither `GB_g()` of a declared array global nor `member(<chain>, k)` over "
                             "the canonical chain rooted at a Viper variable (a formal, a result, a structure local from an `alloc_struct_*` call or a "
                             "temporary assigned once; decision 0043 item 13)")
        elif bt[0] == "gb":
            if bt[1] in arrays:
                out[x] = ({arrays[bt[1]]}, "")
            else:
                out[x] = (set(), f"`{bt[1]}` is not an array global (no row group of it in `program_start`)")
        elif member_counts.get(bt[2]):
            out[x] = (member_counts[bt[2]], "")
        else:
            out[x] = (set(), f"no `alloc_struct_*` of the unit has an array member at {bt[2]} (`len({b}) == N`): the count of a member's rows is the literal "
                             "the allocation of its structure states (decision 0063 item 15)")
    return out


def _block_type_range(cons: str, cell: str, t: str) -> bool:
    """`_std_type_range` for a row of `parse_block_row` (whose text has lost its grouping parentheses, so an `||` inside a conjunction no longer
    parses): an integer, boolean or enumeration range as before; the range of a pointer element `ptr_<t>` (an array member of pointers under `owns(*p)`)
    is `WF` of its value, compared in the same normal form (audit round 4 S1)."""
    if t.startswith("ptr_"):
        # WF of the value, or WF with the guarded length facts of the pointee structure (audit round C2, finding 1: the leaf of an element of an array
        # member that is a `struct S *`, S holding an array member of structures: the recursive type)
        return norm_viper(cons) == norm_viper(_view_wf(f"{cell}.v_{t}")) or _std_type_range(cons, cell, t)
    return _std_type_range(cons, cell, t)


def block_row_problem(kw: str, guard: str | None, q: dict[str, str], gbs: set[str], arrays: dict[str, int], roots: set[str],
                      lens: dict[str, set[int]], perm_formals: frozenset[str]) -> str | None:
    """Decision 0063 item 15: why the quantified `requires` or `ensures` row `q` (under `guard`) over a block TERM is not one of an array atom's
    rows, or None.  The block is `GB_g()` of a declared array global, or `member(<chain>, k)` with the canonical chain rooted at a formal, a
    result or `loc(GB_g(), 0)`; the range is `0 <= k && k < N` with N the count the facts state (`len(GB_g()) == N`, or a `len(member(...)) == N`
    of the method's own clauses); the consequent is `acc(F.f[, a])` (a a `Perm` formal), the flag `F.d_t`, the type's range of `F.v_t`,
    `F.d_t ==> range` or (an `ensures`) `old(F.d_t) ==> F.d_t`, with F = `loc(B, k)`."""
    if guard is not None:
        if GUARD_BAD_RE.search(guard):
            return "the guard holds a permission, a quantifier or a labelled `old`"
        if not guard.strip():
            return "an empty guard"
    bt = block_term(q["b"], gbs, roots, loads=True)
    if bt is None:
        return (f"the block `{q['b']}` is neither `GB_g()` of a declared global nor `member(<chain>, k)` over the canonical chain rooted at a formal, "
                "a result or `loc(GB_g(), 0)` (decision 0043 item 13)")
    if bt[0] == "member" and LOAD_FIELD_RE.search(bt[1]) and "old(" in q["cons"]:
        return ("a kept flag (`old(F.d_t) ==> F.d_t`) over a block reached through a heap read has no rule yet: it needs the tie of the loaded pointer "
                "(decision 0058 item 4, audit B1), which `lint_marked_kept_rows` reads only in the cell form")
    if q["lo"] != "0":
        return f"the range starts at {q['lo']}, not 0 (a part of an array is M4.5e2's)"
    n = int(q["hi"])
    if bt[0] == "gb":
        if bt[1] not in arrays:
            return f"`{bt[1]}` is not an array global (no row group of it in `program_start`)"
        if arrays[bt[1]] != n:
            return f"the count {n} is not the `len({bt[1]}()) == {arrays[bt[1]]}` of the facts"
    elif n not in lens.get(q["b"], set()):
        return f"the method states no `len({q['b']}) == {n}`: the count of a member's rows is the one its facts state"
    cell = q.get("recv") or f"loc({q['b']},{q['k']})"  # a leaf row's receiver (decision 0064 items 9 and 17), else the slot
    cons = q["cons"]
    pm = re.search(rf"{re.escape(cell)}\.v_(ptr_[\w$']+)", cons)
    if pm and q.get("raw") and _std_type_range(q["raw"], cell, pm.group(1)):
        return None  # the leaf of a pointer to a structure: WF and the guarded length facts, possibly with a quantifier (finding 1)
    if "recv" not in q and "len(member(" in cons:
        # the length facts of the elements of an array of structures, one quantifier over the slots (decision 0064 item 3)
        return range_lens_problem(cons, cell)
    m = re.fullmatch(rf"acc\({re.escape(cell)}\.(?P<f>[vd]_[\w$']+)(?:,(?P<amt>[^()&|=]+))?\)", cons)
    if m:
        if m.group("amt") is not None and not (re.fullmatch(IDENT, m.group("amt")) and m.group("amt") in perm_formals):
            return "the amount of an `acc` is a `Perm` formal of the method"
        return None
    if re.fullmatch(rf"{re.escape(cell)}\.d_[\w$']+", cons):
        return None
    sides = _split_top(cons, "==>")
    if len(sides) == 1:
        rm = re.search(rf"{re.escape(cell)}\.v_({IDENT})", cons)
        if rm and _block_type_range(cons, cell, rm.group(1)):
            return None
    elif len(sides) == 2:
        am = re.fullmatch(rf"old\({re.escape(cell)}\.d_({IDENT})\)", sides[0])
        if am and sides[1] == f"{cell}.d_{am.group(1)}":
            return None if kw == "ensures" else "`old` in a `requires`"
        am = re.fullmatch(rf"{re.escape(cell)}\.d_({IDENT})", sides[0])
        if am and _block_type_range(sides[1], cell, am.group(1)):
            return None
    return "the consequent is not `acc(F.f)`, the flag, the type's range or a flag implication"


def _range_acc(consequent: str, k: str, b: str, cell: str | None = None) -> tuple[str, str | None] | None:
    """(field, amount) of `acc(loc(B, k).f[, amount])`, or None; with `cell` (a leaf row's receiver, normalized) of `acc(<cell>.f[, amount])`."""
    if cell is not None:
        lm = re.match(r"^acc\(" + re.escape(cell) + r"\.(?P<f>[vd]_[\w$']+)(?:,(?P<amt>.*))?\)$", _squash(consequent))
        return None if lm is None else (lm.group("f"), lm.group("amt"))
    m = re.match(r"^acc\(\s*loc\(\s*" + re.escape(b) + r"\s*,\s*" + re.escape(k) + r"\s*\)\s*\.(?P<f>[vd]_[\w$']+)\s*(?:,(?P<amt>.*))?\)$",
                 consequent.strip(), re.S)
    if m is None:
        return None
    amount = m.group("amt")
    return m.group("f"), (amount.strip() if amount is not None else None)


def marked_row_shape(code: str) -> bool:
    """Whether `code` is an `assert` of decision 0059 item 7's shape (one field, a heap-free amount if any)."""
    m = QUANT_STMT_RE.match(code)
    if m is None or m.group("kw") != "assert":
        return False
    q = parse_range_quantifier_full(m.group("rest")) or parse_leaf_row(m.group("rest"))
    if q is None:
        # decision 0064 item 15, step B: a row of a written part over `member(X, j)`, the `acc` of one field with no amount (`lint_quantified_rows` ties it)
        wq = parse_block_row(m.group("rest"))
        if wq is not None and wq["b"].startswith("member("):
            shape = written_part_shape(wq)
            return shape is not None and shape[0] == "acc"
    if q is None or q[2]:
        return user_wildcard_row(code)
    acc = _row_acc(q)
    return acc is not None and (acc[1] is None or _heap_free(acc[1]))


SNAPSHOT_RHS_RE = re.compile(rf"^perm\(\s*(?P<x>{IDENT})\s*\.\s*(?P<f>[vd]_[\w$']+)\s*\)$")
HELD_ANTECEDENT_RE = re.compile(rf"^(?P<n>{IDENT})>none$")
BELOW_WRITE_RE = re.compile(rf"^(?P<n>{IDENT})<write$")
ASSIGNMENT_RE = re.compile(r"(?<![\w$'.])(?P<n>[A-Za-z_$][\w$']*)\s*:=")


def amount_snapshots(src: Source, d: Decl) -> dict[str, tuple[str, str]]:
    """Decision 0064 item 15 (Part D, a heap block's array part as a loop place): the `Perm` locals of the method `d` that are assigned exactly ONCE
    and then `perm(X.f)` for a variable X and a field `v_<t>` or `d_<t>` (the first slot's permissions at the loop's entry, S48 (k)'s ghost
    amounts), as {local: (X, f)}.  A local assigned twice, or from anything but that one snapshot, is not here: it is no amount of this rule."""
    out: dict[str, tuple[str, str]] = {}
    if not d.has_body:
        return out
    names = perm_local_names(src, d)
    rhs: dict[str, list[str]] = {}
    for n in range(d.body_line, d.end_line + 1):
        code = src.code[n - 1]
        for m in ASSIGNMENT_RE.finditer(code):
            if m.group("n") in names:
                rhs.setdefault(m.group("n"), []).append(_statement_rhs(code, m.end()))
    for name, rights in rhs.items():
        if len(rights) == 1 and (sm := SNAPSHOT_RHS_RE.match(rights[0])) is not None:
            out[name] = (sm.group("x"), sm.group("f"))
    return out


def _held_prefix(extra: list[str], amounts: dict[str, tuple[str, str]]) -> tuple[list[tuple[str, str]], list[str]]:
    """The antecedent conjuncts `K > none` (and, right after one, `K < write`) of a heap array part's flag, range and value conjuncts, K a snapshot
    local of `amounts`: ([(K, `>`|`<`) ...], the conjuncts after them)."""
    held: list[tuple[str, str]] = []
    i = 0
    while i < len(extra):
        m, w = HELD_ANTECEDENT_RE.match(extra[i]), BELOW_WRITE_RE.match(extra[i])
        if m is not None and m.group("n") in amounts and not (held and held[-1][1] == "<"):
            held.append((m.group("n"), ">"))
        elif w is not None and w.group("n") in amounts and held and held[-1] == (w.group("n"), ">"):
            held.append((w.group("n"), "<"))
        else:
            break
        i += 1
    return held, extra[i:]


def _own_first_slot_range(q, amounts: dict[str, tuple[str, str]]) -> str | None:
    """The base variable X of a heap array part's quantifier `forall k :: { loc(block_of(X), k) } offset_of(X) <= k && k < offset_of(X) + N ==> ...`
    (the range runs over the N slots from X's own offset, X the part's bound local), or None."""
    x = BLOCK_BASE_RE.match(_squash(q[1]))
    if x is None or len(q) > 6:
        return None
    X = x.group(1)
    if _sq(q[4]) != f"offset_of({X})" or re.fullmatch(rf"offset_of\({re.escape(X)}\)\+\d+", _sq(q[5])) is None:
        return None
    return X


def leaf_is_none(q) -> bool:
    """Whether the parsed row `q` is a plain range row (no leaf receiver, the older parser's shape)."""
    return len(q) <= 6


def _loop_conjunct(q, perm_formals: frozenset[str] = frozenset(), amounts: dict[str, tuple[str, str]] | None = None,
                   guarded: bool = True) -> tuple[str, list[str]] | None:
    """(kind, labels read by old[...]) of item 8's four quantified loop conjuncts, or None.  The kinds: `acc` (full permission over
    the range), `flags` (`... && old[L](E.d_t) ==> E.d_t`), `range` (`... && E.d_t ==> lo_T <= E.v_t && E.v_t <= hi_T`) and
    `values` (`... ==> E.v_t == old[L](E.v_t)`).  The `acc` of a range place of a `reads_range` parameter carries the function's own
    amount (decision 0060 item 11): a `Perm` formal of the method (`perm_formals`), never a local or an expression.  The `range` of a pointer element `ptr_<t>` is `WF` of its value (S43 `s43_c_ptr_elem`); a pointer to a structure's extra `WF_S` conjunct is not accepted yet."""
    k, b, extra, cons = q[:4]
    if amounts and leaf_is_none(q):
        # decision 0064 item 15 (Part D): a heap block's array part as a loop place.  Its permission conjunct is at the amount of the part's FIRST slot
        # at the loop's entry, a `Perm` local that the method assigns once `perm(X.f)` (`amount_snapshots`), X the base of the quantifier and f its
        # field, over the range from X's own offset; its flag, range and value conjuncts carry the antecedents `K > none` of the amounts they read (and
        # `K < write` where a write can reach the field), K the snapshots of the same X and the same fields.  All of it under a guard `g ==> `, the one
        # under which the snapshots are read (an unguarded one would read amounts of a part the method never bound).
        X = _own_first_slot_range(q, amounts)
        acc = _range_acc(_squash(cons), k, b, None) if not extra else None
        if acc is not None and acc[1] is not None and acc[1] in amounts and acc[1] not in perm_formals:
            return ("acc", []) if (X is not None and guarded and amounts[acc[1]] == (X, acc[0])) else None
        held, rest = _held_prefix([_squash(x) for x in extra], amounts)
        if held:
            if X is None or not guarded:
                return None
            res = _loop_conjunct((k, b, rest, cons) + tuple(q[4:]), perm_formals)
            cell0 = rf"loc\({re.escape(_squash(b))},{re.escape(k)}\)"
            names = [n for n, _op in held]
            if res is None:
                return None
            if res[0] == "flags":
                m = re.match(rf"^{cell0}\.d_([\w$']+)$", _squash(cons))
                ok = m is not None and held == [(names[0], ">")] and amounts[names[0]] == (X, f"d_{m.group(1)}")
            elif res[0] == "range":
                m = re.match(rf"^{cell0}\.d_([\w$']+)$", rest[0])
                ok = m is not None and [op for _n, op in held] == [">", ">"] and \
                    amounts[names[0]] == (X, f"v_{m.group(1)}") and amounts[names[1]] == (X, f"d_{m.group(1)}")
            elif res[0] == "values":
                m = re.match(rf"^{cell0}\.v_([\w$']+)==old\[", _squash(cons))
                ok = m is not None and [op for _n, op in held] in ([">"], [">", "<"]) and len(set(names)) == 1 and \
                    amounts[names[0]] == (X, f"v_{m.group(1)}")
            else:
                ok = False
            return res if ok else None
    raw_cons = cons
    leaf = q[6] if len(q) > 6 else None  # a leaf row's receiver (decision 0064 item 17)
    cell = rf"loc\({re.escape(_squash(b))},{re.escape(k)}\)" if leaf is None else re.escape(leaf)
    cons = _squash(cons)
    if not extra:
        acc = _range_acc(cons, k, b, leaf)
        if acc is not None and (acc[1] is None or (re.fullmatch(IDENT, acc[1]) and acc[1] in perm_formals)):
            return "acc", []
        m = re.match(rf"^({cell}\.v_[\w$']+)==old\[([\w$']+)\]\(?({cell}\.v_[\w$']+)\)?$", cons)
        if m and m.group(1) == m.group(3):
            return "values", [m.group(2)]
        return None
    if len(extra) != 1:
        return None
    m = re.match(rf"^old\[([\w$']+)\]\(?({cell}\.d_[\w$']+)\)?$", extra[0])
    if m and cons == m.group(2):
        return "flags", [m.group(1)]
    m = re.match(rf"^({cell})\.d_([\w$']+)$", extra[0])
    if m:
        try:
            lows = [_squash(x) for x in _flatten_and(cons)]
        except ValueError:
            return None
        v = rf"{cell}\.v_{re.escape(m.group(2))}"
        if len(lows) == 2 and re.match(rf"^\(*-?\d+\)*<={v}$", lows[0]) and re.match(rf"^{v}<=\(*-?\d+\)*$", lows[1]):
            return "range", []
        # a pointer element: the range of its value is `WF` (null, or the pointer's own block with 0 <= offset <= len), the same
        # text as a view's, over the element read under the quantifier (S43 `s43_c_ptr_elem`); a pointer to a structure adds
        # the guarded `WF_S` conjunct (decision 0043 item 8), which is not accepted here yet
        element = f"loc({_squash(b)},{k}).v_{m.group(2)}" if leaf is None else f"{leaf}.v_{m.group(2)}"
        if len(lows) == 1 and m.group(2).startswith("ptr_") and norm_viper(lows[0]) == norm_viper(_view_wf(element)):
            return "range", []
        # audit round C2, finding 1: WF with the guarded length facts of the pointee, which may hold the quantifier over an array member of structures
        if m.group(2).startswith("ptr_") and len(lows) >= 2 and _std_type_range(raw_cons, f"loc({_squash(b)},{k})" if leaf is None else leaf, m.group(2)):
            return "range", []
    return None


# ---------------------------------------------------------------------------
# Record 0061 item 11: user quantifiers, the marked `wildcard` row, and the heap-free `f__wf` of a quantified contract
# ---------------------------------------------------------------------------
# A user quantifier (a `forall` of `cerastes_forall(f, ...)`) binds `q0`, `q1`, ... (all `Int`) with ONE trigger set holding one term per
# binder, `loc(block_of(X), q<j>)` with X a variable (S44 F-b1), and a body (antecedent and consequent) with no `acc`, `perm`, `forperm`,
# `inhale` or `forall`.  It stands alone in a clause or an `assert` of a method (a conjunct of its own line, under at most a guard `g ==> `),
# in `requires`, `ensures`, `invariant` and `assert`.  The one permission it may hold is the marked row of item 10
# (`assert forall q0: Int :: { loc(block_of(X), q0) } reach ==> acc(loc(block_of(X), q0).f, wildcard)`, an unguarded `assert`, whose
# map entry carries `assert.failed[insufficient.permission]`): `wildcard` stands nowhere else.
# In a `<f>__wf` method (variant M of item 7) the heap does not exist: a read under a binder is a ghost function `hvq_<method>__<p>_pre` or `_post`
# of ONE domain tagged `// cerastes:domain-ok 0061` (bodiless functions of `Int`/`Block` arguments and an `Int` result, no axiom, no body,
# no contract), used by that method alone.  The method's assumed facts (`requires` with a ghost function or a user binder) have four shapes
# (a) the type's range of an application, one trigger set holding the application; (b) an application at a scalar ghost's slot,
# `G(offset_of(X) + c) == ghost`, or two parameters' applications on one block, `block_of(X) == block_of(Y) ==> forall s :: { Gx(s) } { Gy(s) }
# Gx(s) == Gy(s)` (one trigger set per application the fact relates); (c) a reads place keeps its value, `[g ==>] forall s :: { post(s) } { pre(s) }
# offset_of(X) <= s && s < offset_of(X) + n ==> post(s) == pre(s)`; and its `assert` rows are `[PRIOR ==>] (forall q<j> :: { G(q<j>) ... } ...)`
# with every quantifier of the prior a conjunct of its own and none inside a body (the prior stands outside the binder).
USER_QUANT_RE = re.compile(r"(?<![\w$'])forall\s+q[0-9]+\s*[:,]")
WF_QUANT_RE = re.compile(r"(?<![\w$'])forall\s+(?:q[0-9]+|s[0-9]*)\s*[:,]")
USER_BINDER_RE = re.compile(r"^q[0-9]+$")
WF_BINDER_RE = re.compile(r"^(?:q[0-9]+|s[0-9]*)$")
QUANT_FORBIDDEN_RE = re.compile(r"(?<![\w$'])(?:forall|acc|perm|forperm|inhale|assume)(?![\w$'])")
BINDERS_RE = re.compile(r"^forall\s+(?P<binders>" + r"[A-Za-z_$][\w$']*\s*:\s*[A-Za-z_$][\w$']*"
                        r"(?:\s*,\s*[A-Za-z_$][\w$']*\s*:\s*[A-Za-z_$][\w$']*)*)\s*::\s*", re.S)
GHOST_DOMAIN_RE = re.compile(rf"^\s*domain\s+{IDENT}\s*\{{\s*$")
GHOST_FUNCTION_RE = re.compile(rf"^\s*function\s+(?P<name>{IDENT})\s*\((?P<params>[^()]*)\)\s*:\s*(?P<ret>Int|Bool|Ref)\s*$")
GHOST_PARAM_RE = re.compile(rf"^\s*{IDENT}\s*:\s*(?:Int|Block)\s*$")
HVA_PARAM_RE = re.compile(rf"^\s*{IDENT}\s*:\s*(?:Int|Bool|Ref)\s*$")  # decision 0062 item 8: one parameter per C parameter
HVQ_TAIL_RE = re.compile(r"^(?P<p>.+)_(?P<state>pre|post)$")
HV_NAME_RE = re.compile(r"^hv_.+_(?P<state>pre|post)$")
GHOST_DOMAIN_RECORD = "0061"
Q11 = "`forall` outside decision 0061 item 11"


def _split_commas(text: str) -> list[str]:
    parts, depth, cur = [], 0, []
    for ch in text:
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        if ch == "," and depth == 0:
            parts.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
    parts.append("".join(cur))
    return [p.strip() for p in parts if p.strip()]


def parse_quantifier(text: str):
    """`forall b1: T1, b2: T2 :: { t1, t2 } { t3 } BODY` as (binders [(name, type)], trigger sets [[term]], body), or None."""
    text = text.strip()
    m = BINDERS_RE.match(text)
    if m is None:
        return None
    binders = []
    for part in m.group("binders").split(","):
        name, typ = part.split(":")
        binders.append((name.strip(), typ.strip()))
    rest = text[m.end():].lstrip()
    triggers: list[list[str]] = []
    while rest.startswith("{"):
        depth = 0
        end = -1
        for i, ch in enumerate(rest):
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    end = i
                    break
        if end < 0:
            return None
        triggers.append(_split_commas(rest[1:end]))
        rest = rest[end + 1:].lstrip()
    return binders, triggers, rest.strip()


def quantifier_problem(q, ghosts: frozenset[str] | None) -> str | None:
    """Why the parsed quantifier `q` is not a user quantifier of item 11, or None.  `ghosts` None: a method's own clauses (the
    trigger terms are heap cells); a set of names: the quantifier of an `f__wf` method (the terms are applications of those ghost
    functions, the binders may also be `s`, `s0`, ... as the assumed facts and the prior's clauses write them, and the body reads no heap)."""
    binders, triggers, body = q
    names = [n for n, _ in binders]
    pattern = USER_BINDER_RE if ghosts is None else WF_BINDER_RE
    for name, typ in binders:
        if not pattern.match(name):
            return f"the binder `{name}` is not named `q<j>`" if ghosts is None else f"the binder `{name}` is not named `q<j>` or `s<j>`"
        if typ != "Int":
            return f"the binder `{name}` is `{typ}`, not `Int`"
    if len(set(names)) != len(names):
        return "a binder is named twice"
    if ghosts is None and len(triggers) != 1:
        return f"a user quantifier has exactly one trigger set, found {len(triggers)}"
    if not triggers:
        return "a user quantifier has a trigger set"
    parsed: list[dict[str, str]] = []  # per set: binder -> the function (or the heap cell's base) of its term
    for tset in triggers:
        by_binder: dict[str, str] = {}
        for term in tset:
            if ghosts is None:
                m = re.fullmatch(rf"loc\(\s*block_of\(\s*({IDENT})\s*\)\s*,\s*({IDENT})\s*\)", term)
                if m is None or m.group(2) not in names:
                    return f"the trigger term `{term}` is not `loc(block_of(X), q<j>)` with X a variable and `q<j>` a binder"
            else:
                m = re.fullmatch(rf"({IDENT})\(\s*({IDENT})\s*\)", term)
                if m is None or m.group(1) not in ghosts or m.group(2) not in names:
                    return f"the trigger term `{term}` is not an application of a ghost function of the `domain-ok 0061` domain to a binder (no heap term in `f__wf`)"
            if m.group(2) in by_binder:
                return f"a trigger set holds two terms for the binder `{m.group(2)}`"
            by_binder[m.group(2)] = m.group(1)
        if sorted(by_binder) != sorted(names):
            return "each trigger set holds one term per binder (every class's term)"
        parsed.append(by_binder)
    if len(parsed) > 1:
        # question 257 (`f__wf` only): a class read in both states gets one set per state, the sets differing only in the `_pre`/`_post` of a class's function
        state = re.compile(r"_(?:pre|post)$")
        for by_binder in parsed[1:]:
            for b in names:
                if state.sub("", by_binder[b]) != state.sub("", parsed[0][b]):
                    return f"the trigger sets differ only in the state (`_pre`/`_post`) of a class's ghost function, not in its formal or method (`{parsed[0][b]}` against `{by_binder[b]}`)"
        if any(parsed[a] == parsed[b] for a in range(len(parsed)) for b in range(a)):
            return "two trigger sets are the same"
    if QUANT_FORBIDDEN_RE.search(body):
        return "the body holds `acc`, `perm`, `forperm`, `inhale` or `forall`"
    if ghosts is not None and not _heap_free(body):
        return "the body of a row of `f__wf` reads the heap"
    if not carries_type_range(body, names):
        return ("the antecedent does not hold the type's range of the bound variable "
                "(`lo_T <= (q0 - offset_of(X)) && (q0 - offset_of(X)) <= hi_T`; without it the quantifier ranges over values no `i` has)")
    if computed_loc_problem(body, names, ghosts or frozenset()):
        return "the body holds a computed `loc` term over a binder (`loc(block_of(X), q0 + 1)`: T1's form, whose instances never end; use a binder per class)"
    return None


def carries_type_range(body: str, names: list[str]) -> bool:
    """Whether the antecedent (before the first top-level `==>`) holds, as two adjacent conjuncts, the range of an integer type over an
    expression of a binder (a prior inside the binder may stand before it)."""
    parts = _split_top(body, "==>")
    if len(parts) < 2:
        return False
    try:
        conj = [norm_viper(x) for x in _flatten_and(parts[0])]
    except ValueError:
        return False
    for lo_text, hi_text in zip(conj, conj[1:]):
        lo = re.fullmatch(r"(-?[0-9]+)<=(.+)", lo_text)
        hi = re.fullmatch(r"(.+)<=(-?[0-9]+)", hi_text)
        if lo and hi and lo.group(2) == hi.group(1) and (int(lo.group(1)), int(hi.group(2))) in VIEW_STANDARD_RANGES \
                and any(re.search(rf"(?<![\w$']){re.escape(n)}(?![\w$'])", lo.group(2)) for n in names):
            return True
    return False


def computed_loc_problem(text: str, names: list[str], ghosts: frozenset[str] = frozenset()) -> bool:
    """Whether `text` holds a `loc(B, e)` (or an application `G(e)` of a ghost function) whose index `e` is an expression over a binder rather
    than the binder itself: T1's form, whose instances never end (S45 (b))."""
    mentions = lambda index: index not in names and any(  # noqa: E731
        re.search(rf"(?<![\w$']){re.escape(n)}(?![\w$'])", index) for n in names)
    for m in re.finditer(r"(?<![\w$'])loc\(", text):
        depth, i = 1, m.end()
        while i < len(text) and depth:
            depth += {"(": 1, ")": -1}.get(text[i], 0)
            i += 1
        args = _split_commas(text[m.end():i - 1])
        if len(args) == 2 and mentions(args[1].strip()):
            return True
    if ghosts:
        for m in re.finditer(r"(?<![\w$'])(?:" + "|".join(map(re.escape, sorted(ghosts))) + r")\(", text):
            depth, i = 1, m.end()
            while i < len(text) and depth:
                depth += {"(": 1, ")": -1}.get(text[i], 0)
                i += 1
            if mentions(text[m.end():i - 1].strip()):
                return True
    return False


def wildcard_row_problem(q) -> str | None:
    """None if the parsed quantifier is item 10's marked row `forall q<j> :: { loc(block_of(X), q<j>) } reach ==> acc(loc(block_of(X), q<j>).f,
    wildcard)` (one binder, one term, a reach that reads no heap and holds no permission or quantifier), else why not."""
    binders, triggers, body = q
    if len(binders) != 1 or not USER_BINDER_RE.match(binders[0][0]) or binders[0][1] != "Int":
        return "one binder `q<j>: Int`"
    k = binders[0][0]
    if len(triggers) != 1 or len(triggers[0]) != 1:
        return "one trigger set holding `loc(block_of(X), q<j>)`"
    m = re.fullmatch(rf"loc\(\s*block_of\(\s*({IDENT})\s*\)\s*,\s*{re.escape(k)}\s*\)", triggers[0][0])
    if m is None:
        return "the trigger `loc(block_of(X), q<j>)` with X a variable"
    parts = _split_top(body, "==>")
    if len(parts) != 2:
        return "`reach ==> acc(...)`"
    acc = _range_acc(_strip_wrap(parts[1]), k, f"block_of({m.group(1)})")
    if acc is None or acc[1] is None or acc[1].strip() != "wildcard" or not acc[0].startswith(("v_", "d_")):
        return f"the consequent `acc(loc(block_of({m.group(1)}), {k}).f, wildcard)` of the same base"
    if QUANT_FORBIDDEN_RE.search(parts[0]) or not _heap_free(parts[0]):
        return "a reach that reads no heap and holds no permission or quantifier"
    if not carries_type_range(body, [k]):
        return "an antecedent that holds the type's range of the bound variable"
    return None


def _guard_problem(kind: str, guard: str | None) -> str | None:
    if guard is None:
        return None
    if not guard.strip():
        return "an empty guard"
    if GUARD_BAD_RE.search(guard):
        return "the guard holds a permission, a quantifier or a labelled `old`"
    if kind == "assert" and not _heap_free(guard):
        return "the guard of an `assert` reads the heap"
    return None


def loop_prior_verdict(kind: str, rest: str, loop_clauses: frozenset[str]) -> tuple[bool, str, bool] | None:
    """Decision 0061 item 9 (question 258): a loop's obligation `invariant` line and its framing `assert` (entry and back edge) are `PRIOR ==> CONCLUSION`
    where the PRIOR holds the EARLIER invariant clauses of the loop as quantifier conjuncts (0037 item 7's evaluation-order prior: they are checked
    invariants of their own).  None if `rest` has no quantifier conjunct before its first `==>`; else the verdict: every quantifier conjunct is a user
    quantifier of item 11's shape that is the quantifier of an `invariant` line of THIS loop that `loop_clause_scopes` allows here (`loop_clauses`), the other conjuncts are plain conditions
    (an `assert`'s read no heap), and the conclusion is a user quantifier or a plain condition."""
    text = rest.strip()
    if FORALL_RE.match(_strip_wrap(text)):
        return None
    parts = _split_top(text, "==>")
    if len(parts) < 2:
        return None
    prior = parts[0]
    try:
        conjuncts = [_strip_wrap(c) for c in _flatten_and_keeping_quantifiers(prior)]
    except ValueError:
        return None
    if not any(FORALL_RE.match(c) for c in conjuncts):
        return None
    refused = lambda why: (False, f"{Q11}: {why}", False)  # noqa: E731
    if kind not in ("invariant", "assert"):
        return refused("a quantifier in the prior of a clause is admitted only in a loop's obligation `invariant` line and its framing `assert`")
    for c in conjuncts:
        if FORALL_RE.match(c):
            q = parse_quantifier(c)
            why = None if q is None else quantifier_problem(q, None)
            if q is None or why or OLD_LABEL_RE.search(c):
                return refused(f"a quantifier of the prior: {why or ('a labelled `old`' if q is not None else 'not a user quantifier')}")
            if norm_viper(c) not in loop_clauses:
                return refused("a quantifier of the prior is not an earlier `invariant` clause of the loop this line belongs to (an obligation line names the earlier clauses of its own loop, a framing `assert` the clauses of the loop it frames; no other line has a quantifier in its prior)")
        else:
            if GUARD_BAD_RE.search(c) or QUANT_FORBIDDEN_RE.search(c):
                return refused("a condition of the prior holds a permission, a quantifier or a labelled `old`")
            if kind == "assert" and not _heap_free(c):
                return refused("a condition of the prior of an `assert` reads the heap")
    conclusion = _strip_wrap(text[len(prior) + len("==>"):])
    if FORALL_RE.match(conclusion):
        q = parse_quantifier(conclusion)
        why = None if q is None else quantifier_problem(q, None)
        return refused(f"the conclusion: {why or 'not a user quantifier'}") if (q is None or why) else (True, "", False)
    if FORALL_RE.search(conclusion) or QUANT_FORBIDDEN_RE.search(conclusion):
        return refused("the conclusion is a user quantifier or a plain condition")
    return (True, "", False)


def contract_line_verdict(kind: str, rest: str, loop_clauses: frozenset[str] = frozenset()) -> tuple[bool, str, bool]:
    """(ok, why, marked) for a line of a method's own clauses or body that holds a user quantifier (`forall q<j>`)."""
    prior_form = loop_prior_verdict(kind, rest, loop_clauses)
    if prior_form is not None:
        return prior_form
    guard, body = split_guard(rest)
    if body is None:
        return False, f"{Q11}: a user quantifier is a clause of its own (`forall q<j>: Int :: {{ ... }} ...`, under at most a guard `g ==> `)", False
    q = parse_quantifier(body)
    if q is None:
        return False, f"{Q11}: not `forall q<j>: Int[, ...] :: {{ loc(block_of(X), q<j>), ... }} body`", False
    wild = re.search(r"(?<![\w$'.])wildcard(?![\w$'])", body) is not None
    if wild:
        why = None if kind == "assert" else "`wildcard` stands only in an `assert`"
        if why is None and guard is not None or why is None and not FORALL_RE.match(rest.strip()):
            why = "the marked `wildcard` row is unguarded and unwrapped"
        if why is None:
            why = wildcard_row_problem(q)
            if why is not None:
                why = f"the marked `wildcard` row is {why}"
        return (why is None), ("" if why is None else f"{Q11}: {why}"), why is None
    why = quantifier_problem(q, None) or _guard_problem(kind, guard)
    return (why is None), ("" if why is None else f"{Q11}: {why}"), False


def _plain_condition_problem(text: str) -> str | None:
    if not _heap_free(text) or QUANT_FORBIDDEN_RE.search(text):
        return "a plain condition (no heap, permission or quantifier)"
    return None


@dataclasses.dataclass(frozen=True)
class RangeAtom:
    """A `requires` or `ensures` row of the method an `f__wf` belongs to that names a place of a formal: `acc(loc(block_of(X), k).v_T[, amt])`
    over `offset_of(X) <= k < offset_of(X) + count` (`kind` range), or `acc(X.v_T, amt)` (`cell`).  `amount` is a `Perm` formal (a `reads`
    atom) or None (an `owns` atom); `guard` is the clause's guard as written, normalized."""
    clause: str  # requires or ensures
    kind: str  # range or cell
    formal: str
    field: str  # v_<t>
    amount: str | None
    count: str | None  # normalized count of a range
    guard: str | None


@dataclasses.dataclass(frozen=True)
class PfInfo:
    """What the lint knows of one `pf_` function of the file (decision 0062 item 12): its formals, its result type and the conjuncts of its
    `requires` and `ensures` that read no memory (the facts (ii) of an `f__wf` method are exactly these, at the arguments)."""
    name: str
    params: tuple[tuple[str, str], ...]
    ret: str
    requires: tuple[str, ...]
    ensures: tuple[str, ...]
    opaque_requires: bool = False  # a `requires` conjunct applies a `pure` function: it is no conjunct of a guard V the lint can compare
    quantified: tuple[str, ...] = ()  # the user quantifiers (`forall q<j>`) of its `requires`: fact (ii)'s guard V holds each over the `hvq` ghosts (B1)
    atoms: tuple[tuple[str, str, str], ...] = ()  # its range atoms: (pointer parameter, count, field `v_<t>`), read from the `acc(..., wildcard)` lines (S1)
    result_tag: tuple[int, int] | None = None  # the `// cerastes:result <lo> <hi>` tag of its signature line (S2)
    cells: tuple[tuple[str, str], ...] = ()  # its cell atoms: (pointer parameter, field `v_<t>`), read from the `acc(p.v_<t>, wildcard)` lines (R3-S1)


@dataclasses.dataclass
class WfContext:
    """What the facts of an `f__wf` method are checked against (record 0061 item 11): the ghost functions (name -> Viper result type), the
    method's formals, and the atoms of the method it belongs to (`f` of `f__wf`)."""
    ghosts: dict[str, str]
    formals: frozenset[str]
    atoms: list[RangeAtom]
    method: str = ""  # the `__wf` method's own name without `__wf`: ghost functions are named `hvq_<method>__<formal>_<state>`
    pfs: dict[str, PfInfo] = dataclasses.field(default_factory=dict)  # the `pf_` functions of the file (decision 0062 item 12)

    @property
    def names(self) -> frozenset[str]:
        return frozenset(self.ghosts)

    def hvq(self, name: str):
        """The match (groups `p` the Viper formal, `state`) of a ghost function named `hvq_<method>__<formal>_<pre|post>` after THIS method, else None
        (Viper names are program-wide: an unqualified name collides between two functions)."""
        prefix = f"hvq_{self.method}__"
        return HVQ_TAIL_RE.match(name[len(prefix):]) if self.method and name.startswith(prefix) else None


def _formal_names(src: Source, d: Decl) -> list[str]:
    out = []
    for f in method_formals(src, d):
        m = re.match(rf"^\s*({IDENT})\s*:", f)
        if m:
            out.append(m.group(1))
    return out


def method_atoms(src: Source, d: Decl) -> list[RangeAtom]:
    """The place atoms of the clauses of method `d`: item 16's range rows of the value field (`acc(F.v_T[, amt])` under `offset_of(X) <= k <
    offset_of(X) + count`) and the cell rows `acc(X.v_T, amt)` of a `reads` atom, each with its clause's guard."""
    perm_formals = {m.group("name") for f in method_formals(src, d) if (m := PERM_FORMAL_RE.match(f))}
    out: list[RangeAtom] = []
    for lineno in range(d.line, d.end_line + 1):
        m = QUANT_STMT_RE.match(src.code[lineno - 1])
        if m is None or m.group("kw") not in ("requires", "ensures"):
            continue
        clause, rest = m.group("kw"), m.group("rest")
        # a formal with no atom of its own (an alias `q == p` framed by p's atom) is read under a binder through `loc(block_of(q), q0).v_T`: that read
        # names its element field (audit round 2, issue 1)
        for read in re.finditer(rf"loc\(\s*block_of\(\s*({IDENT})\s*\)\s*,[^()]*\)\s*\.(v_[\w$']+)", rest):
            out.append(RangeAtom(clause, "read", read.group(1), read.group(2), None, None, None))
        guard, body = split_guard(rest)
        q = parse_range_quantifier_full(body) if body is not None else None
        if q is not None and not q[2]:
            k, b, _extra, cons, lo, hi = q
            acc = _range_acc(cons, k, b)
            x = BLOCK_BASE_RE.match(_squash(b)).group(1)
            high = norm_viper(hi)
            if acc is not None and acc[0].startswith("v_") and norm_viper(lo) == f"offset_of({x})" and high.startswith(f"offset_of({x})+"):
                amount = acc[1].strip() if acc[1] is not None else None
                if amount is None or amount in perm_formals or (d.kind == "function" and amount == "wildcard"):
                    out.append(RangeAtom(clause, "range", x, acc[0], amount, high[len(f"offset_of({x})+"):],
                                         None if guard is None else norm_viper(guard)))
            continue
        cell_re = rf"acc\(\s*({IDENT})\.(v_[\w$']+)\s*(?:,\s*({IDENT})\s*)?\)"
        if len(_split_top(rest.strip(), "==>")) > 1 or not re.search(cell_re, rest):
            pieces = [(rest, rest)]
        else:
            # a clause that is one conjunction: each conjunct `guard ==> atoms` is a guarded atom of its own (a `reads` atom under a condition on memory)
            try:
                pieces = [(piece, piece) for piece in _flatten_and(rest)]
            except ValueError:
                pieces = [(rest, rest)]
        for piece, _ in pieces:
            parts = _split_top(piece.strip(), "==>")
            for cell in re.finditer(cell_re, piece):
                if cell.group(3) is None or cell.group(3) in perm_formals or (d.kind == "function" and cell.group(3) == "wildcard"):  # an owns(*p) atom has no amount
                    out.append(RangeAtom(clause, "cell", cell.group(1), cell.group(2), cell.group(3), None,
                                         norm_viper(parts[0]) if len(parts) > 1 else None))
    return out


def _guard_conjuncts(guard: str | None) -> list[str] | None:
    """The sorted normalized conjuncts of a guard (`[]` for none), None if it is not a parenthesized conjunction."""
    if guard is None:
        return []
    try:
        return sorted(norm_viper(x) for x in _flatten_and(guard))
    except ValueError:
        return None


def _expected_guard(*atoms: RangeAtom) -> list[str]:
    out: list[str] = []
    for a in atoms:
        if a.guard is not None:
            out += sorted(norm_viper(x) for x in _flatten_and(a.guard))
    return sorted(out)


def _keep_atom_guard(atom: RangeAtom, state: str) -> set[str]:
    """The conjuncts a keep fact's guard holds for the guard of `atom` (state `pre` for the `requires` atom, `post` for its partner): a conjunct that
    reads no memory verbatim; a condition on memory (a read of a cell) as the same condition over the read ghosts of that state, each read `X.v_<t>`
    standing as `@h` (`pre:`/`post:` marks the ghost's state)."""
    if atom.guard is None:
        return set()
    out: set[str] = set()
    for c in _flatten_and(atom.guard):
        n = norm_viper(c)
        if _heap_free(c):
            out.add(n)
        else:
            out.add(f"{state}:" + re.sub(r"(?<![\w$'.])[A-Za-z_$][\w$']*\.v_[\w$']+", "@h", n))
    return out


def _type_range_problem(field: str, ret: str, ghost: str, s: str, qbody: str) -> str | None:
    """None if `qbody` is the type range of the element type of `field` over the application `ghost(s)` (and `ret` is that type's Viper type)."""
    t = field[len("v_"):]
    want = "Ref" if t.startswith("ptr_") else "Bool" if t == "bool" else "Int"
    if ret != want:
        return f"the ghost function `{ghost}` returns `{ret}`, but the element type `{t}` is `{want}`"
    app = f"{ghost}({s})"
    if want == "Bool":
        return "a `Bool` element has no type range (nothing to assume)"
    if want == "Ref":
        return None if norm_viper(qbody) == norm_viper(_view_wf(app)) else "the range of a pointer element is its well-formedness (as today's scalar ghost has it)"
    try:
        parts = [norm_viper(x) for x in _flatten_and(qbody)]
    except ValueError:
        return "the type's range of the application"
    if len(parts) == 2:
        lo = re.fullmatch(rf"(-?[0-9]+)<={re.escape(app)}", parts[0])
        hi = re.fullmatch(rf"{re.escape(app)}<=(-?[0-9]+)", parts[1])
        if lo and hi:
            low, high = int(lo.group(1)), int(hi.group(1))
            if t in VIEW_INT_RANGES:
                ok = (low, high) == VIEW_INT_RANGES[t]
            elif t.startswith("enum_"):
                ok = low <= 0 <= high
            else:
                ok = (low, high) in VIEW_STANDARD_RANGES
            if ok:
                return None
    return f"the range of the element type `{t}` of the application (not another type's, not a narrower one)"


def wf_fact_verdict(rest: str, ctx: WfContext) -> str | None:
    """Why the `requires` of an `f__wf` method that holds a ghost application or a user binder is none of item 7's assumed facts, or None.
    Each fact is checked against the method's own atoms (record 0061 item 11, audit round 1 B2): a false fact is a vacuous prior or a wrong pass."""
    ghosts = ctx.ghosts
    text = rest.strip()
    inner = _strip_wrap(text)
    atoms_of = lambda p: [a for a in ctx.atoms if a.formal == p]  # noqa: E731
    fields_of = lambda p: {a.field for a in ctx.atoms if a.formal == p}  # noqa: E731  (range and cell atoms: reads(*p) and owns(*p) too)
    flat = _squash(inner)

    # the ground forms (no quantifier): (d) of `reads(*p)`, optionally under the two atoms' guards, and (b) the scalar tie
    if not FORALL_RE.search(text):
        parts = _split_top(text, "==>")
        gtext, body = (parts[0], _strip_wrap(text[len(parts[0]) + len("==>"):])) if len(parts) == 2 else (None, inner)
        flat = _squash(body)
        m = re.fullmatch(rf"({IDENT})\((.+)\)==({IDENT})\((.+)\)", flat)
        if m and m.group(1) in ghosts and m.group(3) in ghosts:
            h1, h2 = ctx.hvq(m.group(1)), ctx.hvq(m.group(3))
            if h1 is None or h2 is None or h1.group("p") != h2.group("p") or {h1.group("state"), h2.group("state")} != {"pre", "post"} \
                    or m.group(2) != m.group(4) or h1.group("state") != "post":
                return "the ground form of a kept `reads(*p)` place is `hvq_p_post(offset_of(p)) == hvq_p_pre(offset_of(p))` of one base"
            p = h1.group("p")
            if m.group(2) not in (f"offset_of({p})", f"offset_of({p})+0"):
                return f"the slot of a `reads(*{p})` place is `offset_of({p})`"
            reads = [a for a in atoms_of(p) if a.kind == "cell" and a.clause == "requires" and a.amount is not None]
            partners = [a for a in atoms_of(p) if a.kind == "cell" and a.clause == "ensures" and a.amount is not None]
            for r in reads:
                for e in partners:
                    if r.field == e.field and r.amount == e.amount and _guard_conjuncts(gtext) == _expected_guard(r, e):
                        return None
            return (f"a kept `reads(*{p})` place needs a `requires` atom of `{p}` with an `ensures` partner (same field and amount) and exactly the "
                    "conjunction of their two guards")
        m = re.fullmatch(rf"({IDENT})\((.+)\)==({IDENT})", flat)
        if m and m.group(1) in ghosts and gtext is None:
            h = ctx.hvq(m.group(1))
            if h is None:
                return "the ghost functions of a fact are named `hvq_<method>__<formal>_pre` and `_post` after this `__wf` method"
            p, st = h.group("p"), h.group("state")
            if not atoms_of(p):
                return f"`{p}` has no place atom in the method's contract: a read of `{p}` under a binder needs one"
            idx = m.group(2)
            if idx != f"offset_of({p})":
                if not idx.startswith(f"offset_of({p})+"):
                    return f"an application at a scalar ghost's slot is `hvq_{p}_{st}(offset_of({p}) + e)`"
                e = idx[len(f"offset_of({p})+"):]
                if not _heap_free(e) or QUANT_FORBIDDEN_RE.search(e) or any(w not in ctx.formals for w in re.findall(rf"(?<![\w$']){IDENT}", e)
                                                                           if not w[0].isdigit()):
                    return "the index of a scalar tie is an expression over the method's formals (no heap, permission or quantifier)"
            hv = HV_NAME_RE.match(m.group(3))
            if hv is None or m.group(3) not in ctx.formals:
                return "the scalar ghost of a tie is a formal of the method named `hv_<id>_pre` or `hv_<id>_post`"
            if hv.group("state") != st:
                return (f"a tie joins the ghost function and the scalar ghost of ONE state (`hvq_{p}_{st}` against `{m.group(3)}`): "
                        "a state-crossing tie makes the prior vacuous (audit round 1, B2)")
            return None
        return "none of item 7's facts: a type range, an application at a scalar ghost's slot, `reads` keeps its value, two parameters on one block"

    guard, body = None, None
    if FORALL_RE.match(inner):
        body = inner
    else:
        parts = _split_top(text, "==>")
        if len(parts) >= 2:
            guard = parts[0]
            body = _strip_wrap(text[len(parts[0]) + len("==>"):])
    if body is None or not FORALL_RE.match(body):
        return "none of item 7's facts: a type range, an application at a scalar ghost's slot, `reads` keeps its value, two parameters on one block"
    q = parse_quantifier(body)
    if q is None:
        return "not `forall s: Int :: { ... } ...`"
    binders, triggers, qbody = q
    if len(binders) != 1 or not WF_BINDER_RE.match(binders[0][0]) or binders[0][1] != "Int":
        return "an assumed fact binds one `Int` binder (`s`)"
    s = binders[0][0]
    if QUANT_FORBIDDEN_RE.search(qbody) or not _heap_free(qbody):
        return "the body of an assumed fact holds `acc`, `perm`, `forperm`, `inhale`, `forall`, `old` or a heap read"
    sets = []
    for t in triggers:
        if len(t) != 1:
            return "an assumed fact has one trigger set per application it relates, each holding that application alone"
        mt = re.fullmatch(rf"({IDENT})\(\s*{re.escape(s)}\s*\)", t[0])
        if mt is None or mt.group(1) not in ghosts:
            return f"the trigger term `{t[0]}` is not an application of a ghost function to the binder"
        sets.append(mt.group(1))
    if guard is not None and _plain_condition_problem(guard):
        return f"the guard of an assumed fact is {_plain_condition_problem(guard)}"
    # (a) the type's range of one application (no guard)
    if len(sets) == 1:
        g = sets[0]
        if len({a for a in re.findall(rf"(?<![\w$'])({IDENT})\(\s*{re.escape(s)}\s*\)", qbody) if a in ghosts}) > 1:
            return "a fact that relates two applications has one trigger set per application it relates (a set holding the one term lets the other never trigger)"
        if guard is not None:
            return "the type range of an application is unguarded"
        h = ctx.hvq(g)
        if h is None:
            return "the ghost functions of a fact are named `hvq_<method>__<formal>_pre` and `_post` after this `__wf` method"
        fields = fields_of(h.group("p"))
        if len(fields) != 1:
            return f"`{h.group('p')}` has no (or no single) element field in the method's atoms: its type range is the element type's"
        return _type_range_problem(next(iter(fields)), ghosts[g], g, s, qbody)
    if len(sets) != 2 or sets[0] == sets[1]:
        return "an assumed fact relates one or two applications: one trigger set each"
    hv = [ctx.hvq(g) for g in sets]
    if None in hv:
        return "the ghost functions of a fact are named `hvq_<method>__<formal>_pre` and `_post` after this `__wf` method"
    a, b = sets
    # (d) a reads place keeps its value: tied to a `reads_range` atom with its partner, the atoms' bounds and the conjunction of their guards
    if hv[0].group("p") == hv[1].group("p") and {hv[0].group("state"), hv[1].group("state")} == {"pre", "post"}:
        p = hv[0].group("p")
        post, pre = (a, b) if hv[0].group("state") == "post" else (b, a)
        parts = _split_top(qbody, "==>")
        if len(parts) != 2 or norm_viper(_strip_wrap(parts[1])) != f"{post}({s})=={pre}({s})":
            return "`forall s :: { post(s) } { pre(s) } offset_of(p) <= s && s < offset_of(p) + n ==> post(s) == pre(s)` of one base"
        try:
            ante = [norm_viper(x) for x in _flatten_and(parts[0])]
        except ValueError:
            return "the antecedent is `offset_of(p) <= s && s < offset_of(p) + n`"
        if len(ante) != 2 or ante[0] != f"offset_of({p})<={s}" or not ante[1].startswith(f"{s}<offset_of({p})+"):
            return "the antecedent is `offset_of(p) <= s && s < offset_of(p) + n`"
        count = ante[1][len(f"{s}<offset_of({p})+"):]
        reads = [x for x in ctx.atoms if x.formal == p and x.kind == "range" and x.clause == "requires" and x.amount is not None]
        partners = [x for x in ctx.atoms if x.formal == p and x.kind == "range" and x.clause == "ensures" and x.amount is not None]
        for r in reads:
            for e in partners:
                if (r.field, r.amount, r.count) == (e.field, e.amount, e.count) and count == r.count \
                        and _guard_conjuncts(guard) == _expected_guard(r, e):
                    return None
        return (f"a keep-its-value fact is tied to a `reads_range({p}, n)` atom of the method's `requires` with its `ensures` partner: its count is the "
                "atoms', its guard exactly the conjunction of the two atoms' guards (an unguarded or wider fact lets a wrong contract's `__wf` verify, "
                "S45 `unsound_evidence_unguarded_keep`)")
    # (c) two parameters on one block, only between parameters whose pointees have the same cell field
    if hv[0].group("state") == hv[1].group("state") and hv[0].group("p") != hv[1].group("p"):
        x, y = hv[0].group("p"), hv[1].group("p")
        if guard is None or norm_viper(guard) not in (f"block_of({x})==block_of({y})", f"block_of({y})==block_of({x})") \
                or nb_not_equal(qbody, a, b, s):
            return "`block_of(X) == block_of(Y) ==> forall s :: { Gx(s) } { Gy(s) } Gx(s) == Gy(s)` of two parameters in one state"
        fx, fy = fields_of(x), fields_of(y)
        if not fx or fx != fy or len(fx) != 1:
            return f"two parameters on one block are related only when their pointees have the same cell field (`{x}`: {sorted(fx)}, `{y}`: {sorted(fy)}; 0028 item 1)"
        return None
    return "none of item 7's facts: a type range, an application at a scalar ghost's slot, `reads` keeps its value, two parameters on one block"


def nb_not_equal(qbody: str, a: str, b: str, s: str) -> bool:
    return norm_viper(qbody) not in (f"{a}({s})=={b}({s})", f"{b}({s})=={a}({s})")


def _flatten_and_keeping_quantifiers(text: str, whole: Callable[[str], bool] | None = None) -> list[str]:
    """`_flatten_and`, but a conjunct that is a quantifier (or a guard over one) is not split at the `&&` of its own antecedent; a conjunct for which
    `whole` holds stays one (the well-formedness row of a pointer to a structure with an array member of structures, `struct_wf_member_array_row`)."""
    text = _strip_wrap(text)
    if FORALL_RE.match(text) or (FORALL_RE.search(text) and split_guard(text)[1] is not None) or (whole is not None and whole(text)):
        return [text]
    parts = _split_top(text, "&&")
    if len(parts) == 1:
        return [text]
    if len(_split_top(text, "||")) > 1 or len(_split_top(text, "==>")) > 1:
        raise ValueError("an unparenthesized mix of `&&` with `||` or `==>`")
    out: list[str] = []
    for part in parts:
        out += _flatten_and_keeping_quantifiers(part, whole)
    return out


def wf_assert_verdict(rest: str, ghosts: frozenset[str]) -> str | None:
    """Why the `assert` of an `f__wf` method that holds a user binder is not `[PRIOR ==>] (forall q<j> :: { G(q<j>), ... } ...)`
    (the prior's quantifiers conjuncts of their own, the conclusion a quantifier or a pure condition), or None."""
    text = rest.strip()
    inner = _strip_wrap(text)

    def quantifier(t: str) -> str | None:
        q = parse_quantifier(t)
        if q is None:
            return "not `forall q<j>: Int :: { G(q<j>), ... } ...`"
        return quantifier_problem(q, ghosts)

    if FORALL_RE.match(inner):
        return quantifier(inner)
    parts = _split_top(text, "==>")
    if len(parts) < 2:
        return "a quantifier of an obligation row stands as the conclusion `PRIOR ==> forall ...`, or alone"
    prior = parts[0]
    conclusion = _strip_wrap(text[len(prior) + len("==>"):])
    try:
        conjuncts = [_strip_wrap(c) for c in _flatten_and_keeping_quantifiers(prior, _is_struct_wf_member_array_row)]
    except ValueError:
        return "the prior is a parenthesized conjunction"
    def guarded(t: str, what: str) -> str | None:
        """A quantifier under at most a plain guard `g ==> `."""
        guard, body = split_guard(t)
        if body is None:
            return f"{what} is a quantifier under at most a guard `g ==> `"
        if guard is not None and (not guard.strip() or _plain_condition_problem(guard)):
            return f"the guard of {what} is a plain condition (no heap, permission or quantifier)"
        why = quantifier(body)
        return f"{what}: {why}" if why else None

    for c in conjuncts:
        if _is_struct_wf_member_array_row(c) or _is_struct_lens_quantifier(c):
            continue
        if FORALL_RE.match(c) or (FORALL_RE.search(c) and split_guard(c)[1] is not None):
            why = guarded(c, "a quantifier of the prior")
            if why:
                return why
        elif FORALL_RE.search(c) or not _heap_free(c):
            return "the prior's quantifiers are conjuncts of their own (stand outside the binder) and its conditions read no heap"
    if FORALL_RE.match(conclusion) or (FORALL_RE.search(conclusion) and split_guard(conclusion)[1] is not None):
        return guarded(conclusion, "the conclusion")
    if FORALL_RE.search(conclusion) or not _heap_free(conclusion):
        return "the conclusion is a quantifier or a pure heap-free condition"
    return None


def ghost_domains(src: Source, decls: list[Decl]) -> dict[str, Decl]:
    """The domains tagged `// cerastes:domain-ok 0061` right above them, by name."""
    out: dict[str, Decl] = {}
    for d in decls:
        if d.kind == "domain" and d.name != "Block":
            tag = tag_above(src, d.line, DOMAIN_TAG_RE)
            if tag is not None and tag.group("record") == GHOST_DOMAIN_RECORD:
                out[d.name] = d
    return out


def ghost_domain_functions(src: Source, d: Decl) -> list[tuple[int, str]]:
    """(line, name) of each ghost function declaration inside the domain `d`."""
    out = []
    for lineno in range(d.line + 1, block_close_line(src, d) + 1):
        m = GHOST_FUNCTION_RE.match(src.code[lineno - 1])
        if m:
            out.append((lineno, m.group("name")))
    return out


def ghost_function_types(src: Source, decls: list[Decl]) -> dict[str, str]:
    """name -> Viper result type of every ghost function of the `domain-ok 0061` domains."""
    out: dict[str, str] = {}
    for d in ghost_domains(src, decls).values():
        for lineno in range(d.line + 1, block_close_line(src, d) + 1):
            m = GHOST_FUNCTION_RE.match(src.code[lineno - 1])
            if m:
                out[m.group("name")] = m.group("ret")
    return out


def lint_ghost_domain(src: Source, d: Decl, add: Callable[[int, str], None]) -> None:
    """A domain tagged `domain-ok 0061` holds bodiless ghost functions and nothing else (no axiom, no body, no contract)."""
    what = f"`domain {d.name}` (decision 0061 item 11)"
    if not record_exists(GHOST_DOMAIN_RECORD):
        add(d.line, f"{what}: `// cerastes:domain-ok {GHOST_DOMAIN_RECORD}` needs internal/decisions/{GHOST_DOMAIN_RECORD}-*.md")
    close = block_close_line(src, d)
    if not d.has_body or not GHOST_DOMAIN_RE.match(src.code[d.line - 1]):
        add(d.line, f"{what}: `domain <name> {{` on a line of its own")
    count = 0
    for lineno in range(d.line + 1, close):
        line = src.code[lineno - 1]
        if not line.strip():
            continue
        m = GHOST_FUNCTION_RE.match(line)
        if m and m.group("name").startswith("hva_"):
            # decision 0062 item 8: an application of a `pure` function: `Int`, `Bool` or `Ref` parameters (none allowed: a `pure` function
            # without parameters); its result is its callee's (`Int` or `Bool`: a `pure` function returns an integer, an enumeration or `bool`),
            # which `lint_ghost_domain_use` compares with the callee's signature
            if not all(HVA_PARAM_RE.match(p) for p in m.group("params").split(",") if p.strip()):
                add(lineno, f"{what}: the ghost function `{m.group('name')}` of an application (decision 0062 item 8) has `Int`, `Bool` or `Ref` "
                            "parameters")
            count += 1
        elif m and all(GHOST_PARAM_RE.match(p) for p in m.group("params").split(",") if p.strip()) and m.group("params").strip():
            count += 1
        else:
            add(lineno, f"{what}: a ghost domain holds bodiless `function <name>(<arg>: Int|Block, ...): Int` declarations and nothing "
                        "else (no axiom, no body, no contract, no other type)")
    if src.code[close - 1].strip() != "}" or close == d.line:
        add(close, f"{what}: the closing `}}` stands on a line of its own")
    if count == 0:
        add(d.line, f"{what}: no ghost function")


def ghost_signature(src: Source, lineno: int) -> tuple[list[str], str]:
    """(parameter types, result type) of the ghost function declared on `lineno`."""
    m = GHOST_FUNCTION_RE.match(src.code[lineno - 1])
    params = [p.split(":")[1].strip() for p in m.group("params").split(",") if p.strip()] if m else []
    return params, m.group("ret") if m else ""


def hva_callee(fn: str, method: str, pfs: dict[str, PfInfo]) -> tuple[str, str] | None:
    """(callee, state) of the ghost function `hva_<method>__<callee>_<pre|post>` when `callee` is a `pf_` function of the file."""
    prefix = f"hva_{method}__"
    if not fn.startswith(prefix):
        return None
    m = HVQ_TAIL_RE.match(fn[len(prefix):])
    if m is None or m.group("p") not in pfs:
        return None
    return m.group("p"), m.group("state")


def hva_function_problem(src: Source, fn: str, method: str, signature: tuple[list[str], str], pfs: dict[str, PfInfo]) -> str | None:
    """Why the ghost function `fn` is not an application ghost of `method`'s `__wf` (named `hva_<method>__<callee>_<pre|post>` after a `pf_` function
    of the file, with that function's parameter types and result type), or None."""
    found = hva_callee(fn, method, pfs)
    if found is None:
        return (f"it is not named `hva_{method}__<callee>_<pre|post>` with `<callee>` a `pf_<name>__<id>` function of this file (Viper names are "
                "program-wide: the method's name qualifies the function)")
    info = pfs[found[0]]
    want_params, want_ret = [t for _n, t in info.params], info.ret
    if signature != (want_params, want_ret):
        return (f"its signature ({', '.join(signature[0])}): {signature[1]} is not its callee `{found[0]}`'s "
                f"({', '.join(want_params)}): {want_ret}")
    return None


def lint_ghost_domain_use(src: Source, decls: list[Decl], domains: dict[str, Decl], add: Callable[[int, str], None]) -> None:
    """One domain per `f__wf` method: a ghost function is used only in a `__wf` method, each domain by one method, each method with one domain."""
    pfs = pf_infos(src, decls)
    method_of: dict[int, Decl] = {}
    for d in decls:
        if d.kind == "method":
            for n in range(d.line, d.end_line + 1):
                method_of[n] = d
    inside = {n for d in domains.values() for n in range(d.line, block_close_line(src, d) + 1)}
    users: dict[str, set[str]] = {}
    domains_of: dict[str, set[str]] = {}
    for name, d in domains.items():
        fns = [f for _l, f in ghost_domain_functions(src, d)]
        if not fns:
            continue
        word = re.compile(r"(?<![\w$'])(?:" + "|".join(map(re.escape, fns)) + r")\s*\(")
        users[name] = set()
        for lineno, code in enumerate(src.code, start=1):
            if lineno in inside or not word.search(code):
                continue
            owner = method_of.get(lineno)
            if owner is None or not owner.name.endswith("__wf"):
                add(lineno, f"a ghost function of `domain {name}` outside a `__wf` method (decision 0061 item 11: the heap-free `f__wf` alone reads them)")
                continue
            users[name].add(owner.name)
            domains_of.setdefault(owner.name, set()).add(name)
        for lineno, fn in ghost_domain_functions(src, d):
            for owner in sorted(users[name]):
                base = owner[:-len('__wf')]
                if fn.startswith("hva_"):
                    why = hva_function_problem(src, fn, base, ghost_signature(src, lineno), pfs)
                    if why:
                        add(lineno, f"the ghost function `{fn}` of `domain {name}` (decision 0062 item 8): {why}")
                elif not fn.startswith(f"hvq_{base}__"):
                    add(lineno, f"the ghost function `{fn}` of `domain {name}` is not named `hvq_<method>__<formal>_<pre|post>` after the method `{owner[:-len('__wf')]}` "
                                "that uses its domain (Viper names are program-wide: the method's name qualifies the function, decision 0061 item 7)")
        if not users[name]:
            add(d.line, f"`domain {name}` (decision 0061 item 11): no `__wf` method uses its ghost functions")
        elif len(users[name]) > 1:
            add(d.line, f"`domain {name}` (decision 0061 item 11): one domain per function, but {', '.join(sorted(users[name]))} use its ghost functions")
    for method, names in sorted(domains_of.items()):
        if len(names) > 1:
            add(next(d.line for d in decls if d.kind == "method" and d.name == method),
                f"`method {method}` (decision 0061 item 11): one domain per function, but it uses the ghost functions of {', '.join(sorted(names))}")


@dataclasses.dataclass
class QuantifiedLines:
    """The verdict of item 11's rules on the lines they own: line -> (ok, why, marked wildcard row)."""
    verdicts: dict[int, tuple[bool, str, bool]] = dataclasses.field(default_factory=dict)

    @property
    def wildcard_rows(self) -> frozenset[int]:
        return frozenset(n for n, (ok, _w, marked) in self.verdicts.items() if ok and marked)


def _brace_net(code: str) -> int:
    return code.count("{") - code.count("}")


def loop_clause_scopes(src: Source, d: Decl) -> dict[int, frozenset[str]]:
    """Decision 0061 item 9 (question 258), scoped to the loop (audit round 3, issue 3): for every line of method `d` that may hold a quantifier in its
    prior, the normalized quantifiers it may name.
      * an `invariant` line of a `while` loop: the user quantifiers of the EARLIER `invariant` lines of that same loop (a later clause, or another
        loop's clause, is not a prior of it);
      * a framing `assert`: the user quantifiers of all `invariant` lines of the loop it frames.  An entry `assert` is in the run of `assert`, `label`,
        `var` and assignment lines that ends at the loop's `while`; a back-edge `assert` is in the run of `assert` lines that ends the loop's body.
        (The run cannot say which clause an assert checks, so every clause of that loop is admitted there.)
    Every other line, an `assert` elsewhere included, has no entry: no quantifier may stand in its prior."""
    out: dict[int, frozenset[str]] = {}
    first, last = d.line, d.end_line
    lineno = first
    while lineno <= last:
        code = src.code[lineno - 1]
        if re.match(r"^\s*while\b", code) is None:
            lineno += 1
            continue
        header = lineno
        clauses: list[tuple[int, str]] = []
        invariants: list[int] = []
        j = header + 1
        while j <= last:
            m = QUANT_STMT_RE.match(src.code[j - 1])
            if m is not None and m.group("kw") == "invariant":
                invariants.append(j)
                guard, body = split_guard(m.group("rest"))
                if body is not None and USER_QUANT_RE.match(body):
                    clauses.append((j, norm_viper(_strip_wrap(body))))
            elif _brace_net(src.code[j - 1]) > 0:
                break
            j += 1
        for n in invariants:
            out[n] = frozenset(text for (c, text) in clauses if c < n)
        all_clauses = frozenset(text for (_c, text) in clauses)
        # the entry run: lines of asserts, labels, declarations and assignments before the `while`
        n = header - 1
        while n >= first:
            c = src.code[n - 1]
            kw = QUANT_STMT_RE.match(c)
            if kw is not None and kw.group("kw") == "assert":
                out[n] = out.get(n, frozenset()) | all_clauses
            elif not (c.strip() == "" or re.match(r"^\s*(?:label|var)\b", c) or ":=" in c):
                break
            n -= 1
        # the body: from the first line after the invariants (it opens the braces) to the line that closes them
        depth = 0
        close = None
        k = j
        while k <= last:
            depth += _brace_net(src.code[k - 1])
            if depth <= 0:
                close = k
                break
            k += 1
        if close is not None:
            n = close - 1
            while n > j:
                kw = QUANT_STMT_RE.match(src.code[n - 1])
                if kw is None or kw.group("kw") != "assert":
                    break
                out[n] = out.get(n, frozenset()) | all_clauses
                n -= 1
        lineno = header + 1
    return out


def _reads_as_markers(norm_text: str, accept: Callable[[str, str], bool], cross: bool = False) -> str:
    """The normalized text with each read `(membership ? loc(block_of(p), q<j>).v_<t> : 0)` whose membership is an interval `[r, r + count)` that
    `accept(r, count)` allows replaced by the marker `@rd(p,q<j>)`.  `r` is `p` in a contract (the read pointer's own interval); with `cross`, as in a
    loop's `invariant` line, it may be another pointer (a site atom's, the method's formal `p_a` while the read names the loop's variable `v_a`):
    the membership test is about the cell's block and offset, so the read is of a cell that atom holds either way."""
    def site(m: re.Match) -> str:
        return f"@rd({m.group(1)},{m.group(2)})" if accept(m.group(3), norm_viper(m.group(4))) and (cross or m.group(1) == m.group(3)) else m.group(0)

    return re.sub(rf"block_of\(loc\(block_of\(({IDENT})\),(q[0-9]+)\)\)==block_of\(({IDENT})\)&&offset_of\(\3\)<=offset_of\(loc\(block_of\(\1\),\2\)\)"
                  rf"&&offset_of\(loc\(block_of\(\1\),\2\)\)<offset_of\(\3\)\+([^?]+?)\?loc\(block_of\(\1\),\2\)\.v_[\w$']+:0", site, norm_text)


def app_guard_quantifier_problem(code: str, scope: str, pfs: dict[str, PfInfo], site_counts: dict[str, set[str]], cross: bool = False) -> str | None:
    """Decision 0062 items 7 and 12 (cut 2): a line holds a user quantifier (`forall q<j>`) only as a parenthesized conjunct of the guard of an application
    (in a `pf_` body, a method's `requires` or `ensures`, a call's or a return's `assert` row, a loop's `invariant` line: with `cross`, whose
    reads may name another pointer than the site atom whose interval is their membership), and then it is the quantified `requires` clause of an applied
    callee at that application's arguments.  `scope` is the text that holds the applications (the line; a `pf_` function's whole body), `site_counts` the
    counts of the atoms of the site (a method's or function's own atoms, by pointer) whose interval the element reads' membership may be, besides the
    callee's own interval at the arguments.  The reads are compared as markers.  None when each quantifier of the line is one of those; else why not."""
    expected: set[str] = set()
    allowed = {k: set(v) for k, v in site_counts.items()}
    pending: list[tuple[PfInfo, dict[str, str]]] = []
    for m in PF_ANY_RE.finditer(scope):
        info = pfs.get(m.group("name"))
        if info is None or not info.quantified:
            continue
        args = parenthesized_list(scope, m.end() - 1)
        if len(args) != len(info.params):
            continue
        mapping = {pname: f"({a.strip()})" for (pname, _t), a in zip(info.params, args)}
        for pname, cnt, _f in info.atoms:
            arg = _strip_wrap(mapping.get(pname, ""))
            allowed.setdefault(arg, set()).add(norm_viper(_substitute(cnt, mapping)))
        pending.append((info, mapping))
    for info, mapping in pending:
        counts = {pname: cnt for pname, cnt, _f in info.atoms}
        expected |= {norm_viper(_substitute(_reads_as_markers(norm_viper(q), lambda p, c: counts.get(p) == c), mapping)) for q in info.quantified}
    for m in re.finditer(r"(?<![\w$'])forall\s+q[0-9]+\s*:", code):
        before = code[:m.start()].rstrip()
        if not before.endswith("("):
            return "a quantifier of an application's guard stands parenthesized, as a conjunct"
        depth, end = 1, -1
        for i in range(m.start(), len(code)):
            depth += (code[i] == "(") - (code[i] == ")")
            if depth == 0:
                end = i
                break
        if end < 0:
            return "a quantifier of an application's guard is not closed on its line"
        got = _reads_as_markers(norm_viper(code[m.start():end]), lambda p, c: c in allowed.get(p, set()), cross)
        if got not in expected:
            return ("a quantifier of a line is the quantified clause of the `requires` of a function applied there, at that application's "
                    "arguments, its element reads over the interval of the callee at the arguments or of an atom of the site (the guard of the "
                    "application, cut 2)")
    return None


def pf_body_quantifier_problem(src: Source, d: Decl, pfs: dict[str, PfInfo], code: str) -> str | None:
    """A body line of a `pf_` function (decision 0062 item 12): `app_guard_quantifier_problem` over the whole body, the function's own atoms the site's."""
    body = "\n".join(src.code[max(d.body_line, 1) - 1:d.end_line])
    own = pfs.get(d.name)
    site: dict[str, set[str]] = {}
    for pname, cnt, _f in (own.atoms if own is not None else ()):
        site.setdefault(pname, set()).add(cnt)
    return app_guard_quantifier_problem(code, body, pfs, site)


def classify_quantified_lines(src: Source, decls: list[Decl]) -> QuantifiedLines:
    """Item 11's lines: a clause or statement of a method that holds a user quantifier (`forall q<j>`), and in an `f__wf` method every one
    that holds a user binder (`q<j>`, `s<j>`) or a ghost application.  Lines of the older shapes (`forall k`) are not here."""
    out = QuantifiedLines()
    domain_lines: set[int] = set()
    for d in decls:
        if d.kind == "domain":
            domain_lines.update(range(d.line, d.end_line + 1))
    ghost_types = ghost_function_types(src, decls)
    ghosts = frozenset(ghost_types)
    ghost_re = re.compile(r"(?<![\w$'])(?:" + "|".join(map(re.escape, sorted(ghosts))) + r")\s*\(") if ghosts else None
    pfs = pf_infos(src, decls)
    hva_re = re.compile(r"(?<![\w$'])(?:" + "|".join(map(re.escape, sorted(g for g in ghosts if g.startswith("hva_")))) + r")\s*\(") \
        if any(g.startswith("hva_") for g in ghosts) else None
    for d in decls:
        pure_function = d.kind == "function" and PF_NAME_RE.match(d.name) is not None
        if d.kind != "method" and not pure_function:
            continue
        wf = d.name.endswith("__wf") and not pure_function
        loop_scopes = None
        ctx = None
        if wf:
            # the definition check of a `pf_` function (decision 0062 item 9) reads its atoms from the function's own `requires` lines
            base = next((b for b in decls if b.name == d.name[:-len("__wf")] and (b.kind == "method" or PF_NAME_RE.match(b.name))), None)
            ctx = WfContext(ghost_types, frozenset(_formal_names(src, d)), method_atoms(src, base) if base is not None else [], d.name[:-len("__wf")], pfs)
        for lineno in range(d.line, d.end_line + 1):
            if lineno in domain_lines:
                continue
            code = src.code[lineno - 1]
            m = QUANT_STMT_RE.match(code)
            if m is None:
                if pure_function and lineno > d.body_line > 0 and USER_QUANT_RE.search(code):
                    why = pf_body_quantifier_problem(src, d, pfs, code)
                    out.verdicts[lineno] = (why is None, "" if why is None else f"{Q11}: {why}", False)
                continue
            kind, rest = m.group("kw"), m.group("rest")
            if pure_function:
                # decision 0062 item 12: a user quantifier of 0061's shape stands in a `pf_` function's `requires` and `ensures` (its body and
                # measure hold none: the fallback refuses them)
                if USER_QUANT_RE.search(code):
                    out.verdicts[lineno] = contract_line_verdict(kind, rest) if kind in ("requires", "ensures") else \
                        (False, f"{Q11}: a `pf_` function holds a user quantifier in a `requires` or an `ensures` (decision 0062 item 12), not in a `{kind}`", False)
                continue
            if wf:
                has_user = WF_QUANT_RE.search(code) is not None
                has_ghost = ghost_re is not None and ghost_re.search(code) is not None
                has_hva = hva_re is not None and hva_re.search(code) is not None
                if has_hva:
                    why = hva_line_problem(kind, rest, code, ctx, ghosts)
                    out.verdicts[lineno] = (why is None, "" if why is None else f"a line of an `f__wf` method over an application ghost (decision 0062 item 8): {why}", False)
                    continue
                if not (has_user or (has_ghost and kind in ("requires", "ensures", "invariant"))):
                    continue
                if kind == "requires":
                    why = wf_fact_verdict(rest, ctx)
                elif kind == "assert":
                    why = wf_assert_verdict(rest, ghosts)
                else:
                    why = f"a `{kind}` of an `f__wf` method holds no quantifier and no ghost application (item 7's facts are `requires`, its rows `assert`)"
                out.verdicts[lineno] = (why is None, "" if why is None else f"{Q11}: in an `f__wf` method: {why}", False)
            elif USER_QUANT_RE.search(code) and PF_ANY_RE.search(code) and kind in ("requires", "ensures", "assert", "invariant"):
                # the quantified clause of an applied callee, a conjunct of the application's guard (decision 0062 item 7, cut 2); in a
                # loop's `invariant` line (the user's conjunct, whose value is `(G ? pf : 0)`) and the lines that restate it under `prior ==>`
                # (the obligation lines of item 9's form), whatever else the line holds (the quantifier check reads only its quantifiers)
                site: dict[str, set[str]] = {}
                for atom in method_atoms(src, d):
                    if atom.kind == "range" and atom.count is not None:
                        site.setdefault(atom.formal, set()).add(atom.count)
                why = app_guard_quantifier_problem(code, code, pfs, site, kind == "invariant")
                out.verdicts[lineno] = (why is None, "" if why is None else f"{Q11}: {why}", False)
            elif USER_QUANT_RE.search(code):
                if kind not in ("requires", "ensures", "invariant", "assert"):
                    out.verdicts[lineno] = (False, f"{Q11}: a user quantifier stands in a `requires`, `ensures`, `invariant` or `assert`, not an `{kind}`", False)
                else:
                    if loop_scopes is None:
                        loop_scopes = loop_clause_scopes(src, d)
                    out.verdicts[lineno] = contract_line_verdict(kind, rest, loop_scopes.get(lineno, frozenset()))
    return out


def user_wildcard_row(code: str) -> bool:
    """Whether `code` is item 10's marked `wildcard` row (an unguarded `assert`)."""
    m = QUANT_STMT_RE.match(code)
    if m is None or m.group("kw") != "assert" or not FORALL_RE.match(m.group("rest").strip()):
        return False
    q = parse_quantifier(m.group("rest"))
    return q is not None and wildcard_row_problem(q) is None and "wildcard" in m.group("rest")


WEA_ASSIGN_RE = re.compile(rf"^\s*(?P<n>wea_[\w$']*)\s*:=\s*(?P<f>.+?)\s*$")
WEA_NAME_RE = re.compile(r"(?<![\w$'])wea_[\w$']*")
IF_NOT_RE = re.compile(r"^\s*if\s*\(\s*!\s*\((?P<x>.+)\)\s*\)\s*\{\s*$")
REF_LOCAL_RE = re.compile(rf"^\s*var\s+(?P<n>{IDENT})\s*:\s*Ref\s*$")
BOOL_LOCAL_RE = re.compile(rf"^\s*var\s+(?P<n>{IDENT})\s*:\s*Bool\s*$")
LEN_FACT_RE = re.compile(r"len\(((?:[^()]|\((?:[^()]|\((?:[^()]|\([^()]*\))*\))*\))+)\)==([0-9]+)")


def method_locals(src: Source, d: Decl, regex: re.Pattern[str]) -> set[str]:
    """The names of the locals of kind `regex` (`var x: Ref`, `var x: Bool`) of the method `d`."""
    out: set[str] = set()
    if d.has_body:
        for n in range(d.body_line, d.end_line + 1):
            if (m := regex.match(src.code[n - 1])) is not None:
                out.add(m.group("n"))
    return out


def method_all_len_facts(src: Source, d: Decl) -> dict[str, set[int]]:
    """`method_len_facts` over EVERY line of the method (its loop invariants hold the counts of a carrier's block: `len(member(P, j)) == N`)."""
    out = dict(method_len_facts(src, d))
    aliases: list[tuple[str, str]] = []
    for n in range(d.line, d.end_line + 1):
        line = norm_viper(src.code[n - 1])
        for b, k in LEN_FACT_RE.findall(line):
            out.setdefault(b, set()).add(int(k))
        # the pure conjunct of a ghost pointer, `invariant G ==> E == abp_<i>`: the ghost IS the loaded pointer E, so E's member counts are its own
        am = re.match(rf"^invariant.*==>\(*(?P<e>.+)==(?P<a>abp_[\w$']*)\)*$", line)
        if am is not None:
            aliases.append((am.group("e"), am.group("a")))
    for e, a in aliases:
        for b, counts in list(out.items()):
            if b.startswith(f"member({e},"):
                out.setdefault(f"member({a},{b[len(f'member({e},'):]}", set()).update(counts)
    return out


def wea_snapshots(src: Source, d: Decl) -> tuple[dict[str, tuple[str, int]], list[tuple[int, str]]]:
    """Decision 0064 item 15 (Part D, a written site's or an owner's array part in a loop): the `wea_<...>` Bool locals of `d` are assigned `true` and, once,
    `forall k :: { loc(B, k) } 0 <= k && k < N ==> loc(B, k).d_t` (the flags of the part's block B at the loop's entry): ({name: (B, N)}, problems as
    (line, text)).  A `wea_` local that is not a declared Bool, is assigned anything else, or has no or several flag assignments is a problem."""
    info: dict[str, tuple[str, int]] = {}
    problems: list[tuple[int, str]] = []
    if not d.has_body:
        return info, problems
    bools = method_locals(src, d, BOOL_LOCAL_RE)
    for n in range(d.body_line, d.end_line + 1):
        m = WEA_ASSIGN_RE.match(src.code[n - 1])
        if m is None:
            continue
        name, rhs = m.group("n"), m.group("f")
        if name not in bools:
            problems.append((n, f"`{name}` is assigned but is no declared `Bool` local"))
        elif rhs.strip() == "true":
            continue
        elif FORALL_RE.match(rhs):
            q = parse_block_row(rhs)
            fl = None if q is None else re.fullmatch(rf"loc\({re.escape(q['b'])},{re.escape(q['k'])}\)\.d_[\w$']+", q["cons"])
            if q is None or fl is None or q["lo"] != "0":
                problems.append((n, f"a `wea_` snapshot is `true` or `forall k: Int :: {{ loc(B, k) }} 0 <= k && k < N ==> loc(B, k).d_t` (the flags of one block part)"))
            elif name in info:
                problems.append((n, f"`{name}` is assigned a flag quantifier twice"))
            else:
                info[name] = (q["b"], int(q["hi"]))
        else:
            problems.append((n, f"a `wea_` snapshot is `true` or a quantifier over the flags of one block part, not `{rhs}`"))
    return info, problems


def bound_block_of(src: Source, d: Decl, local: str) -> str | None:
    """The block term B of a local assigned exactly once `loc(B, 0)` (the bound local of a part's rows), normalized, else None."""
    rights: list[str] = []
    for n in range(d.body_line, d.end_line + 1):
        for m in ASSIGNMENT_RE.finditer(src.code[n - 1]):
            if m.group("n") == local:
                rights.append(_statement_rhs(src.code[n - 1], m.end()))
    if len(rights) != 1:
        return None
    bm = re.fullmatch(r"loc\((?P<b>.+),\s*0\s*\)", rights[0].strip(), re.S)
    return None if bm is None else norm_viper(bm.group("b"))


def wea_tie_problem(guard: str | None, b: str, n: int, cons_is_flag: bool, info: dict[str, tuple[str, int]]) -> str | None:
    """The `wea_<...>` names in the guard of a flag row of block B (count N): at most one; it must be a snapshot of B's own flags with the same count, and the
    row's consequent the flag (a `wea` guards the flags kept, never another row)."""
    names = WEA_NAME_RE.findall(guard or "")
    if not names:
        return None
    if len(set(names)) != 1:
        return "a guard that names several `wea_` snapshots"
    if names[0] not in info:
        return f"`{names[0]}` is no flag snapshot of the method"
    if not cons_is_flag:
        return f"the row under the flag snapshot `{names[0]}` is not the flag"
    if info[names[0]] != (b, n):
        return f"the flag snapshot `{names[0]}` is of `{info[names[0]][0]}` with count {info[names[0]][1]}, the row's block is `{b}` with count {n}"
    return None


# A heap read inside a carrier row's block term (`member(loc(member(v_p, 1), 0).v_ptr_struct_0_S, 0)`): no loop invariant or row of the emitter has one.
CARRIER_HEAP_READ_RE = re.compile(r"\.v_ptr_")


def carrier_row_problem(kw: str, guard: str | None, q: dict[str, str], gbs: set[str], arrays: dict[str, int], roots: set[str],
                        lens: dict[str, set[int]], info: dict[str, tuple[str, int]]) -> str | None:
    """Decision 0064 item 15 (Part D): an `invariant` of a written site's or an owner's array part in a loop, `G ==> forall k :: { loc(member(X, j), k) } 0 <= k &&
    k < N ==> C`, X a variable (the site's pointer, the owner's variable, a ghost pointer `abp_...`): a row `block_row_problem` accepts at FULL permission
    (`acc(F.v_t)`, `acc(F.d_t)`, no amount), the flag, the type's range under the flag, over the block `member(X, j)` with the count N some `len(member(X, j)) ==
    N` of the method states; the guard reads the heap only by a canonical null test of a canonical load, and a `wea_` name in it is the flag snapshot of this
    very block."""
    if guard is not None and not _heap_free(guard) and not _guard_has_only_null_tests(guard):
        return "the guard of a carrier row reads the heap other than by a canonical null test of a canonical load"
    if CARRIER_HEAP_READ_RE.search(q["b"]):
        return ("the block of a carrier row (an invariant, a marked or an ending row, the values kept) is `member(X, j)` with X a variable: a block reached "
                "through a heap read stands in a contract clause only (`block_row_problem`), never in a loop's invariant or rows (decision 0064 item 15)")
    why = block_row_problem(kw, guard, q, gbs, arrays, roots, lens, frozenset())
    if why:
        return why
    cell = f"loc({q['b']},{q['k']})"
    return wea_tie_problem(guard, q["b"], int(q["hi"]), re.fullmatch(rf"{re.escape(cell)}\.d_[\w$']+", q["cons"]) is not None, info)


def enclosing_conditions(src: Source, d: Decl, lineno: int) -> list[str | None]:
    """The blocks that enclose line `lineno` of the method `d`, outermost first: the squashed condition `C` of an `if (C) {`, None for any other block (a loop body,
    an `else`).  Read from the lines' own braces (the emitter writes every `{` at a line's end and every `}` alone or as `} else {`)."""
    stack: list[str | None] = []
    for n in range(max(d.body_line, 1) + 1, lineno):
        code = src.code[n - 1].strip()
        if code.startswith("}"):
            if stack:
                stack.pop()
            if code.endswith("{"):
                stack.append(None)
        elif code.endswith("{") and not code.endswith("{ }"):
            im = re.match(r"^if\s*\((?P<c>.*)\)\s*\{$", code)
            stack.append(_sq(im.group("c")) if im is not None else None)
    return stack


def bound_row_guard_problem(src: Source, d: Decl, local: str, lineno: int) -> str | None:
    """A marked row over the bound local `local` (a slot `local := loc(B, 0)` that the method assigns once) stands under the `if (G) {` that encloses that
    assignment, when the assignment stands under one: the local holds no block where G fails, so a row outside G reads a block nobody bound (decision 0064
    item 15, a carrier's entry and back-edge rows)."""
    sites = [n for n in range(d.body_line, d.end_line + 1) for m in ASSIGNMENT_RE.finditer(src.code[n - 1]) if m.group("n") == local]
    if len(sites) != 1:
        return None
    needed = [c for c in enclosing_conditions(src, d, sites[0]) if c is not None]
    if not needed:
        return None
    have = enclosing_conditions(src, d, lineno)
    if needed[-1] not in have:
        return f"the row over `{local}` stands outside the `if ({needed[-1]})` under which `{local}` is bound"
    return None


def _block_equality(conjunct: str) -> bool:
    """`block_of(A) == block_of(B)`, the test inside a place's disjointness term."""
    sides = _split_top(_strip_wrap(conjunct.strip()), "==")
    return len(sides) == 2 and all(_strip_wrap(x.strip()).startswith("block_of(") for x in sides)


def _na_conjunct(conjunct: str) -> bool:
    """Whether a conjunct of a written part's guard is one of the disjointness terms that `wblock_part_distinct` and the place tests write (decision 0064
    item 15, step B): `!(<guard> && <range test>)` with a `block_of(A) == block_of(B)` test inside it, `X ==> block_of(A) != block_of(B)` or a bare
    `block_of(A) != block_of(B)`; any other conjunct (a `!`-led null test included) is the atom's own guard."""
    c = _strip_wrap(conjunct.strip())
    if c.startswith("!"):
        try:
            return any(_block_equality(x) for x in _flatten_and(_strip_wrap(c[1:].strip())))
        except ValueError:
            return False
    sides = _split_top(c, "==>")
    last = _strip_wrap(sides[-1].strip())
    if len(sides) > 2:
        return False
    parts = _split_top(last, "!=")
    return len(parts) == 2 and all(_strip_wrap(x.strip()).startswith("block_of(") for x in parts)


def _row_block_guard(src: Source, lineno: int) -> str | None:
    """The condition C of the `if (C) {` block that holds line `lineno` and nothing but quantified `assert` rows (the entry and back-edge rows of a guarded
    atom: `if (g) { assert forall ... acc(F.v_t)  assert forall ... acc(F.d_t) }`), else None: a block with any other statement is a branch of the program."""
    def is_row(i: int) -> bool:
        return src.code[i - 1].strip().startswith("assert forall")
    top, bottom = lineno, lineno
    while top > 1 and is_row(top - 1):
        top -= 1
    while bottom < len(src.code) and is_row(bottom + 1):
        bottom += 1
    im = re.match(r"^if\s*\((?P<c>.*)\)\s*\{$", src.code[top - 2].strip()) if top >= 2 else None
    closes = bottom < len(src.code) and src.code[bottom].strip() == "}"
    return im.group("c") if im is not None and closes else None


def _guard_set(guard: str | None, skip_na: bool) -> frozenset[str] | None:
    """The normalized conjuncts of `guard` (the disjointness terms left out when `skip_na`), or None when the guard does not split into conjuncts."""
    if guard is None:
        return frozenset()
    try:
        parts = _flatten_and(guard)
    except ValueError:
        return None
    return frozenset(norm_viper(c) for c in parts if not (skip_na and _na_conjunct(c)))


def written_part_shape(q: dict[str, str]) -> tuple[str, ...] | None:
    """What a quantified row `q` (`parse_block_row`) over `member(X, j)` says of its slot F = `loc(B, k)`, as a key the rows and the invariants of a written part
    share: ("acc", f) for `acc(F.f)` with no amount, ("flag", t) for `F.d_t`, ("range", t, R) for the type's range R of `F.v_t` (alone, or under the flag
    in an invariant), else None."""
    cell = f"loc({q['b']},{q['k']})"
    cons = q["cons"]
    m = re.fullmatch(rf"acc\({re.escape(cell)}\.(?P<f>[vd]_[\w$']+)\)", cons)
    if m is not None:
        return ("acc", m.group("f"))
    m = re.fullmatch(rf"{re.escape(cell)}\.d_(?P<t>[\w$']+)", cons)
    if m is not None:
        return ("flag", m.group("t"))
    sides = _split_top(cons, "==>")
    if len(sides) == 2:
        am = re.fullmatch(rf"{re.escape(cell)}\.d_(?P<t>[\w$']+)", sides[0])
        if am is not None and _block_type_range(sides[1], cell, am.group("t")):
            return ("range", am.group("t"), sides[1])
    elif len(sides) == 1:
        rm = re.search(rf"{re.escape(cell)}\.v_(?P<t>{IDENT})", cons)
        if rm is not None and _block_type_range(cons, cell, rm.group("t")):
            return ("range", rm.group("t"), cons)
    return None


def written_part_partners(src: Source, d: Decl, gbs: set[str], arrays: dict[str, int], roots: set[str], lens: dict[str, set[int]],
                          info: dict[str, tuple[str, int]]) -> set[tuple[str, int, tuple[str, ...], frozenset[str]]]:
    """Decision 0064 item 15, step B (a written `owns(*p)` or `owns(p->a)` in a loop invariant): the loop invariants of method `d` over a member block, each
    one `carrier_row_problem` accepts, as (block, count, `written_part_shape`, the atom's own guard: the guard's conjuncts without the disjointness terms)."""
    out: set[tuple[str, int, tuple[str, ...], frozenset[str]]] = set()
    if not d.has_body:
        return out
    for n in range(d.body_line, d.end_line + 1):
        m = QUANT_STMT_RE.match(src.code[n - 1])
        if m is None or m.group("kw") != "invariant":
            continue
        guard, body = split_guard(m.group("rest"))
        cq = parse_block_row(body) if body is not None else None
        if cq is None or not cq["b"].startswith("member(") or carrier_row_problem("invariant", guard, cq, gbs, arrays, roots, lens, info) is not None:
            continue
        shape = written_part_shape(cq)
        own = _guard_set(guard, True)
        if shape is not None and own is not None:
            out.add((cq["b"], int(cq["hi"]), shape, own))
    return out


def written_part_kept(q: dict[str, str]) -> tuple[str, str] | None:
    """(the type t, the label) of a values-kept conjunct of a written part (decision 0064 item 15, step B): `F.v_t == old[L](F.v_t)` over one slot F = `loc(B, k)`
    of the block (the consequent of `parse_block_row`: no grouping parentheses survive, so the call of `old[L]` reads `old[L]F.v_t`), else None."""
    cell = re.escape(f"loc({q['b']},{q['k']})")
    m = re.fullmatch(rf"{cell}\.v_(?P<t>[\w$']+)==old\[(?P<l>[^\]]+)\]{cell}\.v_(?P=t)", q["cons"])
    return None if m is None else (m.group("t"), m.group("l"))


def written_part_kept_problem(guard: str | None, q: dict[str, str], kept: tuple[str, str], gbs: set[str], arrays: dict[str, int], roots: set[str],
                              lens: dict[str, set[int]], info: dict[str, tuple[str, int]],
                              partners: set[tuple[str, int, tuple[str, ...], frozenset[str]]]) -> str | None:
    """Decision 0064 item 15, step B: why the invariant `G ==> forall k :: { loc(member(X, j), k) } 0 <= k && k < N ==> F.v_t == old[L](F.v_t)` (the values of a
    written part kept) is not one, or None.  The block, the count and the guard are those of a row `carrier_row_problem` accepts (judged on the flag of the same
    slot, no flag snapshot in the guard), and the block, the count and the atom's own guard are those of an `acc` of that very field among the invariants of the
    method: the values a part keeps are the values of a part the loop holds."""
    flag = dict(q, cons=f"loc({q['b']},{q['k']}).d_{kept[0]}")
    if WEA_NAME_RE.search(guard or ""):
        return "a values-kept conjunct stands under no flag snapshot"
    why = carrier_row_problem("invariant", guard, flag, gbs, arrays, roots, lens, info)
    if why:
        return why
    own = _guard_set(guard, True)
    if own is None:
        return "a guard that is not a conjunction of conditions"
    if not any(p[0] == q["b"] and p[1] == int(q["hi"]) and p[2] == ("acc", f"v_{kept[0]}") and p[3] == own for p in partners):
        return (f"no loop invariant of the method holds `acc` of `v_{kept[0]}` over `{q['b']}` with count {q['hi']} under the same guard: the values a loop "
                "keeps are those of a part it holds")
    return None


def written_part_row_problem(src: Source, d: Decl, lineno: int, ending: bool, guard: str | None, q: dict[str, str], gbs: set[str], arrays: dict[str, int],
                             roots: set[str], lens: dict[str, set[int]], info: dict[str, tuple[str, int]],
                             partners: set[tuple[str, int, tuple[str, ...], frozenset[str]]]) -> str | None:
    """Decision 0064 item 15, step B: why the quantified `assert` row `q` over `member(X, j)` (line `lineno` of `d`; `ending`: the `if (!(G ==> forall ...)) {
    assert false }` row of the flags or the range, else a marked permission row) is no entry or back-edge row of a written `owns` part of a loop, or None.  The
    row is a row `carrier_row_problem` accepts at full permission (no amount, one field, the count a length fact of the method, a guard that reads the heap only
    by canonical null tests); a marked row is the `acc` row, an ending row the flag or the range; the block, the count, the field and the atom's own guard are
    those of a loop invariant of the same method (`written_part_partners`): the row stands under that guard, as the `if (G) {` block of nothing but the part's
    marked rows (a marked row) or as `G ==> ` (an ending row), and under no disjointness term (a row the part's invariant switches off is still checked)."""
    shape = written_part_shape(q)
    if shape is None:
        if re.fullmatch(rf"acc\(loc\({re.escape(q['b'])},{re.escape(q['k'])}\)\.[vd]_[\w$']+,.+\)", q["cons"]):
            return "the row of a written part has an amount: the part is held at full permission, with no amount term"
        return "the consequent is not `acc(F.f)`, the flag or the type's range of one slot of the part"
    if ending == (shape[0] == "acc"):
        return ("an ending row (`if (!(...)) { assert false }`) is the flag or the range, never an `acc`" if ending
                else "a marked row is an `acc` of one field, not the flag or the range (those are ending rows)")
    effective = guard
    if not ending:
        if guard is not None:
            return "a marked row stands under the `if (G) {` block of the part's rows, never as `G ==> forall`"
        effective = _row_block_guard(src, lineno)
    why = carrier_row_problem("assert", effective, q, gbs, arrays, roots, lens, info)
    if why:
        return why
    row_guard = _guard_set(effective, False)
    if row_guard is None:
        return "a guard that is not a conjunction of conditions"
    if any(_na_conjunct(c) for c in (_flatten_and(effective) if effective is not None else [])):
        return "the row stands under one of the part's disjointness terms: the part's invariant is switched off there, its entry and back-edge rows are not"
    matching = [p for p in partners if p[0] == q["b"] and p[1] == int(q["hi"]) and p[2] == shape]
    if not matching:
        return (f"no loop invariant of the method states `{shape[0]}` of `{q['b']}` with count {q['hi']} and the same field: a row of a written part has the "
                "invariant it carries")
    if all(p[3] != row_guard for p in matching):
        return (f"the row's guard {sorted(row_guard) if row_guard else 'none'} is not the atom's own guard of its invariant "
                f"({'; '.join(str(sorted(p[3])) for p in matching)})")
    return None


def lint_quantified_rows(src: Source, decls: list[Decl], entries: list[MapEntry] | None, add: Callable[[int, str], None],
                         map_name: str = "", user: QuantifiedLines | None = None, view_lines: set[int] | frozenset[int] = frozenset()) -> None:
    """Decision 0059 items 7, 8 and 12: where `forall` and `old[label]` may stand, the shape of a marked row, and the marks.  `view_lines`: the lines of
    the typed views that `lint_views` accepted (decision 0064 item 15: their quantified rows are pinned there, as `alloc_arr_<t>`'s are)."""
    skip: set[int] = set(pf_requires_problems(src, decls)) | set(view_lines)  # decision 0062 item 6: reported by `lint_pf_requires`
    for d in decls:
        if d.kind == "domain" or (d.kind == "method" and (d.name.startswith((ARR_ALLOC_PREFIX, ARR_ZERO_PREFIX)) or is_pinned_array_copy(src, d))):
            skip.update(range(d.line, d.end_line + 1))
        elif d.kind == "method" and (d.name == PROGRAM_START or (d.name.startswith(ALLOC_PREFIX) and STRUCT_ALLOC_RE.match(d.name[len(ALLOC_PREFIX):]))):
            skip.update(range(d.line, d.end_line + 1))  # trusted rows with their own rules (`array_global_group`, `struct_allocation_problem`)
    gb_names = {m.group("name") for l in src.code if (m := GB_DECL_RE.match(l))}
    array_counts = array_global_counts(src, decls)
    struct_counts = struct_array_counts(src, decls)
    block_ctx: dict[int, tuple[set[str], dict[str, set[int]]]] = {}

    def block_ctx_of(md: Decl) -> tuple[set[str], dict[str, set[int]]]:
        if md.line not in block_ctx:
            block_ctx[md.line] = ({*_formal_names(src, md), *(n for n, _t in method_results(src, md))}, method_len_facts(src, md))
        return block_ctx[md.line]

    bound_ctx: dict[int, dict[str, tuple[set[int], str]]] = {}
    labels_before: dict[int, set[str]] = {}
    for d in decls:
        if d.kind != "method":
            continue
        seen: set[str] = set()
        for n in range(d.line, d.end_line + 1):
            labels_before[n] = set(seen)
            lm = re.match(r"^\s*label\s+([\w$']+)\s*$", src.code[n - 1])
            if lm:
                seen.add(lm.group(1))
    marked_lines: set[int] = set()
    method_of: dict[int, Decl] = {}
    for d in decls:
        if d.kind == "method":
            for n in range(d.line, d.end_line + 1):
                method_of[n] = d
    formals_of: dict[int, frozenset[str]] = {}

    def perm_formals(lineno: int) -> frozenset[str]:
        d = method_of.get(lineno)
        if d is None:
            return frozenset()
        if d.line not in formals_of:
            formals_of[d.line] = frozenset(m.group("name") for f in method_formals(src, d) if (m := PERM_FORMAL_RE.match(f)))
        return formals_of[d.line]

    amounts_of_method: dict[int, dict[str, tuple[str, str]]] = {}

    def amounts_at(lineno: int) -> dict[str, tuple[str, str]]:
        d = method_of.get(lineno)
        if d is None:
            return {}
        if d.line not in amounts_of_method:
            amounts_of_method[d.line] = amount_snapshots(src, d)
        return amounts_of_method[d.line]

    wea_of_method: dict[int, tuple[dict[str, tuple[str, int]], list[tuple[int, str]]]] = {}

    def wea_info_at(lineno: int):
        d = method_of.get(lineno)
        if d is None:
            return {}, []
        if d.line not in wea_of_method:
            wea_of_method[d.line] = wea_snapshots(src, d)
        return wea_of_method[d.line]

    carrier_ctx: dict[int, tuple[set[str], dict[str, set[int]]]] = {}
    wp_ctx: dict[int, tuple[set[str], dict[str, set[int]], dict[str, tuple[str, int]], set]] = {}

    def carrier_ctx_of(md: Decl) -> tuple[set[str], dict[str, set[int]]]:
        if md.line not in carrier_ctx:
            carrier_ctx[md.line] = ({*block_ctx_of(md)[0], *method_locals(src, md, REF_LOCAL_RE)}, method_all_len_facts(src, md))
        return carrier_ctx[md.line]

    if user is None:
        user = classify_quantified_lines(src, decls)
    for lineno, code in enumerate(src.code, start=1):
        has_q, has_old = FORALL_RE.search(code), OLD_LABEL_RE.search(code)
        verdict = user.verdicts.get(lineno)
        if verdict is not None and lineno not in skip:
            # decision 0061 item 11: a user quantifier, a marked `wildcard` row, a row or fact of an `f__wf` method
            ok_u, why_u, marked_u = verdict
            if marked_u and ok_u:
                marked_lines.add(lineno)
            if not ok_u:
                add(lineno, why_u)
            if has_old:
                add(lineno, "`old[label]` outside item 8's loop conjuncts of decision 0059 (the flags kept, the values kept)")
            continue
        if not (has_q or has_old) or lineno in skip:
            continue
        if (wm := WEA_ASSIGN_RE.match(code)) is not None and FORALL_RE.search(code):
            # a flag snapshot of a carrier (`wea_snapshots` reads and judges every such assignment)
            md_w = method_of.get(lineno)
            if md_w is not None and any(n == lineno for n, _w in wea_info_at(lineno)[1]):
                add(lineno, next(w for n, w in wea_info_at(lineno)[1] if n == lineno))
            elif md_w is None:
                add(lineno, "a `wea_` snapshot outside a method")
            continue
        code_stmt = code
        ending_row = False
        if (im := IF_NOT_RE.match(code)) is not None and FORALL_RE.search(code) and lineno in method_of:
            # the ENDING row of a carrier's flag or range conjunct, `if (!(G ==> (forall ...))) { assert false }` (decision 0038 item 4's shape for the
            # rows that end their path): judged as the `assert` of the same text, and its next line must be `assert false`
            code_stmt = "assert " + im.group("x")
            ending_row = True
            nxt = src.code[lineno] if lineno < len(src.code) else ""
            if nxt.strip() != "assert false":
                add(lineno, "the ending row of a carrier holds `assert false` on the next line (decision 0038 item 4)")
        m = QUANT_STMT_RE.match(code_stmt)
        kind = m.group("kw") if m else ""
        guard, body = split_guard(m.group("rest")) if m else (None, None)
        plain = bool(m and FORALL_RE.match(m.group("rest").strip()))  # no guard, no wrapping: the older rows' text
        starts_q = body is not None
        q = parse_range_quantifier_full(body) if starts_q else None
        if q is None and starts_q:
            q = parse_leaf_row(body)  # decision 0064 item 17: a row over a leaf of an array of structures (a seventh field, its receiver)
        leaf_row = q is not None and len(q) > 6
        old_labels: list[str] = []
        kept_row = False
        ok = False
        why = ""
        wp_row = parse_block_row(body) if (starts_q and q is None and kind == "assert" and m is not None and lineno in method_of) else None
        if wp_row is not None and wp_row["b"].startswith("member("):
            wmd = method_of[lineno]
            if wmd.line not in wp_ctx:
                wroots, wlens = carrier_ctx_of(wmd)
                wp_ctx[wmd.line] = (wroots, wlens, wea_info_at(lineno)[0],
                                    written_part_partners(src, wmd, gb_names, array_counts, wroots, wlens, wea_info_at(lineno)[0]))
            if not wp_ctx[wmd.line][3]:
                wp_row = None  # a method with no loop over a written part: the older rule's refusal stands
        if wp_row is not None and wp_row["b"].startswith("member("):
            # decision 0064 item 15, step B: an entry or back-edge row of a written part (a quantified `assert` over `member(X, j)`)
            wroots, wlens, winfo, wpartners = wp_ctx[wmd.line]
            why = written_part_row_problem(src, wmd, lineno, ending_row, guard, wp_row, gb_names, array_counts, wroots, wlens, winfo, wpartners) or ""
            ok = not why
            if ok and not ending_row:
                marked_lines.add(lineno)
        elif starts_q and plain and kind in ("assert", "exhale") and (q is None or _row_acc(q) is not None):
            # decision 0059 item 7's marked row (an `acc` of one field) or the range `exhale`
            acc = _row_acc(q) if q is not None else None
            shaped = (q is not None and not q[2] and acc is not None and (acc[1] is None or _heap_free(acc[1]))
                      and (kind == "assert" or acc[1] is None))
            if shaped and kind == "assert":
                marked_lines.add(lineno)
                if lineno in method_of and (xg := BLOCK_BASE_RE.match(_squash(q[1]))) is not None \
                        and (gw := bound_row_guard_problem(src, method_of[lineno], xg.group(1), lineno)) is not None:
                    add(lineno, gw)
            if shaped and acc is not None and acc[1] is not None and acc[1] in amounts_at(lineno) and acc[1] not in perm_formals(lineno):
                # decision 0064 item 15 (Part D): the entry and back-edge rows of a heap array part at its snapshot amount: the amount is the
                # snapshot of the row's OWN first slot and field (`amount_snapshots`), over the range from that slot's offset
                x_own = _own_first_slot_range(q, amounts_at(lineno))
                if x_own is None or amounts_at(lineno)[acc[1]] != (x_own, acc[0]):
                    add(lineno, f"a marked range row at the amount `{acc[1]}`, a snapshot `perm(...)` of another place or another field than the first slot of the "
                                f"row's own range (decision 0064 item 15): the amount of `{acc[0]}` over `{_squash(q[1])}` is the one `perm(X.{acc[0]})` of that range's base X")
            if not shaped:
                add(lineno, f"a quantified `{kind}` that is not a marked range row of decision 0059 item 7: "
                            f"`{kind} forall k: Int :: {{ loc(block_of(X), k) }} lo <= k && k < hi ==> acc(loc(block_of(X), k).f)` "
                            "with one field (an `assert` row may carry an amount) and X a variable, B, lo, hi and the amount reading no heap")
            ok = True  # reported above
        elif q is not None and kind == "invariant":
            c = _loop_conjunct(q, perm_formals(lineno), amounts_at(lineno), guard is not None)
            ok = c is not None and (guard is None or not GUARD_BAD_RE.search(guard))
            if not ok:
                add(lineno, "a quantified loop `invariant` that is not one of decision 0059 item 8's four conjuncts "
                            "(permission over the range with no amount or the method's own `Perm` formal, the flags kept, the type's "
                            "range, the values kept), alone or under a guard `g ==> `")
                ok = True  # reported above
                old_labels = []
            else:
                old_labels = c[1]
        elif leaf_row and not (kind in ("requires", "ensures", "assert") and m is not None and lineno in method_of):
            # a leaf row other than a marked row, a range `exhale` or a loop conjunct, outside a method's clauses and a method's pure rows
            why = "a row over a leaf of an array of structures is a marked `assert`, a range `exhale` or a loop conjunct here"
        elif q is not None and kind in ("requires", "ensures", "assert") and m is not None and lineno in method_of:
            # decision 0060 item 16: a range atom's quantifiers in a method's clauses and its pure rows; decision 0064 items 7 and 17: the same rows per
            # leaf path of the element (the receiver, a chain of member steps below the slot, is the trigger and the cell) and the length facts
            md0 = method_of[lineno]
            if md0.line not in bound_ctx:
                bound_ctx[md0.line] = bound_locals(src, md0, decls, gb_names, array_counts, struct_counts)
            why = range_atom_row_problem(kind, guard, q, perm_formals(lineno), bound_ctx[md0.line].get) or ""
            if not why and WEA_NAME_RE.search(guard or ""):
                # decision 0064 item 15 (Part D): the ending row under a flag snapshot `wea_<...>`: the bound local's block and the count are the snapshot's own
                xw = BLOCK_BASE_RE.match(_squash(q[1])).group(1)
                nw = re.fullmatch(rf"offset_of\({re.escape(xw)}\)\+([0-9]+)", _sq(q[5]))
                flag_w = re.fullmatch(rf"loc\({re.escape(_squash(q[1]))},{re.escape(q[0])}\)\.d_[\w$']+", _squash(q[3])) is not None
                why = wea_tie_problem(guard, bound_block_of(src, md0, xw) or "", int(nw.group(1)) if nw else -1, flag_w, wea_info_at(lineno)[0]) or ""
            if why and leaf_row:
                why = "a row over a leaf of an array of structures: " + why
            ok = not why
        elif q is None and not starts_q and kind == "assert" and m is not None and (wsa := wf_store_assert_problem(m.group("rest"))) is not None:
            # the check of a pointer value a body stores: its well-formedness row under the liveness guard (decision 0043 item 8, 0064 item 3)
            why = wsa
            ok = not why
        elif q is None and not starts_q and kind in ("requires", "ensures", "assert") and m is not None and lineno in method_of \
                and (swf := struct_wf_member_array_row(m.group("rest"))) is not None:
            # decision 0064 items 3 and 17: the well-formedness row of a pointer to a structure with an array member of structures
            why = swf
            ok = not why
        elif q is None and not starts_q and kind == "assert" and m is not None and lineno in method_of \
                and (swp := struct_wf_prior_problem(m.group("rest"))) is not None:
            # the same row as a conjunct of the prior of an `f__wf` method's obligation
            why = swp
            ok = not why
        elif q is None and not starts_q and kind in ("requires", "ensures") and m is not None and lineno in method_of \
                and (bcl := block_clause_problem(kind, m.group("rest"), gb_names, array_counts, *block_ctx_of(method_of[lineno]), perm_formals(lineno))) is not None:
            # decision 0064 item 15: an `owns_block` site's whole block part as one conjunction, its array leaves' quantified conjuncts rows over a member block
            why = bcl
            ok = not why
        elif q is None and starts_q and kind == "invariant" and m is not None and lineno in method_of \
                and (cq := parse_block_row(body)) is not None:
            # decision 0064 item 15 (Part D): a carrier's rows over `member(X, j)` in a loop's invariants (`carrier_row_problem`)
            cmd = method_of[lineno]
            roots_c, lens_c = carrier_ctx_of(cmd)
            kept_w = written_part_kept(cq) if cq["b"].startswith("member(") else None
            if kept_w is None:
                why = carrier_row_problem(kind, guard, cq, gb_names, array_counts, roots_c, lens_c, wea_info_at(lineno)[0]) or ""
            else:
                # decision 0064 item 15, step B: the values of a written part kept (`F.v_t == old[L](F.v_t)`)
                if cmd.line not in wp_ctx:
                    wp_ctx[cmd.line] = (roots_c, lens_c, wea_info_at(lineno)[0],
                                        written_part_partners(src, cmd, gb_names, array_counts, roots_c, lens_c, wea_info_at(lineno)[0]))
                why = written_part_kept_problem(guard, cq, kept_w, gb_names, array_counts, roots_c, lens_c, wea_info_at(lineno)[0], wp_ctx[cmd.line][3]) or ""
                if not why:
                    old_labels = [kept_w[1]]
                    kept_row = True
            ok = not why
        elif q is None and starts_q and kind in ("requires", "ensures") and m is not None and lineno in method_of \
                and (bq := parse_block_row(body) or parse_block_leaf_row(body)) is not None:
            # decision 0063 item 15: a row over a block TERM in a line that has no Viper local (a method's contract)
            md = method_of[lineno]
            if md.line not in block_ctx:
                block_ctx[md.line] = ({*_formal_names(src, md), *(n for n, _t in method_results(src, md))}, method_len_facts(src, md))
            roots, lens = block_ctx[md.line]
            why = block_row_problem(kind, guard, bq, gb_names, array_counts, roots, lens, perm_formals(lineno)) or ""
            ok = not why
        if not ok:
            if has_q:
                add(lineno, "`forall` outside the shapes of decision 0059: the `alloc_arr_<t>` and `zero_arr_<t>` contracts, "
                            "a marked range row (`assert`), the range `exhale` and item 8's loop conjuncts (and `domain Block`'s axioms) "
                            "and decision 0060 item 16's range-atom rows (the quantifiers of a method's `requires` and `ensures` and "
                            "their pure `assert` rows)" + (f": {why}" if why else ""))
        if has_old:
            found = re.findall(r"(?<![\w$'])old\s*\[\s*([^\]]*?)\s*\]", code)
            if not (ok and (q is not None or kept_row) and kind == "invariant" and old_labels and len(found) == len(old_labels)):
                add(lineno, "`old[label]` outside item 8's loop conjuncts of decision 0059 (the flags kept, the values kept)")
            else:
                for lab in found:
                    if lab not in labels_before.get(lineno, set()):
                        add(lineno, f"`old[{lab}]` names a label that no earlier `label {lab}` of the method defines (decision 0059 item 8)")
    if entries is None:
        return
    mname = map_name
    for e in entries:
        if not e.expected_reason:
            if e.vpr_line in marked_lines:
                add(e.vpr_line, f"a marked range row (decision 0059 item 7) whose map entry (`{mname}:{e.map_line}`) has no "
                                f"expected reason: it must be `{MAP_MARKED_ID}[insufficient.permission]`, or the driver treats "
                                "its `insufficient.permission` as an internal error")
            continue
        if e.vpr_line > len(src.code) or not marked_row_shape(src.code[e.vpr_line - 1]):
            add(e.vpr_line, f"the map entry `{mname}:{e.map_line}` carries the expected reason `{e.expected_reason}` but its line is "
                            "not an `assert` of decision 0059 item 7's shape (`assert forall k: Int :: { loc(B, k) } lo <= k && "
                            "k < hi ==> acc(loc(B, k).f)`, one field, B, lo, hi and the amount reading no heap)")


# ---------------------------------------------------------------------------
# Pure functions (decision 0062 item 12)
# ---------------------------------------------------------------------------

PF_FORBIDDEN_RE = re.compile(r"(?<![\w$'.])(?:acc|perm|old|forperm|inhale|assume)(?![\w$'])")
HVA_APP_START_RE = re.compile(r"(?<![\w$'.])hva_[\w$']*\s*\(")
FUNCTION_SIGNATURE_RE = re.compile(r"^\s*(?P<params>.*?)\)\s*:\s*(?P<ret>Int|Bool|Ref|Perm)\b", re.S)


def pf_signature(src: Source, d: Decl) -> tuple[tuple[tuple[str, str], ...], str]:
    """((formal, type) ..., result type) of the function `d`, from its header; ("", ...) parts are empty when it does not parse."""
    text = method_header_text(src, d)
    start = text.find("(", text.find(d.name) + len(d.name))
    if start < 0:
        return (), ""
    params: list[tuple[str, str]] = []
    for part in parenthesized_list(text, start):
        m = re.match(rf"^\s*({IDENT})\s*:\s*([A-Za-z]+)\s*$", part)
        if m is None:
            return (), ""
        params.append((m.group(1), m.group(2)))
    after = text[start:]
    depth, end = 0, -1
    for i, ch in enumerate(after):
        depth += (ch == "(") - (ch == ")")
        if depth == 0:
            end = i
            break
    rm = re.match(r"\s*:\s*([A-Za-z]+)", after[end + 1:]) if end >= 0 else None
    return tuple(params), rm.group(1) if rm else ""


def _memory_free_conjuncts(text: str) -> list[str]:
    """The conjuncts of the clause `text` that read no memory (no heap read, permission, quantifier or application of a `pure` function)."""
    try:
        parts = _flatten_and(text)
    except ValueError:
        parts = [text.strip()]
    return [p for p in parts if _heap_free(p) and not FORALL_RE.search(p) and not PF_ANY_RE.search(p)]


PF_RESULT_TAG_RE = re.compile(r"//\s*cerastes:result\s+(-?[0-9]+)\s+(-?[0-9]+)\s*$")


def _pf_atom(text: str, params: dict[str, str]) -> tuple[str, str, str] | None:
    """(pointer parameter, count, field) of an `acc(loc(block_of(p), k).v_<t>, wildcard)` range line of a `pf_` function, else None."""
    if not WILDCARD_RE.search(text):
        return None
    q = parse_range_quantifier_full(text)
    if q is None or q[2]:
        return None
    k, b, _extra, cons, lo, hi = q
    m = BLOCK_BASE_RE.match(_squash(b))
    if m is None:
        return None
    x = m.group(1)
    acc = _range_acc(cons, k, b)
    high = _squash(_strip_wrap(hi))
    n = high[len(f"offset_of({x})+"):] if high.startswith(f"offset_of({x})+") else ""
    if params.get(x) != "Ref" or acc is None or not acc[0].startswith("v_") or acc[1] is None or acc[1].strip() != "wildcard" \
            or _squash(_strip_wrap(lo)) != f"offset_of({x})" or not n:
        return None
    return (x, n, acc[0])


def pf_infos(src: Source, decls: list[Decl]) -> dict[str, PfInfo]:
    """The `pf_` functions of the file with their signatures, the memory-free conjuncts of their `requires` and `ensures`, their user quantifiers and
    range atoms, and the result tag of the signature line."""
    out: dict[str, PfInfo] = {}
    for d in decls:
        if d.kind != "function" or PF_NAME_RE.match(d.name) is None:
            continue
        params, ret = pf_signature(src, d)
        requires: list[str] = []
        ensures: list[str] = []
        quantified: list[str] = []
        atoms: list[tuple[str, str, str]] = []
        cells: list[tuple[str, str]] = []
        opaque = False
        for kw, _line, text in clause_texts(src, d):
            if kw == "requires":
                requires += _memory_free_conjuncts(text)
                opaque = opaque or PF_ANY_RE.search(text) is not None
                try:
                    parts = _flatten_and(text)
                except ValueError:
                    parts = [text.strip()]
                for part in parts:
                    if USER_QUANT_RE.search(part):
                        quantified.append(part.strip())
                    else:
                        atom = _pf_atom(part, dict(params))
                        if atom is not None:
                            atoms.append(atom)
                        cell = re.fullmatch(rf"acc\(({IDENT})\.(v_[\w$']+),wildcard\)", norm_viper(part))
                        if cell is not None and dict(params).get(cell.group(1)) == "Ref":
                            cells.append((cell.group(1), cell.group(2)))
            elif kw == "ensures":
                ensures += _memory_free_conjuncts(text)
        tag = None
        m = PF_RESULT_TAG_RE.search(src.comments[d.line - 1]) if d.line - 1 < len(src.comments) else None
        if m is not None:
            tag = (int(m.group(1)), int(m.group(2)))
        out[d.name] = PfInfo(d.name, params, ret, tuple(requires), tuple(ensures), opaque, tuple(quantified), tuple(atoms), tag, tuple(cells))
    return out


def decreases_line_set(src: Source) -> set[int]:
    """Every line of a `decreases` clause (its keyword line and the lines that continue it, as `decreases_measures` reads it)."""
    out: set[int] = set()
    for lineno, code in enumerate(src.code, start=1):
        if not DECREASES_LINE_RE.match(code):
            continue
        out.add(lineno)
        for k, more in enumerate(src.code[lineno:], start=lineno + 1):
            if more.strip() and CLAUSE_END_RE.match(more):
                break
            out.add(k)
    return out


def _pf_applications(text: str) -> list[tuple[str, int, int]]:
    """(callee, start, end) of every application of a `pf_` name in `text` (end: just after the closing parenthesis)."""
    out = []
    for m in PF_ANY_RE.finditer(text):
        end = _group_end(text, m.end() - 1)
        out.append((m.group("name"), m.start(), end + 1 if end >= 0 else len(text)))
    return out


def _pf_body_text(src: Source, d: Decl) -> list[tuple[int, str]]:
    """(line, text) of the body of the function `d`; on the line of the opening brace, what follows it."""
    out = []
    for n in range(d.body_line, d.end_line + 1):
        code = src.code[n - 1]
        if n == d.body_line:
            code = code.split("{", 1)[1] if "{" in code else code
        out.append((n, code))
    return out


PRELUDE_SIMPLE = frozenset({"c_pow2", "c_shr", "c_div", "c_rem", "c_udiv", "c_urem"})


def prelude_function_name(name: str) -> bool:
    """A name the prelude emits (src/viper/prelude.cpp): the powers of two, the shift, the divisions and remainders, the bitwise functions and the
    `^` constant-mask functions."""
    return name in PRELUDE_SIMPLE or BIT_FUNCTION_RE.match(name) is not None or xor_mask_function_ok(name)


def lint_inert_functions(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0062 item 12 (the owner's H', question 273): a top-level `function` that is neither a prelude function nor `pf_<name>__<id>` applies
    only prelude functions, so it can neither recurse nor reach a `pf_` function (the cycle rules and the `decreases` lines are those of `pf_`
    functions alone)."""
    functions = {d.name for d in decls if d.kind == "function"}
    for d in decls:
        if d.kind != "function" or prelude_function_name(d.name) or PF_NAME_RE.match(d.name):
            continue
        for n in range(d.line, d.end_line + 1):
            code = src.code[n - 1]
            for m in re.finditer(r"(?<![\w$'.])([A-Za-z_$][\w$']*)\s*\(", code):
                name = m.group(1)
                if name == d.name and code[:m.start()].rstrip().endswith("function"):
                    continue
                if name in functions and not prelude_function_name(name):
                    add(n, f"`function {d.name}` is neither a prelude function nor `pf_<name>__<id>`, so it applies only prelude functions: "
                           f"it applies `{name}` (decision 0062 item 12)")


def lint_pf_wf_methods(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0062 item 9 (S3): every `pf_<name>__<id>` function has its definition-check method `pf_<name>__<id>__wf` (a function whose `__wf` is
    missing is a function whose body and contract no row checked, and facts (i) and (ii) about its applications rest on those rows)."""
    methods = {d.name for d in decls if d.kind == "method"}
    for d in decls:
        if d.kind == "function" and PF_NAME_RE.match(d.name) and d.name + "__wf" not in methods:
            add(d.line, f"`function {d.name}` has no definition-check method `{d.name}__wf`: its body and contract would be checked by no row "
                        "(decision 0062 item 9)")


def lint_pf_result_tags(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0062 item 12 (S2): the `// cerastes:result <lo> <hi>` tag of a `pf_` signature line is a standard C integer range (fact (i)'s range is
    compared with it)."""
    for d in decls:
        if d.kind == "function" and PF_NAME_RE.match(d.name):
            m = PF_RESULT_TAG_RE.search(src.comments[d.line - 1]) if d.line - 1 < len(src.comments) else None
            if m is not None and (int(m.group(1)), int(m.group(2))) not in VIEW_STANDARD_RANGES:
                add(d.line, f"`function {d.name}`: the result tag `{m.group(1)} {m.group(2)}` is no standard C integer range (decision 0062 item 12)")


def lint_prelude_closure(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0062 item 12 (N9): a prelude function applies only prelude functions, and the application graph of the prelude functions has no cycle
    (H' rests on it: a `pf_` function that applies a prelude function reaches no `pf_` function through it and no recursion that the `decreases`
    lines would miss)."""
    functions = {d.name for d in decls if d.kind == "function"}
    prelude = {d.name for d in decls if d.kind == "function" and prelude_function_name(d.name)}
    edges: dict[str, set[str]] = {n: set() for n in prelude}
    for d in decls:
        if d.name not in prelude or d.kind != "function":
            continue
        for n in range(d.line, d.end_line + 1):
            code = src.code[n - 1]
            for m in re.finditer(r"(?<![\w$'.])([A-Za-z_$][\w$']*)\s*\(", code):
                name = m.group(1)
                if name == d.name and code[:m.start()].rstrip().endswith("function"):
                    continue
                if name in functions:
                    if name not in prelude:
                        add(n, f"the prelude `function {d.name}` applies `{name}`, which is no prelude function (decision 0062 item 12)")
                    else:
                        edges[d.name].add(name)
    state: dict[str, int] = {}

    def visit(node: str) -> bool:
        state[node] = 1
        for nxt in sorted(edges[node]):
            if state.get(nxt) == 1 or (nxt not in state and visit(nxt)):
                return True
        state[node] = 2
        return False

    for d in decls:
        if d.kind == "function" and d.name in prelude and d.name not in state and visit(d.name):
            add(d.line, f"the prelude functions apply each other in a cycle through `{d.name}` (decision 0062 item 12: the prelude has no recursion)")
            break


def lint_prelude_decreases(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0062 item 12: lowering prints an empty `decreases` line on every prelude function of a program that holds a `pf_` function, and only
    there (Silicon's termination check of a `pf_` function that applies a prelude function would otherwise fail, S46 (a)); in any other program an
    empty `decreases` on a prelude function is refused.  (A prelude function's measure is refused by the position-map rule: it has no entry.)"""
    if any(d.kind == "function" and PF_NAME_RE.match(d.name) for d in decls):
        return
    measures = decreases_measures(src)
    for d in decls:
        if d.kind == "function" and prelude_function_name(d.name):
            for kw, line, _text in clause_texts(src, d):
                if kw == "decreases" and measures.get(line) == "":
                    add(line, f"`function {d.name}`: an empty `decreases` on a prelude function only stands in a program that holds a `pf_` function "
                              "(decision 0062 item 12)")


def pf_requires_problems(src: Source, decls: list[Decl]) -> dict[int, tuple[str | None, bool]]:
    """Decision 0062 item 6, form Min (S46 (a), `tests/viper-smoke/memory/s46_a_min.vpr`): line -> (why not, whether it is an admitted `acc(..., wildcard)`
    line) for each `requires` line of a `pf_` function that holds no user quantifier (`forall q<j>`: item 11's, read by `classify_quantified_lines`).
    The lines, one conjunct each (`push_clauses` splits them): a parameter's type range (an integer's bounds, a pointer's well-formedness
    `p == null || WF`); a value clause (no memory, permission, quantifier or flag; it may apply another `pure` function); for a range atom on a
    pointer parameter `p` with count `n` the pure lines `p != null`, the well-formedness of `p`, `0 <= n`, `offset_of(p) + n <= len(block_of(p))` (the
    combined line is read too), then `forall k: Int :: { loc(block_of(p), k) } offset_of(p) <= k && k < offset_of(p) + n ==> acc(loc(block_of(p), k).v_<t>,
    wildcard)`, then the element range (the type's bounds of `loc(block_of(p), k).v_<t>`, none for `bool`: a `bool` element has no such line, so it is
    not required); for a cell atom the same of one slot (`offset_of(p) < len(block_of(p))`, `acc(p.v_<t>, wildcard)`, the bounds of `p.v_<t>`).  No
    `d_<t>` line, no `lv` line, no flag line (form Full is not admitted), and `wildcard` stands only in the `acc` line."""
    out: dict[int, tuple[str | None, bool]] = {}
    for d in decls:
        if d.kind != "function" or PF_NAME_RE.match(d.name) is None or not d.has_body:
            continue
        params = dict(pf_signature(src, d)[0])
        refs = [n for n, t in params.items() if t == "Ref"]
        notnull: set[str] = set()
        wf: set[str] = set()
        nonneg: set[str] = set()
        bound: set[tuple[str, str]] = set()
        cell_bound: set[str] = set()
        acc_rows: dict[tuple[str, str, str], int] = {}  # (pointer, count or "", field) -> its line
        elements: set[tuple[str, str, str]] = set()  # the atoms whose element-range line was read
        for kw, line, text in clause_texts(src, d):
            if kw != "requires" or USER_QUANT_RE.search(text):
                continue
            flat = norm_viper(text)
            why: str | None = None
            wild = False
            if re.search(r"(?<![\w$'])(?:lvloc|lv)(?![\w$'])|\.\s*d_|(?<![\w$'])d_", text):
                why = "a flag or a liveness line (`d_<t>`, `lv`) is no line of form Min: every flag and the liveness are rows at the application"
            elif WILDCARD_RE.search(text):
                q = parse_range_quantifier_full(text)
                cell = re.fullmatch(rf"acc\(({IDENT})\.(v_[\w$']+),wildcard\)", flat)
                if cell and params.get(cell.group(1)) == "Ref":
                    x = cell.group(1)
                    acc_rows[(x, "", cell.group(2))] = line
                    if not (x in notnull and x in wf and x in cell_bound):
                        why = f"the cell atom of `{x}` needs its pure lines first: `{x} != null`, the well-formedness of `{x}` and `offset_of({x}) < len(block_of({x}))`"
                    wild = why is None
                elif q is not None and not q[2]:
                    k, b, _extra, cons, lo, hi = q
                    x = BLOCK_BASE_RE.match(_squash(b)).group(1)
                    acc = _range_acc(cons, k, b)
                    high = _squash(_strip_wrap(hi))
                    n = high[len(f"offset_of({x})+"):] if high.startswith(f"offset_of({x})+") else ""
                    if x not in refs or acc is None or not acc[0].startswith("v_") or acc[1] is None or acc[1].strip() != "wildcard" \
                            or _squash(_strip_wrap(lo)) != f"offset_of({x})" or not n:
                        why = ("`wildcard` stands only in `forall k: Int :: { loc(block_of(p), k) } offset_of(p) <= k && k < offset_of(p) + n ==> "
                               "acc(loc(block_of(p), k).v_<t>, wildcard)` of a pointer parameter `p`")
                    else:
                        acc_rows[(x, n, acc[0])] = line
                        if not (x in notnull and x in wf and norm_viper(n) in nonneg and (x, norm_viper(n)) in bound):
                            why = (f"the range atom of `{x}` needs its pure lines first: `{x} != null`, the well-formedness of `{x}`, `0 <= n` and "
                                   f"`offset_of({x}) + n <= len(block_of({x}))`")
                        wild = why is None
                else:
                    why = "`wildcard` stands only in the `acc` line of an atom's `forall` or cell (form Min)"
            elif FORALL_RE.search(text):
                q = parse_range_quantifier_full(text)
                if q is None or q[2]:
                    why = "a quantified `requires` of a `pure` function is the range atom's `acc` or element-range line, or item 11's user quantifier"
                else:
                    k, b, _extra, cons, lo, hi = q
                    x = BLOCK_BASE_RE.match(_squash(b)).group(1)
                    high = _squash(_strip_wrap(hi))
                    n = high[len(f"offset_of({x})+"):] if high.startswith(f"offset_of({x})+") else ""
                    cell = f"loc({_squash(b)},{k})"
                    fields = [f for (px, pn, f) in acc_rows if px == x and pn == n]
                    m = re.search(re.escape(cell) + r"\.v_(" + IDENT + ")", _squash(cons))
                    if x not in refs or not n or _squash(_strip_wrap(lo)) != f"offset_of({x})" or m is None:
                        why = ("the element-range line of a range atom: `forall k: Int :: { loc(block_of(p), k) } offset_of(p) <= k && k < offset_of(p) + n "
                               "==> <the type's bounds of loc(block_of(p), k).v_<t>>`")
                    elif f"v_{m.group(1)}" not in fields:
                        why = f"the element range of `{x}`'s `v_{m.group(1)}` comes after its `acc(..., wildcard)` line (form Min)"
                    elif not _std_type_range(_strip_wrap(cons), cell, m.group(1)):
                        why = f"the consequent is not the type's range of `v_{m.group(1)}`"
                    else:
                        elements.add((x, n, f"v_{m.group(1)}"))
            elif (cell_read := next((x for x in refs if re.search(rf"(?<![\w$'.]){re.escape(x)}\.v_", flat)), None)) is not None \
                    and not PF_FORBIDDEN_RE.search(text) and "acc(" not in flat:
                # the element range of a cell atom: the type's bounds of `p.v_<t>`, after the atom's `acc(p.v_<t>, wildcard)`
                t = re.search(rf"{re.escape(cell_read)}\.v_({IDENT})", flat).group(1)
                if not _std_type_range(text.strip(), cell_read, t):
                    why = f"the consequent is not the type's range of `{cell_read}.v_{t}`"
                elif (cell_read, "", f"v_{t}") not in acc_rows:
                    why = f"the element range of `{cell_read}.v_{t}` comes after its `acc({cell_read}.v_{t}, wildcard)` line (form Min)"
                else:
                    elements.add((cell_read, "", f"v_{t}"))
            elif HEAP_READ_NO_APPLICATION_RE.search(text) or PF_FORBIDDEN_RE.search(text) or "acc(" in flat:
                why = ("a `requires` of a `pure` function is a value clause (no heap read, permission, `old`), a type range or an atom's line "
                       "(`owns`, `live`, guards over atoms and markers are not read, M4.5f1)")
            else:
                # a plain line: it may be a pure line of an atom (collected for the `acc` lines after it) or a cell atom's element range
                for pn in refs:
                    if flat == f"{pn}!=null":
                        notnull.add(pn)
                    elif flat == norm_viper(_view_wf(pn)):
                        wf.add(pn)
                    elif (m := re.fullmatch(rf"offset_of\({re.escape(pn)}\)\+(.+)<=len\(block_of\({re.escape(pn)}\)\)", flat)):
                        bound.add((pn, m.group(1)))
                    elif flat in (f"offset_of({pn})<len(block_of({pn}))", f"offset_of({pn})+1<=len(block_of({pn}))"):
                        cell_bound.add(pn)
                    elif (m := re.fullmatch(rf"{re.escape(pn)}!=null&&{re.escape(pn)}==loc\(block_of\({re.escape(pn)}\),offset_of\({re.escape(pn)}\)\)&&"
                                            rf"0<=offset_of\({re.escape(pn)}\)&&offset_of\({re.escape(pn)}\)<=len\(block_of\({re.escape(pn)}\)\)&&"
                                            rf"0<=(.+)&&offset_of\({re.escape(pn)}\)\+\1<=len\(block_of\({re.escape(pn)}\)\)", flat)):
                        notnull.add(pn)
                        wf.add(pn)
                        nonneg.add(m.group(1))
                        bound.add((pn, m.group(1)))
                m = re.fullmatch(r"0<=(.+)", flat)
                if m:
                    nonneg.add(m.group(1))
            out[line] = (why, wild)
        for key, line in acc_rows.items():
            if key not in elements and key[2] != "v_bool" and out.get(line, (None, False))[0] is None:
                out[line] = (f"the atom of `{key[0]}` needs its element-range line (the type's bounds of its `{key[2]}`) after its `acc(..., wildcard)` line (form Min)", False)
    return out


def lint_pf_requires(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> dict[int, tuple[str | None, bool]]:
    """Report `pf_requires_problems`; returns them for the rules that exempt those lines (`wildcard`, the older quantifier rules)."""
    verdicts = pf_requires_problems(src, decls)
    for line, (why, _wild) in sorted(verdicts.items()):
        if why is not None:
            add(line, f"`requires` of a `pure` function (decision 0062 item 6, form Min): {why}")
    return verdicts


def lint_pure_functions(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0062 item 12: a `pf_<name>__<id>` function is bodied, with exactly one `decreases` line (a measure, or empty for a function that does
    not apply itself; `_` and `*` are refused by the termination rules for every function); its `ensures` lines hold no `acc`, `perm`, `old`,
    `inhale` or `assume`, nor its body; every application of the function itself in its body begins a line (the position map is keyed by line); a
    function that applies itself has a measure; the only cycle is such a self-application of the body (no mutual recursion, no cycle through a
    `requires`, an `ensures` or a `decreases`).  A name that starts `pf_` is a `pf_<name>__<id>`."""
    pfs = {d.name: d for d in decls if d.kind == "function" and PF_NAME_RE.match(d.name)}
    for d in decls:
        if d.kind == "function" and d.name.startswith("pf_") and d.name not in pfs:
            add(d.line, f"`function {d.name}`: a function whose name starts `pf_` is a `pf_<name>__<id>` (decision 0062 item 12)")
    measures = decreases_measures(src)
    body_edges: dict[str, set[str]] = {}
    contract_edges: dict[str, set[str]] = {}
    for name, d in pfs.items():
        what = f"`function {name}` (decision 0062 item 12)"
        clauses = clause_texts(src, d)
        decreases = [c for c in clauses if c[0] == "decreases"]
        if len(decreases) != 1:
            add(d.line, f"{what}: exactly one `decreases` line, a measure or empty (Silicon checks the termination of a function only where one is "
                        f"present), found {len(decreases)}")
        for kw, line, text in clauses:
            if kw == "ensures" and PF_FORBIDDEN_RE.search(text):
                add(line, f"{what}: an `ensures` of a `pure` function reads no permission: no `acc`, `perm`, `old`, `forperm`, `inhale` or `assume`")
        body_edges[name], contract_edges[name] = set(), set()
        for kw, line, text in clauses:
            contract_edges[name] |= {a for a, _s, _e in _pf_applications(text)}
        if not d.has_body:
            continue
        for n, code in _pf_body_text(src, d):
            if PF_FORBIDDEN_RE.search(code):
                add(n, f"{what}: the body of a `pure` function holds no `acc`, `perm`, `old`, `forperm`, `inhale` or `assume`")
            apps = _pf_applications(code)
            body_edges[name] |= {a for a, _s, _e in apps}
            own = [(s, e) for a, s, e in apps if a == name]
            if own and (len(own) > 1 or code[:own[0][0]].replace("(", "").strip()):
                add(n, f"{what}: an application of `{name}` in its own body begins a line of its own (the position map is keyed by line: "
                       "each such application has its own `termination.failed` entry)")
        if name in body_edges[name]:
            if decreases and measures.get(decreases[0][1], "") in ("", "_"):
                add(decreases[0][1], f"{what}: it applies itself, so its `decreases` has a measure (an empty one would not check its termination)")
    # the graph of applications: the only cycle is a function applying itself in its own body
    nodes = list(pfs)
    edges = {n: {a for a in body_edges.get(n, set()) | contract_edges.get(n, set()) if a in pfs} for n in nodes}
    index: dict[str, int] = {}
    low: dict[str, int] = {}
    stack: list[str] = []
    on: set[str] = set()
    components: list[list[str]] = []
    counter = [0]

    def connect(v: str) -> None:
        index[v] = low[v] = counter[0]
        counter[0] += 1
        stack.append(v)
        on.add(v)
        for w in sorted(edges[v]):
            if w not in index:
                connect(w)
                low[v] = min(low[v], low[w])
            elif w in on:
                low[v] = min(low[v], index[w])
        if low[v] == index[v]:
            comp = []
            while True:
                w = stack.pop()
                on.discard(w)
                comp.append(w)
                if w == v:
                    break
            components.append(comp)

    for n in nodes:
        if n not in index:
            connect(n)
    for comp in components:
        if len(comp) > 1:
            for n in sorted(comp):
                add(pfs[n].line, f"`function {n}` (decision 0062 item 12): it is part of a cycle of `pure` functions ({', '.join(sorted(comp))}): "
                                 "the only cycle is a function applying itself in its own body")
        elif comp[0] in contract_edges[comp[0]]:
            add(pfs[comp[0]].line, f"`function {comp[0]}` (decision 0062 item 12): it applies itself in a `requires`, an `ensures` or a `decreases`: "
                                   "the only cycle is a function applying itself in its own body")


def lint_pf_applications(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0062 item 12: an application of a `pf_` function stands in a `pf_` function (never in its `decreases`), in a method's `requires`,
    `ensures`, `invariant` or `assert` (on the line of its keyword) and never in a `__wf` method, a prelude or other function, or a `decreases`;
    its callee is a `pf_` function of the file."""
    funcs = {d.name for d in decls if d.kind == "function"}
    decreases_lines = decreases_line_set(src)
    owner_of: dict[int, Decl] = {}
    for d in decls:
        for n in range(d.line, d.end_line + 1):
            owner_of[n] = d
    for lineno, code in enumerate(src.code, start=1):
        for m in PF_ANY_RE.finditer(code):
            name = m.group("name")
            if code[:m.start()].rstrip().endswith(("function", "method")):
                continue  # the declaration of the function itself, or of the `__wf` method of one (`pf_<name>__<id>__wf`)
            owner = owner_of.get(lineno)
            where = f"an application of `{name}` (decision 0062 item 12)"
            if PF_NAME_RE.match(name) is None or name not in funcs:
                add(lineno, f"{where}: the callee is a `pf_<name>__<id>` function of this file")
            if owner is None or owner.kind not in ("method", "function"):
                add(lineno, f"{where}: stands in a `pf_` function or a method's clause")
            elif lineno in decreases_lines:
                add(lineno, f"{where}: stands in no `decreases` (a measure calls nothing)")
            elif owner.kind == "function":
                if PF_NAME_RE.match(owner.name) is None:
                    add(lineno, f"{where}: stands in a `pf_` function, not in `function {owner.name}`")
            elif owner.name.endswith("__wf"):
                add(lineno, f"{where}: stands in no `__wf` method (variant M's method applies the ghost functions `hva_<method>__<callee>_<pre|post>`)")
            elif not re.match(r"^\s*(?:requires|ensures|invariant|assert)\b", code):
                add(lineno, f"{where}: stands in a method's `requires`, `ensures`, `invariant` or `assert`, on the line of its keyword")


def _split_args(text: str) -> list[str]:
    return [a.strip() for a in _split_commas(text)]


def _hva_applications(text: str) -> list[tuple[str, list[str], str]]:
    """(name, arguments, text) of every application of an `hva_` ghost function in `text`."""
    out = []
    for m in HVA_APP_START_RE.finditer(text):
        end = _group_end(text, m.end() - 1)
        if end < 0:
            out.append((m.group().rstrip("( \t"), [], text[m.start():]))
            continue
        name = m.group().rstrip("( \t")
        out.append((name, _split_args(text[m.end():end]), text[m.start():end + 1]))
    return out


def _substitute(text: str, mapping: dict[str, str]) -> str:
    if not mapping:
        return text
    pattern = re.compile(r"(?<![\w$'.])(" + "|".join(map(re.escape, sorted(mapping, key=len, reverse=True))) + r")(?![\w$'])")
    return pattern.sub(lambda m: mapping[m.group(1)], text)


def _conjunct_set(text: str) -> list[str]:
    try:
        parts = _flatten_and(text)
    except ValueError:
        parts = [text.strip()]
    return sorted(norm_viper(p) for p in parts)


def _callee_quantifier(qtext: str, info: PfInfo, args_raw: list[str], ctx: WfContext, state: str) -> str | None:
    """The user quantifier `qtext` of a `pf_` callee's `requires` as fact (ii)'s guard V holds it: the callee's quantified clause at the arguments over the
    `hvq_<method>__<formal>_<state>` ghost of the method's pointer formal that the argument of the pointer parameter `p` is.  The callee's own text reads
    an element as `(site membership ? loc(block_of(p), q<j>).v_<t> : 0)` (its contract form, the membership being `p`'s atom's interval, which the
    quantifier's antecedent makes true) or as `loc(block_of(p), q<j>).v_<t>`; both become the ghost; the trigger too; the other parameters become the
    arguments; normalized.  None when an argument it reads through is no formal of the method with such a ghost, or a membership is not the callee's
    own atom's interval."""
    amap = {pname: _strip_wrap(a.strip()) for (pname, _t), a in zip(info.params, args_raw)}
    counts = {pname: cnt for pname, cnt, _field in info.atoms}
    failed = False

    def ghost(pointer: str) -> str:
        nonlocal failed
        arg = amap.get(pointer, "")
        name = f"hvq_{ctx.method}__{arg}_{state}"
        if re.fullmatch(IDENT, arg) is None or arg not in ctx.formals or name not in ctx.ghosts:
            failed = True
        return name

    def site(m: re.Match) -> str:
        nonlocal failed
        if counts.get(m.group(1)) != norm_viper(m.group(3)):
            failed = True
        return f"{ghost(m.group(1))}({m.group(2)})"

    text = norm_viper(qtext)
    text = re.sub(rf"block_of\(loc\(block_of\(({IDENT})\),(q[0-9]+)\)\)==block_of\(\1\)&&offset_of\(\1\)<=offset_of\(loc\(block_of\(\1\),\2\)\)"
                  rf"&&offset_of\(loc\(block_of\(\1\),\2\)\)<offset_of\(\1\)\+([^?]+?)\?loc\(block_of\(\1\),\2\)\.v_[\w$']+:0", site, text)
    text = re.sub(rf"loc\(block_of\(({IDENT})\),(q[0-9]+)\)\.v_[\w$']+", lambda m: f"{ghost(m.group(1))}({m.group(2)})", text)
    text = re.sub(rf"\{{loc\(block_of\(({IDENT})\),(q[0-9]+)\)\}}", lambda m: f"{{{ghost(m.group(1))}({m.group(2)})}}", text)
    if failed:
        return None
    return norm_viper(_substitute(text, {pname: f"({a})" for pname, a in amap.items()}))


def hva_fact_problem(rest: str, ctx: WfContext) -> str | None:
    """Why the `requires` of an `f__wf` method that holds an application ghost `hva_<method>__<callee>_<pre|post>` is none of decision 0062 item 8's
    facts, or None: (i) the type range of an application (an integer type's bounds, unguarded); (ii) the callee's `ensures` at the arguments, under
    exactly the callee's `requires` that read no memory at those arguments (no guard when it has none); (iii) the keep fact
    `G ==> post(args) == pre(args)` under the guards of a `reads_range` atom of the method and its `ensures` partner and the interval inclusion."""
    text = rest.strip()
    parts = _split_top(text, "==>")
    if len(parts) > 2:
        return "a fact has at most one guard `G ==> `"
    guard = parts[0].strip() if len(parts) == 2 else None
    body = _strip_wrap(text[len(parts[0]) + len("==>"):]) if len(parts) == 2 else _strip_wrap(text)
    quant_conj: list[str] = []
    if guard is not None:
        if not guard:
            return "the guard of a fact is a plain condition (no heap read, permission or quantifier)"
        try:
            gparts = _flatten_and(guard)
        except ValueError:
            gparts = [guard.strip()]
        quant_conj = [g for g in gparts if FORALL_RE.search(g)]
        plain = " && ".join(g for g in gparts if not FORALL_RE.search(g))
        if re.search(r"(?<![\w$'])hvq_", plain):
            return "a line applies both an `hvq_` and an `hva_` ghost function (only fact (ii)'s guard V holds `hvq_` ghosts, inside the callee's quantified clauses)"
        if plain and (not _heap_free(plain) or QUANT_FORBIDDEN_RE.search(plain)):
            return ("the guard of a fact is a plain condition (no heap read, permission or quantifier; fact (ii)'s guard V also holds the callee's "
                    "quantified clauses over the method's `hvq` ghosts, decision 0062 item 8)")
    if FORALL_RE.search(body):
        return "a quantified fact over application ghosts (decision 0062 item 7, cut 2) is not one of the facts (i) to (iii)"
    if re.search(r"(?<![\w$'])hvq_", body):
        return "a line applies both an `hvq_` and an `hva_` ghost function (only fact (ii)'s guard V holds `hvq_` ghosts, inside the callee's quantified clauses)"
    apps = _hva_applications(body)
    if not apps or not _heap_free(body) or QUANT_FORBIDDEN_RE.search(body):
        return "the body of a fact reads no heap and holds no permission or quantifier"
    keys = set()
    for name, args, full in apps:
        found = hva_callee(name, ctx.method, ctx.pfs)
        if found is None or name not in ctx.ghosts:
            return f"`{name}` is not an application ghost `hva_{ctx.method}__<callee>_<pre|post>` of this method's domain with a `pf_` callee of this file"
        info = ctx.pfs[found[0]]
        if len(args) != len(info.params):
            return f"`{name}` takes {len(info.params)} arguments, found {len(args)}"
        if not all(_heap_free(a) and not QUANT_FORBIDDEN_RE.search(a) for a in args):
            return "the arguments of an application in a fact read no heap"
        keys.add((found[0], tuple(norm_viper(a) for a in args)))
    if len(keys) != 1:
        return "a fact is about one application (one callee, one list of arguments)"
    callee, args_n = next(iter(keys))
    info = ctx.pfs[callee]
    args_raw = apps[0][1]
    flat = norm_viper(body)
    # (iii) the keep fact
    keep = re.fullmatch(rf"({IDENT})\((.*)\)==({IDENT})\((.*)\)", flat)
    if keep and len(apps) == 2 and keep.group(1) in ctx.ghosts and keep.group(3) in ctx.ghosts:
        (n1, _a1, _t1), (n2, _a2, _t2) = apps
        s1, s2 = hva_callee(n1, ctx.method, ctx.pfs)[1], hva_callee(n2, ctx.method, ctx.pfs)[1]
        if (s1, s2) != ("post", "pre") or flat != norm_viper(f"{n1}({', '.join(_a1)}) == {n2}({', '.join(_a2)})"):
            return "the keep fact is `post(args) == pre(args)` of one callee at the same arguments (the post-state application on the left)"
        if guard is None:
            return ("the keep fact is guarded by the guards of a `reads_range` atom of the method and its `ensures` partner and the interval inclusion "
                    "(an unguarded fact lets a wrong contract's `__wf` verify: it holds on every path, even where an `owns_range` alias may change the "
                    "place)")
        if quant_conj:
            return "the keep fact's guard holds no quantifier"
        have = set(_conjunct_set(guard))
        if not info.atoms and not info.cells:
            return (f"the keep fact is about a callee that names memory: `{callee}` has no range or cell atom in its `requires`, so its value does not depend "
                    "on a place the method's `reads_range` or `reads` atom keeps")
        if info.atoms and info.cells:
            return f"`{callee}` holds a range atom and a cell atom: the keep fact is not written for it (a false alarm, fail closed)"
        amap = {pname: a.strip() for (pname, _t), a in zip(info.params, args_raw)}
        cmap = {pname: f"({a})" for pname, a in amap.items()}
        kind = "range" if info.atoms else "cell"
        mine = {}
        for c in have:
            pre, post = re.search(r"(?<![\w$'])hv_[\w$']*_pre(?![\w$'])", c), re.search(r"(?<![\w$'])hv_[\w$']*_post(?![\w$'])", c)
            if pre and post:
                mine[c] = "?" + c
            elif pre or post:
                mine[c] = ("pre:" if pre else "post:") + re.sub(r"(?<![\w$'])hv_[\w$']*_(?:pre|post)(?![\w$'])", "@h", c)
        for r in ctx.atoms:
            if r.kind != kind or r.clause != "requires" or r.amount is None:
                continue
            for e in ctx.atoms:
                if (e.kind, e.clause, e.formal, e.field, e.amount, e.count) == (kind, "ensures", r.formal, r.field, r.amount, r.count):
                    need = _keep_atom_guard(r, "pre") | _keep_atom_guard(e, "post")
                    ok = True
                    callee_atoms = [(pname, cnt, field) for pname, cnt, field in info.atoms] if kind == "range" else [(pname, "1", field) for pname, field in info.cells]
                    for pname, cnt, field in callee_atoms:
                        arg = _strip_wrap(amap.get(pname, ""))
                        if field != r.field or re.fullmatch(IDENT, arg) is None:
                            ok = False
                            break
                        need.add(norm_viper(f"(block_of({arg}) == block_of({r.formal}))"))
                        need.add(norm_viper(f"(offset_of({r.formal}) <= offset_of({arg}))"))
                        count = _substitute(cnt, cmap) if kind == "range" else "1"
                        need.add(norm_viper(f"((offset_of({arg}) + {count}) <= (offset_of({r.formal}) + {r.count if kind == 'range' else '1'}))"))
                    got = {mine.get(c, c) for c in have}
                    if ok and got == need:
                        return None
        return ("the keep fact is guarded by exactly the guards of a `reads_range(p, n)` or `reads(*p)` `requires` atom of the method (a condition on memory "
                "over the entry state's read ghosts `hv_<k>_pre`) and of its `ensures` partner (over `hv_<k>_post`; same field, amount and count), the same "
                "block, and for every atom of the callee its interval at the arguments inside the method's atom's interval (`offset_of(p) <= offset_of(arg)` "
                "and `offset_of(arg) + count <= offset_of(p) + n`; one slot for a cell), read from the callee's own atoms (decision 0062 item 12)")
    # (i) the type range of an application
    app_text = norm_viper(apps[0][2])
    rng = [norm_viper(x) for x in (_flatten_and(body) if "&&" in body else [body])]
    if len(rng) == 2 and all(app_text in r for r in rng) and len(apps) == 2:
        lo = re.fullmatch(rf"(-?[0-9]+)<={re.escape(app_text)}", rng[0])
        hi = re.fullmatch(rf"{re.escape(app_text)}<=(-?[0-9]+)", rng[1])
        if lo and hi and guard is None:
            # an unguarded range is the type's (item 8 (i)); any other unguarded bound would be an `ensures` fact (ii) without its guard
            if info.ret != "Int":
                return f"`{callee}` returns `{info.ret}`: a `Bool` result has no type range to assume"
            got_range = (int(lo.group(1)), int(hi.group(1)))
            if info.result_tag is None:
                if got_range in VIEW_STANDARD_RANGES:
                    return (f"fact (i) is about `{callee}`, whose signature line carries no `// cerastes:result <lo> <hi>` tag (an integer, non-enumeration "
                            "result always has one): its range is checked against the tag")
            else:
                if got_range in VIEW_STANDARD_RANGES and got_range != info.result_tag:
                    return (f"fact (i)'s range {list(got_range)} is not the result type's of `{callee}`, whose signature line tags "
                            f"`// cerastes:result {info.result_tag[0]} {info.result_tag[1]}`")
                if got_range == info.result_tag and got_range in VIEW_STANDARD_RANGES:
                    return None
            # any other bound is read as fact (ii): the callee's `ensures`, guarded by the callee's `requires` unless it has none
    # (ii) the callee's ensures at the arguments, under the callee's value requires and pure lines
    if info.opaque_requires:
        return (f"`{callee}`'s `requires` applies a `pure` function: its value clauses are not comparable, so fact (ii) is not assumed for it "
                "(a false alarm, never a proof)")
    mapping = {pname: f"({a})" for (pname, _t), a in zip(info.params, args_raw)}
    full_app = apps[0][2]
    mapping["result"] = full_app
    if any(a[2] != full_app and norm_viper(a[2]) != norm_viper(full_app) for a in apps):
        return "a fact (ii) is about one application"
    want = {norm_viper(_substitute(c, mapping)) for c in info.ensures}
    got = _conjunct_set(body)
    if not got or not all(g in want for g in got):
        return (f"the body of a fact (ii) is conjuncts of `{callee}`'s `ensures` that read no memory, at the arguments, `result` being the application "
                "(a clause that reads memory, holds a quantifier or an application is left out)")
    need_v = {norm_viper(_substitute(c, {k: v for k, v in mapping.items() if k != "result"})) for c in info.requires}
    state = hva_callee(apps[0][0], ctx.method, ctx.pfs)[1]
    for qtext in info.quantified:
        expected = _callee_quantifier(qtext, info, args_raw, ctx, state)
        if expected is None:
            return (f"`{callee}`'s `requires` holds a quantified clause that fact (ii)'s guard V must hold over the `hvq_{ctx.method}__<formal>_{state}` "
                    "ghosts of the method's own pointer formals at the arguments: an argument is no formal of this method, or it has no such ghost")
        need_v.add(expected)
    if guard is None:
        if need_v:
            return (f"fact (ii) is guarded by `{callee}`'s `requires` that read no memory, at the arguments: the `ensures` of a function may be false "
                    "where its `requires` does not hold, and an unguarded fact would make every row of the method vacuous there")
        return None
    if set(_conjunct_set(guard)) != need_v:
        return (f"the guard of fact (ii) is exactly `{callee}`'s `requires` that read no memory (its parameters' ranges, its pure lines and value clauses) "
                "and its quantified clauses over the method's `hvq` ghosts, at the arguments (decision 0062 item 8: a V without the quantified clause "
                "proves what the callee never promised)")
    return None


def hva_line_problem(kind: str, rest: str, code: str, ctx: WfContext, ghosts: frozenset[str]) -> str | None:
    """Why a line of an `f__wf` method that applies an `hva_` ghost is not one of decision 0062 item 8's: its `requires` are the facts (i) to (iii)
    (never mixed with an `hvq_` ghost), a plain `assert` row reads no heap, a quantified one is item 11's."""
    if kind == "requires":
        return hva_fact_problem(rest, ctx)
    if kind != "assert" and any(g.startswith("hvq_") and re.search(rf"(?<![\w$']){re.escape(g)}\s*\(", code) for g in ghosts):
        return "a line applies both an `hvq_` and an `hva_` ghost function"
    if kind == "assert":
        if WF_QUANT_RE.search(code):
            return wf_assert_verdict(rest, ghosts)
        if not _heap_free(rest) or QUANT_FORBIDDEN_RE.search(rest):
            return "a row of an `f__wf` method reads no heap and holds no permission"
        return None
    return f"a `{kind}` of an `f__wf` method holds no application ghost (the facts are `requires`, its rows `assert`)"


def lint_pf_rows(src: Source, decls: list[Decl], by_line: dict[int, list[MapEntry]], add: Callable[[int, str], None], map_name: str) -> None:
    """Decision 0062 item 12, the map rows of a `pf_` function: each `requires` line `not.wellformed` (`internal`); each `ensures` line
    `postcondition.violated` (`postcondition`) and `not.wellformed` (`internal`); its `decreases` line with a measure `termination.failed`
    (`termination`); each body line that begins with an application of the function itself `termination.failed` (`termination`) and no other line
    of the body any entry."""
    measures = decreases_measures(src)

    def problem(e: MapEntry, why: str) -> None:
        add(e.vpr_line, f"the map entry `{map_name}:{e.map_line}`: {why}")

    for d in decls:
        if d.kind != "function" or PF_NAME_RE.match(d.name) is None or not d.has_body:
            continue
        for kw, line, _text in clause_texts(src, d):
            ids = {e.error_id: e for e in by_line.get(line, [])}
            if kw == "requires":
                for e in by_line.get(line, []):
                    if e.category != "internal":
                        problem(e, f"a `requires` of a `pure` function is checked for well-formedness only: its entry is `internal`, found `{e.category}`")
            elif kw == "ensures":
                need = {"postcondition.violated": "postcondition", "not.wellformed": "internal"}
                for error_id, category in need.items():
                    if error_id not in ids:
                        add(line, f"an `ensures` of `function {d.name}` needs a position-map entry with the Viper error id `{error_id}` "
                                  f"(category `{category}`, decision 0062 item 12)")
                    elif ids[error_id].category != category:
                        problem(ids[error_id], f"its category is `{category}`, found `{ids[error_id].category}` (decision 0062 item 12)")
            elif kw == "decreases" and measures.get(line, "") not in ("", "_"):
                for e in by_line.get(line, []):
                    if e.category != "termination":
                        problem(e, f"the entry of a `decreases` measure is `termination`, found `{e.category}`")
        for n, code in _pf_body_text(src, d):
            own = [m for m in PF_APP_RE.finditer(code) if m.group("name") == d.name]
            starts = bool(own) and not code[:own[0].start()].replace("(", "").strip()
            for e in by_line.get(n, []):
                if not (starts and e.error_id == "termination.failed" and e.category == "termination"):
                    problem(e, "a body line of a `pure` function has an entry only when it begins an application of the function itself, "
                               "and then `termination.failed` (`termination`)")
            if starts and not any(e.error_id == "termination.failed" for e in by_line.get(n, [])):
                add(n, f"a body line of `function {d.name}` that begins an application of itself needs the `termination.failed` entry of that call")


def lint_vpr(text: str, map_text: str | None, name: str = "<vpr>",
             map_name: str | None = None) -> list[str]:
    """Problems of one Viper file with its position map (None: the map is missing)."""
    src = split_source(text)
    problems: list[str] = []
    decls = declarations(src)

    def add(lineno: int, why: str) -> None:
        problems.append(f"{name}:{lineno}: {why}")

    for lineno, (code, comment) in enumerate(zip(src.code, src.comments), start=1):
        if m := INHALE_WORD_RE.search(code):
            tag = INHALE_TAG_RE.search(comment) if comment else None
            if tag is None:
                add(lineno, f"`{m.group(1)}` without `// cerastes:inhale-ok <reason>` "
                            "(invariant 4)")
            elif not tag.group("reason"):
                add(lineno, "`// cerastes:inhale-ok` needs a reason")
    ghost_domain_decls = ghost_domains(src, decls)
    for d in decls:
        if d.kind == "import":
            add(d.line, "`import`: an emitted file is self-contained (the lint reads only this "
                        "file, so an imported one would escape it)")
        elif d.kind == "domain":
            if d.name in ghost_domain_decls:
                lint_ghost_domain(src, d, add)
            elif not lint_domain(src, d, add):
                add(d.line, f"`domain {d.name}`: the prelude has no domains but the memory model's `domain Block` "
                            "with `// cerastes:domain-ok <record>` right above it (decision 0012 item 11, "
                            "decision 0028 item 2) and the ghost domains of an `f__wf` method with "
                            "`// cerastes:domain-ok 0061` (decision 0061 item 11)")
        elif d.kind == "function" and not d.has_body:
            add(d.line, f"bodyless `function {d.name}`: functions need bodies "
                        "(decision 0012 item 11)")
        elif d.kind == "method" and not d.has_body:
            tag = trusted_tag(src, d.line)
            if tag is None:
                add(d.line, f"bodyless `method {d.name}` without `// cerastes:trusted-contract "
                            "<reason>` (decision 0012 item 1)")
            elif not tag.group("reason"):
                add(d.line, "`// cerastes:trusted-contract` needs a reason")
    ok_axioms = tagged_axiom_lines(src, decls, add)
    for lineno, code in enumerate(src.code, start=1):
        if re.search(r"\baxiom\b", code) and lineno not in ok_axioms:
            add(lineno, "`axiom`: the prelude has no axioms but the ones of the memory model's Block domain, "
                        "each with `// cerastes:axiom-ok <record>` right above it (decision 0012 item 11, "
                        "decision 0028 item 2)")
        if NEW_RE.search(code):
            add(lineno, "`new(...)`: its result is not `WF` (decision 0028 item 2); objects come from the "
                        "trusted `alloc_T` methods")

    methods = {d.name for d in decls if d.kind == "method"}
    call_re = CallStatement()
    check_lines: dict[int, str] = {}
    declaration_lines = {d.line for d in decls}
    measures = decreases_measures(src)
    exempt_ensures = bit_range_ensures_lines(src, decls)
    call_word_re = re.compile(r"(?<![\w$'])(" + "|".join(map(re.escape, sorted(methods))) + r")\s*\(") \
        if methods else None
    for lineno, code in enumerate(src.code, start=1):
        # every check is on its own line (decision 0012 item 13), so that its map entry
        # is the line: a keyword that does not start its line, or a second one, hides one
        words = list(CHECK_WORD_RE.finditer(code))
        for k, w in enumerate(words):
            if k > 0 or code[:w.start()].strip():
                add(lineno, f"`{w.group(1)}` does not start its line: every check is on its "
                            "own line (decision 0012 item 13)")
        if m := CHECK_LINE_RE.match(code):
            if lineno in exempt_ensures:
                pass  # the range postcondition of a bitwise prelude function: no map entry
            elif m.group(1) == "decreases" and measures[lineno] in ("", "_"):
                pass  # an empty measure (a method outside any recursion) or `_` (trusted) is no check
            else:
                check_lines[lineno] = m.group(1)
        elif lineno not in declaration_lines and call_word_re is not None \
                and (c := call_word_re.search(code)):
            if (m := call_re.match(code)) and m.group("name") in methods:
                check_lines[lineno] = f"call of {m.group('name')}"
            else:
                add(lineno, f"the call of method `{c.group(1)}` is not alone on its line: every "
                            "call is a statement of its own (decision 0012 item 13)")

    lint_termination_clauses(src, decls, add, measures)
    lint_pure_functions(src, decls, add)
    lint_pf_applications(src, decls, add)
    lint_inert_functions(src, decls, add)
    lint_prelude_decreases(src, decls, add)
    lint_prelude_closure(src, decls, add)
    lint_pf_wf_methods(src, decls, add)
    lint_pf_result_tags(src, decls, add)
    pf_requires = lint_pf_requires(src, decls, add)
    lint_jumps(src, decls, add)
    lint_flags(src, add)
    lint_remainders(src, decls, add)
    lint_allocations(src, decls, add)
    lint_array_allocations(src, decls, add)
    lint_array_copy_calls(src, decls, add)
    lint_scope_exits(src, decls, add)
    lint_range_length_facts(src, decls, add)
    lint_block_length_facts(src, decls, add)
    has_globals, global_facts = lint_globals(src, decls, add)
    quantified = classify_quantified_lines(src, decls)
    lint_ghost_amounts(src, decls, add, quantified.wildcard_rows | frozenset(n for n, (why, wild) in pf_requires.items() if wild and why is None))
    lint_ghost_domain_use(src, decls, ghost_domain_decls, add)
    lint_perm_locals(src, decls, add)
    lint_owner_loop_ghosts(src, decls, add)
    lint_loop_entry_ghosts(src, decls, add)
    lint_abp_ghosts(src, decls, add)
    lint_carrier_clauses(src, decls, add)
    lint_owning_global_tags(src, decls, add)
    lint_ghost_refs(src, decls, add)
    loop_rows = frozenset(e.vpr_line for e in (parse_map(map_text, "")[0] if map_text is not None else [])
                          if e.message.startswith(LOOP_LEAK_ROW_MESSAGE))
    forperm_lines = lint_heap_token(src, decls, add, loop_rows)
    view_lines: set[int] = set()
    lint_views(src, decls, add, has_globals, global_facts, view_lines)
    lint_marked_kept_rows(src, decls, add)

    if map_text is None:
        lint_quantified_rows(src, decls, None, add, user=quantified, view_lines=view_lines)
        problems.append(f"{name}: no position map ({map_name or name + MAP_SUFFIX})")
        return problems
    entries, map_problems = parse_map(map_text, map_name or name + MAP_SUFFIX)
    problems += map_problems
    lint_quantified_rows(src, decls, entries, add, map_name or name + MAP_SUFFIX, user=quantified, view_lines=view_lines)
    lint_not_invalidating_rows(src, decls, entries, add)
    by_line: dict[int, list[MapEntry]] = {}
    for e in entries:
        by_line.setdefault(e.vpr_line, []).append(e)
    for lineno, kind in sorted(check_lines.items()):
        if lineno not in by_line:
            add(lineno, f"`{kind}` line has no entry in the position map")
            continue
        allowed = KIND_IDS["call" if kind.startswith("call of ") else kind]
        for e in by_line[lineno]:
            if e.error_id not in allowed:
                problems.append(f"{map_name or name + MAP_SUFFIX}:{e.map_line}: the entry for "
                                f"{name} line {lineno} has the Viper error id `{e.error_id}`, "
                                f"which a `{kind}` line cannot produce (expected "
                                + " or ".join(f"`{i}`" for i in sorted(allowed)) + ")")
    lint_pf_rows(src, decls, by_line, add, map_name or name + MAP_SUFFIX)
    for lineno in sorted(forperm_lines):
        for e in by_line.get(lineno, []):
            if e.category != "leak":
                problems.append(f"{map_name or name + MAP_SUFFIX}:{e.map_line}: the entry for {name} line {lineno}, a `forperm` "
                                f"assertion, has the category `{e.category}`: `forperm` is only a `leak` row (decision 0047 item 7)")
    nlines = len(src.code) - (1 if src.code and src.code[-1] == "" and text.endswith("\n") else 0)
    for lineno, group in sorted(by_line.items()):
        if lineno > nlines or not src.code[lineno - 1].strip():
            for e in group:
                problems.append(f"{map_name or name + MAP_SUFFIX}:{e.map_line}: entry for "
                                f"{name} line {lineno}, which is "
                                + ("past the end of the file" if lineno > nlines
                                   else "blank or a comment"))
        ids = [e.error_id for e in group]
        for error_id in sorted({i for i in ids if ids.count(i) > 1}):
            problems.append(f"{map_name or name + MAP_SUFFIX}: line {lineno} has two entries "
                            f"with the Viper error id `{error_id}`")
    return problems


def _top_level_items(measure: str) -> list[str]:
    """The comma-separated items of a `decreases` measure (commas inside brackets do not split)."""
    items, depth, cur = [], 0, ""
    for ch in measure:
        if ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        if ch == "," and depth == 0:
            items.append(cur.strip())
            cur = ""
        else:
            cur += ch
    items.append(cur.strip())
    return items


def loop_body_line(src: Source, lineno: int, column: int) -> int:
    """The line of the `{` that opens the body of the `while` at (`lineno`, `column`): a `{` that follows `::` or the `}` of a
    trigger set belongs to a quantifier of an invariant (`forall k: Int :: { loc(b, k) } ...`, decision 0059 item 8), not to the body."""
    tail = ""
    for ln in range(lineno, len(src.code) + 1):
        code = src.code[ln - 1]
        for ch in (code[column:] if ln == lineno else code):
            if ch.isspace():
                continue
            if ch == "{" and not (tail.endswith("::") or tail.endswith("}")):
                return ln
            tail = (tail + ch)[-2:]
    return len(src.code)


def lint_termination_clauses(src: Source, decls: list[Decl], add: Callable[[int, str], None],
                             measures: dict[int, str] | None = None) -> None:
    """Decision 0013 item 5: Silicon checks termination only where a `decreases` clause is present, so every
    method (the `__wf` methods too, with an empty measure) and every `while` has one; and `decreases _`, which
    assumes termination (invariant 4), is allowed only on a bodyless method with the trusted-contract tag.
    A clause is read whole when it runs over several lines (`decreases_measures`)."""
    if measures is None:
        measures = decreases_measures(src)
    for d in decls:
        if d.kind != "method":
            continue
        header_end = d.body_line - 1 if d.has_body else d.end_line
        lines = range(d.line, max(d.line, header_end) + 1)
        clauses = [ln for ln in lines if DECREASES_LINE_RE.match(src.code[ln - 1])]
        if not clauses:
            add(d.line, f"`method {d.name}` has no `decreases` clause: Silicon checks termination only "
                        "where one is present (an empty `decreases` for a method outside any recursion, "
                        "decision 0013 item 5)")
    for lineno, measure in sorted(measures.items()):
        if measure == "_":
            owner = next((d for d in decls if d.line <= lineno <= d.end_line), None)
            ok = (owner is not None and owner.kind == "method" and not owner.has_body
                  and trusted_tag(src, owner.line) is not None)
            if not ok:
                add(lineno, "`decreases _` assumes termination (invariant 4): it is allowed only on a "
                            "bodyless method that carries `// cerastes:trusted-contract <reason>`")
        if "*" in _top_level_items(measure):
            add(lineno, "`decreases *` assumes termination (invariant 4, decision 0013): Silicon "
                        "does not check the termination of what carries it; the emitter produces "
                        "a measure, an empty `decreases`, or `decreases _` on a trusted bodyless method")
        if re.search(r"(?<![\w$'])if(?![\w$'])", measure):
            add(lineno, "conditional `decreases <measure> if <condition>` is not allowed (decision 0013): "
                        "termination is checked only where the condition holds, so the measure must be "
                        "unconditional")
    for lineno, code in enumerate(src.code, start=1):
        for _w in WHILE_WORD_RE.finditer(code):
            body = loop_body_line(src, lineno, _w.end())
            if not any(DECREASES_LINE_RE.match(src.code[ln - 1]) for ln in range(lineno, body)):
                add(lineno, "`while` loop without a `decreases` clause: Silicon checks termination only "
                            "where one is present (decision 0013 item 5)")


def lint_jumps(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0014 item 9: control flow is structured (a `while` for every cycle).  Silicon
    checks the termination of `while` loops only; a cycle made of a backward `goto`, or a loop
    that a `label ... invariant` introduces, is unchecked, so none is allowed.  Labels are
    per method: the same name in two methods is fine, in one method it is a problem."""
    tokens: list[tuple[str, int]] = []
    for lineno, line in enumerate(src.code, start=1):
        tokens += [(m.group(), lineno) for m in TOKEN_RE.finditer(line)]
    for d in decls:
        if d.kind != "method":
            continue
        labels: dict[str, int] = {}  # name -> line of its first definition
        body = [(t, ln) for t, ln in tokens if d.line <= ln <= d.end_line]
        label_blocks, goto_blocks = block_nesting(body)
        for target, lineno, k_goto, blocks in goto_blocks:
            if target not in label_blocks:
                continue  # an undefined label is Silicon's business
            k_label, inside = label_blocks[target]
            if k_label > k_goto and inside != blocks[:len(inside)]:
                add(lineno, f"`goto {target}` into a block: the label is inside a `{{ ... }}` (a `while` "
                            "body or an `if` or `else` branch) that does not enclose the `goto`; a jump "
                            "may only go outward or stay in its block (it would skip the loop's "
                            "invariant check or the branch's condition; decision 0015)")
        for k, (text, lineno) in enumerate(body):
            nxt = body[k + 1] if k + 1 < len(body) else ("", 0)
            if text == "label" and nxt[0]:
                name = nxt[0]
                if name in labels:
                    add(lineno, f"duplicate `label {name}` in `method {d.name}` (first on line "
                                f"{labels[name]}): a label is defined once per method")
                else:
                    labels[name] = lineno
                after = body[k + 2][0] if k + 2 < len(body) else ""
                if after == "invariant":
                    add(lineno, f"`label {name}` carries an `invariant`: a label with invariants "
                                "is a loop that Silicon does not check for termination "
                                "(decision 0014); loops are `while`")
            elif text == "goto" and nxt[0] in labels:
                add(lineno, f"backward `goto {nxt[0]}` (the label is on line {labels[nxt[0]]}): a "
                            "cycle that Silicon does not check for termination (decision 0014); "
                            "loops are `while`")


FLAG_ASSIGN_RE = re.compile(r"^\s*d_(?P<name>[A-Za-z_$][\w$']*)\s*:=(?P<value>.*)$")
FLAG_ANYWHERE_RE = re.compile(r"(?<![\w$'])d_[\w$']*\s*:=")
FLAG_DECL_INIT_RE = re.compile(r"(?<![\w$'])var\s+d_[\w$']*\s*:\s*Bool\s*:=")
DUMMY_VALUE_RE = re.compile(r"^\s*(?:-?\s*\d+|\(\s*-\s*\d+\s*\)|true|false|null)\s*;?\s*$")
# The flag of an address-taken cell is a field of the cell (decision 0032 item 11): `<recv>.d_<t> := <value>`.
# A global's cell is `loc(GB_<name>(), 0)` (decision 0041 item 4): the same rule, the receiver is that term.
GLOBAL_CELL_RE = r"loc\(\s*GB_[\w$']+\(\)\s*,\s*0\s*\)"
ELEMENT_FLAG_RE = re.compile(r"^\s*loc\(.*\)\s*\.\s*d_[A-Za-z_$][\w$']*\s*:=")
FIELD_FLAG_RE = re.compile(r"^\s*(?P<recv>[A-Za-z_$][\w$']*|" + GLOBAL_CELL_RE + r")\.d_(?P<name>[A-Za-z_$][\w$']*)\s*:=(?P<value>.*)$")


# The owner ghosts of an owning local (decision 0052 item 3) are assigned between its write and its flag: `held_<v> := ...` and
# `tv_<v> := ...` are writes of other variables, so a flag set after them still directly follows the write of its variable.
OWNER_GHOST_ASSIGN_RE = re.compile(r"^\s*(?:held|tv)_[\w$']*\s*:=")


def lint_flags(src: Source, add: Callable[[int, str], None]) -> None:
    """Decision 0019 item 5: the ghost flag `d_<name>` of a local without an initializer is set only by a
    write of that local, and a reset gives the local its dummy value.  A flag that could be set without
    the write would make an `uninitialized` obligation hold for a variable that has no value: the one
    place where a lowering bug turns into a wrong `verified`.  So `d_<name> := true` must follow
    `<name> := ...` directly, `d_<name> := false` must precede `<name> := <literal>` directly, and a flag
    is assigned nothing else.  Neighbours are the previous and next lines that are neither blank nor a
    comment (the emitter writes one statement per line)."""
    lines = [(ln, code) for ln, code in enumerate(src.code, start=1) if code.strip()]
    for k, (lineno, code) in enumerate(lines):
        if FIELD_FLAG_RE.match(code) is None and ELEMENT_FLAG_RE.match(code):
            add(lineno, "the flag of an array element is written through a `Ref` temporary (`t := loc(block_of(a), i)`, then "
                        "`t.v_<t> := ...` and `t.d_<t> := true`), not through `loc(...)` itself: the flag rule reads a "
                        "`<recv>.d_<t> := true` that directly follows `<recv>.v_<t> := ...` with an identifier receiver "
                        "(decision 0059 item 12)")
            continue
        if FLAG_DECL_INIT_RE.search(code):
            add(lineno, "a flag is declared without an initializer (`var d_<name>: Bool`): it holds a value only "
                        "after a reset (decision 0019 item 5)")
        fm = FIELD_FLAG_RE.match(code)
        if fm is not None and len(FLAG_ANYWHERE_RE.findall(code)) == 1:
            recv, name = fm.group("recv"), fm.group("name")
            value = fm.group("value").strip().rstrip(";").strip()
            if ";" in value:
                add(lineno, "an assignment to a flag does not stand alone on its line: the flag rule reads "
                            "one statement per line (decision 0019 item 5)")
            elif value != "true":
                add(lineno, f"`{recv}.d_{name}` is assigned `{value}`: the flag of a cell is assigned only `true`, "
                            "by the write of its value (the allocation gives `false`; decision 0032 item 11, "
                            "decision 0019 item 5)")
            else:
                prev = lines[k - 1][1] if k > 0 else ""
                if not re.match(r"^\s*" + re.escape(recv) + r"\.v_" + re.escape(name) + r"\s*:=", prev):
                    add(lineno, f"`{recv}.d_{name} := true` does not directly follow a write `{recv}.v_{name} := ...`: "
                                "a flag is set only by the write of its cell (decision 0032 item 11, "
                                "decision 0019 item 5)")
            continue
        m = FLAG_ASSIGN_RE.match(code)
        if len(FLAG_ANYWHERE_RE.findall(code)) > (1 if m else 0) or (m is None and FLAG_ANYWHERE_RE.search(code)):
            add(lineno, "an assignment to a flag does not stand alone on its line: the flag rule reads "
                        "one statement per line (decision 0019 item 5)")
            continue
        if m is None:
            continue
        name, value = m.group("name"), m.group("value").strip().rstrip(";").strip()
        if value == "true":
            j = k - 1
            while j > 0 and OWNER_GHOST_ASSIGN_RE.match(lines[j][1]):  # the owner ghosts of an owning local (decision 0052 item 3)
                j -= 1
            prev = lines[j][1] if j >= 0 else ""
            if not re.match(r"^\s*" + re.escape(name) + r"\s*:=", prev):
                add(lineno, f"`d_{name} := true` does not directly follow a write `{name} := ...`: a flag is "
                            "set only by the write of its variable (decision 0019 item 5)")
        elif value == "false":
            nxt = lines[k + 1][1] if k + 1 < len(lines) else ""
            r = re.match(r"^\s*" + re.escape(name) + r"\s*:=(?P<rest>.*)$", nxt)
            if r is None or DUMMY_VALUE_RE.match(r.group("rest")) is None:
                add(lineno, f"`d_{name} := false` is not directly followed by `{name} := <literal>`: a reset "
                            "gives the variable its dummy value (decision 0019 item 5)")
        else:
            add(lineno, f"`d_{name}` is assigned `{value}`: a flag is assigned only `true`, by the write of "
                        "its variable, or `false`, by a reset (decision 0019 item 5)")


def block_nesting(body: list[tuple[str, int]]):
    """({label: (token index, the braces that enclose its first definition)}, [(label, line, token
    index, the braces that enclose the `goto`)]) for the tokens of one method.  A brace is the
    number of its `{` (outermost first); every `{ ... }` counts, the method body, a `while` body,
    an `if` or `else` branch, and also the balanced trigger sets of quantifiers, which are closed
    before any `label` or `goto` that follows them."""
    labels: dict[str, tuple[int, tuple[int, ...]]] = {}
    gotos: list[tuple[str, int, int, tuple[int, ...]]] = []
    stack: list[int] = []
    count = 0
    for k, (text, lineno) in enumerate(body):
        nxt = body[k + 1][0] if k + 1 < len(body) else ""
        if text == "{":
            count += 1
            stack.append(count)
        elif text == "}":
            if stack:
                stack.pop()
        elif text == "label" and nxt:
            labels.setdefault(nxt, (k, tuple(stack)))
        elif text == "goto" and nxt:
            gotos.append((nxt, lineno, k, tuple(stack)))
    return labels, gotos


def trusted_tag(src: Source, line: int):
    """The `cerastes:trusted-contract` tag for the method at `line` (1-based): on
    that line, or in the run of comment-only lines right above it."""
    m = TRUSTED_TAG_RE.search(src.comments[line - 1]) if src.comments[line - 1] else None
    i = line - 2
    while m is None and i >= 0 and not src.code[i].strip() and src.comments[i]:
        m = TRUSTED_TAG_RE.search(src.comments[i])
        i -= 1
    return m


# ---------------------------------------------------------------------------
# Directories, the manifest, the command line
# ---------------------------------------------------------------------------

class LintUsageError(Exception):
    pass


def verify_tests_expect_viper(root: Path | None = None) -> bool:
    """Whether a test of the tree (tests/ and the documentation examples) runs
    `cerastes verify` and must leave Viper behind (`verified` or `error`, in C
    or C++): then an absent or empty build/vpr is a failure."""
    base = ROOT if root is None else root
    paths = [str(base / p) for p in rt.DEFAULT_PATHS if (base / p).is_dir()]
    if not paths:
        return False
    for f in rt.discover(paths).tests:
        exp = rt.read_header(f)
        if isinstance(exp, rt.Expectation) and rt.is_verify_test(f, exp) and rt.vpr_expected(exp):
            return True
    return False


RUN_ID_RE = re.compile(r"^# run-id: (?P<id>\S+)$")


def read_manifest(directory: Path) -> tuple[list[str] | None, str | None, list[str]]:
    """(the manifest's entries or None if there is no MANIFEST, its run id,
    problems).  The first line is `# run-id: <id>`; other `#` lines are comments."""
    path = directory / MANIFEST
    if not path.is_file():
        return None, None, []
    lines = [l for l in path.read_text().split("\n") if l.strip()]
    problems = []
    run_id = None
    if lines and (m := RUN_ID_RE.match(lines[0])):
        run_id = m.group("id")
        lines = lines[1:]
    else:
        problems.append(f"{path}: the first line is not `# run-id: <id>`: the files cannot be "
                        "tied to a run")
    entries = [l for l in lines if not l.startswith("#")]
    seen: set[str] = set()
    for e in entries:
        if e.startswith("/") or ".." in Path(e).parts or not e.endswith(".vpr"):
            problems.append(f"{path}: `{e}` is not a path of a .vpr file below the directory")
        if e in seen:
            problems.append(f"{path}: `{e}` is listed twice")
        seen.add(e)
    return entries, run_id, problems


def lint_directory(directory: Path, expect_viper: Callable[[], bool] | None = None
                   ) -> tuple[list[str], int, str | None, str | None]:
    """(problems, number of .vpr files linted, a note, the run id) for a directory;
    `expect_viper` (default: verify_tests_expect_viper) says whether the tree has
    a verify test that must leave Viper behind."""
    if expect_viper is None:
        expect_viper = verify_tests_expect_viper
    problems: list[str] = []
    files = sorted(p for p in directory.rglob("*.vpr") if p.is_file()) if directory.is_dir() else []
    manifest, run_id, manifest_problems = read_manifest(directory)
    problems += manifest_problems
    if manifest is None and not files:
        if expect_viper():
            return [f"{directory}: no {MANIFEST} and no .vpr file, but a verify test of the "
                    "tree expects Viper (run scripts/run_tests.py first)"], 0, None, None
        return [], 0, "no emitted Viper to lint", None
    if manifest is not None:
        if not manifest:
            problems.append(f"{directory / MANIFEST}: the manifest is empty: an empty set "
                            "must not pass")
        for e in manifest:
            if not (directory / e).is_file():
                problems.append(f"{directory / MANIFEST}: `{e}` is listed but not there "
                                f"(fewer files than the {len(manifest)} the run expects)")
    elif files:
        problems.append(f"{directory}: .vpr files but no {MANIFEST}: they cannot be tied to the "
                        "current run (a stale or partial directory)")
    if manifest is not None:
        listed = set(manifest)
        for f in files:
            if f.relative_to(directory).as_posix() not in listed:
                problems.append(f"{f}: not listed in the current {MANIFEST} (run "
                                f"{run_id or '?'}): a stale file, or one from a partial run")
    if not files:
        problems.append(f"{directory}: no .vpr file to lint")
    for f in files:
        problems += lint_file(f)
    return problems, len(files), None, run_id


def lint_file(path: Path) -> list[str]:
    mapf = path.with_name(path.name + MAP_SUFFIX)
    try:
        text = path.read_text()
    except OSError as err:
        return [f"{path}: cannot read ({err})"]
    map_text = mapf.read_text() if mapf.is_file() else None
    return lint_vpr(text, map_text, str(path), str(mapf))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("paths", nargs="*", help="directories or .vpr files (default: build/vpr)")
    args = parser.parse_args(argv)
    paths = [Path(p) for p in args.paths] or [ROOT / DEFAULT_DIR]
    problems: list[str] = []
    linted = 0
    notes: list[str] = []
    runs: list[str] = []
    for p in paths:
        if p.is_dir() or (not p.exists() and not args.paths):
            try:
                probs, n, note, run_id = lint_directory(p)
            except rt.UsageError as err:
                print(f"lint_viper: {err}", file=sys.stderr)
                return 2
            problems += probs
            linted += n
            if note:
                notes.append(note)
            if run_id:
                runs.append(run_id)
        elif p.is_file() and p.suffix == ".vpr":
            problems += lint_file(p)
            linted += 1
        else:
            print(f"lint_viper: {p}: not a directory or a .vpr file", file=sys.stderr)
            return 2
    for line in problems:
        print(line)
    if problems:
        print(f"lint_viper: {len(problems)} problems in {linted} files")
        return 1
    if notes and linted == 0:
        print(f"lint_viper: {notes[0]}")
    else:
        print(f"lint_viper: {linted} files clean" + (f" (run {', '.join(runs)})" if runs else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
