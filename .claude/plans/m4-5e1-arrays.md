# Plan: M4.5e1 — Arrays (five sub-sessions, records from 0059)

**Approved by the owner on 2026-11-11**, at a session that only planned (question 224). Its commit also split
M4.5e1 into five sub-sessions (e1a–e1e).

**Trimmed at M4.5e1b's planning (2026-11-11, owner's choice) to what e1c–e1e still need.** e1a is done (record
0059, `internal/status.md`), and e1b has a plan of its own, `.claude/plans/m4-5e1b-ranges-in-contracts.md`. **The
text before the trim is `git show dc40377:.claude/plans/m4-5e1-arrays.md`**: its section 3 is e1a's approved plan
(scope, survey facts, P78 and S43, the record's draft, parts, tests, flips, docs, acceptance), its section 4 e1b's
sketch, its section 8 questions 229–233 as asked. Section numbers are kept, so older references still resolve.

e1c–e1e each start with their probes and spikes as written here, then plan again (section 1).

## 1. Context

**Why.** M4.5e1 is the arrays session of the C memory model (records 0028 items 1, 2, 11, 12 and 16; 0029 items 1,
3 and 4). Its planning (question 224) found it does not fit one context.

**Where it stands.** e1a built local arrays as blocks, `a[i]`, `&a[i]`, `p[i]`, the decay, initializers,
`out-of-bounds`, the marked range rows in the driver, loops carrying arrays and the `offset_of == 0` backstops
(record 0059; its as-built names are in e1b's plan, section 12). e1b is planned in full.

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

**Standing:** 69 (no EDG work), 77 (one context), 160 (a new test's expect line may be corrected after its first
run, each listed in status), 176 (the owner handles CI); `check.sh` once, at the end, on the x86-64 box; no test
runs while a fix is queued; no Claude trailer; commits stay local.

### The sub-sessions

| Sub-session | Scope | Needs | `src/` estimate | State |
|---|---|---|---|---|
| **e1a Arrays inside one function** | local arrays as blocks, `a[i]`, `&a[i]`, initializers, scope exits, the marked range rows, `out-of-bounds`, loops over arrays, sequencing, pointers into arrays, the `offset_of == 0` restatements | — | landed 1,943 | **done** (record 0059) |
| **e1b Ranges in contracts** | `owns_range`, `reads_range`, element reads in contracts, calls and returns over ranges, `f__wf`, loops over parameter ranges, markers over ranges, `owning(n)` on a parameter, array parameters, the `carrays.c` port | e1a | 2,050–3,050 | **planned in full**: `.claude/plans/m4-5e1b-ranges-in-contracts.md` |
| **e1c Quantifiers** | `cerastes_forall` (question 226), triggers, `old` under a binder, max, the fill loop, the tutorial chapter | e1b | 1,200–2,000 | **plan mode again** (section 5) |
| **e1d Pure functions and the exact sum** | `pure` bodies as Viper functions, calls of `pure` functions in annotations, recursion with `decreases`, the exact sum (question 227) | e1c | 1,200–2,000 | **plan mode again** (section 6) |
| **e1e Arrays in aggregates and storage** | array members, arrays of structures, 2-D arrays, global and `static` arrays, array compound literals, 0024 item 6 lifted for array types | e1b (e1c for its quantified tests) | 1,500–2,500 | **plan mode again** (section 7) |

The estimates are insertions without self-tests; each sub-session's own planning measures them again. Each
sub-session is a session of its own (question 77): its branch and worktree, its record (the next free number), its
twins, docs, audits and one box `check.sh`. **M4.5e2** needs e1a and e1b (and e1e for `malloc(n * sizeof(struct
S))`); it may run before e1c–e1e.

**What "plan mode again" means.** The sub-session starts outside plan mode with its Step 0, Step 1, probes and
spikes exactly as written here (they change no `src/`), commits their results to `internal/il-inventory.md`,
then enters plan mode. That planning takes the spike results and the as-built facts of the sub-sessions before it,
refines the record draft and the encoding steps, lists the Flips, and is saved as
`.claude/plans/m4-5e1<x>-<topic>.md`. (e1b did not: the owner had it planned in full, with a decision table per
probe and spike in place of the second planning; a later sub-session may be planned the same way.)

## 2. Rules for every sub-session

`internal/handoff.md` §4's rules and §6's process notes apply. e1b's plan (its section 2) is the model for the
team: who owns which files, when an agent is replaced, the build and runner rules, the audit rounds and the
advisor's checkpoints. The rules this plan relies on:
- **Team** (`.claude/agents/`): three Sonnet agents with disjoint files and the Opus `soundness-auditor`,
  read-only. The orchestrator is Opus and consults the Fable advisor before a record is accepted, at any cut or
  stop-and-ask, and before the box run.
  - **edg-backend:** `src/backend`, `src/driver`, `include/cerastes.h`, the joined builds,
    `regen_vpr.py`/`vprdiff.py` after each part.
  - **il-analyst:** the probes and spikes; then `src/vir` and `src/viper` with the VIR self-test cases. A fresh
    il-analyst takes over after the spikes and wherever its context would run low (`lower.cpp` is 19,293 lines).
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

In sections 5–7 below, `E` is `src/backend/encode.cpp` and `L` is `src/vir/lower.cpp`, with lines at `f57ee43`
(before e1a): each sub-session's planning refreshes them.

## 4. M4.5e1b — Ranges in contracts (record 0060; planned in full)

**Its plan is `.claude/plans/m4-5e1b-ranges-in-contracts.md`** (approved 2026-11-11). The sketch that stood here
(what was decided, what was proposed, P79 and S44, the known pins) is section 4 of
`git show dc40377:.claude/plans/m4-5e1-arrays.md`; the new plan fixes its proposals and replaces its "plan again"
by decision tables. Against the sketch, two things moved to M4.5e2 (questions 236 and 237): atoms on one element,
and range atoms written in loop invariants.

## 5. M4.5e1c — Quantifiers (the next free record) — ⟲ plan mode again after P80 and S45

**Scope** (question 226). `cerastes_forall(f, free...)` in `requires`, `ensures`, `CERASTES_INVARIANT` and
`CERASTES_CHECK`; max; the fill loop of e1a's pin; the rest of the `carrays.c` port; the tutorial chapter "Arrays
and quantifiers". The C++ lambda form stays M7's (`reject_forall_pointer.cpp` keeps its line).

### 5.1 Decided

- `f` names a `pure` function with a body that is one `return E;`; its first parameter is the bound variable and
  the others receive the trailing arguments in order (0029 item 4; P53: `enk_routine` in C23, a constant address
  in C++20). The quantifier means `E` over every value of the first parameter's type.
- `E` is substituted at the site; no Viper function is emitted for `f` here, and a call of `f` stays rejected.

### 5.2 Proposed, for the re-plan to fix

- **The pure body reader:** finds the one `return E;`, refuses anything else (`unsupported_annotation_expression`),
  and encodes `E` in contract mode with the parameters bound. e1d reuses it.
- **The binder and triggers** (0028 item 2; S24). The Viper binder is the absolute slot of the first element read
  that is indexed by the bound variable plus a constant (`p[i]`, `p[i + 1]`), with the trigger `{ loc(block_of(p),
  k) }`; the other reads are rewritten over it. A body with no such read, or an index with another coefficient
  (`a[2 * i]`), is rejected and pinned.
- **Arity and types:** a count or type mismatch between `f`'s parameters and the arguments is rejected (0006
  item 9 left it to the session that gives `forall` its meaning).
- **Framing and well-formedness under the binder:** every read of `E` must be framed, and every operation of `E`
  defined (0010 item 11), for every value of the bound variable that reaches it. `f__wf` reads no heap today
  (0037 item 7), so a read under a binder needs a ghost that depends on the binder: S45 decides between a ghost
  `Map[Int, T]` formal per read and a `f__wf` that holds the range permissions. **The choice binds e1d:** a
  heap-free `f__wf` cannot evaluate a call of a heap-reading `pure` function (`r == sum_to(a, n)` in an
  `ensures`), so S45 chooses with that shape in view and says what e1d would need under each variant.
- **Flags:** a read under the binder states that the element holds a value (0038 item 6), so a quantified
  invariant can say which elements a loop has written; that lifts e1a's pin.
- **`old` under the binder** (question 232): `cerastes_old(p[i])` is allowed in the body of a `pure` function
  that only an `ensures` names; `array_test2`'s `update(0, 10, vs)` and `to_verify`'s lists need it.
- **Max:** `max` returns the value with `forall i: a[i] <= r`; `max_index` returns `r` with `0 <= r && r < n` and
  `forall i: a[i] <= a[r]` (there is no `exists`).

### 5.3 P80 and S45 (run before the re-plan)

- **P80:** `cerastes_forall(f, a, n)` and `cerastes::forall(f, a, n)` in every annotation kind, with `f` and
  `&f`; the IL of a `pure` function's body (the `return`'s expression, its parameter references) and what
  `annotation_checks.cpp` already enforces on it; a mismatch of count and of type; `cerastes_old` inside a `pure`
  body.
- **S45:** the absolute-slot rewriting for one array, for `a[i] <= a[i + 1]`, and for two arrays (`a[i] ==
  b[i]`); the fill loop with a quantified invariant; max; a copy loop; `forall` with `old`; both variants of
  `f__wf` under a binder; an array of 4 and a symbolic `n`. **Stop rules:** a matching loop (a time over 60 s), a
  correct shape that fails, or neither `f__wf` variant holding: stop and ask.

### 5.4 Tests and docs

- Positives with twins: a quantified `requires` used at a read; a quantified `ensures` proved by a loop; the fill
  loop (the pin's twin); max and `max_index` (twins: an off-by-one range, `<` for `<=`); sortedness; the
  `carrays.c` functions in full; every rejection of 5.2.
- Docs: `annotations.adoc` (`forall`); `memory.adoc` "Arrays"; the tutorial chapter with max and the fill loop,
  its `console` blocks copied from runs.

## 6. M4.5e1d — Pure functions and the exact sum (the next free record) — ⟲ plan mode again after P81 and S46

**Scope** (question 227): the slice of M4.5f1/f2 that an exact sum needs. M4.5f1 keeps predicates, `fold` and
`unfold`, and pure functions over predicates.

### 6.1 Proposed, for the re-plan to fix

- **A `pure` function with a body `return E;` becomes a Viper function** with that body: its `requires` (values,
  and `reads(*p)` or `reads_range(p, n)` for what it reads; `owns` atoms rejected), an optional `ensures`, and
  `decreases` when it is recursive (0013: required in a cycle).
- **Calls of `pure` functions** are accepted in annotations and in `pure` bodies (0011 item 7's
  `call_in_annotation` lifted for them), each with explicit rows for the callee's `requires`; a call from code
  stays rejected.
- **The body's obligations** (overflow, division, bounds, framing, flags: 0010 item 11) are checked once, in a
  definition-check method beside `f__wf`, under the function's `requires`; the Viper function holds only the
  meaning. Viper proves the `ensures` and the termination measure.
- **The exact sum:**
  ```c
  CERASTES_PURE int sum_to(const int *a, int n)
      CERASTES_REQUIRES(0 <= n && n <= 1000 && cerastes_reads_range(a, n) && cerastes_forall(small_at, a, n))
      CERASTES_ENSURES(r, -1000 * n <= r && r <= 1000 * n)
      CERASTES_DECREASES(n)
  {
      return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
  }
  ```
  and `sum` with the invariant `s == sum_to(a, i)` and `ensures(r, r == sum_to(a, n))`.
- **Stays rejected:** a `pure` body that is not one `return`; mutual recursion; a `pure` function returning a
  pointer or a structure; predicates (M4.5f1).

### 6.2 P81 and S46 (run before the re-plan)

- **P81:** the IL of a call of a `pure` function in each annotation kind and inside a `pure` body; a contract and
  a `decreases` on a `pure` function; recursion; today's diagnostics for each.
- **S46:** a heap-reading Viper function with a quantified read permission in its precondition (what the pinned
  Silicon does with the amount); the recursive `sum_to` with its `decreases` and bounds `ensures`; `sum`'s loop
  (does Silicon unfold `sum_to(a, i + 1)` unaided, or does the loop need a `check`); a function applied under a
  `forall`; the definition-check method; the contract's well-formedness method with `r == sum_to(a, n)` in an
  `ensures`, under the `f__wf` variant e1c chose; a twin whose body overflows; a non-terminating twin. **Stop rules:** the
  loop not verifying without a lemma (then the record says what the user writes, or the owner decides); any
  program over 60 s.

### 6.3 Tests and docs

- The exact sum with twins (an off-by-one invariant, a dropped bound: `overflow`); a non-recursive `pure`
  function used in a contract; every rejection.
- Docs: `annotations.adoc` (`pure`); the tutorial chapter gains the exact sum; handoff §3's M4.5f1 notes that this
  consumes (R1–R13 in `pure` bodies, `static_assert_pure_body.c`).

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

None: 229–233 are answered (section 1; the questions as asked are in the commit named at the top). e1b's
planning opened 238–240 (`internal/handoff.md` §8).

## 9. Inputs e1c–e1e still take (from handoff §3 and §4.2 at M4.5e1's planning)

- P38's braced initializers: a list of more than 1,000 elements keeps no `expr` for its elements
  (`REDUCE_BACKING_EXPRESSION_USE`), only an `orig_type`; read every element through `constant_value`, which
  rejects that shape, never through `ck_aggregate`'s values directly, or a wrapping element would pass unchecked.
  P78 (b) found the loss only for static storage, so it is e1e's. [M4.4a, record 0022 item 3]
- Record 0056 item 11's escape scan must see an array member decaying to a pointer as an escape of the struct
  local. [d2b2c re-audit] (e1e)
- M4.4b2's types computed from a constant (`sizeof`, `typeof`, typedefs), and a bound that names a local constant
  (P78 F1). (e1e)
- The port `carrays.c`: e1b ports its five functions with single-element facts
  (`tests/memory/verifast/carrays.c`); its lists become `forall` in e1c.
- From e1b's plan: a `must_init` filler that fills with a loop is a pinned false alarm until e1c's quantified
  invariants (question 239); ranges over structures, array members and global arrays are e1e's.
- **The survey's contradictions, settled:** `reject_forall_pointer.cpp` is the C++ lambda form (M7), not e1's;
  0024's "until M4.5e" also names `_BitInt`, vectors and non-type template arguments, which stay rejected;
  "M4.5e" in 0022 and 0047 for `n * sizeof(T)`, `calloc(n, …)` and `realloc` is M4.5e2.
