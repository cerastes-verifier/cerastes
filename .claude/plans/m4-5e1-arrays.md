# Plan: M4.5e1 — Arrays (seven sub-sessions, records from 0059)

**Approved by the owner on 2026-11-11**, at a session that only planned (question 224). Its commit also split
M4.5e1 into five sub-sessions (e1a–e1e); e1c's planning (2026-11-23) added a sixth, e1f.

**Trimmed at M4.5e1b's planning (2026-11-11, owner's choice) to what e1c–e1e still need.** e1a is done (record
0059, `internal/status.md`), and e1b has a plan of its own, `.claude/plans/m4-5e1b-ranges-in-contracts.md`. **The
text before the trim is `git show dc40377:.claude/plans/m4-5e1-arrays.md`**: its section 3 is e1a's approved plan
(scope, survey facts, P78 and S43, the record's draft, parts, tests, flips, docs, acceptance), its section 4 e1b's
sketch, its section 8 questions 229–233 as asked. Section numbers are kept, so older references still resolve.

**Trimmed again at M4.5e1c's planning (2026-11-23).** e1b is done (record 0060), and e1c has a plan of its own,
`.claude/plans/m4-5e1c-quantifiers.md`, planned in full as e1b was. **The text before this trim is `git show
bdf3581:.claude/plans/m4-5e1-arrays.md`**: its section 5 is e1c's sketch (decided, proposed, P80 and S45, tests).
The same planning added a sixth sub-session, **e1f** (section 10).

**Trimmed a third time at M4.5e1d's planning (2026-11-24).** e1c is done (record 0061), and e1d has a plan of its
own, `.claude/plans/m4-5e1d-pure-functions.md`, planned in full as e1b and e1c were. **The text before this trim
is `git show 9ab9a1e:.claude/plans/m4-5e1-arrays.md`**: its section 6 is e1d's sketch (proposed, P81 and S46,
tests and docs).

**Trimmed a fourth time at M4.5e1e's planning (2026-11-25).** e1d is done (record 0062). e1e does not fit one
session and is two (question 290): **e1e1** has a plan of its own, `.claude/plans/m4-5e1e1-arrays-of-scalars.md`,
planned in full as e1b, e1c and e1d were, and **e1e2** was planned in full after it (2026-11-25):
`.claude/plans/m4-5e1e2-arrays-of-structures.md`, sized at 4,700–8,450 lines and run under the 12-hour rule (question 310). **The
text before this trim is `git show 68384d1:.claude/plans/m4-5e1-arrays.md`**: its section 7 is e1e's sketch
(decided and proposed, P82 and S47, the known pins) and its section 9 e1e's inputs.

What is left here is the umbrella: the owner's decisions for all of M4.5e1, the table of sub-sessions, the rules
they share, and e1f's section (10), the one sub-session that has no plan of its own yet.

## 1. Context

**Why.** M4.5e1 is the arrays session of the C memory model (records 0028 items 1, 2, 11, 12 and 16; 0029 items 1,
3 and 4). Its planning (question 224) found it does not fit one context.

**Where it stands.** e1a built local arrays as blocks, `a[i]`, `&a[i]`, `p[i]`, the decay, initializers,
`out-of-bounds`, the marked range rows in the driver, loops carrying arrays and the `offset_of == 0` backstops
(record 0059; its as-built names are in e1b's plan, section 12). e1b built ranges in contracts (record 0060:
`owns_range`, `reads_range`, element reads in contracts, calls and returns over ranges, loops over parameters'
ranges, markers over ranges, `owning(n)` on a parameter, array parameters). e1c built quantifiers (record 0061:
`cerastes_forall` with an inlined `pure` body in contracts, loop invariants and checks, `f__wf` variant M, the
`carrays.c` port in full, the tutorial chapter; `cerastes_old` in loop invariants was cut to M4.5e2, question 260;
`internal/status.md`). e1d built `pure` functions (record 0062: a `pure` function that is called or carries a
contract as a Viper function with its definition check, applications in checks, contracts, calls and loop
invariants, the exact sum; no cut). e1e1 is planned in full.

**Owner decisions.**
- **226 `forall` in C: inline the body.** `cerastes_forall(f, free...)`, where `f` is a `pure` function whose body
  is one `return E;`. The quantifier means `E` with `f`'s first parameter bound, substituted at the `forall` site
  (0006 item 9's meaning). It does not wait for M4.5f1; a call of a `pure` function anywhere else stays rejected
  until e1d.
- **227 The exact sum is in M4.5e1.** `r == a[0] + … + a[n-1]` needs a recursive `pure` function with an encoded
  body: a slice of M4.5f1/f2 comes into e1 as sub-session e1d.
- **228 `cerastes_reads_range(p, n)` is added** beside `owns_range` (e1b).
- **229–233** (each "Follow recommendation"): `*p` one past the end is `permission` (M4.5e2 decides its bounds
  row); `int a[static N]` is rejected (M8); `owns_range(p, n)` implies `p != NULL`; `cerastes_old(p[i])` is
  allowed in a `pure` body that only an `ensures` names (e1c); e1e's planning may split it in two.
- **236, 237** (e1b's planning): an atom on one element (`owns(p[i])`) and a range atom written in a loop invariant
  are M4.5e2's, not e1b's.
- **At e1c's planning (2026-11-23):** question 238 (`reads_range`'s amount from the range's first slot) is a false
  alarm, not unsoundness, and is to be fixed: e1c runs the spike (its S45 (r)), and the fix is a session of its
  own, **e1f**, which also builds question 242's row order D for `owns_range` (section 10). `cerastes_old` in a loop
  invariant is in e1c, as its last part and its first cut (taken: question 260, M4.5e2).
- **At e1d's planning (2026-11-24):** e1d is planned in full; a `pure` function is read when it is called or
  carries a contract; a `pure` function with no body in the unit cannot be called; one session with split points
  (questions 264–272: e1d's plan, sections 1 and 13).
- **At e1e's planning (2026-11-25):** e1e is cut in two, "scalars, then structures" (question 290); a structure
  with an array member is copied and passed by value in M4.5e1e (question 291), in e1e1 unless that would pass
  the twelve-hour cap (question 300); a question goes to the Fable advisor first; a plan carries a test budget
  (e1e1's plan, sections 3, 12 and 15; questions 292–299 are its defaults, for review).

**Standing:** 69 (no EDG work), 77 (one context), 160 (a new test's expect line may be corrected after its first
run, each listed in status), 176 (the owner handles CI); `check.sh` once, at the end, on the x86-64 box; no test
runs while a fix is queued; no Claude trailer; commits stay local.

### The sub-sessions

| Sub-session | Scope | Needs | `src/` estimate | State |
|---|---|---|---|---|
| **e1a Arrays inside one function** | local arrays as blocks, `a[i]`, `&a[i]`, initializers, scope exits, the marked range rows, `out-of-bounds`, loops over arrays, sequencing, pointers into arrays, the `offset_of == 0` restatements | — | landed 1,943 | **done** (record 0059) |
| **e1b Ranges in contracts** | `owns_range`, `reads_range`, element reads in contracts, calls and returns over ranges, `f__wf`, loops over parameter ranges, markers over ranges, `owning(n)` on a parameter, array parameters, the `carrays.c` port | e1a | landed 2,754 | **done** (record 0060) |
| **e1c Quantifiers** | `cerastes_forall` (question 226), triggers, `old` under a binder, max, the fill loop, the full `carrays.c` port, the tutorial chapter; the spike for e1f (`cerastes_old` in loop invariants cut to M4.5e2) | e1b | landed 3,514 (estimate 1,500–2,200) | **done** (record 0061) |
| **e1d Pure functions and the exact sum** | read `pure` functions as Viper functions with their contracts and definition checks, self-recursion with `decreases`, calls of `pure` functions in checks, contracts and loop invariants, the exact sum (question 227) | e1c | landed 3,654 (estimate 3,350–5,000), in 11.5 hours | **done** (record 0062) |
| **e1e1 Arrays of scalars in structures and static storage** | global and `static` arrays, array members, array compound literals, 0024 item 6 lifted for array types of scalars; structure values with array members (question 291, its Part E) | e1b | 2,340–4,475 for Parts A–D, 600–1,050 more for Part E (three surveys at `68384d1`) | **planned in full**: `.claude/plans/m4-5e1e1-arrays-of-scalars.md` |
| **e1e2 Arrays of structures and arrays of arrays** | arrays of structures, ranges and `forall` over them, 2-D arrays, `int a[][3]`; what e1e1 cuts or moves | e1e1 | 4,700–8,450 with what e1e1 left (1,700–3,100 was the estimate without it) | **planned in full:** `.claude/plans/m4-5e1e2-arrays-of-structures.md` |
| **e1f Range call rows** | the minimum read amount over a range at a call (question 238) and `owns_range`'s row order D (question 242); one set of Viper flips over e1b's range calls | e1c (its spike S45 (r)) | 150–400 | **planned after e1c**, from the spike (section 10) |

The estimates are insertions without self-tests; each sub-session's own planning measures them again. Each
sub-session is a session of its own (question 77): its branch and worktree, its record (the next free number), its
twins, docs, audits and one box `check.sh`. **M4.5e2** needs e1a and e1b (and e1e2 for `malloc(n * sizeof(struct
S))`); it may run before e1e and e1f.

**No sub-session plans again after its spikes.** The owner had e1b, e1c, e1d and e1e1 each planned in full, with
a decision table per probe and spike in place of the second planning this plan first foresaw; e1e2 was planned in
full the same way after e1e1, from what e1e1 built.

## 2. Rules for every sub-session

`internal/handoff.md` §4's rules and §6's process notes apply. e1b's, e1c's, e1d's and e1e1's plans (their section
2) are the model for the team: who owns which files, when an agent is replaced, the build and runner rules, the audit rounds and the
advisor's checkpoints. The rules this plan relies on:
- **Team** (`.claude/agents/`): three Sonnet agents with disjoint files and the Opus `soundness-auditor`,
  read-only. The orchestrator is Opus and consults the Fable advisor before a record is accepted, at any cut or
  stop-and-ask, and before the box run.
  - **edg-backend:** `src/backend`, `src/driver`, `include/cerastes.h`, the joined builds,
    `regen_vpr.py`/`vprdiff.py` after each part.
  - **il-analyst:** the probes and spikes; then `src/vir` and `src/viper` with the VIR self-test cases. A fresh
    il-analyst takes over after the spikes and wherever its context would run low (`lower.cpp` is 23,269 lines).
  - **test-mutator:** tests, twins, exactly the Flips, the compiler matrix (`native-ok`: clang and GCC, both modes,
    `-Wall -Wextra -Werror`), `run_tests.py`, `lint_viper.py` and their self-tests, the doc example files.
  - **soundness-auditor:** one round per diff on a snapshot commit in `.claude/worktrees/<session>-audit`, one
    re-audit of blocking fixes; should-fix items not taken go to handoff §3.
  - **Orchestrator:** Steps 0 and 1, the record, the vprdiff verdict per part, docs prose, integration, status and
    handoff.
- **`lower.cpp` is held by one agent at a time,** with a commit or a snapshot between holders.
- **Every agent:** any question stops that work and goes to the orchestrator, who takes it to the Fable advisor
  first and to the owner only if it is still open (owner, 2026-11-25); it runs no test its plan's test budget
  does not list, and every test run is on the x86-64 box; each report is also written to
  `build/<session>/<role>-report-<n>.md`; builds are serialized (`lockf build/<session>/build.lock cmake --build
  build/cmake`) and held before a directory-wide run; one agent runs `run_tests.py`, the others use
  `build/cerastes verify <file>`; a test generator lives in `build/<session>/` and is never re-run over the tree;
  code style by `scripts/format.sh`, brace initialization, `U`, one declarator per declaration, no comment on a
  brace line.
- **A registration or a declaration lands with its reader** (P77: a registered name the encoder does not read is
  silently ignored).
- **Fail closed between parts:** what a later part or sub-session lifts is rejected with a diagnostic naming it,
  and pinned by a test.
- **Session tools and spike summaries are committed** (`scripts/`, `internal/il-inventory.md`).

## 3. M4.5e1a — Arrays inside one function (record 0059; done 2026-11-11)

Done as planned, nothing cut. Its approved plan is section 3 of `git show dc40377:.claude/plans/m4-5e1-arrays.md`;
what was built is record 0059 (each item says so), `internal/status.md`'s entry and `internal/il-inventory.md`,
chapter M4.5e1a (P78, S43).


## 4. M4.5e1b — Ranges in contracts (record 0060; done 2026-11-12)

**Its plan is `.claude/plans/m4-5e1b-ranges-in-contracts.md`** (approved 2026-11-11; executed without re-planning;
what was built is record 0060 and `internal/status.md`'s entry). The sketch that stood here
(what was decided, what was proposed, P79 and S44, the known pins) is section 4 of
`git show dc40377:.claude/plans/m4-5e1-arrays.md`; the new plan fixes its proposals and replaces its "plan again"
by decision tables. Against the sketch, two things moved to M4.5e2 (questions 236 and 237): atoms on one element,
and range atoms written in loop invariants.

## 5. M4.5e1c — Quantifiers (record 0061; done 2026-11-24)

**Its plan is `.claude/plans/m4-5e1c-quantifiers.md`** (approved 2026-11-23). The sketch that stood here (what was
decided, what was proposed, P80 and S45, the tests and docs) is section 5 of `git show
bdf3581:.claude/plans/m4-5e1-arrays.md`; the new plan fixes its proposals and replaces its "plan again" by decision
tables. Against the sketch: the `f__wf` choice has a stated order (today's heap-free method extended by ghost
functions first, a heap form with a boundary as the fallback), a body may read several arrays or neighbours (one
binder per read class, decided by the spike), `forall` in a `check` has its own rows, `cerastes_old` in a loop
invariant was added as Part D, and the spike for e1f rides along. Done with one cut: Part D (question 260) went to
M4.5e2; what was built is record 0061 and `internal/status.md`'s entry.

## 6. M4.5e1d — Pure functions and the exact sum (record 0062; done 2026-11-25)

**Its plan is `.claude/plans/m4-5e1d-pure-functions.md`** (approved 2026-11-24; executed without re-planning, no
cut; what was built is record 0062 and `internal/status.md`'s entry). The sketch that stood here is section 6 of
`git show 9ab9a1e:.claude/plans/m4-5e1-arrays.md`. Against the sketch:
- a `pure` function is read when it is called or carries a contract, and one that only a `cerastes_forall` names
  stays inlined and unread (so no existing program's Viper changes);
- the definition check is the function's own `__wf` method (the body as one more clause), not a method beside it;
  Viper's own check of the function proves its `ensures` and its termination, and those two errors are reported
  from it;
- an application in a contract, a function body and a loop invariant is guarded, so that Viper's own check never
  reports beside a row (S45 (d) (7)'s twin gave two errors without the guard); in `f__wf` it is a ghost function
  with three assumed facts, the callee's `ensures` under the callee's `requires`;
- the protected exact sum returns `long long` (its bound follows from the element type); the `int` form with
  `cerastes_forall(small_at, a, n)` in the function's `requires` is a second test and a pre-agreed cut;
- a `pure` function with no body in the unit cannot be called; mutual recursion, a call under a quantifier's
  binder, a `cerastes_forall` inside a called body and a call in a measure stay rejected (M4.5f2, M4.5e2, M4.5f1).

## 7. M4.5e1e — Arrays in aggregates and storage (two sessions: e1e1, record 0063, planned in full; e1e2)

**e1e1's plan is `.claude/plans/m4-5e1e1-arrays-of-scalars.md`** (written 2026-11-25); **e1e2's plan is
`.claude/plans/m4-5e1e2-arrays-of-structures.md`** (written 2026-11-25, after e1e1; e1e1's section 14 was its start). The sketch that stood here (decided and proposed, P82 and S47, the known pins) is section 7
of `git show 68384d1:.claude/plans/m4-5e1-arrays.md`. Against the sketch:
- **the cut is by element type, not "aggregates; storage"** (question 233's seam): e1e1 takes every array of
  scalars (globals, `static` locals, members, literals, computed types), which needs one extension of e1a's and
  e1b's range machinery (any block term as a range's block); e1e2 takes arrays of structures (a member tail
  under the element slot) and arrays of arrays (two bound variables, of which one S26 fixture is all the
  evidence);
- **a structure is no longer a finite list of cells:** the leaves get an object view (cells and array leaves) and
  a value view (every element a leaf), and the old view keeps an array member as one poisoned leaf, so that a
  site that was not migrated stops with an internal error instead of reading the array as one cell;
- **a by-value structure with an array member is not left rejected** (the sketch's "to be confirmed"): question
  291 puts the copy (a trusted `copy_arr_<t>`) and by-value passing (one Viper value per element, at most 64) in
  M4.5e1e;
- **the note on `is_global_ref` was overstated:** the shortcut fires for slot 0 only, and an array global needs an
  interval hit;
- whole-array atoms (`owns(g)`, `owns(p->a)`) are e1e1's, and a range on part of a global array or of an array
  member is M4.5e2's;
- an owning field or a marker beside an array member, and heap storage of such a structure, are rejected in e1e1
  and lifted by e1e2 (questions 292–294, defaults for review); arrays of owning pointers stay M4.8's and `const`
  arrays M4.6's.

## 8. Open questions

None here: 229–233 are answered (section 1), and so are 238–289 (the records' headers; `internal/handoff.md` §8)
and 290, 291 and 300 (e1e1's plan, section 15, which also lists its defaults 292–299).

## 9. Inputs

e1e's inputs moved into e1e1's plan (its sections 4, 13 and 14) at its planning; what stood here is section 9 of
`git show 68384d1:.claude/plans/m4-5e1-arrays.md`.

## 10. M4.5e1f — Range call rows (the next free record) — planned after e1c, from its spike

**Scope** (owner, 2026-11-23, questions 238 and 242). Two changes to the rows `call_atoms` writes for a range atom,
in one session, because both change the Viper of e1b's existing range calls and are re-proved and flipped once:
- **The read amount** (238). `reads_range`'s amount at a call is taken from the range's first slot, lowered by the
  caller's own read amounts (record 0060 item 6). A caller whose holding over the range is uneven for another
  reason (a callee kept a read fraction of one element and gave the liveness back) gets a false `permission`. It
  is fail-closed: the amount can only be too high for another slot, and every slot is still asserted at it. The
  fix is the minimum over the range, which `perm` cannot state under a quantifier in an `assert`; the candidate
  is a tagged `inhale` that defines it (a bound over the range and a witness), **the encoder's first `inhale`**
  (invariant 4: its record argues that a minimum over a non-empty finite range exists, and the lint admits
  exactly that shape).
- **Order D** (242). For `owns_range` the marked rows before the shape row, the liveness row and the amount: one
  Silicon error in every twin of S44 (b), where the built order gives two rows (shown as one error) for a callee
  that forwards `(p, n + 1)` of its own range. `reads_range` keeps its order (its amount needs the liveness cell
  first).

**Inputs.** e1c's spike S45 (r) (`internal/il-inventory.md`, chapter M4.5e1c: whether the pinned Silicon takes
`perm` under a quantifier in an `inhale` or in a helper's `ensures`, the pin's verdict with it, order D in the
built rows); the pins `range_call_reads_unequal_pin.c`, `range_call_reads_two_atoms_pin.c` and `_local.c`; the
fixtures `s44_b_*_orderD*`; handoff §3 "M4.5e1f". **If the spike shows no form Silicon takes,** the session builds
order D alone and question 238 goes to M4.8's work on read amounts (question 66), by the owner's decision then.

**Shape of the session.** Small (150–400 `src/` lines): its record, the lint rule and its twins, the two changes,
the flips (every `.vpr` of a test with a range call: outcomes unchanged, the changed lines classified by exact
inverse rules, e1a's lesson), the pins re-expected as positives with twins, one audit round, one box `check.sh`. It
needs e1c only for the spike; it may run before or after e1e1 and e1e2. After e1e1, an array leaf of a `reads`
atom takes its amount from its first slot too (e1e1's record, item 7), so the fix of question 238 has to reach it.
