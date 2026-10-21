# Handoff after M4.2a — forward goto, functions that do not return, assert (2026-10-21)

The next session reads this file after `CLAUDE.md`, its own section in `plan.md`, `internal/status.md` and
`internal/decisions/`. `status.md` has the formal entry. This file adds what a cold start would otherwise have to
rediscover. The M4.1 handoff is `git show 60d3fbe:internal/handoff.md`; the M3 close-out handoff
`git show a4da52c:internal/handoff.md`.

**Planning update (2026-10-21, after M4.2a).** The owner split the CI session: three sessions run before M4.2b,
**S1** the EDG upstream merge and pin move (box, §4.1), **S2** CI (§4.2) and **S3** the mutation triage (§5), each
planned at ≤ 4 h with at most three Sonnet agents (edg-backend, il-analyst, test-mutator) and one read-only Opus
soundness-auditor. If the orchestrator or any agent has a question, the session stops that work and asks the owner.
§4 and §5 hold the facts found while planning.

**S1 is done (2026-10-21, branch `edg-upstream-merge`; result in §4.1).** The fork's `cerastes` merges upstream
`main` `21065e551e` (`97c2141be9`, not pushed); cerastes pins it; box suites, tests, `.vpr` identity and `check.sh`
green; 0020's Linux failure explained (stale box binaries). **Owner steps after S1, in this order:**
1. Push the fork's `cerastes`: `git -C ~/Documents/compiler push origin cerastes` (GitHub has P `b9519388ea`; the pin
   `97c2141be9` is one merge ahead; `check.sh` reports `NOT YET PUSHED` until then, and S2's CI cannot fetch it).
2. Bring the branch into `main` and rebuild:
   ```
   cd ~/Documents/cerastes/cerastes
   git merge --ff-only edg-upstream-merge
   git submodule update third_party/edg
   scripts/bootstrap.sh --build-only
   ```
3. Push cerastes `main`. Then S2 (§4.2) can start.
*(Done by the owner, 2026-10-21.)*

**S2 is done (2026-10-21, branch `ci`, PR #1; result in §4.2).** `.github/workflows/ci.yml` runs `scripts/ci.sh`
(the owner's Command Line Tools asserted exactly, GCC 16.2.0, full bootstrap) and the full `scripts/check.sh` on
GitHub's `xcode-27` runner (decision 0016). **Owner steps after S2, in this order:**
1. Merge PR #1 into `main` (it carries your `59e1bf3` and the S2 commits; or locally `git merge --ff-only ci` in
   the main checkout and push `main`). The push to `main` starts the first CI run there (its own group; it is never
   cancelled).
2. After the first green run on `main`: make the `CI` job `check (macOS, Command Line Tools 27.0)` a required check
   on `main` (§11 question 6, answered yes).
Then S3 (§5).

## 1. State in one paragraph

The owner split M4.2 into **M4.2a** (forward `goto`, functions that do not return, `assert`), **M4.2b** (named
constants and enumerations) and **M4.2c** (locals without an initializer) (2026-10-21; `plan.md`, §8.1). **M4.2a is
done** on branch `m4.2a-goto-noreturn-assert` (based on `main` `518e0de`): user gotos that jump outward and forward,
`[[noreturn]]`/`_Noreturn` functions (new category `noreturn`), the trusted C library functions `abort`/`exit`/
`_Exit`, and `assert` from `<assert.h>` checked as `check`, with decision 0015, about 90 new tests, the VIR self-test at
230 cases and the docs. The branch's first commit moves the pin back to P `b9519388ea`: `main`'s `518e0de` had pinned
the pushed `0bcd9bf28a`, which lacks `RAF_NO_CONSTANT_EVALUATION`, so `main` did not build. The CI and mutation triage
sessions (§4, §5) have still not run; M4.2a went first at the owner's request. *(2026-10-21: the owner pushed the
fork's `cerastes` (P) and cerastes `main` (`243cf57`, the M4.2a branch tip); §2 is done.)*

## 2. Owner steps, in this order (all done, 2026-10-21)

1. **Push the fork's `cerastes`** (`git -C ~/Documents/compiler push origin cerastes`): GitHub has `0bcd9bf28a`; the
   pin P `b9519388ea` is 6 commits ahead. `check.sh` reports `NOT YET PUSHED` until then, and a fresh clone of
   cerastes cannot fetch the pin.
2. **Bring the branch into `main`.** The work is on `m4.2a-goto-noreturn-assert` in the worktree
   `.claude/worktrees/m3-closeout`:
   ```
   cd ~/Documents/cerastes/cerastes
   git merge --ff-only m4.2a-goto-noreturn-assert
   git submodule update third_party/edg      # the main checkout's EDG is at 0bcd9bf; the branch pins P
   scripts/bootstrap.sh --build-only
   ```
   `518e0de` "Update edg version" is what moved the pin back (probably a commit that picked up a stale submodule
   checkout). If you meant to pin `0bcd9bf`, say so: cerastes needs P.
3. **Push cerastes `main`.**
4. Unchanged: branch `0020` (`c539de4101`) kept for the upstream PR, its Linux failure unexplained (§4); the fork's
   `.tmp/agent-*` evidence directories.

## 3. What the session did

| Item | Outcome |
|---|---|
| Planning | M4.2 sized at about three sessions by exploration and a Plan-agent review; the owner split it (M4.2a/b/c) and pre-approved the five retirements, the `noreturn` category, `assert` as `check`, and the amended meaning of "verified" |
| Pin | restored to P on the branch (`48e4b45`) |
| Plan | `plan.md` M4 table and §8.1 here: M4.2a/b/c, with the review's findings for M4.2b and M4.2c |
| Decision 0015 | outward-and-forward `goto` (over VIR blocks after the encoder's flattening, with guards for generated blocks); noreturn (`Function::noreturn`, `ensures false`, `assert false` with `noreturn` at each reachable return and at the end; a user's `[[noreturn]]` is a trusted user contract only when no system header wrote the flag); the trusted libc table (`abort`/`exit`/`_Exit`: name, system file, C linkage, signature, flag); `assert` by the macOS expansion's exact shape (through folded constants); amends 0011–0014 |
| Probes and spikes | P19–P22, S11–S13, all confirmed (`il-inventory.md`, "M4.2a") |
| Code | `src/vir` (Goto/Label, `goto_problems`, noreturn lowering, assert-origin `Check`), `src/backend/encode.cpp` (user gotos and labels, label attributes on the entity, noreturn, the libc table, the assert matcher, `ensures_on_noreturn`), `verify_main.cpp` (#1305 suppressed) |
| Tooling | `lint_viper.py`: every `goto` outward and forward at token level; `mutate.py`: deletes `goto L;`, never a label, buckets a deleted call-free `assert(...)`; `noreturn` in the harness |
| Tests | 90 new files in `tests/scalar` (27 positives, 63 twins and rejections); `reject_goto.c`, `reject_noreturn.c` retired (owner); VIR self-test 204 → 230 lowering cases (75 → 87 rejection cases) |
| Docs | Verifying a program (goto, functions that do not return, `assert`, the meaning of a result, the rejected list), Error messages (`noreturn`), Annotations (`check` vs `assert`), Overview |
| Audits (Opus) | 0015: 1 blocking (a lint closed list for `ensures false` that rejected a user's trusted `ensures(false)`: dropped), 9 should-fix taken; code: 0 blocking, 2 should-fix taken (a noreturn flag written by a system header was trusted as the user's claim; missing tests); tooling re-audit: 0 blocking, 3 should-fix taken (the assert bucket with calls, `mutate.py --validate` on a new test, closed-list leftovers) |

## 4. The EDG session (S1) and the CI session (S2) (owner 2026-10-16, 2026-10-20, split 2026-10-21)

### 4.1 S1 — EDG upstream merge and pin move (box; about 2–2.5 h; branch `edg-upstream-merge`)

Needs the x86-64 box (memory `x86-box-edg-suites`; the box sleeps, ask the owner to wake it). Agents: edg-backend
(the fork merge, the box runs, the pin move), il-analyst (0020 from the box outputs), soundness-auditor (the merge,
the evidence).

1. **Merge upstream EDG `main`** into the fork's `cerastes` (`--no-ff`, never rebase; decision 0009). Upstream is
   `dedc820c91` (2026-10-21), 9 commits after `307002eae6`, all fetched in `~/Documents/compiler` as `edgcpp/main`:
   - `52a125e154` is our 0005 (`__BLOCKS__`), re-committed by the owner: `git range-diff` shows it identical to the
     fork's `7173149c6b`, so it merges as a duplicate;
   - `4545bf930c` (#229, `if constexpr` with non-brace statements, `lexical.c`): cerastes's `if constexpr` tests all
     use braces;
   - `ed0a9efb06` (#309) moves the position of cleanup actions to the closing brace, in `lower_il.c`: cerastes runs
     with no lowering (`src/backend/edg_il.h:35`), so positions should not move (the `.vpr` identity check proves it);
   - `dedc820c91` (#218, new since the M4.2a handoff) suppresses `noreturn_function_does_return` when a noreturn
     constructor or destructor is called (`statements.c`; the fork's hunk there is far away): the four warnings
     `tests/il/m3/p15b_noreturn.cpp` lists involve neither;
   - `5bfce0ac36` (#302, base-class initializer order); `3c62b61f23` moves the box's Docker images to
     `ghcr.io/edgcpp`; CI, docs and issue-template commits.
   Expect one textual conflict, `src/Changes` (keep both sides); any other conflict: stop and ask. Then the full box
   suites (`changes,regressions,imported,edg,cwg,stdlib`, both 64-bit configurations) before the pin moves; known
   baseline mismatches `cwg1835`, `cwg2881`. **Every box run starts from a clean build** (`rm -rf
   ~/edg-cerastes/build/gcc`): see item 2.
2. **Branch `0020` alone fails its own test on Linux** (M3 handoff §4): `GH-lambda-template-param-qualified.sft.cpp`
   cases 1 and 2, `FAIL--OUTPUT MISMATCH`, passes at the chain tip and natively. Explain it before the owner files 0020.
   **Hypothesis (planning, 2026-10-21): a stale incremental build on the box.** `edg-docker-test` reconfigures only
   when `build/gcc` is missing and otherwise rebuilds files newer than their objects; the box tree is rsynced with the
   Mac's file times. Run `0020d` checked out `c539de4101` over a Mac checkout of `0006`, so only `il.c`, `lexical.c`
   and `overload.c` got fresh times, while about 17 other files that differ from the previous box build (the fork
   series: `scope_stk.h`, `symbol_tbl.h`, `il_def.h`, `attribute.c`, …, which change struct layouts) kept stale
   objects. The test and its `.rto` expectations are identical at `0020` and at the tip, and no later commit touches
   the check. Confirm on the box: `~/edg-cerastes-runs/0020d/run.log` should compile only those three files; then a
   clean-build `run.sh 0020e c539de4101 changes` should pass. If it still fails: stop and ask (a real 0020 bug).
3. **Pin move**, locally while the box runs: keep `build/vpr` as the baseline, `git -C third_party/edg fetch
   fork-local` and `fetch origin` (its `origin/cerastes` is stale at `0bcd9bf28a`), check out the merge, `git add
   third_party/edg`, `scripts/bootstrap.sh --build-only`, `run_tests.py -j 8`, `diff -r -x MANIFEST <baseline>
   build/vpr` (byte-identical expected; any golden, `expect:` or `.vpr` difference: stop and ask). Then `check.sh`,
   decision 0001's EDG row, the 0020 bug file, an Opus audit of the merge and the evidence.

**Acceptance:** 0020 PASS on a clean build; the full suites at the merge with only the two known mismatches; every
test's `.vpr` byte-identical; `check.sh` green; no open blocking audit item.

**Result (S1, 2026-10-21, about 35 min of work plus box time).**
- **Merge:** upstream had moved again that morning to `21065e551e` ("Fix filter command escaping and script name",
  tests only: 285 files under `tests/tests/edg/cmeerw`), so the fork merges that tip: `97c2141be9`. Conflicts:
  `src/Changes` (both sides kept, the fork's first) and, unexpectedly, `util/Changes` and
  `util/make_predef_macro_table`: the fork carries the session's original 0005, upstream the owner's re-commit (#307);
  upstream's text taken (it drops the fork's comment). The Opus audit reproduced both as exact unions/upstream text,
  P→merge under `src/` equal to upstream's own delta, the fork series intact, no header changed.
- **Box (`run.sh` now builds from scratch and reports `BUILD FAILED`):** `U-full` at `97c2141be9`, all six suites,
  both configurations: match except the known `cwg1835` (`PASS--OUTPUT MISMATCH`) and `cwg2881`
  (`ABORT--OUTPUT MISMATCH`); changes 154/154, regressions 5/5, imported 9075/9075, edg 238/238, stdlib 3/3 (cp
  overlay).
- **0020:** run `0020d`'s incremental build linked stale fork-series objects against 0020's `il.c`; the link failed
  and `edg-docker-test` ran the previous box binaries (0020's previous revision, which lacks the `template`-keyword
  rule), so exactly cases f5 and f6 failed. A clean build of `c539de4101` matches `changes` (`0020e`) and
  `changes,regressions,imported` (`0020f`, which also covers the imported expectations 0020 changes) in both
  configurations. Written up in `internal/edg_bugs/0020-*.md`; the owner files it.
- **cerastes at the pin:** 2624 passed, 0 failed, 1 skipped (native-ok 642/642), as at M4.2a; 667 `.vpr` and maps
  byte-identical to P's (checked after the test run and after `check.sh`); `check.sh` green (15 min 43 s).
- **Audit (Opus):** 0 blocking. Taken: the 0020 write-up's mechanism (above), a clean Linux run of 0020's imported
  expectations (`0020f`), 0001's pin history. Notes: the fork keeps two tests for the same `__BLOCKS__` fix (its
  original `GH-no-blocks-predefine` and upstream's `GH307`; owner: leave it until a fork commit touches
  `tests/changes`); #229 matters once M5 encodes template instances (add a test with an unbraced discarded
  `if constexpr` branch then); `--dump_configuration`'s build date does not identify the pin.

### 4.2 S2 — CI (about 3 h; branch `ci`; needs S1 pushed)

Owner decisions (2026-10-21): GitHub's standard **`xcode-27`** runner (GA 2026-10-15: macOS 27.0.1, Xcode 27.0
`27A266a`, clang 21.0.0, SDKs 27.0 and 27.2, JDK 17, gcc-13/14/15, no sbt or shellcheck), the **full `check.sh`** on
pull requests, pushes to `main` and `workflow_dispatch`; the repository is public, so the runner is free. **The owner
pushes** the branch at checkpoints; the session watches the runs read-only (`gh run watch`, `gh run view
--log-failed`). Agents: edg-backend (`scripts/ci.sh`, the workflow), test-mutator (harness and self-tests),
il-analyst (drift analysis), soundness-auditor.

1. **Decision 0016-ci** first (the next free number; M4.2b's and M4.2c's records become 0017 and 0018): runner,
   triggers, per-ref concurrency (pull requests cancel, `main` never), `contents: read`; the toolchain match and its
   known difference (Xcode 27.0's clang against the owner's Command Line Tools `clang-2100.3.34.2`; SDK 27.0 selected
   and asserted, not 27.2; `EDG_CLANG_VER` 210000); Homebrew `gcc` with major 16 asserted (the owner's `g++-16`);
   caches for inputs only (sbt and Coursier, Silicon's `target` and stamp keyed on the silicon and silver gitlinks and
   the JDK, Z3 by asset and SHA, npm), never EDG or cerastes build outputs; drift policy: stop and ask, never edit an
   expectation to match CI; the x64 Docker fallback recorded, not built (glibc's `assert` shape `__assert_fail`, the
   macOS-only `posix_spawn_file_actions_addchdir` in `src/driver/silicon.cpp`, the Darwin-only `bootstrap.sh`).
   Amend 0004 ("CI runs only the docs") and 0001 (the runner, the new action SHAs).
2. **What CI must work around** (planning facts): a shallow submodule checkout lacks `refs/remotes/origin/cerastes`,
   so `edg_tree.py check` fails (fetch `+refs/heads/cerastes:refs/remotes/origin/cerastes`, blobless, enough history
   for `--is-ancestor`); `run_tests.py`'s default `-j` is cpu/2 (1 on 3 cores) and the `check.sh` lines for
   `run_tests.py` and `mutate.py --validate` are pinned by self-tests (`test_run_tests.py:3151`, `3165`), so the job
   count comes from an environment variable `CERASTES_JOBS` read by `run_tests.py` and `mutate.py` (with a self-test
   twin); `bootstrap.sh` needs Homebrew's `openjdk@17` and installs sbt and shellcheck; the EDG checkout is 3.7 GB
   (`tests/` 2.6 GB: a sparse checkout is the fallback if the disk is tight); 7 GB of RAM may need `-j 2`.
3. **`scripts/ci.sh`** (prints `uname`, `df -h`, the SDK, clang and gcc versions; sets `DEVELOPER_DIR`; asserts SDK
   27.0 and gcc 16; fetches the fork ref; `bootstrap.sh`; asserts a GCC in `build/env.sh`; `check.sh`), and
   **`.github/workflows/ci.yml`** (pinned `checkout` with `submodules: recursive` and `persist-credentials: false`,
   `setup-node`, `actions/cache`, `upload-artifact` for logs on failure; `timeout-minutes: 150`). Self-test guards in
   the style of docs.yml's (`test_run_tests.py` 3114, 3235–3270), each with a twin: `ci.yml` runs `scripts/ci.sh` with
   no `|| true`, `; true`, `set +e`, `continue-on-error` or `if:`; no `--update-golden`, no `CERASTES_EDG_SRC`;
   `ci.sh` runs `check.sh` unconditionally.
4. **Rehearsal** (= `check.sh` run #1): a fresh clone of the branch under `build/ci-rehearsal` (submodules with
   `--reference`) running `CERASTES_JOBS=3 scripts/ci.sh`. Then the owner's push, the run (60–90 min), at most two
   infrastructure fix rounds; drift on the runner: il-analyst diffs it, the session stops and asks.

**Acceptance:** a green `ci.yml` run on GitHub (URL recorded; the log shows SDK 27.0 and gcc-16; test counts equal to
the local ones, native-ok with both compilers); self-tests and `check.sh` green; no open blocking audit item.

## 5. The mutation triage session (S3; about 3.5–4 h; after the CI session, before M4.2b; branch `mutation-triage`)

**Planning facts and owner decisions (2026-10-21).** Independent of the box. Agents: test-mutator A (tooling:
`mutate.py`, `lint_viper.py`, their self-tests, `tests/README.md`), test-mutator B (strengthened tests, twins, list
proposals), soundness-auditor (the tooling diff, every new list entry); il-analyst only if a survivor needs IL
evidence.
- **Weak tests are strengthened in place** (owner): a surviving mutant of a file can only be killed by that file's own
  contract, so the file's `ensures`/`check` are tightened; its `expect:` lines are untouched, it must verify in every
  mode it declares, and each new clause gets a must-fail twin. Doc examples change their pages (`build_docs.sh`,
  `check_doc_examples.py --html build/doc`).
- **Size now:** `tests/scalar` has 140 positives, **3522 mutants** (`--list`, 90 s), 738 in the bucket (`--validate`);
  a full run at `-j 8` is about 45–90 min. Run it once, alone, after the tooling and the strengthening, capped at
  100 min; nothing else but text review meanwhile.
- **Trap 1:** a run over a subset reports every list entry whose file is not selected as "no such mutant is
  generated" and exits 1 (`m4.2a-mutate.log` has 125 such lines): check staleness only for selected files.
- **Trap 2:** a `doc/…` entry would fail `check.sh`'s `mutate.py --validate` (default `tests/scalar`; the line and
  `DEFAULT_PATHS` are pinned by self-tests): make `--validate` also generate every file the list names.
- **Corrections to the counts below:** the per-file leaver list adds up to 73, the reason breakdown to 74 (the static
  recompute settles it; import both modules, because `e04ce6e`'s `--list` alone starts a full run). The two
  `cond_div` files have **22** M4.1 proposals, not 34; only 10 are test weakness (the literals `3→2/4`, `1→0/2` and
  the deleted `d += 1`, five per file); deleting the `continue;` is a true equivalent (the last statement of the loop
  body), as are the measure, invariant, `20→21` and `n <= 6` mutants.
- **Tooling items** (with self-test twins): progress on stderr with `flush` via `submit`/`as_completed` in `run_all`
  (stdout stays as it is: the self-tests read it); traps 1 and 2; `assert(e)` modelled as a CHECK clause (0015) before
  the 12 "weaker assert" proposals are listed; audit A items 1–5 (§6); compile each clause once (`_holds`;
  `switch_nested.c` alone takes 60 s to generate); the duplicated test name in `test_mutate.py` (377 and 451: the
  case-label test never runs). Proposals in `m4.2a-survivors-proposed.txt` have three columns; the list takes two.
- **Stop rule:** more than about 10 new weak tests beyond those named here go to the handoff.
- **Acceptance:** the full run exits 0 (0 unexplained survivors, 0 tool failures, no `NO KILLED MUTANT`);
  `--validate` clean; the mutate and lint self-tests green; docs 0 problems; `check.sh` green; no open blocking audit
  item.

The classifier now buckets a contract mutant only when the grid models it (signed builtin integer parameters,
linear, no `==`/`!=` flip) and, for a stronger `requires` or a weaker `ensures`, the function has no use in the
file. 297 → 223 candidates in `tests/scalar`: 74 leave, none join (31 caller, 12 caller + unmodelled, 29 unmodelled,
2 bodyless). Recompute the list statically: generate with `git show e04ce6e:scripts/mutate.py` (old) and the current
one, and compare `Subject.safe`. Per file: `call_contract_only.c` 3, `call_in_expression.c` 6,
`call_precondition.c` 1, `call_result_conversion.c` 6, `check_stmt.c` 6, `compound_mixed.c` 6,
`compound_ushort.c` 4, `contract_wf.c` 3, `default_argument.cpp` 8, `default_argument_expr.cpp` 4,
`measure_function_expression.c` 4, `measure_lexicographic.c` 2, `measure_mutual.c` 6, `measure_not_needed.c` 2,
`recursion.c` 2, `shift_promoted.c` 5, `ushort_scale.c` 5.

Steps: run `python3 scripts/mutate.py tests/scalar -j 8`; each surviving leaver is killed (a new test, never an
edited expect line) or listed in `tests/mutants-equivalent.txt` with a reason, reviewed by the Opus auditor. Audit A
expects 7 of the 8 caller-ensures leavers to be killed already; `call_result_conversion.c:9:40 100->101` is safe.
Fold in audit A's should-fix items (§6) at the same time. Until then M3's "0 unexplained survivors" holds for the
old bucket only.

**M4.1's survivors (added 2026-10-20).** The mutation run on the 31 new positives (`python3 scripts/mutate.py -j 8
<files>`; 1403 mutants: 991 killed, 299 in the safe-direction bucket, 27 stillborn) left **70 survivors**, which the
session's stop rule moved here. test-mutator's proposals are in `internal/triage/m4.1-survivors-proposed.txt` (id, tab,
reason; copied from `build/m4-staging/` in M4.2a). 34 of them are in `continue_cond_div.c` and
`do_continue_cond_div.c`: test weakness, not equivalence (deleting the `continue;` survives) — strengthen those two
positives (an exact `ensures` on a result the continue branch changes; the twins may keep a weaker invariant so that
the reported error stays `division-by-zero`, said in their headers). The other 36 look like genuine equivalents or
safe-direction mutants (the last `break` of a switch, dead initialisers, measures of loops that never repeat, slack
bounds, `while (1)` → `while (2)`, weaker `ensures` on unsigned parameters, which the classifier does not model);
each needs the auditor's review before it enters `tests/mutants-equivalent.txt`.

**M4.2a's survivors (added 2026-10-21).** `python3 scripts/mutate.py -j 8` over the 27 new positives and 4 doc
examples (`internal/triage/m4.2a-mutate.log`): 913 mutants, 476 killed, 220 safe-direction, 13 stillborn, 200
survivors and 4 tool failures. The 15 survivors and 4 tool failures of `verifying/goto_exit.c` are gone: Silicon timed
out on its nonlinear contract, so the example was replaced by a linear one with an exact contract (30 mutants, 0
survivors). test-mutator classified the other 185 read-only, by rule per file, in
`internal/triage/m4.2a-survivors-proposed.txt` (id, class, reason): 139 equivalent, 31 safe-direction the
classifier does not model (asserts, `||` ensures, a bodyless callee's ensures), 15 weak tests, 0 suspicious. Most
equivalents are guards that make a function end the program on more inputs: the amended meaning of "verified" (0015
item 4) cannot see them. The weak tests are in `assert.c` (doc: pin `r == (a + b) / 2`), `noreturn_fixed.c` (doc: add
`ensures r > 0`), `goto_in_loop_body.c`, `goto_cleanup.c`, `goto_out_of_loops.c`, `exit_main.c`. Each entry needs the
auditor's review before it enters `tests/mutants-equivalent.txt`. M4.1's 70 proposals are now tracked too:
`internal/triage/m4.1-survivors-proposed.txt`.

## 6. Should-fix items and known gaps

**Audit A (tooling), deferred to the triage session** (none affects the 223 current candidates):
1. Python's meaning of chained comparisons and of `not` differs from C's (`x < r < 1`, `!r < 1`, `r == x > 0` can be
   bucketed wrongly): allow `!` only on a parenthesized operand, one comparison per operand, or parse with C precedence.
2. A literal coefficient of `*` is not monotone (`3 * x + y < 10` → `4 * x …` incomparable but bucketed): make it
   `unknown`, or drop `*`.
3. Contract well-definedness after a dropped guard (0010 item 11): a drop-conjunct/swap-logic mutant can remove the
   guard of arithmetic in the clause or in a later `ensures`.
4. `has_caller` misses uses inside a `#define` body and in a default argument.
5. The termination lint is line-based; a clause split over lines (`decreases` / `*` on the next line) passes. The
   emitter writes one line per clause, so this is defence in depth.

**Audit B (pin, registration, tests).** Its blocking item is fixed and re-audited: a substituted lambda whose own
signature holds an error type (a `decltype(p)` parameter the body does not use) was accepted; now
`annotation_substitution_error`. The records it found inaccurate (0006, 0007, the Annotations page) are corrected.
Deferred (probes in the job's scratch directory are gone; the shapes are given here):
1. **False `spec_call_in_code` from leftover closure copies:** a predicate called inside a `forall` lambda in a
   *function template's* contract is rejected, even when the lambda does not name `T` (accepted in a non-template
   function and in a class-template member). Series L leaves one closure copy per instance (two for member
   templates) that no attribute reaches, so its call node is not exempt. Fails closed (a false rejection); fix with
   the EDG follow-up of question 2 or by exempting closures reachable only from a `lambda` record of a contract.
2. **The `operator=` branch of `spec_marker_on_special_member`** lost its only test (`spec_marker_on_assignment.cpp`
   now pins EDG's defaulted-function error): add a twin with a user-provided `operator=` marked `CERASTES_PURE`.
3. **P changes overload choice where constant evaluation is a substitution failure:** e.g. a return type
   `decltype(S<p(sizeof(T))>{}, 1)` or `enable_if_t<q(sizeof(T))>` makes EDG choose another overload than clang/GCC.
   Today each is rejected at its line, only because the back end's pass over all routine nodes finds the trace in
   the prototype: add two tests so narrowing that pass cannot make it silent.
4. **Tests for the back end's constant-context pass in never-instantiated templates** (bit-field width, enumerator,
   `noexcept` specifier, `explicit`, `alignas`, a block `static_assert` with a dependent operand): 2–3 tests.
5. **Existing gaps (not from this diff):** a class-scope `static_assert(q(sizeof(T)))` in a never-instantiated class
   template is accepted (never evaluated; harmless); a `CERASTES_PURE` override of a non-spec virtual function,
   called through a base reference at run time, is accepted (M7, virtual dispatch).
6. **The walker oracle** cannot tell an orphan closure copy from the used one (same name, same body in normal cases).

**Found this session:**
- **EDG at P leaves an error node without a diagnostic** in the substituted contract of a function template whose
  `forall` lambda takes `decltype(p)` of a function parameter (series L's rescan cannot substitute it). The back-end
  check `annotation_substitution_error` rejects it, so it is safe, but that check is now load-bearing, not only
  defence in depth. A fork follow-up should make EDG diagnose it (question 2).
- **Unreached cerastes-side checks:** the R4a constant-context IL traces in `src/backend/annotation_checks.cpp` (array
  bounds at namespace scope, bit-fields, `noexcept`/`explicit(bool)` specifiers, template arguments, `static_assert`,
  `case`, enumerators, `constexpr` variables, `if constexpr`, `alignas`) are no longer reached by a test: EDG errors
  first and the back end does not run. They stay as defence in depth (0006 says so).
- **Accepted, correct:** in a member of a class template, a `forall` lambda parameter written `decltype(member)` is
  accepted and substituted correctly (il-analyst, `--il_display` check); `sizeof(T)` in a lambda body is accepted.

**M4.1 code audit notes (2026-10-20, not blocking):**
1. *Closed in M4.2a:* `lint_viper.py` now checks every `goto` outward and forward at token level.
2. Three encoder rejection branches look unreachable and have no test (a `break`/`continue` whose target cannot be
   determined, a non-integer scrutinee, a case value of another type); they fail closed.
3. The probe/spike evidence of M4.1 is under `build/m4.1/` (gitignored; the facts are in `il-inventory.md`);
   two spike headers (S1b, S7f) state stale expectations, the record's claims are right.

**M4.2a audit notes (2026-10-21, not blocking; every should-fix item was taken):**
1. **Who wrote the noreturn flag.** The IL keeps only a routine's first declaration position and no list of its
   declarations (`GENERATE_SOURCE_SEQUENCE_LISTS` is off). The trust "declared noreturn here" therefore checks the first
   declaration and that no `noreturn` attribute entry sits outside a verified file. `_Noreturn` leaves no entry, so a
   `_Noreturn` written in a system header would not be seen. None exists in the pinned SDK (grep). The sound check is
   EDG's source sequence lists, a configuration change (decision 0002) for a later session. The same limit makes the
   libc table check "declared only in system files" on the first declaration only (0015 item 5; harmless, since
   must-match stops a user redeclaration from adding a contract).
2. **Unreached guards** (defence in depth, commented in `encode.cpp`): a declaration directly in a compiler-generated
   block that is not flattened (P20 found no such shape), and a `cerastes::` attribute on a label (EDG rejects it
   first, #1835).
3. **A compiler-generated goto or label without break/continue flags** is now treated as a user one, and
   `goto_problems` fails closed on it. Before M4.2a it was rejected; no program is known to produce one.
4. **Diagnostics.** Goto rejections use the generic `unsupported_statement` text, which ends "(decision 0012)", not
   0015. A misplaced GNU `__attribute__((noreturn))` is reported as `unrecognized_attribute` and spelled
   `[[noreturn]]` (EDG keeps it as unrecognized).
5. **Untested.** `_Noreturn int main` in C23 is rejected, but EDG's warning #3271 (a noreturn function with a non-void
   return type) also prints, which a `rejected` test cannot list; only the C++ variant is tested. The near-miss tests
   of a user `abort`/`exit` are C++ only: C compilers treat `abort`/`exit` as builtins, so they cannot be `native-ok`.
6. **`mutate.py --list`** took about 3 minutes under load (0.6 s at the M3 close-out on an idle machine).
7. The lint's existing "while without decreases" rule reads up to the first `{`, so a quantifier trigger inside an
   invariant would be misread as the loop body. The emitter writes no trigger today (test-mutator).

## 7. Known gaps carried

- Termination: a recursion cycle through another unit (0013 item 5) — the contract manifest, M6 (M7 if it does not fit).
- The `predicate`/`pure` marker on the first declaration — M5b (an EDG registration feature).
- Still rejected for `forall` in templates: a lambda depending on `T` only through `decltype` of a function parameter
  (above), function parameter packs (`attr_function_parameter_pack`), `sizeof...` of a template pack, a lambda that
  names the template it is in (`h` undefined). EDG bug 0021 does not arise: cerastes reads C++ in Clang emulation.
- Closed: the five compile-time uses of `constexpr` spec functions that left no trace (P, registered).
- Attributes in positions where EDG records nothing (0011 item 7); M3's rejected constructs (0012 item 12) for M4 and M8.
- From M4.2a (0015): a `goto` that enters a block, an `if` branch or a switch body (C allows it; Silicon's handling
  of a jump into an `if` branch is unproven, S9d); a label that is the whole branch of an `if` without braces; GNU
  computed `goto`; `quick_exit`, `__builtin_trap`, `__builtin_unreachable`/`unreachable()`, `std::terminate`; an
  `assert` in a `for` increment; glibc's `assert` expansion. Each is rejected with a diagnostic and an M8 entry.

## 8. Session plans: M4 (the rest of C) and M5 (generic templates)

**Re-sequencing (owner, 2026-10-20).** Planning M4 (generic templates) showed that Gate G2's six VeriFast tests use
pointers, references, structs and constructors, which M3's scalar fragment cannot express. The owner renumbered the
plan (`plan.md`, "Milestone renumbering"): the **new M4 is the rest of C** (strictly C, sub-milestones M4.1–M4.8,
pointers in M4.5a–f, VerCors as the reference); generic templates are **M5** (M5a–M5g); concepts M6; **C++ memory
and classes M7** (references, `new`/`delete`, classes; G2's parts that need them are deferred there); inventory M8;
evaluation M9. Session order: M4.1 → M4.2a (owner, 2026-10-21: before CI) → EDG upstream merge (§4.1) → CI (§4.2) →
mutation triage (§5) → M4.2b → M4.2c → M4.3 … M4.8 → M5a … M5g → M6.

**Rules for every session below** (memory `session-size-few-hours`): planned at ≤ 4 h with fixed scope and stop
rules; owner decisions and pre-approved expectation changes asked up front; the decision record before code
(invariant 6); must-verify/must-fail pairs (invariant 2); docs in the same session (invariant 8); three Sonnet agents
with disjoint files (edg-backend: `src/backend`, `src/driver`, build; il-analyst: probes, IL evidence, Silicon
spikes, `src/vir` + `src/viper`; test-mutator: tests, harness, `mutate.py`, `lint_viper.py`) and the Opus
soundness-auditor (one round per diff plus a re-audit of blocking fixes; should-fix items to the handoff);
single-file runs while working (`build/cerastes verify` directly: `run_tests.py` locks `build/vpr`); `check.sh` at
most twice; anything that needs the x86-64 box is its own session. Record numbers are the next free ones when written
(0014 is M4.1's).

### 8.1 M4 — the rest of C

**M4.1 Control flow** (done 2026-10-20; see `internal/status.md`).

**M4.2 Scalar declarations — split into M4.2a, M4.2b, M4.2c** (owner, 2026-10-21: planning found about three
sessions of work; a Plan-agent review found the cross-mode traps listed below). Owner decisions for all three:
retirements pre-approved (`reject_goto.c`, `reject_noreturn.c` in M4.2a; `reject_case_variable.cpp`, `reject_enum.c`
in M4.2b; `reject_uninit_local.c` in M4.2c — each deleted in the commit that lands its feature, replaced by
positives, error twins and rejection tests of what stays rejected; `reject_global.c`, a mutable global, stays); a new
category `noreturn`; a failing `assert(e)` is `check`; "verified" means returns satisfying `ensures` or ends the
program through a noreturn call (0013 item 6 amended).

**M4.2a Jumps and ends** (run 2026-10-21; record 0015). Forward `goto` and labels, "outward and forward" (the label
is a later statement of a block enclosing the goto, after the encoder's flattening of generated blocks; never into a
block, branch, switch body or loop; never past a declaration); `[[noreturn]]`/`_Noreturn` (`ensures false`, every
reachable return asserts false, category `noreturn`); trusted libc `abort`/`exit`/`_Exit`; `<assert.h>` `assert`
recognised by the macOS expansion's shape and checked as `check`. See `internal/status.md` for what landed.

**M4.2b Named constants and enumerations** (record 0017; 0016 is CI's).
- `const`/`constexpr` globals, constant-initialized, defined in a verified file: uses take EDG's folded value as a
  literal; a `globals__wf` method encodes each initializer by its original expression (used or not), asserts its
  obligations at the initializer, then asserts equality with EDG's value as `internal` (as `DoOnce` does). Also: C's
  `const int g;` (tentative, zero), const globals in contracts and annotations (`variable()` rejects them in
  contracts today, `encode.cpp:642`), `const int g = g + 1` (C++), a self-test hook perturbing the literal (exit 3).
  Rejected: `extern const` without a definition here, dynamic initialization, system-file globals, `volatile`,
  `static const` locals (M4.5b with the other statics).
- **Cross-mode trap (review, blocking):** a const global in a `do … while` condition is a folded constant 0 in C++
  (`DoOnce`, no measure) but an `enk_variable` in C (a measured loop): a global in the condition must never make a
  `DoOnce`, in either mode (0014 item 5's argument relies on it).
- `case k` (const local), `case N` (const global), `case RED` (enumerator): `vir::Var::is_const`, `check_program`
  rejects an `Assign` to a const local and accepts only const locals in case values (`check.cpp:317`), an `internal`
  guard per case value against EDG's `case_value`. Case-value obligations are path-dependent today (asserted before
  the dispatch, `lower.cpp:563-579`; an `if (0) switch` skips `case 1 << 40`, a C 6.6p4 constraint violation): assert
  them path-independently.
- Enumerations, one meaning (the stricter rule in both modes): an enum type's values are its C++ value range
  ([dcl.enum]/8, computed by cerastes; the underlying type's range when fixed), every conversion to an enum type is
  checked against it (`conversion`); enumerators are `int` constants (with a fixed underlying type, the enum's type,
  in C23 and C++); enumerator values not fitting `int` rejected; implicit promotion of a non-constant enum operand in
  arithmetic or ordering comparisons rejected (C promotes `unsigned int`-compatible enums to `unsigned int`, C++ to
  `int`), `==`/`!=`/`switch`/assignment/explicit casts allowed (mode-independent while every enum value is in its
  value range); compound assignment and `++`/`--` on enums rejected (they bypass EDG's conversion nodes in C, 0010
  item 6); a `bool` underlying type rejected; `enum class` (C++) accepted. Enumerators without an initializer have no
  `expr`.
- Probe P25: EDG's compatible type for C enums with and without negative enumerators (clang/GCC are not evidence),
  enumerator types per mode, fixed underlying types, the conversions EDG inserts; fix il-inventory's contradiction
  (L1084 "`tk_enum`" vs L1344 "`tk_integer` with `enum_type`").

**M4.2c Locals without an initializer** (record 0018). Recommended (review): definite assignment over VIR — one
forward pass suffices because every jump is forward (intersection at joins and labels; a loop head's state is its
entry state); no new category; the rejection names the read. Alternative: a ghost "defined" flag per local with
category `uninitialized` (flags false at method entry, since Viper locals start arbitrary, `emit.cpp:181`; reads
include compound assignment, `++`/`--`, arguments, returns and annotations; a monotone loop invariant for locals
declared outside the loop; the automatic range invariant, `lower.cpp:676-691`, must not fail on an unassigned local);
decide by spike. Small: may share a session with M4.3's start.

**M4.3 Expressions and sequencing.** The comma operator; assignments as values; `x++` inside expressions; calls in
guarded operands (`&&`, `||`, `?:`) and loop conditions (lowered as branches and re-evaluation); unsequenced calls
(C's sequencing rules: an unsequenced modification of one object is undefined behaviour and must be rejected or
detected). Record (sequencing), amending 0012 items 4 and 6. Mutation operators for the new forms.

**M4.4 Bitwise operators.** `& | ^ ~` and `&= |= ^=` on signed and unsigned types (C23 two's complement). Spike first
(Silicon `v.26.08`): total prelude functions with bodies over `Int` (bounded recursion on bits) vs an SMT bit-vector
interpretation domain (needs a lint rule and a record: invariant 4). Record (bitwise), amending 0010 item 9. Note for
M5: integral `T` still rejects bitwise operators; per-instance fallback now verifies them (`low_bit`).

**M4.5 Pointers** (one 4.x, sessions a–f).

VerCors summary (this session's study of `utwente-fmt/vercors` `dev`, 2026-10-20; ideas only — MPL-2.0, no files
copied): a C pointer is a value of a `pointer` ADT pairing an abstract `block` with an integer offset
(`res/universal/res/adt/pointer.pvl`); `ptr_deref(p)` = `loc(pointer_block(p), pointer_offset(p))` gives a Viper
`Ref`, and each element type has its own Viper field; a nullable pointer is `Option<pointer>` (every dereference
checks non-null); `ptr_add` is abstract with postconditions (a one-past-the-end pointer cannot be formed);
`\pointer(p, n, perm)` desugars to non-null, bounds and a quantified permission over `p[0..n)`; address-taken
locals become 1-element non-null blocks (`VariableToPointer`); structs are by-value classes whose fields are
pointers to field cells; `malloc` is accepted only as `(T*)malloc(n*sizeof(T))`, `free` exhales the block's
permissions; relational pointer comparison across blocks is false instead of an error. Caveats to avoid: the prelude
is axiom-heavy and three axioms look inconsistent outside their triggers (`pointer.pvl` stride axiom, `option.pvl`
`some(option_get(opt)) == opt` unguarded, `const_pointer.pvl` injectivity); `int` and `short` share one field (issue
#1357, effective types); `free(&local)` verifies (#1237); integer overflow unchecked by default; no `switch`, `do`,
enums, unions or string literals. VeriFast's C coverage (its own front end) is the parity target: points-to chunks,
`malloc_block`, `chars`, predicates with `open`/`close`, lemmas, `/*@truncating@*/` casts, string and `malloc`
contracts in `bin/*.h`; canonical examples to port are listed per session below.

- **M4.5a Design** (no encoder code): records for (1) the memory model — cells and typed value fields (one field per
  C type, never shared between `int` and `short`), the pointer representation (block + offset, or one ref per cell
  with an explicit block-length ghost), NULL, provenance and one-past-the-end, address-taken locals, how
  permissions map to `owns`/`reads`; (2) **the C spec syntax**: today `owns`, `reads`, `old`, `owns_range`, `forall`
  are C++ only (`include/cerastes.h:61-75`, 0008 item 6) and `forall` needs a lambda, which C lacks — options:
  declared functions taking `const void*` (the IL keeps the typed argument under the conversion), `_Generic`
  macros, or a binder-introducing attribute (EDG's result-name mechanism, 0007 item 6); (3) the new error categories
  (permission, null dereference, out of bounds, invalid free, …) and their messages in source terms. Silicon spikes
  for each candidate encoding, including the axioms each needs. If an EDG fork commit is needed, a box session is
  added before M4.5b.
- **M4.5b Pointers to scalars and globals:** `&x`, `*p`, NULL, pointer parameters with `owns`/`reads`/`old(*p)`,
  pointer equality, an aliasing trap that must fail, mutable globals and `static` locals as cells with frames; ports
  of VeriFast's `examples/swap.c`, `examples/address_of_local.c`, `examples/globals.c`.
- **M4.5c Structs:** field cells, `.`/`->`, struct values (copies, initializers, by-value parameters and results),
  nested structs, pointers to fields; ports of `tutorial_solutions/account.c`, `deposit.c`.
- **M4.5d malloc and free:** trusted contracts for `malloc`, `calloc`, `realloc`, `free`, `abort`, `exit` (system
  files are used only through contracts, 0011 item 4), allocation blocks, ownership on free, leaks (record); ports of
  `tutorial_solutions/malloc_block.c`, `examples/cell.c`.
- **M4.5e Arrays and pointer arithmetic:** local arrays, `a[i]`, `p + i`, `p - q`, comparisons within one object,
  `owns_range` and quantified permissions, `forall` in C; ports of `examples/carrays.c`, `examples/insertion_sort.c`,
  array sum and max.
- **M4.5f Predicates and pure functions:** predicate and pure bodies encoded (0011 item 4 lifted), `fold`/`unfold`,
  recursive predicates; a linked list (push, pop, length), `tutorial_solutions/stack.c`, `reverse`. Lemmas and ghost
  inductive data are open design points (`internal/upstream/attribute-registration.md` Appendix A): a record decides
  whether they are in M4.

**M4.6 Chars and strings:** string literals as read-only arrays, `char` buffers, `strlen`/`memcpy`/`memcmp`/`strcmp`
contracts, `main(argc, argv)`; ports of `tutorial_solutions/strlen.c`, `memcpy.c`, `memcmp.c`.

**M4.7 Function pointers:** contracts on function types, calls through pointers; ports of `tutorial_solutions/byref.c`,
`examples/functions/functions.c`.

**M4.8 C parity close-out:** the VeriFast C differential set (the ports above, run with VeriFast in Docker as M5c
sets up, or recorded outcomes), the full mutation run over the M4 tests, the Memory and ownership page complete,
M4's acceptance (plan.md), and the deferred list for M8 (unions, floating point, `volatile`/`_Atomic`/`_BitInt`/
vector types, variadic functions and `printf`, several translation units, concurrency).

### 8.2 M5 — generic templates (designed in the M4.1 session)

The M3 handoff §6 still holds: VIR's `IntType` needs a type form for `T`; templates in verified files are rejected
today (`encode.cpp:1462-1467` declarations, `:1495-1497` routines, `:926-929` calls); generic proofs read contracts
from the root template; contracts can use lambdas naming `T`. Latent hazard: a template instance whose body is in a
verified file takes `callee_for_call`'s has-body path (`encode.cpp:911-915`) and would become a bodyless VIR
function with no `trusted_reason`; only the declaration-level rejection keeps it unreachable today — M5e must handle
instances before that rejection is lifted.

**M5a Eligibility** (cerastes-only; verification results unchanged).
- `src/backend/generic_eligibility.{h,cpp}`: `check_generic_eligibility(template, prototype)` → verdict (eligible;
  per type parameter opaque or integral; for ineligible ones a tag, a text and the first offending position). A
  pure function: no diagnostics, no exit status can change.
- Integral marks: only `eok_land` conjuncts of the four constraint places (il-inventory A2–A8); `std::integral`
  identified by entity (concept template `integral` in namespace `std`, through inline namespaces), argument the
  implicit one or one `tak_type` `tptk_param` at the template's depth, typerefs skipped. Other concepts,
  disjunctions, `!`, requires-expressions are ignored (they only narrow the instances; the parameter stays opaque).
- Value classes for dependent nodes (non-dependent subtrees are left to the encoder's own allow-list): Lit (`int`
  literal 0..32767), Val(p), Prom(p) (`+ - * / %` of one parameter, at most one literal), Bool (comparisons of one
  parameter; `&& || !`; `==`/`!=` on bools), Opaque(p) (`==`/`!=` of one opaque parameter in annotations only).
  `ck_template_param`/`tpck_expression` constants are looked inside (gotcha 6: `(T)100`); other tpck kinds fail.
  Conversions to `T` must be explicit casts of Val/Prom/NonDep builtin integers; an implicit promoted → `T`
  conversion (`return v + 1;`, no conversion node in the IL) is `conversion`.
- Tags: `member`, `nontype_parameter`, `template_template_parameter`, `parameter_pack`, `unnamed_parameter`,
  `spec_function`, `constexpr`, `signature_type` (`T*`, `T&`, `T&&`, `auto`, `Box<T>` — revisit `T*` with an opaque
  pointee now that M4.5 brings pointers), `default_argument`, `dependent_call` (incl. `old`/`forall`/`owns` over
  `T`), `member_access`, `pointer_operation`, `sizeof`, `cast`, `bitwise_or_shift`, `opaque_operation`, `operands`,
  `unary_arithmetic`, `increment_or_compound_assignment`, `conversion`, `not_bool`, `local_initializer`,
  `unsupported_statement`, `measure`, `unsupported_operation`, `internal`.
- Completeness backstop: a second pass with EDG's `traverse_statement`/`traverse_expr` (template-parameter
  constants included) collects every dependent node; one the classifier did not visit makes the template `internal`;
  a hook `--cerastes_generic_selftest` proves it fires.
- Observation: `--cerastes_generic` (cerastes-fe) adds to each function-template prototype in the walker listing
  `generic eligible T=integral U=opaque` or `generic ineligible <tag> @ L<n>:<c>: <text>`; harness header
  `// golden-options: --cerastes_generic` (only with `expect: golden`); the walker oracle counts the lines as a
  derived category (not IL facts). The 49 existing prototype goldens do not change.
- Tests: `tests/templates/eligibility/` (each ineligible tag next to its eligible minimal pair; verdicts
  pre-registered in `// verdict:` comments before goldens exist) and the six ports' verdicts
  (`tests/templates/verifast/`).
- Timeline: il-analyst probes P1–P8 (35 min box: `T i = 0` vs `(T)0`, contract param refs and opaque `r == x`,
  non-dependent calls in a prototype and `old(v)`, `t.m`/`-v`/`v++`/`v += 1`/`(int)v`/`sizeof(T)`/lambdas, parameter
  kinds, loops with attributes and `return v + 1`, `r == (0 <= v && v < 10)`, `a_constant.expr` vs `variant.expr`);
  test-mutator harness header + oracle category; orchestrator the record; edg-backend the checker; goldens checked
  by il-analyst; one Opus audit. Stop rules: cut the oracle's structural checks, then tag coverage automation, then
  `main` in the ports; any rule family not done by 2:10 returns ineligible "not checked" (fail closed).

**M5b EDG (box).** After the S1 upstream merge (§4.1); best just before M5g. Fork commit Q: a registration flag
refusing an accumulating attribute that first appears on a later declaration (check site
`attach_decl_attributes_to_entity`, `decls.c:806-812`, next to `reconcile_must_match_attributes`,
`attribute.c:4131-4230`; next free flag bit `0x8`, `attribute.h:401-437`; precedent
`ec_comparison_defaulted_in_class_must_be_first_decl`, `decls.c:10631`; `first_decl` is set on namespace-scope
paths — check member and template paths; GH tests in `tests/tests/changes/GH-*.sft.cpp` with an `-ok` companion,
`src/Changes`, `doc/source/ext_intf.rst`); cerastes registers it for `predicate`/`pure`
(`register_attributes.cpp:51-52`). Fork commit R: EDG diagnoses series L's silent error node for a `decltype(p)`
lambda parameter (§6); the `decltype` tests move from `cerastes-annotation_substitution_error` to EDG's diagnostic
at the same line (owner's rule). Plus the close-out audit B's missing tests (§6 items 2–4). Fail-before/pass-after,
Opus audit, the full box suites (baseline mismatches `cwg1835`, `cwg2881`), pin move, 0006/0007 and the upstream
draft amended. Stop rules: box unavailable → topic branch, pin moved in a short follow-up; new box mismatch → no pin
move.

**M5c Generic infrastructure and opaque `T`.** A generic Viper method per eligible template, verified even when
nothing instantiates it; calls of instances and ineligible templates stay rejected. Opaque `T`: one empty Viper
domain per type parameter, admitted by `lint_viper.py` only after `// cerastes:opaque-type <template> <param>
<reason>`, with an empty body and a `c_T_` name; `==`/`!=` between values of one opaque parameter. Soundness
(parametricity): no functions, axioms or quantifiers over the sort, so validity over every non-empty interpretation
covers each instance; instances restricted to M3's scalar types (integers and `bool`; floating point excluded, NaN).
VeriFast in Docker (`--platform linux/amd64`, image digest pinned in 0001, `~/Documents/verifast/bin` mounted
read-only, both provers as `verifast_both`), recorded outcomes of the six originals in
`tests/differential/verifast/recorded/` so `check.sh` never needs Docker. Stop rules: F11 (non-dependent calls in a
prototype) not resolving → reject those calls and record; Docker not working in 30 min → run it on the box in M5b.

**M5d Integral `T`.** Four ghost formals per integral parameter (`T_MIN`, `T_MAX`, `PT_MIN`, `PT_MAX`) with
VeriFast's linear constraints as method `requires` (`127 <= T_MAX <= 2^128-1`, `-T_MAX-1 <= T_MIN <= 0`,
`T_MAX <= PT_MAX`, `INT_MAX <= PT_MAX <= 2^128-1`, `-PT_MAX-1 <= PT_MIN <= T_MIN`; INT_MAX from EDG's target): no
domain, no axiom, no assume. Signedness unknown: every symbolic operation is checked as signed against symbolic
bounds (sound for unsigned instances: a result in range means the wrap is the identity); M3's shortcuts that assume
symmetric ranges (`lower.cpp:236` unary minus `a != lo`, `:301` division `!(a == lo && b == -1)`) are not reused —
the general range check on the result instead. `promoted(T) = PT`, `usual_arithmetic(PT, int) = PT` computed by
cerastes (the IL types dependent operations `tptk_unknown`). `cerastes --selftest` checks every target integer type
except `bool` satisfies the constraints; the `consistent` must-fail test (`check(false)`) shows they are not
vacuous. Port `integral_wrong_spec` here (no `main`, no instances), its expectations written from the record before
the first run. Risk: `v * v` against symbolic bounds slow in Z3. Stop rule: loops in generic bodies move to M5f.

**M5e Instances, call sites, fallback, reporting.** Callers call each instance's own method (the M3 encoder on the
instance IL, the contract from the root template, 0006). One Silicon run holds every generic method and every
instance method; the position map gains a `#method` table (first/last line, role, template, display name). Per
template: generic verified → covered (a covered instance that fails is exit 3, "discrepancy" — owner question);
generic failed and instances exist → the generic's errors hidden, the instances' results count (notes
`in the instantiation f<long>`); generic failed and no instance → the generic's errors reported. Coverage: integral
arguments builtin integers other than `bool`; opaque arguments M3 scalar types; not an explicit specialization; not
in a recursion cycle. Notes on stdout after the verdict: `<file>:<line>: note: per-instantiation|not-generic|
not-verified: …`, pinned by `// expect: note <kind> @ L<n>`; 0011 items 9–10, `tests/README.md` and
`parse_verify_output` amended. The count `n`: plain bodies + one per template verified once + one per instance
verified separately. Port `integral_fallback`.

**M5f Specializations, recursion, headers, correspondence.** Explicit specializations verified as plain functions
with the primary's contract (EDG enforces must-match), never covered; specializations of system templates stay
rejected; explicit instantiations are ordinary instances (differs from VeriFast). Call graph keyed by instance
routine; a template in a cycle is not covered (Viper checks a measure only within an SCC of the emitted graph).
Templates in user headers verified per including unit; system templates only through contracts.
`internal/soundness/prototype-correspondence.md` finished (each session adds the constructs it accepts) with an
il-analyst oracle comparing prototype and instance listings for every builtin integer type. Mutation run on
`tests/templates`.

**M5g Gate G2.** Ports in `tests/templates/verifast/` with adaptation headers and line maps to the originals;
`scripts/vf_mutants.py` applies each `mutate.py` edit to the original through the map; VeriFast on originals and
mutants; `internal/differential.md`. Agreement: the same verdict and, when both fail, the same failing lines (and
the same path, generic or per instance). Explained, non-blocking classes: the C meaning of contract arithmetic (0010
item 11), unsigned wrapping, termination (0013), the eligibility path when the verdict agrees, limits from the
target. Anything else stops the session and blocks M6. G2 scope: `integral` (with `pick(1, p)`, pointers from
M4.5), `integral_fallback`, `integral_per_instantiation` (with `low_bit`, bitwise from M4.4),
`integral_wrong_spec`, and `generic_body`'s `copy_through_locals` (adapted: `T z = y;`), `second`,
`declared_first`; **deferred to M7** (owner): `generic_body`'s `assign`/`same_ref` and
`function_template_general_contract`.

**Questions to settle before M5c** (ask then): a failing covered instance → exit 3 (recommended) or ignored as in
VeriFast; notes on stdout with `expect: note` lines; the agreement definition and its explained classes.

## 9. Process notes

- **What kept it short:** fixed scope with stop rules, owner decisions asked up front (upstream merge deferred, the
  r4a rule pre-approved), one audit round per diff plus one re-audit of blocking fixes, single-file harness runs
  during the work, three Sonnet agents with disjoint files.
- **Worktree isolation:** the session ran in a git worktree; its submodules were cloned with `--reference` to the main
  checkout's module stores (`git submodule update --init --reference …`), and bootstrap took a few minutes.
- **check.sh found two harness consequences** of the pin (a stale self-test allowlist entry, the oracle's closure
  rule): run it once early in a session that moves the pin.
- **The approved r4a rule was too narrow** for a file whose other errors stop the back end; asking up front with a
  concrete fallback would have saved a round trip.
- **M4.1 (2026-10-20):** the implementation (encoder, VIR, 86 tests) was done in about 1 h 15 min; the
  test-mutator's mutation runs on the new positives (two runs, one discarded after a rebuild) took most of the rest.
  Run mutation on new tests once, after the last rebuild, with a time box; the session's clock readings mid-run were
  misleading, so check `date` against the budget before starting any long run.
- **Commits:** `git add <paths> && git commit` commits everything staged, including an earlier `git rm`: stage
  deletions only in the commit that lands their feature (one record commit picked them up and was redone).
  It happened again in M4.2a (the tooling commit took two staged retirements; redone at once): run `git rm` just
  before the commit that lands the feature, or commit with explicit paths.
- **M4.2a (2026-10-21):** asking the owner up front (pin, split, pre-approvals, categories, the meaning of "verified")
  and a Plan-agent review before execution cut the scope to what fits; the record's audit ran in parallel with the
  probes, and the implementation (VIR, encoder, 90 tests) took about 30 min. The first mutation run did not finish
  in its 20-minute cap: it overlapped full test runs and two auditors (load average 30), and `mutate.py` prints
  nothing until the end, so a capped run yields nothing. Run mutation alone on a quiet machine, and consider making
  `mutate.py` report as it goes (triage session).
- **Doc pitfalls (M4.2a):** `[[noreturn]]` in text becomes an AsciiDoc anchor even inside `+…+`, so it clashes with
  a section id `noreturn`; `__name` in backticks needs `+…+`; a possessive after a code span (`` `main`'s ``) breaks
  the next span. `build_docs.sh` and `check_doc_examples.py --html build/doc` catch all three.

## 10. Acceptance (commands and results)

| # | Command | Result |
|---|---|---|
| 1 | `scripts/check.sh` (at `610e7e1`, one run) | all green, 17 min 5 s: harness self-test 931 (1 skipped), oracle self-test 74, lint self-test 142, mutate self-test 174; tests 2624 passed, 0 failed, 1 skipped (native-ok: Apple clang 642, GCC 642); 667 `.vpr` lint clean; VIR self-test 230 lowering + 87 rejection cases, 0 problems, 230 passed, lint clean; oracle 134 goldens, 1915 facts, 0 failed; docs 0 problems (source and `--html`); `NOT YET PUSHED` for the pin, as expected |
| 2 | `python3 scripts/run_tests.py -j 8 tests/scalar doc/modules/ROOT/examples` (before check.sh) | 1594 passed, 0 failed (native-ok: Apple clang 335, GCC 335); baseline at the session start 1320 |
| 3 | `python3 scripts/run_tests.py --count-error-twins` | 140 (113 + 27 new positives) |
| 4 | `python3 scripts/run_tests.py --check-headers` | 954 / 954 ok |
| 5 | `diff -r -x MANIFEST build/m4.2a-baseline-vpr build/vpr` (the existing tests' Viper vs the session's baseline) | all 555 `.vpr` files (and maps) of the M3/M4.1 tests and doc examples byte-identical; none missing; also reproduced independently by the code auditor |
| 6 | `python3 scripts/mutate.py -j 8 <27 new positives + 4 doc examples>` (26 min, quiet machine) | 913 mutants: 476 killed, 220 bucket, 13 stillborn, 200 survivors, 4 tool failures (all in the replaced `goto_exit.c`; its successor: 30 mutants, 0 survivors); **185 survivors → §5**, classified, 0 suspicious |
| 7 | `python3 scripts/mutate.py --list tests/scalar` before/after the tooling change | the 2717 existing ids unchanged |
| 8 | audits (Opus) | 0015: 1 blocking (dropped from the design), 9 should-fix taken; code: 0 blocking, 2 should-fix taken; tooling re-audit: 0 blocking, 3 should-fix taken; notes §6 |
| 9 | wall-clock | 08:19–about 11:00 (planned ≤ 4 h) |

## 11. Questions for the owner

Each has a recommended answer. Answer inline, as before.

1. **The EDG pin.** `main`'s `518e0de` moved it to `0bcd9bf28a`; this branch moves it back to P `b9519388ea` (§2).
   Is P the pin you want (push the fork first)? *Recommended: yes.* **Answered (owner, 2026-10-21): yes; the fork's
   `cerastes` (P) and `main` are pushed.**
2. **Order after M4.2a:** CI (§4), then the mutation triage (§5), then M4.2b — or M4.2b first? *Recommended: CI first:
   it settles the upstream EDG merge and the pin before more code depends on it.* **Answered (owner, 2026-10-21): CI
   first, split into three sessions: EDG upstream merge (§4.1), CI (§4.2), mutation triage (§5), then M4.2b.**
3. **M4.2a's surviving mutants** (§5) go to the triage session with M4.1's, and `mutate.py` reports as it goes?
   *Recommended: yes.* **Answered (owner, 2026-10-21): yes; weak tests are strengthened in place (§5).**
4. **EDG's source sequence lists** (`GENERATE_SOURCE_SEQUENCE_LISTS`) would make "who wrote the noreturn flag" and
   "declared only in system files" exact (§6, M4.2a audit note 1). It is an EDG configuration change (decision 0002;
   IL layout and goldens). Evaluate it in M4.2b's planning (named constants also care where a declaration is)?
   *Recommended: yes, as a spike there; until then the attribute-entry check stands.*
5. **A `goto` into a block or an `if` branch** stays rejected (0015 item 2; stricter than C). Lift it only when a
   ported example needs it, after a Silicon spike with must-fail twins? *Recommended: yes.*

**S2 (CI), 2026-10-21:**

6. **Make CI a required check on `main`** (branch protection or a ruleset requiring `check (macOS, Command Line
   Tools 27.0)`)? *Recommended: yes, after the first green run on `main`; Dependabot's weekly pull requests then
   cannot merge red.* If you also enable a merge queue, `ci.yml` needs a `merge_group` trigger (and the guard's pinned
   `on:` with it). **Answered (owner, 2026-10-21): yes.**
7. **When the preview image changes its tools** (a new Command Line Tools, SDK build or Homebrew `gcc`), `ci.sh`
   fails by design. Then: move the assertion and 0001 together after checking the change on your Mac (same
   package), or wait for GitHub? *Recommended: move both, in a short session with a rehearsal; never loosen the
   assertion alone.* **Answered (owner, 2026-10-21): yes** (0016 item 2).
8. **Tighten `timeout-minutes`** (240 now) once two runs on `main` are green: max(150, 1.5 × measured)?
   *Recommended: yes; S2's runs give the first numbers (§4.2).* **Answered (owner, 2026-10-21): yes** (0016 item 9;
   the first session that sees two green runs on `main` does it, with the guard's bound).
9. **Tie each action SHA to 0001** in the guard (today any 40-hex commit passes; a commit that exists only in a fork
   resolves too, so review and Dependabot are the check)? *Recommended: not now; revisit if anyone but you and
   Dependabot edits the workflows.* **Answered (owner, 2026-10-21): not now.**
