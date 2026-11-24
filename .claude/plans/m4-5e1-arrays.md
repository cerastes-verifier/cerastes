# Plan: M4.5e1 — Arrays (six sub-sessions, records from 0059)

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

e1e starts with its probes and spikes as written here, then plans again (section 1), unless the owner has it
planned in full as e1b, e1c and e1d were.

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
`internal/status.md`). e1d is planned in full.

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

**Standing:** 69 (no EDG work), 77 (one context), 160 (a new test's expect line may be corrected after its first
run, each listed in status), 176 (the owner handles CI); `check.sh` once, at the end, on the x86-64 box; no test
runs while a fix is queued; no Claude trailer; commits stay local.

### The sub-sessions

| Sub-session | Scope | Needs | `src/` estimate | State |
|---|---|---|---|---|
| **e1a Arrays inside one function** | local arrays as blocks, `a[i]`, `&a[i]`, initializers, scope exits, the marked range rows, `out-of-bounds`, loops over arrays, sequencing, pointers into arrays, the `offset_of == 0` restatements | — | landed 1,943 | **done** (record 0059) |
| **e1b Ranges in contracts** | `owns_range`, `reads_range`, element reads in contracts, calls and returns over ranges, `f__wf`, loops over parameter ranges, markers over ranges, `owning(n)` on a parameter, array parameters, the `carrays.c` port | e1a | landed 2,754 | **done** (record 0060) |
| **e1c Quantifiers** | `cerastes_forall` (question 226), triggers, `old` under a binder, max, the fill loop, the full `carrays.c` port, the tutorial chapter; the spike for e1f (`cerastes_old` in loop invariants cut to M4.5e2) | e1b | landed 3,514 (estimate 1,500–2,200) | **done** (record 0061) |
| **e1d Pure functions and the exact sum** | read `pure` functions as Viper functions with their contracts and definition checks, self-recursion with `decreases`, calls of `pure` functions in checks, contracts and loop invariants, the exact sum (question 227) | e1c | 3,350–5,000 (a design check calibrated on e1c) | **planned in full**: `.claude/plans/m4-5e1d-pure-functions.md` |
| **e1e Arrays in aggregates and storage** | array members, arrays of structures, 2-D arrays, global and `static` arrays, array compound literals, 0024 item 6 lifted for array types | e1b (e1c for its quantified tests) | 1,500–2,500 | **plan mode again** (section 7) |
| **e1f Range call rows** | the minimum read amount over a range at a call (question 238) and `owns_range`'s row order D (question 242); one set of Viper flips over e1b's range calls | e1c (its spike S45 (r)) | 150–400 | **planned after e1c**, from the spike (section 10) |

The estimates are insertions without self-tests; each sub-session's own planning measures them again. Each
sub-session is a session of its own (question 77): its branch and worktree, its record (the next free number), its
twins, docs, audits and one box `check.sh`. **M4.5e2** needs e1a and e1b (and e1e for `malloc(n * sizeof(struct
S))`); it may run before e1c–e1e.

**What "plan mode again" means.** The sub-session starts outside plan mode with its Step 0, Step 1, probes and
spikes exactly as written here (they change no `src/`), commits their results to `internal/il-inventory.md`,
then enters plan mode. That planning takes the spike results and the as-built facts of the sub-sessions before it,
refines the record draft and the encoding steps, lists the Flips, and is saved as
`.claude/plans/m4-5e1<x>-<topic>.md`. (e1b, e1c and e1d did not: the owner had each planned in full, with a
decision table per probe and spike in place of the second planning; e1e may be planned the same way.)

## 2. Rules for every sub-session

`internal/handoff.md` §4's rules and §6's process notes apply. e1b's, e1c's and e1d's plans (their section 2) are
the model for the team: who owns which files, when an agent is replaced, the build and runner rules, the audit rounds and the
advisor's checkpoints. The rules this plan relies on:
- **Team** (`.claude/agents/`): three Sonnet agents with disjoint files and the Opus `soundness-auditor`,
  read-only. The orchestrator is Opus and consults the Fable advisor before a record is accepted, at any cut or
  stop-and-ask, and before the box run.
  - **edg-backend:** `src/backend`, `src/driver`, `include/cerastes.h`, the joined builds,
    `regen_vpr.py`/`vprdiff.py` after each part.
  - **il-analyst:** the probes and spikes; then `src/vir` and `src/viper` with the VIR self-test cases. A fresh
    il-analyst takes over after the spikes and wherever its context would run low (`lower.cpp` is 20,479 lines).
  - **test-mutator:** tests, twins, exactly the Flips, the compiler matrix (`native-ok`: clang and GCC, both modes,
    `-Wall -Wextra -Werror`), `run_tests.py`, `lint_viper.py` and their self-tests, the doc example files.
  - **soundness-auditor:** one round per diff on a snapshot commit in `.claude/worktrees/<session>-audit`, one
    re-audit of blocking fixes; should-fix items not taken go to handoff §3.
  - **Orchestrator:** Steps 0 and 1, the record, the vprdiff verdict per part, docs prose, integration, status and
    handoff.
- **`lower.cpp` is held by one agent at a time,** with a commit or a snapshot between holders.
- **Every agent:** any question stops that work and goes to the owner; each report is also written to
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

In section 7 below, `E` is `src/backend/encode.cpp` and `L` is `src/vir/lower.cpp`, with lines at `f57ee43`
(before e1a): its planning refreshes them.

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

## 6. M4.5e1d — Pure functions and the exact sum (record 0062; planned in full)

**Its plan is `.claude/plans/m4-5e1d-pure-functions.md`** (approved 2026-11-24). The sketch that stood here (the
slice of M4.5f1/f2 an exact sum needs, what was proposed, P81 and S46, the tests and docs) is section 6 of `git
show 9ab9a1e:.claude/plans/m4-5e1-arrays.md`; the new plan fixes its proposals and replaces its "plan again" by
decision tables. Against the sketch:
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

## 7. M4.5e1e — Arrays in aggregates and storage (the next free record) — ⟲ plan mode again after P82 and S47

**Scope.** Array members (`member(r, k)` of length n), arrays of structures, arrays of arrays (a row is
`member(loc(b, i), 0)`; 0028 item 16), global and `static` arrays, array compound literals, array types in
`sizeof(type)`, `typeof`, `_Alignof` and typedefs (0024 item 6 lifted for arrays), structure copies with array
members, range atoms and `forall` over them. Its planning may split it in two (aggregates; storage): question 233.

### 7.1 Decided and proposed

- **Decided** (0028 items 2 and 16; S26): over an array of structures there is one quantifier per member and
  field, triggered `{ loc(member(loc(block_of(a), k), j), 0) }`; a matrix's contract fixes `offset_of(a) == 0`;
  `a[0][3]` and `a[2][0]` fail their bounds; `&a[0][3]` is formed.
- **Globals:** `len(GB_g()) == N` in `global_facts` (`L` 6277–6294) and the lint (`global_facts_atoms`);
  `program_start` gives the initial values, zero as one quantifier, written elements through `constant_value`
  (P38: a list over 1,000 elements keeps no `expr`, and that shape is rejected, never read from `ck_aggregate`);
  `owns(g)` and `reads(g)` of an array global mean every element. **`is_global_ref` and `inv_place_disjunct`
  (`L` 6267–6271, 3809–3817) treat any `loc(GB_…, i)` as one place: wrong once a global has several slots.**
- **Escape scan** (0056 item 11): an array member decaying to a pointer is an escape of its struct local
  (`escapes_in_expr`, `L` 2930).
- **Stay rejected, to be confirmed:** an owning field inside an array of structures and arrays of owning pointers
  (closures over ranges: M4.8 or M4.5f2); a by-value structure with an array member, unless S47 shows the
  quantified copy is cheap; `const` arrays (M4.6).

### 7.2 P82 and S47 (run before the re-plan)

- **P82:** array members and their initializers (brace elision), `s.a[i]`, `p->a[i]`, `&s.a[i]`, the decay of
  `s.a`; `struct S v[3]`, `v[i].x`; `int m[2][3]`, `m[i][j]`, `m[i]` as a value; global and `static` arrays with
  and without initializers, a 1,001-element list; `(int[]){1, 2, 3}`; `sizeof(int[3])`, a typedef of an array
  type and its uses.
- **S47:** S26's member fixtures re-run in the encoder's shapes with e1a's loops and e1b's atoms; a global array
  through `program_start` with a loop in `main`; a structure copy with an array member; costs. **Stop rule:** any
  program over 60 s.

### 7.3 Known pins

`global_array.c`, `reject_struct_array_member.c`, the array compound literal messages (`E` 9663, 9735), and
the list from records 0024 and 0018 in `tests/scalar/negative` (`reject_sizeof_*array*`, `reject_typedef_array_*`,
`reject_alias_array.cpp`, `reject_alignof_array.c`, `reject_datasizeof_array.cpp`, `reject_decltype_array.cpp`,
`reject_enum*_array*`, `reject_static_assert_sizeof_array.c`, `reject_contract_sizeof_array.c`,
`reject_skipped_*array*`; `reject_sizeof_compound_literal.c` and `reject_sizeof_string.c` are M4.6's);
`reject_struct_flexible_member.c` stays (M8).

## 8. Open questions

None here: 229–233 are answered (section 1; the questions as asked are in the commit named at the top), and so
are 238–263 (record 0061's header; `internal/handoff.md` §8) and 264–272 (e1d's plan, section 13).

## 9. Inputs e1e still takes (from handoff §3 and §4.2 at M4.5e1's planning; e1c's and e1d's moved into their plans)

- P38's braced initializers: a list of more than 1,000 elements keeps no `expr` for its elements
  (`REDUCE_BACKING_EXPRESSION_USE`), only an `orig_type`; read every element through `constant_value`, which
  rejects that shape, never through `ck_aggregate`'s values directly, or a wrapping element would pass unchecked.
  P78 (b) found the loss only for static storage, so it is e1e's. [M4.4a, record 0022 item 3]
- Record 0056 item 11's escape scan must see an array member decaying to a pointer as an escape of the struct
  local. [d2b2c re-audit] (e1e)
- M4.4b2's types computed from a constant (`sizeof`, `typeof`, typedefs), and a bound that names a local constant
  (P78 F1). (e1e)
- From e1b's plan: ranges over structures, array members and global arrays are e1e's.
- **The survey's contradictions, settled:** `reject_forall_pointer.cpp` is the C++ lambda form (M7), not e1's;
  0024's "until M4.5e" also names `_BitInt`, vectors and non-type template arguments, which stay rejected;
  "M4.5e" in 0022 and 0047 for `n * sizeof(T)`, `calloc(n, …)` and `realloc` is M4.5e2.

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
needs e1c only for the spike; it may run before e1d and e1e.
