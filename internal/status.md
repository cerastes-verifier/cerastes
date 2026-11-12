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
`git show 023ea50:internal/status.md`, M4.5d2b2d's `git show d88a457:internal/status.md`, and M4.5e1a's `git show 685034b:internal/status.md`.

## M4.5e1b — Ranges in contracts (2026-11-11/12): done on branch `m4.5e1b-ranges-in-contracts`

Plan: `.claude/plans/m4-5e1b-ranges-in-contracts.md` (approved 2026-11-11; executed without re-planning). Record 0060
(amends 0011, 0028, 0029, 0037, 0038, 0039, 0040, 0042, 0059). Read `internal/handoff.md` next: §1, §2, the rules of §4,
§6, §8 (questions 238–246; 243–246 the answers built overnight). Branched from `origin/main` = `685034b` (the owner had
pushed the planning). Session from 2026-11-11 18:45 to 2026-11-12 (about 7 h before the box run).

**Owner decisions in the session:** the split point at Commit 2 (Parts A+B at +1,602 `src/` lines, over the plan's
1,600, about 250 of them staging code Parts C and D deleted): "Continue (Recommended)". At 2026-11-11 evening the owner
logged off with: "You continue but if you have any questions use the same process you have now, but automatically
accept the recommended answer for the remainder of this session and this session only". **Answers built that way**
(each in handoff §8, numbered 243–246, for confirmation): Part E kept whole, no pre-agreed cut (the owner's completeness preference; the
session stayed under 12 h); a local array's place frames `q[i]` read through a pointer in an invariant (record 0060
item 11); the back-edge rows of read range places in the sum form; audit round 4's S1 (an exit 3 after a wrong
program's correct error) not fixed, listed for M4.8.

**Step 0.** CI recorded only: the 2026-11-11 nightly (`f57ee43`) failed on the Linux jobs at apt (DNS: "Temporary failure
resolving archive.ubuntu.com"), macOS cancelled at 6 h; the 2026-11-10 and 2026-11-09 nightlies cancelled (question 176).
Baseline: 5,743 `.vpr`/`.map`, VIR self-test 1,825 lowering and 303 rejection cases, frozen binaries in
`build/m4.5e1b/cerastes-base/`. R1: the eight array-parameter forms verify, native-ok. R2: every range construct failed
at the atom; twenty messages named M4.5e1b or "M4.5e1"; VLA parameters on a bodiless declaration verified (question
241). R3: 1,125 `rejected` files, 1,974 runs pass (7 Linux-only skips), `build/m4.5e1b/s0/r3.sh`. M: two tags
(`sizeof_array_argument`, `array_parameter_mismatch`); a shorter array argument is silent (no tag).

**Done** (edg-backend, three il-analysts and the test-mutator, Sonnet; the soundness-auditor, Opus; the advisor at the
plan's checkpoints 1, 3 and 5 and at Step 0; at most three Sonnet agents alive at once):
- **P79, S44, record** (`42d4832`, `0d2b9e0`, `b033296`): P79 all "Expected" (F2 C++'s parameter bound under a
  generated cast, F3 an earlier declaration's bound through its secondary declaration, F4 `owning(n)`'s argument by
  position); S44's 55 fixtures (`s44_*`), every spike on its "Expected" branch except (b), which took its table's "If not"
  branch (the shape row first at a call); the addendum after advisor checkpoint 1 (the built order in every caller
  fixture, the null row in the accesses).
- **Parts A and B** (`cb23a33`): the atoms, element reads in contracts, `f__wf`, partners (kind, parameter, count) and by
  path, the callee's returns; staging pins S1, S2.
- **Part C** (`a2c8d54`): calls over ranges (the shape row first, separation against every cell, item 6's amount with
  the caller's own read amounts, sum-form marked rows where two read ranges meet), placement, `LoopWrites`.
- **Parts D and E** (Commit 4): loops over parameters' ranges (sum form at entry and back edge; framing of reads through
  range, local-array and cell places); markers over ranges; `owning(n)` on a parameter and `CERASTES_OWNING_N`;
  `restrict` backed by a range; array parameters, VLA declarations, `int a[][3]`, the two new tags; the carrays port;
  the user pages; `internal/encoding.md` §3; the developer guide's paragraph.
- **Audits:** round 1 on the record (B1: the partners-by-path rule missing for ranges, a false proof through a trusted
  contract; seven should-fix, all taken); round 2 on A+B (no blocking; five should-fix, taken); round 3 on C (B1: an exit
  3 on a correct program, two read ranges meeting at a lowered slot, fixed by sum-form rows at calls; S1 taken); round 4
  on D+E, the pages and the port (no blocking; round 3's B1/S1 re-checked; S2–S4 taken; S1 not taken, listed). The plan's
  one re-audit was not needed: round 4 had no blocking item and checked round 3's fixes as named items. Reports
  `build/m4.5e1b/auditor-report-{1..4}.md`.

**Deviations from the plan, for confirmation:**
- il-analyst-1 was resumed after Commit 1 for the S44 addendum (advisor checkpoint 1), the runner lent back to it.
- Part C's and Part D's tests were pre-registered while the previous part was audited (new files only; each commit was
  cut from its audited snapshot plus its fixes).
- The staging S2 rejection was widened to the lowering's gate (loops with a `return` or outward `goto` too).
- `range_owning_reject_arguments.c` was dropped: EDG's registration refuses `owning(n, m)` before cerastes.
- The session's tools (`build/m4.5e1b/s0/r3.sh`, `r3_collect.py`, `widen.sh`) were not committed (handoff §3,
  "Process"; §4's rule, question 193).
- Record item 8's planned order (marked rows first) became the shape row first (S44 (b)'s table); item 8's argument that
  no range sum row is needed at a call (audit round 1, S5) was broken by audit round 3 and replaced by sum-form rows.

**Corrections of new tests after their first run** (question 160, or a new test's program before its first commit):
the S1 pin's tag (`unsupported_expression`); `range_reject_count_double.c` (a literal `2.0`); `range_reject_partner_cast_one_side.c`
replaced by `range_partner_noop_cast.c` and `range_reject_partner_value_cast.c` (a written no-op cast is one VIR count);
`range_framing_alias_cell.c` (`cerastes_live(q)` added), `range_framing_ensures_no_atom.c` (+`postcondition`, 0040 item
4), `range_guards_unchecked.c` (`null-dereference` and `lifetime`), `range_partner_guard_mismatch.c` (one return error),
the three S2 loop pins (one line); `range_reject_cell_through_range_load.c` (+the `ensures` site); `range_call_separation.c`
(`ok2` as a `T**` parameter with 0045 item 10's tie); `range_call_reads_unequal_pin.c` (reworked twice to reach question
238's limit); `range_loop_callee_kept.c` (the body's closing brace), `range_loop_pin_assigned_param.c` (rewritten to read
through the parameter), `range_loop_element_unframed.c` (two errors: no inductive form exists),
`range_loop_pointer_into_array_elsewhere.c` (`live(r)`); `range_marker_reject_reads_range.c`,
`range_owning_reject_count_beside.c`, `range_owning_reject_owns_beside.c` (L9); `array_param_reject_array_of_arrays.c`
(+`unsupported_type`); `array_param_owns_range.c` and its twin (`old` in the callee's `ensures`).

**Not done / limits** (handoff §3): question 237's assigned-parameter false alarms; question 238's limit (one- and two-atom
pins); question 239's loop-filler pin; audit round 4's S1 (an exit 3 after a wrong program's correct error); a
pre-existing exit 3 found on the baseline (a pass-through of `owns(*pp) && owns(**pp)` without 0045 item 10's tie,
`build/m4.5e1b/t4/sep/d1.c`).

**Checks** (the Mac, on the final build, every build held):
- The session's tests: 684 runs of 200 test files, the 55 S44 fixtures, the carrays port and the four doc examples
  passed (native-ok clang 114, GCC 114); R3 2,060 runs, only Flips A1–A4 removed and 47 new pins added; vprdiff 5,743
  of 5,743 existing `.vpr`/`.map` identical (6,029 regenerated, none missing); VIR self-test 2,044 lowering and 324
  rejection cases, 0 problems, the 1,825 baseline cases byte-identical (`VIR_CASES_MIN` 2044), every session case
  through Silicon (the slowest a failing twin, 19.5 s); lint clean on 6,029 + 2,044 files; `test_lint_viper.py` 809
  (was 782); `test_run_tests.py` 1,221, OK (6 min 25 s); `--check-headers` 5,420; format 87 files; `build_docs.sh` and
  `check_doc_examples.py --html` (26 pages, 379 blocks).
- `src/` and `include/` against `685034b` (measured): +2,754/−155 without the self-tests (the plan's 2,050–3,050), +6,289/−173
  with them; at the split points Parts A+B were +1,602 (about 250 staging lines, deleted later) and Parts A–C +2,039.
- Before the box: a widened run of the existing tests no earlier run covered (183 golden, `rejected` and
  `frontend-refused` files in `tests/` and the doc examples: 399 runs passed) and `test_walker_oracle.py` (79, OK).
- The box (`ubuntu-26.04` container, the branch at `c58060e` as a bundle from `3ceb13c`; a fresh bootstrap build): a
  pre-check — regeneration 6,033 files, 0 missing (the new array-parameter rules reach no glibc declaration the tests
  use), lint clean; 677 runs of the session's tests and the seven Linux-only tests passed (native-ok clang 114, GCC 114:
  the box's compilers warn on no positive) — then `CERASTES_LONG=1 scripts/check.sh` (`-j32`) on `c58060e`: **all green** in 84 minutes (14,871 passed, 0 failed,
  10 skipped; native-ok clang 2,836 and GCC 2,836; the long tier ran; lint 6,033 files clean; VIR self-test 2,044
  lowering cases, 324 rejection cases, 0 problems, 2,044 passed through Silicon; the walker oracle; `test_run_tests.py`
  1,221, `test_walker_oracle.py` 79, `test_lint_viper.py` 809; `--check-headers` 5,420; format 87 files; the doc
  examples (355) and docs; vlibc's headers). Nothing in `src/`, `include/`, `scripts/` or `tests/` changed after it.

**First step for the next session:** the owner merges `m4.5e1b-ranges-in-contracts` and answers questions 238–246 inline (243–246
are the overnight answers) (handoff §2, §8); then M4.5e1c (quantifiers) from `.claude/plans/m4-5e1-arrays.md`
section 5: its Step 0, P80 and S45 (they change no `src/`), then plan again (handoff §2 has the opening prompt).

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
