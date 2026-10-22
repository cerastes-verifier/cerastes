# Status

Session handoff log, newest first. Each entry says what is done, what is not,
any surprises, the decision records added and the exact first step for the
next session.

## M4.2b1 — Named constants and case values (2026-10-21): done on branch `m4.2b1-named-constants`; PR pending

Read `internal/handoff.md` next: the "M4.2b1 is done" owner steps at the top, §6 the M4.2b1 audit notes, §8.1 M4.2b2,
§11 questions 13–15.

**Owner decisions (2026-10-21, M4.2b planning)**
- M4.2b is split: **M4.2b1** (this session: named constants, case values; record 0017) and **M4.2b2** (enumerations;
  the next free record, 0018; M4.2c then 0019). The enum design from the planning review is in handoff §8.1.
- Named constants and case values are checked in a **prologue at the entry of every method** (not a separate
  `globals__wf` method); a failing initializer hides the unit's other errors (accepted).
- For M4.2b2: an out-of-range conversion to an enum with a fixed unsigned base is a `conversion` error. **Relook
  unsigned wrapping**: integer semantics should model real integers, not how C and C++ support them (handoff §11
  question 13; memory).
- No spike of EDG's source sequence lists in M4.2b (§11 question 4 answered).

**Done**
- **Decision 0017-named-constants**; amends 0010 items 12–13, 0011 items 7 and 13, 0012 items 4 and 12, 0014 items 5–6.
  Probes P23–P24 and spikes S14–S15 in `il-inventory.md` ("M4.2b1"); il-inventory L1084 corrected (an enumeration is
  `tk_integer` with `enum_type`).
- **Named constants:** a file-scope `const` (C++ also `constexpr`), constant-initialized (`initk_static`), in a verified
  file, a value type, not `thread_local`, not weak, only allow-listed attributes; C's `static const int z;` is 0. A use
  is EDG's value, in code, contracts, `check`, invariants, measures and (C++) case values; each initializer is encoded
  as written and checked in every method's prologue (obligations at their lines, then an `internal` guard against
  EDG's value). Rejected: mutable globals (`reject_global.c` unchanged), `extern const` without a definition, C
  `const int z;` (common symbol), weak constants and weak functions, dynamic initialization, `thread_local`,
  `volatile`, other attributes, globals from system files (at the use).
- **Case values:** checked in the prologue of the function's method and of its `__wf` method (path-independent; an
  `if (0) switch` or an unsatisfiable `requires` no longer hides a bad case value); the dispatch compares with EDG's
  value. C++ `case k`/`case N` (a named constant, or a folded `const`/`constexpr` local, transitively) accepted;
  `reject_case_variable.cpp` retired (pre-approved).
- **C:** `--diag_error=1901` makes a `const` object in a C constant expression (case label, `_Static_assert`,
  file-scope initializer) a front-end error (strict ISO C, §11 question 14); the encoder also rejects case values and
  constant initializers naming an object.
- **Pragmas** (re-audit, blocking; predates this session): `#pragma clang attribute` applied attributes EDG never
  records (`loader_uninitialized`, `weak`) with only warning #161. Verify mode makes #161 and #1675 (unrecognized
  pragmas) errors and rejects EDG's own diagnostic pragmas in verified files (`diagnostic_pragma_in_verify`), which
  could turn those errors back into warnings.
- **`do … while (c)`** is a `DoOnce` only when `c` names no variable (`names_object`): one meaning in both modes.
- **Hook** `perturb_folded` (exit 3 through the internal guards), with harness self-tests.
- **Code:** `src/backend/encode.cpp` (`encode_named_constants`, `variable()`, `local_const_use`, `names_object`, case
  values, weak functions, the hook), `src/driver/verify_main.cpp` (#1901, the hook), `src/vir` (`NamedConst`,
  `Case::folded`, `check_program` rules, the prologue, `closed_expr`), `src/backend/register_attributes.cpp` (the
  diagnostic-pragma recorder); VIR self-test 230 → 246 lowering cases (87 → 93 rejection cases).
- **Tests:** 60 new files in `tests/scalar` (8 positives, 22 error twins, 30 rejection tests; both modes where both
  accept the code), 2 doc examples, harness self-tests for `perturb_folded`; `reject_case_variable.cpp` retired. The
  positives were strengthened in place after the first mutation run (callers at the boundaries, exact `ensures`, no
  dead code).
- **Docs:** Verifying a program (Named constants with two examples, switch, `do`, the single-unit assumption, the
  rejected list), Integer semantics (constants), Annotations (constants in contracts, `do … while (0)`), Overview.

**Acceptance**
- `scripts/check.sh` green at `59d6103` (13 min 46 s, one run): self-tests 954 (skipped=1) + 74 + 152 + 225; tests
  2819 passed, 0 failed, 1 skipped (native-ok: Apple clang 659, GCC 659); 786 `.vpr` lint clean; VIR self-test 246/246
  (+ 93 rejection cases); walker oracle 134 goldens, 0 failed; docs 0 problems; `mutate.py --validate` 149 tests,
  4046 mutants, 436 equivalents, 748 in the bucket.
- Emitted Viper of every pre-existing test (733 `.vpr`) byte-identical to the session's baseline, except the 9 runs of
  `switch_case_expr.c` and its three twins and of `switch_unsigned_case.c` and its twin, whose case values moved to
  the prologue (reviewed: obligations and an `internal` guard at entry, dispatch on EDG's value).
- `--count-error-twins` 149 (141 + 8); `--check-headers` 1049/1049.
- Mutation: first run over the new positives, the doc example and the two switch tests whose Viper changed: 429
  mutants, 293 killed, 7 listed equivalents held, 32 safe-direction, 89 unexplained, 8 stillborn; after the positives
  were strengthened, the 8 positives and the doc example: 277 mutants, 265 killed, 6 safe-direction, **0 unexplained**,
  6 stillborn, **0 tool failures**; no new list entries.
- Audits (Opus, one reviewer): 0017 — 2 blocking (weak constants; C tentative definitions with external linkage), 6
  should-fix, 9 notes, all blocking and should-fix taken; code — 1 blocking (attributes on constants not checked), 2
  should-fix, taken; re-audit — 1 blocking (`#pragma clang attribute`, unrecognized pragmas), fixed; final re-audit of
  the pragma fix: 0 blocking.
- Wall-clock: 22:28–00:20 (planning before).

**Not done / deferred**
- M4.2b2 (enumerations, handoff §8.1) and M4.2c.
- Audit notes (handoff §6, M4.2b1): C `_Static_assert` naming a constant under `#pragma diag_suppress 1901`; static
  assertions are not encoded; messages; #177 noise; `mutate.py` does not mutate constant initializers (the planned
  extension was cut).

**Surprises**
- C23 `constexpr` does not parse in this EDG configuration (§11 question 15); EDG folds `const` objects in C as an
  extension (#1901), so C uses of constants are folded constants too.
- Silicon continues after a failed assert with its condition assumed (S14 corrected mid-session); the prologue's
  argument rests on its asserts being closed.
- The Opus audits found two wrong "verified" results in the first design (weak constants; C tentative definitions,
  common symbols under clang's default `-fcommon`) and one in the code (attributes such as `loader_uninitialized` on a
  constant), plus a `#pragma weak f` hole for functions that predates this session.
- In C++ a `#pragma weak` before any declaration of its name is ignored by clang++ and EDG alike.

**Decision records added:** 0017-named-constants; amended 0010, 0011, 0012, 0014.

**First step for the next session (M4.2b2).** Confirm the owner merged the `mutation-triage` and
`m4.2b1-named-constants` PRs and CI is green on `main`. Then handoff §8.1 "M4.2b2" (the next free record, 0018): probes
first (the enum IL facts it lists), with §11 question 13 (unsigned wrapping) in mind.

## S3 — Mutation triage (2026-10-21): done on branch `mutation-triage`; PR pending

Read `internal/handoff.md` next: the "S3 is done" owner steps at the top, §5 the result, §8.1 M4.2b.

**Owner decisions (2026-10-21)**
- S3 runs now, before M4.2b (the plan's order); it ends on a branch, and the owner opens the PR.
- **Audit A item 3, "verifier-checked":** the safe-direction bucket has no guard rule. A bucketed mutant is a correct
  program only if its mutated clause is well-defined. A surviving mutant has already passed cerastes' own contract
  well-definedness check (0010 item 11), which `guard_kept.c` and its `negative/guard_*` twins pin. The literal rule
  would have moved 145 mutants out of the bucket.
- Defaults accepted with the plan:
  - the full run covers `tests/scalar` and the 4 M4.2a doc examples (the other 27 doc examples go to M4.8);
  - the classifier does not learn typed locals: a weaker `check` or `assert` of a local is listed;
  - `internal/triage/` is removed, being resolved (`git show 50f07ca:internal/triage/<file>` keeps it).

**Done**
- **`mutate.py`**
  - a progress line per finished run on stderr;
  - a run checks list entries only for the files it selects (trap 1);
  - `--validate` also generates every file the list names (trap 2);
  - `assert(e);` modelled as a CHECK clause;
  - C-exact reading: chained comparisons, a loose `!` and a logic group inside arithmetic or a comparison (nested
    brackets too) are `unknown`, and so is any `*` (audit A items 1–2);
  - `has_caller` counts uses in `#define` bodies and default arguments (item 4);
  - each clause compiled once: `--validate` 93 s → 3 s.
- **`lint_viper.py`:** the termination rules see a `decreases` clause split over lines (item 5).
- **Self-tests:**
  - the shadowed duplicate test runs, with a duplicate-name check;
  - the order test is event-driven;
  - `test_mutate.py` 180 → 225, `test_lint_viper.py` 142 → 152.
- **Weak tests strengthened in place**, 17 files, never an `expect:` line, with 33 new must-fail twins:
  - M4.1: `continue_cond_div.c`, `do_continue_cond_div.c`, `do_zero_expr.c`, `switch_call.c`;
  - M4.2a: `goto_in_loop_body.c`, `goto_cleanup.c`, `goto_out_of_loops.c`, `exit_main.c`, and the doc examples
    `verifying/assert.c` and `error-messages/noreturn_fixed.c`;
  - the M3 leavers: `call_precondition.c`, `call_result_conversion.c`, `default_argument.cpp`,
    `default_argument_expr.cpp`, `measure_function_expression.c`, `shift_promoted.c`, `ushort_scale.c`;
  - the well-definedness pins `guard_kept.c` and its eight `guard_*` twins, `measure_loop_overflow_later.c`, and
    `main_implicit_post.c`.
- **`tests/mutants-equivalent.txt`:** 125 → 436 entries, every new one reviewed by the Opus auditor.
- **Leavers resolved:**
  - the 74 M3 leavers: 25 already killed, 4 stillborn, 24 killed by strengthened tests, 21 listed;
  - the 30 that `*` moves out: 1 killed, 29 listed;
  - M4.1's and M4.2a's survivors: the weak ones killed, the rest listed with corrected reasons.

**Acceptance**
- **Full mutation run**, in three shards balanced by verifier runs (`-j 10`, 18:13–19:55, 1 h 41 min), over `tests/scalar` and the four doc examples: each shard exits 0. Together: 145 tests, 3880 mutants: 2652 killed, 436 equivalent (reviewed; every list entry survived), 733 safe-direction, **0 unexplained survivors**, 59 stillborn, **0 tool failures**
- `scripts/check.sh` green (14 min 7 s, at the final tests and list): self-tests 951 + 74 + 152 + 225; tests 2694 passed, 0 failed, 1 skipped (native-ok: Apple clang 644, GCC 644); 733 `.vpr` lint clean; VIR self-test 230/230; walker oracle 134 goldens, 0 failed; docs 0 problems. Run #1 (14 min 37 s, before the audit fixes): green, 2678 tests
- `--validate`: 141 tests, 3801 mutants, 436 equivalents, 742 in the bucket.
- `run_tests.py --count-error-twins` 141; `--check-headers` 988/988.
- The generated ids are identical to HEAD's for every file the session did not edit.
- Audits (Opus):
  - round 1 (tooling): 1 blocking, fixed;
  - round 2 (entries and tests): 1 blocking (`do_zero_expr.c` had slack; killed), 2 kills preferred and done;
  - the re-audit: every item closed, no new blocking item.
- Wall-clock: 16:47–20:15.

**Not done / deferred**
- The 27 verified doc examples outside the M4.2a four have never been mutated (M4.8).
- Audit notes (handoff §6):
  - `assert` is recognised by name, not as the `<assert.h>` macro;
  - a literal mutated past `LLONG_MAX` changes type;
  - `test_the_guard_rule_is_gone` is a `hasattr` check.

**Surprises**
- The literal guard rule (audit A item 3) would have moved 145 mutants out of the bucket. The same argument applies
  to 242 relop/literal mutants. Hence the owner's verifier-checked rule.
- `do_zero_expr.c`'s two "equivalent" proposals were stronger `ensures` on a slack bound: a weak test (round 2).
- Silicon checks an invariant's well-definedness where it is evaluated (at entry and after each iteration), not in
  an arbitrary loop-head state, so a dropped invariant guard is reported only when a reachable evaluation is
  ill-defined (sound; agent A's first pin verified, so `guard_changed_invariant.c` uses a changed guard).
- Generating mutants took 2 minutes (each clause was compiled once per grid point).

**Decision records added:** none (the bucket's argument is in `mutate.py` and `tests/README.md`).

**First step for the next session (M4.2b).** Confirm the owner merged the `mutation-triage` PR and CI is green on
`main`. Then handoff §8.1 "M4.2b" (record 0017), with §11 question 4 (a spike of EDG's source sequence lists) in
its planning.

## S2 — CI (2026-10-21): done on branch `ci` (PR #1, green); merge pending

Read `internal/handoff.md` next: the "S2 is done" owner steps at the top, §4.2 the result, §5 the mutation triage
session S3.

**Owner decisions (2026-10-21, S2 planning and close)**
- **Toolchain:** CI selects the runner's Command Line Tools (`DEVELOPER_DIR=/Library/Developer/CommandLineTools`),
  not Xcode 27.0. The `xcode-27` image ships CLT 27.0.0.0.1788430756, the owner's exact package.
- **First runs** come from a pull request the owner opens (PR #1). `workflow_dispatch` needs the file on `main`.
- **`check.sh`'s VIR line** drops its hard-coded `-j 8`. The default is `CERASTES_JOBS`, else half the cores.
- **GCC:** the owner reinstalled Homebrew `gcc` (16.2.0). It is now the macOS 27 bottle with sysroot SDK 27.0; it
  was the macOS 26 bottle, against SDK 26.5.
- **Handoff questions 6–9:**
  - CI becomes a required check on `main` after the first green run there;
  - when the image's tools change, move the assertion and 0001 together after a rehearsal;
  - tighten the timeout after two green runs on `main`;
  - no SHA allowlist for now.

**Done**
- **Decision 0016-ci.** 0001 amended: the CI actions, the host toolchain (SDK build `26A425`), a new GCC row, Python.
  0004 item 4 amended. `CLAUDE.md` gains a CI line.
- **`.github/workflows/ci.yml`**, one job on `xcode-27`:
  - triggers: pull requests, pushes to `main`, dispatch; permissions `contents: read`;
  - concurrency: PRs cancel superseded runs; `main` runs never cancel;
  - `timeout-minutes: 240`;
  - a full checkout with submodules, so `edg_tree.py` finds `origin/cerastes`;
  - Python 3.14 and Node 26;
  - the Silicon jar cached by the silicon and silver gitlinks;
  - `scripts/ci.sh`, then `scripts/check.sh`, as separate steps;
  - the evidence uploaded with `always()`: `build/vpr`, `build/env.sh`, the toolchain report, `build/*.log`.
- **`scripts/ci.sh`**
  - asserts:
    - the CLT package;
    - the `/usr/bin` compiler shims;
    - the clang banner;
    - SDK 27.0, build `26A425`;
    - GCC 16.2.0, whose sysroot is that SDK;
    - the EDG submodule is not shallow and has `origin/cerastes`;
    - `edg_tree.py check`;
    - 21 environment variables are unset;
  - installs GCC when missing, and runs the full `bootstrap.sh`;
  - checks `build/env.sh` and writes `build/ci-toolchain.txt`;
  - never installs anything already present, so a rehearsal leaves the owner's Mac unchanged.
- **Harness**
  - `rt.default_jobs()` reads `CERASTES_JOBS` (`run_tests.py`, `mutate.py`). It is read lazily, and `-j` wins.
    Twins.
  - `CiGuardTests` pins the whole active text of `ci.yml` (only the SHAs, names, timeout and job count may vary)
    and of `ci.sh` (only comments may vary). Printable ASCII only. 223 and 402 rejected mutations.
  - `workflow_problems` (`docs.yml`) now rejects a job-level `if:` or `continue-on-error`, and `if :`.
  - Self-tests: harness 931 → 951, mutate 174 → 180.
- **Docs and comments:** `tests/README.md` (Parallelism, CI) and the `docs.yml` header comment. No `doc/` page:
  nothing user-visible.

**Acceptance**
- **CI run [39664543570](https://github.com/cerastes-verifier/cerastes/actions/runs/39664543570)** at `c959f59`: green
  in 44 min (`check.sh` 41 min).
  - Log: CLT 27.0.0.0.1788430756, clang-2100.3.34.2, SDK 27.0 `26A425`, g++-16 16.2.0 (sysroot SDK 27.0), Python
    3.14.8, image `20261012.0260.1`.
  - Every count equals the fresh-clone run #2:
    - self-tests 951 (skipped=2), 74, 142, 180;
    - tests 2624 passed, 0 failed, 1 skipped (native-ok: Apple clang 642, GCC 642);
    - VIR 230/230;
    - oracle 134 goldens, 0 failed;
    - docs green.
  - The uploaded `build/vpr` (1335 files) is byte-identical.
- **Local `check.sh`, twice, in fresh clones** with the runner's environment (`CERASTES_JOBS=3`, the CLT): both
  green.
  - Run #1 at `6bfb1c3`: 21 min. Run #2 at `c80d0d9`: 20 min.
  - The final `c959f59` changes only self-test code and 0016; its full harness self-test gave 951 OK.
- **Local baseline with the reinstalled GCC:** 2624/0/1, native-ok 642/642; `.vpr` byte-identical to S1's.
- **Audits (Opus):** three rounds, 0 blocking. Every should-fix item is taken (handoff §4.2).
- **Wall-clock:** 14:39–16:35 (planning before).

**Not done / deferred**
- The guard does not tie each action SHA to 0001 (owner: not now).
- `check.sh`'s own guard still misses a line wrapped in `if false; then … fi`. This predates S2.
- Tightening the timeout waits for two green runs on `main`.
- S3 (mutation triage, handoff §5), then M4.2b.

**Surprises**
- `xcode-27` is a public preview, and it ships the owner's exact Command Line Tools.
- The owner's GCC had been compiling the native-ok runs against SDK 26.5.
- The runner is quick:
  - checkout 2.5 min;
  - `ci.sh` 3.5 min cold, 1 min 45 s with the jar cached;
  - tests 2.4× slower than locally at the same `-j 3`.
- Each audit round found a new way past the text guards. Pinning the whole active text (ASCII only) ended it.
- `pull_request` runs need no workflow on `main`; `workflow_dispatch` does.

**Decision records added:** 0016-ci; 0001 and 0004 amended.

**First step for the next session (S3, mutation triage).** Confirm that the owner merged PR #1 and that the CI run
on `main` is green. Then handoff §5.

## S1 — EDG upstream merge and pin move (2026-10-21): done on branch `edg-upstream-merge`; fork push, fast-forward and pushes pending

Read `internal/handoff.md` next (the "S1 is done" owner steps at the top, §4.1 the result, §4.2 the CI session).

**Owner decisions (2026-10-21, planning round)**
- The CI session is split: **S1** EDG upstream merge (box), **S2** CI, **S3** mutation triage, then M4.2b; each
  ≤ 4 h with at most three Sonnet agents and one read-only Opus auditor; any question stops the work and goes to
  the owner (`plan.md`, handoff §4, §5).
- S2: GitHub's standard `xcode-27` runner, the full `check.sh` on pull requests, pushes to `main` and dispatch; the
  owner pushes at checkpoints. S3: weak tests strengthened in place (never an `expect:` line).
- The fork's duplicate `__BLOCKS__` test stays for now; the owner files 0020 upstream.

**Done**
- **Plan:** `plan.md` and the handoff describe the three sessions with the facts found while planning (commit
  `d344584`); M4.2b's and M4.2c's records become 0017 and 0018 (CI takes 0016).
- **Fork:** `97c2141be9` merges upstream `main` `21065e551e` (10 commits after `307002eae6`; upstream moved past
  the planned `dedc820c91` that morning, tests only) into `cerastes`: `src/Changes` both sides kept,
  `util/Changes` and `util/make_predef_macro_table` upstream's text (its re-commit of our 0005).
- **Box:** the full suites at the merge match (known `cwg1835`, `cwg2881`); `run.sh` now builds from scratch and
  reports `BUILD FAILED` when ninja stops.
- **0020's Linux failure explained:** the box's incremental build failed to link and `edg-docker-test` ran the
  previous binaries (0020's previous revision), so the two cases of the last amend failed; a clean build passes
  (`internal/edg_bugs/0020-*.md`).
- **Pin** `97c2141be9` (commit `1b5c352`); decision 0001's EDG row and pin history.

**Acceptance**
- `scripts/check.sh` green (15 min 43 s, one run): tests 2624 passed, 0 failed, 1 skipped (native-ok: Apple clang
  642, GCC 642); VIR self-test 230/230 (+ 87 rejection cases); walker oracle 134 goldens, 0 failed; docs 0 problems.
- Every test's emitted Viper (667 `.vpr` and maps) byte-identical to P's.
- Box `U-full` at `97c2141be9`, `changes,regressions,imported,edg,cwg,stdlib`, both 64-bit configurations: match
  except the two known; clean builds of `c539de4101`: `0020e` (`changes`) and `0020f`
  (`changes,regressions,imported`) match in both configurations.
- Audit (Opus): 0 blocking; three should-fix items taken (handoff §4.1).
- Wall-clock: about 11:29–12:05 plus box waits.

**Not done / deferred**
- S2 (CI) and S3 (mutation triage), handoff §4.2 and §5.
- The fork's duplicate `GH-no-blocks-predefine` test (owner: leave it).

**Surprises**
- Upstream moved twice in two days (`dedc820c91`, then `21065e551e`).
- The 0005 duplicate conflicted after all: the fork carries the session's original 0005, upstream the owner's
  re-commit.
- A failed link on the box does not fail `edg-docker-test`: it tests whatever binaries are left.

**Decision records added:** none; 0001 amended.

**First step for the next session (S2, CI).** Confirm the owner pushed the fork's `cerastes` (`97c2141be9`) and
fast-forwarded and pushed `main` (handoff, top). Then handoff §4.2: decision 0016 first.

## M4.2a — forward goto, functions that do not return, assert (2026-10-21): done on branch `m4.2a-goto-noreturn-assert`; fork push, fast-forward and pushes pending

Read `internal/handoff.md` next (§2 owner steps, §4 the CI session, §5 the mutation triage session, §6 audit notes,
§11 questions).

**Owner decisions (2026-10-21)**
- M4.2 is split into **M4.2a** (this session), **M4.2b** (`const`/`constexpr` globals, `case k`, enumerations) and
  **M4.2c** (locals without an initializer); only M4.2a ran. It ran before the CI and triage sessions.
- The EDG pin is restored to P `b9519388ea` on the branch: `main`'s `518e0de` pinned `0bcd9bf28a`, which lacks
  `RAF_NO_CONSTANT_EVALUATION`, so `main` did not build.
- Pre-approved retirements: `reject_goto.c` and `reject_noreturn.c` (done here); `reject_case_variable.cpp`,
  `reject_enum.c` (M4.2b) and `reject_uninit_local.c` (M4.2c).
- A new error category `noreturn`; a failing `assert(e)` is `check`.
- "Verified" now means: on every input satisfying `requires`, no undefined behaviour, and the function returns
  satisfying its `ensures` or ends the program by calling a function that does not return (0013 item 6 amended).

**Done**
- **Plan:** `plan.md`'s M4 table and handoff §8.1 hold M4.2a/b/c, with a Plan-agent review's findings for M4.2b
  (cross-mode traps of const globals in `do … while (0)`, case-value guards, the enum one-meaning rules) and M4.2c
  (definite assignment recommended).
- **Decision 0015:** forward `goto` "outward and forward" (the label is a later statement of a block enclosing the
  `goto`, no declaration between; never into a block, branch, switch body or loop); functions that do not return
  (`ensures false`, `assert false` with `noreturn` at each reachable return and the end; `ensures` on them rejected;
  a user's `[[noreturn]]` on a declaration is a trusted user contract unless a system header wrote the flag); the
  trusted libc table (`abort`, `exit`, `_Exit`); `assert` by the exact shape of the macOS expansion, through folded
  constants, checked as `check`. Amends 0011–0014.
- **Probes and spikes:** P19–P22, S11–S13, all confirmed (`il-inventory.md`, "M4.2a").
- **Code:** `src/vir` (Goto/Label, `vir::goto_problems`, `Function::noreturn`, the assert-origin `Check`),
  `src/backend/encode.cpp` (user gotos and labels with label attributes from the entity, block-flattening guards,
  noreturn, the libc table, the assert matcher, `ensures_on_noreturn`), `verify_main.cpp` (#1305 suppressed).
- **Tooling:** `lint_viper.py` checks every `goto` outward and forward at token level (closes the M4.1 audit note);
  `mutate.py` deletes `goto L;` and buckets a deleted call-free `assert(...)`; the category `noreturn` in the harness.
- **Tests:** 90 new files in `tests/scalar` (27 positives, 63 twins and rejections, both modes where C++ accepts
  them); `reject_goto.c`, `reject_noreturn.c` retired; VIR self-test 204 → 230 lowering cases (75 → 87 rejection
  cases).
- **Docs:** Verifying a program (goto; functions that do not return; `assert`; the meaning of a result; the rejected
  list), Error messages (`noreturn`), Annotations (`check` vs `assert`), Overview.

**Acceptance** (handoff §10)
- `scripts/check.sh` green at `610e7e1` (17 min 5 s, one run): self-tests 931 + 74 + 142 + 174; tests 2624 passed,
  0 failed, 1 skipped (native-ok: Apple clang 642, GCC 642); 667 `.vpr` lint clean; VIR self-test 230/230 (+ 87
  rejection cases); walker oracle 134 goldens, 0 failed; docs 0 problems.
- Every existing test's emitted Viper is byte-identical to the session's baseline (555 files).
- `--count-error-twins` 140 (113 + 27); `--check-headers` 954/954.
- Mutation run on the new positives and doc examples: 913 mutants, 476 killed, 220 bucket, 13 stillborn, 200
  survivors (185 after the replaced doc example; classified, 0 suspicious; to the triage session, handoff §5).
- Audits (Opus): 0015 (1 blocking, dropped from the design), the code (0 blocking), the tooling (0 blocking); every
  should-fix item taken.
- Wall-clock: about 2 h 40 min (08:19–11:00).

**Not done / deferred**
- M4.2b and M4.2c (handoff §8.1).
- Audit notes (handoff §6): the IL keeps only the first declaration's position, so who wrote a noreturn flag is
  checked through attribute entries (`_Noreturn` leaves none); the sound check needs EDG's source sequence lists.
- glibc's `assert` expansion (for a Linux CI runner, handoff §4).
- The CI and mutation triage sessions (unchanged, next).

**Surprises**
- `main`'s last commit had moved the EDG pin back to a commit without `RAF_NO_CONSTANT_EVALUATION`.
- EDG keeps a label's attributes on the label entity, not on the label statement; a label that is the whole branch
  of an `if` without braces sits in a compiler-generated block.
- A plain user declaration of `quick_exit` before `<stdlib.h>` takes its noreturn flag from the SDK's `__dead2`
  (code audit): the flag alone does not say who claimed it.
- EDG folds `!(0)` in `assert(0)`, so the matcher reads folded constants' original expressions (0010 item 12).
- Asciidoctor registers `[[noreturn]]` in text as an anchor id even inside literal monospace (duplicate-id warning).
- Committing the tooling picked up two staged `git rm`s (the M4.1 process note); redone before anything else.

**Decision records added:** 0015-goto-noreturn-assert; amended 0011, 0012, 0013, 0014.

**First step for the next session (CI).** Confirm the owner pushed the fork's `cerastes` (P) and fast-forwarded
`m4.2a-goto-noreturn-assert` (handoff §2). Then the CI session as in handoff §4 (box: upstream EDG merge, the
0020-alone Linux failure), then the mutation triage session (§5), then M4.2b.

## M4.1 — control flow, and the plan re-sequenced (2026-10-20): done on branch `m4.1-control-flow`; fast-forward and pushes pending

Read `internal/handoff.md` next (§2 owner steps, §4 the CI session, §5 the mutation triage session, §8 the M4 and M5
session plans, §11 questions).

**Owner decisions (2026-10-20)**
- **Re-sequencing** (planning M4 showed Gate G2's VeriFast tests need pointers, references and classes): the new
  **M4 is the rest of C** (strictly C; M4.1–M4.8, pointers in M4.5a–f, VerCors as the reference); generic templates
  are **M5** (M5a–M5g); concepts M6; **C++ memory and classes M7** (G2's parts that need them are deferred there);
  inventory M8; evaluation M9. Records were renumbered everywhere (each notes it); `status.md` history keeps the old
  numbers (`plan.md`'s table maps them).
- EDG work of M5 (first-declaration marker, the silent error node) is its own session with the box (M5b).
- Order: this session (M4.1, cerastes-only) → CI → mutation triage → M4.2 ….
- M4.1: `do … while (0)` needs no measure; `[[fallthrough]]` allowed; `reject_break.c`, `reject_continue.c`,
  `reject_do_while.c`, `reject_switch.c` and the Verifying page's switch example retired (pre-approved).

**Done**
- **Plan:** `plan.md` renumbered (table at the top), the new M4 with its 4.x sessions, M5 with M5a–M5g, M7 "C++
  memory and classes"; the handoff §8 holds every M4 and M5 session plan (designed this session: the M5 eligibility
  checker, the opaque/integral `T` models without axioms, one-run fallback, G2 scope; the VerCors study for M4.5).
- **Decision 0014 (control flow):** structured VIR jumps (`Break`/`Continue` to the innermost target, `Switch`/`Case`,
  loop forms `While`/`DoWhile`/`DoOnce`, a `for` loop's `step`) lowered to forward gotos; invariants and the measure
  checked on `continue` paths; `do` invariants at the start of every iteration; `do … while (0)` as straight-line
  code with an internal guard; `switch` dispatch with case values encoded by their original expression; the
  rejections; #546 suppressed (cerastes rejects every such jump itself). Amends 0010, 0011, 0012, 0013.
- **Code:** `src/backend/encode.cpp` (target stack with label-shape checks, `do`, `switch`, `step`, allow-list),
  `src/vir` (forms, `check_program` rules, lowering), `src/driver/verify_main.cpp` (#546), VIR self-test
  (204 lowering + 75 rejection cases).
- **Tooling:** `mutate.py` deletes `break;`/`continue;` and case bodies; `lint_viper.py` rejects backward gotos,
  labels with invariants, duplicate labels.
- **Tests:** 86 new files in `tests/scalar` (31 positives, 55 twins and rejections; both modes where C++ accepts them); the M3 rejection tests of `break`, `continue`, `do` and `switch` retired; VIR self-test 155 → 204 lowering cases (+ 75 rejection cases). 70 surviving mutants of the new tests go to the triage session (handoff §5)..
- **Docs:** Verifying a program (a control-flow section with a verified example; the rejected list; the
  rejected-construct example is now `rejected_double.cpp`), Annotations (`do` invariant meaning, `decreases` before
  `do`, the `do … while (0)` exemption), Overview.

**Acceptance**
- `scripts/check.sh` green (14 min 31 s): self-tests 931 + 74 + 125 + 154; tests 2350 passed, 0 failed, 1 skipped
  (native-ok: Apple clang 586, GCC 586); 555 `.vpr` lint clean; VIR self-test 204/204 (+ 75 rejection cases);
  walker oracle 134 goldens, 0 failed; docs 0 problems.
- Every M3 test's emitted Viper is byte-identical to the session's baseline (412 files).
- `--count-error-twins` 113 (82 + 31); `--check-headers` 861/861.
- Mutation run on the 31 new positives: 1403 mutants, 991 killed, 299 bucket, 27 stillborn, 70 survivors
  (to the triage session, handoff §5).
- Audits (Opus): 0014 (1 blocking, fixed: case values by their original expression) and the code (0 blocking);
  every should-fix item of both taken; notes in handoff §6.

**Not done / deferred**
- Forward user `goto` and labels (M4.2's first item; `reject_goto.c` unchanged).
- 70 surviving mutants of the new tests (handoff §5), moved to the triage session by the stop rule.
- Audit notes (handoff §6): the lint does not flag a forward `goto` into a loop body (only VIR's nesting rule and
  Silicon's exception guard it); three encoder rejection branches look unreachable and have no test.
- The CI session and the mutation triage session (unchanged, next).

**Surprises**
- EDG folds constant expressions it only warns about (`case 1 << 40` → 0, `2147483647 + 1` in C23) and verify mode
  suppressed those warnings: case values had to be encoded by their original expression (Opus audit, blocking).
- EDG accepts Duff's device, GNU case ranges and a jump over a declaration (only warnings), and Silicon throws on a
  `goto` into a loop: each is rejected by cerastes.
- A `do … while (0)` lowered as a Viper loop with `decreases 0` loses the facts its body establishes; straight-line
  code keeps them.

**Decision records added:** 0014-control-flow; amended 0010, 0011, 0012, 0013; all records renumbered.

**First step for the next session (CI).** Confirm the owner's fast-forward of `m4.1-control-flow` and the fork push
(handoff §2). Wake the box, merge upstream EDG `main` into the fork's `cerastes` (handoff §4), run the full box
suites, move the pin, run `check.sh`; explain the 0020-alone Linux failure; then CI.

## M3 close-out (2026-10-20): done on branch `worktree-m3-closeout`; fast-forward and pushes pending

Read `internal/handoff.md` next (§2 owner steps, §4 the CI session, §5 the mutation triage session, §11 questions).

**Owner decisions (2026-10-20)**
- Scope: the M3 close-out only, planned at ≤ 4 h. CI stays its own session before M4.
- Pin P only. The upstream EDG merge (`307002eae6` → `ed0a9efb06`) and its box run open the CI session, an
  exception to CLAUDE.md's session-start merge. The branch-0020-alone Linux failure goes to the CI session too.
- Expectation rewrites: a rejection may move from a `cerastes-*` diagnostic to an EDG one at the same line; r4a
  keeps the 18 contexts EDG now rejects, and the 6 it accepts are covered by single-context tests.
- Handoff answers folded into `plan.md`: the first-declaration marker rule in M4, the contract manifest in M5 (M6
  if it does not fit), a "CI session (before M4)" section.

**Done**
- **Pin at P** `b9519388ea` (the fork's `cerastes` fast-forwarded over 0020, L, M, N, P; chain worktrees and temp
  branches removed, `0020` kept).
- **forall over `T`** (R1 moves): positives for function templates and member templates with goldens checked
  against `--il_display` by a second agent, twins, the doc example, the Annotations page, 0006.
- **`RAF_NO_CONSTANT_EVALUATION`** registered for `predicate`/`pure`: the front end refuses compile-time evaluation
  of spec functions (the five no-trace contexts closed); 14 expectation rewrites under the owner's rule, r4a by the
  owner's decision; docs, 0006, 0007, the upstream draft.
- **Audit B's blocking item:** a substituted lambda whose own signature holds an error type (series L leaves one,
  silently, for a `decltype(<parameter>)` lambda parameter) is now `annotation_substitution_error`, with two twins.
- **Tooling:** `mutate.py --list`; `lint_viper.py` rejects `decreases *` and conditional `decreases`; the
  safe-direction classifier buckets only clauses the grid models and functions with no use in the file (audit A's
  blocking item fixed and re-audited); 0013 item 5.
- **Harness:** `annotation_substitution_error` has a source test now; the walker oracle recognises the closure
  copies series L makes for instances (narrow rule, twins).

**Acceptance** (handoff §10)
- `scripts/check.sh` green at `05da953` (10 min 19 s): self-tests 931 + 74 + 113 + 142; tests 2064 passed, 0
  failed, 1 skipped (native-ok: Apple clang 523, GCC 523); 412 `.vpr` lint clean; VIR self-test 155/155; oracle
  134 goldens, 0 failed; docs 0 problems.
- `--count-error-twins` 82; `mutate.py --validate` 1314 mutants, 223 bucket candidates; `--list` 0.6 s, no run.
- Audits A and B (Opus): one blocking item each, fixed and closed on re-audit.
- Wall-clock: about 1 h for the execution (planned ≤ 4 h).

**Not done / deferred**
- **Mutation triage** (its own short session, handoff §5): 74 mutants left the safe-direction bucket (cap 30), so
  M3's "0 unexplained survivors" holds for the old bucket only until they are killed or listed; audit A's
  should-fix items go with it.
- The upstream EDG merge, the 0020 Linux failure, CI (the CI session, handoff §4).
- Audit B's should-fix items (handoff §6): a false `spec_call_in_code` from series L's leftover closure copies,
  three missing tests (the `operator=` marker branch, overload choice changed by P, the constant-context pass in
  never-instantiated templates); the EDG follow-up for the silent error node (question 2).

**Surprises**
- At P, EDG leaves an error node without a diagnostic in a function template's contract whose `forall` lambda
  takes `decltype(p)` of a parameter: the back-end check `annotation_substitution_error` is now what rejects it.
- Registering the flag makes EDG reject most compile-time uses itself, which stops the back end in the whole file:
  the pre-approved "same line" rule did not fit r4a, and the cerastes-side constant-context checks lost their tests.
- Series L copies a lambda's closure class for each instance without a `lambda` record; the walker oracle needed a
  rule for it.

**Decision records:** none added; 0006, 0007, 0013 amended.

**First step for the next session (CI).** Confirm the owner's fast-forward and pushes (handoff §2). Wake the box,
merge upstream EDG `main` into the fork's `cerastes` (handoff §4), run the full box suites, move the pin, run
`check.sh`; explain the 0020-alone Linux failure; then CI.

## M2 re-look items and M3 — scalar encoder and driver (2026-10-16 to 2026-10-19): done; pushes and the pin move pending

Read `internal/handoff.md` next (§2 owner steps, §4 the EDG chain, §9 questions).

**Owner decisions this session (2026-10-16)**
- Narrowing to a signed type that does not fit is an error, implicit or explicit; to unsigned it wraps (0010).
- Annotation arithmetic has its C/C++ meaning (0010 item 11).
- `cerastes verify` verifies the primary file and user headers; system files only by contract, decided by
  canonical path (0011).
- Termination is checked: `decreases` on loops and recursive functions, total correctness (0013).
- `constexpr` stays allowed on `predicate`/`pure`; compile-time uses are rejected (IL pass + fork commit P).
- Lambdas in template contracts can name `T`, member templates included (EDG 0020 + series L, M).
- Mutation acceptance: safe-direction contract mutants (stronger `requires`, weaker `ensures`/`check`) form their
  own counted bucket; the rest is killed or listed as reviewed equivalents.
- CI deferred to its own session; full EDG suites run on the x86-64 box by the session; no Claude co-author
  trailer on any commit; approved expectation changes: `cerastes_h.cpp` rewritten (R4e), 12 lines added to
  `tests/il/m3/r4a_constant_contexts.cpp`, the R1 test moves (applied with the pin move, next session).

**Done**
- **Session start:** upstream `307002eae6` merged into the fork (`0bcd9bf28a`), pinned (`7aec17d`), box-checked.
- **Re-look items** (handoff §3): the walker oracle committed and in `check.sh`; R4a–R4e and the R2 findings
  closed on the cerastes side (`annotation_macro_changed`, `cerastes_header_in_namespace`,
  `in_class_specialization_contract`, `spec_marker_on_special_member`, `spec_marker_in_system_file`, more
  `spec_call_in_code` and `old_outside_ensures`); `RECORD_MACROS_IN_IL=1`; `<cerastes.h>` fails closed; brand
  files; records.
- **M3:** decision records 0010–0013 and `internal/encoding.md`; `src/vir` and `src/viper` (VIR, typing, lowering,
  the prelude with bodies only, the emitter and position map, a 155-case Silicon-checked self-test); the IL → VIR
  encoder (`src/backend/encode.cpp`); `build/cerastes verify` (allow-listed options, tool paths fixed at
  configure time, Silicon in a process group with timeout and signal cleanup, a strict verdict parser, mapping by
  line + id + reason, exit codes 0–3, `--selftest` with 81 checks, self-test hooks that can never verify);
  eleven error categories; `scripts/lint_viper.py`, `scripts/mutate.py`, the harness for verify tests (streams,
  categories, `@ file:L` for headers, helper headers, dual modes, error twins, a `build/vpr` lock); `tests/scalar`
  (82 positives, 175 negatives, 6 helper headers); user docs: Verifying a program, Integer semantics, Error
  messages (one section per category), the Installation walkthrough, Annotations updated (`decreases`, R4 rules).
- **EDG:** bug 0020 (three audit rounds), series L, M, N, P built, audited and box-checked on temp branches
  (handoff §4); bug files 0020 and 0021.

**Acceptance**
- `scripts/check.sh` green (821 s on a quiet machine): self-tests 931 + 68 + 107 + 117; tests 2043 passed,
  0 failed, 1 skipped (native-ok: Apple clang 516, GCC 516); 412 emitted `.vpr` lint clean; VIR self-test 155/155;
  walker oracle 131 goldens, 0 failed; docs 0 problems.
- `run_tests.py --count-error-twins`: 82 (every positive has an `error` twin in each of its modes).
- Mutation run (test-mutator, full `tests/scalar`): 892 killed, 260 safe-direction, 125 listed equivalents,
  0 unexplained survivors, 19 stillborn; the 125 equivalents reviewed and accepted by the Opus auditor
  (2026-10-19), who also re-checked every bucket entry densely (none mislabelled).
- `build/cerastes verify doc/.../verifying/midpoint.cpp --keep-vpr=…`: exit 0, `.vpr` and `.map` written, lint
  clean; `…/midpoint_overflow.cpp`: exit 1, `midpoint_overflow.cpp:10: error: overflow: …`.
- Every encoder and driver change audited (Opus): records twice, harness three rounds, VIR libraries, driver,
  encoder, the delta with termination; every blocking finding fixed.

**Not done / deferred**
- **Merge the EDG chain and move the pin** (next session's first step, handoff §4): fast-forward the fork's
  `cerastes` to P `b9519388ea` (the auditor approved the merge; the box's full suite matches at the tip except the
  known `cwg1835`/`cwg2881`), pin it, register `RAF_NO_CONSTANT_EVALUATION` for `predicate`/`pure`, add the 5
  remaining expect lines to `r4a_constant_contexts.cpp`, apply the approved R1 test moves (`forall` over `T`),
  update 0006 item 9, 0007, the Annotations page and the upstream draft.
- **Branch `0020` alone fails its own new test on Linux** (`GH-lambda-template-param-qualified`, both modes, FAIL
  — it passes at the chain tip): explain before the owner files 0020 upstream.
- **Tooling should-fix items from the last audit:** `mutate.py`'s direction classifier decides on a grid of plain
  integers (restrict the bucket to linear signed clauses and treat `==`/`!=` flips as unknown, or use Z3 with C
  types); the bucket ignores callers in the file; `mutate.py --list` starts a run; `lint_viper.py` accepts
  `decreases *` and conditional `decreases` (the emitter produces neither).
- Known gaps in handoff §5 item 2 (cross-unit termination; first-declaration marker rule; attributes EDG drops).
- CI (its own session); the contract manifest.

**Surprises**
- The audits found blocking holes at every stage, and several were unsound in subtle ways: unsequenced calls behind
  a non-returning callee, `[[assume]]`, `-U__cplusplus` and `#define ensures requires` silently dropping or turning
  contracts, bodies attached to system templates skipped, a wrong overload silently chosen by EDG fix 0020 (twice).
- C++ forward progress made partial correctness unsound for C++: hence termination in M3.
- `edg_x86_64_cp` expectation files are overrides; the "known cp differences" of earlier handoffs were an artefact.
- Fork commit J's fail-closed rule had leaked to ordinary attributes since M2 (fixed in L).
- Concurrent harness runs raced on `build/vpr` (now locked); 14 stale background shells (now a standing rule).

**Decision records added:** 0010-integer-semantics, 0011-verify-driver, 0012-scalar-encoding, 0013-termination;
amended 0002 (`RECORD_MACROS_IN_IL`), 0006 (R4 rules), 0008 (fail-closed header, protected names).

**First step for the next session.** Confirm the owner's pushes (handoff §2). Then, in `~/Documents/compiler` (the
owner may have another branch checked out: never check out in the main tree), fast-forward `cerastes` to
`b9519388ea` (`git fetch . b9519388ea:cerastes`), move the pin, rebuild, register `RAF_NO_CONSTANT_EVALUATION`,
run `check.sh`, and do the C6 follow-ups listed above; then the CI session.

## EDG bugs 0003–0007 (2026-10-15): fixed; branches ready for the owner's PRs

**Done.** Each bug has a fix branch in `~/Documents/compiler`. Each is based
on upstream `main` `d6f83174c7`, except 0003, which is stacked on 0004. The
fork's `cerastes` merges each one with `--no-ff`, and cerastes pins the
result. The pin is now `4ff94f065c`.

| Bug | Branch (for the PR) | What |
|---|---|---|
| 0005 | `0005` = `7173149c6b` (owner re-committed, GH #307, PR open) | no `__BLOCKS__`/`__block` in the macOS clang bases |
| 0006 | `0006` = `7b4d725551` | interference-size macros in the macOS clang bases |
| 0007 | `0007` = `b49d77f810` + `3f91f945f9` (squash) | `GNU_VECTOR_TYPES_ALLOWED` on macOS |
| 0004 | `0004` = `35e10894c9` | `enable_if` parameters after a function parameter pack |
| 0003 | `0003` = `1419c40a6d`, on `0004` | function parameter packs in `enable_if` conditions |
| 0016 | `0016` = `46d897f6b0` | adjacent function parameter packs (0003 needs it) |

- With 0005–0007, all 29 libc++ headers tried parse with stock `cpfe` and
  with `cerastes-fe` on SDK 27.0. `tests/edg-smoke/libcxx_headers*` keep it
  so, and the Installation page and decision 0001 say so. `<simd/simd.h>`
  does not parse (0013).
- cerastes now requires `GNU_VECTOR_TYPES_ALLOWED` (IL layout):
  `scripts/check_edg_config.sh` and `src/backend/edg_il.h`.
- The handoff (§6, §7) gains the M3/M4 notes on vector types and dependent
  subscripts.
- Every fix had:
  - an Opus review, often several rounds;
  - fail-before/pass-after natively and in Docker;
  - Docker `changes,regressions` in both configurations with no mismatch.

  0004 and 0003 were also mutation-tested and checked against clang with a
  probe matrix (C++11–23). `check.sh` is green at each pin.
- 0004 and 0003 change only the `enable_if` path. The fork series still
  rejects pack references in registered attributes (commit B).

**Not done / owner decisions.**
- Owner, 2026-10-15: fix only EDG bugs that block cerastes. 0008–0015 and
  0017–0019 were found along the way and are recorded, open, unfixed.
- 0003 has known limitations, stated in its message: packs inside a
  requires-expression in the condition, a rare nested-type shape, and
  reference/forwarding packs (`Ts&&...`), whose calls are now rejected
  (0015).

**Surprises.**
- 0004's first version leaked its renumbering into requires-expressions and
  concept satisfaction. It rejected valid code and poisoned cached concept
  results, and the Docker suites did not catch it. The review did.
- Enabling vector types natively exposed upstream asserts in the
  C-generating back end (0011) and gaps in vector support (0012–0014). None
  affects cerastes-fe.
- The owner re-commits branches with a one-line message and a `GH<N>` test
  name when filing. A branch's commit then differs from the one merged into
  the fork's `cerastes` (as with 0001/A).

**Evidence.** In the fork's `.tmp/agent-bugs/`: per bug, the analysis, the
review probes and the mutants. Remove it when no longer needed.

**Owner steps.**
1. Push the fork's `cerastes` (`4ff94f065c`), then cerastes `main`. `check.sh`
   reports NOT YET PUSHED until then.
2. PR the branches. 0004 must land before 0003. Squash 0007's two commits.

**Questions for the owner.**
1. Lift the fork series' rejection of pack references in registered
   attributes, now that 0003/0004 show how (the same renumbering in
   `copy_of_attributes_with_substitution`)? *Recommended: not before M4
   needs contracts on variadic templates.*
2. File 0016 upstream with 0003, or separately? *Recommended: separately and
   first, since it needs no `enable_if`.*

**First step for the next session.** Unchanged from M2: confirm the pushes,
merge upstream `main` into the fork (0009), move the pin, run `check.sh`.
Then the CI session before M3.

## M2 — Annotation layer (2026-10-14 to 2026-10-15): done locally; pushes pending

Read `internal/handoff.md` next. Its §2 lists the owner's push steps in
order, and §10 has the questions.

**Owner decisions this milestone**
- EDG work is a local commit series on the fork's `cerastes` branch; the
  owner pushes.
- **cerastes pins the fork** (decision 0009, 2026-10-14). It replaces
  "upstream only". Each session starts by merging upstream `main` into the
  fork (never rebase).
- The macros are named `CERASTES_*`.
- Loop invariants are prefix only.
- `forall` takes its free variables as trailing arguments.
- Redeclarations must match (the VeriFast rule).
- M1's probes were renamed from `cerastes::` to `unreg_ns::`.
- The full EDG `all` suite on the x86-64 box is the owner's step.

**Done**
- **The EDG fork series** A, FILT, B, C, D0, D, F, E, G, H, I, J and K, on
  base `034385ba0a`, with the pin at K (`23f55e3628`). It adds:
  - a general attribute-registration mechanism, with no `cerastes` string
    in EDG;
  - attributes after the parameter list attached to the routine;
  - must-match across redeclarations and explicit specializations;
  - class-template substitution;
  - result names;
  - substitution that fails closed.

  Each commit has `GH-*` tests, fail-before against its parent, and a
  `changes,regressions` run against the expectations (handoff §3).
- **cerastes registers its eight attributes** from the driver
  (`src/backend/register_attributes.cpp`). The walker prints each one, on
  its entity, with a typed expression tree: dependent in prototypes,
  substituted in instances.
- **24 cerastes-side checks** (`src/backend/diagnostics.def`) reject what
  EDG cannot. They fail closed on anything the listing cannot represent.
- **`<cerastes.h>`** has the `CERASTES_*` macros and the declared-only spec
  functions. It keys on `__has_cpp_attribute(cerastes::requires)`. The
  same tests build with Apple clang and GCC 16 under
  `-Wall -Wextra -Werror` (`native-ok`).
- **Tests.** `tests/annotations/` has 242 files:
  - 188 negatives, each with a twin;
  - positives with goldens, each checked against `--il_display`.

  The harness gained `frontend-refused`, `native-ok`, canaries and
  `--check-headers`. There are 675 self-tests.
- **Docs.** The Annotations page and "Building with ordinary compilers";
  every example is a test.
- **Upstream draft:** `internal/upstream/attribute-registration.md`.
- **Audits.** CP1 to CP8b (Opus). Every blocking finding is fixed; the
  open should-fix items are in handoff §4 item 8.

**Acceptance.** See handoff §9.
- `check.sh` is green.
- The annotation tests and doc examples pass, with native-ok on Apple clang
  and GCC.
- The docs build with zero warnings.
- **Pending the owner's pushes** (fork, then cerastes, then the website):
  publication at https://cerastes.dev/cerastes/doc/.

**Not done / deferred**
- **The full EDG suite** (`imported,edg,cwg,stdlib`, both x86-64
  configurations) has not been run at K. The run at J found the crash that
  K fixes; it is the owner's step on the x86-64 box (handoff §2).
- The mutation runs for H and I were never repeated after their final
  versions.
- The open CP7/CP8 items (handoff §4 item 8).
- CI for the repository (a session before M3).

**Surprises**
- **The full suite found a front-end crash** in our own series. G stored,
  and I dereferenced, an uninitialized template symbol for explicit
  specializations declared inside a class template. It crashed the x86
  ASan build on `gnats-23971*`, `d00244a` and `ms_cpgen01`. Native arm64
  survived by luck, and `cerastes-fe` runs the same path. Fixed by K. The
  lesson: `changes,regressions` is not enough before a pin moves.
- **The audits found real bugs at every round.** Silent drops, misattached
  attributes, parameter numbering after packs, crashes, and an EDG change
  that altered ordinary overload resolution (the first J, amended before
  it was pinned).
- **Narrowing a check without an audit regressed it** (CP8). The shadow
  checks now fail closed.
- **The session's scratchpad was lost at a context reset.** The per-commit
  evidence and the auditor's oracle went with it. Evidence now lives in
  the fork's `.tmp/agent-*` (handoff §3).

**Decision records added:** 0005-edg-source-override,
0006-annotation-language-v0, 0007-edg-attribute-registration,
0008-cerastes-header, 0009-edg-fork-pin.

**First step for the next session.** Confirm that the owner has pushed the
fork's `cerastes` branch, then cerastes `main`, then the website (handoff
§2). Then merge upstream `main` into the fork's `cerastes` (0009), move the
pin and run `check.sh`. Then the CI session (a decision record; a macOS
runner first, x64 Docker if the goldens drift), before M3.

## M1.5 — User documentation site (2026-10-14): done locally; live deploy pending the owner's push

Read `internal/handoff.md` next.

**Owner decisions this session**
- **Q1–Q8.** The answers to the M1 handoff questions are folded into
  0001, 0003, CLAUDE.md and plan.md (commit `78c8010`).
- **No docs-only repo** (Q4). The docs are this repository's own Pages site.
- **The address** is **https://cerastes.dev/cerastes/doc/**. The org
  convention is `cerastes.dev/<repo>/doc/` (decision 0004).
- **C++ in AsciiDoc prose** is written `pass:[C++]`.
- **Developer docs** move from the `docs` directory to `internal` (a
  separate mechanical commit after the M1.5 commit).
- **Scope for the EDG proposal.** VeriFast parity and beyond, with specs in
  C++ grammar inside `[[cerastes::…]]` and no new EDG grammar.

**Done**
- **Antora component in `doc/`.**
  - `antora.yml` (`ROOT`, unversioned);
  - `local-playbook.yml` (builds the worktree into `build/doc/`, fails on
    warnings, lunr search);
  - `package.json` and its lockfile (Antora 3.2.1 and lunr 1.0.0-alpha.13,
    exact pins);
  - `.node-version` (26).
- **UI.** The default Antora UI, vendored as `doc/ui/antora-ui-default.zip`
  (SHA-256 checked, and forced with `--ui-bundle-url`), plus supplemental
  files in `doc/ui/supplemental/`:
  - an Ink navbar with the lockup;
  - Archivo;
  - the favicons;
  - Sand used only as a graphic on light grounds;
  - links in a docs-only `#7f5f2d`.

  The assets are byte-identical copies from the brand kit and the website.
  Provenance is in `doc/ui/README.md`.
- **Pages.** Overview (what cerastes is, the pipeline, honest status) and
  Installation (requirements, bootstrap, check, trying the front end,
  building the docs, known limitations). Every claim was checked: a claims
  ledger by il-analyst, then re-verification by the auditor.
- **Examples are tests.**
  - `doc/modules/ROOT/examples/installation/hello.cpp` (`frontend-ok`).
  - `run_tests.py` discovers `doc/modules/ROOT/examples/` by default, and a
    missing default directory is an error.
  - `scripts/check_doc_examples.py` is the source lint. Its rules (a)–(e) and
    their extensions are in `tests/README.md`.
  - `--html build/doc` is the rendered backstop.
- **Scripts.**
  - `scripts/build_docs.sh [PLAYBOOK]` is new.
  - `bootstrap.sh` installs Homebrew `node` and runs `npm ci --ignore-scripts`
    in `doc/`, behind a stamp of the lockfile and `package.json` that is
    checked against `node_modules/.package-lock.json`. With `--build-only`, a
    stale install is an error.
  - `check.sh` adds the source lint to its lint step, and a final docs step:
    the build, then `--html`.
- **CI** (`.github/workflows/docs.yml`).
  - Pull requests run the lint, the build and `--html`.
  - Pushes to `main` that touch `doc/**` or the doc scripts do the same, then
    deploy to Pages. The Antora site is staged at `doc/` in the artifact, with
    a root redirect and a root `404.html`.
  - Actions are pinned to commit SHAs, on `ubuntu-24.04`. Deploys share a
    concurrency group, so they land in order. Least privilege, and checkout
    does not keep its credentials.
- **Self-tests** went from 140 to 305. Each new check has a twin. The
  end-to-end twins build real Antora sites and show three things:
  - warnings fail `build_docs.sh`, but not bare Antora;
  - lint-evading pages (q1–q18, e1/e2/e5) fail the source lint, `--html`,
    or both;
  - the `check.sh` and CI wiring cannot be weakened (`|| true`, comments,
    `continue-on-error`).
- **Upstream draft for the owner.** `internal/upstream/attribute-registration.md`
  plus 16 probes (handoff §5).
- **Decision records.**
  - 0004-docs-site (accepted).
  - 0001 gains the docs pins and the GitHub Actions SHAs, and corrects the
    libc++ limitation: `<utility>` fails too, and there are three causes.
  - 0003 records the owner's push and pointer answers.

**Acceptance.** See "M1.5 acceptance" in `internal/handoff.md`.
- **Met locally:** zero-warning docs build in `check.sh`; every doc example
  runs as a test; branding matches cerastes.dev. The screenshots are
  `internal/images/m1.5-docs-overview.png`, served under `/cerastes/doc/`, and
  `internal/images/m1.5-website-reference.png`.
- **Pending the owner's push:** the live URL returns 200, search works live,
  and a docs-only commit appears with no manual step.

**Not done / deferred**
- CI for the rest of the repository; the owner wants it soon, on x64 Docker
  if macOS runners cannot reproduce the goldens. It needs a decision record.
- The "Documentation" link on cerastes.dev (website repo, owner).
- Fast-forwarding the fork branch (owner).

**Surprises**
- `https://cerastes.dev` had no certificate for the custom domain (a
  Cloudflare-proxied `www`); `.dev` is HSTS-preloaded. The owner fixed it.
- Antora resolves non-`./` playbook paths against the current directory.
- A bare `C++` in AsciiDoc swallows text silently.
- `<utility>` was misreported as parsing in M0.
- The default Antora UI hard-codes its colours, so `cerastes.css` overrides
  selectors.

**Decision records added:** 0004-docs-site.

**First step for the next session (M2).** Read `CLAUDE.md`, the M2 section
of `plan.md`, this file, `internal/handoff.md` §5 and the upstream draft
`internal/upstream/attribute-registration.md`. Check whether the owner has
fast-forwarded the fork branch and filed the Discussion (handoff §8). Then
write M2's decision record for the attribute design (invariant 6), sized for
the full scope in the draft.

## Decision 0003 (2026-10-13): EDG fork branch

The owner created branch `cerastes` on the fork
https://github.com/mborland/compiler. `internal/decisions/0003-edg-fork-branch.md`
records how it is used:
- every EDG source change is made there, never in `third_party/edg`;
- the submodule only ever pins upstream `main` commits, and no commit of this
  repository records a fork pointer;
- the branch is based on the pinned upstream commit, and is separate from
  the `cir` branch;
- each commit follows EDG's `CONTRIBUTING.md` checklist, and general
  mechanisms are preferred to cerastes-specific code inside EDG;
- cerastes code that needs an unmerged EDG change waits until the pin moves.

**Owner rules (2026-10-13):** sessions may commit and push to the fork's
`cerastes` branch. Sessions never open issues, PRs or Discussions against
upstream; the owner files them from text drafted in `internal/handoff.md`.
Recorded in 0003 item 6, CLAUDE.md and the `edg-backend` agent.

**Not done:** the branch tip is `90b66472`, the fork's old `main`, 22 commits
behind the pin `9847356d`. It must be fast-forwarded before its first commit.
This session had no GitHub push credentials, so that is handoff question 1.
From now on, every handoff ends with a "Questions for the owner" section
(`internal/handoff.md` §8).

**Decision records added:** 0003-edg-fork-branch (accepted). G1 passed, so
the fallback record plan.md had reserved 0003 for was never written. A
fallback needed after a later pin move takes the next free number.

## M1 — EDG IL inventory and integration spike (2026-10-13): done, Gate G1 PASS

Read `internal/handoff.md` next. It has the re-look list, the friction log and
what M1.5, M2 and M4 inherit.

**Done**
- **Integration (decision 0002, accepted).** cerastes compiles EDG's
  front-end sources itself, in the "cerastes configuration", and links an
  in-process `back_end()`. Nothing in `third_party/edg` changes; the pin is
  still `9847356d`.
  - The configuration: no lowering, `ALL_TEMPLATE_INFO_IN_IL`,
    `RECORD_UNRECOGNIZED_ATTRIBUTES`, `EXTRA_SOURCE_POSITIONS_IN_IL`, used
    templates instantiated in the TU, and no removal of unneeded entities.
  - The build:
    - top-level `CMakeLists.txt`, `cmake/EdgFrontEnd.cmake`;
    - the `edg_config` interface target, the only place the EDG macros are
      defined;
    - `edg_fe`, compiled with stock cpfe's exact flags;
    - `src/backend/`, `src/driver/`.
  - The output is `build/cerastes-fe`, built by `scripts/bootstrap.sh` and
    run with `scripts/cerastes-fe.sh`.
  - Guards: `static_assert`s in `src/backend/edg_il.h`, and
    `scripts/check_edg_config.sh` as a `check.sh` step.
- **IL walker** (`--cerastes_walk`). It prints a deterministic listing:
  - routines (plain, prototype, instance, specialization), with body or
    nobody;
  - signatures, template arguments and constraints;
  - every unrecognized or `cerastes::` attribute with its owner;
  - attributed statements;
  - the operations and template-parameter constants in prototype bodies,
    including those nested in `tpck_expression` constants;
  - a final `end walk:` line that the harness requires;
  - cross-check warnings for G1 regressions (`--cerastes_walk_selftest`
    proves two of the three can fire; the third covers pruned bodies, and
    `back_end()` refuses to run in the pruning mode).
- **Harness.** New things in `scripts/run_tests.py`:
  - the `// expect: golden` kind, which compares stdout plus a stderr
    section;
  - `--update-golden`, which `check.sh` never runs, enforced by a self-test;
  - orphan-golden detection;
  - a `.harness-skip` marker, allowed only in `tests/differential/verifast`;
  - `CERASTES_REQUIRE_E2E=1` in `check.sh`, so end-to-end self-tests fail
    instead of skipping.

  `scripts/test_run_tests.py` has grown from 23 to 140 self-tests,
  including mutation twins for every harness check.
- **Tests (`tests/il/`).**
  - G1 probes `g1/check{1..5}`.
  - Annotation-free copies of the six VeriFast template tests,
    `verifast-plain/`.
  - Inventory probes `inventory/`.
  - Walker smoke tests `walker/`.
  - Contrast probes `contrast/`, which rule out vacuous passes.

  All are golden tests. Each golden was checked against `il_display` before
  it was cut, by an agent other than the walker's author: il-analyst for g1,
  verifast-plain, inventory and walker; test-mutator for contrast. The
  auditor independently matched every prototype's `op` lines in all goldens,
  and the attribute lines of the g1, inventory and sampled contrast goldens. The VeriFast
  originals are byte-identical in `tests/differential/verifast/` (generic2
  `0fb63506`) and skipped by the harness.
- **`internal/il-inventory.md`.**
  - The five G1 checks, with `il_display` evidence.
  - Every Clang query used by VeriFast's `GenericFunctionChecker`,
    `collectIntegralTypeParams`, `usesGenericProof` and the call-site and
    specialization code (74 rows), mapped to its EDG IL field.
  - The gotchas for M2 and M4.
- **Agents.**
  - `.claude/agents/edg-backend.md` and `il-analyst.md` are new, Sonnet.
  - `soundness-auditor` is now Opus, read-only, and also reviews gate
    evidence and harness changes.
  - `test-mutator` is now Sonnet, owns the harness, and is told that golden
    files are expectations.

**Gate G1: PASS.** Proceed to M2.
- Check 1 PASS: `v + 1` is `eok_add` with `type_kind tk_template_param`, and
  `(T)` is its own `eok_cast`.
- Check 2 PASS: a template that is never instantiated has a prototype
  routine with a full body.
- Check 3 PASS: instances carry `assoc_template` and deduced or explicit
  `template_arg_list`.
- Check 4 PASS: `std::integral` is reachable from all four constraint
  locations.
- Check 5 PASS for every placement the plan lists: prefix on a function
  declaration, loop statements and null statements, each plain, in a
  function-template prototype and in its instance.
- The placement after the parameter list, where M2 puts `requires`/`ensures`,
  is not carried reliably by the unrecognized path. It lands on the function
  type and is lost on redeclarations. So M2 takes plan.md's "check 5 fails"
  branch for that placement and registers the attributes explicitly. The Opus
  auditor reproduced the evidence and agreed with this verdict in review
  passes 2, 3, 3b, 3c and 3d. The front-end fallback record is not needed.

**Acceptance (all met):** see "M1 acceptance" in `internal/handoff.md` for the
commands and their results.

**Not done / deferred**
- VeriFast is still not built (0001); M4 decides.
- The walker reports only the primary source file. Code in user headers is
  counted, not shown, and M3 needs a policy for it.
- No `doc/` page. M1 adds no user-visible behaviour; `cerastes-fe` is a
  developer tool.

**Surprises**
- Turning off EDG's back ends silently turns on automatic template
  instantiation (`.ti` files, `tim_none`). The stock `cmake_defines.h` also
  pre-empts `host_envir.h`'s derived defaults. So every load-bearing macro is
  pinned, including `DEFAULT_REMOVE_UNNEEDED_ENTITIES=0`, without which
  templates that are never instantiated could be pruned.
- `il_display` does not print attributes attached to statements, or some
  template-parameter constants. A grep of it once produced a false "attribute
  lost" report.
- `(T)100` is not an `eok_cast`. It is a `tpck_expression` constant wrapping
  a `tk_unknown` cast, which M4's fail-closed rule must special-case
  narrowly.
- Class-template members do keep unrecognized attributes, despite
  `attribute.c:10906`.
- New `.claude/agents` types are only loaded at session start.

**Decision records added:** 0002-backend-integration (accepted).

**First step for the next session (M1.5, by plan order).** Read `CLAUDE.md`,
the M1.5 section of `plan.md`, this file, `internal/handoff.md` §4 and
`internal/decisions/`. Then write `internal/decisions/0004-docs-site.md` (0003 is
the EDG fork-branch record; see the entry above). If the owner wants M2
first, do this instead: once the fork branch `cerastes` has been
fast-forwarded to the pin, start the upstream issue and PR cycle for
registering the `cerastes::` attributes (decision 0003, handoff §5). Nothing in M2 can
depend on that registration until it is upstream.

## Plan update (2026-10-13): user documentation with Antora

Added **M1.5 — User documentation site (Antora)** to `plan.md`. User docs are
an Antora component in `doc/`, published at https://cerastes.dev/doc/. The
default hosting is an org repo named `doc`, whose GitHub Pages project site
GitHub serves under the org's `cerastes.dev` domain; it is recorded in
decision 0004 when M1.5 runs. New global invariant 9 (CLAUDE.md invariant 8):
user-visible behavior is documented in `doc/` in the milestone that adds it,
and every doc example is a test. M2–M7 each gained a docs task. The
annotation reference moves from the planned `internal/annotations.md` to `doc/`.
M1 is unaffected. M1.5 depends only on M0 and must run before M2. Creating
the `doc` repo, enabling Pages and adding the dispatch token are owner steps.

## M0 — Bootstrap and toolchain (2026-10-13): done

**Done**
- Repository skeleton: `LICENSE.txt` (Apache-2.0 WITH LLVM-exception),
  `.gitignore`, `README.md`, `CLAUDE.md`, `.claude/agents/`
  (`soundness-auditor`, `test-mutator`), `internal/status.md`,
  `internal/decisions/0001-toolchain-pins.md`, `tests/README.md`.
- Placeholders renamed in `plan.md`: `vcpp` → `cerastes`,
  `[[verifier::…]]` → `[[cerastes::…]]`, `<verifier.h>` → `<cerastes.h>`,
  `namespace verifier` → `namespace cerastes`, `__VCPP__` → `__CERASTES__`.
  "verifier" as an ordinary noun and the `VF_*` macro names were left as they
  are.
- Submodules: `third_party/edg` = upstream `edgcpp/compiler` at its `main`
  head (`9847356d`); `third_party/silicon` = `v.26.08-release` (`6b540aa4`),
  with nested `silver` at `f02e7010`. Invariant 5 in `plan.md` and
  `CLAUDE.md` now says the EDG pin is always an upstream commit.
- `scripts/bootstrap.sh`: idempotent. Installs the Homebrew prerequisites
  (cmake, ninja, python, shellcheck, sbt, openjdk@17); initializes missing
  submodules without moving checked-out ones; builds EDG out of tree
  (`build/edg`, stock `macos-arm-clang-debug` preset); runs `sbt assembly`
  for Silicon, skipped when a build stamp shows the jar is current; downloads
  Z3 4.16.0 and checks its SHA-256; writes `build/env.sh` (`Z3_EXE`,
  `SILICON_JAR`, `CERASTES_JAVA`, `CPFE`, `EDG_BASE`, scraped clang include
  paths). `--build-only` is the build step that `check.sh` uses.
- `scripts/cpfe.sh`: runs `cpfe` with clang emulation and the system include
  paths (C or C++ chosen by the mode option).
- `scripts/run_tests.py`: parses `expect:`/`mode:` headers; runs `.vpr`
  through Silicon (all errors reported) and `frontend-ok` C/C++ tests through
  `cpfe`; matches errors exactly by category, optional Viper reason and
  line; reports wrong line, wrong reason, missing and unexpected errors;
  always fails on timeouts, crashes and unrecognized messages.
  `scripts/test_run_tests.py` (23 tests) checks all of this, including
  end-to-end runs on mutated copies of `fail.vpr`.
- Tests: `tests/viper-smoke/{ok,fail,fail_kinds}.vpr`,
  `tests/edg-smoke/hello.{cpp,c}`.
- `scripts/check.sh`: build → lint (`bash -n`, shellcheck, `py_compile`,
  header check) → harness self-test → tests. Green.

**Acceptance (all met)**
- Clean clone: `git clone` of the M0 commit, then
  `scripts/bootstrap.sh && scripts/check.sh`, was green in 1 min 45 s. That
  run fetched the submodules from GitHub, configured and built EDG, built
  Silicon and downloaded Z3. Homebrew packages and the sbt/coursier caches
  were already warm, so a first run on a new machine takes longer. A second
  `bootstrap.sh` in the same tree is a 1.5 s no-op.
- `fail.vpr` is reported as `postcondition:assertion.false @ L12`, and
  mutating its expected line or category makes the harness fail.
- `cpfe` parses a C++20 and a C23 hello world with no diagnostics.

**Not done / deferred**
- `scripts/lint_viper.py` and `scripts/mutate.py` belong to M3; `check.sh`
  has a lint step ready for them.
- `.c`/`.cpp` tests with `verified`/`error`/`rejected` expectations fail with
  "needs the cerastes driver (M3)" until the driver exists.
- VeriFast is not built. The local `generic2` binary is a Linux x86-64 ELF;
  M4 decides how to run it (see 0001).

**Surprises**
- EDG at the pinned commit cannot parse `<iostream>`, `<string>`, `<vector>`
  or `<algorithm>` from the macOS SDK 27.0 libc++: errors in
  `__algorithm/find.h` and `simd_utils.h`. `<concepts>`, `<cstdint>`,
  `<type_traits>`, `<limits>`, `<utility>` and `<cstdio>` are fine, so the
  smoke test uses `<cstdio>`. Worth an upstream EDG issue; it will matter once
  tests include container headers.
- Silicon's pinned sbt 1.6.2 cannot run on JDK 27 (Homebrew's `openjdk`), so
  the toolchain uses `openjdk@17`.
- Homebrew's Z3 is 5.1.0, but Silicon is tested against 4.16.0, so Z3 is a
  pinned download.
- Silicon pads error indices (`[ 0]`) once there are ten or more errors.
  The harness handles this, and `fail_kinds.vpr` (11 errors) covers it.
- Silicon has no machine-readable error ids on the command line. The harness
  classifies errors by silver's fixed message text, and `fail_kinds.vpr`
  pins that table to the Silicon version.

**Decision records added:** 0001-toolchain-pins.

**First step for M1:** read `CLAUDE.md`, the M1 section of `plan.md`, this
file and `internal/decisions/`. Then decide where the cerastes EDG configuration
lives. Under the upstream-only pin, prefer a configuration that needs no EDG
source change (for example a macro-conf or defines block kept in this repo
and passed to EDG's CMake); otherwise upstream it first. Then copy the six
VeriFast tests from `~/Documents/verifast/tests/cxx/templates/` (branch
`generic2`, `0fb63506`) into `tests/differential/verifast/`. Those files have
no `expect:` headers, so `run_tests.py` must be taught to skip that directory
or they will fail its header check.
