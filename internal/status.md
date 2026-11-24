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
`git show 023ea50:internal/status.md`, M4.5d2b2d's `git show d88a457:internal/status.md`, M4.5e1a's `git show 685034b:internal/status.md`, and M4.5e1b's `git show 7964d33:internal/status.md`.

## M4.5e1c — Quantifiers (2026-11-23/24): done on branch `m4.5e1c-quantifiers`, not merged

Plan: `.claude/plans/m4-5e1c-quantifiers.md` (approved 2026-11-23; executed without re-planning). Record 0061 (accepted;
amends 0006, 0011, 0028, 0029, 0037, 0038, 0041, 0059, 0060). Read `internal/handoff.md` next: §1, §2, §3 (the sessions
this one feeds: M4.5e1f, M4.5e1d, M4.5e2, M4.8), §8 (questions 253-263, **the overnight ones for review**). Branched
from local `main` = `bf820fe` (`origin/main` = `aa4a9fe` plus one commit holding the owner's inline answers to
247-252, committed at the session's start so that a fast-forward is not blocked). Session from 2026-11-23 about 22:35
UTC to 2026-11-24 about 07:15 UTC.

**Owner decisions in the session:** 253 (the multi-class one-term false alarm: M4.5e2), 254 (the skipped-write twin's two
errors expected, a one-error twin added), 255 (method-qualified ghost names; four narrowings as listed limits). At
about 02:30 UTC the owner went to bed: "for the rest of the session accept what the recommended answer on a question
would be after following your normal question and answer generation process and then add to the handoff so I can see
if these need relooked tomorrow". **Built that way, each in handoff §8 for review:** 256 (split point 1: continue, a
ceiling for Part C), 257 (a trigger set per state in `f__wf`), 258 (the lint admits earlier quantified clauses in a loop
obligation's guard), 259 (narrowings in loops and checks, M4.5e2), **260 (split point 2: cut 1, Part D's `cerastes_old`
in loop invariants to M4.5e2)**, 261 (audit round 3's exit 3: arithmetic over a non-first class's read refused in loops
and checks, M4.5e2), 262 (only top-level conjuncts in loops and checks, M4.8; checks split per conjunct), 263 (Part C past
its ceiling by round 3's fixes). Each recommendation was the fail-closed option; nothing was parked.

**Step 0.** CI recorded only (the push run of `aa4a9fe` in progress; the 2026-11-23 nightly and PR #35's run failed on the
docs lockfile `fabcd25` fixed; earlier nightlies cancelled at 6 h). `doc/`'s lockfile one hoisted `@antora/logger`.
Baseline: 6,029 `.vpr`/`.map`, VIR self-test 2,044 lowering and 324 rejection cases, frozen binaries (they read
`include/` live). R1: the five pins and `static_assert_pure_body.c` as their expect lines; a marked element read in a loop
invariant is accepted with its flag row (the plan's "both verify" could not hold for the pin's program, whose `must_init`
row needs a quantified invariant). R2: 99 programs, every row rejected; a `pure` function with a contract verifies when no
`forall` names it; three pins for `cerastes_old` in loop invariants. R3: `scripts/rejections.py`, 2,327 runs. M: GCC's
`-Wtype-limits` on unsigned `0 <= i`, `-Wsign-compare`, unused parameters of `f`: tests written around them.

**Done** (edg-backend, il-analyst-1/2/3 and two test-mutators, Sonnet; the soundness-auditor, Opus; the advisor at
checkpoints 1-5 and at Step 0; at most three Sonnet agents alive at once):
- **P80, S45, record, Commit 1** (`cbd3386`): P80 all "Expected" except (b), whose "If not" (cut 3: `cerastes_forall` is
  the spelling in both modes) was taken; S45 (a), (c)-(g) "Expected", (b) T2, (d) variant M, (k) and (r) recorded; two
  outcomes outside a table asked (253, 254); an S45 addendum after advisor checkpoint 1 (two classes through `f__wf`, the
  one-term workaround, the guarded keep fact, 17 fixtures; 116 in all); `scripts/rejections.py`; the lint of item 11.
- **Parts A and B** (`8a8128c`): VIR `Forall`, `Bound`, `ForallArg`; the reader; contracts, calls, returns; `f__wf`
  variant M; 103 test files.
- **Part C** (`4689b7b`): loop invariants and checks, the per-conjunct split, the check rows; the C23 Bool fix; 47 test
  files and the pins of 259, 261, 262; the `carrays.c` port in full; the user pages and the tutorial chapter "Arrays and
  quantifiers"; `internal/encoding.md`; the developer guide.
- **Part D: cut** (question 260), its design kept in record 0061 item 12 and handoff §3 (M4.5e2) with its example.
- **Audits** (reports `build/m4.5e1c/auditor-report-{1..4}.md`): round 1 on the record and S45 (B1 a trailing
  argument's obligations, B2 the lint admitting the unguarded keep fact and a state-crossing tie; eleven should-fix, all
  taken); round 2 on A+B (no soundness hole; the lint refusing an atom-less formal's fact; pins); round 3 on C (no
  soundness hole; an exit 3 on wrong programs, question 261; positions, question 262; the lint's scope of 258); round 4 on
  Commit 3, the pages and the port, also the re-audit (no soundness hole, no blocking item; text fixes, all taken).

**Deviations from the plan, for confirmation:** branched from `bf820fe` (above); the record committed in steps (the
draft, then round 1's and each later revision) rather than once; Part B's, Part C's and the doc examples' tests
pre-registered while the previous step was audited or built (new files only); the first test-mutator replaced by a fresh
one from its state file after about 940k tokens (the advisor's advice), and the auditor kept for all four rounds;
round 4 served as the plan's one re-audit (every earlier blocking item re-verified there, as in e1b); the audit worktree
was created nested inside the session worktree (git-excluded; removed at the end); edg-backend's permission classifier
refused one edit while builds were on hold, and the orchestrator's explicit assignment then let it through; the S45
fixtures keep their S45-era unqualified ghost names (the lint is not run on fixtures; handoff §3).

**Corrections of new tests after their first run** (question 160), and new test programs corrected before their first
commit: `forall_invariant_unframed_read.c` (two errors at L19, then, after round 3's note 7 guarded the flag line by the
framing, one `permission` at L18); `forall_one_term_sibling.c` and `forall_reject_do_while_invariant.c` (EDG's #550 on
a local, programs rewritten); eight pins with undefined identifiers, unlisted warnings or a wrong line (the namespace
pins list the front end's `set_but_not_used`; the function-pointer form pins `unsupported_type` and `forall_function`);
`(void)a;` on a local array (an array used as a value in C++) in three files and one pin, and #550 in eleven negatives
and one example; the shape-rule pin's line (L19 to L14); S45 fixtures (listed in the chapter). Question 254's rule:
`forall_fill_local_skip.c` expects two errors.

**Not done / limits** (handoff §3): Part D (cut 1); the false alarms of questions 253 and 257's alternative; questions
255, 259, 261, 262's narrowings; audit round 4's notes (carrays.c's line map off by one since e1b; the check's marked-row
message for a kept element); the S45 fixtures' ghost names.

**Checks** (the Mac, on the final build, every build held):
- The session's tests: 636 outcomes of 298 files (Parts B and C, the pins, the port, the doc examples, Flips B, R2's pins,
  the 116 S45 fixtures), native-ok clang 82 and GCC 82; the widened run of existing tests no run had covered (front-end ok, warning, refused and error tests, the doc examples' rejected and front-end tests, the goldens, the other rejected tests: 511 files, 1,226 outcomes passed, native-ok clang 343 and GCC 343; `test_walker_oracle.py` 79; the vlibc header and contract checks), no changed outcome (Flips E).
- regeneration on `7964d33`: 6,213 of 6,213; vprdiff 6,027 of 6,029 existing `.vpr`/`.map` identical, `carrays.c`'s two modes the only difference (Flips C); lint clean on 6,213 files; VIR self-test 2,202 lowering and 358 rejection cases, 0 problems,
  the 2,044 baseline cases byte-identical (`VIR_CASES_MIN` 2202), every session case through Silicon; `rejections.py`:
  nothing removed or changed against the base; `test_lint_viper.py` 843; `test_run_tests.py` 1,266, OK; `--check-headers`
  5,713; format 89 files; `build_docs.sh` and `check_doc_examples.py` (source and `--html`).
- `src/` and `include/` against `bf820fe` (measured): +3,514/−294 without the self-test cases (the plan's 1,500–2,200, with
  Part D), +6,038/−294 with them; Parts A+B +2,483 at split point 1, Part C about +900 (questions 256, 263).
- The box: `CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` **all green** on `7964d33` in the `ubuntu-26.04` container
  on the x86-64 box (a fresh bootstrap by `ci-linux.sh`), 89 min, exit 0, no rerun: 15,496 tests passed, 0 failed, 10
  skipped (native-ok clang and GCC 2,918 each), the long tier ran; lint 6,217 files clean (the Mac's 6,213 and the four
  Linux-only tests); VIR self-test 2,202 lowering and 358 rejection cases, 0 problems, every case through Silicon; the
  self-tests, format, the doc examples (400 blocks) and the docs (27 pages) clean. A pre-check passed first (regeneration
  6,217 of 6,217, lint clean, the session's 658 tests). The closing commits change only this file, the handoff and
  `plan.md`, which `check.sh` does not read. Report `build/m4.5e1c/box-report.md`, log
  `~/cerastes-s8/p-ubuntu-26.04/e1c-check.log`.

**First step for the next session:** the owner reviews handoff §8's overnight questions (256-263, cut 1 above all),
merges the branch (`git merge --ff-only m4.5e1c-quantifiers` in the main checkout, then removes the worktree), and
starts the next session from `plan.md` (M4.5e1d, M4.5e1e or M4.5e1f; M4.5e1f is short and planned in the umbrella plan's
section 10 with S45 (r)'s result in handoff §3).

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
