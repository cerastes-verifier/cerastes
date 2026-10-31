# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, M4.5a's
`git show 72e30b3:internal/status.md`, S5's `git show 794e960:internal/status.md`, M4.5b1a's
`git show bdc6bd0:internal/status.md`, M4.5b1b's `git show 8003dca:internal/status.md`, M4.5b1c1's
`git show eab4058:internal/status.md`, M4.5b1c2's `git show a5e3e30:internal/status.md`, and M4.5b1c3's
`git show 5ffb1a2:internal/status.md`.

## M4.5b2a — mutable globals and `static` locals (2026-10-30): done on branch `m4.5b2a-globals`

Plan: `.claude/plans/m4-5b2a-globals.md`. Read `internal/handoff.md` next: §3 "From M4.5b2a", §4.2 (M4.5b2b), §8 (no
question open).

**Owner decisions (2026-10-30):** `const` objects are M4.6's (question 78); written atoms on moved pointers are M4.8's,
with question 66 (79); a `static` local's permission reaches its function by an implicit `owns` in its contract and in
every transitive caller's (80); this session's flips (81); for M4.5b2b, the call's flag row becomes `uninitialized` (82)
and strict mode's initialized-pointee rule is a static rejection (83); initial values by linkage (84). At planning, the
M4.5b2 row was split in two (sizing per question 77): this session (globals and `static` locals) and M4.5b2b (P4222's
markers and `--strict-init`, record 0042). PR #21 (M4.5b1c3) was still open, so the branch starts at `5ffb1a2`.

**Done**
- **Record 0041** (accepted; revised after P63's hazards H1–H8, audit round 1 on the design (two blocking items: B1 an
  internal C++ global written before `main` through an escaped pointer, B2 address comparisons the C++ front end folds;
  ten should-fix items and thirteen notes, all taken but N9, by design), S31, and audit round 2 on the code (no blocking
  item in about 230 probes; SF1–SF4 taken, so no re-audit)), amending 0017, 0025, 0028, 0032, 0037, 0038 and 0039: each
  mutable global and `static` local a block named by a `unique` `GB_` function in the `Block` domain; a trusted
  `program_start()` called first by the file-scope `main`, its first `ensures` the global facts, which are the first
  `requires` of every other method; initial values by linkage and language, escape through an initializer, C's
  tentative definitions unknown (`-fcommon`); initializers checked in every prologue, unknown ones included; both IL
  shapes of `&g`, folded casts as conversions, folded comparisons encoded from their original expression;
  `owns(g)`/`reads(g)`/`old(g)`/`decreases(g)` and `&g` as a value in contracts; atom sites keyed by place; the liveness
  disjunct; held globals as loop places; a key per global in the Sequencer and global atoms as memory calls (0025 item
  2's placement for globals); `static` locals by implicit atoms over the call graph, their declarations emitting
  nothing; the trust assumptions of a single unit stated.
- **Code:** `src/vir/lower.cpp` (commit `2466d3c`: atom sites by `site_key`/`site_ref`, no behaviour change, byte-identical
  on 2718 kept programs; then globals), `vir.h`, `vir.cpp`, `check.cpp`, `lowered.{h,cpp}`; `src/viper/emit.cpp`,
  `prelude.{h,cpp}`, `selftest.cpp`; `src/backend/encode.cpp` (mutable globals, address constants, initial values, uses,
  contracts, `static` locals and their propagation, `Program::entry`, the rejections), `il_query.{h,cpp}` (secondary
  variable declarations, `static` local initializers); `scripts/lint_viper.py` (`lint_globals`).
- **Tests:** `tests/memory/` 27 positives and 75 negatives (55 error twins, 20 rejections) for globals and `static`
  locals; the ports `tests/memory/verifast/global_points_to_syntax.c` (both functions) and `globals.c` (scalar part) with
  twins; S31's 17 fixtures `tests/viper-smoke/memory/s31_*.vpr` (the `e2_global_*` fixtures marked superseded); VIR
  self-test `selftest_cases12.inc` (32 cases; `VIR_CASES_MIN` 1386); lint self-test 335; two doc examples.
- **Docs:** Memory and ownership ("Global variables", "`static` local variables", the not-yet list), Annotations (the
  spec-function table), Error messages (`permission`, `unsequenced`), Verifying (the single-unit assumption's trust
  assumptions, the rejected list), Overview; `internal/encoding.md` §3 and the memory prelude; `il-inventory.md` P63 and
  S31.

**Flips (pre-approved, question 81):** retired `tests/scalar/negative/reject_global.c`,
`tests/scalar/negative/reject_static_local.c` and `tests/memory/negative/reject_pointer_global.c`, each replaced by a
positive of the same shape with a twin (`global_declared_unused.c`, `static_local_unused.c`,
`global_pointer_declared_unused.c`); `reject_address_of_const_global.c` and `reject_case_const_address_taken.cpp` keep
their `expect:` lines, their Pins naming M4.6, and `reject_owns_of_param_cell.c` its, naming no session. No other
existing test's `expect:` line changed.

**Acceptance**
- The baseline (`main` `774fde4`, the tree of `5ffb1a2`, in the main checkout): 7259 passed, 0 failed, 1 skipped; VIR
  self-test 1354.
- **Viper against the baseline:** the refactor's binary regenerated all 2718 kept `.vpr`/`.map` files byte-identical; the
  globals binary likewise, except three `main` tests that a too-strict new check rejected (an `ensures` on the entry),
  fixed and then byte-identical (end-to-end comparison, `build/m4.5b2a/vprdiff-e2e.txt`).
- The targeted run (`tests/memory`, `tests/scalar`, `tests/viper-smoke/memory`, four doc example directories): 6336
  passed, 52 failed, every failure in this session's new tests (a front-end warning, two twins that failed twice, two
  rejection categories) or the three `main` tests; all fixed and re-run green.
- `check.sh` (the full check, `CERASTES_CHECK_BASE` unset; twice: the first run stopped at the tests on four test files
  added after audit round 2, which EDG's #550 made fail; they read their globals in code now): **all green**. Harness
  self-test 1024 tests (1 skip), 79, lint self-test 335, 291; tests **7577 passed, 0 failed, 1 skipped** (native-ok runs:
  Apple clang 1448, GCC 1448); Viper lint 2878 files clean; VIR self-test **1386** lowering and 237 rejection cases, 0
  problems, all 1386 through Silicon; walker oracle 136 goldens, 1924 facts, 0 failed; docs green. `build/vpr` against
  the baseline: 320 added files, none changed or missing; the VIR self-test against the baseline: added cases only. No
  Linux run (CI covers it).

**Surprises**
- The front end folds C++ address comparisons (`&g + 1 == &h` to `false`), so a folded constant whose original holds an
  address is read from the original (audit round 1 B2); casts fold into address constants (P63 H3), and `&g` has two IL
  shapes (H1).
- Apple clang 21 passes `-fcommon` by default, so a C tentative definition with external linkage can merge with another
  file's initialized definition: its value is unknown (audit round 1 S1).
- EDG warns #550 on a global that only contracts read; the compilers do not (an M8 warning-sweep entry).
- A new entry rule rejected `ensures` on `main`, caught by the baseline comparison, not by a test of this session.

**Decision records added:** 0041-globals-and-static-locals.

**First step for the next session (M4.5b2b).** Confirm the owner merged PR #21 and this branch's PR with both CI jobs
green; then plan M4.5b2b from handoff §4.2 (P4222's markers, `--strict-init`, questions 82 and 83), saving the plan as
`.claude/plans/m4-5b2b-<topic>.md`.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| M4.5b2a Globals and `static` locals | 2026-10-30 | `m4.5b2a-globals` | 0041 | mutable globals and `static` locals as `unique` blocks, `program_start` for `main` with initial values by linkage, frames through `owns(g)`/`reads(g)`, implicit atoms for `static` locals; M4.5b2 split, the markers to M4.5b2b |
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
