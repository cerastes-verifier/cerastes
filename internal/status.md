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
`git show 19d977a:internal/status.md`, M4.5d2b2b's `git show e3f4b9f:internal/status.md`, and M4.5d2b2c's
`git show 023ea50:internal/status.md`.

## M4.5d2b2d — `not_invalidating`, `--strict-owner`, marked fields (2026-11-10): done on branch `m4.5d2b2d-invalidation-profile`

Plan: `.claude/plans/m4-5d2b2d-invalidation-profile.md`. Record 0058 (amends 0029 items 1–2, 0042 items 4 and 8, 0052
item 4, 0054 item 2, 0055 items 3 and 11). Read `internal/handoff.md` next: §1, §2, §3 "M4.8", §8 (questions 224 on).

**Owner decisions in the session** (2026-11-10), each the recommendation, each recorded in record 0058:
- **218** N1's swap may change the Viper of the other programs whose bodiless contracts hold both asserts;
- **219** a third N1 flip, `owning_global_call_dangling_closure.c` (it corrected 218's premise);
- **220** fix the return-row order now, with three flips (audit round 2 on Parts S and F, B1);
- **221** keep the tied-only Kept row, and its amendment of 0054 item 2;
- **222** the re-audit's older exit 3 goes to M4.8;
- **223** three VIR cases' `fail_only` index follows 220's reorder (it corrected 220);
- and 215, 216 (M4.8), answered in the handoff before the session.

**Told to the owner and corrected later** (each in §8 of the handoff): 219 said no VIR case was affected by N1, one was
(its asserts swapped, still verified; the orchestrator's grep was misread); 220 said `@ L58` went x2→x1, the file held
three (x3→x2); 220 said `q7_alias.c` would give one user error, it gives two (its `leak`, printed before, and the
`permission`); 223 says the existing VIR cases had not been through Silicon before 220 was asked.

**Step 0 and Step 1.**
- `main` and `origin/main` held M4.5d2b2c (`023ea50`, merged and pushed; handoff §1's "not merged" was stale); the
  branch is from it. The survey refresh held: `encode.cpp`, `vir.h` and `vir.cpp` unchanged since `262edb3`, `lower.cpp`'s
  functions moved uniformly.
- CI: the 2026-11-10 nightly, the first under the 360-minute limit, was cancelled at 6 h 01 min; no `profiles=all`
  dispatch has run (`macos-26` stays at `dispatch`, question 172). No CI commit.
- Baseline: 5,144 `.vpr`/`.map` and the VIR self-test's 1,732 cases from bootstrap's build, with frozen base binaries.

**Done** (the three Sonnet roles and the Opus reviewer from `.claude/agents/`; edg-backend and the il-analyst were each
replaced by a fresh agent when their context ran low; the orchestrator consulted the Fable advisor before the record,
each owner question, the box run and the close):
- **Behaviour-neutral preparation** (`55bd461`, `bfa0537`): the marker plumbing names each kind (an unknown one is an
  internal error); `gives_result_block`, `param_consumes_block`. vprdiff: all identical.
- **Record, P77, S42** (`0c06e56`): P77 (`not_invalidating` on parameters, `ref_to_uninit` on fields, `no_dangling`'s
  #1097; a registered name missing from `kNames` is silently ignored); S42's 33 fixtures (the draft's implicit clause,
  tied under the site's own null test, let a free-and-null body verify; the framing atom's guards hold in every shape).
- **N1** (`09d75e6`): the bodiless tie above the partner-guard row; three flips.
- **Part N** (`14a4cd3`, `8bc0960`): `cerastes::not_invalidating`, its partner check, the spliced `p->q == old(p->q)`,
  the `f__wf` row taken-implies-returned, the postcondition message; the library-header pin.
- **Part S** (`28bf393`): `--strict-owner`, p4 and p6 (`owner_result_unowned`, `owner_release_unowned`); with the
  option on, the 4,555 programs it accepts have byte-identical Viper.
- **Docs N and S** (`3d03e46`).
- **Part F with audit rounds 1 and 2's fixes** (`9d41e43`): `ref_to_uninit` on fields, the strict-init rule per leaf, the
  marked closure Kept only where tied (round 1's B1: 0054 item 5's internal row exited 3 when a body replaced the block),
  S2, S3, S5 (marked variables close nothing), F2 (question 202's three rejections gone, four pins retired for
  positives), S1's no-framer rejection (an exit 3), N2–N4.
- **Round 2 on Parts S and F** (`714796f` the return-row order, B1, question 220; `f0fd5b9` S2, N2, N3, N4 and 0054's
  docs; `5293eb1` S1, a tie counted by position), the re-audit's notes and question 222 (`133665f`), a lint false
  positive in the session's own rule (`67d901a`), question 223 and the member-function pins (`3a2a042`).
- **Tests.** 171 new tests in `tests/memory` (four retired pins deleted, seven expectations flipped: N1's three, B1's three
  of which two in `tests/long`, and question 223's three VIR indexes), 40 S42 fixtures, 34 doc examples; VIR self-test
  1,765 cases (was 1,732); `test_lint_viper.py` 736 (was 717; two rules); `test_run_tests.py` 1,221 (was 1,216).
  `src/` and `include/`: +1,962, −173 (the plan's 740–1,170 without self-tests).
- **Docs:** Annotations (`not_invalidating`, marked members), Verifying (`--strict-owner`, strict-init's marked fields),
  Memory and ownership (the P4417R0 table, marked owning fields and pointees, marked owning globals, the not-yet list),
  Error messages, the tutorial's heap chapter, the developer guide's architecture page, `tests/README.md`,
  `include/cerastes.h`, `internal/encoding.md`, `internal/il-inventory.md`.
- **Audits** (one Opus reviewer, resumed per round): round 1 on the draft (B1, S1–S9), round 2 on Part N (no blocking
  item), round 2 on Parts S and F (B1, S1, S2), the re-audit (no blocking item; its S1 is question 222). No wrong verdict
  was found in any round; every blocking item was an exit 3.

**Corrections of new tests after their first run** (question 160): `marked_pointee_loop_call_false_alarm.c`'s two
back-edge errors at the loop body's closing brace (L31, not L28); eight new tests used a set-but-unused `y` (EDG's #550
in C++20), fixed in their source, outcomes and lines unchanged.

**Not done.** The older exit 3 of question 222 (M4.8); the pinned loop false alarms of record 0058 item 4 ("Limits");
three marked-closure lint shapes the Viper cannot show (question 225). No cut was taken.

**Surprises.**
- **Exit 3s, not wrong verdicts.** Every blocking audit item was an exit 3, and two predated the session (0054 item 5's
  kept row; the return rows' order), made reachable by what the session accepted.
- **A registered attribute name is silently ignored** until `kNames` has it (P77), so the registration and the reader
  land together.
- **The model does not prove a fresh block differs from a freed one**, so no row can keep flags across a possible
  replacement (round 1's B1): only a tied site keeps them.
- **Scope.** `src/` grew by about 1,960 lines with the self-tests (the plan's 740–1,170 without): the audits' fixes and
  S5 (about 45 lines) added to it.

**Checks** (all on the final source tree, `3a2a042`):
- vprdiff of the final regeneration against the session's baseline: 384 entries differ, all allowed (N1's eleven
  programs, the 227 the reorder permuted), 0 unexpected; 31 existing VIR cases permuted, 28 unchanged in outcome and three
  re-indexed (223).
- The Mac (`5293eb1`, then the last commits' files singly): 733 runs of the session's 255 tests and doc examples passed
  with the N1 neighbours, `--long` 4, check-headers 4,884, `test_run_tests.py` 1,221, regeneration 5,412 files and the VIR
  self-test 1,765 cases lint clean, the session's 33 VIR cases through Silicon, format, docs.
- The box, before `check.sh`: the session's 391 new, changed or permuted programs timed in both modes (774 runs, the
  slowest 84 s, exit codes 0–2 only), so none moved to `tests/long/`.
- `CERASTES_LONG=1 scripts/check.sh` on the box: BOX_RESULT.

**First step for the next session:** merge and push (handoff §2), then M4.5e1's planning session (question 224).

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
