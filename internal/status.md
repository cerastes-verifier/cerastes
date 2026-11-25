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
`git show 023ea50:internal/status.md`, M4.5d2b2d's `git show d88a457:internal/status.md`, M4.5e1a's `git show 685034b:internal/status.md`, M4.5e1b's `git show 7964d33:internal/status.md`, M4.5e1c's `git show c65f618:internal/status.md`, M4.5e1d's `git show 4e28487:internal/status.md`, and M4.5e1e1's `git show f07d80a:internal/status.md`.

## M4.5e1e2, first session — Arrays of structures in code (Parts A and B of six); `restrict` found unsound on `main`, narrowed, one form still open (2026-11-25): closed at split point 1 after Commit 2 `b9e1ab9` and a one-case fix `aebf158`; the full check: red at `b9e1ab9` on one new self-test case, **green at `aebf158`**

**Wrong programs verified on `main` before this session, and the fix is on this branch, not yet on `main`:** C's
`restrict` beside another route to the same object (`int f(int *restrict p)` with `owns(*p)`, `p == &g`, a write
through `p`, a write to `g`, a read through `p`) verified, though C makes it undefined and `gcc -O2` returns
another value. Audit round 1 found it (it is not from this session's work); two probe rounds and audit round 2
found more routes (through a callee that owns the global, a pointer a callee hands back, the global named inside
a callee's body, a range atom that may be empty). Record 0064 item 19 rejects every form found by then, fail closed. **One
more form was found by the last audit read and is NOT fixed: it still verifies on this branch** (two range
parameters with the same count, called with a count of 0, beside a global the contract holds and the body never
names: `owns_range(p, n) && owns_range(q, n) && owns(g) && p == &g && q == &g`; record 0064 item 19, "KNOWN, OPEN
AND UNSOUND"). **The rule is an enumeration, not a proof: three audit reads in one day each found a form the rule
then in force missed.** **The complete rule for `restrict` is the owner's** (handoff §8, question 326; the
recommendation is to reject `restrict` altogether until it is modelled).

Plan: `.claude/plans/m4-5e1e2-arrays-of-structures.md` (one plan for what it expected to be two sessions; executed
without re-planning, from its section 5). Record 0064 (accepted for the items built: 2–6 and 10–12 for local
arrays of structures in code, 16, 17's first rules, 19; the others "decided, built by the continuation"). Read
`internal/handoff.md` next: §1, §2, §8.
**Branched from local `main` = `f07d80a`** (at the session's start `origin/main` was `76fc7b1`, its ancestor, without
the plan; by the session's end `origin/main` is `f07d80a` too: the owner pushed `main` meanwhile, so the branch is a
fast-forward of both).
Session from 2026-11-25 13:57 UTC to about 23:50 UTC (9 h 50). The owner gave no overnight rule; every question went to
the Fable advisor first; four are open for the owner (320, 322's scalar forms, 325, 326).

**The session closed at split point 1** (question 310's 12-hour rule; advisor, checkpoint 3): at Commit 2, 7 h 48 in,
Part C (1,250–2,150 `src/` lines by the plan) would have needed about three hours of building,
an audit round and its fixes, and the two-hour close-out on a box loaded by other users: past fourteen hours.
**Parts C, D, E and F run in a continuation session from the same plan** (`M4.5e1e2, from Part C`).

**Done** (commits on `m4.5e1e2-arrays-of-structures`):
- **Commit 1** `571fe59` (15:13 UTC): record 0064's draft revised by P83 and S48's first pass and by the advisor's
  rulings 321–323; the chapter `## M4.5e1e2` of `internal/il-inventory.md`; 37 S48 fixtures; 12 staging pins from
  Step 0; `scripts/box_lane.sh`'s `setup` fix.
- **Commit 3** `aebf158` (22:47 UTC): the one VIR self-test case the first check run failed on (below).
- **Commit 2** `b9e1ab9` (21:45 UTC; Parts A and B, record item 19, the fixes of audit rounds 1 and 2; its tree
  for `src/`, `include/` and `scripts/` is the last joined proof's, `refs/tmp/e1e2-j2` = `6a8d73b`, and `tests/`
  is that proof's plus four new test files corrected and re-run afterwards):
  - **Part A:** leaf paths with index steps (`LeafStep`, `LeafPath`, `object_leaf_paths`); the six silent traps the
    planning found, each an internal error or the general path, each pinned; three backstop gaps in `check.cpp`
    closed. Behaviour-neutral.
  - **Part B:** automatic local arrays of structures: `v[i].x`, `v[i].t.x`, `p[i].x` on any pointer to a
    structure, `&v[i]`, `&v[i].x`, the decay, braced initializers with a nested list per element and C's zero
    fill (`alloc_arr_struct_<n>_<tag>`, `zero_arr_struct_<n>_<tag>`), elements as structure values (`t = v[i]`,
    `v[i] = v[j]`, `f(v[i])`), scope exits per leaf, loops with one place per leaf (reach by field), invariant
    reads of `v[e].x` for a named local array, sequencing keys with index steps, `sizeof(v)`.
  - **Question 304's rejection, as the compilers measure it** (record item 16): wider than planned (questions 321
    and 327).
  - **The `restrict` rule** (record item 19, question 326).
  - **S48 in full:** 79 fixtures (54 of the spike's two passes, 25 added after audit round 1), every decision
    table on its first branch; depth 3 admitted.
  - **The lint's first rules** (`alloc_arr_<E>`, `zero_arr_<E>`, leaf rows), 32 self-tests.
  - **The pages:** `memory.adoc` ("Arrays of structures"; the `restrict` paragraphs, whose old text was false),
    `verifying.adoc`, `overview.adoc`, `error-messages.adoc`, `tutorial/arrays.adoc`; `internal/encoding.md`, the
    developer guide.

**Not done** (the continuation's, from the same plan; nothing was cut): Part C (ranges over structures in
contracts, global and `static` arrays of structures, array members of structures), Part D (the six leftovers of a
structure with an array member: questions 292–294 and 306, partnerless and written atoms), Part E (arrays of
arrays, array-of-rows parameters, `v[i].a[j]`, depth 3), Part F (quantifiers and `pure` applications over members);
the tutorial section "Arrays in aggregates" (question 319: the session that lands Part C); the lint's rules for
Parts C–F. Each is still rejected with a diagnostic and a pin that names "M4.5e1e2 (continuation)".

**What the spikes and probes decided** (record 0064's table has every row): P83 (a)–(g) all "Expected"; S48 (c)
both forms passed the spike's pairings, so Form P, which audit round 1 then showed was written too narrowly (a
false alarm in a forwarding function; an alias through a nested leaf reaching Silicon's own check, exit 3): the
record's item 7 now writes it per pair of leaf paths with a slot row for two ranges, and eight fixtures added
after the audit pass it; S48 (h) heap and (i) markers (both protected) passed in the first form; (k) and (l) are
built, not left rejected; (m) admitted depth 3. No cut was taken.

**Audits** (Fable at `xhigh`, a fresh instance per round; reports `build/m4.5e1e2/auditor-report-{1,2}.md`):
- **Round 1** on Commit 1 (the record, the spike): blocking 1, the `restrict` hole on the base (above); blocking
  2, `restrict_backed`'s reachability walker missing from Part A's table (fixed on its own snapshot); should-fix
  3–4 the pairing rows, 5 item 16's C clause, 6 spike evidence (25 fixtures added), 7 the trusted methods' names,
  8 a scope-exit pin, 9 a second walker, 10 stale text: all taken.
- **Round 2** on the joined snapshot of Parts A and B (`83f447c`): **Parts A and B held** (about 190 probe
  functions and files, no unsound result; a verdict per row of Part A's table, all "holds"; item 16's two newly
  accepted forms zero every leaf; S48's later 42 fixtures and the lint's refusal of 24 contract mutants
  reproduced). **Blocking 1 and 2: two more `restrict` forms verified, both on the base** (the global named inside
  a callee's own body; an atom that is a range whose count can be 0): item 19's clause (iv) and pair rule, built
  and proved on their own snapshot (`refs/tmp/e1e2-rf10`; six mutants killed). Should-fix 3 (a bodiless callee:
  covered by clause (iv)), 4 (three over-strict cases listed; a global array's atom now exempts), 5 (`v[i] =
  f();` keeps its rejection with a message of its own), 6 (eight rules without a C-level must-fail test: tests
  added): taken. **Not taken, in handoff §3 with reasons:** 7 (the lint does not tie scope-exit rows to the
  allocation; the self-test pin stands) and 8 (a pin unreachable until Part C).
- **The short read** (a third fresh auditor, on `2ac4b4d`: the fix diff, the route enumeration, the pins, the
  pages; `auditor-report-2b.md`): round 2's ten probes are rejected and the four existing `restrict` tests keep
  their outcomes; **blocking 1, one more `restrict` form verifies (above): recorded open and unsound, not fixed,
  by the advisor's stop rule for this session**; blocking 2, three new tests failed as written (corrected under
  question 160, below); blocking 3 and 4, two page statements were false (the ranges section claimed a check
  that is not made; an error message was misquoted): corrected; should-fix 5, 6, 8 (the pages) and 10 (the
  record's wording) taken; **not taken, in handoff §3:** 7 (the encoder's text at the `restrict_backed` call
  site names the wrong atom for two shapes: a wrong text on a correct rejection) and the false text of C++'s
  rejection of an element that reads its own array; 9 (rules pinned only by VIR cases, listed there).

**Questions** (handoff §8):
- **Open for the owner:** 320 (the parameter spelling `int (*a)[3]`: an existing pin rejects it and the plan lists
  that pin both as unchanged and as lifted; fail closed; Part E); 322's second half (a truth test or null
  comparison of `&a[i]` on an array of scalars: GCC warns, cerastes accepts; not unsoundness); 325 (a lone zero not
  written as a literal, `{(0)}`: clang's C warns, cerastes accepts; not unsoundness); **326 (`restrict`: the
  complete rule)**.
- **Settled with the advisor, for review:** 321 (question 304's rejection is the measured set of both compilers),
  322 (the address tests on rows and elements of arrays of aggregates), 323 (the depth-2 copy is built although
  its spike took 35 s), 324 (25 fixtures over the spike's cap), 327 (C++ rejects every unbraced designated
  structure or array member, as clang++ warns `-Wc99-designator` without a flag), 328 (a function holding several
  arrays of structures is slow: the test split, the limit listed), 329 (audit round 2's two `restrict` forms:
  clause (iv) and the pair rule built; one VIR self-test case re-expected; **and the stop rule: no third fix
  cycle on `restrict` in this session**, under which the form the short read found stays open), 330 (the red
  check: the one test case fixed and the check run a second time).
- **Settled by the orchestrator inside the record's rules, listed for review:** the staging pin for
  `sizeof(struct P[3])` is Part E's; loop reach is by field as item 10 states (question 318's default), with two
  pre-registered tests rewritten before any run; `p[i].x = p[i].x++` through a pointer is the path-precise
  `unsequenced`; C++'s `{0}` over a chain of only members ending in an array is accepted (the compilers are
  silent; the base rejected it); designated lists follow the measured predicates.

**Programs accepted before this session that are now rejected** (each fail closed, each measured, none held by
an existing test or doc example; record 0064 items 16 and 19): in C++, `{0}` whose elided subaggregate's parent
is an array or a structure of two or more members, and every unbraced designated structure or array member
(`{.p = 0}`); in C, a nested braced list whose element elides braces (`{{0}}` forms GCC flags) and a non-zero
unbraced designated value (`{.p = 1}`); in C, `restrict` in a function whose body names or calls its way to an
object the parameter could point into, passes the pointer (or any pointer of that kind) to another function, or
whose contract sets it beside a range that may be empty (the last also where record 0060 item 14 accepted it:
`owns_range(p, n)` beside `reads_range(q, m)` with neither count stated positive). **Newly accepted:** C's lone `{0}` over a structure whose first member is
a structure that begins with an array, and C++'s `{0}` over a chain of only members ending in an array (both
rejected before by a defect; the compilers are silent).

**Deviations from the plan, for confirmation:**
- Briefings carried an "Additions" block (the lane environment file `build/m4.5e1e2/box.env`; the other
  sessions' lanes are copy sources only; lane-free first legs because the lanes did not exist yet).
- Runs on the Mac: P83 (a)–(g) with the frozen front end (single-file front-end probes), then once in the `il`
  lane: identical; test-mutator-1's preliminary compiler matrix and front-end typo checks of its probes (the box
  matrix is the one on record); edg-backend-2's single-file `cerastes verify` probes of tags and item-16 forms,
  redone in its lane for its report's tables; the docs builds (as the plan has them).
- **Amendment lines:** records 0012, 0037, 0043, 0059, 0060 and 0063 each got a line naming what 0064 changed in
  them so far; **not done:** the lines in 0024, 0028, 0044, 0045, 0047, 0052, 0055, 0058, 0061 and 0062, whose
  amending items are the continuation's (Parts C to F), and which the session that builds each item adds.
- The lint's first rules missed Commit 1 (the advisor: do not hold round 1 for them) and are in Commit 2.
- Instances: edg-backend 1→3, il-analyst 1→1b→1c for the spike and 2→2b→2c for Part B (2c built the two `restrict`
  fixes of audit round 2 and corrected the self-test case), test-mutator 1→4, a fresh auditor per round and for
  the short read. Two il-analyst instances ran at once for an hour (Part B's lowering; the audit-requested
  fixtures), within three Sonnet agents alive.
- Part B's tests were written before audit round 1 returned and revised from the record's diff; audit round 2
  started while those tests had their first run.
- **Test counts against the caps:** 56 new test files (45 `array_struct_*` for Part B against its cap of 35, and 11
  `restrict_body_*` for item 19, which the plan did not have): beyond the first 35, two twins asked by audit round
  1's record changes, the designated-list pins and positives' pins, the invariant-read pair, the pointer-base twin
  and pin, and the eleven files audit round 2 asked for (three for Part B, eight for `restrict`); four pins were
  retired (the plan's Flips A for Part B). S48 has 79 fixtures against 56 (question 324).
- **The full check ran twice**, and this session's new VIR self-test cases had been proved by the il-analysts' own
  sweeps (Silicon by hand, comparing the SET of error lines with the expect lines) instead of through
  `scripts/run_tests.py`, which counts errors exactly: one case with two errors at one line passed the sweep and
  failed the check (question 330).
- Runs outside the plan's test budget: the second check run; the walker oracle and the two tree checks alone; the `restrict` probes on the frozen base (two rounds, question 326); the
  compiler measurements and frozen-base probes for questions 321, 322 and 327; a compile pass of all 5,344 test
  and example sources (for the count of existing files a rule could change); item-16 probe sets by edg-backend;
  the lowering's five mutant builds; 18 scratch variants of S48 fixtures; the cost curve for question 328; the
  auditors' probes; the fix agents' probe sets on the auditors' shapes.
- A network cut ended all three agents at 14:30 UTC; each was resumed with nothing lost.

**Corrections of new tests after their first run** (question 160; no verdict changed; `build/m4.5e1e2/
test-mutator-report-{11,12}.md`, `il-analyst-report-{1,2,3}.md`):
- S48 fixtures: `s48_e_vacuity_fail` (expect L169 → L171: the callee lacked the global facts), `s48_h_*` (three
  expect lines +1: a missing field declaration), `s48_n_four_leaves` (an assert the design does not support
  removed), `s48_j_*` (two files merged to fit a vacuity twin), `s48_v_g_w_copy_fail` (split in two: Silicon stops
  a path at its first failing assert).
- Part B: `array_struct_loop.c`, `negative/array_struct_loop_fill_other_leaf.c`,
  `negative/array_struct_loop_inv_written.c` (EDG's #550 and an unused-but-set variable: a real read added);
  `negative/array_struct_sequencing_pointer_same.c` (a second real error, `overflow`, designed out);
  `restrict_body_owns_global.c` (its `main` was itself `unsequenced`); `negative/restrict_body_reads_global_wrong.c`
  (its `main` did not fix `g`'s value). No `expect:` line changed in any of them.
- Before any run: the two loop positives and the pointer-base sequencing row (above).
- After the first run, restructured for cost (question 328), expect lines unchanged: `array_struct_init.c` (its
  five arrays, one per function now) and `negative/array_struct_copy_wrong.c` (two elements).
- At the last run (report 14), three new files that had not been run before: `negative/array_struct_reject_more.c`
  and `negative/array_struct_reject_more_c.c` (two own-initializer rows moved to the c23-only file: in C++ the
  brace-elision message comes first with another tag); `negative/restrict_body_reach_forms.c` (a function dropped:
  an `ensures`-only atom on a global is rejected at the contract by record 0052, so item 19's clause (ii) cannot be
  reached that way; rows `@ L25, L35, L44` became `@ L20, L30`); `restrict_body_pair_ok.c` (`(void)n;` for the
  native compile).
- After the full check's first run: the VIR self-test case `e1e2b_loop_fill_other_leaf_fail` (its check now
  asserts one leaf fact; its header and expected category are unchanged).

**VIR self-test cases re-expected** (the plan's Flips F; hand-built cases, not `expect:` lines):
`arr_rej_elem_addr_over_struct_pointer` (`selftest_cases28.inc`) now expects the array-member rejection, since
Part B accepts an element of a pointer to a structure in code; **`rngm_restrict_range_other_covered`
(`selftest_cases30.inc`) went from accepted to rejected**: `restrict p` backed by `owns_range(p, n)` beside
`reads_range(q, m)` with neither count stated positive is the unsound shape the pair rule removes (the advisor's
ruling, question 329), and a sibling with `n > 0 && m > 0` is the positive. Part A's guard case
`e1e2a_guard_array_of_structures_alloc` (new in this session) became a lowering case when Part B gave it rows.

**Limits listed by this session** (record 0064): **`restrict` is not proved, and one form is known to verify
wrongly (first paragraph above; item 19)**; loop reach is by field, so a loop that writes `v[i].x` loses
`v[k].y` of the same type (item 10; pinned `array_struct_loop_keep_false_alarm.c`; M4.8); an application over a
pointer into an array of structures in an invariant is a false alarm; `&p[0]` in a truth test is rejected though
GCC is silent on it; under `--strict-init` a local array of structures needs an initializer (`uninit` is rejected
on it); a function holding several arrays of structures is slow (1 to 5 arrays of two-leaf structures: 7, 11, 17, 36, 65 s; five mixed ones 735 s, a timeout; item 6, question 328); the `restrict` rule rejects three shapes C allows (a read-only use
of the global, `same(p)`, a guarded atom).

**Checks** (every test run on the box, lanes `~/cerastes-e1e2/{base,il,edg,work,audit}`):
- **Neutrality:** no pre-session program's `.vpr` or `.map` changed at any proof: 6,683 of 6,683 byte-identical
  (Part A alone; each half of Part B; the joined proof `refs/tmp/e1e2-j1` = `83f447c`: 6,746 regenerated, 0
  missing, 63 new); the 2,448 baseline VIR cases byte-identical (2,499 lowering and 429 rejection cases, 0
  problems); `rejections.py --base`: no changed row, 8 removed (the four retired pins), 18 added.
  **The final joined proof** after audit round 2's fixes (`refs/tmp/e1e2-j2` = `6a8d73b`; lane `work`, its `git
  status` empty): 6,750 regenerated, 0 missing, 6,683 identical to the baseline, 67 new; VIR self-test 2,512
  lowering and 449 rejection cases, 0 problems, one baseline case re-expected by ruling
  (`rngm_restrict_range_other_covered`); `rejections.py --base`: no changed row of an existing file (the eight
  existing `restrict` pins compared row by row), 8 removed, 23 added.
- **The lint:** `test_lint_viper.py` 1,038 OK; `lint_viper.py` clean over the joined regeneration (6,746) and the
  2,499 VIR cases.
- **New tests, each run once on the box, timed:** S48's 79 fixtures (7–85 s); Part B's 48 files: 42 passed at
  once, six after one test-side correction, and `array_struct_init.c` timed out in c23 (five arrays of structures
  in one function; split into one array per function in the same file, it passes in 22.9 s; the copy twin, 122 s,
  was made smaller: 60.8 s; question 328).
- **Preflight, once, after the last fixes, on the final joined build** (`refs/tmp/e1e2-j2`; reports
  `edg-backend-report-8.md`, `test-mutator-report-14.md`), all exit 0: `format.sh --check` (91 files conform);
  `py_compile` over `scripts/*.py`; `run_tests.py --check-headers` (6,305 headers); `mutate.py --validate` (374
  tests, 12,864 mutants generated); `test_run_tests.py` (OK, 1 skipped); `test_lint_viper.py` (OK, 1,038 tests);
  `lint_viper.py` clean over the regeneration (6,750 programs) and over the VIR self-test's output (2,512 cases);
  `VIR_CASES_MIN` raised to 2512; `rejections.py --base` (only the planned rows); on the Mac `build_docs.sh` exit
  0 and `check_doc_examples.py` source (26 pages, 383 examples, 0 problems) and `--html` (27 files, 0 problems),
  rerun after the last page edit. The new and changed tests of the last cycle each ran once on that build: all
  pass (slowest 62 s, `negative/array_struct_copy_wrong.c`).
- **`CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` on the box** (`~/cerastes-s8/p-ubuntu-26.04`), two runs
  (the plan: one; the advisor's ruling 330):
  - **First run, at Commit 2 `b9e1ab9`: RED** (exit 1; 21:48:55 to 22:42:43 UTC; log
    `build/m4.5e1e2/check-run1-b9e1ab9.log`). Passed: the trees, the build, lint, the EDG configuration, the
    harness self-test, **the tests (16,797 passed, 0 failed, 10 skipped; native-ok runs: clang 3,123, GCC
    3,123)**, the Viper lint; the VIR self-test wrote 2,512 lowering and 449 rejection cases with 0 problems.
    **Failed: the VIR cases through the harness, 2,511 passed, 1 failed**: `e1e2b_loop_fill_other_leaf_fail`,
    a new hand-built case whose check asserted two leaf facts at one line, so Silicon reported two errors where
    its header registers one (`unexpected error: assert:assertion.false @ L144 … v_long == 6`). The steps after
    it did not run; run alone afterwards on that commit they pass (`walker_oracle.py`: 136 goldens, 1,924 facts,
    0 failed; the two tree checks).
  - **The fix, `aebf158`:** that case alone (`src/viper/selftest_cases35.inc`, +5 −3: it asserts the one fact
    that fails; no rule of the lowering or the checker changed); proved in the `il` lane (the case passes through
    `run_tests.py`; the self-test's output differs from before in that case only; all 65 VIR cases of this
    session through the harness: 65 passed, 0 failed).
  - **Second run, at `aebf158`: GREEN** (exit 0, "check.sh: all green", the long tier ran; 22:47:45 to 23:42:07
    UTC; log `build/m4.5e1e2/check-run2-aebf158.log`): tests 16,797 passed, 0 failed, 10 skipped (native-ok
    runs: clang 3,123, GCC 3,123); the VIR self-test 2,512 lowering and 449 rejection cases, 0 problems, and its
    cases through the harness 2,512 passed, 0 failed; the walker oracle, the docs and both tree checks passed.
    **`aebf158` is the commit checked; the closing commit differs from it only in `internal/` and `plan.md`.**
- `src/` and `include/` against `f07d80a`, measured at the joined proof: +2,025 −251 without the self-test cases
  (+3,757 −254 with them); at the final proof, with the `restrict` rule and round 2's fixes: **+2,387 −306
  (+4,852 −311 with the self-test cases)**: `encode.cpp` +681, `lower.cpp` +672, `vir.cpp` +689 (about 330 of
  them the `restrict` rules), `check.cpp` +264. The plan estimated 1,400–2,400 for Parts A and B.
- Hours per step: Commit 1 at 1 h 16; Part A proved at 0 h 46 (built during Step 1); audit round 1 returned at 2 h
  31; Part B's halves proved at 4 h 35 and its joined proof at 5 h 03; audit round 2 back at 6 h 08; its fix
  cycle (two `restrict` rules, eleven new test files, a network cut) proved at 7 h 15; the short read back at 7 h
  36; Commit 2 at 7 h 48; the full check's first run from 7 h 52 to 8 h 46 (red); the fix committed and the second run
  started at 8 h 51 and ended green at 9 h 45.

**First step for the next session:** the owner reviews handoff §8 (four open questions; 326 first) and merges or
pushes the branch (handoff §2). Then the continuation: `claude --agent orchestrator --effort high`, first message
`M4.5e1e2, from Part C`. It runs the plan's Step 0 again on the new base (worktree, lanes, baseline, R1 and R3; no
P83, no S48: every row has run) and goes on from Part C's line in the plan's "Order of work", with the audit
rounds numbered on from 3.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| M4.5e1e2 (first session) Arrays of structures in code; `restrict` | 2026-11-25 | `m4.5e1e2-arrays-of-structures` (not merged at the session's end) | 0064, for the items built (0012, 0037, 0043, 0059, 0060, 0063 amended) | Parts A and B of six: leaf paths and the traps closed; automatic local arrays of structures in code; question 304's rejection as the compilers measure it; `restrict` found unsound on `main` by audit: every form found but the last is rejected, one is recorded open and unsound (item 19, question 326 for the owner); S48 in full (79 fixtures); closed at split point 1 under the 12-hour rule, Parts C–F to the continuation |
| M4.5e1e1 Arrays of scalars in structures and static storage | 2026-11-25 | `m4.5e1e1-arrays-of-scalars`, PR #37 `76fc7b1` | 0063 (0012, 0024, 0028, 0041, 0043, 0044, 0059, 0060, 0061, 0062 amended) | global and `static` arrays, array members, array typedefs and `sizeof`, C array literals, structure copies and by-value passing up to 7 array elements; a compound-literal sequencing hole open since M4.5c2a closed; questions 301–309, no cut; the full check run by the owner after the session (green) |
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
