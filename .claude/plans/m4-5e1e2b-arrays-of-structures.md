# Plan: M4.5e1e2, the continuation — `restrict` rejected, then Parts C to F (record 0064)

**Written on 2026-11-25 (02:00 UTC on the 26th) at a session that only planned, after M4.5e1e2's first session.**
That session built Parts A and B of `.claude/plans/m4-5e1e2-arrays-of-structures.md` ("the first plan") and
closed at split point 1. Question 310 said the parts not begun run from the first plan with no second planning;
**the owner asked for this plan instead** (2026-11-25: "Make a plan to implement the next milestone using the
handoff and planning documents"), because the first session changed what the rest stands on: record 0064 was
revised in eleven items, every line number of the first plan moved, `restrict` was found unsound, and four owner
questions were open. **This plan replaces the first plan for everything not yet built.** The first plan stays as
the history of Parts A and B, of P83 and of S48. **Record 0064 is the rule text**: this plan cites its items and
carries only what changes in them (section 6).

**Written for this ensemble** (owner, 2026-11-25): a Fable orchestrator at `high`, the main session, started with
`claude --agent orchestrator --effort high` (`.claude/agents/orchestrator.md`); three Sonnet implementers from
`.claude/agents/` (`edg-backend`, `il-analyst`, `test-mutator`); the Fable `soundness-auditor` at `xhigh`; the
Fable advisor. Section 2 assigns every task and section 12 holds each agent's briefing, ready to send.

**What the orchestrator reads before acting:** this plan, all of it; the entry "M4.5e1e2, first session" of
`internal/status.md`; `internal/handoff.md` §1, §2 and the rules of §4, §6 and §8; **record 0064, all of it** (it
is the design); record 0063 items 4, 5, 7, 9, 10 and 13; and, before the part named: 0060 items 3 to 11 (Part
C1); 0041 items 3 and 5, 0044 item 8 (Part C2); 0047 items 2, 4 and 6 to 8, 0052 item 2, 0055 items 3 and 5, 0058
item 4, 0045 items 2 to 7, 10 and 12 (Part D); 0060 item 15 (Part E); 0061 items 1, 3 and 6 to 11, 0062 items 4
and 7 (Part F); 0037 item 2 and 0043 item 10 (Part R, for what is withdrawn). **Execution starts at section 5**
(Step 0). The first message of the session names this file: `M4.5e1e2 continuation
(.claude/plans/m4-5e1e2b-arrays-of-structures.md)`.

Branch `m4.5e1e2b-arrays-of-structures` and worktree `.claude/worktrees/m4.5e1e2b` (**both exist**: this
planning created them from `f313023`, the tip of the first session's branch, and committed this plan there; the
worktree is source only, with no submodules and no build), `build/m4.5e1e2b/`, the box lanes
`~/cerastes-e1e2/{base,il,edg,work,audit}` (the first session's, reused), snapshot refs `refs/tmp/e1e2b-*`,
record `internal/decisions/0064-arrays-of-structures-and-arrays-of-arrays.md` (no new record), probe **P84**, the
chapter `## M4.5e1e2` of `internal/il-inventory.md`, self-test cases in `src/viper/selftest_cases36.inc`,
questions from **338** (331 to 337 are this planning's defaults, section 14).

## 1. Context

**Where things stand** (the tree at `f313023`; `internal/status.md` has the first session's entry).
- **Built:** Part A (leaf paths with index steps, every trap of the first planning made loud) and Part B
  (automatic local arrays of structures in code: `v[i].x`, `p[i].x` on any `struct S *`, `&v[i]`, initializers
  with C's zero fill, element copies, scope exits, loops with one place per leaf, invariant reads of `v[e].x`,
  sequencing keys); question 304's brace-elision rule as the compilers measure it (item 16); a `restrict` rule
  (item 19) that is **not complete**; S48 in full (79 fixtures, every decision table on its first branch, depth 3
  admitted); the lint's first rules. The full check is green at `aebf158`.
- **Not built:** ranges over structures in contracts, arrays of structures in static storage and as members,
  the six leftovers of a structure with an array member, arrays of arrays, quantifiers over members. Each is
  rejected today with a diagnostic and a pin that names its part.
- **Unsound today:** `restrict` (question 326). On `main` wrong programs verify; on the branch one form still
  does. Part R of this plan removes the acceptance altogether.

**Owner decisions.**
- **At this planning (2026-11-25, the question dialog):**
  - **326** (`restrict`: unsound on `main`, one form open on the branch): **"Reject restrict
    (Recommended)"**. Every `restrict` parameter is rejected with a diagnostic that says to drop the keyword;
    the acceptance rules of records 0037, 0043, 0044, 0052, 0060 and 0064 item 19 are withdrawn and their code
    removed; it is built as the continuation's first part with its own audit read; **the owner approved exactly
    17 files** (section 9, Flips R: 8 positives become rejection pins, their 9 error twins are retired; what
    the harness's twin rules add to that is question 337); a
    path-precise model of `restrict` is an M8 entry. The alternatives were to keep the enumeration and close
    the known form, and a path-precise session first.
  - **320** (the parameter spelling `int (*a)[3]`): **"Accept it in Part E (Recommended)"**: the pin
    `tests/memory/negative/array_reject_pointer_to_array.c` is retired for a positive of the same program
    with an error twin, and a new pin records that a pointer to an array as a local stays rejected (M8).
  - **322, second half** (a truth test or null comparison of an element's address on an array of scalars):
    **"Reject them too (Recommended)"**: one rule for every element address, by the tag
    `array_address_null_test`, early in the continuation. An existing file whose outcome changes comes to the
    owner before it is touched.
  - **325** (a lone zero that is not the literal `0`): **"Probe, reject if the IL can tell"**: Step 0 runs one
    IL probe (P84 (a)); if the IL tells the token `0` from the other spellings, they are rejected like the
    compilers in Part R; if not, the entry goes to M8.
- **Standing from the first planning:** 310 (the 12-hour rule: at each commit the next part starts unless the
  remaining work would pass twelve hours; nothing is cut to hold a session short; **what it said about "no
  second planning" is replaced by this plan**), 311, 312 and 304 (built), 313 to 319 (the defaults record 0064
  builds on; 316 is moot once `restrict` is rejected).
- **Standing:** 69 (no EDG work), 77 (one context, under about 12 h), 160 (a new test's expect line may be
  corrected after its first run, each listed in status), 176 (the owner handles CI), 277 (a twin with several
  real errors lists every one), 285 and 299 (a test slow on the box only because of what it must refute goes
  to `tests/long/` with a short default-tier twin); `check.sh` once, at the end, on the x86-64 box; every test
  run on the box; no test runs while a fix is queued; no Claude trailer; commits stay local.

**Terms** are record 0064's ("Decision", first paragraph): scalar, verified structure, aggregate element, leaf
path, index step, member step, depth.

**Scope.**
- **In, in this order** (record items in brackets):
  - **Part R — `restrict` rejected, and two small rules** [19 rewritten, 5, 16]: the rejection of every
    `restrict` parameter; question 322's rule for element addresses of arrays of scalars; question 325's rule
    if P84 (a) allows it.
  - **Part C1 — ranges over structures in contracts** [7, 8, 10 for ranges]: `owns_range(p, n)` and
    `reads_range(p, n)` with `p` a pointer parameter to a verified structure; element reads `p[e].x` in
    `requires`, `ensures`, `decreases` and under `old`; calls, returns, `f__wf`, the pairing rows; loops over a
    parameter's range, with `p[e].x` read in their invariants; `owning(n)` on such a parameter; P4222's
    markers over such a range.
  - **Part C2 — arrays of structures in static storage and as members** [9, 10 for globals, 12 for members]:
    global and `static` local arrays of structures with their initial values, `owns(g)` and `reads(g)`; an
    array member whose elements are structures (`t.v[i].x`, `owns(*p)`, `owns(p->v)`) in every storage of its
    structure; the value view over such members.
  - **Part D — the leftovers of a structure with an array member** [15]: heap storage (question 292), an
    `owning` or marked member beside an array member (293), markers on a pointer to such a structure (294),
    contract places through a heap read (306), partnerless `ensures` atoms, written atoms in loop invariants.
  - **Part E — arrays of arrays** [13; 2, 5, 6, 9 at depths 2 and 3]: `T m[N][M]` in every storage,
    `m[i][j]`, rows as pointers, computed types, array-of-rows parameters in the three spellings (question
    320), `v[i].a[j]`, depth 3.
  - **Part F — quantifiers and `pure` applications** [14]: members of a structure element under a binder;
    member arrays and, in a contract, global arrays as the pointer argument.
  - **The pages** with each part, **the tutorial section "Arrays in aggregates"** with Part C1 (question 319),
    extended by Part E.
- **Not here:** the table of record 0064 item 1, as section 6 amends it (the `restrict` row goes, since every
  `restrict` is rejected; the `int (*a)[3]` row goes, since question 320 accepts it). Each row is rejected and
  pinned, its message naming the session that lifts it.

**Size** (`src/` and `include/` insertions without the VIR self-test cases). The first plan estimated Parts A
and B at 1,400 to 2,400 and they landed 2,057 without the `restrict` rules (2,387 with them), in the upper
half, as M4.5e1e1 did. The ranges below are the first plan's, raised by a fifth for what audits add, and Part C
is cut in two (question 331):

| Part | Content | Estimate |
|---|---|---|
| R | `restrict` rejected; questions 322 and 325 | a net removal: about 150 added, 700 deleted |
| C1 | ranges over structures in contracts | 800–1,400 |
| C2 | static storage and members | 700–1,200 |
| D | the leftovers | 800–1,500 |
| E | arrays of arrays | 800–1,500 |
| F | quantifiers and applications | 600–1,100 |

**3,700 to 6,700 in all**, beside it the lint (500 to 900 lines with as much again in its self-test). **This is
more than one session.** By the rates section 7 gives, a session reaches Part C2's or Part D's commit before
the twelve-hour rule closes it; the parts not begun then run from this same plan in a further session
(section 7, "Split points"). Two more sessions are likely, three possible.

## 2. The ensemble

| Role (model) | Owns | Never touches |
|---|---|---|
| **Orchestrator** (Fable, `high`; the main session) | Step 0's first item; record 0064's amendments, written before code from section 6; the vprdiff verdict per part; the user pages in `doc/` and the tutorial section; status, handoff, `plan.md`'s Done row or sessions row; the amendment lines in the amended records; every commit; briefing (section 12), resuming and replacing the agents; `build/m4.5e1e2b/orchestrator-state.md` (kept current, times from `date -u`) | `src/`, `tests/`, `scripts/` (their owners make every change there, the small ones too) |
| **edg-backend** (Sonnet) | `src/backend/**`, `src/driver/**`; the build system, `scripts/box_lane.sh`, `scripts/check.sh` (its `VIR_CASES_MIN`, set at Commit 1 and in the preflight), the box lanes and the baseline; **the joined build of each part** with `scripts/regen_vpr.py` and `scripts/vprdiff.py` on it; `internal/encoding.md`, the developer guide's paragraph | `tests/`, `scripts/lint_viper.py`, `src/vir`, `src/viper` |
| **il-analyst** (Sonnet; **a fresh instance per part**: `-R` for P84, Part R and Part C1, `-C2`, `-D`, `-E`, `-F`) | `src/vir/**` (`vir.h`, `vir.cpp`, `check.cpp`, `lower.cpp`, `lowered.*`), `src/viper/**`, `src/viper/selftest_cases36.inc`; the probes of P84 under `build/m4.5e1e2b/p84/` and their paragraph in the chapter `## M4.5e1e2` of `internal/il-inventory.md` | `src/backend`, `tests/` |
| **test-mutator** (Sonnet) | `tests/**` (tests, twins, pins, exactly the Flips of section 9), `tests/mutants-equivalent.txt`, compiler measurements, `scripts/run_tests.py`, `scripts/lint_viper.py`, `scripts/rejections.py` and their self-tests, the example files under `doc/modules/ROOT/examples/` | `src/` |
| **soundness-auditor** (Fable, `xhigh`, read-only) | one round per commit that touches `src/` (section 7: rounds R, C1, C2, D, E, F, named by their part), each on a snapshot commit in `.claude/worktrees/m4.5e1e2b-audit` and the box's `audit` lane, each before its part's commit; **a fresh instance per round**, briefed with the earlier reports | everything (never edits) |
| **Advisor** (Fable) | every question first (section 11), and the checkpoints below | — |

- **At most three Sonnet agents alive at once:** edg-backend, the current il-analyst, the test-mutator. **Every
  role is a chain of instances**: each agent keeps `build/m4.5e1e2b/<role>-state.md` current (what is done,
  what is in progress, the files it holds, the next step), and when the orchestrator judges its context low, or
  at the hand-overs this table names, a fresh instance starts from that file, this plan, record 0064 and the
  last commit. This is the plan, not a deviation. **An il-analyst is replaced, never resumed, at a part's end;
  inside a part it is replaced at its half's proof if the orchestrator judges its context low** (`lower.cpp` is
  25,453 lines; Part B's lowering, about 800 changed lines of it, needed two instances and its audit fixes a
  third).
- **The advisor** is the `advisor` tool; if the orchestrator has none, an Agent call with `model: fable`,
  briefed with the question and the paths to read. Step 0 checks that one of the two answers.
- **Advisor checkpoints:** (1) the record's amendments (section 6) with P84's outcome mapped to its table,
  before audit round R; (2) every question, and any outcome outside a table (section 11); (3) each split point
  (section 7); (4) any cut; (5) before the box run.
- **Every test run is on the x86-64 box** (owner, 2026-11-24), **in a lane of its own per agent that builds:**
  `~/cerastes-e1e2/{base,il,edg,work,audit}` on `matt@clang4.cpp.al`, driven by `scripts/box_lane.sh` (image
  `cerastes-e1d:26.04`; containers `cerastes-e1e2-<lane>`, started with `--init`). `base` holds the baseline
  at the session's base commit and is not rebuilt; `il` is the current il-analyst's (its VIR cases through
  Silicon); `edg` is edg-backend's (its half's build and `rejections.py`); **`work` holds the joined builds,
  the regenerations and the test-mutator's runs**; `audit` the auditor's snapshots. **Every shell call that
  runs `box_lane.sh` sources `build/m4.5e1e2b/box.env` first** (Step 0 writes it; environment variables do not
  persist between an agent's calls, and the script's built-in defaults name another session's lanes until
  Step 0's change makes it refuse to run without them). Work is shipped as a commit object in a git bundle
  (`box_lane.sh ship`); **a snapshot name is fresh for every ship** (a re-used name fails silently on a
  non-fast-forward fetch), and **`git status --short` in the lane is empty before a joined proof** (a ship
  leaves untracked files behind: two retired pins survived one in the first session and cost a re-run). The
  Mac builds to compile and runs nothing but a single-file probe whose box round trip would cost more than the
  run (say so when choosing it). The final `check.sh` runs in `~/cerastes-s8/p-ubuntu-26.04`.
- **Builds:** every agent that edits `src/` builds its own change on the Mac through the lock (`lockf
  build/m4.5e1e2b/build.lock cmake --build build/cmake`) and in its own lane. The `work` lane is built by
  edg-backend only, for a joined proof; nothing builds there during a run.
- **The runner** (`scripts/run_tests.py`; it locks `build/vpr`) has one holder at a time in the `work` lane:
  the test-mutator. The il-analyst runs it in the `il` lane, over a directory that holds only its part's VIR
  cases (section 3, rule 5). Everyone else uses `build/cerastes verify <file> --keep-vpr`.
- **`lower.cpp`, `vir.h`, `vir.cpp`, `check.cpp`, `lowered.*`, `emit.cpp` have one holder at a time:** the
  il-analyst of the part, with a commit or an audit snapshot between holders. **The il-analyst that starts
  from a round's snapshot applies that round's fixes to those files.** An agent that needs a change in
  another's file asks the orchestrator, who passes it on.
- **Work ahead of the lowering.** While the il-analyst builds a part, edg-backend may build the next part's
  encoder half on a snapshot ref of its own (`check.cpp` still refuses the construct, so nothing is accepted
  early: section 7, "Fail closed between parts"), and the test-mutator writes the next part's tests from the
  record. A half built ahead is joined only at its own part's joined proof.
- **Every agent** (in each briefing of section 12):
  - reads `CLAUDE.md`, the plan sections its briefing names, record 0064 and the files it is assigned; edits
    only its own files;
  - **any question goes to the orchestrator, who takes it to the advisor first** (section 11); the agent does
    not wait idle: it goes on with whatever the question does not block;
  - writes each report to `build/m4.5e1e2b/<role>-report-<n>.md` as well as returning it, and keeps its state
    file current (the auditor cannot write: the orchestrator cuts each snapshot commit and the audit worktree,
    ships the snapshot to the `audit` lane, saves each audit report to
    `build/m4.5e1e2b/auditor-report-<part>.md`, **and copies the auditor's probe files into
    `build/m4.5e1e2b/audit-<part>/` the moment the report arrives**: they live outside the tree and vanish);
  - follows the build, lane and runner rules above and section 3's budget: **it runs no test that section 3
    does not list**, and reports the wall time of every new test's first run;
  - a test generator lives in `build/m4.5e1e2b/` and is never re-run over the tree;
  - tool code in the owner's style (`scripts/format.sh`; brace initialization, `U`, one declarator per
    declaration, no comment on a brace line); no Claude trailer on anything.
- **A reader lands with what it reads** (P77): nothing is registered and no name is added to `<cerastes.h>`;
  the encoder's acceptance of a construct lands in the part whose lowering gives it rows.
- **Session tools are committed** (question 193): anything Step 0 scripts goes to `scripts/` with a self-test,
  or its summary to `internal/il-inventory.md`.

## 3. The test budget

**The owner's rule** (2026-11-25: "I believe a wasteful amount of testing is occuring"): every test run this
session makes is in the table below with its reason, and a run that is not there is not made.

**The rules.**
1. **Byte identity replaces re-running.** After each part edg-backend regenerates every kept `.vpr` and `.map`
   (`scripts/regen_vpr.py`, about 25 s on the box) and compares with the baseline (`scripts/vprdiff.py`).
   **No existing program's Viper or map changes in this session, with one listed exception:** the 17 files of
   Flips R lose their Viper (8 become rejections, 9 are deleted); nothing else may differ or be missing.
   While that holds, no existing test is run before the box check. **Byte identity says nothing about a lint
   that changed:** a joined proof after a change to `scripts/lint_viper.py` also runs the lint over the
   regeneration and over the VIR self-test's output and `test_lint_viper.py`.
2. **A new test runs once,** when its part is joined, and again only if `vprdiff` against the previous part's
   regeneration shows its Viper or map changed, or its expectation was corrected. A part's run never includes
   the earlier parts' tests.
3. **Every new test is timed at its first run, on the box.** Over 60 s: reported in the part's report. Over
   120 s: a smaller program, or `tests/long/` with a short default-tier twin (question 285's rule), settled
   with the advisor at that part, not at the end.
4. **Counts are capped** (section 8: 160 test files in all). One positive and one error twin per rule of the
   record, one pin per rejected row. An audit's requested pins and twins come on top; a part that needs more
   than its cap for another reason goes to the advisor.
5. **The VIR self-test.** `cerastes-vir-selftest` writes every case without Silicon (seconds) and the lint
   reads them. **A part's new or changed cases go through `scripts/run_tests.py --expect-passed <n>` over a
   directory that holds exactly those cases, as `check.sh` runs the whole directory** — in the half's own
   proof and again in the preflight. A sweep that compares sets of error lines is not that run: the harness
   counts errors, and a case with two errors at one line passed such a sweep and turned the first session's
   check red (question 330).
6. **The lint:** each new rule has a refusing twin in `scripts/test_lint_viper.py`. No separate mutant sweeps
   of the lint or the harness.
7. **Front-end tests, goldens and the walker oracle** run only if `walker.cpp`, `register_attributes.cpp`,
   `annotation_checks.cpp`, `il_query.cpp` or `include/cerastes.h` changed, and then only `tests/annotations`
   and `tests/il`. This plan changes none of them.
8. **One validation pass after the last audit's fixes** (the preflight below), then the box check. A
   should-fix that touches no test becomes a listed limit, not a diagnosis task. An auditor's probe that runs
   past ten minutes is stopped and reported as a timeout.
9. **Cost shapes the tests** (question 328): a function that holds several arrays of structures is slow (1 to
   5 arrays of two-leaf structures in one function: 7, 11, 17, 36 and 65 s; five mixed ones timed out). Tests
   use two-leaf structures and at most two arrays of aggregates per function; a vacuity twin holds several
   `assert false` only if no path passes two of them (Silicon stops a path at its first failing assert).
10. **Never cut:** invariant 2's pairs, the audit of each `src` commit before it lands, the neutrality gate,
    the one full `check.sh`.

| Run | Who, where | When | Why |
|---|---|---|---|
| baseline: `regen_vpr.py` and the VIR self-test output at the base commit | edg-backend, lane `base` | Step 0 | the reference for rule 1 |
| R1, R3 and R4 (section 5) | test-mutator, lane `work`, frozen binaries | Step 0 | today's outcomes of every pin of section 9; `rejections.py`'s base; the `restrict` files' rows |
| P84's probes: the front end alone, single files | il-analyst-R, lane `il` | Step 0 | question 325; the typedef spellings of Part E |
| the compiler cells for question 322's scalar forms, if the first session's kept matrix lacks them | test-mutator, lane `work` | Part R | the rule is the measured set |
| the half's own proof: the build; the new VIR cases through `run_tests.py --expect-passed` (rule 5) | each implementer, its lane | each part | a half is proved before it is joined |
| joined proof: regen and `vprdiff` against the baseline and the previous part; the VIR self-test's output against the baseline's (existing cases identical, but for Part R's listed cases); `rejections.py --base`; **where the lint changed since the last joined proof:** `lint_viper.py` over the regeneration and over the VIR self-test's output, and `test_lint_viper.py` | edg-backend (the lint runs: test-mutator), lane `work` | each part, once | rule 1; the Flips |
| the part's new tests, once, timed | test-mutator, lane `work` | each part, after the joined proof, builds held | invariant 2; rule 3 |
| the auditor's probes | soundness-auditor, lane `audit` | each round | its judgement, bounded by rule 8 |
| **preflight**, once: `format.sh --check`; `py_compile`; `run_tests.py --check-headers`; `mutate.py --validate`; `test_run_tests.py`; `test_lint_viper.py`; regen and `lint_viper.py` over it; the VIR self-test directory, its lint, the session's cases through `run_tests.py --expect-passed`, `VIR_CASES_MIN`; **the steps of `check.sh` that follow the self-test** (the walker oracle, the tree checks), so the one check cannot stop before them; `rejections.py --base`; `build_docs.sh`; `check_doc_examples.py` (source and `--html`) | test-mutator and edg-backend, lane `work` (the docs on the Mac) | after the last audit's fixes | `check.sh`'s own steps for what the session touched, so the 55-minute run cannot fail late on them |
| `CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` | orchestrator, `~/cerastes-s8/p-ubuntu-26.04` | once, after the last code change | the full check (invariant 8 of `plan.md`) |

**Not run:** a full `run_tests.py` outside `check.sh`; any existing test whose Viper is unchanged; `mutate.py`'s
mutation run (M4.8's); P83 and S48 again (every row ran; their fixtures are tests the full check runs); the
self-tests of tools the session did not touch, which `check.sh` runs anyway.

**The target:** the box check starts within two hours of the last part's joined proof (the last audit round,
its fixes in one batch, the preflight), with the pages, the status entry and the handoff written while agents
run.

## 4. Survey facts (tree at `f313023`; read, not run, by four read-only surveys on 2026-11-25; a sample of each was checked against the tree)

`E` is `src/backend/encode.cpp` (26,777 lines), `L` is `src/vir/lower.cpp` (25,453), `C` is
`src/vir/check.cpp` (6,307), `V` is `src/vir/vir.cpp` (2,756), `H` is `src/vir/vir.h` (1,521), `P` is
`scripts/lint_viper.py` (6,912; its self-test 10,885 lines, 929 tests), `ST` is `src/viper/selftest.cpp`
(1,971). A line number is where a function starts unless a range is given; **an agent re-locates by name
before it edits**, since each part moves the lines of the next.

- **Nothing needs the EDG fork, and nothing is registered.** `include/cerastes.h` does not change.
- **How each layer reports.** `E`: `reject(Diag, position, text)`; an array type's reason is a string in a
  `VType` reported by `reject_type` (2694). `L`: `internal_error(loc, msg)` (2132; exit 3), and four guards
  that push a `Diag` directly (2149, 2168, 5674, 5857). `C`: `err(loc, msg)` (318). `V` has no error path.
- **The encoder's gates that name this milestone: 42 sites** (`E`; 8 name a part, 34 name only "M4.5e1e2").
  - **Part C1:** `contract_element_address` 1252 (`p[e].x` in a contract); `subscript_address` 1562 (`p[e].x`
    through a pointer in a loop invariant or measure); `spec_range_atom` 11652 ("over structures"; 11631 and
    11644 keep their M4.5e2 rejection and lose the words "structures: M4.5e1e2"); `owning_count` 4923.
  - **Part C2:** `array_vtype` 587 (an array of structures that is not an automatic local; it is reached
    without the `aggregate_elements` flag from `array_type_reason` 831, `global_array_vtype` 882,
    `array_literal_decay` 15612 and `struct_info` 25976; only `declaration` 19214 passes the flag);
    `place_address` 16127 (a use of an array whose declaration was rejected); `collect_array_elems` 2016 (a
    nested braced list in an array-of-scalars initializer; its array-of-arrays half is Part E's).
  - **Part D:** `read_marker` 3787, `apply_function_markers` 4563, `read_variable_markers` 14661 (markers and
    `owning` on a pointer to a structure with an array member: questions 292 and 294); `allocation_view`
    18421, `heap_call` 18556, `spec_block_atom` 18639, `invariant_block_atom` 18733 (the heap: 292);
    `struct_info` 25882 and 25931 (an `owning` or `ref_to_uninit` member that points at such a structure) and
    26102 (an array member beside an `owning` or marked member: 293); `reject_array_behind_load` 10783 (306);
    `finish_atoms` 13557 (partnerless); `invariant_atom` 11784 (written atoms). `heap_view` (18239–18304)
    still has its internal-error guard.
  - **Part E:** `array_vtype` 579 (an array of arrays) and 607–612 (an element structure with an array member,
    by `vir_element_has_array_member` 9054); `struct_element_pointee_reason` 1361–1372 (reported at
    `subscript_address` 1546); `check_signature` 3380 (`int a[][3]`, `int a[2][3]`); `describe_operation` 184
    (a row `m[i]` used as a pointer; reported at 16739 and 20766); `collect_array_elems` 2016.
  - **Part F:** `reject_named_array_in_contract` 1016 and 1032; `contract_element_address` 1192 and 1205;
    `subscript_address` 1436, 1487 and 1539; `array_decay` 1664; `member_array_decay` 1708; `elem_value_read`
    1760 (stays; re-pointed to M4.5e2); `application` 6978; `forall_base_ok` 12307 (its code branch, 12311);
    `read_forall_function` 12662 (a parameter that points to a structure).
  - **Not lifted** (its text is re-pointed): `struct_store_value` 19944 (`v[i] = f();`: question 333, M4.8);
    `struct_info` 25850 (`owning(n)` on a member: M4.5e2).
- **The encoder's gates that name another session and stay:** pointers to arrays outside a parameter (`E`
  `literal_address` 15794, `address_of` 16366 and 16408, `declaration_level_checks` 26189: M8); the M4.5e2
  texts in `contract_element_address` 1224, `contract_element_finish` 1293 and 1304, `subscript_address` 1602,
  `application` 6884–7058, `spec_range_atom` 11577 and 11698, `forall_base_ok` 12310, `forall_site` 13049 to
  13205.
- **Array parameters and their written type** (`E`). The adjusted type of `int a[]`, `int a[][3]`, `int
  a[2][3]` and `int (*a)[3]` is a plain pointer; the spelling is told by the parameter's `declared_type`
  (`tk_array` against `tk_pointer`): `check_array_parameter_bound` 643–662 (only the outer bound is checked),
  `check_array_parameter_declarations` 3137–3256 (its `shapes` lambda, 3188, never compares an inner
  dimension), `check_signature` 3357–3383, `declared_array_dimensions` 9090–9102 (new: `int a[][3]` counts 2,
  `int (*a)[3]` counts 0), `reject_computed_sizeof_operand` 7606 and 7624 (`sizeof(a)`: `int (*a)[3]` is not
  flagged, as the compilers have it). Every spelling of a pointer to a row is refused today by `vtype`'s
  pointer branch (2837–2881, the text at 2875, `unsupported_type`); `int a[][3]` is refused twice (3380 and
  that branch). `edg_index_depth` 9066 exists, unused, for the depth limit.
- **The encoder's machinery, where it is now** (`E`): `signature_vtype` 478, `array_vtype` 541–630,
  `check_array_type_bound` 735–788, `array_type_reason` 824, `global_array_vtype` 863, `reject_array_designator`
  1949, `collect_array_elems` 1967, `array_initializer` 2066–2212, `lone_zero_chain` 2216,
  `aggregate_array_initializer` 2237–2413 (Part B's reader of a nested list per element, through
  `collect_init_elems` 19530–19780), `vtype` 2732–2886, `read_struct_global_init` 9214,
  `read_array_element` 9453, `read_array_global_init` 9547, `static_local_decl` 9654,
  `add_static_local_atoms` 9798, `contract_place_address_raw` 10867, `spec_atom` 11301,
  `spec_range_atom` 11569, `forall_site` 12928, `struct_value_node` 15252, `object_value` 15313,
  `aggregate_value` 15907, `place_address` 16013, `contract_field_chain` 16198, `address_of` 16340,
  `operation` 16541, `declaration` 19139, `copy_struct` 19785, `struct_store_value` 19924,
  `struct_initializer` 20089, `struct_info` 25635–26131; the predicates `vir_struct_has_array` 9010,
  `vir_type_has_array` 9022, `vir_aggregate_has_marked` 9029, `vir_value_array_elements` 9106,
  `vir_value_over_limit` 9118, `vir_struct_has_marked` 9124, `edg_type_has_array` 9138,
  `pointer_to_array_struct` 9156 (it strips one pointer level).
- **`restrict` today** (Part R removes it). `E`: `vtype` 2849–2857 reads `TQ_RESTRICT` (rejected unless
  `allow_restrict` and C; a local, a global, a pointee and C++'s `__restrict` are `unsupported_type`);
  `function_of` 5770 records the head position in `restrict_at_`, 5787–5799 sets `params[k].is_restrict`,
  5848–5889 asks `vir::restrict_hazard` and `vir::restrict_backed` and rejects (`unsupported_annotation`, at
  the head); `report_restrict_bodies` 5901–5922 and `recheck_restrict_after_static_atoms` 9768–9796 (called
  from `Encoder::run` 26503–26504); `pure_function_rules` 6306–6318 and `read_forall_function` 12625–12637
  reject a `restrict` parameter of a `pure` function and of a quantifier's function outright. `V`:
  `may_hold_pointer_to` 1965, `holds_cell` 2016, `reaches_cell` 2032, `value_hands_pointer` 2043, the pair
  rule 2066–2234, `restrict_backing` 2247, `restrict_covering` 2280, `restrict_hazard` 2305–2419,
  `restrict_pairs` 2424, `restrict_backed` 2476, `restrict_pair_reason` 2505, `RestrictScan` 2525–2687,
  `restrict_body_reason` 2690; declared at `H` 1192, 1197, 1208, 1217; the flag is `H` 873. `C`: 1029, 1557
  to 1573, 5313. **`lower.cpp` never reads the flag: an accepted `restrict` adds nothing to the Viper.**
  Self-test cases: 61 (23 accepted, 38 rejected) in `selftest_cases8.inc` (625, 713, 723),
  `selftest_cases30.inc` (2565–2632) and `selftest_cases35.inc` (192; `define_restrict_body_cases` 395;
  `define_restrict_fix_cases` 1790), and `check_restrict_reach` (`selftest_cases35.inc` 325, called at `ST`
  1967). **Tests:** 71 files under `tests/` hold the word; 8 verify with a `restrict` parameter on a
  definition, 9 are their error twins, 33 already reject on a `restrict` line (30 parameters, all
  `unsupported_annotation` at the function's head; C++'s `__restrict`, a local and a global,
  `unsupported_type`), 1 rejects at a quantifier (`forall_reject_f_restrict.c`), 20 hold it only in a comment.
  **No test has `restrict` on a declaration without a body; no doc example, no vlibc header and not
  `cerastes.h` uses it.**
- **What Part B built for a leaf of an array of structures** (`L`): `ElemLeaf` 2195 and `elem_leaves` 2201 (the
  cell leaves of an element, member steps of any depth; an array leaf is an internal error, 2215: Part E);
  `leaf_slot` 2226 (the cell at slot `k` through a member path); `leaf_perm` 2231 (the quantified permission
  of one leaf); `agg_array_alloc_method` 1140; `ArrayPlace` 16526–16550 with `agg`, `leaf`, `leaf_text` and
  the helpers `ap_slot` 16553, `ap_perm` 16559, `ap_what` 16565; `array_place_hit` 4913–4936 (the leaf hit:
  one `parent_of(block_of(·))` per member step, never a block comparison); `inv_member_elem` 4850; in `C`,
  `local_array_type` 561, `local_array_reason` 574, `elem_key` 5702 and `in_loop_measure_` 5419. **There is no
  single leaf-range descriptor:** `range_perm` 4421, `range_quant` 4464, `in_range` 4474, `range_shape` 4484,
  `arr_quant` 7234 and the `over_slots` lambda (1283) still take one element type and one binder. Part C1
  gives the range helpers the leaf path.
- **Part A's guards that still stand** (`L`; each is lifted by the part that gives its site rows, and keeps its
  self-test case as a positive):
  - **for Part C1:** `elem_fkey` 2264 (callers `memory_fields` 9880, `call` 12735 and 12762); `element_range`
    4493 (callers `range_atom_value` 7951, `atom` 8231, `call_atoms` 12266, `atom_returns` 15787,
    `range_returns` 15939, `pf_atom_quants` 23359 to 23371, `apply_check` 24112); `site_fields` 7253 (its
    range branch calls `elem_cell_suffix` 2252 at 7260); `inv_member_elem`'s guard 4859 (a pointer base);
  - **for Part C2:** `report_aggregate_element` 2162 behind `checked_object_leaves` 2177 (21 callers) and
    `checked_value_leaves` 2184 (19 callers); `element_suffix` 2588; `leaves_of` 2141 (the old view);
  - **for Part D:** `atom` 8204 (an array member under a marker or without a partner);
  - **for Part E:** `elem_leaves` 2215; `check_flat_elem_base` 5670 (trap 1); `elem_base_kind_ok` 5851 (trap
    2); `fkey` 9722.
- **The walkers those guards dominate, as they read today** (`L`; record item 4; Part C1 gives each the
  general path or shows it needs none): `entry_checks` 15100 and `result_splits` 14953 work on sites that take
  a cell and skip range sites, so a range over structures gets no per-leaf rows from them;
  `given_result_splits` 14874 gives a structure element only the liveness split; `ens_amount` 7678 and
  `exclusive_pair` 7657 do not depend on the element type; `wf_keep_facts` 24380 calls `use_cell_type` on a
  `reads_range`'s pointee and `site_fields`, so it stops on a range over structures; `site_pointer_written`
  21383 tests one cell suffix, which for a structure element is never in the written set (**it would answer
  "not written"**); `flag_follows` 22114 matches a cell by member path with no index step;
  `note_global_member_write` 16777 walks `checked_value_leaves`; `need_lb` 17046 needs no element handling;
  `split_candidates` 14818 splits an array of structures against its first element only.
- **The checker's refusals that name this milestone** (`C`): `array_with_marked` 513 (293);
  `local_array_reason` 587 (depth 2: Part E); `global` 722 and `global_array` 778 (initial values of an array
  of arrays); `atoms` 1480 (partnerless: Part D); `range_atom_operands` 1658 (the condition at 1655: Part C1);
  `init_markers` 1884 (294); the `Deref` case 3690 (members and globals under a binder: Part F); the `Atom`
  case 3837 (292), 3852 and 3863 (written atoms), 3919 and 3925 (306); the `ElemAddr` case 4146 (306), 4192
  (a contract: Part C1), 4200 (a loop invariant or measure: Part C1), 4208 (an array-member pointee: Part E).
  Arrays of structures as members and globals are refused through `array_local_type` 535 and `array_reason`
  544 with record 0059's text. The quantifier body's whitelist is the switch at 3360–3387. Functions:
  `accepted_pointer` 340, `contains_by_value` 369, `owning_reaches` 391, `struct_type` 413, `pointer_reason`
  518, `global` 607, `global_array` 746, `range_atom_operands` 1616, `init_markers` 1863, `array_of_named`
  2358, `forall_loop_base` 2376, `forall_shape` 2416, `forall_node` 2644, `declare` 2740, `expr` 3354 (cases
  `AddrOf` 3620, `Deref` 3683, `Atom` 3751, `MemberAddr` 3946, `ElemAddr` 4096), `struct_has_array` 4450,
  `value_array_elems` 4470, `struct_has_marked` 4511, `apply` 5070, `object_of` 5760. `V`: `collect_leaves`
  53, `struct_object_leaves` 154, `object_leaf_paths` 242, `Program::array_of` 463, `Builder::elem_addr` 1050,
  `place_root` 1690, `atom_place` 1732. `H`: `LeafStep` 266, `LeafPath` 281, `kMaxByValueArrayElements` 301.
- **The lowering's sites, where they are now** (`L`; section 7's tables use these names): `leaf_count` 103,
  `leaf_span` 123, `has_pointer_leaf` 133, `ArrLeaf` 1054, `array_alloc_method` 1245, `alloc_struct_method`
  1391, `program_start_method` 1486–1627, `array_leaf_start_rows` 1641, `array_start_rows` 1655, `cell_suffix`
  2431, `use_cell_type` 2489, `array_alloc_site` 2537, `leaf_cells` 2675, `obj_cells` 2689, `leaf_arrays`
  2704, `struct_lens` 2756, `owners_of` 2812, `has_owning_leaf` 2824, `leaf_owners_of` 2836, `owner_value`
  2905, `owner_cell` 2930, `owner_text` 2936, `global_owners_of` 2954, `flags_kept_entry` 3587,
  `flags_kept_row` 3638, `zero_fill` 4332, `array_scope_exit` 4513, `scope_exit` 4565, `inv_elem` 4762,
  `inv_deref` 4884, `inv_range_chain` 4937, `inv_read_at` 4969, `inv_interval_frame` 5048, `inv_framing_any`
  5086, `struct_store` 5409, `rebind_member_root` 5499, `array_copy_stmt` 5528, `array_zero_stmt` 5628,
  `elem_base_key` 5864, `same_elem_base` 5877, `elem_key` 5918, `block_marker` 6272, `marked_field_site` 6297,
  `marked_through_load` 6308, `block_cells` 6460 and 6469, `block_lens` 6483, `block_pure` 6493, `block_conj`
  6533, `block_mode` 6569, `block_atom` 6596, `block_rows` 6689, `block_call_rows` 6791, `site_arrays` 7194,
  `arr_block_term` 7228, `overlap_pairs` 7298, `arr_disjoint` 7331, `site_marker` 7581, `cov_of` 7763,
  `leaf_cover_read` 7821, `range_atom_value` 7885, `atom` 7960–8258, `is_named_array` 8396, `global_facts`
  8462, `fq_base_formals` 8643, `fq_collect` 8734, `fq_binder_class` 8773, `fq_resolve` 8797, `fq_slot` 8860,
  `forall_expr` 8893–9131, `fq_loop_read` 9146, `fq_check_rows` 9235, `contract_read` 9334–9671, `fkey` 9717,
  `key_of_addr` 9831, `memory_fields` 9873, `struct_cell_write` 10341, `elem_value` 10925, `member_addr`
  11015, `elem_addr` 11079–11165, `call_atoms` 11557–12341, `call` 12343, `struct_ptr_uses` 12867,
  `atom_returns` 15216–15793, `range_returns` 15800–15958, `exempt_owned_fields` 15964, `block_returns` 16153,
  `expr_touches` 16355, `ap_ref` 16572, `RangePlace` 16592, `range_row_amount` 16618, `range_marked_rows`
  16643, `range_places` 16678, `elem_array_var` 16824, `writes_in_expr` 16838, `writes_in` 16932, `lb_chain`
  17088, `held_keys` 17181, `loop_places` 17239–17843, `written_atoms` 17883, `resolve_written` 17996, `loop`
  18406–20066 (Part B's per-leaf code is at 18659, 19413–19447 and 19973–19975), `owner_loop_sites` 20573,
  `owner_marked` 20929, `owner_invariants` 21993, `owner_rows` 22171, `owner_flag_rows` 22324,
  `has_range_atom` 22753, `push_clauses` 22820, `param_vars` 22845, `result_vars` 22864, `pf_atom_quants`
  23343, `apply_frame` 23494, `apply_inv` 23859, `apply_check` 24004, `wf_place` 24635, `wf_method`
  24659–25344.
- **Self-test mechanics** (`ST`): case builders `lc` 606 (accepted and lowered), `rc` 610 (refused by the
  checker with a text), `rl` 616, `ru` 622 (refused, lowered anyway, must give an internal error), `rh` 627;
  the includes are 1524–1557 and the `define_*` calls 1866–1871; `text_checks` (90, applied 1651–1658) pins
  emitted text. `selftest_cases35.inc` is the first session's (2,447 lines); **this session's cases go in a
  new file, `selftest_cases36.inc`**, one `#include` and one `define_*` call per group.
  `VIR_CASES_MIN=2512` (`scripts/check.sh` 104); the cases run through the harness at line 114.
- **The lint** (`P`). The first session added the clause sets of an allocation of an array of structures
  (`aggregate_array_problem` 3746, the `AGG_*` shapes 3735–3743), `parse_leaf_row` 4024 and `_row_acc` 4057,
  and taught `array_allocation_problem` 3685, `lint_array_allocations` 3878, `_range_acc` 4350 and
  `lint_quantified_rows` 5468 about leaf rows. To extend: `parse_struct_chain` 734, `struct_wf_lens_problem`
  756, `struct_allocation_problem` 798, `BLOCK_ROW_RE` 1012, `parse_block_row` 1016, `block_term` 1023,
  `struct_array_counts` 1180, `array_global_group` 1199, `BLOCK_BASE_RE` 1645, `member_bound_slots` 2126,
  `QUANT_HEAD_RE` 3971 (one binder), `parse_range_quantifier_full` 3986, `range_atom_row_problem` 4106,
  `array_global_counts` 4173, `method_len_facts` 4188, `BOUND_RHS_RE` 4203 (a literal slot only),
  `member_array_counts` 4207, `bound_locals` 4218, `block_row_problem` 4300; the user-quantifier rule is
  `USER_QUANT_RE` 4432, `parse_quantifier` 4465 and `quantifier_problem` 4495 (the trigger must be
  `loc(block_of(X), q<j>)`, 4520).
- **Pins** (section 9 has every row). 20 test files name this milestone: 11 staging pins the first session
  added, each naming its part, and 9 older pins. All are `// mode: c23, c++20`.
- **Docs today** (`doc/modules/ROOT/pages/`). `memory.adoc` (2,156 lines): `restrict` 261–308; `#structures`
  397; `#array-members` 515 (what a structure with an array member cannot do yet: 572–579; members that are
  arrays of structures: 581–593); `#arrays` 595; `#arrays-of-structures` 667 (its not-yet list 715–724);
  `#ranges` 726 (801–802; `restrict` 814–821); `#array-parameters` 823 (835–837); `#quantifiers` 839
  (908–921); `#globals` 969; `#global-arrays` 1028 (1056–1062); `#heap` 1335; `#owning-fields` 1700;
  `#not-yet` 2043 (the arrays bullet 2062–2076; `restrict` 2143 and 2147–2151). `verifying.adoc` 1267–1271,
  1290–1294 (`restrict`), 1307–1311. `overview.adoc` 99–103. `annotations.adoc` 306–307, 958–973, 1026–1028
  (`restrict` on a quantifier's function: still true), 1036–1046, 1055–1058. `error-messages.adoc` 754–768.
  `tutorial/arrays.adoc`: "What is next" 189–200. No doc example holds an array of structures, a matrix or
  `restrict`. `internal/encoding.md`: §2 at 24, §3 at 212, §6 at 1383, §8 at 1457; `restrict` at 1101–1103,
  1614 and 1859.
- **Records:** 0012, 0037, 0043, 0059, 0060 and 0063 carry a line that names 0064; **0024, 0028, 0044, 0045,
  0047, 0052, 0055, 0058, 0061 and 0062 do not yet**, and get theirs from the part that builds the item which
  amends them. The chapter `## M4.5e1e2` of `internal/il-inventory.md` starts at line 8919 and is the file's
  last.
- **Tools:** `scripts/box_lane.sh` (`snap`, `setup`, `ship`, `build`, `run`, `--self-test`, `--dry-run`)
  defaults to `cerastes-e1e1` (lines 38 and 39). `scripts/run_tests.py --expect-passed N` (3153).
  `regen_vpr.py --out`, `vprdiff.py <base> <new>`, `rejections.py <out> [--base]`.
- **Costs of a round trip:** the box regenerates 6,750 programs in about 25 s; its full check took 54 minutes
  twice (16,797 tests). It was under other users' load (20 to 140) for most of the first session: times
  measured there are upper bounds.

## 5. Step 0 (execution starts here)

1. **Orchestrator.**
   - **The base.** `git fetch`. The session's branch `m4.5e1e2b-arrays-of-structures` and its worktree
     `.claude/worktrees/m4.5e1e2b` exist. **The base is the branch's tip, and it must contain `aebf158` (the
     commit the full check passed on) and this plan** (`git merge-base --is-ancestor aebf158 HEAD`); if it
     does not, stop and say so. If `origin/main` or local `main` holds commits the branch lacks (the owner
     merged meanwhile), merge it into the branch (a merge, never a rebase; a conflict goes to the owner). Say
     in status which commit the base is. The base is green: the full check passed at `aebf158`, and the
     commits after it touch only `internal/`, `plan.md` and `.claude/plans/`.
   - **The worktree.** Submodules with `--reference` to the main checkout's module stores (edg, silicon, then
     silver); `scripts/bootstrap.sh`. Older worktrees and branches are left alone (the owner's step). `npm ls
     @antora/logger` in `doc/` shows one hoisted copy (nested copies: the advisor, then the owner). CI
     recorded only (`gh run list --workflow ci.yml --limit 5`).
   - **The first session's reports.** Copy `.claude/worktrees/m4.5e1e2/build/m4.5e1e2/` — the files
     `auditor-report-*.md`, `*-report-*.md`, `*-state.md` and the directories `audit1`, `audit2`, `audit3`,
     `s0/restrict`, `restrict2` — into `build/m4.5e1e2b/prev/`, so that they outlive that worktree. Section 13
     digests what Parts C to F need from them; the auditors are told to read them there. If that directory is
     gone, section 13 stands alone: say so in status and go on.
   - One question to the advisor, to check that it answers.
2. **edg-backend.**
   - `build/m4.5e1e2b/box.env` (`CERASTES_BOX_DIR=cerastes-e1e2`, `CERASTES_BOX_PREFIX=cerastes-e1e2-`), with
     the comment that every shell call sources it. **`scripts/box_lane.sh` is changed to refuse to run when
     either variable is unset** (question 336; today it falls back to another session's lanes), with its
     self-test.
   - The lanes: ship the base to each of `base`, `il`, `edg`, `work` and `audit`, remove what `git status
     --short` still lists there (untracked leftovers; the ignored build directories stay), rebuild. `df -h`
     first; fewer lanes with a note in status if the disk is short. One ship–build–run round trip is timed;
     over five minutes goes to the advisor.
   - The baseline before any change to `src/`: `scripts/regen_vpr.py` and the VIR self-test into the `base`
     lane's `build/e1e2b-base/`; frozen binaries kept in the `work` lane (`build/e1e2b-frozen/`).
3. **test-mutator, on the frozen build** (lane `work`, both modes):
   - **R1** today's outcome of every pin section 9 lists, row by row (each must be what section 9 says; a
     difference goes to the orchestrator before anything is flipped).
   - **R3** `scripts/rejections.py build/m4.5e1e2b/s0/rejected-base.txt`. After each part the same run with
     `--base` may differ only by section 9.
   - **R4** the 71 `restrict` files: each one's rows today (tag, line), kept as
     `build/m4.5e1e2b/s0/restrict-rows.txt`, the reference for Flips R's "unchanged by design" claim.
   - These are the only `run_tests.py` runs before Part R's joined proof.
4. **il-analyst-R: P84** (the front end alone, both modes, `build/m4.5e1e2b/p84/`; every claim with its
   `--il_display` or `--cerastes_walk` excerpt; the result is a paragraph `### P84` in the chapter `##
   M4.5e1e2` of `internal/il-inventory.md`). A row's last column says what is built for each outcome.

| Probe | What is asked | What is built |
|---|---|---|
| **(a) the lone zero's spelling** (question 325) | In an initializer where `{0}` elides braces, does the IL give the initializer constant of `{0}` a field that `{(0)}`, `{'\0'}`, `{false}`, `{NULL}`, `{nullptr}`, an enumerator, `{1 - 1}`, `{-0}`, `{+0}` and `{(int)0}` do not share (a kept `expr`, a literal flag, a constant kind), and is a macro that expands to `0` like the literal? clang's C is silent on `0`, `0u`, `0L`, `0x0`, `00`, `0b0`, `0ULL` and such a macro. Evidence is an IL field; re-reading the source text at the constant's position is not | **The IL tells every spelling clang flags from every spelling it accepts:** Part R rejects, in C, a lone zero that is not an integer literal wherever item 16's C++ predicate applies (an elided subaggregate whose parent is an array or a structure of two or more members), `unsupported_expression`, with pins. **It tells some of them** (say, a cast or a parenthesis but not an enumerator): **the advisor** (the default: reject the ones it tells, list the rest for M8). **It tells none:** nothing is built; the entry goes to M8's warning sweep with the probe's evidence |
| **(b) typedef'd row parameters** (Part E; record item 13) | `typedef int row[3];` with the parameters `row *a`, `row a[]` and `row a[2]`: the adjusted type, the `declared_type`, and whether the row's bound keeps its `expr` through the typedef | **The same pointer to the row's `tk_array` entry, the bound readable as written** (0063 item 14's typedef rule: a file-scope typedef's bound is a prologue constant): Part E accepts these spellings as it accepts `int (*a)[3]`. **Otherwise:** they stay rejected (pinned, M8), recorded |

5. **In parallel with items 2 to 4:** the orchestrator writes record 0064's amendments from section 6
   (invariant 6: before Part R's code). Advisor checkpoint 1 when P84's outcome is in.

## 6. Record 0064: what the orchestrator amends before code

Record 0064 as it stands at `f313023` is the rule text for Parts C to F. The orchestrator makes these
amendments and no others before Part R's code; an audit round's blocking finding amends it again.

- **The header.** The status line says what the continuation builds from which plan; the owner's answers of
  this planning (326, 320, 322, 325); the defaults 331 to 337; "the plan's section 8" in items 4 and 15 becomes
  "the continuation plan's section 7", and "audit round 4" in item 15 becomes "the audit round of Part D".
  **Amends** gains 0040 item 3, 0044 item 11, 0045 item 13, 0047 item 4, 0052 item 6 (d), 0054 item 2 and 0060
  item 14 (their `restrict` clauses, withdrawn by item 19).
- **Item 1.** The table of what is not here loses "`restrict` backed by a range over structures" (every
  `restrict` is rejected, item 19) and the row "a parameter written as a pointer to an array: pending question
  320" (accepted, item 13); it gains "a structure value stored into an element (`v[i] = f();`): M4.8"
  (question 333). In the "In" list, array-of-rows parameters name the three spellings without the condition.
- **Item 5** (question 322, the owner). The truth-test and null-comparison rule covers **every address formed
  by an index**: `&E[i]` and `&E[i].x` for `E` a named array, an array member or a pointer, whatever the
  element (`if (&a[1])`, `&a[1] == NULL`, `&p[1] != NULL` on an `int *p`, `&s.a[i]`, `&p->a[i]`), by the tag
  `array_address_null_test`. It is one predicate: `&p[0]` on a pointer is rejected with the other indexes,
  over-strict by the one cell GCC is silent on (question 335, listed). The paragraph "are unchanged here and
  are the owner's" goes.
- **Item 7.** "`restrict` backed by a range over structures" leaves the "Rejected" list.
- **Item 12.** `v[i] = f();` and `v[i] = (struct P){…};` stay rejected, with their own message, until M4.8
  (question 333); "the continuation may lift it" goes.
- **Item 13** (question 320, the owner). `T (*a)[M]` is accepted as `T a[][M]` and `T a[N][M]` are: the three
  spellings share one adjusted type (P83 (d)). The interim rule ("only when its written type is an array
  declarator") goes. A typedef'd spelling follows P84 (b). A pointer to an array as a local, a member, a
  result or under `&` stays M8's.
- **Item 16.** The lone zero that is not a literal follows P84 (a): the rule, or "left to M8" with the probe's
  evidence (question 325).
- **Item 19, rewritten** (question 326, the owner: "Reject restrict"). Its new text:
  - **The rule.** A pointer parameter qualified `restrict` (spelled `restrict`, `__restrict`, `__restrict__`,
    `int p[restrict]` or through a typedef) is rejected, `unsupported_annotation`, at the function's head (the
    position the earlier `restrict` rejections used), once per function, naming every such parameter: on a
    definition, and on a declaration without a body (question 332). **The qualifier is read where it is read
    today:** from the definition's own parameters when the unit holds the definition, from the declaration
    otherwise. So `restrict` written only on a prototype of a function that the unit defines without it
    stays accepted, as it is today: C attaches the promise to the block of the definition (6.7.3.1), and
    that definition makes none. The message says what to do: "cerastes
    does not model `restrict`; dropping the keyword does not change what a program with defined behaviour
    does, so drop it to verify the function". `restrict` on a local, a global, a member, a pointee or a
    result, and C++'s `__restrict`, stay `unsupported_type` as before (0032 item 2, 0037 item 15). A `pure`
    function's and a quantifier function's `restrict` parameter keep their own rejections (0043 item 10, 0061
    item 3).
  - **Why.** `restrict` promises that an object modified through the parameter is reached only through
    pointers based on it (C23 6.7.3.1), and a compiler may reorder accesses on that promise. Permissions
    belong to cells, not to provenance, so owning `*p` does not keep the promise: the function may reach the
    same cell by another name. Records 0037, 0043, 0044, 0052, 0060 and this item's first version accepted
    `restrict` under conditions that enumerate the routes to another name; three audit reads in one day each
    found a route the rule then in force missed, and one form still verified when the first session closed
    (two ranges of the same count 0 beside a global the contract holds). An enumeration is not a proof.
    Removing the keyword removes only undefined behaviour, so the rejection costs a user one word.
  - **What it withdraws.** The acceptance of 0037 item 2, 0040 item 3 (`owning` as the `owns` conjunct), 0043
    item 10, 0044 item 11, 0045 item 13, 0047 item 4 and 0052 item 6 (d) (block sites), 0054 item 2, 0060
    item 14 and this item's clauses (i) to (iv), the exemption and the pair rule. The first version's text
    stays in `git show f313023:internal/decisions/0064-arrays-of-structures-and-arrays-of-arrays.md`, with
    the programs that verified wrongly.
  - **Where.** The encoder rejects in `function_of`; `check.cpp` refuses any parameter that carries the flag
    (the backstop, with a self-test case); `vir.cpp`'s `restrict` functions go.
  - **Later.** A model of `restrict` (every access based on `p` inside `p`'s atom, every other access outside
    it, with a soundness argument in `internal/soundness/`) is an M8 entry. M4.6 decides what a library
    declaration with `restrict` parameters (`memcpy`) needs when vlibc's `<string.h>` arrives.
- **The table "Facts and the branches taken"** gains P84's two rows.
- **Consequences** gains: programs that verified with a `restrict` parameter are rejected until the keyword is
  dropped (8 tests; no doc example); the address tests of item 5 on arrays of scalars are rejected (no
  existing file holds one, by the first session's three passes).

**No existing program's Viper or map changes in this session, but for Flips R** (section 3, rule 1). Every
other item adds rows only for a program that holds a construct rejected today; where an existing function is
touched, the text it writes for a program without the new constructs is the same, and `vprdiff` shows it after
every part.

## 7. Parts, owners, order

`E`, `L`, `C`, `V`, `H`, `P` and the line numbers are section 4's. **Each part's column names the functions;
the record's items say what each must do.**

| Part | edg-backend | il-analyst | test-mutator |
|---|---|---|---|
| **R** `restrict` rejected; questions 322 and 325 | `function_of` 5787–5889: one rejection per function for its `restrict` parameters (item 19's message, tag and position), in place of the `restrict_hazard` and `restrict_backed` calls; `report_restrict_bodies` 5901–5922, `recheck_restrict_after_static_atoms` 9768–9796 and their members and calls go; `vtype` 2849–2857, `pure_function_rules` 6306 and `read_forall_function` 12625 keep their rejections (texts re-pointed). **Question 322:** `struct_element_address` 1855–1910 and `detect_array_address_test` 1914–1935 take an element of any type and a `->` chain (`&p->a[i]`); the diagnostic's words. **Question 325,** if P84 (a) says so: the C clause in `collect_init_elems` 19530 and `lone_zero_chain` 2216. `internal/encoding.md` 1101–1103, 1614, 1859 | **il-analyst-R, on edg-backend's snapshot** (the encoder calls `vir::restrict_hazard`, `restrict_backed` and `restrict_body_reason`, so the tree does not build until those calls are gone: "Order of work", step 2). `V`/`H`: every function only the `restrict` rules call goes (`may_hold_pointer_to` 1965, `holds_cell`, `reaches_cell`, `value_hands_pointer`, the pair rule, `restrict_backing`, `restrict_covering`, `restrict_hazard`, `restrict_pairs`, `restrict_backed`, `restrict_pair_reason`, `RestrictScan`, `restrict_body_reason`), **found by a search for each one's callers across `src/`** (the compiler does not name an unused function that `vir.h` declares); the flag `is_restrict` stays. `C`: 1557–1573 becomes the backstop (any parameter with the flag is refused, naming item 19); 1029 and 5313 keep refusing. Self-tests: section 9, Flips S | Flips R (the 17 files; the new positive `restrict_dropped.c` and its error twin; in the 34 pins that stay, the `Pins:` comment and, in 14 of them, the `twin:` line); the pins of question 322 and, if built, 325; `run_tests.py --check-headers`; `rejections.py --base` against R4 |
| **C1** ranges over structures in contracts | `spec_range_atom` 11652; `owning_count` 4923; `contract_element_address` 1252, `contract_place_address_raw` 10867 and `contract_field_chain` 16198 (`p[e].x` in a contract); `subscript_address` 1555–1566 (`p[e].x` in a loop invariant or measure, where the loop carries `p`'s range; with only a structure atom on `p` it stays rejected, the message saying to write `p->x`); a marker whose atom is a range over structures (`read_marker`); the C++ message for an element that reads its own array (the handoff's untaken should-fix: "used in its own initializer", as C prints) | **il-analyst-R** (the same instance goes on from Part R; replaced at its half's proof if its context is low). `C`: `range_atom_operands` 1655–1658; the `ElemAddr` case 4192 and 4200; the contract root rules of `Deref` 3683 and `MemberAddr` 3946 through `place_root`'s `ElemAddr` step (`V` 1690); `atom_place` (`V` 1732). `L`: **Part C1's table below**; the range helpers take a leaf path (`range_perm`, `range_quant`, `in_range`, `range_shape`, `arr_quant`: the same text as today for a scalar element); the pairing rows as item 7 writes them (`overlap_pairs` 7298, `arr_disjoint` 7331); the guards `elem_fkey`, `element_range`, `site_fields` and `inv_member_elem`'s lifted, each walker they dominate given the general path | Part C1's tests; the lint's range-atom rows per leaf path (`parse_range_quantifier_full`, `range_atom_row_problem`, `_range_acc`, the loop conjuncts) and **the rule that ties a scope exit's rows to its allocation** (audit round 2, issue 7); Flips A (C1); the doc examples and the tutorial's examples |
| **C2** static storage and members | `array_vtype` 587 for a global (`global_array_vtype` 863), a `static` local (`static_local_decl` 9654, `add_static_local_atoms` 9798) and a member (`struct_info` 25976): the flag `aggregate_elements` where the part gives rows; `read_array_global_init` 9547 and `read_array_element` 9453 for structure elements (through `collect_init_elems`; both escape directions per leaf); `struct_initializer` 20089 and `read_struct_global_init` 9214 for a member that is an array of structures (through `aggregate_array_initializer`); `place_address` 16124; the value view in `object_value` 15313, `aggregate_value` 15907, `signature_vtype` 478 and `copy_struct` 19785 (item 12's limit of 7 over leaves, `vir_value_array_elements` 9106); `collect_array_elems` 2016's text | **il-analyst-C2.** `C`: `array_local_type` 535 and `array_reason` 544 for a member and a global; `struct_type` 413, `global` 607, `global_array` 746; the Sequencer's `struct_store_leaves` with the index steps of an array member of aggregates, and its pin (audit round 2, issue 8). `L`: `program_start_method` 1486 with `array_start_rows` 1655 and `global_facts` 8462 (item 9's row group per leaf path); `struct_lens` 2756 and `alloc_struct_method` 1391 for a member that is an array of aggregates; `leaf_arrays` 2704, `site_arrays` 7194, `ArrLeaf` 1054, `arr_block_term` 7228; `atom` 7960 for `owns(*p)` and `owns(p->v)` with such a member; `held_keys` 17181 and `loop_places` 17239 (a global in `main`, a held global elsewhere, a held structure's member); the guard `report_aggregate_element` 2162 lifted from each of the 40 callers of `checked_object_leaves` and `checked_value_leaves` that the part gives rows, kept on the others; the value-view sites (`param_vars` 22845, `result_vars` 22864, `elem_value` 10925, `leaf_span` 123, `struct_store` 5409, `note_global_member_write` 16777); the walkers audit round 2's note 13 names (a structure global's leaves and `Aggregate` in `check.cpp`; the old view's users, loud through `leaves_of` 2141) | Part C2's tests; the lint's `array_global_group`, `struct_allocation_problem` and `struct_wf_lens_problem` for members and globals of structures; Flips A (C2) |
| **D** the leftovers | the gates of section 4's Part D list: `allocation_view` 18421, `heap_call` 18556, **`heap_view` 18239–18304 on the object view** (its guard goes), `spec_block_atom` 18639, `invariant_block_atom` 18733; `struct_info` 25882, 25931 and 26102; `read_marker` 3787, `apply_function_markers` 4563, `read_variable_markers` 14661; `reject_array_behind_load` 10783; `finish_atoms` 13557; `invariant_atom` 11784; `pointer_to_array_struct`'s one level | **il-analyst-D.** `C`: `array_with_marked` 513, `atoms` 1480, `init_markers` 1884, the `Atom` case 3837, 3852, 3863, 3919 and 3925, the `ElemAddr` case 4146, each lifted with its rule. `L`: **Part D's table below** | Part D's tests; the lint's typed views, `owns_block` lines, `free` rows and marked closures with an array leaf; Flips A (D) |
| **E** arrays of arrays | `array_vtype` 579 and 607–612; **a pointer to a row as a parameter's type** in the three spellings and, by P84 (b), the typedef'd ones: `vtype` 2837–2881, `check_signature` 3367–3383 (one report, not two), `check_array_parameter_declarations` 3137 (the inner dimensions compared) and `check_array_parameter_bound` 643; `edg_index_depth` 9066 for the limit of 3; `subscript_address` (`m[i][j]`; 1544–1546 for an array-member pointee) and `contract_element_address`; `describe_operation` 184 with `operation` 16541 (a row's decay); `collect_array_elems` 2016, `array_initializer` and `aggregate_array_initializer` for rows; `reject_computed_sizeof_operand` 7596, `scan_type` 7732, `array_type_reason` 824 and the typedef helpers, `check_array_type_bound` 735 per dimension; `global_array_vtype` and `read_array_global_init` for rows; `struct_info` for a matrix member; `v[i].a[j]`. `address_of` 16366 and 16408 and `declaration_level_checks` 26189 keep M8 for every other pointer to an array; `read_forall_function` keeps refusing such a parameter | **il-analyst-E.** `C`: `accepted_pointer` 340 and `pointer_reason` 518 for a `Ptr(Array)` **parameter only**; a row's address as an `ElemAddr`'s base and as a decay; `local_array_reason` 587, `array_reason` 544, `global` 722 and `global_array` 778; the `ElemAddr` case 4208; the Sequencer (`object_of` 5760, `elem_key` 5702). `L`: `elem_addr` 11079, `inv_elem` 4762 and `lb_chain` 17088 over an `ElemAddr` base (`loc(member(loc(B, i), 0), j)`, the second bound against the row's length), lifting `check_flat_elem_base` 5670; `same_elem_base` 5877, `elem_base_key` 5864 and `wf_method`'s tie with every index in the key, lifting `elem_base_kind_ok` 5851; `elem_leaves` 2215 (an array leaf of an element); **the leaf path with two and three index steps through every helper of Parts B, C1 and C2**; `alloc_arr_row_<t>_<bounds>` and its `zero_arr`; `fkey` 9722; `program_start`'s nested values; a range over rows; the copy `v[i] = v[j]` with `copy_arr_<t>`'s guard (item 12) | Part E's tests; the lint's second and third binder (`QUANT_HEAD_RE`, `BLOCK_ROW_RE`) and a row's bound local (`BOUND_RHS_RE`); Flips A (E) |
| **F** quantifiers and applications | `reject_named_array_in_contract` 1011–1037 with its callers in `application` 6873 and `forall_site` 12928; `contract_element_address` 1192 and 1205; `subscript_address` 1436, 1487 and 1539; `array_decay` 1664; `member_array_decay` 1708; `application` 6978; `forall_base_ok` 12307; `read_forall_function` 12662 (a parameter that points to a verified structure). `elem_value_read` 1760 stays (re-pointed to M4.5e2) | **il-analyst-F.** `C`: the body whitelist 3360–3387 and the `Deref` rule 3685–3691 (`MemberAddr` over the bound element); `forall_shape` 2416, `forall_node` 2644, `forall_loop_base` 2376, `array_of_named` 2358 (a member array; a contract's base); `apply` 5070. `L`: `fq_collect`, `fq_binder_class` (the class key gains the member steps), `fq_resolve`, `fq_slot`, `forall_expr`, `fq_loop_read`, `fq_check_rows`, `fq_base_formals`, `is_named_array`; `apply_frame`, `apply_inv`, `apply_check` (a member array through its bound local); `wf_method` (the ghost ties of element keys over a `GlobalAddr` or `MemberAddr` base under a binder) | Part F's tests; the lint's user-quantifier trigger with member steps and in the block-term form (`quantifier_problem` 4520); Flips A (F) |

**Part C1's table: where one range becomes one row group per leaf path** (`L`; **audit round C1 gives a verdict
per site**).

| Contract lines and facts | Calls and returns | Loops and invariant reads | The dominated walkers |
|---|---|---|---|
| `range_atom_value` 7885; `pf_atom_quants` 23343; `contract_read` 9334 (`p[e].x` framed by the range when its element slot lies inside, item 7); `wf_place` 24635; `wf_method` 24659 (the range facts, the `hvq` ghosts: one per leaf path and slot, never shared); `elem_key` 5918; `site_fields` 7253; `memory_fields` 9873; `has_range_atom` 22753 and `push_clauses` 22820; `cov_of` 7763, `leaf_cover_read` 7821 | `call_atoms` 11557–12341 (the per-atom record holds one bound temporary and one row group per leaf path: the shape row once with the length facts, then the marked rows, value then flag, and the flag and range rows per leaf; the messages of item 8); `range_returns` 15800; `atom_returns` 15216 (the sum terms and the array parts); `writes_in_expr` 16838 and `call` 12735 and 12762 (**a callee's writes through a range over structures are recorded per leaf field**: trap 3) | `RangePlace` 16592 per leaf path; `range_places` 16678; `range_row_amount` 16618; `range_marked_rows` 16643; `loop` 18406 (the place guards, entry rows, conjuncts and back edge of a range place per leaf, as Part B wrote an array place's); `inv_range_chain` 4937, `inv_read_at` 4969, `inv_interval_frame` 5048, `inv_framing_any` 5086; `inv_member_elem` 4850 for a pointer base whose range the loop carries; `apply_frame`, `apply_inv`, `apply_check` and the `fq_*` rows only as far as they must keep refusing (Part F) | `entry_checks` 15100, `result_splits` 14953, `given_result_splits` 14874, `ens_amount` 7678, `exclusive_pair` 7657, `wf_keep_facts` 24380, `site_pointer_written` 21383 (**today it would answer "not written" for a structure element**), `flag_follows` 22114, `note_global_member_write` 16777, `need_lb` 17046: each given the leaf paths or shown, in the report, to need none (section 4 has how each reads today) |

**Part D's table: the leftover sites** (`L`; **audit round D gives a verdict per site**).

| Heap (292) | Owners (293) | Markers (294) | Loads, partnerless, written (306) |
|---|---|---|---|
| `block_cells` 6460 and 6469 with an array leaf's lines (its callers: `block_conj` 6533, `block_atom` 6596, `block_rows` 6689, `call` 12343, `contract_read` 9334, `loop_places` 17239, `loop` 18406, the written-block functions); `block_lens` 6483 and `block_pure` 6493 (the member lengths); `block_call_rows` 6791, `block_returns` 16153; the leak row near `exempt_owned_fields` 15964; `obj_cells` 2689 (it drops array leaves, relying on question 292's refusal) | `owners_of` 2812, `has_owning_leaf` 2824, `leaf_owners_of` 2836, `owner_value` 2905, `owner_cell` 2930, `owner_text` 2936, `global_owners_of` 2954, `exempt_owned_fields` 15964, `owner_loop_sites` 20573, `owner_invariants` 21993, `owner_rows` 22171, `owner_flag_rows` 22324: each walks cell leaves and passes over an array leaf | `marked_field_site` 6297, `marked_through_load` 6308, `block_marker` 6272, `block_mode` 6569, `site_marker` 7581, `owner_marked` 20929, `flags_kept_entry` 3587 and `flags_kept_row` 3638; the array-part guards of `atom` 8204, `call_atoms`, `atom_returns` and `array_leaf_start_rows` 1641 for a marker | the array-part sites of `atom`, `call_atoms` and `atom_returns` with a loaded root bound to a `Ref` local; `contract_read` for `p->q->a[e]`; the same guards for an output amount (partnerless); `written_atoms` 17883 and `resolve_written` 17996 |

**Fail closed between parts.** What a later part lifts is rejected with a diagnostic and a staging pin: the 11
the first session added name their part, and a part retires exactly its own (Flips A). In `src/vir` the gate is
`check.cpp`'s: it refuses whatever no part has given rows, before lowering can see it. A session that closes at
a split point leaves the pins of the parts it did not begin as they are.

**Order of work.**
1. **Step 0** (section 5), with the record's amendments.
2. **Part R, its halves in this order** (they are not independent: the encoder and `check.cpp` call the
   functions that go). (i) edg-backend, after its Step 0 task, removes the encoder's calls into
   `vir::restrict_*` and writes the rejection; its snapshot is `refs/tmp/e1e2b-r1`. il-analyst-R runs P84
   meanwhile and prepares `check.cpp`'s backstop and the three cases. (ii) il-analyst-R, on that snapshot,
   removes `check.cpp`'s calls and the functions. (iii) The joined proof. The test-mutator makes Flips R and
   writes Part R's tests beside (i) and (ii); the orchestrator rewrites the `restrict` paragraphs of the
   pages (section 10). Advisor checkpoint 1. **Audit round R** (a short read), **Commit 1**: the record as
   amended, P84's paragraph, Part R, its flips and pages. il-analyst-R starts Part C1 as soon as its Part R
   half is in the joined snapshot; it does not wait for the round.
3. **Part C1.** **Audit round C1. Commit 2. Split point 1.**
4. **Part C2** (il-analyst-C2 may start from round C1's snapshot while the round runs, and applies its
   fixes). **Audit round C2. Commit 3. Split point 2.**
5. **Part D** (il-analyst-D). **Audit round D. Commit 4. Split point 3.**
6. **Part E** (il-analyst-E). **Audit round E. Commit 5. Split point 4.**
7. **Part F** (il-analyst-F). **Audit round F**, with the pages. **Commit 6.**
8. **The close-out, whatever commit is the session's last:** the doc examples (test-mutator); the user pages
   (orchestrator, drafted during the waits); `internal/encoding.md` and the developer guide (edg-backend);
   the amendment lines in the records the built items amend; the last audit's fixes in one batch; the
   preflight; advisor checkpoint 5; the box check; record 0064 accepted **for the items built**, the others
   "decided, built by the next session"; status, handoff, `plan.md`'s Done row (when Part F has landed) or its
   sessions row.

**Proofs, per part.**
- **Each half alone:** the il-analyst proves its half by the build and by the new VIR cases through
  `cerastes-vir-selftest` and `scripts/run_tests.py --expect-passed` (section 3, rule 5), which need no
  encoder; edg-backend proves its half by its build and by `rejections.py --base` against section 9. Both run
  `scripts/format.sh`.
- **The joined proof,** once both halves have reported: edg-backend cuts the snapshot `refs/tmp/e1e2b-j<n>` (a
  fresh name each time), ships it to the `work` lane, checks that `git status --short` there is empty, builds,
  runs `scripts/regen_vpr.py` into a directory of its own and `scripts/vprdiff.py` against the baseline (**no
  existing program's `.vpr` or `.map` differs or is missing, but for Flips R**) and against the previous
  part's regeneration, compares the VIR self-test's output with the baseline's, and runs `rejections.py
  --base`; where the lint changed, the test-mutator runs it over both outputs with its self-test. Then, with
  builds held, the test-mutator runs **the part's new tests and the earlier tests whose Viper that list shows
  changed, once**, timed. A failure goes through the orchestrator to the half that owns the failing file; the
  joined proof is repeated once, after the last fix, for what that fix changed.
- **The audit snapshot and the commit's tree are the tree the joined proof passed on,** taken from the
  snapshot ref, whatever the worktree holds by then.

**Audits** (a fresh Fable auditor per round, briefing 12.5; a snapshot commit, the local audit worktree and
the box's `audit` lane). Each round's list, for 12.5's last line:
- **Round R (a short read of a small diff):** is there any path on which a function with a `restrict`
  parameter is still accepted (a prototype before a definition, a typedef'd restricted pointer, `int
  p[restrict]`, a declaration without a body, a `static` function never called, a `pure` function, a
  quantifier's function); the backstop in `check.cpp`; that nothing else changed (the neutrality numbers; the
  33 pins' tags and lines against R4); the rows of the 8 re-expected files; question 322's predicate against
  the measured cells (a form GCC flags that is accepted; a form accepted before that no compiler flags and is
  now rejected, other than `&p[0]`); question 325's rule if built; the pages' `restrict` statements.
- **Round C1:** **Part C1's table, a verdict per site**; the alias traps (`f(v, 3, &v[1].x)`, `f(v, 3,
  &v[1])`, two overlapping ranges, each through a nested leaf and through a forwarding function); **the three
  pairings no fixture covers** (a `reads` site against an `owns` in the nested pairings; a cell held by a
  fraction; a forwarded structure atom of the element's own type); trap 3 (a callee's writes through a range
  over structures recorded per leaf field: a loop that keeps a value such a callee wrote); the amounts (the
  minimum over every leaf); `f__wf`'s ghosts (two leaves or two slots sharing one); an element read framed
  although its slot is outside the range; the length facts (a path on which they are assumed and never
  discharged); a range whose element structure has an `owning` or marked member, or an array member; that
  each lifted guard's walkers were given the general path.
- **Round C2:** `program_start` per leaf (the zero rest over the wrong elements; linkage; both escape
  directions through a pointer leaf; a list past P38's limit); the trusted row group (unsatisfiable, a
  (receiver, field) pair given twice, a fact not true of C); a member that is an array of structures (its
  length facts in `struct_lens`; `owns(*p)` and `owns(p->v)`; `f(&t, &t.v[1])`); the value view against the
  object view (a leaf dropped or doubled; the limit of 7); the Sequencer's `struct_store_leaves`; a `static`
  local's implicit atom; every caller of `checked_object_leaves` and `checked_value_leaves` either general or
  still guarded.
- **Round D:** **Part D's table, a verdict per site**; **the typed view's contract** for a structure with an
  array member (it is trusted: unsatisfiable, a pair given twice, a fact not true of C); a heap structure's
  array leaf after `free`, and in the leak check; `calloc`'s zero over every element; that an owner site may
  pass over an array leaf only because arrays of owning pointers are rejected (check the rejection); a marked
  closure's kept-flag row across a replaced block; a load-rooted array leaf keyed by the old pointer; an
  output amount for a partnerless array leaf; a written atom whose amount is not the first slot's.
- **Round E:** traps 1 and 2 (the address of `m[i][j]`; the second bound against the row; keys and ghosts with
  every index); a row's decay kept past its block; a row passed to a range callee that keeps one element;
  `Ptr(Array)` reaching any position but a parameter; the three spellings and the typedef'd ones (an inner
  bound not read as written; two declarations whose inner bounds differ); a range over rows whose count the
  inner bound does not match; the trusted `alloc_arr_row_*` (two row types sharing one method); depth 3;
  depth 4 rejected; the copy `v[i] = v[j]` with `i == j`.
- **Round F, with the pages:** a member class whose trigger or framing names another leaf's place; a member
  array or a global array argument read without an atom that covers it; an application over a member array
  whose callee's `requires` is not discharged; the pages' and the tutorial's statements about what is checked
  and what is trusted. **When the session closes earlier, its last round's snapshot holds the pages.**
- **Blocking fixes** are re-audited by the next round, or by a short read of the fix's diff when it is the
  last round's. Should-fix items not taken go to handoff §3 with their reason. **A soundness finding that is
  not of this plan's parts** (as `restrict` was in the first session) is reported at once, gets a fail-closed
  fix if one exists, and is the owner's otherwise; **one fix cycle, not three**: a second finding in the same
  rule after its fix goes to the advisor with "reject the construct" as the default.

**Cuts** (each leaves its pin, re-pointed to the session named, and the pages say what landed). Every spike
row took its first branch, so no cut is owed to a spike. **A cut is taken only on an audit finding, with the
advisor; time is never a reason** (question 310: a part that does not fit moves whole to the next session). In
this order: (1) a marker over a range of structures (M4.5e2, with question 287's filler); (2) `owning(n)` over
structures (M4.5e2); (3) depth 3 (M8); (4) a written atom and a partnerless atom on a structure with an array
member (M4.5e2); (5) the typedef'd row spellings (M8); (6) Part F's member classes (M4.5e2).
**Not cut without the owner:** the `restrict` rejection and question 322's rule; ranges over structures with
calls, returns, loops and the alias traps; global and member arrays of structures; heap storage, an `owning` or
marked member, markers and heap-read places for a structure with an array member (questions 292 to 294 and
306: they have moved twice already); a local matrix with `m[i][j]`; array-of-rows parameters in the three
spellings (question 320).

**Split points** (question 310). After each of Commits 2 to 5 the orchestrator reports `git diff --numstat
<base> -- src/ include/` without the self-test cases against section 1's sizes, the hours used, and what the
next part owes. **The rates measured in the first session:** about 400 `src/` lines an hour while a part is
built, both halves in parallel; an audit round 60 to 75 minutes, beside the next part's build; a round's fix
cycle 30 to 70 minutes; the close-out two hours (the pages, the preflight, the 55-minute box check, status and
handoff). It takes the sum to the advisor (checkpoint 3). **The next part starts unless that sum would pass
twelve hours;** otherwise the session closes at this commit with step 8, and the handoff names the part the
next session starts at. The decision and its numbers go into the status entry; the owner is not asked. **The
owner is asked** only if a part already begun would pass the cap, or if the orchestrator and the advisor judge
that its context will not carry what is left of it. **The next session** runs from this same plan: it is
started with the same command and `M4.5e1e2 continuation, from Part <X>
(.claude/plans/m4-5e1e2b-arrays-of-structures.md)`; the closing session writes its branch and worktree names
into handoff §2 (`m4.5e1e2c-arrays-of-structures`, `.claude/worktrees/m4.5e1e2c`, `build/m4.5e1e2c/`, the same
lanes); it runs Step 0 again on the new base (the worktree, the lanes, the baseline, R1 and R3; no P84) and
goes on from that part's line in "Order of work".

## 8. Tests (`tests/memory/`, scalar-only ones in `tests/scalar/`, negatives in the area's `negative/`; `.c` in `c23, c++20`; each positive has a twin failing for one reason at one line)

**At most 160 new test files in all**, the part's cap in brackets: one positive and one error twin per rule of
the record, one pin per rejected row of item 1's table; a rule that two shapes exercise the same way gets one
of them. A test's expectation is pre-registered from the record before its first run (question 160 for
corrections, each listed in status). Section 3's rule 9 shapes every test that holds an array of aggregates.

- **Part R [9].** `array_address_scalar_reject.c`: one row per measured form of question 322 (`if (&a[1])`,
  `&a[1] == NULL`, `&p[1] != NULL`, `!&s.a[i]`, `&p->a[i] == NULL`, `&p[0] != NULL`), with a positive that
  holds the forms that stay accepted (`p == NULL` of a pointer variable, `&a[1] == q` against another
  pointer, `a[1] == 0`). If
  P84 (a) builds question 325: `struct_init_reject_nonliteral_zero_c.c` (one row per spelling) and a positive
  with the literal spellings clang accepts (`0u`, `0x0`, a macro that is `0`). The positive `restrict_dropped.c` and its error twin `restrict_dropped_post.c` (Flips R). One
  new pin, `restrict_reject_declaration.c` (a prototype with a `restrict` parameter and a contract, no body,
  called), which also holds the form that stays accepted: a `restrict` prototype before a definition
  without the keyword. The 8 re-expected files are Flips R, not new tests.
- **Part C1 [33].** `range_struct_*`. `owns_range` over structures in a callee that writes `p[i].x`,
  `reads_range` in one that reads, each called with a local array (twins: a write under `reads_range`,
  `permission`; `n` past the array; a caller that does not hold the array). Element reads in a `requires`,
  an `ensures` with `old`, a `decreases` (twins: `postcondition`; unframed, `permission`). **The alias
  traps:** `f(v, 3, &v[1].x)` against `owns_range(p, n) && owns(*q)`, `f(v, 3, &v[1])` against a structure
  atom, two overlapping ranges (each `permission`; each positive passes another object or a disjoint half);
  the same through a nested structure's leaf (`&v[1].t.a`, a `struct T` atom on `&v[1].t`); **the three
  pairings no fixture covers** (a `reads` site in a nested pairing, a cell the caller holds by a fraction, a
  forwarded structure atom), each a positive with a twin. A function that forwards two of its own ranges. A
  loop over a parameter's range that sums and one that fills, each reading `p[e].x` in its invariant
  (twins); a loop after a callee wrote through the range (the value is not kept: a twin). A call that
  forwards `(p, n)`. `owning(n)` on a parameter of structures. A marker over a range of structures (a filler
  of two elements; twin: one member left unwritten, `uninitialized`). The listed false alarm `&p[n]` passed
  as a `struct S *` (a pin named `…_false_alarm.c`). `array_struct_reject_own_init.cpp` (C++'s message for an
  element that reads its own array). Pins: `owns_range(&v[1], n)`, `owns(p[i])`,
  `owns(v[i].x)`, `&p[i]` in a contract, a range atom in a loop invariant, a range over a result (M4.5e2); a
  range whose element structure has an `owning` member (M4.8).
- **Part C2 [26].** `global_array_struct_*`, `struct_member_array_struct_*`. `struct S g[3] = {{1, 2}, {3}}`
  with its listed and zero values, no initializer with internal linkage, the unknown forms (twins as
  M4.5e1e1's global arrays have them), `owns(g)` and `reads(g)` callees from `main`, a loop in `main`, a
  `static` local array of structures through a caller, a pointer leaf and its escape twin in a `.cpp`. An
  array member of structures: `t.v[i].x` in a local and under `owns(*p)`, `owns(p->v)` alone, its nested
  initializer, `f(&t, &t.v[1])` (`permission`), a structure global with one, a whole-structure copy within
  the by-value limit and the limit's pin, a write to `t.v[0].x` inside a value stored to `t`
  (`unsequenced_modification`). Pins: `owns_range(g, n)` and `owns_range(p->v, n)` (M4.5e2); `v[i] = f();`
  (M4.8).
- **Part D [35].** `struct_array_heap_*`, `struct_array_owner_*`, `struct_array_marked_*`,
  `struct_array_chain_*`. `malloc` and `calloc` of a structure with an array member, elements written and
  read, `free` (twins: a read of a `malloc`ed element before a write, `uninitialized`; a use after `free`; a
  second `free`, `invalid-free`; a `leak`). `owns_block(p)` in a constructor's and a destructor's contract;
  `owning` on a parameter, a result, a local and a member that point at such a structure (twin each: a leak
  or a use after the callee took it). An `owning` member beside an array member: allocate, use both, free
  (twins: the owned block leaked; a positive in which the array element is read after the owned block was
  freed). A marked member beside an array member. `ref_to_uninit` on a pointer to such a structure with a
  filler that writes every element (twins: the caller reads first, `uninitialized`; a `must_init` filler
  that misses one element). Heap-read places: `owns(*p->q)` and `p->q->a[i]` in a contract (twin each). A
  partnerless `ensures(r, owns(*r))` of such a structure (twin). A written `owns(*p)` in a loop invariant
  (twin). One `--strict-init` and one `--strict-owner` program over such a structure (`// verify-options:`).
- **Part E [37].** `matrix_*`, `array_rows_*`. A local matrix: `m[i][j]` written and read (twins: the first
  index outside, the second index outside, an unwritten element). Initializers: nested, partial, `{}`, C's
  `{0}` (twin; pins: `{1, 2, 3, 4, 5, 6}`, a designator, C++'s `{0}`). A nested fill loop and a nested sum
  (twins). A row `m[i]` passed to an `owns_range(q, 3)` and a `reads_range(q, 3)` callee and read back
  (twins). **An array-of-rows parameter:** `int a[][3]` with `owns_range(a, n)` in a callee that writes
  `a[i][j]`, called with `m` (twins: a caller holding fewer rows; a write under `reads_range`); the spellings
  `int a[2][3]` and **`int (*a)[3]`** (question 320: the retired pin's program as a positive, with an error
  twin); the typedef'd spellings as P84 (b) decides. `sizeof(m)`, `sizeof(m[0])`, `sizeof(int[2][3])`,
  `sizeof(struct P[3])`, a row typedef, a `static_assert` (twins: a `check` on the wrong value; a wrapped
  inner bound, `overflow` at the bound). A global matrix with initial values and a `static` local one
  (twins). A matrix member `u.m[i][j]`, in a local and under `owns(*p)`. `v[i].a[j]` over an array of
  structures with an array member (twins: each index outside), and the copy `v[i] = v[j]` of such elements
  (timed: 35 s in the spike). `&m[i][j]` stored and read back. Sequencing between two elements. Depth 3: a
  positive and a twin; depth 4: a pin (M8). Pins: `int (*p)[3] = m;` as a local and `&m[i]` (M8), `sizeof(a)`
  of the parameter (`sizeof_array_argument`), `owns_range(m[i], 3)` in a contract (M4.5e2).
- **Part F [20].** `forall_struct_*`, `forall_member_*`. `cerastes_forall` over `p[i].x` in the `requires` and
  `ensures` of a fill over structures, with its loop invariant (twins: `invariant-preserved`,
  `postcondition`); in a `check` over a local array of structures; a body that compares two members
  (`p[i].x <= p[i].y`; twin). A member array as the pointer argument: `cerastes_forall(f, p->a, 4)` in a
  contract under `owns(*p)`, and `(f, s.a, 4)` in a loop invariant and a `check` (twins: `precondition`;
  unframed, `permission`). A global array in a contract under `reads(g)` (twin). `pure` applications
  `sum(p->a, 4)` in an `ensures` and `first(g, 4)` in a `requires` (twins). Pins: `p[i].a[j]` and `m[i][j]`
  under a binder, a row `m[r]` as the argument, `p->q->a` (M4.5e2).
- **VIR self-test:** the session's cases in `src/viper/selftest_cases36.inc`; `VIR_CASES_MIN`
  (`scripts/check.sh`, edg-backend's) is set to the final count in the preflight. Part R retires accepted
  cases, so the count dips below 2512 at Commit 1: edg-backend sets it to that commit's count there, and
  status says so.
- **Byte identity:** `scripts/vprdiff.py` against the baseline after each part (section 3, rule 1).

## 9. Flips (pre-approved with this plan; any other changed outcome goes to the owner)

Every row below was read at `f313023` by the planning's survey, and Step 0's R1 checks each on the frozen
build before anything is touched. A flip is made by the part named, on the program as it stands. **In a file
with several rows, a lifted row keeps its line number with a comment in place of its lost expect line**
(M4.4c's rule), and the lifted construct gets a positive of its own with an error twin. Paths are in
`tests/memory/negative/` unless given.

- **R. `restrict`, approved by the owner at this planning** (question 326; exactly these 17 files).
  - **Re-expected from `verified` to `rejected cerastes-unsupported_annotation`, and moved to
    `tests/memory/negative/` under the same names** (the harness wants a must-fail test of this area under
    `negative/`, with a `twin:` line; `tests/README.md`, "Twins"). One row per function that has a `restrict`
    parameter, at the function's head line; the `native-ok` line goes unless the harness takes it beside a
    rejection; the `Pins:` comment names item 19; the `twin:` line names the new positive below.
    `block_restrict.c` (L10), `owning_restrict.c` (L8), `range_restrict.c` (L10, L18, L27),
    `restrict_body_owns_global.c` (L11, L20), `restrict_body_pair_ok.c` (L18, L26, L36, L46, L55),
    `restrict_owned.c` (L8, L18), `struct_global_restrict.c` (L14), `struct_value_restrict.c` (L12); a line
    shifts by what the header gains or loses. **The tag and the rejection are approved; a row that lands
    anywhere but the head of its function is reported by the test-mutator before the file is edited and
    settled with the advisor.**
  - **Retired** (the error twins of the above; a rejection pin has no error twin): `block_restrict_post.c`,
    `owning_restrict_post.c`, `range_restrict_wrong_post.c`, `restrict_body_owns_global_wrong.c`,
    `restrict_body_pair_ok_wrong.c`, `restrict_body_reads_global_wrong.c`, `restrict_owned_post.c`,
    `struct_global_restrict_post.c`, `struct_value_restrict_post.c`.
  - **The planning derived the 17 twice:** by its survey, and by a search of `tests/` for every file with
    `restrict` in code and no `rejected` row, which returns these 17 and no other.
  - **A new positive, which the rule needs** (invariant 2) **and the harness needs** (a `twin:` must name a
    test that verifies, and 14 pins name one of the 8): `tests/memory/restrict_dropped.c`, the functions of
    `restrict_owned.c` and `range_restrict.c` with the keyword dropped, as the diagnostic tells a user to
    write them (`verified`, `native-ok`, both modes), with its error twin
    `negative/restrict_dropped_post.c` (a wrong postcondition, both modes).
  - **Unchanged in their `expect:` rows by design: the 33 pins that already reject on a `restrict` line, and
    `forall_reject_f_restrict.c`.** The new rejection has the tag and the position the old ones had. What
    changes in them is the message, the `Pins:` comment and, **in the 14 whose `twin:` line names one of the
    8, that line, which becomes `tests/memory/restrict_dropped.c`** (question 337): `restrict_body_pair.c`,
    `reject_struct_value_restrict_static_atom.c`, `reject_restrict_reads.c`,
    `reject_restrict_owns_block_load.c`, `reject_restrict_cpp.cpp`, `reject_restrict_none.c`,
    `reject_restrict_owns_block_global.c`, `restrict_body_callee_arg.c`, `restrict_body_global.c`,
    `restrict_body_callee_pointer.c`, `restrict_body_callee_global.c`, `restrict_body_reach_forms.c`,
    `reject_struct_global_restrict_ptr_leaf.c`, `range_restrict_reject_unbacked.c`. **If R4's rows and Part
    R's `rejections.py` run differ in a tag or a line for any of the 34, that file goes to the owner before
    it is touched** (invariant 1). Whether to thin these pins, which now pin one rule 34 times, is question
    334.
- **S. The VIR self-test** (hand-built cases, not `expect:` lines; status lists each). Part R: the 23 accepted
  `restrict` cases and the 38 rejected ones (section 4 has their files and lines) are replaced by three
  rejection cases for the backstop (a `restrict` parameter with `owns`; with a range; on a function with no
  body), and `check_restrict_reach` goes with the function it called. Part C1: the case at
  `selftest_cases28.inc` whose text holds "structures: M4.5e1e" is re-expected with the lift. Each part: a
  Part A guard case whose guard the part lifts becomes a lowering case.
- **A. Retired for a positive of the same program, with an error twin:**
  - Part C1: `range_reject_struct_pointee.c` (L14); the staging pins `range_reject_struct_elem_reads.c` (L16,
    L23, L30) and `range_reject_struct_owning_count.c` (L14, L15); `array_struct_reject_loop_pointer.c` (L18)
    if its function holds a range of `p` (if it holds only a structure atom, the pin stays and is re-pointed:
    Flips C).
  - Part C2: the staging pin `global_array_reject_struct_elements.c` (L15, L23, L24);
    `struct_array_reject_members.c` L36 (a member `struct P v[2]`).
  - Part D: `struct_array_reject_expressions.c` L23 (`malloc`); `struct_array_reject_members.c` L21 (an
    `owning` member beside an array member); `struct_array_reject_annotations.c` L34 and L37 (partnerless),
    L39 (`ref_to_uninit`), L44 (`owning` on a pointer to such a structure), L54 (a written atom), L81 and L87
    (heap-read places); the staging pins `struct_array_reject_heap.c` (L20, L24, L29, L31, L35, L36),
    `struct_array_reject_marked_members.c` (L11, L25) and `struct_array_reject_markers.c` (L15, L22, L34).
  - Part E: `array_reject_array_of_arrays.c` (L10); `array_param_reject_array_of_arrays.c` (both rows of L9);
    `tests/scalar/negative/reject_sizeof_2d_array.c` (L9); `struct_array_reject_members.c` L41 (a member `int
    m[2][3]`); the staging pins `array_reject_matrix_forms.c` (L16, L17, L20, both rows of L26, L40),
    `array_reject_struct_elem_array_member.c` (L15) and `array_reject_struct_sizeof_type.c` (L15);
    **`array_reject_pointer_to_array.c` (L8), approved by the owner at this planning** (question 320), with a
    new pin `array_reject_pointer_to_array_local.c` for `int (*p)[3] = m;` (M8).
  - Part F: `global_array_reject_expressions.c` L52 and L57; `struct_array_reject_annotations.c` L62 and L75;
    the staging pin `forall_reject_member_array_code.c` (L23, L35, both rows of L52).
- **B. Re-expected from a rejection to an error:** none. If a part's `rejections.py` run shows an existing pin
  that item 13's rule for computed types turns from a rejection into an error (a wrapped inner bound in a type
  operand), **it goes to the owner before it is touched** (invariant 1).
- **C. Stay rejected; only the message or the `Pins:` comment changes:** `struct_array_reject_result_decay.c`
  (it names "M4.5e1e2/M4.5f3" and becomes M4.5f3's alone); `array_struct_reject_more.c` and
  `array_struct_reject_more_c.c`'s `v[i] = f();` rows (M4.8, question 333). Both files keep every row where
  it is: Part C1's fix of the C++ message for an element that reads its own array is pinned by a new file,
  `array_struct_reject_own_init.cpp`, and if that fix changes a row of an existing file, the file goes to
  the owner first; `array_param_reject_array_of_arrays.c`'s comment "naming e1e" and
  `tests/scalar/negative/reject_typedef_function_array_param.c`'s "e1e" (M4.7), if they survive Flips A; the
  rows whose comment says "to confirm at the joined run"; a row a cut re-points.
- **D. Unchanged:** `global_array_reject_expressions.c` L31, L37, L41, L47; `struct_array_reject_annotations.c`
  L68; `struct_array_reject_expressions.c` L18, L24, L25; `struct_array_reject_members.c` L26, L31, L46, L51,
  L56; `tests/scalar/negative/reject_sizeof_pointer_to_array.c`; every pin that names M4.5e2, M4.5f1, M4.5f3,
  M4.6, M4.7, M4.8 or M8. **If R1 or a part's `rejections.py` run shows one of them changed, it goes to the
  owner.**
- **E. Lint self-tests** that pin "this shape is refused" for a shape item 17 now admits (a second binder, a
  range row with a member step after its index step, a `program_start` group per leaf path): the test-mutator
  reports which before editing, and status lists them.
- **F. Questions 322 and 325.** Both are new rejections of forms no existing test or doc example holds (the
  first session's three passes; R3 and Part R's `rejections.py` and `vprdiff` runs check it again). **An
  existing file whose outcome either rule changes goes to the owner before it is touched** (invariant 1),
  with the recommended flip.
- **Goldens and the front end:** none. **A golden changing goes to the owner.** No tag loses its last test.

## 10. Docs (invariant 8; every example is a test)

**A session that closes at a split point writes the pages for what it landed.** Line numbers are section 4's.

- **Part R, `memory.adoc`:** 261–308 becomes one paragraph (a `restrict` parameter is rejected; what the
  keyword promises; why owning the object does not keep the promise; drop the keyword, which changes nothing
  for a program with defined behaviour); 814–821 loses its `restrict` clause; 2143 and 2147–2151 become one
  bullet (`restrict` on a parameter, with the reason and M8). `verifying.adoc` 1290–1294. `annotations.adoc`
  1026–1028 stays. `#arrays` and `#arrays-of-structures`: the truth-test rule for every element address.
- **`memory.adoc`, with the part named.** "Structures": `#array-members` (515) gains members whose elements
  are structures (C2) or arrays (E); 572–579 loses what Part D lifts and 581–593 what C2 lifts. "Arrays of
  structures" (667): 715–724 loses what C1, C2 and E lift; a subsection "Arrays of arrays" (rows, `m[i][j]`,
  a row as a pointer) with Part E. "Ranges" (726): ranges over structures, what `owns_range(p, n)` gives per
  member, the alias traps (C1). "Array parameters" (823, 835–837): array-of-rows parameters, the three
  spellings (E). "Facts about every element" (839, 908–921): members under the binder, member and global
  arrays as arguments (F). "Global variables" (`#global-arrays` 1028, 1056–1062) (C2). "The heap" (1335): a
  structure with an array member on the heap (D). "Owning fields" (1700): beside an array member (D). "What
  is not supported yet" (2043, 2062–2076): item 1's rows a user can meet, each with its session.
- **`verifying.adoc`** 1267–1271 and 1307–1311 (which arrays and array types are accepted). **`overview.adoc`**
  99–103. **`annotations.adoc`:** `owns_range` and `reads_range` over structures and rows (958–973);
  1036–1046 (the pointer argument of a quantifier); 306–307.
- **`error-messages.adoc`** (754–768): the forms `out-of-bounds` (the second index), `permission`,
  `uninitialized`, `lifetime`, `invalid-free` and `leak` gain; the `restrict` rejection's message.
- **Examples** (`doc/modules/ROOT/examples/memory/`): `array_of_structs.c` and
  `array_of_structs_alias_error.c` (C1), `struct_array_heap.c` and its error variant (D), `matrix.c` and
  `matrix_index_error.c` (E), `forall_struct.c` (F); the existing examples unchanged.
- **The tutorial** (questions 298 and 319): a section **"Arrays in aggregates"** in `tutorial/arrays.adoc`,
  before "What is next" (189): an array of points filled and summed through a range contract, the alias trap
  as its failing variant (with Part C1), and a matrix once Part E has landed; `console` blocks copied from
  runs in `doc/modules/ROOT/examples/tutorial/` (`aggregates_points.c`, `aggregates_points_alias.c`,
  `aggregates_matrix.c`); "What is next" loses what landed.
- **`internal/encoding.md`:** §2 (the array types, `Ptr(Array)` parameters), §3 (a paragraph per part after
  the first session's), §6, §8; the `restrict` lines. **The developer guide** where it names the leaf
  enumeration and the trusted methods. **One amendment line in each amended record,** written with the part
  that builds the amending item: 0037, 0040, 0043, 0044, 0045, 0047, 0052, 0054 and 0060 with Part R (their
  `restrict` clauses withdrawn); 0024 (E), 0028 (C2, E), 0044 (C2), 0045 (D), 0047 (D), 0052 (D), 0055 (D),
  0058 (D), 0061 (F), 0062 (F). `tests/README.md` only if a rule of the harness changes.

## 11. Questions, stops, acceptance

**Every question goes to the advisor first** (owner, 2026-11-25: "If you or the subagents have questions first
consult the Fable advisor, and then if you still need an answer or guidance you may stop and ask me"). An
agent's question goes to the orchestrator; the orchestrator states it to the advisor with the facts and a
recommended answer, saying first whether the matter is fail-closed or unsound; the agent goes on with what the
question does not block.
- **The advisor settles:** a choice inside the record's stated rules; a choice between fail-closed
  alternatives (a rejection with a pin, a listed false alarm); test mechanics, message wording and a
  question-160 correction; a table's "the advisor" row; a cut on section 7's list and a split-point decision
  under its rules; whether a slow test is restructured or moved under question 285's rule.
- **Goes to the owner, and the work it blocks waits:** an `expect:` change outside section 9 and question 160
  (invariant 1); **any existing program's Viper or map changing, other than Flips R**; a golden changing; a
  cut that section 7 protects or that is not on its list; a soundness finding that has no fail-closed fix (a
  wrong program verified, a trusted contract the auditor does not accept); a part already begun that would
  pass the twelve-hour cap; anything the advisor says is the owner's.
- **Each answer the advisor settles is logged** in `internal/handoff.md` §8 under "Settled with the advisor in
  M4.5e1e2's continuation, for the owner's review": numbered from 338, the soundness statement first, the
  answer taken, the alternative. The status entry lists them.
- **While the owner is away,** a question that is the owner's waits; the session goes on with everything it
  does not block. The overnight rule (build the fail-closed recommendation and log it) applies only if the
  owner gives it for this session.

**Other stops:** an exit 3 or a time over 120 s not settled within its part (section 3, rule 3); a second
failure of a joined proof on the same cause.

**Acceptance.**
- The preflight of section 3, once, after the last audit's fixes. Advisor checkpoint 5.
- `CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` **once**, after the last code change, on the x86-64 box
  (the branch as a git bundle into `~/cerastes-s8/p-ubuntu-26.04`): green. It is not skipped; if the owner
  ends the session before it, status says "not run" in its first line.
- Record 0064 accepted for the items built, with P84's rows in its table.
- The status entry (sizes measured against section 1, the hours per step, the deviations, the corrections
  under question 160, each cut and split taken, each advisor-settled question, each VIR case re-expected) and
  the handoff with its Questions section and what the session leaves (section 14). **If the session closed at
  a split point,** the handoff's §1 and §2 name the part the next session starts at and how it is started
  (section 7, "Split points"). Commits stay local.

## 12. Briefings (the orchestrator sends these; `<n>` is the report's number)

A briefing is all an agent knows. Each one below is sent as written, with the bracketed parts filled in; a
later task goes to the same agent by SendMessage in the short form of 12.7. A fresh instance of a role gets the
role's briefing again with "You continue from `build/m4.5e1e2b/<role>-state.md`; read it first" and the last
commit's hash in place of the first task.

**12.0 The common block** (the first paragraphs of every briefing):

```
You are one agent in an execution session of cerastes, a C/C++ verifier (EDG front end -> VIR -> Viper/Silicon
-> Z3). The session continues M4.5e1e2: arrays whose elements are structures or arrays. Its first session
built local arrays of structures; this one rejects C's `restrict` (it was accepted unsoundly), then builds
ranges over structures in contracts, arrays of structures in static storage and as members, what a structure
with an array member still cannot do, arrays of arrays, and quantifiers over members.

Work only in the worktree .claude/worktrees/m4.5e1e2b (branch m4.5e1e2b-arrays-of-structures); name it in
every command, because a shell `cd` can leave it. The session's plan is
.claude/plans/m4-5e1e2b-arrays-of-structures.md. The design is the decision record
internal/decisions/0064-arrays-of-structures-and-arrays-of-arrays.md: where the plan and the record differ on
a rule, the record wins and you tell the orchestrator. Read CLAUDE.md, the plan sections and the record items
named below before you change anything. Line numbers in the plan were read at the session's base commit; find
each function by name before you edit it.

A wrong program reported as "verified" is the worst outcome this project has. Where the record does not state a
rule, do not choose one: ask. A construct the record does not accept is rejected with a diagnostic at its C
line, never approximated.

Rules for every agent in this session:
- Edit only the files listed as yours. A change you need in another file goes to the orchestrator.
- A question goes to the orchestrator, who takes it to the advisor. Give the facts and what you would do, say
  first whether the matter is fail-closed or unsound, and go on with whatever the question does not block.
- Tests run on the x86-64 box only (matt@clang4.cpp.al), in your lane, and only the runs the plan's section 3
  lists for you. Time every new test at its first run and report the time. Run nothing on a build that a
  queued fix will replace.
- Every shell call that uses scripts/box_lane.sh starts with `. build/m4.5e1e2b/box.env &&`. The lanes are
  ~/cerastes-e1e2/{base,il,edg,work,audit}. Other sessions' directories on the box are not yours: never build
  in them, run in them or remove them.
- Build on the Mac through `lockf build/m4.5e1e2b/build.lock cmake --build build/cmake`, and in your lane with
  scripts/box_lane.sh. Only edg-backend builds the `work` lane, and nothing builds there during a run.
- A snapshot shipped to a lane gets a name never used before. After a ship, `git status --short` in the lane
  must be empty before the lane's result counts.
- scripts/run_tests.py has one holder at a time in the `work` lane. Unless you are told you hold it, run
  single files with `build/cerastes verify <file> --keep-vpr`.
- Keep build/m4.5e1e2b/<role>-state.md current: what is done, what is in progress, which files you hold, the
  exact next step. A fresh instance may have to continue from that file alone.
- Write every report to build/m4.5e1e2b/<role>-report-<n>.md and return it too. Report what happened: a check
  that did not run is "not run", and a failure comes with its output.
- A script that writes tests lives in build/m4.5e1e2b/ and is never run again once a file it wrote is in the
  tree.
- Tool code follows CLAUDE.md's style (scripts/format.sh; brace initialization, `U` on unsigned literals, one
  declarator per declaration, no comment on a line with a brace).
- Never edit an `expect:` line of an existing test, except the Flips of the plan's section 9.
- Do not commit; the orchestrator cuts every commit. No Claude attribution on anything.
- If a permission is denied, stop that action and report it.
```

**12.1 edg-backend** (first task: Step 0, then Part R):

```
[12.0]
Your role is edg-backend. Yours: src/backend/**, src/driver/**, the build system, scripts/box_lane.sh, the
five box lanes and the baseline, the joined build of each part with scripts/regen_vpr.py and
scripts/vprdiff.py, internal/encoding.md and the developer guide's paragraph. Never yours: tests/,
scripts/lint_viper.py, src/vir, src/viper. Your lane is ~/cerastes-e1e2/edg. State file:
build/m4.5e1e2b/edg-backend-state.md.

Read: the plan's sections 1 to 5, 7 and 9; record 0064, all of it (item 19 as the orchestrator has rewritten
it); record 0063 items 4, 5 and 13.

Task 1, Step 0 (plan section 5, item 2). Write build/m4.5e1e2b/box.env. Make scripts/box_lane.sh refuse to
run when CERASTES_BOX_DIR or CERASTES_BOX_PREFIX is unset, with its self-test. Ship the base to each lane,
make `git status --short` empty there (the ignored build directories stay), rebuild, and time one
ship-build-run round trip. Take the baseline in the `base` lane before any change to src/:
scripts/regen_vpr.py and the VIR self-test's output into build/e1e2b-base/. Keep frozen binaries in `work`
(build/e1e2b-frozen/). Report 1: the lanes, the round-trip time, the baseline's counts.

Task 2, Part R, your half (plan section 7, row R; record items 19, 5 and 16). Yours comes first: the
il-analyst removes vir's restrict functions on your snapshot, and the tree does not build before your calls
into them are gone. Every function with a `restrict` parameter is rejected once, at the position the old
`restrict` rejections used, with the tag unsupported_annotation and item 19's message; the calls into vir's
restrict functions and the two later passes go. Cut the snapshot refs/tmp/e1e2b-r1 as soon as that much
builds and tell the orchestrator, then go on. Then question 322's rule for every element address, and question 325's rule if the orchestrator
tells you P84 (a) allows it. Proof: your build; scripts/rejections.py with --base shows, for the 34 pins that
already reject `restrict`, the same tag and line as before (the message may differ), new rows only for the 8
files of Flips R, and nothing else changed. Report 2: what you removed and added, the proof's output, and any
existing file whose outcome changed that the plan's section 9 does not list.
```

**12.2 il-analyst-R** (P84, then Part R's half, then Part C1):

```
[12.0]
Your role is il-analyst, the first of this session. Yours: src/vir/** (vir.h, vir.cpp, check.cpp, lower.cpp,
lowered.*), src/viper/**, the session's self-test cases in src/viper/selftest_cases36.inc, the probes under
build/m4.5e1e2b/p84/ and the paragraph "### P84" in the chapter "## M4.5e1e2" of internal/il-inventory.md.
Never yours: src/backend, tests/. You are the only holder of the src/vir and src/viper files until your part's
audit snapshot is cut. Your lane is ~/cerastes-e1e2/il. State file: build/m4.5e1e2b/il-analyst-state.md.

Read: the plan's sections 1, 3, 4, 5 (item 4), 7 and 13; record 0064, all of it; record 0063 items 4, 5, 7
and 10; record 0060 items 3 to 11; the chapter "## M4.5e1e2" of internal/il-inventory.md (S48's rows (b), (c)
and (d) and "S48 after audit round 1" hold the Viper shapes Part C1 writes).

Task 1, P84 (plan section 5, item 4): the two probes, front end only. A claim about the IL cites the probe
file and line with an excerpt you produced; never round up. Report 1: for each row, which branch of its table
it took, with the evidence.

Task 2, Part R, your half (plan section 7, row R; plan section 9, Flips S). It starts from edg-backend's
snapshot refs/tmp/e1e2b-r1, in which the encoder no longer calls vir's restrict functions: the orchestrator
tells you when it exists, and until then you prepare check.cpp's backstop and the cases without removing
anything the encoder still calls. Then remove check.cpp's calls and, from vir.cpp and vir.h, every function
that only the restrict rules call. Find them by searching for each function's callers across src/: the
compiler does not report an unused function that vir.h declares. The flag is_restrict stays, and check.cpp
refuses any parameter that carries it. Replace the restrict self-test cases by the three backstop cases.
Proof: the whole tree builds with -Werror; the VIR self-test's output is identical to the baseline's for
every case that is not a restrict case; the three new cases pass through scripts/run_tests.py
--expect-passed. Report 2, then go on.

Task 3, Part C1 (plan section 7, row C1 and Part C1's table; record items 7, 8 and 10). Three things carry
the soundness of this part, so say in your report how each is met:
1. No existing program's Viper may change. A function you touch must write the same text as before for a
   program without the new constructs; edg-backend's vprdiff will show it.
2. Every walk of a structure's or an array's leaves either handles the general leaf path or stops with an
   internal error on a path it does not handle. A leaf must never be read as a cell of another shape. When
   you lift a guard, every walker it stood in front of gets the general path, or your report says why it
   needs none.
3. Every obligation is an explicit row in source terms before the access or call it guards (record 0028 item
   11), and every row that pairs two sites is written per pair of leaf paths by slots, as item 7 writes it:
   one parent_of(block_of(.)) per member step, the slot row for two ranges, never a bare block comparison.
check.cpp refuses whatever this part does not lift, and refuses it before lowering can see it.
Proof of your half: the build; the new VIR cases through cerastes-vir-selftest and
scripts/run_tests.py --expect-passed <n> over a directory that holds exactly those cases, in your lane;
scripts/format.sh. Report 3: the functions you changed, each site of the plan's table with what it does now,
the guards you lifted and the ones you kept, the cases and their times, and anything you left as a listed
limit.
```

**12.3 il-analyst-`<part>`** (the lowering half of Part `<X>`: C2, D, E or F; a fresh instance):

```
[12.0]
Your role is il-analyst, for the VIR and lowering half of Part <X>. Yours: src/vir/** (vir.h, vir.cpp,
check.cpp, lower.cpp, lowered.*), src/viper/**, and the session's self-test cases in
src/viper/selftest_cases36.inc. Never yours: src/backend, tests/. You are the only holder of those files
until your part's audit snapshot is cut. Your lane is ~/cerastes-e1e2/il. State file:
build/m4.5e1e2b/il-analyst-state.md (your predecessor's notes are in it; read them first), and your
predecessor's last report, build/m4.5e1e2b/il-analyst-report-<n>.md.

Read: the plan's sections 1, 3, 4, 7 (Part <X>'s row and its table) and 13; record 0064, all of it; record
0063, all of it; [the records the plan's opening names for Part <X>]; the chapter "## M4.5e1e2" of
internal/il-inventory.md (S48's rows [<rows>] hold the Viper shapes this part writes); [the audit report
whose fixes you apply first, if one is named below].

Task. Build Part <X>'s lowering and checks as the record's items [<items>] state them. [Points 1 to 3, the
check.cpp sentence and the proof of 12.2's task 3, unchanged.] Report: the functions you changed, each site
of the plan's table with what it does now, the guards you lifted and the ones you kept, the cases and their
times, and anything you left as a listed limit.
```

Rows of S48 per part: C2 (e), (f); D (h), (i), (k), (l); E (g), (m) and the `zero_arr` fixtures of "S48 after
audit round 1"; F (j).

**12.4 test-mutator** (first task: Step 0; then each part's tests):

```
[12.0]
Your role is test-mutator. Yours: tests/** (new tests, twins, pins and exactly the Flips of the plan's section
9), tests/mutants-equivalent.txt, compiler measurements, scripts/run_tests.py, scripts/lint_viper.py,
scripts/rejections.py with their self-tests, and the example files under doc/modules/ROOT/examples/. Never
yours: src/. You hold scripts/run_tests.py in the `work` lane (~/cerastes-e1e2/work). State file:
build/m4.5e1e2b/test-mutator-state.md.

Read: the plan's sections 1, 3, 5 (item 3), 8 and 9; record 0064 items 5, 7, 16, 17 and 19; tests/README.md.

Task 1, Step 0 on the frozen build (plan section 5, item 3): R1, R3 and R4. Report 1: each run's result
against what the plan's section 9 says, row by row, and anything that differs.

Task 2, Part R. Make Flips R exactly as section 9 lists them: 8 files re-expected and moved to
tests/memory/negative/, 9 retired, the new positive restrict_dropped.c with its error twin; in the 34 pins
that stay, only the `Pins:` comment and, in the 14 that section 9 names, the `twin:` line. No `expect:` row
of the 34 changes. If a row of the 8 lands anywhere but the head of its function, or a tag or line of the 34
differs from R4, report it before you edit that file. scripts/run_tests.py --check-headers must pass on the
result. Write Part R's new tests
(section 8), each expectation registered before its first run. If the first session's compiler matrix (under
build/m4.5e1e2b/prev/) lacks a cell for a scalar form of question 322, measure that cell (clang and GCC, both
modes, no flag and -Wall) and report the table.

Then, for each part, on the orchestrator's message: write the part's tests from the record (plan section 8),
each expectation registered before the first run; make the part's Flips; extend the lint as the plan's
section 7 names for the part, each new rule with a refusing twin in scripts/test_lint_viper.py (before you
edit a self-test that pins "this shape is refused" for a shape the record now admits, report which); after
the joined proof, with builds held, run the part's new tests once, timed. A new test that fails for its
designed reason at another line or a neighbouring category may be corrected once (question 160); list every
correction. A twin has one failing row per path: after a failed check Silicon goes on with that value
unconstrained, so give a twin an `ensures` that holds for any value. A test that holds arrays of structures
uses two-leaf structures and at most two such arrays in one function (the plan's section 3, rule 9).
```

**12.5 soundness-auditor, round `<part>`** (a fresh instance each round):

```
You audit one snapshot of cerastes, a C/C++ -> Viper verifier, for unsoundness, in the continuation of the
session M4.5e1e2. You are read-only: never edit, write or commit. Your report is returned as text; the
orchestrator saves it. Keep your probe programs in a directory outside the repository and name it in your
report, with the load-bearing probe texts inline: the orchestrator copies them.

The snapshot is the commit <hash>, checked out in .claude/worktrees/m4.5e1e2b-audit, with its build in the
box lane ~/cerastes-e1e2/audit (run probes there; every shell call that uses scripts/box_lane.sh starts with
`. build/m4.5e1e2b/box.env &&`; stop any probe that runs past ten minutes and report it as a timeout). The
diff under audit is `git diff <previous snapshot or base> <hash> -- src/ scripts/lint_viper.py tests/
internal/decisions/0064-*.md doc/`.

Read first: .claude/plans/m4-5e1e2b-arrays-of-structures.md sections 1, 6 and 7 (the audit list for round
<part> is there, and it is your checklist); the record 0064; record 0063; the chapter "## M4.5e1e2" of
internal/il-inventory.md; and the earlier audit reports: the first session's in
.claude/worktrees/m4.5e1e2b/build/m4.5e1e2b/prev/auditor-report-{1,2,2b}.md and this session's in
.claude/worktrees/m4.5e1e2b/build/m4.5e1e2b/auditor-report-*.md (do not repeat a finding that was fixed; do
check that each blocking fix holds).

What counts most, in this order: a program that is wrong and verifies; a trusted contract (alloc_arr_<E>,
zero_arr_<E>, program_start, a typed view) that can be unsatisfiable, gives a (receiver, field) pair twice, or
tells a caller something that is not true of C; a leaf read as a cell of another shape; a length fact that is
assumed on some path and never discharged; a pair of sites that can overlap and escapes the pairing rows, the
exclusion, the framing disjuncts or f__wf's facts; a construct accepted that the record rejects; a rule with
no must-fail test. A wrong program that verifies and is not of this diff is reported too, first, with the
build it also verifies on. Write probe programs for what you suspect, and say for each claim whether you
reproduced it.

Report: a numbered list with file:line and severity (blocking / should fix / note), then the list of claims
you reproduced and those you could not. For round <part> also: [the round's list, from section 7].
```

**12.6 The advisor** (each question):

```
Continuation of session M4.5e1e2 of cerastes. Question <number>, from <who>.
Fail-closed or unsound: <one sentence>.
Facts: <what was run or read, with paths>.
The plan's rule that applies: <section and row, or "none">.
Recommended answer: <...>. Alternative: <...>.
What it blocks until answered: <...>.
```

**12.7 A later task, by SendMessage** (to edg-backend and the test-mutator between parts):

```
Part <X>, your half. Read the plan's section 7 row <X> again and record 0064 items <items> (the record changed
since your last task: <what>). Gates to lift: section 4's list for Part <X>. Hold builds from <time> for the
joined run. Report to build/m4.5e1e2b/<role>-report-<n>.md: what you changed, your proof (<the build and
rejections.py --base against section 9 | the part's tests, once, timed>), and what you left.
```

## 13. Inputs (what the first session learned that Parts C to F stand on; moved here from `internal/handoff.md` §3 and digested from that session's reports, which Step 0 copies to `build/m4.5e1e2b/prev/`)

- **The record changed after the first plan was written:** items 4, 5, 6, 7, 11, 12, 16 and 19. The first
  plan's section 7 is not its text any more. The fixtures `s48_c_formP*.vpr` show the pairing rows in a
  superseded form; `s48_c2_*.vpr`, `s48_z_*.vpr` and `s48_v_*.vpr` show the current ones.
- **The pairing rows** (item 7; audit round 1, issues 3 and 4). A row is written per pair of leaf paths of one
  field, the element index recovered from the other site's cell by one `parent_of(block_of(·))` per member
  step of the leaf, innermost first; two ranges are paired by the slot row, never by the block row that 0060
  item 8 writes for scalar ranges (Silicon does not decide it where a function forwards two of its own ranges
  over structures); a site that is a nested slot may be paired by its slot. A single index inversion for every
  leaf lets an alias through a nested leaf pass the row and reach Silicon's own check (exit 3). Behind every
  pairing row stands the call's own precondition, so a row Silicon cannot decide is a false `permission` or
  an exit 3, never a proof. Three pairings have no fixture: a `reads` site against an `owns` in the nested
  pairings, a cell held by a fraction, a forwarded structure atom of the element's own type (the auditor's
  probes `fwd_row` and `fwd3_row`, in `prev/audit1/`, decided the last two).
- **Part B's lowering, for whoever extends it** (il-analyst-2's report 3). Every leaf access is
  `MemberAddr(ElemAddr(base, ix), k)`. The bound row is strict where a member step follows (`v[N].x` is
  `out-of-bounds`; `&v[N]` as a value is allowed). `p[i].x` on a pointer takes the null row and the liveness
  row once per access, then the bound, then the cell's; with a too-large index on a pointer that holds only
  `owns(*p)`, the bound row and the cell's write row both fail at one store (two errors; tests expect both).
  The whole-element sequencing row is written for each leaf of the element against the write, so `g(v[i],
  v[j].x++)` has two rows of which one can fail. Loop reach is by field (item 10; question 318): `writes_in`
  is unchanged, and a place is reached when its leaf's cell suffix is in the written fields or the array is
  stored to by name. `inv_interval_frame` skips leaf places, so an application over a pointer into an array
  of structures in an invariant is a false alarm (listed). `split_candidates`' branch for an array of
  structures adds case splits no program was found to need; its case pins them by the emitted text.
- **The encoder, for whoever extends it** (edg-backend's reports 4 to 6). `array_vtype`'s second parameter
  admits a structure element; only `declaration` passes it. `aggregate_array_initializer` reads one nested
  list per element through `collect_init_elems`, sets `Var::zero_filled` when a leaf is unwritten, evaluates
  every element before the first store in C and stores in order in C++. `copy_struct` fixes an element root
  into temporaries (`copy_base`, `copy_index`) unless it is constant. `place_address` gives a use of a
  rejected array its own row (so a pin's use line keeps its row). A computed array of structures (a typedef,
  `typeof`) goes through `array_type_reason`, which does not pass the flag: Part E's.
- **`restrict` and Parts C to F.** Once Part R has landed there is no `restrict` rule for a later part to keep
  in step: the first session's notes about `restrict_backed`'s walker, `holds_cell` and the `held` exemption
  are moot.
- **Part E.** Depth 3 is admitted (S48 (m): 10 s a program). The trusted methods' names identify the element
  type exactly (`alloc_arr_row_int_3`; two row types never share one, which would state one `len` for both).
  The copy `v[i] = v[j]` of elements with an array member took 35 s in the spike (its tests are timed under
  rule 3); its guard `d != s` is written between two elements of one array and for a place reached through a
  pointer, never between two named objects (S47's F2). The single report for `int a[][3]` comes with its
  pin's flip. In C the decay of a global array is folded into a `ck_address` constant in an initializer's
  rvalue positions and as a whole call argument (item 5; P83 (a)).
- **Should-fix items the first session did not take, and where they go here.** The lint does not tie a scope
  exit's rows to its allocation (round 2, issue 7): Part C1's lint work. The Sequencer's prefix match has no
  case with `t.v[0].x` inside a value stored to `t` (round 2, issue 8): Part C2. C++ rejects an element that
  reads its own array with the brace-elision message: Part C1, edg-backend. The encoder's text at the
  `restrict_backed` call site and the rules pinned only by hand-built VIR cases (the short read, issues 7 and
  9): gone with Part R.
- **Costs** (question 328; record item 6): section 3's rule 9. The nested pairing twins are the slow ones (48
  to 85 s in the spike): Part C1 times them under rule 3. S48 (n): an array of structures with four leaves
  and a loop over each took 59 s.
- **Process facts.** Every role needed three or four instances in the first session; Part B's lowering needed
  three. Two il-analyst instances may run at once only when one of them holds no `src/vir` file (probes,
  fixtures). An auditor's probes live outside the tree and are copied when its report arrives. A network cut
  ended all three agents once; each was resumed with nothing lost. The commit's tree is the joined proof's
  tree, composed from the snapshot ref.
- **From the first plan's Inputs, still binding:** 0028 items 2 and 16 (the layout; one quantifier per member
  and field, each with its own receiver as its trigger: one quantifier holding two members' permissions fails
  Silicon's injectivity check); S47's F1 (a quantified chunk is matched by its pattern, not by the receiver's
  spelling), F3 (after a failed `assert` Silicon assumes it: one failing row per path in a twin) and F5 (one
  liveness conjunct per root); the by-value limit of 7 (question 309); an allow-list is never computed by
  `grep` before a build; the audit worktree needs no bootstrap (a source-only `git worktree add --detach`
  plus the lane's build of exactly that source); proofs made while another agent builds are provisional; a
  twin that runs further than its positive needs an `ensures` that holds at the wider state; before a
  question about a lowering change, run the end-to-end vprdiff and put every changed VIR case through
  Silicon; `tail -n 3`, not `tail -3`, on the box; containers are started with `--init`; a `pkill` inside
  `docker exec bash -c` kills its own shell (`docker restart` stops a lane cleanly).

## 14. What the session leaves, and the questions

- **For handoff §3 at the session's end:** M4.5e2 takes ranges on part of an array of aggregates, atoms on one
  element, `malloc(n * sizeof(struct S))`, a second index or a row under a binder, `p[e].x` in a loop
  invariant under a structure atom, and the re-argument of the `offset_of == 0` backstops; M4.5e1f takes the
  first-slot amount of each leaf path with question 238; M4.5f3 takes structure values and copies past the
  by-value limit; M4.6 takes `restrict` on library declarations; M4.8 takes element structures with `owning`
  or marked members, arrays of owning pointers, item 10's reach rule, the cost of several arrays of
  aggregates in one function, `uninit` on an array of aggregates and `v[i] = f();`; M8 takes a model of
  `restrict`, pointers to arrays outside parameters, depth 4 or more, designators, and question 325 if P84
  (a) found the IL silent. A cut adds its row.
- **Answered by the owner at this planning (2026-11-25, the question dialog):** 326 "Reject restrict
  (Recommended)", 320 "Accept it in Part E (Recommended)", 322 (second half) "Reject them too (Recommended)",
  325 "Probe, reject if the IL can tell" (section 1 has each in full).
- **Defaults this plan builds on, for the owner's review** (each fail closed: a rejection with a diagnostic
  and a pin, or a listed false alarm, never a wrong proof; answer in `internal/handoff.md` §8 before the
  session, or leave them):
  - **331** Part C is two parts with a commit, an audit round and a split point each: C1 (ranges over
    structures in contracts), then C2 (static storage and members). Alternative: one part, as the first plan
    had it.
  - **332** The `restrict` rejection covers a declaration without a body as well as a definition. No test,
    doc example or vlibc header has such a declaration today; M4.6 decides what `memcpy`'s declaration needs.
    Alternative: accept and ignore `restrict` on a declaration without a body now (its contract is trusted
    anyway).
  - **333** A structure value stored into an element (`v[i] = f();`, `v[i] = (struct P){1, 2};`) stays
    rejected, with its message, until M4.8; the user writes `struct P t = f(); v[i] = t;`. Alternative: lift
    it in Part C2 with 0044 item 5's rules.
  - **334** The 33 pins that already reject `restrict` in particular situations stay, although each now pins
    the one rule; only their `Pins:` comments change. Alternative: retire all but one per spelling (the
    owner's, under invariant 1).
  - **335** Question 322's rule is the predicate the first session built for arrays of structures, so `&p[0]`
    in a truth test or null comparison is rejected with the other indexes although GCC is silent on that one
    form. Alternative: accept `&p[0]` on a pointer for scalars and structures alike.
  - **336** `scripts/box_lane.sh` refuses to run without its two lane variables instead of falling back to
    M4.5e1e1's lanes.
  - **337** The harness's twin rules shape Flips R: the 8 re-expected files move to `tests/memory/negative/`,
    a new positive `tests/memory/restrict_dropped.c` (the keyword dropped) with an error twin stands for the
    rule, and the 14 existing pins whose `twin:` line named one of the 8 name it instead. No `expect:` row
    of an existing pin changes. Alternative: keep the 8 as positives by deleting the keyword from them,
    which changes no outcome but leaves eight tests that no longer test what their names say.
- **Open questions: none.** A question that arises in the session goes to the advisor first (section 11); the
  session's questions start at 338.
