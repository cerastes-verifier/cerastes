# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`.

## M4.4c — Writes outside the chain, `void` `?:` statements, warnings in tests, `-Wall` shapes (2026-10-26): done on branch `m4.4c-writes-void-warnings`; PR pending

Read `internal/handoff.md` next: §2 the owner steps, §3 the open notes, §4.1 M4.4d, §4.2 M4.4e, §8 questions 43–46.

**Owner decisions (2026-10-26, planning and during the session)**
- **Planning:** one session with Q37 last and a cut order; the planning review's **finding A** fixed first; measured
  Viper changes allowed for writes and snapshots of call results; a flagged snapshot's `uninitialized` check stays in the
  chain; `x ? RED : NA` rejected with Q37; §11 questions 40–42 as recommended; the cleanup of `status.md`, `handoff.md`
  and `plan.md` at the end.
- **During the session:** audit round 1's further shapes (`-Wconstant-logical-operand`,
  `-Wtautological-constant-out-of-range-compare`, `-Wtautological-overlap-compare`, `-Wdangling-else`, `-Wempty-body`)
  added (R9–R13); audit round 2's five more listed, and a new session **M4.4e** sweeps every default and `-Wall`
  warning of clang and GCC.

**Done**
- **Decision 0025** (`0423c33`, revised after audit rounds 1 and 2). Amends 0011, 0014, 0019, 0020, 0021, 0023, 0024.
  Probes P46 (the IL of a `void` `?:`), P46b (macro body versus argument positions), P47 (finding A reproduced), P48 and
  P50 (compiler matrices, clang 21 and GCC 16, both modes, three tiers), P49 (the region walk), P50b (written and
  generated blocks, `else` positions, null statements) are in `il-inventory.md`, "M4.4c".
- **Finding A fixed:** an `uninitialized` check in a guarded operand is an access of its local, so a later write in the
  same expression follows it; `int x; int y = (x = (g(f(a) > 0 && x > 0), 2));` is an `uninitialized` error (it
  verified since M4.3a).
- **Writes and snapshots leave the chain** (Q28): `(x = f(a)) + (y = g(b))` verifies; fail-closed checks on the
  placement emission uses, with three VIR hooks (`perturb_write_slot`, `delay_write_slot`, `drop_flag_access`).
- **`void` `?:` in statement position is an `if`** (Q30): statements, discarded commas, `for` steps (a step may be an
  `if` of steps).
- **`verified` and `error` tests may list front-end warnings** (Q31), from a closed list (`assign_where_compare_meant`).
- **The static-assertion cross-check covers function bodies** (Q40), with the hook `drop_block_static_assertion`.
- **13 `-Wall` shape rules** (Q37 and the owner's additions): R1 `logical_operand_parentheses`, R2
  `shift_operand_parentheses`, R3 `logical_not_comparison`, R4 `chained_comparison`, R5 `self_comparison`, R6
  `truth_value_compare`, R7 `int_in_truth_context`, R8 two enumerations (`unsupported_expression`), R9
  `constant_logical_operand`, R10 `constant_out_of_range_compare`, R11 `overlapping_comparisons`, R12 `dangling_else`,
  R13 `empty_body`.
- **Code:** `src/vir` (`lower.cpp`, `lower.h`, `check.cpp`), `src/viper` (VIR cases), `src/backend` (`encode.cpp`,
  `il_query.*`, `diagnostics.def`, `verify_config.h`), `src/driver/verify_main.cpp`; **VIR self-test** 671 → 686
  lowering cases, 199 → 203 rejection cases (`VIR_CASES_MIN` 686).
- **Tests:** 215 new files in `tests/scalar`: 10 positives, 15 error twins, 4 rejection pins that stay (flagged snapshots, a call-free branch with a write, a `void` `?:` whose comma value is used, C++ `return c ? v() : w();`) and 186 `reject_warn_*` rejection tests (one per shape); 3 doc examples (`call_writes.c`, `void_conditional.c`, `warning_shapes.c`). Retired (pre-approved): `reject_unsequenced_assignments.c`, `reject_void_conditional_arms.c`,
  the VIR case `calls_rej_two_writes`; the four `reject_assert_shape_*.c` lost their `unsupported_type` line (a comment
  keeps the line numbers).
- **Harness:** `run_tests.py` (Q31, the warning allowlist), its self-tests, `tests/README.md`.
- **Docs:** Verifying a program (`void` `?:`, "Shapes the compilers warn about", calls and assignments, static
  assertions in bodies, the not-supported list), Integer semantics (two enumerations, truth-value comparisons),
  Overview; three doc examples; `internal/encoding.md`.

**Acceptance**
| # | Command | Result |
|---|---|---|
| 1 | `scripts/check.sh` #2 (16:37–17:19, after audit round 2's fixes) | **all green**: self-tests harness 981 (skipped 1), oracle 79, lint 190, mutate 291; tests 5386 passed, 0 failed, 1 skipped (native-ok: Apple clang 1019, GCC 1019); 1820 `.vpr` lint clean; VIR self-test 686/686 (+ 203 rejection cases); walker oracle 135 goldens, 1922 facts, 0 failed; docs 0 problems; `mutate.py --validate` 325 tests, 11241 mutants |
| 2 | `scripts/check.sh` #1 (15:50–16:23) | 5378 passed, 6 failed: R10 fired in three error tests (contracts on narrow results, a named constant), which the agents' sweep had skipped; R10 became code-only, with literals and enumerators only |
| 3 | `diff -rq -x MANIFEST build/m4.4c-baseline-vpr build/vpr`; the VIR self-test against `build/m4.4c-baseline-vir` | all 1768 `.vpr` and maps of the baseline identical, none missing, 104 new; the 671 old VIR cases identical |
| 4 | `run_tests.py --check-headers`, `--count-error-twins`; `build/cerastes --selftest` | 2027/2027 (with the mutation twin); 323; 94 checks |
| 5 | `mutate.py -j 8` over the 10 new positives and 2 doc examples (no external cap; logs `build/m4.4c/mutate1.log`–`mutate4.log`) | run 1: 1619 mutants, 979 killed, 186 safe-direction, 149 stillborn, 303 survivors, 2 tool failures (Silicon timeouts of a body `p \| q` against an exact `&` contract; that contract now uses `&&`); three files strengthened in place and re-run (0 tool failures), one new twin; **284 reviewed equivalents, 0 unexplained** (Opus: 26 reasons corrected, 6 proposed entries killed by strengthening instead); `mutate.py --validate` 325 tests, 11277 mutants, 863 entries |
| 6 | audits (Opus) | round 1 (record, P46–P48): 0 blocking, every should-fix taken; round 2 (code, tests): 0 blocking, 3 should-fix (2 fixed, 1 listed: M4.4e, owner); re-check: 0 blocking, 1 should-fix (three shapes, listed for M4.4e) |
| 7 | wall-clock | 14:36–21:30 (planning before), about 7 h against the planned 5 h: the mutation run and its review took about 4 h of it |

**Not done / deferred**
- M4.4d, M4.4e (new), the EDG box session (with `#if`), M5.0.
- The notes in handoff §3.

**Surprises**
- **Finding A**, a soundness hole since M4.3a, found by the planning review, not by a test.
- **Each audit found more compiler warnings** outside the list (round 1: five families, added; round 2: five, listed):
  hence M4.4e.
- **Byte identity held completely:** item 2 changed no existing program's Viper (π was already creation order for all
  of them).
- **The mutation run took about 4 h with its review** (planned 30 min): 1619 mutants over 12 files and 303 survivors,
  most of them equivalents by the design of the near-miss tests; two Silicon timeouts came from a body `p | q` against
  an exact `&` contract (0023's documented limit).
- **check.sh #1 found R10 in three error tests' contracts** that the agents' sweep had skipped (it left out `negative/`
  for `error` tests); R10 became code-only.

**Decision records added:** 0025-writes-void-conditional-warnings; amended 0011, 0014, 0019, 0020, 0021, 0023, 0024.

**First step for the next session (M4.4d).** Confirm that the owner merged M4.4c's PR and that CI is green on `main`.
Then handoff §4.1 (record 0026; old §11 question 27).

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
