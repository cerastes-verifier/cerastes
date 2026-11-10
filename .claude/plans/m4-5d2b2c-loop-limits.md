# Plan: M4.5d2b2c — loop limits of owning fields, long-running tests (records 0056, 0057)

**Approved by the owner on 2026-11-10**, at a session that only planned. Its commit also split M4.5d2b2c in two
(question 198), wrote M4.5d2b2d's plan (`.claude/plans/m4-5d2b2d-invalidation-profile.md`) and consolidated
`internal/handoff.md` and `plan.md`. Questions 194–197 were answered in the handoff before it, 198–201 at it (and
202 and 203, which are M4.5d2b2d's).

Branch `m4.5d2b2c-loop-limits`, worktree `.claude/worktrees/m4.5d2b2c`, both from the base Step 0 names.

## Context

**Scope** (question 198):
- **Part L:** record 0055 item 7's loop limits (question 195). Six pinned false alarms are lifted (limits 2–6);
  five pins stay under the written-invariant rule (limits 1 and 7; question 201).
- **Part T:** question 196. A long-running test tier, and the two slow programs tried under 1800 s.
- **Not here:** `not_invalidating`, `--strict-owner`, marked fields and N1 (M4.5d2b2d); N2 and automatic closure
  sites for borrowed structures (M4.8).

**The eleven pins** (`tests/memory/negative/owning_field_loop_*_false_alarm.c`). Status's "15 pinned false alarms"
are these plus four hand-written S40 (d) fixtures (`tests/viper-smoke/memory/s40_d_*_false_alarm.vpr`), which are
design evidence and stay.

| Limit | Pin (`owning_field_loop_…_false_alarm.c`) | Cause (planning survey) | Read | Here |
|---|---|---|---|---|
| 5 | `pointer_arg` | not a lost flag: a callee's `owns(*pp)` havocs the leaf's value; `wef` starts false on the path that enters null | in code | lifted (item 2) |
| 5 | `owning_local_maybe_null` | a lone written owner with a closure gets no value set | in code | lifted (item 3) |
| 2 | `alias_call` | (i) the ghost is refreshed only for `&s` by syntax; (ii) `callee_takes_owner` is never true for a leaf site | (i) in code, (ii) **inferred** (run 2) | lifted (item 4) |
| 4 | `main_holds` | in `main` the leaf is neither carried nor framed, and its value is lost too | in code | lifted (item 5) |
| 3 | `global_pointer` | a `WBlock`'s cells are never places, so `gp->data` cannot be framed | in code | lifted (item 6) |
| 6 | `chain3` | `closure_deep = false` for struct-local leaves; only the last cell's flag is guarded | in code | lifted (item 7), timed |
| 1 | `alias` | not an alias limit: the loop replaces the block without the written `owns(*p)` its twin has | **inferred** (run 1) | stays (item 8) |
| 7 | `owner_replaced`, `_alias`, `_in_expr` | need a fact true only in the first iteration | **inferred** (run 6) | stay (item 8) |
| 7 | `owner_renewed` | the contract of `renew` consumes the structure's cells | **inferred** (run 7) | stays (item 8) |

The survey could not run the verifier. The owner's answer 201 rests on the three inferred rows of limits 1 and 7,
so **runs 1, 6 and 7 of S41 (0) gate item 8 and Flips C**: a run that contradicts its row stops that work and goes
to the owner.

**Size** (insertions; `lower.cpp` / VIR self-test cases):

| Item | `lower.cpp` | Self-tests | Risk |
|---|---|---|---|
| Limit 5 | 45–75 | 60–100 | low |
| Limit 2 | 60–100 | 60–100 | low–medium |
| Limit 4 | 60–100 | 50–80 | medium |
| Limit 3 | 80–130 | 50–80 | medium–high |
| Limit 6 | 50–90 | 40–60 | high |
| **Sum** | **295–495** | **260–420** | |

Beside it: `lint_viper.py` for the new ghost; the harness tier (`run_tests.py`, `check.sh`, about 150–300 lines of
Python with self-tests); about 20 tests. d2b2b's four re-audit fixes took 419 + 248 lines, so the range is honest.

**Key survey facts** (tree at `262edb3`; `src/vir/lower.cpp` unless named):
- **Owners:** `OwnerRef` 815–833; `owners_in_scope` and its leaf forms 2196–2224; `held_updates` 2234–2332 (framed
  update 2283–2287, plain update only for `&s` by syntax 2291–2293, `gives_param_leaf` 2458); `holds_global_at_entry`
  2397; `leak_exemption` 11231–11287 with `exempt_owned_fields` 11204 (any depth already).
- **Flags:** `flags_kept_entry` 2635–2684 (`wef_`; the `wfc_` snapshot 2668–2680), `flags_kept_row` 2685,
  `callee_takes_owner` 2604–2630 (tests only `site_global` and `site_cell_var`), `stores_type` 15009.
- **Reach:** `loop_calls_reach_cells_of` 2739–2769; `framed_owner_ghosts` 2770–2829 (`by_type` 2809–2810);
  `leaf_held_agrees` 2576, `leaf_held_row` 2586; `writes_in_expr` 11770–11856.
- **The loop:** `loop()` 13276–~14563; `owner_loop_sites` called at 13309; `others` (the value-set trigger)
  13469–13476; `inv_all` built 14038–14128 (struct-local leaf cells 14092–14128, globals skipped at 14097); entry
  rows 14143, the `hp_` snapshot 14157; invariant-read diagnostic 14188–14190.
- **Sites:** `owner_loop_sites` 15070–15363 (`by_type` and the `main` exclusion 15140–15143; struct-local leaf
  15149–15161; owning global 15255–15268 with the comment at 15361–15362; owning-local closure 15281–15303;
  leaf closure 15306–15360); `closure_deep` 15164, tested 15176, cleared 15352; `make_closure` 15202–15241;
  `may_replace_local` 14909–14950; `resolve_blocks` 15441–15552 (the S1 flag guard 15510–15537);
  `wblock_invariants` 15604, `wblock_rows` 15675 (pointer pre-reads hoisted 15681–15696).
- **Places:** `loop_places` 12126–12713 (heap places 12471–12645; the takeover 13326–13349).
- **Harness:** `scripts/run_tests.py` constants 163–170 (`VERIFY_TIMEOUT_S = 600`), `parse_header` 403–654,
  `verify_command` 1822–1827 (passes no `--timeout`, so every verify test runs under the driver's 300 s),
  `evaluate_verify` 1735–1757, the PR tier 2717–2760, the summary 3184. The driver: `--timeout=<n>`, 1 to 86400,
  default 300 (`verify_main.cpp` 70, 86, 224–234); reaching it is exit 3. `tests/README.md` "Time limits" 586–593
  wrongly says a verify run has ten minutes.
- **The main checkout's `build/` is stale** (built 2026-11-09 19:23, before d2b2b's last source change). Every
  run uses the session worktree's build.
- **Neither slow program was kept.** `g3_two.c` lived in a removed worktree; `cover_twice.c` was an audit probe.

## Owner decisions

**At this planning (2026-11-10):**
- **198:** two sessions; this one is the loop limits and the long-running tests.
- **199:** each slow program is tried once under 1800 s on the quiet box; what does not pass goes to M4.8.
- **200:** the long-running suite is built now, as an opt-in tier; CI is unchanged.
- **201:** the written-invariant rule stays. The five pins keep their `expect:` lines; their headers name the
  invariant the loop needs, and each gets a verifying twin that writes it.

**Answered in the handoff before it:**
- **195:** d2b2c takes the pinned loop limits first.
- **196** (the owner's words): "Increase time past 300s, if the increase does not allow the program to pass, then
  move to M4.8. As we continue to make progress we will likely need other long running tests, and could make a
  long-running tests suite like boost math and boost multiprecision."

**Standing:** 69 (no EDG work); 77 (one context); 160 (a new test's expect line may be corrected after its first
run when it fails for its designed reason at a neighbouring line or category; each is listed in status); 176 (the
owner handles CI); `check.sh` once, at the end, on the x86-64 box only; no Claude trailer; commits stay local; an
agent's question stops that work and goes to the owner.

## Step 0 — before anything

1. `git fetch`, then check that `origin/main` holds the planning commit (the commit that added this file). If it
   does, branch from `origin/main`; if not, from local `main`, and say so in status.
2. No box check is pending: d2b2b's is green on `aae35c2`.
3. Everything below is committed on the session branch, never on `main`.

## Step 1 — worktree, CI, baseline (no encoder code)

1. `git worktree add .claude/worktrees/m4.5d2b2c -b m4.5d2b2c-loop-limits <base>`; submodules with `--reference`
   (edg, silicon, then silver); `scripts/bootstrap.sh`.
2. **CI, one small commit if anything changes** (questions 172, 176):
   - `gh run list --workflow ci.yml --event workflow_dispatch`: if a `profiles=all` dispatch ran and `macos-26` is
     green, promote it to `nightly` in `scripts/ci_toolchains.toml` with the guard self-test's change; if red,
     record what differs; if none ran, leave it and say so.
   - `gh run list --workflow ci.yml --event schedule --limit 3`: record in the handoff whether the nightly fits in
     360 minutes.
3. **Baseline**, in the session worktree before any change to `src/`: `scripts/regen_vpr.py` and the VIR self-test
   into `build/m4.5d2b2c/baseline/`; a frozen copy of the build as `build/m4.5d2b2c/cerastes-base` for the probes.

## S41 — runs and spikes, before code

Results go to `internal/il-inventory.md`, a new chapter `## M4.5d2b2c` after line 8013, with the kept fixtures in
`tests/viper-smoke/memory/s41_*`. A result that contradicts an item changes the record before code.

**S41 (0): seven runs of the frozen build** (test-mutator; programs in `build/m4.5d2b2c/s41/0/`, each a pin with
one change):
1. The `alias` pin with `CERASTES_INVARIANT(cerastes_owns(*p))`; with `owns(*v)` instead; and without the alias and
   without an invariant.
2. The `alias_call` pin with `refill(&s)` in place of `refill(ps)`.
3. The `global_pointer` pin with the written site `gp == NULL || cerastes_owns_block(gp)`.
4. The `main_holds` pin with the written site `g.data == NULL || cerastes_owns_block(g.data)`.
5. The `pointer_arg` pin with a written site on `s.data`; the `maybe_null` pin with one on `v->data`.
6. The `owner_replaced` pin with `v == NULL || i != 0 || v->data == NULL || cerastes_owns_block(v->data)`.
7. Two straight-line calls `renew(pv); renew(pv);` with no loop.

**S41 (a)–(c): hand-edited Viper** (il-analyst; each with Silicon's verdict and time, quiet machine):
- **(a)** limit 3: the `global_pointer` loop with the block's cells as places for the later site. Verified; the twin
  without `free(gp->data)` fails `leak` at the back edge.
- **(b)** limit 6: `chain3` with every intermediate flag in the guard and each pre-read after the earlier sites'
  rows. Timed in both modes; **over 60 s, limit 6 stays pinned**.
- **(c)** limit 4: the synthetic site in `main` under `hs`. Positives: the pin, and a global that `main` never
  names. Twin: `main` leaks another block (`leak` at `main`'s end).

Limits 5 and 2 need no Viper spike: runs 2 and 5 show the invariants a user can already write.

## Record 0056 — loop limits of owning fields (written before code)

Amends 0055 item 7 and 0054 items 3 and 5. Drafted at the start, revised after S41 and audit round 1.

1. **Scope:** items 2–7 lift six false alarms; item 8 restates the rule for the other five pins. A limit that is
   not built keeps its pin untouched.

2. **Flags of a site the loop cannot fill** (limit 5, `pointer_arg`).
   - `wef`'s default gains `|| !loop_may_fill(loop, site)`.
   - `loop_may_fill` is a new scan: the loop holds a store of a non-null value of the site's pointer type, or a
     call whose callee has an `ensures` block site through a load of that type.
   - Sound: `known && wef ==> flag` stays a checked invariant, with its entry check and its `uninitialized`
     back-edge row. A user can write the same as a plain site on `s.data`.

3. **The value set of a lone written owner with a closure** (limit 5, `maybe_null`).
   - `others` (13469–13476) also counts an owner that has a synthetic closure site, so 0054 item 3(c)'s value set
     `v == ev_v || v == null` is emitted and an owner that entered null stays null.
   - Sound: the value set appears only where `valset` already holds, and it is a checked invariant.

4. **A leaf's ghost after a call through an alias** (limit 2).
   - **(a)** In `held_updates`, an argument that is not `&s` by syntax but has the root's struct pointer type,
     where `gives_param_leaf` holds, runs the plain update under `arg == &s` (0054 item 6's guarded form). The
     same for struct-global leaves. Sound: `held` rises only where the callee's `ensures` returns the leaf's block,
     and a true `held` adds the lifetime row.
   - **(b)** `callee_takes_owner` is true for a leaf site when a callee's `requires` has a block site through a
     load of the leaf's pointer type (0054 item 5's rule, never built for leaves). It feeds a checked invariant.
   - **Consequence:** in `owning_field_alias_fill_leak.c` the error moves from the closing line to the return
     (Flips, B).

5. **In `main`, a global leaf reached only by type** (limit 4).
   - The leaf gets its synthetic site under a ghost `hs := held && perm(cell) > none`, taken at loop entry before
     the entry rows and conjoined to the site's guard (`WBlock::g`, in `resolve_blocks`).
   - Invariants: `hs ==> (held == (L != NULL))` and `!hs ==> !held`.
   - Sound: where `hs` is false nothing is asked or carried; where it is true every clause has its entry and
     back-edge rows, and `main`'s exemption covers only the carried token.

6. **A site's block cells are places for later sites** (limit 3, and the base of limit 6).
   - In `resolve_blocks`, each resolved site's block cells become readable places, guarded by the site's `on`, for
     later sites and for the user's invariants. This replaces the special case at 14092–14128.
   - `owner_loop_sites` builds the closure (`make_closure`) for an owning `struct S *` global and for struct-global
     leaves, with a replace test for globals like `may_replace_local`'s.
   - Sound: a place only frames reads; each new site is 0054 item 4's carrier with its rows.

7. **Chains at any depth in loops** (limit 6).
   - `closure_deep` goes. Each intermediate cell's flag joins the guard, and each site's pointer pre-read moves
     after the rows of the earlier sites.
   - S41 (b) decides: over 60 s in either mode, the limit stays pinned and the item is struck.

8. **The written-invariant rule** (question 201; restates 0055 item 7's first bullet).
   - A loop that replaces the block of an owning field of a **borrowed** structure writes `owns(*p)` in its
     invariant (`owning_field_loop_replace.c`). The `alias` pin is this case.
   - A loop that replaces **the owner itself** and frees the field only in its first iteration writes the guarded
     site of run 6: only a written invariant can state a fact of one iteration.
   - `renew` takes `owns(**pp)` and returns only `owns(*pp)`, so its contract consumes the structure's cells. If
     run 7 confirms it, that pin is an ordinary must-fail.
   - Automatic closure sites for borrowed structures are not built: they would change existing loops' Viper and
     give a structure with two owning fields the 218 s shape. M4.8 may revisit them with the cost work.
   - If run 1 shows that the written invariant does not make the alias loop verify, the takeover (13326–13349)
     learns a never-assigned local initialized from a parameter (at most 40 lines); beyond that, stop and ask.

9. **The lint** (`lint_viper.py`): the ghost `hs` and its one assignment; the new closure sites reuse existing
   rules. **Categories:** none new.

10. **Facts:** S41. **Still pinned after this session:** item 8's five; any limit cut; N2 (no pin; M4.8).

## Record 0057 — long-running tests (written before the harness change)

Amends 0016 item 9's sentence on the limits and `tests/README.md`'s grammar.

1. **The tier.** A verify test (`.c`, `.cpp`) that needs more than the driver's 300 s lives under `tests/long/`,
   in its area's subdirectory, and starts with `// timeout: <seconds>` (301 to 3600: twice the 1800 s a program
   may take when it is tried, item 8).
2. **The header is the tier.** `// timeout:` outside `tests/long/` is a header error, and so is a file under
   `tests/long/` without it. The harness passes `--timeout=<n>` to `cerastes verify` and gives that run `n + 120`
   seconds of its own.
3. **Opt-in** (question 200). `run_tests.py` skips `tests/long/` unless `--long` is given; `check.sh` passes
   `--long` when `CERASTES_LONG=1`. A path named on the command line is run. The PR tier never selects a long
   test.
4. **Never silent.** Skipped long tests are counted and printed in the summary, and `check.sh`'s last line says
   whether the long tier ran. Their headers and twins are checked in every run, skipped or not
   (`--check-headers`, the twin rule). A long test that reaches its limit fails (exit 3) like any other.
5. **The limit of a test** is at least twice its measured time on the quiet box, rounded up to a minute, the same
   in both modes.
6. **Everything else applies:** `expect:` lines, twins (a must-verify long test has a must-fail twin, which may be
   long too), `native-ok`. `mutate.py` skips `tests/long/` and says so.
7. **CI is unchanged** (question 176). The session's box check runs with `CERASTES_LONG=1`. A `dispatch` job for
   the tier is the owner's to add.
8. **The two programs** (question 199), recreated in `build/m4.5d2b2c/slow/`:
   - `two_fields.c`: `tests/memory/owning_field_loop_replace_written.c` with a second owning field replaced under
     a second written invariant (S40 (g) `g3_two.c`: verified, 218.5 s on a loaded Mac). Its twin drops one `free`.
   - `cover_twice.c`, from the audit probe, one statement per line:
     ```c
     struct S { int *q [[cerastes::owning]]; int n; };
     struct S *where(void) CERASTES_ENSURES(r, cerastes_owns(*r)) { static struct S s; return &s; }
     void user(void) { struct S *p = where(); struct S *p2 = where();
                       if (p->q != NULL) { free(p->q); free(p2->q); } p->q = NULL; }
     ```
     Its designed outcome is pre-registered from record 0055 item 3 ("Covered atoms") and from its committed
     neighbour `negative/owning_field_cover_caller_free.c` (`invalid-free` twice at the free, `permission` at the
     caller's return): `invalid-free` at each `free`, and the caller's `permission` row where it appears. Question
     160 applies to the lines.
   - Each is run once per mode with `--timeout=1800` on the quiet box. Finished with its designed outcome: it
     becomes a long test. Not finished: its time goes to the handoff for M4.8. **A failing program that verifies,
     an exit 3 that is not the timeout, or a result with none of the designed errors stops the session.**

## Parts and commits

1. **Commit 1:** Step 1's CI commit, if any.
2. **Commit 2:** records 0056 and 0057 (drafts, revised after S41 and audit round 1); S41 in `il-inventory.md`
   with its fixtures.
3. **Commit 3, Part T:** the tier in `run_tests.py` and `check.sh`, its self-tests, `tests/README.md`.
4. **Commit 4, Part L-1:** limits 5 and 2 (items 2–4), with their tests and Flips A and B.
5. **Commit 5, Part L-2:** limits 4, 3 and 6 (items 5–7), with their tests, the lint rule and Flips A.
6. **Commit 6:** item 8's pins (Flips C) and their verifying twins; the long tests of item 8 of record 0057.
7. **Commit 7:** the audits' fixes.
8. **Commit 8:** docs, status, handoff (with its Questions section), `plan.md`'s Done row, both records accepted.

**Order of work.** S41 (0) and Part T run while the il-analyst spikes S41 (a)–(c) and the orchestrator drafts the
records. Part L-1 is built while the spikes finish; Part L-2 starts only after Part L-1's commit, since both edit
`lower.cpp`. The two slow programs run on the box in the background (up to two hours).

**Audits** (soundness-auditor, read-only, on snapshot commits in a second worktree): round 1 on both record drafts
with S41; round 2 on Part L-1, on Part L-2 and on Part T's harness diff (can a long test pass vacuously or vanish
uncounted?); one re-audit of the blocking fixes.

**Pre-agreed cuts, in this order:** limit 6, then limit 3, then limit 4. A cut limit's pin is not touched and the
limit is re-filed in handoff §3 under M4.8. Limits 5 and 2 are not cut without asking.

## Team (from `.claude/agents/`: one Opus reviewer, three Sonnet agents; the orchestrator is Opus)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | S41 (a)–(c). Then Part L-2 in `src/vir/lower.cpp` (items 5–7) and its VIR self-test cases (`src/viper/selftest_cases24.inc`, `VIR_CASES_MIN`). A fresh il-analyst takes over if its context runs low. |
| **edg-backend** (Sonnet) | Part L-1 in `src/vir/lower.cpp` (items 2–4) and its VIR self-test cases; the builds; `regen_vpr.py`/`vprdiff.py` after each part. It hands `lower.cpp` over at Part L-1's commit. |
| **test-mutator** (Sonnet) | S41 (0); Part T (`run_tests.py`, `check.sh`, `test_run_tests.py`, `mutate.py`, `tests/README.md`); the tests and twins of every limit; exactly the Flips; `lint_viper.py` with its self-test; the two slow programs. |
| **soundness-auditor** (Opus, read-only) | The audits above. Should-fix items not taken go to handoff §3. |
| **Orchestrator** (Opus) | Steps 0 and 1; records 0056 and 0057; the vprdiff allow-list per part, fixed before that part's build; docs prose; `encoding.md`; integration; serialized builds (`lockf build/m4.5d2b2c/build.lock`); status and handoff. |
| **Advisor** (Fable, optional; owner, 2026-11-10) | Consulted by the orchestrator before the records are accepted, at any cut or stop-and-ask, and before the box run. It is the `advisor` tool where the session has one, otherwise a read-only agent on the Fable model given this plan and the question. |

**One exception to handoff §4's disjoint files, on purpose:** Part L is all in `lower.cpp`, the file that
exhausted one agent twice in M4.5d2b2b. So edg-backend builds Part L-1 there and commits; only then does the
il-analyst start Part L-2. They never hold the file at the same time.

**Rules for every agent:**
- Any question stops that work and goes to the owner.
- Each report is also written to `build/m4.5d2b2c/<role>-report-<n>.md`.
- Agents hold builds before a directory-wide run; only one agent runs `run_tests.py`; single-file runs use
  `build/cerastes verify`.
- Code style: `scripts/format.sh`, brace initialization, `U`, one declarator per declaration, no comment on a
  brace line.
- A test generator lives in `build/m4.5d2b2c/` and is never re-run over files in the tree.

## Tests (each positive has a twin failing for one reason at one line; `.c` in `c23, c++20`)

- **Limit 5:** the two former pins verify. Twins: without `*s.data = 1` (`uninitialized` at the return); without
  `free(v->data)` after the loop (`leak`).
- **Limit 2:** the former pin verifies, and so does the same loop with `refill(&s)`. Twins: without `free(s.data)`
  after the loop (`leak` at the return); an unwritten `malloc` stored before `refill(ps)` (`uninitialized`).
- **Limit 4:** the former pin verifies; a loop in `main` beside a global it never names verifies. Twin: another
  block leaked after the loop (`leak` at `main`'s end).
- **Limit 3:** the former pin verifies, without and with a written invariant. Twin: without `free(gp->data)`
  (`leak` at the back edge).
- **Limit 6:** the former pin verifies. Twin: without `free(a.b->c->x)` (`leak` at the back edge).
- **Item 8:** the alias loop with the written `owns(*p)` verifies; the replaced-owner loop with the guarded site
  verifies. A twin that does not verify is not forced: its run goes to status and the handoff's Questions, and
  the pin stays as it is.
- **Part T:** self-tests for the header grammar (both header errors), the skip count, the command line that
  carries `--timeout`, `--long` and `CERASTES_LONG=1`; the long tests themselves.
- **Byte identity:** `scripts/vprdiff.py` against the baseline after each part; only the allow-list differs.

## Flips (pre-approved with this plan; any other changed outcome stops the session)

**A. Retired for a positive of the same program, with the twins above** (only where the limit is built):

| Pin (`tests/memory/negative/`) | Positive (`tests/memory/`) |
|---|---|
| `owning_field_loop_pointer_arg_false_alarm.c` | `owning_field_loop_pointer_arg.c` |
| `owning_field_loop_owning_local_maybe_null_false_alarm.c` | `owning_field_loop_owning_local_maybe_null.c` |
| `owning_field_loop_alias_call_false_alarm.c` | `owning_field_loop_alias_call.c` |
| `owning_field_loop_main_holds_false_alarm.c` | `owning_field_loop_main_holds.c` |
| `owning_field_loop_global_pointer_false_alarm.c` | `owning_field_loop_global_pointer.c` |
| `owning_field_loop_chain3_false_alarm.c` | `owning_field_loop_chain3.c` |

**B. One expectation moves:** `tests/memory/negative/owning_field_alias_fill_leak.c`, `error leak @ L27` becomes
`error leak @ L26` (item 4(a): the named-leaf row now sees the block the callee gave through the alias). If its
first run shows anything else, stop and ask.

**C. Renamed with `git mv`, headers reworded, `expect:` lines kept** (question 201):
- `owning_field_loop_alias_false_alarm.c` becomes `owning_field_loop_alias_no_invariant.c`.
- `owning_field_loop_owner_replaced_false_alarm.c`, `…_replaced_alias_…` and `…_replaced_in_expr_…` become
  `…_no_invariant.c`.
- `owning_field_loop_owner_renewed_false_alarm.c` becomes `owning_field_loop_owner_renewed_consumed.c`, only if
  run 7 confirms the cause.
- These five record the encoder's actual errors, and answer 201 keeps their `expect:` lines. **If a part changes
  what one of them reports (other errors, a verified result, an exit 3), that is a stop-and-ask**, not a
  re-recording.

**D. Viper changes with the same outcome** (the vprdiff allow-list, fixed per part before its build):
`owning_field_alias_fill.c`; `owning_field_loop_owning_local_free.c`, `…_free_empty.c` and
`negative/…_free_leak.c`; `owning_field_loop_global_pointer_n.c` and `negative/…_n_leak.c` (they gain a closure
site: time them); `owning_field_loop_chain.c` and `negative/owning_field_loop_chain_drop.c`; the named-leaf loops
(a `wef` literal).

**Stay as written:** the four `s40_d_*_false_alarm.vpr` fixtures; `heap_loop_replace_false_alarm.c`,
`heap_loop_swap_false_alarm.c`, `loop_written_moved_owns_false_alarm.c`,
`tests/vlibc/negative/vlibc_alloc_loop_replace_false_alarm.c`; every pin that names M4.5d2b2c's former second half
(M4.5d2b2d's).

## Docs (invariant 8; every example is a test)

- **`memory.adoc`:** the Loops paragraph of `[#owning-fields]` (1244–1254) says when a loop needs the written
  invariant, with the alias loop as a tested example. In `[#not-yet]`, the bullet at 1367–1372 keeps only its
  second half (the `static` structure returned on some paths), and the bullet at 1373–1381 is cut to what is left.
- **`tests/README.md`:** the header grammar (`// timeout:`), "Time limits" corrected (a verify test runs under the
  driver's 300 s), a section "Long-running tests", the area table.
- **The developer guide's Testing page** and **`CLAUDE.md`'s Commands**: `run_tests.py --long`,
  `CERASTES_LONG=1 scripts/check.sh`.
- **`internal/encoding.md`**, **`internal/il-inventory.md`** (chapter `## M4.5d2b2c`), record 0016 item 9's
  sentence.
- The tutorial's heap chapter only if one of its `console` blocks changes.

## Stop and ask

- Any agent question.
- Any changed outcome outside Flips; any existing program's Viper changing outside the allow-list.
- A spike or run that contradicts items 2–6 or 8.
- A cut beyond the pre-agreed ones, or of limit 5 or 2.
- An exit 3, or a timeout over 120 s outside `tests/long/`, not fixed within its part.
- Record 0057 item 8's "any other outcome".

## Acceptance (run at the end)

- Before the box run: `scripts/regen_vpr.py` over the final build, `scripts/lint_viper.py` over its output and a
  fresh VIR self-test directory, and the session's VIR cases through Silicon.
- `CERASTES_LONG=1 scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): **once, after the last code change, on
  the x86-64 box**, the branch as a git bundle into `~/cerastes-s8/p-ubuntu-26.04`. Green, long tier included.
- `python3 scripts/vprdiff.py build/m4.5d2b2c/baseline/vpr build/vpr`: only the allow-list differs.
- The VIR self-test at the new `VIR_CASES_MIN` (1707 today); `test_lint_viper.py` (660 today); `test_run_tests.py`.
- `scripts/format.sh --check`; no new comment on a brace line in the diff of `src/`; `scripts/build_docs.sh`.
- The status entry (done and not done, the flips and corrections, the cuts, the two programs' times, records 0056
  and 0057, the first step for M4.5d2b2d) and the handoff with its Questions section.
- Commits stay local on the branch; the owner pushes.

## Left for later

- **M4.5d2b2d** (`.claude/plans/m4-5d2b2d-invalidation-profile.md`): `not_invalidating`, `--strict-owner`, marked
  fields, N1.
- **M4.8:** any limit cut here; N2 with question 194; automatic closure sites for borrowed structures (question
  201); a slow program that did not pass under 1800 s.

## Inputs (from handoff §3 at this planning)

Verbatim from `git show 262edb3:internal/handoff.md` §3. Line numbers in these notes are older than the survey
above; where they differ, the survey holds.

- **d2b2b's cuts:** none was taken. Its S40 (d) chose no dropped-block row through a pointer (the end-of-function row
  reports the block; record 0055 item 4), a design decision, not cut 2: a row that names the other owner needs a ghost
  per heap object (M4.5f). [d2b2b]
- **From M4.5d2b2b** (record 0055 item 7, "Further limits"; each a pinned false alarm in `tests/memory/negative/`,
  `owning_field_loop_*_false_alarm.c`, and in `memory.adoc`'s not-yet list; question 195: d2b2c fixes them first):
  - a loop through a non-owning heap alias of a borrowed structure (`v = p;`, `free(v->q)`): 0047 item 10's heap place
    does not carry its pointee's closure (audit S5's second half, not built);
  - a call through an alias `ps = &s` in a loop: the leaf's ghost is not refreshed (only `&s` itself is matched);
  - an owning `struct S *gp` global's field freed in a loop: the loop's places miss the cells of the global's block,
    and a written invariant fails the same way;
  - in `main`, a struct global's leaf reached only by type: no carry, since `program_start` does not fix a global's
    value; the il-analyst's proposal is a ghost-guarded synthetic site that carries the leaf only where `held_g` was
    true at entry;
  - the flag of a leaf carried by type, or of an owning local that may be null at the loop's entry, is lost when the
    loop neither stores the type nor hands the owner on (`uninitialized` on a later read; 0054 item 5's `wef`
    heuristic and the full carrier's flag loss);
  - a three-level chain in a loop (one level of closure is carried; recursing hit unplaced cells);
  - a loop that may replace the owner itself (no closure is carried across the replacement).
  Also from d2b2b's audits: a bodiless untied borrow gets two messages, the first misleading when the borrow is
  unguarded (round 2 on Part A, N1); a `static` cache cannot hand out a pointer into the buffer its leaf owns (N2;
  0054 item 7's cover does not reach blocks owned by a static leaf). [d2b2b]

**M4.8 C parity close-out, from M4.5d2b2b** (with questions 66 and 189): two owning fields replaced in one loop under
written invariants take 218.5 s (S40 `g3_two.c`, loaded machine; one field 6.3 s); the audit probe `cover_twice.c`
(two calls of a function returning `&s` of a `static` struct with an owning field, then frees through both, a failing
program) takes over 300 s. [d2b2b]
