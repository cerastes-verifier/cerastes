# 0063 — Arrays of scalars in structures and static storage (M4.5e1e1)

- **Status:** accepted (2026-11-25, M4.5e1e1; the full `check.sh` on the box is left to the owner, at the owner's
  request), written before code (invariant 6) from section 7 of the plan
  (`.claude/plans/m4-5e1e1-arrays-of-scalars.md`, written at M4.5e1e's planning and executed without re-planning);
  revised by P82's and S47's decision tables (the plan's section 6; `internal/il-inventory.md`, chapter M4.5e1e1;
  44 fixtures `tests/viper-smoke/memory/s47_*.vpr`; every row on its first branch, the table at the end), Step 0's
  runs and the advisor's rulings 301–303; and by audit round 1 (Opus, on Commit 1 `d9a51fc` and Part A's design: no
  blocking item; should-fix S1 the poison over every walk of a structure's members, item 4; S2 the copy's guard,
  item 13; S3 the rows that pair array leaves over slot terms, item 7; S4 the bound local's grammar, item 15; S5 the
  count's source, item 15; S6 `int g[];`, item 2; S7 the decay by its `expr` and both escape directions, item 9;
  notes N2–N5, N7–N10 taken in items 2, 7, 9, 12, 13 and the tests; `build/m4.5e1e1/auditor-report-1.md`), with the
  advisor's rulings 305–307; and by audit round 2 (a fresh Opus auditor, on Parts A, B and D at `2aa4903`: one
  blocking item, B1, a write inside a compound literal used as an object, unsound since M4.5c2a, item 12, question
  308; should-fix S1–S2 tests, S3 a file-scope typedef whose bound names a constant, item 14, S4 a page; notes N1, N2,
  N5 taken in items 4, 7 and 10; `build/m4.5e1e1/auditor-report-2.md`; its re-reads found B1' (a writing literal as a
  whole element of a braced list) and closed it, `auditor-report-2b.md`, `-2c.md`); and by audit round 3 (a fresh
  Opus auditor, on Part C at `77d557b`: no blocking item; should-fix S1 a structure global's array leaf without
  liveness in `main`'s loops, S2 a call row's amount under a quantifier, S3 the lint's guarded length fact, S4 an
  abnormal end in a heap unit, all taken; notes N1, N4, N9 listed false alarms, items 7 and 10; N5–N7 items 4 and 6;
  `build/m4.5e1e1/auditor-report-3.md`); and by audit round 4 in two halves (4a on Commit 3 and the pages, 4b on Part E:
  no blocking item; the lint for pointer-element array members, a false message for a C++ temporary, the copy's
  message and the pages, all taken; `auditor-report-4a.md`, `-4b.md`), with the advisor's ruling 309 (the by-value
  limit, item 13). Owner decisions:
  - At M4.5e1e's planning (2026-11-25): **290** M4.5e1e is two sessions, "scalars, then structures" (this record's
    arrays of scalars; M4.5e1e2's arrays of structures and of arrays); **291** a structure with an array member is
    copied and passed and returned by value in M4.5e1e ("Copy and by-value now"; item 13, the plan's Part E);
    **300** Part E runs in this session unless the remaining work would pass the twelve-hour cap.
  - Defaults the plan builds on, for the owner's review (each fail closed; `internal/handoff.md` §8): **292** heap
    storage of a structure with an array member rejected (M4.5e1e2); **293** a structure with an array member and an
    `owning` or marked member rejected (M4.5e1e2); **294** `ref_to_uninit` and `must_init` on a pointer to a
    structure with an array member rejected (M4.5e1e2); **295** a structure value of more than 64 array elements not
    passed or returned by value (M4.5f3), a copy having no limit; **296** a whole-structure copy needs every element
    of the source's array members initialized; **297** if Silicon does not take the trusted copy, the copy is
    element by element under 295's limit; **298** no new tutorial section (M4.5e1e2's); **299** question 285's
    answer as the standing rule for a test slow on the box.
  - At M4.5e1's planning: 226–233, 236, 237 (`.claude/plans/m4-5e1-arrays.md`, section 1).
  - Standing: 69 (no EDG work), 77 (one context, under about 12 h), 160 (a new test's expect line corrected after
    its first run, listed in status), 176 (the owner handles CI), 277 (a twin with several real errors lists every
    one), 285 (a test slow on the box only because of what it must refute goes to `tests/long/` with a short twin).
  - Questions settled with the advisor in this session: 301–303, 305–309 (`internal/handoff.md` §8); 304 is open for
    the owner.
- **Amended by 0064** (M4.5e1e2's first session, 2026-11-25): item 6's rule for `{0}` and brace elision is
  replaced by the measured rule of both compilers, designated lists included (record 0064 item 16; question 304
  answered by it); item 4's leaf views gain leaf paths with index steps and the `aggregate_element` flag (0064
  item 4). The other items 0064 amends (1, 2, 5, 7 to 10, 13 to 15) are built by its continuation: items 5 and 7
  to 10 and 13 for arrays of structures by its first session (2026-11-26), and **items 1 and 7's six leftovers of a
  structure with an array member (questions 292 to 294 and 306, partnerless atoms, written atoms) by its second
  session, the same day (0064 item 15), for array members of scalars**; **arrays of arrays (items 2, 9, 13 and 14: matrices as globals, `static` locals and members of structures, and array types of arrays and of structures in the computed positions) by its fourth session (2026-11-27, 0064 item 13), except three dimensions and whole structure values that hold a matrix (0064's status line says what of them is built).**
- **Milestone:** M4.5e1e1.
- **Amends:** 0012 item 11 (the lint); 0024 item 6 (array types of scalars); 0028 items 5 and 16 (as built for
  arrays); 0041 items 2, 4, 5, 10 and 12; 0043 items 2, 3, 4, 6, 7, 8, 9, 11 and 13; 0044 items 2, 3, 4, 7 and 8;
  0059 items 1, 2, 4, 8 and 11; 0060 items 1 and 9; 0061 item 1 and 0062 item 4 (a named global array as an
  argument).

## Context

Record 0059 (M4.5e1a) made automatic local arrays of scalars objects: a block of N slots, `a[i]` with
`out-of-bounds`, braced initializers with C's zero fill, the marked range rows of record 0028 item 11, loops that
carry arrays as range places. Record 0060 (M4.5e1b) gave ranges of them to contracts, 0061 (M4.5e1c) quantifiers
over them, 0062 (M4.5e1d) `pure` functions. Every other array is still rejected with a diagnostic naming M4.5e1e:
a global table, a buffer in a structure, `typedef int vec3[3]`, `sizeof(int[3])`. Record 0028 item 16 fixed their
layout in M4.5a (an array member is the block `member(r, k)` of length N) and S26 measured it on Silicon; none of it
was built for arrays. M4.5e1e was cut in two at its planning (question 290): this record takes arrays whose elements
are scalars in the places e1a left out; M4.5e1e2 takes arrays whose elements are structures or arrays.

Facts: P82 (the IL of global, `static`, member and literal arrays, their initializers, structure values with array
members and computed array types) and S47 (Silicon on the shapes of items 3–13), in `internal/il-inventory.md`,
chapter M4.5e1e1; the table at the end maps each outcome to the branch of the plan's decision table it took.

## Decision

Below, a **scalar** is `bool`, an integer type, an enumeration or a pointer to an object type (0059 item 1's element
types); an **array** is one dimension of scalars with a constant bound of at least 1.

**No existing program's Viper or map changes.** Every item below adds rows only for a program that holds a
construct rejected before this record; where an existing function is touched (`inv_place_disjunct`, `global_facts`,
`struct_lens`, the leaf enumerations), the text it writes for a program without the new constructs is the same, and
`scripts/vprdiff.py` against the session's baseline shows it after every part.

1. **Scope.** In:
   - **(G) global and `static` local arrays:** definitions with and without a braced initializer; `g[i]` read and
     written, `&g[i]`, the decay of `g`, `sizeof(g)`; initial values by linkage (0041 item 3); `owns(g)` and
     `reads(g)` (every element), element reads in contracts, `old`, `decreases`; the implicit atoms of a `static`
     local array; loops; `program_start`; `g` passed to a callee's `owns_range` or `reads_range`; a named global
     array as an argument of `cerastes_forall` and of a `pure` function in a loop invariant or a `check`.
   - **(M) array members:** `struct S { int a[4]; int n; }` in every storage 0043 and 0044 give a structure (a
     structure local, a `struct S *` object, a structure global and `static` structure local, a nested member
     `s.t.a[i]`); `s.a[i]`, `p->a[i]`, `&s.a[i]`, the decay of `s.a` and `p->a`, `sizeof(s.a)`; braced initializers
     with a nested list for the member; `owns(*p)` and `reads(*p)` (the array's elements with the other leaves),
     `owns(p->a)` and `reads(p->a)` (the whole array member), element reads `p->a[i]` in contracts; calls, returns,
     `f__wf`, loops, scope exits, sequencing.
   - **(L) array compound literals in C:** `(int[]){1, 2, 3}` as an object of its block.
   - **(T) array types the front end computes** (0024 item 6, for arrays of scalars): typedefs and aliases; `typeof`
     and `decltype`; `sizeof` and `_Alignof` of such types; a bound that names a local constant where P82 (f) shows
     it can be read.
   - **(V) structure values with array members** (question 291): the whole-structure copy inside a function;
     by-value parameters and results; `f().a[i]`; compound literals of such structures.

   Everything else stays rejected, each with a diagnostic naming where it is lifted and a pin:

   | Construct | Lifted by |
   |---|---|
   | an array of structures (`struct S v[3]`, `p[i].x`, a range over structures); an array of arrays (`int m[2][3]`, `int a[][3]`, `sizeof(int[2][3])`); an array member whose elements are structures or arrays | M4.5e1e2 |
   | `cerastes_forall` and `pure` applications whose pointer argument is a member (`p->a`, `s.a`), or a global array in a contract; structure members under a binder | M4.5e1e2 |
   | heap storage of a structure with an array member (`malloc(sizeof(struct S))`, `owns_block`, `free`) | M4.5e1e2 (question 292) |
   | a structure with both an array member and an `owning` or marked member; `ref_to_uninit` and `must_init` on a pointer to a structure with an array member; a written atom in a loop invariant on a structure place with an array member | M4.5e1e2 (questions 293, 294) |
   | a contract place reached through a heap read (0045) with a structure that has an array member anywhere on its path, root included (`owns(*p->q)`, `p->q->a[i]` in a contract) | M4.5e1e2 (question 306) |
   | an `ensures` atom with no `requires` partner (0052) on an array member or a structure with an array member (`owns(*r)` of a result) | M4.5e1e2 |
   | a range on part of a global array or of an array member (`owns_range(g, n)`, `owns_range(p->a, n)`, `&g[2]` as a base); an atom on one element; `&g[i]` and `&p->a[i]` in a contract; a global initializer `&g[2]`, `int *gp = s.a;` or `&s.a[k]`; `g + i` | M4.5e2 |
   | a structure value that holds more than 7 array elements, passed or returned by value (a copy has no limit) | M4.5f3 (questions 295 and 309: array values as ghost sequences) |
   | a read `pure` function over a structure or a global (0062 item 1, 0041 item 12) | M4.5f1 |
   | `const` arrays in any storage (a `const` global table, `static const`); `char s[] = "abc"`, `sizeof("abc")`, `sizeof((char[4]){0})` | M4.6 |
   | an array of owning pointers; `owning` or `ref_to_uninit` on an array | M4.8 |
   | array designators (`{[2] = 5}`), VLAs, a zero-length array, flexible array members, a pointer to an array (`int (*p)[3]`, `sizeof(char(*)[4])`), `[static N]`, `thread_local` arrays, a block-scope `extern` array, `__datasizeof`, `_Alignof` of an expression, a typedef of a function type with an array parameter | M8 (function types: M4.7) |
   | an array compound literal in C++ (a GNU extension), at file scope, `const`, or in a loop condition, a `for` step or a `do` condition (0044 item 7's list) | M8 |
   | array types as arguments of type traits and variable templates (`__is_same(char[4], char[4])`); `std::array<char, 4>` | M5.0, M5 |

2. **Which arrays.** One dimension; the element a scalar that 0059 item 2 accepts (not `const`, `volatile` or
   `_Atomic`; a `const` array in any storage is M4.6's, and **the `const` rule is tested before the array rule**, so
   its message names M4.6); a constant bound of at least 1, or none with a braced initializer.
   - **The bound is read as written** (0059 item 2, `check_array_bound_constant`): a bound with an `expr` is a
     checked constant with its obligations at the bound's position and the `internal` guard that the value as written
     is EDG's (0017 item 4). For a global, a `static` local, a member and a file-scope typedef it is one of the unit's
     prologue constants (0041 item 5's list), named for the message ("the bound of `g`", "the bound of `S::a`", "the
     bound of `vec3`"); for a block-scope typedef and a `typeof` it is a constant of its function. A literal, a
     macro's literal and an enumerator keep no `expr` and need no check (P78 (a)). **In C++ every bound that is not
     a local array's** (a global's, a member's, a typedef's, a type operand's) is an `unsigned long` constant whose
     `expr` is a compiler-generated cast over the written tree, a literal `[3]` included (P82 (b), as P79 F2 found for
     parameter bounds): the bound is read under that generated cast, through the written tree.
   - **A member's bound is a prologue constant whenever its structure type is accepted** (as an object's type, a
     pointee, a type operand, in a static assertion; audit round 1, N9), so `sizeof(struct S)` never skips a wrapped
     member bound.
   - **Rejected:** item 1's table; `owning`, `ref_to_uninit` and `uninit` on an array member or a global array (0059
     item 2's rule for a local; `unsupported_annotation`); `int g[];`, a tentative array definition without a bound,
     like the compilers (clang and GCC warn by default; EDG completes it to one element silently; audit round 1, S6).

3. **Layout** (0028 items 5 and 16, as decided in M4.5a).
   - A global or `static` local array `T g[N]` is the one block `GB_g()` of `len` N; element `k` is `loc(GB_g(), k)`
     in `T`'s value and flag fields. The global facts state `len(GB_g()) == N && lvloc(GB_g()) == loc(GB_g(), -2)`.
   - An array member `T a[N]` at index `k` of the structure slot `X` is the block `member(X, k)` of `len` N; element
     `i` is `loc(member(X, k), i)`. It shares its root's liveness cell (`member_lv`). `struct_lens` (0043 item 8's
     `WF_S`), `alloc_struct`'s `ensures` and a structure global's facts state `len(member(X, k)) == N` where they
     state `== 1` for every other member.
   - Both hold in 0028 item 2's model (`len` is free; a member block is a fresh term), and they are hypotheses
     discharged by an allocation, by `program_start` or by a caller, never axioms.

4. **VIR.**
   - `Type::Array` is the type of a `Global` and of a `StructMember`, beside an automatic local's. An array still has
     no value: its address (`AddrOf` of an array local, `GlobalAddr` of an array global, `MemberAddr` of an array
     member) is accepted as the base of an `ElemAddr` and as the operand of a whole-array atom, and nowhere else.
   - **Three views of a structure's leaves.** The **object view** (`struct_object_leaves`): each leaf is a cell or an
     **array leaf** (the member's path, the element type, the count). The **value view** (`struct_value_leaves`, item
     13): every element of an array member is a leaf of its own, in index order (`StructLeaf::elem`). **The old view,
     `struct_leaves`, is unchanged and keeps returning an array member as one leaf of the array's type: it is the
     poison.** A site that still treats that leaf as a cell is stopped: **every walk of a structure's members** in
     lowering — `leaf_cells` and every `struct_leaves` loop, and also `leaf_count`, `leaf_span` (and so `field_of`),
     `has_pointer_leaf` (and so `struct_ptr_uses` and `expr_touches`) and `struct_lens` (audit round 1, S1: by
     reading, `leaf_span` would make `f().n` read `a[1]` in the value view), and `alloc_struct_method`'s member walk
     (audit round 2, N5: it wrote `len(member(r, k)) == 1` for every member of a trusted contract; Part C replaces
     it) — reports an internal error on an array member until its part gives that walk a view (exit 3;
     `cell_suffix` already does),
     `check.cpp` refuses a structure with an array member in every position not yet lifted, and the encoder rejects it
     there first with a diagnostic at the C line (invariants 3 and 7). For a structure without an array member the
     three views are the same list, so every existing program is lowered as before.
   - `Global` gains the initial values of an array: the listed prefix (one `GlobalLeaf` per written element), the
     zero rest, or "unknown".
   - `Stmt::Kind::ArrayZero` (the address of an array member: C's zero fill of the elements an initializer does not
     write; e1a's `Var::zero_filled` stays for a local), `Stmt::Kind::ArrayCopy` (item 13) and `Expr::Kind::ElemValue`
     (element `i` of an array member of a structure **value**, item 13).
   - `check.cpp` backstops each rule of items 2–14 (positions, types, a count that matches the type, an `ElemAddr`
     base that is one of the three array addresses, `ArrayCopy` between two array members of one type).

5. **Rows in code.**
   - **The bound local** (S47 (a)). Where lowering writes a row over an array's slots in code (a scope exit, a loop's
     entry, conjuncts and back edge, a call's and a return's marked rows), it first binds the array's first slot to a
     Viper local (`t := loc(member(X, k), 0)`, `t := loc(GB_g(), 0)`) and writes **e1a's and e1b's rows over that
     local, unchanged**. A marked row's block is then still `block_of` of a Viper local (0059 item 7's argument that
     its reason is unambiguous holds as written). The block-term form (`loc(member(X, k), j)`, `loc(GB_g(), j)`, with
     `X` the canonical member chain of 0043 item 13) stands only in lines that have no local: a method's contract,
     `alloc_struct`'s `ensures`, `program_start`, the global facts, `struct_lens`. (If S47 (a) refuses the bound local,
     every row is in the block-term form.)
   - **An element access** `g[i]`, `s.a[i]`, `p->a[i]`, `t.s.a[i]` is `ElemAddr` over the array's address, with 0059
     item 4's rows in its order. A global array and an array member are **named** bases: no null or liveness row of
     their own (a global's block never ends; a member's is its structure's). For `p->a[i]` the root's row is 0043 item
     5's (`p != null`, `null-dereference`), once, and **the access is not a use of `p`'s value** (no liveness row):
     `elem_addr` passes the place mark to a `MemberAddr` base. Then the bound, against the length item 3's facts give
     (`out-of-bounds`: "reads `s.a[i]`, whose index may be outside `s.a`"), `permission` and, for a read,
     `uninitialized`. The decay of `s.a` or `p->a` used as a value, and `&p->a[i]`, form `&p->a` first: 0043 item 5's
     row for the address of a member (`null-dereference`, then `p`'s liveness).
   - **The decay of a global array as a constant** (P82 (a)): in a call argument `f(g)` and an initializer `int *r
     = g;` (both modes), and in C as the base of `&g[i]`, the front end folds the decay into a `ck_address` constant
     of `g` at offset 0 with no subobject path, whose `expr` is the written `array_to_pointer(g)`. It is read from its
     `expr` as the decay (0041 item 6's rule: a constant that holds an address is never read by EDG's value). `&g[k]`
     and `g + k` carry a subobject path (and `&g[1]` an offset), which is how they are told from the decay.
   - **Rejected like the compilers** (0059 item 4, extended to the new bases by P82 (g)): a constant index outside a
     global array or an array member (EDG's #175, already an error in verify mode); a truth test or a null comparison
     of such an array's address (`array_address_null_test`); a plain `char` index.

6. **Structure objects with an array member** (amends 0043 items 4, 6 and 8).
   - **Allocation:** `alloc_struct_<n>_<tag>()` ensures, for an array member's block, `len(member(X, k)) == N` and,
     each as its own quantifier with the trigger `{ loc(member(X, k), j) }` over `0 <= j && j < N`, full permission to
     the value field, to the flag field, and every flag false. Consistent as 0043 item 4's is: the member blocks are
     fresh terms, and each (receiver, field) pair is given once.
   - **Scope exit:** per array leaf, e1a's rows over the bound local (the two marked rows, `lifetime`: "the address of
     an element of `s.a` may still be in use when its lifetime ends"), with the cell leaves' rows, then one `exhale`.
   - **Initializers** (0043 item 6's rules for the list): the array member takes a nested braced list in e1a's element
     rules (each element an assignment conversion read through `init_element_value`; an array designator rejected,
     M8; a subobject initialized twice rejected in C; in C every element of the whole structure list is evaluated
     before the first store, in C++ in order). The elements the list does not write are zero: `ArrayZero` on the
     member, then the written elements' stores. A list that elides the member's braces is rejected like the compilers
     (`-Wmissing-braces`), except `{}` in both modes and C's `{0}`. **In C++ a `{0}` whose `0` elides an array
     member's braces is rejected** (question 303: clang++ warns `-Wmissing-braces` at `-Wall` on it, P82 (g); no
     program with an array member was accepted before, so this is the union rule of 0020 item 5 applied, not a new
     rejection; C is silent in both compilers). Measured on clang 21 (Step 0's M): clang++ flags the elided braces of
     an array or structure member only in an aggregate of two or more members; `struct W { int a[4]; } w = {0};` is
     silent at every tier and is accepted in both modes, and so is `{{0}, 0}`; `{{0}}`, which omits a trailing
     member, warns only at `-Wextra` (accepted). GCC's `-Wmissing-braces` fires in C only. C++'s `{0}` over a structure whose first member is a nested
     structure, accepted since 0043 although clang++ warns on it at `-Wall` too (the matrix's
     `init_local_nested_struct_zero.cpp`), is unchanged here and is question 304, for the owner. **Elision is told by
     the flag** (P82 (c)): `{1, 2, 3, 4, 5}` and
     `{{1, 2, 3, 4}, 5}` have the same nested IL, and only the inner aggregate's `explicit_braces_on_aggregate` tells
     them apart; `{0}`'s inner aggregate has no such flag. C++ designated lists carry a compiler-generated leading
     placeholder (an empty aggregate for an array first member), skipped as 0043 item 6 skips its zero fillers. An
     excess inner element is the front end's error in both modes. **An initializer element that reads, writes or
     takes the address of an element of an array member of the structure being initialized** (`{{1, s.a[0]}, 2}`,
     through `ElemAddr` at any depth) **is rejected** (`unsupported_declaration`, as 0043 item 6 rejects writes and
     addresses); a read of a scalar member keeps 0043 item 6's rule (`{1, s.x}`: `uninitialized` in C). The
     `ArrayZero` of a member therefore comes before the C temporaries' evaluation without any element of it being
     observable early.
   - **Without an initializer** every flag stays false; `cerastes::uninit` on the structure local is accepted and
     emits nothing (0043 item 9).

7. **Contracts** (amends 0041 item 7, 0043 item 9, 0060 items 1 and 9).
   - **Atoms.** `owns(g)` and `reads(g)` of an array global: every element (the two quantified permissions, full or
     at the atom's ghost amount, the flags, the element range, one liveness fraction), in the block-term form over
     `loc(GB_g(), k)`, `0 <= k && k < N`. `owns(*p)` and `reads(*p)` of a structure with an array member: 0043 item
     9's lines for the cell leaves and, per array leaf, the same four lines over `loc(member(X, k), j)`. `owns(p->a)`
     and `reads(p->a)` (and `g.a` of a structure global): that array leaf alone, with one liveness fraction. **As
     built for an array global** (Part B): `owns(g)` and `reads(g)`, written or implicit (a `static` local array), are
     lowered as e1b's range atom over the whole array (`OwnsRange`/`ReadsRange`, base `loc(GB_g(), 0)`, the count the
     literal N), so every rule of 0060 items 4–11 — the amounts, the shape, marked, flag and range rows at calls and
     returns, the sum forms, the separation rows, the partners — applies to it as written. **An atom
     never names part of an array here:** `owns_range` on a global or member base, an atom on one element and
     `&p->a[i]` in a contract stay M4.5e2's.
   - **Element reads:** `g[e]`, `p->a[e]`, `r->a[e]` of a pointer result and `g.a[e]` in `requires`, `ensures`, a
     function's `decreases` and under `old`, the index reading no memory (0060 item 5's rule). A read is framed by an
     atom that covers its array leaf: the disjunct is "the site's guard, the same block, the index inside `[0, N)`",
     and `f__wf` asserts the disjunction with the bound (`permission`: "the contract of `f` reads `p->a[i]` without
     owning it"; an index outside is the same row, as 0060 item 5 has no separate bound row). `f__wf`'s ghost value of
     a read is keyed by root, member path, index and state.
   - **Overlap is by slots** (0043 item 9, the soundness core). An array leaf of an atom is, for every rule that pairs
     two sites (the exclusion of `owns` and `reads`, the amount sums at calls and returns, the entry separation,
     `f__wf`'s distinctness and equal-ghost facts, framing disjuncts, the case splits, loop places), **a range of N
     slots at `loc(member(X, k), 0)` or `loc(GB_g(), 0)`**: it meets a cell `q` of its element field when `q` lies in
     that block within `[0, N)` (`in_range`'s test), and another array leaf or range when the intervals of one block
     meet. So `f(&s, &s.a[1])` with `owns(*p) && owns(*q)` fails at the call (`permission`), as `f(&s, &s.x)` does,
     and so does `f(&g[1])` against `owns(*p) && owns(g)`.
   - **The rows that pair an array leaf with another site are e1b's, over slot terms** (audit round 1, S3, N3; the
     standing row rules of 0028 item 11 and 0060 items 7–9, which the lint and the map's expected-reason column
     carry): no entry separation is asserted for an array leaf (as for a range, 0060 item 7); at a call and a return,
     with a cell `q` of its field where either atom is an `owns`, the pure interval separation row `!(g && block_of(q)
     == block_of(t) && 0 <= offset_of(q) && offset_of(q) < N)` (`permission`), `t` the leaf's bound local; with another
     array leaf or a range of the field where either is an `owns`, 0060 item 8's two-range row, and for two array
     leaves of one structure type at one index the first-slot inequality `t1 != t2` — **never a bare block
     comparison** such as `member(p, 0) != member(q, 0)`, which Silicon does not decide from quantified permissions
     (the audit's p10a; `loc(member(p, 0), 0) != loc(member(q, 0), 0)` verifies, p10b); for two `reads` sites, the
     sum form of 0060 items 8 and 9 in marked `acc` rows, never `perm` under a quantifier. A row Silicon cannot decide
     is a false `permission` or an exit 3, never a proof (the audit's p1: without any row the call's own exhale
     fails).
   - **For an array global** (audit round 2, N2): `f__wf` emits no distinctness fact between a cell and the array
     atom and no slot ghost for its elements (both only weaken); its "a `reads` place keeps its value" fact covers
     `reads(g)` (0060 item 5's argument: no verified callee writes a slot it holds only through read atoms).
   - **`f__wf`'s assumed facts over array leaves are by slot** (audit round 1, N4), never a comparison of member
     paths: an `owns` cell against an `owns` array leaf `g_i && g_j ==> !(block_of(q) == B && 0 <= offset_of(q) &&
     offset_of(q) < N)`; two `owns` array leaves the first-slot inequality; equal ghosts `loc(B, i) == loc(B', j) ==>
     h == h'`. Each is implied by the `requires`.
   - **Amounts** (0037 item 6, 0060 item 6): a `reads` atom's amount at a call is the minimum over its cell leaves
     and, for each array leaf, the leaf's first slot (0060 item 6's rule, with its listed limit for an uneven
     holding: question 238, M4.5e1f).
   - **Listed false alarms** (audit round 3): `f__wf` does not tie a member element's pre- and post-state ghosts
     (N1: `reads(p->a)` with `requires -1000 < p->a[0] < 1000` and `ensures r == p->a[0] + p->a[0]` reports `overflow`;
     only weaker, since ghosts are keyed with the state); an implicit `owns(s)` of a `static` structure local never
     covers a partnerless atom through its array leaf (N4: `int *get(void)` with `ensures(r, owns(*r))` returning
     `&c.a[1]` reports at the return).
   - **Rejected** (`unsupported_annotation`): 0043 item 9's overlapping atoms on one root, now also `owns(*p) &&
     owns(p->a)`; a marker on a pointer to a structure with an array member (question 294); a written atom in a loop
     invariant on such a structure place.

8. **Calls and returns** (amends 0037 items 8 and 10, 0043 item 9). A call's and a return's rows for an atom are per
   leaf: the cell rows as before, and per array leaf e1b's marked rows (value, then flag) over the bound local, with
   the messages in source terms ("calls `f`, which needs every element of `s.a`"). The decay of a global array or of
   an array member passed to a callee's `owns_range(q, n)` or `reads_range(q, n)` is bound to a local first (S44
   F-b1) and takes 0060 item 8's rows as any pointer does.

9. **`program_start`, initial values, `static` locals** (amends 0041 items 3–5 and 12, 0044 item 8).
   - **An array global's rows**, positioned at its declaration, in this order: the two quantified permissions over
     `[0, N)`; the flags, every one true; the values; `acc(lvloc(GB_g()).lv)`. The values, by 0041 item 3's linkage
     rule: **known** — one conjunct per written element (`loc(GB_g(), i).v_T == c_i`; a pointer element `null` or
     `loc(GB_h(), 0)`), and one quantifier for the rest (`m <= k && k < N ==> … == 0`, `false` or `null`);
     **unknown** — the element range (or `WF`) as one quantifier.
   - **Initializers.** A braced list of constant elements, each read through `constant_value` (P38, measured again by
     P82 (a): in a list of 1,002 elements or more, an element under an implicit conversion keeps only its
     `orig_type` and no `expr`, and that element is rejected by `constant_value`'s own rule, never read from the
     aggregate; a literal of the element's own type and a folded computed element keep what they had), each with an
     `expr` checked in the prologue as 0041 item 5 checks a scalar global's. No designators (M8). `int *gp = g;` is the
     address of element 0 (P82 (a): a `ck_address` of `g`, offset 0, no subobject path; item 5); `&g[k]` (a subobject
     path) is M4.5e2's. An `extern` declaration and its later definition are one variable; `int g[] = {…}` takes its
     bound from the list; a `static` local array's initializer is its scope's `local-static-variable-init` entry,
     as a scalar `static` local's is (0041 item 5).
   - **Escape through an element** (0041 item 3's rule, per element as 0044 item 8 has it per leaf): in C++ a global
     whose address is the initial value of an element of an array whose values are unknown is itself unknown,
     transitively (`static int h; int *tab[2] = {&h, 0};` with `tab` of external linkage: another unit's initializer
     may write `*tab[0]` before `main`). **And the other direction:** in C++ an array global whose decay is the
     initial value of a global of unknown value (`static int g[3] = {1, 2, 3}; int *gp = g;` with `gp` of external
     linkage, or the element of such an array, or the leaf of such a structure global: `static int g[3]; struct P {
     int *p; } ps = { g };` with `ps` external) has unknown values in every element, transitively (another unit's
     initializer may write `gp[1]` before `main`); a twin relies on such an element's value and must fail.
   - **The decay is recognized by its `expr`** (audit round 1, S7 (c)): a `ck_address` over an array global is the
     decay only when its `expr` is `array_to_pointer` of the variable, with offset 0 and no subobject path as
     consistency checks; any other `ck_address` over an array (`&g[k]`, `g + k`, a folded cast) is rejected.
     `int *gp = s.a;` and `&s.a[k]` as a global initializer (a structure global's array member: a `ck_address` with
     a subobject path) are rejected as `&h.x` is (0044 item 8; item 1's table, M4.5e2).
   - P38's loss of `expr` is per list (audit round 1, N5): in a list of 1,002 elements or more, every element under an
     implicit conversion loses it, from index 0.
   - **A `static` local array** is a global whose name its function sees (0041 item 12), with the implicit `owns` atom
     of item 7's array form in its function's and its callers' contracts.
   - **A structure global** with an array member: its row gains the array leaf's lines; its facts the length.
   - `program_start` stays a trusted contract pinned line by line (item 15); its rows give each (receiver, field) pair
     once.

10. **Loops** (amends 0038 item 3, 0041 item 10, 0043 item 11, 0059 item 8). A loop that touches memory takes, as
    **array places** with 0059 item 8's entry rows, conjuncts and back-edge rows over the bound local: each global
    array the function holds (every one in `main`; elsewhere those its `requires` atoms name, at `write` or at the
    atom's amount, under its guard), each array leaf of every structure object in scope, and each array leaf of a held
    structure place of a parameter (guarded and at its amount, as a range place is).
    - **Reach:** a store by name to an element of a global array the function holds as its own place reaches that
      array only (0044 item 8's rule); a store to an array member of a structure local, any store through a pointer of
      the element field, and a call whose `requires` owns a cell of that field reach every array place of the field
      (0043 item 11's rule for members, unchanged).
    - **A global's place is hit by an interval:** `inv_place_disjunct`'s shortcut for a global is the identical slot-0
      term today; an array global's place is hit when the read's block is `GB_g()` and its offset is in `[0, N)`. For
      a scalar global the emitted text is unchanged. **As built** (audit round 2, N1): an array global is never a
      cell place; in `main` it is e1a's array place, elsewhere e1b's range place over its bound local
      (`ga_<name>__<id>`, assigned `loc(GB_g(), 0)` once in every method of a unit with an array global), both hit by
      the interval test, and `inv_place_disjunct` is unchanged.
    - Invariant and measure reads of `g[e]`, `s.a[e]` and `p->a[e]` are framed by these places, with 0059 item 8's
      `out-of-bounds` and `uninitialized` obligations.
    - **Listed false alarm** (audit round 3, N9; 0043 item 11's rule, by design): a store to a member of one structure
      local in a loop — an element of an array member or a cell member of the element's field — reaches every array
      place of that field, so a loop that writes `t.a[2]` or `t.n` cannot keep `s.a`'s values (all `int`); a `check`
      after it relying on them fails.

11. **Sequencing** (0039 item 3, 0059 item 9). Object keys gain the element of a global array and of an array member
    (`g[0]` and `g[1]` are different objects; a global array is no longer one object). Two unsequenced accesses to
    elements of one array, one a write, with indexes that are not both constants, get the path-precise `unsequenced`
    row; the same element named twice with a write is rejected like the compilers (`unsequenced_modification`).

12. **Array compound literals** (C; amends 0044 item 7). `(T[]){…}` and `(T[N]){…}` designate an unnamed array
    object of the enclosing block: a hidden array local, allocated at block entry as e1a's are, exited with its
    block; its value is its decay. **As built:** the hidden local is declared just before the statement that holds
    the literal (0044 item 7's block rule), zero-filled at that declaration when the list leaves elements out (as S47
    (f)'s fixture has it: no pointer to the object exists before the literal is evaluated, and every evaluation is a
    fresh entry of its block, since a backward `goto`, a loop condition, a `for` step and a `do` condition holding a
    literal are rejected), and its listed elements are stored inside the expression where the literal is evaluated,
    in list order (two elements with effects are rejected, `unsequenced_calls`, so the order is not observable);
    `&(int[]){…}` (a pointer to an array) stays rejected (M8). 0044 item 7's rejections hold for it (C++, file scope, `const`, a loop condition, a
    `for` step, a `do` condition, `return` of its address). P82 (e): the literal is `array_to_pointer` over an
    `enk_temp_init` flagged `is_compound_literal`, its elements in e1a's aggregate shape; **C++20 accepts array
    literals with no diagnostic at all**, so the encoder's rejection is the only guard there. A literal with a
    storage-class specifier (`(static int[]){…}`) is EDG's error #80 today (GCC 16 accepts it); the encoder rejects it
    should a later EDG accept it (audit round 1, N10).
    **A write inside a literal used as an object** (audit round 2, B1; question 308, the advisor's ruling; **unsound
    since M4.5c2a** for the scalar and structure object forms of 0044 item 7): the object form is built as a comma of
    element stores ending with the address, and the sequencing rules took that comma's left side as a sequence
    point, so `x = (int[]){x++}[0];`, `x = *&(int){x++};` and `*p = *&(int){(*p)++};` verified although a compound
    literal's initializers are not full expressions (C11 6.8p4: each is `x = x++`). **Rejected, by position:** a
    compound literal used as an object (an array literal, `&(T){…}`, `*&(int){…}`) whose element writes (an
    assignment, `++`, `--`, a compound assignment, anywhere in the element, among a call's arguments too; the call
    itself is sequenced and exempt) is `unsupported_expression` unless the literal is the whole full expression (an
    expression statement, after a `(void)`), a declaration's whole initializer or a whole element of a braced
    initializer list (C23: each is a full expression). **Between the elements of a braced list** (audit round 2's
    re-read, B1'): the list rule of 0021 and 0043 item 6 ("two elements of an initializer list, one of them with a side
    effect", `unsequenced_calls`) now sees a write inside a compound literal of an element, so `int *a[2] =
    {(int[]){x++}, (int[]){x}};` is rejected as `{x++, x}` is (it verified, fixing one order; the scalar form `&(int){x++}`
    since M4.5c2a). Over-strict (it also rejects the defined `f((int[]){x++}[0])`); lifted when the
    sequencing rules see inside a literal (M8). The value form keeps 0044 item 5's `unsequenced_modification`.

13. **Structure values with an array member** (question 291; amends 0043 item 7 and 0044 items 2–5).
    - **The copy** (`t = s`, `*p = *q`, `p->t = s.t`, an initializer from an object). The cell leaves as 0043 item 7
      has them. Per array leaf: the source's permission (a marked row, `permission`) and **every source flag**
      (`uninitialized`: "copies `s`, whose member `s.a` may hold an element without a value"; question 92's rule, for
      elements: question 296), the destination's permission at `write` (a marked row); then, **unless source and
      destination are one object**, `copy_arr_<t>(d, s, N, rd)`. An exact self-copy is guarded out (`*p = *p`
      verifies, as before); partial overlap cannot arise (two structure places of one type are one slot or disjoint).
      **The guard `d != s` is written wherever either place is reached through a pointer** (`*p = *q`, `*p = s`, `s
      = *p`, `p->t = s.t`; audit round 1, S2: `struct S *p = &s; *p = s;` is valid C, an exact overlap) **and not
      between two named objects** (structure locals, globals, a by-value parameter's copy): S47 (g) found that
      Silicon proves `block_of(d) != block_of(s)` for two distinct structure locals (and for a local against a
      `reads` parameter) from what the function holds, but does not decide a guard `d != s` between them from
      quantified permissions alone, so a guard there would leave a feasible branch that copies nothing (a false
      alarm, never a proof). Between two places reached through `owns` parameters of a structure whose leaves of the
      element's field are all array members, the guard is not decided by Silicon either, but on the branch where it
      is false `d == s` and every relation the copy states already holds, so it costs nothing (audit round 4b, N6:
      the feared false alarm does not reproduce). Under the guard, `d != s` gives `block_of(d) != block_of(s)`: both are
      first slots `loc(member(X, k), 0)` and `loc(member(Y, k), 0)` of array leaves at one index of one structure
      type, so `d != s` makes `X != Y`, `member_inv` makes the blocks differ.
    - **`copy_arr_<t>`** is a trusted method (`// cerastes:trusted-contract`), one per element cell type, as
      `zero_arr_<t>` is: it requires `1 <= n`, that `d` and `s` are the first slots of blocks of length `n`,
      `block_of(d) != block_of(s)` and every source flag set; it takes and returns full permission to the
      destination's value and flag fields over `n` slots and the amount `rd` (`none < rd && rd <= write`) of the
      source's value and flag fields — `rd` is the source's first-slot amount, 0060 item 6's minimum of the value and
      flag permissions at the first slot, so `write` when the caller owns the source whole (the audit's p2 verifies
      it; the S47 fixture passes half of it, which verifies too) — and ensures that each
      destination value is the source's old value at the same index, each destination flag is set, each source value
      is unchanged and **each source flag stays set** (S47 (g): without it the source's next read fails its
      `uninitialized` row).
    - **Disjointness is the copy's own `requires`,** so an omitted guard cannot give a proof: each `requires` of the
      call is preceded by its explicit row (0028 item 11) — the marked rows and the flags (`permission`,
      `uninitialized`) as above, and `block_of(d) != block_of(s)` as an `internal` row. That row can fail only where
      lowering wrote no guard for two places that may be one object: an exit 3, never a proof and never a user
      error. **It is the contract of the loop `d[k] =
      s[k]` over two disjoint ranges** (the model of 0028 item 2: disjoint blocks); S47 (g) asserts `false` after it
      and it fails. If S47 (g) does not pass, the copy is written element by element under question 295's limit, with
      no trusted method (question 297).
    - **By value.** A structure value is the tuple of its value-view leaves: an array member contributes one Viper
      formal, result or temporary per element (`p_s__k_m<j>_e<i>`). Everything 0044 items 2–5 say of a leaf holds for
      an element leaf: a parameter's copy block is written from the formals at entry; an object argument and `return
      s;` read every element (the base's rows, then each element's permission and flag); an `Aggregate` carries one
      argument per element (zero for the elements a literal does not write). `f().a[i]`, and `s.a[i]` of a by-value
      parameter in a contract, are `ElemValue`: the element at `i` (an `ite` chain over the member's elements) with one
      `out-of-bounds` obligation.
    - **The limit** (question 295, changed by question 309): a structure type whose value view holds more than **7**
      array elements is rejected as a by-value parameter, result, argument or literal value (`unsupported_type`, "…
      more than 7 array elements passed by value (array values: M4.5f3 …)"). A copy has no limit. The number is one
      constant, `vir::kMaxByValueArrayElements`, read by the encoder and by `check.cpp`'s backstop. **Why 7:** 64 did
      not verify (a 64-element structure taken and returned by value timed out at Silicon's 600 s cap); on the worst
      by-value shape (taken and returned by value, a caller passing an object and storing the result), measured alone
      on the box: 6 elements 19 s, 7 25 s, 8 35 s, 12 129 s, 16 over 120 s (about 1.4 times per element, not
      diagnosed); 7 is the largest count under about 30 s. The limit is per structure type: a call that passes several
      such structures by value can still time out, which is an exit 3, never a proof (listed).

14. **Computed array types** (amends 0024 item 6 for arrays of scalars).
    - A typedef or alias of an array type **declared in a verified file** is accepted when the type is item 2's, with
      its bound checked as written at the typedef; **one declared anywhere else** (a system header's `jmp_buf`,
      `va_list` where it is an array) has no bound this unit checks as written, and an object, a member or a type
      operand that reaches an array type through it stays rejected (`unsupported_type`, pinned); an array declared
      through a typedef, `typeof` or `decltype` is the array its type names, and its bound was checked where it was
      written (the typedef, the named array, or the `typeof`'s own type operand).
    - As the type operand of `sizeof` and `_Alignof`, and as the type of the operand of `sizeof` when that operand
      names an array (`g`, `s.a`, `p->a`; a local as since e1a), an array type is accepted when **every bound in it is
      read as written** with its obligations at that position; the value is then EDG's (the `internal` guard ties the
      bound). `sizeof(char[65536u * 65536u + 1])` is an `overflow` error at the bound, not a rejection. The same rule
      in static assertions (`scan_type`) and C++ enumerators.
    - **Still rejected:** an array of arrays or of structures and a pointer to an array inside such a type (M4.5e1e2,
      M8); the operand a compound literal or a string (M4.6); `__datasizeof`; `_Alignof` of an expression; `_BitInt`,
      vectors, class templates with constant arguments; type traits' arguments (M5.0); C's enumerator rule (0018 item
      10) first.
    - A bound that names a local constant (`const int N = 4; int a[N];`, C++ only: in C it is a VLA): the bound is
      `N`'s own checked constant. P82 (f): the array type's bound constant has no `expr`, but its `array-bound
      referrer` (a `local-expr-node-ref`) holds `var N`, and `N`'s initializer keeps its computed tree, so the
      bound is read through `N` (cut 3 is not taken).
    - **A system header's array typedef differs by host** (P82 (f)): `jmp_buf` is `int[48]` on macOS and `struct
      __jmp_buf_tag[1]` (an array of one structure) on Linux, `va_list` an array of a structure on Linux and no
      array on macOS. The system-typedef rule is tested before the element rule, so `jmp_buf b;` is rejected the
      same way on both hosts (`unsupported_declaration` at the declaration, measured on the box and the Mac with
      Part D's build), and one pin over the real `<setjmp.h>` covers it (question 301). A library header of the
      harness's library fixture is verified like user code (0046), so it counts as a verified file here: a typedef
      there is accepted with its bound checked.
    - **An unused block-scope typedef or alias of an array type stays rejected** (question 302): both compilers warn
      at `-Wall` (`-Wunused-local-typedef`, `-Wunused-local-typedefs`, both modes), a warning family with no rule of
      its own in cerastes (an unused `typedef int T;` is accepted, listed for M8's warning sweep), so by the plan's
      default for such a family it is rejected like the compilers (`unsupported_declaration` at the typedef, the
      diagnostic of 0024 item 6 kept, its message naming M8). "Unused" is read on the strict side: no declaration,
      cast or type operand of the function reaches an array type through it. `tests/scalar/negative/
      reject_typedef_array_block.c` therefore stays a pin (the plan's Flips A to Flips C).

15. **The lint** (amends 0012 item 11, 0043 item 13, 0059 item 12).
    - **The block grammar:** where a line has no local (item 5), a quantified row's block is `GB_<g>()` of a declared
      array global or `member(<chain>, k)` with `<chain>` 0043 item 13's canonical chain rooted at a formal, `r` or
      `loc(GB_<g>(), 0)`; the bounds `0` and the literal count the facts state. Nothing else. **In a method body
      the block-term form is refused:** a row in code is over a bound local (item 5).
    - **The bound local** is e1a's array local for the lint: a `Ref` local assigned once, after its root, exactly
      `loc(GB_<g>(), 0)` or `loc(member(<chain>, k), 0)`, whose rows are e1a's and e1b's with the block
      `block_of(t)` and the literal bounds `0` and the count the block's length fact states (S47 wrote them so;
      every other start than `offset_of(t)` or `0` for such a local, a local bound to another slot, or a block
      outside the grammar is refused). The chain's root may be a formal, `r`, `loc(GB_<g>(), 0)`, a structure local's
      Viper local (assigned by `alloc_struct_*`; its count from the allocation's `len`), and a pointer temporary
      (`call_atoms`' bound argument, a load for 0045's places) whose length fact the method states (audit round 1,
      S4 (2)). **As settled with Part C's lowering:** a member's bound local is a temporary (`t__<n>`; the lint keys on its assignment, not its name), assigned once, directly
      `loc(member(<chain>, k), 0)` with `<chain>` rooted at a Viper variable only (a formal, `r`, a structure local, or
      a call-row temporary bound from the argument first; never an expression); its count is the literal of the
      member at that index in an `alloc_struct_*` of a structure with that member (emitted for every such structure,
      the next bullet), the same literal `struct_lens` states. An element access (no quantifier) is written over the block term directly (`len(member(X, k))`, `loc(member(X, k), i)`). A `reads` site with array parts binds its call amount to a `Perm` local, the minimum over its cell leaves, each array part's first slot, the liveness cell and the method's own read amounts (0060 item 6's rule). `owns(p->a)` and `reads(p->a)` stay plain atoms whose
      pointee is the array (a structure-like site with no cell leaf and one array part), not range atoms: the range
      machinery is keyed by a formal, the member needs its root and path. The audit found the
      rule not needed for 0059 item 7's single reason (S4 (1): any `block_of` of a Viper local with literal bounds and
      a heap-free amount reads no heap; a wrong binding gives a false alarm or an exit 3, never a proof); it pins the
      shape. The S47 fixtures write some rows in code (call and return rows among them, S4 (4)) in the block-term form,
      rows over a copy of a bound local, and element flags directly; those are fixture differences (the chapter lists
      them), not shapes lowering writes.
    - **The count's source** (audit round 1, S5): lowering emits `alloc_struct_<n>_<tag>()` for every structure type
      with an array member, called or not (a trusted method nobody calls, as `program_start` is in a unit without
      `main`), so the lint's count of every array member always has its pinned source.
    - `alloc_struct_*`: per array member exactly `len(member(X, k)) == N` and the three quantifiers of item 6, each
      (receiver, field) once. `struct_lens`' conjunct: `== N` for an array member, with the count of the allocation of
      that structure. The global facts: `len(GB_g()) == N` for an array global, the same N in all three places the
      facts are pinned.
    - `program_start`: an array global's row group exactly as item 9 orders it (a duplicate group, a missing zero
      rest, a value conjunct for a slot at or past the listed prefix are refused); a structure global's array leaf
      likewise.
    - `copy_arr_<t>` and `zero_arr_<t>` are trusted methods pinned by their clause sets; with `alloc_*` they are
      exempt from "first `requires` is the global facts" (a defect found by reading at the planning and fixed here:
      a program with a global and a zero-filled local array was refused by the lint; Step 0's R4).
    - A flag write to an element of a global array or of a member goes through a `Ref` temporary (`lint_flags`
      unchanged).
    - Each rule with a refusing twin in `scripts/test_lint_viper.py`.

16. **Categories and tags:** none new. **Facts:** P82, S47 (`internal/il-inventory.md`, chapter M4.5e1e1).

## Facts and the branches taken

Each probe and spike of the plan's section 6, its outcome, the branch of its decision table, and the item it
revised. (To be filled from P82 and S47.)

| Row | Outcome | Branch taken | Item revised |
|---|---|---|---|
| P82 (a) global and `static` arrays | e1a's `tk_array` entry; `initk_static` over a `ck_aggregate`, trailing zeros absent; an `extern` declaration and its definition one variable; `static` locals through `local-static-variable-init`; P38's loss of `expr` from 1,002 elements, only for elements under an implicit conversion; the decay folded into a `ck_address` (offset 0, no path, `expr` the decay) in arguments, initializers and C's `&g[i]` base; `&g[k]` and `g + k` carry a path | Expected (refined: the 1,002 threshold; the decay constant) | 5, 9 |
| P82 (b) array members | the field a `tk_array` entry; `s.a[i]` `subscript(array_to_pointer(dot_field(s, a)), i)`, `p->a[i]` over `points_to_field`, nested over `dot_field` chains; `sizeof(s.a)` a constant keeping `enk_sizeof`; C++'s class tests pass (trivial implicit `operator=`); **C++ non-local bounds are `unsigned long` under a generated cast** | Expected (refined: the C++ bound cast) | 2 |
| P82 (c) initializers with an array member | nested `ck_aggregate`s; elision has the nested IL too and is told only by the inner aggregate's `explicit_braces_on_aggregate`; `{0}`'s inner has no such flag; array designators are `ck_designator` with `array_element`; C++ adds a leading placeholder in designated lists; an excess inner element is an error in both modes | Expected | 6 |
| P82 (d) structure values | `eok_assign` and `dik_expression` copies, never a call or an element-wise expansion; 0044's argument and result shapes; `mk().a[1]` a subscript over a decayed field of the call | Expected | 13 |
| P82 (e) array compound literals | `array_to_pointer(enk_temp_init)` with `is_compound_literal`; e1a's aggregate shapes; C wraps a `while` in a block, C++ does not; **C++20 accepts them silently** | Expected (C++: the encoder rejects) | 12 |
| P82 (f) computed types | a typedef is a `tk_typeref` over the array entry with its bound's `expr`; `typeof` and `decltype` add a typeref over the same entry; a type operand's array entry is reachable from the `sizeof` node; a local-constant bound's referrer holds `var N` and `N`'s initializer keeps its tree; `jmp_buf` is `int[48]` on macOS and an array of one structure on Linux | Expected; cut 3 not taken; the system-typedef pin through a test library header | 14 |
| P82 (g) the compiler matrix M (Step 0) | clang 21 and GCC 16, both modes, three tiers, 1,884 compiles on the box (the Mac's identical): every positive shape compiles with `-Wall -Wextra -Werror`; EDG's #175 fires on exactly clang's constant-index set; `array_address_null_test` covers the truth tests and null comparisons; elision warns at `-Wall` (`-Wmissing-braces`), clang++ also on `{0}` over an array member in an aggregate of two or more members; an unused block-scope typedef warns at `-Wall` in both compilers | the rule; questions 302 and 303 | 5, 6, 14 |
| S47 (a) the bound local | the structure's member chunks from `alloc_struct` in the block-term form, then e1a's rows and the scope exit's `exhale` over `loc(block_of(t), k)` with `t := loc(member(s, 1), 0)` verify; the global variant too; each twin one error; 5 fixtures, 6.6–7.6 s on the box | Expected: the bound local in code, the block-term form only where no local exists; the lint's `forall k` rules unchanged | 5 |
| S47 (b) a global array | `program_start`'s row group, the facts with N, `main`'s sum and fill loops, `owns(g)` and `reads(g)` callees, `g` to `owns_range`; every twin one error; 8 fixtures, 6.4–8.7 s | Expected | 3, 7–10 |
| S47 (c) an array member through a pointer | `owns(*p)` with the array leaf in the block-term form, `p->a[i]` and `p->n`, a caller with a structure local, `reads(*p)`; the alias twin `(&s, &s.a[1])` fails at the per-slot sum row; `WF_S` without the member's length fails at the bound row (the length is needed); 7 fixtures, 7.5–12.8 s | Expected | 3, 6–8 |
| S47 (d) loops over a member's array | a structure local's array leaf as an array place beside its cell places, `p->a` under `owns(*p)`; one liveness row per root shared by cell and array places; with a store through an `int *` in the body the values-kept conjunct must be absent (with it, the invariant fails to be preserved); 4 fixtures, 8.7–10.7 s | Expected | 10 |
| S47 (e) depth | `t.s.a[i]` through a pointer to `T`, the chain `loc(member(loc(member(p, 0), 0), 0), i)`; `alloc_struct` with the slot fact `r != loc(member(r, 0), 0)`; 2 fixtures, 8.8 s | Expected (cut 5 not taken) | 3, 6 |
| S47 (f) an array literal | a hidden array local of its block, `zero_arr` and the element stores inside the expression, the decay as the value; the twin reads past it; 2 fixtures, 7.5–8.1 s | Expected (cut 4 not taken) | 12 |
| S47 (g) the trusted copy | `copy_arr_int` between two structure locals, from `*p` at a read amount, the guarded self-copy; the vacuity twin after the call fails; the source flags must be kept by its `ensures`; `block_of(d) != block_of(s)` is proved for distinct locals with no guard, while a guard `d != s` between them is not decided from quantified permissions; 7 fixtures, 7.9–13 s (a generator typo in the destination row was found by the first run, the positive failing, and fixed) | Expected (the trusted copy; no element-wise fallback, question 297 not needed) | 13 |
| S47 (h) cost | two array members with a loop over each 17.7 s; a global array of 1,000 elements with 16 listed values 5.6 s; S45 (k)'s scale program at K = 16 with one array member 42.5 s (the box) | recorded (under the table's 60 s line) | — |
| S47 (x) for M4.5e1e2 | (x1) `owns_range` over `struct P { int x; int y; }`, one quantifier per leaf with the trigger `{ loc(member(loc(block_of(p), k), j), 0) }`, loop conjuncts per leaf, a caller with an array of structures: verified 9.2–9.9 s, the twin fails; (x2) `int m[2][3]` with a two-variable permission `{ loc(member(loc(block_of(m), i), 0), j) }` and nested loops: verified 8.9 s, the twin and the vacuity twin fail; no `qp.not.injective` | recorded only | — |
| Step 0 R1–R4 | R1: all 50 pins of the plan's section 10 rejected, no Flips D pin changed; R2: nothing of item 1 verified before this record, nine staging pins added; R3: 2,572 rejection runs recorded; R4: the lint refused a program with a global and a zero-filled local array (no test combined them), fixed | the plan's rule; R4's defect fixed (item 15) | 15 |

## Consequences

- C programs that keep arrays of scalars in global and `static` storage, in structures and in compound literals
  verify, with whole-array ownership in contracts, element reads in contracts and loops that carry them; array
  typedefs and `sizeof` of array types are checked as written.
- Every structure with an array member costs a quantified permission per array leaf in its allocation, scope exit,
  atoms and loops; a structure value with array members costs one Viper value per element at every call and return.
- Ranges on part of a global array or of an array member, element atoms and pointer arithmetic wait for M4.5e2;
  arrays of structures and of arrays for M4.5e1e2.
- Programs accepted before this record keep their Viper and maps byte for byte.
