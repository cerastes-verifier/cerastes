# Handoff after the second session of M4.5e1e2's continuation (2026-11-26) — Parts E and F run next, from the same plan

The next session reads this file after `CLAUDE.md` and its plan. `status.md` has the formal entry of the last finished
session (the continuation's second: Part D of the continuation plan's six) and what happened in it. This
file adds what a cold start would otherwise have to rediscover, and holds only what is live (owner, 2026-10-26).

**What the next session needs here:** §1, §2, the rules of §4 and §6, and §8 (**three things are for the owner:
question 354, the limit this session put on `calloc`; question 357, a lint edit that the permission system
denied; question 348, a rejection it kept; none blocks the next session**; the advisor's rulings 348–357 are
for the owner's review). What the next session needs from §3 is under "From the second session of M4.5e1e2's
continuation" and, below it, "From the first session of M4.5e1e2's continuation".

It was consolidated at M4.5d2b2b's planning (2026-11-09), at M4.5d2b2c's (2026-11-10), at M4.5e1's and at
M4.5e1b's (both 2026-11-11), at M4.5e1c's (2026-11-23), at M4.5e1d's (2026-11-24), at M4.5e1e's and at M4.5e1e2's
(both 2026-11-25), and at the planning of M4.5e1e2's continuation (2026-11-25), each at the owner's request; §1,
§2 and the end of §8 were rewritten at the close of the continuation's first session and again at the close of
its second (both 2026-11-26). **The text before that planning is `git show f313023:internal/handoff.md`, the
text the continuation's first session started from is `git show ae622e5:internal/handoff.md`, and the text
this session started from, with the owner's inline answers, is `git show 493a923:internal/handoff.md`**; the
ones before are named there. Each note below keeps its origin in brackets. Session plans approved from
M4.5b1a on are in `.claude/plans/`; the notes a session consumes move into its plan's "Inputs".

## 1. State

- **`main` is merged locally and not pushed.** The owner merged the continuation's first session into local
  `main` (`e82f894`); this session committed the owner's own inline handoff answers there (`493a923`) and
  branched from it. **`origin/main` is still `f07d80a`**, which accepts every unsound `restrict` form (found
  by the audits of M4.5e1e2's first session; local `main` rejects every `restrict` parameter since the
  continuation's Part R). Pushing is the owner's (§2, step 1).
- **The second session of M4.5e1e2's continuation** (2026-11-26; plan
  `.claude/plans/m4-5e1e2b-arrays-of-structures.md`; record 0064, accepted for the items built) is on the
  branch `m4.5e1e2c-arrays-of-structures` (worktree `.claude/worktrees/m4.5e1e2c`), which started at
  `493a923`. It landed **Part D in one commit, Commit 4 `86442ce`**: a structure with an array member of scalars
  on the heap (`malloc`, `calloc` up to 24 values, `free`, `cerastes_owns_block`), with an `owning` or marked
  member beside the array, under `owning`, `ref_to_uninit` and `must_init` pointers, reached through a
  pointer read from memory in a contract, given by an `ensures` atom with no `requires` partner, carried by
  loops and named by `owns` atoms written in loop invariants (record 0064 item 15). The closing commits
  after it change only `internal/` and `plan.md`. Two audit rounds and a short read of the fixes (a fresh
  Fable auditor each) found no wrong program that verifies; what they did not read is in the status entry,
  "Audits". **The full check (`CERASTES_LONG=1`, on the box, run once): green at Commit 4 `86442ce`** (17,488 tests passed, 0 failed, 10 skipped; the VIR self-test 2,814 cases; 58 minutes on a loaded box), the last commit that touches anything but `internal/` and `plan.md`.
- **What is left of M4.5e1e2: Parts E and F,** from the same plan, without a new planning (question 310's
  12-hour rule; the session closed at the plan's split point after Commit 4 because Part E, with its audit
  and the close-out, would have ended past fourteen hours). **Part E:** arrays of arrays, array-of-rows
  parameters (the three spellings and, by P84 (b), the typedef'd ones), `v[i].a[j]`, depth 3, the tutorial's
  matrix example. **Part F:** `forall` and `pure` applications over structure members, member arrays and
  contract-position global arrays. Each is rejected today with a diagnostic and a pin that names its part.
  **An encoder draft of Part E exists on the branch `m4.5e1e2c-part-e-encoder-draft`, unproved end to end
  and unaudited** (§3).
- **Listed limits this session added** (record 0064 item 15; each fail closed): `calloc` of a structure with
  an array member is rejected above 24 values (question 354); a structure whose array member holds
  structures is rejected at Part D's six positions (question 348); a read of an element that only a loop
  wrote, in a `malloc`ed structure, is `uninitialized` until Part F; the false alarms and the shapes with no
  verdict in the time limit that the audits measured; two tests set aside under
  `internal/pending/m4.5e1e2c/` (question 357). Question 343's exit 3 stays listed until M4.5e2 (the owner).
- **The box:** the lanes `~/cerastes-e1e2/{base,il,edg,work,audit}` (containers `cerastes-e1e2-<lane>`, image
  `cerastes-e1d:26.04`) are reused by the next session; `base` holds the baseline at `493a923` (the next
  session's Step 0 takes a new one at its own base), `edg` and `audit` the final build of Part D, `il` the
  Part E draft's build. The check lane `~/cerastes-s8/p-ubuntu-26.04` sits detached at `86442ce`, with this session's
  `check-e1e2c-c4.log`, `.exit` and `.start` beside the earlier sessions' logs. The box was under other
  users' load (40 to 75) for part of the session; times measured there are upper bounds.
- **CI:** unchanged (question 176 the owner's; the last scheduled run on `main`, 2026-11-25, failed after
  6 h 02, recorded only). **vlibc** `6e41fbe`, **vlibm** `1e6982e`, the website and **the EDG pin**
  `f0ed5525af` are unchanged. The Mac's disk had 24 GiB free (98% used) during the session.

## 2. Owner steps

1. **Push `main`, then merge and push this session's branch; recommended now.** `origin/main` (`f07d80a`)
   still accepts the unsound `restrict` forms; local `main` (`493a923`) does not. The branch
   `m4.5e1e2c-arrays-of-structures` is a fast-forward of local `main`: `git merge --ff-only
   m4.5e1e2c-arrays-of-structures` on `main`, then push (never reset or rebase). The full check is green at Commit 4 `86442ce`; the commit after it
   changes no code.
2. **Answer, when convenient (none blocks the next session; §8, "Questions for the owner"):** 354 (the
   `calloc` limit), 357 (the denied lint edit and the two tests set aside), 348 (where the rejection for
   structures whose array member holds structures is lifted).
3. **Review, when convenient:** the advisor's rulings of this session, 348–357 (§8), and the deviations in
   `internal/status.md`; the earlier ones were let stand by the owner on 2026-11-26.
4. **Start the next session:** `claude --agent orchestrator --effort high`, first message `M4.5e1e2
   continuation, from Part E (.claude/plans/m4-5e1e2b-arrays-of-structures.md)`. It runs from the same plan,
   without a planning: a new branch `m4.5e1e2d-arrays-of-structures` from the tip of
   `m4.5e1e2c-arrays-of-structures` (or from `main` once that is merged), the worktree
   `.claude/worktrees/m4.5e1e2d`, the session directory `build/m4.5e1e2d/`, the same lanes. It runs the
   plan's Step 0 again on the new base (the worktree with submodules and build, the lanes, the baseline, R1
   and R3; no P84), copies this session's reports from `.claude/worktrees/m4.5e1e2c/build/m4.5e1e2c/` to its
   `prev/`, and goes on from Part E's line in the plan's section 7, "Order of work". Its questions start at
   358. Record 0064's items 13 and 14 are "decided, built by the next session". §3, "From the second session
   of M4.5e1e2's continuation", holds what it must know about Part E's encoder draft before it briefs
   anyone.
5. **Remove, when convenient:** the earlier sessions' worktrees and merged branches (as before:
   `.claude/worktrees/m4.5e1e2`, `m4.5e1e2-audit`, `m4.5e1e2b`, the refs `refs/tmp/e1e2-*` and
   `refs/tmp/e1e2b-*`); this session's `.claude/worktrees/m4.5e1e2c-audit` and `.claude/worktrees/m4.5e1e2c-e`
   (the Part E draft's worktree: its content is on the draft branch) and the refs `refs/tmp/e1e2c-*`. **Keep
   `.claude/worktrees/m4.5e1e2c/build/m4.5e1e2c/` until the next session's Step 0 has copied it** (every
   agent report, the audit reports and the auditors' probes under `audit-{D1,D2,D2b}/`, the briefings as
   sent, the orchestrator's state log). On the box, `~/cerastes-e1d/` and `~/cerastes-e1e1/` may go.
6. **CI** (question 176): unchanged.

## 3. Open notes, by the session that consumes them

One line per note, its origin in brackets (session and audit item); the full wording is in the commit named above.
Notes that a session consumes move into its plan's "Inputs".

**From the second session of M4.5e1e2's continuation** (2026-11-26; record 0064 item 15; the reports are in
`.claude/worktrees/m4.5e1e2c/build/m4.5e1e2c/`: `auditor-report-{D1,D2}.md` with probes under
`audit-{D1,D2}/`, the agents' reports, the briefings as sent, the state log):
- **The next session (Parts E and F), beyond the plan's section 13 and the list of the session before (kept
  below):**
  - **An encoder draft of Part E exists and is NOT proved end to end and NOT audited.** It is the branch
    `m4.5e1e2c-part-e-encoder-draft` (`2ff1f39`, one commit on top of this session's Commit 4 `86442ce`: `src/backend/encode.cpp`
    +1,812 −162 against Part D's, with Part D's `calloc` cap carried in, and its `internal/encoding.md`
    paragraph). What stands behind it: it builds; with it no existing program's Viper changes and every Part
    D output is identical; no pin became `<verified>`; 82 + 49 single-file probes in both modes reach
    `check.cpp`'s refusal (nothing of Part E is accepted until the lowering exists). Its VIR shapes were read
    from a trace in C23 only. Reports: `edg-backend-report-{3,4,5,9}.md`. The next session may build on it
    or start again from the plan; either way its edg-backend re-reads it against record 0064 items 13, 2,
    5, 6, 9 and 12 before the lowering is briefed.
  - **The plan's premise "`check.cpp` refuses each construct until the lowering is joined" is not true of
    every gate.** In Part D five positions verified on the encoder half alone. For Part E the list is
    `edg-backend-report-3.md`: every gate has a refusal behind it except (1) a typedef'd or computed array of
    structures with no array member, (2) type operands (`sizeof`, `_Alignof`, a static assertion) and typedef
    declarations naming a matrix or an array of aggregates, (3) an uncalled bodiless declaration with a row
    parameter. In the draft (1) and (2) are CLOSED (the flag `kOpenComputedAggregateTypes` is false; one
    condition in `array_vtype`) and are opened only when the Part E il-analyst exists; (3) is lifted.
  - **Two enumerations of a structure value's cells must become one** (record 0064 item 12, question 353):
    the draft's encoder walks `expand_value_cells`; `vir.cpp`'s value view (`struct_value_leaves`) leaves a
    matrix member unexpanded and gives an element structure's array leaf two cells with one (path, element)
    key; the lowering's `param_vars`, `result_vars`, `elem_value` and `leaf_span` read the latter. Until
    there is one definition `check.cpp` must refuse a structure value of depth 2 or more. Round E's list
    gains it, with the by-value test (a distinct value per cell, each read back; a swapped twin).
  - `Builder::elem_addr` types an `ElemAddr` over `Ptr(Array)` as the pointer to the element, wrong for a row
    parameter's `a[i]`; the draft works around it in the encoder (`elem_addr_of`, a `const_cast`): the Part E
    il-analyst fixes the Builder and the workaround goes.
  - **The draft accepts shapes outside the plan's section 8 list** (designated lists for a matrix member,
    matrices of enumerations, of `bool` and of pointers, depth-3 members): each gets its test pair or stays
    refused with a pin; the test cap for Part E (37) goes to the advisor at Part E's briefing.
  - Rulings for Part E already taken: 352 (a `const` row parameter stays rejected, M4.6, a pin row per
    spelling, `const row *a` included); 353 (whole copies of the new structure types cell by cell, limit 7
    cells). Pin rows owed: `int (*restrict a)[3]` draws the `restrict` rejection, `int *restrict a[restrict
    2]` draws `unsupported_type` at the function line.
  - **Question 348 applies to Part E's types:** a structure with a matrix member, or with a member array of
    structures, stays rejected at Part D's six positions (the predicate `vir_type_has_aggregate_array`
    counts an array of arrays; probed on the draft).
  - **Part D's lowering took four il-analyst instances** (1,650 lines of `lower.cpp` in 3 h 20): brief each
    instance with ONE column of the part's table and an order, each step proved and snapshotted before the
    next; an instance that changes text an older program can reach must gate it (the first cut of the heap
    array place changed three existing self-test cases until `array_block_possible` gated it).
  - **Audits in two rounds per part read more than one round did**: round D1 on the columns built first,
    round D2 on the rest with its diff sized to be read in full.
  - Mechanics: a snapshot for a lane is composed with a temporary index from snapshot refs, never from the
    worktree while an il-analyst edits it (`git read-tree <encoder ref>`, `git rm -r --cached -q -f src/vir
    src/viper`, `git read-tree --prefix=src/vir/ <lowering ref>:src/vir`, the same for `src/viper`, `git
    add -A tests doc scripts internal plan.md`), and checked by `git diff --name-only` against each source
    ref before it is shipped; an auditor's report is not on disk when it returns: extract it from the
    agent's transcript with a script (do not read the transcript); the harness reminder that asks for a
    Claude trailer and a session line on commits is overridden by `CLAUDE.md`.
  - **Should-fix items the audits left, and where they go** (each fail closed): a guard on a written atom
    that reads an address-taken local by name is accepted and lowered as a real read, although record item
    15's first wording refused every memory-reading guard (the short read D2b, finding 1): the shared
    predicate `vir::reads_heap` does not count a `Local` whose address is taken; changing it reaches other
    records' rules, so it was left, with the record saying what is built (M4.5e2, with the written `reads`
    forms). An "internal check" exit 3 on a recursive function with a heap-read atom and no `p->q ==
    old(p->q)` (audit D1, note 7; the same on the base build: `build/m4.5e1e2c/audit-D1/h04b_decreases_load_cell.c`):
    M4.8's exit-3 list. `owns(r->a)` on a bodiless function's result is rejected by 0052 item 2a's check
    although 2a allows a place reached through the result (over-strict; M4.8). The on-owner erase of written
    atoms covers cell entries only (`lower.cpp`, the loop over `wats`, not `wars`; audit D2, note 10: the
    probe behaves; no test).
  - **Accepted shapes whose Viper the lint refuses, none held by a suite test** (a test of one turns the
    check red on the lint, not on the verifier; beyond the four classes listed below for the session
    before): (e) the partner-sum amount on an array part reached through a heap read, two atoms of one kind
    through two loaded pointers (question 357: the edit that would admit it was denied by the permission
    system; `internal/pending/m4.5e1e2c/`); (f) two owning globals of such structures in one loop (a carrier
    row's guard is a two-level conditional load: audit D2's `o02_swap`); (g) an owning global of a
    structure that holds such a structure by value (`c2_owner_byvalue`); (h) a written atom under a guard
    that reads an address-taken local. The lint is no backstop for a written atom's guard lowered to a
    constant (audit D2b, note 4): the checker's refusal and the lowering's internal error are.
  - **The lint's rules for Part D** are in `scripts/lint_viper.py` with their twins in
    `scripts/test_lint_viper.py` (classes `PartDHeapViewTests`, `PartDBlockClauseTests`,
    `PartDEmittedOutputTests`, `PartDLoopPlaceTests`, `PartDLoopCarrierTests`, `PartDViewsTests`,
    `PartDWrittenPartTests`); what each does not check is in the test-mutators' reports 3, 4, 6, 7, 9 and 12.
  - Costs to shape tests by: a loop that reassigns an owning global's block took 75 to 78 s per harness call
    (both modes) on the box, the false-alarm pin of the reassigned owning member 69 s per mode; `calloc` of
    a structure near the cap about 21 s; a written atom with a moved pointer 28 to 35 s.
- **M4.5e2** (with the earlier list below): the quantified zero line of `calloc`'s contract, which lifts the
  cap of 24 (question 354); Part D's six positions for a structure whose array member holds structures or
  arrays (question 348, unless the owner places it elsewhere); written `reads` atoms, a pointer read through
  memory and memory-reading guards on written atoms in loop invariants; a loop invariant that reads an
  element of an allocated structure or of a moved written atom (a false `permission` today).
- **M4.8** (with the earlier list below): the false alarms of record 0064 item 15's "Owners in loops" (a
  block the loop gives an owner reads `uninitialized`; an owning member's block freed and allocated again:
  six rows, also on the base; a marked member's block replaced; two owning globals swapped; a callee with a
  heap-read atom called in a loop) and its shapes with no verdict in the time limit; the member atom printed
  `&p->a` in some messages.

**From the first session of M4.5e1e2's continuation** (record 0064; the reports are in
`.claude/worktrees/m4.5e1e2b/build/m4.5e1e2b/`: `auditor-report-{R,C1,C2,C2b}.md`, the agents' reports, the state
log):
- **The next session (Parts D, E, F), beyond its plan's section 13:**
  - **The sequencing rows have nothing behind them.** For a call that holds a range over structures, a member
    array or a global array of structures, the `unsequenced` rows are the only check (with the rows made
    `assert true`, Silicon verifies the overlap: audit round C1, finding 1, replayed for Part C2's storages).
    They are pinned by C tests and self-test cases per storage; **each new storage or depth of Parts D and E
    needs its own pin pair** (`*_seq.c`, `negative/*_seq_unsequenced.c`) and so do `f__wf`'s ghosts
    (`*_wf.c`, `negative/*_wf_division.c`).
  - `apply_frame` and `contract_read` skip every site whose pointee is a structure (round C1's fixes), so a
    `pure` application or a `forall` beside a range over structures is framed by scalar sites only; Part F,
    which puts members under the binder, starts from that.
  - The length facts of a global array of structures are clauses of their own after the global facts' clause
    (`global_fact_clauses()` in `lower.cpp`; the lint expects that shape).
  - A `must_init` filler over a range of structures that fills with a loop is a false alarm until Part F's
    written quantified invariants.
  - Part E: a pin row for `int (*restrict a)[3]` and `int *restrict a[restrict 2]` (audit round R, note 8);
    an array of structures declared through a `typedef` of the array type, or as a type operand, is still
    rejected and goes with item 13's computed types.
  - Costs to shape tests by: each structure local with a member array of structures adds about 45 s to its
    function on the loaded box (six in one `main`: no verdict in 300 s); two `reads_range` over one structure
    type in a function with a loop took 81 s, and its error twin got no verdict in 560 s.
  - Mechanics that cost time: a snapshot composed with a temporary index needs `git rm -r --cached -q -f`
    before `git read-tree --prefix` (without `-f` it fails and leaves the old tree); never pass `--help` to
    `cerastes-vir-selftest` (it writes its output into a directory of that name); the harness cannot pin an
    exit 3; an agent that waits on a ref another agent is to cut should be given the recipe to cut it.
  - **From round C2b, left alone so that Commit 3's `src/` stayed the audited snapshot:** a stale name in a
    comment (`src/vir/lower.cpp` about 8917 says `global_fact_quantifiers`; the function is
    `global_fact_clauses`); the binder rule's second step (`k1` to `k2`, record 0064 item 3) has no case and
    cannot be reached until Part E accepts an element structure with an array member: Part E adds the case
    and gives `global_facts` its nesting test.
  - **The lint still refuses four classes of accepted shapes that no suite test holds** (round C2b, finding
    4; `build/m4.5e1e2b/audit-C2b/lint-keep-session.txt`, 71 problems over its probes): (a) `assert PRIOR ==>
    …` rows whose prior holds the well-formedness row of a GLOBAL structure with a member array of
    structures over `loc(GB_gt(), 0)` (`struct_wf_member_array_row` takes its pointer by an identifier
    pattern); (b) "the length facts are not a conjunction" for a `struct T *` read from an element; (c)
    `program_start` rows of an array global whose pointer leaf has an unknown value; (d) loop invariants
    beside a local array of `struct T *`. A test of such a shape turns the check red on the lint, not on the
    verifier: give the lint its rule with the test.
  - **Question 346's two forms touch the code Part D works in** (`encode.cpp`'s `object_value`, about 15686
    to 15737, and the `old` path about 12475): in C `fp(gt.v[1])` is rejected because the front end folds
    the element's address into a constant (`ck_address` with a subobject path) that `object_value` refuses,
    while C++ keeps the subscript and is accepted; `cerastes_old(gt.v[e].x)` is rejected. Which session
    lifts them is not decided (§8).
  - **Question 347's pair, for whoever revisits the global facts' clauses:**
    `build/m4.5e1e2b/audit-C2b/t/q05_reads_gt_nonmain.c` (17 s with the length quantifier inside the facts'
    conjunction, no verdict in 200 s with clauses of its own) and `build/m4.5e1e2b/audit-C2b/e/e06_split_gt.c`
    (no verdict in 200 s either way), with round C2's `m02` (`audit-C2/m/`).
  - **`internal/encoding.md` does not yet describe two things Commit 3 built last** (edg-backend's file; its
    last update predates both): the global facts' length quantifiers as clauses of their own
    (`global_fact_clauses()`) and the binder rule for a length quantifier under another quantifier
    (`length_binder`; record 0064 item 3). The next session's edg-backend adds them with its first part.
- **M4.5e2:** `&g[e]` and `&p[e]` in a contract; an atom on one element of an array of structures
  (`owns(g[1].x)`); question 343's row, if the owner places it there (§8).
- **M4.5f3:** a contract read of an element of a structure passed by value (`t.v[1].y` of a parameter `struct
  path t`, `f().v[i]`), with array values.
- **M4.8:** `f__wf`'s missing tie between the two states of a member array's element, for scalars (record 0063
  item 7) and structures alike (question 345; pinned); loop reach by field through a member array (a loop
  that writes `t.v[i].x` loses `v[k].y` and the structure's own `int` cells); the two costs above; a contract
  that owns one object twice and says so verifies vacuously (`owns_range(p, n) && owns_range(q, n) && p == q`,
  also the cell and scalar-range forms, on the base too: audit round C1, note 12; no caller can meet it);
  `!!&p[1]` in an `ensures` draws two rejection rows where one would do (round C1, note 14); an initializer
  list of about a thousand two-leaf structure elements gets no verdict (Silicon's own stack overflows on
  `program_start`'s clauses; exit 3; 300 elements verify in 29 s: round C2, note 5); the cost of a function
  that holds a global structure with a member array of structures and calls another (question 347; round
  C2's note 6: a five-leaf check after one call, no verdict in 560 s); three messages not in source terms
  (`(&gt.v[0])[1].x`; "passes `gt.v[0].y` and a range to `gq`"; a write under `reads` said to be an object
  that "may have ended": round C2, note 7); the cross-state row of
  `negative/struct_member_array_struct_wf_division.c`, dropped until the tie exists.
- **M8:** question 342's family, which question 322's rule rejects although no compiler flags it (a truth
  test of an element address written by a macro, `assert(&a[1] != NULL)`; in C `&a[1] == (int *)0`); the
  address tests without an index (`&s.x == NULL`, `if (&x)`: audit round R, note 5; accepted, the compilers
  warn); the lone zero the IL cannot tell from a literal (`{(0)}`, `{__extension__ 0}`, designated and nested
  zeros: question 339; accepted, clang reports them); `restrict` in the type of a cast or of a compound
  literal (`(int *restrict)p`: accepted, promises nothing by C23 6.7.3.1; with the model of `restrict`); the
  `unsupported_annotation` message's tail cites decision 0037 as well as 0064 for the `restrict` rejection
  (one tail per tag in `diagnostics.def`).

**From M4.5e1e2's first session** (record 0064). **What the continuation needs has moved into its plan**
(`.claude/plans/m4-5e1e2b-arrays-of-structures.md`, section 13, with the should-fix items its audits left placed
in Parts C1 and C2); the reports are in `.claude/worktrees/m4.5e1e2/build/m4.5e1e2/`, which the continuation's
Step 0 copies. What stays here is for other sessions:
- **M4.8:** loop reach by field for arrays of structures (question 318; pinned
  `array_struct_loop_keep_false_alarm.c`); the cost of several arrays of structures in one function (question
  328: 1 to 5 arrays of two-leaf structures in one function take 7, 11, 17, 36 and 65 s on the box, and five mixed
  ones timed out); an element structure with an `owning` or marked member; `uninit` on an array of aggregates (so
  that `--strict-init` can hold a local array of structures without an initializer); a structure value stored
  into an element (`v[i] = f();`, question 333); an application over a pointer into an array of structures in a
  loop invariant (a false alarm: `inv_interval_frame` skips leaf places); `&p[0]` in a truth test, rejected
  though GCC is silent on it (question 335).
- **M4.6:** what a library declaration with `restrict` parameters (`memcpy`) needs, since every `restrict`
  parameter is rejected (question 332).
- **M8:** a model of `restrict` (question 326; `plan.md`'s M8 entry), with `-Wrestrict` (GCC flags `f(a, a)` at
  `-Wall`; no rule) and the reassigned parameter (`p = q;` then accesses through `p`: the compilers measured
  agree with the model, and C23 6.7.3.1 is arguable on it); the lone zero that is not a literal (question 325),
  where the IL cannot tell it (the continuation's probe P84 (a): the parenthesized and nested forms; above).
- **Any session that writes tests over arrays of structures:** two-leaf structures and at most two such arrays
  in one function; several `assert false` in one vacuity twin only if no path passes two of them (Silicon stops
  a path at its first failing assert); `box_lane.sh ship` leaves untracked files in a lane (`git status --short`
  there must be empty before a joined proof), and a snapshot name must be fresh for every ship (a re-used name
  fails silently on a non-fast-forward fetch).

**From M4.5e1e1** (record 0063; its plan's sections 14 and 15; the audit reports `build/m4.5e1e1/auditor-report-*.md`
go with the session's worktree):
- **M4.5e1e2:** moved into its plan (`.claude/plans/m4-5e1e2-arrays-of-structures.md`, sections 1 and 14): the six
  leftovers of a structure with an array member, and member and global arrays under a quantifier.
- **M4.5e2:** part of a global array or an array member in a contract (`owns_range(g, n)`, `owns_range(p->a, n)`,
  `&g[2]`), atoms on one element, `g + i`, `int *gp = s.a;` and `&g[k]` as global initializers.
- **M4.5e1f:** the first-slot amount of an array leaf or of `owns(g)` (question 238's limit applies to them).
- **M4.5f3:** structure values with more than 7 array elements by value (question 309: the worst by-value shape took
  19 s at 6 elements, 25 s at 7, 35 s at 8, 129 s at 12, over 120 s at 16, on a quiet box lane — about 1.4 times
  per element, not diagnosed; 64 timed out at 600 s); one constant `vir::kMaxByValueArrayElements` and three tests
  to raise it.
- **M4.8:** the listed false alarms of record 0063 (a store by name to any member of a structure local reaches every
  array place of its field, items 10 and 0043 item 11; `f__wf` does not tie a member element's two states; a
  `static` structure local's implicit atom does not cover a partnerless atom through an array leaf; the copy's guard
  between two `owns` places whose leaves of the field are all array members); arrays of owning pointers; the
  `unsequenced_calls` message wording for a write that is not a call (audit 2c, N1); the lint's `bool` array of
  unknown value with no listed element (audit 2, N4).
- **M8:** array designators, VLAs, zero-length and flexible members, pointers to arrays (`&(int[]){…}`), `[static
  N]`, `thread_local` and block-scope `extern` arrays, `__datasizeof`, `_Alignof` of an expression, C++ array
  literals; a compound literal whose element writes in a position other than a whole statement, initializer or list
  element (question 308's over-strictness); an unused block-scope array typedef (question 302, the warning sweep).

**From M4.5e1d** (record 0062; its plan's section 13):
- **M4.5e2:** applications under a quantifier's binder and a `cerastes_forall` naming a function with a contract
  (questions 265, 266); `old` of an application; pointer-argument forms beyond a parameter, a pointer variable or a
  local array (`&a[1]`, `p + 1`); an application over a range of pointer elements in a `check` or a loop invariant
  (R3-S2's rejection); **R4-S5** (question 287, placed here by the owner: a loop that fills a `must_init` range
  while its invariant applies a pure function over the part not yet filled times out, exit 3, instead of
  reporting `uninitialized`; fail closed; il-analyst-3's report 8 found the `uninitialized` row itself fails in
  11 s and the time is Silicon going on past it; fixed with range atoms in loop invariants and the loop's flag
  rows).
- **M4.5f1:** a `cerastes_forall` inside a called `pure` body, calls in a `decreases`, the atoms a `pure` function's
  contract refuses (`owns`, `owns_range`, `live`, conditional atoms; any atom in its `ensures`), pure functions over
  predicates.
- **M4.5f2:** mutual recursion between `pure` functions and cycles through contracts.
- **M4.8:** item 8's listed limits (the definition of `g` unknown in `f__wf`, a callee known only through its
  `ensures`, question 282's pin), question 279's two facts (fact (i) for an enumeration result, fact (ii) for a callee
  whose `requires` holds an application), question 267's missing range fact outside `f__wf`, a marked range framing
  no application (item 7), one site holding a callee's whole interval (item 7).
- **M6:** a `pure` function with no body in the unit (question 269, the contract manifest).
- **Any session:** the false facts over sums of 3–4 elements take 226–356 s of Z3 on the box against about 30 s on the
  Mac (`tests/long/memory/negative/pure_sum_caller_write_between.c`, `pure_check_local_false_fact.c`, question 285);
  a cheaper unfolding at a `check` would be an encoding change (record 0062 item 7).

**M4.8, from M4.5e1c** (record 0061 item 1; question 249: "M4.8 must handle them"): `forall` outside a positive
position (nested, under `!`, as an operand of `==` or arithmetic, in a `?:` condition, in an atom's guard, in a measure,
under `old`, in a `do … while (0)` invariant, an `exists`); on the left of `||` (question 255); in a loop invariant or a
`check`, under `||` or in a `?:` arm (question 262: lower them as contract clauses are lowered).

**M4.8 C parity close-out, from M4.5d2b2b and M4.5d2b2c's planning** (with questions 66 and 189):
- N2: a `static` cache cannot hand out a pointer into the buffer its leaf owns (0054 item 7's cover does not reach
  blocks owned by a static leaf; `cov_of` covers only by implicit permission atoms). The cover already timed out
  once (record 0055 item 3), so it goes with question 194 (owner, "Follow recommendation": the cover rule stays,
  record 0055 item 3, "Covered atoms"; a function that returns `&s` on some paths and a fresh structure whose owning
  field holds a block on others is reported; revisit with the cost work if a ported example needs it).
  [d2b2b audit; d2b2c planning]
- Automatic closure sites for a borrowed structure's owning fields in a loop (question 201: not built; a loop that
  replaces such a block writes `owns(*p)` in its invariant). They would change existing loops' Viper and give a
  structure with two owning fields the 218 s shape; revisit with the cost work. [d2b2c planning]
- **From M4.5d2b2d** (record 0058):
  - **The older exit 3** (question 222; audit re-audit S1, the same on the base): a permission atom read through a
    block site's cell is checked before that block's own rows, at returns (`atom_returns` before `block_returns`) and at
    calls (the atom rows before `block_call_rows`). Reorder, do not guard, as record 0058 item 4's "Return rows in
    order" did for block sites. Only a program that breaks its own contract reaches it:
    `struct N { int *c; int v; }; void drop(struct N *p) CERASTES_REQUIRES(cerastes_owns_block(p) &&
    cerastes_owns(*p->c)) CERASTES_ENSURES(cerastes_owns_block(p) && cerastes_owns(*p->c)) { free(p); }` exits 3 (a user
    `permission` is right); at a call, `use(p)` with the same bodiless contract after `free(p)` in a caller whose
    `requires` is the same and whose `ensures` is `cerastes_owns(*p->c)`. [d2b2d re-audit]
  - Pinned false alarms (record 0058 item 4, "Limits"): a loop calling a `ref_to_uninit` callee on a struct local with
    an owning field (`marked_pointee_loop_call_false_alarm.c`, 0056's current-value limit); a written invariant site
    over a marked field's unwritten buffer (`marked_field_loop_written_invariant.c`); a written `owns_block(p->q)` loop
    invariant on a marked owning field closes below it (`closure_marked` is read for contracts only). [d2b2d]
  - Three lint shapes of the marked closure (a `requires` site without a bare flag conjunct, no flag asserts in the
    Uninit modes, a given site without flags) need the site's mode, which the Viper does not name. Question 225
    ("Follow recommendation."): here, by a mode comment emitted beside each block site (no Viper semantics change),
    with `lint_viper.py` reading it. [d2b2d]
- **From M4.5d2b2c** (record 0056; none cut a limit, so nothing of its cut order is here):
  - **Question 196's two slow programs** (the owner: "Increase time past 300s, if the increase does not allow the
    program to pass, then move to M4.8"; question 199: 1800 s, once each on the quiet box). Both ran past 1800 s in
    both modes: `cover_twice.c` (two calls of a function returning `&s` of a `static` struct with an owning field,
    then frees through both, a failing program; exit 3 at 1801 s, the base encoder) and `two_fields.c` with its twin
    (two owning fields replaced in one loop under written invariants; 1801 s, Part L-2's build; its Viper is the
    same from the base), recreated in `build/m4.5d2b2c/slow/` of the session worktree and on the box in
    `~/cerastes-s8/p-ubuntu-26.04/build/slow-d2b2c/`. S40 (g)'s lost `g3_two.c` took 218.5 s on a loaded Mac (one
    field 6.3 s), so the two programs may differ in detail. They belong with the cost work (questions 66, 189).
    [d2b2b, d2b2c]
  - The places test of record 0056 item 6 (question 212: "Keep as built"; the recommendation was to narrow it): a
    site's cells are places only where the loop writes neither the site's pointer nor, by name, any address-taken
    local; pins
    `owning_field_loop_chain_cell_write_false_alarm.c` (verified on the base) and
    `owning_field_loop_chain_null_owner_false_alarm.c`. Narrowing it, with the base's live-guard places where the
    pointer may be written, is the fix the auditor proposed. [d2b2c audit round 2 on L-2, B2]
  - Item 5's entry snapshot: a loop in `main` that hands a struct global to a filler through a pointer it holds
    (`pg = &g`) raises `held` while `hsl` is false (`owning_field_loop_main_alias_false_alarm.c`). [d2b2c audit round 1, S2]
  - The replace tests carry no closure across a possible replacement of an owning global pointer or a struct global
    reached through a pointer: six pinned false alarms (`owning_global_loop_replace_*`, `owning_global_leaf_replace_*`);
    the guarded written site of record 0056 item 8 may serve there, untested. [d2b2c]
  - The older `stores_type` clause of the `wef` rule (0054 item 3): a loop copying a whole structure that holds an
    `int` beside an owning pointer is a false `uninitialized` at the back edge
    (`owning_field_loop_struct_copy_int_false_alarm.c`, the same on the base). Question 216 (the recommendation):
    fixed here with question 212's places test, both refinements of 0054's carriers. [d2b2c]
  - `owning_field_loop_owner_replaced_alias_false_alarm.c` (question 201's fifth pin): the loop frees and replaces an
    owning local `v` through `pv = &v` and frees `v->data` in its first iteration only; the invariant it needs,
    `v == NULL || i != 0 || v->data == NULL || owns_block(v->data)`, is rejected for an address-taken owner
    (`unsupported_expression`) and is `permission` when spelled `(*pv)->data`. Question 215 (the recommendation):
    M4.8 admits a written block site on an address-taken owning local's field (record 0054 item 6's owner in
    memory), with the other loop false alarms; the pin stays until then. [d2b2c]
  - Item 2's call route has no pin (only a marked global's deep atom reaches it) and the whole-structure route is pinned
    in the VIR self-test only; both are the conservative direction. [d2b2c]
  - **Costs to watch:** six tests moved to `tests/long/memory/` (question 217; 120–641 s on the box, 20–84 s on the
    Mac); the slowest left in the default tier take 60–95 s on the box at twelve parallel runs
    (`owning_field_loop_global_pointer_written{,_drop}.c`, `_alias_written.c`, `_chain_drop.c`, `_chain.c`, the doc
    example `memory/owning_field_loop_alias.c`), and CI's PR tier will select them on the push of this branch (three
    cores; the driver's 300 s). [d2b2c]
  - Record 0056 item 6's value-rule rows ("… pointing to another object than the one it began with", "… holding
    something other than what it began with") are backstops: no test produces them (the auditor reached them only by
    injecting a write into the Viper); the Error messages page lists them. [d2b2c re-audit]
  - A loop that frames a struct local out still loses an `int` member's value when the loop reaches it otherwise (the
    re-audit's r3 and r5 had to return `s.data == NULL ? 1 : 0` instead of `s.n`): the same on the base. [d2b2c]

**From M4.5e1b, for later sessions** (record 0060; the audit reports `auditor-report-{1..4}.md` were in the session
worktree's `build/m4.5e1b/` and are gone with it; record 0060's header digests each round):
- M4.5e2: an atom on one element (236); a range atom written in a loop invariant (237); the assigned range parameter's
  false alarms (`range_loop_pin_assigned_param.c`: a read through `p = &p[1]` in a loop is `lifetime`; a call through a
  copy of the original pointer in such a loop is a false `permission`, audit round 4 `c/d2c.c`); ranges whose base is
  not a pointer parameter (results, globals, loaded pointers, members; M4.5e1e1 gives a global array and an array
  member their whole-array atoms, and a range on part of one stays here); counts and guards of range atoms that read
  memory; partnerless `ensures` ranges and `ensures` ranges with another count; `owning(n)` beyond parameters; `&p[e]`
  and `p + i` in contracts. [record 0060 item 1]
- M4.5e1f: questions 238 and 242, in the block below.
- M4.8: `owns_range` and `reads_range` on one parameter in one list, and a range beside a cell atom of the same
  parameter; audit round 4's S1 — a write through a `reads_range` element in a loop beside a second, overlapping
  read range: the user's `permission` at the write, then an exit 3 (the read-only place's values-kept conjunct
  after the failed write row is assumed; fix: a marked write row, which changes existing Viper; question 246); a
  pre-existing exit 3 on the baseline too: a pass-through of `owns(*pp) && owns(**pp)` without 0045 item 10's
  tie; the write row's message for a `reads` place says "the object it points to may have ended" (cells and
  ranges alike, audit round 4 (g)); the driver prints one error for rows sharing a line, category and message, so
  a dropped row in such a test cannot show (audit round 2 (d)); no twin isolates the `reads` cell's return sum
  term for a range (audit round 2 S4, round 3 (e): without it the range's sum-form row fails instead).
- M8: VLA parameters and `[static N]` (questions 230, 241); a local array shorter than `int a[4]`'s bound passed
  (silent at the front-end tiers; GCC `-Wstringop-overflow=` at `-O2`); `int a[0]` and `int a[const 3]` parameters
  (accepted, the compilers silent).
- **The three probes the notes above name** (they lived only in `build/m4.5e1b/`; each in C23):
  - audit round 4's S1 (the `permission` at the write, then an exit 3):
    `void f(int *p, int n, int *q, int m) requires(cerastes_reads_range(p, n) && cerastes_reads_range(q, m) && n > 0
    && m > 0) { int i = 0; invariant(0 <= i && i <= 1) decreases(1 - i) while (i < 1) { p[0] = 1; i = i + 1; } }`
    (the `CERASTES_` macro spellings);
  - question 237's copy (a false `permission` at the call): `peek` taking and returning
    `reads_range(p, n)`, and `int f(const int *p, int n) requires(cerastes_reads_range(p, n) && n > 2) { const int
    *q = p; p = &p[1]; int i = 0; invariant(0 <= i && i <= 1) decreases(1 - i) while (i < 1) { peek(q, n); i = i + 1;
    } return 0; }`;
  - the pre-existing exit 3: `void f2(int **pp)` with `requires(cerastes_owns(*pp) &&
    cerastes_owns(**pp))` and the same `ensures`, body `(void)pp;`, and `void ok2(int **pp)` with the same contract
    calling `f2(pp)`; adding `*pp == cerastes_old(*pp)` to both `ensures` (0045 item 10) verifies.

**M4.5e1f Range call rows** (owner, 2026-11-23; after e1c, from its spike S45 (r); `.claude/plans/m4-5e1-arrays.md`
section 10)
- **Question 238, relooked.** It is a false alarm, not unsoundness: `reads_range`'s amount at a call is taken from
  the range's first slot, lowered by the caller's own read amounts (record 0060 item 6), so it can only be too
  high for another slot, and every slot is still asserted at that amount (the marked rows, in the sum form where
  two read ranges meet): a correct program is rejected with `permission`, nothing wrong verifies (audit rounds
  1–3). It bites only a caller whose holding over the range is uneven for another reason: a callee kept a read
  fraction of one element and gave the liveness back (`requires(reads(*x)) ensures(live(x))`), then a later call
  reads the whole range, once or twice (`range_call_reads_unequal_pin.c`, `range_call_reads_two_atoms_pin.c`,
  `_local.c`). A local array cannot reach it (a fraction kept there is a real `lifetime` error).
- **The candidate fix** is the minimum over the range. `perm` cannot stand under a quantifier in an `assert` (S24),
  so the minimum is defined by a tagged `inhale` (a bound over the range and a witness slot; sound as a definition:
  a non-empty finite range has a minimum) and the existing rows are then asserted at it. It would be **the
  encoder's first `inhale`** (invariant 4: its own record and lint shape), and it changes the Viper of every
  existing `reads_range` call. M4.5e1c's spike S45 (r) records whether the pinned Silicon takes that form (or the
  same facts as a bodiless helper's `ensures`) and what the pin does with it; nothing is built from it in e1c.
- **Question 242, order D.** With the shape row first at a call (the built order), a callee forwarding `(p, n + 1)`
  of its own `owns_range` fails two Silicon rows, shown as one error. The order "marked rows, shape row, liveness
  row, amount" for `owns_range` gave one row in every twin of S44 (b) (`s44_b_*_orderD*`); a `reads_range` keeps
  its order (its amount needs the liveness cell first). The owner: built with 238's fix, in this session (the same
  call rows, one set of Viper flips).
- **If the spike finds no form Silicon takes:** the session builds order D alone, and the owner decides then
  whether 238 goes to M4.8's work on read amounts (question 66).
- **S45 (r)'s result (M4.5e1c, 2026-11-23; `internal/il-inventory.md` chapter M4.5e1c, "S45 (r)"; fixtures
  `tests/viper-smoke/memory/s45_r_*.vpr`): Silicon takes the form.** `perm(…)` under a quantifier in a guarded `inhale`
  type-checks and runs (no helper method needed). `range_call_reads_unequal_pin.c`'s kept Viper with `var mv, md: Perm`,
  `inhale (n > 0) ==> (forall k :: { loc(B, k) } in-range ==> m <= perm(loc(B, k).f))` and a witness `inhale (n > 0) ==>
  (in-range(w) && m == perm(loc(B, w).f))` per field, and the call's amount from `mv`, `md` (`/ 2`): **the pin
  verifies** (2.4 s; it fails today at the call's marked row). A slot held at `none` is one error (`s45_r_slot_none_fail`);
  `assert false` right after the `inhale`s is reported (consistent, `s45_r_inhales_consistent_fail`). **Order D**
  applied to every `owns_range` call of `range_call_fwd_one_more.c`'s kept Viper: one error where the built order gives
  two (`s45_r_orderD_fwd` against `s45_r_orderB_fwd_built`); its `shorter` and `tail` verify in both. So M4.5e1f can
  build both: its record argues the `inhale` as a definition of the minimum, with a lint shape, and expects one set of
  Viper flips over e1b's range calls.

**From M4.5e1a, for later sessions:**
- M4.8: a pointer local the loop reassigns has no loop place (`*pk` fails as for scalars, and `pk[i]` adds `lifetime` and
  `out-of-bounds`); three rows share the message "holding something other than what it began with" and no test isolates
  `flag_follows`' row (`lower.cpp` near 17943 at `ba3034b`); `split_candidates` splits an array cell against its first
  element only. [e1a; round 3 notes 7, 8]
- M8 (the warning sweep): `_Alignof` of a scalar expression is accepted although clang warns by default
  (`-Wgnu-alignof-expression`), as before e1a; `register` without `&` stays accepted. Array designators wait for an EDG
  change that keeps the designator's expression (question 234). [round 3, item 1; round 1, B1]

**M4.5e2 Pointer arithmetic** (needs e1a and e1b; e1e2 for arrays of structures on the heap)
- Signed/unsigned and enumeration variants through `void*` cannot be written [M4.5a]; accepted completeness gap for
  its relational comparisons: `&s.x < &s.y` is reported, since members are different blocks [M4.5a].
- `vlibc_realloc`, `vlibc_aligned_alloc`, `vlibc_free_sized`, `vlibc_free_aligned_sized` get contracts with their
  standard names'; `free_sized` needs a deallocation shape with a size [0050 item 6].
- **The `offset_of == 0` backstops rest on two facts M4.5e2 breaks** (audit round 3 and the re-audit): every heap
  block has one slot, and no store gives an owner an interior pointer. While both hold, the premise that 0059 item 10
  adds to `flag_follows`' user `permission` row and to the ending leak rows is redundant; with `malloc(n * sizeof(T))`
  and `p + i` an owner may hold `&p[1]`, and those rows (and the lint's loop-carry exemption, which accepts carried
  terms without the conjunct) must be re-argued, with tests, before M4.5e2 lands. [e1a rounds 3, 4]
- **From M4.5e1b's planning** (questions 236 and 237; e1b pins each, naming M4.5e2): an atom on one element in a
  contract (`owns(p[i])`, with `owns(*(p + i))`; `place_root` has no step for `ElemAddr`, and place keys, partners
  and loop places need one); a range atom written in a loop invariant, for a pointer the function assigns or a
  heap array held through a local (0040 item 5's written atoms over quantified permissions: no `perm` snapshot
  under a quantifier), with the pinned false alarm of a loop in a function that assigns its range parameter; a
  range whose base is `result`, a global, a member or a loaded pointer; a count that reads memory; a partnerless
  `ensures` range and one with another count than its `requires` partner; `&p[e]` and `p + i` in a contract.
  [e1b planning]
- From M4.5e1's plan: whether `*p` gains a bounds row (question 229: e1a reports a `*p` one past the end as
  `permission`); `owning(n)` on results, locals, fields and globals (e1b gives it to parameters only); the pins
  `reject_malloc_size_*.c`, `reject_calloc_size_count.c`, `reject_realloc.c`, `reject_struct_member_arith.c`,
  `global_address_arith_cpp.cpp`, `tests/vlibc/negative/reject_vlibc_realloc.c`; 0047 item 6's `offset_of(p) == 0`
  free row gets its C test (`free(p + 1)`). [e1 planning]

- **From M4.5e1c** (record 0061; questions 253, 259, 260): `cerastes_old` in loop invariants (cut 1, question 260: e1c's
  Part D; record 0061 item 12 has the design, P80 (f) the IL (the shapes of an `ensures`), S45 (g) the Viper (an
  unlabeled `old(…)` in a `while` invariant is the method's entry; an `OLDFRAME` obligation line; fixtures
  `s45_g_*`), audit round 1's N17 (exclude an address-taken parameter), Step 0 R2's five further shapes (a reassigned
  pointer parameter, a local array, a global, an assigned by-value parameter, `*p` through a reassigned pointer: each
  two errors today, each to get a message of its own); pins `old_loop_invariant_reject_{cell,elem,scalar}.c`, retired
  then); the `forall` index forms other than `i`, `i + c`, `i - c` and under a conversion; base forms (`&a[1]`, `p + 1`,
  a loaded or address-taken pointer); memory-reading trailing arguments and `cerastes_old` as one; the multi-class
  false alarm where only one class's term occurs (question 253, `forall_one_term_*`); question 259's narrowings (in a
  `check`, a read the binder does not index and pointer elements; in loops and checks, `*p` of an array argument and a
  pointer argument used as a value); question 255's multi-class guard that reads memory and arithmetic over two
  classes; question 261 (in loops and checks, arithmetic, a unary operator or a conversion over a read of a class other
  than the first, which lowering would route to the first class's view: an exit 3 before the refusal); audit round 4's
  notes: `carrays.c`'s line map has been off by one since e1b (VeriFast's lines 1, 10, 22 are the port's 11, 23, 39;
  Flips C left it unchanged), and a `check`'s marked row for a pointer base says "the object it points to may have
  ended" when a callee kept the element. Part D's tutorial example, for the arrays chapter's "in-place loop" section (its off-by-one variant passes
  `i + 1` for `i` to the first invariant's `cerastes_forall`: `invariant-entry` at that invariant):

  ```c
  // expect: verified
  // expect: native-ok
  // mode: c23, c++20
  // An in-place loop: cerastes_old in the invariant names each element's value when the function was entered.
  // tag::source[]
  #include <cerastes.h>
  
  CERASTES_PURE bool small(int k, const int *a, int n)
  {
    return !(0 <= k && k < n) || (0 <= a[k] && a[k] < 100);
  }
  CERASTES_PURE bool added_one(int k, const int *a, int n)
  {
    return !(0 <= k && k < n) || a[k] == cerastes_old(a[k]) + 1;
  }
  CERASTES_PURE bool unchanged_from(int k, const int *a, int n, int i)
  {
    return !(i <= k && k < n) || a[k] == cerastes_old(a[k]);
  }
  
  void inc_all(int *a, int n)
      CERASTES_REQUIRES(cerastes_owns_range(a, n) && n >= 0 && cerastes_forall(small, a, n))
      CERASTES_ENSURES(cerastes_owns_range(a, n) && cerastes_forall(added_one, a, n))
  {
    int i = 0;
    CERASTES_INVARIANT(0 <= i && i <= n && cerastes_forall(added_one, a, i))
    CERASTES_INVARIANT(cerastes_forall(unchanged_from, a, n, i))
    CERASTES_DECREASES(n - i)
    while (i < n) {
      a[i] = a[i] + 1;
      i = i + 1;
    }
  }
  // end::source[]
  ```

**M4.5f1/f2 Predicates**
- M4.5e1d takes the slice an exact array sum needs (question 227; its plan): read `pure` functions as Viper
  functions, their calls in annotations, self-recursion with `decreases`. M4.5f1 keeps predicates, `fold`/`unfold`
  and pure functions over predicates, and takes what e1d's plan leaves it (its section 1's table: a
  `cerastes_forall` inside a called `pure` body, calls in measures, the atoms and the reads a `pure` function's
  `requires` refuses, pointer and structure results). [e1 planning; e1d planning]
- For `predicate` bodies (M4.5f1): they reject globals and `static` locals (0041 item 12) [b2a]; R1–R13 and the
  static-assertion cross-check do not reach them [M4.4c] (e1d's plan, record 0062 item 3, says what reaches a read
  `pure` body; `static_assert_pure_body.c` is unchanged while its function is unread).
- A list traversal's truth tests need liveness inside f2's predicates; `forperm` sees only top-level tokens [M4.5a].

**M4.6 Chars and strings**
- The SDK's `__DARWIN_ALIAS(sym)` asm labels (`_stdlib.h:189`, `_string.h:95`, `strtod`, `strerror`) and glibc's C23
  `__isoc23_*` redirects: record 0046 item 8 refuses every trusted contract on such a function; a record must tell the
  platform's own labels from a user's before vlibc covers them [d1 audit 2 note 7]. vlibc's first body in a header
  re-examines EDG's discretionary errors in system files (record 0046 item 3).
- `main(int, char **)` in a unit with globals is not the entry and fails closed, but its message asks for `reads(g)` in a
  contract on `main`, which is rejected; M4.6 makes `main(argc, argv)` the entry [b2a audit 2 N3].
- `const` objects as cells (question 78, §4.3); `c | 0x20` case bits (0027).
- **vlibc's core** is written to be verified after M4.6 (arrays, pointer arithmetic, char storage); the glue stays trusted.
  vlibc's run test of an allocating SIGABRT handler calls a function that is not async-signal-safe (test-only) [d2v].

**M4.7 Function pointers:** keep `&main` rejected, so `program_start` is reached once [b2a audit 1 N10]; a marker in a
function-pointer typedef (`typedef void (*fp)(int *p CERASTES_MUST_INIT);`) reaches no function today and must be read
(or rejected) when calls through pointers arrive [b2b audit 2 S4].

**M4.8 C parity close-out**
- Exit 3 beside a correct error, pre-existing (reproduced on the baseline `7f11706`): in a function WITH a body, an `ensures`
  atom whose guard differs from its `requires` partner's (`requires(owns(*p) && (c == 0 || owns(*q)))`,
  `ensures(owns(*p) && (c != 0 || owns(*q)))`) is reported by `f__wf`'s partner row (`permission`), and the return's
  internal row "a pointer parameter points into its block" also fails (exit 3); guard that row by the partner's path, or
  make it a `permission` row [c2b test-mutator; the c2b pins use bodiless functions].
- The cost of aliasing `reads`/`live` parameters (question 66): the cell splits at calls were checked by timing, not
  re-audited (four and five parameters 5.7 s and 22 s, six time out with exit 3, documented) [b1b]; entry case splits
  multiply loop verification (one guarded parameter and one cell 2.7 s, two and three 37 s; four aliasing `reads` with a
  loop about 50 s under load); tests stay at three or fewer [b1c1]; the slowest b1c2 tests take 9–14 s under load 150–175
  [b1c2].
- Written atoms on moved pointers (question 79): a written atom whose pointer the loop moves keeps its entry liveness
  amount, a pinned false alarm (`tests/memory/negative/loop_written_moved_owns_false_alarm.c`) [b1c3 round 2 S4].
- A loop over `reads(*p)` and a global written by name may need intermediate asserts for Silicon to keep `*p`'s value
  (prover incompleteness, not the encoding) [b2a audit 2 N2].
- Exit-3 hazard, no case known: Silicon continuing past a failed explicit check and tripping over its own check on a
  store through an unknown pointer (found once in `main`, fixed by splitting against every held global in
  `result_splits`; no general guard) [b2a].
- Mutation: the mask positives were never mutated (question 49), nor the verified doc examples (question 10) [M4.4d];
  the markers' expected mutants are a dropped marker and `must_init`/`ref_to_uninit` swapped; the `g_r` guard of
  `ref_to_uninit`'s kept-flag row is never exercised by a test (`f__wf` reports the shapes that would need it first)
  [b2b audit 1 N8, audit 2 N5].
- From the mutation review: the doc example `verifying/void_conditional.c` keeps a redundant `requires` conjunct (four
  listed equivalents); a body `p | q` against an exact `&` contract runs to the timeout (0023's limit), so
  `not_eq_and_bools` states its contract with `&&` [M4.4c].
- Masks that are constants only through a call argument (`f(v, 4294967295u)` with `requires((v & m) == v)`): Silicon's own
  precondition check may run to the timeout (exit 3; question 51); the exact-set fallback keeps today's false alarms
  (question 50) [M4.4d].
- **Costs to watch:** two written `owns_block` sites through loads over parameter places cost about 33 s for the loop
  alone and about 148 s with one drop in the body, quiet. The tests pin cheaper shapes (question 186). The `?:`
  framing chains repeat in every pair row and every `forperm` instantiation; the reads are hoisted, and the next step
  is the pair and exclusivity structure. Question 189 (owner, 2026-11-09): this cost is fixed here with question 66;
  owning fields make the shape more common (a structure with two owning fields under a written loop invariant: the
  218.5 s and 1800 s programs above). [d2b2a, d2b2b planning]
- **Record 0054's documented limits** (its items 2–5; the twelve bullets are in
  `git show 2430529:internal/handoff.md` §3 "From M4.5d2b2a" and in M4.5d2b2b's plan's Inputs): candidates for the
  close-out's review of false alarms. [d2b2a]
- **Costs to watch:** B1's splits against outside holders grow the paths exponentially with the number of holders (S37's
  adversarial fixture 93 s); a function with several pointer variables beside a heap loop may time out [audit 2 N10].

**M5.0 and M5** (also §4.4)
- `mutate.py`'s classifier does not read local declarations (about 20 listed equivalents rest on a reviewer's reading),
  before M5a; a user C++ function named `assert` would be bucketed by `mutate.py` [S3 note 1].
- Templates [M2 audit B]: a predicate called inside a `forall` lambda in a function template's contract is falsely
  rejected (`spec_call_in_code`); the `operator=` branch of `spec_marker_on_special_member` lost its test; P changes
  overload choice where constant evaluation is a substitution failure (two tests to add); 2–3 tests for the back end's
  constant-context pass in never-instantiated templates; a class-scope `static_assert(q(sizeof(T)))` in a
  never-instantiated class template is accepted (harmless); EDG at P leaves an error node without a diagnostic for a
  `decltype(p)` lambda parameter (`annotation_substitution_error` is load-bearing, M5b fork commit R).
- A closed 128-bit claim over a shifted constant other than 1 is undecided (folding `k * c_pow2(n)` would change
  existing Viper) [M4.4b1 note 6].

**M7 C++ memory and classes**
- **For M7:** vlibc's allocations are rejected in C++ ([intro.object]/10–13 create objects only for `std::malloc` and its
  family); placement `new` (and C++23's `start_lifetime_as`) lift it [0050 item 7].

**M8 Inventory** (the warning sweep is an M8 entry in `plan.md`)
- From M4.5c2a's planning (question 100): struct `?:`, comma and assignment values (`t = c ? s : u;`, `t = (f(), s);`,
  `(t = f()).x`) stay rejected, over-strict [c2a planning].
- From M4.5c2a (record 0044's M8 row): rejected or over-strict — a struct object argument, copy source or destination
  through a loaded pointer or a base with a side effect (`g(*w->q)`, `g(*p++)`, `return *get();`, `(*pp)->t = f();`), a
  compound literal in a loop condition, a `for` step or a `do` condition, a literal used as an lvalue without `&`,
  `volatile`/`_Atomic` literals, `&h.x` as a global initializer, a nested member initialized from a struct value
  (`{s, 1}`), in C++ a value aggregate with an effect beside another non-constant element and a folded `S{…}.x`, C++'s
  `s = S{s.x++, 0}` (defined in C++17, rejected `unsequenced_modification` with a message saying so), a C++20
  designated list skipping a nested member (`T{.z = 1}`), and a temporary with a nested struct member as an argument
  inside a variable's initializer (`int r = get(T{{1, 2}, 3});`: EDG drops its braces there) [c2a, audits].
- From M4.5c1 (record 0043): rejected or over-strict — out-of-order C designators (valid C; EDG #2904-D, C++ requires the
  order), brace elision over a nested struct (`-Wmissing-braces`), a nested braced element whose braces flag EDG drops
  (C++ `S s8{1, {s8.a, 0}}`, rejected as elision), a struct copy in a `for` step, `&s.x`/`&s.t` of an `uninit` struct
  local under `--strict-init`, `{.y = (*&s).x}` in a struct's own initializer (it forms `&s`), `&p->x` with only `live(p)`
  (`null-dereference`: `p` may be one past the end); not measured — member self-assignment `s.x = s.x` and
  self-comparison `s.x == s.x`; accepted with no native positive possible (`-Wextra` only) — a positional list shorter
  than the struct and C++'s `{0}`; positions — own-initializer rejections at the declaration's line [c1].
- Warnings: EDG's #550 on a global that code only writes and only contracts read (clang and GCC are silent; tests read
  such globals in code) [b2a]; record 0025 item 6's "still accepted" shapes (`RED == RED`, clang's
  `-Wparentheses-equality`, `-Wswitch-bool`, `-Wfor-loop-analysis`, `-Wmisleading-indentation`, GCC's
  `-Wmultistatement-macros`, clang's `-Wempty-body` on loops, R10 with a named constant or a constant expression, R7 not
  looking into a `?:` arm that is a comma ending in a constant); untested shapes `else ;` and `do ; while (c);` [M4.4c].
- Rejected more broadly than the compilers [M4.4c]: R13's `;` on the line after the condition and an empty macro
  invocation as the body; R7's `*` through both `?:` arms in every conversion to `bool`; R9 on a 0 or 1 right operand from
  a macro and R11 on an operand from a macro under a written `&&`. R10 and R11 apply in code only, R5 and R9 in
  annotations too (question 44).
- Over-strict, listed: `bool b = &g`, `!&g`, `&g == NULL`, `static int b = (&g == &h)`, C++ `static int *p = &a` of a
  `static` local [b2a audit 2 N6]; C++'s multi-level qualification conversion (`int**` → `const int* const*`), a
  definition that must repeat its prototype's `owning`, two indistinguishable declarations with `owning` [b1c3];
  `(*p = f(a)) + (*q = g(b))` stays `unsequenced_calls`, a caller breaking an exclusion resting on a value `requires` is
  reported `permission` before `precondition` [b1c2]; `restrict` beside a same-typed parameter with only `live` [b1b];
  `a && (y = x = 1);` and a discarded `a && (flag = x);` with a `bool` target [M4.3b note 5]; the pragma rule rejects
  harmless `#pragma clang/GCC diagnostic` (an allow-list would lift it) [M4.2b1 note 1].
- C++'s `p == NULL ? d : *p` with a `const` pointee stays rejected (an lvalue `?:`), with a readable message [b1b].
- `--strict-init` reads declarations, not paths (record 0042 item 8): it accepts an `uninit` local that was already
  given a value, a second `init(&x)` and a marked parameter passed on after a write, and rejects a null argument, a
  pointer local holding `&x`, a reassigned marked parameter and `init(c ? &x : &y)`; C++'s `&*q` is rejected in code
  (since 0032) where C folds it to `q` [b2b].
- An uncalled `owning` declaration is not checked (`owning` beside `reads(*p)` is reported only once the function is
  called or defined); uncalled `must_init`/`ref_to_uninit` declarations are [b2b audit 2 S4].
- Mode differences, not meanings: `x = ((void)(a > 0 && x > 0), 2)` is `uninitialized` in C and rejected in C++; `x ? RED :
  GREEN` as a value accepted in C and rejected in C++ (0018 item 8) [M4.4c]; `c >= +GREEN` accepted in C, rejected in C++
  [M4.2c note 1].
- Not on an allow-list, no wrong verdict: asm labels on globals; a user `environ`/`__progname` without the system header
  (fine on macOS, Linux unchecked) [b2a audit 2 N5].

**Standing** (no session; act when the code is next touched)
- *From M4.5c2b:* call-site messages for a load-rooted callee place use the callee's spelling (`**pp`), not the
  argument's (`*p` for `&p`); `need_lb` is conservative (any by-name cell write gives every loading place a loop-entry
  snapshot: more case splits, no verdict); a loop moving a pointer between two parameters' objects through a global root
  (`gp`) hits 0040 item 5's swap false alarm like a local pointer does (question 79, M4.8); the existing `.map` texts keep
  `(p->q)->x` for invariant reads through a pointer member (P69's pins), while new load-rooted contract places are spelled
  as written (`Expr::user_spelling`); three S35 contrast fixtures expect `verified` of an unsound form on purpose
  (`s35_n_bodiless_retarget_nocheck`, `s35_o_old_chain_state_postdisjunct`, `s35_f_reads_fact_unguarded`), each beside its
  must-fail twin; `site_guard` reached in a loop annotation with a heap-reading guard is an internal error (unreachable:
  `resolve_written` lowers those guards) [c2b audit 2 N4–N6].
- *From M4.5c2a:* a write by name to a member of a struct LOCAL or a by-value parameter's copy in a loop still reaches
  every automatic place of its field (0043 item 11); globals got the precise rule (record 0044 item 8), locals would change
  existing programs' Viper (completeness only) [c2a]. The lint's `Addr` rule for struct-global rows cannot tell two struct
  types of one shape apart (the `.vpr` names no type on a global; the encoder and `check_program` match the types) [c2a
  audit 2 S4]. A hidden compound-literal local is named by its type's spelling (`(struct S){...}`), not its source text;
  a returned struct's dangling pointer leaf is reported without naming the leaf (`a.q`) [c2a audit 2 N9]. The Error
  messages page and record 0043 item 7 had quoted a copy message ("copies `s`, whose member …") the tool never printed;
  the page now quotes the printed one [c2a].
- *Fail-closed corners from M4.5c1:* `overlap_pairs` compares only the two addresses when both pointee types are equal,
  the `one_type` path filter skips reads-sum terms, `f__wf`'s equal-ghost skip for one root with two paths: complete only
  while types prevent `p == &q->t`, a miss costing Silicon's own error (exit 3), never an assumed fact (record 0043 item
  9) [c1 audit 2 N2]; storing `&p->t` into a cell skips `ptr_use`'s early `WF_S` row, asserted where it is assumed
  [c1 audit 2 N3].
- *By design:* C++17 sequences `g += f()` and `*p = (**pp = 5)` (the right operand first), but 0039's C rule stays in both
  modes [b2a N9, b1c3 N4]; `return p = &x;` and `int *r = &x; return r;` stay accepted (the caller's first use fails
  `lifetime`; questions 65, 70) and EDG's #1056 prints beside an R14 rejection [b1c2]; in an invariant `==` is identity
  with no liveness check [b1c1 note 7].
- *Messages:* `-Dfalse=1` (a program macro over vlibc's contract text) is reported three times, at the platform's
  declarations of `abort`/`exit`/`_Exit`, as "cannot be defined here"; it should point at the command line [d1 audit 2
  note 6]; a `#line` in a vlibc header is reported with the `#pragma GCC system_header` message (record 0046 item 3) [d1].
  `int *volatile gp` is named "volatile int *" [b2a N7]; an `annotation_mismatch` between declarations written
  by one macro names the parameter by position [b1c3]; `cerastes_old(*p)` in a loop invariant is rejected in C as "a
  conversion to or from a pointer type", in C++ as `old_outside_ensures` [b1c1]; rejection texts end "(decision 0012)" or
  "(decision 0037)" where a later record decides them, and the type message still says "only integer types, bool and the
  enumerations … are" [M4.2a–M4.2c, b1a, b1c1]; a block-scope `extern const` naming an internal-linkage C++ constant is
  called "a static local variable" [M4.2b1 note 2]; C's `case -1:` over an enumeration prints 4294967295 [M4.4a note 7];
  `d == (x ? (y, RED) : GREEN)` gives one diagnostic in C, two in C++ [M4.4b1 note 2]; lowering's
  internal-inconsistency diagnostics have no position; two failing `requires` clauses at one call print the same
  message twice (tests choose contracts that fail once) [M4.4d]; prototype `must_init` and definition `ref_to_uninit` give
  two `annotation_mismatch` errors (one per attribute), and two marker errors with one tag on one line collapse to one
  (`reject()`'s de-duplication); the markers' rejections end "(decision 0037)" like every `unsupported_annotation`; an
  unframed post-state read through an alias of a `ref_to_uninit` parameter (`ensures(*q == 1)`, `q == p`) is
  `permission` in `f__wf` and also `postcondition` at the return [b2b].
- *Positions:* with a `continue` in the body, a written atom's back-edge error is reported at the loop statement
  [b1c3 N3]; a loop body without braces likewise (no test) [b1c1 S5]; in C++20 a loop's IL position is its first
  attribute's (P59).
- *Fail-closed checks and their limits:* the memory-order check cannot see obligations that read cells, inline reads, or
  a set that is too large (0039 item 4, 0026 item 4; pinned by the audits' probes) [b1c2]; the termination-set fix was
  not re-audited (smaller sets are the safe direction; `noreturn_call_in_expression_termination.c`) [b1c2]; item 2's
  checks of M4.4c read the placement emission uses, a set computed too large is still invisible (pins
  `call_write_noreturn_divzero.c`, `never_returns_call_divzero.c`) [M4.4c]; `Acc::owner` can point at a dead stack `Full`
  of a then-arm while the else-arm is lowered (safe today; a heap-stable id would be sturdier) [b1c2 note 2].
- *Trusted reasoning checked by counterexample, not proof:* `f__wf`'s added facts (aliasing values equal, a `reads`
  place's post value equals its pre value, `owns` pairs distinct in one clause list) [b1b]; by-name cell checks are
  `permission`, so an encoder bug dropping a cell's permission would read as a user error (a VIR hook could pin it) [b1b].
- *Not re-audited* (the cap): b1c3's second B1 fix (in a contract, every pointer conversion changing pointee
  qualifiers, and `owns`/`owning` of a `const` place, rejected; pinned by `reject_contract_qualifier_*`,
  `reject_owns_const_*`, `reject_owning_const_int.c`) [b1c3].
- *Self-test pins:* the VIR self-test's written-atom cases pin failing rows by index (`fail_only(category, loc, count,
  index)`), so a change in row order breaks them, not a verdict [b1c3]; the VIR mask cases pass with either encoding by
  design [M4.4d round 2 note 10]; `calls_drop_exact_set_hook_off` duplicates `calls_fixed_order_hook_off` (harmless)
  [M4.4d].
- *Lint:* a `Perm` local as a non-first target of a multi-target call is not seen, an invariant's `acc` amount is not
  required to be a `Perm` local, and the "while without decreases" rule reads the first line holding `{` as the body
  [b1c1 note 8, M4.2a note 7]; the lint of `%` and `\` reads only the divisor and exempts the prelude bodies by name (the
  guarantee is structural: `low::mod_pow2`/`div_pow2`) [M4.4d]; `lint_flags` checks the flag on the line after the write
  only [M4.3a note 3].
- *Tooling:* `build/cerastes verify --keep-vpr` with no path writes `<stem>.vpr` into the current directory (P60) [b1c2];
  `mutate.py` does not mutate namespace-scope initializers, a literal mutated past `9223372036854775807` changes type,
  `--list` is slow under load, a mutation wrapper's cap must stay above the driver's `--timeout` (or Silicon processes are
  orphaned), `drop-complement` on a C++ destructor and `swap-bitwise` on `(T)&x` give stillborn mutants [M4.2b1 note 7,
  S3 note 2, M4.2a note 6, M4.4b1 notes 4, 7]; the walker oracle cannot tell an orphan closure copy from the used one, a
  phantom `on type <unnamed>` line could pass, and it lists neither a first declaration's own attribute after the
  parameter list nor one on a template definition [M2 audit B note 6, M4.4b2 note 5].
- *Driver and platforms* [S5]: `posix_spawn_file_actions_addchdir(_np)`'s result is not checked and Silicon is given the
  relative `program.vpr` (passing the absolute path removes it); `LC_ALL=en_US.UTF-8` may not exist on a bare Ubuntu;
  C++'s `assert((bool)x)` loses the user's cast on Linux, `assert(E::b)` with a scoped enumeration compiles only under
  glibc; `__thread` and the x86 GNU attributes are accepted on Linux only (0031 item 7); a hand-written
  `((void)sizeof(x++), x) ? (void)0 : __assert_fail(…)` on Linux matches and verifies (sound; require the `sizeof`'s
  operand to be a call of `__assert_single_arg`); the glibc matcher's near misses can only run against glibc
  (question 59).
- *Recognition by name or shape:* `assert` by its macOS expansion and `__assert_rtn`; the character-literal trigger
  compares VIR type names (`int_kind` would be sturdier) [S3, M4.4b2 note 3]; the noreturn trust checks a routine's first
  declaration (source sequence lists, on since M4.4b2, can now make it exact, and the libc table's "declared only in
  system files") [M4.2a note 1].
- *Fail-closed corners and unreached guards:* `names_object` false on an unknown node kind; a C case value of an unknown
  shape; defensive branches with no test (M4.1's undetermined `break`/`continue` target, a non-integer scrutinee, a case
  value of another type; a declaration directly in an unflattened generated block; a `cerastes::` attribute on a label;
  `comparison()`'s raw operands of two enumerations; "an enumerator whose enumeration this version cannot find";
  `enum_case_value`'s constant without `expr`; the exit-3 paths for emitter problems and "encoded twice", only
  `--selftest` sets the flag) [M4.1–M4.4a].
- *Accepted shapes worth a test:* `0 && (x = 1)`, `1 || x++` (folded, rejected); `assert((note(x), x >= 0))` (rejected by
  `check_program`); `decltype(x++)` (accepted, clang warns); a `_Noreturn int main` in C23 (only C++ tested); a typedef
  whose first reason is not an array is rejected only at a use; `_Static_assert(1.5, "")` names its type in C but not in
  C++ [M4.3a, M4.2a, M4.4b2]. Not pinned: a bare `p;` with `p` dangling reports `lifetime` (EDG's #174, and #550 in C++,
  keep it out of `error` tests; the `(void)p;` twins pin the rule) [b1a].
- *Costs to watch:* liveness checks at every pointer use cost about 1.5–1.7× time and text on S26's scale program; deep
  `^` mask chains are fast to depth 32 for range claims, identities only to the measured depth (0027 item 4) [M4.5a].
- *EDG, not chased:* in C23, `typeof(*cp)` for a `const int* cp` drops `const` (P53; harmless for `cerastes_old`) [M4.5a].
- **Lint:** `owner_assignment_ok` admits `held_X := <x> != null` anywhere. This is harmless (audit round 2 on C/D,
  N2), but it is a place where a later rule could be tightened. [d2b2a]
- **Toolchain-dependent facts found:** EDG's CMake takes `EDG_BASE` from the building compiler (bootstrap names the clang
  base); EDG's clang-18 emulation differs on eight tests (record 0053 item 11); with clang 18 and 20 vlibc's `<stdlib.h>`
  shim meets glibc's declarations with an incompatible linkage through libc++'s `<coroutine>` (question 170); glibc
  2.39 has no `__assert_single_arg`, the C23 one-argument `assert` the encoder matches (0031 item 5); ShellCheck 0.9.0
  (Ubuntu 24.04) adds SC2015. [S8]
- **Argument order:** GCC evaluates a call's arguments in another order than clang; `f(g(x), std::move(x))` lost
  `x` in the GCC build (`vir.cpp`'s builder, fixed). The GCC-built profile now catches such code, and clang-tidy's
  `bugprone-use-after-move` finds it on any compiler (the audit ran Homebrew's clang-tidy 23.1.1 over all 29 TUs of
  `build/cmake/compile_commands.json`: only the two fixed lines); a lint step would need a pinned clang-tidy. [S8 audit]
- **The position map accepts a row without a file** (`src/viper/posmap.cpp:25` stops the run, exit 3, only for a row
  without a line or column): adding `e.file.empty()` (and an empty `note_file` when there is a note) would make a lost
  file fail safe on every compiler, must-verify tests included (invariant 7). Not done in S8: the VIR self-test's
  synthetic programs may have rows without a file, so it needs a full-suite run. [S8 audit, should-fix 2]
- `include/cerastes.h`'s fail-closed `#error` (the cerastes front end without its attributes, record 0008 item 2) is
  one line again under `clang-format off`, now at line 42 (was 40); no test reaches it (reviewer round 1 S3). [S7]
- **Stale fact found, not fixed:** 0005 item 5 still describes the upstream-only submodule guard that 0009
  superseded. (`register_attributes.h` no longer counts the attributes; `plan.md` §0 now marks `internal/soundness/`
  as M5's.) [S6]
- **`examples/dev/`** is in the policy lists but has no files: the developer guide quotes test headers in `text` blocks
  (0049 item 5). [S6]

## 4. Session plans

**Rules for every session** (memory `session-size-few-hours`):
- **Size:** what fits in one context, under about 12 h unless a reason is given (owner, 2026-10-30, question 77; was
  ≤ 4 h), with fixed scope and stop rules.
- **Up front:** owner decisions and pre-approved expectation changes are asked before the work.
- **Invariants:** the decision record before code (6); must-verify and must-fail pairs (2); docs in the same session
  (8).
- **Team:** three Sonnet agents with disjoint files (edg-backend: `src/backend`, `src/driver`, build; il-analyst:
  probes, IL evidence, Silicon spikes, `src/vir` + `src/viper`; test-mutator: tests, harness, compiler matrices,
  `mutate.py`, `lint_viper.py`) and the Fable soundness-auditor at `xhigh` (owner, 2026-11-25; Opus before; one
  round per diff plus a re-audit of blocking fixes; should-fix items not taken go to §3). The orchestrator is Fable
  at `high` (owner, 2026-11-25; Opus before): the main session, started with `claude --agent orchestrator --effort high`
  (`.claude/agents/orchestrator.md` holds what the pasted opening prompts repeated; the plan names the records it
  reads and its starting section, `plan.md`'s "Session protocol"). It consults a Fable advisor (owner,
  2026-11-10); **from M4.5e1e1 every question goes to the advisor first, and to the owner only if the advisor
  cannot settle it** (owner, 2026-11-25; §8). **A session's plan assigns the work** (owner, 2026-11-11: the
  orchestrator does not decide the ensemble): section 2 of M4.5e1e1's and M4.5e1e2's plans is the model, and from
  M4.5e1e2 the plan also holds each agent's briefing, ready to send (owner, 2026-11-25; section 12 of the
  continuation's plan is the latest). At most
  three Sonnet agents are alive at once. Every role is a chain of instances, each started from its predecessor's
  state file (`build/<session>/<role>-state.md`): M4.5e1e1 needed four of each role, the auditor included (a fresh
  auditor per round). The il-analyst is replaced, never resumed, after the spikes and wherever the plan says
  (`lower.cpp` is 24,893 lines). `lower.cpp` and the VIR files have one holder at a time, with a commit or a snapshot between; an agent that needs the file
  while another holds it asks the orchestrator. Each report also goes to `build/<session>/<role>-report-<n>.md`.
- **While working:** single-file runs (`build/cerastes verify` directly: `run_tests.py` locks `build/vpr`); builds
  serialized (`lockf build/<session>/build.lock cmake --build build/cmake`).
- **`check.sh` once, at the end, after the last code change** (owner, 2026-11-09: "Since each run of check.sh now
  takes hours we should change our future work to run it less"; targeted runs and the neutrality regeneration
  before it), **on the x86-64 box only, never the Mac** (owner, 2026-11-09: "limit to just the linux because it's
  much faster, and doesn't lock up the local machine"): the branch as a git bundle into S8's `ubuntu-26.04`
  container, `~/cerastes-s8/p-ubuntu-26.04`, then `scripts/check.sh` there. Every other test run is on the box too
  (owner, 2026-11-24; §6): the Mac builds to compile.
- **A test budget** (owner, 2026-11-25: "I believe a wasteful amount of testing is occuring and these sessions are
  taking 10-12 hours each. If that's needed so be it"). A session's plan lists every test run with its reason
  (M4.5e1e1's section 3 is the model), and a run that is not listed is not made: byte identity of the kept Viper
  (`vprdiff.py`) stands in for re-running a test; a new test runs once, at its part, timed on the box; no full
  `run_tests.py` outside the one `check.sh`; spike fixtures and test files are capped. Never cut: invariant 2's
  pairs, the audit of each `src` commit, the neutrality gate, the one full check.
- **x86-64:** the EDG suites on the x86-64 host are their own session, but any x86-64 test run a session needs goes
  to `matt@clang4.cpp.al` automatically (owner, 2026-10-29: not Docker multi-arch on the Mac, which is only the
  fallback when the host is down).
- **Records and plans:** a record takes the next free number when it is written (0064 is M4.5e1e2's; 0065 is the
  next). The approved plan is saved as `.claude/plans/<milestone>-<topic>.md` (plan.md, "Session
  protocol"); a sub-session that plans again after its spikes saves that plan the same way.
- **What a worktree takes with it** (owner, 2026-11-09, question 193): session tools and spike summaries are
  committed. `vprdiff.py` and `regen.py` were lost with d2b1's worktree and again with d2b2a's, and so was S39's
  summary. From M4.5d2b2b they are `scripts/vprdiff.py` and `scripts/regen_vpr.py`, and a spike's summary goes into
  `internal/il-inventory.md` beside its probes. e1b's rejection rerun (`r3.sh`, `widen.sh`) was lost the same way;
  from M4.5e1c it is `scripts/rejections.py`.

Finished sessions: `plan.md`'s Done table has a row each, `.claude/plans/` their plans (from M4.5b1a on),
`internal/status.md` their results and the commits it names. The warning sweep (was M4.4e) is an M8 entry in
`plan.md`.

**Pending side sessions** (none blocks M4.5e1e2's continuation):
- **The EDG box session** (§4.1, pending since M4.2c).
- **The vlibc `<stdlib.h>` shim session** (owner, 2026-11-09, question 170: "Follow recommendation using a vlibc
  session"). With clang 20 and 18, vlibc's `<stdlib.h>` shim meets glibc's declarations with an incompatible linkage
  through libc++'s `<coroutine>` (`reject_coroutine.cpp`'s `incompatible_linkage_specifier`; record 0053). A vlibc
  session finds the cause and fixes the shim; then the clang-20 profiles drop their exclusion
  (`scripts/ci_toolchains.toml`).
- **The claims audit is not done** (owner, 2026-11-04: "I can have the soundness auditor run in a different session"):
  a read-only soundness-auditor pass over `bb41d55..s6-tutorial`'s pages (`pages/tutorial/`, `pages/dev/`,
  `overview.adoc`), the policy-list diff and the website's paragraph, for statements that are false or overclaim about
  behaviour, trust or a result's meaning; its fixes are doc edits plus a rerun of the changed examples. [S6]

### 4.1 EDG box session (pending since M4.2c)

**EDG box session after M4.2c** (owner, 2026-10-22). The owner merged `0022` into the fork's `cerastes`
(`f0ed5525af`, which also brings upstream `main` `ec2f12cffa`, 11 commits after `21065e551e`) and M4.2c moved the pin
there (`check.sh` green, every `.vpr` byte-identical, the three #2656 tests added). Still to do on the box: (1) the full
suites at `f0ed5525af` in both 64-bit configurations (`changes,regressions,imported,edg,cwg,stdlib`; known `cwg1835`,
`cwg2881`), and whether `0022`'s `.rto` needs `edg_x86_32` overrides; (2) old §11 question 16: reproduce the C
enumerator-list typing on unmodified upstream and draft its bug file (no fork fix). Clean builds every run (memory:
stale binaries after a failed link). (3) **`#if`** (owner, 2026-10-23, M4.4b planning; old §11 question 32): EDG folds a
preprocessing expression with no diagnostic for an unsigned wrap (`#if 0u - 1 > 0`), and in C23 its signed overflow is
#61, a warning verify mode suppresses for bodies; nothing reaches the IL (`preproc.c:460-491`). A fork commit adds a
discretionary diagnostic, distinct from #61, for a preprocessing expression whose arithmetic wraps or overflows, with
`GH-*` tests and the full box suites before the pin moves; the cerastes driver then makes it an error in verify mode.
(4) **The upstream merge** (owner, 2026-10-29, M4.5b1c2 planning: deferred to this session rather than done at a
session start): the fork's `cerastes` is 29 commits behind upstream `main` (`a0325e00e6`, fetched 2026-10-30 at
M4.5b1c3's start); merge it
(never rebase), run the full suites on the box, then move the pin with `check.sh` green and the goldens' diffs reviewed.

### 4.2 M4.5 Pointers (sessions b1–f3; designed in M4.5a)

**The design** is records 0028 (the memory model: E2, the `Block` domain, liveness, explicit checks), 0029 (the
annotations and C's spec functions), 0030 (vlibc), 0032 (b1a's slice) and 0033–0036 (the four axioms); evidence in
`il-inventory.md` "M4.5a" and "M4.5b1a"; the encoding pinned by the fixtures in `tests/viper-smoke/memory/`.

**The VeriFast port inventory** was `build/m4.5a/ports.md` in M4.5a's worktree (gitignored, and gone with it); its
conclusions are below per session. `malloc_block.c` holds no `malloc` (an address-taken struct local);
`insertion_sort.c` is a linked list whose ghost machinery is not portable as written (an array insertion sort in
M4.5e2 replaces it as a benchmark); 14 of the 22 files run VeriFast with `-disable_overflow_check`, so their ports
need `requires` bounds; `functions.c`'s predicate families and constructors, consumed fractions, backward `goto` and
`open_module` are M8 entries.

**Finished: b1a to e1e1** (records 0032–0063). `plan.md`'s Done table has a row each and `.claude/plans/` their
plans; the narrative of their splits is in `git show 2430529:internal/handoff.md` §4.2. Each session rejects what a
later one makes precise (fail closed). **Sizes against estimates**, for the next plans: M4.5d2b2b landed about 2,750
`src/` lines against 1,700–3,300; M4.5d2b2c about 900 lines of `lower.cpp` against 295–495 (the owner's in-session
answers, questions 206–214); M4.5d2b2d about 1,960 against 740–1,170 (the audits' fixes); M4.5e1a +1,943/−93 without self-tests (+3,457 with
them) against 2,200–3,450; M4.5e1b +2,754/−155 (+6,289 with them) against 2,050–3,050; M4.5e1c +3,514/−294 (+6,038
with them) against 1,500–2,200, with one cut; M4.5e1d +3,654/−195 (+5,721 with them) against 3,350–5,000 (a design
check against `lower.cpp` calibrated on what e1c landed), with no cut, in 11.5 hours; M4.5e1e1 +4,944/−439 (+7,596 with
them) against 2,940–5,525, with no cut, in under eight hours without the box check (its commits: plumbing with
global arrays and computed types 2,280, array members 2,116, structure values 665). M4.5e1e2's estimate
(4,700–8,450 in six parts) was calibrated on those commits and on three read-only surveys of the tree at
`45d26bc`; its first session landed +2,387/−306 (+4,852 with the self-test cases; about 330 of them the
`restrict` rules) against 1,400–2,400 for Parts A and B, in 9 h 50 with the box check run twice. The
continuation's estimate (3,700–6,700 for Parts C1, C2, D, E and F, a net removal for Part R) is the first plan's
raised by a fifth, with four read-only surveys of the tree at `f313023`.

**Pre-approved** (owner): a session may retire or re-expect exactly the rejection pins its plan lists (its "Flips"),
and no others; a pin whose construct stays rejected keeps its `expect:` line, and only its message and `Pins:`
comment name the new session.

**M4.5e1 Arrays** (planned 2026-11-11; `.claude/plans/m4-5e1-arrays.md`; questions 226–228). Seven sub-sessions
(e1f added at e1c's planning, 2026-11-23; e1e cut in two at its own, 2026-11-25, question 290), each a session of
its own with its record, twins, docs, audits and one box `check.sh`:

| Sub-session | Scope | Plan |
|---|---|---|
| **e1a** (done 2026-11-11, record 0059) | local arrays as blocks, `a[i]`, `&a[i]`, `p[i]`, the decay, initializers, scope exits, the marked range rows, `out-of-bounds`, loops over arrays, sequencing, the `offset_of == 0` backstops, `sizeof(a)` | section 3; `internal/status.md` |
| **e1b** (done 2026-11-12, record 0060) | `owns_range`, `reads_range`, element reads in contracts, array parameters, calls and returns over ranges, `f__wf`, loops over parameter ranges, markers over ranges, `owning(n)` on a parameter, the `carrays.c` port | `.claude/plans/m4-5e1b-ranges-in-contracts.md`, complete: P79 and S44 with decision tables, no second planning |
| **e1c** (done 2026-11-24, record 0061) | `cerastes_forall` in contracts, loop invariants and checks; the fill loop, max, the `carrays.c` port, the tutorial chapter; S45 (r) for e1f; Part D cut to M4.5e2 (question 260) | `.claude/plans/m4-5e1c-quantifiers.md`; `internal/status.md` |
| **e1d** (done 2026-11-25, record 0062) | read `pure` functions as Viper functions with their contracts and definition checks, self-recursion with `decreases`, calls of `pure` functions in checks, contracts and loop invariants, the exact sum | `.claude/plans/m4-5e1d-pure-functions.md`; `internal/status.md` |
| **e1e1** (done 2026-11-25, record 0063) | arrays of scalars as globals and `static` locals, as members of structures, as compound literals; 0024 item 6 lifted for array types of scalars; structure values with array members (question 291, its Part E) | `.claude/plans/m4-5e1e1-arrays-of-scalars.md`, complete: P82 and S47 with decision tables, a test budget, no second planning |
| **e1e2** (record 0064; **Parts A and B done 2026-11-25; Parts R, C1, C2 and D done 2026-11-26**; Parts E and F next, from the same plan) | done: leaf paths with the traps closed, automatic local arrays of structures in code (`v[i].x`, `p[i].x`, copies, loops), question 304's rejection as measured; every `restrict` parameter rejected (question 326) with questions 322 and 325; ranges over structures in contracts; global, `static` and member arrays of structures; the tutorial section; the six leftovers of a structure with an array member of scalars (questions 292–294, 306, partnerless and written atoms, with loops; `calloc` up to 24 values, question 354; not for an array member of structures, question 348). To do: arrays of arrays (array-of-rows parameters in three spellings and the typedef'd ones, question 320; depth 3; the tutorial's matrix: Part E); quantifiers and `pure` applications over members and member arrays (Part F) | `.claude/plans/m4-5e1e2b-arrays-of-structures.md` for what is left (Parts E and F; its test budget, briefings and the 12-hour rule with split points; started with `M4.5e1e2 continuation, from Part E (…)`, §2 step 4; an unproved, unaudited encoder draft of Part E is on the branch `m4.5e1e2c-part-e-encoder-draft`, §3); `.claude/plans/m4-5e1e2-arrays-of-structures.md` is the history of Parts A and B, P83 and S48 |
| **e1f** | range call rows: the minimum read amount over a range (238), `owns_range`'s row order D (242) | section 10: planned after e1c, from its spike S45 (r); §3 "M4.5e1f" |

No sub-session plans again after its spikes any more: the owner had e1b, e1c, e1d, e1e1 and e1e2 each planned in
full (2026-11-11, 2026-11-23, 2026-11-24 and twice on 2026-11-25), each probe and spike with a table of what is
built per outcome. A plan too large for one session is run under the 12-hour rule (question 310): the parts not
begun run from the same plan in a continuation session. **One exception so far** (owner, 2026-11-25): after
M4.5e1e2's first session the owner had the rest planned again, since that session revised its record in eleven
items, found a soundness hole outside its scope and left four owner questions; the continuation's sessions run
from that second plan, with no third planning.

**M4.5e2 Pointer arithmetic** (needs e1a and e1b; may run before e1e and e1f; its `malloc(n * sizeof(struct S))`
waits for e1e2). `p + i`, `p - i`, `p - q`, relational
comparisons within one object, `malloc(n * sizeof(T))`, `calloc(n, …)`, `realloc`; relational comparisons and
`p - q` under `pointer-comparison` (the category and the `==` exception of 0028 item 3 landed in M4.5b1b);
`out-of-bounds` and a one-past-the-end `&a[n]` are already e1a's. Port: an array insertion sort (in place of
`insertion_sort.c`). §3 "M4.5e2" has what e1's plan leaves it.

**M4.5f1 Predicates and pure functions.** Predicate bodies encoded (0011 item 4 lifted), `fold`/`unfold`,
non-recursive predicates, pure functions over predicates; e1d encodes read `pure` functions and their calls
(question 227) and leaves M4.5f1 the rows of its plan's section 1 table. Port: `cell.c`. R1–R13 and the
static-assertion cross-check reach a read `pure` body from e1d, and `predicate` bodies here.

**M4.5f2 Recursive predicates.** A linked list (push, pop, length), recursive owning fields (`next`), leaks inside
predicates (0028 item 6). Ports: `stack.c`, `sum.c`, `popn.c`.

**M4.5f3 Lemmas and ghost sequences.** Its record decides whether lemma functions and ghost sequences are in M4 (they
are needed for content specifications: `strlen`'s zero-terminated string, `memcpy`'s and `memcmp`'s contents).
`internal/upstream/attribute-registration.md` Appendix A lists the open design points. Ports: `reverse.c`, `sum_full.c`,
`push_all.c`.

### 4.3 M4.6–M4.8

**M4.6 Chars and strings:** string literals as read-only arrays, `char` buffers, vlibc's `<string.h>` (`strlen`,
`memcpy`, `memcmp`, `strcmp`: contracts on the standard names, and implementations `vlibc_strlen`, `vlibc_memcpy`,
`vlibc_memcmp`, `vlibc_strcmp` verified in vlibc's CI, record 0048; M4.6 decides the rule that verifies vlibc's `src/`,
0048 item 5), `main(argc, argv)`; character
access to other objects stays rejected (0028 item 1); `c | 0x20`-style case bits are decided (0027). Ports:
`strlen.c`, `memcpy.c`, `memcmp.c`. Open point: a zero-terminated buffer of unknown length needs a recursive predicate or a
built-in notion (M4.5f3 decides). **`const` objects in the model** (owner, 2026-10-30, question 78, the recommendation):
field names ignore `const`, so record 0040 item 2 rejects, in a contract, every pointer conversion that changes pointee
qualifiers, every comparison across them and every `owns`/`owning` of a `const` place (fail closed, after M4.5b1c3's
audit round 2 B1 and its re-audit); `char *strchr(const char *s, int c)` with `ensures(r == NULL || r points into s)`
needs them. M4.6 models a `const` object as a cell the function holds below `write` after its initializer (record 0028
amended: allocation, scope exit and `owns` of a `const` place), then lifts those rules; `&K` of a `const` global and an
address-taken `const` local (M4.5b2a's pins `reject_address_of_const_global.c`, `reject_case_const_address_taken.cpp`)
come with it.

**M4.7 Function pointers:** contracts on function types, calls through pointers; ports of `tutorial_solutions/byref.c`,
`examples/functions/functions.c` (without its predicate families, an M8 entry).

**M4.8 C parity close-out:** the VeriFast C differential set (the ports above, run with VeriFast in Docker as M5c
sets up, or recorded outcomes), the full mutation run over the M4 tests and the verified doc examples never mutated
before (owner, 2026-10-27, §8 question 10), the mask positives with mutants of a mask constant counted as "undecided"
(§8 question 49), the cost of aliasing `reads`/`live` parameters (§8 question 66: fix it here, e.g. Silicon's joins
or a per-call split order; record 0037, Consequences), with it the false alarm of written atoms on moved pointers (owner,
2026-10-30, question 79, the recommendation: per-place liveness amounts at the loop head, or the share named by the
parameter's ghost amount; pinned by `tests/memory/negative/loop_written_moved_owns_false_alarm.c`, §3), the Memory and
ownership page complete, M4's acceptance (plan.md),
and the deferred list for M8
(unions, floating point, `volatile`/`_Atomic`/`_BitInt`/vector types, variadic functions and `printf`, several
translation units, concurrency).

### 4.4 M5.0 and M5 — type traits and generic templates (designed in the M4.1 session)

**M5.0 Type traits** (owner, 2026-10-26, old §11 question 39; `plan.md` "Before M5"). After M4.8 and before M5a. M4.4b2
skips a static assertion whose condition calls a function, uses a type trait (`enk_builtin_operation`, a variable
template such as `std::is_integral_v<T>`), a concept-id, or names an entity of a system header (`std::numeric_limits`).
M5.0 decides how cerastes reads such conditions (and type traits elsewhere: contracts, `if constexpr`), so that M5's
integral `T` and M6's requirement contracts rest on it. Planned at its own planning.

**Before M5: typed locals in the mutation classifier** (owner, 2026-10-27, §8 question 11: "Not now, but annotate that
this needs to be addressed before M5"). `mutate.py`'s classifier does not read local declarations, so about 20 listed
equivalents (e.g. `assert_holds.c`) rest on a reviewer's reading; teach it to read them, with its own audit, before M5a
(in M5.0 or a session of its own).

**From P38 (M4.4a, record 0022 item 3):** a non-type template argument keeps no `expr` in this configuration
(`BACKING_EXPR_FOR_NONTYPE_TEMPL_ARG` is false), so `t<0u - 1u>()` reaches an instance as the plain constant
`4294967295U`, which cannot be told from a literal. Under 0022 (no wrapping) M5 must not trust such a constant: check a
template argument's conversion at the place where it is written (the template-id in the caller), or reject arguments
that are not literals, decided in M5a's record.

The M3 handoff §6 still holds: VIR's `IntType` needs a type form for `T`; templates in verified files are rejected
today (in `encode.cpp`: declarations in `Encoder::declaration_level_checks`, routines in `Encoder::run` through
`template_declared_here`, calls in `callee_for_call`; S7 replaced the line numbers, record 0051 item 8); generic proofs read contracts
from the root template; contracts can use lambdas naming `T`. Latent hazard: a template instance whose body is in a
verified file takes `callee_for_call`'s has-body path (`encode.cpp`) and would become a bodyless VIR
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
  by il-analyst; one audit. Stop rules: cut the oracle's structural checks, then tag coverage automation, then
  `main` in the ports; any rule family not done by 2:10 returns ineligible "not checked" (fail closed).

**M5b EDG (box).** Best just before M5g. Fork commit Q: a registration flag
refusing an accumulating attribute that first appears on a later declaration (check site
`attach_decl_attributes_to_entity`, `decls.c:806-812`, next to `reconcile_must_match_attributes`,
`attribute.c:4131-4230`; next free flag bit `0x8`, `attribute.h:401-437`; precedent
`ec_comparison_defaulted_in_class_must_be_first_decl`, `decls.c:10631`; `first_decl` is set on namespace-scope
paths — check member and template paths; GH tests in `tests/tests/changes/GH-*.sft.cpp` with an `-ok` companion,
`src/Changes`, `doc/source/ext_intf.rst`); cerastes registers it for `predicate`/`pure`
(the `predicate` and `pure` rows of `register_attributes.cpp`'s table). Fork commit R: EDG diagnoses series L's silent error node for a `decltype(p)`
lambda parameter (§3, "M5.0 and M5"); the `decltype` tests move from `cerastes-annotation_substitution_error` to EDG's diagnostic
at the same line (owner's rule). Plus the close-out audit B's missing tests (§3, "M5.0 and M5"). Fail-before/pass-after,
the audit, the full box suites (baseline mismatches `cwg1835`, `cwg2881`), pin move, 0006/0007 and the upstream
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
symmetric ranges (in `lower.cpp`, `unary`'s `Neg` obligation `a != lo` and `binary`'s division
`!(a == lo && b == -1)`) are not reused —
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

## 5. Known gaps carried

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
  `assert` in a `for` increment. Each is rejected with a diagnostic and an M8 entry (glibc's `assert` expansion is
  recognised since S5, record 0031 item 5).
  A `goto` into a block or an `if` branch is lifted only when a ported example needs it, after a Silicon spike with
  must-fail twins (owner, 2026-10-27, §8 question 5).

## 6. Process notes

- **The box from M4.5e1d on** (owner, 2026-11-24/25: all testing there, several agents at once, fewer intermediate
  checks). Lanes: `docker commit cerastes-s8-ubuntu-26.04 cerastes-e1d:26.04` (the toolchain `ci-linux.sh` installed),
  `cp -a ~/cerastes-s8/p-ubuntu-26.04 ~/cerastes-e1d/<lane>`, then `docker run -d --init --name cerastes-e1d-<lane> -v
  ~/cerastes-e1d/<lane>:/work/cerastes -e CERASTES_CI_PROFILE=ubuntu-26.04 -e CERASTES_JOBS=32 cerastes-e1d:26.04 sleep
  infinity` (`--init` reaps Silicon's zombie Z3s; without it hundreds pile up). Ship work as a commit object (a
  temporary index: `GIT_INDEX_FILE=… git read-tree HEAD; git add -A; git write-tree; git commit-tree`, a ref under
  `refs/tmp/`) in a bundle, fetch it in the lane, check it out detached, `scripts/bootstrap.sh --build-only`. One lane
  per agent that builds; a neutrality baseline and its comparison on the same host (the Mac and the box differ by
  glibc parameter names and absolute paths). `tail` there is uutils (`tail -n 3`, not `tail -3`). The box's Z3 is
  3–6.5 times slower than the Mac's per query on a hard refutation, while its 172 threads win on suites.
- **Where M4.5e1d's 11.5 hours went** (its orchestrator's log; M4.5e1e1's plan, section 3, has the times): 4 h 25
  came after the last part was built and proved. A full `run_tests.py` on the Mac (15,982 runs, 1 h 50) that the
  box check then repeated; each part's run repeating the earlier parts' tests (333, 436, 483 runs) although
  `vprdiff` had shown their Viper byte-identical; two tests found slow on the box only at hour 9, because the
  Mac's Z3 is 3–6.5 times faster per hard query; a third validation round after the last audit. **From M4.5e1e1 a
  plan carries a test budget** (§4): the box's lanes from Step 0, a new test timed there at its first run, one
  preflight built from `check.sh`'s own steps, then the box check.
- **M4.5e1e1 under the test budget** (its status entry): Commit 1 one hour in, Commit 2 at 5 h, Commit 3 at 6 h 10,
  Commit 4 at 7 h 35, the close minutes later, without the box check or `mutate.py --validate` (the owner ended the
  session and ran the full check afterwards: green). All nine questions went to the advisor and none waited on the
  owner. Commits 2 and 3 were composed from proved snapshots (`refs/tmp/e1e1-*`) while the next part was built in the
  same worktree; M4.5e1e2's plan makes that its method (a commit's tree is the tree its joined proof passed on).
- **M4.5e1e2's first session under the budget** (its status entry): Commit 1 at 1 h 16; Part A proved at 0 h 46
  (built during Step 1); audit round 1 back at 2 h 31 (74 minutes); Part B's halves proved at 4 h 35, joined at 5 h
  03; audit round 2 back at 6 h 08 (60 minutes). Where the time went beyond the plan: a base unsoundness in
  `restrict` that round 1 found and that took a record item, two probe rounds, a predicate, and a second fix cycle
  after round 2 found more; question 304's rule, which had to be measured three times as each measurement showed
  a wider set (the matrix, the nested forms, the designators); and a box loaded by other users (load 20–140), which
  roughly doubled every Silicon time. The session closed at split point 1. Every role needed three or four
  instances; the il-analyst's Part B lowering needed two (`lower.cpp` fills a context in about 3,000 changed
  lines). An auditor's probes live outside the tree (its instructions forbid writing there): copy them into
  `build/<session>/audit<n>/` the moment its report arrives. **The full check's first run was red on a new VIR
  self-test case** that reported two errors at one line: the il-analysts had proved their cases with sweeps of
  their own (Silicon by hand, comparing sets of error lines), and the harness counts errors. **From now on a
  part's new VIR cases go through `scripts/run_tests.py --expect-passed <n>` over a directory holding them, as
  `check.sh` does, in the half's own proof and again in the preflight**; and the preflight also runs the steps of
  `check.sh` that follow the self-test (the walker oracle, the tree checks), so that the one check cannot stop
  before them.
- **The planning of the continuation** (one session that only planned, 2026-11-26 01:31 to about 02:25 UTC):
  the handoff, the first plan, record 0064 and the three audit reports read in full by the planner; four read-only
  surveys in parallel (the encoder's gates, the lowering's sites and guards, tests with lint and docs, every use of
  `restrict`), each asked for file-and-line tables with "not found" in place of a guess, each spot-checked against
  the tree; then the owner's dialog, which needed the `restrict` survey's exact file list (invariant 1: the owner
  approves the files whose outcome a decision changes, so the list goes into the question). **A plan cites the
  record's items and carries only the amendments; it does not restate the record** (the first plan's section 7
  was out of date within hours of its session's start).
- **What keeps a session short:** fixed scope with stop rules; the owner's decisions and pre-approved expectation
  changes asked up front, each with a concrete default; the record written while the probes' results come in, audited in
  the background; three agents with disjoint files; single-file runs during the work; questions to the advisor
  first, the asking agent going on with what the question does not block (M4.5e1d waited on the owner fourteen
  times, questions 273–286).
- **Worktree.** Each session creates its own worktree, `.claude/worktrees/<session>` on its branch from `origin/main`
  (`git worktree add .claude/worktrees/m4.5b2b -b <branch> origin/main`; the old `.claude/worktrees/m4.2b1` is gone since
  M4.5b2b), with its submodules cloned with `--reference` to the main checkout's module stores (`third_party/edg`,
  `third_party/silicon`, then `silver` inside it), then `scripts/bootstrap.sh`. A shell `cd` elsewhere may leave the
  worktree: name it in every command.
- **Builds and baselines.** Build in the session worktree, never in the main checkout (its `build/` is whatever was
  last built there: on 2026-11-10 it predated d2b2b's last source change). Take the baseline before any change to
  `src/`: `scripts/regen_vpr.py` (every kept `.vpr` and `.map` without Silicon, about a minute) and the VIR self-test
  into `build/<session>/baseline/`, with a frozen copy of the build for probes. `scripts/vprdiff.py` against it after
  each part answers "did any existing program's Viper change" long before `check.sh`; give each concurrent
  regeneration its own output directory (two runs sharing one raced and printed false `MISSING` lines). Agents build
  with `lockf build/<session>/build.lock cmake --build build/cmake`, and nothing builds while `check.sh` runs. Run
  `scripts/bootstrap.sh` in a worktree that was last built for another branch (a stale EDG stamp stops
  `run_tests.py`). A session that moves the pin runs `check.sh` once early.
- **Dependabot and the CI guard** (M4.5b1c1): `scripts/test_run_tests.py`'s `CiGuardTests` holds a model of
  `.github/workflows/ci.yml` with every action's SHA, and record 0001 lists them; a Dependabot bump changes only
  `ci.yml`, so `main` failed the harness self-test from PR #16's merge (`fca0cbc`, `actions/setup-python` v7.1.0) until
  this branch updated both. Each later bump needs the same two edits (or a change to the guard).
- **Parallel Silicon runs saturate the Mac** (M4.5b1b: load averages up to 215 while an agent ran a test directory and
  another timed programs): time single programs only on a quiet machine, and say so with each timing.
- **Twins and Silicon's continuation:** after a failed check Silicon goes on with the failing value unconstrained, so a
  twin whose failed read flows into an `ensures` fails twice; give such twins an `ensures` that holds for any value.
- **Compiler matrices** (P41, P48, P50): clang and GCC, both modes, three tiers (no flag, `-Wall`, `-Wall -Wextra`); the
  union rule of 0020 item 5 rejects what warns at the first two tiers. Measure the near misses too, natively: a positive
  that holds them must compile with `-Wall -Wextra -Werror`.
- **Mutation** runs once, after the last rebuild, alone on a quiet machine, with no external cap below the driver's
  `--timeout` (M4.4b1: a SIGKILLed driver orphans Silicon). `mutate.py` reports progress on stderr; check `date` against
  the budget before starting.
- **Commits.** `git add <paths> && git commit` commits everything staged, including an earlier `git rm`: delete a retired
  test in the working tree, and stage the deletion only in the commit that lands its feature. No Claude trailer; never
  push.
- **Doc pitfalls.** `[[cerastes::owning]]` in prose becomes an anchor (with `::`, even `+…+` and `{empty}` leave one, and
  `check_doc_examples`' source lint refuses `++…++`): name the attribute (`cerastes::owning`) and let the example show
  the brackets; a code span holding `**` needs `+…+`, or `pass:[…]` when it also holds ` + `; a backtick next to a double quote
  (`` `*q`" ``) is a curved quote and breaks the span (M4.5b1c3). `[[noreturn]]` in text becomes an anchor even inside `+…+`; `__name` in backticks needs `+…+`; a
  possessive after a code span breaks the next span; `pass:[…]` holding `<` or `>` is refused by
  `check_doc_examples.py` (write `+…+`). `build_docs.sh` and `check_doc_examples.py --html build/doc` catch them.
- **Test generators** (M4.5c1, M4.5c2a, M4.5d2b2a): a script that writes many tests lives in `build/<session>/`, not
  `/tmp`, and is never re-run once any test it wrote is in the tree: a re-run silently restored an owner-approved
  `expect:` change in M4.5c1 and wiped three hand-added expect lines in M4.5d2b2a (the audit caught it). After the
  first full run the files in the tree are the source of truth; fix comments by hand.
- **A test that loses an `expect:` line** (pre-approved) keeps its line numbers with a comment in its place
  (M4.4c, the four `reject_assert_shape_*.c`).
- **Builds during a suite run** (M4.5c2a): `lockf` serializes builds, not a build against a running `run_tests.py`; an
  agent's rebuild replaced `build/cerastes` as the first suite run started. Tell every agent to hold builds before a
  directory-wide run, and treat a run that overlapped a build as provisional.
- **Snapshot worktrees for the audits and a second line of work** (M4.5d2b2a, M4.5d2b2b): each audit runs on a
  snapshot commit in a second worktree (`.claude/worktrees/<session>-audit`) with its own build, so a part is audited
  while the next is built, with no mixed working tree; d2b2b also built its lowering in `<session>-aux` (detached,
  committed there). Snapshots are made with a temporary index (`GIT_INDEX_FILE=… git add -A; git write-tree;
  git commit-tree`), and the branch's commits from the snapshots' trees (`git read-tree`, `git ls-tree -r <snap> --
  src/ | git update-index --index-info`, `git commit-tree`, `git update-ref`, then `git reset -q`). A shell `rm`
  inside `bash -c` is refused by the safety check: `git read-tree` replaces a temporary index anyway. The harness
  refuses a subagent's second hand-back, which silently drops the report: have agents write each report to
  `build/<session>/…-report-<n>.md` as well.
- **Before the one box `check.sh`:** run `scripts/regen_vpr.py` over the final build and `scripts/lint_viper.py` over
  its output and a fresh VIR self-test directory, and run the session's VIR cases through Silicon. M4.5d2b2b's first
  box run had to be stopped: the last fixes had changed Viper shapes the lint pins, and the lint step comes late in
  `check.sh`. [d2b2b]
- **Every session that changes CI:** a new toolchain is one `[[profile]]` in `scripts/ci_toolchains.toml` at `dispatch`,
  promoted to `nightly` after a green full check; the developer guide's "Adding a toolchain" has the fields. The guard
  pins every assertion line of `ci.sh`/`ci-linux.sh` literally (messages included) and `ci.yml` as a pattern tree, so a
  changed assertion or step is a self-test change too; the two references are pinned in `CI_REFERENCE_PROFILES`. [S8]
- **Linux rehearsals on the box:** `~/cerastes-s8/run.sh setup|update|check <profile>` (a fresh copy of `base`, a
  container per profile from a clean `ubuntu` image without compilers, `ci-linux.sh <profile>` then `check.sh`). Ship
  commits as a git bundle (`git bundle create … <branch>`, then `run.sh update`). Never pipe a macOS `tar` into the
  trees (it adds AppleDouble `._*` files, and `scripts/._*.sh` breaks `check.sh`'s lint), and never clean them with
  `find -name "._*"`: EDG's test tree has directories named `._…`. [S8]
- **Before an owner question about a lowering change** (M4.5d2b2d): run the end-to-end vprdiff against the session's
  baseline and put every changed VIR self-test case through Silicon first; questions 219, 220 and 223 each needed a
  correction because one of the two was skipped. [d2b2d]
- **Tool pitfalls** (M4.5d2b2d): `patch -p1` leaves a `.orig` beside the patched file in `src/` (it reached an audit
  snapshot once); `cerastes-vir-selftest --help` takes `--help` as its output directory and writes it; the box's
  `ubuntu` containers have no `bc` (time with `date +%s%N` and shell arithmetic); a `pkill -f <word>` inside
  `docker exec bash -c "…<word>…"` kills its own shell, and `docker restart <container>` is the clean way to stop
  everything in a session's container. [d2b2d]
- **Lessons of M4.5e1a:** (a) an allow-list computed by `grep` before a build must use the emitted spelling (the token
  is the field `.tok`, so a search for `tok(` predicted 849 of 1,080 files): after the build, classify every changed
  line by exact inverse rules (each line rewritten back must equal its baseline line), never by deleting conjuncts and
  comparing. (b) The audit worktree needs no bootstrap: a source-only `git worktree add --detach` of the snapshot plus
  a copy of the session's `build/cerastes`, `cerastes-fe` and `cerastes-vir-selftest` built from exactly that source
  (and a copy of `fe-base.sh` pointing at them). (c) C++ positions an attributed statement at its first attribute (C at
  the keyword): a new row reported "at the loop" uses `Stmt::keyword_loc`. (d) Proofs made while another agent builds
  are provisional: the commit's validation runs once, on one build, with every build held. (e) A twin that runs a loop
  further than its positive needs an `ensures` that holds at the wider exit state, or it fails twice. [e1a]
- **Every session:** new tool code in the style (`CLAUDE.md`'s conventions, the developer guide's "Code style");
  `scripts/format.sh` before committing; brace initialization, `U` and one declarator per declaration are review rules.
  A new `.cpp` under `src/` changes `object_identity.py`'s TU-count constants (`EXPECTED_TU_ENTRIES` 30,
  `EXPECTED_TU_FILES` 29): update them in the same commit, or the tool refuses. [S7]
- **Refactors:** `python3 scripts/object_identity.py --before <base>` (and `--tokens none|braces`, `--formatted-by`)
  proves a behaviour-neutral change long before `check.sh`; it compiles only the running platform's code, so a refactor
  of platform code also runs it on the x86-64 box. [S7]
- **Dependabot's npm bumps:** after one, check that `doc/package-lock.json` has one hoisted `@antora/logger` (`npm ls
  @antora/logger`); nested copies silently stop `build_docs.sh` from failing on warnings (`DocBuildWarningsTests`
  catches it). [S7, `9cff0ad`] PR #35 (Antora 3.2.3, 2026-11-23) did it again and left `main` red until `fabcd25`
  (`npm dedupe`, then `npm ci`, `scripts/build_docs.sh` and `test_run_tests.py DocBuildWarningsTests`).
- **Not done by S7** (questions 150, 151, answered 2026-11-05 with the recommendations): about 2,500 lines of `src/`
  over 120 columns (almost all whole-line comments and long string literals) are re-wrapped as they are touched; the
  132 `{ // comment` brace lines stay until touched, and **no new or touched line puts a comment on a line with a
  brace** (`{ // …`, `} // …`; owner: "in the future I don't want comments on the same lines as braces"; `CLAUDE.md`,
  the developer guide's "Code style"). A `for`-init with two declarators (`encode.cpp`, the line-scanning loop) is the
  declarator rule's one exception. [S7]
- **Tutorial upkeep** (0049 item 4): the arrays chapter is written (M4.5e1c, with quantifiers); M4.5e1d extends it
  with the exact sum and M4.5e2 with pointer arithmetic (and the in-place loop of cut 1, §3), M4.5f2 owes predicates and a linked list, M4.6 strings, M4.7 function pointers, M5 generic templates, M6
  requirement contracts, M7 references and classes. Every session that changes a message, a category or an accepted
  construct a chapter shows updates its prose and `console` blocks: the blocks are copied from a run in
  `doc/modules/ROOT/examples/tutorial/` (`../../../../../build/cerastes verify <file>`), and only review checks them. A
  check that reruns each shown command and compares the output is possible later work (0049, Consequences). [S6]

## 7. Acceptance

Each session's acceptance commands are in its plan (`.claude/plans/`); the results of finished sessions are in
`internal/status.md` (the latest entry) and the commits its history names.

## 8. Questions for the owner

**Three things are for the owner** ("Questions for the owner" at the end of this section): 354 (the limit
on `calloc`), 357 (a lint edit the permission system denied; two tests set aside) and 348 (a rejection kept
for structures whose array member holds structures). **343 was answered by the owner on 2026-11-26, inline:
"Follow alternative plan since it sounds like the correct fix."** (the exit 3 stays listed until M4.5e2), and
on the same day the owner let the advisor's rulings 338–342 and 344–347 and the earlier defaults and rulings
(331–337, 321–324, 327–330, 313–319, 301–309) stand ("Follow recommendation"). **For the owner's review,
none blocking:** 348–357, the advisor's rulings in the continuation's second session. Questions 310–312 were
answered at M4.5e1e2's planning and 304 inline before it. An answer is written inline, as before. Answered
questions 1–246 are recorded in the decision records, the session plans, `internal/status.md` and the earlier
handoffs:
`git show 2430529:internal/handoff.md` §8 digests questions 65–187, `git show 262edb3:internal/handoff.md` has
188–193, `git show f57ee43:internal/handoff.md` §8 has 194–223 with the owner's words (224 and 225 only as
asked: their answers are below), `git show bdf3581:internal/handoff.md` §8 has 238–246 as asked, with the
owner's inline answers to 238–242, and `git show 9ab9a1e:internal/handoff.md` §8 has 247–263 as asked. Records
0055–0058 hold what each of 194–223 decided; the ones that steer M4.8 are re-filed in §3. Questions 264–272 are
M4.5e1d's planning's, 273–289 its session's, 290–300 M4.5e1e's planning's, 301–309 M4.5e1e1's, 310–319
M4.5e1e2's planning's, 320–330 its first session's, 331–337 its continuation's planning's, 338–347 the
continuation's first session's and 348–357 its second session's, below.

**Standing answers that later sessions act on:**
- **69:** the upstream EDG merge and the pin move wait for the box session (§4.1).
- **77:** sessions are sized to one context, under about 12 h unless a reason is given.
- **160:** a new test's expect line may be corrected after its first run when it fails for its designed reason at
  another line or a neighbouring category; each correction is listed in the status entry.
- **172:** after the owner's `profiles=all` dispatch, the next session promotes `macos-26` to `nightly` if its job is
  green, or records what differs.
- **176:** the owner handles CI (the nightly timeout, the `profiles=all` dispatch).
- **No test runs while a fix is queued** (owner, 2026-11-10, M4.5d2b2c): "For all of these fix now items you should
  discontinue the testing runs since we know that things are going to change." Stop any suite or timing run on a build
  a queued fix will replace; the implementer's own proof of its fix still runs; one test run after the last fix.
- **Questions go to the Fable advisor first** (owner, 2026-11-25, M4.5e1e's planning: "If you or the subagents
  have questions first consult the Fable advisor, and then if you still need an answer or guidance you may stop
  and ask me"). It replaces 2026-11-24's "stop and ask me" for every question. An agent's question goes to the
  orchestrator, who takes it to the advisor with the facts and a recommended answer; the agent goes on with what
  the question does not block. **The advisor settles** a choice inside the record's stated rules, a choice
  between fail-closed alternatives, test mechanics, message wording and a question-160 correction, a cut on the
  plan's list and a split point under the plan's rules. **Still the owner's, and the work it blocks waits:** an
  `expect:` change outside the plan's Flips and question 160 (invariant 1), any existing program's Viper, map or
  golden changing, a cut the plan protects or does not list, a soundness finding with no fail-closed fix, the
  twelve-hour cap, and what the advisor says is the owner's. (The list of what the advisor settles is the
  planner's reading of the owner's sentence, for review.) **Each answer the advisor settles is logged here**,
  numbered on, the soundness statement first, with the alternative. M4.5e1c's overnight rule (build the
  recommended, fail-closed answer and log it here) applies only when the owner gives it for that session.
- **285, as the standing rule** (question 299): a test that is slow on the box because of what it must refute
  goes to `tests/long/` with a short default-tier twin.

Other answers that steer a later session are in that session's notes (§3, §4) or its plan: 66, 79, 189, 194, 196,
212, 215, 216, 222, 225, 246, 249, 262 and 267 (M4.8), 78 (M4.6), 120, 229, 236, 237, 253, 255, 259, 260, 261,
265, 266 and 287 (M4.5e2, and M4.5f1 and M4.5f2 for 266), 290–309 (M4.5e1e1's plan and record 0063), 304 and 310–337 (M4.5e1e2's two plans and record 0064), 326 and 332 (M8 and M4.6), 226–228 and 233 (M4.5e1's plan), 232, 239, 247–252, 254,
256–258 and 263 (M4.5e1c's plan and record 0061), 238 and 242 (M4.5e1f), 252 and 264–272 (M4.5e1d's plan), 269
(M6), 230, 234 and 241 (M8).

**Answered 224–286** (2026-11-11 to 2026-11-25: M4.5e1's planning and sessions e1a–e1d). Each is recorded with
the owner's words in the header of its record (0059 for 224–235, 0060 for 236–246, 0061 for 247–263, 0062 for
264–286) and in section 1 of its session's plan; the questions as asked, and the digests that stood here, are in
`git show 68384d1:internal/handoff.md` §8. M4.5e1a's to M4.5e1d's listed deviations (their status entries) stand.
What still steers a later session:
- **238 and 242** (`reads_range`'s amount from the range's first slot, a false alarm and not unsoundness; row
  order D): both are built in M4.5e1f, from e1c's spike S45 (r) (§3, "M4.5e1f").
- **249, 255 and 262** (nested `forall`, `!forall`, `exists`; a `forall` on the left of `||`; a `forall` only as a
  top-level conjunct in a loop invariant or a `check`): M4.8 (§3; `plan.md`'s M4.8 row).
- **253, 255, 259, 260, 261, 265 and 266** (the multi-class one-term false alarm; a multi-class guard that reads
  memory; the narrowings in loops and checks; `cerastes_old` in loop invariants; arithmetic over a non-first
  class's read; a `forall` naming a function with a contract; calls under a binder): M4.5e2 (§3). 266 also sends
  a `forall` inside a called `pure` body and calls in measures to M4.5f1, and mutual recursion to M4.5f2.
- **267 and 279** (a caller learns an application's range only from the function's `ensures`; two facts left
  out of `f__wf`): M4.8 (§3, "From M4.5e1d"). **269** (a `pure` function with no body in the unit): M6.
- **285 and 286** (two tests over 120 s on the box moved to `tests/long/`; Silicon capped at 16 provers, record
  0011 item 11): 285 is the standing rule above.

**Answered after M4.5e1d (2026-11-25, inline; the owner's words): 287–289,** each "Follow recommendation". The
questions as asked are in `git show 68384d1:internal/handoff.md` §8.
- **287** (fail closed: exit 3, never "verified"; audit round 4's R4-S5: a loop that fills a `must_init` range while
  its invariant also applies a pure function over the part not yet filled times out instead of reporting
  `uninitialized`): **M4.5e2** fixes it, with range atoms in loop invariants and the loop's flag rows (§3 "M4.5e2";
  `plan.md`'s e2 row).
- **288** (test mechanics): `tests/memory/negative/pure_sum_caller_write_between_short.c` (the two-element twin
  question 285's move made necessary) is kept.
- **289** (docs): the tutorial's `arrays_sum_offbyone.c` keeps `n < INT_MAX` in its `requires`.

**Answered 290–309** (2026-11-25: M4.5e1e's planning, 290, 291 and 300 in the question dialog; 292–299 the defaults
M4.5e1e1's plan built on; 301–309 settled with the Fable advisor in M4.5e1e1 under the owner's overnight instruction,
each fail closed). Record 0063's header has each, and the questions as asked, with the owner's words and the
advisor's rulings, are `git show 45d26bc:internal/handoff.md` §8. **301–309 are still the owner's to review.** What
still steers a later session:
- **292–294 and 306** (heap storage of a structure with an array member; an `owning` or marked member beside one;
  markers on a pointer to one; contract places through a heap read): lifted by M4.5e1e2's Part D, protected there.
- **295 and 309** (a structure value of more than 7 array elements is not passed or returned by value; 64 timed out
  at 600 s): M4.5f3; one constant, `vir::kMaxByValueArrayElements`, and three tests.
- **298** (the tutorial section "Arrays in aggregates"): M4.5e1e2. **299** is the standing rule above.
- **302** (an unused block-scope array typedef stays rejected) and **308** (a write inside a compound literal used as
  an object is rejected by position; unsound from M4.5c2a until M4.5e1e1, over-strict now): M8.
- **303** (C++'s `{0}` over an array member in an aggregate of two or more members is rejected) and **304**, below.

**304, answered by the owner (2026-11-25, written into this file):** "Follow the rejection like the compiler
warning." The question (not unsoundness: an over-acceptance of a warning shape) was whether C++'s `{0}` over a
structure whose first member is a nested structure, accepted since M4.5c1 although clang++ warns `-Wmissing-braces`
at `-Wall`, should agree with question 303's rejection of the array-member form. It is rejected too, by one rule for
both member kinds, and **built in M4.5e1e2** (question 312; its plan's record item 16, Part B and Flips G). The
planning's survey read the 38 C and C++ files under `tests/` and `doc/` that hold `{0}`: none uses the form in C++,
so no existing outcome changes.

**Answered at M4.5e1e2's planning (2026-11-25), in the question dialog:**
- **310** M4.5e1e2 as `plan.md` scopes it sizes at about 4,500–8,000 `src/` lines (4,700–8,450 after the surveys);
  how should sessions run from its one complete plan? **"12-hour rule (Recommended)"**: the order arrays of
  structures (code, then contracts and storage) → the leftovers of questions 292–294 and 306 → arrays of arrays →
  quantifiers; at each commit the next part starts unless the remaining work would pass twelve hours; parts not
  begun run from the same plan in a second session, with no second planning; nothing is cut to hold a session short
  (the owner had the continuation planned again all the same, 2026-11-25: its sessions run from that second plan).
  The alternatives were two sessions with a fixed stop after the leftovers, and the leftovers last.
- **311** Has the full `CERASTES_LONG=1 scripts/check.sh` run green on M4.5e1e1 since its session closed without it?
  **"Yes, it was green"**. M4.5e1e2's Step 0 makes no check of its base.
- **312** Where is question 304's rejection built? **"In M4.5e1e2 (Recommended)"**. The alternatives were a short
  session of its own and M8's warning sweep.

**Defaults M4.5e1e2's plan builds on, for the owner's review** (the planner's and the advisor's; each fail closed: a
rejection with a diagnostic and a pin, or a listed false alarm, never a wrong proof; answer inline, or leave them):
- **313** An array or a range whose element structure has an `owning` or marked member is rejected (M4.8, with arrays
  of owning pointers). Alternative: quantified closures over owning fields now, as a part of its own.
- **314** No leaf path is deeper than two index steps (`int m[2][3]`, `v[i].a[j]`); depth 3 is admitted only if the
  spike S48 (m) verifies it under 30 s a program, and is M8's otherwise. Alternative: depth 2 fixed, without the spike.
- **315** A pointer to an array is a parameter's type only (`int a[][3]`, `int (*a)[3]`); as a local, a member, a
  result or under `&` it stays rejected (M8). Alternative: pointer-to-array locals now.
- **316** `restrict` backed by a range over structures is rejected (M4.8). Alternative: 0043 item 10's argument
  extended to ranges now.
- **317** A whole-structure copy of a structure whose member is an array of structures is written leaf by leaf under
  the by-value limit of 7 and rejected beyond it (M4.5f3); no trusted `copy_arr_<E>` is added. Alternative: a trusted
  copy per element structure.
- **318** In a loop, a store to `v[i].x` of a local array or through a pointer reaches every place of that field (0043
  item 11's rule): the automatic "values kept" conjunct is lost for `v[k].y` of the same field, a listed false alarm
  with a written invariant as the way out (M4.8). Alternative: a per-leaf rule by name now.
- **319** The tutorial section "Arrays in aggregates" is written by the session that lands arrays of structures in
  contracts (the plan's Part C) and extended by the one that lands arrays of arrays (Part E).

**M4.5e1e2's questions (2026-11-25, its execution session).** Each went to the Fable advisor first.

**Answered by the owner at the planning of M4.5e1e2's continuation (2026-11-25, the question dialog).** Each
had gone to the Fable advisor first, in the first session, and was the owner's.
- **326** (**unsound, and on `main` today**; found by audit round 1 of M4.5e1e2's first session, not introduced
  by it). C's `restrict` beside another route to the same object verifies a program C leaves undefined (§1 has
  two of the programs; record 0064 item 19 at `f313023` has every form found and the enumeration that rejected
  all but the last). The question was what the continuation builds: reject every `restrict` parameter; keep the
  enumeration and add the auditor's rule for the one open form; or a session of its own for a path-precise rule
  first. **"Reject restrict (Recommended)"**: every `restrict` parameter is rejected with a diagnostic that says
  to drop the keyword (dropping it changes nothing for a program with defined behaviour); the acceptance rules
  of records 0037, 0040, 0043, 0044, 0045, 0047, 0052, 0054, 0060 and 0064 item 19 are withdrawn and their code
  removed; it is the continuation's first part, with an audit read of its own. **The owner approved the 17
  files whose outcome changes** (invariant 1): 8 positives under `tests/memory/` become rejection pins
  (`block_restrict.c`, `owning_restrict.c`, `range_restrict.c`, `restrict_body_owns_global.c`,
  `restrict_body_pair_ok.c`, `restrict_owned.c`, `struct_global_restrict.c`, `struct_value_restrict.c`) and their
  9 error twins under `tests/memory/negative/` are retired; the 33 pins that already reject `restrict` and
  `forall_reject_f_restrict.c` keep their rows, and one that moves comes to the owner first. A path-precise
  model of `restrict` is an M8 entry (`plan.md`).
- **320** (fail closed either way). The pin `tests/memory/negative/array_reject_pointer_to_array.c` rejects the
  **parameter** `int f(int (*p)[3])`, and M4.5e1e2's first plan both listed that pin as unchanged and scoped the
  spelling as an array-of-rows parameter. **"Accept it in Part E (Recommended)"**: the pin is retired for a
  positive of the same program with an error twin, and a new pin records that a pointer to an array as a local
  stays rejected (M8). The alternative was to keep the pointer spelling rejected until M8.
- **322, its second half** (not unsoundness: a warning shape). A truth test or null comparison of an element's
  address on an array of scalars (`if (&a[1])`, `&a[1] == NULL`, `&p[1] != NULL`, `&s.a[i]`, `&p->a[i]`): GCC
  warns `-Waddress` at `-Wall`, clang is silent, cerastes has accepted them since M4.5e1a, and the first session
  rejects the same tests on arrays of structures. **"Reject them too (Recommended)"**: one rule for every
  element address, by the tag `array_address_null_test`, in the continuation's first part; an existing file
  whose outcome changes comes to the owner first (three passes found none). The alternative was M8's sweep.
- **325** (not unsoundness: a warning shape). A lone zero that is not the literal `0` where `{0}` elides braces
  (`{(0)}`, `{'\0'}`, `{false}`, `{NULL}`, an enumerator): clang's C warns `-Wmissing-braces` at `-Wall`, GCC is
  silent, cerastes accepts them by their value. **"Probe, reject if the IL can tell"**: the continuation's Step
  0 runs one IL probe (P84 (a)); if the IL tells the token `0` from the other spellings they are rejected like
  the compilers in its first part, and if not the entry goes to M8's sweep. The recommendation had been to
  leave them to M8 without a probe.

**Settled with the advisor in M4.5e1e2, for the owner's review:**
- **321** (not unsoundness: an over-acceptance of warning shapes). Question 304's rejection was planned as "C++'s
  `{0}` over a first member that is a structure or an array, in an aggregate of two or more members". The compiler
  matrix (clang 21.1.8, GCC 16.0.1, 668 shapes) measured more: clang++ flags (`-Wmissing-braces` at `-Wall`) every
  list in which an elided subaggregate's parent is an array or a structure of two or more members (so also
  `struct P v[2] = {0};` and `int m[2][3] = {0};` in C++), and GCC flags in C every elision except the lone
  whole-initializer `{0}` (so a nested `{{0}}` whose inner `0` elides braces). **Answer taken:** the plan's rule
  for this row, "the measured set is the rule": record 0064 item 16 states both predicates; C's lone `{0}` over
  a structure whose first member is a structure that begins with an array, which the base wrongly rejected, is
  accepted. Step 0 found no existing test or doc example that changes outcome; one that does goes to the owner
  before it is touched. Alternative: the planned narrower wording.
- **322** (not unsoundness). GCC warns `-Waddress` at `-Wall` on a truth test or null comparison of a row or of
  an element's address (`if (m[0])`, `&v[1] == NULL`); no rule covered it. **Answer taken:** rejected for rows and
  for element addresses of arrays of aggregates (`array_address_null_test`, record 0064 item 5), built from the
  matrix's cells. **The same forms on an array of scalars (`if (&a[1])`, `&a[1] == NULL`) are accepted since
  M4.5e1a and are left unchanged: whether to reject them was the owner's** (answered above: "Reject them too"; the
  continuation's Part R builds it). Alternative: one rule now for every
  element address.
- **323** (fail closed: a cost, never a proof). The spike's copy `v[i] = v[j]` between two elements whose
  structure has an array member verified, with its guard decided, in 34.5 s and 35.2 s on the box, over the
  spike table's 30 s line. **Answer taken:** it is built (Part E), its tests timed under the test budget's rule 3
  (over 120 s: `tests/long/` with a short twin). Alternative: reject such copies until M4.5f3.

- **324** (evidence completeness, not unsoundness). The spike S48 was capped at 56 fixtures and ran 54; audit
  round 1 asked for more (a vacuity twin for seven rows, the trusted `zero_arr_<E>`, and fixtures for the pairing
  rows as the audit rewrote them). **Answer taken:** about 15 fixtures are added on top of the cap, as the plan's
  budget allows for an audit's requests.

- **327** (not unsoundness: an over-acceptance of a warning shape; the default was built, for the owner's review).
  clang++ warns `-Wc99-designator` without a flag ("brace elision for designated initializer is a C99 extension") on
  every designated structure or array member whose value has no braces of its own (`struct T t = {.p = 0};`), and
  cerastes has accepted that form in C++ since M4.5c1. **Answer taken:** rejected in C++ (`unsupported_expression`,
  record 0064 item 16), by the union rule of record 0020 item 5 and the rule 0059 already has for that warning on
  array designators; C is unchanged (both compilers are silent on a zero; a non-zero value there is rejected, as
  GCC flags it). No existing test or doc example holds one. Alternative: leave the C++ acceptance for M8's warning
  sweep.

- **328** (fail closed: a timeout is exit 3, never "verified"). A function that holds several local arrays of
  structures is slow: Part B's test `array_struct_init.c` held five in one function and timed out in C at Silicon's
  300 s cap (260 s in C++), while each array alone verifies in about 10 s. Measured alone on the box in C: 1, 2, 3, 4, 5
  arrays `struct P a[2]` in one function take 7, 11, 17, 36 and 65 s; the original five mixed arrays 735 s; five
  `int a[2]` arrays 14 s, as on the base (the cost is new with the per-leaf permissions; no case split is emitted). **Answer taken:** the test is split into one array per function, record
  0064 item 6 lists the limit, and the cost goes to M4.8's cost items (with questions 66 and 189). Alternative: a
  diagnosis in the lowering before Commit 2.

- **329** (**unsound on `main`**; audit round 2; fail-closed fixes built, for the owner's review with 326). After
  the first `restrict` rule was built, round 2 found two more forms that verify: the `restrict` function passes
  `p` to a callee that holds only `owns(*x)` and whose own body names the global; and an atom that should keep
  another pointer apart is a range whose count can be 0 (`owns(*p) && owns_range(q, n) && q == p` called with `n
  == 0`). **Answer taken:** item 19 gained clause (iv) (no pointer that can reach the parameter's object is passed
  to any function) and the pair rule (an atom excludes only when it is sure to hold something, or both atoms are
  ranges with the same count). No test under `tests/` changed outcome; one hand-built VIR self-test case that
  pinned the unsound acceptance (`rngm_restrict_range_other_covered`) was re-expected as a rejection. The
  auditor's own fix for the second form (a range counts only with a count positive by syntax) would have flipped
  `tests/memory/range_restrict.c`, which is the owner's; the pair rule is narrower and leaves it verified.
  Alternative: leave both forms open for the owner's complete rule.

- **330** (test mechanics; fail closed: a red check reports nothing as verified). The full check at Commit 2 failed
  on one new hand-built VIR self-test case that reported two errors at a line where its header registers one. **Answer
  taken:** the case was corrected (it asserts one fact) and the check run a second time on the fix commit.
  Alternative: close the session with the check red.

**The defaults the continuation's plan builds on (2026-11-25, its planning), for the owner's review** (the
planner's; each fail closed: a rejection with a diagnostic and a pin, or a matter of process, never a wrong
proof; answer inline, or leave them):
- **331** (process). The first plan's Part C is two parts in the continuation, each with a commit, an audit
  round and a split point: C1 (ranges over structures in contracts), then C2 (static storage and members).
  Alternative: one part, as the first plan had it.
- **332** (fail closed: a rejection). The `restrict` rejection covers a declaration without a body as well as
  a definition. No test, doc example or vlibc header has such a declaration today; M4.6 decides what `memcpy`'s
  declaration needs. Alternative: accept and ignore `restrict` on a declaration without a body now (its contract
  is trusted anyway).
- **333** (fail closed: a rejection stays). A structure value stored into an element (`v[i] = f();`, `v[i] =
  (struct P){1, 2};`) stays rejected until M4.8; the user writes `struct P t = f(); v[i] = t;`. Alternative:
  lift it in Part C2 with record 0044 item 5's rules.
- **334** (test mechanics). The 33 pins that already reject `restrict` in particular situations stay, although
  each now pins the one rule; only their `Pins:` comments change. Alternative: retire all but one per spelling,
  which is the owner's under invariant 1.
- **335** (fail closed: over-strict by one form). Question 322's rule is the predicate the first session built
  for arrays of structures, so `&p[0]` in a truth test or a null comparison is rejected with the other indexes
  although GCC is silent on that one form. Alternative: accept `&p[0]` on a pointer, for scalars and structures
  alike.
- **336** (process). `scripts/box_lane.sh` refuses to run without its two lane variables instead of falling
  back to M4.5e1e1's lanes.
- **337** (test mechanics; no `expect:` row of an existing pin changes). The harness wants a must-fail test
  under `negative/` with a `twin:` line that names a test that verifies, and 14 of the pins that already reject
  `restrict` name one of the 8 re-expected files as their twin. So the 8 move to `tests/memory/negative/`, a
  new positive `tests/memory/restrict_dropped.c` (the same functions with the keyword dropped, as the
  diagnostic tells a user to write them) with an error twin stands for the rule, and those 14 pins and the 8
  name it in their `twin:` line (the plan's Flips R lists the 14). Alternative: keep the 8 as positives by
  deleting the keyword from them, which changes no outcome but leaves eight tests that no longer test what
  their names say.

**Settled with the advisor in M4.5e1e2's continuation, for the owner's review** (the session of 2026-11-26;
numbered from 338):
- **338** (fail closed: a rejection either way; the one form accepted silently is neither verified nor
  called). Record 0064 item 19 says a `restrict` parameter is rejected "at the function's head", on a
  declaration without a body too (default 332). As built, a definition's diagnostic is at its head, but a
  declaration without a body is read lazily, as before Part R: its diagnostic is at the first call of it in the
  unit (one row, however often it is called), or at the declaration when its signature is read early (a
  structure result or a pointer-to-structure parameter, a parameter marker, `not_invalidating`, a block atom
  in its contract); a bodiless `restrict` prototype that the unit never calls and that nothing reads early
  draws no diagnostic. **Answer taken:** keep that, and say it in item 19, in the page and in the tests (a row
  per placement and per spelling of the keyword; 20 single-file runs on the build found no spelling that
  verifies). An eager rejection of every such prototype was not built: it would reject a unit for a
  declaration it only includes and never uses (a header's `memcpy`, M4.6), and a flip of that kind would be
  the owner's. Alternative: the eager loop in the encoder's declaration pass.
- **339** (not unsoundness: a warning shape; fail closed either way). Question 325's probe, P84 (a), took its
  middle branch: the front end's IL tells a literal zero from `'\0'`, an enumerator, `1 - 1`, `false`, `NULL`,
  `nullptr`, `-0`, `+0` and `(int)0` by fields of the constant, but no field records a parenthesis, so `(0)`,
  `((0))`, `(0u)` and a macro whose body is `(0)` look like the literal. **Answer taken** (the plan's default
  for that branch): in C, an undesignated lone zero that is the whole initializer and elides braces where
  record 0064 item 16's predicate applies is rejected (`unsupported_expression`) when the IL shows it is not a
  literal; the parenthesized spellings stay accepted and are listed for M8's warning sweep with the probe's
  evidence, and so are designated elements and zeros inside nested lists, which the probe did not run.
  Alternative: build nothing and send the whole entry to M8.

- **340** (fail closed: the file stays rejected by its other row). The staging pin
  `tests/memory/negative/forall_reject_member_array_code.c` has two rows at L52 (`forall_function` and
  `unsupported_annotation`), and the plan's Flips A retires both in Part F. The second row is the old rejection
  of `owns_range` over structures on that line, which Part C1's lift removes. **Answer taken:** Part C1 retires
  exactly that one row (its line kept by a comment), as a pre-approved flip made one part early; the
  `forall_function` row stays for Part F; any other row that moves at Part C1's joined proof is a new question.
  Alternative: the owner decides.
- **341** (fail closed; not unsoundness: no body is verified and the contract is trusted either way). Audit
  round R found that a called function without a body whose declarations disagree on `restrict` (one with the
  keyword, one without, in either order; in C++ plain first and `__restrict` second) is accepted: the front
  end's merged type drops the qualifier. The record had no rule for which declaration counts. **Answer
  taken:** a function without a body is rejected if any of its declarations in the unit gives a parameter the
  qualifier, in both modes, reported lazily as question 338 has it; built with Commit 2, a pin per form the
  audit found; Commit 1's build still accepts the disagreeing form, and the record and the page say so.
  Alternative: state the rule as built (only the merged type counts).
- **342** (fail closed: over-strict; for the owner's review because it widens question 335 from one form to a
  family). Item 5's rule (a truth test or null comparison of an address formed by an index) also rejects a
  test written through a macro (`assert(&a[1] != NULL)`, `ISNULL(&a[1])`) and, in C, `&a[1] == (int *)0`.
  Neither compiler flags these (only g++ the cast form, in C++), and before Part R cerastes accepted them for
  arrays of scalars. No existing test or doc example holds one. **Answer taken:** listed as over-strict in
  record 0064 item 5 and on the page, for M8's warning sweep (the front end does not record that a test came
  from a macro). Alternative: narrow the predicate now.
- **Listed with them, from audit round R** (no question; for the owner's eye): the one form of `restrict`
  still accepted is the type of a cast or of a compound literal (`(int *restrict)p`, `*(int *restrict){p}`),
  which C23 6.7.3.1 gives no promise by the auditor's reading and which one compiled program confirms; not
  shown wrong, on the base too, an M8 entry with the model of `restrict` (record 0064 item 19).

- **343, OPEN FOR THE OWNER** (fail closed: an internal error, exit 3, never "verified"; it went to the advisor
  first, who left the fix to the owner because it changes existing programs' Viper). A structure pointer
  formed by indexing a pointer base outside the slots the function's atoms cover (`&p[n]` one past a range of
  structures, `&p[i]` with `i` in `[0, n]`, and `&p[1]` under `owns(*p)`) cannot be shown well-formed as a
  structure pointer. Passing it to a function with no atom on that parameter, directly or through a local copy,
  or returning it, ends in exit 3 at Silicon's own check (the explicit row record 0028 item 11 asks for is
  missing). It predates Part C1: `g(&p[1])` under `owns(*p)` does the same on Commit 1's build, since the
  first session's Part B accepted `&p[i]` on a pointer to a structure. Record 0064 item 7 lists the limit and
  the page says it; the planned pin `range_struct_past_end_false_alarm.c` did not land (the harness cannot pin
  an exit 3). Seven probes: `build/m4.5e1e2b/q343/`. **Recommended:** one explicit row, `permission`, in source
  terms ("passes `&p[n]`, which may point outside the structures this function holds"), written at a call for
  every structure-pointer argument and at a `return` for a structure-pointer result; approve that the Viper
  of existing programs with such arguments or results gains that row (a neutrality run then lists them), and
  build it at the start of the next session, before Part D. **Alternative:** keep the exit 3 listed
  until M4.5e2, which gives pointers into arrays their bounds rows anyway.

- **344** (fail closed either way). Record 0064 item 7 lists `g[e].x`, a member of an element of a global
  array of structures, among the reads a contract may make, but neither Part C1's nor Part C2's row of the
  plan named it, and the encoder rejected it; `p->v[e].x`, an element of a member array under `owns(*p)`, is
  not named by the record at all (0063 item 7 has both forms for arrays of scalars). **Answer taken:** both
  are built in Part C2, because the lowering's contract-read path for a structure element extended to these
  bases at small cost (framed by the whole-array atom of the global, or by the structure atom's array part),
  each with a positive, an unframed twin and a wrong-value twin; a second index, a base reached through a
  load and an element of a structure passed by value stay rejected. Alternative: keep both rejected with a
  pin until M4.5e2.

- **345** (fail closed: a false alarm, never a proof). `f__wf`, the check that a contract is well formed, has
  no tie between the entry and exit values of a member of an element of a member array of structures reached
  through a pointer: with `struct T { struct P v[2]; int n; }`, `requires reads(*p) && p->v[0].x != 0` and
  `ensures reads(*p) && (r == 0 || r == 10 / p->v[0].x)` report `division-by-zero` at the `ensures`, under
  `reads(p->v)` too. A scalar member (`p->n`), a nested structure's member (`p->a.x`), a parameter's range
  (`p[e].x`) and a global array (`g[e].x`) are tied and verify. Record 0063 item 7 lists the same false alarm
  for a member array of scalars (`reads(p->a)`, `p->a[0]`: its N1), so this is that limit carried to arrays of
  structures, not a new one. **Answer taken:** listed in record 0064 item 7 and pinned; the positive control
  of the `f__wf` pins for a member array was reworded so that it verifies without the tie. Alternative: give
  `f__wf` the tie for array leaves of a structure atom, for scalars and structures at once (it changes the
  Viper of existing programs with `reads(p->a)`, which is the owner's to approve).

- **346** (fail closed: two rejections with a diagnostic). Audit round C2 (finding 4) found two forms that
  Part C2's build rejects and record 0064 did not list, the second against the record's own words (item 7
  named `gt.v[e].x` as a contract read and allowed element reads under `old`): `fp(gt.v[1])`, an element of a
  member array of a GLOBAL structure passed by value (rejected with a message about "a pointer read from
  memory", though none is read; `struct P e = gt.v[1];`, `fp(t.v[1])` of a local and `fp(g[1])` are accepted),
  and `cerastes_old(gt.v[1].x)` (rejected, while `cerastes_old(g[1].x)`, `cerastes_old(p->v[1].y)`,
  `cerastes_old(gt.n)` and `gt.v[1].x` outside `old` are accepted). **Answer taken:** both are listed in
  record 0064 (items 7 and 12) and on the page, each with a pin; the record's item 7 is corrected to what the
  build does. Lifting them is encoder work that would have needed a further audit read at the session's end;
  which session lifts them is not decided (§3 files them for the next session's attention; they touch the
  code Part D works in). Alternative: lift both before Commit 3. **Amended the same day:** the first form is
  rejected in C only. The record's sentence generalised a C probe; the joined proof then found that C++
  accepts `fp(gt.v[1])` (the C front end folds the element's address into a constant the encoder refuses,
  the C++ front end keeps the subscript), and the second audit read (round C2b) probed the accepted C++ path
  twenty ways without finding a wrong result. The pin is split: the `cerastes_old` row in both modes, the
  by-value row in C only, and a C++ positive with a twin (a wrong element; a caller without `gt`'s atom)
  tests the accepted path. The acceptance in C++ rests on those probes.
- **347** (fail closed: a timeout is exit 3, never a proof; this session's own change caused it). To give
  the Viper lint the shape it expects, the lowering now states the length facts of a global array of
  structures, and of a structure global's member array, as clauses of their own after the global facts
  (`global_fact_clauses()`), where Part C2's first build stated them inside the facts' conjunction. The
  conjunction is the same (audit round C2b read it). But one program that had a verdict lost it: a function
  other than `main` with `requires reads(gt) && gt.v[1].x == 3` that calls a `reads(gt)` function and checks
  that leaf took 17 s on the build before the split and gets no verdict in 200 s after it, on a quiet box
  (the auditor's `q05`; the orchestrator's bisect in lanes `edg` and `il`). The two-call chain over such a
  global (`e06`) had no verdict in 200 s before the split either, and round C2 had found the same for a
  five-leaf check before it, so the family was already fragile. No committed build accepted these programs
  (Commit 2 rejects the type), no existing program's Viper changed, and every test and self-test case whose
  Viper the split changes was run on a split build (15 test files, none over 60 s, and 39 cases). **Answer taken:** the split is kept, because
  Commit 3's `src/` had to equal the snapshot the second audit read took, and a revert needed new lowering, a
  lint rule for the fused form, a third read and reruns at the session's end; the cost is listed in record
  0064 item 12 and on the page. **Alternative, for a later session:** the fused form (or the split only in
  `program_start`) with a lint rule for it, measured on `build/m4.5e1e2b/audit-C2b/t/q05_reads_gt_nonmain.c`
  and `e/e06_split_gt.c`.

**Settled with the advisor in the second session of M4.5e1e2's continuation (2026-11-26), for the owner's review:**

- **348** (fail closed: a rejection with a diagnostic, pinned). Record 0064 item 15 and the continuation
  plan's Part D table were written before Part C2 and speak of a structure with an array member of SCALARS
  (`struct SA { int n; int a[4]; }`, the only shape the spike's rows (h), (i), (k) and (l) verified). Since
  Part C2 a structure may hold a member that is an array of structures (`struct T { struct P v[2]; int n; }`),
  and every gate Part D lifts tests "has an array member" whatever the element. Lifted as written, such a
  structure would reach heap storage, the marker sites, heap-read places, partnerless atoms and written atoms
  with Viper shapes no fixture has verified, one of them a trusted contract (the typed view of `malloc` and
  `calloc` holding one quantifier per member of every element under the allocation's token). **Answer taken:**
  Part D builds its six positions for array members of scalars; a structure whose array member has aggregate
  elements (structures, and from Part E arrays) stays rejected at each of the six, by the encoder at the C
  line, by `check.cpp` as the backstop and by an internal error in lowering at each site that would otherwise
  take Part C2's general leaf path; everything Part C2 accepts for such a structure is unchanged; the record
  (item 1's table, item 15's last clause) and the page list it, the message names M4.5e2. **Alternative:**
  the general leaf path at these positions now (about 80 to 150 lines more, four shape families to put
  through Silicon first, about ten more tests). **For the owner:** which session lifts it; the record names
  M4.5e2, which builds `malloc(n * sizeof(struct S))` and with it the same per-member rows under a token.
- **349** (fail closed: test mechanics). A `must_init` filler that leaves an element of an array member
  unwritten: record 0064 item 15 said `uninitialized`; record 0042 item 5 gives `must_init`'s flag row the
  category `postcondition`, and three existing tests expect it. **Answer taken:** `postcondition` at the
  closing brace; item 15's wording corrected (a caller's read of an element nobody wrote is `uninitialized`).
- **350** (a soundness condition, kept). The continuation plan's Flips A (D) listed three rows of
  `tests/memory/negative/struct_array_reject_annotations.c` as retired for positives. Two of them must not
  be: L37, `void give(struct S *p) ensures(owns(p->a));`, is a partnerless atom on a FORMAL of a function
  without a body, which record 0052 item 2a rejects because a trusted contract would hand the caller a second
  write permission to cells it already holds; L39, a marked parameter with no atom, is rejected by record
  0042 item 4. **Answer taken:** each row of Flips A (D) was decided by the joined proof: a row that still
  rejects with the same tag at the same line kept its `expect:` line (its message read: 2a's and 0042's), a
  row that lifted was retired with a positive of the same program and a twin. L37 and L39 are not flipped;
  both implementers showed 2a's check still in front of the lifted gate, with two self-test cases.
  Alternative: none (flipping L37 would reopen the hole 0052 item 2a closed).
- **351** (fail closed: cost). `calloc`'s typed view for a structure with an array member states "every
  element is zero" as one clause per element, since the IR has no quantifier over a member array's elements
  outside a `pure` function. **Answer taken:** kept (a quantified zero line would be new trusted text with
  no fixture); see 354 for what it cost.
- **352** (fail closed: a rejection; Part E's). A `const` row parameter (`const int a[][3]`, `const int
  (*a)[3]`, `const row *a`) is rejected as a `const` array (M4.6), stricter than `const int *p`. **Answer
  taken:** stays rejected, a pin row per spelling with Part E (record 0064 item 13). Alternative: accept it
  in Part E with the row's reads as `reads`.
- **353** (fail closed: a rejection beyond a limit; Part E's). A whole-structure copy, by-value passing and a
  compound literal of a structure with a matrix member, or with a member array of structures whose elements
  have an array leaf, are written cell by cell in the encoder draft, the limit of 7 counting cells.
  **Answer taken:** as drafted, record 0064 item 12 amended; **the encoder's cell order and the lowering's
  value view must be one enumeration before any such value is accepted** (they differ today). Alternative:
  an array copy per nested array leaf (a new shape for the lowering).
- **354, FOR THE OWNER, first** (fail closed: a rejection with a diagnostic in place of an exit 3; inside
  question 292's scope). With one zero clause per element (351), `calloc` of a structure with `int a[64]`
  got no verdict: Silicon's own stack overflows in the view's well-formedness method. Measured through the
  driver on the box, both modes: an array alone verifies up to 50 elements (78 s) and fails from 52; the
  limit is a JVM stack depth, so it may differ by platform. **Answer taken:** `calloc` of a structure that
  has an array member is rejected at the call when the structure holds more than 24 values in all (each
  element and each other member counted; about half the largest passing count); `malloc` has no limit. The
  diagnostic says to use `malloc` and write the elements, and says what that costs today: elements written
  one by one can be read back, but a read of an element that only a loop wrote is reported as
  `uninitialized` (a listed false alarm until Part F's written quantified invariants). On macOS the test
  at the cap verifies in 5 s in both modes (one probe; where the edge lies there was not measured).
  **Alternatives:** a
  different framing of the per-element reads in the contract's well-formedness method (lowering work of
  unknown size), or one quantified zero line per array member in the view (new trusted text; the record
  names M4.5e2, which needs it for `calloc(n, sizeof(T))`). **Recommended:** let the cap stand until M4.5e2
  builds the quantified line; raise the constant only after a run on macOS shows the margin.
- **355** (fail closed: stricter than the record's first wording). A written atom's array part in a loop
  invariant was to take the first slot's ghost amount (the spike's fixture); the il-analyst built it at
  `write`, because with the ghost amount an `owns` invariant could stand over a fraction at the loop's
  entry. **Answer taken:** as built; the record's clause was written after the code (a deviation, listed).
  **With it, a cut in part that was not asked first** (the plan's cut 4; reported to the advisor at the
  close, who accepted it on question 348's ground: no fixture, fail closed): only `owns(*p)` and
  `owns(p->a)` on a pointer variable are built; a written `reads(*p)` or `reads(p->a)` and a written atom
  whose pointer is read through memory stay rejected (the spike's fixture covered neither), each with a
  diagnostic and a pin row; the record names M4.5e2. Alternative: build them in the next session, each
  with a fixture put through Silicon first.
- **356** (fail closed: test mechanics). The twin of the owning-member loop test, a loop that replaces the
  member's block without freeing it, reports three rows where one was registered. **Answer taken:** the wrong
  form drew no row that the CORRECT form (free the block, then allocate another) does not also draw, so it
  tested nothing and was dropped; the positive got a twin that fails for a reason of its own (a loop that
  frees the member's block and uses it: `permission`); the correct form, which reports six rows, is pinned
  as a listed false alarm (`negative/struct_array_loop_owner_member_reassign_false_alarm.c`; the same six
  rows for a structure without an array member on the base build). Alternative: no pin.
- **357, FOR THE OWNER: a permission was denied** (fail closed: nothing was loosened). The Viper lint
  refuses the emitted Viper of two new tests, `struct_array_chain_wf.c` and its `_wf_division` twin in their
  first form: a contract with two `reads` atoms through two loaded pointers gives the array part's
  `ensures` rows and return rows record 0045 item 7's partner-sum amount (nested conditionals over the
  method's `Perm` formals), which the lint's rule for quantified rows does not admit. A test-mutator tried to
  edit `scripts/lint_viper.py` to admit that amount and to narrow the kept-flag refusal; **the permission
  system denied the edit ("Undermining Oversight")**, the agent stopped, and neither the orchestrator nor
  another agent made it. Both programs give their registered results on the build; only the lint refuses
  their Viper. **Answer taken (with the advisor):** the two files are kept unchanged under
  `internal/pending/m4.5e1e2c/` with the proposed rule and its author's soundness note; a reduced
  pair with one loaded pointer (no partner sum; its Viper passes the lint unchanged) is in the suite, so the
  `other_root` row has no C-level pin; the shape is listed as a class of
  accepted programs whose Viper the lint refuses; round D2 read the amount code (`ens_amount`, the
  obligation of a contract without a body, the method's rows, the array part's return rows; not the rest of
  `atom_returns`): a bodiless method's `ensures` does carry the amount and its callers inhale it, the
  contract's well-formedness method carries the obligation that makes it the partner's own formal, and no
  caller with a verdict ended with more permission than it gave (eight probes; six timed out); the short
  read D2b confirmed that the refusals in question are unchanged by the session's other lint work. **For the owner:** approve the lint rule (then a session applies it with its
  twins and restores the two files), or keep the shape listed.

**Settled with the advisor in the third session of M4.5e1e2's continuation (2026-11-26/27, Part E), for the
owner's review** (written as each was settled; 363 is the owner's):
- **358** (fail closed: test mechanics; invariant 2). The plan's 37 test files for Part E came to 44 by the
  test-mutator's count. **Answer taken:** the 37 limits files, never must-fail rows: the `static` local
  matrix gets a second failing function in the global matrix's twin, the typedef'd row spellings a row in
  the spellings' twin that fails on the second index, and the loop's first-index twin stays cut (the same
  bound row as the local matrix's). Alternative: three more files.
- **359** (test mechanics). A local matrix appears in no contract, so it has no `f__wf` pair. **Answer
  taken:** `f__wf` pairs only for the storages whose contract reads the record builds (a row parameter's
  range, `v[i].a[j]`, and, with 362, a global matrix and a matrix member); sequencing pairs for those and
  the local matrix.
- **360** (test mechanics). C's lone `{0}` for a matrix sits in a positive under `#ifndef __cplusplus`.
  **Answer taken:** as written, since no `expect:` row of any file sits inside such a block.
- **361** (fail closed: an acceptance with a pin as its alternative). The encoder draft accepts a row
  parameter forwarded whole as a call argument (`g(a)`). **Answer taken:** accepted, built with the row
  parameters, on condition of a self-test case through Silicon for the `owns` and the `reads` form and a
  test pair; if Silicon does not take it the encoder refuses it. Alternative: refuse it (one line).
- **362** (a choice inside the record's rules; the record was silent). Contract reads of a matrix in
  static storage and of a matrix member (`g[e][e2]`, `p->m[e][e2]`, `gt.m[e][e2]`, `r->m[e][e2]`).
  **Answer taken:** built, as question 344's reads were in Part C2 (the encoder sends them down the row
  parameter's branch); question 346's `old` limit and question 345's missing tie carry over. Alternative:
  rejected with a pin (M4.5e2). Record 0064 item 13.
- **363, FOR THE OWNER, first** (fail closed: a rejection). Record 0064 item 2's wording admits a matrix of
  structures (`struct P m[2][2]`, a row parameter `struct P a[][2]`), and the encoder draft accepted it; no
  spike fixture, no trusted method name and no planned test covers a row whose cells are structures. The
  advisor ruled that leaving unbuilt a shape the record admits is not its to decide. **Taken until the
  owner answers:** the encoder rejects it at its C line (`unsupported_type`), `check.cpp` keeps its
  refusal, a pin holds a row per storage, item 1's table has the row. **Recommended:** M4.5e2 builds it
  with question 348's shapes, after a spike fixture. **Alternative:** a part of its own in the next
  session. An array of structures whose element has an array member (`v[i].a[j]`) is built.
- **364** (fail closed: a rejection). A contract read of a cell of a matrix member of a structure passed
  or returned by value (`v.m[i][j]`, `r.m[i][j]`). **Answer taken:** rejected (`unsupported_annotation`,
  M4.5f3), as record item 12 rejects `t.v[1].y`; `g().m[1][2]` in code is rejected too, where `g().a[1]` is
  accepted (listed). Question 353's test changes form, not strength: the order of cells inside the matrix
  is pinned by the self-test, and the alignment of the matrix's cells against the members beside it (the
  unsound direction) by a by-value pair with a `requires` on those members and a twin shifted by one cell.
  Alternative: build a two-index value read (a new lowering shape with no fixture).
- **365** (fail closed: a rejection kept). `sizeof(a[0])` of a row parameter verifies in code in all six
  spellings and is rejected in a contract (`unsupported_type`, "an array type in sizeof …"). **Answer
  taken:** record item 13 says "in code"; the contract form stays rejected, with a pin row. It is
  over-strict: the frozen base accepts `sizeof` of a global array, of `g[0]` and of an array member in a
  contract (probed). Alternative: accept it in a contract (the value is the front end's constant).
- **366** (wording; fail closed either way). The encoder draft gave `&m[i]`, the address of a row,
  `unsupported_expression`; the base prints `unsupported_type` for `&a` of a one-dimensional array.
  **Answer taken:** every address of a row draws `unsupported_type`; `&u.m`, the address of a whole
  member array, keeps the base's `unsupported_expression` (an existing row). Alternative: keep the
  draft's tag and correct the new pin under question 160.
- **367** (fail closed: a false alarm that the record's rule removes). The first lowering of ranges over
  rows did not tie the entry and exit ghosts of `a[e][e2]` in `f__wf`, so `requires(reads_range(a, n) &&
  a[0][0] != 0)` with `ensures(… r == 10 / a[0][0])` reported `division-by-zero`; the registered positive
  found it at the second joined proof. **Answer taken:** record item 7 ties a parameter's range, so the
  tie is built: only under a `reads` site, the key holding the formal and both indexes, with a twin under
  `owns_range` and one over another cell; if it needs an assumed fact of a new shape it is listed and
  pinned as question 345's is. Alternative: list it as a false alarm now.
- **368** (test mechanics). A row parameter forwarded with a count the caller does not hold draws two
  `permission` errors at the call, where the draft registered one. **Answer taken:** two expect lines:
  record item 7 gives a forwarder two errors (the shape row and the first leaf's marked row) and a local
  array one short a single one; a correction under question 277, listed in status.
- **369** (fail closed in the verifier; a gap in the Viper lint that predates this session). The lint's
  `leaf_value` accepts ANY literal range `lo <= v <= hi` as "the range of an unknown initial value" in
  `program_start`'s rows for scalars, one-dimensional arrays and structure leaves: an existing fixture with
  its upper bound changed to 5 lints clean (`build/m4.5e1e2d/tm4/lax/garr_narrow.vpr`). The lowering states
  the type's own range, so nothing is wrong today; the lint would not catch a lowering bug there, and those
  rows are trusted text. **Answer taken:** not edited in this session (it would change the lint's verdicts
  under every earlier `program_start` with one late audit round to read it); this session's new group
  rules (matrices, matrix members, arrays of structures with array members) require the type's own range,
  with a narrowed twin. **For the owner (recommended): tighten `leaf_value` to the type's own range at the
  start of the next session, checked against the whole regeneration.** **Answered by the owner on 2026-11-27
  (in the session, about 03:38 UTC): "I'll follow your recommendation on the issue with 369 for the next
  session."** The next session's first step tightens it (with question 373's rules, recommended with it).
- **370** (test mechanics). Two depth-3 rows of the return-address pin: no new file; they go back into
  `negative/matrix_return_local_address_members.c` with slice 4.
- **371** (test mechanics). The staging pin for slice 4's constructs was registered from tags only; its
  lines are registered from a probe run of the gate build before its first run.
- **The lint's three lifted refusals for global matrices** (not a numbered question; for review with the
  second audit round's reading): the row-length fact of a global matrix with a count other than 1 in a
  contract, a literal first range over the global's array place in `main`'s loops, and two-index rows over a
  global's block term were refused before and are admitted by new rules, each tied to `program_start`'s own
  group for that global (record 0064 items 3, 9, 10 and 17); no amount, guard or kept-flag rule was touched
  (question 357's boundary). Provisional until the audit has read the lint's diff.
- **372** (test mechanics: how the lint's rules for a matrix member reached through a pointer get the
  member's counts). **Answer taken:** a per-unit context that the lint sets from the unit's own pinned
  `alloc_struct_<n>_<tag>` and resets for every unit, consulted only to accept rows equal to that
  allocation's counts, with a self-test for the reset. Alternative: thread the counts through every caller.
- **373** (fail closed in the verifier; gaps in older lint rules, found by audit round E2a's mutants). The
  lint admits a one-index permission row over the slots of an array-of-STRUCTURES global's block (it reads
  the elements flat), a row with an untied field over a one-dimensional global, and a doubled field in a
  contract's rows (the auditor's mutants Z1, Z2, Z3 and A5, `build/m4.5e1e2d/audit-E2a/lint/`). No lowering
  path emits any of them. **Answer taken:** not tightened in this session, as with 369; for a MATRIX the
  flat row is refused now. **For the owner, with 369 (recommended): tighten these older rules at the start
  of the next session, checked against the whole regeneration.**
- **374, for the owner's review** (the Viper lint; nothing unsound was accepted by it: the verifier's
  verdicts do not pass through the lint). A test-mutator lifted a refusal of the lint that is
  guard-shaped, which the session's own rule said to stop on and report first (question 357's boundary:
  no amount, guard or kept-flag rule is loosened to make a test's Viper pass). The rule: `block_clause_problem`
  refused the structure's guarded well-formedness conjunct (`(p != null && offset_of(p) <
  len(block_of(p))) ==> (the member lengths)`) inside a CELL atom's clause when the structure has a
  matrix member, because the row-length quantifier in it was no known row. The new branch accepts that
  conjunct only when a row-length fact in it has a column count other than 1, and judges its content with
  the unedited rule for a structure's length facts. The agent reported it as provisional and gave the
  revert. **Answer taken (the advisor):** it stands provisionally on conditions: refusing twins that tie
  the guard (the guard on another root than the length facts; a guard that is not the canonical
  null-and-bounds test; the conjunct inside a trusted method), written before the commit, and the third
  audit round reads it first; without the twins the branch is reverted and the one self-test case that
  holds the shape goes. **For the owner:** let it stand, or have it reverted (then that contract shape,
  an atom on a scalar member beside a contract read of the matrix member, is an accepted program whose
  Viper the lint refuses, listed). **What was done since (2026-11-27):** the twins are written and the
  branch was tightened once by them (a guard and facts that both named another root than the clause's own
  pointer were accepted, and are refused now). Measured: the lint before the branch, and the base's lint,
  refuse the Viper of every program with an atom on a scalar member of a structure that has a matrix
  member, programs the verifier accepts and the record accepts (item 13), so this branch is the lint's
  side of that rule. **The third condition was narrowed by the advisor** (its first wording contradicted
  the scalar-cell ruling): the pinned internal trusted methods (allocation, zero fill, copy) refuse the
  conjunct; a DECLARED function's contract may hold it only for the root that has the cell atom in the
  same clause (there the function's own first `ensures`, which the older rules admit, already states the
  same length facts for the result: checked in the fixture `real_mmp_cell_atom_result.vpr`, lines 84 and
  86, so the conjunct assumes nothing new). Still owed at the time of writing and checked by the third
  audit round: a twin with another N or M inside the declared method, and what the branch's test "a
  column count other than 1" does for a structure whose member array holds structures with an array
  member (it is not a matrix and also has such a count).
- **375** (the Viper lint; fail closed: without it the lint refuses the Viper of accepted programs). The
  kept-flag and kept-value rows of a loop over a local array of structures whose element has an array
  member (`v[i].a[j]`) were refused because the lint did not know the place. **Answer taken (the
  advisor):** the place may be made known, on the terms of the earlier cases (a global matrix's place, a
  matrix member's place): through a rule tied to the pinned allocation's own counts, member index, leaf
  length and field; the kept-flag, amount and guard rule functions stay textually identical (the agent
  shows the empty diff); refusing twins on real text; every kept-row line that stops being refused is
  listed; if the rows do not pass once the place is known, the agent stops. Provisional until the third
  audit round. The alternative: leave the rows refused, and then no loop over such an array passes the
  full check's lint step.
- **376** (fail closed: a false alarm). The contract pair `requires reads_range(p, n) && p[0].a[1] != 0`,
  `ensures … 10 / p[0].a[1]` over structures with an array member reported `division-by-zero` on the
  build that first accepted it: nothing tied the two states of `p[e].a[e2]`. **Answer taken (the
  advisor):** build the tie, as question 367 was answered for a row parameter and as record 0064 item 7
  sorts ties by root: `p[e].a[e2]` and `g[e].a[e2]` are tied (only under a `reads` site; every index and
  the member path in the key; no assumed fact of a new shape, else stopped and listed); a member array
  reached through a pointer (`p->v[e].a[e2]`) has no tie, question 345's listed false alarm. The
  alternative: list all of them as 345's false alarm and write the contract with the guard in each clause.
- **377** (fail closed: a rejection). A loop invariant that reads `p[e].a[e2]` under a range over
  structures with an array member is not built; the encoder rejects it at the invariant's line. The
  plan's test list for Part E names no such read, and this session's method (the advisor at the start:
  a shape outside the list gets a test pair or stays refused with a pin) covers it: **taken as a listed
  rejection with a pin row, M4.5e2**, in record 0064 item 10 and on the page. A local `v[e].a[e2]` in an
  invariant is accepted. **For the owner's review**, because the plan lets a cut outside its list be
  taken only by the owner, and one may read this as such a cut; the alternative is to build it in the
  next session before Part F.
- **378** (fail closed either way: a question of the category). `v[0].a[1] = v[0].a[1]++` on a local
  array of structures with an array member is reported by the lowering as a verification error
  (`unsequenced`), where its three siblings (`a[1] = a[1]++`, `v[0].x = v[0].x++`, `s.a[1] = s.a[1]++`)
  are rejected like the compilers (`unsequenced_modification`), which is what record 0064 item 11 says
  for the same leaf of a named array. **Answer taken (the advisor):** follow item 11: the Sequencer's key
  gains the element's index, the member path and the member's index; if that is not in the slice-3
  joined build, the difference is stated in item 11, pinned by a test that registers `unsequenced`, and
  listed here. **Not built at this snapshot: the lowering's sub-step for it is pending**, and the form is reported as `unsequenced` until it lands.
- **379 and 380** (the Viper lint; fail closed in the verifier: its verdicts do not pass through the
  lint; two gaps in rules older than this session, found by a test-mutator's twins and NOT edited, on
  question 357's boundary). **379:** the two-binder rows of a loop invariant, and of a loop body's marked
  `assert` rows, over a parameter's range are not tied to the range's length facts (a row over `j < 4`
  for a row of three cells is accepted by the lint; Silicon itself refuses such an invariant at entry).
  **380:** the guarded well-formedness conjunct `len(member(p, k)) == N` in a `requires` or `ensures` is
  not tied to the structure allocation's own N (changing 3 to 4 is accepted by the lint, also in text of
  Part D; a function emitted so would be verified under a wrong length and no caller could call it).
  **Answer taken (on question 373's ruling, the same class):** listed with 369 and 373 for the lint step
  at the start of the next session, which the owner has agreed to; nothing is tightened in this session.
  The alternative: tighten both now, which adds strictness to kept-flag-shaped rows without the whole
  regeneration having been checked against it.
- **381** (fail closed: a rejection). `cerastes_old(g[1].a[2])`, the entry value of a cell of an array
  member of an element of a GLOBAL array of structures, is rejected by the encoder at the `ensures`. The
  rule that rejects it was written in this session for a global structure's matrix member (question
  346's limit) and reaches this form too; the lowering would accept it and has cases for it that verify
  (reachable from the self-test only). Measured beside it, on the base and on this build: `old` of
  `g[e].x`, of a global matrix's cell, of a row parameter's cell, of `p[e].a[e2]` in a range and of
  `p->m[e][e2]` under the structure's atom are all accepted, each with a wrong twin failing. **Answer
  taken (the advisor):** keep it rejected and pinned (`negative/array_rows_struct_elem_reject_old.c`), the
  message reworded, a sentence in record 0064 item 7; nothing is opened that no audit round has read end
  to end. **For the owner's review:** the alternative is to open it (narrow the encoder's rule to the
  global structure's forms; the lowering is there), in the next session with an audit read.
- **382 to 384, three rejections taken on audit round E3a's findings** (each fail closed; for the owner's
  review, since each leaves unbuilt a shape that verified and was right in the auditor's probes).
  **382:** an element structure that holds, by value, a structure with an array member (`struct N {
  struct P p; int k; } v[2]`, `v[i].p.a[j]`) was accepted by the encoder and stopped at the checker's
  refusal; neither record 0064 nor the plan's test list names it. **Answer taken (the advisor):** the
  encoder rejects it in every storage and computed-type position, with a pin (record item 2); M4.5e2.
  **383:** a marker (`ref_to_uninit`, `must_init`) over a range of structures whose element has an array
  member verified, and emits a kept-flag row over two bound variables that the Viper lint refuses; a
  lint rule for it would be a kept-flag rule (question 357's boundary). **Answer taken:** the plan's cut
  (1), "a marker over a range of structures", for this element type only: rejected by the encoder with
  a pin; markers over a range of structures without an array member and Part D's markers on a pointer
  to one such structure stay accepted. **384:** an atom under a guard (`c == 0 || owns(*p)`, an arm of `?:`)
  on one of Part E's objects verified, and emits a guarded well-formedness row that only a guard-shaped
  lint rule could admit. **Answer taken:** rejected by the encoder with pins, for exactly the forms
  measured to emit that row: an atom on a structure with a matrix member or with a member array of
  structures that have an array member, and the whole-array atom of a global matrix or of a global
  array of structures with an array member; the unguarded atoms stay accepted, as do guarded atoms on
  Part D's structures and guarded ranges. The alternative for 383 and 384: accept them and write the two lint
  rules, with the owner's word on question 357's boundary.
- **385** (the Viper lint, question 374's branch; nothing unsound was accepted by the verifier). Audit
  round E3a judged the branch "stands with a change": with a second structure type in the file whose
  matrix member has other counts, the branch admitted a cell clause stating a row length that
  contradicts the function's own well-formedness row, also in the trusted `ensures` of a declared
  function, where a caller would assume both; and a clause holding only `p != null` and the conjunct,
  with no cell atom, was admitted. **Answer taken (the auditor's change, as a tightening):** the conjunct
  is accepted in a cell clause only when it is textually the guarded conjunct of the same method's
  well-formedness row for that pointer, in the same kind of clause; the auditor's mutants become
  refusing twins. Question 374 for the owner stands as written, with this change made.
- **386** (the Viper lint, the tightening of question 385; nothing loosened in the verifier). A formal
  parameter has a well-formedness row in the method's `requires` and none in its `ensures`, so the rule
  "the conjunct equals the same method's well-formedness row, in the same kind of clause" would refuse
  the Viper of an accepted program whose `ensures` holds a cell atom. **Answer taken (the advisor):** for
  a formal only, an `ensures` cell clause is tied to the `requires` row (the length facts depend on the
  pointer, not on the heap, and a formal does not change); refusing twins for a conjunct that differs
  from that row and for a formal with no such row. On the last audit round's list.
- **387** (test mechanics; a cost to know). `negative/matrix_loop_global_main.c` (four global matrices,
  four loops and four checks in `main`) took 147 s in C and 90 s in C++ as single runs and 269 s in the
  harness, past the plan's 120 s. Measured: no single loop carries the time and 2 by 2 matrices are no
  faster; with the four loops in four functions it takes 11 s: the cost is the number of loops over
  global matrices in ONE function. **Answer taken (the advisor):** restructure, keeping `main` (the test
  is about loops in `main`): two loops per `main` if that is under the line, else one per file, each new
  `main` with a control that verifies without the loop; the cost is listed in record 0064 item 10. Not
  moved to `tests/long`: the slowness is not what the test must refute.
- **388** (test mechanics, invariant 1's edge). A new test, `negative/matrix_typedef_wrong.c`, was
  corrected once after its first run (two checks in one `main`: Silicon stops a path at the first), and
  the correction was itself defective: its checks then failed because the value was unknown, not because
  it was wrong. **Answer taken (the advisor):** a second edit is allowed (the values stated in the two
  `requires`; no `expect:` line moves; the file had no harness run in between); listed. With it a rule
  for this round's twins: every must-fail `check` twin written or corrected is shown to verify with the
  right value.
- **389** (the Viper lint; two edits a test-mutator made and flagged instead of settling; both fail
  closed to revert; nothing in the verifier). (1) In question 374's branch one pattern was widened: the
  row-length fact it looks for may stand at any member index, not only at member 0, so that the branch
  also covers a structure whose member array holds structures with an array member (two lines of one
  self-test case); the tie that carries the branch since question 385 (the conjunct is textually the
  same method's own well-formedness row) is unchanged. (2) The rule for the `program_start` group of a
  global array of structures with an array member does not ask for a range quantifier for a trailing
  `bool` leaf whose values are unknown; that a `bool` cell has no range fact was written in
  `internal/encoding.md` and in the lint's scalar rule, and is now in record 0064 item 9. **Answer
  taken:** both stand provisionally and are on the last audit round's list. Also listed, not built: no
  lint rule ties a structure local's scope exit to the rows its allocation gave (a dropped `exhale` is
  not refused, for scalar leaves too: older than this session; for the lint step with 369 and 373).
- **390** (the Viper lint; fail closed in the verifier). The last audit round (E3b) found three more
  accepted shapes whose Viper the lint refuses for want of an ordinary rule (an element copy whose
  source is an element of a global array; a one-cell array leaf in the element of a member array; an
  element whose only member is an array, in a global structure's member array, copied in `main`), and
  showed that the lint's table collision is one declaration away from every unit that uses the member
  form. **Answer taken (the advisor):** listed, with questions 369, 373, 379 and 380, for the lint step
  that opens the next session; no rule and no rejection was added, because either would have entered
  the commit after the last audit read, and no test holds the shapes (record 0064 item 17 has the
  list). The alternative: write the three rules, or reject the three shapes, before the commit, with
  another audit read.
- **391** (the Viper lint; nothing in the verifier; on question 357's boundary). The test-mutator's lint
  change for structure values (11 lines, "Rule A") loosens a rule at question 374's site: the guarded
  well-formedness row of a pointer to a structure that has a matrix and a direct scalar array member may
  state that array's length (the Viper of `*p = *q` needs it, and so does a plain `owns(*p)` on such a
  structure). The agent flagged it instead of settling it. **Answer taken (the short audit read: "stands
  with a change"; the advisor: it stays as built in this session):** the change the auditor asks for,
  the lint's table keyed by the structure's type instead of by member index over the whole unit, is the
  lint step that opens the next session; until then a length that is not the type's own passes where
  another declared structure has a scalar array at that member index, an understated length of 1
  passes, and a second declared structure with another count at that index makes the right row refused
  (record 0064 item 17). **For the owner**, with question 374: this is the third place in the session
  where a guard-shaped or kept-flag-shaped lint rule was widened (374, 375, 391).
- **392** (fail closed: internal errors, exit 3; how each was handled). The cases written for structure
  values found two internal errors in the tree of this session's first commit, and the audit read of the
  second commit's tree found a third, all in one family (a row of a matrix asked for something only a
  structure or a cell has): a structure whose first member is a matrix beside an owned or read second
  pointer; a call of a function with two `reads` atoms of a structure with a matrix member; a structure
  with a matrix member in a function that needs a pointer split. **Answer taken (the plan's stop rule: an
  exit 3 is settled within its part; the first two decided by the orchestrator and confirmed by the
  advisor afterwards, the third with the advisor):** the first two were fixed at once, with cases,
  mutation checks, C-level pairs and an audit read, in the second commit; the third was fixed in a third
  commit of its own (16 lines of the lowering, 16 cases, a C-level pair, its own short audit read), inside a
  time box of about forty minutes set with the advisor: had it not been proved in that time, the check
  would have run on the second commit and this error would have been the owner's first question. With the
  fix those programs get their ordinary verdicts. **The first commit alone has all three and the second
  alone has the third** (each measured on a build of that commit). *Alternative not taken:* list the third
  for the next session. *What the lowering owner's table of the family rests on:* reading, not proof (every
  caller of each function that raises one of the two internal-error texts; a group of 14 sites for atoms
  written in loops rests on the encoder's rejections of question 348): `build/m4.5e1e2d/il-analyst-report-10.md`.

- **393** (fail closed, but without a diagnostic: the tool is killed by a signal, exit status 139; nothing
  is reported verified; older than this session). A structure of about 4,095 scalar members passed by
  value ends `cerastes verify` in a segmentation fault. The last audit read found it beside the fix's new
  stop (a value of 4,096 cells or more now ends in an internal error, exit 3, with a message whose words
  "is cut there" are wrong for a structure without an array). Measured: on the build of this session's
  base (`42880a1`) 4,095 and 4,096 cells both end in exit 139, in both modes; on the audited build 4,095
  cells end so and 4,096 in the exit 3; 3,000 cells do not crash in 25 s; with an unlimited stack the
  4,095-cell program does not crash (the auditor's run), so it is stack exhaustion; where the recursion is
  was not looked for. **Answer taken (advisor): not fixed in this session** (a change under `src/` after
  the last audit read costs a new snapshot, join, proof and read, and no verdict is wrong); listed here
  and in record 0064 item 12. *Recommended:* the encoder rejects a by-value structure above a stated
  number of cells with a diagnostic at its C line (invariant 3: rejected, never a crash), chosen where
  the tool still answers (the audit saw no verdict in 240 s for 100 cells on a busy box), in M4.8 with
  the other costs; *alternative:* the next session, with the stop's message.
- **394** (fail closed: a false alarm of the base, on any structure; not from this session's code). A
  function that owns `*p` (a structure) and `*y` (an `int`), calls a declared `int *pick(int *a)` whose
  contract keeps `owns(*a)` and returns `r != NULL && owns(*r)`, writes `*r` and returns under
  `ensures(owns(*p))` is reported as "returns without owning `*p`", although the C is fine; the twins
  whose callee takes nothing verify. The cause is the auditor's reading of the kept Viper, not a trace:
  the call lends half of ALL the function holds on the liveness cell of `y`'s block to a callee that
  keeps the atom, and where `*p` and `*y` may share a block what is left can be under `*p`'s own amount.
  For a structure with a matrix member this shape was exit 3 before the third fix and has the base's row
  now. **Answer taken (advisor): listed.** *Recommended:* M4.5e1f (range call rows), which rewrites the
  amounts lent at a call, decides whether the amount lent for an atom the callee keeps is bounded by
  that atom's own amount; *alternative:* M4.8 with the cost of aliasing `reads`/`live` parameters
  (question 66).
- **Not taken from the last audit read (S5), each with its reason** (none is a wrong verdict; each
  changes `src/` or `tests/` after the read, which would cost a new snapshot, join, proof and, for `src/`,
  another read): (1) the text pin of the self-test cases `e1e2e_split_*_local_pick_*`, the only guard of
  the split's candidate for a matrix member, cannot tell the matrix's first cell from the first row's
  slot (by the auditor's reading, a lowering that dropped the row step would pass it): the pin should
  name each form with its own member index: **first item of the three-dimensions step, which reopens
  `split_candidates`**; (2) no C-level failing twin where the unknown pointer is a matrix cell (invariant
  2 is met by the lowering's case; the auditor's twin text is in its report, finding 5): with (1); (3)
  the 4,096 stop's message (393); (4) the prose in the header of
  `tests/long/memory/negative/matrix_value_copy_ptr_two_wrong.c` still says the file moved with
  `// timeout: 1800`, while its header line is 3000 since the third commit; (5) measured again: in C the
  address of a matrix cell of a GLOBAL structure as a call argument inside a declaration's initializer
  (`int *q = pick2(&g.m[1][0], &x, c);`) is rejected (`unsupported_type`), in C++ accepted: the listed
  initializer form of record 0064 item 13, one step wider than the record's example.

The next questions start at 395.

### Questions for the owner (after the second session of M4.5e1e2's continuation, 2026-11-26)

**Nothing here blocks the next session; it can start as it is.** Each item is explained above under its number.

1. **Pushing** (§2, step 1): `origin/main` (`f07d80a`) still accepts every unsound `restrict` form; local `main`
   has the merge the owner made and one commit of this session (the owner's handoff answers). **Recommended:**
   push `main`, then `git merge --ff-only m4.5e1e2c-arrays-of-structures` and push again.
2. **354, the limit on `calloc`** (fail closed: a rejection with a diagnostic). A structure with an array
   member can be allocated with `calloc` only if it holds at most 24 values in all (each array element and
   each other member counted); above that the call is rejected and the diagnostic says to use `malloc`.
   The reason is a tool limit, not a rule of C: the contract of `calloc` states "every element is zero" one
   clause per element, and beyond about 50 elements Silicon's own stack overflows while it checks that
   contract (no verdict; 24 is about half of what passed on the box, since the limit may differ by
   platform). The cost to a user: a `malloc`ed structure's elements written one by one can be read back,
   but a read of an element that only a loop wrote is reported as `uninitialized` until Part F brings
   written quantified invariants. **Recommended:** let the cap stand; M4.5e2 replaces the per-element
   clauses by one quantified line per array member (it needs that line for `calloc(n, sizeof(T))` anyway),
   which removes the cap. **Alternative:** have the next session rework how the contract's well-formedness
   method frames the per-element reads, and raise or drop the cap then.
3. **357, a permission was denied** (fail closed: nothing was loosened). A test-mutator tried to change
   `scripts/lint_viper.py` so that it would admit the Viper of two new tests (an amount written as nested
   conditionals over a method's `Perm` formals, on rows of an array part reached through a pointer read
   from memory). The permission system denied the edit as undermining oversight; the agent stopped and the
   edit was not made by anyone. The two tests, which pass on the verifier, are kept unchanged under
   `internal/pending/m4.5e1e2c/` with the proposed rule, its author's own soundness note and what audit
   round D2 found when it read the code behind the shape (no caller gains permission in the probes that
   returned a verdict). A reduced form of the pair is in the suite. **Recommended:** decide at the next
   planning whether the lint should admit that amount; if yes, a session writes the rule with refusing
   twins, has it audited, and restores the two tests. **Alternative:** leave the shape listed as one the
   lint refuses (then no suite test may hold it).
4. **348, structures whose array member holds structures** (fail closed: a rejection). Everything Part D
   built is for array members of scalars. A structure such as `struct path { struct point v[2]; int n; }`
   stays rejected on the heap, beside an `owning` or marked member, under markers, behind a heap read, with
   a partnerless atom and under a written atom in a loop invariant, because those sites would need Viper
   shapes no spike has verified, one of them a trusted contract. **Recommended:** M4.5e2 lifts it with
   `malloc(n * sizeof(struct S))`, whose rows are the same. **Alternative:** a part of its own in the next
   session, after Part E, with a spike first.
5. **349–353, 355, 356, the advisor's other rulings in this session** (above). **Recommended:** let them
   stand. The ones that depart from the plan as written: 350 (two rows the plan listed as flips were kept
   rejected, one of them for soundness), and 355's second half: of the atoms a loop invariant may write on
   a structure with an array member only the `owns` forms on a pointer variable were built; written `reads`
   forms and atoms through a pointer read from memory stay rejected (fail closed; M4.5e2), a cut the
   orchestrator took in a briefing and the advisor accepted only afterwards. 352 and 353 are rulings for Part E, taken from the encoder draft.
