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
`git show 023ea50:internal/status.md`, and M4.5d2b2d's `git show d88a457:internal/status.md`.

## M4.5e1a — Arrays inside one function (2026-11-11): done on branch `m4.5e1a-arrays-in-functions`

Plan: `.claude/plans/m4-5e1-arrays.md` section 3 (there is no `m4-5e1a-*.md`). Record 0059 (amends 0011, 0012, 0024,
0028, 0032, 0038, 0039, 0042, 0043, 0054). Read `internal/handoff.md` next: §1, §2, §3 "M4.5e1b", §4.2, §8 (questions
234 on).

**Owner decisions:** 229–233 answered inline before the session ("Follow recommendation" each), committed to local
`main` as `1c5f9bd`; the branch is from it (not from `origin/main` = `d88a457`, which lacks only that commit). Open,
each built as its recommendation: **234** array designators rejected in C too (audit round 1, B1); **235** the 1,080
`.vpr` files of Flips D, where the pre-build allow-list predicted 849.

**Step 0.** CI recorded only: no `workflow_dispatch` run; the 2026-11-10 nightly cancelled at 6 h 01 min; the
2026-11-11 one running. Baseline: 5,412 `.vpr`/`.map`, the VIR self-test's 1,765 cases, frozen binaries in
`build/m4.5e1a/cerastes-base/`. R1: `int a[static 3]` verified in C (question 230: now rejected); R2's messages kept;
R3: 1,875 runs of the `rejected` tests, 0 mismatches on the base.

**Done** (the three Sonnet roles and the Opus reviewer from `.claude/agents/`; the il-analyst was replaced once, after
S43; the orchestrator consulted the Fable advisor before audit round 1, at the split point, at Part D's stop and
before the box run):
- **Record, P78, S43** (`05b4075`, `d09c3fc`): P78 (F1 a bound naming a local constant, F2 `int a[0]`, F3 C's silent
  excess initializers, F4 `address_taken`); S43's 49 fixtures (G2–G4 pass; the draft's `p[0]` concern refuted), with
  G2's probe and the element-type loop addendum (56 fixtures `s43_*`). The Step-0 matrix (#175, #1162 and the
  compilers' shapes) and audit round 1 (B1, B2: array designators rejected) in the record.
- **Parts A and B** (`0209647`): VIR's `Array`/`ElemAddr`, the lowered `Forall` and labelled `Old`, the marked map
  column and the driver's exact-reason gate; local arrays as blocks (`alloc_arr_<t>`, `zero_arr_<t>`), `a[i]`/`&a[i]`,
  initializers with C's zero fill, `out-of-bounds`, scope exits as marked range rows, sequencing between elements;
  #175, #1162 and #138 errors; `array_address_null_test`, `c99_designator`, `char_subscripts`; audit round 2's B1
  (`register`) and the VIR self-test's reasons.
- **Parts C, D and E** (`ccb5910`): loops carrying arrays (range places, marked entry and back-edge rows at the loop
  keyword, quantified conjuncts, the reach rule), the decay and `p[i]` on any pointer, the `offset_of == 0` backstops
  (1,080 `.vpr` files, each changed line classified), `sizeof(a)`, `uninit` on arrays; audit round 3's fixes.
- **Tests.** 217 new test files in `tests/memory` (positives and twins, pins), 56 S43 fixtures, 6 doc examples (counted
  by `git diff --diff-filter=A 1c5f9bd`); the staging pins written and deleted inside the session; Flip A's
  `reject_array.c` retired, Flip B's line, Flip C's `Pins:` comments. VIR self-test 1,825
  cases (was 1,765; 303 rejection cases), every expect line now naming Silicon's reason; `test_lint_viper.py` 782 (was
  736); `test_run_tests.py` 1,221.
- **`src/` sizes** (`git diff --numstat 1c5f9bd`, measured): without self-tests, Parts A and B +1,575/−67, Parts C–E
  with audit round 3's fixes +480/−138, +1,943/−93 in all (the plan's 2,200–3,450 with Parts A–E); with self-tests
  +3,457/−96.
- **Docs:** Memory and ownership "Arrays", Error messages `out-of-bounds`, Verifying (the compilers' shapes,
  `--strict-init`, not-supported), Annotations (`uninit`), the developer guide's map description, `tests/README.md`,
  `internal/encoding.md` §3 "Arrays", `internal/il-inventory.md` chapter M4.5e1a, ten records amended. No tutorial
  chapter (e1c, 0049 item 4).
- **Audits** (one Opus reviewer, resumed per round): round 1 on the draft (B1, B2), round 2 on Parts A+B (B1
  `register`, five should-fix), round 3 on Parts C–E (no blocking item), the re-audit of round 3's fixes (no blocking
  item; its lint should-fix taken in `d507551`). No false proof in any round; round 1's B1 (a wrapping designator unchecked) and round 2's B1 (a `register`
  array, and `&` of a `register` scalar since M4.5b1a) were holes, closed.

**Deviations from the plan, for confirmation:**
- edg-backend built all of Part A (the plan split it with the il-analyst, then on S43) and Part B's `check.cpp`.
- Part B's encoder half started before audit round 1 returned (the record draft existed).
- Parts A and B were committed together from the audited snapshot plus round 2's separable fixes; Parts C, D and E were
  audited in one round and committed together (Part D's encoder began in the shared tree while Part C's lowering
  finished, so no Part C-only build existed).
- The plan's constant-index twins became rejections (EDG's #175 an error in both modes, also inside `if (0)`, where only
  EDG warns); C's excess initializers are #1162 made an error.
- The array loop rows are positioned at the loop keyword (`Stmt::keyword_loc`, new), so both modes report one line.
- VIR self-test expect lines name Silicon's reason (audit round 2): every existing case's header changed.
- The lint exempts three exact loop-carry shapes from the `offset_of == 0` rule (records 0050, 0056: not restated).
- Audit round 1 item 15's predicted `pk[0]` false alarm did not reproduce; its pin became a positive with twins.
- `internal/encoding.md` and the developer guide's paragraph were delegated to edg-backend; the orchestrator wrote the
  user pages and the six doc examples itself (the plan has the test-mutator write examples).
- Four Sonnet agents were spawned in all, never more than three alive at once (the il-analyst was replaced after S43).

**Corrections of new tests after their first run** (question 160): `array_reject_self_reading_init.c` lost an unrun
`frontend-warning` line; the `p[1]` twin on a parameter with only `owns(*p)` expects two errors (S43 N1); Flip A's twin
`r == a + 1` overflowed in its own `ensures`; four Part C tests whose own invariants or bodies were wrong;
`array_ptr_walk_far.c` and `array_sizeof_bound_bytes.c` widened their `ensures` (a second error);
`array_loop_clauses_above_keep.c` L21 → L22; `array_ptr_loop_reassigned_false_alarm.c` replaced (above).

**Not done.** Nothing of the plan's scope was cut. Pinned limits: a fill loop over an uninitialized array followed by a
read (`uninitialized`, e1c); a loop that writes an array loses its values (`array_loop_self_write_false_alarm.c`, e1c);
handoff §3 has the rest.

**Surprises.**
- **Front-end facts the plan could not see:** C silently drops excess initializers; a designator keeps only its folded
  index; `register` arrays verified; C++ positions an attributed statement at its first attribute.
- **The Flips D allow-list was short** (849 against 1,080): the search missed the token's `.tok` spelling; every changed
  line was then classified by exact inverse rules.

**Checks.**
- The Mac, on the final build (`ccb5910`): 787 runs of the session's tests, S43 fixtures and doc examples passed
  (native-ok clang 150, GCC 150); VIR self-test 1,825 cases, the 60 array cases through Silicon (1,816 with reasons
  earlier, and the 156 changed bodies); R3 1,968 runs, only the flips and 95 new pin runs differ; vprdiff 4,332
  identical, 1,080 allowed, 0 unexpected, no `.map`; lint clean on 5,743 + 1,825 files; `test_lint_viper.py` 781,
  `test_run_tests.py` 1,221, `--check-headers` 5,164, format 86 files, docs; after the re-audit's lint fix
  (`d507551`) 5,743 + 1,825 files lint clean and `test_lint_viper.py` 782.
- The box (`ubuntu-26.04` container, `-j32`), before `check.sh`, on `ccb5910`: the 552 tests behind the 1,080 Flips D
  files (6 in `tests/long`), 2,042 runs passed (native-ok clang 479, GCC 479; 837 s); the six slowest default-tier tests
  before and after the backstops, one at a time: 794 s against 803 s in all (ratio 1.01, the slowest 74.9 s, none
  doubled); the `tests/long` ones 111–over 300 s (within their `timeout:` headers).
- `CERASTES_LONG=1 scripts/check.sh` on the box (`ubuntu-26.04` container, `-j32`) on `3ceb13c`, the branch's last
  change before this status commit: **all green** in 78 minutes (14,208 passed, 0 failed, 10 skipped; native-ok clang
  2,722 and GCC 2,722; the long tier ran; lint 5,747 files clean; VIR self-test 1,825 cases, 0 problems, 1,825 passed
  through Silicon, lint clean; the walker oracle; `test_run_tests.py` 1,221, `test_lint_viper.py` 782; format 86 files;
  docs). Before it, a Linux pre-check: regeneration 5,747 files, lint clean, R3 1,960 runs with 0 mismatches.

**First step for the next session:** merge and push (handoff §2), then M4.5e1b: its Step 0 and the probes and spikes of
plan section 4 (P79, S44), then plan again.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
