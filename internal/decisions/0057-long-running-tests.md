# 0057 — Long-running tests (M4.5d2b2c)

- **Status:** accepted (2026-11-10, M4.5d2b2c), written before the harness change (invariant 6); item 4 revised after
  audit round 1 (question 207), item 8 states the tries; items 1–4 revised after
  Part T was built, with the five points the draft left open (the areas, the header's scope, the PR tier refused
  beside the long tier, the summary clause, the last line's wording). Owner decisions from
  M4.5d2b2c's planning, 2026-11-10 (plan `.claude/plans/m4-5d2b2c-loop-limits.md`), each the recommendation unless
  quoted:
  - **196** (the owner's words): "Increase time past 300s, if the increase does not allow the program to pass, then
    move to M4.8. As we continue to make progress we will likely need other long running tests, and could make a
    long-running tests suite like boost math and boost multiprecision."
  - **199:** each of question 196's two slow programs is tried once under 1800 s on the quiet box; what does not pass
    goes to M4.8.
  - **200:** the long-running suite is built now, as an opt-in tier; CI is unchanged.
  - Standing: 176 (the owner handles CI).
- **Milestone:** M4.5d2b2c.
- **Amends:** 0016 item 9 (its sentence on the driver's and the harness's limits), `tests/README.md`'s header grammar
  and "Time limits".

## Context

**Every verify test runs under the driver's 300 s.** `cerastes verify` stops Silicon after `--timeout=<n>` seconds (1
to 86400, default 300; reaching it is exit 3, `verify_main.cpp`). `run_tests.py` passes no `--timeout`
(`verify_command`), so each verify test has 300 s for Silicon, inside the harness's own `VERIFY_TIMEOUT_S = 600` for the
whole run. `tests/README.md`'s "Time limits" says a verify run has ten minutes, which is the harness's cap, not the
limit a test meets first.

**Two programs need more.** M4.5d2b2b's S40 (g) verified two owning fields replaced in one loop under written
invariants in 218.5 s on a loaded Mac (`g3_two.c`), and its audit probe `cover_twice.c` (a failing program) ran past
300 s. Question 196 asks for a longer limit and, beyond these two, a tier for long tests as later sessions find them.

**The full check already takes hours** (owner, 2026-11-09: `check.sh` once per session, on the x86-64 box), so a long
test cannot simply join the default run.

## Decision

1. **The tier.** A verify test (`.c`, `.cpp`) that needs more than the driver's 300 s lives under `tests/long/<area>/`,
   where `<area>` is one of the verify areas `annotations`, `scalar`, `memory`, `vlibc` (a file directly in
   `tests/long/` or in another subdirectory is a header error), and starts with a header `// timeout: <seconds>`, a
   whole number from 301 to 3600 (twice the 1800 s a program may take when it is tried, item 8). Every rule the harness
   keys on a directory (both modes for `.c`, `native-ok`, the negative directories, twins) reads `tests/long/<area>`
   as it reads `tests/<area>`, so a long positive needs its twin and its modes like any other.

2. **The header is the tier.**
   - `// timeout:` in a file outside `tests/long/` is a header error, and so is a test under `tests/long/` without it,
     or with a value that is not a whole number of seconds or is outside 301–3600, or twice, or on a test that is not
     `verified` or `error` (a `rejected` test never reaches Silicon).
   - The harness passes `--timeout=<n>` to `cerastes verify` for that test (each mode, after the test's
     `verify-options`), and gives that run `n + 120` seconds of its own in place of `VERIFY_TIMEOUT_S`. The value is
     its own field of the parsed header, so the tools that forward `verify-options` (`mutate.py`, `regen_vpr.py`)
     never pass it.
   - Like `expect:`, `mode:` and `twin:`, a `timeout:` line after the first line of code is an error.

3. **Opt-in** (question 200).
   - `run_tests.py` skips `tests/long/` unless `--long` is given. A path named on the command line inside `tests/long/`
     is run without `--long` (naming a long test is asking for it).
   - `check.sh` passes `--long` when the shell sets `CERASTES_LONG=1`; unset or `0`, the long tier is skipped; any
     other value, the empty string included, is an error (as `CERASTES_JOBS` validates its value).
   - The PR tier (`--pr-base`, `CERASTES_CHECK_BASE`) never selects a long test, changed, sampled or named. `--long`
     with `--pr-base` (exit 2) and `CERASTES_LONG=1` with `CERASTES_CHECK_BASE` (exit 1, before the build) are
     refused: the PR tier would skip what the variable asks to run.

4. **Never silent.**
   - Skipped long tests are counted and printed: each gets a `SKIP` line, and the summary line gains
     `, L long skipped` whenever L is not zero (a skipped long test never counts as passed, and never as "left to the
     full check" in the PR tier). `check.sh`'s last line says whether the long tier ran: `check.sh: long tier: ran
     (CERASTES_LONG=1)` or `check.sh: long tier: skipped (set CERASTES_LONG=1 to run tests/long)`.
   - A skip happens only after the test's header has been read and checked: a bad header under `tests/long/` fails the
     default run.
   - Their headers and twins are checked in every run, skipped or not: `--check-headers` reads `tests/long/`, and the
     twin rule (`tests/README.md`, "Twins") sees long tests and their twins in either tree. A long positive may count
     a twin in either tier; a positive outside `tests/long/` needs an `error` twin outside it in each of its modes
     (owner, 2026-11-10, question 207: else the must-fail half of invariant 2 would run only at a session's box check,
     never in CI or under `mutate.py`; audit round 1, S6).
   - A long test that reaches its limit fails (exit 3), like any other test; a limit is there to catch a hang, never to
     measure speed.

5. **The limit of a test** is at least twice its measured time on the quiet x86-64 box, rounded up to a whole minute,
   the same for both modes (the header is one value per file).

6. **Everything else applies.** `expect:` lines, `mode:`, twins (a must-verify long test has a must-fail twin, which may
   be long too, and then has its own `timeout:`), `native-ok`, `verify-options:` (except `--timeout`, which only the
   header sets: `--timeout` in `// verify-options:` is a header error). `mutate.py` skips `tests/long/` and says so in
   its output; `regen_vpr.py` regenerates long tests like any other (it does not run Silicon).

7. **CI is unchanged** (question 176): neither tier sets `CERASTES_LONG`. The session's box check runs with
   `CERASTES_LONG=1`. A `dispatch` job for the tier is the owner's to add.

8. **The two programs** (question 199), recreated in `build/m4.5d2b2c/slow/`:
   - `two_fields.c`: `tests/memory/owning_field_loop_replace_written.c` with a second owning field replaced under a
     second written invariant (S40 (g) `g3_two.c`: verified, 218.5 s on a loaded Mac). Its twin drops one `free`.
   - `cover_twice.c`, from the audit probe, one statement per line:
     ```c
     struct S { int *q [[cerastes::owning]]; int n; };
     struct S *where(void) CERASTES_ENSURES(r, cerastes_owns(*r)) { static struct S s; return &s; }
     void user(void) { struct S *p = where(); struct S *p2 = where();
                       if (p->q != NULL) { free(p->q); free(p2->q); } p->q = NULL; }
     ```
     Its designed outcome is pre-registered from record 0055 item 3 ("Covered atoms") and from its committed neighbour
     `tests/memory/negative/owning_field_cover_caller_free.c` (`invalid-free` twice at the free, `permission` at the
     caller's return): `invalid-free` at each `free`, and the caller's `permission` row where it appears. Question 160
     applies to the lines.
   - Each is run once per mode with `--timeout=1800` on the quiet box, with the build that has the session's last change
     to loop encoding (or, if run earlier, its `.vpr` is compared with the final build's and the try stands only if they
     are byte-identical). Finished with its designed outcome: it becomes a long test with item 5's limit. Not finished:
     its time goes to the handoff for M4.8. **A failing program that verifies, an exit 3 that is not the timeout, or a
     result with none of the designed errors stops the session.**

   - **The tries** (2026-11-10, the x86-64 box `clang4.cpp.al` in S8's `ubuntu-26.04` container, both modes run at
     once, load 2–16 on 172 cores). `cover_twice.c`, with the base encoder (the box's build at `aae35c2`, whose `src/`
     is the session's base): exit 3 at 1801 s in both modes ("Silicon did not finish within 1800 seconds"). `two_fields.c`
     and its twin `two_fields_drop.c`, with Part L-2's build: exit 3 at 1801 s in all four runs. The Viper of
     `two_fields.c` is byte-identical from the base build and from Part L-2's, so the session did not slow it; the
     program was recreated from the description of S40 (g)'s lost `g3_two.c` (218.5 s on a loaded Mac) and may differ
     from it in detail. **Neither finished, so neither becomes a long test: both go to the handoff for M4.8 with these
     times, and `tests/long/` starts empty.** None of the outcomes is the stop of this item (no verified failing
     program, no exit 3 other than the timeout).

9. **Facts:** the driver's `--timeout` (`src/driver/verify_main.cpp`), `verify_command` and `VERIFY_TIMEOUT_S`
   (`scripts/run_tests.py`); the two programs' tries (item 8).

## Consequences

- A program that needs more than 300 s can be a test without slowing the default run, and the full check on the box
  runs it.
- The default run prints how many long tests it skipped, so the tier cannot vanish unnoticed.
- 0016 item 9's sentence reads: the driver's 300 s Silicon limit (a test under `tests/long/` sets its own, record 0057)
  and the harness's 600 s limits are unchanged.
- Later sessions add long tests as they find them (question 196's "boost math and boost multiprecision" suite).
