# Plan: M4.5e1b — Ranges in contracts (record 0060)

**Approved by the owner on 2026-11-11**, at a session that only planned. It supersedes section 4 of
`.claude/plans/m4-5e1-arrays.md` ("plan mode again after P79 and S44"): **e1b does not plan again.** Its probes
and spikes run first (section 5), each with a decision table that says what is built for each outcome; an outcome
outside its table stops the session and asks the owner.

**Written for this ensemble** (owner, 2026-11-11): an Opus orchestrator; three Sonnet implementers from
`.claude/agents/` (`edg-backend`, `il-analyst`, `test-mutator`); the Opus `soundness-auditor`; a Fable advisor the
orchestrator consults. Section 2 assigns every task; the orchestrator does not re-divide the work.

Branch `m4.5e1b-ranges-in-contracts`, worktree `.claude/worktrees/m4.5e1b`, `build/m4.5e1b/`, record
`internal/decisions/0060-ranges-in-contracts.md`.

## 1. Context

**Why.** e1a gave arrays that live inside one function. A function still cannot say that it owns or reads
`p[0..n)`: `owns_range` is rejected, an element cannot be read in a contract, and `p[i]` of a pointer cannot be
read in a loop invariant. e1b lets ranges cross calls, as M4.5b1b did for single cells (record 0037).

**Owner decisions.**
- **228** `cerastes_reads_range(p, n)` beside `owns_range`. **230** `int a[static N]` stays rejected (M8).
  **231** `owns_range(p, n)` and `reads_range(p, n)` imply `p != NULL`; `(NULL, 0)` is written
  `n == 0 || owns_range(p, n)`.
- **236** (this planning) an atom on one element, `owns(p[i])`, is **M4.5e2's**, with `owns(*(p + i))`.
- **237** (this planning) a range atom written in a loop invariant is **M4.5e2's**: until pointer arithmetic and
  heap arrays every range a loop can reach is an automatic place.
- **Standing:** 69 (no EDG work), 77 (one context), 160 (a new test's expect line may be corrected after its
  first run, each listed in status), 176 (the owner handles CI); `check.sh` once, at the end, on the x86-64 box;
  no test runs while a fix is queued; no Claude trailer; commits stay local.

**Scope.**
- **In:** `cerastes_owns_range(p, n)`, `cerastes_reads_range(p, n)` and the C++ `cerastes::` forms, as atoms in a
  function's `requires` and `ensures`, `p` a pointer parameter; element reads `p[e]` in `requires`, `ensures` and
  a function's `decreases`; `cerastes_old(p[e])`; calls and returns over ranges; `f__wf`; loops over the ranges
  of parameters (automatic range places); `p[e]` of any pointer and an array used as a pointer in loop invariants
  and measures; P4222's markers on a parameter whose atom is a range; `owning(n)` on a parameter with
  `CERASTES_OWNING_N(n)`; `restrict` backed by a range; array parameters (`T a[]`, `T a[N]`) tested and
  documented, `sizeof` of one rejected; the `carrays.c` port with single-element facts.
- **Not here** (rejected, each pinned, the message naming where it is lifted):

| Construct | Lifted by |
|---|---|
| an atom on one element (`owns(p[i])`, `reads(a[i])`); `&p[e]` and `p + i` in a contract | M4.5e2 (236) |
| a range atom in a loop invariant; a loop in a function that assigns its range parameter (a false alarm) | M4.5e2 (237) |
| a range whose base is not a pointer parameter (`result`, a global, a member, a loaded pointer); a count that reads memory; a partnerless `ensures` range; an `ensures` range with another count than its `requires` partner; `owning(n)` on results, locals, fields and globals | M4.5e2 (members: e1e) |
| `owns_range` and `reads_range` on one parameter in one clause list; a range atom beside a cell atom of the same parameter | M4.8 |
| `cerastes_forall`; a `must_init` filler that fills with a loop (a false alarm); the full lists of `carrays.c` | e1c |
| ranges over structures, array members, global arrays | e1e |
| `int a[static N]`, VLA parameters (`int a[n]`, `int a[*]`) | M8 |
| `owns_range` of a `const` pointee (write `reads_range`); a range over `void *` or a function pointer | — |

**Size** (`src/` insertions without self-tests; e1a landed 1,943 against 2,200–3,450): Part A 250–400, Part B
700–1,000, Part C 400–600, Part D 400–600, Part E 300–450; 2,050–3,050 in all.

## 2. The ensemble

| Role (model) | Owns | Never touches |
|---|---|---|
| **Orchestrator** (Opus) | Steps 0–1; record 0060; the vprdiff verdict per part; user pages in `doc/`; status, handoff, `plan.md`'s Done row; every commit; briefing and resuming the agents | `src/`, `tests/`, `scripts/` (it delegates) |
| **edg-backend** (Sonnet) | `src/backend/**`, `src/driver/**`, `include/cerastes.h`; **in Part A only** `src/vir/**` and `src/viper/**` (no il-analyst holds them yet); the build system, the baseline, and **the joined build of each part** with `scripts/regen_vpr.py` and `scripts/vprdiff.py` on it; `internal/encoding.md`, the developer guide's paragraph | `tests/`, `scripts/lint_viper.py`, `src/vir` after Part A |
| **il-analyst-1** (Sonnet) | P79 and S44; the chapter `## M4.5e1b` of `internal/il-inventory.md`; the fixtures `tests/viper-smoke/memory/s44_*.vpr`. It ends when the chapter is committed | `src/` |
| **il-analyst-2** (Sonnet, fresh at Part B) | `src/vir/**` (`vir.h`, `vir.cpp`, `check.cpp`, `lower.cpp`, `lowered.*`), `src/viper/**`, `src/viper/selftest_cases30.inc`, for Parts B and C | `src/backend`, `tests/` |
| **il-analyst-3** (Sonnet, fresh at Part D) | the same files, for Parts D and E. Briefed with this plan, record 0060, the S44 chapter and Part C's commit | `src/backend`, `tests/` |
| **test-mutator** (Sonnet) | `tests/**` (tests, twins, pins, exactly the Flips, the port), the compiler matrix, `scripts/run_tests.py`, `scripts/lint_viper.py` and their self-tests, the doc example files under `doc/modules/ROOT/examples/` | `src/` |
| **soundness-auditor** (Opus, read-only) | four rounds and one re-audit (section 7), each on a snapshot commit in `.claude/worktrees/m4.5e1b-audit`; resumed between rounds | everything (never edits) |
| **Advisor** (Fable) | consulted by the orchestrator at the five checkpoints below | — |

- **Five Sonnet agents over the session, at most three alive at once:** edg-backend, the current il-analyst,
  test-mutator. edg-backend and the test-mutator are resumed between tasks (SendMessage); an il-analyst is
  replaced, not resumed, at the two hand-overs above (`lower.cpp` is 19,293 lines). This is the plan, not a
  deviation.
- **Builds:** every agent that edits `src/` builds its own change through the lock (`lockf
  build/m4.5e1b/build.lock cmake --build build/cmake`). Nothing builds during a directory-wide run.
- **The runner** (`scripts/run_tests.py`; it locks `build/vpr`) has one holder at a time: the test-mutator for
  R3; il-analyst-1 from R3's end until Commit 1 (S44's `.vpr` fixtures, one file per run: they cannot go through
  `cerastes verify`); the test-mutator from Commit 1 on. Everyone else uses `build/cerastes verify <file>
  --keep-vpr`.
- **The advisor** is the `advisor` tool; if the orchestrator has none, an Agent call with `model: fable`, briefed
  with the question and the paths to read. Step 0 checks that one of the two answers.
- **`lower.cpp`, `vir.h`, `vir.cpp`, `check.cpp` have one holder at a time:** edg-backend in Part A, il-analyst-2
  in Parts B and C, il-analyst-3 in Parts D and E, with a commit between holders. An agent that needs a change in
  another's file asks the orchestrator, who passes it on.
- **Advisor checkpoints:** (1) the record draft with S44's outcomes mapped to section 5's tables, before audit
  round 1; (2) any outcome outside a table; (3) the split point at Part C's commit; (4) any cut; (5) before the
  box run.
- **Every agent** (the orchestrator puts this in each briefing):
  - reads `CLAUDE.md`, this plan, record 0060 and the files it is assigned; edits only its own files;
  - any question stops its work and goes to the orchestrator; an owner question goes to the owner;
  - writes each report to `build/m4.5e1b/<role>-report-<n>.md` as well as returning it;
  - follows the build and runner rules above;
  - a test generator lives in `build/m4.5e1b/` and is never re-run over the tree;
  - tool code in the owner's style (`scripts/format.sh`; brace initialization, `U`, one declarator per
    declaration, no comment on a brace line); no Claude trailer.
- **A declaration lands with its reader** (P77): the header's new names and macro land in the part that reads
  them (Parts B and E), never in Part A.
- **Fail closed between parts:** what a later part lifts is rejected with a diagnostic and a staging pin, written
  and deleted inside the session.

## 3. Survey facts (tree at `dc40377`; read, not run; names, with lines where they help)

- **Nothing needs the EDG fork.** A spec function is a declaration in `<cerastes.h>` plus name lists;
  `owning` is registered `{"owning", "?(X)", "r|v|p|d", RAF_ENTITY_ATTACHED}` and `arguments_of`
  (`annotation_il.h`) returns its argument expression. Unobserved: how the IL names a sibling parameter inside
  `owning(n)` (P79 (c)).
- **Names** (six lists): `encode.cpp` `enum class SpecFn`, `spec_fn` (where `cerastes::owns_range` and `forall`
  are `SpecFn::unsupported`), `declared_in_cerastes_header` and the dispatch in `call`; `il_query.cpp`
  `is_spec_function_name`; `annotation_checks.cpp` `is_spec_name` and `is_protected_macro_name`, and
  `check_attribute`'s early return for `owning` (its argument is unchecked today);
  `src/driver/edg_args.cpp` `is_cerastes_name`; `annotations.adoc`'s reserved names.
- **Header:** `cerastes::owns_range(const T* first, decltype(sizeof 0) count)` exists (C++ only); there is no
  `reads_range`, no C spelling, no `CERASTES_OWNING_N`. Two goldens print `owns_range`'s signature
  (`tests/annotations/spec_functions/spec_calls.cpp.golden`, the doc example `annotations/spec_functions.cpp`):
  **its declaration does not change.** No golden lists the header's other declarations.
- **Encoder** (`src/backend/encode.cpp`, 20,724 lines): `spec_atom`, `invariant_atom`, `place_pointer`,
  `contract_place_address`, `contract_read`, `old_call` (C++), `old_c` (C), `finish_atoms` (numbers the amounts),
  `apply_owning`, `read_marker` (rejects `owning(n)`), `check_marker_atoms`, `reject_marked_read`,
  `check_signature` (types a parameter from EDG's adjusted pointer type and reads `declared_type` only to reject
  `[static N]`), `subscript_address` (rejects a subscript in a contract and `p[i]` of a pointer in a loop
  clause), `array_decay`, `subscript_read`, `reject_computed_sizeof_operand`, `encode_loop_clauses`. The modes are
  flags on `Ctx` (`contract`, `annotation`, `loop_clause`, `loop_invariant`, `annot`, `old_depth`).
- **Array parameters are accepted today as pointers** (`int a[]`, `int a[3]`, `const int a[]`), with no test;
  `sizeof(a)` of one is accepted with the pointer's size (clang warns by default).
- **VIR:** an atom is `Expr::Kind::Atom` with `AtomKind {Owns, Reads, Live, OwnsBlock}`, the pointer `a`, the
  `amount` number; guards live on `AtomSite` (`contract_atoms`, `AtomCollector`); `atom_place`, `place_root`
  (steps through `MemberAddr`, `Deref`, `Field` only), `PlaceKey`, `restrict_backed`, `restrict_hazard`.
  `check.cpp`: `Checker::atoms`, `init_markers`, and `Checker::expr`'s `ElemAddr` case, which refuses it in
  `Requires` and `Ensures`.
- **Lowering** (`src/vir/lower.cpp`): `atom` (the modes `CMode::Method`, `Call`, `Ret`, `Wf`), `amount_name`,
  `amount_bounds`, `entry_checks`, `call` → `call_atoms` (per-atom rows; separation over `exclusive_pair` and
  `overlap_pairs`; splits; amounts `(g ? m : write) / (N + 1)` with `min_perm`; sums), `ret` → `postconditions`
  → `atom_returns`, `wf_method` and `contract_read`'s `Wf` branch (ghost formals `hv_<k>_pre|_post`, keyed by
  `GKey`), `site_marker`, `loop_places`, `loop`, `ArrayPlace`, `LoopWrites`, `inv_elem` (a named local array
  only), `elem_addr` (refuses contract modes), `range_perm` (literal bounds, full permission only),
  `emit_marked_row`, `array_scope_exit`, `split_candidates`. **A parameter's cell place is carried at its entry
  snapshot `perm(…)`**, which a quantifier cannot hold.
- **Lowered program and driver:** `LExpr::Kind::Forall`, `old_(e, label)`, `Check::expected_reason`; the map's
  `assert.failed[insufficient.permission]`; `map_errors`' exact-reason gate.
- **Lint** (`scripts/lint_viper.py`): `lint_quantified_rows` admits a `forall` only in the `Block` axioms, the
  `alloc_arr_`/`zero_arr_` contracts, a marked `assert`, the range `exhale`, and four loop conjuncts. **A
  `forall` in a `requires` or `ensures` is refused today.** `lint_ghost_amounts`, `lint_perm_locals`
  (`perm_value_ok`), `lint_flags`.
- **Self-tests:** the latest case file is `selftest_cases29.inc`, registered three times in `selftest.cpp`;
  `VIR_CASES_MIN=1825` (`scripts/check.sh`); `selftest_cases28.inc` has `arr_rej_elem_addr_in_requires`,
  `_in_ensures` and `_in_atom`. `test_lint_viper.py` 782 tests, `test_run_tests.py` 1,221.
- **S43 already settled** (`il-inventory.md`, chapter M4.5e1a): Silicon ends the path at a failed marked row
  (G2); `old[label]` under a quantifier keeps values (G3); no `qp.not.injective` when cells, arrays and
  parameters mix, and two `reads(*p)` atoms may take fractions of a quantified chunk (G4); a pure row that fails
  is assumed and the next row may fail too (N1: `p[1]` under `owns(*p)` is two errors); each quantifier stands
  alone, never a conjunction under one binder. The callee's side of a range is pinned since M4.5a
  (`e2_array_sum.vpr`: `OWNR`, `INITR`); **no fixture has a caller of a range**.

## 4. Step 0 and Step 1

1. **Orchestrator.** `git fetch`; branch from `origin/main` (it holds this plan's commit once the owner pushes;
   otherwise from local `main`, and say so in status). `git worktree add .claude/worktrees/m4.5e1b -b
   m4.5e1b-ranges-in-contracts <base>`; submodules with `--reference` (edg, silicon, then silver);
   `scripts/bootstrap.sh`. CI recorded only (`gh run list --workflow ci.yml --event schedule --limit 3`). One
   question to the advisor, to check that it answers (section 2).
2. **edg-backend.** The baseline before any change to `src/`: `scripts/regen_vpr.py` and the VIR self-test into
   `build/m4.5e1b/baseline/`; frozen binaries in `build/m4.5e1b/cerastes-base/`.
3. **test-mutator, on the frozen build** (`build/m4.5e1b/s0/`, both modes):
   - **R1** array parameters today: `int first(int a[])`, `int a[3]`, `const int a[]`, each with `owns(*a)` or
     `reads(*a)`: the outcome and `native-ok`. The survey read them as verified; they become Part E's positives.
     **If one is not verified today:** Part E lifts it only when the cause is one site in `check_signature` or
     `vtype`; anything else stops and asks.
   - **R2** today's diagnostic for every construct of section 1's scope and table (the baseline of Flips B), and
     the list of every message in `src/` that names M4.5e1b or plain "M4.5e1" (`grep -n 'M4\.5e1b\|M4\.5e1[^a-e]'
     src/`; the survey found thirteen and five). Each is lifted by this session or re-pointed to the session
     section 1's table names: `old_c`'s "an element: M4.5e1" is lifted; `spec_block_atom`,
     `invariant_block_atom`, `invariant_atom`'s root rule and `struct_info`'s `owning(n)` go to M4.5e2;
     `apply_function_markers`' `owning(n)` on a function goes to M4.5e2 and gets the pin it lacks.
   - **R3** every `rejected` test under `tests/scalar/negative`, `tests/memory/negative`, `tests/vlibc/negative`
     and `tests/annotations/negative`, kept as `build/m4.5e1b/s0/rejected-base.txt`. After each part the same
     run may differ only by section 9.
   - **M** the compiler matrix (clang and GCC, both modes, no flag / `-Wall` / `-Wall -Wextra`) for: `sizeof(a)`
     and `sizeof(a) / sizeof(a[0])` of an array parameter; two declarations of one function with different
     parameter bounds; a local array shorter than the bound passed to `int a[4]`; `int a[0]` and `int a[const 3]`
     parameters. **Rule (not a decision):** a shape that warns without a flag or at `-Wall` in either compiler is
     rejected like the compilers, with its own tag and a pin; the rest are accepted and listed for M8's sweep.
     The tags: `sizeof_array_argument` (the `sizeof` shapes), `array_parameter_mismatch` (two declarations),
     `array_argument_too_short` (a shorter array); a shape the matrix shows silent gets no tag.
   - R3 is the only `run_tests.py` run until il-analyst-1's P79 has finished; S44's runs start after R3.

## 5. P79 and S44 (il-analyst-1, before any encoder code; results in `internal/il-inventory.md`, `## M4.5e1b`)

**P79 (IL, both modes, `build/m4.5e1b/p79/`; every claim with its `--il_display` or `--cerastes_walk` excerpt).**

| Probe | Expected | If not |
|---|---|---|
| (a) `cerastes_owns_range(p, n)`, `cerastes_reads_range(p, n)` (C variadic; `n` a `char`, `short`, `long`, `unsigned`, `size_t`); `cerastes::owns_range(p, n)` (the conversion of `n` to `size_t`, of `int *` to `const int *`) | each conversion is a compiler-generated cast over the written argument; the encoder reads the operand, never the cast's value (0022) | a generated cast that cannot be told from a written one: **stop and ask** (the alternative is a template signature and two golden changes) |
| (b) parameters `int a[]`, `int a[3]`, `int a[const 3]`, `const int a[]`, a macro, enumerator, folded and wrapping bound, `int a[n]`, `int a[*]`, `int a[0]`, a leading marker; two declarations with different bounds; `sizeof(a)` | `declared_type` keeps the array type and its bound's `expr` as a local array's does (P78 (a)); `sizeof(a)` is a constant whose `expr` is `enk_sizeof` over the parameter | a bound without `expr` where P78 found one: bounds are read only where `constant_value` accepts them, the rest rejected (as 0059 item 2) |
| (c) `[[cerastes::owning(n)]] int *p` with `n` an earlier parameter, a later one, a constant, `n + 1`; both positions; two declarations that name the parameter differently | the argument is an expression over parameter references that `interpret_param_ref` resolves by position; a later parameter is EDG's #20 | not resolvable by position: `owning(n)` takes a constant or a parameter named alike on every declaration, anything else rejected and pinned |
| (d) `cerastes_old(p[i])`, `cerastes::old(p[i])` | C: `address_of(subscript(p, i))` (or the folded `eok_padd`); C++: a reference bound to the subscript | either C shape is read as the element; a third shape: **stop and ask** |
| (e) `p[i]` in `requires`, `ensures`, `decreases`; `i` the result name in an `ensures` | the node shapes of code (P78 (c)) | — |
| (f) a leading `CERASTES_MUST_INIT` / `CERASTES_REF_TO_UNINIT` on `int a[]` | a marked pointer parameter (P64) | rejected and pinned |

**S44 (hand-written Viper in the shapes of section 6; fixtures `tests/viper-smoke/memory/s44_*.vpr`; every row
expanded as lowering writes it; each positive with failing twins; one file at a time through
`scripts/run_tests.py`; times on a quiet machine).** A twin must fail with **exactly one** error unless the table
says otherwise. Every fixture method ends with an `assert false` twin (vacuity).

| Spike | Programs | Built when it passes | If it does not |
|---|---|---|---|
| **(a) the callee** | `owns_range` and `reads_range` in a method contract, unguarded and under `n == 0 \|\|` and `p == null \|\|`; reads and writes of `p[i]`; the return rows. Twins: a callee keeps `&p[1]` (the marked return row, alone); `p[n]` (two errors, N1) | the guard as an implication outside each quantifier (`g ==> forall …`), and `if (g) { marked row }` | the guard inside the antecedent; if neither: stop and ask |
| **(b) the caller on a local array** | whole array; a sub-range `(&a[1], 2)`; twice in a row; two disjoint sub-ranges to a two-range callee; an array written element by element, then passed (the flags and range rows must be provable). Twins: one element too many; an unwritten element; overlapping `owns` ranges; `(NULL, 0)`; a negative count | **the row order of 6.8** (marked rows first) | the order with the shape row first; if a twin still gives two errors, the tests expect both (N1's rule) and status lists them. A correct program that fails: stop and ask |
| **(c) `reads_range` amounts** | the amount from the first slot; a caller with full permission; one that borrowed the range itself; `(a, a)`; overlapping sub-ranges; a write after the call (the permission came back). Twins: a write in the callee; a caller holding unequal fractions (the listed limit) | 6.6's amount, no extra row | add a no-op split `if (block_of(p) == block_of(q)) { }`; then the sum row of (e) at calls; then **fallback F1**: overlapping ranges are refused at the call unless both are `reads_range` of one argument spelling, pinned and listed. Non-aliasing ranges failing: stop and ask |
| **(d) a range beside a cell** | `owns_range(a, n)` with `owns(*q)` (separation; twin `q == &a[1]`); `reads_range(a, n)` with `reads(*q)`, `q == &a[1]` (verifies) | 6.8's separation rows; the cell's sum row gains the range's term | as (c) |
| **(e) sum rows** | a marked row whose amount depends on `k`: `acc(loc(B, k).f, amt_i + (in_j(k) ? amt_j : none))`, at the return of a function with two overlapping `reads_range` atoms; twin: a callee kept a fraction | the sum row at returns wherever two read atoms of one field may overlap | per-atom rows only; the corner (an exit 3, never a proof) is listed in the record and in handoff §3 for M4.8 |
| **(f) `f__wf` and the contract's reads** | ghost formals for `p[i]` and `p[j]`; framing by a range atom; `old(p[i])`; the guarded read `(F ? loc(…).v : 0)` beside the quantified permission in the method's `ensures`. Twins: `p[n]`; a read with no atom | 6.5 | stop and ask |
| **(g) loops** | a fill loop over an `owns_range` parameter; a search loop over `reads_range` (values kept by `old[lh]`); two range parameters; a range and a `reads(*q)` cell with `q` inside it; a guarded range; a `return` in the body. Twins: a callee keeps an element in the body; an index one too far | 6.11, the `reads` range carried at the function's own `amt_k` | overlapping read ranges that fail: the later place takes only what the earlier ones leave (its antecedent excludes them); still failing: a pinned false alarm, listed. The fill and search loops over 30 s each: stop and ask |
| **(h) markers** | `ref_to_uninit` with `owns_range` (no flags on entry, `old(flag) ==> flag` under the quantifier on return); a straight-line `must_init` filler; a caller with an `uninit` array. Twin: the filler leaves one element (`postcondition`'s row) | 6.12 | stop and ask |
| **(i) placement** | `x = a[0] + sum(&a[1], 2)`; twin: the access inside the callee's owned range | 6.10's rows | cut 2 of section 7 (the rejection) |
| **(k) cost** | two range parameters, two loops and three calls in one method; S43 (h)'s scale program with a range callee | recorded | **over 60 s: stop and ask** |

**Stop rules for all of S44:** a second Silicon error after a marked row; `qp.not.injective`; an outcome the
table does not name.

## 6. Record 0060 — ranges in contracts (the orchestrator writes it before code from this section; revised only by section 5's tables and audit round 1)

Amends 0028 items 7, 8, 11 and 13; 0029 items 1 and 3; 0037 items 2, 5–10 and 15; 0038 items 3 and 6; 0039
item 4; 0040 item 3; 0042 items 4–8; 0059 items 1, 8 and 10; 0011 item 10's messages.

1. **Scope:** section 1, with its table.
2. **Spellings.** `<cerastes.h>` declares, in both modes, `cerastes_owns_range(...)` and
   `cerastes_reads_range(...)` (C23's variadic form, `noexcept` inside `extern "C"` in C++), and in namespace
   `cerastes` a `reads_range` with `owns_range`'s signature, which stays as it is. Each takes a pointer and a
   count. The names join the six lists of section 3 and pass `declared_in_cerastes_header`. In code they are
   `spec_call_in_code`; in a `check`, `call_in_annotation`.
3. **The atom.** `owns_range(p, n)` and `reads_range(p, n)`, in 0037 item 5's positions, in a function's
   `requires` and `ensures`.
   - **`p`** is a pointer parameter (in an `ensures`, its entry value) to `bool`, an integer type, an enumeration
     or a pointer to an object type; for `owns_range` the pointee is not `const` (0040 item 2).
   - **`n`** is an integer expression over parameters and constants that reads no memory, with 0010 item 11's
     obligations checked in `f__wf`; its value is the written expression's (P79 (a)); in an `ensures` the
     parameters are entry values.
   - **VIR:** `AtomKind::OwnsRange` and `ReadsRange` on `Expr::Kind::Atom`, with the count as a new operand;
     `Builder::range_atom`; a structural equality of pure expressions for the partner rule.
   - **Partners:** an `ensures` range atom pairs with the `requires` atom of the same kind, parameter and count
     (structurally equal). A `requires` range with no partner is kept by the callee (0037 item 13).
   - **Rejected** (`unsupported_annotation`, one pin each): section 1's table rows on bases, counts and partners;
     the same kind twice on one parameter; a wrong number of arguments; a structure pointee (e1e).
4. **Viper meaning** (`atom`, `CMode::Method`). Under the atom's guard `g`, each line its own conjunct, each
   quantifier over the absolute slot with the trigger `{ loc(block_of(p), k) }` and the range
   `offset_of(p) <= k && k < offset_of(p) + n`:
   - `p != null && WF(p) && 0 <= n && offset_of(p) + n <= len(block_of(p))`;
   - `acc(loc(block_of(p), k).v_T[, amt])`; `acc(loc(block_of(p), k).d_T[, amt])`: full for `owns_range`, the
     atom's ghost amount for `reads_range`;
   - the flags, `loc(block_of(p), k).d_T`; the type's range (`WF` for a pointer element, none for `bool`);
   - `acc(lvloc(block_of(p)).lv, amt)`.
   One ghost `Perm` formal per `requires` atom, numbered with the cell atoms (0037 item 6). An empty range
   (`n == 0`) holds the liveness fraction only.
5. **Element reads in contracts, and `f__wf`** (0037 item 7).
   - `p[e]` is accepted in `requires`, `ensures` and a function's `decreases`, and inside `cerastes_old` and
     `cerastes::old` in an `ensures`, for a pointer parameter `p` and an index that reads no memory. VIR:
     `Deref(ElemAddr(p, e))`; `check.cpp` accepts that read in a contract and still refuses the address as a
     value.
   - **Framing.** A read of the address `q` is framed by a cell atom as today (`g_i && q_i == q`) or by a range
     atom: `g_i && block_of(q) == block_of(p_i) && offset_of(p_i) <= offset_of(q) && offset_of(q) <
     offset_of(p_i) + n_i`. `f__wf` asserts the disjunction (`permission`: "the contract of `f` reads `p[i]`
     without owning it"); no separate bound row.
   - **Ghost values.** One ghost formal per written read, keyed by base, index and state; the existing fact
     `q_a == q_b ==> hv_a == hv_b` now compares element addresses; a `reads_range` place's post value equals its
     pre value.
   - **In the method's contract** each read is the `ite` chain over its framing disjuncts (0037 item 7).
   - In a range atom's `Wf` form only the pure line of item 4 remains.
6. **Amounts at a call** (0037 item 6). `amt_k := (g_k ? m_k : write) / (N + 1)`. For `owns_range`, `m_k` is
   `perm` of the liveness cell. For `reads_range` it is `n > 0 ? min(perm(F.v_T), perm(F.d_T), perm(L)) :
   perm(L)`, with `F` the range's first slot `loc(block_of(p), offset_of(p))`: `perm` cannot stand under a
   quantifier, so one cell represents the range and the marked rows then check all of it at that amount.
   **Listed limit:** a caller that holds unequal fractions over the range gets a false `permission` (pinned;
   question 238).
7. **Entry.** The liveness case splits of 0037 item 9 include range atoms. No separation is asserted for ranges
   (0028 item 11).
8. **Calls** (`call_atoms`; every row atomic, under the atom's guard).
   - **Before the amounts**, with the cell atoms' rows: for `reads_range`, `!(n > 0 && (perm(F.v_T) == none ||
     perm(F.d_T) == none))`, and for both kinds the liveness row (`permission`).
   - **Separation**, with the cell pairs: for a pair of one field with at least one `owns` or `owns_range`, a
     range and a cell `x`: `!(block_of(x) == block_of(p) && offset_of(p) <= offset_of(x) && offset_of(x) <
     offset_of(p) + n)`; two ranges: `!(block_of(p) == block_of(q) && 0 < n && 0 < m && offset_of(p) <
     offset_of(q) + m && offset_of(q) < offset_of(p) + n)` (`permission`: "passes `a[0..n)` and `b[0..m)` to
     `f`, which may be one object, once to own it").
   - **After the amounts, per range atom, in this order:** (1) the two marked rows, value then flag, at `write`
     or `amt_k`, inside `if (g) { }` (`permission`: "calls `f`, which needs to own `a[0..n)`, without owning
     it"; `reads_range`: "… to read …"); (2) the shape row `!(p == null || !WF(p) || n < 0 || len(block_of(p)) <
     offset_of(p) + n)` (`permission`, the same message); (3) the flags over the range (`uninitialized`: "passes
     `a` to `f`, whose parameter `p` needs every element of `p[0..n)` to have a value"; none for a marked
     parameter); (4) the type's range over the range, under the flag (`permission`, as the cell's range row).
     The marked rows come first because a failed marked row ends its path: a range that is too short, a null
     pointer with a positive count and an unowned range are then one error each.
   - **Sums:** the liveness sums include range atoms; a `reads` cell's sum gains `(g_r && x in range r ? amt_r :
     none)` per read range; a range's own sum row only as S44 (c) and (e) decide.
   - The explicit rows cover every conjunct of item 4, so Silicon's own precondition check never fails first.
9. **Returns** (`atom_returns`), for each `ensures` range atom under its guard, after the separation rows of the
   `ensures` pairs: the marked rows (`permission`: "returns without owning `p[0..n)`, which its contract
   promises"; `reads_range`: "… without the permission to read `p[0..n)` it borrowed"; the sum form where S44
   (e) gives it); the shape row (`internal`: the entry facts); the flags (`permission`; for `must_init`
   `postcondition`, "returns without initializing every element of `p[0..n)`, which `cerastes::must_init` on `p`
   promises"; for `ref_to_uninit` the `internal` row `old(flag) ==> flag` under the quantifier); the type's
   range; the liveness sum.
10. **Accesses, sequencing and placement.** `p[i]` in a body keeps 0059 item 4's rows; under a range atom the
    bound follows from the atom's pure line and `0 <= i && i < n` (an unprovable index is two errors, N1). A
    call whose callee holds a range, beside an access or another memory call in one full expression, gets 0039
    item 4's rows with the range forms of item 8's separation (`unsequenced`). A callee's `owns_range` reaches
    its field in `LoopWrites` (0038 item 3's havoc rule).
11. **Loops** (0059 item 8, for a parameter's range).
    - **Places.** Each `requires` range atom of a pointer parameter the function never assigns is a range place:
      base `p`, bounds `offset_of(p)` and `offset_of(p) + n` (entry values), the amount `write` (`owns_range`)
      or the function's own `amt_k` (`reads_range`), the guard `g`.
    - **Rows:** 0059 item 8's, with the place's bounds, amount and guard: the marked entry rows ("this loop
      begins without the permission to every element of `p[0..n)`"), the label, the conjuncts (permissions,
      flags kept, the type's range, and the values kept when no write reaches the field **or** the place is
      read-only), the marked back-edge rows, the liveness snapshot and row as a parameter's cell place has them.
    - **Beside cell places:** a parameter's cell place gains, for every range place `r` of its field, the guard
      "not inside `r`", with a case split before the snapshots (a cell inside a range is carried by the range;
      what the function holds beyond it stays in the frame).
    - **Reads in invariants and measures:** `p[e]` of any pointer and `a` used as a pointer (`q == a`) are
      accepted. A read is framed by a range place (the interval test of item 5) or a cell place; unframed, it is
      `permission` ("this loop invariant reads `p[e]`, which this loop does not hold"); a named array keeps
      e1a's `out-of-bounds` row; the flag obligation is 0038 item 6's.
    - **Not built** (question 237): a range atom in an invariant is rejected; a loop in a function that assigns
      its range parameter has no range place (a pinned false alarm).
12. **Markers** (0042 items 4–8). A marker's atom may be `owns_range(p, n)`. `ref_to_uninit`: no flags on
    entry, the range under the flag, `old(flag) ==> flag` on return, no flags row at the caller. `must_init`:
    every element's flag on return. `reads_range` beside a marker and a contract read `p[e]` of a marked
    parameter are rejected as their cell forms are. `--strict-init` reads an array argument as it reads `&x`. A
    `must_init` filler that fills with a loop cannot state its invariant before e1c (a pinned false alarm;
    question 239).
13. **`owning(n)` on a parameter** (0029 item 1, 0040 item 3). `p == NULL || owns_range(p, n)` first in both
    clause lists, `n` as P79 (c) reads it, merged with an explicit `owns_range(p, n)` of the same count as 0040
    item 3 merges `owns(*p)`; declarations must agree on the count (`annotation_mismatch`); the argument gets
    the annotation checks (`check_attribute`). `CERASTES_OWNING_N(n)` is `[[cerastes::owning(n)]]`, and nothing
    in another compiler. The other targets stay rejected, their messages naming M4.5e2.
14. **`restrict`** (0037 item 2). `restrict_backed` takes `owns_range(p, n)`, unguarded or after `p == NULL ||`,
    in both lists, as the backing atom; every other parameter of the pointee type needs an `owns`, `reads`,
    `owns_range` or `reads_range` of its own.
15. **Array parameters.** `T a[]` and `T a[N]` are pointer parameters; a written bound is checked as a local
    array's is (0059 item 2) and otherwise ignored; `sizeof` of one is rejected like the compilers (a new tag);
    the matrix's other warning shapes likewise (Step 0, M; question 240); `[static N]` and VLA parameters stay
    rejected (M8).
16. **The lint.** New admitted shapes, each with twins: a range atom's quantifiers in `requires` and `ensures`
    (permission with or without a `Perm` formal as amount, flags, range or `WF`, `old(flag) ==> flag`); marked
    rows with `offset_of` bounds, an amount, and inside `if (g) { }`; the pure quantified `assert` rows of items
    8 and 9; the loop conjuncts with an amount and a guard; the amount expression of item 6. Nothing else that
    holds a `forall` is admitted.
17. **Categories:** none new. New diagnostic tags only from item 15. **Facts:** P79, S44.

## 7. Parts, owners, order

| Part | Content | edg-backend | il-analyst | test-mutator |
|---|---|---|---|---|
| **A** plumbing, behaviour-neutral | VIR and lowered-program plumbing; nothing accepted yet | `AtomKind::OwnsRange`/`ReadsRange`, the count operand, `Builder::range_atom`, the pure-expression equality, every kind switch (`AtomCollector`, `atom_place`, `Checker::atoms`: refused everywhere); `range_perm` with expression bounds and an amount | il-analyst-1: P79, S44 | Step 0 (R1–R3, M); the lint rules of item 16 against S44's fixtures, with twins |
| **B** the callee | items 2–5, 7, 9 | header names; `spec_fn`, `call`, `spec_atom` (range forms), `finish_atoms` (partners), `subscript_address` and `contract_read` for `p[e]`, `old_call`, `old_c`; the name lists | **il-analyst-2**: the guarded marked row in the form S44 (a) chose; `atom` in `Method`, `Ret` and `Wf`; `contract_read` for an element; `wf_method`; `atom_returns`; `check.cpp`; VIR cases in `selftest_cases30.inc` | Part B's tests; Flips A1, B; staging pins S1, S2 |
| **C** the caller | items 6, 8, 10 | the encoder's placement rule (0039 item 4) for range callees; then `internal/encoding.md` for Parts A–C. **No Part D or E code before Commit 3** | il-analyst-2: `call_atoms` (rows, separation, amounts, sums), the sequencing keys, `LoopWrites` | Part C's tests; S1 deleted |
| **D** loops | item 11 | `subscript_address` and `array_decay` in loop clauses | **il-analyst-3**: range places in `loop_places` and `loop`, `inv_elem` for any pointer, the cell-place guards and splits | Part D's tests; Flips A2, A3; S2 deleted |
| **E** markers, `owning(n)`, `restrict`, array parameters | items 12–15 | `read_marker`, `apply_owning`, `check_marker_atoms`, `reject_marked_read`, `check_attribute`, `CERASTES_OWNING_N`, strict-init arguments, `check_signature` and `reject_computed_sizeof_operand` (item 15), `diagnostics.def` | il-analyst-3: `site_marker` and the marked rows of items 4, 8, 9; `restrict_backed` | Part E's tests; Flip A4; the port |

**Staging pins** (test-mutator writes them with Part B; the named part deletes them): **S1** a call of a
function whose contract holds a range atom (until Part C); **S2** a loop that touches memory in a function whose
contract holds a range atom (until Part D). Markers and `owning(n)` beside a range stay rejected by today's
rules until Part E, and inside Part E the gate is the VIR checker's: `check.cpp`'s `init_markers` refuses a
marker whose atom is a range until il-analyst-3 lifts it together with `site_marker`'s rows, so an encoder that
is ahead of the lowering cannot make a marked range assume its flags.

**Order of work.**
1. Step 0 and Step 1. Then, in parallel: il-analyst-1 on P79 and S44; edg-backend on Part A; the test-mutator on
   R1–R3 and M, then the lint rules; the orchestrator on record 0060's draft.
2. **Commit 1:** record 0060 (draft, revised by section 5's tables), the `## M4.5e1b` chapter, the S44 fixtures.
   Advisor checkpoint 1, then audit round 1 on it. il-analyst-1 ends.
3. **Part A** may be built before round 1 returns (it accepts nothing). It is proved neutral (`vprdiff`: every
   `.vpr` and `.map` identical) and joins Part B's commit.
4. **Part B starts only when audit round 1 has returned and the record holds its blocking fixes** (invariant
   6). edg-backend and il-analyst-2 work in parallel against Part A's VIR; the test-mutator writes the tests from
   the record, expectations pre-registered. Audit round 2 on the snapshot. **Commit 2** (Parts A and B).
5. **Part C:** il-analyst-2 on the caller; edg-backend on the placement rule. The tree holds nothing of Parts D
   and E, so Commit 3 is Part C alone. Audit round 3. **Commit 3.** **Split point 1.**
6. **Parts D and E** (after Commit 3): il-analyst-3 holds `src/vir`, Part D first, then Part E's lowering;
   edg-backend does Part D's encoder side, then Part E's. Audit round 4 on both. **Commit 4.**
7. The port and the doc examples (test-mutator), the user pages (orchestrator), `internal/encoding.md` and the
   developer guide (edg-backend); the re-audit of blocking fixes; the checks of section 11; record 0060
   accepted; status, handoff, `plan.md`'s Done row.

**Proofs, per part.**
- **Each half alone** (the encoder half cannot run end to end before the lowering half exists): the il-analyst
  proves its half by the new VIR cases through `cerastes-vir-selftest` and Silicon, which need no encoder;
  edg-backend proves its half by its build and by R3's run of the rejections (they stop before lowering) against
  section 9. Both run `scripts/format.sh`.
- **The joined proof**, once both halves have reported: edg-backend makes the joined build, runs
  `scripts/regen_vpr.py` into a directory of its own and `scripts/vprdiff.py` against the baseline (expected in
  every part: nothing differs); then, with builds held, the test-mutator runs the part's tests once. A failure
  goes through the orchestrator to the half that owns the failing file; the joined proof is repeated once, after
  the last fix.
- The audit snapshot is taken from the tree the joined proof passed on.

**Audits** (one resumed Opus reviewer; a snapshot commit and a copy of the binaries built from exactly it):
- **Round 1, the record with P79 and S44:** reproduce the gates; is any atom shape vacuous; can the
  representative-cell amount let a callee take more than the caller holds anywhere; is a marked row's reason
  still unambiguous with an amount and inside `if (g)`; the count read as written; the partner rule.
- **Round 2, Parts A and B:** a range atom or an element read accepted outside items 3 and 5; an unframed
  contract read; two reads sharing a ghost; `n` in an `ensures` not the entry value; a return row that hides
  another reason; Part A's neutrality; a lint rule a wrong shape can satisfy.
- **Round 3, Part C:** every conjunct of the callee's atom has its explicit row; the separation formulas with
  empty ranges and false guards; the amounts with `n == 0`; placement; the havoc of a callee's `owns_range`.
- **Round 4, Parts D and E:** each loop conjunct against the scalar row it mirrors; the cell-in-range guard;
  values kept only where no write can reach or the place is read-only; no flag assumed for a marked range;
  `owning(n)` equal to its written form; `restrict`'s argument; the array-parameter rejections.
- **One re-audit** of blocking fixes. Should-fix items not taken go to handoff §3.

**Pre-agreed cuts, in this order** (each leaves its pin, re-pointed): (1) `restrict` backed by a range (M8);
(2) item 10's placement rows: instead, a call that holds a range atom beside another cell access or memory call
in one full expression is rejected (`unsequenced_calls`; M4.8); (3) `owning(n)` on a parameter (M4.5e2);
(4) markers over ranges (e1c). Parts B, C and D, array parameters and the port are not cut without asking.

**Split points.** At Commit 2 and Commit 3 the orchestrator reports `git diff --numstat <base> -- src/` without
the self-tests against section 1's sizes. **Stop and ask** if Parts A and B passed 1,600 lines, if Parts A–C
passed 2,300, or if, at Commit 3, the orchestrator and the advisor judge that its context will not carry Parts D
and E with their audit (then e1b ends at Commit 3 with S2 kept, and the handoff plans Parts D and E as a session
of their own).

## 8. Tests (`tests/memory/range_*.c`, `array_param_*.c`; `.c` in `c23, c++20` where valid; each positive has a twin failing for one reason at one line, except where N1 gives two)

- **Part B.** A getter and a setter under `owns_range` with `0 <= i && i < n` (twins: `i <= n`, two errors; a
  wrong `ensures`, `postcondition`). `reads_range` (twin: a write, `permission`). `cerastes_old(p[i])` in both
  spellings (`p[j] == cerastes_old(p[j])` under `j != i`; twin without it). Framing: `requires(owns(*p) && p[0] >
  0)` verifies; `p[1]` there, `p[n]` under a range and a read with no atom are `permission` at the contract. An
  element in a function's `decreases`. Guards (`n == 0 ||`, `p == NULL ||`). An empty range. A callee that keeps
  `&p[1]` (`permission` at the return). Every rejection of item 3 and of section 1's table, one pin each.
- **Part C.** A local array passed whole, as a sub-range, twice; two sub-ranges to a two-range callee;
  `(a, a)` to a `reads_range` pair; a borrowed range passed on whole, shorter, and as `(&p[k], n - k)`. Twins:
  one element too many (`permission`), an unwritten element (`uninitialized`), overlapping owned ranges, a null
  pointer, a negative count, a callee that keeps the range (`lifetime` at the caller's scope exit). Values: kept
  across a `reads_range` call (a `check`), lost across `owns_range` (twin). Placement with its twin
  (`unsequenced`). The unequal-fractions pin.
- **Part D.** A fill loop over an initialized `owns_range` parameter (twin: `i <= n`). A search loop over
  `reads_range` whose `ensures` reads `p[r]` under `r != -1` (twin: a wrong `ensures`). Values kept after a
  read-only loop; lost after a writing one (twin). `p[i]` in an invariant and a measure; `q == a` in an invariant.
  A callee keeping an element in the body; a `return` in the body; a range and a cell parameter in one loop; two
  ranges. Pins: a range atom in an invariant; the assigned range parameter.
- **Part E.** A straight-line `must_init` filler and its caller with an `uninit` array (twin: one element left,
  `postcondition`); `ref_to_uninit` read before a write (`uninitialized`); `--strict-init` forms; the loop filler
  pin. `owning(n)` and `CERASTES_OWNING_N` (twins and every rejection of item 13). `restrict` with a range.
  Array parameters: R1's programs as positives with `native-ok`; `owns_range(a, 3)` on `int a[3]`; `sizeof`
  rejected; the matrix's shapes.
- **The port** `tests/memory/verifast/carrays.c`, with its adaptation header and line map to
  `~/Documents/verifast/examples/carrays.c` (at `0fb63506`): `array_test1` (`owns_range(a, 3)`); `array_test2`
  with a parameter `n` in place of VeriFast's ghost `?n` and `ensures(a[0] == 10 && a[1] == cerastes_old(a[1]))`;
  `to_verify` with `arr[3] == 30` before and `arr[3] == 31 && result == 30` after; `array_test_fail` (`a[-1]`,
  VeriFast's L35) and `array_test_fail2` (`a[5]`, L42), each `out-of-bounds` and `permission` at its line (N1).
  The full list contracts are e1c's.
- **VIR self-test:** the session's cases in `selftest_cases30.inc`; `VIR_CASES_MIN` raised.
- **Byte identity:** `scripts/vprdiff.py` against the baseline after each part: no existing program's `.vpr` or
  `.map` changes in this session.

## 9. Flips (pre-approved with this plan; any other changed outcome stops the session)

- **A. Retired for a positive of the same program, with a twin** (all in `tests/memory/negative/`):
  A1 `array_reject_contract_element.c` (`rejected cerastes-unsupported_expression @ L8`);
  A2 `array_reject_invariant_pointer_element.c` (`@ L11`); A3 `array_reject_invariant_decay.c` (`@ L12`);
  A4 `reject_owning_count.c` (`rejected cerastes-unsupported_annotation @ L8`).
- **B. Stay as written; only the message and the `Pins:` comment change:** `reject_owns_range_pointer.cpp` (its
  pointee is `const`: 0040 item 2; a `reads_range` positive beside it); `array_reject_element_atom.c` (M4.5e2,
  question 236); `array_reject_ref_to_uninit_array.c` (an array is not a pointer: no session);
  `reject_forall_pointer.cpp` (it shares `owns_range`'s message); `reject_owning_count_local.c`,
  `reject_owning_global_count.c`, `reject_owning_field_count.c` (M4.5e2); `array_reject_static_param.c` (M8);
  `tests/scalar/negative/reject_typedef_function_array_param.c` (e1e).
- **C. VIR self-test:** `arr_rej_elem_addr_in_requires`, `_in_ensures` and `_in_atom` in `selftest_cases28.inc`:
  each is kept where its node is still refused (the address as a value, an atom on an element) or replaced by an
  accepted case with a failing twin where its node is now a read. The il-analyst reports which before editing,
  and status lists them. Cut 3 or 4 keeps A4 or the marker pins as they are.
- **D. Goldens and existing Viper:** none.

## 10. Docs (invariant 8; every example is a test)

- **`annotations.adoc`:** "Specification functions" (the count of functions, the table's `owns_range` rows, a
  `reads_range` row, the "does not accept `owns_range`" sentence near 859; `forall` stays "not yet"); "Owning
  parameters" (297: `owning(n)`); the lists near 503 and 524 (M4.5e2); the reserved names (1282).
- **`memory.adoc`:** "Arrays" (480–567) gains "Ranges in contracts" and "Array parameters" (owning and reading a
  range, sub-ranges at calls, an element in a contract, loops over a parameter's range, the two-error index, the
  limits of items 6 and 12); the not-yet lists at 551–566 and 1634.
- **`error-messages.adoc`:** the new messages under `permission`, `uninitialized`, `postcondition` and
  `unsequenced`; each new tag. **`verifying.adoc`:** "Shapes the compilers warn about" (the array-parameter
  shapes); the not-supported lists (1222, 1248–1257); `--strict-init` and arrays. **`overview.adoc`** 84 and 92;
  **`tutorial/index.adoc`** 87–88 and **`tutorial/heap.adoc`** 258–259 (their "not supported yet" sentences; no
  chapter: e1c, 0049 item 4); `ordinary-compilers.adoc` 114.
- **`include/cerastes.h`**'s comments; **`tests/README.md`** (the `memory/` row); **`internal/encoding.md`** §3
  "Ranges in contracts (M4.5e1b)" and §8; the developer guide where it names the quantified shapes; one
  amendment line in each amended record.

## 11. Stop and ask; acceptance

**Stop and ask:** any agent question; any changed outcome outside section 9; any existing program's Viper or map
changing; a P79 or S44 outcome outside section 5's tables; a cut beyond section 7's; a split point; an exit 3 or
a time over 120 s not fixed within its part.

**Acceptance.**
- Before the box run, on the final build: `scripts/regen_vpr.py`, `scripts/lint_viper.py` over its output and
  over a fresh VIR self-test directory, the session's VIR cases through Silicon, R3's run against section 9,
  `scripts/vprdiff.py build/m4.5e1b/baseline/vpr <regen>` with nothing differing. Advisor checkpoint 5.
- `CERASTES_LONG=1 scripts/check.sh` **once**, after the last code change, on the x86-64 box (the branch as a
  git bundle into `~/cerastes-s8/p-ubuntu-26.04`, `~/cerastes-s8/run.sh update` then `check`): green.
- `test_lint_viper.py`, `test_run_tests.py` (the tag-coverage test names each new tag), `run_tests.py
  --check-headers`, `scripts/format.sh --check`, `scripts/build_docs.sh`.
- The status entry (sizes measured, deviations, corrections under question 160) and the handoff with its
  Questions section and e1c's as-built inputs. Commits stay local.

## 12. Inputs (moved here from `internal/handoff.md` §3 at this planning; lines at `dc40377`)

- **e1a's as-built names:** `Type::Kind::Array`, `Program::array_of`, `Expr::Kind::ElemAddr`,
  `Builder::elem_addr`, `elem_place`, `postfix_operand_text`, `Var::zero_filled`, `Stmt::keyword_loc`; in the
  encoder `array_vtype`, `written_as_array_declarator`, `check_array_bound`, `subscript_address`,
  `subscript_read`, `array_decay`, `collect_array_elems`, `array_initializer`; in lowering `ArrayAlloc`,
  `array_alloc_method`, `zero_fill`, `elem_addr`, `array_scope_exit`, `range_perm`, `emit_marked_row`,
  `ArrayPlace`, `LoopWrites::cells`, `inv_elem`, `split_candidates` (an array cell: its first element only), the
  `offset_of == 0` backstops in `holds_token`, `leak_exemption` and `flag_follows`.
- **Costs:** three arrays and two loops 3.4 s; S26's scale program at K = 16 4.9 s, 13.3 s with one array.
- **N1:** `p[1]` under `owns(*p)` fails twice (bound, then permission); tests expect both.
- **From e1a's audit round 3:** `sizeof` of an array parameter is accepted today although clang warns by
  default (item 15 rejects it).
- **Lessons of e1a that bind here:** an allow-list is never computed by `grep` before a build (this session
  expects no Viper change at all); the audit worktree needs no bootstrap (a source-only `git worktree add
  --detach` plus copies of the three binaries built from exactly that source); a row reported "at the loop" uses
  `Stmt::keyword_loc`; proofs made while another agent builds are provisional; a twin that runs further than its
  positive needs an `ensures` that holds at the wider state.

## 13. What e1b leaves, and the questions it opens

- **For handoff §3 at the session's end:** M4.5e2 takes element atoms (236), written range atoms and the
  assigned-parameter pin (237), ranges on other bases and partnerless ranges, `owning(n)` beyond parameters;
  e1c takes the loop filler pin and the full `carrays.c` contracts; M4.8 takes whatever S44 (c), (e) and (g)
  list, and cut 2 if taken.
- **Open questions for the owner** (each built as its recommendation unless answered; the next handoff's
  questions start at 241):
  - **238** `reads_range`'s amount comes from the range's first cell, so a caller holding unequal fractions over
    a range gets a false `permission`. **Recommended:** accept it as a pinned, listed limit.
  - **239** a `must_init` filler that fills with a loop is a pinned false alarm until e1c's quantified
    invariants. **Recommended:** accept.
  - **240** array-parameter shapes that clang or GCC warn about without a flag or at `-Wall` (mismatched bounds
    across declarations, a shorter array argument, if the matrix shows them) are rejected like the compilers.
    **Recommended:** follow the matrix, as 0020 item 5's rule does.
