# 0059 — Arrays inside one function (M4.5e1a)

- **Status:** accepted (2026-11-11, M4.5e1a), written before code (invariant 6) from the plan's section 3.5
  (`.claude/plans/m4-5e1-arrays.md`, approved 2026-11-11); revised after P78 and S43
  (`internal/il-inventory.md`, chapter M4.5e1a), the Step-0 runs and the compiler matrix, and audit round 1 (B1 and B2:
  array designators rejected; items 3–17 taken or stated), audit round 2 on Parts A and B (B1: `register`, item 2's
  #138; items 2–5 taken), audit round 3 on Parts C–E (no blocking item; `sizeof` only, a loaded base's spelling, the
  array-place alias guards) and the re-audit of its fixes (no blocking item; the lint's loop-carry families tied to
  their rows). Each item says what was built. Owner decisions:
  - At M4.5e1's planning (2026-11-11): **226** `forall` in C inlines a `pure` function's body (e1c); **227** the exact
    sum is in M4.5e1 (e1d); **228** `cerastes_reads_range` is added beside `owns_range` (e1b); the split into e1a–e1e.
  - Before this session (2026-11-11), each the recommendation: **229** a `*p` one past the end of an array is reported
    as `permission` (item 4); **230** `int a[static N]` parameters are rejected, an M8 entry (item 1); 231–233 are
    e1b's, e1c's and e1e's.
  - Open at the session's end, each built as its recommendation (handoff §8): **234** array designators rejected in C
    too (item 3); **235** the 1,080 existing `.vpr` files item 10 changes (item 10).
  - Standing: 69 (no EDG work), 77 (one context), 160 (a new test's expect line corrected after its first run, listed
    in status).
- **Milestone:** M4.5e1a.
- **Amends:** 0011 item 10; 0012 item 12; 0024 item 6; 0028 items 4, 10 and 11; 0032 items 3 (the driver's
  `--diag_error` list gains EDG's #1162 and #138 in C and #175 in both modes), 4 and 8; 0038 items 3 and 4; 0039 item 3; 0042
  items 3 and 5; 0043 item 6 (#1162 reaches struct initializers too); 0054 items 3 and 6.

## Context

Record 0028 designed arrays with the rest of the C memory model: a block of `len(b)` slots (item 1), element `k` the
`Ref` `loc(b, k)`, quantified permissions over the absolute slot with the trigger `{ loc(block_of(a), k) }` (item 2),
`out-of-bounds` as a category (item 10), and the marked range rows a permission check over a range needs because Viper
refuses `perm` under a quantifier (item 11, the owner's rule after S25). Every session since M4.5b1a built the
one-slot case: every block has `len == 1` and its cell is `loc(b, 0)`, VIR has no array type and no index node, and the
encoder rejects every array construct. The design is pinned by hand-written fixtures (`e2_array_sum{,_fail}.vpr`,
`e2_array_elem_swap_fail.vpr`, `s26_*`), none of which allocates a local array block, and record 0038's automatic loop
invariant snapshots `perm` per cell, which Viper refuses under a quantifier.

M4.5e1 (arrays) was split at its planning into five sub-sessions (question 224). This one, e1a, takes arrays that live
and die inside one function: automatic local arrays, indexing, initializers, scope exits, loops, sequencing, and
pointers into arrays. Ranges in contracts (`owns_range`, `reads_range`, array parameters) are e1b's, quantifiers e1c's,
`pure` functions and the exact sum e1d's, arrays in aggregates and storage e1e's, pointer arithmetic M4.5e2's.

## Decision

1. **Scope.** In: an automatic local array `T a[N]` of one dimension, where `T` is `bool`, an integer type, an
   enumeration or a pointer to an object type, and `N` is a constant (`int a[] = {…}` included), with or without a
   braced initializer; `a[i]` read, written, compound-assigned, incremented and decremented; `&a[i]` for `0 <= i <=
   N`; reads of `a[i]` in `CERASTES_INVARIANT`, loop measures and `CERASTES_CHECK`; loops; scope exits; sequencing
   between elements; the decay of `a` (`int *p = a;`, `f(a)`); `p[i]` and `&p[i]` on any pointer; `sizeof(a)`;
   `cerastes::uninit` on an array local. A new category, `out-of-bounds` (item 5), and the marked range rows in the
   driver (item 7). Everything else stays rejected, each with a diagnostic naming where it is lifted and a pin:

   | Construct | Lifted by |
   |---|---|
   | `owns_range`, `reads_range`, atoms on elements (`owns(a[i])`, `owns(*&a[i])`), an element or an array as a pointer in a contract, `p[i]` of a pointer and an array as a pointer in a loop invariant or measure, `owning(n)` on a parameter | e1b |
   | `int a[static N]` parameters (`unsupported_declaration`; R1: accepted as `int *` in C until this session, the bound ignored; a front-end error in C++) | M8 (question 230: C makes a shorter argument undefined, and no contract states it) |
   | `cerastes_forall` (the C++ lambda form: M7) | e1c |
   | array members, arrays of structures, arrays of arrays, global and `static` arrays, array compound literals, array types in `sizeof(type)`, `typeof` and typedefs | e1e |
   | `p + i`, `p - q`, relational comparisons of pointers, `malloc(n * sizeof(T))`, `calloc(n, …)`, `realloc`, `owning(n)` on results, locals, fields and globals | M4.5e2 |
   | `const` arrays, `char s[] = "abc"`, `sizeof("abc")` | M4.6 |
   | arrays of owning pointers (`owning` on `int *a[2]`) | M4.8 |
   | VLAs, a zero-length array, flexible array members, a pointer to an array (`int (*p)[3]`), range-based `for` | M8 (range-`for`: M7) |

2. **Array objects** (0028 item 1). A local `T a[N]` is one block of `len` N; element `k` is `loc(b, k)`, in the value
   and flag fields of `T`'s cell type (0032 item 3's fields: `v_int`/`d_int`, …, a pointer element by its pointee,
   `v_ptr_int`). The variable's Viper local holds `loc(b, 0)`. An array local is always a block, whether or not the
   front end flags it address-taken: P78 F4 found that `a[i]` in code sets `address_taken`, but `sizeof(a)` and reads
   only in `CERASTES_CHECK` or `CERASTES_INVARIANT` do not.
   - **The bound** is read through `constant_value` from its written expression (0024: a constant is checked as
     written). P78 (a), corrected by audit round 1 (item 3): a literal, a macro that expands to one and an enumerator keep
     no `expr` (there is nothing to check: the value is the one written, an enumerator's checked by 0018); a folded
     expression keeps its `expr`, `int a[65536u * 65536u + 1]` (folded to one element) included, so a wrapping bound is
     checked as written: `int a[65536u * 65536u + 1];` is one `overflow` error at the bound ("unsigned `unsigned int`
     multiplication may overflow (cerastes does not wrap)", exit 1, both modes; `array_wrapped_bound.c`), and a
     namespace-scope `const unsigned N = 65536u * 65536u + 1;` used as a bound is one `overflow` at `N`'s line, as it
     is without an array (`array_namespace_bound.cpp`); a namespace-scope
     `const` or `constexpr`, a static member and an `inline constexpr` keep `expr: var N` and are 0017's named
     constants, checked in the prologue. The bound is at least
     1. `int a[] = {…}` takes its bound from the list (the IL's array type).
   - **A bound that names a local constant** (C++'s `const int N = 4; int a[N];`; in C a VLA, EDG's #1901, an error in
     verify mode) keeps no `expr`: the IL sets `constant_bound_expr_in_local_expr_node_ref` and the written bound is in
     a `local-expr-node-ref` (P78 F1). A wrapped one cannot be told from a written one
     (`const unsigned N = 65536u * 65536u + 1;` gives `int [1]`), so it is rejected (and with it a `constexpr` or
     `static const` local and a local lambda's call, which set the same flag: audit round 1, item 4) (`unsupported_declaration`, e1e,
     with 0024 item 6's types computed from constants), as the plan's stop rule for (a) says.
   - **`register` arrays** (audit round 2, B1): `register int a[2] = {1, 2}; return a[1];` verified, with only EDG's
     warning #138, though C23 6.3.2.1p3 makes the decay of a `register` array undefined (clang refuses it, GCC is
     silent). The same warning was all that `register int x; int *p = &x;` gave, a constraint violation (6.5.3.2p1)
     accepted since M4.5b1a. The driver makes #138 an error in C verify mode; C++ already refuses both (#2798).
   - **Rejected:** the rows of item 1's table; `int a[0]` (EDG accepts it with a warning, P78 F2: a zero-length
     array, M8); `static`, `extern` and thread storage (`unsupported_declaration`); a `const`, `volatile` or `_Atomic`
     element type (`unsupported_type`); `owning` or `ref_to_uninit` on the array (`unsupported_annotation`; P78 (d):
     a marker reaches the encoder in the leading position and after the declarator-id before the bound, `int a
     [[cerastes::uninit]] [3]`; after the bound it is an EDG error).

3. **Allocation** (0028 item 4, 0032 item 4). One trusted method per element cell type, tagged
   `// cerastes:trusted-contract` like `alloc_<t>`:
   `alloc_arr_<t>(n: Int) returns (r: Ref)`, `requires n >= 1`, ensuring `r != null`, `r == loc(block_of(r), 0)`,
   `len(block_of(r)) == n`, `lvloc(block_of(r)) == loc(block_of(r), -2)` with full permission to its `lv`, and, each as
   its own quantifier over the absolute slot with the trigger `{ loc(block_of(r), k) }` (0028 item 2), full permission
   to `v_<t>` and to `d_<t>` of every slot in `[0, n)`, and every flag false.
   - It is called where the array's block is entered (0032 item 4: at the entry of the enclosing block, and before a
     `switch`'s dispatch), each entry allocating anew.
   - **A braced initializer** runs at the declaration: `zero_arr_<t>(r, n)` (trusted, tagged: it takes and returns the
     same quantified permissions and ensures every element zero, `false` or `null`, and every flag true; an
     enumeration's element is 0, which is within every enumeration's range, 0018), then one store
     per written element, in the IL's order. It is C's rule (C23 6.7.11p11 and p21): elements without an initializer
     are initialized as objects with static storage duration. A list that writes all N elements skips the fill. In VIR
     the declaration carries the fill (a `zero_filled` flag on the array's variable), and the written elements follow
     as ordinary stores to `ElemAddr(AddrOf(a), k)`.
   - **Every element's constant goes through `init_element_value`** (P38: a list of more than 1,000 elements keeps no
     `expr` for its elements, only an `orig_type`; reading `ck_aggregate`'s values directly would let a wrapping
     element pass unchecked). Non-constant elements follow the rule struct initializers already have
     (`collect_init_elems`), in the IL's order (P78 (b)).
   - **The IL** (P78 (b)): every automatic array's initializer is `initk_dynamic`, `dik_constant` over a `ck_aggregate`
     when every element is constant, else `dik_nonconstant_aggregate` with `ck_dynamic_init` elements, in written
     order; trailing zeros are not materialized (`{1}` over four elements is one element); `{}` is an empty aggregate
     in C23 and C++20; a designator is `ck_designator` with `array_element`, the elements after it undesignated, in
     written order. A list of 1,001 constants keeps every element's `expr` in automatic storage, so P38's lost `expr`
     does not arise here, and the elements still go through `init_element_value`.
   - **Too many elements** (`int a[3] = {1, 2, 3, 4};`) are EDG's error #146 in C++ and only warning #1162 in C, where
     the IL keeps the first three elements and drops the rest (P78 F3): the encoder cannot see them. The driver makes
     #1162 an error in C verify mode (`--diag_error`, beside 0032 item 3's list), as clang (`-Wexcess-initializers`)
     and GCC warn by default. It applies to every braced initializer, structures included.
   - **An element that reads, writes or takes the address of the array being declared** (`{1, a[0]}`, `{1, (a[0] =
     6)}`, `{1, g(&a[0])}`, and `{1, g(a)}` once decay is accepted) is rejected, as a local initialized from itself is
     (0012 item 12) and as 0043 item 6 rejects it for structures (its audit B1). Non-constant elements keep 0043 item
     6's order: in C every element is evaluated into a temporary before the first store (C23 6.7.11p24 leaves their
     order unspecified), in C++ each is stored in order.
   - **Array designators are rejected in both modes** (audit round 1, B1 and B2). Outside templates a `ck_designator`
     keeps only EDG's folded `array_element`, with no expression (`il_def.h:5551-5554`), and verify mode suppresses
     #61–63, which rests on every constant being checked as written (0024): `{[65536u * 65536u + 1] = 7}` would write
     element 1 unchecked against 0022, and `int c[] = {[65536u * 65536u] = 9}` would give the array a wrong bound.
     With them go an index given twice (`{[0] = f(), [0] = 2}`: clang `-Winitializer-overrides`, GCC
     `-Woverride-init-side-effects`, and C23 may leave the overridden element unevaluated) and GNU's range designator
     (`{[1 ... 2] = 5}`, `ck_init_repeat`). In C the diagnostic is `unsupported_declaration`, lifted when the front end
     keeps the designator's expression (M8: an EDG change, question 69); in C++ it is `c99_designator`, as the
     compilers warn (clang `-Wc99-designator` by default; GCC 16 refuses `{[2] = 5}` and `{[2] = 5, 6}`).
   - **Without an initializer** the flags stay false: reading an element that was not written is `uninitialized`
     (0019, path-precise).
   - **The model** (0028 item 2): a fresh allocated block of `len` n whose slots `0..n-1` are distinct `Ref`s
     (`loc_inv`), with no permission outside `[0, n)`. No trusted contract states anything else; S43 (g) asserts `false`
     after each trusted call and at the end of each fixture method, and it fails.

4. **Element places.** VIR gains `ElemAddr(base, index)`: the address of the element `index` slots after the address
   `base`, lowered to `loc(block_of(base), offset_of(base) + index)`.
   - `a[i]` is `Deref(ElemAddr(AddrOf(a), i))`; a write is a `Store` to it; `&a[i]` is the node as a value; the decay
     of `a` is `AddrOf(a)` with the element pointer type. `i[a]` is `a[i]` (P78 (c)).
   - **Rows, in this order, each atomic** (0028 item 11): for a pointer base, `null-dereference` and `lifetime`
     (0037's `ptr_use`); the bound, `out-of-bounds`: in place position `0 <= offset + i && offset + i < len`, in value
     position `0 <= offset + i && offset + i <= len` (one past the end is formed, never accessed); then `permission`
     (`perm > none` for a read, `== write` for a write) and, for a read, `uninitialized`.
   - **On a pointer known through `owns(*p)` or `reads(*p)`** the place bound holds for `p[0]`: the emitted atom already
     carries `offset_of(p) < len(block_of(p))` (S43, refuting the draft's concern that `p[0]` would fail where `*p`
     verifies). `p[1]` there fails twice, at the bound (`out-of-bounds`) and, since Silicon goes on with the failed
     row assumed, at the permission row (`permission`): two real cases (the object may end at `p`, or go on without
     the callee holding it), and no row order reports one (S43 N1). Tests expect both.
   - A named array in scope needs no null or liveness row (its block was entered and has not been left).
   - **A plain `char` index** (`a[c]`) is rejected like the compilers (`char_subscripts`: clang and GCC
     `-Wchar-subscripts` at `-Wall`; audit round 1, item 6); `signed char` and `unsigned char` indexes, a cast to
     `int` and a character constant (`a['\1']`, on which no compiler warns) are accepted. The same rule holds for
     `p[c]` on a pointer (both compilers warn), from Part D.
   - **A constant index outside a named array** is rejected like the compilers (the owner's rule for warning-level
     hazards, 0020 item 5's union of the first two tiers): clang warns by default (`-Warray-bounds`) on `a[3]`,
     `a[-1]`, `&a[4]`, `&a[-1]`, `3[a]`, macro, enumerator and folded bounds and C's and C++'s `const int k = 3;
     a[k]`, and GCC only with optimization. EDG's own warning #175 (`subscript_out_of_range`) fires on each of them,
     and also inside `if (0) { … }` where clang is silent (the stricter reading), so the driver makes #175 an error in
     verify mode in both modes. `&a[3]`, `p[3]` through a pointer, `sizeof(a[3])` and `0 && a[3]` warn nowhere: the
     first three get the path-precise rows (or none: `sizeof` does not evaluate), and the last a row under a false
     guard. Every other index gets the path-precise row.
   - **Messages** name the place as written: "reads `a[i]`, whose index may be outside `a`", "writes `a[i]`, whose index
     may be outside `a`", "forms `&a[i]` beyond one past the end of `a`", "writes `p[i]` outside the object `p` points
     into".
   - **`*p` keeps today's rows**, so that every existing program's Viper is unchanged: a `*p` one past the end is
     reported as `permission`, since no permission exists at `len` (question 229; M4.5e2 decides the bounds row with
     `p + i`).
   - The index is any integer expression of the language's integer types; it is evaluated once, and its value (not a
     conversion of it) is the offset: `a[-1]` is outside, never `a[SIZE_MAX]`.

5. **`out-of-bounds`** (0028 item 10) joins the driver's categories: an index or a formed address outside the object,
   reported at the subscript. It is added in every copy of 0011 item 10's table (`run_tests.py`'s `DRIVER_CATEGORIES`
   and its two mirrors in `test_run_tests.py`, `tests/README.md`, this record's amendment of 0011, the Error messages
   page) with a must-fail example.

6. **Scope exits** (0032 item 8). At every way out of the array's block, in 0032's order: one marked range row per
   field (value and flag), "the address of an element of `a` may still be in use when its lifetime ends"
   (`lifetime`), the liveness row as for a cell, then the `exhale` of both quantified permissions and of the liveness
   cell. After a failed marked row Silicon reports nothing else on that path (S43, gate G2: even an `assert false`
   placed after it is not reported), so the `exhale` needs no guard.

7. **Marked range rows** (0028 item 11; the owner's rule after S25).
   - **Why the reason is unambiguous** (audit round 1, item 13, reproduced): `B` is `block_of` of a Viper local, `lo`
     and `hi` are literals or locals, the trigger a domain term, and there is no amount; an incompleteness of Silicon's
     quantified permissions would show as a false `permission` or `lifetime`, never as a false proof.
   - **Shape:** `assert forall k: Int :: { loc(B, k) } lo <= k && k < hi ==> acc(loc(B, k).f[, amount])`, one field
     per row, `B`, `lo`, `hi` and the amount reading no heap. Such a row holds no heap read other than the checked
     permissions, so its `insufficient.permission` cannot come from anything else on the line.
   - **Map:** `low::Check` and `MapEntry` gain an expected reason. A marked row's second column is written
     `assert.failed[insufficient.permission]`; every other row is written as today, so no existing `.map` changes.
   - **Driver:** `map_errors` accepts on a marked row exactly `insufficient.permission`, mapped to the row's category
     and message, and on every other row exactly what it accepts today. Any other reason on a marked row, and
     `insufficient.permission` on an unmarked row, is exit 3. `qp.not.injective` stays exit 3.
   - **Lint:** a row with an expected reason has exactly this shape; a `forall` appears only in the shapes this record
     lists (the two trusted contracts, marked rows, the range `exhale`, item 8's invariant conjuncts).

8. **Loops** (0038 items 3 and 4, for a block with more than one slot). A loop that touches memory takes each array
   in an open scope as a **range place**.
   - **Entry**, after the scalar snapshots: the label `lh_<loop>`; a marked row per field, "this loop begins without the
     permission to every element of `a`" (`permission`; it can fail only after a callee kept an element); the liveness
     snapshot as for a cell.
   - **Conjuncts** (`internal` rows): full permission to both fields over `[0, N)`; the flags kept, `forall k :: in
     range && old[lh](flag) ==> flag`; the type's range, `forall k :: in range && flag ==> lo_T <= v && v <= hi_T` (none
     for `bool`; a pointer element: `WF`); and, when no write in the loop can reach the array, the values kept, `forall
     k :: in range ==> v == old[lh](v)`.
   - The flag and range conjuncts are 0042 item 5's loop rows (question 89) over the range, and like them they go on
     every array the loop holds, reached or not: a `ref_to_uninit` callee in the body may set a flag, and an inner loop
     must not lose the outer loop's row. Each mirrors a scalar row of `lower.cpp`'s `loop` (at `1c5f9bd`): the
     permission and liveness conjuncts 14553–14566; the flags kept, `kd > none && ds ==> flag`, 14616–14619; the type's
     range, `held && flag ==> range`, 14605–14615; the values kept, the `!pl.reach` arm, 14593–14599; the back-edge row
     about 15003–15014. The marked entry rows have no scalar precedent (a cell's entry snapshot is what is held, 0038
     item 4); S43's `s43_c_entry_keep_fail` shows that without them Silicon's own "invariant might not hold on entry"
     comes first. (The label comes after the entry rows, as in `s43_c_sum.vpr`; no semantic difference.)
   - **S43 (c), gate G3:** `old[lh]` under a quantifier keeps the values with the pinned Silicon (2.0–2.9 s per
     program); an ablation of each conjunct makes a fixture fail, so each is needed.
   - **A write can reach an array** when the loop holds a store to one of its elements by name, a store through a
     pointer of its cell field, or a call whose `requires` owns a cell of that field (0038 item 3's rule). A store by
     name to another array or to a scalar cell does not reach it.
   - **Back edge:** a marked row per field, "an iteration of this loop ends without the permission to `a` that it began
     with" (`permission`), before Silicon's own invariant check.
   - **Invariant and measure reads** of `a[e]`: framed by the range place; an index outside it is `out-of-bounds` ("this
     loop invariant reads `a[e]`, whose index may be outside `a`"); the flag obligation is 0038 item 6's
     (`uninitialized`).
   - **A missed write cannot give a false proof** (audit round 1, item 14): every conjunct is a checked invariant, so a
     values-kept conjunct on an array a write reaches, or any conjunct that is not inductive, fails as Silicon's
     `invariant.not.preserved` on an `internal` row (exit 3), never as a proof.
   - **As built** (Part C and audits 2–3): the entry rows and the back-edge rows' fallback are positioned at the loop
     keyword (`Stmt::keyword_loc`; C++ positions an attributed statement at its first attribute, so `Stmt::loc` would
     differ between modes). A loop holds every array allocated in an open block: one declared later in its block (the
     block allocates at entry), one declared in a `for` init, and one in the body (allocated and exited each
     iteration). Beside a parameter whose atom is conditional (`c == 0 || owns(*p)`), a parameter place's guards gain
     `block_of(p) != block_of(a)` and the liveness inequality for each array, with case splits before the snapshots, as
     0038 item 3 gives cells (an exit 3 before the fix). `p[i]` of a pointer and an array used as a pointer stay
     rejected in invariants and measures until e1b (pinned).
   - **What e1a cannot state:** after a loop that fills an array declared without an initializer, a read is
     `uninitialized` (only a quantified invariant says which elements were written). It is pinned
     (`array_loop_fill_then_read_false_alarm.c`) and e1c gives it its verifying twin.

9. **Sequencing** (0028 item 12; 0039 item 3). Two unsequenced accesses to elements of one field, at least one a write,
   get 0039 item 3's `unsequenced` row that their addresses differ (S43 (e): `loc(b, i) != loc(b, j)` holds exactly when
   `i != j`). Accesses that name one element as written are rejected like the compilers
   (`unsequenced_modification`): `a[i] = a[i]++`, `a[i]++ + a[i]` and `a[0] = a[0]++` (GCC `-Wsequence-point` at
   `-Wall`, both modes; clang is silent); `a[i] = i++` and `a[i++] = i` are 0020 item 4's rule on the local `i` (clang
   `-Wunsequenced` in C, GCC in both). No compiler warns on `a[i] = a[j]++` or `*p = a[0]++` (`int *p = a;`): they get
   the path-precise row.

10. **Pointers into arrays.** `p[i]` and `&p[i]` on any pointer use item 4's rows; `return a;` is
    `return_local_address` (0039's R14), and so is `return &a[i];` (Part B). `p[i]` on a pointer to a structure
    (`p[i].x`) is an array of structures, rejected until e1e. A truth test or a null comparison of an array's address (`if (a)`, `!a`,
    `a == NULL`, `a == 0`, `a != NULL`) is rejected like the compilers (`array_address_null_test`: clang
    `-Wpointer-bool-conversion` and `-Wtautological-pointer-compare` by default, GCC `-Waddress` at `-Wall`).
    - **Interior pointers and owners** (handoff §3, from M4.5d2b2a's audit round 1; S43 (f)). Once `&p[1]` exists, a
      pointer into a heap block need not be the block's start. `holds_token` and the three `leak_exemption` terms gain
      `offset_of(p) == 0`, and 0054 item 3(e)'s flag clause is restated on `hb_w`'s cell: without it an owner holding
      `&p[1]` of a one-object block would exempt the block at `main`'s end and the leak would be missed (S43 (f), in
      hand-written Viper). The change is defensive (audit round 1, item 10): `offset_of == 0` is already required by
      `block_rows` for `free` and `owns_block`, by `block_conj`, by the three owning-assignment rows and by
      `held_updates`, so no assignment the encoder writes hands an owner `&p[1]`: storing `&p[1]` into an owning
      global is `permission` at the store, and `free(&p[1])` is already `invalid-free`. The restated rows are
      backstops: no encoder path reaches them with an interior pointer, so only the hand-written fixtures `s43_f_*`
      show the hole they close, and no VIR case can. The Viper of every existing program with an owner changes in
      those rows, outcomes unchanged (the plan's Flips D): 1,080 `.vpr` files (no `.map`), each changed line a
      held-ghost or token update, a leak row's exemption term, or the flag clause gaining `offset_of(x) == 0`. The
      allow-list computed before the build predicted 849 (its search missed the token's `.tok` spelling; question 235).
    - **Until M4.5e2** (audit round 3 and the re-audit): the restated premise is redundant in `flag_follows`' user row
      and the ending leak rows only while every heap block has one slot and no store gives an owner an interior
      pointer; M4.5e2's `malloc(n * sizeof(T))` and `p + i` break both, and it re-argues them (handoff §3).
    - **No case splits are added** for pointers into arrays: a pointer assigned `&a[i]` or `a` and read after the
      assignment is framed by the array's range place; a pointer the loop reassigns has no loop place of its own, as
      for scalar cells.
    - **The loop-carried exemption terms** of records 0050 and 0056 (`t__N && …`, `(v != null && kt_h > none) && …`
      and their alias-guarded forms) are not restated: they are reached only through owners whose updates already
      require `offset_of == 0`; the lint pins them as their own family.
    - **Loop places of pointer locals** (audit round 1, item 15, predicted a false `out-of-bounds` for `pk[0]`): it does
      not reproduce. A pointer local the loop does not reassign keeps its loop place and `pk[0]` verifies wherever `*pk`
      does (`array_ptr_loop_pk_cell.c`); one the loop reassigns already fails at `*pk` (`null-dereference`,
      `permission`), as for scalars.

11. **`sizeof` and markers.** `sizeof(a)` of an array local is `N * sizeof(T)`, computed from the checked bound
    (P78 (c): `sizeof(a)`, `sizeof(a[0])` and their quotient fold to constants that keep `expr`); only `sizeof`:
    `_Alignof(a)` and C++'s `alignof(a)` of an expression are a GNU extension clang warns about by default
    (`-Wgnu-alignof-expression`), and `__datasizeof(a)` is clang's own, so they stay rejected (audit round 3, item 1).
    Only an array written with an array declarator qualifies; type operands, `typeof`, `decltype` and typedefs stay
    rejected (e1e). `cerastes::uninit` on an array local without an initializer is
    accepted (0042 item 3), and `--strict-init` asks for it as for a scalar (0042 item 8).

12. **The lint and the self-tests.** `lint_viper.py` pins the `alloc_arr_*` and `zero_arr_*` contracts (beside
    `alloc_contract`'s `len == 1` rule for `alloc_<t>`), item 7's shapes, `old[label]` only in item 8's conjuncts, and
    element flag writes through a `Ref` temporary. The VIR self-test gains the session's cases.

13. **Facts:** P78 and S43 (`internal/il-inventory.md`, chapter M4.5e1a; 49 fixtures `tests/viper-smoke/memory/s43_*.vpr`,
    each positive with its twin). Gates G2, G3 and G4 pass (no `qp.not.injective` in any fixture: the mixing method,
    `swap(&a[0], &a[1])` against the per-atom amounts, two `reads(*p)` atoms given `&a[i]` and `&a[j]`, a heap block
    beside an array). Costs (S43 (h), the Mac at load 3–7): three arrays and two loops 3.4 s; S26's scale program
    (rebuilt from its description, K = 16) 5.0 s, and 13.3 s with one array added. **Still rejected:** item 1's
    table.

## Consequences

- C programs that keep an array inside one function verify: indexing with its bounds, initializers with C's zero fill,
  pointers to elements passed to callees that own one element, loops whose automatic invariant carries the array.
- A loop that fills an array declared without an initializer cannot be followed by a read until e1c's quantified
  invariants (a pinned false alarm).
- The driver accepts a second Silicon reason, on marked rows only; every other row is gated as before.
- Every program with a heap owner changes its Viper in the token rows (`offset_of == 0`), outcomes unchanged.
