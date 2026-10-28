# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, and M4.4d's `git show 9017105:internal/status.md`.

## M4.5a — The memory model for C by design; `|` and `^` with constant masks (2026-10-27): done on branch `m4.5a-memory-design`

Read `internal/handoff.md` next: §2 the owner steps, §3 the open notes ("From M4.5a"), §4.4 S5 and M4.5b1–f3, §8 the new
questions (53–58).

**Owner decisions (2026-10-27, planning and during the session)**
- **Planning:** M4.4e (the warning sweep) deferred to M8; Q49–Q51 as recommended; **Q52** folded into M4.5a (record
  0027); `[[cerastes::owns]]` as an **owning pointer** (renamed `[[cerastes::owning]]` the same day, "instead of a pointer
  as owns make it owning"); **vlibc** as contract headers, verified implementations, explicit `<vlibc/…>` and a
  transparent `<stdlib.h>` shim ("combine all 3"); P4222's markers in the `cerastes::` spelling, opt-in strict mode.
- **During the session:** **S5 Linux CI** (`ubuntu-24.04` x86-64) right after M4.5a, before M4.5b1; MSVC and arm64 Linux
  to M8. After S24: encoding **E2**. After S25: `insufficient.permission` accepted on marked range rows only. After audit
  round 2: **member blocks** (bounds per sub-object) and a **liveness token** with `live(p)`. After S26: a fourth axiom,
  `member_lv`.

**Done**
- **Record 0027** (amends 0023 item 2, 0026 items 7–10): `|` with a constant mask inline (`%`/`\` forms), `^` as prelude
  functions `c_[u]xor_[c]mask_N_k` with a range-guarded postcondition (S23, S23b: an inline `let` timed out on nested
  chains); range claims through chains decided at every depth measured, identities to measured depths (item 4).
  **Code:** `src/vir/lower.cpp` (`mask_rewrite`), `src/viper/prelude.{cpp,h}`; lint exemption for the mask functions.
- **Records 0028–0030** (design only), each audited: 0028 the memory model (E2: a pointer as a `Ref` over a `Block`
  domain with four axioms `loc_inv`, `loc_nonnull`, `member_inv`, `member_lv`; one field per C type; `WF` clauses; member
  blocks; liveness at slot −2; the heap token at slot −1 and `owns_block`; leaks by `forperm`; `reads` by ghost
  fractions; explicit checks and marked range rows; `void*` restricted; sequencing on cells and calls; eight new
  categories), 0029 the annotations (`[[cerastes::owning]]`, `cerastes::uninit`/`ref_to_uninit`/`must_init` with
  `--strict-init`, C's `cerastes_owns`/`reads`/`owns_range`/`owns_block`/`live`/`old`/`forall`), 0030 vlibc. No EDG fork
  commit needed (P52–P54).
- **Fixtures:** `tests/viper-smoke/memory/`, 55 hand-written Viper files of the chosen encoding (24 must-verify, 31
  must-fail), run by `check.sh`.
- **Tests:** 11 positives and 35 twins `tests/scalar/[negative/]mask_or_xor_*`, the doc example
  `integer-semantics/mask_or_xor.c`; VIR cases 835 → 1123 (`VIR_CASES_MIN` 1123); harness and lint tests.
- **Docs:** Integer semantics (`|`, `^`, chains); `internal/encoding.md` §2. Nothing user-visible about memory yet.
- **Plan:** `plan.md` (M4 table, S5, M4.5b1–f3, M8 entries); handoff §4.4 with each session's plan and the VeriFast
  port inventory's conclusions.

**Acceptance**
| # | Command | Result |
|---|---|---|
| 1 | `scripts/check.sh` #1 | **all green** (19:12–20:23): harness self-tests 999 + 79 + 228 + 291 OK; tests 5763 passed, 0 failed, 1 skipped (`native-ok` Apple clang 1077, GCC 1077); Viper lint 2028 files clean; VIR self-test 1123 passed, 206 rejection cases, 0 problems; walker oracle 135 goldens, 1922 facts, 0 failed; docs 161 code blocks, 0 problems. `DriverCategoryDocsTests` rerun after the 0011 footer: OK. No second run |
| 2 | Baseline (`run_tests.py -j 8`, before any build) | 5566 passed, 0 failed, 1 skipped; 1934 `.vpr` |
| 3 | M3 (`build/m4.5a/M3.md`) | 198 sources rerun: 587 passed; 30 `.vpr` changed, each an `\|`/`^` constant mask; VIR 35 of 835 old cases changed, each explained; 0 verdict changes; no `let` |
| 4 | S23, S23b | 4728 Silicon asserts and 1804 Z3 bit-vector proofs: every form equals the prelude function; chains of 8/16/32 in about 2 s as functions |
| 5 | The new tests (both modes, `native-ok` clang and GCC) | 146 runs passed; VIR 1123/1123; `--check-headers` 2184/2184; `--count-error-twins` 349 |
| 6 | `run_tests.py tests/viper-smoke/memory` | 55 passed; the four axioms' 16 subsets each give one error in the consistency method; vacuity 139 points all reachable |
| 7 | Audits (Opus) | round 2 (0028): 5 blocking, 8 should-fix; round 3 (re-audit, 0029, 0030): 3 blocking, 14 should-fix; round 4 (re-audit, 0027's code): 1 reopened (C's implicit pointer conversions, fixed in the text, not re-audited: M4.5b1), 0 blocking on the code, 4 should-fix; all taken |
| 8 | wall-clock | 13:55–20:30 (planning before), against the planned 5 h: the audit rounds and the chain thresholds took the rest |

**Not done / deferred**
- Mutation of the mask positives (handoff §8 question 49: M4.8's run).
- The driver and encoder changes the records ask of M4.5b1 (the `--diag_error` for C's incompatible pointer
  conversions, the axiom records and lint tags) and of M4.5d1 (the attribute allow-list by position).

**Surprises**
- **Nested `^` chains:** exact `let` forms timed out at depth 8 because Z3 had to prove the whole term's range;
  functions with a proved range postcondition fixed ranges, but identities through signed chains still hit Z3 heuristics,
  sensitive to the postcondition's shape (C's `int`-valued `==` against C++'s `Bool`).
- **Four audit rounds on the memory model** found real holes each time: bounds per sub-object, two calls ordered against
  each other, dangling pointers, an object and its first member through `void*`, then C's implicit pointer conversions.
- **Silicon details** the encoder must respect: Viper refuses `perm` under a quantifier (range checks fail with
  `insufficient.permission`), refuses arithmetic in triggers, needs `a != b` at entry and an `lvloc` case split before
  fractions so it never reports a cascade.

**Decision records added:** 0027-or-xor-constant-masks, 0028-memory-model, 0029-ownership-annotations, 0030-vlibc.

**First step for the next session (S5 Linux CI).** Confirm that the owner merged PR #12 and this branch's PR and that CI
is green on `main`; check that GCC 16 and clang 21 can be installed on `ubuntu-24.04` (handoff §8 question 57). Then
handoff §4.4, "S5 Linux CI".

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
