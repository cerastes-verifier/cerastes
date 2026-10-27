# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, and M4.4c's entry is
`git show fce46de:internal/status.md`.

## M4.4d — Exact sets for preconditions and commas; constant masks as remainders (2026-10-27): done on branch `m4.4d-exact-sets`

Read `internal/handoff.md` next: §2 the owner steps, §3 the open notes, §4.2 M4.4e, §8 the new questions.

**Owner decisions (2026-10-27, planning and during the session)**
- **Planning:** handoff answers 5, 10–12 and 43–48 recorded (Q11: typed locals in the mutation classifier before M5;
  Q48 deferred); **Q47:** the `bit_mask.c [c23]` hang fix (masks as remainders) folded into M4.4d.
- **During the session:** the M1 stop (the VIR case `hook_drop_flag_access` stopped firing, its program now taking step
  1): a self-test hook `force_base_sets` makes every full expression take step 2, and the case sets it.

**Done**
- **Decision 0026** (`cd2a4cd`, revised after audit rounds 1 and 2). Amends 0012, 0020, 0021, 0023, 0025. Measurements
  M1, the force-base run, M2, S22 and the hang runs are in `il-inventory.md`, "M4.4d".
- **Part A, exact sets in two steps:** a callee's `requires` has the set PRE(c), and a comma's value carries its floor;
  each full expression (and each branch's own) uses the exact family when it is a chain, else today's, else rejects
  with today's conflict. Dual sets in one lowering pass; `flush` projects onto the chosen half. `h((validate(y), y))`,
  `100 / (validate(y), y)`, `(nonzero(d), 1) && 100 / d` verify (the never-returning `(nr(0), 1) && …` in the VIR cases); `take(f(x), h(y))` with one `requires` per argument is
  accepted; `h(f(x), y) + 100 / g(z)` is accepted by the fallback.
- **Part B, constant masks as remainders:** `a & (2^k − 1)` is `a % 2^k`, `a & ~(2^k − 1)` is `2^k * (a \ 2^k)` (audit
  round 2: the first form `a − a % 2^k` doubled at each nesting level), the constant recognised by exactly evaluating
  the operand as lowered (`INT_MIN`, `UINT_MAX`, `~0u` in its own type); `%` and `\` only through `low::mod_pow2` /
  `div_pow2`, and linted.
- **Fail closed:** every base half inside its exact half (internal error; hook `drop_exact_set`); hook `force_base_sets`
  (lowering and driver).
- **Code:** `src/vir` (`lower.cpp`, `lower.h`, `lowered.h`, `lowered.cpp`), `src/viper/emit.cpp`, `src/backend/encode.cpp`
  and `src/driver/verify_main.cpp` (the hook); **VIR self-test** 686 → 835 lowering cases, 203 → 206 rejection and hook
  cases (`VIR_CASES_MIN` 835).
- **Tests:** 15 positives and 39 twins in `tests/scalar` (6 part A, 9 masks), among them 7 pins against sets that are too
  large and 2 cross-type mask pins; 2 doc examples (`verifying/comma_order.c`, `integer-semantics/mask_remainder.c`).
  Retired (pre-approved): `reject_unsequenced_split_requires.c`.
- **Tooling:** `lint_viper.py` refuses a `%` or `\` outside the prelude functions that use them unless the divisor is a
  power-of-two literal; harness tests for the `force_base_sets` hook and for the rewrite firing.
- **Docs:** Verifying a program ("Calls in expressions": the exact sets, one `requires` per argument, the fallback and
  its false alarms), Integer semantics (constant masks decided); `internal/encoding.md` §2, §3 and the hooks.

**Acceptance**
| # | Command | Result |
|---|---|---|
| 1 | `scripts/check.sh` #2 (11:41–12:22) | **all green**: harness 990 (skipped 1), oracle 79, lint 202, mutate 291; tests 5566 passed, 0 failed, 1 skipped (native-ok: Apple clang 1053, GCC 1053); 1934 `.vpr` lint clean; VIR self-test 835/835 (+ 206 rejection and hook cases); walker oracle 135 goldens, 1922 facts, 0 failed; docs 0 problems |
| 2 | `scripts/check.sh` #1 (11:36–11:40) | stopped at the harness self-test: the new `force_base_sets` test expected exit 1, but a hooked run that reports errors exits 3 (0011 item 13); the test was corrected |
| 3 | After audit round 2's fixes (complement form, `c_pow2` bound, lint of `\`, harness tests): deltas | the 68 mask sources 240 passed; every changed line exactly `(X − X % P)` → `(P * (X \ P))`; S22 for the new form 591 asserts and Z3 for k = 0..128; 20 more one-verifier hang runs, none hung; 14 nested complement masks about 1 KB (was 284 KB); 124 VIR mask `.vpr` passed; lint clean on all; `test_lint_viper.py` 215; the two new driver tests pass |
| 4 | M1, the force-base run, M2 (`il-inventory.md`) | M1: only the retired test flipped; 12 `.vpr` changed (6 tests), each explained; 0 programs accepted by the fallback, 27 fallback rejections; force-base: 1780 of 1822 `.vpr` identical, the 42 others identical outside the masks; M2: 42 `.vpr` changed, all mask programs, 0 verdict changes |
| 5 | The hang (`build/m4.4d/hang/`, `hang2/`) | `bit_mask.c`, `bit_mask_post.c` [c23], `--numberOfParallelVerifiers 1`: 72 runs, 0 hangs, 1.4–1.6 s (before: 10 of 10 hung) |
| 6 | `mutate.py -j 8` over the 6 part-A positives and `verifying/comma_order.c` (no external cap; logs `build/m4.4d/mutate1.log`–`mutate4.log`) | run 1: 154 mutants, 96 killed, 19 safe-direction, 39 survivors, 0 tool failures; five files strengthened in place (validate's unused result dropped, callees made to need their bounds, values pinned, the guard test's call made to return on some inputs so the code after it is reachable, and the operands of `&&` made to matter); final: 160 mutants plus the 36 of `call_comma_branch.c`'s rerun, **0 unexplained**, 14 reviewed equivalents (Opus: 1 proposed entry killed by strengthening instead); `mutate.py --validate` 340 tests, 11732 mutants, 877 entries. The mask positives were not mutated (cut 1) |
| 7 | audits (Opus) | round 1 (record, prototype): 0 blocking, every should-fix taken; round 2 (code, tests): 0 blocking, 3 should-fix taken; re-audit: 0 blocking, 1 should-fix (the new form's evidence, now in `il-inventory.md` and 0026 items 8 and 10); the equivalents review: 0 blocking, 2 should-fix (both taken) |
| 8 | `test_run_tests.py` (991, skipped 1), `test_lint_viper.py` (215); the 56 new test files on the final tree (184 passed, native-ok 34/34); `--check-headers` 2082/2082, `--count-error-twins` 338; `build/cerastes --selftest` 94 | all pass |
| 9 | wall-clock | 09:07–13:45 (planning before), about 4 h 40 min against the planned 5 h |

**Not done / deferred**
- Mutation of the mask positives (cut 1 of the plan; to M4.8's run over the M4 tests).
- M4.4e, the EDG box session (with `#if`), M5.0; the notes in handoff §3.

**Surprises**
- **M1's one stop:** an old hook case lost its reach, because exact sets already order the shape the guard protects in
  step 1; the guard matters only for step 2, now reached by `force_base_sets`.
- **No existing test moves a precondition** (no `f(a, g(b))` with a `requires` on `a` only) and none is accepted by the
  fallback: the visible changes are two comma shapes (12 `.vpr`).
- **Viper's `%` was never used with a negative dividend before;** S22 and Silicon's own sources settle it (SMT-LIB `mod`).
- **Nested complement masks doubled the emitted text** (audit round 2): replaced by `2^k * (a \ 2^k)` before the
  commit.
- **The mask rule decides only masks written as constants:** a mask that becomes constant through a call argument still
  reaches Silicon's own precondition check as `c_uand_N` (a timeout, exit 3; audit round 2).

**Decision records added:** 0026-exact-sets-and-masks; amended 0012, 0020, 0021, 0023, 0025.

**First step for the next session (M4.4e).** Confirm that the owner merged M4.4c's PR (#11) and this branch's PR, and
that CI is green on `main`. Then handoff §4.2 (record 0027).

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
