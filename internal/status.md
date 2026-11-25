# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, M4.5a's
`git show 72e30b3:internal/status.md`, S5's `git show 794e960:internal/status.md`, M4.5b1a's
`git show bdc6bd0:internal/status.md`, M4.5b1b's `git show 8003dca:internal/status.md`, M4.5b1c1's
`git show eab4058:internal/status.md`, M4.5b1c2's `git show a5e3e30:internal/status.md`, M4.5b1c3's
`git show 5ffb1a2:internal/status.md`, M4.5b2a's `git show 38877b4:internal/status.md`, M4.5b2b's
`git show 1bfa705:internal/status.md`, M4.5c1's `git show cbdcce5:internal/status.md`, and M4.5c2a's
`git show 7f11706:internal/status.md`, M4.5c2b's `git show d26a87c:internal/status.md`, and M4.5d1's
`git show 9d98fe8:internal/status.md`, M4.5d2a's `git show bb41d55:internal/status.md`, S6's `git show 9255b93:internal/status.md`, M4.5d2v's `git show c2bc48d:internal/status.md`, S7's `git show a660abc:internal/status.md`, M4.5d2b1's
`git show c1b691d:internal/status.md`, S8's `git show 15bcdc6:internal/status.md`, M4.5d2b2a's
`git show 19d977a:internal/status.md`, M4.5d2b2b's `git show e3f4b9f:internal/status.md`, M4.5d2b2c's
`git show 023ea50:internal/status.md`, M4.5d2b2d's `git show d88a457:internal/status.md`, M4.5e1a's `git show 685034b:internal/status.md`, M4.5e1b's `git show 7964d33:internal/status.md`, and M4.5e1c's `git show c65f618:internal/status.md`.

## M4.5e1d — Pure functions and the exact sum (2026-11-24/25): done; on `main` and pushed (`68384d1`)

Plan: `.claude/plans/m4-5e1d-pure-functions.md` (approved 2026-11-24; executed without re-planning). Record 0062
(accepted; amends 0006, 0010, 0011, 0012, 0013, 0024, 0025, 0037, 0041, 0042, 0044, 0061). Read `internal/handoff.md`
next: §1, §2, §3 "From M4.5e1d", §8. The owner answered questions 287–289 on 2026-11-25, each "Follow
recommendation" (287: M4.5e2 fixes R4-S5). Branched from local `main` = `c65f618` (the planning
commit). Session from 2026-11-24 about 16:00 UTC to 2026-11-25 about 02:10 UTC; Commit 4 is `2fc6a21`.

**Owner decisions in the session** (record 0062's header has each with its reason): 273 (H', the lint's name rule), 274
(digits-only ids), 275 (Silicon's axiomatisation warning accepted for `pf_` functions), 276 (keep fact (iii)), 277
(multi-error twins list every error), 278 (a `check`'s pure lines first), 279 (two unsafe facts as listed limits,
M4.8), 280 (two rejection pins moved to a `check`), 281 (a read pure function counts as one function verified), 282
(the definition-unknown false alarm pinned), 283 (pure lines as rows in loop invariants), 284 (an application in a
`do … while (0)` invariant takes the `check` form), **285** (two tests over 120 s on the box moved to `tests/long/`;
the recommendation was two-element programs), 286 (Silicon's prover count capped at 16, fixed in this session).
Mid-session the owner moved **all testing to the x86-64 box** ("in the future do all testing on the ssh box rather
than doing some there and some on my local machine, unless there's a huge performance difference"), allowed several
agents on it at once, and asked for fewer intermediate checks.

**Done** (edg-backend, il-analyst-1/2/3 and test-mutator-1/2/3, Sonnet, each successor started from its
predecessor's state file; the soundness-auditor, Opus, kept for all rounds; the advisor at the plan's checkpoints):
- **Part A** (`ed16ecc`): VIR and lowered-program plumbing, behaviour-neutral; the driver's one accepted Silicon warning.
- **Record, P81, S46, Commit 1** (`92294b7`, revised by audit round 1 in `9ca3a6c`): P81 (a)–(f) and S46 (a)–(k) in
  record 0062's facts table (92 `s46_*` fixtures); the lint (H', `PF_NAME`, the `hva_` facts, the guard quantifier
  checks, prelude acyclicity), the harness.
- **Part B** (`e3ed705`): read pure functions as Viper functions (`pf_<name>__<id>`, form Min, the result tag), their
  `__wf` definition checks, self-recursion with `decreases`, applications in checks and in pure bodies.
- **Part C** (`10d23cd`): applications in an ordinary function's `requires` and `ensures`, at calls and returns.
- **Part D** (Commit 4): applications in loop invariants (item 7's framing at entry and back edge, the obligation rows,
  question 283's pure-line rows, the guarded value), question 284's `do … while (0)` form, **the exact sum
  (`tests/memory/pure_sum.c`) verifies in both modes**; the application messages in the interval form `a[0..n)`;
  Silicon's prover cap (question 286); the pages (annotations, verifying, error messages, memory, the tutorial's
  "The exact sum" in "Arrays and quantifiers"), `internal/encoding.md` §2–§8, the developer guide.
- **Audits** (`build/m4.5e1d/auditor-report-{1,2,3,4,4b}.md`): round 1 on the record and S46 (B1 the lint admitted
  fact (ii) without the callee's quantified clause, B2 an application under `cerastes::old` compiles in C++; S1–S6,
  N1, N2, N9; all taken); round 2 on A+B and round 3 on C (no blocking item; their should-fix items taken and
  pinned); round 4 on Part D and the pages, probes on the box, also the plan's re-audit (no blocking item; R4-S1–S4
  taken, R4-S5 a listed limit); 4b, a short read of the post-audit delta (the spelling, the cap, one self-test index).
- **Cuts:** none taken (cuts 1–5 all stayed in). Split points: A–C at +3,351/−193 against 4,200.

**Deviations from the plan, for confirmation:**
- Testing moved to the box mid-session (owner). Box lanes, each a copy of the `ubuntu-26.04` checkout with its own
  container from a committed image (`cerastes-e1d:26.04`): `~/cerastes-e1d/{base,work,audit}`; the neutrality
  baseline re-made on the box (6,217 programs; Mac and box differ only by glibc parameter names in 32 `.vpr` and
  absolute paths in maps, platform not encoding). The Mac ran one cross-check: a self-test case's Silicon verdict.
- Snapshots for audits and box runs as commit objects through a temporary index (`refs/tmp/e1d-*`), not commits.
- `arrays_sum_offbyone.c` (the tutorial's failing example) bounds `n < INT_MAX` in its `requires` so that its errors
  are only the four the text explains: an orchestrator choice, for review.
- Question 285's move left `pure_sum_caller.c` without a default-tier error twin (question 207's rule), so
  `tests/memory/negative/pure_sum_caller_write_between_short.c` (the same program over 2 elements) was added.
- The application messages name the read interval as `a[0..n)` (and a decayed local array by its name), e1b's call
  and return messages unchanged.

**Corrections of new tests after their first run** (question 160), and before it: `pure_check_parameter_unframed.c`
(three errors at L22: the pure line first, the framing, the argument's overflow); `pure_reads_cell.c` and
`pure_reads_cell_unwritten.c` (EDG's #550 in C++: `q` used); `pure_calls_pure_in_requires.c` and its `_outside` twin
(item 8's listed limit: the positive gets an `ensures`, L22 → L23); before any run, from the frozen binary or an owner
answer: `reject_pure_contract_cycle.c` (a prototype, L10 → L12), `reject_pure_body_assign.c` (front-end warning, L12
→ L13), `reject_pure_void_result.c`, `pure_keep_fact_owns_alias.c`, `reject_pure_call_under_old.cpp`
(`unsupported_annotation_expression`), `pure_check_local_count_past.c` (`precondition`, question 278),
`pure_loop_application_unframed.c` (three errors, question 283), `pure_forall_and_check_unframed.c` (replacing a
memory-free forall that 0061 rejects); the self-test case `ctd_forall_sum_noclause` (its row index named L115's entry
row when the preserved row fails: i is 0 at entry; Silicon says "not preserved" on the box and the Mac).

**Not done / limits** (handoff §3 "From M4.5e1d"): record 0062 item 8's listed limits and question 279's two facts
(M4.8); question 267's range fact (M4.8); R4-S5 (a `must_init` filler whose invariant applies a function over the
range still being filled: exit 3 on the box, fail closed; question 287); a marked range frames no application (item
7); the false facts over 3–4 element sums cost 226–356 s of Z3 on the box (now in `tests/long/`); everything item 1
and question 266 leave to M4.5e2, M4.5f1, M4.5f2 and M6.

**Checks:**
- The Mac, before the move (test-mutator-3 report 21, on the build of 2026-11-24 22:34 UTC): `run_tests.py` 15,982
  passed, 0 failed, 10 skipped; the self-tests (`test_run_tests.py` 1,282, `test_lint_viper.py` 939, `test_mutate.py`
  299, `test_vlibc_contracts.py` 141, `test_walker_oracle.py` 79); the vlibc checks; `mutate.py` on
  `static_assert_pure_body.c` (no unexplained survivor); `build_docs.sh`, `check_doc_examples.py` (source and `--html`).
- The box, on snapshot E (`324e711`; Commit 4 differs from it by one self-test index, a `silicon.h` comment, a
  `test_run_tests.py` assertion, the long tests' `timeout:` and record text): regeneration 6,403 of 6,403; the
  6,217 baseline programs byte-identical; no `.vpr` changed by the post-audit fixes (40 maps by the spelling only);
  VIR self-test 2,318 lowering and 375 rejection cases, 0 problems (`VIR_CASES_MIN` 2318), through Silicon 2,317 of
  2,318 before the index correction; lint clean (6,403 and 2,318 files); `--selftest` 195; format 90 files;
  `rejections.py` unchanged; test-mutator-3 report 23: the session's default-tier tests 518 of 518 (native-ok 72 and
  72), `pure_sum.c` verified, the four doc examples, the two long tests 4 of 4 (210–267 s each with the cap on a
  quieter box, up to about 14 min under load; their `timeout:` raised from 900 to 1800 before the box check),
  the short twin (13–22 s), `test_lint_viper.py` 939, `--check-headers` 5,946, `build_docs.sh`, `check_doc_examples.py`
  (source and `--html`); `test_run_tests.py` 1,287 with one failure, the harness test that pinned one
  `os.cpu_count()` (the cap adds a second; the test now pins both, its two classes rerun, 15 OK): the full pass is
  the box check's. The Mac compiles Commit 4 (`--selftest` 195). The auditor's unbounded d12q run had no verdict after 75 min (never "verified") and was stopped.
- `src/` and `include/` against `c65f618` (measured): +3,654/−195 without the self-test cases, +5,721/−195 with them.
- The box: `CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` **all green** on `2fc6a21` in the `ubuntu-26.04`
  container (`~/cerastes-s8/p-ubuntu-26.04`, an incremental build over e1c's tree), 50 min (e1c's took 89: the
  prover cap), exit 0, no rerun: 16,000 tests passed, 0 failed, 10 skipped (native-ok clang and GCC 2,988 each), the
  long tier ran (the two new long tests in it); lint 6,403 files clean; VIR self-test 2,318 lowering and 375
  rejection cases, 0 problems, every case through Silicon (2,318 passed); `test_run_tests.py` 1,287 OK and the other
  self-tests; format 90 files; `--check-headers` 5,946; the doc examples (407 blocks) and the docs (27 pages) clean.
  No older test timed out under the cap. The closing commit changes only this file, the handoff and `plan.md`,
  which `check.sh` does not read. Log `~/cerastes-s8/p-ubuntu-26.04/e1d-check.log`.

**After the session** (2026-11-25): the branch went to `main` by fast-forward and was pushed (`main` =
`origin/main` = `68384d1`). M4.5e1e's planning followed, at a session that only planned: the owner cut M4.5e1e in
two (question 290: M4.5e1e1, arrays of scalars in structures and static storage; M4.5e1e2, arrays of structures
and arrays of arrays), put whole-structure copies and by-value passing of a structure with an array member in
M4.5e1e (question 291), to run in e1e1 unless that would pass twelve hours (question 300), asked that questions
go to the Fable advisor first, and asked for less testing (a test budget per plan).

**First step for the next session:** the owner reviews handoff §8's questions 292–299 (defaults, each fail
closed) and the Flips of the plan's section 10, pushes `main`, and removes e1d's worktrees `.claude/worktrees/m4.5e1d` and
`.claude/worktrees/m4.5e1d-audit` and its branch (handoff §2). Then **M4.5e1e1** from
`.claude/plans/m4-5e1e1-arrays-of-scalars.md` with the opening prompt of handoff §2 (no plan mode), or M4.5e1f
(short) or M4.5e2 first.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| M4.5e1d Pure functions and the exact sum | 2026-11-24/25 | `m4.5e1d-pure-functions`, `68384d1` (fast-forward) | 0062 (0006, 0010, 0011, 0012, 0013, 0024, 0025, 0037, 0041, 0042, 0044, 0061 amended) | a `pure` function that is called or has a contract as a Viper function with its definition check; self-recursion with `decreases`; applications in checks, contracts, calls and loop invariants; the exact sum verifies; testing moved to the x86-64 box; Silicon's prover cap; questions 273–286, no cut |
| M4.5e1c Quantifiers | 2026-11-23/24 | `m4.5e1c-quantifiers` | 0061 (0006, 0011, 0028, 0029, 0037, 0038, 0041, 0059, 0060 amended) | `cerastes_forall` with an inlined `pure` body in contracts, loop invariants and checks; `f__wf` variant M; the fill loop, max, the carrays port, the tutorial chapter; S45 (r) for e1f; Part D cut to M4.5e2 (question 260); questions 253–263 (256–263 built overnight, for review) |
| M4.5e1b Ranges in contracts | 2026-11-11/12 | `m4.5e1b-ranges-in-contracts` | 0060 (0011, 0028, 0029, 0037, 0038, 0039, 0040, 0042, 0059 amended) | `owns_range`/`reads_range` in contracts, element reads in contracts, calls and returns over ranges, loops over parameters' ranges, markers over ranges, `owning(n)` on a parameter, `restrict` with a range, array parameters (two new tags), the carrays port; questions 238–246 |
| M4.5e1a Arrays inside one function | 2026-11-11 | `m4.5e1a-arrays-in-functions` | 0059 (0011, 0012, 0024, 0028, 0032, 0038, 0039, 0042, 0043, 0054 amended) | local arrays as blocks, `a[i]`, `&a[i]`, `p[i]`, the decay, initializers with C's zero fill, `out-of-bounds`, the marked range rows in the driver, loops carrying arrays, sequencing, `sizeof(a)`, `uninit` on arrays; #175, #1162, #138 as errors; array designators rejected (234); the `offset_of == 0` backstops (235) |
| M4.5d2b2d `not_invalidating`, `--strict-owner`, marked fields | 2026-11-10 | `m4.5d2b2d-invalidation-profile` | 0058 (0029, 0042, 0052, 0054, 0055 amended) | `cerastes::not_invalidating` checked at the definition (taken implies returned; `p->q == old(p->q)` spliced, a bodiless borrow's tie implied); `--strict-owner` (P4417R0's p4 and p6); `ref_to_uninit` on fields (marked closures keep flags only where tied) and on pointers to structures with owning fields (closing nothing); N1; two older exit 3s fixed (0054's kept row, the return rows' order), a third to M4.8 |
| M4.5d2b2c Loop limits, long-running tests | 2026-11-10 | `m4.5d2b2c-loop-limits` | 0056, 0057 (0016 amended) | six of d2b2b's loop false alarms lifted (a site the loop cannot fill keeps its flags, the value set of a lone owner, a leaf's ghost through an alias, `main`'s global leaf under an entry snapshot, a site's cells as places, chains at any depth), the escape scan, two exit 3s fixed; five pins kept under the written-invariant rule; the long-running tier (`tests/long/`, `CERASTES_LONG=1`); question 196's two programs past 1800 s, to M4.8 |
| M4.5d2b2b Owning fields | 2026-11-09/10 | `m4.5d2b2b-owning-fields` | 0055 | `owning` on members: the closure of every atom over owning fields (acyclic, any depth), the value rule at stores, named leaves (struct locals, globals, `static` locals, by-value copies) as owners in memory, copies, loops; P4417R0's `[[owner]]` mapped; the session tools `regen_vpr.py` and `vprdiff.py`; seven loop limits pinned for d2b2c |
| M4.5d2b2a Owners in memory | 2026-11-09 | `m4.5d2b2a-owners-in-memory` | 0054 | block sites through loads and on globals; owning and marked globals, `static` locals and address-taken locals (the deep atom; static owners exempt at `main`'s end); written `owns_block` invariants; loop carriers by current state lifting d2b1's four false alarms; the static struct leaf cover; d2b2 split (owning fields and P4418R0 to d2b2b) |
| S8 CI toolchain profiles | 2026-11-06 | `s8b-ci-profiles`, PR #33 | 0053 (0001, 0016, 0031 amended) | CI asserts declared majors and records exact builds; toolchain profiles (`scripts/ci_toolchains.toml`), a matrix with a `profiles` job, cadences `every`/`nightly`/`dispatch`, exclusions; the guard as rules and a schema; GCC builds of cerastes and EDG; a builder evaluation-order bug found by GCC fixed |
| M4.5d2b1 Partnerless results, owning locals | 2026-11-05 | `m4.5d2b1-results-locals` | 0052 | `ensures` atoms with no partner (covered by implicit atoms or given an output amount; bodiless contracts fresh results only), owning automatic locals (leaks at every exit and reassignment, loops), marked locals and P4222R3's strict rules, block-site guards over memory, `cerastes_owns_vlibc_block`, two d2a holes closed; written `owns_block` invariants cut to d2b2 |
| S7 Code style | 2026-11-05 | `s7-code-style` | 0051 (0001 amended) | cerastes's tool code in the owner's Boost style: the pinned clang-format 23.1.3 (`format.sh`, a `check.sh` step, fail-closed guards), then brace initialization, `U` and one declarator per declaration; proved by `object_identity.py` (objects and IR at every commit, macOS and Linux; tokens) and by `.vpr`/`.map` identical over the suite and the VIR self-test; the docs lockfile fixed |
| M4.5d2v vlibc's allocator | 2026-11-04/05 | `m4.5d2v-allocator` | 0050 (0047 amended) | the M4.5d2a re-audit's B1–S3; vlibc's allocator (`vlibc_malloc` and six more, a C23 core over trusted glue, sanitized and threaded tests); blocks know their allocator (the token's value); the pairing check; contract-less library routines; vlibc allocations rejected in C++; two older exit-3 holes fixed; vlibc and vlibm in the owner's Boost style |
| S6 Tutorial and developer guide | 2026-11-04 | `s6-tutorial` | 0049 | a VerCors-style tutorial (seven chapters, 20 verified programs and 24 failing variants, both modes) and a developer guide (eight pages) as sections of the one site; the overview rewritten; the website's landing page; a chapter per concept from now on (questions 131–133) |
| M4.5d2a The heap | 2026-11-03/04 | `m4.5d2a-heap` | 0047 | vlibc's `malloc`, `calloc`, `free` (trusted, reserved identifiers, shapes not names); typed views at `void*` → `T*` (one object, `sizeof(T)`); heap blocks with the free token; `invalid-free`; leaks by `forperm` (after `l_end`, at back edges; `main` included); `owns_block`; `owning`/`ref_to_uninit` on functions; sequencing over block sites; loops over blocks held through locals; d2 split (d2v allocator, d2b) |
| M4.5d1 vlibc in cerastes | 2026-11-03 | `m4.5d1-vlibc` | 0046 | `third_party/vlibc` pinned and guarded; vlibc's shim first in verify mode; library files by path; trust from vlibc's tags, "does not return" from vlibc, reserved names, attributes by position; three routes redirecting trusted calls closed (older than vlibc); `trusted_libc_reason` gone; d2's dependency map |
| M4.5c2b Places through a heap read | 2026-11-03 | `m4.5c2b-heap-read-places`, PR #26 | 0045 | contract and invariant places through loads (`**pp`, `*gp`, `p->q->x`, `*s.q`, `*r.q`) with the entry-state rule, the post-state reading, amounts that follow objects, bodiless ties, heap-reading guards, `eb`/`lb` loop places, written atoms through loads and on struct places, `restrict` over load roots; `live(r.q)` to M4.5d2 |
| M4.5c2a Struct values and struct storage | 2026-11-02/03 | `m4.5c2a-struct-values`, PR #25 | 0044 | by-value struct parameters and results as tuples of leaves, `f().x`, C++ temporaries, compound literals (C objects, C++ values), struct globals and `static` struct locals, member measures, `restrict` beside by-value structs; invariant reads through a loaded pointer pinned; M4.5c2 split, heap-read places and written struct atoms to M4.5c2b |
| M4.5c1 Struct objects | 2026-11-02 | `m4.5c1-struct-objects`, PR #24 `8f45843` | 0043 | C structures with member blocks, struct locals as blocks, `.`/`->`, pointers to members, initializers, copies, `struct S *` objects with `WF_S`, contract atoms on struct places with overlap per leaf cell, markers per leaf; `restrict` closed beside loaded pointers (a hole since 0037); M4.5c split, owning fields and P4418R0 to M4.5d2 |
| M4.5b2b Initialization through pointers | 2026-11-02 | `m4.5b2b-init-markers` | 0042 | P4222's markers (`uninit` on locals, `ref_to_uninit`/`must_init` on pointer parameters), every call flag row `uninitialized`, `--strict-init` with the `// verify-options:` header, the loop row on every loop place (question 89, fixing 0038's nested-loop exit 3); M4.5b2 complete |
| M4.5b2a Globals and `static` locals | 2026-10-30 | `m4.5b2a-globals`, PR #22 `38877b4` | 0041 | mutable globals and `static` locals as `unique` blocks, `program_start` for `main` with initial values by linkage, frames through `owns(g)`/`reads(g)`, implicit atoms for `static` locals; M4.5b2 split, the markers to M4.5b2b |
| M4.5b1c3 Pointers to pointers, owning, `*result`, written atoms | 2026-10-30 | `m4.5b1c3-pointers-to-pointers` | 0040 | `T**` at any depth and pointer cells, `[[cerastes::owning]]` on parameters, `*result` as a value, written `owns`/`reads`/`live` in loop invariants (question 68's false alarms gone); conversions and contract comparisons across `const` rejected (a hole since 0037 closed); M4.5b1 complete |
| M4.5b1c2 Sequencing and placement | 2026-10-29 | `m4.5b1c2-sequencing` | 0039 | writes to cells inside expressions, sequencing on objects (`unsequenced`; same-object shapes rejected like the compilers), calls beside accesses, `return &local;` rejected (R14), `c ? owns : reads` path-precise and partners by path, function measures over memory |
| M4.5b1c1 Loops and jumps | 2026-10-29 | `m4.5b1c1-loops` | 0038 | loops over memory with automatic permission invariants ("every place held"), invariants and loop measures over memory, scope exits at jumps; written atoms in invariants cut to b1c3, function measures over memory to b1c2; the `address_of_local.c` port |
| M4.5b1b Pointer parameters and contracts | 2026-10-28/29 | `m4.5b1b-pointer-params` | 0037 | pointer parameters and results, `&param`; `owns`/`reads`/`live`/`old` in both spellings with ghost amounts; framing per path; `permission` at calls and returns; `pointer-comparison`; `restrict` on owned C parameters |
| M4.5b1a The model, address-taken locals | 2026-10-28 | `m4.5b1a-pointers` | 0032–0036 | the `Block` domain and its four axioms; address-taken locals as blocks, `&`, `*`, NULL, `==`, liveness and scope exits inside one function; `null-dereference`, `permission`, `lifetime`; C's incompatible pointer types as errors |
| S5 Linux CI | 2026-10-28 | `s5-linux-ci` | 0031 | `check-linux` on `ubuntu-26.04` in both tiers; Linux bootstrap; glibc `assert`; the same EDG language on both platforms; eight macOS-only tests tagged |
| M4.5a Memory-model design, `\|`/`^` masks | 2026-10-27 | `m4.5a-memory-design` | 0027–0030 | the C memory model designed and audited (E2, four axioms, member blocks, liveness), ownership annotations, vlibc; `\|`/`^` constant masks decided; S5 Linux CI next |
| M4.4d Exact sets, constant masks | 2026-10-27 | `m4.4d-exact-sets` | 0026 | preconditions and comma values get exact sets, with today's as the fallback; one `requires` per argument accepted; constant masks as remainders, the `bit_mask.c [c23]` hang gone; hooks `force_base_sets`, `drop_exact_set` |
| M4.4c Writes outside the chain, `void` `?:`, `-Wall` shapes | 2026-10-26 | `m4.4c-writes-void-warnings` | 0025 | finding A fixed (a flagged read is an access); writes and snapshots leave the chain; a `void` `?:` statement is an `if`; warnings in `verified`/`error` tests; the static-assertion cross-check in bodies; 13 `-Wall` shape rules; M4.4e planned |
| M4.4b2 Values the front end computes | 2026-10-26 | `m4.4b2-front-end-values`, `8bff6ba` | 0024 | static assertions checked as written (file scope through source sequence lists, now on); wrapped character literals are `conversion` errors; types computed from a constant rejected in `sizeof`/`typeof`/typedefs until M4.5e |
| M4.4b1 Bitwise operators | 2026-10-23/24 | `m4.4b1-bitwise`, PR #9 `effc63f` | 0023 | `& \| ^ ~` and `&= \|= ^=` exact (prelude functions per width with proved range postconditions); seven bitwise `-Wall` shapes rejected; multi-character literals refused; item 5's truth values one verdict in both modes |
| M4.4a No wrapping | 2026-10-23 | `m4.4a-no-wrapping`, PR #8 `00f60d2` | 0022 | unsigned arithmetic and conversions checked (`overflow`, `conversion`, `shift`), `c_wrap` gone; lowering's internal errors exit 3; enumeration `==`/`!=` with possibly negative operands rejected |
| M4.3b Calls in expressions | 2026-10-23 | `m4.3b-calls`, PR #7 `58d208c` | 0021 | calls in guarded operands and loop conditions; the dependency-chain rule (`unsequenced_calls`); explicit termination checks; `self_assignment` |
| M4.3a Side effects in expressions | 2026-10-22 | `m4.3a-side-effects`, PR #6 `d694606` | 0020 | assignments, `++`/`--` and commas as values; C's sequencing rule (`unsequenced_modification`); the first `-Wall` shapes |
| M4.2c Locals without an initializer | 2026-10-22 | `m4.2c-uninitialized-locals`, PR #5 `1c1cc29` | 0019 | category `uninitialized` by a ghost flag; jumps past such declarations; EDG #2656 fixed on fork branch `0022` (pin `f0ed5525af`); `-Wswitch`'s rules |
| M4.2b2 Enumerations | 2026-10-22 | `m4.2b2-enumerations`, PR #4 `4622259` | 0018 | enumeration types with value ranges, every conversion into one checked |
| M4.2b1 Named constants and case values | 2026-10-21 | `m4.2b1-named-constants`, PR #3 `ae2d2a4` | 0017 | `const`/`constexpr` globals and case values checked in each method's prologue |
| S3 Mutation triage | 2026-10-21 | `mutation-triage`, PR #2 `915e372` | — | 17 tests strengthened, 33 twins, 436 reviewed equivalents |
| S2 CI | 2026-10-21 | `ci`, PR #1 `e1f2860` | 0016 | `check.sh` on GitHub's `xcode-27` runner, required on `main` |
| S1 EDG upstream merge | 2026-10-21 | `edg-upstream-merge` | — (0001) | fork `cerastes` merged upstream `main`; 0020's Linux failure was stale box binaries |
| M4.2a Jumps and ends | 2026-10-21 | `m4.2a-goto-noreturn-assert` | 0015 | outward-and-forward `goto`, `noreturn` (category), trusted `abort`/`exit`/`_Exit`, `assert` as `check` |
| M4.1 Control flow | 2026-10-20 | `m4.1-control-flow`, `29a3dd3` | 0014 | `break`, `continue`, `do`, `switch`; the plan re-sequenced (new M4) |
| M3 close-out | 2026-10-20 | `worktree-m3-closeout`, `bfeda03` | — | M3 acceptance, mutation run |
| M2 re-look and M3 Scalar encoder | 2026-10-16/19 | — | 0010–0013 | VIR, emitter, driver, scalar encoding, termination |
| EDG bugs 0003–0007 | 2026-10-15 | fork branches | — | fixed, branches for the owner's PRs |
| M2 Annotation layer | 2026-10-14/15 | — | 0005–0009 | `[[cerastes::…]]` registration in the EDG fork, `<cerastes.h>`, the fork pin |
| M1.5 Documentation site | 2026-10-14 | — | 0004 | Antora docs at cerastes.dev/cerastes/doc/ |
| M1 IL inventory · Gate G1 | 2026-10-13 | — | 0002, 0003 | G1 PASS; in-process back end |
| M0 Bootstrap | 2026-10-13 | — | 0001 | toolchain pins, harness, `check.sh` |
