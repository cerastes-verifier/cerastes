# 0064 — Arrays of structures and arrays of arrays (M4.5e1e2)

- **Status:** draft (2026-11-25, M4.5e1e2), written before code (invariant 6) from section 7 of the plan
  (`.claude/plans/m4-5e1e2-arrays-of-structures.md`, written at a session that only planned and executed without
  re-planning). To be revised by P83's and S48's decision tables (the plan's section 6; `internal/il-inventory.md`,
  chapter M4.5e1e2; the fixtures `tests/viper-smoke/memory/s48_*.vpr`; the table at the end) and by audit round 1.
  Owner decisions:
  - At M4.5e1e2's planning (2026-11-25): **310** one complete plan run under the 12-hour rule, in the order arrays
    of structures (code, then contracts and storage), the leftovers, arrays of arrays, quantifiers; the parts a
    session does not begin run from the same plan in a second session, and nothing is cut to hold a session short;
    **311** the full check ran green on M4.5e1e1; **312** question 304's rejection is built here (item 16);
    **304** (inline, 2026-11-25): "Follow the rejection like the compiler warning."
  - Defaults the plan builds on, for the owner's review (each fail closed; `internal/handoff.md` §8): **313** an
    array or a range whose element structure has an `owning` or marked member is rejected (M4.8); **314** no leaf
    path deeper than two index steps, depth 3 only if S48 (m) verifies it under 30 s a program; **315** a pointer
    to an array is a parameter's type only (M8 elsewhere); **316** `restrict` backed by a range over structures is
    rejected (M4.8); **317** a whole-structure copy of a structure whose member is an array of structures is
    written leaf by leaf under the by-value limit of 7 and rejected beyond it (M4.5f3); **318** a store to `v[i].x`
    in a loop reaches every place of its field (a listed false alarm, M4.8); **319** the tutorial section "Arrays
    in aggregates" is written by the session that lands Part C and extended by the one that lands Part E.
  - From M4.5e1e's planning and M4.5e1e1: 290–300 and 301–309 as built (record 0063); 292–294 and 306 are lifted
    here (item 15); 298 puts the tutorial section here.
  - Standing: 69 (no EDG work), 77 (one context, under about 12 h), 160 (a new test's expect line corrected after
    its first run, listed in status), 176 (the owner handles CI), 277 (a twin with several real errors lists every
    one), 285 and 299 (a test slow on the box only because of what it must refute goes to `tests/long/` with a
    short twin).
  - Questions of this session (`internal/handoff.md` §8): **320 is open for the owner** (the parameter spelling
    `T (*a)[M]`, item 13); settled with the advisor: **321** (item 16 is the measured rule of both compilers),
    **322** (the address tests of item 5; the scalar-array forms go to the owner), **323** (the depth-2 copy is
    built; its time is recorded, item 12).
- **Milestone:** M4.5e1e2.
- **Amends:** 0012 item 11 (the lint); 0024 item 6 (array types of aggregates); 0028 items 2 and 16 (as built); 0043
items 6, 7, 9 and 11; 0044 items 2–5 and 8; 0045 items 1 and 12; 0047 items 2, 4, 6 and 8; 0052 item 2; 0055
items 2, 3 and 5; 0058 item 4; 0059 items 1–4, 6, 8 and 9; 0060 items 1, 3–5, 8, 9, 11–13 and 15; 0061 items 1, 3
and 6; 0062 item 4; 0063 items 1, 2, 4–10 and 13–15.

## Context

Record 0063 (M4.5e1e1) gave arrays of scalars every storage: globals, `static` locals, structure members, compound
literals, structure values. An array whose element is a structure or an array is still rejected with a diagnostic
that names M4.5e1e2, and six rules for a structure with an array member were left to this record (questions
292–294 and 306, partnerless atoms, written atoms). Record 0028 items 2 and 16 fixed the layout in M4.5a (element
`i` of an array is the slot `loc(B, i)`; a structure element's member `k` is the block `member(loc(B, i), k)`; row
`i` of `int m[2][3]` is `member(loc(B, i), 0)`), and M4.5e1e1's spike S47 (x) verified its two unknown shapes on
the box: one quantifier per leaf over an array of structures (9.2–9.9 s) and a permission over two bound variables
for a matrix (8.7–8.9 s).

Facts: P83 (the IL of arrays of structures and of arrays, their initializers, array-of-rows parameters, members,
globals, quantifier bodies and arguments, and the compiler matrix M) and S48 (Silicon on the shapes of items 3–15),
in `internal/il-inventory.md`, chapter M4.5e1e2; the table at the end maps each outcome to the branch of the
plan's decision table it took.

## Decision

**Terms.** A **scalar** is `bool`, an integer type, an enumeration or a pointer to an object type. A **verified
structure** is 0043 item 2's, with the array members 0063 gave it. An **aggregate element** is an array element
that is a verified structure or an array. A **leaf path** runs from an object's first slot to one scalar cell by
**index steps** (`[i]`, with a count) and **member steps** (`.k`); between two index steps stands the member
step 0 that 0028 item 16 gives a row. Its **depth** is its number of index steps: 0063 accepted depth 1 with the
index step last (`s.a[i]`, `g[i]`); this record accepts index steps anywhere, to depth 2.

**No existing program's Viper or map changes in this session.** Every item adds rows only for a program that
holds a construct rejected today; where an existing function is touched, the text it writes for a program
without the new constructs is the same, and `vprdiff` shows it after every part. **Accepted forms that become rejections:**
item 16's measured elision rule (the owner's answer to question 304, as measured: question 321), in C++ and, for
nested lists, in C. Step 0 found no existing test or doc example whose outcome it changes; one that does at Part
B's joined proof goes to the owner before it is touched (the plan's Flips G).

1. **Scope.**
   - **In:**
     - **(S) Arrays of structures.** In code (the plan's Part B): an automatic local `struct S v[N]`; `v[i].x`, `v[i].t.x`,
       `p[i].x` on any `struct S *`, `&v[i]`, `&v[i].x`, the decay of `v`; an element as a structure value
       (`t = v[i]`, `v[i] = t`, `v[i] = v[j]`, `f(v[i])`, `struct S t = v[i]`); braced initializers with a nested
       list per element and C's zero fill; scope exits; loops; sequencing; `sizeof(v)` (an element structure that
       itself has an array member is depth 2 and waits for Part E). In contracts and storage
       (Part C): `owns_range(p, n)` and `reads_range(p, n)` with `p` a pointer parameter to a verified structure;
       element reads `p[e].x` in `requires`, `ensures`, `decreases` and under `old`; calls, returns, `f__wf`; loops
       over a parameter's range; `owning(n)` on such a parameter; P4222's markers over such a range; global and
       `static` local arrays of structures with their initial values, `owns(g)` and `reads(g)`; an array member
       whose elements are structures (`t.v[i].x`, `owns(*p)`, `owns(p->v)`), in every storage of its structure.
     - **(H) The leftovers of a structure with an array member** (Part D): heap storage (`malloc(sizeof(struct
       S))`, `calloc`, `free`, `owns_block`; question 292); an `owning` or marked member beside an array member
       (293); `ref_to_uninit` and `must_init` on a pointer to such a structure (294); contract places through a
       heap read (`owns(*p->q)`, `p->q->a[i]`; 306); an `ensures` atom with no `requires` partner; a written atom
       in a loop invariant on such a place.
     - **(A) Arrays of arrays** (Part E): `T m[N][M]` as a local, a global, a `static` local and a member;
       `m[i][j]`, `&m[i][j]`, a row `m[i]` used as a pointer; `sizeof(m)`, `sizeof(m[0])`, `sizeof(int[2][3])`,
       typedefs of such types; nested braced initializers; **array-of-rows parameters** (`int a[][3]`, `int
       a[2][3]`, and the same type spelled `int (*a)[3]`) with `owns_range(a, n)` over rows, `a[i][j]`, and `m`
       passed to one; nested loops; and the other depth-2 path, an array of structures whose member is an array
       (`v[i].a[j]`, `p[i].a[j]`).
     - **(Q) Quantifiers and `pure` applications** (Part F): under a binder, a member of a structure element
       (`p[i].x`, `v[i].t.x`); as the pointer argument of `cerastes_forall` and of a `pure` application, the decay
       of an array member (`p->a`, `s.a`) and a named global array in a contract.
     - **(I) Question 304's rejection, as measured** (item 16; Part B).
     - **The pages, and the tutorial section "Arrays in aggregates"** (question 298).
   - **Not here** (rejected, each pinned, the message naming where it is lifted):

   | Construct | Lifted by |
   |---|---|
   | an array or a range whose element structure has an `owning` or marked member; an array of owning pointers; `owning`, `ref_to_uninit` or `uninit` on an array of aggregates; `restrict` backed by a range over structures | M4.8 |
   | `malloc(n * sizeof(struct S))`, `calloc(n, sizeof(struct S))`, `realloc`; a range on part of an array (`owns_range(&v[1], n)`, `owns_range(g, n)`, `owns_range(p->v, n)`, a row `m[i]` as a range's base in a contract); an atom on one element (`owns(p[i])`, `owns(v[i].x)`); `&p[i]`, `&p[i].x` and `&m[i][j]` in a contract; a range whose base is not a pointer parameter; a range atom in a loop invariant; `p + i` | M4.5e2 |
   | under a binder: a second index (`p[i].a[j]`, `m[i][j]`), a row `m[r]` or `&v[1]` as the pointer argument; a memory-reading argument (`p->q->a`) | M4.5e2 |
   | a `forall` inside a `forall`; the positions 0061 item 1 leaves to M4.8 | M4.8 |
   | a leaf path of depth 3 or more (`int c[2][3][4]`, an array of structures whose member is a matrix), unless S48 (m) admits depth 3 | M8 |
   | a parameter written as a pointer to an array (`int (*a)[3]`): pending question 320, rejected until the owner answers | the owner (question 320); M8 otherwise |
   | a pointer to an array anywhere but a parameter (`int (*p)[3]` as a local, a member, a result; `&m[i]`, `&a`; a cast to one); `sizeof(char(*)[4])` | M8 |
   | a structure value that holds more than 7 array-element leaves, passed or returned by value (0063 item 13; a copy has no limit); an array as a value | M4.5f3 |
   | a read `pure` function over a structure or a global (0062 item 1) | M4.5f1 |
   | `const` arrays in any storage; string-initialized arrays | M4.6 |
   | array designators, VLAs, zero-length and flexible members, `[static N]`, `thread_local` and block-scope `extern` arrays, C++ array literals, `_Alignof` of an expression | M8 |

2. **Which arrays** (amends 0059 item 2, 0063 item 2). The element is a scalar that 0059 item 2 accepts, a
   verified structure, or an array of these; **no leaf path is deeper than 2** (3 if S48 (m) admits it). The
   `const` rule is tested first (M4.6), then: an element structure with an `owning` or marked member at any
   depth is rejected (`unsupported_type`, M4.8). **Every bound is read as written** (0063 item 2's rule, each
   dimension on its own: a prologue constant for a global, a member and a file-scope typedef, a constant of its
   function otherwise; "the bound of `m`", "the second bound of `m`"). `owning`, `ref_to_uninit` and `uninit` on
   an array of aggregates are rejected as on any array.
3. **Layout** (0028 items 2 and 16, as decided in M4.5a).
   - Element `i` of the array block `B` is the slot `loc(B, i)`. For a structure element, its member `k` is the
     block `member(loc(B, i), k)`, exactly as 0043 item 3 and 0063 item 3 lay out a structure at any slot. For an
     array element (a row), the row is the block `member(loc(B, i), 0)` of the row's length.
   - **The length facts of a range of slots:** `forall k: Int :: { loc(B, k) } lo <= k && k < hi ==>` the
     conjunction, for the element at `loc(B, k)`, of what `struct_lens` states of one structure slot (`len(member(X,
     j)) == 1`, or the count for an array member; a nested structure's own facts; 0043 item 8's slot facts), or
     `len(member(loc(B, k), 0)) == M` for a row. They are stated by an allocation (`alloc_arr_<E>`), by
     `program_start` and the global facts, by `struct_lens` for an array member of aggregates, and by the first
     line of a range atom over structures or rows. They hold in 0028 item 2's model (`len` is free; a member
     block is a fresh term) and are hypotheses discharged by an allocation, by `program_start` or by a caller,
     never axioms.
   - Every member block, at any depth, has its root block's liveness cell (`member_lv`): one liveness cell per
     root, as before.
4. **VIR** (amends 0063 item 4).
   - `Type::Array`'s element may be a `Struct` or an `Array`. `Ptr(Array)` is a type of a **parameter** only (item
     13), never of a local, a member, a result or a cell.
   - **Leaf paths.** `object_leaf_paths(type)` gives every leaf of an object type as its steps (member `k`; index,
     with its count) and its cell type, in declaration order, one path per leaf whatever its counts. For a type
     0063 accepted it is `struct_object_leaves`' list (a cell: member steps only; an array leaf: member steps,
     then one index step), and **every site that 0063 gave rows writes the same text for those types**. **The
     poison rule continues:** a walk of leaves that has not been given the general path reports an internal
     error on a path it does not handle (an index step that is not last; two index steps) — exit 3, never a cell
     read as something else — `check.cpp` refuses an array of aggregates in every position a part has not
     lifted, and the encoder rejects it there first with a diagnostic at the C line (invariants 3 and 7).
   - **The traps are closed before any gate is lifted.** The planning's survey found six places where lowering
     or the checker would treat an aggregate element wrongly without an error (a nested `ElemAddr` as a flat
     pointer; element keys that ignore an index; a range over structures with no field, so a callee's writes go
     unrecorded; walks that count an array by its element; nest walkers that skip elements; a structure element
     compared as one field), each masked only by a refusal upstream, and three positions only the encoder
     refuses. Part A makes each an internal error or gives it the general path, closes the three positions in
     `check.cpp`, and pins every guard with a self-test case (the plan's section 8, Part A's table).
   - **Places.** `ElemAddr`'s base may be a pointer to a structure and the address of an array of aggregates.
     Its result for a structure element is that element's address (a `struct S *` value: `&v[i]`); for a row it
     is the row's address, accepted only as the base of another `ElemAddr` and as the row's decay.
     `MemberAddr` is accepted over an `ElemAddr`. `place_root` steps through an `ElemAddr`; places are spelled as
     written (`p[i].x`, `m[i][j]`, `v[i].a[j]`), never `(&p[i])->x`.
   - `Var::zero_filled` and `ArrayZero` hold for an array of aggregates (every leaf of every unwritten element);
     `ArrayCopy` is unchanged (an array leaf of scalars inside one structure, wherever that structure stands);
     the value view expands every index step (item 12).
   - `check.cpp` backstops each rule of items 2–16.
5. **Rows in code** (amends 0059 item 4, 0063 item 5).
   - **An element access** takes 0059 item 4's rows at each index step, in source order, then the cell's.
     `v[i].x`: the bound of `i` against `len(block_of(v))` (`out-of-bounds`: "reads `v[i].x`, whose index may be
     outside `v`"), then `permission` and, for a read, `uninitialized` on the leaf cell. `m[i][j]`: the first
     bound, then `j` against the row's length ("… whose second index may be outside `m[i]`"), then the cell's
     rows. A named array is a named base (no null or liveness row). `p[i].x` on a pointer takes e1a's rows for
     `p[i]` (`null-dereference`, `p`'s liveness, the bound) and then the cell's. **A member step adds no row,**
     but the slot it steps from must exist: `&v[i].x`, `v[i].x` and `m[i][j]` need `i` strictly inside, while
     `&v[i]` and `&m[i][j]` as values allow one past the end at their last index (0059 item 10).
   - **A row used as a pointer** (`m[i]` as an argument or an initializer) is the row's decay: the bound of `i`,
     strictly inside, then a pointer to the row's element 0, a valid `T *` by the length facts.
   - **The decay of a global array of aggregates as a constant** (P83 (a), extending 0063 item 5): in C the
     front end folds the decay into a `ck_address` constant in every rvalue position of a declaration's
     initializer, and in both modes when it is a whole call argument; never on either side of an assignment and
     never in an annotation. The constant has the decay's type (`int (*)[3]` for a matrix) and is read from its
     `expr`, the written `array_to_pointer`, as 0063 item 5 reads it.
   - **The bound local** (0063 item 5). A row over an array's slots in code is written over a Viper local bound
     to the array's first slot, with the leaf's receiver built from it: `loc(member(loc(block_of(t), k), j), 0)`
     for a cell leaf of an array of structures (a longer member chain for a nested structure), and over two
     bound variables for a depth-2 leaf, `loc(member(loc(block_of(t), k), j), l)`. Each permission is its own
     quantifier with its own receiver as the trigger (0028 item 2: one quantifier holding two members'
     permissions fails Silicon's injectivity check). A marked row still reads no heap (0059 item 7).
   - **Rejected like the compilers** (P83 (h)'s matrix): a constant index outside any dimension (EDG's #175,
     which fires on every such form measured; `&m[2][0]` and `&m[1][3]` draw no front-end error and take the
     path-precise rows above). **A truth test or a null comparison of a row or of the address of an element of
     an array of aggregates** (`if (m[0])`, `!m[0]`, `m[0] == NULL`, `if (&v[1])`, `&v[1] == NULL`, `if
     (&m[1][1])`; `&v[1].x == NULL`), `array_address_null_test`: GCC warns `-Waddress` at `-Wall` on each (clang
     only on the name's and a member array's decay, which the tag already covers). The rule is built from the
     matrix's cells (question 322): a form enters it when a compiler flags it without a flag or at `-Wall`;
     the pointer-based forms (`&p[1] == NULL` on a `struct P *p`) enter only if their cells, measured before
     Part B's flips, are flagged. **The same tests on an element address of an array of scalars** (`if
     (&a[1])`, `&a[1] == NULL`), which GCC flags too and the base accepts, **are unchanged here and are the
     owner's** (question 322: rejecting them changes programs accepted before this record; not unsoundness).
6. **Local arrays of aggregates** (amends 0059 items 3 and 6, 0043 item 6, 0063 item 6).
   - **Allocation:** one trusted method per element type, `alloc_arr_<E>(n)` (`<E>` the structure's tag or the
     row's type), in `alloc_arr_<t>`'s shape: the block's length and liveness facts, item 3's length facts, and
     per leaf path full permission to the value field and to the flag field (each its own quantifier) with every
     flag false. `zero_arr_<E>` is `zero_arr_<t>` per leaf. Each (receiver, field) pair is given once.
   - **Scope exit:** per leaf path the two marked rows over the bound local (`lifetime`: "the address of an
     element of `v` may still be in use when its lifetime ends"), then the `exhale`s.
   - **Initializers:** each element takes a nested braced list in its own rules (0043 item 6 for a structure,
     0059 for a row); unwritten elements and leaves are zero (`zero_arr_<E>` first, then the written leaves'
     stores); in C every element of the whole list is evaluated before the first store; elision of an element's
     braces is rejected like the compilers except `{}` and C's `{0}`, and **C++'s `{0}` follows item 16**; an
     element that reads, writes or takes the address of the array being initialized is rejected.
7. **Contracts** (amends 0060 items 1 and 3–5, 0063 item 7).
   - **A range atom over structures or rows.** `owns_range(p, n)` and `reads_range(p, n)` with `p` a pointer
     parameter to a verified structure (or to a row, item 13): 0060 item 4's first line with item 3's length
     facts over `[offset_of(p), offset_of(p) + n)`; then, **per leaf path of the element,** 0060 item 4's two
     permissions, flags and type range, each a quantifier over the absolute slot with the leaf's receiver as its
     trigger (`{ loc(member(loc(block_of(p), k), j), 0) }`; two bound variables for a depth-2 leaf); one liveness
     fraction. Partners, the guard outside each quantifier and the ghost amount are 0060 items 3 and 4's.
   - **Element reads** `p[e].x`, `p[e].t.x`, `p[e].a[e2]` and `g[e].x` in `requires`, `ensures`, a function's
     `decreases` and under `old`, the indexes reading no memory. A read is framed by a range atom when **its
     element slot** lies in the range (0060 item 5's disjunct on `&p[e]`; the member steps then name a leaf the
     atom holds), and by a structure atom whose place is that slot.
   - `owns(g)` of a global array of aggregates is the range atom over the whole array (as 0063 built it for
     scalars); `owns(*p)` of a structure with an array member of aggregates takes that member's leaf paths with
     its other leaves; `owns(p->v)` is that member alone.
   - **Overlap is by slots** (0043 item 9 and 0063 item 7, the soundness core). A leaf path of a site is a set
     of slots; every rule that pairs two sites (the exclusion of `owns` and `reads`, the amount sums at calls and
     returns, `f__wf`'s distinctness and equal-ghost facts, framing disjuncts, loop places) pairs a leaf path
     with a cell, an array leaf or another leaf path of the same field by those slots. So `f(v, 3, &v[1].x)`
     with `owns_range(p, n) && owns(*q)` fails at the call (`permission`), and so does `f(v, &v[1])` with
     `owns_range(p, 3) && owns(*q)` over structures.
   - **The pairing rows** (S48 (c) decided between the two forms: **Form P is built for every pairing**; both
     forms passed every pair and no alias twin verified in either). **Form P:** a
     pure membership row, as 0063 item 7's interval row is: against a cell `q`, with `X` standing for
     `offset_of(parent_of(block_of(q)))`, `!(g && lo <= X && X < hi && q == loc(member(loc(block_of(t), X), j),
     0))` (the receiver term written out, so the permission of that slot is in view). **Form S:** the sum form
     of 0060 items 8 and 9, one marked `acc` row holding both sites' permissions (`permission`). Form S is
     not built for a pairing in which either site is an `owns`; two `reads` sites that may meet keep the sum
     form of 0060 items 8 and 9 and 0063 item 7, which checks their amounts and separates nothing. Never a bare
     block comparison, never `perm` under a quantifier. A row Silicon cannot decide is a false `permission` or
     an exit 3, never a proof.
   - **Amounts** (0060 item 6): a `reads_range`'s amount is the minimum over each leaf path's first slot, the
     liveness cell and the caller's own read amounts of those fields (question 238's listed limit applies).
   - **Rejected** (`unsupported_annotation`): item 1's M4.5e2 rows; `restrict` backed by a range over
     structures; two atoms on one parameter that overlap (`owns_range(p, n) && owns(*p)`).
8. **Calls and returns** (amends 0060 items 8 and 9, 0063 item 8). A range atom's rows are per leaf path: the
   shape row once (with the length facts), then e1b's marked rows (value, then flag) and its flag and range rows
   for each leaf, over the bound argument; the messages in source terms ("calls `f`, which needs `p[0..n)`: the
   member `x` of every element"). An array of aggregates passed to a range parameter is bound to a local first.
9. **`program_start`, initial values, `static` locals** (amends 0063 item 9, 0044 item 8). A global array of
   aggregates has, per leaf path, 0063 item 9's row group (the two quantified permissions, the flags, the
   values, once the liveness cell) and item 3's length facts in the global facts. **The values, known:** one
   conjunct per leaf of each element the list writes (the written constant, or zero for a leaf the element's
   list leaves out), and per leaf path one quantifier for the elements the list does not reach. **Unknown** (by
   0041 item 3's linkage rule and 0063 item 9's two escape directions, per leaf): the type range as one
   quantifier per leaf. Every element constant is read through `constant_value` (P38; P83 (f): the loss of `expr`
   is keyed to the outermost list, so a list of 1,002 elements or more loses it for every leaf below it, whatever
   the nesting, and such a leaf under an implicit conversion is rejected by `constant_value`'s own rule). A `static` local array of
   aggregates is such a global with its implicit atom.
10. **Loops** (amends 0059 item 8, 0060 item 11, 0063 item 10). An array of aggregates the function holds is,
    **per leaf path,** an array place (a local, a global in `main`) or a range place (a parameter's range, a
    held global elsewhere, a held structure place's member), with 0059 item 8's entry rows, conjuncts and
    back-edge rows over the bound local; one liveness conjunct per root (S47's F5). **Reach** follows the rules
    in force: a store by name to a leaf of a global array the function holds reaches that leaf path only (0044
    item 8); a store to a member of an element of a local array, any store through a pointer of the leaf's
    field, and a call whose `requires` owns a cell of that field reach every place of the field (0043 item 11).
    **Listed false alarm:** a loop that writes `v[i].x` cannot keep `v[k].y` by the automatic conjunct when
    both are of one field; a written quantified invariant keeps it (Part F), and the precise rule is M4.8's.
11. **Sequencing** (0039 item 3, 0059 item 9, 0063 item 11). Object keys gain the index steps: `v[0].x` and
    `v[1].x`, and `v[i].x` and `v[i].y`, are different objects; two unsequenced accesses to the same leaf path
    of one array, one a write, with indexes that are not all constants, get the path-precise `unsequenced` row;
    the same leaf named twice with a write is `unsequenced_modification`.
12. **Structure values and elements** (amends 0043 item 7, 0044 items 2–5, 0063 item 13). An element `v[i]` or
    `p[i]` is a structure place: a copy's source and destination, a by-value argument, an initializer's source,
    with 0043 item 7's and 0063 item 13's rows, after the index's own rows. Cell leaves are copied by leaf
    stores, which need no guard (`v[i] = v[i]` rewrites each value). **For an element structure's array leaves
    (Part E), `copy_arr_<t>`'s guard `d != s` is written where the two places can be one object:** a place
    reached through a pointer (0063 item 13) and two elements of one array (`v[i] = v[j]`); **never between two
    named objects or between an element and another named object**, where Silicon would leave the guard's
    branch open and skip the copy (S47's F2). A structure whose
    member is an array of structures is a value by its value view: every leaf of every element is one Viper
    value, and `kMaxByValueArrayElements` (7) counts those leaves; a whole-structure copy of it is leaf stores
    and `copy_arr_<t>` calls per element — **rejected beyond 7 such leaves until M4.5f3** (no trusted
    `copy_arr_<E>` is added here). An array is never a value. **S48 (g):** the guard between two elements of one
    array is decided by Silicon in both directions and is needed (without it the copy's `requires` fails once);
    the copy `v[i] = v[j]` of `struct W { int a[4]; int n; }` elements verified in 34.5 s and 35.2 s on the
    box, over the spike's 30 s line: it is built (question 323), and Part E's tests of it are timed under the
    test budget's rule 3.
13. **Arrays of arrays** (amends 0060 item 15, 0063 item 14). A row is an array object in its own block; a
    matrix's leaf path has two index steps, and so has an array member of an element structure (`v[i].a[j]`).
    - **Array-of-rows parameters.** `T a[][M]`, `T a[N][M]` and `T (*a)[M]` declare a parameter of type pointer
      to a row. **The spelling `T (*a)[M]` is pending question 320 and stays rejected until the owner
      answers:** `tests/memory/negative/array_reject_pointer_to_array.c` pins exactly that parameter as
      rejected (0059 item 1, M8), the plan lists that pin as unchanged while its scope accepts the spelling,
      and flipping it is the owner's (invariant 1). The interim rule is fail closed: **a parameter is accepted
      only when its written type (the parameter type's `declared_type`, P83 (d)) is an array declarator**;
      a spelling through a typedef (`row *a`, `row a[]`) is probed by Part E before it is lifted, and rejected
      otherwise. Such a parameter is accepted with `owns_range(a, n)` or `reads_range(a, n)` over `n` rows (item 7), `a[i][j]`
      in code and in contracts, and a row `a[i]` used as a pointer. The outer bound of `T a[N][M]` is checked as
      written and is no promise (0060 item 15); the inner bound is part of the type. A matrix passed to such a
      parameter is its decay, bound to a local. `sizeof(a)` of such a parameter is rejected
      (`sizeof_array_argument`). P83 (d): the three spellings share one adjusted type, pointer to the row's
      array type with the row's bound and its `expr`; the written type is the parameter type's `declared_type`
      (so the spelling can be told), `[]` has no bound constant, and a call with a matrix of another inner bound
      is the front end's error in both modes (in C its #167, which the driver makes an error).
    - **Computed types** (0063 item 14, extended): a typedef, `typeof`, `decltype`, and the type operand of
      `sizeof` and `_Alignof` may name an array of aggregates when every bound in it is read as written.
14. **Quantifiers and applications** (amends 0061 items 1, 3 and 6, 0062 item 4).
    - **Under the binder** a read may step through members of the indexed element: `p[i ± c].x`, `v[i].t.x`.
      Its read class is the pair (argument, member steps), its trigger that leaf's receiver (`{
      loc(member(loc(block_of(X), q), j), 0) }`), and 0061 items 6–10's framing, bound and flag rows are written
      per class against that leaf's range or array place.
    - **The pointer argument** of `cerastes_forall` and of a `pure` application may be the decay of an array
      member of scalars (`p->a` and `s.a`; in a contract, of a structure the contract owns or reads through a
      pointer parameter) and a named global array in a contract (P83 (g): each is `array_to_pointer` over a
      field or a variable, with no load; a global is not folded in an annotation; a `pure` application's
      argument is under a compiler-generated qualification cast, which is peeled as 0060 item 3 peels the
      top-level generated conversion). It reads no memory (an address is formed, not
      loaded); `p->q->a` stays rejected (M4.5e2). The class's block is the member's or the global's block term
      in a contract line, and its bound local in code.
15. **The leftovers of a structure with an array member** (amends 0063 items 1 and 7). Each site below takes
    the object view (a cell leaf as before, an array leaf with 0063's quantified lines), and audit round 4 gives
    a verdict per site of the plan's section 8 table.
    - **Heap storage** (question 292; amends 0047 items 2, 4, 6 and 8). `malloc(sizeof(struct S))`,
      `calloc(1, sizeof(struct S))` and `free` take a structure with array members: the typed view's `ensures`
      and `owns_block` state the member lengths and hold each array leaf by its quantified permissions and
      flags (`calloc`: every element zero and set; `malloc`: the flags unknown, as 0047 item 8); `free`'s rows
      are per leaf (`invalid-free`); the leak check reads the block's token as before.
    - **An `owning` or marked member beside an array member** (293; amends 0055 items 2, 3 and 5, 0058 item 4).
      The structure is accepted. An array leaf of scalars is never an owner (arrays of owning pointers are
      M4.8's), so every owner site walks the cell leaves and passes over an array leaf; the closure of an atom
      holds the array leaf's lines beside its owners' blocks.
    - **Markers on a pointer to such a structure** (294; amends 0042, 0058 item 4). `ref_to_uninit`: the
      `requires` site claims no flag of an array leaf either, and the kept-flag row is one quantifier per array
      leaf (`old(flag) ==> flag`, where 0058 item 4 keeps it for a cell). `must_init`: every element's flag is
      set at the return (`uninitialized`).
    - **Contract places through a heap read** (306; amends 0045 item 1). A load-rooted place whose structure has
      an array member takes 0045's rows with the array leaf's lines keyed by the loaded pointer; `p->q->a[e]`
      is an element read through that place.
    - **An `ensures` atom with no `requires` partner** (amends 0052 item 2) on a structure with an array member:
      covered or given per leaf, an array leaf by its quantified lines and, where 0052 gives an output amount,
      one amount for the site.
    - **A written atom in a loop invariant** on such a structure place (amends 0045 item 12, 0040 item 5): the
      array leaf's conjuncts are a range place's, its amount the first slot's (0060 item 6). If S48 (k) refuses
      it, it stays rejected with the range atoms of question 237 (M4.5e2).
16. **Brace elision under `{0}`: the measured rule** (the owner's answer to question 304, "Follow the rejection
    like the compiler warning"; question 321, the advisor's ruling by the plan's P83 (h) row, "the measured set
    is the rule"; amends 0043 item 6, 0063 item 6). The planning worded the rejection as "C++'s `{0}` over a
    first member that is a structure or an array, in an aggregate of two or more members"; the matrix M
    (clang 21.1.8 and GCC 16.0.1 on the box, 668 shapes, both modes, three tiers; the Mac's compilers agree)
    measured a wider set, and the rule is that set, 0020 item 5's union of the first two tiers over both
    compilers:
    - **In C++** (clang++ `-Wmissing-braces` at `-Wall`; GCC's C++ never warns): a braced list is rejected when
      some subaggregate whose braces it elides has, as its parent, **an array (of any length, one element
      included) or a structure with two or more members**. An elided subaggregate that is the only member of
      its structure never makes the list rejected, at any depth. Storage and position (a local, a global, a
      compound literal, a member, an element) do not matter, and neither does the value (`{1}` is flagged
      where `{0}` is). So C++'s `struct P v[2] = {0};`, `int m[2][3] = {0};`, `{0}` over a structure of two
      members whose first is a structure or an array, and `{0}` over a one-member structure that holds a
      matrix, an array of structures or a structure beginning with an array (the inner elision's parent is an
      array or has two members) are rejected; `{0}` over `struct T { struct P p; }`, over `struct T { int a[4];
      }` and over a chain of only members is accepted, as before.
    - **In C** (GCC `-Wmissing-braces` at `-Wall`; clang's C is silent on every `{0}`, `{}` and `{{0}}` form):
      elision is accepted only where **the whole initializer of the object is the lone `{0}`** (C's universal
      zero initializer, at any depth: a local, a global, a compound literal `(T){0}`, an array `T v[2] = {0}`,
      a matrix). A nested list whose `0` elides a subaggregate's braces (`struct W { T t; int k; } w = {{0}};`
      with `T` an aggregate whose first member is one; `T v[2] = {{0}}` likewise) is rejected like GCC.
      **C's lone `{0}` over a structure whose first member is a structure that begins with an array is
      accepted** (both compilers are silent): the base rejects it in both modes with a message that says C
      accepts it (`collect_init_elems`' `read_list` under the elided recursion; Step 0's R2), a defect this
      item fixes for C.
    - `{}` elides nothing and is accepted in both modes. A list with every level's braces written is accepted.
      A non-zero list that elides braces stays rejected in both modes as 0043 item 6 and 0063 item 6 have it
      (stricter than clang++ for an only member; unchanged).
    - **What changes for programs accepted before this record:** in C++, `{0}` over a first member that is a
      nested structure in an aggregate of two or more members (question 304); and, where the base accepts
      them, the nested `{{0}}` forms GCC flags in C and the forms clang++ flags in C++ (the test-mutator
      probes the frozen base on each form the base can express, and the result is recorded here before Part
      B's flips). Step 0 read the 37 files under `tests/` and `doc/` that hold `{0}`: none changes outcome
      under the C++ clause. **An existing file whose outcome changes at Part B's joined proof goes to the
      owner before it is touched** (the plan's Flips G). This goes beyond question 304's wording and is logged
      for the owner's review (question 321).
    - The rule is in `collect_init_elems`, which locals, globals and values share. A C positive in the tests
      avoids every row GCC flags (the `native-ok` policy).
17. **The lint** (amends 0012 item 11, 0060 item 16, 0061 item 11, 0063 item 15).
    - **The receiver grammar** gains index steps: a quantified row's receiver is a chain of `loc(<block>, <bound
      variable or 0>)` and `member(<ref>, <literal>)` steps from `block_of(<bound local>)` in code and from a
      block term of 0063 item 15's grammar where a line has no local, with one bound variable per index step,
      each ranging over literal or `offset_of` bounds as today; the trigger is the receiver.
    - The length facts of item 3 are pinned where they are stated; `alloc_arr_<E>` and `zero_arr_<E>` by their
      clause sets, each (receiver, field) once; `program_start`'s row group per leaf path; a user quantifier's
      class trigger with member steps (item 14).
    - Each rule with a refusing twin in `scripts/test_lint_viper.py`.
18. **Categories and tags:** none new. **Facts:** P83, S48.

## Facts and the branches taken

Each probe and spike of the plan's section 6, its outcome, the branch of its decision table, and the item it
revised. (To be filled from P83 and S48.)

| Row | Outcome | Branch taken | Item revised |
|---|---|---|---|
| P83 (a) arrays of structures in code | the variable's type is e1a's `tk_array` entry over the structure; `v[i].x` is `dot_field(subscript(array_to_pointer(var v), i), x)` and `p[i].x` the same over `var p` with no `array_to_pointer`; copies are `eok_assign` or `dik_expression`, never a call; a by-value argument is the subscript; `sizeof(v) / sizeof(v[0])` folds to a constant that keeps its `expr`. In C the decay of a global array is folded into a `ck_address` constant in every rvalue position of a declaration's initializer and as a whole call argument (both modes), never on either side of an assignment or in an annotation | Expected (refined: where the decay is folded) | 5 |
| P83 (b) initializers of arrays of aggregates | one nested `ck_aggregate` per written element, trailing elements absent; elision is told only by the inner aggregate lacking `explicit_braces_on_aggregate`, and `{0}` over an array of structures has the shape of an elided `{3}`; an array designator is a `ck_designator`; an excess element is the front end's error in both modes | Expected | 6, 16 |
| P83 (c) arrays of arrays | a `tk_array` entry over a `tk_array` entry, each bound with its own `expr` in every position (in C++ every non-local bound of both dimensions under a generated cast); `m[i][j]` is `subscript(array_to_pointer(subscript(array_to_pointer(var m), i)), j)`; a row's use is `array_to_pointer` over the inner subscript | Expected | 2, 13 |
| P83 (d) array-of-rows parameters | `int a[][3]`, `int a[2][3]` and `int (*a)[3]` share one adjusted type, pointer to the row's `tk_array` entry with its bound's `expr`; the written type is the parameter type's `declared_type` (so the spelling can be told), and `[]` has no bound constant; `sizeof(a)` is the pointer's size; a wrong inner bound at a call is an error in both modes (in C EDG's #167, which the driver already makes an error) | Expected (cut 5 not taken) | 13 |
| P83 (e) members and nesting | the field's type is (a)'s or (c)'s entry and the accesses compose; the C++ class tests of 0043 item 2 pass for a structure with a member that is an array of structures, a matrix member and an element structure with an array member (two trivial `operator=` each) | Expected | 2, 15 |
| P83 (f) globals | `initk_static` over (b)'s shapes; a `static` local through `local-static-variable-init`; pointer leaves are `ck_address`; P38's loss of `expr` is keyed to the outermost list (1,001 elements keep every `expr`, 1,002 lose it for every leaf below; `unsigned char big[2][1002]` keeps all) | Expected (refined: P38 by the outermost list) | 9 |
| P83 (g) quantifier bodies and arguments | a body's read is (a)'s shape over the parameter; each `cerastes_forall` argument is `array_to_pointer` over a field or a variable, with no load, and a global is not folded in an annotation; a `pure` application's argument is under a compiler-generated qualification cast, which `application` peels | Expected (refined: the generated cast) | 14 |
| P83 (h) the compiler matrix M (Step 0) | clang 21.1.8 and GCC 16.0.1 on the box, both modes, three tiers, 668 shapes, 8,016 compiles (the Mac's compilers agree on all but 8 cells, none of `-Wmissing-braces` or `-Wsequence-point`): every positive shape compiles with `-Wall -Wextra -Werror`; EDG's #175 fires on every constant-index form but `&m[2][0]` and `&m[1][3]`; `sizeof(a)` of a parameter warns for the `[]` spellings and not for `int (*a)[3]`; **question 304's measured set is wider than the planned wording** (clang++: an elided subaggregate whose parent is an array or a structure of two or more members; GCC's C: any elision but the lone whole-initializer `{0}`); **one family with no rule:** GCC `-Waddress` at `-Wall` on a truth test or null comparison of a row or an element address | the rule; "If not" for question 304 (the measured set is the rule, question 321) and for the new family (question 322) | 5, 16 |
| S48 (a) a local array of structures | `alloc_arr_P(3)` in item 6's shape (the length-facts quantifier, one quantifier per leaf and permission), `v[i].x` read and written for a symbolic `i`, `&v[i]` passed to an `owns(*p)` callee with its cell lines taken out of the quantified chunks and given back, the scope exit per leaf: verified; the twins one error each (the bound row; the first marked scope-exit row); 4 fixtures, 7.1–7.7 s | Expected | 3, 5, 6 |
| S48 (b) a range over structures | `owns_range` in a callee that writes `p[i].x` with the caller reading it back; `reads_range` at a ghost amount with a forwarded `(p, n)` call; a return's rows: verified; the twins one error each (a write under `reads_range`; a caller holding one element fewer, at the first marked row); 5 fixtures, 6.9–13.1 s | Expected | 7, 8 |
| S48 (c) the pairing rows | each of the three pairings (a range of structures beside a cell, beside a structure atom, two ranges) in both forms: every positive verifies and every alias twin is exactly one error (Form P: `assertion.false` at the pure row; Form S: `insufficient.permission` at the first marked sum row); no alias twin verifies in either form; 8 fixtures, 16.8–25.2 s | Expected: **Form P for every pairing** (both forms pass every pair; the table's tie-break) | 7 |
| S48 (d) loops over an array of structures | a local array and a parameter's range, each leaf its own place with the entry marked rows and the conjuncts, one liveness conjunct per root; a loop writing `p[i].x` under a quantified invariant: verified; the twins one `invariant-preserved` each (the user's conjunct; the values-kept conjunct of leaf `y` with a store through an `int *` in the body, so that conjunct must be absent there); 4 fixtures, 9.6–15.9 s | Expected | 10 |
| S48 (e) a global array of structures | `program_start` with item 9's row group per leaf (`struct S g[3] = {{1, 2}, {3}}`: one conjunct per leaf of each written element, one quantifier per leaf for the rest), `main`'s loop, an `owns(g)` callee (which needs the global facts among its `requires`, as 0041 has them): verified; the twins one error each; 4 fixtures, 6.6–9.5 s | Expected (cut 3 not taken) | 9 |
| S48 (f) an array member of structures | `alloc_struct_T` for `struct T { struct P v[2]; int n; }` with the nested length facts and one quantifier per leaf of `v` and permission over `loc(member(loc(member(r, 0), k), j), 0)`; `t.v[i].x` as a local and under `owns(*p)`: verified; `f(&t, &t.v[1])` one error at the Form P slot row; 3 fixtures, 9.1–10.6 s | Expected (cut 4 not taken) | 3, 7 |
| S48 (g) depth 2 | an array-of-rows parameter with `owns_range(a, n)` over two bound variables and a caller passing `m` (8.6 s; `m[0][3]` one error at the second bound); a row `m[1]` bound to a local and passed to a one-variable `owns_range(q, 3)` callee (8.6 s: Silicon takes the one-variable quantifier out of the two-variable chunk and gives it back; the keeping callee one error); `v[i].a[j]` over `struct W { int a[4]; int n; } v[2]`; the copy `v[i] = v[j]` under the guard `d != s`, which Silicon decides in both directions (the unguarded twin one `precondition` error at the copy): verified, **34.5 s and 35.2 s at load about 2, over the row's 30 s line** | Expected for the row call, the parameter and `v[i].a[j]` (cut 5 not taken); the copy's time to the advisor (question 323) | 5, 12, 13 |
| S48 (n) cost | an array of structures with four leaves and a loop over each 58.9 s (load 0.6); S45 (k)'s scale program at K = 16 holding one array of structures 52.7 s (load 1.3); a global array of 1,000 structures with 16 listed 8.5 s | recorded (under the 60 s line, two of them closely) | — |
| S48 (h) heap storage | (second pass) | | |
| S48 (i) markers | (second pass) | | |
| S48 (j) a read class with member steps | (second pass) | | |
| S48 (k) a written atom | (second pass) | | |
| S48 (l) a partnerless atom | (second pass) | | |
| S48 (m) depth 3 | (second pass) | | |
| Step 0 R1–R3 | R1: every pin of the plan's section 10 has the outcome it lists (30 Flips D rows unchanged; no extra or absent row); one pin contradicts the plan's scope (`array_reject_pointer_to_array.c` is a parameter: question 320). R2: 174 probes, 340 runs: nothing of item 1's "In" list verifies before this record; C's `{0}` over a nested structure that begins with an array is rejected on the base in both modes (the suspected defect, confirmed); 12 staging pins (38 rows) added. R3: 2,569 rejection runs recorded (2,593 with the pins) | the plan's rule; question 320 to the owner | 13, 16 |

## Consequences

- C programs that keep arrays of structures and arrays of arrays in automatic, global and `static` storage and in
  structures verify, with ranges over structures in contracts, element reads in contracts, loops that carry them
  per leaf path, and array-of-rows parameters; a structure with an array member can live on the heap, hold an
  `owning` or marked member, and be reached through a heap read in a contract.
- Every array of aggregates costs a quantified permission per leaf path and field in its allocation, scope exit,
  atoms and loops, and a depth-2 leaf a permission over two bound variables.
- Ranges on part of an array of aggregates, atoms on one element, `malloc(n * sizeof(struct S))` and a second
  index under a binder wait for M4.5e2; element structures with `owning` or marked members for M4.8; pointers to
  arrays outside parameters and depth 3 or more (unless S48 (m) admits it) for M8.
- Programs accepted before this record keep their Viper and maps byte for byte; one form accepted before is now
  rejected in C++ (item 16), which no existing test or doc example holds.
- **A session that closes at a split point** accepts this record for the items built; the others stay "decided,
  built by the continuation".
