# Plan: M4.5e1e2 — Arrays of structures and arrays of arrays (record 0064)

**Superseded for everything not yet built (2026-11-25, after its first session).** That session built Parts A
and B from this plan, ran P83 and S48 in full, and closed at split point 1. **Parts C to F, and the rejection
of `restrict` the owner decided afterwards (question 326), run from
`.claude/plans/m4-5e1e2b-arrays-of-structures.md`**, which the owner asked for in place of "no second
planning": record 0064 was revised in eleven items after this plan was written (its section 7 below is no
longer the record's text), every line number of sections 4 and 8 has moved, and questions 320, 322, 325 and
326 are answered there. This file stays as the history of Parts A and B, of the probes and of the spike; its
sections 5, 6 and 8 to 13 are not to be executed again.

**Written on 2026-11-25 at a session that only planned, after M4.5e1e1.** It supersedes section 14 of
`.claude/plans/m4-5e1e1-arrays-of-scalars.md` ("a short planning of its own"): **M4.5e1e2 does not plan again.**
Its probes and spikes run first (section 6), each with a decision table that says what is built for each outcome.
**It is one plan for what will likely be two sessions** (question 310, section 8's split points): a session that
closes at a split point hands the parts it did not begin to a continuation, which runs from this same plan.

**Written for this ensemble** (owner, 2026-11-25): a Fable orchestrator at `high`, the main session, started with
`claude --agent orchestrator --effort high` (`.claude/agents/orchestrator.md`); three Sonnet implementers from
`.claude/agents/` (`edg-backend`, `il-analyst`, `test-mutator`); the Fable `soundness-auditor` at `xhigh`; the
Fable advisor. Section 2 assigns every task and section 13 holds each agent's briefing, ready to send.

**What the orchestrator reads before acting:** this plan, all of it; the M4.5e1e1 entry of `internal/status.md`;
`internal/handoff.md` §1, §2 and the rules of §4, §6 and §8; and these records: **0063, all of it** (this session
extends each of its items); 0028 items 2, 11 and 16; 0043 items 3–9, 11 and 13; 0044 items 2–5 and 8; 0059 items
2–8; 0060 items 3–11 and 16; 0061 items 1, 3, 6, 9–11; 0062 items 1, 4 and 7; and, before Part D, 0047 items 2, 4
and 6–8, 0052 item 2, 0055 items 3 and 5, 0058 item 4, 0045 items 2–7, 10 and 12. **Execution starts at section
5** (Step 0).

Branch `m4.5e1e2-arrays-of-structures`, worktree `.claude/worktrees/m4.5e1e2`, `build/m4.5e1e2/`, record
`internal/decisions/0064-arrays-of-structures-and-arrays-of-arrays.md` (the next free number; if M4.5e1f has run
first, the one after its record, and "0064" below reads as that number), probes **P83**, spikes **S48**, the
chapter `## M4.5e1e2` of `internal/il-inventory.md`, questions from **320** (310–312 were answered at this planning;
313–319 are its defaults, section 15).

## 1. Context

**Why.** M4.5e1e1 (record 0063) gave arrays of scalars every storage: globals, `static` locals, structure
members, compound literals, structure values. An array whose element is a structure or an array is still
rejected with a diagnostic that names M4.5e1e2 (section 4 has the sites and the pins), and six rules for a
structure with an array member were left to this session (questions 292–294 and 306, partnerless atoms, written
atoms). Record 0028 items 2 and 16 fixed the layout in M4.5a (element `i` of an array is the slot `loc(B, i)`;
a structure element's member `k` is the block `member(loc(B, i), k)`; row `i` of `int m[2][3]` is
`member(loc(B, i), 0)`), and M4.5e1e1's spike S47 (x) verified its two unknown shapes on the box: one quantifier
per leaf over an array of structures (9.2–9.9 s) and a permission over two bound variables for a matrix (8.7–8.9
s).

**Owner decisions.**
- **310** (this planning, 2026-11-25): one complete plan; **the 12-hour rule** with the order arrays of
  structures (code, then contracts and storage) → the leftovers → arrays of arrays → quantifiers. At each commit
  the next part starts unless the remaining work would pass twelve hours; **parts not begun run from this plan
  in a second session, with no second planning. Nothing is cut to hold a session short.** The planning sized all
  of it at 4,700–8,450 `src/` lines (M4.5e1e1 landed 4,944 in under eight hours), so two sessions are likely.
- **311** (2026-11-25): the full check ran green on M4.5e1e1 after its session; the base is green.
- **304 and 312** (2026-11-25; 304 answered inline in the handoff: "Follow the rejection like the compiler
  warning."): C++'s `{0}` over a structure whose first member is a nested structure, in an aggregate of two or
  more members, is rejected as the array-member form is (question 303), and it is built here (Part B; Flips G).
- **290–300** (M4.5e1e's planning) and **301–309** (settled with the advisor in M4.5e1e1; record 0063's header):
  as built. 292–294 and 306 are lifted here (Part D); 298 puts the tutorial section "arrays in aggregates" here.
- **Standing:** 69 (no EDG work), 77 (one context, under about 12 h), 160 (a new test's expect line may be
  corrected after its first run, each listed in status), 176 (the owner handles CI), 277 (a twin with several
  real errors lists every one), 285 and 299 (a test slow on the box only because of what it must refute goes to
  `tests/long/` with a short default-tier twin); `check.sh` once, at the end, on the x86-64 box; every test run
  on the box; no test runs while a fix is queued; no Claude trailer; commits stay local.

**Terms.** A **scalar** is `bool`, an integer type, an enumeration or a pointer to an object type. A **verified
structure** is 0043 item 2's, with the array members 0063 gave it. An **aggregate element** is an array element
that is a verified structure or an array. A **leaf path** runs from an object's first slot to one scalar cell by
**index steps** (`[i]`, with a count) and **member steps** (`.k`); between two index steps stands the member
step 0 that 0028 item 16 gives a row. Its **depth** is its number of index steps: 0063 accepted depth 1 with the
index step last (`s.a[i]`, `g[i]`); this record accepts index steps anywhere, to depth 2.

**Scope.**
- **In:**
  - **(S) Arrays of structures.** In code (Part B): an automatic local `struct S v[N]`; `v[i].x`, `v[i].t.x`,
    `p[i].x` on any `struct S *`, `&v[i]`, `&v[i].x`, the decay of `v`; an element as a structure value
    (`t = v[i]`, `v[i] = t`, `v[i] = v[j]`, `f(v[i])`, `struct S t = v[i]`); braced initializers with a nested
    list per element and C's zero fill; scope exits; loops; sequencing; `sizeof(v)` (an element structure that
    itself has an array member is depth 2 and waits for Part E). In contracts and storage
    (Part C): `owns_range(p, n)` and `reads_range(p, n)` with `p` a pointer parameter to a verified structure;
    element reads `p[e].x` in `requires`, `ensures`, `decreases` and under `old`; calls, returns, `f__wf`; loops
    over a parameter's range; `owning(n)` on such a parameter; P4222's markers over such a range; global and
    `static` local arrays of structures with their initial values, `owns(g)` and `reads(g)`; an array member
    whose elements are structures (`t.v[i].x`, `owns(*p)`, `owns(p->v)`), in every storage of its structure.
  - **(H) The leftovers of a structure with an array member** (Part D): heap storage (`malloc(sizeof(struct
    S))`, `calloc`, `free`, `owns_block`; question 292); an `owning` or marked member beside an array member
    (293); `ref_to_uninit` and `must_init` on a pointer to such a structure (294); contract places through a
    heap read (`owns(*p->q)`, `p->q->a[i]`; 306); an `ensures` atom with no `requires` partner; a written atom
    in a loop invariant on such a place.
  - **(A) Arrays of arrays** (Part E): `T m[N][M]` as a local, a global, a `static` local and a member;
    `m[i][j]`, `&m[i][j]`, a row `m[i]` used as a pointer; `sizeof(m)`, `sizeof(m[0])`, `sizeof(int[2][3])`,
    typedefs of such types; nested braced initializers; **array-of-rows parameters** (`int a[][3]`, `int
    a[2][3]`, and the same type spelled `int (*a)[3]`) with `owns_range(a, n)` over rows, `a[i][j]`, and `m`
    passed to one; nested loops; and the other depth-2 path, an array of structures whose member is an array
    (`v[i].a[j]`, `p[i].a[j]`).
  - **(Q) Quantifiers and `pure` applications** (Part F): under a binder, a member of a structure element
    (`p[i].x`, `v[i].t.x`); as the pointer argument of `cerastes_forall` and of a `pure` application, the decay
    of an array member (`p->a`, `s.a`) and a named global array in a contract.
  - **(I) Question 304's rejection** (Part B).
  - **The pages, and the tutorial section "Arrays in aggregates"** (question 298).
- **Not here** (rejected, each pinned, the message naming where it is lifted):

| Construct | Lifted by |
|---|---|
| an array or a range whose element structure has an `owning` or marked member; an array of owning pointers; `owning`, `ref_to_uninit` or `uninit` on an array of aggregates; `restrict` backed by a range over structures | M4.8 |
| `malloc(n * sizeof(struct S))`, `calloc(n, sizeof(struct S))`, `realloc`; a range on part of an array (`owns_range(&v[1], n)`, `owns_range(g, n)`, `owns_range(p->v, n)`, a row `m[i]` as a range's base in a contract); an atom on one element (`owns(p[i])`, `owns(v[i].x)`); `&p[i]`, `&p[i].x` and `&m[i][j]` in a contract; a range whose base is not a pointer parameter; a range atom in a loop invariant; `p + i` | M4.5e2 |
| under a binder: a second index (`p[i].a[j]`, `m[i][j]`), a row `m[r]` or `&v[1]` as the pointer argument; a memory-reading argument (`p->q->a`) | M4.5e2 |
| a `forall` inside a `forall`; the positions 0061 item 1 leaves to M4.8 | M4.8 |
| a leaf path of depth 3 or more (`int c[2][3][4]`, an array of structures whose member is a matrix), unless S48 (m) admits depth 3 | M8 |
| a pointer to an array anywhere but a parameter (`int (*p)[3]` as a local, a member, a result; `&m[i]`, `&a`; a cast to one); `sizeof(char(*)[4])` | M8 |
| a structure value that holds more than 7 array-element leaves, passed or returned by value (0063 item 13; a copy has no limit); an array as a value | M4.5f3 |
| a read `pure` function over a structure or a global (0062 item 1) | M4.5f1 |
| `const` arrays in any storage; string-initialized arrays | M4.6 |
| array designators, VLAs, zero-length and flexible members, `[static N]`, `thread_local` and block-scope `extern` arrays, C++ array literals, `_Alignof` of an expression | M8 |

**Size** (`src/` and `include/` insertions without the VIR self-test cases; calibrated on what M4.5e1e1's
commits landed: plumbing with global arrays and computed types 2,280, array members 2,116, structure values 665;
its plan's estimate for all of M4.5e1e2 without the leftovers was 1,700–3,100):

| Part | Content | `encode.cpp` | `lower.cpp`, `check.cpp`, `vir.*` | Sum |
|---|---|---|---|---|
| A | leaf paths with index steps; the six silent traps closed; behaviour-neutral | — | 350–600 | 350–600 |
| B | (S) arrays of structures in code; (I) | 450–800 | 600–1,000 | 1,050–1,800 |
| C | (S) ranges over structures, globals, members | 350–650 | 900–1,500 | 1,250–2,150 |
| D | (H) the leftovers | 200–400 | 550–1,000 | 750–1,400 |
| E | (A) arrays of arrays, depth 2 | 300–550 | 450–850 | 750–1,400 |
| F | (Q) quantifiers and applications | 200–400 | 350–700 | 550–1,100 |

**4,700–8,450 in all**; beside it the lint, 600–1,100 lines with as much again in its self-test. Parts A–D
(3,400–5,950) are an M4.5e1e1-sized session; by question 310's rule Parts E and F run in the same session only
if they fit under twelve hours.

## 2. The ensemble

| Role (model) | Owns | Never touches |
|---|---|---|
| **Orchestrator** (Fable, `high`; the main session) | Steps 0–1; record 0064, written before code from section 7; the vprdiff verdict per part; the user pages in `doc/` and the tutorial section; status, handoff, `plan.md`'s Done row; every commit; briefing (section 13), resuming and replacing the agents; `build/m4.5e1e2/orchestrator-state.md` (kept current, times from `date -u`) | `src/`, `tests/`, `scripts/` (their owners make every change there, the small ones too) |
| **edg-backend** (Sonnet) | `src/backend/**`, `src/driver/**`, `include/cerastes.h`; **in Part A only** `src/vir/**` and `src/viper/**`; the build system, the box lanes and the baseline; **the joined build of each part** with `scripts/regen_vpr.py` and `scripts/vprdiff.py` on it; `internal/encoding.md`, the developer guide's paragraph | `tests/`, `scripts/lint_viper.py`, `src/vir` and `src/viper` after Part A |
| **il-analyst-1** (Sonnet) | P83 and S48; the chapter `## M4.5e1e2` of `internal/il-inventory.md`; the fixtures `tests/viper-smoke/memory/s48_*.vpr`. It ends when the chapter is committed | `src/` |
| **il-analyst-2, -3, -4** (Sonnet; a fresh one at Part B, at Part D and at Part E) | `src/vir/**` (`vir.h`, `vir.cpp`, `check.cpp`, `lower.cpp`, `lowered.*`), `src/viper/**`, the next free self-test case file. -2: Parts B and C; -3: Part D; -4: Parts E and F | `src/backend`, `tests/` |
| **test-mutator** (Sonnet) | `tests/**` (tests, twins, pins, exactly the Flips), `tests/mutants-equivalent.txt`, the compiler matrix, `scripts/run_tests.py`, `scripts/lint_viper.py`, `scripts/rejections.py` and their self-tests, the example files under `doc/modules/ROOT/examples/` | `src/` |
| **soundness-auditor** (Fable, `xhigh`, read-only) | one round per commit that touches `src/` and one on the record (section 8), each on a snapshot commit in `.claude/worktrees/m4.5e1e2-audit` and the box's `audit` lane, each before its part's commit; **a fresh instance per round**, briefed with the earlier reports | everything (never edits) |
| **Advisor** (Fable) | every question first (section 12), and the checkpoints below | — |

- **At most three Sonnet agents alive at once:** edg-backend, the current il-analyst, the test-mutator. **Every
  role is a chain of instances** (M4.5e1e1 needed four of each): each agent keeps
  `build/m4.5e1e2/<role>-state.md` current (what is done, what is in progress, the files it holds, the next
  step), and when the orchestrator judges its context low, or at the hand-overs this table names, a fresh
  instance starts from that file, this plan, record 0064 and the last commit. This is the plan, not a deviation.
  An il-analyst is replaced, never resumed, at a hand-over (`lower.cpp` alone fills a context).
- **The advisor** is the `advisor` tool; if the orchestrator has none, an Agent call with `model: fable`, briefed
  with the question and the paths to read. Step 0 checks that one of the two answers.
- **Advisor checkpoints:** (1) the record draft with P83's and S48's outcomes mapped to section 6's tables,
  before audit round 1; (2) every question, and any outcome outside a table (section 12); (3) each split point
  (section 8); (4) any cut; (5) before the box run.
- **Every test run is on the x86-64 box** (owner, 2026-11-24), **in a lane of its own per agent that builds:**
  `~/cerastes-e1e2/{base,il,edg,work,audit}` on `matt@clang4.cpp.al`, set up with `scripts/box_lane.sh`
  (`internal/handoff.md` §6 has the recipe it wraps; image `cerastes-e1d:26.04`; containers
  `cerastes-e1e2-<lane>`, started with `--init`). `base` holds the baseline at the session's base commit and is
  not rebuilt; `il` is the current il-analyst's (its VIR cases through Silicon); `edg` is edg-backend's (its
  half's build and `rejections.py`); **`work` holds the joined builds, the regenerations and the test-mutator's
  runs**; `audit` the auditor's snapshots. Work is shipped as a commit object in a git bundle (`box_lane.sh
  ship`). The Mac builds to compile and runs nothing but a single-file probe whose box round trip would cost
  more than the run (say so when choosing it). The final `check.sh` runs in `~/cerastes-s8/p-ubuntu-26.04`.
- **Builds:** every agent that edits `src/` builds its own change on the Mac through the lock (`lockf
  build/m4.5e1e2/build.lock cmake --build build/cmake`) and in its own lane. The `work` lane is built by
  edg-backend only, for a joined proof; nothing builds there during a run.
- **The runner** (`scripts/run_tests.py`; it locks `build/vpr`) has one holder at a time in the `work` lane: the
  test-mutator, except while il-analyst-1 runs S48's fixtures (until S48's second pass is in the chapter).
  Everyone else uses
  `build/cerastes verify <file> --keep-vpr`.
- **`lower.cpp`, `vir.h`, `vir.cpp`, `check.cpp`, `lowered.*`, `emit.cpp` have one holder at a time:**
  edg-backend in Part A, then the il-analyst of the part, with a commit or an audit snapshot between holders.
  **The il-analyst that starts from a round's snapshot applies that round's fixes to those files.** An agent
  that needs a change in another's file asks the orchestrator, who passes it on.
- **Every agent** (in each briefing of section 13):
  - reads `CLAUDE.md`, this plan, record 0064 and the files it is assigned; edits only its own files;
  - **any question goes to the orchestrator, who takes it to the advisor first** (section 12); the agent does
    not wait idle: it goes on with whatever the question does not block;
  - writes each report to `build/m4.5e1e2/<role>-report-<n>.md` as well as returning it, and keeps its state
    file current (the auditor cannot write: the orchestrator cuts each snapshot commit and the audit worktree,
    ships the snapshot to the `audit` lane, and saves each audit report to
    `build/m4.5e1e2/auditor-report-<n>.md` itself);
  - follows the build, lane and runner rules above and section 3's budget: **it runs no test that section 3
    does not list**, and reports the wall time of every new test's first run;
  - a test generator lives in `build/m4.5e1e2/` and is never re-run over the tree;
  - tool code in the owner's style (`scripts/format.sh`; brace initialization, `U`, one declarator per
    declaration, no comment on a brace line); no Claude trailer on anything.
- **A reader lands with what it reads** (P77): nothing is registered and no name is added to `<cerastes.h>` in
  this session; the encoder's acceptance of a construct lands in the part whose lowering gives it rows.
- **Fail closed between parts:** what a later part lifts is rejected with a diagnostic and a staging pin (section
  5's R2), written and retired inside the session; in `src/vir` the gate is `check.cpp`'s.
- **Session tools are committed** (question 193): anything Step 0 or a spike scripts goes to `scripts/` with a
  self-test, or its summary to `internal/il-inventory.md`.

## 3. The test budget

**The owner's rule** (2026-11-25: "I believe a wasteful amount of testing is occuring"): every test run this
session makes is in the table below with its reason, and a run that is not there is not made. M4.5e1e1 ran under
the same rules: its last part was proved 1 h 30 before the closing commit, against 4 h 25 in M4.5e1d.

**The rules.**
1. **Byte identity replaces re-running.** After each part edg-backend regenerates every kept `.vpr` and `.map`
   (`scripts/regen_vpr.py`, about 25 s for 6,700 programs on the box) and compares with the baseline
   (`scripts/vprdiff.py`). **No existing program's Viper or map changes in this session** (section 7 says why;
   question 304's new rejection reaches no existing program): while that holds, no existing test is run
   before the box check. A test's last pass stands while its `.vpr` and `.map` are identical to that run's,
   `src/driver/**` and `scripts/run_tests.py` are unchanged since, and the host is the same. **Byte identity
   says nothing about a lint that changed:** a joined proof after a change to `scripts/lint_viper.py` also runs
   the lint over the regeneration and over the VIR self-test's output (about a minute each) and
   `test_lint_viper.py`.
2. **A new test runs once,** when its part is joined, and again only if `vprdiff` against the previous part's
   regeneration shows its Viper or map changed, or its expectation was corrected. A part's run never includes
   the earlier parts' tests.
3. **Every new test is timed at its first run, on the box.** Over 60 s: reported in the part's report. Over
   120 s: a smaller program, or `tests/long/` with a short default-tier twin (question 285's rule), settled with
   the advisor at that part, not at the end.
4. **Counts are capped:** at most 56 S48 fixtures (section 6 gives each row its count) and the per-part test
   counts of section 9 (190 files in all). One positive and one error twin per rule of the record, one pin per
   rejected row of section 1's table. An audit's requested pins and twins come on top (M4.5e1e1 ended 38 over
   its 150 that way); a part that needs more than its cap for another reason goes to the advisor.
5. **The VIR self-test:** `cerastes-vir-selftest` writes every case without Silicon (seconds) and the lint reads
   them; **only the session's new or changed cases go through Silicon**, by name.
6. **The lint:** each new rule has a refusing twin in `scripts/test_lint_viper.py`. No separate mutant sweeps of
   the lint or the harness.
7. **Front-end tests, goldens and the walker oracle** run only if `walker.cpp`, `register_attributes.cpp`,
   `annotation_checks.cpp`, `il_query.cpp` or `include/cerastes.h` changed, and then only `tests/annotations`
   and `tests/il`. This plan changes none of them.
8. **One validation pass after the last audit's fixes** (the preflight below), then the box check. A should-fix
   that touches no test becomes a listed limit, not a diagnosis task. An auditor's probe that runs past ten
   minutes is stopped and reported as a timeout.
9. **Never cut:** invariant 2's pairs, the audit of each `src` commit before it lands, the neutrality gate, the
   one full `check.sh`.

| Run | Who, where | When | Why |
|---|---|---|---|
| baseline: `regen_vpr.py` and the VIR self-test output at the base commit | edg-backend, lane `base` | Step 0 | the reference for rule 1 |
| R1–R3 and M (section 5) | test-mutator, lane `work`, frozen binaries | Step 0 | today's outcomes; the staging pins; `rejections.py`'s base |
| S48's fixtures, one file per run | il-analyst-1, lane `work` | section 6 | the decision tables |
| the half's own proof: new VIR cases through Silicon; the build | each implementer, its lane | each part | a half is proved before it is joined |
| joined proof: regen and `vprdiff` against the baseline and the previous part; the VIR self-test's output against the baseline's (existing cases identical); `rejections.py --base`; **where the lint changed since the last joined proof:** `lint_viper.py` over the regeneration and over the VIR self-test's output, and `test_lint_viper.py` | edg-backend (the lint runs: test-mutator), lane `work` | each part, once | rule 1; the Flips |
| the part's new tests, once, timed | test-mutator, lane `work` | each part, after the joined proof, builds held | invariant 2; rule 3 |
| the auditor's probes | soundness-auditor, lane `audit` | each round | its judgement, bounded by rule 8 |
| **preflight**, once: `format.sh --check`; `py_compile`; `run_tests.py --check-headers`; `mutate.py --validate`; `test_run_tests.py`; `test_lint_viper.py`; regen and `lint_viper.py` over it; the VIR self-test directory, its lint, `VIR_CASES_MIN`; `rejections.py --base`; `build_docs.sh`; `check_doc_examples.py` (source and `--html`) | test-mutator and edg-backend, lane `work` (the docs on the Mac) | after the last audit's fixes | `check.sh`'s own steps for what the session touched, so the 50-minute run cannot fail late on them |
| `CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` | orchestrator, `~/cerastes-s8/p-ubuntu-26.04` | once, after the last code change | the full check (invariant 8 of `plan.md`) |

**Not run:** a full `run_tests.py` outside `check.sh`; any existing test whose Viper is unchanged; `mutate.py`'s
mutation run (M4.8's); the self-tests of tools the session did not touch, which `check.sh` runs anyway.

**The target:** the box check starts within two hours of the last part's joined proof (the last audit round,
its fixes in one batch, the preflight), with the pages, the status entry and the handoff written while agents
run.

## 4. Survey facts (tree at `45d26bc`; read, not run, by three read-only surveys; names with lines where they help)

`E` is `src/backend/encode.cpp` (26,155 lines), `L` is `src/vir/lower.cpp` (24,893), `C` is `src/vir/check.cpp`
(6,110), `V` is `src/vir/vir.h` (1,463) and `vir.cpp` (2,133), `P` is `scripts/lint_viper.py` (6,719; its
self-test 10,466 lines, 897 tests).

- **Nothing needs the EDG fork, and nothing is registered.** `include/cerastes.h` does not change.
- **The encoder's tagged gates:** 38 rejection sites in `E` name M4.5e1e2 (none elsewhere in `src/backend`).
  - **(S), 6:** `array_vtype` 582 (an array of structures; its tag is the caller's: a declaration 18694/18706, a
    global 8026, a `static` local near 9189, a literal 15124, a member 25358, a typedef 881/913, `sizeof` 7264,
    `static_assert` 7335); `contract_element_address` 1220 and `subscript_address` 1478 (an element of a pointer
    to a structure); `owning_count` 4552; `spec_range_atom` 11171; `place_address` 15629 (a structure place that
    is a subscript: `v[i].x`, `p[i].x`, `&v[i].x`, `v[i] = s`).
  - **(A), 3, and one shared:** `array_vtype` 577; `check_signature` 3009 (`int a[][3]`); `describe_operation` 184
    (a decay of something unnamed: `m[i]`); `collect_array_elems` 1852 (a nested braced list).
  - **(Q), 12:** `reject_named_array_in_contract` 986 and 1002 (called from `application` 6557 and `forall_site`
    12572); `contract_element_address` 1161 and 1174; `subscript_address` 1386 and 1436; `array_decay` 1583;
    `member_array_decay` 1627; `elem_value_read` 1679; `forall_base_ok` 11828; `read_forall_function` 12180;
    `application` 6574, which is **dead** (6476–6480 returns first for every call in a quantifier body).
  - **(H):** heap, 5: `allocation_view` 17909, `heap_call` 18044, `spec_block_atom` 18127, `invariant_block_atom`
    18221, `struct_info` 25263 (an `owning` member pointing at such a structure); shared by questions 292 and
    294, 3: `read_marker` 3419, `apply_function_markers` 4195, `read_variable_markers` 14182; question 293:
    `struct_info` 25484; 294: `struct_info` 25312; 306: `reject_array_behind_load` 10302 (from 10323, 10378 and
    the two `owns_block` atoms); partnerless: `finish_atoms` 13077; written atoms: `invariant_atom` 11304.
  - Three more name it inside an M4.5e2 message (`spec_range_atom` 11151, 11164; `struct_info` 25231).
- **The encoder's untagged gates** (no "e1e2" in their text; each still blocks a construct once the tagged gate
  is lifted):
  - `vtype`'s pointer branch 2494–2505 refuses every pointer to an array (`int a[][3]` is reported twice at one
    line today, by 3005–3011 and through 3013–3022); `address_of` 15849–15856 and 15891–15897 and
    `declaration_level_checks` 25568–25570 name M8 for pointers to arrays (they stay; only a parameter's type is
    lifted); `read_forall_function` 12142–12146 for such a parameter of a quantifier's function.
  - `m[i][j]`: `subscript_address` 1452–1459 in code and `contract_element_address` 1184–1190 in a contract
    (their text names only string literals); `sizeof m[0]`: `reject_computed_sizeof_operand` 7270–7280.
  - `v[i]` and `p[i]` as a structure value or a copy's source: `struct_value_node` 14821 and `copy_struct` 19238.
  - `p[i].x` in a contract: `contract_place_address_raw`'s `no_place()` 10502 (message 10394–10396) and
    `contract_field_chain` 15708–15710.
  - (Q) in a contract, behind `reject_named_array_in_contract`: the decay rejections `array_decay` 1557–1558 and
    `member_array_decay` 1612–1613, `forall_base_ok`'s contract branch 11812/11827, `application`'s
    bare-parameter test 6640–6653, and perhaps `forall_site`'s reads-memory test 12631–12635.
  - Heap: `heap_view` 17724–17789 has an `internal_error` guard (17726–17730) and the last `vir::struct_leaves`
    call of `E` (17756).
  - `mk().a`'s decay is `place_address` 15640 ("a struct place this version cannot read").
- **The encoder's machinery to extend** (`E`): `array_vtype` 539–597, `global_array_vtype` 830–850,
  `check_array_type_bound` 702–755 (a bound checked as written once), `array_type_reason` 791–800 and the
  typedef helpers 803–918, `collect_array_elems` 1787–1863, `array_element_value` 1865–1888, `element_store`
  1891–1895, `array_initializer` 1900–2041 (all scalar elements only), `collect_init_elems` 18994–19206 (its
  callers `read_struct_global_init` 8774, `aggregate_value` 15448, `struct_initializer` 19550),
  `read_array_element` 8972–9061 and `read_array_global_init` 9066–9130, `subscript_address` 1326–1546,
  `array_decay` 1550–1601, `member_array_decay` 1606–1638, `place_address` 15522–15643, `address_of`
  15825–15961, `contract_element_address` 1128–1231 with `contract_element_finish` 1235–1294, `spec_atom`
  10818–10993, `spec_range_atom` 11086–11238, `check_signature` 2890–3035 with
  `check_array_parameter_declarations` 2765–2884 and `check_array_parameter_bound` 610–629, `copy_struct`
  19211–19317 (one `ArrayCopy` per array member, 19288–19316), `object_value` 14829–14880, `aggregate_value`
  15416–15518, `elem_value_read` 1644–1744, the predicates 8606–8683 (`vir_struct_has_array`,
  `vir_type_has_array`, `vir_value_array_elements`, `vir_value_over_limit`, `vir_struct_has_marked`,
  `edg_type_has_array`, `pointer_to_array_struct`, which strips one pointer level only), `owning_leaves`
  4753–4765 (the object view; it skips array leaves), `forall_site` 12445–12735 and `application` 6467–6676
  (a named array outside a contract is accepted at 12576–12615 and 6561–6611).
- **Brace elision is all in `collect_init_elems`** (three callers, so locals, globals and values alike). A
  nested structure: `elision = !explicit_braces_on_aggregate && !by_designator` (19058), rejected unless the
  elided aggregate is the list's only element (19059) — **that exception has no language or member-count
  condition, which is question 304**. An array member: `read_list = !elided && sole && zero && (c_mode ||
  st.members.size() < 2U)` (19117; question 303). **Likely defect, read not run:** C's `{0}` over a structure
  whose first member is a nested structure that begins with an array makes `read_list` false under the elided
  recursion (19109/19117) and is rejected at 19129–19134 with a message that says C accepts `{0}` (P83 (h)
  measures it).
- **Already accepted** (read, not run): a `struct S a[]` parameter at its signature (only its element uses are
  rejected); `not_invalidating` on a pointer to a structure with an array member (`read_marker` 3413); `sizeof`
  of `s.a` and `p->a`.
- **VIR can already hold the new types; only the checker refuses them.** `Program::array_of` (`V` cpp 361–372)
  takes any element; `Builder::elem_addr` (cpp 935–954) types `m[i]` as `Ptr(int[3])` and `m[i][j]` as
  `Ptr(int)`; `low::forall_` (`lowered.h` 145) and the emitter (`src/viper/emit.cpp` 127–151) already print
  several binders with multi-term trigger sets (`fq_check_rows`, `L` 8823, uses them). The refusals: `C`
  `array_local_type` 524–532 with `array_reason` 533–545 (untagged; reached from `struct_type` 402–493, `global`
  561–695, `global_array` 700–744, `declare` 2669–2728 and `AddrOf` 3567–3570); `accepted_pointer` 340–355 with
  `pointer_reason` 507–520 for `Ptr(Array)` (a parameter 948–953; a value 3326–3331; `array_addr_ok_`, 5289, is
  never set for an `ElemAddr` typed `Ptr(Array)`); the `ElemAddr` rule 4008–4120 (a pointer base to a structure
  4088–4097; in a contract 4031–4073); `range_atom_operands` 1545–1599 (1583–1588); the contract root rules of
  `Deref` 3652–3676 (3663) and `MemberAddr` 3886–3897 (3893), which use `place_root`; under a binder `Deref`
  3611–3619 and the body whitelist 3287–3312 (no `MemberAddr`, `GlobalAddr`); `forall_shape` 2345–2435,
  `forall_node` 2573–2639, `forall_loop_base` 2305–2316, `array_of_named` 2287–2302; `apply` 4927–5041.
- **The silent traps: each is wrong today for an aggregate element and is masked only by a checker or encoder
  refusal** (a verdict per row in audit round 2; Part A makes each one loud before any gate is lifted):
  1. `L` `elem_addr` (10606–10677), `inv_elem` (4473–4551, at 4530) and `lb_chain` (16578) treat an `ElemAddr`
     base as a flat pointer: `m[i][j]` would be `loc(B, i + j)` with its bound against the row count.
  2. `L` `same_elem_base` (5437–5462), `elem_base_key` (5428–5435) and `wf_method` (24614) read `.index` of a
     base that is neither a `GlobalAddr` nor a `MemberAddr`, which defaults to 0: `m[0][j]`, `m[1][j]` and
     `p0[j]` would share one element key and one ghost.
  3. A range over structures: `L` `site_fields` (6809–6838) returns the empty set, so `writes_in_expr`
     (16318–16410) records no write by such a callee, a loop's reach is false and it states that values are
     kept; `memory_fields` (9423–9474) makes the key `"struct@id"` that no store carries; `fkey` of an `Array`
     dereferences a null `int_type` (9289).
  4. `L` `leaf_count` (103–119), `leaf_span` (121–129) and `has_pointer_leaf` (131–149; so `struct_ptr_uses`
     12379 and `expr_touches` 15860), `C` `value_array_elems` (4350–4365), and `V` `collect_leaves` (cpp 52–96)
     and `may_hold_pointer_to` (cpp 1850–1890, the `restrict` test) count or type an array by its element and
     never look inside a structure element.
  5. `C`'s nest walkers `struct_has_array` (4336–4347), `struct_has_marked` (4367–4379), `contains_by_value`
     (359–377) and `owning_reaches` (380–400) skip array elements: a marked member, an owning cycle or a
     by-value cycle inside an array of structures would pass.
  6. `L` `cell_suffix` (2202–2256) returns `struct_suffix` for a structure without an error, so the loop's
     `ArrayPlace` reach tests (18135, 18167, 18192, 18206), `site_fields`, `overlap_pairs` (6856–6884),
     `arr_disjoint` (6889–6921) and the `inv_*` framing matches (4576–4785) would compare a structure element
     as one field; `use_cell_type` (2260–2271) is what stops most (S) sites today.
- **Gaps in `check.cpp`'s backstop** (only the encoder refuses these today): `invariant(owns(*q))` on a
  structure with an array member (3776–3779 covers `owns(q->a)` only; lowering exits 3 through `leaves_of`); a
  partnerless `ensures` atom on such a structure (lowering exits 3 at 7762, 11181, 15191); **`owns(*p->q)`
  through a heap read with such a pointee, which neither the checker nor lowering refuses** (no array-part site
  looks at `via`; only `E` 10302 stops it). And **no self-test pins a lowering guard:** there is no raw-lowering
  (`ru`) case at this tree (`643befb` removed the last six).
- **The poison guards in lowering today:** `leaves_of` (`L` 2029–2045; through `leaf_cells` 2425 and
  `block_cells` 6016–6028 it covers every heap view, `written_atoms` 17424, `memory_fields` 9463 and
  `writes_in_expr` 16391); `cell_suffix` on an `Array`; `use_cell_type` on a `Struct`; the array-part guards in
  `atom` (7760–7766), `call_atoms` (11179–11185), `atom_returns` (15189–15195) and `array_leaf_start_rows`
  (1533–1537); `array_copy_stmt` 5145, `array_zero_stmt` 5245, `elem_value` 10468–10482, `elem_addr` 10619,
  `contract_read` 8971; `apply_inv` 23332, `apply_check` 23483, `fq_collect` 8304, `fq_loop_read` 8709.
- **A range is keyed to one root and one field.** `V` `atom_place`'s range branch (cpp 1627–1638) takes a
  `Formal` or an array global, no path; `RangePlace` (`L` 16072–16087) and `ArrayPlace` (16031–16049) hold one
  element type and suffix; `call_atoms`' per-atom record (11089–11106) holds one bound temporary per part. The
  range helpers hardcode one binder `k` and one receiver: `range_perm` 4171–4184, `range_quant` 4214–4222,
  `in_range` 4224–4231 (it keys on `block_of`, so a leaf of a structure element never hits), `range_shape`
  4234, `arr_quant` 6790–6803, `over_slots` 1171, and the quantifiers at 1338 and 1550. **No helper builds
  `loc(member(loc(block_of(p), k), j), 0)` or a second binder.** Section 8 has the fan-out sites.
- **M4.5e1e1's helpers for an array leaf** (`L`): `ArrLeaf` 1051–1059, `leaf_arrays` 2454–2472, `obj_cells`
  2439–2453 (it drops array leaves by design), `leaf_place` 2406, `site_arrays` 6750–6768, `arr_block_term` 6784,
  `array_start_rows` 1543–1605, `ap_ref` 16052, `rebind_member_root` 5114–5138 (a chain's root as a Viper
  variable), the `array_global_*` family 7938–8009. **The bound local has no helper:** it is written inline
  (`new_temp(Ref)` and an `assign`) at 4306, 5157, 5254, 15200, 16175, 18160 and 18185, and `call_atoms` has its
  own `bind` lambda (11110).
- **Already there for (S) and (Q):** `elem_addr` composed with `member_addr` gives the planned term
  `loc(member(loc(block_of(p), offset_of(p) + i), k), 0)`; but `member_addr` (`L` 10556–10599) marks a place
  only for an inner `MemberAddr`, so an `ElemAddr` base is lowered as a value (one past the end passes at 10665:
  `v[N].x` would be a `permission` error, not `out-of-bounds`). A global array as a quantifier's or a `pure`
  function's argument in a loop invariant or a `check` is checked, lowered and pinned (`garr_forall_check`,
  `garr_pure_invariant`); (Q) owes the contract position, member arrays and members under a binder.
- **Self-test mechanics:** a new case file is one `#include` in `src/viper/selftest.cpp` (1520–1552) and one
  `define_*()` call (1826–1852); helpers `lc`, `rc`, `rl`, `ru` (raw lowering), `rh` (602–623).
- **Stale texts in `src/`:** `C` 1587, 4095 and 3617 still say "M4.5e1e", and `src/viper/selftest_cases28.inc`
  21 expects the substring "structures: M4.5e1e"; `E` 184 says "not a local array" although globals, members
  and literals decay since M4.5e1e1; `E` 1220 cites 0059 where 1478 cites 0063; the comment at `E` 2993–2994
  and `internal/encoding.md` 1087 say "e1e"; `E` 15532 is stale; `pointer_to_array_struct`'s comment (8674) says
  any pointer level is stripped.
- **Pins:** 11 test files name M4.5e1e2, with 36 expect rows; 23 rows are lifted here and 13 stay (section 10
  has every row). The Hp, Hw and 306 rows name no session in their text; `must_init` has no row of its own, and
  question 293 has no marked-member row (R2 adds both as staging pins).
- **Question 304 changes no existing file.** 38 C and C++ files under `tests/` and `doc/` hold `{0}`; none puts
  it over a nested-structure first member of an aggregate of two or more members in C++ (the two C++ files are
  `struct_init_cpp_zero_check.cpp`, scalar members, and `struct_array_reject_zero_cpp.cpp`, question 303's pin).
  The matrix probe `init_local_nested_struct_zero.cpp` is not a test. C's exception is pinned by
  `tests/memory/negative/reject_struct_init_elision.c`.
- **The lint refuses both new shapes today** (read, not run), and no self-test holds either: `QUANT_HEAD_RE`
  (3827) and `BLOCK_ROW_RE` (1012) take one binder; `parse_range_quantifier_full` (3842) needs the trigger
  `loc(block_of(X), k)`; `BLOCK_BASE_RE` (1645) is `block_of(<identifier>)`; `parse_struct_chain` (734) and
  `block_term` (1023) refuse `loc(block_of(p), k)` as a chain root; `BOUND_RHS_RE` (4021) binds a literal slot
  only; `alloc_arr_*` (3628) and `zero_arr_*` (3641) are pinned clause sets over declared `v_t`/`d_t` fields;
  `array_global_group` (1199) pins `program_start`'s array row group; the user-quantifier rule needs the trigger
  `loc(block_of(X), q<j>)` (4232). M4.5e1e1's rules to extend: `parse_block_row` 1016, `block_row_problem` 4118,
  `bound_locals` 4036, `member_bound_slots` 2126, `struct_allocation_problem` 798 (an array member 837–877),
  `struct_wf_lens_problem` 756, `method_len_facts` 4006, `member_array_counts` 4025, `struct_array_counts` 1180,
  `array_global_counts` 3991, the dispatch 5371–5386.
- **Fixtures to start from:** `tests/viper-smoke/memory/s47_x1_array_of_struct{,_caller,_fail}.vpr`,
  `s47_x2_matrix{,_fail,_vacuity_fail}.vpr`, `s26_member_array_of_structs`, `s26_member_matrix`,
  `s26_member_malloc_struct_array`.
- **Self-tests:** the next free case file is `src/viper/selftest_cases35.inc`; `VIR_CASES_MIN=2448`
  (`scripts/check.sh` 104).
- **Docs today** (`doc/modules/ROOT/pages/`): `memory.adoc` 530–537 (what a structure with an array member cannot
  do yet), 539–540, 730–731 (`int a[][3]`), **802–810 (stale since M4.5e1e1: it still lists global, `static`,
  member and literal arrays as unsupported)**, 813–815 (quantifier arguments), 951–956, 1957–1969 (the not-yet
  bullet); `verifying.adoc` 1242–1245 and 1283; `overview.adoc` 101; `annotations.adoc` 1039–1041;
  `tutorial/arrays.adoc` 189–198 ("What is next"). Anchors of `memory.adoc`: structures 355, `#array-members`
  473, arrays 553, ranges 625, array parameters 717, quantifiers 733, globals 864, `#global-arrays` 923, the
  heap 1230, owning fields 1595, not-yet 1938. `internal/encoding.md`: §2 181, §3 1207–1249, §8 1839.
- **Tools:** `scripts/box_lane.sh` (`snap`, `setup`, `ship`, `build`, `run`, `--self-test`) has M4.5e1e1's lane
  names as defaults (`CERASTES_BOX_DIR=cerastes-e1e1`, the container prefix): this session sets both for
  `cerastes-e1e2`. `regen_vpr.py --out`, `vprdiff.py <base> <new>`, `rejections.py <out> [--base]`.
- **Costs of a round trip:** the box regenerates 6,700 programs in about 25 s; its full check took 50 minutes
  with Silicon capped at 16 provers.

## 5. Step 0 and Step 1 (execution starts here)

1. **Orchestrator.** `git fetch`. The base is the commit of `main` that holds this plan (local `main` if
   `origin/main` does not hold it yet; say which in status. On 2026-11-25 local `main` and `origin/main` each
   had their own merge of M4.5e1e1, with the same tree for `src/`, `tests/` and `scripts/`: the owner's to
   reconcile, and no reason to stop). `git worktree add .claude/worktrees/m4.5e1e2 -b
   m4.5e1e2-arrays-of-structures <base>`; submodules with `--reference` (edg, silicon, then silver);
   `scripts/bootstrap.sh`. Older worktrees and branches (e1d's, e1e1's) are left alone (the owner's step). `npm
   ls @antora/logger` in `doc/` shows one hoisted copy (**nested copies: the advisor, then the owner**). CI
   recorded only (`gh run list --workflow ci.yml --limit 5`). Whether M4.5e1f has landed (the record's number).
   One question to the advisor, to check that it answers. **The base is green:** the owner ran the full check
   on M4.5e1e1 after its session (question 311, 2026-11-25), so no check of the base is made.
2. **edg-backend.** The box lanes with `scripts/box_lane.sh` (`~/cerastes-e1e2/{base,il,edg,work,audit}`; if
   `~/cerastes-e1e1/work` is still there and its tree equals the base for `src/`, `include/`, `tests/` and
   `scripts/`, copy the lanes from it and skip the builds; `df -h` first, and fewer lanes with a note in status
   if the disk is short). One ship–build–run round trip is timed; over five minutes goes to the advisor. The
   baseline before any change to `src/`: `scripts/regen_vpr.py` and the VIR self-test into the `base` lane's
   `build/e1e2-base/`; frozen binaries kept in the `work` lane for R1–R3.
3. **test-mutator, on the frozen build** (lane `work`, both modes):
   - **R1** today's outcome of every pin section 10 lists (unchanged by this session unless section 10 says so).
   - **R2** today's diagnostic for each construct of section 1's "In" list and each row of its table, one small
     program each. **One staging pin per construct that a later part of this session lifts and that no existing
     pin covers,** each retired by the part that lifts it (Flips A). `grep -n 'e1e2' src/` against section 4's
     counts.
   - **R3** `scripts/rejections.py build/m4.5e1e2/s0/rejected-base.txt`. After each part the same run with
     `--base` may differ only by section 10.
   - **M** is P83 (h)'s compiler matrix (section 6).
   - R3 is the only `run_tests.py` run until S48's start.
4. **Step 1, in parallel:** il-analyst-1 on P83 and S48; edg-backend on Part A; the test-mutator on the lint
   rules of record item 17 against S48's fixtures, with twins; the orchestrator on record 0064's draft from
   section 7.

## 6. P83 and S48 (il-analyst-1, before any encoder code; results in `internal/il-inventory.md`, `## M4.5e1e2`)

**P83 (IL, both modes, `build/m4.5e1e2/p83/`; every claim with its `--il_display` or `--cerastes_walk`
excerpt; P82's scratch readers are described in the M4.5e1e1 chapter).** A row's "If not" says what is built
instead; "the advisor" means section 12's route.

| Probe | Expected | If not |
|---|---|---|
| (a) **arrays of structures in code:** `struct S v[3];`, `v[i].x`, `v[i].t.x`, `p[i].x`, `&v[i]`, `&v[i].x`, `f(v)`, `t = v[i]`, `v[i] = t`, `v[i] = v[j]`, `f(v[i])`, `struct S t = v[i]`, `sizeof(v)`, `sizeof(v) / sizeof(v[0])` | the variable's type is e1a's `tk_array` entry over the structure's type; `v[i].x` is `dot_field(subscript(array_to_pointer(var v), i), x)` and `p[i].x` the same over `var p`; a copy is `eok_assign` (or `dik_expression`) over the subscript lvalue, never a call (P65, P82 (d)); a by-value argument is the subscript | another access shape in one mode: one reading rule, recorded. A copy the IL shows as a call or an expansion: **the advisor** |
| (b) **initializers of arrays of aggregates:** `{{1, 2}, {3, 4}}`, `{{1, 2}}`, `{0}`, `{}`, the elided `{1, 2, 3, 4}`, `{{.x = 1}}`, `{[1] = {1, 2}}`, non-constant elements, an excess element; a matrix's `{{1, 2, 3}, {4, 5, 6}}`, `{{1}, {4}}`, `{1, 2, 3, 4, 5, 6}`; the same as static lists | one nested `ck_aggregate` per written element, trailing elements absent; elision told from a nested list only by each inner aggregate's `explicit_braces_on_aggregate` (P82 (c)); an array designator a `ck_designator`; P38's limit per list | elision not told from a nested list: every list without the inner braces is rejected except `{0}` and `{}`, recorded |
| (c) **arrays of arrays:** `int m[2][3];`, a bound `[2 * 2]` in each position, `typedef int row[3]; row m2[2];`, `m[i][j]`, `&m[i][j]`, `m[i]` as an argument and an initializer, `f(m)`, `sizeof(m)`, `sizeof(m[0])`, `sizeof(int[2][3])`, `_Alignof(int[2][3])`; a global matrix's decay | a `tk_array` entry whose element is a `tk_array` entry, each bound with its own `expr` (P82 R2's generated cast in C++); `m[i][j]` is `subscript(array_to_pointer(subscript(array_to_pointer(var m), i)), j)`; a row's use is `array_to_pointer` over the inner subscript; a global's decay may be folded into a `ck_address` whose `expr` is the written decay (P82 R1) | an inner bound whose `expr` cannot be reached: arrays of arrays with a computed inner bound are rejected (pinned, M8), recorded. Another access shape: one reading rule |
| (d) **array-of-rows parameters:** `int a[][3]`, `int a[2][3]`, `int (*a)[3]`, a wrapped bound in each position; `a[i][j]`, `a[i]`, `sizeof(a)`; a call `f(m)`, and one with a matrix of another inner bound | the adjusted type is a pointer to the row's `tk_array` entry, the row's bound with its `expr`; the written outer bound is where P79 found a one-dimensional parameter's; `f(m)` passes `array_to_pointer(var m)`; a wrong inner bound is the front end's error in both modes | the row's bound not readable as written: array-of-rows parameters stay rejected (**cut 5 without asking**, M8). The three spellings not all one type: the ones that are are accepted, the others pinned |
| (e) **members and nesting:** `struct T { struct S v[2]; int n; }`, `struct U { int m[2][3]; }`, `struct W { int a[4]; int n; } w[3]`; `t.v[i].x`, `p->v[i].x`, `u.m[i][j]`, `w[i].a[j]`; C++'s class tests of 0043 item 2 on each | the field's type is the `tk_array` entry of (a) or (c); the accesses compose (a)'s and P82 (b)'s shapes; trivial special members | a C++ structure failing the class tests: **the advisor** (accepted in C only, pinned in C++) |
| (f) **globals:** `struct S g[2] = {{1, 2}};`, `int gm[2][3] = {{1}, {2}};`, a `static` local of each, a pointer leaf `{{&h, 1}}`, a list past P38's limit | `initk_static` over (b)'s shapes; each element constant keeps its `expr` under P38's rule | an element whose constant cannot be read through `constant_value`: rejected by that rule (pinned) |
| (g) **quantifier bodies and arguments:** a `pure` body that reads `p[i].x` and `p[i + 1].t.x`; `cerastes_forall(f, p->a, 4)`, `(f, s.a, 4)` and `(f, g, 4)` in a `requires`, an invariant and a `check`; a `pure` application `sum(p->a, 4)` | the body's read is (a)'s shape over the parameter; each argument is `array_to_pointer` over a field or a variable (a global's possibly R1's folded constant), with no load | an argument that arrives as a temporary or under a conversion `forall_site` cannot peel: that argument form stays rejected (pinned, M4.5e2), recorded |
| (h) **the compiler matrix M** (the test-mutator; clang and GCC, both modes, no flag / `-Wall` / `-Wall -Wextra`): (b)'s initializer forms; **question 304's forms again** (a nested-structure first member and an array first member, in aggregates of one and of two members, `{0}` and `{}`, as a local, a global, an element and a compound literal); a constant index outside each dimension; `if (m[0])`, `m[0] == NULL`; `sizeof(a)` of an array-of-rows parameter; `sizeof(m) / sizeof(m[0])` | **rule, not a decision** (0020 item 5's union of the first two tiers): what warns without a flag or at `-Wall` is rejected like the compilers by the rule its one-dimensional form has; every positive compiles with `-Wall -Wextra -Werror`; question 304's set is exactly what clang++ flags at `-Wall` | a new warning family with no existing rule: **the advisor** (reject like the compilers, with a pin, is the default). Question 304's measured set differing from item 16's wording: the measured set is the rule, and Flips G is re-listed before Part B's flips are made (**the advisor**; a file outside Flips G changing is the owner's) |

**S48 (hand-written Viper in the shapes of section 7; fixtures `tests/viper-smoke/memory/s48_*.vpr`; every row
expanded as lowering will write it, from the frozen build's kept Viper of M4.5e1e1's tests and from S47's
fixtures, `s47_x1_*` and `s47_x2_*` first; one file at a time through `scripts/run_tests.py` on the box; times
on a quiet lane).** A twin must fail with **exactly one** error unless the table says otherwise. Every row has
an `assert false` twin (vacuity). The count in brackets is the row's cap. **Two passes:** rows (a)–(g) and (n)
gate Commit 1; rows (h)–(m) are run right after, while audit round 1 reads the record, and their outcomes are
appended to the chapter and land with Commit 2 (il-analyst-1 ends then, and il-analyst-2 starts only after it).

| Spike | Programs | Built when it passes | If it does not |
|---|---|---|---|
| **(a) a local array of structures** [4] | `alloc_arr_S` in item 6's shape (S47 (x1)'s caller is the start); `v[i].x` read and written; `&v[i]` passed to a callee with `owns(*p)` (its cell lines taken out of the quantified leaf chunks and given back); the scope exit per leaf. Twins: an index outside (`out-of-bounds`); a callee that kept one member of one element (the marked row, `insufficient.permission`) | items 3, 5 and 6 | a positive failing or over 30 s: **the advisor** (protected: the owner if no form passes) |
| **(b) a range over structures** [5] | beside S47 (x1): `owns_range` in a callee that writes `p[i].x` with the caller reading it back; `reads_range` at a ghost amount (the minimum over each leaf's first slot); a call that forwards `(p, n)`; a return's rows. Twins: a write under `reads_range` (`permission`); a caller holding one element fewer (the marked row) | items 7 and 8 | as (a) |
| **(c) the pairing rows** [8] (**protected**) | each in Form P and in Form S of item 7: a range of structures beside a cell (`f(v, 3, &other)` verifies; `f(v, 3, &v[1].x)` fails once); beside a structure atom (`&t` verifies; `&v[1]` fails once); two ranges of structures (disjoint halves verify; overlapping fail once) | item 7's pairing rows in **the form that passes every pair** (Form P if both do) | one form passing only some pairs: that form where it passes, the other for the rest, recorded. A pairing no form decides: **the advisor, then the owner**. An alias twin verifying in a form: that form is never used, and if in both, **stop, the owner** |
| **(d) loops over an array of structures** [4] | a local array and a parameter's range, each leaf an array or range place with one liveness conjunct per root; a loop that writes `p[i].x` under a quantified invariant over `x`. Twins: `invariant-preserved`; a store through an `int *` with the values-kept conjunct present (the conjunct must be absent) | item 10 | a positive failing: **the advisor** |
| **(e) a global array of structures** [4] | `program_start` with item 9's row group per leaf (`struct S g[3] = {{1, 2}, {3}}`), `main`'s loop, an `owns(g)` callee. Twins: a `check` against a zero leaf; the unknown-value form with a `check` on a value | item 9 | **cut 3 without asking** (global and `static` arrays of aggregates stay rejected, pinned to the continuation session's first part) |
| **(f) an array member of structures** [3] | `struct T { struct S v[2]; int n; }` as a local and under `owns(*p)`: `alloc_struct_T` with the nested length facts, `t.v[i].x`. Twin: `f(&t, &t.v[1])` with `owns(*p) && owns(*q)` (`permission`) | items 3 and 7 for a member | **cut 4 without asking** (an array member of aggregates stays rejected, M8) |
| **(g) depth 2** [6] | beside S47 (x2): an array-of-rows parameter with `owns_range(a, n)` over two bound variables and a caller passing `m`; a row `m[i]` passed to a callee's `owns_range(q, 3)` through a local bound to the row's first slot (e1b's rows unchanged over it) with the caller reading the element back; `v[i].a[j]` over an array of structures with an array member; **the copy `v[i] = v[j]` of such elements under item 12's guard** (`i == j` verifies and copies nothing; `i != j` copies; the guard is decided). Twins: `m[0][3]` (the second bound); a row callee that keeps one element | items 5, 12 and 13 | the row call failing: a row as a range argument is rejected (pinned, M4.5e2), recorded. The parameter failing: **cut 5 without asking**. Depth 2 failing or over 30 s altogether: **the advisor** (Part E then closes at what passes) |
| **(n) cost** [3] | an array of structures with four leaves and a loop over each; S45 (k)'s scale program at K = 16 holding one array of structures; a global array of 1,000 structures with 16 listed | recorded | **over 60 s: the advisor** |
| **(h) heap storage** [4] (**protected**) | the typed view of `malloc(sizeof(struct S))` for a structure with an array member (the token, the member lengths, the array leaf's quantified lines); `p->a[i]` written and read; `free`'s rows per leaf. Twins: a second `free` (`invalid-free`); a leak | item 15, heap | **the advisor; no form passing: the owner** (question 292 has waited once already; Part D's other items go on meanwhile) |
| **(i) markers** [3] (**protected**) | a `ref_to_uninit` pointer to a structure with an array member (no flags claimed; the kept-flag quantifier); a `must_init` filler. Twins: the caller reads an element the callee did not write (`uninitialized`); a filler that misses one element | item 15, markers | **the advisor; no form passing: the owner** (question 294) |
| **(j) a read class with member steps** [3] | a user quantifier over `p[q].x` in a `requires`, an `ensures` and a loop invariant of a fill loop, with `f__wf`'s variant M over the leaf; one over a member array in the block-term form (`{ loc(member(p, 1), q) }`). Twin: `invariant-preserved` | item 14 | a positive failing: **the advisor**; no trigger form passing: **cut 1 without asking** (Part F's member classes) |
| **(k) a written atom** [2] | `owns(*p)` written in a loop invariant for a structure with an array member, the leaf's amount its first slot's; one twin | item 15, written atoms | stays rejected with question 237 (M4.5e2), no stop |
| **(l) a partnerless atom** [2] | `ensures(r, owns(*r))` of a fresh structure with an array member, given its output amount; one twin | item 15, partnerless | stays rejected (pinned, M4.5e2), no stop |
| **(m) depth 3** [3] | `int c[2][3][4]` with a permission over three bound variables and nested loops; a twin; times | depth 3 is admitted (item 2) if every program verifies under 30 s | depth 3 stays rejected (M8), no stop |

**Stop rules for all of S48:** a second Silicon error after a marked row; `qp.not.injective`; a native error in a
positive; an outcome no table names. Each goes to the advisor first.

## 7. Record 0064 — arrays of structures and arrays of arrays (the orchestrator writes it before code from this section; revised only by section 6's tables and audit round 1)

Amends 0012 item 11 (the lint); 0024 item 6 (array types of aggregates); 0028 items 2 and 16 (as built); 0043
items 6, 7, 9 and 11; 0044 items 2–5 and 8; 0045 items 1 and 12; 0047 items 2, 4, 6 and 8; 0052 item 2; 0055
items 2, 3 and 5; 0058 item 4; 0059 items 1–4, 6, 8 and 9; 0060 items 1, 3–5, 8, 9, 11–13 and 15; 0061 items 1, 3
and 6; 0062 item 4; 0063 items 1, 2, 4–10 and 13–15.

**No existing program's Viper or map changes in this session.** Every item adds rows only for a program that
holds a construct rejected today; where an existing function is touched, the text it writes for a program
without the new constructs is the same, and `vprdiff` shows it after every part. **One accepted form becomes a
rejection:** item 16, in C++ only (the owner's answer to question 304). No existing test or doc example holds
that form (section 4), so no existing outcome changes; Flips G says what happens if one does.

1. **Scope:** section 1, with its table.
2. **Which arrays** (amends 0059 item 2, 0063 item 2). The element is a scalar that 0059 item 2 accepts, a
   verified structure, or an array of these; **no leaf path is deeper than 2** (3 if S48 (m) admits it). The
   `const` rule is tested first (M4.6), then: an element structure with an `owning` or marked member at any
   depth is rejected (`unsupported_type`, M4.8). **Every bound is read as written** (0063 item 2's rule, each
   dimension on its own: a prologue constant for a global, a member and a file-scope typedef, a constant of its
   function otherwise; "the bound of `m`", "the second bound of `m`"). `owning`, `ref_to_uninit` and `uninit` on
   an array of aggregates are rejected as on any array.
3. **Layout** (0028 items 2 and 16, as decided in M4.5a).
   - Element `i` of the array block `B` is the slot `loc(B, i)`. For a structure element, its member `k` is the
     block `member(loc(B, i), k)`, exactly as 0043 item 3 and 0063 item 3 lay out a structure at any slot. For an
     array element (a row), the row is the block `member(loc(B, i), 0)` of the row's length.
   - **The length facts of a range of slots:** `forall k: Int :: { loc(B, k) } lo <= k && k < hi ==>` the
     conjunction, for the element at `loc(B, k)`, of what `struct_lens` states of one structure slot (`len(member(X,
     j)) == 1`, or the count for an array member; a nested structure's own facts; 0043 item 8's slot facts), or
     `len(member(loc(B, k), 0)) == M` for a row. They are stated by an allocation (`alloc_arr_<E>`), by
     `program_start` and the global facts, by `struct_lens` for an array member of aggregates, and by the first
     line of a range atom over structures or rows. They hold in 0028 item 2's model (`len` is free; a member
     block is a fresh term) and are hypotheses discharged by an allocation, by `program_start` or by a caller,
     never axioms.
   - Every member block, at any depth, has its root block's liveness cell (`member_lv`): one liveness cell per
     root, as before.
4. **VIR** (amends 0063 item 4).
   - `Type::Array`'s element may be a `Struct` or an `Array`. `Ptr(Array)` is a type of a **parameter** only (item
     13), never of a local, a member, a result or a cell.
   - **Leaf paths.** `object_leaf_paths(type)` gives every leaf of an object type as its steps (member `k`; index,
     with its count) and its cell type, in declaration order, one path per leaf whatever its counts. For a type
     0063 accepted it is `struct_object_leaves`' list (a cell: member steps only; an array leaf: member steps,
     then one index step), and **every site that 0063 gave rows writes the same text for those types**. **The
     poison rule continues:** a walk of leaves that has not been given the general path reports an internal
     error on a path it does not handle (an index step that is not last; two index steps) — exit 3, never a cell
     read as something else — `check.cpp` refuses an array of aggregates in every position a part has not
     lifted, and the encoder rejects it there first with a diagnostic at the C line (invariants 3 and 7).
   - **The traps are closed before any gate is lifted.** The planning's survey found six places where lowering
     or the checker would treat an aggregate element wrongly without an error (a nested `ElemAddr` as a flat
     pointer; element keys that ignore an index; a range over structures with no field, so a callee's writes go
     unrecorded; walks that count an array by its element; nest walkers that skip elements; a structure element
     compared as one field), each masked only by a refusal upstream, and three positions only the encoder
     refuses. Part A makes each an internal error or gives it the general path, closes the three positions in
     `check.cpp`, and pins every guard with a self-test case (the plan's section 8, Part A's table).
   - **Places.** `ElemAddr`'s base may be a pointer to a structure and the address of an array of aggregates.
     Its result for a structure element is that element's address (a `struct S *` value: `&v[i]`); for a row it
     is the row's address, accepted only as the base of another `ElemAddr` and as the row's decay.
     `MemberAddr` is accepted over an `ElemAddr`. `place_root` steps through an `ElemAddr`; places are spelled as
     written (`p[i].x`, `m[i][j]`, `v[i].a[j]`), never `(&p[i])->x`.
   - `Var::zero_filled` and `ArrayZero` hold for an array of aggregates (every leaf of every unwritten element);
     `ArrayCopy` is unchanged (an array leaf of scalars inside one structure, wherever that structure stands);
     the value view expands every index step (item 12).
   - `check.cpp` backstops each rule of items 2–16.
5. **Rows in code** (amends 0059 item 4, 0063 item 5).
   - **An element access** takes 0059 item 4's rows at each index step, in source order, then the cell's.
     `v[i].x`: the bound of `i` against `len(block_of(v))` (`out-of-bounds`: "reads `v[i].x`, whose index may be
     outside `v`"), then `permission` and, for a read, `uninitialized` on the leaf cell. `m[i][j]`: the first
     bound, then `j` against the row's length ("… whose second index may be outside `m[i]`"), then the cell's
     rows. A named array is a named base (no null or liveness row). `p[i].x` on a pointer takes e1a's rows for
     `p[i]` (`null-dereference`, `p`'s liveness, the bound) and then the cell's. **A member step adds no row,**
     but the slot it steps from must exist: `&v[i].x`, `v[i].x` and `m[i][j]` need `i` strictly inside, while
     `&v[i]` and `&m[i][j]` as values allow one past the end at their last index (0059 item 10).
   - **A row used as a pointer** (`m[i]` as an argument or an initializer) is the row's decay: the bound of `i`,
     strictly inside, then a pointer to the row's element 0, a valid `T *` by the length facts.
   - **The bound local** (0063 item 5). A row over an array's slots in code is written over a Viper local bound
     to the array's first slot, with the leaf's receiver built from it: `loc(member(loc(block_of(t), k), j), 0)`
     for a cell leaf of an array of structures (a longer member chain for a nested structure), and over two
     bound variables for a depth-2 leaf, `loc(member(loc(block_of(t), k), j), l)`. Each permission is its own
     quantifier with its own receiver as the trigger (0028 item 2: one quantifier holding two members'
     permissions fails Silicon's injectivity check). A marked row still reads no heap (0059 item 7).
   - **Rejected like the compilers** (P83 (h)'s matrix): a constant index outside any dimension (EDG's #175); a
     truth test or null comparison of a row's or an element's address; whatever else warns without a flag or at
     `-Wall`.
6. **Local arrays of aggregates** (amends 0059 items 3 and 6, 0043 item 6, 0063 item 6).
   - **Allocation:** one trusted method per element type, `alloc_arr_<E>(n)` (`<E>` the structure's tag or the
     row's type), in `alloc_arr_<t>`'s shape: the block's length and liveness facts, item 3's length facts, and
     per leaf path full permission to the value field and to the flag field (each its own quantifier) with every
     flag false. `zero_arr_<E>` is `zero_arr_<t>` per leaf. Each (receiver, field) pair is given once.
   - **Scope exit:** per leaf path the two marked rows over the bound local (`lifetime`: "the address of an
     element of `v` may still be in use when its lifetime ends"), then the `exhale`s.
   - **Initializers:** each element takes a nested braced list in its own rules (0043 item 6 for a structure,
     0059 for a row); unwritten elements and leaves are zero (`zero_arr_<E>` first, then the written leaves'
     stores); in C every element of the whole list is evaluated before the first store; elision of an element's
     braces is rejected like the compilers except `{}` and C's `{0}`, and **C++'s `{0}` follows item 16**; an
     element that reads, writes or takes the address of the array being initialized is rejected.
7. **Contracts** (amends 0060 items 1 and 3–5, 0063 item 7).
   - **A range atom over structures or rows.** `owns_range(p, n)` and `reads_range(p, n)` with `p` a pointer
     parameter to a verified structure (or to a row, item 13): 0060 item 4's first line with item 3's length
     facts over `[offset_of(p), offset_of(p) + n)`; then, **per leaf path of the element,** 0060 item 4's two
     permissions, flags and type range, each a quantifier over the absolute slot with the leaf's receiver as its
     trigger (`{ loc(member(loc(block_of(p), k), j), 0) }`; two bound variables for a depth-2 leaf); one liveness
     fraction. Partners, the guard outside each quantifier and the ghost amount are 0060 items 3 and 4's.
   - **Element reads** `p[e].x`, `p[e].t.x`, `p[e].a[e2]` and `g[e].x` in `requires`, `ensures`, a function's
     `decreases` and under `old`, the indexes reading no memory. A read is framed by a range atom when **its
     element slot** lies in the range (0060 item 5's disjunct on `&p[e]`; the member steps then name a leaf the
     atom holds), and by a structure atom whose place is that slot.
   - `owns(g)` of a global array of aggregates is the range atom over the whole array (as 0063 built it for
     scalars); `owns(*p)` of a structure with an array member of aggregates takes that member's leaf paths with
     its other leaves; `owns(p->v)` is that member alone.
   - **Overlap is by slots** (0043 item 9 and 0063 item 7, the soundness core). A leaf path of a site is a set
     of slots; every rule that pairs two sites (the exclusion of `owns` and `reads`, the amount sums at calls and
     returns, `f__wf`'s distinctness and equal-ghost facts, framing disjuncts, loop places) pairs a leaf path
     with a cell, an array leaf or another leaf path of the same field by those slots. So `f(v, 3, &v[1].x)`
     with `owns_range(p, n) && owns(*q)` fails at the call (`permission`), and so does `f(v, &v[1])` with
     `owns_range(p, 3) && owns(*q)` over structures.
   - **The pairing rows** (S48 (c) decides between the two forms; one of them for every pairing). **Form P:** a
     pure membership row, as 0063 item 7's interval row is: against a cell `q`, with `X` standing for
     `offset_of(parent_of(block_of(q)))`, `!(g && lo <= X && X < hi && q == loc(member(loc(block_of(t), X), j),
     0))` (the receiver term written out, so the permission of that slot is in view). **Form S:** the sum form
     of 0060 items 8 and 9, one marked `acc` row holding both sites' permissions (`permission`). Never a bare
     block comparison, never `perm` under a quantifier. A row Silicon cannot decide is a false `permission` or
     an exit 3, never a proof.
   - **Amounts** (0060 item 6): a `reads_range`'s amount is the minimum over each leaf path's first slot, the
     liveness cell and the caller's own read amounts of those fields (question 238's listed limit applies).
   - **Rejected** (`unsupported_annotation`): section 1's M4.5e2 rows; `restrict` backed by a range over
     structures; two atoms on one parameter that overlap (`owns_range(p, n) && owns(*p)`).
8. **Calls and returns** (amends 0060 items 8 and 9, 0063 item 8). A range atom's rows are per leaf path: the
   shape row once (with the length facts), then e1b's marked rows (value, then flag) and its flag and range rows
   for each leaf, over the bound argument; the messages in source terms ("calls `f`, which needs `p[0..n)`: the
   member `x` of every element"). An array of aggregates passed to a range parameter is bound to a local first.
9. **`program_start`, initial values, `static` locals** (amends 0063 item 9, 0044 item 8). A global array of
   aggregates has, per leaf path, 0063 item 9's row group (the two quantified permissions, the flags, the
   values, once the liveness cell) and item 3's length facts in the global facts. **The values, known:** one
   conjunct per leaf of each element the list writes (the written constant, or zero for a leaf the element's
   list leaves out), and per leaf path one quantifier for the elements the list does not reach. **Unknown** (by
   0041 item 3's linkage rule and 0063 item 9's two escape directions, per leaf): the type range as one
   quantifier per leaf. Every element constant is read through `constant_value` (P38). A `static` local array of
   aggregates is such a global with its implicit atom.
10. **Loops** (amends 0059 item 8, 0060 item 11, 0063 item 10). An array of aggregates the function holds is,
    **per leaf path,** an array place (a local, a global in `main`) or a range place (a parameter's range, a
    held global elsewhere, a held structure place's member), with 0059 item 8's entry rows, conjuncts and
    back-edge rows over the bound local; one liveness conjunct per root (S47's F5). **Reach** follows the rules
    in force: a store by name to a leaf of a global array the function holds reaches that leaf path only (0044
    item 8); a store to a member of an element of a local array, any store through a pointer of the leaf's
    field, and a call whose `requires` owns a cell of that field reach every place of the field (0043 item 11).
    **Listed false alarm:** a loop that writes `v[i].x` cannot keep `v[k].y` by the automatic conjunct when
    both are of one field; a written quantified invariant keeps it (Part F), and the precise rule is M4.8's.
11. **Sequencing** (0039 item 3, 0059 item 9, 0063 item 11). Object keys gain the index steps: `v[0].x` and
    `v[1].x`, and `v[i].x` and `v[i].y`, are different objects; two unsequenced accesses to the same leaf path
    of one array, one a write, with indexes that are not all constants, get the path-precise `unsequenced` row;
    the same leaf named twice with a write is `unsequenced_modification`.
12. **Structure values and elements** (amends 0043 item 7, 0044 items 2–5, 0063 item 13). An element `v[i]` or
    `p[i]` is a structure place: a copy's source and destination, a by-value argument, an initializer's source,
    with 0043 item 7's and 0063 item 13's rows, after the index's own rows. Cell leaves are copied by leaf
    stores, which need no guard (`v[i] = v[i]` rewrites each value). **For an element structure's array leaves
    (Part E), `copy_arr_<t>`'s guard `d != s` is written where the two places can be one object:** a place
    reached through a pointer (0063 item 13) and two elements of one array (`v[i] = v[j]`); **never between two
    named objects or between an element and another named object**, where Silicon would leave the guard's
    branch open and skip the copy (S47's F2). A structure whose
    member is an array of structures is a value by its value view: every leaf of every element is one Viper
    value, and `kMaxByValueArrayElements` (7) counts those leaves; a whole-structure copy of it is leaf stores
    and `copy_arr_<t>` calls per element — **rejected beyond 7 such leaves until M4.5f3** (no trusted
    `copy_arr_<E>` is added here). An array is never a value.
13. **Arrays of arrays** (amends 0060 item 15, 0063 item 14). A row is an array object in its own block; a
    matrix's leaf path has two index steps, and so has an array member of an element structure (`v[i].a[j]`).
    - **Array-of-rows parameters.** `T a[][M]`, `T a[N][M]` and `T (*a)[M]` declare a parameter of type pointer
      to a row. It is accepted with `owns_range(a, n)` or `reads_range(a, n)` over `n` rows (item 7), `a[i][j]`
      in code and in contracts, and a row `a[i]` used as a pointer. The outer bound of `T a[N][M]` is checked as
      written and is no promise (0060 item 15); the inner bound is part of the type. A matrix passed to such a
      parameter is its decay, bound to a local. `sizeof(a)` of such a parameter is rejected
      (`sizeof_array_argument`).
    - **Computed types** (0063 item 14, extended): a typedef, `typeof`, `decltype`, and the type operand of
      `sizeof` and `_Alignof` may name an array of aggregates when every bound in it is read as written.
14. **Quantifiers and applications** (amends 0061 items 1, 3 and 6, 0062 item 4).
    - **Under the binder** a read may step through members of the indexed element: `p[i ± c].x`, `v[i].t.x`.
      Its read class is the pair (argument, member steps), its trigger that leaf's receiver (`{
      loc(member(loc(block_of(X), q), j), 0) }`), and 0061 items 6–10's framing, bound and flag rows are written
      per class against that leaf's range or array place.
    - **The pointer argument** of `cerastes_forall` and of a `pure` application may be the decay of an array
      member of scalars (`p->a` and `s.a`; in a contract, of a structure the contract owns or reads through a
      pointer parameter) and a named global array in a contract. It reads no memory (an address is formed, not
      loaded); `p->q->a` stays rejected (M4.5e2). The class's block is the member's or the global's block term
      in a contract line, and its bound local in code.
15. **The leftovers of a structure with an array member** (amends 0063 items 1 and 7). Each site below takes
    the object view (a cell leaf as before, an array leaf with 0063's quantified lines), and audit round 4 gives
    a verdict per site of section 8's table.
    - **Heap storage** (question 292; amends 0047 items 2, 4, 6 and 8). `malloc(sizeof(struct S))`,
      `calloc(1, sizeof(struct S))` and `free` take a structure with array members: the typed view's `ensures`
      and `owns_block` state the member lengths and hold each array leaf by its quantified permissions and
      flags (`calloc`: every element zero and set; `malloc`: the flags unknown, as 0047 item 8); `free`'s rows
      are per leaf (`invalid-free`); the leak check reads the block's token as before.
    - **An `owning` or marked member beside an array member** (293; amends 0055 items 2, 3 and 5, 0058 item 4).
      The structure is accepted. An array leaf of scalars is never an owner (arrays of owning pointers are
      M4.8's), so every owner site walks the cell leaves and passes over an array leaf; the closure of an atom
      holds the array leaf's lines beside its owners' blocks.
    - **Markers on a pointer to such a structure** (294; amends 0042, 0058 item 4). `ref_to_uninit`: the
      `requires` site claims no flag of an array leaf either, and the kept-flag row is one quantifier per array
      leaf (`old(flag) ==> flag`, where 0058 item 4 keeps it for a cell). `must_init`: every element's flag is
      set at the return (`uninitialized`).
    - **Contract places through a heap read** (306; amends 0045 item 1). A load-rooted place whose structure has
      an array member takes 0045's rows with the array leaf's lines keyed by the loaded pointer; `p->q->a[e]`
      is an element read through that place.
    - **An `ensures` atom with no `requires` partner** (amends 0052 item 2) on a structure with an array member:
      covered or given per leaf, an array leaf by its quantified lines and, where 0052 gives an output amount,
      one amount for the site.
    - **A written atom in a loop invariant** on such a structure place (amends 0045 item 12, 0040 item 5): the
      array leaf's conjuncts are a range place's, its amount the first slot's (0060 item 6). If S48 (k) refuses
      it, it stays rejected with the range atoms of question 237 (M4.5e2).
16. **C++'s `{0}` over a nested aggregate** (the owner's answer to question 304; amends 0043 item 6, 0063 item
    6). In C++ a list whose `0` elides the braces of a first member that is a structure or an array, in an
    aggregate of two or more members, is rejected like the compilers (clang++ warns `-Wmissing-braces` at
    `-Wall`): one rule for both member kinds, 0020 item 5's union of the first two tiers. C's `{0}` and `{}` in
    both modes stay accepted, and so does C++'s `{0}` over an aggregate of one member. The rule is one condition
    in `collect_init_elems` (the nested-structure exception takes the array member's `c_mode ||
    st.members.size() < 2U`), which locals, globals and values share. P83 (h) re-measures the forms, among them
    C's `{0}` over a first member that is a structure beginning with an array, which C accepts and the encoder
    must too.
17. **The lint** (amends 0012 item 11, 0060 item 16, 0061 item 11, 0063 item 15).
    - **The receiver grammar** gains index steps: a quantified row's receiver is a chain of `loc(<block>, <bound
      variable or 0>)` and `member(<ref>, <literal>)` steps from `block_of(<bound local>)` in code and from a
      block term of 0063 item 15's grammar where a line has no local, with one bound variable per index step,
      each ranging over literal or `offset_of` bounds as today; the trigger is the receiver.
    - The length facts of item 3 are pinned where they are stated; `alloc_arr_<E>` and `zero_arr_<E>` by their
      clause sets, each (receiver, field) once; `program_start`'s row group per leaf path; a user quantifier's
      class trigger with member steps (item 14).
    - Each rule with a refusing twin in `scripts/test_lint_viper.py`.
18. **Categories and tags:** none new. **Facts:** P83, S48.

## 8. Parts, owners, order

Record items per part — A: 4. B: 2, 3, 5, 6, 11, 12 (elements), 16. C: 7–10, 12 (members). D: 15. E: 13, and 2,
5, 6, 9 at depth 2. F: 14. Item 17 (the lint) with each part. Line numbers are section 4's.

| Part | edg-backend | il-analyst | test-mutator |
|---|---|---|---|
| **A** leaf paths and the traps; behaviour-neutral, nothing accepted | **in `src/vir/**`:** `LeafStep`, `LeafPath` and `object_leaf_paths` beside the three views (`collect_leaves` descends into an element structure and a row; the 0063 views unchanged for the types 0063 accepted, and an internal error for an aggregate element); **the six traps of section 4, each loud** (table below); `check.cpp`'s three backstop gaps closed; the nest walkers descend into elements; the place spellings `p[i].x`, `m[i][j]` in `member_place` and `elem_place`; **one raw-lowering (`ru`) self-test case per guard**; the stale texts of `C` 1587, 3617, 4095 | il-analyst-1: P83, S48 | Step 0 (R1–R3, M, R2's staging pins); the lint's receiver grammar and length facts against S48's fixtures, with twins |
| **B** (S) in code; (I) | `array_vtype` 582 (a structure element; an `owning` or marked member at any depth rejected; **an element structure with an array member rejected until Part E**, a staging pin) and its callers for a local; `collect_array_elems` 1852 and `array_initializer` (a nested list per element through `collect_init_elems`' rules, the zero fill, C's all-elements-first, own-initializer reads); `subscript_address` 1473–1480; `place_address` 15625–15630 (an element as a structure place); `struct_value_node` 14821 and `copy_struct` 19238 (an element as a value and a copy's source; the guard wherever a place is an element); `address_of` (`&v[i]`, `&v[i].x`); the decay of `v`; `sizeof`; **question 304:** `collect_init_elems` 19059 with its message, and the C form of 19109/19117 if P83 (h) confirms it; `E` 184's text | **il-analyst-2.** `C`: `array_local_type` and `array_reason` for a structure element in `declare` and `AddrOf`; the `ElemAddr` rule for a pointer to a structure and for `MemberAddr` over it in code; the Sequencer's keys with index steps (`object_of` 5554–5628). `L`: **one leaf-range descriptor and one quantifier builder** (the base local, the index bounds, the steps after each index, the cell type), used by `range_perm`, `range_quant`, `in_range`, `arr_quant` and `over_slots` with the same text as today for a scalar element; **one helper for the bound local**; `array_alloc_method` per element structure (`alloc_arr_<E>`, `zero_arr_<E>`), `array_alloc_site`, `element_suffix`, `zero_fill`, `alloc_cell`; `elem_addr` and `member_addr` (an `ElemAddr` base is a place: the index strictly inside, the bound before the member step, no liveness row for a named array); `deref`, `store`, `cell_write` wording; `array_scope_exit` and `scope_exit` per leaf path; `ArrayPlace` per leaf path with `loop`'s collection (18122–18209), reach tests, entry rows, conjuncts and back edge; `inv_elem`, `array_place_hit` and the `inv_*` framing for `v[i].x`; `writes_in`, `writes_in_expr`, `elem_array_var`, `key_of_addr`, `memory_fields`; `struct_store` and `struct_cell_write` with an element place; `has_pointer_leaf` with `struct_ptr_uses`, `expr_touches`, `split_candidates` | Part B's tests; the lint's `alloc_arr_<E>` and `zero_arr_<E>` clause sets and the rows over a local array of structures; Flips A (B) and G |
| **C** (S) in contracts and storage | `spec_range_atom` 11167–11173; `owning_count` 4549–4554; `contract_element_address` 1217–1221, `contract_place_address_raw` 10502 and `contract_field_chain` 15708–15710 (`p[e].x` in a contract); a marker whose atom is a range over structures; `global_array_vtype`, `read_array_global_init` and `read_array_element` for structure elements (through `collect_init_elems`; both escape directions per leaf), `static_local_decl`, `add_static_local_atoms`; `struct_info` 25326–25371 (a member that is an array of structures), `struct_initializer`, `read_struct_global_init`; the value view in `object_value`, `aggregate_value`, `signature_vtype` and `copy_struct` (item 12's limit over leaves) | **il-analyst-2.** `C`: `range_atom_operands` 1583–1588; the contract rules of `Deref` 3663, `MemberAddr` 3893 and `ElemAddr` 4031–4060 through `place_root`'s `ElemAddr` step; `struct_type`, `global`, `global_array`; `V` `atom_place`. `L`: **the fan-out table below**; the pairing rows in the form S48 (c) chose (`overlap_pairs`, `arr_disjoint`); `program_start_method` with `array_start_rows` and `global_facts`; `struct_lens` and `alloc_struct_method` for a member that is an array of aggregates; `leaf_arrays`, `site_arrays`, `ArrLeaf`; `held_keys`, `loop_places`; the value-view sites (`param_vars`, `result_vars`, `elem_value`, `leaf_span`, `struct_store`) | Part C's tests; the lint's range-atom rows per leaf path (`parse_range_quantifier_full`, `range_atom_row_problem`, `_range_acc`, `_loop_conjunct`), `array_global_group`, `struct_allocation_problem`; Flips A (C) and F |
| **D** (H) the leftovers | the gates of section 4's (H) list: `allocation_view` 17909, `heap_call` 18044, **`heap_view` 17724–17789 on the object view** (its guard and the last `struct_leaves` call go), `spec_block_atom` 18127, `invariant_block_atom` 18221; `struct_info` 25263, 25312 and 25484; `read_marker` 3419, `apply_function_markers` 4195, `read_variable_markers` 14182; `reject_array_behind_load` 10302; `finish_atoms` 13077; `invariant_atom` 11304; `pointer_to_array_struct`'s levels | **il-analyst-3.** `C`: `array_with_marked` 495–504, `init_markers` 1813, the atom rules 3764, 3779 and 3837, `ElemAddr` 4051–4058, and Part A's three backstops, each lifted with its rule. `L`: **the leftover table below** | Part D's tests; the lint's typed views, `owns_block` lines, `free` rows and marked closures with an array leaf; Flips A (D) |
| **E** (A) arrays of arrays | `array_vtype` 577; **a `Ptr(Array)` parameter:** `vtype` 2494–2505, `check_signature` 3005–3022 (one report, not two), `check_array_parameter_declarations`, `check_array_parameter_bound`; `subscript_address` 1452–1459 and `contract_element_address` 1184–1190 (`m[i][j]`); `describe_operation` 184 with `operation` 16224 (a row's decay); `collect_array_elems` and `array_initializer` for rows; `reject_computed_sizeof_operand` 7257–7280, `scan_type`, `array_type_reason`, the typedef helpers, `check_array_type_bound` per dimension; `global_array_vtype` and `read_array_global_init` for rows; `struct_info` for a matrix member; `v[i].a[j]`. `address_of` 15849–15897 and `declaration_level_checks` 25568 keep M8 for every other pointer to an array; `read_forall_function` 12142 keeps refusing such a parameter | **il-analyst-4.** `C`: `accepted_pointer` and `pointer_reason` for a `Ptr(Array)` **parameter only**; `array_addr_ok_` for a row (an `ElemAddr` typed `Ptr(Array)`, as an `ElemAddr`'s base and as a decay); `array_reason` for a row element; `global_array` 676 and 732; the Sequencer. `L`: `elem_addr`, `inv_elem`, `lb_chain` over an `ElemAddr` base (`loc(member(loc(B, i), 0), j)`, the second bound against the row's length); `same_elem_base`, `elem_base_key` and `wf_method` 24614 with every index in the key; **the descriptor at depth 2 through every helper of Parts B and C**; `alloc_arr_<row>`; `fkey` of an `Array`; `program_start`'s nested values; a range over rows | Part E's tests; the lint's second binder (`QUANT_HEAD_RE`, `BLOCK_ROW_RE`) and a row's bound local (`BOUND_RHS_RE`); Flips A (E) |
| **F** (Q) quantifiers and applications | `reject_named_array_in_contract` 978–1004 with its callers 6557 and 12572; `contract_element_address` 1157–1176; `subscript_address` 1386 and 1436; `array_decay` 1557–1583; `member_array_decay` 1612–1627; `forall_base_ok` 11812–11828; `read_forall_function` 12172–12181 (a parameter that points to a structure); `forall_site` 12572–12635; `application` 6557–6653, and the dead 6570–6576 removed. `elem_value_read` 1679 stays (re-pointed to M4.5e2) | **il-analyst-4.** `C`: the body whitelist 3287–3312 and `Deref` 3611–3619 (`MemberAddr` over the bound element); `forall_shape`, `forall_node`, `forall_loop_base`, `array_of_named` (a member array; a contract's base); `apply` 5010–5037. `L`: `fq_collect`, `fq_binder_class` (the class key gains the member steps), `fq_resolve`, `fq_slot`, `forall_expr`, `fq_loop_read`, `fq_check_rows`, `fq_base_formals`, `is_named_array`; `apply_frame`, `apply_inv`, `apply_check` (a member array through its bound local); `wf_method` 24612 (the ghost ties of element keys over a `GlobalAddr` or `MemberAddr` base under a binder) | Part F's tests; the lint's user-quantifier trigger with member steps and in the block-term form; Flips A (F) |

**Part A's table: the traps** (each row gets an internal error or the general path in Part A, a raw-lowering or
checker self-test case that pins it, and **a verdict in audit round 2**; a part that later gives a site its
rows removes that site's guard and keeps its pin as a positive case).

| Trap (section 4) | Sites | Made safe by |
|---|---|---|
| 1 a nested `ElemAddr` read as a flat pointer | `L` `elem_addr`, `inv_elem`, `lb_chain` | an internal error for an `ElemAddr` whose base is an `ElemAddr` or is typed `Ptr(Array)` without being one of the three array addresses (Part E gives it rows) |
| 2 element keys that ignore the base's index | `L` `same_elem_base`, `elem_base_key`, `wf_method` 24614 | an internal error for a base that is none of `Formal`, `GlobalAddr`, `MemberAddr`, `AddrOf` (Part E: every index in the key) |
| 3 a range over structures with no field | `L` `site_fields`, `memory_fields`, `fkey` | an internal error for a structure or array element (Part C: one field per leaf path) |
| 4 walks that count an array by its element | `L` `leaf_count`, `leaf_span`, `has_pointer_leaf`; `C` `value_array_elems`; `V` `collect_leaves`, `may_hold_pointer_to` | `leaf_count`, `leaf_span` and `value_array_elems` sum, over the leaf paths, **the product of each path's index counts** (one path stands for that many value leaves; the numbers are today's for every type 0063 accepted, which `vprdiff` shows); `has_pointer_leaf` and `may_hold_pointer_to` descend (a pointer anywhere inside counts); the 0063 views stop on an aggregate element |
| 5 nest walkers that skip elements | `C` `struct_has_array`, `struct_has_marked`, `contains_by_value`, `owning_reaches` | each descends into an element structure and a row |
| 6 a structure element compared as one field | `L` `cell_suffix` at the loop's reach tests, `site_fields`, `overlap_pairs`, `arr_disjoint`, the `inv_*` matches, `element_suffix` | `element_suffix` and those sites stop on an aggregate element (Parts B and C: per leaf path) |
| the backstop gaps | `C`: a written `owns(*q)` in an invariant, a partnerless `ensures` atom, and an atom through a heap read, each on a structure with an array member | three refusals in `check.cpp`, each with a rejection case (Part D lifts them) |

**Part C's table: where one range becomes one row group per leaf path** (`L`; **audit round 3 gives a verdict
per site**).

| Contract lines and facts | Calls and returns | Loops and invariant reads |
|---|---|---|
| `range_atom_value` 7443–7515; `pf_atom_quants` 22789; `contract_read` 8948–8976 and 9017–9031; `wf_place` 24081; `wf_method`'s range facts (near 24490) and `hvq` ghosts 24549–24700; `elem_key` 5474; `site_fields` 6809; `memory_fields` 9423; `has_range_atom` 22199 and `push_clauses` 22266; `atom` 7756–7792 for a member that is an array of aggregates; `cov_of`, `leaf_cover_read` | `call_atoms` 11148–11830 (the per-atom record holds one bound temporary and one row group per leaf path; the marked rows 11721, the flag rows 11764 and 11774); `range_returns` 15305–15463; `atom_returns`' sum terms (15115) and array parts 15179–15298; `writes_in_expr` 16357–16376 | `RangePlace` 16072 per leaf path; `range_places` 16158–16227; `range_row_amount` 16098; `range_marked_rows` 16123; the loop's place guards 18239–18281, entry rows 18853–18875, conjuncts 18877–18960 and back edge 19413–19437; `inv_range_chain` 4576, `inv_read_at` 4608, `inv_interval_frame` 4687, `inv_framing_any` 4724–4785; `apply_frame`, `apply_inv`, `apply_check` and the `fq_*` rows only as far as they must keep refusing (Part F) |

**Part D's table: the leftover sites** (`L`; **audit round 4 gives a verdict per site**).

| Heap (292) | Owners (293) | Markers (294) | Loads, partnerless, written (306) |
|---|---|---|---|
| `block_cells` 6016 with an array leaf's lines (its callers: `block_conj` 6099, `block_atom` 6175, `block_rows` 6325, `call` 12322, `contract_read` 9104, `loop_places` 16884 and 17208, `loop` 19001, `resolve_blocks` 20783, the `wblock_*` functions 20906–21193); `block_lens` 6039 and `block_pure` 6049 (the member lengths); `block_call_rows` 6347, `block_returns` 15658; the leak row 15498–15573; `obj_cells` at 17111, 17133 and 17278 (they drop array leaves, relying on question 292's refusal) | `owners_of` 2562, `has_owning_leaf` 2574, `leaf_owners_of` 2586, `owner_value` 2655, `owner_cell` 2680, `owner_text` 2686, `global_owners_of` 2704, `exempt_owned_fields` 15469, `named_leaf` in `owner_loop_sites` 20019–20374, `owner_invariants` 21517, `owner_rows` 21665, `owner_flag_rows` 21782: each walks cell leaves and passes over an array leaf | `marked_field_site` 5853, `marked_through_load` 5864, `block_marker` 5828, `block_mode` 6125, `site_marker` 7139, `owner_marked` 20375, `flags_kept_entry` 3337 and `flags_kept_row` 3388; the array-part guards of `atom` 7760, `call_atoms` 11179, `atom_returns` 15189 and `array_leaf_start_rows` 1533 for a marker | the array-part sites of `atom`, `call_atoms` and `atom_returns` with `via` (a loaded root bound to a `Ref` local); `contract_read` for `p->q->a[e]`; the same guards for an output amount (partnerless); `written_atoms` 17359–17468 and `resolve_written` 17472–17556 |

**Staging pins:** R2's. After each commit, what a later part lifts is still rejected by a pin that names its
part: after Commit 2, ranges over structures, global arrays and members of structures (Part C); **an element
structure with an array member, `v[i].a[j]`, from Part B until Part E** (depth 2), in `E` and in `check.cpp`;
matrices (Part E); the (Q) forms (Part F). A session that closes at a split point re-points the pins of the
parts it did not begin to "M4.5e1e2 (continuation)".

**Order of work.**
1. Step 0 and Step 1 (section 5).
2. **Commit 1:** record 0064 (draft, revised by section 6's tables), the `## M4.5e1e2` chapter with P83 and S48's
   first pass, the fixtures, R2's pins, the lint's first rules. Advisor checkpoint 1, then **audit round 1** on
   it. il-analyst-1 goes on with S48's second pass and ends when it is in the chapter.
3. **Part A** is built during Step 1 (it accepts nothing) and proved neutral; it joins Part B's commit.
   edg-backend may build its halves of Parts B and E's encoder rules that only reject while round 1 runs.
4. **Part B's lowering starts when audit round 1 has returned and the record holds its blocking fixes**
   (invariant 6). edg-backend and il-analyst-2 work in parallel against Part A's VIR; the test-mutator writes
   the tests from the record. **Audit round 2** on the snapshot of A and B. **Commit 2. Split point 1.**
5. **Part C** (il-analyst-2 may start from round 2's snapshot while the round runs, and applies its fixes).
   **Audit round 3. Commit 3. Split point 2.**
6. **Part D** (il-analyst-3). **Audit round 4. Commit 4. Split point 3.**
7. **Part E** (il-analyst-4). **Audit round 5. Commit 5. Split point 4.**
8. **Part F** (il-analyst-4). **Audit round 6**, with the pages. **Commit 6.**
9. **The close-out, whatever commit is the session's last:** the doc examples (test-mutator); the user pages
   and, from Part C on, the tutorial section (orchestrator, drafted during the waits); `internal/encoding.md`
   and the developer guide (edg-backend); the last audit's fixes in one batch; the preflight; the box check;
   record 0064 accepted **for the items built**, the others marked "decided, built by the continuation";
   status, handoff, `plan.md`'s Done row or its sessions row.

**Proofs, per part.**
- **Each half alone:** the il-analyst proves its half by the new VIR cases through `cerastes-vir-selftest` and
  Silicon (the new cases only), which need no encoder; edg-backend proves its half by its build and by
  `rejections.py --base` against section 10. Both run `scripts/format.sh`.
- **The joined proof,** once both halves have reported: edg-backend cuts the snapshot `refs/tmp/e1e2-j<n>`,
  ships it to the `work` lane, builds, runs `scripts/regen_vpr.py` into a directory of its own and
  `scripts/vprdiff.py` against the baseline (**no existing program's `.vpr` or `.map` differs**) and against the
  previous part's regeneration, compares the VIR self-test's output with the baseline's, and runs
  `rejections.py --base`; where the lint changed, the test-mutator runs it over both outputs with its
  self-test. Then, with builds held, the test-mutator runs **the part's new tests and the earlier tests whose
  Viper that list shows changed, once**, timed. A failure goes through the orchestrator to the half that owns
  the failing file; the joined proof is repeated once, after the last fix, for what that fix changed.
- The audit snapshot and the commit's tree are the tree the joined proof passed on.

**Audits** (a fresh Fable auditor per round, briefing 13.5; a snapshot commit, the local audit worktree and the
box's `audit` lane). The round's extra, for 13.5's last line:
- **Round 1, the record with P83 and S48:** reproduce the gates; **the trusted contracts** (`alloc_arr_<E>`,
  `zero_arr_<E>`, `program_start`'s row groups per leaf: unsatisfiable, a pair given twice, a fact that is not
  true of C); **the length facts** (a path on which they are assumed and never discharged; a slot they do not
  cover); **overlap by slots** (a cell, a structure atom or a range that points into an array of structures and
  escapes the sum rows, the exclusion, the framing disjuncts or `f__wf`'s facts; the form S48 (c) chose, with
  its alias twins); **Part A's table: is it complete** (any other site of `lower.cpp`, `check.cpp` or `vir.cpp`
  that would read an aggregate element as a cell); item 16 against the matrix; every row of section 1's table
  has a rejection.
- **Round 2, Parts A and B:** **reproduce S48's second pass** (rows (h) to (m), which ran after round 1) and
  the branch each took; Part A's neutrality and **a verdict per row of its table**; `v[i].x` (the bound
  before the member step; `v[N].x`); `p[i].x` (the null and liveness rows, once); allocation, zero fill and
  scope exit per leaf (a leaf missed, or given twice); initializers (elision accepted, an element written
  twice, the zero fill skipped, the array read in its own initializer); element copies (`v[i] = v[j]` with `i
  == j`; a source member without a value); loops (a store through an `int *` that keeps `v[k].x`'s values);
  the Sequencer's keys (two elements as one object, or one as two); `&v[i]` kept by a callee past the array's
  block; an element structure with an `owning` or marked member at any depth; item 16's exact set.
- **Round 3, Part C:** **the fan-out table, a verdict per site**; the alias traps; trap 3 (a callee's writes
  through a range over structures recorded by field); the amounts (the minimum over every leaf); `f__wf`'s
  ghosts (two leaves or two slots sharing one); an element read framed although its slot is outside the range;
  `program_start` per leaf (the zero rest over the wrong elements; linkage; both escape directions through a
  pointer leaf); a member that is an array of structures (its length facts in `struct_lens`); the value view
  against the object view (a leaf dropped or doubled; the limit).
- **Round 4, Part D:** **the leftover table, a verdict per site**; **the typed view's contract** for a
  structure with an array member (it is trusted, and its spike S48 (h) ran after round 1: unsatisfiable, a pair
  given twice, a fact not true of C); a heap structure's array leaf after `free`,
  and in the leak check; `calloc`'s zero over every element; that an owner site may pass over an array leaf
  only because arrays of owning pointers are rejected (check the rejection); a marked closure's kept-flag row
  across a replaced block; a load-rooted array leaf keyed by the old pointer; an output amount for a
  partnerless array leaf; a written atom whose amount is not the first slot's.
- **Round 5, Part E:** traps 1 and 2 (the address of `m[i][j]`; the second bound against the row; keys and
  ghosts with both indexes); a row's decay kept past its block; a row passed to a range callee that keeps one
  element; `Ptr(Array)` reaching any position but a parameter; an inner bound not read as written; a range over
  rows whose count the inner bound does not match; depth 3 as S48 (m) decided it.
- **Round 6, Part F, with the pages:** a member class whose trigger or framing names another leaf's place; a
  member array or a global array argument read without an atom that covers it; an application over a member
  array whose callee's `requires` is not discharged; the pages' and the tutorial's statements about what is
  checked and what is trusted. **When the session closes earlier, its last round's snapshot holds the pages.**
- **Blocking fixes** are re-audited by the next round, or by a short read of the fix's diff when it is the last
  round's. Should-fix items not taken go to handoff §3 with their reason.

**Pre-agreed cuts** (each leaves its pin, re-pointed to the session named, and the pages say what landed).
**A cut is taken only by its spike's table or by an audit finding with the advisor; time is never a reason
(question 310: a part that does not fit moves whole to the continuation).** In this order: (1) Part F's member
classes (S48 (j)); (2) a marker over a range of structures (M4.5e2, with question 287's filler); (3) global and
`static` arrays of aggregates (S48 (e); the continuation's first part); (4) an array member of aggregates (S48
(f); M8); (5) array-of-rows parameters (P83 (d), S48 (g); M8); (6) `owning(n)` over structures (M4.5e2); (7) a
written atom and a partnerless atom on a structure with an array member (S48 (k), (l); M4.5e2).
**Not cut without the owner:** local arrays of structures with element access, element copies and loops; ranges
over structures with calls, returns, loops and the alias traps; heap storage, an `owning` or marked member,
markers and heap-read places for a structure with an array member (questions 292–294 and 306: they have moved
once already); a local matrix with `m[i][j]`; question 304.

**Split points** (question 310). After each of Commits 2 to 5 the orchestrator reports `git diff --numstat
<base> -- src/ include/` without the self-test cases against section 1's sizes, the hours used, and what the
next part owes: its build at the rate measured so far, its audit round, the fixes, then the close-out (the
pages, the preflight, the 50-minute box check, status and handoff: about two hours). It takes the sum to the
advisor (checkpoint 3). **The next part starts unless that sum would pass twelve hours;** otherwise the
session closes at this commit with step 9, and the handoff names the part the continuation starts at. The
decision and its numbers go into the status entry; the owner is not asked. **The owner is asked** only if a
part already begun would pass the cap, or if the orchestrator and the advisor judge that its context will not
carry what is left of it. **The continuation** is started with the same command and "M4.5e1e2, from Part <X>"
(the closing session writes its branch and worktree names into handoff §2, since this session's are taken by
then: `m4.5e1e2b-<topic>` and `.claude/worktrees/m4.5e1e2b`, `build/m4.5e1e2b/`, lanes `~/cerastes-e1e2`):
it runs Step 0 again on the new base (worktree, lanes, baseline, R1 and R3; no P83, and only the S48 rows not
yet run), then goes on from that part's line in "Order of work", with the audit rounds numbered on.

## 9. Tests (`tests/memory/`, scalar-only ones in `tests/scalar/`, negatives in the area's `negative/`; `.c` in `c23, c++20`; each positive has a twin failing for one reason at one line)

**At most 190 test files in all** (M4.5e1e1 added 182 and six doc examples): one positive and one error twin per
rule of the record, one pin per rejected row of section 1's table; a rule that two shapes exercise the same way
gets one of them. The part's cap is in brackets. A test's expectation is pre-registered from the record before
its first run (question 160 for corrections, each listed in status).

- **Part B [35].** `array_struct_*`. A local array of structures: member writes and reads by index beside each
  other (twins: an index outside, `out-of-bounds`; an unwritten member, `uninitialized`); a nested structure's
  member `v[i].t.x`. Initializers: nested lists, a partial list with the zero fill, `{}`, C's `{0}`, a
  designated member inside an element, non-constant elements (twin: a `check` on a wrong value; pins: elision,
  an array designator). `struct S *p = v; p[i].x` (twin: a read after the array's block, `lifetime`). `&v[i]`
  passed to an `owns(*p)` callee and read back (twin: a callee that keeps the element, `lifetime` at the scope
  exit). `&v[i].x` stored and read back. Element values: `t = v[i]`, `v[i] = t`, `v[i] = v[j]` with `i == j`
  possible, `f(v[i])` by value (twins: a source member unwritten, `uninitialized`; a `check` on a wrong value).
  Loops: a sum over `v[i].x`, a fill (twins: `invariant-preserved`; item 10's listed false alarm as a pin named
  `…_false_alarm.c`). Sequencing: `v[0].x = v[1].x++` verifies, `v[i].x = v[j].x++` is `unsequenced`.
  `sizeof(v) / sizeof(v[0])`. C++ spellings in one `.cpp`. **Question 304:** the C++ rejection as a local, a
  global and a value; C's `{0}` over the same structures (positive, with a twin); C++'s `{0}` over a one-member
  aggregate; C's `{0}` over a first member that is a structure beginning with an array. Pins: an element
  structure with an `owning` member and one with a marked member (M4.8); `uninit` on an array of structures.
- **Part C [45].** `range_struct_*`, `global_array_struct_*`, `struct_member_array_struct_*`. `owns_range` over
  structures in a callee that writes `p[i].x`, `reads_range` in one that reads, each called with a local array
  (twins: a write under `reads_range`, `permission`; `n` past the array; a caller that does not hold the array).
  Element reads in a `requires`, an `ensures` with `old`, a `decreases` (twins: `postcondition`; unframed,
  `permission`). **The alias traps:** `f(v, 3, &v[1].x)` against `owns_range(p, n) && owns(*q)`, `f(v, 3,
  &v[1])` against a structure atom, two overlapping ranges (each `permission`; each positive passes another
  object or a disjoint half). A loop over a parameter's range that sums and one that fills (twins). A call that
  forwards `(p, n)`. `owning(n)` on a parameter of structures. A marker over a range of structures (a filler
  of two elements; twin: one member left unwritten, `uninitialized`). Globals: `struct S g[3] = {{1, 2}, {3}}`
  with its listed and zero values, no initializer with internal linkage, the unknown forms (twins as M4.5e1e1's
  global arrays have them), `owns(g)` and `reads(g)` callees from `main`, a loop in `main`, a `static` local
  array of structures through a caller, a pointer leaf and its escape twin in a `.cpp`. An array member of
  structures: `t.v[i].x` in a local and under `owns(*p)`, `owns(p->v)` alone, its nested initializer, `f(&t,
  &t.v[1])` (`permission`), a structure global with one, a whole-structure copy within the by-value limit and
  the limit's pin. Pins: `restrict` with a range over structures (M4.8); `owns_range(&v[1], n)`, `owns(p[i])`,
  `owns(v[i].x)`, `&p[i]` in a contract, a range atom in a loop invariant, a range over a result (M4.5e2).
- **Part D [35].** `struct_array_heap_*`, `struct_array_owner_*`, `struct_array_marked_*`,
  `struct_array_chain_*`. `malloc` and `calloc` of a structure with an array member, elements written and read,
  `free` (twins: a read of a `malloc`ed element before a write, `uninitialized`; a use after `free`; a second
  `free`, `invalid-free`; a `leak`). `owns_block(p)` in a constructor's and a destructor's contract; `owning`
  on a parameter, a result, a local and a member that point at such a structure (twin each: a leak or a use
  after the callee took it). An `owning` member beside an array member: allocate, use both, free (twins: the
  owned block leaked; the array element read through a freed owner's sibling is not affected, as a positive). A
  marked member beside an array member. `ref_to_uninit` on a pointer to such a structure with a filler that
  writes every element (twins: the caller reads first, `uninitialized`; a `must_init` filler that misses one
  element). Heap-read places: `owns(*p->q)` and `p->q->a[i]` in a contract (twin each). A partnerless
  `ensures(r, owns(*r))` of such a structure (twin). A written `owns(*p)` in a loop invariant (twin). One
  `--strict-init` and one `--strict-owner` program over such a structure (`// verify-options:`).
- **Part E [35].** `matrix_*`, `array_rows_*`. A local matrix: `m[i][j]` written and read (twins: the first index
  outside, the second index outside, an unwritten element). Initializers: nested, partial, `{}`, C's `{0}`
  (twin; pins: `{1, 2, 3, 4, 5, 6}`, a designator). A nested fill loop and a nested sum (twins). A row `m[i]`
  passed to an `owns_range(q, 3)` and a `reads_range(q, 3)` callee and read back (twins). **An array-of-rows
  parameter:** `int a[][3]` with `owns_range(a, n)` in a callee that writes `a[i][j]`, called with `m` (twins: a
  caller holding fewer rows; a write under `reads_range`); the spellings `int a[2][3]` and `int (*a)[3]`.
  `sizeof(m)`, `sizeof(m[0])`, `sizeof(int[2][3])`, a row typedef, a `static_assert` (twins: a `check` on the
  wrong value; a wrapped inner bound, `overflow` at the bound). A global matrix with initial values and a
  `static` local one (twins). A matrix member `u.m[i][j]`, in a local and under `owns(*p)`. `v[i].a[j]` over an
  array of structures with an array member (twins: each index outside). `&m[i][j]` stored and read back.
  Sequencing between two elements. Depth 3: a positive and a twin if S48 (m) admits it, a pin otherwise. Pins:
  `int (*p)[3] = m;` and `&m[i]` (M8; `array_reject_pointer_to_array.c` stays as it is), `sizeof(a)` of the
  parameter (`sizeof_array_argument`), `owns_range(m[i], 3)` in a contract (M4.5e2).
- **Part F [25].** `forall_struct_*`, `forall_member_*`. `cerastes_forall` over `p[i].x` in the `requires` and
  `ensures` of a fill over structures, with its loop invariant (twins: `invariant-preserved`, `postcondition`);
  in a `check` over a local array of structures; a body that compares two members (`p[i].x <= p[i].y`; twin). A
  member array as the pointer argument: `cerastes_forall(f, p->a, 4)` in a contract under `owns(*p)`, and
  `(f, s.a, 4)` in a loop invariant and a `check` (twins: `precondition`; unframed, `permission`). A global
  array in a contract under `reads(g)` (twin). `pure` applications `sum(p->a, 4)` in an `ensures` and `first(g,
  4)` in a `requires` (twins). Pins: `p[i].a[j]` and `m[i][j]` under a binder, a row `m[r]` as the argument,
  `p->q->a` (M4.5e2).
- **VIR self-test:** the session's cases in `src/viper/selftest_cases35.inc`; `VIR_CASES_MIN` raised.
- **Byte identity:** `scripts/vprdiff.py` against the baseline after each part: no existing program's `.vpr` or
  `.map` changes in this session.

## 10. Flips (pre-approved with this plan; any other changed outcome goes to the owner)

The planning's survey read every test that names M4.5e1e2: 11 files, 36 expect rows, all `// mode: c23, c++20`.
A flip is made by the part named, on the program as it stands. **In a file with several rows, a lifted row
keeps its line number with a comment in place of its lost expect line** (M4.4c's rule), and the lifted construct
gets a positive of its own with an error twin.

- **A. Retired for a positive of the same program, with an error twin** (all in `tests/memory/negative/` unless
  a path is given):
  - Part B: `array_reject_struct_elements.c` (L14); `array_reject_struct_subscript.c` (L16).
  - Part C: `range_reject_struct_pointee.c` (L14); `struct_array_reject_members.c` L36 (a member `struct P
    v[2]`).
  - Part D: `struct_array_reject_expressions.c` L23 (`malloc`); `struct_array_reject_members.c` L21 (an `owning`
    member beside an array member); `struct_array_reject_annotations.c` L34 and L37 (partnerless), L39
    (`ref_to_uninit`), L44 (`owning` on a pointer to such a structure), L54 (a written atom), L81 and L87
    (heap-read places).
  - Part E: `array_reject_array_of_arrays.c` (L10); `array_param_reject_array_of_arrays.c` (both rows of L9);
    `tests/scalar/negative/reject_sizeof_2d_array.c` (L9); `struct_array_reject_members.c` L41 (a member `int
    m[2][3]`).
  - Part F: `global_array_reject_expressions.c` L52 and L57; `struct_array_reject_annotations.c` L62 and L75.
  - Every staging pin R2 adds, by the part that lifts it.
- **B. Re-expected from a rejection to an error:** none. If R1 finds an existing pin that item 13's rule for
  computed types would turn from a rejection into an error (a wrapped inner bound in a type operand), **it goes
  to the owner before it is touched** (invariant 1).
- **C. Stay rejected; only the message or the `Pins:` comment changes:** `struct_array_reject_result_decay.c`
  (it names "M4.5e1e2/M4.5f3" and becomes M4.5f3's alone: a pointer into a structure value's array);
  `array_param_reject_array_of_arrays.c`'s comment "naming e1e" and
  `tests/scalar/negative/reject_typedef_function_array_param.c`'s "e1e" (M4.7); the rows whose comment says "to
  confirm at the joined run"; the pins of the messages in `src/` that say "M4.5e1e" (`check.cpp` 1587, 3617,
  4095), each lifted or re-pointed by the part that owns its construct; a row whose spike table says "stays
  rejected" is re-pointed to the session that table names.
- **D. Unchanged:** the 13 rows of the four multi-row files that belong to other sessions
  (`global_array_reject_expressions.c` L31, L37, L41, L47; `struct_array_reject_annotations.c` L68;
  `struct_array_reject_expressions.c` L18, L24, L25; `struct_array_reject_members.c` L26, L31, L46, L51, L56);
  `array_reject_pointer_to_array.c`, `reject_sizeof_pointer_to_array.c` and every pin that names M4.5e2,
  M4.5f1, M4.5f3, M4.6, M4.7, M4.8 or M8. **If R1 or a part's `rejections.py` run shows one of them changed,
  it goes to the owner.**
- **E. Lint self-tests** that pin "this shape is refused" for a shape item 17 now admits (a second binder, a
  receiver with a member step after an index step, an `alloc_arr_*` for a structure): the test-mutator reports
  which before editing, and status lists them.
- **F. The VIR self-test and the front end:** `src/viper/selftest_cases28.inc` 21 expects a rejection text that
  holds "structures: M4.5e1e"; the il-analyst of Part C re-expects that case with the lift and status lists it.
  Front-end tests and goldens: none (**a golden changing goes to the owner**). No tag loses its last test.
- **G. Question 304** (the owner, 2026-11-25; question 312). The rejection is new behaviour in C++ and **no
  existing test or doc example changes outcome** (section 4). Part B adds its pins and positives (section 9).
  **So this plan lists no flip for it. If R1, R3 or Part B's `rejections.py` run shows an existing file whose
  outcome it changes, that file goes to the owner before it is touched** (invariant 1; handoff §4.2: a session
  re-expects exactly the pins its plan lists), with the recommended flip: C-only, or full braces, whichever
  keeps what the file tests. Part B's other work goes on meanwhile.

## 11. Docs (invariant 8; every example is a test)

**A session that closes at a split point writes the pages for what it landed**; the tutorial section is written
by the session that lands Part C, and extended by the one that lands Part E.

- **`memory.adoc`.** "Structures" (355): `#array-members` (473) gains members whose elements are structures or
  arrays; 530–537 and 539–540 lose what Part D and Part C lift. "Arrays" (553): a subsection "Arrays of
  structures" (the layout in a sentence, `v[i].x`, initializers, elements as values, `&v[i]`) and one "Arrays
  of arrays" (rows, `m[i][j]`, a row as a pointer); **802–810 rewritten** (stale since M4.5e1e1). "Ranges"
  (625): ranges over structures, what `owns_range(p, n)` gives per member, the alias traps. "Array parameters"
  (717, 730–731): array-of-rows parameters. "Facts about every element" (733, 813–815): members under the
  binder, member and global arrays as arguments. "Global variables" (864, `#global-arrays` 923, 951–956). "The
  heap" (1230): a structure with an array member on the heap. "Owning fields" (1595): beside an array member.
  "What is not supported yet" (1938, 1957–1969): section 1's table rows a user can meet, each with its session.
- **`verifying.adoc`** 1242–1245 and 1283 (which arrays and array types are accepted; question 304's rejection
  with the other `-Wmissing-braces` forms). **`overview.adoc`** 101. **`annotations.adoc`:** `owns_range` and
  `reads_range` over structures and rows; 1039–1041 (the pointer argument of a quantifier).
- **`error-messages.adoc`:** the forms `out-of-bounds` (the second index), `permission`, `uninitialized`,
  `lifetime`, `invalid-free` and `leak` gain.
- **Examples** (`doc/modules/ROOT/examples/memory/`): `array_of_structs.c` and `array_of_structs_alias_error.c`
  (Part C), `struct_array_heap.c` and its error variant (Part D), `matrix.c` and `matrix_index_error.c` (Part
  E), `forall_struct.c` (Part F); the existing examples unchanged.
- **The tutorial** (question 298): a section **"Arrays in aggregates"** in `tutorial/arrays.adoc`, before "What
  is next" (189): an array of points filled and summed through a range contract, the alias trap as its failing
  variant, and a matrix once Part E has landed; `console` blocks copied from runs in
  `doc/modules/ROOT/examples/tutorial/` (`aggregates_points.c`, `aggregates_points_alias.c`,
  `aggregates_matrix.c`); "What is next" loses what landed.
- **`internal/encoding.md`:** §2 (leaf paths, the array types, `Ptr(Array)` parameters), §3 "Arrays of
  structures and arrays of arrays (M4.5e1e2)" after 1207–1249, §6, §8; 1087's "e1e". **The developer guide**
  where it names the leaf enumeration and the trusted methods. One amendment line in each amended record.
  `tests/README.md` only if a rule of the harness changes.

## 12. Questions, stops, acceptance

**Every question goes to the advisor first** (owner, 2026-11-25: "If you or the subagents have questions first
consult the Fable advisor, and then if you still need an answer or guidance you may stop and ask me"). An agent's
question goes to the orchestrator; the orchestrator states it to the advisor with the facts and a recommended
answer, saying first whether the matter is fail-closed or unsound; the agent goes on with what the question does
not block.
- **The advisor settles:** a choice inside the record's stated rules; a choice between fail-closed alternatives
  (a rejection with a pin, a listed false alarm); test mechanics, message wording and a question-160 correction;
  a table's "the advisor" row; a cut on section 8's list and a split-point decision under its rules; whether a
  slow test is restructured or moved under question 285's rule.
- **Goes to the owner, and the work it blocks waits:** an `expect:` change outside section 10 and question 160
  (invariant 1); **any existing program's Viper or map changing**; a golden changing; a cut that section 8
  protects or that is not on its list; a soundness finding that has no fail-closed fix (a wrong program
  verified, a trusted contract the auditor does not accept); a protected spike row failing in every form; a
  part already begun that would pass the twelve-hour cap; anything the advisor says is the owner's.
- **Each answer the advisor settles is logged** in `internal/handoff.md` §8 under "Settled with the advisor in
  M4.5e1e2, for the owner's review": numbered from 320, the soundness statement first, the answer taken, the
  alternative. The status entry lists them.
- **While the owner is away,** a question that is the owner's waits; the session goes on with everything it does
  not block. The overnight rule (build the fail-closed recommendation and log it) applies only if the owner
  gives it for this session.

**Other stops:** an exit 3 or a time over 120 s not settled within its part (section 3 rule 3); a second
failure of a joined proof on the same cause.

**Acceptance.**
- The preflight of section 3, once, after the last audit's fixes. Advisor checkpoint 5.
- `CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` **once**, after the last code change, on the x86-64 box
  (the branch as a git bundle into `~/cerastes-s8/p-ubuntu-26.04`): green. It is not skipped; if the owner
  ends the session before it, status says "not run" in its first line.
- Record 0064 accepted, with its table of the branches P83 and S48 took.
- The status entry (sizes measured against section 1, the hours per step against section 3's target, the
  deviations, the corrections under question 160, each cut and split taken, each advisor-settled question) and
  the handoff with its Questions section and what the session leaves (section 15). **If the session closed at a
  split point,** the handoff's §1 and §2 name the part the next session starts at, and that session is started
  with the same command and "M4.5e1e2, from Part <X>". Commits stay local.

## 13. Briefings (the orchestrator sends these; `<n>` is the report's number, `<k>` the instance's)

A briefing is all an agent knows. Each one below is sent as written, with the bracketed parts filled in; a later
task goes to the same agent by SendMessage in the short form of 13.7. A fresh instance of a role gets the role's
briefing again with "You continue from `build/m4.5e1e2/<role>-state.md`; read it first" and the last commit's
hash in place of the first task.

**13.0 The common block** (the first paragraphs of every briefing):

```
You are one agent in an execution session of cerastes, a C/C++ verifier (EDG front end -> VIR -> Viper/Silicon
-> Z3). The session is M4.5e1e2: arrays whose elements are structures or arrays, and six rules that the last
session left for structures with an array member.

Work only in the worktree .claude/worktrees/m4.5e1e2 (branch m4.5e1e2-arrays-of-structures); name it in every
command, because a shell `cd` can leave it. The session's plan is
.claude/plans/m4-5e1e2-arrays-of-structures.md and its decision record is
internal/decisions/0064-arrays-of-structures-and-arrays-of-arrays.md (until the record is committed, the plan's
section 7 is its text). Read CLAUDE.md, the plan sections and the record items named below before you change
anything.

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
- Build on the Mac through `lockf build/m4.5e1e2/build.lock cmake --build build/cmake`, and in your lane with
  scripts/box_lane.sh. Only edg-backend builds the `work` lane, and nothing builds there during a run.
- scripts/run_tests.py has one holder at a time. Unless you are told you hold it, run single files with
  `build/cerastes verify <file> --keep-vpr`.
- Keep build/m4.5e1e2/<role>-state.md current: what is done, what is in progress, which files you hold, the
  exact next step. A fresh instance may have to continue from that file alone.
- Write every report to build/m4.5e1e2/<role>-report-<n>.md and return it too. Report what happened: a check
  that did not run is "not run", and a failure comes with its output.
- A script that writes tests lives in build/m4.5e1e2/ and is never run again once a file it wrote is in the
  tree.
- Tool code follows CLAUDE.md's style (scripts/format.sh; brace initialization, `U` on unsigned literals, one
  declarator per declaration, no comment on a line with a brace).
- Never edit an `expect:` line of an existing test, except the Flips of the plan's section 10.
- Do not commit; the orchestrator cuts every commit. No Claude attribution on anything.
- If a permission is denied, stop that action and report it.
```

**13.1 edg-backend** (first task: Step 0, then Part A):

```
[13.0]
Your role is edg-backend. Yours: src/backend/**, src/driver/**, the build system, the five box lanes and the
baseline, the joined build of each part with scripts/regen_vpr.py and scripts/vprdiff.py,
internal/encoding.md and the developer guide's paragraph. In Part A only, src/vir/** and src/viper/** are
yours too. Never yours: tests/, scripts/lint_viper.py, and src/vir and src/viper after Part A.
Your lane is ~/cerastes-e1e2/edg. State file: build/m4.5e1e2/edg-backend-state.md.

Read: the plan's sections 1 to 5 and 8, and section 7's items 2 to 5; record 0063, items 3 to 5 and 15.

Task 1, Step 0 (plan section 5, item 2). Set up the lanes base, il, edg, work and audit under
~/cerastes-e1e2 with scripts/box_lane.sh (set its two variables for this session; if ~/cerastes-e1e1/work is
still there with the base's tree for src/, include/, tests/ and scripts/, copy from it). Time one
ship-build-run round trip. Take the baseline in the `base` lane before any change to src/:
scripts/regen_vpr.py and the VIR self-test's output into build/e1e2-base/. Keep frozen binaries in `work`.
Report 1: the lanes, the round-trip time, the baseline's counts.

Task 2, Part A (plan section 8, row A; record 0064 item 4). It is behaviour-neutral: nothing new is accepted.
Proof: your build; scripts/vprdiff.py shows every .vpr and .map identical to the baseline; the VIR self-test's
output is identical for every existing case (your new raw-lowering and rejection cases are added to it).
Report 2: what you added, the proof's output, and every walk of leaves you left guarded
with its file and line, for audit round 2.
```

**13.2 il-analyst-1** (P83 and S48):

```
[13.0]
Your role is il-analyst, the first of this session. Yours: the probes under build/m4.5e1e2/p83/, the chapter
"## M4.5e1e2" of internal/il-inventory.md, and the fixtures tests/viper-smoke/memory/s48_*.vpr. Never yours:
src/. Your lane is ~/cerastes-e1e2/il; for S48 you hold scripts/run_tests.py in the `work` lane, one fixture
per run, until your second pass is in the chapter. State file: build/m4.5e1e2/il-analyst-state.md.

Read: the plan's sections 1, 4, 6 and 7; record 0063's items 3 to 7, 9, 10, 13 and 15; in
internal/il-inventory.md the chapter "## M4.5e1e1" (P82's readers, S47's method, and "S47 (x)", whose
fixtures s47_x1_* and s47_x2_* are your starting point).

Task. Run P83's rows (a) to (g) and then S48, as the plan's section 6 lays them out. Each row has a table:
"Expected", and what is built if not. Your job is to say which branch each row took, with evidence, and never
to round up: a claim about the IL cites the probe file and line with an excerpt you produced; a spike's
verdict is the run's output and its wall time on a quiet lane. A twin must fail with exactly one error at its
registered line. Hand-written Viper is written the way lowering will write it (take the shapes from the kept
Viper of the frozen build), and every difference between a fixture and what lowering writes is listed.
An outcome that no table names, a second Silicon error after a marked row, qp.not.injective, or a native error
in a positive: stop that row and send it to the orchestrator.

Two passes. Pass 1: P83, then S48 rows (a) to (g) and (n); report 1 when they are in the chapter. Pass 2: S48
rows (h) to (m); report 2. Each report has one table: row, fixtures with times, outcome, branch taken, the
record item it revises. Stay under the caps in the brackets of section 6 (56 fixtures in all).
```

**13.3 il-analyst-`<k>`** (the lowering half of Part `<X>`; `<k>` = 2 for B and C, 3 for D, 4 for E and F):

```
[13.0]
Your role is il-analyst, for the VIR and lowering half of Part <X>. Yours: src/vir/** (vir.h, vir.cpp,
check.cpp, lower.cpp, lowered.*), src/viper/**, and the session's self-test cases in
src/viper/selftest_cases35.inc. Never yours: src/backend, tests/. You are the only holder of those files
until your part's audit snapshot is cut. Your lane is ~/cerastes-e1e2/il. State file:
build/m4.5e1e2/il-analyst-state.md (your predecessor's notes are in it; read them first).

Read: the plan's sections 1, 3, 4, 7 and 8 (Part <X>'s row and its site table); record 0064, all of it;
record 0063, all of it; the chapter "## M4.5e1e2" of internal/il-inventory.md (the branch each spike took
decides what you build); [the audit report whose fixes you apply first, if one is named below].

Task. Build Part <X>'s lowering and checks as the record's items [<items>] state them. Three things carry the
soundness of this part, so say in your report how each is met:
1. No existing program's Viper may change. A function you touch must write the same text as before for a
   program without the new constructs; edg-backend's vprdiff will show it.
2. Every walk of a structure's or an array's leaves either handles the general leaf path or stops with an
   internal error on a path it does not handle. A leaf must never be read as a cell of another shape.
3. Every obligation is an explicit row in source terms before the access or call it guards (record 0028 item
   11), and every row that pairs two sites is by slots, in the form S48 (c) chose.
check.cpp refuses whatever this part does not lift, and refuses it before lowering can see it.
Proof of your half: the new VIR cases through cerastes-vir-selftest and Silicon in your lane, the new or
changed cases only, by name; scripts/format.sh. Report: the functions you changed, each site of the plan's
table with what it does now, the cases and their times, and anything you left as a listed limit.
```

**13.4 test-mutator** (first task: Step 0; then the lint; then each part's tests):

```
[13.0]
Your role is test-mutator. Yours: tests/** (new tests, twins, pins and exactly the Flips of the plan's section
10), tests/mutants-equivalent.txt, the compiler matrix, scripts/run_tests.py, scripts/lint_viper.py,
scripts/rejections.py with their self-tests, and the example files under doc/modules/ROOT/examples/. Never
yours: src/. You hold scripts/run_tests.py in the `work` lane (~/cerastes-e1e2/work), except while
il-analyst-1 runs S48's fixtures. State file: build/m4.5e1e2/test-mutator-state.md.

Read: the plan's sections 1, 3, 5, 9 and 10, and section 7's items 16 and 17; tests/README.md; record 0063
items 6 and 15.

Task 1, Step 0 on the frozen build (plan section 5, item 3): R1, R2 with its staging pins, R3, and the
compiler matrix M of P83 (h), which includes question 304's forms. Report 1: each run's result against what
the plan expects, the staging pins you added, the matrix as a table, and anything that differs from the
plan's section 4.

Task 2, the lint (record 0064 item 17), against S48's fixtures as they arrive: each new rule with a refusing
twin in scripts/test_lint_viper.py. Before you edit a self-test that pins "this shape is refused" for a shape
the record now admits, report which (Flips E).

Then, for each part, on the orchestrator's message: write the part's tests from the record (plan section 9),
each expectation registered before the first run; make the part's Flips; after the joined proof, with builds
held, run the part's new tests once, timed. A new test that fails for its designed reason at another line or a
neighbouring category may be corrected once (question 160); list every correction. A twin has one failing row
per path: after a failed check Silicon goes on with that value unconstrained, so give a twin an `ensures` that
holds for any value.
```

**13.5 soundness-auditor, round `<r>`** (a fresh instance each round):

```
You audit one snapshot of cerastes, a C/C++ -> Viper verifier, for unsoundness, in the session M4.5e1e2.
You are read-only: never edit, write or commit. Your report is returned as text; the orchestrator saves it.

The snapshot is the commit <hash>, checked out in .claude/worktrees/m4.5e1e2-audit, with its build in the
box lane ~/cerastes-e1e2/audit (run probes there; stop any probe that runs past ten minutes and report it as a
timeout). The diff under audit is `git diff <previous snapshot or base> <hash> -- src/ scripts/lint_viper.py
tests/ internal/decisions/0064-*.md`.

Read first: .claude/plans/m4-5e1e2-arrays-of-structures.md sections 1, 7 and 8 (the audit list for round <r>
is there, and it is your checklist); the record 0064; record 0063; the chapter "## M4.5e1e2" of
internal/il-inventory.md; and the earlier audit reports build/m4.5e1e2/auditor-report-*.md in the session
worktree (do not repeat a finding that was fixed; do check that each blocking fix holds).

What counts most, in this order: a program that is wrong and verifies; a trusted contract (alloc_arr_<E>,
zero_arr_<E>, program_start, a typed view) that can be unsatisfiable, gives a (receiver, field) pair twice, or
tells a caller something that is not true of C; a leaf read as a cell of another shape; a length fact that is
assumed on some path and never discharged; a pair of sites that can overlap and escapes the sum rows, the
exclusion, the framing disjuncts or f__wf's facts; a construct accepted that the record rejects; a rule with
no must-fail test. Write probe programs for what you suspect, and say for each claim whether you reproduced it.

Report: a numbered list with file:line and severity (blocking / should fix / note), then the list of claims
you reproduced and those you could not. For round <r> also: [the round's extra, from section 8].
```

**13.6 The advisor** (each question):

```
Session M4.5e1e2 of cerastes. Question <number>, from <who>.
Fail-closed or unsound: <one sentence>.
Facts: <what was run or read, with paths>.
The plan's rule that applies: <section and row, or "none">.
Recommended answer: <...>. Alternative: <...>.
What it blocks until answered: <...>.
```

**13.7 A later task, by SendMessage** (to edg-backend and the test-mutator between parts):

```
Part <X>, your half. Read the plan's section 8 row <X> again and record 0064 items <items> (the record changed
since your last task: <what>). Gates to lift: section 4's list for <(S) | (A) | (Q) | (H)>. Hold builds from
<time> for the joined run. Report to build/m4.5e1e2/<role>-report-<n>.md: what you changed, your proof
(<the build and rejections.py --base against section 10 | the part's tests, once, timed>), and what you left.
```

## 14. Inputs (moved here from `internal/handoff.md` §3 and from section 14 of M4.5e1e1's plan at this planning)

- **Decided in M4.5a** (0028 items 2 and 16; S26): the layout of section 7 item 3; over an array of structures
  one quantifier per member and field, each with its own receiver as its trigger (one quantifier holding two
  members' permissions fails Silicon's injectivity check); `a[0][3]` and `a[2][0]` fail their bounds.
- **S47 (x)** (`internal/il-inventory.md`, chapter M4.5e1e1): (x1) `owns_range(p, n)` over `struct P { int x;
  int y; }` with one quantifier per leaf and permission, a length-facts quantifier with the trigger
  `loc(block_of(p), k)`, e1b's loop conjuncts per leaf, an element read as bound, permission and flag rows, and a
  caller with a local `struct P v[3]`: 9.2–9.9 s, the twin one error; (x2) `int m[2][3]` with each permission
  one quantifier over two bound variables, nested loops each carrying the two-variable place and its own
  liveness snapshot, the access as two bounds then the cell's rows: 8.7–8.9 s. No `qp.not.injective`.
- **From M4.5e1e1** (record 0063): the three leaf views and the poison rule (item 4); the bound local (item 5;
  S47's F1: a quantified chunk is matched by its pattern, not by the receiver's spelling); the pairing rows over
  slot terms, never a bare block comparison (item 7); S47's F2 (a guard `d != s` between two named objects is
  not decided from quantified permissions: write it only where a place is reached through a pointer or an
  index), F3 (after a failed `assert` Silicon assumes it: one failing row per path in a twin) and F5 (one
  liveness conjunct per root); the by-value limit of 7 (question 309: about 1.4 times per element); the lint's
  bound-local and block grammar (item 15); its listed false alarms (items 7, 10 and 13).
- **Process facts from M4.5e1e1:** every role needed four instances, each started from its predecessor's state
  file; Commits 2 and 3 were composed from proved snapshots (`refs/tmp/e1e1-*`) while the next part was built in
  the same worktree. **Both are this plan's method:** a commit's tree is the tree its joined proof passed on,
  taken from the snapshot ref (`refs/tmp/e1e2-j<n>`), whatever the worktree holds by then.
- **What the handoff held for this session** (its §3, "From M4.5e1e1"): the six leftovers (section 1's (H)); an
  array member or a global array in a contract as a quantifier's or a `pure` function's argument, and structure
  members under a binder (section 1's (Q)). From M4.5e1b's notes: "ranges whose base is not a pointer
  parameter" stay M4.5e2's; a range **over** structures on a pointer parameter is here.
- **M4.5e2 needs this session only for `malloc(n * sizeof(struct S))`**; the rest of M4.5e2, and M4.5e1f, can
  run before or after.
- **Lessons that bind here:** an allow-list is never computed by `grep` before a build; the audit worktree needs
  no bootstrap (a source-only `git worktree add --detach` plus the lane's build of exactly that source); proofs
  made while another agent builds are provisional; a twin that runs further than its positive needs an `ensures`
  that holds at the wider state; before a question about a lowering change, run the end-to-end vprdiff and put
  every changed VIR case through Silicon; `tail -n 3`, not `tail -3`, on the box; containers are started with
  `--init`; a `pkill` inside `docker exec bash -c` kills its own shell (`docker restart` stops a lane cleanly).

## 15. What the session leaves, and the questions

- **For handoff §3 at the session's end:** M4.5e2 takes ranges on part of an array of aggregates, atoms on one
  element, `malloc(n * sizeof(struct S))`, a second index or a row under a binder, and the re-argument of the
  `offset_of == 0` backstops (unchanged here: no owner gains an interior pointer); M4.5e1f takes the first-slot
  amount of each leaf path with question 238; M4.5f3 takes structure values and copies past the by-value limit;
  M4.8 takes element structures with `owning` or marked members, arrays of owning pointers, `restrict` with a
  range over structures and item 10's reach rule; M8 takes pointers to arrays outside parameters, depth 3 or
  more unless admitted, and designators. A cut or a "stays rejected" branch adds its row.
- **Answered by the owner at this planning (2026-11-25):**
  - **310** How sessions run from this one plan: "12-hour rule (Recommended)" — the order arrays of structures
    (code, then contracts and storage) → the leftovers → arrays of arrays → quantifiers; the next part starts
    unless the remaining work would pass twelve hours; parts not begun run from this plan in a second session.
    The alternatives were two sessions with a fixed stop after the leftovers, and the leftovers last.
  - **311** Whether the full check ran on M4.5e1e1: "Yes, it was green".
  - **312** Where question 304's rejection is built: "In M4.5e1e2 (Recommended)". Question 304 itself, inline:
    "Follow the rejection like the compiler warning."
- **Defaults this plan builds on, for the owner's review** (each fail closed: a rejection with a diagnostic and
  a pin, or a listed false alarm, never a wrong proof; answer in `internal/handoff.md` §8 before the session, or
  leave them):
  - **313** An array or a range whose element structure has an `owning` or marked member is rejected (M4.8, with
    arrays of owning pointers). Alternative: quantified closures over owning fields now (a part of its own).
  - **314** No leaf path is deeper than two index steps; depth 3 is admitted only if S48 (m) verifies it under
    30 s a program (M8 otherwise). Alternative: depth 2 fixed, without the spike.
  - **315** A pointer to an array is a parameter's type only (`int a[][3]`, `int (*a)[3]`); as a local, a
    member, a result or under `&` it stays rejected (M8). Alternative: pointer-to-array locals now.
  - **316** `restrict` backed by a range over structures is rejected (M4.8). Alternative: 0043 item 10's
    argument extended to ranges now.
  - **317** A whole-structure copy of a structure whose member is an array of structures is written leaf by
    leaf under the by-value limit of 7 and rejected beyond it (M4.5f3); no trusted `copy_arr_<E>` is added.
    Alternative: a trusted copy per element structure.
  - **318** In a loop, a store to `v[i].x` of a local array or through a pointer reaches every place of that
    field (0043 item 11's rule): the automatic "values kept" conjunct is lost for `v[k].y` of the same field, a
    listed false alarm with a written invariant as the way out (M4.8). Alternative: a per-leaf rule by name now.
  - **319** The tutorial section "Arrays in aggregates" is written by the session that lands Part C and
    extended by the one that lands Part E.
- **Open questions: none.** A question that arises in the session goes to the advisor first (section 12); the
  session's questions start at 320.
