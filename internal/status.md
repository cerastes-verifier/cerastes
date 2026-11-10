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
`git show 19d977a:internal/status.md`, and M4.5d2b2b's `git show e3f4b9f:internal/status.md`.

## M4.5d2b2c — loop limits of owning fields, long-running tests (2026-11-10): done on branch `m4.5d2b2c-loop-limits`

Plan: `.claude/plans/m4-5d2b2c-loop-limits.md`. Records 0056 (amends 0055 item 7, 0054 items 3 and 5) and 0057 (amends
0016 item 9 and `tests/README.md`'s grammar). Read `internal/handoff.md` next: §1, §2, §3 "M4.5d2b2d" and "M4.8", §8
(questions 215 on).

**Owner decisions.** At planning: 195, 198–201 (the plan's "Owner decisions"). In the session (2026-11-10), each asked
when an agent's finding or a stop rule required it, all the recommendation except 209 and 212:
- **204** item 2 applies to every synthetic site (eleven programs' Viper changes by `wef_` literals only);
- **205** the VIR self-test case `owning_leaf_call_alias_leak` follows Flip B;
- **206** (audit round 1, S1) build the pre-read move after all;
- **207** (audit round 1, S6) a positive outside `tests/long/` needs a twin outside it;
- **208** (audit round 2 on L-1, S1) `loop_may_fill`'s call route counts only non-plain `ensures` sites;
- **209** (S2) **build the escape scan now** (the recommendation was M4.8);
- **210** (S3) fix the call-row exit 3 now, bounded;
- **211** (audit round 2 on L-2, B1) fix the loop-entry exit 3 now;
- **212** (B2) **keep the broad places test as built** (the recommendation was to narrow it): two programs are pinned
  false alarms, M4.8;
- **213** (S2) add the two struct-global replace routes now;
- **214** (re-audit, B1) narrow the escape scan's framing now (only structures with an owning leaf, only loops with no
  exit from their scope);
- **217** (before the box run) the six tests slow on the box move to `tests/long/` (record 0057 item 5);
- and, while 214 was queued (a standing instruction since; memory `verify-delta-not-full-check`): "For all of these fix
  now items you should discontinue the testing runs since we know that things are going to change."

**Step 0 and Step 1.**
- `origin/main` held the planning commit (`e3f4b9f`); the branch is from `origin/main`.
- CI: no `profiles=all` dispatch has run, so `macos-26` stays at `dispatch` (question 172). The three nightlies listed
  (2026-11-07 to 11-09) were cancelled at 3 h 40 min (220 min) and all started before the 360-minute limit (`99b587e`);
  no nightly has run under it yet. No CI commit.
- The baseline (5,028 `.vpr`/`.map`, the VIR self-test's 1,707 cases) was taken in the session worktree from bootstrap's
  build before any change to `src/`; the frozen base build `build/m4.5d2b2c/cerastes-base` served S41.

**Done** (the three Sonnet roles from `.claude/agents/` and the Opus reviewer, each kept for the whole session and
resumed between tasks; the orchestrator consulted the Fable advisor):
- **S41** (`75caf32`, revised in `3e86985`): (0) seven runs of the base on the pins with one change each, every one
  agreeing with the survey (limits 1, 2(ii) and 7 confirmed); (a)–(c) hand-edited Viper for limits 3, 6 and 4, each
  verified with a failing twin (nine fixtures `tests/viper-smoke/memory/s41_*`); limit 6 at 12.1–16.6 s, under the
  60 s rule, so no cut; (d) the programs items 5–7 change.
- **Part T** (`4103504`, record 0057): the long-running tier `tests/long/<area>/` with `// timeout:` (301–3600 s),
  `--long`, `CERASTES_LONG=1 scripts/check.sh`, counted skips, the PR tier refusing it, `mutate.py` skipping it, the
  twin rule of question 207; self-tests that bite (42 + 7 weakenings, then 4 more).
- **Part L-1** (`49ea34c`, record 0056 items 2–4, edg-backend): limits 5 and 2 lifted; Flips A (three pins), Flip B and
  its self-test mirror (205); the lint rule for the guarded update.
- **Part L-2** (`006e742`, items 5–7, il-analyst): limits 4, 3 and 6 lifted; Flips A (three pins); the lint rule for the
  `hsl_` ghost; `chain3` at 21–22 s built (the pair splits between a dependent closure site and its earlier site are
  left out, no-op splits).
- **Flips C** (`68ba68e`, item 8, question 201): four pins renamed (`_no_invariant` ×3 with verifying `_written` twins,
  `owner_renewed_consumed`, an ordinary must-fail by S41 (0) run 7).
- **The audits' fixes** (`738971c`, edg-backend): 208, the escape scan (209, then 214), the call-row order (210), the
  loop-entry order (211), the replace routes (213).
- **Tests.** 69 new test and fixture files and one documentation example, seven renamed (both modes; the final run on
  the last build: 408 passed, the only failures the two retired pins and two positives fixed under question 160); VIR
  self-test 1,732 cases (was 1,707; the session's 25 through Silicon); `test_lint_viper.py` 717 (was 660), each new
  rule mutation-checked; `test_run_tests.py` 1,216.
- **Docs** (commit 8): Memory and ownership ("When a loop needs its invariant written" with a tested example, the
  not-yet list), Error messages (the value rule's two `permission` rows), the developer guide's Testing page and
  `CLAUDE.md` (the long tier), `tests/README.md` (Part T), `internal/encoding.md`, record 0016 item 9; both records
  accepted.
- **Audits** (read-only Opus, one agent resumed per round): round 1 on both drafts with S41 and Part T's harness (no
  blocking item; S1–S6); round 2 on Part L-1 (B1: the route twins did not pin their routes; S1–S3 became 208–210); round
  2 on Part L-2 (B1 an exit 3 at the loop entry, B2 the places test; S1–S3); the re-audit of the fixes (B1, the escape
  scan's framing too broad: 214); a focused review of 214 (no blocking item: the exit test complete, the three users on
  one predicate). No wrong verdict was found in any round.

**Flips and pins** (the plan's, plus the owner's in-session answers):
- **A:** six pins retired for positives with twins (`pointer_arg`, `owning_local_maybe_null`, `alias_call`,
  `global_pointer`, `main_holds`, `chain3`). **B:** `owning_field_alias_fill_leak.c` L27→L26 and its VIR mirror (205).
  **C:** four renamed; `owning_field_loop_owner_replaced_alias_false_alarm.c` kept as it is (its owner is address-taken,
  so the guarded site cannot be written: rejected, or `permission` through `(*pv)->data`; handoff question).
  **D:** the allow-listed Viper changes, every outcome unchanged.
- **Pins added this session and retired by it:** `owning_field_loop_site_{call,store}_written_false_alarm.c` (L-1's
  route tests; 208 and 209 lift them).
- **New pinned false alarms** (each a correct program, its actual errors recorded, for M4.8): item 5's entry snapshot
  (`owning_field_loop_main_alias_false_alarm.c`), the owning global's replace routes
  (`owning_global_loop_replace_{name,expr,ptrptr,call_type}_false_alarm.c`) and the struct-global leaf's
  (`owning_global_leaf_replace_{member,struct}_false_alarm.c`), question 212's places test
  (`owning_field_loop_chain_{cell_write,null_owner}_false_alarm.c`), and the older `stores_type` heuristic
  (`owning_field_loop_struct_copy_int_false_alarm.c`, the same on the base).

**Corrections of new tests after their first run** (question 160; each listed, none audited separately):
- `negative/owning_field_loop_owning_local_maybe_null_leak.c` L32→L33 (the closing brace, not the return).
- `negative/owning_field_loop_chain_free_mid.c` and `…_chain3_free_mid.c`: a `leak` at the return added (the one mistake
  also leaves the middle block owned on the other paths); `negative/owning_field_loop_main_alias_false_alarm.c` records
  the encoder's actual errors (two `permission` at the call, an `invalid-free`).
- `negative/owning_field_loop_{chain3,chain,global_pointer}_freed_before.c` and `…_escape_call_unwritten.c`: the loop's
  attribute lines joined to its `while` line (C++20 reports a loop's entry row at its first attribute, P59), every
  later line moved up two.
- `negative/owning_field_loop_site_call_entry_unwritten.c` L41→L47: the same `uninitialized` at the return, not at the
  loop's entry row.
- `owning_field_loop_return_noname.c` and `…_return_nested.c` (the re-audit's r3 and r5): `return s.data == NULL ? 1 :
  0;` in place of `return 1;` (EDG's #550 and GCC's `-Werror` on a struct local set but never used; reading `s.n`
  instead was `overflow`, since the loop does not keep that member's value, as on the base).

**Not done.**
- Question 196's two programs: `cover_twice.c` (base encoder) and `two_fields.c` with its twin (Part L-2) each ran past
  1800 s in both modes on the quiet box (exit 3 at 1801 s); all three programs' Viper is byte-identical from the final
  build, so the tries stand. Neither became a long test; both go to M4.8 (question 199).
- **The long tier's first tests** are this session's own (question 217): timed on the box before `check.sh` (all 115
  new or changed programs, both modes, twelve at a time, load 2–10), six took 120–641 s there against 20–84 s on the
  Mac: the struct-global replace pins (628–641 s, 516–519 s), `owning_field_loop_chain3.c` and its twin `_chain3_drop.c`
  (177 s, 247 s), `_chain_cell_write_false_alarm.c` (170 s) and `_chain3_free_mid.c` (120 s). They moved to
  `tests/long/memory/` with limits of 1320, 1080, 360, 540, 360 and 360 s (each `// timeout:` line takes the slot of
  a joined comment line, so no `@ L` line moved); `tests/long` and the re-pointed twin `_chain3_freed_before.c`: 22
  passed on the Mac. Every other session test took at most 95 s on the box.
- The pins above; the call route of item 2 has no pin (only a marked global's deep atom reaches it, and VIR cannot
  express it alone); the whole-structure route is pinned only in the VIR self-test.
- No cut was taken.

**Surprises.**
- **The box is slower than the Mac on these shapes**, four to six times on the chain and closure loops, not
  uniformly (`pointer_arg` 1.3 times): a session times its new tests on the box before `check.sh` (record 0057 item 5).
- **The spike's description.** The il-analyst reported that S41 (b) left the deeper pre-read beside the others; the
  auditor found the spike had moved it, and that the order turns a base exit 3 into mapped rows (206).
- **Twins that pin nothing.** L-1's first route twins entered the loop with a block, so `wef` came from the entry flag
  and no route was tested; a twin pins a route only on the path that enters null (the auditor's models).
- **A benign verified twin.** A VIR escaping twin verified because the carrier invariant `hp ==> held == (s.q != null)`
  with `held` never assigned in a body without calls or frees fixes `s.q`; with a call in the body it fails as designed.
- **Cost.** The built `chain3` first took 38–72 s against the spike's 12–16 s: the third site's no-op case splits.
- **Scope.** `lower.cpp` grew by 901 lines (−102) against the plan's 295–495: the owner's in-session answers 206–214
  added the escape scan, two orderings and the routes.

**Checks**
- vprdiff against the baseline: 114 entries differ (47 programs in both modes, and the ten retired or renamed pins only
  in the base), every one allow-listed or explained (the parts' allow-lists, question 204's `wef_` literals, item 12's
  row reorders, item 11's five, 211's eleven).
- The VIR self-test (1,732 lowering cases, 274 rejection cases, 0 problems; the session's cases through Silicon),
  `lint_viper.py` over the final regeneration and the VIR output, `test_lint_viper.py`, `test_run_tests.py`,
  `test_mutate.py`, `format.sh --check`, `build_docs.sh` with `check_doc_examples.py --html`.
- `CERASTES_LONG=1 scripts/check.sh` (full, `CERASTES_JOBS=32`, clean `build/cmake`) on the x86-64 box in S8's
  `ubuntu-26.04` container, on `ee2ae5c` (the last change; the commit after it only records this result): **all
  green**, 12,745 passed, 0 failed, 10 skipped (the nine macOS-only tests of record 0031 and the VeriFast originals),
  `native-ok` runs clang 2,445 and GCC 2,445; `lint_viper` 5,148 files clean; the VIR self-test 1,732 lowering cases
  and 274 rejection cases, 0 problems, and its 1,732 cases through Silicon passed and lint-clean; the walker oracle 136
  goldens, 1,924 facts, 0 failed; the last line `check.sh: long tier: ran (CERASTES_LONG=1)`, the six long tests'
  16 runs (both modes and `chain3`'s `native-ok`) all passed. It took 73 minutes (20:49–22:02 UTC); the box's load
  was 0.4 at the start and its 15-minute average about 100 a minute after the end (the cause not determined).

**First step for the next session:** M4.5d2b2d (`.claude/plans/m4-5d2b2d-invalidation-profile.md`): its Step 0 checks
that `origin/main` holds this branch's merge, and its survey refresh re-reads `lower.cpp`'s line numbers (this session
moved them).

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
