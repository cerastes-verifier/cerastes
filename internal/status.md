# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, M4.5a's
`git show 72e30b3:internal/status.md`, S5's `git show 794e960:internal/status.md`, M4.5b1a's
`git show bdc6bd0:internal/status.md`, M4.5b1b's `git show 8003dca:internal/status.md`, M4.5b1c1's
`git show eab4058:internal/status.md`, M4.5b1c2's `git show a5e3e30:internal/status.md`, M4.5b1c3's
`git show 5ffb1a2:internal/status.md`, and M4.5b2a's `git show 38877b4:internal/status.md`.

## M4.5b2b — initialization through pointers (2026-11-02): done on branch `m4.5b2b-init-markers`

Plan: `.claude/plans/m4-5b2b-init-markers.md`. Read `internal/handoff.md` next: §2 (owner steps), §3 (notes by session),
§4.2 (M4.5c), §8 (no question open).

**Owner decisions (2026-11-02):** at planning, every call flag row becomes `uninitialized`, global and `static`-local
atoms included (question 85; three more self-test pins pre-approved), contract reads of a marked pointee are rejected
statically (86), an initialized pointee stays initialized after a `ref_to_uninit` call (87, amending 0028 item 14), a
marker needs an `owns(*p)` atom (88); during the session, after audit round 2, the loop row `held && flag ==> range` goes
on every loop place in every function (89). Step 1 consolidated the handoff (§3 regrouped by the session that consumes
each note) and `plan.md`. The old session worktree was gone, so the session ran in a new worktree
`.claude/worktrees/m4.5b2b` and took its baseline in a detached one, `.claude/worktrees/m4.5b2b-base` (`38877b4`).

**Done**
- **Record 0042** (accepted; revised after P64, audit round 1 on the design (no blocking item; S1–S7 and N1–N8 taken)
  and audit round 2 on the code (no blocking item, no unsound verdict; S1–S4 taken, S2 through question 89; so no
  re-audit)), amending 0019, 0028 item 14, 0029 item 2, 0037 items 7, 8 and 10, 0038 item 3, 0039 item 8, 0040 item 3 and
  0041's call rows: `[[cerastes::uninit]]` on automatic locals (emits nothing; `uninit_initialized`);
  `[[cerastes::ref_to_uninit]]` and `[[cerastes::must_init]]` on pointer parameters as `vir::Param::init` (read like
  `owning` over every declaration; the flag dropped from the `requires` atom with `flag ==> range`; `must_init`'s return
  row `postcondition`; `ref_to_uninit`'s `(g_r && old(flag)) ==> flag`; contract reads of the pointee rejected by
  spelling and never framed by a marked site; markers rejected on non-pointers, `void *`, `const` pointees, `noreturn`
  functions (`must_init`), `pure`/`predicate` functions, system redeclarations, without an `owns` atom or beside
  `reads`; uncalled marked declarations checked); every call flag row `uninitialized` with its own message;
  `--strict-init` (`uninit_unmarked`, `uninit_ref_initialized`, static by question 83; the shared scan
  `vir::assigned_params` now counts assignments inside stores); the loop row on every loop place (question 89), which also
  fixes 0038's exit 3 on nested loops; `owning` joins the protected macro names and the spec-function and
  system-redeclaration rejections.
- **Code:** `src/backend/encode.cpp` (`read_marker`, `check_marker_atoms`, `reject_marked_read`, the spec-function and
  system-redeclaration rejections, `uninit` in `declaration()`, `strict_init_arguments`), `register_attributes.cpp`,
  `annotation_il.h`, `annotation_checks.cpp`, `diagnostics.def` (three tags; `annotation_mismatch` names the attribute),
  `verify_config.h`; `src/driver/verify_main.cpp` (`--strict-init`); `src/vir/vir.{h,cpp}` (`InitMarker`, the scan),
  `check.cpp` (the backstop), `lower.cpp` (`site_marker` and the rows); `include/cerastes.h` (three macros);
  `scripts/run_tests.py` and `mutate.py` (the `// verify-options:` header, its near-miss check).
- **Tests:** `tests/memory` 21 positives and 82 negatives (31 error twins, 51 rejections, EDG's own refusals included);
  5 macro pins in `tests/annotations/negative`; S32's 11 fixtures (`tests/viper-smoke/memory/s32_*`; `s25_d_init.vpr`
  marked superseded); VIR self-test `selftest_cases13.inc` (28 cases; `VIR_CASES_MIN` 1414); harness self-tests for the
  header; 10 doc examples.
- **Docs:** Memory and ownership ("Initialization through pointers", the not-yet list), Annotations ("Initialization
  markers", "Declarations that disagree", the protected names), Error messages (`uninitialized` at a call,
  `postcondition` of `must_init`), Verifying (`--strict-init` and "The strict initialization mode", the marker, the trust
  note); `internal/encoding.md` §3 and §8; `il-inventory.md` P64 and S32 (G1 corrected).

**Flips (pre-approved, questions 82, 85 and 89):** the six VIR self-test pins `selftest_cases8.inc:73`,
`selftest_cases11.inc:137-138` and `selftest_cases12.inc:113, :206, :283`, re-expected with one fewer `permission` row per
atom (two at `:206`), same failing rows; question 89 needed no pin. No existing test's `expect:` line changed.

**Acceptance**
- The baseline (`38877b4`, detached worktree): 7577 passed, 0 failed, 1 skipped; VIR self-test 1386.
- **Viper against the baseline** (`build/m4.5b2b/mapdiff.py`; every kept program regenerated, and `build/vpr` after
  `check.sh`): of 2878 kept programs, 286 `.map` files differ only in call flag rows (586 rows, `permission` →
  `uninitialized`), 92 `.vpr` files gain only question 89's loop rows (130 lines), nothing else differs and nothing is
  missing; 116 programs added. The VIR self-test: 74 maps in call flag rows, 31 files in loop rows, 28 cases added.
- `check.sh` (the full check, `CERASTES_CHECK_BASE` unset; once): **all green**. Harness self-test 1044 tests (2 skips), 79,
  lint self-test 335, mutate self-test 292; tests **7923 passed, 0 failed, 1 skipped** (native-ok runs: Apple clang 1506,
  GCC 1506); Viper lint 2994 files clean; VIR self-test **1414** lowering and 246 rejection cases, 0 problems, all 1414
  through Silicon; walker oracle 136 goldens, 1924 facts, 0 failed; docs green. No Linux run (CI covers it).

**Surprises**
- Silicon goes on past a failed assertion with it assumed, stopping a path only when that makes it infeasible; a probe
  with a fresh cell (whose `!d` contradicted the assumed flag) suggested otherwise, and `il-inventory.md`'s G1 said so.
  The auditor's loop probe showed the difference; G1 is corrected.
- 0038's loops lost a reached place's `flag ==> range` across an inner loop, an exit 3 on a correct program since
  M4.5b1c1, found by audit round 2 and fixed by question 89.
- A marker inside a function-pointer type lands on the inner function type and reaches no function (inert until M4.7).

**Decision records added:** 0042-initialization-through-pointers.

**First step for the next session (M4.5c).** Confirm the owner merged this branch's PR with both CI jobs green; then
plan M4.5c from handoff §4.2 and §3 "M4.5c Structs" (field cells, `.` and `->`, struct values, `owns(*s)`, owning fields
`d`, and `owns(**pp)`/`live(*pp)` of question 74), saving the plan as `.claude/plans/m4-5c-<topic>.md`.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
