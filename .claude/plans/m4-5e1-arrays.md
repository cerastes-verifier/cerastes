# Plan: M4.5e1 — Arrays (five sub-sessions, records from 0059)

**Approved by the owner on 2026-11-11**, at a session that only planned (question 224). Its commit also split
M4.5e1 into five sub-sessions (e1a–e1e) and consolidated `internal/handoff.md` and `plan.md`. Questions 224 and
225 were answered in the handoff before it, 226–228 at it. **e1a is ready and runs from section 3.** e1b–e1e each
start with their probes and spikes as written here, then plan again (section 1, "What 'plan mode again' means").

## 1. Context

**Why.** M4.5e1 is the arrays session of the C memory model (records 0028 items 1, 2, 11, 12 and 16; 0029 items 1,
3 and 4). Its planning (question 224) found it does not fit one context: nothing of it exists yet.

**What the planning survey found** (tree at `f57ee43`; read, not run: no build or verifier ran at planning).
- VIR has no array type, no index node and no quantifier; the lowered program has no `forall` and no `inhale`; the
  emitted Viper has no quantified permission. Every block has `len == 1` and its cell is `loc(b, 0)` (slots `-1` and
  `-2` are the token and the liveness cell).
- **Marked range rows do not exist** (0028 item 11; 0047's context says so): the position map has no expected
  reason, and `map_errors` (`src/driver/report.cpp:163-177`) accepts only `assertion.false`.
- **`out-of-bounds` is not a category** (`scripts/run_tests.py:363-367`), though 0028 item 10 gives it "an index".
  It lands in e1a, not M4.5e2.
- The encoder rejects every array construct by a `tk_array` arm, by `operation`'s default, or by a message naming
  M4.5e1 or M4.5e2. `<cerastes.h>` has no `cerastes_owns_range`, `cerastes_forall` or `CERASTES_OWNING_N`.
- The design is pinned by hand-written fixtures in `tests/viper-smoke/memory/`: `e2_array_sum{,_fail}.vpr`,
  `e2_array_elem_swap_fail.vpr`, `s26_live_array_sum_nolive_fail.vpr`, `s26_member_matrix{,_fail}.vpr`,
  `s26_member_array_of_structs{,_fail}.vpr`, `s26_member_malloc_struct_array.vpr`. No fixture allocates a local
  array block, and record 0038's automatic loop invariant snapshots `perm` per cell, which Viper refuses under a
  quantifier: both are S43's.

**Owner decisions at this planning (2026-11-11).**
- **226 `forall` in C: inline the body.** `cerastes_forall(f, free...)`, where `f` is a `pure` function whose body
  is one `return E;`. The quantifier means `E` with `f`'s first parameter bound, substituted at the `forall` site
  (0006 item 9's meaning). It does not wait for M4.5f1; a call of a `pure` function anywhere else stays rejected
  until e1d.
- **227 The exact sum is in M4.5e1.** `r == a[0] + … + a[n-1]` needs a recursive `pure` function with an encoded
  body: a slice of M4.5f1/f2 comes into e1 as sub-session e1d.
- **228 `cerastes_reads_range(p, n)` is added** beside `owns_range` (0040 rejects `owns` of a `const` place, so
  `sum(const int *a, int n)` needs it).
- **The split** is this plan's (the owner's request allowed it).

**Standing:** 69 (no EDG work), 77 (one context), 160 (a new test's expect line may be corrected after its first
run, each listed in status), 176 (the owner handles CI); `check.sh` once, at the end, on the x86-64 box; no test
runs while a fix is queued; no Claude trailer; commits stay local.

### The sub-sessions

| Sub-session | Scope | Needs | `src/` estimate | State of this plan |
|---|---|---|---|---|
| **e1a Arrays inside one function** | local arrays as blocks, `a[i]`, `&a[i]`, initializers (P38), scope exits, the marked range rows, `out-of-bounds`, loops over arrays, sequencing, pointers into arrays (`p[i]`, decay), the `offset_of == 0` restatements | — | 2,200–3,450 | **ready** (section 3) |
| **e1b Ranges in contracts** | `owns_range`, `reads_range`, array parameters, calls and returns over ranges, `f__wf`, loops over parameter ranges, markers over ranges, `owning(n)` on a parameter, the `carrays.c` port | e1a | 2,000–3,000 | spikes specified; **plan mode again** (section 4) |
| **e1c Quantifiers** | `cerastes_forall` (question 226), triggers, `old` under a binder, max, the fill loop, the tutorial chapter | e1b | 1,200–2,000 | **plan mode again** (section 5) |
| **e1d Pure functions and the exact sum** | `pure` bodies as Viper functions, calls of `pure` functions in annotations, recursion with `decreases`, the exact sum (question 227) | e1c | 1,200–2,000 | **plan mode again** (section 6) |
| **e1e Arrays in aggregates and storage** | array members, arrays of structures, 2-D arrays, global and `static` arrays, array compound literals, 0024 item 6 lifted for array types | e1b (e1c for its quantified tests) | 1,500–2,500 | **plan mode again** (section 7) |

The estimates are insertions without self-tests, made by reading against M4.5d2b2b (2,750) and M4.5d2b2d (1,960);
each sub-session's own planning measures them again. Each sub-session is a session of its own (question 77): its
branch and worktree, its record (the next free number), its twins, docs, audits and one box `check.sh`.
**M4.5e2** needs e1a and e1b (and e1e for `malloc(n * sizeof(struct S))`); it may run before e1c–e1e.

**What "plan mode again" means.** The sub-session starts outside plan mode with its Step 0, Step 1, probes and
spikes exactly as written here (they change no `src/`), commits their results to `internal/il-inventory.md`,
then enters plan mode. That planning takes the spike results and the as-built facts of the sub-sessions before it,
refines the record draft and the encoding steps, lists the Flips, and is saved as
`.claude/plans/m4-5e1<x>-<topic>.md`.

## 2. Rules for every sub-session

`internal/handoff.md` §4's rules and §6's process notes apply. The ones this plan relies on:
- **Team** (`.claude/agents/`): three Sonnet agents with disjoint files and the Opus `soundness-auditor`,
  read-only. The orchestrator is Opus and may consult the Fable advisor before a record is accepted, at any cut or
  stop-and-ask, and before the box run.
  - **edg-backend:** `src/backend`, `src/driver`, `include/cerastes.h`, the builds, `regen_vpr.py`/`vprdiff.py`
    after each part.
  - **il-analyst:** the probes and spikes; then `src/vir` and `src/viper` with the VIR self-test cases. A fresh
    il-analyst takes over if its context runs low (`lower.cpp` is 18,728 lines).
  - **test-mutator:** tests, twins, exactly the Flips, the compiler matrix (`native-ok`: clang and GCC, both modes,
    `-Wall -Wextra -Werror`), `run_tests.py`, `lint_viper.py` and their self-tests, the doc example files.
  - **soundness-auditor:** one round per diff on a snapshot commit in `.claude/worktrees/<session>-audit`, one
    re-audit of blocking fixes; should-fix items not taken go to handoff §3.
  - **Orchestrator:** Steps 0 and 1, the record, the vprdiff allow-list (fixed per part before its build), docs
    prose, `internal/encoding.md`, integration, status and handoff.
- **`lower.cpp` is held by one agent at a time,** with a commit or a snapshot between holders.
- **Every agent:** any question stops that work and goes to the owner; each report is also written to
  `build/<session>/<role>-report-<n>.md`; builds are serialized (`lockf build/<session>/build.lock cmake --build
  build/cmake`) and held before a directory-wide run; one agent runs `run_tests.py`, the others use
  `build/cerastes verify <file>`; a test generator lives in `build/<session>/` and is never re-run over the tree;
  code style by `scripts/format.sh`, brace initialization, `U`, one declarator per declaration, no comment on a
  brace line.
- **A registration or a declaration lands with its reader** (P77: a registered name the encoder does not read is
  silently ignored).
- **Fail closed between parts:** what a later part or sub-session lifts is rejected with a diagnostic naming it,
  and pinned by a test.
- **Session tools and spike summaries are committed** (`scripts/`, `internal/il-inventory.md`).

## 3. M4.5e1a — Arrays inside one function (record 0059; ready)

Branch `m4.5e1a-arrays-in-functions`, worktree `.claude/worktrees/m4.5e1a`, `build/m4.5e1a/`. **This file is
e1a's approved plan** (there is no `m4-5e1a-*.md`); its status entry and handoff name it.

### 3.1 Scope

- **In:** an automatic local array `T a[N]` (one dimension; `T` is `bool`, an integer type, an enumeration or a
  pointer to an object type; `N` a constant, `int a[] = {…}` included), with or without a braced initializer;
  `a[i]` read, written, compound-assigned, incremented; `&a[i]` for `0 <= i <= N`; reads of `a[i]` in
  `CERASTES_INVARIANT` and `CERASTES_CHECK`; loops; scope exits; sequencing between elements; decay (`int *p =
  a;`, `f(a)`), `p[i]` and `&p[i]` on any pointer; `sizeof(a)`; `cerastes::uninit` on an array local.
- **New category:** `out-of-bounds`. **New driver feature:** marked range rows.
- **Not here** (rejected, each pinned, the message naming where it is lifted):

| Construct | Lifted by |
|---|---|
| `owns_range`, `reads_range`, atoms on elements, `owning(n)` on a parameter, `[static N]` parameters | e1b |
| `cerastes_forall` (the C++ lambda form: M7) | e1c |
| array members, arrays of structures, arrays of arrays, global and `static` arrays, array compound literals, array types in `sizeof(type)`, `typeof` and typedefs | e1e |
| `p + i`, `p - q`, relational comparisons, `malloc(n * sizeof(T))`, `calloc(n, …)`, `realloc`, `owning(n)` on results, locals, fields and globals | M4.5e2 |
| `const` arrays, `char s[] = "abc"`, `sizeof("abc")` | M4.6 |
| arrays of owning pointers | M4.8 |
| VLAs, a zero-length array, flexible array members, a pointer to an array (`int (*p)[3]`), range-based `for` | M8 (range-`for`: M7) |

### 3.2 Survey facts (`f57ee43`; `E` = `src/backend/encode.cpp`, `L` = `src/vir/lower.cpp`)

- **IL (P51, P44, P65):** `a[i]` is `subscript(array_to_pointer(var a), var i)`, operands as written (`i[a]`
  swaps them); `int *p = a` is `array_to_pointer(var a)` and sets `a.address_taken`; a local's `&a[2]` is
  `address_of(subscript(array_to_pointer(var a), 2))`; an array type is `tk_array` with `number_of_elements` and
  `bound_constant` (a VLA: `is_vla`, `bound_constant` NULL); a braced list is `ck_aggregate`
  (`first_constant`…`next`, `ck_designator`).
- **Types (`E`):** `vtype` 784–935 (`tk_array` 883–885), `vtype_object` 431–439, `signature_vtype` 442–462,
  `global_vtype` 465–487, `reject_type` 746–766, `computed_type_reason` 18468–18532 (`tk_array` 18489),
  `reject_computed_sizeof_operand` 4196–4211, the typedef rule 19321–19333, `struct_info`'s array arm
  19157–19160.
- **Declarations (`E`):** `declaration` 12812–12997 (type check 12871–12885; a cell at 12964–12968:
  `v->address_taken || is_struct`), `initializer` 12999–13038, `constant_initializer` 13040–13053,
  `collect_init_elems` 13123–13273 (a non-field designator rejected at 13156; elements `ck_dynamic_init`,
  `ck_integer`, `ck_address` only), `init_element_value` 13486–13518 (every constant through
  `constant_initializer`), `struct_initializer` 13523–13715, `zero_leaf` 13091.
- **`constant_value`** `E` 4000–4160: the P38 guard is 4135–4140 ("a folded constant whose original expression is
  not recorded"); a constant with neither `expr` nor `orig_type` falls through to `literal` (4159).
- **Expressions (`E`):** `operation` 10375–10547 has no case for `eok_subscript`, `eok_array_to_pointer`,
  `eok_padd`, `eok_psubtract`, `eok_pdiff` (default at 10545; `describe_operation` 162–189). Places:
  `place_address` 9929–10044, `address_of` 10218–10326 (the unflagged-variable inconsistency 10309–10324),
  `indirect_read` 10329–10373, `assign_target` 13891–13976, `lvalue_var` 13759–13795, `assignment_parts`
  13978–14126. Shape matchers: `place_variable` 9363, `is_member_node` 10046, `returned_local_address`
  14723–14811 (no `array_to_pointer` case), `names_local` 15043, `names_object` 15085, `same_pure_expr` 17296.
  Contract-mode places: `place_pointer` 6666–6704 ("a local or an element: M4.5e1" at 6698), `contract_place_address`
  6715–6834, `contract_read` 7625; `spec_atom` 7123–7293 (elements rejected at 7258–7265).
- **Loops (`E`):** `encode_loop_clauses` 12725–12809. **Diagnostics:** `diagnostics.def` `unsupported_type`
  99–101 (its text lists the accepted types).
- **VIR:** `Type` `vir.h:70-167` (kinds Void, Bool, Int, Ptr, Struct; `Program::pointer_to` `vir.cpp:282-293`);
  `Expr::Kind` `vir.h:286-400` (`MemberAddr` 371–386); `PlaceKey` 1066–1084; `Var` 532–565. Kind switches that
  need the new node: `vir.cpp` `guard_ok` 1085, `AtomCollector` 1129, `scan_assigned_expr`, `reads_heap`,
  `place_root` 1226; `check.cpp` `Checker::expr` 2250–2831, `declares_cell` 238, `pointer_node` 252,
  `accepted_pointer` 335, `value_type` 498, `stmt` 1770–2223, `store_value` 3103, sequencing 3693–4195;
  `L` `lower_node` 7507–7699, `escapes_in_expr` 2924–2974 (2930), `expr_touches` 12088, `lb_chain` 12538,
  `closed_expr` 17550.
- **Lowering (`L`):** `alloc_methods` 980–1035, `alloc_struct_method` 1040–1110, `alloc_cell` 3556–3574,
  `scope_end` 3593–3611, `scope_exit` 3612–3654, `deref` 3888–3924, `ptr_use` 3942–3980, `one_past_end`
  3985–4005, `store` 4116–4194, `cell_write` 7311–7467, `wf_expr` 5291–5306, the sequencing keys `key_of_cell`
  6644, `key_of_addr` 6708, `register_access` 7001, `loop_places` 12669–13267 (cells of open scopes 12979–13022),
  `loop` 13830–15147 (snapshots 14501–14548, conjuncts 14553–14610), `inv_cell` 3715, `inv_deref` 3754,
  `framing_condition` 3820, `holds_token` 2240–2244, `held_updates` 2264–2397, `leak_exemption`'s terms 11728,
  11758, 11765–11795, `flag_follows` 17195 (called from `owner_invariants` 17172–17178).
- **Lowered program and map:** `LExpr::Kind` `lowered.h:57-77` (`ForPerm` is the only binder), `LStmt::Kind`
  166–180, `low::Meta`/`Check` 133–150; `emit.cpp` `expr` 53–121, `line` 143–166; `MapEntry` `posmap.h:13-27`;
  `parse_position_map` `report.cpp:49-99`, `MapRow` `report.h:17-30`; `kReasons` `silicon_output.cpp:56-83`
  (`insufficient.permission` 62, `qp.not.injective` 65).
- **Lint (`scripts/lint_viper.py`):** `alloc_contract` 632–640 pins `len == 1` and `loc(…, 0)`; `lint_flags`
  3206 wants a flag write's receiver to be an identifier or `loc(GB_x(), 0)`; `lint_perm_locals` 1583;
  `INHALE_TAG_RE` 163. No rule knows `forall`.
- **Harness:** `DRIVER_CATEGORIES` `run_tests.py:363-367`, mirrored at `test_run_tests.py:1991` and `:11673`, in
  `tests/README.md` 409–438, `error-messages.adoc` and record 0011 item 10. `VIR_CASES_MIN=1765`
  (`scripts/check.sh:104`); the VIR case files are `src/viper/selftest_cases2..27.inc`, each registered three
  times in `selftest.cpp` (1455–1498, 1501–1526, 1748–1796).

### 3.3 Step 0 and Step 1

1. `git fetch`; branch from `origin/main` if it holds the planning commit, else from local `main` (say so in
   status). No box check is pending. Everything is committed on the session branch.
2. `git worktree add .claude/worktrees/m4.5e1a -b m4.5e1a-arrays-in-functions <base>`; submodules with
   `--reference` (edg, silicon, then silver); `scripts/bootstrap.sh`.
3. CI (questions 172, 176), recorded only: `gh run list --workflow ci.yml --event workflow_dispatch` and
   `--event schedule --limit 3`.
4. Baseline before any change to `src/`: `scripts/regen_vpr.py` and the VIR self-test into
   `build/m4.5e1a/baseline/`; a frozen copy of the build as `build/m4.5e1a/cerastes-base`.
5. **Step-0 runs** (test-mutator, the frozen build, programs in `build/m4.5e1a/s0/`, both modes). They gate what
   the survey only read:
   - **R1:** `int first(int a[]) CERASTES_REQUIRES(cerastes_owns(*a)) { return *a; }`, and the same with
     `int a[3]`, `const int a[]`, `int a[static 3]`. Read: accepted as `int *` (0042 item 4 calls `int p[]` a
     pointer parameter); P44's table says rejected. The result is recorded; `[static N]` accepted today is a
     finding for the owner (question 230).
   - **R2:** today's diagnostic for `p[0]`, `&p[1]` and `p + 1` on a pointer parameter, `char s[] = "abc"`,
     `int a[3]; return sizeof(a);`. They become the baseline of 3.8's "re-messaged" list.
   - **R3:** the outcome of every `rejected` test under `tests/scalar/negative`, `tests/memory/negative` and
     `tests/vlibc/negative` on the frozen build (they stop before Silicon), kept as
     `build/m4.5e1a/s0/rejected-base.txt`. After each part the same run on the new build may differ only by 3.8.
   - One `run_tests.py` runner at a time: R3 runs while the il-analyst does P78 (which uses only
     `scripts/cerastes-fe.sh`); S43's runs start when R3 has finished.

### 3.4 P78 and S43 (il-analyst, before code; results in `internal/il-inventory.md`, chapter `## M4.5e1a`)

**P78 (IL, both modes, `build/m4.5e1a/p78/`; every claim with its `--il_display` excerpt):**
- **(a) Bounds.** Where the bound's written expression is kept, for `int a[3]`, `int a[2 * 3]`, an enumerator
  bound, a macro bound, `int a[65536u * 65536u + 1]` (P44: one element), `int a[] = {1, 2, 3}`, C++'s `const int
  N = 4; int a[N];` and the same in C (a VLA); EDG's diagnostics for `int a[0]` and `int a[-1]`.
  **Stop rule:** if a wrapped bound cannot be told from a written one, only bounds that `constant_value` accepts
  are taken and the rest are rejected; ask before anything wider.
- **(b) Initializers of automatic arrays.** `initk_static` or `initk_dynamic`; the element kinds; whether trailing
  zeros are materialized; `{0}`, `{}` (C23 and C++), `{[2] = 5}` and `{[2] = 5, 6}` (C), `int a[3]{1, 2, 3}`
  (C++), a non-constant element (`{x, y + 1, f()}`) and its order in the IL, a list of 1,001 constants (P38's
  shape for automatic storage), too many elements.
- **(c) Expressions.** `a[i]`, `i[a]`, `a[i] = v`, `a[i] += v`, `a[i]++`, `&a[i]`, `&a[3]`, `*a`, `int *p = a;`,
  `f(a)`, `p[i]` and `&p[i]` on a pointer local and a parameter, `a == p`, `if (a)`, `return a;`, `sizeof(a)`,
  `sizeof(a[0])`, `sizeof(a) / sizeof(a[0])` (folded: is an `expr` kept?); `CERASTES_INVARIANT(a[i] >= 0)` and
  `CERASTES_CHECK(a[0] == 1)`; whether `a.address_taken` is set by `a[i]` alone.
- **(d) Markers.** `[[cerastes::uninit]] int a[3];`, `owning` on `int *a[2]`, both positions.

**S43 (hand-written Viper in the target shapes; fixtures kept as `tests/viper-smoke/memory/s43_*.vpr`, each
positive with a failing twin; one file at a time through `scripts/run_tests.py`; times on a quiet machine):**
- **(a) The block.** `alloc_arr_int(n)` (3.5 item 3), element reads and writes with the explicit rows of item 4,
  the scope exit as marked range rows and an `exhale`. Twins: an index of `n` and of `-1` (`assertion.false`); a
  read of an unwritten element; a callee that keeps `owns(*p)` of one element, then the scope exit.
  **Gate G2:** after a failed marked range row Silicon reports nothing else on that path. If it does, the row's
  companion statement (the `exhale`, the invariant) needs a guard, and the record says which before code.
- **(b) The initializer.** `zero_arr_int(r, n)` then two stores; `a[3] == 0` holds; a twin asserts it is 1.
- **(c) The loop.** Item 8's form: the label, the entry and back-edge marked rows, the quantified conjuncts,
  values and flags kept through `old[lh]`. Programs: a fill loop; a read-only sum over an initialized array with a
  bound on the sum; a nested loop whose outer loop writes the array and whose inner loop only reads it (0042
  item 5's case); a loop that calls a `ref_to_uninit` callee on `&a[i]`, then reads the array after the loop; an
  array declared inside a loop body. Twins: an index one too far in the loop; a callee keeping an element in the
  body (the back-edge row); a read after the `ref_to_uninit` loop of an element it never filled. **Gate G3:** `old[label]` under a quantifier in an
  invariant is accepted by the pinned Silicon and keeps the values. **Stop rule:** if no variant verifies the fill
  and the read-only loops under 30 s each, stop and ask.
- **(d) Mixing (gate G4).** One method with a scalar cell (`alloc_int`), an array of `int` and a pointer
  parameter with `owns(*p)`: the existing shapes (the `perm` rows, `swap(&a[0], &a[1])` against the per-atom
  amounts of `e2_swap.vpr`, record 0038's scalar snapshots beside the array's conjuncts) still verify, and each
  twin fails with its own reason. Two more shapes: a callee with two `reads(*p)` atoms given `&a[i]` and `&a[j]`
  with symbolic indices (0037 item 8's fractions and case splits over a quantified chunk); and a heap block with
  its `forperm` leak row in the same method as an array. A `qp.not.injective` anywhere is a stop.
- **(e) Sequencing.** `assert loc(b, i) != loc(b, j)` holds exactly when `i != j`.
- **(f) One past the end of a heap block.** `q := loc(block_of(p), 1)` for a one-slot heap block `p`: with today's
  `holds_token(q)` an owner holding `q` exempts the block at `main`'s end (the leak is missed); with
  `offset_of(q) == 0` added the leak row fails.
- **(g) Vacuity.** `assert false` after `alloc_arr_int`, after `zero_arr_int` and at the end of each fixture
  method fails.
- **(h) Cost.** A method with three arrays and two loops; S26's scale program with one array added. Record the
  times; **over 60 s stops and asks**.

### 3.5 Record 0059 — arrays inside one function (written before code; revised after P78, S43 and audit round 1)

Amends 0028 items 4, 10 and 11, 0011 item 10, 0012 item 12, 0024 item 6, 0032 items 4 and 8, 0038 items 3 and 4,
0039 item 3, 0042 items 3 and 5, 0054 items 3 and 6.

1. **Scope:** 3.1, with its table of what stays rejected.
2. **Array objects.** A local `T a[N]` is one block of `len` N (0028 item 1); element `k` is `loc(b, k)`, in the
   value and flag fields of `T`'s cell type (0032 item 3). The variable's Viper local holds `loc(b, 0)`.
   - The bound is read through `constant_value` from its written expression (0024: checked as written), and is at
     least 1.
   - Rejected (`unsupported_declaration` or `unsupported_type`): the rows of 3.1; `static`, `extern` and
     thread storage; a `const`, `volatile` or `_Atomic` element type; `owning` or `ref_to_uninit` on the array.
3. **Allocation.** One trusted method per element cell type, tagged `// cerastes:trusted-contract`:
   `alloc_arr_<t>(n: Int) returns (r: Ref)`, `requires n >= 1`, ensuring `r != null`, `r == loc(block_of(r), 0)`,
   `len(block_of(r)) == n`, `lvloc(block_of(r)) == loc(block_of(r), -2)` with full permission to it, and, each as
   its own quantifier over the absolute slot with the trigger `{ loc(block_of(r), k) }` (0028 item 2), full
   permission to `v_<t>` and to `d_<t>` of every slot in `[0, n)` and every flag false.
   - It is called where the array's block is entered (0032 item 4).
   - **A braced initializer** runs at the declaration: `zero_arr_<t>(r, n)` (trusted, tagged: it takes and returns
     the same permissions and ensures every element zero, `false` or `null`, and every flag true), then one store
     per written element, in the IL's order. It is C's rule: elements without an initializer are initialized as
     static objects are. A list that covers all N elements skips the fill.
   - Every element's constant goes through `init_element_value` (P38: never `ck_aggregate`'s values directly).
     Non-constant elements follow the rule struct initializers already have (`collect_init_elems`).
   - Without an initializer the flags stay false: reading an element that was not written is `uninitialized`.
4. **Element places.** VIR gains `ElemAddr(base, index)`: the address of the element `index` slots after the
   address `base`, lowered to `loc(block_of(base), offset_of(base) + index)`.
   - `a[i]` is `Deref(ElemAddr(AddrOf(a), i))`; a write is a `Store` to it; `&a[i]` is the node as a value; the
     decay of `a` is `AddrOf(a)` with the element pointer type.
   - **Rows, in this order, each atomic** (0028 item 11): for a pointer base, `null-dereference` and `lifetime`
     (`ptr_use`); the bound, `out-of-bounds`: in place position `0 <= offset + i && offset + i < len`, in value
     position `<= len` (one past the end is formed, never accessed); then `permission` (`perm > none` for a read,
     `== write` for a write) and, for a read, `uninitialized`.
   - A named array in scope needs no null or liveness row.
   - Messages name the place as written: "reads `a[i]`, whose index may be outside `a`", "forms `&a[i]` beyond
     one past the end of `a`", "writes `p[i]` outside the object `p` points into".
   - `*p` keeps today's rows, so that every existing program's Viper is unchanged: a `*p` one past the end is
     reported as `permission`, since no permission exists at `len` (question 229; M4.5e2 decides the bounds row).
5. **`out-of-bounds`** joins the driver's categories in every copy of 0011 item 10's table, with its section in
   Error messages and a must-fail example.
6. **Scope exits** (0032 item 8). For an array: one marked range row per field, "the address of an element of `a`
   may still be in use when its lifetime ends" (`lifetime`), the liveness row as today, then the `exhale` of both
   quantified permissions and the liveness cell.
7. **Marked range rows** (0028 item 11; the owner's rule after S25).
   - **Shape:** `assert forall k: Int :: { loc(B, k) } lo <= k && k < hi ==> acc(loc(B, k).f[, amount])`, one
     field per row, `B`, `lo`, `hi` and the amount reading no heap.
   - **Map:** `low::Check` and `MapEntry` gain an expected reason. A marked row's second column is written
     `assert.failed[insufficient.permission]`; every other row is written as today, so no existing `.map` changes.
   - **Driver:** `map_errors` accepts on a marked row exactly `insufficient.permission`, mapped to the row's
     category, and on every other row exactly what it accepts today. `qp.not.injective` stays exit 3.
   - **Lint:** a row with an expected reason has exactly this shape; a `forall` appears only in the shapes this
     record lists (the two trusted contracts, marked rows, the range `exhale`, item 8's invariant conjuncts).
8. **Loops** (0038 items 3 and 4, for a block with more than one slot). A loop that touches memory takes each
   array in an open scope as a **range place**.
   - **Entry**, after the scalar snapshots: the label `lh_<loop>`; a marked row per field, "this loop begins
     without the permission to every element of `a`" (`permission`; it can fail only after a callee kept an
     element); the liveness snapshot as for a cell.
   - **Conjuncts** (`internal` rows): full permission to both fields over `[0, N)`; the flags kept, `forall k ::
     in range && old[lh](flag) ==> flag`; the type's range, `forall k :: in range && flag ==> lo_T <= v && v <=
     hi_T` (none for `bool`; a pointer element: `WF`); and, when no write in the loop can reach the array, the
     values kept, `forall k :: in range ==> v == old[lh](v)`.
   - The flag and range conjuncts are 0042 item 5's loop rows (question 89) over the range, and like them they go
     on every array the loop holds, reached or not: a `ref_to_uninit` callee in the body may set a flag, and an
     inner loop must not lose the outer loop's row. The record states, from 0042 item 5 and `loop`'s code, which
     scalar row each conjunct mirrors, before code.
   - **A write can reach an array** when the loop holds a store to one of its elements by name, a store through a
     pointer of its cell field, or a call whose `requires` owns a cell of that field (0038 item 3's rule). A store
     by name to another array or to a scalar cell does not reach it.
   - **Back edge:** a marked row per field, "an iteration of this loop ends without the permission to `a` that it
     began with" (`permission`), before Silicon's own invariant check.
   - **Invariant and measure reads** of `a[e]`: framed by the range place; an index outside it is
     `out-of-bounds` ("this loop invariant reads `a[e]`, whose index may be outside `a`"); the flag obligation is
     0038 item 6's (`uninitialized`).
   - **What e1a cannot state:** after a loop that fills an array declared without an initializer, a read is
     `uninitialized` (only a quantified invariant says which elements were written). It is pinned
     (`array_loop_fill_then_read_false_alarm.c`) and e1c gives it its verifying twin.
9. **Sequencing** (0028 item 12; 0039 item 3). Two unsequenced accesses to elements, at least one a write, get
   the `unsequenced` row that their addresses differ. Accesses that name one element as written (`a[i] = a[i]++`)
   are rejected like the compilers (`unsequenced_modification`); the test-mutator's matrix fixes the shapes.
10. **Pointers into arrays** (Part D). `p[i]` and `&p[i]` on any pointer use item 4's rows; `return a;` is
    `return_local_address` (R14); `if (a)` and `a == NULL` follow the compilers (the matrix).
    - **Interior pointers and owners** (handoff §3; S43 (f)): `holds_token` and the three `leak_exemption` terms
      gain `offset_of(p) == 0`, and 0054 item 3(e)'s flag clause is restated on `hb_w`'s cell. The Viper of every
      existing program with an owner changes in those rows, outcomes unchanged (Flips, D).
11. **`sizeof` and markers** (Part E). `sizeof(a)` of an array local is `N * sizeof(T)`, computed from the checked
    bound; type operands stay rejected (e1e). `cerastes::uninit` on an array local without an initializer is
    accepted (0042 item 3), and `--strict-init` asks for it as for a scalar (0042 item 8).
12. **The lint and the self-tests:** `alloc_arr_*` and `zero_arr_*` contracts; item 7; `old[label]` only in
    item 8's conjuncts; element flag writes through a `Ref` temporary. **Categories:** one new.
13. **Facts:** P78, S43. **Still rejected:** 3.1's table.

### 3.6 Parts and commits

| Part | Content | `src/` estimate |
|---|---|---|
| A | plumbing, behaviour-neutral | 300–450 |
| B | arrays in straight-line code | 1,000–1,500 |
| C | loops and invariants | 450–750 |
| D | pointers into arrays, the `offset_of == 0` restatements | 350–600 |
| E | `sizeof(a)`, `uninit` on arrays | 80–150 |

1. **Commit 1:** record 0059 (draft), P78 and S43 in `il-inventory.md` with the fixtures. Audit round 1 on it.
2. **Part A** (vprdiff: every `.vpr` and `.map` identical).
   - **il-analyst:** `vir.h`/`vir.cpp`: `Type::Kind::Array` (element type and count, interned by
     `Program::array_of` like `pointer_to`), `Expr::Kind::ElemAddr` with its builder and `place_text` (`a[i]`);
     `check.cpp`: both accepted nowhere yet (an `rc` case each). `lowered.h`: `LExpr::Kind::Forall` (one `Int`
     binder, triggers, body), a label on `Old`, `Check::reason`; `emit.cpp`; `posmap.cpp` (item 7's column).
   - **edg-backend:** `report.h`/`report.cpp`: `MapRow`'s expected reason, `parse_position_map`, the gate in
     `map_errors`; `src/driver/selftest.cpp` cases for a marked row that fails with its reason, with another
     reason, and an unmarked row with `insufficient.permission` (exit 3).
   - **test-mutator:** `lint_viper.py`: item 7's and item 12's rules with twins in `test_lint_viper.py`. The
     category is not added here: its self-test compares every copy, and the page's example needs Part B.
3. **Part B.** Its commit holds `out-of-bounds` in every copy at once: `DRIVER_CATEGORIES` and its two mirrors in
   `test_run_tests.py`, `tests/README.md`'s table, record 0011 item 10's table, and the Error messages section
   with its must-fail example (test-mutator; the record and the page's prose are the orchestrator's).
   - **edg-backend (`E`):** `vtype`'s `tk_array` arm (through `vtype_object` only; signatures, globals and struct
     members keep rejecting, re-messaged); the bound through `constant_value`; `computed_type_reason` no longer
     refuses a local's own array type. `declaration`: an array local is always a cell (12964–12968), its storage
     and qualifier rejections; `array_initializer` beside `struct_initializer`, on `collect_init_elems` (array
     designators) and `init_element_value`. `operation`: `eok_subscript` over `eok_array_to_pointer` of a local
     array; `place_address`, `address_of`, `indirect_read`, `assign_target`, `assignment_parts`; the shape
     matchers of 3.2. Invariant and check reads (`contract_place_address`, `contract_read`). `diagnostics.def`'s
     `unsupported_type` text.
   - **il-analyst (`L`, `check.cpp`, self-tests):** `alloc_arr_<t>` and `zero_arr_<t>` in `alloc_methods`;
     `alloc_cell`; `ElemAddr` in `lower_node` with item 4's rows in `deref`, `store` and `cell_write`; the array
     arm of `scope_exit`; the sequencing keys; every kind switch of 3.2; `check.cpp` accepts arrays as locals
     and `ElemAddr` over a local array. VIR cases in `selftest_cases28.inc`; `VIR_CASES_MIN` raised.
   - **Staging:** a loop while an array is in an open scope, and `eok_array_to_pointer` outside a subscript, are
     rejected until Parts C and D (two pins, deleted by those parts).
   - **test-mutator:** 3.7's Part B tests; Flips A and B.
4. **Part C** (after Part B's commit). **A fresh il-analyst starts here**, briefed with this plan, record 0059,
   S43's chapter and Part B's commit; the first one has carried P78, S43 and Part B. It holds `lower.cpp` for
   Parts C and D.
   - **il-analyst:** the range `Place` in `loop_places`; item 8 in `loop` (entry rows, label, conjuncts, back-edge
     rows); `inv_cell`/`inv_deref`/`framing_condition` for `a[e]`; the reach rule.
   - **edg-backend:** element reads in `encode_loop_clauses`; lifts the loop staging.
5. **Part D.**
   - **edg-backend:** decay as a value, `eok_subscript` over any pointer, `&p[i]`; `returned_local_address`'s
     decay case.
   - **il-analyst, then edg-backend's rebuild:** item 10's `offset_of == 0` restatements and the `hb_w` clause.
     The allow-list is made before the build: `grep -l` over `build/m4.5e1a/baseline/vpr` for the token
     permission in a `held_` assignment or a leak row. Time the six slowest default-tier tests of handoff §3
     before and after, on the box.
   - **test-mutator:** the lint rules that pin the changed rows (`lint_owner_loop_ghosts`, `lint_carrier_clauses`,
     `lint_heap_token`, `lint_owning_global_tags`: whichever read `holds_token`'s shape) follow them, each with
     its self-test, in the same commit; the lint runs over a fresh regeneration before the commit (M4.5d2b2b's
     first box run was stopped by exactly this).
6. **Part E** (edg-backend): item 11.
7. **Then:** the audits' fixes; docs, status, handoff, `plan.md`'s Done row, record 0059 accepted.

**Order of work.** P78, S43 and the Step-0 runs run while the orchestrator drafts the record and edg-backend and
the test-mutator build Part A's driver, lint and harness sides. Part B's two halves are built in parallel against
Part A's VIR; the test-mutator writes Part B's tests from the record, expectations pre-registered.

**Audits:** round 1 on the record draft with P78 and S43; round 2 on Parts A and B together (can an unmarked row
pass with `insufficient.permission`; can a marked row hide another reason; is any array construct accepted
outside the record), on Part C, on Part D; one re-audit.

**Pre-agreed cuts, in this order:** Part E (to e1e); Part D (to e1b, whole, with item 10). Parts B and C are not
cut without asking. **Split points** (M4.5d2b2c and M4.5d2b2d each landed about twice their estimate): at Part
B's commit, report its `src/` lines against 3.6's table, and stop and ask if they passed 1,500 or the context will
not carry Part C; at Part C's commit, if the context will not carry Part D, stop and ask.

### 3.7 Tests (`tests/memory/array_*.c`, `.c` in `c23, c++20`; each positive has a twin failing for one reason at one line)

- **Part B.**
  - `int a[3] = {1, 2, 3};` returning `a[0] + a[2]` with `ensures(r == 4)`. Twins: `r == 5` (`postcondition`);
    `a[3]` and `a[-1]` (`out-of-bounds`).
  - A write to `a[1]` keeps `a[0]` (a `check`). Twin: the wrong value (`check`).
  - `a[i]` under `requires(0 <= i && i < 3)`. Twins: `i <= 3`; `-1 <= i` (`out-of-bounds` at the access).
  - No initializer: write then read one element. Twin: read another (`uninitialized`).
  - `{1}` over four elements: `a[3] == 0`. Twin: `a[3] == 1` (`check`). `{}`; `{[2] = 5}` (C); `int a[] = {…}`.
  - Element types: `bool`, `long`, `unsigned char`, an enumeration, `int *` (`*ps[0] = 1;`).
  - `a[i] += 1`, `a[i]++` at a boundary value. Twin: `overflow`.
  - `set(&a[1], 5)` with `owns(*p)`; `swap(&a[0], &a[1])`. Twins: `swap(&a[0], &a[0])` (`permission`); a callee
    that keeps the element (`lifetime` at the scope exit); `&a[4]` (`out-of-bounds`); `int *e = &a[3]; *e = 1;`
    (`permission`, item 4).
  - `a[i] = a[j]++` under `requires(i != j)`. Twin: without it (`unsequenced`). The same-element shapes the
    matrix finds (`rejected`).
  - A bound that wraps as written (`int a[65536u * 65536u + 1]`): what P78 (a) and 0024 give, pre-registered in
    the record.
  - Every rejection of 3.1's table and of item 2, one pin each.
- **Part C.**
  - A fill loop over an initialized array. Twin: `i <= N` (`out-of-bounds`).
  - A read-only loop with a bound on the sum, the values kept. Twin: the loop writes the array first
    (`invariant-preserved`).
  - An invariant reading `a[i]`. Twins: an index outside (`out-of-bounds`); an unwritten element
    (`uninitialized`).
  - A callee keeping an element in the body (`permission` at the back edge); an array declared in the body; a
    `break` and a `return` inside the loop; a nested loop.
  - The pin `negative/array_loop_fill_then_read_false_alarm.c` (`uninitialized`), naming e1c.
- **Part D.**
  - `int *p = a; p[2] = 7;` then `check(a[2] == 7)`. Twin: `p[3]` (`out-of-bounds`).
  - `fill_one(a)` with `owns(*p)`. `p[1]` on a parameter with only `owns(*p)` (`out-of-bounds`).
  - `&p[1]` of a one-object heap block stored into an owning global: `leak` at `main`'s end (S43 (f)); `free(&p[1])`
    (`invalid-free`).
  - `return a;` (`rejected return_local_address`).
- **Part E.** `sizeof(a) / sizeof(a[0])` as a loop bound; `uninit` on an array under `--strict-init`. Twin: without
  the marker (`rejected uninit_unmarked`).
- **Byte identity:** `scripts/vprdiff.py` against the baseline after each part; only Flips D differs.

### 3.8 Flips (pre-approved with this plan; any other changed outcome stops the session)

- **A. Retired for a positive of the same program, with a twin:** `tests/scalar/negative/reject_array.c`
  (`rejected cerastes-unsupported_declaration @ L9`) becomes `tests/memory/array_local_unused.c`.
- **B. One line goes:** `tests/scalar/negative/reject_range_for.cpp` keeps `rejected
  cerastes-unsupported_statement @ L13`; its `@ L12` line (the array) becomes a comment, so the line numbers stay.
- **C. Stay as written, message and `Pins:` re-pointed where the text names M4.5e1:**
  `tests/memory/negative/global_array.c`, `reject_struct_array_member.c` (e1e), `reject_struct_flexible_member.c`
  and `tests/scalar/negative/reject_vla.c` (M8), `reject_owns_range_pointer.cpp` and `reject_owning_count.c`
  (e1b), `reject_forall_pointer.cpp` (M7; the C form e1c), `reject_owning_count_local.c`,
  `reject_owning_field_count.c`, `reject_owning_global_count.c`, `reject_malloc_size_*.c`,
  `reject_calloc_size_count.c`, `reject_realloc.c`, `reject_struct_member_arith.c`, `global_address_arith_cpp.cpp`
  (M4.5e2), and record 0024 item 6's and 0018 item 10's pins in `tests/scalar/negative`
  (`reject_sizeof_*array*`, `reject_typedef_array_*`, `reject_alias_array.cpp`, `reject_alignof_array.c`,
  `reject_datasizeof_array.cpp`, `reject_decltype_array.cpp`, `reject_enum*_array*`,
  `reject_static_assert_sizeof_array.c`, `reject_contract_sizeof_array.c`, `reject_skipped_*array*`,
  `reject_sizeof_compound_literal.c`, `reject_sizeof_string.c`: e1e, M4.6).
- **D. Viper changes with the same outcome:** the programs of Part D's allow-list (item 10's rows), and nothing
  else.

### 3.9 Docs (invariant 8; every example is a test)

- **`memory.adoc`:** a section "Arrays" (a local array, indexing and its checks, initializers, pointers to
  elements, one past the end, loops and what needs a quantified invariant); the not-yet sentences at 465, 537,
  825–826 and the list at 1516–1533.
- **`error-messages.adoc`:** `out-of-bounds`, with a must-fail example. **`verifying.adoc`:** 1199 and 1223–1229
  (arrays and `sizeof`). **`annotations.adoc`:** unchanged but for `uninit` on an array.
- **`tests/README.md`:** the category table (409–438). **The developer guide:** the marked range rows in the
  architecture page's map description.
- **`internal/encoding.md`:** §3 "Arrays (M4.5e1a)", the map format (916), the prelude methods (892), the rejected
  lists (1022, 1254–1256). **`internal/il-inventory.md`:** chapter `## M4.5e1a`. Records 0011, 0012, 0024, 0028,
  0032, 0038, 0039, 0042, 0054: one amendment line each.
- No tutorial chapter yet: e1c completes the concept (0049 item 4).

### 3.10 Stop and ask; acceptance

**Stop and ask:** any agent question; any changed outcome outside 3.8; any existing program's Viper changing
outside Flips D; a gate (G2–G4, P78 (a)'s stop rule) or a spike that contradicts items 2–10; a cut beyond the
pre-agreed ones; the split point; an exit 3 or a time over 120 s not fixed within its part.

**Acceptance** (as M4.5d2b2d's, with `build/m4.5e1a/`):
- Before the box run: `scripts/regen_vpr.py` over the final build, `scripts/lint_viper.py` over its output and a
  fresh VIR self-test directory, the session's VIR cases through Silicon, R3's run against 3.8.
- `CERASTES_LONG=1 scripts/check.sh` once, after the last code change, on the x86-64 box (the branch as a git
  bundle into `~/cerastes-s8/p-ubuntu-26.04`): green.
- `python3 scripts/vprdiff.py build/m4.5e1a/baseline/vpr build/vpr --allow <Flips D>`: nothing else differs.
- The VIR self-test at the new `VIR_CASES_MIN`; `test_lint_viper.py`; `test_run_tests.py` (the category self-test
  names `out-of-bounds` in every copy); `scripts/format.sh --check`; `scripts/build_docs.sh`.
- The status entry and the handoff with its Questions section; e1b's as-built inputs (section 4.5) written into
  the handoff. Commits stay local.

## 4. M4.5e1b — Ranges in contracts (the next free record) — ⟲ plan mode again after P79 and S44

**Scope.** `cerastes_owns_range(p, n)` and `cerastes_reads_range(p, n)` (question 228) with the C++
`cerastes::` forms; array parameters; a range atom in `requires`, `ensures` and `CERASTES_INVARIANT`; calls,
returns and entry facts over ranges; `f__wf`; loops over the ranges of parameters; `old(p[i])`; P4222's markers
over a range; `owning(n)` on a parameter; the `carrays.c` port without ghost lists. Part D of e1a, if it was cut.

### 4.1 Decided already (records 0028, 0029, 0037; the fixtures)

- **Meaning** (0028 items 2 and 7, 0037 item 6): `p != null`, `WF(p)`, `0 <= n`, `offset_of(p) + n <=
  len(block_of(p))`, then one quantifier per field over the absolute slot, `offset_of(p) <= k && k < offset_of(p)
  + n`, trigger `{ loc(block_of(p), k) }`: the value and flag permissions (full for `owns_range`, the atom's ghost
  amount for `reads_range`), the flags set, the type's range, and a fraction of the block's liveness cell.
  `e2_array_sum.vpr`'s `OWNR` and `INITR` are this shape.
- **Explicit rows** (0028 item 11): a permission check over a range is a marked row (e1a's item 7); the flags and
  the bounds are pure rows (`assertion.false`). Entry separation is asserted for single places only.
- **Positions** are an atom's (0037 item 5). `n` reads no heap and, in an `ensures`, means its entry value.
- **Spellings:** C23 variadic declarations in both modes; the names join `spec_fn` (`E` 7076–7118, where
  `owns_range` and `forall` are "unsupported" today), `is_spec_function_name` (`il_query.cpp:54-64`),
  `is_spec_name` (`annotation_checks.cpp:175-180`) and the protected names (1636–1662).

### 4.2 Proposed, for the re-plan to fix

- **The ghost amount of `reads_range`.** `perm` is refused under a quantifier, so the caller's amount comes from
  one representative cell (`perm` of the range's first slot when `n > 0`), and the marked row then checks the
  whole range at that amount. A caller holding unequal fractions over the range gets a false `permission`; the
  record lists it.
- **Calls:** per range atom, under its guard: null, `WF`, the bound `offset + n <= len` (`permission`, "calls `f`,
  which needs to own `p[0..n)` …"), the marked rows, the flags row (`uninitialized`); separation of a range from
  every other atom of its field (`permission`: a block and an overlap test, 0028 item 13's form); then the
  amounts, the value `requires`, the call.
- **Returns:** the separation rows, then each range atom's marked rows and flags row (`permission`), before
  Silicon's postcondition check.
- **`f__wf`:** a read `p[e]` in a contract is framed by a range atom when the block is the atom's and the slot is
  in its interval, or by a cell atom as today; one ghost value formal per written read, with `e1 == e2 ==> h1 ==
  h2` between two reads of one base.
- **Loops:** the range of a parameter the function never assigns is an automatic range place (e1a's item 8) at
  the atom's amount; a pointer the function reassigns writes the atom in its invariant (0040's written atoms).
- **Sub-ranges at calls:** `f(a, k); g(&a[k], n - k)` (Silicon splits and merges the quantified chunk).
- **Array parameters:** `int a[]` and `int a[N]` are pointer parameters (R1); `[static N]` is rejected
  (question 230).
- **Null and empty:** `owns_range(p, n)` implies `p != NULL`, as `owns(*p)` does; a function that accepts
  `(NULL, 0)` writes `n == 0 || owns_range(p, n)` (question 231).
- **Markers:** `ref_to_uninit` drops and `must_init` adds the flags over the range of a parameter's atom.
- **`owning(n)`** on a parameter is `p == NULL || owns_range(p, n)` in both clause lists, with
  `CERASTES_OWNING_N(n)`; the other targets stay rejected (M4.5e2).
- **`restrict`** beside range atoms: 0037 item 2's conditions with a range in the atom's place, or a pin.
- **`old`:** `cerastes_old(p[i])` is the element's entry value; `old` of a whole array stays rejected (0029 item 3).

### 4.3 P79 and S44 (run before the re-plan)

- **P79 (IL, both modes):** `cerastes_owns_range(p, n)` and `cerastes_reads_range(p, n)` (the default argument
  promotions on `n`; the C++ template's conversion of `n` to `size_t`, which must be read as written, 0022);
  `int a[]`, `int a[3]`, `int a[static 3]`, `int a[const 3]` parameters with a leading marker; `owning(n)` on a
  parameter in both positions; `cerastes_old(p[i])` and `cerastes::old(p[i])`; `p[i]` in a `requires`, an
  `ensures` and a `decreases`.
- **S44 (hand-written Viper, fixtures `s44_*`, each with a twin):**
  - a callee with `owns_range` called on a local array, on a sub-range, twice in a row; a caller holding too
    short a range (the marked row), an unwritten element (the flags row);
  - `reads_range` with the per-atom amount from a representative cell: called with `(a, a)`, with a fraction the
    caller itself borrowed, and from a loop;
  - a range and a cell of one block in one contract (`owns_range(a, n)` beside `owns(*p)`), the separation row
    and its twin;
  - returns: a function that drops one element (the marked row first, Silicon's postcondition never);
  - the loop over a parameter's range at a ghost amount, values kept, with a `reads_range` sum bounded by a
    conjunction of single-element facts;
  - `f__wf` with indexed reads as ghost formals;
  - `must_init` over a range (a filler) and its twin that leaves one element.
  - **Stop rules:** a correct shape that fails, a second Silicon error after a marked row, `qp.not.injective`, or
    any program over 60 s: stop and ask, and the re-plan starts from that result.

### 4.4 Known pins, tests and docs

- **Flips to pre-approve at the re-plan:** `tests/memory/negative/reject_owning_count.c` (retired for a
  positive); `reject_owns_range_pointer.cpp` stays `rejected` (its pointee is `const`: 0040), re-messaged, with a
  `reads_range` positive beside it; the staging pins of e1a's atoms on elements.
- **Ports** (`tests/memory/verifast/carrays_*.c`, with line maps to `~/Documents/verifast/examples/carrays.c`):
  `array_test1`; `array_test_fail` (`a[-1]`, L35) and `array_test_fail2` (`a[5]`, L42), both `out-of-bounds`;
  `array_test2` and `to_verify` with single-element facts (`a[0] == 10 && a[1] == cerastes_old(a[1])`); their
  full list contracts are e1c's.
- **Docs:** `annotations.adoc` 834–859 (`owns_range`, `reads_range`), 502 (`owning(n)`); `memory.adoc` "Arrays"
  gains ranges in contracts and array parameters; Error messages; `include/cerastes.h`.

### 4.5 Inputs the re-plan takes

S44's results; P79; R1; e1a's as-built `ElemAddr`, range `Place` and marked-row helpers (names and lines); whether
e1a cut Part D; the times of S43 (h).

## 5. M4.5e1c — Quantifiers (the next free record) — ⟲ plan mode again after P80 and S45

**Scope** (question 226). `cerastes_forall(f, free...)` in `requires`, `ensures`, `CERASTES_INVARIANT` and
`CERASTES_CHECK`; max; the fill loop of e1a's pin; the rest of the `carrays.c` port; the tutorial chapter "Arrays
and quantifiers". The C++ lambda form stays M7's (`reject_forall_pointer.cpp` keeps its line).

### 5.1 Decided

- `f` names a `pure` function with a body that is one `return E;`; its first parameter is the bound variable and
  the others receive the trailing arguments in order (0029 item 4; P53: `enk_routine` in C23, a constant address
  in C++20). The quantifier means `E` over every value of the first parameter's type.
- `E` is substituted at the site; no Viper function is emitted for `f` here, and a call of `f` stays rejected.

### 5.2 Proposed, for the re-plan to fix

- **The pure body reader:** finds the one `return E;`, refuses anything else (`unsupported_annotation_expression`),
  and encodes `E` in contract mode with the parameters bound. e1d reuses it.
- **The binder and triggers** (0028 item 2; S24). The Viper binder is the absolute slot of the first element read
  that is indexed by the bound variable plus a constant (`p[i]`, `p[i + 1]`), with the trigger `{ loc(block_of(p),
  k) }`; the other reads are rewritten over it. A body with no such read, or an index with another coefficient
  (`a[2 * i]`), is rejected and pinned.
- **Arity and types:** a count or type mismatch between `f`'s parameters and the arguments is rejected (0006
  item 9 left it to the session that gives `forall` its meaning).
- **Framing and well-formedness under the binder:** every read of `E` must be framed, and every operation of `E`
  defined (0010 item 11), for every value of the bound variable that reaches it. `f__wf` reads no heap today
  (0037 item 7), so a read under a binder needs a ghost that depends on the binder: S45 decides between a ghost
  `Map[Int, T]` formal per read and a `f__wf` that holds the range permissions. **The choice binds e1d:** a
  heap-free `f__wf` cannot evaluate a call of a heap-reading `pure` function (`r == sum_to(a, n)` in an
  `ensures`), so S45 chooses with that shape in view and says what e1d would need under each variant.
- **Flags:** a read under the binder states that the element holds a value (0038 item 6), so a quantified
  invariant can say which elements a loop has written; that lifts e1a's pin.
- **`old` under the binder** (question 232): `cerastes_old(p[i])` is allowed in the body of a `pure` function
  that only an `ensures` names; `array_test2`'s `update(0, 10, vs)` and `to_verify`'s lists need it.
- **Max:** `max` returns the value with `forall i: a[i] <= r`; `max_index` returns `r` with `0 <= r && r < n` and
  `forall i: a[i] <= a[r]` (there is no `exists`).

### 5.3 P80 and S45 (run before the re-plan)

- **P80:** `cerastes_forall(f, a, n)` and `cerastes::forall(f, a, n)` in every annotation kind, with `f` and
  `&f`; the IL of a `pure` function's body (the `return`'s expression, its parameter references) and what
  `annotation_checks.cpp` already enforces on it; a mismatch of count and of type; `cerastes_old` inside a `pure`
  body.
- **S45:** the absolute-slot rewriting for one array, for `a[i] <= a[i + 1]`, and for two arrays (`a[i] ==
  b[i]`); the fill loop with a quantified invariant; max; a copy loop; `forall` with `old`; both variants of
  `f__wf` under a binder; an array of 4 and a symbolic `n`. **Stop rules:** a matching loop (a time over 60 s), a
  correct shape that fails, or neither `f__wf` variant holding: stop and ask.

### 5.4 Tests and docs

- Positives with twins: a quantified `requires` used at a read; a quantified `ensures` proved by a loop; the fill
  loop (the pin's twin); max and `max_index` (twins: an off-by-one range, `<` for `<=`); sortedness; the
  `carrays.c` functions in full; every rejection of 5.2.
- Docs: `annotations.adoc` (`forall`); `memory.adoc` "Arrays"; the tutorial chapter with max and the fill loop,
  its `console` blocks copied from runs.

## 6. M4.5e1d — Pure functions and the exact sum (the next free record) — ⟲ plan mode again after P81 and S46

**Scope** (question 227): the slice of M4.5f1/f2 that an exact sum needs. M4.5f1 keeps predicates, `fold` and
`unfold`, and pure functions over predicates.

### 6.1 Proposed, for the re-plan to fix

- **A `pure` function with a body `return E;` becomes a Viper function** with that body: its `requires` (values,
  and `reads(*p)` or `reads_range(p, n)` for what it reads; `owns` atoms rejected), an optional `ensures`, and
  `decreases` when it is recursive (0013: required in a cycle).
- **Calls of `pure` functions** are accepted in annotations and in `pure` bodies (0011 item 7's
  `call_in_annotation` lifted for them), each with explicit rows for the callee's `requires`; a call from code
  stays rejected.
- **The body's obligations** (overflow, division, bounds, framing, flags: 0010 item 11) are checked once, in a
  definition-check method beside `f__wf`, under the function's `requires`; the Viper function holds only the
  meaning. Viper proves the `ensures` and the termination measure.
- **The exact sum:**
  ```c
  CERASTES_PURE int sum_to(const int *a, int n)
      CERASTES_REQUIRES(0 <= n && n <= 1000 && cerastes_reads_range(a, n) && cerastes_forall(small_at, a, n))
      CERASTES_ENSURES(r, -1000 * n <= r && r <= 1000 * n)
      CERASTES_DECREASES(n)
  {
      return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
  }
  ```
  and `sum` with the invariant `s == sum_to(a, i)` and `ensures(r, r == sum_to(a, n))`.
- **Stays rejected:** a `pure` body that is not one `return`; mutual recursion; a `pure` function returning a
  pointer or a structure; predicates (M4.5f1).

### 6.2 P81 and S46 (run before the re-plan)

- **P81:** the IL of a call of a `pure` function in each annotation kind and inside a `pure` body; a contract and
  a `decreases` on a `pure` function; recursion; today's diagnostics for each.
- **S46:** a heap-reading Viper function with a quantified read permission in its precondition (what the pinned
  Silicon does with the amount); the recursive `sum_to` with its `decreases` and bounds `ensures`; `sum`'s loop
  (does Silicon unfold `sum_to(a, i + 1)` unaided, or does the loop need a `check`); a function applied under a
  `forall`; the definition-check method; the contract's well-formedness method with `r == sum_to(a, n)` in an
  `ensures`, under the `f__wf` variant e1c chose; a twin whose body overflows; a non-terminating twin. **Stop rules:** the
  loop not verifying without a lemma (then the record says what the user writes, or the owner decides); any
  program over 60 s.

### 6.3 Tests and docs

- The exact sum with twins (an off-by-one invariant, a dropped bound: `overflow`); a non-recursive `pure`
  function used in a contract; every rejection.
- Docs: `annotations.adoc` (`pure`); the tutorial chapter gains the exact sum; handoff §3's M4.5f1 notes that this
  consumes (R1–R13 in `pure` bodies, `static_assert_pure_body.c`).

## 7. M4.5e1e — Arrays in aggregates and storage (the next free record) — ⟲ plan mode again after P82 and S47

**Scope.** Array members (`member(r, k)` of length n), arrays of structures, arrays of arrays (a row is
`member(loc(b, i), 0)`; 0028 item 16), global and `static` arrays, array compound literals, array types in
`sizeof(type)`, `typeof`, `_Alignof` and typedefs (0024 item 6 lifted for arrays), structure copies with array
members, range atoms and `forall` over them. Its planning may split it in two (aggregates; storage): question 233.

### 7.1 Decided and proposed

- **Decided** (0028 items 2 and 16; S26): over an array of structures there is one quantifier per member and
  field, triggered `{ loc(member(loc(block_of(a), k), j), 0) }`; a matrix's contract fixes `offset_of(a) == 0`;
  `a[0][3]` and `a[2][0]` fail their bounds; `&a[0][3]` is formed.
- **Globals:** `len(GB_g()) == N` in `global_facts` (`L` 6277–6294) and the lint (`global_facts_atoms`);
  `program_start` gives the initial values, zero as one quantifier, written elements through `constant_value`
  (P38: a list over 1,000 elements keeps no `expr`, and that shape is rejected, never read from `ck_aggregate`);
  `owns(g)` and `reads(g)` of an array global mean every element. **`is_global_ref` and `inv_place_disjunct`
  (`L` 6267–6271, 3809–3817) treat any `loc(GB_…, i)` as one place: wrong once a global has several slots.**
- **Escape scan** (0056 item 11): an array member decaying to a pointer is an escape of its struct local
  (`escapes_in_expr`, `L` 2930).
- **Stay rejected, to be confirmed:** an owning field inside an array of structures and arrays of owning pointers
  (closures over ranges: M4.8 or M4.5f2); a by-value structure with an array member, unless S47 shows the
  quantified copy is cheap; `const` arrays (M4.6).

### 7.2 P82 and S47 (run before the re-plan)

- **P82:** array members and their initializers (brace elision), `s.a[i]`, `p->a[i]`, `&s.a[i]`, the decay of
  `s.a`; `struct S v[3]`, `v[i].x`; `int m[2][3]`, `m[i][j]`, `m[i]` as a value; global and `static` arrays with
  and without initializers, a 1,001-element list; `(int[]){1, 2, 3}`; `sizeof(int[3])`, a typedef of an array
  type and its uses.
- **S47:** S26's member fixtures re-run in the encoder's shapes with e1a's loops and e1b's atoms; a global array
  through `program_start` with a loop in `main`; a structure copy with an array member; costs. **Stop rule:** any
  program over 60 s.

### 7.3 Known pins

`global_array.c`, `reject_struct_array_member.c`, the array compound literal messages (`E` 9663, 9735), and
3.8 C's list from records 0024 and 0018; `reject_struct_flexible_member.c` stays (M8).

## 8. Open questions (handoff §8, each with its recommendation; answer inline before the sub-session named)

- **229 (e1a)** `*p` one past the end is reported as `permission`, because `*p` gets no bounds row and every
  existing program's Viper stays as it is. **Recommended:** as planned; M4.5e2 decides the row with `p + i`.
- **230 (e1b)** `int a[static N]`. **Recommended:** rejected, an M8 entry (C makes a shorter argument undefined,
  and no contract states it); if R1 shows it is accepted today, e1a rejects it at once.
- **231 (e1b)** `owns_range(p, n)` implies `p != NULL`. **Recommended:** yes, as `owns(*p)`; `(NULL, 0)` is
  written `n == 0 || owns_range(p, n)`.
- **232 (e1c)** `cerastes_old(p[i])` in the body of a `pure` function that only an `ensures` names.
  **Recommended:** accept.
- **233 (e1e)** its planning may split it in two. **Recommended:** decide there, from its measured sizes.

## 9. Inputs (from handoff §3 and §4.2 at this planning; `git show f57ee43:internal/handoff.md`)

- P38's braced initializers: a list of more than 1,000 elements keeps no `expr` for its elements
  (`REDUCE_BACKING_EXPRESSION_USE`), only an `orig_type`; read every element through `constant_value`, which
  rejects that shape, never through `ck_aggregate`'s values directly, or a wrapping element would pass unchecked.
  [M4.4a, record 0022 item 3]
- The `held_v`/`held_g` updates and `main`'s exemption need `offset_of == 0` once interior pointers exist, and
  the flag clause of 0054 item 3(e) is restated on `hb_w`'s cell. [d2b2a audit round 1] (e1a, item 10)
- Record 0056 item 11's escape scan must see an array member decaying to a pointer as an escape of the struct
  local. [d2b2c re-audit] (e1e)
- The marked range rows in the driver (owner, after S25). (e1a, item 7)
- M4.4b2's types computed from a constant (`sizeof`, `typeof`, typedefs). (e1a for a local's own type; e1e)
- The port `carrays.c` (lists become `forall`; its `should_fail` lines L35 and L42: below, and one past the end).
  (e1b, e1c)
- **The survey's contradictions, settled here:** `out-of-bounds` lands in e1a (0028 item 10), not M4.5e2;
  `reject_forall_pointer.cpp` is the C++ lambda form (M7), not e1's; 0024's "until M4.5e" also names `_BitInt`,
  vectors and non-type template arguments, which stay rejected; "M4.5e" in 0022 and 0047 for `n * sizeof(T)`,
  `calloc(n, …)` and `realloc` is M4.5e2.
