# Plan: M4.5e1c — Quantifiers (record 0061)

**Approved by the owner on 2026-11-23**, at a session that only planned. It supersedes section 5 of
`.claude/plans/m4-5e1-arrays.md` ("plan mode again after P80 and S45"): **e1c does not plan again.** Its probes
and spikes run first (section 5), each with a decision table that says what is built for each outcome; an outcome
outside its table stops the session and asks the owner.

**Written for this ensemble** (owner, 2026-11-23, as for e1b): an Opus orchestrator; three Sonnet implementers
from `.claude/agents/` (`edg-backend`, `il-analyst`, `test-mutator`); the Opus `soundness-auditor`; a Fable
advisor the orchestrator consults. Section 2 assigns every task; the orchestrator does not re-divide the work.
**The session starts outside plan mode**, from the opening prompt in `internal/handoff.md` §2, and reads the
records this plan names (0060, 0059 and the ones section 6 amends), not all of `internal/decisions/`.

Branch `m4.5e1c-quantifiers`, worktree `.claude/worktrees/m4.5e1c`, `build/m4.5e1c/`, record
`internal/decisions/0061-quantifiers.md`.

## 1. Context

**Why.** e1a and e1b gave arrays, ranges in contracts and loops that carry them, but nothing can be said about
every element at once: a fill loop cannot say which elements it has written (three pinned false alarms), `max`
cannot state its result, and the `carrays.c` port has single-element facts only. e1c gives C its quantifier.

**Owner decisions.**
- **226** `cerastes_forall(f, free...)`: `f` is a `pure` function whose body is one `return E;`; the quantifier
  means `E` with `f`'s first parameter bound, inlined at the site. No Viper function is emitted for `f`, and a
  call of `f` anywhere else stays rejected until e1d.
- **232** `cerastes_old(p[i])` is allowed in the body of a `pure` function that only an `ensures` names.
- **239** the `must_init` loop filler stays pinned; e1c gives it a verifying sibling.
- **At this planning (2026-11-23):**
  - **238** is a false alarm, not unsoundness (0060 item 6: the amount can only be too high for another slot, and
    every slot is still asserted at it). The owner wants it fixed: **e1c runs the spike** (S45 (r)); the fix is a
    short session of its own after e1c, **M4.5e1f**, with its record, lint rule and audit, so that e1c keeps its
    "no existing Viper changes" proof.
  - **242** `owns_range`'s row order D is built **with 238's fix, in M4.5e1f** (the same call rows, one set of
    Viper flips).
  - **`cerastes_old` in a loop invariant** (the function's entry state) is **in e1c, as its last part and its
    first cut** (Part D).
  - The docs lockfile (`main` was red after Dependabot's PR #35) was fixed at this planning (`fabcd25`).
- **Standing:** 69 (no EDG work), 77 (one context), 160 (a new test's expect line may be corrected after its first
  run, each listed in status), 176 (the owner handles CI); `check.sh` once, at the end, on the x86-64 box; no
  test runs while a fix is queued; no Claude trailer; commits stay local.

**Scope.**
- **In:** `cerastes_forall(f, free...)` (a new declaration, both modes) and `cerastes::forall(f, free...)` with
  a function name, in a function's `requires` and `ensures`, a loop's `CERASTES_INVARIANT` and a
  `CERASTES_CHECK`; the pure-body reader; arity and type checks; element reads under the binder with their
  framing, bounds and flags; `cerastes_old` under the binder in an `ensures`; `f__wf` for quantified contracts;
  calls and returns; quantified loop invariants (the fill loop, the `must_init` loop filler, max, `max_index`,
  search, sortedness of neighbours, copy); `cerastes_old` in loop invariants (Part D); the `carrays.c` port in
  full; the tutorial chapter "Arrays and quantifiers"; the spike for M4.5e1f.
- **Not here** (rejected, each pinned, the message naming where it is lifted):

| Construct | Lifted by |
|---|---|
| the lambda form `cerastes::forall([](int i, …) { … }, …)` | M7 |
| `f` a `predicate` (M4.5f1); `f` with a `requires`, `ensures` or `decreases`; a call of a `pure` function anywhere, in `E` too; recursion | e1d |
| a `forall` inside `E` (nested); under `!`; as an operand of `==` or arithmetic; in a `?:` condition; in the guard of an atom; in a function's `decreases` or a loop measure; under `old`; in a `do … while (0)` invariant; an `exists` | M4.8 |
| an `E` with no element read indexed by the bound variable; an index that names the bound variable in another form than `i`, `i + c`, `i - c` (`a[2 * i]`, `a[n - i]`, `a[b[i]]`) | M4.5e2 |
| a quantified read whose base argument is not a bare pointer parameter (contract) or a pointer variable or named local array (loop invariant, `check`): `&a[1]`, `p + 1`, a loaded pointer | M4.5e2 |
| under the binder: structure members (`p[i].x`), array members, global arrays | e1e |
| a trailing argument that reads memory (the read is written inside `f`) | M4.5e2 |
| `cerastes::old` (the C++ spelling) inside a `pure` body: the front end's `old_outside_ensures` stays | M7 |
| `cerastes_old` in a `check`; in a loop invariant, a place not rooted in a parameter the function never assigns | M4.5e2 |
| `reads_range`'s minimum amount (238), `owns_range`'s row order D (242) | M4.5e1f |

**Size** (`src/` insertions without self-tests; e1b landed 2,754 against 2,050–3,050): Part A 200–300, Part B
850–1,200 (encoder 350–500, lowering 500–700), Part C 300–450, Part D 150–250; 1,500–2,200 in all.

## 2. The ensemble

| Role (model) | Owns | Never touches |
|---|---|---|
| **Orchestrator** (Opus) | Steps 0–1; record 0061; the vprdiff verdict per part; user pages in `doc/` and the tutorial chapter's prose; status, handoff, `plan.md`'s Done row; every commit; briefing and resuming the agents | `src/`, `tests/`, `scripts/` (it delegates) |
| **edg-backend** (Sonnet) | `src/backend/**`, `src/driver/**`, `include/cerastes.h`; **in Part A only** `src/vir/**` and `src/viper/**`; the build system, the baseline, and **the joined build of each part** with `scripts/regen_vpr.py` and `scripts/vprdiff.py` on it; `internal/encoding.md`, the developer guide's paragraph | `tests/`, `scripts/lint_viper.py`, `src/vir` after Part A |
| **il-analyst-1** (Sonnet) | P80 and S45; the chapter `## M4.5e1c` of `internal/il-inventory.md`; the fixtures `tests/viper-smoke/memory/s45_*.vpr`. It ends when the chapter is committed | `src/` |
| **il-analyst-2** (Sonnet, fresh at Part B) | `src/vir/**` (`vir.h`, `vir.cpp`, `check.cpp`, `lower.cpp`, `lowered.*`), `src/viper/**`, `src/viper/selftest_cases31.inc`, for Part B | `src/backend`, `tests/` |
| **il-analyst-3** (Sonnet, fresh at Part C) | the same files, for Parts C and D. Briefed with this plan, record 0061, the S45 chapter and Part B's commit | `src/backend`, `tests/` |
| **test-mutator** (Sonnet) | `tests/**` (tests, twins, pins, exactly the Flips, the port), the compiler matrix, `scripts/run_tests.py`, `scripts/lint_viper.py`, `scripts/rejections.py` and their self-tests, the example files under `doc/modules/ROOT/examples/` | `src/` |
| **soundness-auditor** (Opus, read-only) | four rounds and one re-audit (section 7), each on a snapshot commit in `.claude/worktrees/m4.5e1c-audit`, each before its part's commit; resumed between rounds | everything (never edits) |
| **Advisor** (Fable) | consulted by the orchestrator at the checkpoints below | — |

- **Five Sonnet agents over the session, at most three alive at once:** edg-backend, the current il-analyst,
  test-mutator. edg-backend and the test-mutator are resumed between tasks (SendMessage); an il-analyst is
  replaced, not resumed, at the two hand-overs (`lower.cpp` is 20,479 lines). This is the plan, not a deviation.
- **Builds:** every agent that edits `src/` builds its own change through the lock (`lockf
  build/m4.5e1c/build.lock cmake --build build/cmake`). Nothing builds during a directory-wide run.
- **The runner** (`scripts/run_tests.py`; it locks `build/vpr`) has one holder at a time: the test-mutator for
  R3; il-analyst-1 from R3's end until Commit 1 (S45's `.vpr` fixtures, one file per run); the test-mutator from
  Commit 1 on. Everyone else uses `build/cerastes verify <file> --keep-vpr`.
- **The advisor** is the `advisor` tool; if the orchestrator has none, an Agent call with `model: fable`, briefed
  with the question and the paths to read. Step 0 checks that one of the two answers.
- **`lower.cpp`, `vir.h`, `vir.cpp`, `check.cpp`, `lowered.*` have one holder at a time:** edg-backend in Part A,
  il-analyst-2 in Part B, il-analyst-3 in Parts C and D, with a commit between holders. An agent that needs a
  change in another's file asks the orchestrator, who passes it on.
- **Advisor checkpoints:** (1) the record draft with P80's and S45's outcomes mapped to section 5's tables,
  before audit round 1; (2) any outcome outside a table; (3) the split points at Commit 2 and at Commit 3;
  (4) any cut; (5) before the box run.
- **Every agent** (the orchestrator puts this in each briefing):
  - reads `CLAUDE.md`, this plan, record 0061 and the files it is assigned; edits only its own files;
  - any question stops its work and goes to the orchestrator; an owner question goes to the owner;
  - writes each report to `build/m4.5e1c/<role>-report-<n>.md` as well as returning it (the soundness-auditor
    cannot write: the orchestrator cuts each snapshot commit and the audit worktree, copies the binaries built
    from exactly that source, and saves each audit report to `build/m4.5e1c/auditor-report-<n>.md` itself);
  - follows the build and runner rules above;
  - a test generator lives in `build/m4.5e1c/` and is never re-run over the tree;
  - tool code in the owner's style (`scripts/format.sh`; brace initialization, `U`, one declarator per
    declaration, no comment on a brace line); no Claude trailer.
- **A declaration lands with its reader** (P77): `cerastes_forall` is declared in the part that reads it (B).
- **Fail closed between parts:** what a later part lifts is rejected with a diagnostic and a staging pin, written
  and deleted inside the session; in `src/vir` the gate is `check.cpp`'s (it refuses a `Forall` in an invariant
  or a `check` until il-analyst-3 lifts it with the rows of Part C).
- **Session tools are committed** (question 193; `build/m4.5e1b/` and its `r3.sh`, `widen.sh` are gone with
  e1b's worktree): Step 0's rejection rerun is `scripts/rejections.py`, in Commit 1.

## 3. Survey facts (tree at `bdf3581`; read, not run; names, with lines where they help)

- **Nothing needs the EDG fork.** P53: in C23 `cerastes_forall(nonneg, a, n)` keeps `enk_routine nonneg` among
  the variadic arguments (`&nonneg`: `eok_address_of`); in C++20 `cerastes::forall(nonneg, a, n)` gives
  `enk_constant &nonneg`, a lambda `enk_lambda`. P53's header was the probe's own.
- **Header** (`include/cerastes.h`, 161 lines): `template <class F, class... Free> bool forall(F body, const
  Free&... free) noexcept;` in `namespace cerastes`; **no `cerastes_forall`**; `CERASTES_PURE` is
  `[[cerastes::pure]] inline` (`inline` alone for other compilers).
- **Names:** `encode.cpp` `enum class SpecFn` (8295; no `forall` member), `spec_fn` (8318: `cerastes::forall` is
  `SpecFn::unsupported`, the only such name, matched by name without the header test), `call` (10028): in a
  contract or a loop clause `unsupported_annotation` "\"cerastes::forall\" (a quantifier: M4.5e1c)" (10076, the
  only message in `src/` that names e1c); **in a `check`** the 10045 block is skipped and the call is
  `call_in_annotation`; in code the front end's `spec_call_in_code`. `il_query.cpp` `is_spec_function_name` (54),
  `annotation_checks.cpp` `is_spec_name` (175) and `is_protected_macro_name` (1632), `src/driver/edg_args.cpp`
  `is_cerastes_name` (110) hold `forall`, none holds `cerastes_forall`.
- **`pure` functions today:** `Encoder::run` (21412) skips every spec routine before `check_signature` and
  `function_of`: no `vir::Function`, **a contract written on a `pure` function is neither encoded nor rejected**;
  it runs `reject_markers_in_spec_function`, `reject_struct_value_in_spec_signature`,
  `reject_globals_in_spec_body` (0041 item 12). `annotation_checks.cpp` checks neither the body's shape nor its
  side effects; `check_old_in_spec_body` (661) reports `old_outside_ensures` for **`cerastes::old` only** (the C
  macro `cerastes_old` expands to `cerastes_old_`, which it does not match). The one-`return` reader exists for
  lambdas: `expr_tree.cpp` `lambda_body` (243). Pins: `tests/annotations/negative/old_in_pure_body.cpp`,
  `old_dependent_in_pure_body.cpp`, `old_address_in_pure_body.cpp`, the doc example
  `annotations/errors/old_in_pure.cpp` (all the C++ spelling); `tests/scalar/static_assert_pure_body.c`.
- **Two resolution paths in the encoder:** an annotation's parameters are `enk_param_ref`, resolved by position
  to `Formal` (`param_reference`, 7410; `interpret_param_ref`); a body's are `enk_variable`, resolved through
  `Ctx::vars` (`variable()`, 5799, which refuses a non-global variable under `cx_.contract`). **No map from a
  variable to an expression exists.** `Ctx` flags: `contract`, `annotation`, `check_clause`, `loop_clause`,
  `loop_invariant`, `annot`, `old_depth`. Readers: `contract_element_address` (684), `subscript_address` (816),
  `contract_read` (9131), `old_c` (8988: `old_outside_ensures` unless `cx_.annot == Annot::ensures`), `old_call`
  (7752, C++, contracts only), `annotation_statement` (14482, `check`), `encode_loop_clauses` (14520).
- **VIR** (`vir.h`): no quantifier, binder or bound variable. `check.cpp`: a `Local` in a contract is refused
  (2554); `Old` outside an `ensures` is refused (2599); `Deref(ElemAddr(Formal, index))` is a contract read
  (2754, 3086); `same_pure_expr`, `reads_heap`.
- **Lowered program:** `LExpr::Kind::Forall` (`lowered.h` 57–119) has **one binder name, always `Int`, and
  trigger sets of one term**; every site uses the binder `k` (`range_perm` 3791, `range_quant` 3820, the loop
  conjuncts 16155–16211, `array_alloc_method`). `emit.cpp` 117. `emit_marked_row` (3807) sets
  `expected_reason = "insufficient.permission"`; `report.cpp` `map_errors` accepts no other reason.
- **Contracts in lowering:** `contract_read` (6947–7220): the `ite` chain over the framing disjuncts of the
  sites seen so far (a range site: `g && in_range`), `old(...)` under `cold_`; the `Wf` branch (7146) returns a
  ghost formal `hv_…_pre|_post` (`GKey`, `elem_key`) and one `permission` obligation. **`f__wf` holds no heap
  permission** (`wf_method` 19899–20376; `atom()` returns only its pure line in `Wf`); each obligation is
  `assert prior && guard ==> cond`. **`wf_method` also writes**, over the same ghosts: the partners-by-path rows
  of cell, range and block sites (0039 item 7, 0060 item 3), `overlap_obligations` (6772), the ties of bodiless
  sites through a load or on a global (0045 item 10, 0054), the `not_invalidating` rows (0058 item 2), the
  `decreases` clauses' obligations, and the assumed `facts_` (equal addresses have equal ghosts; a `reads`
  place's post value is its pre value). `push_clauses` (19413) splits a clause per conjunct only when it holds a
  range atom. At a call (`call()` 9561): `call_atoms` first, then **one `precondition` obligation per clause**,
  read in the caller's state; the Viper call keeps an `internal` backstop; `bind`/`bound_args` give a range
  atom's base a local (S44 F-b1). At a return (`postconditions()` 13217): one `postcondition` row per clause.
  **Lowering has no `inhale` and `LStmt` no inhale kind.**
- **Loops:** invariants in `CMode::Inv` (16350–16393); `inv_elem` (4037: a named array: `ite(in bounds, a[e],
  dummy)` and two obligations, `out-of-bounds` and `uninitialized`), `inv_read_at` (4129) with
  `inv_range_chain`, `framing_condition` (4195: `prior && guard ==> OR D_i`, asserted at entry 16489 and at the
  back edge 16738); **an obligation is an `invariant` line of its own before the user's clause**
  (`obligation_clause`, 19127); `stmt_touches`/`expr_touches` (13371, 13321) decide whether the loop carries its
  places. The array and range places' flag conjunct is "flags kept" only (`old[lh](flag) ==> flag`), and the
  values-kept conjunct is dropped once a write reaches: the cause of both e1a pins.
- **`check`** lowers as code (`CMode::None`, 11521): `elem_addr` (8522) and `deref()` (4285) flush
  `null-dereference`, `lifetime`, `out-of-bounds`, `permission` (`perm(v) > none`) and `uninitialized` rows as
  asserts before the check's own. S24: **`perm` may not stand under a quantifier in an `assert`**, and a trigger
  with `+` is "not a valid Trigger".
- **Lint** (`scripts/lint_viper.py`): `QUANT_HEAD_RE` (3315) admits one `Int` binder and one trigger set;
  `lint_quantified_rows` (3531) admits only the range shapes of 0059 and 0060 (`range_atom_row_problem` 3413,
  `_loop_conjunct` 3492); a bodiless `function` and any untagged `inhale` are refused.
- **Self-tests:** the latest case file is `selftest_cases30.inc`; `VIR_CASES_MIN=2044` (`scripts/check.sh`);
  `test_lint_viper.py` 809 tests, `test_run_tests.py` 1,221.
- **Pins e1c meets** (`tests/memory/negative/`): `array_loop_fill_then_read_false_alarm.c` (`uninitialized @
  L19`), `array_loop_self_write_false_alarm.c` (`postcondition @ L23`), `range_marker_pin_loop_filler.c`
  (`postcondition @ L18`), `reject_forall_pointer.cpp` (the lambda form, `@ L9`), `array_loop_bool_post.c` (its
  comment names e1c; exact counting needs e1d). **None of the first three programs writes a quantified
  invariant, so each still fails at its line after e1c.**
- **Already settled:** S24 (the absolute-slot binder, `e2_array_sum.vpr`'s `BETWEEN`); S43 G2 (a failed marked
  row ends its path), G3 (`old[label]` under a quantifier), N1, N2 (a conjunction in one invariant clause is two
  errors); S44 (f) (a guarded read beside the quantified permission), (g) (`search`'s quantified progress
  invariant verifies after the place's permissions, 1.9 s), F-b1, F-g1.

## 4. Step 0 and Step 1

1. **Orchestrator.** `git fetch`; branch from `origin/main` (it holds this plan's commit and the lockfile fix
   once the owner pushes; otherwise from local `main`, and say so in status). `git worktree add
   .claude/worktrees/m4.5e1c -b m4.5e1c-quantifiers <base>`; submodules with `--reference` (edg, silicon, then
   silver); `scripts/bootstrap.sh`. `npm ls @antora/logger` in `doc/` shows one hoisted copy (**nested copies:
   stop and ask**). CI recorded only (`gh run list --workflow ci.yml --limit 5`). One question to the advisor,
   to check that it answers.
2. **edg-backend.** The baseline before any change to `src/`: `scripts/regen_vpr.py` and the VIR self-test into
   `build/m4.5e1c/baseline/`; frozen binaries in `build/m4.5e1c/cerastes-base/`.
3. **test-mutator, on the frozen build** (`build/m4.5e1c/s0/`, both modes):
   - **T0** `scripts/rejections.py <out-file> [--base <file>]`: runs every `rejected` test under
     `tests/scalar/negative`, `tests/memory/negative`, `tests/vlibc/negative` and `tests/annotations/negative`
     and every `frontend-error` test under `tests/annotations`, writes one line per run (file, mode, tag, line)
     and, with `--base`, prints what differs. A self-test in `scripts/test_run_tests.py`. Committed in Commit 1.
   - **R1** today's outcome of the four pins of section 3 and of `static_assert_pure_body.c` (unchanged by this
     session), and of one program per positive of section 8 (each must be rejected at the `forall` today). **And
     the fact the `must_init` sibling rests on:** `range_marker_pin_loop_filler.c`'s loop with the invariant
     `i == 0 || p[0] == 0` added (an element of a `must_init` parameter read in a loop invariant; the cell form
     verifies today, `init_loop.c`), and a `must_init` function whose `ensures` reads `p[0]`. Expected: both
     verify. **If the invariant read is refused:** Part C lifts it only when the refusal is one encoder site
     and `inv_read_at` already carries the flag obligation; anything else stops and asks (the sibling is
     protected).
   - **R2** today's diagnostic for every construct of section 1's scope and table, among them: `cerastes_forall`
     (undeclared), `cerastes::forall(f, a, n)` in a `requires`, an invariant and a `check`; `cerastes_old(*p)`,
     `cerastes_old(p[i])` and `cerastes_old(n)` in a loop invariant, both modes (Part D's baseline; the survey
     found no pin: the test-mutator adds one per shape now, to be retired by Part D); a `pure` function with a
     contract. `grep -n 'M4\.5e1c' src/` (the survey found one message).
   - **R3** `scripts/rejections.py build/m4.5e1c/s0/rejected-base.txt`. After each part the same run may differ
     only by section 9.
   - **M** the compiler matrix (clang and GCC, both modes, no flag / `-Wall` / `-Wall -Wextra`) for the new
     source shapes: a `CERASTES_PURE` function that only annotations name (`inline`, with and without a prior
     prototype); a body with `cerastes_old(a[i])`; a `size_t` bound variable under `0 <= i`. **Rule (not a
     decision):** every positive compiles with `-Wall -Wextra -Werror`; a shape that warns without a flag or at
     `-Wall` is written the other way in the tests and noted in the record; nothing new is rejected from M.
   - R3 is the only `run_tests.py` run until il-analyst-1's P80 has finished; S45's runs start after R3.

## 5. P80 and S45 (il-analyst-1, before any encoder code; results in `internal/il-inventory.md`, `## M4.5e1c`)

**P80 (IL, both modes, `build/m4.5e1c/p80/`; every claim with its `--il_display` or `--cerastes_walk` excerpt;
`cerastes_forall` declared in the probe's own header after `<cerastes.h>`, as P53 did).**

| Probe | Expected | If not |
|---|---|---|
| (a) `cerastes_forall(f, a, n)` and `cerastes_forall(&f, a, n)` in `requires`, `ensures` (the result name as an argument), an invariant, a `check`; C23 and C++20 | the callee is `enk_routine cerastes_forall`; the first argument is `enk_routine f`, `eok_address_of(enk_routine f)` or a constant address of `f`, under at most one compiler-generated conversion; each trailing argument is the written expression under at most one compiler-generated promotion | a third shape for `f`: **stop and ask** |
| (b) `cerastes::forall(f, a, n)` with an `int`, a pointer parameter, a local array, the result name and `n + 1` as trailing arguments | `enk_constant &f` (P53); each trailing argument reaches the IL as the written expression under one reference binding that a single peel rule reads (an array binds as the array) | **cut 3 without asking:** in verify mode the namespace spelling with a function is `unsupported_annotation` ("write `cerastes_forall`"; M7), pinned; `cerastes_forall` is the spelling in both modes |
| (c) the body of a `pure` function: `inline` and not, defined before and after the function that names it, with a prototype first; `return E;` alone, inside an extra block, with a declaration, with two statements | the routine's scope gives `assoc_block`; one `stmk_return` with its `expr`, as `lambda_body` reads a lambda; parameters are `enk_variable`s that map to the routine's parameter positions; `a[i]` is the subscript node of code (P78 (c)); the body is in the IL although nothing calls `f` | the body of an unreferenced `inline` function missing in a mode: **stop and ask**. An extra block around the one `return`: read through it; anything else is "not one `return`" |
| (d) the front end today, on: naming `f` (not calling it) as an argument in an annotation; `cerastes_old(a[i])` and `cerastes::old(a[i])` inside a `pure` body; an assignment or `++` inside a `pure` body | naming `f` in an annotation raises nothing (in code it is `spec_call_in_code`); the C spelling of `old` raises nothing, the C++ spelling `old_outside_ensures`; a side effect in the body raises nothing (the reader must refuse it) | naming `f` is reported: one exemption for the first argument of the two `forall` spellings in `annotation_checks.cpp`, with its front-end tests; anything wider: **stop and ask**. The C spelling of `old` refused: `old` under the binder and Part D are cut (cuts 1 and 5), no front-end expectation changes |
| (e) one argument too many, one too few, an `int` for a `long` parameter, a `char` for an `int`, `int *` for `const int *`, `const int *` for `int *` | no front-end diagnostic in either spelling; the IL shows each written argument's type under its generated conversion | — (the encoder's rule compares the written types; item 3) |
| (f) `cerastes_old(p[i])`, `cerastes_old(*p)`, `cerastes_old(n)` in a loop invariant, `i` a local (Part D) | the shapes of an `ensures` (P58, P79 (d)): `address_of(subscript(p, i))`, the folded `&*p` in C, `&n` | a new shape: **stop and ask** before Part D only |

**S45 (hand-written Viper in the shapes of section 6; fixtures `tests/viper-smoke/memory/s45_*.vpr`; every row
expanded as lowering writes it, from the frozen build's kept Viper of e1b's tests; each positive with failing
twins; one file at a time through `scripts/run_tests.py`; times on a quiet machine).** A twin must fail with
**exactly one** error unless the table says otherwise. Every fixture method ends with an `assert false` twin
(vacuity). User quantifiers bind `q0`, `q1`, …; `i` stands for `q0 - offset_of(p) - c`.

| Spike | Programs | Built when it passes | If it does not |
|---|---|---|---|
| **(a) one read class, contracts** | `reads_range(a, n)` with `requires forall q0 :: { loc(block_of(a), q0) } (lo_T <= i && i <= hi_T) ==> (!(0 <= i && i < n) \|\| (F(q0) ? loc(block_of(a), q0).v_int : 0) >= 0)`, the same form in an `ensures` with the result; the body uses the fact at a read; a caller on a local array proves the `requires` (the `precondition` row after the atom's rows, the base in a local) and uses the `ensures`; a guarded form (`n == 0 \|\|` outside the quantifier). Twins: a caller with one negative element (`precondition`); a wrong `ensures` (`postcondition`) | item 6's single-class form and item 7's rows | the plain read where the framing is the atom's own range; a correct program that fails: **stop and ask** |
| **(b) several read classes** | neighbours, `a[i] <= a[i + 1]`: a `requires` used at two reads; a loop that checks sortedness with the quantified invariant; a failing twin of each. Two arrays, `a[i] == b[i]`: a copy loop that proves it; a caller that uses it. Each in **T2** (one binder per class, `q1 == offset_of(b) + i + c` in the antecedent, one trigger set holding every class's term) and **T1** (one binder, the other reads as computed terms, one term) | **T2**, when every positive verifies and every twin fails once, each within 30 s | **T1** under the same test. Neither: **cut 4 without asking** (an `E` with more than one read class is rejected and pinned; single-class bodies are built). Recorded, not gating: a use where only one of the two terms occurs in the program |
| **(c) loops** | the fill loop over `int a[3];` then a read (the first pin's sibling): the flag clause `forall q0 :: { loc(B, q0) } reach(q0) ==> loc(B, q0).d_int` as an invariant line before the user's clause; the fill loop over a `must_init` `owns_range` parameter, to its return row; a loop that writes `a[i] = a[i]` under a quantified value invariant; max and `max_index` over `reads_range`; the search of S44 (g) with its progress clause in this form. Twins: an invariant one element too far (`uninitialized`, at entry); a body that skips the write (`invariant-preserved`); `<` for `<=` in max (`postcondition`) | item 9 | the fill loop or max failing, or any of them over 30 s: **stop and ask** (protected) |
| **(d) `f__wf`** | one contract, `requires(reads_range(a, n) && forall small)`, `ensures(reads_range(a, n) && forall le(…, r) && forall kept)`, in both variants. **M (today's heap-free method, extended):** a read under a binder is a ghost function of the absolute slot, one per base parameter and state (`hvq_<p>_pre(s)`, `_post`, in a tagged domain without axioms); assumed beside today's `facts_`: its type's range; equality with a scalar ghost of the same block at that slot, and between two parameters' functions on one block; post equals pre inside a `reads` atom with a partner; in `f__wf` a quantifier's trigger terms are the ghost applications. **Every row `wf_method` writes today stays as it is.** **H (a heap form):** `f__wf` takes the `requires` atoms in their method form as its own `requires`, asserts the `requires` clauses' obligations over guarded heap reads, calls a bodiless `f__wfp` (the `requires` atoms in, the `ensures` atoms out, each `reads` place with a partner `== old`), then asserts the `ensures` clauses' obligations. Discriminators: (1) a post-state read under a binder; (2) `old` under a binder; (3) an obligation that rests on an earlier quantified clause (`a[i] + 1` under `a[i] < 100`), and a later unquantified one (`a[0] + 1`); twins: (4) a read one past the range (`permission`), an overflow under the binder (`overflow`); (5) vacuity (`assert false` last: reported); (6) e1b's guard-mismatch contract and an overlap twin (their rows still fail, alone). **For e1d, recorded only:** (7) `r == sum_to(a, n)` with a heap-reading Viper function, under H; under M, the application as an uninterpreted ghost with the callee's `ensures` as a fact and its `reads_range` framed by interval inclusion | **M**, when (1)–(6) pass with one error per twin: nothing of today's `f__wf` changes for any contract, and a contract without a quantifier keeps its method byte for byte | **H**, when it passes (1)–(6), **with this boundary:** a contract that holds a quantifier may then hold only value clauses, cell atoms on a parameter's own cell (`*p`), range atoms and guards that read no memory; a place through a load or a member, a block site, a structure pointee, a `not_invalidating` parameter and an `ensures` atom without a partner beside a quantifier are each rejected and pinned (M4.8), and the heap form carries the partners-by-path rows and `overlap_obligations`. Neither: **stop and ask** |
| **(e) `old` under a binder** | `set_one(a, n, j, v)` with `ensures forall: !(0 <= i && i < n && i != j) \|\| a[i] == old(a[i])`; a caller that reads two untouched elements after it. Twin: the body writes a second element (`postcondition`) | item 8 | **stop and ask** |
| **(f) `check`** | per read class, in this order: for a pointer base the null row and the liveness row (`kq := perm(lvloc(…).lv)` bound first, then `forall q0 :: reach ==> kq > none`); the bound; the marked row `assert forall q0 :: { loc(B, q0) } reach(q0) ==> acc(loc(B, q0).v_int, wildcard)` (`permission`); the flag row; then the fact (`check`). On a local array, an `owns_range` and a `reads_range` parameter. Twins: a range a callee kept (`permission`); one unwritten element (`uninitialized`); an index outside a local array (`out-of-bounds`); a false fact (`check`) | item 10 | the marked row refused or not one error: **cut 2 without asking** (`forall` in a `check` stays rejected, pinned, M4.8) |
| **(g) `old` in a loop invariant** | `inc_all(a, n)` over `owns_range`, bounded values, `ensures forall: a[i] == old(a[i]) + 1`; invariants "below `i`: `== old + 1`" and "from `i` on: `== old`" with Viper's `old(…)`; a cell form `*p == old(*p) + i`. Twin: an invariant one element off | item 12 | **cut 1 without asking** |
| **(k) cost** | fill, max and copy with a symbolic `n` and with an array of 4; one method with two quantified invariants and three calls of quantified contracts; S43 (h)'s scale program at K = 16 with one quantified callee | recorded | **over 60 s: stop and ask** |
| **(r) for M4.5e1f; nothing is built from it here** | question 238's pin (`range_call_reads_unequal_pin.c`'s kept Viper) with the amount from a minimum over the range: `var m: Perm`, `inhale forall k :: { loc(B, k) } lo <= k && k < hi ==> m <= perm(loc(B, k).v_int)` and a witness `inhale lo <= w && w < hi && m == perm(loc(B, w).v_int)`, per field, under `n > 0`; then e1b's rows at `min(m_v, m_d, perm(L), own amounts) / (N + 1)`. Twins: a slot held at `none` (one error); `assert false` after the inhales (reported: they are consistent). If Silicon refuses `perm` under a quantifier in an `inhale`: the same facts as the `ensures` of a bodiless helper. Order D for `owns_range` in the built rows, with the forwarded `(p, n + 1)` twin | **recorded only**: accepted or refused, the pin's verdict, times; into the chapter and handoff §3 "M4.5e1f". Only a program Silicon runs becomes an `s45_r_*` fixture; a form it refuses to type-check is quoted in the chapter with Silicon's message | recorded as it falls; no stop |

**Stop rules for all of S45 (not (r)):** a second Silicon error after a marked row; `qp.not.injective`; "not a
valid Trigger" on a shape whose row names no fallback; an outcome no table names.

## 6. Record 0061 — quantifiers (the orchestrator writes it before code from this section; revised only by section 5's tables and audit round 1)

Amends 0006 item 9 and its `old` rule; 0011 items 4, 7 and 10's messages; 0028 item 2; 0029 items 3 and 4; 0037
item 7; 0038 item 6; 0041 item 12; 0059 item 8; 0060 items 1, 11, 12 and 16.

1. **Scope:** section 1, with its table.
2. **Spellings.** `<cerastes.h>` declares `cerastes_forall(...)` in both modes (C23's variadic form, `noexcept`
   inside `extern "C"` in C++), beside the other `cerastes_` names; `cerastes::forall`'s declaration does not
   change. A new `SpecFn::forall`; both names join the name lists of section 3 and pass
   `declared_in_cerastes_header`. In code they are `spec_call_in_code`.
3. **The quantifier** `forall(f, e_1, …, e_m)`.
   - **Positions:** a function's `requires` and `ensures`, a loop's invariant, a `check`; as a conjunct, on
     either side of `||` or as an arm of `?:` (a positive position). Every other position is rejected
     (`unsupported_annotation`, section 1's table row).
   - **`f`** is named as `f` or `&f`: a `pure` function (not a `predicate`), defined in a verified file of the
     unit, whose body is exactly one `return E;`; no `requires`, `ensures` or `decreases`; not a member function,
     a template instance or variadic. Its first parameter, the **bound variable**, has an integer type other
     than `bool` and the enumerations; every other parameter is a `bool`, an integer, an enumeration, or a
     pointer to one of those or to a pointer. A function that is not such a function: **`forall_function`**
     (new tag; its message says which rule); a contract on `f` and a `predicate`: `unsupported_annotation`
     (e1d, M4.5f1).
   - **Arguments:** exactly one per remaining parameter; the written argument's type (under at most one
     compiler-generated conversion: C's promotions, C++'s reference binding) is its parameter's type up to
     top-level qualifiers, `T *` for `const T *` allowed: **`forall_argument_mismatch`** (new tag) otherwise.
     An argument reads no memory. A pointer argument whose parameter `E` subscripts with the bound variable is,
     in a contract, a bare pointer parameter; in a loop invariant or a `check`, a pointer variable that is not
     address-taken or a named local array (its decay collapsed to the array, so `inv_elem` and the array rows
     still match). The rest is rejected (table).
   - **Meaning:** for every value `i` of the bound variable's type, `E` with the other parameters replaced by
     the arguments, each argument read once, at the site, in the site's state.
4. **The reader** (encoder). `E` is encoded at the site, in the site's mode, with a substitution map from `f`'s
   parameters to the bound variable and the encoded arguments (`Ctx`; `variable()` consults it first). `E` may
   hold what an annotation expression of the site may hold, element reads `p[e]` and `*p` through `f`'s pointer
   parameters, and `cerastes_old` by item 8. Refused in `E`, at the site, each with the tag the same construct
   has in an annotation and a pin: a call (a `pure` function: e1d), an atom, a `forall`, an assignment, `++`,
   `--`, a comma, a global (0041 item 12 holds for the inlined body). The shape rules that reach annotation
   expressions (R5, R9) reach `E`; `static_assert_pure_body.c`'s exemption is unchanged. A `pure` function that
   no `forall` names is still not read.
5. **VIR.** `Expr::Kind::Forall` (the bound variable's id and `IntType`, the body, the site's spelling
   `forall(f, a, n)` as `c_text`, `f`'s name) and `Expr::Kind::Bound`; `Builder::forall`, `Builder::bound`.
   `check.cpp` backstops items 3, 6 and 8: positions, no nesting, no atom, call or write inside, `Bound` only
   under its `Forall`, the read classes' shape, `Old` inside only where item 8 or 12 allows.
6. **Binders and triggers** (lowering; 0028 item 2).
   - A **read class** is a pair (base, constant `c`) of the reads `base[i + c]` in `E`. `E` has at least one
     class, and every index that names the bound variable has the form `i`, `i + c` or `i - c` (rejected
     otherwise, in the encoder and in `check.cpp`).
   - The first class's absolute slot is the binder `q0`, and `i` is `q0 - offset_of(base_0) - c_0`. The
     antecedent holds the type's range of `i`. Further classes are bound as S45 (b) decides (T2: a binder
     `q_j` each, `q_j == offset_of(base_j) + i + c_j`, one trigger set over every class's term; T1: computed
     terms). The trigger terms are `loc(block_of(X), q_j)` with `X` a variable (S44 F-b1): the formal in a
     method's contract, the bound local at a call (`bind`, extended from a range atom's base to every pointer
     argument a callee's quantifier subscripts), the pointer variable or the array's local in a loop and a
     `check`. In `f__wf` (variant M) no heap term exists: the trigger terms are the ghost applications
     `hvq_<p>_<state>(q_j)`.
   - User quantifiers bind `q0`, `q1`, … (the range quantifiers keep `k`); `LExpr::Forall` takes several
     binders and trigger sets of several terms.
7. **Meaning by mode.**
   - **The method's contract:** a clause that holds a quantifier is split per conjunct (`push_clauses`); a
     guard stands outside (`g ==> forall …`); each read under the binder is its own `ite` chain over the
     framing disjuncts (0037 item 7, 0060 item 5), with `old(…)` by item 8.
   - **A call:** the clause's `precondition` row, as today (one per clause, after `call_atoms`' rows, in the
     caller's state, the bases bound); the Viper call's own precondition stays the `internal` backstop. **A
     return:** the clause's `postcondition` row. Callers learn an `ensures` from the method's postcondition, as
     today.
   - **`f__wf`:** the variant S45 (d) chose. Expected, **M:** the method stays heap-free and keeps every row
     it writes today (framing, 0010 item 11's obligations, partners by path, the overlap obligations, the ties,
     the `not_invalidating` rows); a read under a binder is the ghost function of its base parameter and state
     at the read's absolute slot, with the assumed facts of S45 (d), each argued in the record as 0037 item 7
     argues today's (one location in one state has one value; a `reads` place is not written between entry and
     return). Under the fallback **H**, S45 (d)'s boundary is part of item 3's rejections. Either way a
     contract without a quantifier keeps its `f__wf` byte for byte.
   - **Obligations of `E`** (0010 item 11's, framing, and in loops and checks the bound and the flag): each is
     one quantified row over its own read class's binder (an arithmetic obligation: over `q0`), under the
     site's `prior` and the path inside `E`. It is reported **at the `forall` site**, in the operation's
     category, its message naming `f` ("in `le_at`: reads `a[i]` without owning it"), with a note at the
     operation's line in `f`.
8. **`old` under the binder.** `cerastes_old(p[e])` and `cerastes_old(*p)` in `E` (the C spelling, in both
   modes; `cerastes::old` there stays the front end's error). A `forall` whose `f` holds one is accepted where
   `old` is accepted: in an `ensures` (and by item 12 in a loop invariant); elsewhere `old_outside_ensures`, at
   the site. The read is the entry state's, framed by the `requires` atoms.
9. **Loops** (0038 item 6, 0059 item 8, 0060 item 11). A quantified invariant comes after the places' own
   conjuncts. Before it, in evaluation order, one invariant line per obligation (`obligation_clause`), each
   over its read class's own binder and trigger: the framing against the loop's places (`permission`, "this loop
   invariant reads `a[i]`, which this loop does not hold"; asserted at entry and at the back edge as
   `framing_condition` does), a named array's bound (`out-of-bounds`), and **the flag** (`uninitialized`): a
   quantified invariant that reads elements states that they hold values, at the head and after the loop. A
   loop whose invariant holds a quantified read carries its places (`expr_touches`). No quantifier in a measure.
   This gives the fill loop, the `must_init` loop filler and the self-write loop their verifying forms; the three
   pinned programs, which write no such invariant, fail as before.
10. **`check`.** The rows of S45 (f), then the fact (`check`).
11. **The lint.** A user quantifier: binders `q<j>: Int`, one trigger set of `loc(block_of(X), q<j>)` terms with
    `X` a variable, a body with no `acc`, `perm`, `forperm`, `inhale` or `forall`; admitted in a method's
    `requires` and `ensures`, an `invariant` and an `assert`. The marked `wildcard` row only as an `assert`.
    `f__wf` (variant M): one domain per function, tagged `// cerastes:domain-ok 0061`, holding ghost functions
    and no axiom; in a `__wf` method a user quantifier's trigger terms are ghost applications, and the assumed
    facts over the ghost functions have exactly the four shapes of S45 (d). (Variant H instead: `acc` in a
    `__wf` method's `requires` in the shapes a method's contract admits; `__wfp` bodiless, tagged
    `// cerastes:trusted-contract`, called only from its `__wf`.) Each rule with twins. Nothing else that holds
    a `forall` is admitted.
12. **`cerastes_old` in a loop invariant** (Part D). The C spelling; a place that `old` accepts in an `ensures`,
    rooted in a pointer parameter the function never assigns, or a by-value parameter (its entry value); an
    index is read in the current state and reads no memory. Lowered as Viper's `old(…)` of the chain over the
    `requires` sites; its framing is an invariant obligation (`permission`: "this loop invariant reads the entry
    value of `p[i]`, which the function's `requires` does not own"). A marked parameter's place is refused as in
    contracts. Not in a measure, not in a `check`.
13. **Categories:** none new. **Tags:** `forall_function`, `forall_argument_mismatch`. **Facts:** P80, S45.

## 7. Parts, owners, order

| Part | Content | edg-backend | il-analyst | test-mutator |
|---|---|---|---|---|
| **A** plumbing, behaviour-neutral | VIR and lowered-program plumbing; nothing accepted yet | `Expr::Kind::Forall` and `Bound`, the builders, every kind switch (`same_pure_expr`, `reads_heap`, `expr_touches`, `writes_in_expr`, the sequencing walkers, the printers; `check.cpp`: refused everywhere); `LExpr::Forall` with several binders and multi-term trigger sets, `emit.cpp` | il-analyst-1: P80, S45 | Step 0 (T0, R1–R3, M); the lint rules of item 11 against S45's fixtures, with twins |
| **B** contracts | items 2–8 | the header's declaration; the name lists; `SpecFn::forall`; the reader (`f`'s resolution and rules, the body's one `return`, arity and types, the substitution map, `E` in the site's mode), wired in `call()` for contracts; item 8's rule in `old_c`; the two tags in `diagnostics.def`; `annotation_checks.cpp` only as P80 (d)'s table says | **il-analyst-2**: `check.cpp`'s rules; the read classes and binders; `Forall` in `Method`, `Call` (the bound bases) and `Ret`; the obligations under binders; `contract_read`'s `Wf` branch and `wf_method` in the variant S45 (d) chose (M: the ghost functions, their domain and facts; no existing row touched); VIR cases in `selftest_cases31.inc` | Part B's tests; staging pins S1, S2 |
| **C** loops and checks | items 9, 10 | `call()` for loop invariants and checks; the argument rules for locals and arrays | **il-analyst-3**: `Forall` in `CMode::Inv` (`inv_elem`, `inv_read_at`, `framing_condition`, `obligation_clause`), `expr_touches`; the check rows | Part C's tests and the pins' siblings; S1, S2 deleted |
| **D** `old` in loop invariants | item 12 | `old_c` in a loop invariant; the reader's rule for item 8 extended | il-analyst-3: `Old` in `CMode::Inv`, its obligation | Part D's tests; R2's pins retired |

**Staging pins** (test-mutator writes them with Part B; Part C deletes them): **S1** a `forall` in a loop
invariant, **S2** a `forall` in a `check`, each still rejected after Part B.

**Order of work.**
1. Step 0 and Step 1. Then, in parallel: il-analyst-1 on P80 and S45; edg-backend on Part A; the test-mutator on
   T0, R1–R3 and M, then the lint rules; the orchestrator on record 0061's draft.
2. **Commit 1:** record 0061 (draft, revised by section 5's tables), the `## M4.5e1c` chapter, the S45 fixtures,
   `scripts/rejections.py`, R2's pins. Advisor checkpoint 1, then audit round 1 on it. il-analyst-1 ends.
3. **Part A** may be built before round 1 returns (it accepts nothing). It is proved neutral (`vprdiff`: every
   `.vpr` and `.map` identical) and joins Part B's commit.
4. **Part B starts only when audit round 1 has returned and the record holds its blocking fixes** (invariant
   6). edg-backend and il-analyst-2 work in parallel against Part A's VIR; the test-mutator writes the tests from
   the record, expectations pre-registered. Audit round 2 on the snapshot. **Commit 2** (Parts A and B). **Split
   point 1.**
5. **Part C:** il-analyst-3 on the lowering; edg-backend on the wiring. The tree holds nothing of Part D.
   Audit round 3 on the snapshot. **Commit 3.** **Split point 2.**
6. **Part D** (unless cut): il-analyst-3, then edg-backend's part. Audit round 4 on the snapshot. **Commit 4.**
7. The port, the doc and tutorial examples (test-mutator); the user pages and the chapter (orchestrator);
   `internal/encoding.md` and the developer guide (edg-backend); the re-audit of blocking fixes; the checks of
   section 11; record 0061 accepted; status, handoff, `plan.md`'s Done row.

**Proofs, per part.**
- **Each half alone:** the il-analyst proves its half by the new VIR cases through `cerastes-vir-selftest` and
  Silicon, which need no encoder; edg-backend proves its half by its build and by R3's run of the rejections
  against section 9. Both run `scripts/format.sh`.
- **The joined proof**, once both halves have reported: edg-backend makes the joined build, runs
  `scripts/regen_vpr.py` into a directory of its own and `scripts/vprdiff.py` against the baseline (expected in
  every part: nothing differs); then, with builds held, the test-mutator runs the part's tests once. A failure
  goes through the orchestrator to the half that owns the failing file; the joined proof is repeated once, after
  the last fix.
- The audit snapshot is taken from the tree the joined proof passed on.

**Audits** (one resumed Opus reviewer; a snapshot commit and a copy of the binaries built from exactly it):
- **Round 1, the record with P80 and S45:** reproduce the gates; can a trigger choice make an assumed quantifier
  stronger than its source; is the type-range guard on the right side; is the chosen `f__wf` not weaker than
  today's on any contract (a false assumed fact over the ghost functions and the vacuous `prior` it gives, a
  read framed too early, a row of today's `wf_method` that a quantified clause escapes); the reading of `f` by
  position; is any inlined obligation lost.
- **Round 2, Parts A and B:** a `forall` accepted outside item 3; a body construct read but not checked (a side
  effect, a call, a global); an argument read twice or in the wrong state; `old` outside item 8; an obligation
  under the binder dropped or asserted without its path; the `precondition` row against the callee's clause;
  a trusted (bodiless) quantified contract that lets `assert false` verify after a call; Part A's neutrality;
  every other contract's `f__wf` unchanged; a lint rule a wrong shape can satisfy.
- **Round 3, Part C:** each obligation clause against the scalar clause it mirrors; a flag assumed at the loop
  head but asserted nowhere; a framing test that an element a callee kept can pass; a loop that should carry
  its places and does not; the check rows' order and single reason.
- **Round 4, Part D (with the pages and the port):** `old` in an invariant reading a state other than entry; a
  place the function reassigns or that no `requires` atom frames; a marked parameter; the pages' statements.
  When cut 1 is taken, round 4 is the pages and the port alone.
- **One re-audit** of blocking fixes. Should-fix items not taken go to handoff §3.

**Pre-agreed cuts, in this order** (each leaves its pin, re-pointed; the tutorial and the pages say what
landed): (1) Part D, `cerastes_old` in loop invariants (M4.5e2); (2) `forall` in a `check` (M4.8); (3) the
namespace spelling with a function (M7); (4) an `E` with more than one read class (M4.5e2); (5) `old` under the
binder, only by P80 (d)'s table (the port then keeps its single-element facts). Parts A–C for one read class, the
fill loop, max, the port's `forall` contracts and the tutorial chapter are not cut without asking.

**Split points.** At Commit 2 and Commit 3 the orchestrator reports `git diff --numstat <base> -- src/` without
the self-tests against section 1's sizes. **Stop and ask** if Parts A and B passed 1,600 lines, if Parts A–C
passed 2,200, or if, at Commit 3, the orchestrator and the advisor judge that its context will not carry Part D
with its audit and the docs (then cut 1 is taken and step 7 follows).

## 8. Tests (`tests/memory/forall_*.c`, negatives in `tests/memory/negative/`; `.c` in `c23, c++20`; each positive has a twin failing for one reason at one line)

- **Part B.** A callee whose `requires` says every element is non-negative and that reads one; its caller on a
  local array (twin: a negative element, `precondition`). A straight-line function that proves a quantified
  `ensures` over three elements; a caller that uses it (twin: a wrong `ensures`, `postcondition`). Guards
  (`n == 0 ||`, `c ? … : …`). `set_one` with `cerastes_old` under the binder (twin: a second write). A trusted
  declaration with a quantified contract. Obligations at the site: a read one past the range (`permission`), an
  overflow in `f` (`overflow`), a later clause resting on an earlier quantified one (verifies). Neighbours and
  two arrays in a contract (unless cut 4). `cerastes::forall(f, …)` in a `.cpp` (unless cut 3). One pin per
  rejection of items 3 and 4 and of section 1's table; both new tags.
- **Part C.** `forall_fill_local.c` (the sibling of `array_loop_fill_then_read_false_alarm.c`; twins: the
  invariant one too far, the write skipped); `forall_fill_range.c`; `forall_must_init_filler.c` (the sibling of
  `range_marker_pin_loop_filler.c`; twin: it fills `n - 1`); `forall_self_write.c` (the sibling of
  `array_loop_self_write_false_alarm.c`); `forall_max.c` and `forall_max_index.c` (twins: an off-by-one range,
  `<` for `<=`); `forall_search.c` (`r == -1` implies no element equals `x`); a sortedness check and a copy
  (unless cut 4); an unframed read and an index outside a named array in an invariant; `check(forall …)` on a
  local array and on both kinds of range with one twin per row of S45 (f) (unless cut 2).
- **Part D.** `inc_all` with its two invariants (twin: one element off); `*p == cerastes_old(*p) + i`;
  `cerastes_old(n)`; the rejections of item 12.
- **The port** `tests/memory/verifast/carrays.c` (its `expect:` lines and line map unchanged, its adaptation
  header rewritten): `array_test1` gains "every element keeps its entry value"; `array_test2` `a[0] == 10` and
  "every other element keeps its entry value" (VeriFast's `update(0, 10, vs)`); `to_verify` "`arr[i] == 10 * i`
  for every `i`" before and, after, the same for `i != 3` with `arr[3] == 31`.
- **VIR self-test:** the session's cases in `selftest_cases31.inc`; `VIR_CASES_MIN` raised.
- **Byte identity:** `scripts/vprdiff.py` against the baseline after each part: no existing program's `.vpr` or
  `.map` changes in this session.

## 9. Flips (pre-approved with this plan; any other changed outcome stops the session)

- **A. Retired for a positive of the same program, with a twin:** only the pins R2 adds in Step 0 for
  `cerastes_old` in a loop invariant (Part D; kept if cut 1 is taken).
- **B. Stay as written; only the message or the `Pins:` comment changes:** `reject_forall_pointer.cpp` (the
  lambda form: M7; its message now names `cerastes_forall`); `array_loop_fill_then_read_false_alarm.c`,
  `array_loop_self_write_false_alarm.c` and `range_marker_pin_loop_filler.c` (each names its verifying sibling;
  the program, which writes no quantified invariant, fails as before); `array_loop_bool_post.c` (exact counting
  is e1d's).
- **C. The port:** `carrays.c`'s contracts are strengthened; its expectations stay.
- **D. Lint self-tests** that pin "a `forall` here is refused" for a shape item 11 now admits: the test-mutator
  reports which before editing, and status lists them.
- **E. Front-end tests, goldens and existing Viper:** none (the C++ spelling of `old` in a `pure` body keeps its
  error; the header's new declaration changes no golden: **if one changes, stop and ask**).

## 10. Docs (invariant 8; every example is a test)

- **`annotations.adoc`:** the spec-function table (858–860: `forall`'s C spelling and meaning); 883–885; the
  `=== forall` section (892–950) rewritten around the function form (what `f` must be, the arguments, where it
  may stand, what `cerastes verify` checks), the lambda form kept as the front end's (M7); "Predicates, pure
  functions" (759: a `pure` body read by `forall`); `old` (1204–1210: the C spelling under a binder; Part D);
  the reserved names (1306).
- **`memory.adoc`:** "Arrays" gains "Facts about every element" (the fill loop, max, the flags an invariant
  states, `old` under a binder, in-place loops if Part D landed); 636–638 and 659–665 rewritten; the not-yet
  lists (627, 1715).
- **`error-messages.adoc`:** the two tags; the site-and-note form of an obligation inside `f`; the new
  `permission`, `uninitialized` and `out-of-bounds` messages. **`verifying.adoc`** 1212–1261;
  **`overview.adoc`** 93–96; **`tutorial/heap.adoc`** 258–259.
- **The tutorial chapter** `pages/tutorial/arrays.adoc`, "Arrays and quantifiers" (0049 item 4), after "The
  heap": a local array and a loop; a function over a range; a fact about every element (the fill loop and its
  invariant, with the off-by-one variant); max; an in-place loop (Part D). Examples
  `examples/tutorial/arrays_*.c`, each error variant "Derived from"; `console` blocks copied from runs;
  `nav.adoc`; `tutorial/index.adoc` (71–92).
- **`include/cerastes.h`**'s comments; **`tests/README.md`**; **`internal/encoding.md`** §2, §3 "Quantifiers
  (M4.5e1c)" and §8; the developer guide where it names the quantified shapes and `f__wf`; one amendment line in
  each amended record.

## 11. Stop and ask; acceptance

**Stop and ask:** any agent question; any changed outcome outside section 9; any existing program's Viper or map
changing; a P80 or S45 outcome outside section 5's tables; a cut beyond section 7's; a split point; an exit 3 or
a time over 120 s not fixed within its part.

**Acceptance.**
- Before the box run, on the final build: `scripts/regen_vpr.py`, `scripts/lint_viper.py` over its output and
  over a fresh VIR self-test directory, the session's VIR cases through Silicon, `scripts/rejections.py` against
  section 9, `scripts/vprdiff.py build/m4.5e1c/baseline/vpr <regen>` with nothing differing. Advisor
  checkpoint 5.
- `CERASTES_LONG=1 scripts/check.sh` **once**, after the last code change, on the x86-64 box (the branch as a
  git bundle into `~/cerastes-s8/p-ubuntu-26.04`, `~/cerastes-s8/run.sh update` then `check`): green.
- `test_lint_viper.py`, `test_run_tests.py` (the tag-coverage test names both new tags), `run_tests.py
  --check-headers`, `scripts/format.sh --check`, `scripts/build_docs.sh`.
- The status entry (sizes measured, deviations, corrections under question 160, each cut taken) and the handoff
  with its Questions section, S45 (r)'s result under "M4.5e1f" and e1d's as-built inputs. Commits stay local.

## 12. Inputs (moved here from `internal/handoff.md` §3 and `.claude/plans/m4-5e1-arrays.md` at this planning)

- **From e1a:** the fill loop over an array declared without an initializer and the loop that writes an array
  (the two pins); a write in a loop condition (`while (a[0]++ < 1)`) loses the values the same way.
- **From e1b:** the `must_init` loop-filler pin (239); the full contracts of `carrays.c`; the search loop's
  progress clause, written today as an invariant over single reads.
- **From M4.5f1's notes** (handoff §3): R1–R13 and the static-assertion cross-check do not reach `pure` bodies
  (item 4 says what reaches `E`); `pure` bodies reject globals and `static` locals (0041 item 12).
- **For e1d** (this session records it): the `f__wf` variant built and S45 (d)'s row (7), a function
  application in `f__wf` under each variant (the umbrella plan's "the choice binds e1d"); the reader, which e1d
  reuses; `lint_viper.py`'s `HEAP_READ_RE` (a marked row's bounds and amount pass as heap-free when they
  hold no `.`; a heap-reading function application would pass: e1d tightens it).
- **Costs:** S44 (g): the fill and search loops 1.9 s each; (k): two ranges, two loops, three calls 4.5 s; the
  scale program at K = 16 with a range call 11.8 s.
- **Lessons that bind here:** an allow-list is never computed by `grep` before a build (this session expects no
  Viper change at all); the audit worktree needs no bootstrap (a source-only `git worktree add --detach` plus
  copies of the three binaries built from exactly that source); a row reported "at the loop" uses
  `Stmt::keyword_loc`; proofs made while another agent builds are provisional; a twin that runs further than its
  positive needs an `ensures` that holds at the wider state; a conjunction in one invariant clause is two errors
  (N2); after a failed check Silicon goes on with the failing value unconstrained.

## 13. What e1c leaves, and the questions it opens

- **For handoff §3 at the session's end:** M4.5e1f takes S45 (r)'s result (238, 242); e1d takes section 12's
  inputs; M4.5e2 takes the index forms, the base forms, memory-reading arguments, `old` in a `check`, and cuts 1
  and 4 if taken; M4.8 takes nesting, `exists`, the other positions, and cut 2 if taken; M7 takes the lambda
  form, the C++ spelling of `old` in a `pure` body, and cut 3 if taken.
- **Open questions for the owner** (each built as its recommendation unless answered; the next handoff's
  questions start after them):
  - **247** An obligation inside `f` (an overflow, an unowned read) is reported at the `forall` site, with a note
    at the operation in `f`. **Recommended:** the site (one `f` serves many sites, and a contract's errors are
    reported at the contract).
  - **248** A trailing argument reads no memory; the read is written inside `f`. **Recommended:** accept as
    e1c's rule (M4.5e2 lifts it with the other argument forms).
  - **249** Only positive positions: a nested `forall`, `!forall` and `exists` are rejected until M4.8 reviews
    them. **Recommended:** accept (neighbour sortedness, max and the fill loop need none).
  - **250** The three false-alarm pins keep their names and expectations; their comments name their verifying
    siblings. **Recommended:** accept (a rename would move twin references for no change of outcome).
  - **251** Under a binder `old` is written `cerastes_old` in both modes; `cerastes::old` in a `pure` body stays
    the front end's error until M7. **Recommended:** accept (no front-end expectation changes).
  - **252** A contract on a `pure` function is neither read nor rejected today; e1c rejects it only where a
    `forall` names the function. **Recommended:** accept; e1d gives such contracts their meaning.
