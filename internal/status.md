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
`git show c1b691d:internal/status.md`, and S8's `git show 15bcdc6:internal/status.md`.

## M4.5d2b2a — block sites through memory, owners in memory, block carriers in loops (2026-11-09): done on branch `m4.5d2b2a-owners-in-memory`

Plan: `.claude/plans/m4-5d2b2a-owners-in-memory.md`. Record 0054 (amends 0029 item 1, 0041 items 7, 10 and 12, 0045
items 2, 7 and 10, 0047 items 4, 7 and 10, 0052 items 2, 3, 4, 6(a) and 8). Read `internal/handoff.md` next: §1, §2, §3
"From M4.5d2b2a", §4.2 (d2b2b) and §8 (questions 188 on).

**Owner decisions** (2026-11-09).
- At planning: 173 (d2b2 split; d2b2b takes owning fields and P4418R0), 174 (static owners exempt at `main`'s end,
  P4417R0), 175 (`owning` on a parameter stays a borrow), 176 (the nightly CI timeout is the owner's), 177
  (`--strict-owner`), 178 (`cerastes::not_invalidating`, checked), 179 (no `no_dangling` in C). 177–179 are d2b2b's.
- During the session: 180 (the alias pin's positive adds an invariant), 181 (the flag after a handoff: tried, and it
  verifies), 182 (the test-mutator's eight pre-registered readings), 183, 185 and 187 (consequential extra lines
  listed; 185 not the recommendation), 184 (two VIR cases' outcomes as part of the flips), 186 (a cheaper twin, the
  cost recorded).

**Done** (all three Sonnet roles and the Opus reviewer, from `.claude/agents/`):
- **Step 1 and the record.**
  - `e0d9816`: the planning commit.
  - `491d00c` and `5434b8e`: record 0054, written before code. It was revised after spike S39 (60 kept fixtures,
    `tests/viper-smoke/memory/s39_*`), audit round 1 (B1–B3) and its re-audit (S1, S2).
  - P75 in `il-inventory.md`.
- **Part A, block sites through loads and on globals** (`owns_block(*pp)`, `(p->q)`, `(g)`, `(*gp)`, `r->q`, by-value
  leaves):
  - a neutral re-keying first (`block_key` gone; every consumer on `site_key`/`site_root`), proved byte-identical;
  - whole-key duplicate and "beside" rules;
  - post-state reads in the leak exemption and the returned owners;
  - entry and loop snapshots;
  - the bodiless tie with the partner's guard (audit B1).
- **Part B, loop carriers by current state** (the entry snapshot kept as the exclusivity clause; sharing pairs; value
  sets; held-guarded owners the loop does not name; the flag that follows the block): the four pinned d2b1 false
  alarms are lifted. Written `owns_block` invariants (a plain whole-block carrier, pair rows at entry and back edge, a
  back-edge `forperm` whenever a loop has a site).
- **Part C, owners in memory:**
  - owning and marked globals and `static` locals: the deep atom `owns(g) && (g == NULL || owns_block(g))`, `held_g`,
    store rows, framed updates, `main`'s exemption, the `program_start` tag, synthetic loop sites;
  - owning address-taken locals (no cut taken).
- **Part D:** the static struct leaf cover.
- **Tests.**
  - 193 new test and example files: positives with twins, the audits' probes, pins for every still-rejected shape.
  - 12 retired pins (the plan's Flips).
  - 4 pins re-messaged.
  - VIR self-test 1669 cases (was 1642).
  - Lint rules for every new Viper shape (`test_lint_viper.py` 539).
- **Docs:**
  - Memory and ownership: blocks through memory, written invariants, owners that change hands in loops, owning
    globals, the not-yet list;
  - Annotations, Error messages, Verifying;
  - the tutorial's heap chapter (an owner that outlives a call);
  - `internal/encoding.md`.
- **Audits** (read-only Opus): round 1 on the draft, then its re-audit; round 2 per part on snapshots (`f9ee68e` A,
  `18cf5b5` B, `81e46a0` C/D); one re-audit of round 2's blocking fixes (`1a1d172`). No wrong verdict survived.
  - Round 2 Part A: B1 bodiless path partners, and rules without tests.
  - Round 2 Part B: B1, a written site that drops a block without a leak row.
  - Round 2 C/D: B1, six VIR cases; B2, loops over owners in memory untested; S3–S5, three exit-3 paths. All fixed.

**Checks**
- `check.sh` (full) on `b6b3298`: **not completed in the session.** The Mac's run passed its build, the Viper lint
  (4668 files clean) and the VIR self-test (1669 lowering cases, 270 rejection cases, 0 problems). It was stopped in
  its main suite when the owner moved the end-of-session `check.sh` to the x86-64 box (2026-11-09: "much faster, and
  doesn't lock up the local machine"). The box run, in the `ubuntu-26.04` container at `CERASTES_JOBS=32`, started
  2026-11-10 00:13 UTC. It was still running when the owner ended the session. Its result is in
  `~/cerastes-s8/p-ubuntu-26.04/d2b2a-check.{exit,log}` (handoff §2).
- The x86-64 box (`ubuntu-26.04` profile, S8's container `cerastes-s8-ubuntu-26.04`, branch shipped as a git bundle,
  clean `build/cmake`): `run_tests.py -j 16 tests/memory tests/vlibc doc/modules/ROOT/examples` at `b6b3298` gives
  6026 passed, 0 failed, 1 skipped (native-ok clang 1237, GCC 1237).
- Targeted runs before the one `check.sh` (owner, 2026-11-09: run it once, at the end): every owner, block-site, loop
  and related doc test after the last patch, 1259 passed, 0 failed. The first `check.sh`, stopped once a patch was
  still to come, had passed its main suite on the pre-patch tree (11779 passed, 0 failed).
- vprdiff against the baseline (`15bcdc6`, 11080 passed): every existing program's Viper is byte-identical except
  the ten allow-listed owner-loop tests of record item 3 (`build/m4.5d2b2a/allow-partB.txt`), with the same outcomes.

**Not done:** owning fields and P4418R0 (d2b2b).

**Surprises.**
- **The carriers.** A guard on current values alone loses two blocks' distinctness, as 0052 had recorded. The audit
  predicted the regression and S39 part 2 confirmed it, so the hybrid design keeps the entry snapshot.
- **A lost hand edit.** A test generator re-run wiped three hand-added expect lines (question 183). The audit caught
  it, and handoff §6's rule was restated to the agent.
- **Cost.** Two written sites through loads over parameter places cost 33–148 s (question 186).
- **CI.** The nightly CI has timed out at 220 minutes since 2026-10-29 (question 176). The owner raised the limit
  to 360 minutes (`99b587e`); the guard's bound and record 0016 item 9 follow it.

**First step for the next session:** M4.5d2b2b's planning (owning fields, P4418R0 per questions 175, 177–179;
handoff §3 "M4.5d2b2b" and "From M4.5d2b2a").

*Planned on 2026-11-09, in a session that only planned: plan `.claude/plans/m4-5d2b2b-owning-fields.md`, owner
questions 188–193. M4.5d2b2 is three sessions now: d2b2b is owning fields, d2b2c `not_invalidating` and
`--strict-owner`. The same session consolidated `internal/handoff.md` and `plan.md` (the earlier text is
`git show 2430529`): the handoff's §3 "From M4.5d2b2a" is now in the plan's Inputs and under M4.8, Standing and §6.
**The first step is the plan's Step 0** (read the box's `check.sh` result).*

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
