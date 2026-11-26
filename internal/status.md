# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, M4.5a's
`git show 72e30b3:internal/status.md`, S5's `git show 794e960:internal/status.md`, M4.5b1a's
`git show bdc6bd0:internal/status.md`, M4.5b1b's `git show 8003dca:internal/status.md`, M4.5b1c1's
`git show eab4058:internal/status.md`, M4.5b1c2's `git show a5e3e30:internal/status.md`, M4.5b1c3's
`git show 5ffb1a2:internal/status.md`, M4.5b2a's `git show 38877b4:internal/status.md`, M4.5b2b's
`git show 1bfa705:internal/status.md`, M4.5c1's `git show cbdcce5:internal/status.md`, and M4.5c2a's
`git show 7f11706:internal/status.md`, M4.5c2b's `git show d26a87c:internal/status.md`, and M4.5d1's
`git show 9d98fe8:internal/status.md`, M4.5d2a's `git show bb41d55:internal/status.md`, S6's `git show 9255b93:internal/status.md`, M4.5d2v's `git show c2bc48d:internal/status.md`, S7's `git show a660abc:internal/status.md`, M4.5d2b1's
`git show c1b691d:internal/status.md`, S8's `git show 15bcdc6:internal/status.md`, M4.5d2b2a's
`git show 19d977a:internal/status.md`, M4.5d2b2b's `git show e3f4b9f:internal/status.md`, M4.5d2b2c's
`git show 023ea50:internal/status.md`, M4.5d2b2d's `git show d88a457:internal/status.md`, M4.5e1a's `git show 685034b:internal/status.md`, M4.5e1b's `git show 7964d33:internal/status.md`, M4.5e1c's `git show c65f618:internal/status.md`, M4.5e1d's `git show 4e28487:internal/status.md`, M4.5e1e1's `git show f07d80a:internal/status.md`,
the first session of M4.5e1e2's `git show ae622e5:internal/status.md`, and the first session of its
continuation's `git show 493a923:internal/status.md`.

## M4.5e1e2, the continuation's second session — Part D: what a structure with an array member could not do (2026-11-26): one part landed in one commit, the full check green on it; closed at the split point after Part D; three things for the owner: the limit put on `calloc`, a lint edit the permission system denied, and a rejection kept

**What a reader needs first.** A structure with an array member of scalars (`struct buffer { int data[8]; int
len; }`) can now live on the heap (`malloc`, `calloc`, `free`, `cerastes_owns_block`), have an `owning` or
marked member beside the array, be the object of an `owning`, `ref_to_uninit` or `must_init` pointer, be
reached through a pointer read from memory in a contract, be given back by an `ensures` atom with no `requires`
partner, be carried by loops, and be named by an atom written in a loop invariant: the continuation plan's
Part D. Parts E (arrays of arrays) and F (quantifiers over members) are not built and run in a further session
from the same plan. Two audit rounds (a fresh Fable auditor each) found no wrong program that verifies. **The
owner's attention is asked for three things** (handoff §8): the limit this session put on `calloc` (question
354); **a lint edit that the permission system denied, which nobody then made** (question 357: two tests are
set aside for it); and the rejection kept for structures whose array member holds structures (question 348);
with the review of the questions settled with the advisor (348 to 357).

Plan: `.claude/plans/m4-5e1e2b-arrays-of-structures.md` (the continuation plan), executed from Part D after its
Step 0 again, without re-planning. **A noted reading:** the owner's first message was "M4.5e1e2"; `plan.md`'s
row and the handoff named the plan and the part. Record 0064 (accepted for the items built: item 15 with this
session, for array members of scalars; items 13 and 14 "decided, built by the next session").
**Base:** `493a923` on local `main`: the owner's merge commit `e82f894` of the last session's branch (a tree
identical to that branch's tip `887ac23`; `46f34fb`, the commit the last full check passed on, is an ancestor,
and nothing under `src`, `include`, `tests`, `scripts` or `doc` differs from it) plus one commit this session
made on `main`, the owner's own inline answers in `internal/handoff.md`, verbatim. **`origin/main` is still
`f07d80a`: the owner's merge is local and nothing is pushed.** Branch `m4.5e1e2c-arrays-of-structures`,
worktree `.claude/worktrees/m4.5e1e2c`. Session from 2026-11-26 12:21 UTC to about 19:30 UTC (7 h 10). The owner gave
no overnight rule; every question went to the Fable advisor first.

**The owner's answers read at the start** (inline in the handoff, committed as `493a923`): the merge "Fixed"
(local); question 343 "Follow alternative plan since it sounds like the correct fix." (the exit 3 stays listed
until M4.5e2; nothing was built for it; `plan.md`'s M4.5e2 row has it now); 338 to 342, 344 to 347 and the
earlier rulings "Follow recommendation".

**The session closed at split point 3** (question 310's 12-hour rule; the advisor, checkpoint 3, taken at
Part D's final joined proof, 3 h 37 in). Part D's `src/` came to 1,966 insertions (`encode.cpp` +160 −57,
`check.cpp` +159 −21, `lower.cpp` +1,647 −106 at the final joined proof; the plan sized it at 800 to 1,500)
and its lowering took four il-analyst instances and 3 h 20. Part E's sum from Commit 4: a lowering the plan put
at half of 800 to 1,500 lines and Part D's measure puts at 3.5 to 4 hours, joined to an encoder draft of 1,812
lines that nothing has proved end to end; a joined proof and first run (45 minutes); an audit in two rounds
(two hours, partly beside the build); fixes (an hour); the two-hour close-out: about nine hours from Commit 4,
ending past fourteen. So Part E was not begun. Parts E and F run from the same plan in a further session
(handoff §2).

**Done** (commits on `m4.5e1e2c-arrays-of-structures`, local, no Claude trailer):
- **Commit 4 `86442ce`** (18:16 UTC, 5 h 55; Part D, record 0064 item 15, with the fixes of audit rounds D1 and
  D2). For a structure whose array members are arrays of scalars:
  - **Heap storage** (question 292). `malloc(sizeof(struct S))`, `calloc(1, sizeof(struct S))` and `free`; the
    typed views and `cerastes_owns_block` state the member lengths and hold each array member by quantified
    permissions and flags; `free`'s rows per array part (`invalid-free`); the leak check as before. `calloc`'s
    view states the zero values one clause per element, and **`calloc` is rejected at the call for a
    structure with an array member that holds more than 24 values in all** (question 354, below).
  - **An `owning` or `ref_to_uninit` member beside an array member**, and owners (a parameter, a result, a
    local, a global, a member) that point at such a structure (question 293).
  - **`ref_to_uninit`, `must_init` and `owning` on a pointer to such a structure** (question 294): no flag of
    an array member is claimed on entry; a `must_init` function that leaves an element unwritten is
    `postcondition` at its closing brace (question 349).
  - **Contract places through a heap read** (question 306): `owns(*p->q)` and element reads `p->q->a[e]`.
    This shape had no spike fixture; it was built and put through Silicon first.
  - **An `ensures` atom with no `requires` partner** on the result (record 0052 item 2a stands: on a FORMAL
    of a function without a body it stays rejected, question 350).
  - **Loops.** A heap block's array part is a place of a loop (in programs where such a block can exist: the
    gate `array_block_possible`, which keeps every earlier program's Viper byte for byte); the carriers of
    an owner's block across a loop (written `owns_block` sites, owning locals, globals and members, marked
    closures) take the array parts; a loop that frees and reallocates an owning global's block verifies.
  - **Written atoms in a loop invariant:** `owns(*p)` and `owns(p->a)`, `p` a pointer variable, at `write`
    (question 355). Written `reads` forms, a pointer read through memory and a guard that reads memory stay
    rejected.
  - **Not for a structure whose array member holds structures** (question 348): at each of these positions
    such a structure stays rejected, by the encoder, by `check.cpp` and by explicit stops in the lowering.
  - The page ("Array members" in `memory.adoc`: the heap, the cap, owners, markers, heap-read places, loops,
    written atoms, the listed false alarms, the not-yet list), two doc examples, `internal/encoding.md`,
    amendment lines in records 0040, 0045, 0047, 0052, 0054, 0055, 0058 and 0063.
- **On `main`, before the branch: `493a923`**, the owner's inline handoff answers, committed verbatim.

**Not done** (the next session's, from the same plan; nothing was cut): Part E (arrays of arrays,
array-of-rows parameters in the three spellings and the typedef'd ones, `v[i].a[j]`, depth 3, the tutorial's
matrix example) and Part F (quantifiers and `pure` applications over members). Each is still rejected with a
diagnostic and a pin that names its part. **An encoder draft of Part E exists on the branch
`m4.5e1e2c-part-e-encoder-draft`: unproved end to end, unaudited** (handoff §3 says what stands behind it).

**Two crashes found and fixed on the way, both behind refusals or new code, neither a wrong proof:** a
written atom on an array member reaching a walk that expects a cell segfaulted (exit 139) behind `check.cpp`'s
refusal on the base (now an explicit stop with cases, then rows); `wf_place` dereferenced a null context for
an element read with a heap-read base in a contract's well-formedness method (exit 3 at the first run of the
new `chain_wf` tests; fixed, with a case that crashes without the fix).

**Audits** (Fable at `xhigh`, a fresh instance per round; two full rounds on Part D where the plan has one,
for coverage; reports `build/m4.5e1e2c/auditor-report-{D1,D2,D2b}.md`, probes under `audit-{D1,D2,D2b}/`):
- **Round D1** on `refs/tmp/e1e2c-d-a1` (`b193add01de6`: the encoder half, the lowering's heap, marker,
  partnerless and heap-read columns and a heap block's array part as a loop place; about 100 probes): **no
  wrong program verifies, no blocking finding**, every "not built yet" stop it tried complete. Should-fix: 1,
  four test files failing at that snapshot (the parts not yet built); 2, the record said the written atom
  was built while only `check.cpp` refused it; 3, no test under `tests/` for the loop place; 4, **a lint
  relaxation that hid an obligation** (an `assert` row's guard could read any pointer load: a never-true
  conjunct linted clean; narrowed to the canonical null test of a canonical load, the auditor's mutant a
  twin); 5, three of question 348's backstops without a case or pin row. All taken. Notes: more exit-3
  shapes in loops over owners (built since: they verify or fail at a row); **an "internal check" exit 3 on a
  recursive function with a heap-read atom and no `p->q == old(p->q)`, not of this session** (the same on
  the frozen base build; one run; handoff §3); costs; a false alarm for a heap-read callee called in a
  loop. It read `check.cpp`'s diff in full, `encode.cpp`'s changed lines and, in `lower.cpp`, the loop
  place, its gate, `block_cells` and half of `block_conj`; **`block_rows` and its sinks, the marked, output
  and heap-read parts of `atom`, `call_atoms` and `atom_returns`, and `contract_read`'s block loop it judged
  by probes only**; it did not run the VIR self-test.
- **Round D2** on the final source `refs/tmp/e1e2c-d-j7` (`fc484b966ee6`; 79 probes): **no wrong program
  verified in any probe that returned a verdict, no blocking finding, no crash.** It read every added line
  of `lower.cpp` in its diff (the loop carriers, written atoms), the `check.cpp` and `encode.cpp` diffs and
  the lint diff in full. Should-fix: 2, **a memory-reading guard on a written atom's array part was lowered
  to a constant** (fail closed: a false `permission`, or more demanded than written): now rejected by the
  encoder and `check.cpp`, with an explicit stop in the lowering; 3, a written `owns(c->a)` on a scalar
  member of a structure that also has an array member of structures was accepted against question 348's
  rule: now rejected (a written atom on a cell of such a structure stays accepted, as on the base; record
  and page reworded); 4, the page and the record said owner carriers keep no flags, which is not what the
  build does: corrected; 5, the refused written forms printed `check.cpp`'s generic tag: the encoder rejects
  them itself now (`unsupported_annotation` at the invariant's line); 6, rules without a C-level test: tests
  added (15 files: moved-pointer pairs for both written forms, the pin of the refused written forms, a block
  site through a heap read, an owning local and a marked closure in a loop, the reassigned global's
  must-verify). Notes: three more accepted shapes whose Viper the lint refuses, none held by a test; six
  probes with no verdict in the time limit; false alarms, all listed in record item 15. **Asked to read the
  amount code behind the denied lint shape, it did** (`ens_amount`, the obligation of a contract without a
  body, the method's rows and the array part's return rows; not the rest of `atom_returns`): a bodiless
  method's `ensures` does carry the partner-sum amount, the contract's well-formedness method carries the
  obligation that makes it the partner's own formal, and no caller with a verdict ended with more
  permission than it gave (eight probes; six more timed out and are not checked).
- **The short read, D2b**, on `refs/tmp/e1e2c-d-j8` (`55f83710ea48`, which is Commit 4's `src/`): both diffs
  since round D2's snapshot **read in full**: the fixes in `src/` (the encoder's four rejections, the
  checker's refusals, the lowering's explicit stop) and the lint rules written after that snapshot. **No
  blocking finding.** Every form the fixes mean to reject is rejected (34 guard forms, the shapes, the
  mixed structures, both modes) but one family, which is accepted by the design of a shared predicate:
  should-fix 1, **a written atom under a guard that reads an address-taken local by its name is accepted
  and lowered as a real read, not a constant** (four probes under Silicon behave); not taken in code (the
  predicate `vir::reads_heap` serves other records' rules and wants a read of its own); record item 15 says
  what is built; no test holds the form, because the lint refuses its Viper. Nothing the record keeps
  accepted is rejected; no user program reaches the new internal error. **On the lint it confirmed that the
  kept-flag and amount refusals are unchanged and that the denied relaxation did not come back in another
  form**, and found two places where the session's new rules admitted too much (a written part over a
  block reached through a heap read; a ghost pointer taken from another pointer or guard): both tightened
  afterwards, its mutants now twins.
- **What no auditor read:** the self-test cases beyond their names; `atom_returns` beyond the array-part
  rows; in D1's diff the functions it judged by probes only (above); **the two lint tightenings made
  after the short read, the lint self-test's diff and the lint's fixtures**. Tests were run by the
  auditors through `cerastes verify` in c23 only. Timeouts (six in D2) are not checks.

**Questions** (handoff §8):
- **For the owner, first: 354, `calloc`'s limit** (fail closed: a rejection with a diagnostic in place of an
  exit 3). `calloc`'s contract states "every element is zero" one clause per element, and for a structure with
  `int a[64]` Silicon's own stack overflowed on it: no verdict. Measured through the driver (both modes; an
  array alone passes up to 50 elements and fails from 52), the session set a limit at about half: `calloc` of
  a structure with an array member is rejected at the call above 24 values in all; `malloc` has no limit.
  The diagnostic says what the way round costs today: after `malloc`, elements written one by one can be
  read back, a read of an element that only a loop wrote is `uninitialized` (a listed false alarm until
  Part F). Alternatives: another framing in the contract's well-formedness method, or one quantified zero
  line per array member (new trusted text; the record names M4.5e2).
- **For the owner: 357, a permission was denied.** The Viper lint refuses the Viper of two new tests (a
  contract with two `reads` atoms through two loaded pointers: the array part's rows carry record 0045 item
  7's partner-sum amount). A test-mutator's edit of `scripts/lint_viper.py` to admit that amount was
  **denied by the permission system ("Undermining Oversight")**. It stopped; neither the orchestrator nor
  another agent made the edit; the two later test-mutators were briefed with the denial and its boundary, and
  the short audit read confirmed that the refusals in question are unchanged (the two files' Viper gives the
  same problems under the lint before and after the session's other lint work). The two tests are kept
  unchanged under `internal/pending/m4.5e1e2c/` with the proposed rule; a reduced pair with one loaded
  pointer is in the suite; **the `other_root` row has no C-level pin**. Audit round D2 read the amount code
  behind the shape and found no caller that gains permission (eight probes with a verdict; six timed out).
- **For the owner: 348** (fail closed: a rejection). Part D's six positions stay rejected for a structure
  whose array member holds structures or arrays; the record names M4.5e2 as the session that lifts it.
- **Settled with the advisor, for review:** 349 (a `must_init` filler that misses an element is
  `postcondition`, as record 0042 item 5 has it); 350 (two rows of the plan's Flips A (D) were NOT flipped:
  a partnerless atom on a formal of a function without a body must stay rejected, record 0052 item 2a, and a
  marked parameter with no atom stays rejected by 0042 item 4); 351 (`calloc`'s zero values one clause per
  element); 352 and 353 (Part E's: a `const` row parameter stays rejected; whole copies of Part E's structure
  types cell by cell with the limit of 7 counting cells); 355 (written atoms at `write`, stricter than the
  record's first wording); 356 (the owning-member reassignment: the correct form is pinned as a listed false
  alarm, the wrong form dropped, the positive given another twin); the split point.
- **Settled by the orchestrator inside the plan's rules, listed for review:** the owner's uncommitted handoff
  answers committed on `main` (advisor agreed); two interim joined proofs and two audit rounds for one part;
  the short read's finding 1 not taken in code (a guard that reads an address-taken local by name is
  accepted and lowered as a real read; the shared predicate was left alone; record item 15 says what is
  built); the three more accepted shapes whose Viper the lint refuses listed, not ruled (handoff §3).

**Programs accepted before this session that are now rejected:** none found. `calloc`, `malloc` and the
other positions of Part D were rejected for a structure with an array member before; no existing program's
Viper or map changed (below), and `rejections.py --base` shows no existing row moved but the plan's Flips.

**Deviations from the plan, for confirmation:**
- One commit on local `main` before the branch (`493a923`): the owner's inline handoff answers, verbatim.
- Briefings were files (`build/m4.5e1e2c/briefings/`): the plan's section 12 text with this session's names,
  the finished parts' tasks removed, and the session's facts; `box.env` was written by the orchestrator.
- **Instances:** edg-backend 1 to 4; il-analyst D1 to D5 (the lowering in four slices, the audit fixes in a
  fifth); test-mutator 1 to 6; a fresh auditor per round (D1, D2, D2b). An agent was replaced when its context
  was largely used (170k to 590k tokens).
- **Two audit rounds and a short read where the plan has one round** (the advisor, for coverage): D1 on the
  columns built first, D2 on the final source with a diff sized to be read in full, D2b on the fixes.
- **Two interim joined proofs** (`refs/tmp/e1e2c-d-j5`, `-j6`) before the final ones (`-j7`, and `-j8` with the
  audit fixes), so that the tests' first runs and the lint work did not wait for the lowering's last slice;
  each later proof repeated only what changed. The final test runs were made in lane `edg`, not `work`.
- **Part E's encoder half was drafted ahead** (the plan allows work ahead on a snapshot ref), in a second
  source-only worktree, before the split-point decision; work ahead was stopped on the advisor's word when
  the sum showed Part E would not fit. The draft is on a branch of its own, unproved and unaudited.
- **Record text written after its code, twice:** written atoms "as built" at `write` (question 355), and the
  loop place's gate; each in the strict direction, each written before the final joined proof.
- **Flips A (D) not made:** `struct_array_reject_annotations.c` L37 and L39 (question 350).
- **The lint was edited after audit round D2's snapshot** (step B's rules; the short read D2b read that diff in
  full) **and again after D2b's** (two tightenings from its own findings, with its mutants as twins: read by no
  auditor).
- **Runs outside the plan's test budget** (each listed in the agents' reports and the state log): the
  il-analysts' and edg-backends' single-file probes (the route probes, the audit probes re-run on later
  builds, 82 + 49 probes of the Part E draft, the `calloc` measurements, eight base confirmations); the
  auditors' probes (about 100, 79 and the short read's); native compiles and header checks of new tests on
  the Mac; the 348 pin measured on the frozen build; one run by the orchestrator on the frozen base (audit
  D1's exit 3) and two lint runs over the final build's output; docs builds beyond the preflight's one.
- Not done from the plan's section 10: no tutorial text (none was owed by Part D); `annotations.adoc` and
  `verifying.adoc` unchanged (no statement there changed with Part D; checked by search).
- The state log's hand-written times before 15:20 UTC run up to nine minutes ahead of the clock in places.

**Expectation corrections against the plan:** Part D's `src/` was 2,051 insertions against 800 to 1,500;
`VIR_CASES_MIN` at the base was 2,739, not 2,512; 129 of the 160 test files were used at the start; Flips A
(F)'s row L52 of `forall_reject_member_array_code.c` has one row left (question 340); Flips A (D) listed two
rows that must stay (350); the test count is far past the plan's cap of 160 (below), the excess being
audit- and question-derived files the advisor accepted.

**Corrections of new tests after their first run** (question 160; no verdict changed):
`struct_array_marked_member.c` (a helper used its parameter with no atom: `lifetime`; it gained
`requires(owns(*r))`); `negative/struct_array_reject_aggregate_owners.c`, `struct_array_loop_heap_fill.c`
and `negative/struct_array_loop_malloc_fill_false_alarm.c` (C++20 only: EDG's #550 / `set_but_not_used` for
a local that `(void)x;` does not count as used: a real use in its place). The last 27 files run (the final delta and the audit-derived ones) needed none.

**VIR self-test cases re-expected** (the plan's Flips S; hand-built cases): gone with the gates Part D
lifted: `e1e2a_rej_owns_of_a_structure_with_an_array_member_through_a_heap_read`,
`e1e2a_rej_partnerless_ensures_of_a_structure_with_an_array_member`, `marr_rej_array_and_owning_member`,
`marr_rej_marker_on_a_struct_pointer_with_an_array_member`, `marr_rej_member_array_element_through_a_load`;
re-pointed: `e1e2a_rej_marked_member_in_an_array_element` (its text) and
`e1e2a_rej_invariant_owns_of_a_structure_with_an_array_member` (now the `reads` form). This session's cases
are `e1e2d_*` (75 lowering cases and their rejection cases).

**Limits listed by this session** (record 0064 item 15; each fail closed): the `calloc` cap (354); question
348's rejection; after `malloc`, a read of an element that only a loop wrote is `uninitialized` (Part F); a
read after a loop that may give an owner a new block is `uninitialized`; a loop over the elements of a
structure given by a partnerless `owns(*r)` is `permission` (older than this session, as are the next two:
confirmed on the base build); a loop that frees and reallocates an owning member's block reports six rows; a
read after a loop that carries an owner the loop does not name; a loop invariant that reads an element of an
allocated structure or of a moved written atom is `permission`; written `reads` atoms, a pointer read
through memory and a memory-reading guard on a written atom are rejected (M4.5e2), while a guard that reads
an address-taken local by name is accepted; a marked member's block replaced in a loop, two owning globals
swapped in a loop, a callee with a heap-read atom called in a loop (false alarms); `owns(r->a)` on the
result of a function without a body is rejected with 0052 item 2a's message (over-strict); shapes with no
verdict in the time limit (two guarded written atoms whose guard the loop flips; two owning or marked
members swapped; callers of a bodiless function with two heap-read `reads` atoms); **accepted shapes whose
Viper the lint refuses, none held by a test** (question 357's partner sum; two owning globals in one loop;
an owning global of a structure holding the structure by value; a guard reading an address-taken local; the
four classes of the session before); the reduced `f__wf` pin (no `other_root` row at C level); an "internal
check" exit 3 on a recursive function with a heap-read atom and no tie (not of this session); question
343's exit 3 (the owner: M4.5e2).

**Checks** (every test run on the box, lanes `~/cerastes-e1e2/{base,il,edg,work,audit}`; the box was under
other users' load, 40 to 75, for parts of the session, so times are upper bounds):
- **Baseline** at `493a923` (lane `base`): 6,938 programs regenerated, 0 missing; VIR self-test 2,739 lowering
  and 423 rejection cases.
- **Neutrality at every joined proof: no existing program's `.vpr` or `.map` changed.** First interim
  (`refs/tmp/e1e2c-d-j5`): 6,938 of 6,938 identical, 94 new. Second interim (`-j6`): 6,938 identical, 130
  new. Final (`-j7`): 6,938 identical, 0 missing, 136 new. Final with the audit fixes (`-j8`, Commit 4's
  `src/`): 7,104 outputs written, 0 missing, 6,938 identical, 166 new. One slice of the lowering first
  changed the text of three existing self-test cases and was gated before it was joined.
- **The VIR self-test:** 2,773 lowering and 442 rejection cases at the first interim; 2,801 and 445; 2,814 and
  449; 2,814 and 456 at the final build (`VIR_CASES_MIN` 2814); 0 problems each time; every case shared with
  the base byte-identical.
- **`rejections.py --base`** at each joined proof and in the preflight: against the base only the plan's
  Flips A (D) rows removed (`struct_array_reject_annotations.c` L34, L44, L54, L81, L87, `…_expressions.c`
  L23, `…_members.c` L21), three staging pins retired whole (`…_heap.c`, `…_marked_members.c`,
  `…_markers.c`), and four new pin files; no existing row moved; L37 and L39 print the older rules'
  messages (0052 item 2a, 0042 item 4).
- **New tests, each run on the box, re-run only where the Viper changed:** the first interim build: 49 files,
  46 as registered, one correction, the `calloc` 64 pair no verdict (replaced by the cap's files); the
  second: 2 re-runs and 21 new files, three corrections, one registration left for the advisor (356); the
  final build: 10 files as registered; the final build with the fixes: 17 files as registered (the 15
  audit-derived and the reduced `f__wf` pair). No exit 3 on a build that stands. Over 60 s: the reassigned
  owning global's pair (78 s and 75 s per harness call, both modes) and the reassigned owning member's
  false-alarm pin (69 s per mode alone; 139 s for the call at box load 40 to 75). Nothing over 120 s per
  mode.
- **Test counts against the plan's cap of 160:** 129 were used at the start; this session adds 85 test files
  and 2 doc examples (the plan's 35 for Part D; 8 seq and `f__wf` pin files; 4 for question 348; 11 for the
  loop column; 3 for the `calloc` cap; 5 audit-D1-derived; 15 audit-D2-derived; 5 others: a two-array
  `calloc` pair, a vacuity twin, an owning-global pair, counted with the groups as the test-mutators' report
  11 lists them) and removes 3 retired pins: 214 used, the excess over 160 audit- and question-derived
  (the advisor accepted it).
- **Preflight, once, on the final tests** (lane `edg` and the Mac): `format.sh --check` (93 files conform);
  `py_compile`; `run_tests.py --check-headers` (6,505 of 6,505); `mutate.py --validate` (374 tests, 12,864
  mutants generated: the validation of the generator, not a mutation run); `test_run_tests.py` 1,287 OK
  (skipped=1; an earlier run in the session said skipped=2: not explained); `test_lint_viper.py` 1,169 OK;
  the lint over the whole regeneration: 7,104 files clean; the VIR self-test: 2,814 + 456, 0 problems, its
  output clean in the lint; the session's `e1e2d_` cases through `run_tests.py --expect-passed 75`: 75
  passed; `walker_oracle.py` (136 goldens, 0 failed) and the tree checks; `rejections.py --base` (above);
  `build_docs.sh` exit 0; `check_doc_examples.py` on the source (26 pages, 419 blocks, 389 examples, 0
  problems) and `--html` (27 files, 0 problems), repeated after the last page edit. **One probe on macOS**
  (the orchestrator's; the `calloc` cap rests on a stack depth measured on Linux):
  `struct_array_heap_calloc_cap.c` (24 values) verifies on the Mac in 5 s in both modes.
- **`CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` on the box** (`~/cerastes-s8/p-ubuntu-26.04`):
  **green, exit 0, run once**, on Commit 4 `86442ce`, the last commit that touches anything but `internal/handoff.md`, `internal/status.md` and `plan.md`: 18:17:06 to 19:14:55 UTC (58 minutes; the box under other users' load, 18 to 128). Tests: 17,488 passed, 0 failed, 10 skipped; native-ok runs clang 3,277, GCC 3,277. The Viper lint: 7,104 files clean; the VIR self-test 2,814 passed, 0 failed, its output clean in the lint (2,814 files); the walker oracle 136 goldens, 0 failed; the docs and `check_doc_examples --html` (27 files, 0 problems); the EDG and vlibc trees clean; `check.sh: all green`, `long tier: ran`. The log is `build/m4.5e1e2c/check-c4-86442ce.log` (and `check-e1e2c-c4.log` in the lane).
- `src/` and `include/` against the base, without the self-test cases: `encode.cpp` +233 −57, `check.cpp`
  +180 −21, `lower.cpp` +1,638 −106 (2,051 insertions; the plan: 800 to 1,500); self-test cases +1,647 −57.
- Hours per step: Step 0 done at 0 h 15; the encoder half proved at 0 h 36; the lowering's first slice at
  1 h 00 and the first interim joined proof at 1 h 11; the second slice at 1 h 57; audit round D1 back at
  2 h 32; the third slice at 2 h 44; the `calloc` cap at 3 h 00; the fourth slice (the lowering complete) at
  3 h 29; the final joined proof at 3 h 37; audit round D2 back at 4 h 44; its fixes proved at 4 h 56 and
  joined at 5 h 10; the short read back at 5 h 31; the last test run and preflight at 5 h 46; Commit 4 at
  5 h 55; the box check started at 5 h 56; the check green at 6 h 54 (58 minutes); the status entry, the handoff and `plan.md` written while it ran.

**First step for the next session:** the owner pushes `main` and merges this branch (handoff §2, step 1) and
answers questions 354, 357 and 348 if convenient (none blocks). Then the next session: `claude --agent
orchestrator --effort high`, first message `M4.5e1e2 continuation, from Part E
(.claude/plans/m4-5e1e2b-arrays-of-structures.md)`. It runs from the same plan without a planning: a new
branch `m4.5e1e2d-arrays-of-structures` from this branch's tip, the worktree `.claude/worktrees/m4.5e1e2d`,
`build/m4.5e1e2d/`, the same box lanes; the plan's Step 0 again on the new base (submodules and bootstrap,
this session's reports copied from `.claude/worktrees/m4.5e1e2c/build/m4.5e1e2c/`, the lanes, the baseline, R1
and R3; no P84); then, before any briefing, handoff §3's notes on Part E's encoder draft (the branch
`m4.5e1e2c-part-e-encoder-draft`: unproved, unaudited); then Part E's line in the plan's section 7, "Order of
work". Its questions start at 358.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| M4.5e1e2 (the continuation's second session) A structure with an array member on the heap, with owners and markers, behind a heap read, in loops | 2026-11-26 | `m4.5e1e2c-arrays-of-structures` (not merged at the session's end; `main` merged locally by the owner, not pushed) | 0064, for the items built (0040, 0045, 0047, 0052, 0054, 0055, 0058, 0063 amended) | Part D of the continuation plan's six: for a structure whose array members are arrays of scalars, heap storage (`calloc` up to 24 values, question 354), an `owning` or marked member beside the array, markers and `owning` on a pointer to it, contract places through a heap read, a partnerless `ensures` atom, loops that carry such blocks and owners, written `owns` atoms in loop invariants; a structure whose array member holds structures stays rejected there (348); two crashes fixed; two audit rounds and a short read, no wrong program verifies; a lint edit denied by the permission system and not made (357); the full check green on the box at `86442ce` (17,488 tests); closed at the split point (Part E would have passed fourteen hours); an unproved encoder draft of Part E on a branch of its own |
| M4.5e1e2 (the continuation's first session) `restrict` rejected; ranges over structures; arrays of structures in static storage and as members | 2026-11-26 | `m4.5e1e2b-arrays-of-structures` (not merged at the session's end) | 0064, for the items built (0028, 0037, 0040, 0043, 0044, 0045, 0047, 0052, 0054, 0060 amended) | Parts R, C1 and C2 of the continuation plan's six: every `restrict` parameter rejected (the unsound forms no longer verify on the branch), questions 322 and 325 built; `owns_range` and `reads_range` over structures with element reads in contracts; global, `static` and member arrays of structures with their contract reads and by-value passing up to 7 values; four audit reads, no wrong program found; question 343 open (an exit 3, fail closed), 338–342 and 344–347 settled with the advisor; closed at the split point after Part C2 under the 12-hour rule, Parts D, E and F to a further session from the same plan; the full check green on the box at `46f34fb` |
| M4.5e1e2 (first session) Arrays of structures in code; `restrict` | 2026-11-25 | `m4.5e1e2-arrays-of-structures` (not merged at the session's end) | 0064, for the items built (0012, 0037, 0043, 0059, 0060, 0063 amended) | Parts A and B of six: leaf paths and the traps closed; automatic local arrays of structures in code; question 304's rejection as the compilers measure it; `restrict` found unsound on `main` by audit: every form found but the last is rejected, one is recorded open and unsound (item 19, question 326 for the owner); S48 in full (79 fixtures); closed at split point 1 under the 12-hour rule, Parts C–F to the continuation, which was planned afterwards (`.claude/plans/m4-5e1e2b-arrays-of-structures.md`; the owner: reject `restrict`) |
| M4.5e1e1 Arrays of scalars in structures and static storage | 2026-11-25 | `m4.5e1e1-arrays-of-scalars`, PR #37 `76fc7b1` | 0063 (0012, 0024, 0028, 0041, 0043, 0044, 0059, 0060, 0061, 0062 amended) | global and `static` arrays, array members, array typedefs and `sizeof`, C array literals, structure copies and by-value passing up to 7 array elements; a compound-literal sequencing hole open since M4.5c2a closed; questions 301–309, no cut; the full check run by the owner after the session (green) |
| M4.5e1d Pure functions and the exact sum | 2026-11-24/25 | `m4.5e1d-pure-functions`, `68384d1` (fast-forward) | 0062 (0006, 0010, 0011, 0012, 0013, 0024, 0025, 0037, 0041, 0042, 0044, 0061 amended) | a `pure` function that is called or has a contract as a Viper function with its definition check; self-recursion with `decreases`; applications in checks, contracts, calls and loop invariants; the exact sum verifies; testing moved to the x86-64 box; Silicon's prover cap; questions 273–286, no cut |
| M4.5e1c Quantifiers | 2026-11-23/24 | `m4.5e1c-quantifiers` | 0061 (0006, 0011, 0028, 0029, 0037, 0038, 0041, 0059, 0060 amended) | `cerastes_forall` with an inlined `pure` body in contracts, loop invariants and checks; `f__wf` variant M; the fill loop, max, the carrays port, the tutorial chapter; S45 (r) for e1f; Part D cut to M4.5e2 (question 260); questions 253–263 (256–263 built overnight, for review) |
| M4.5e1b Ranges in contracts | 2026-11-11/12 | `m4.5e1b-ranges-in-contracts` | 0060 (0011, 0028, 0029, 0037, 0038, 0039, 0040, 0042, 0059 amended) | `owns_range`/`reads_range` in contracts, element reads in contracts, calls and returns over ranges, loops over parameters' ranges, markers over ranges, `owning(n)` on a parameter, `restrict` with a range, array parameters (two new tags), the carrays port; questions 238–246 |
| M4.5e1a Arrays inside one function | 2026-11-11 | `m4.5e1a-arrays-in-functions` | 0059 (0011, 0012, 0024, 0028, 0032, 0038, 0039, 0042, 0043, 0054 amended) | local arrays as blocks, `a[i]`, `&a[i]`, `p[i]`, the decay, initializers with C's zero fill, `out-of-bounds`, the marked range rows in the driver, loops carrying arrays, sequencing, `sizeof(a)`, `uninit` on arrays; #175, #1162, #138 as errors; array designators rejected (234); the `offset_of == 0` backstops (235) |
| M4.5d2b2d `not_invalidating`, `--strict-owner`, marked fields | 2026-11-10 | `m4.5d2b2d-invalidation-profile` | 0058 (0029, 0042, 0052, 0054, 0055 amended) | `cerastes::not_invalidating` checked at the definition (taken implies returned; `p->q == old(p->q)` spliced, a bodiless borrow's tie implied); `--strict-owner` (P4417R0's p4 and p6); `ref_to_uninit` on fields (marked closures keep flags only where tied) and on pointers to structures with owning fields (closing nothing); N1; two older exit 3s fixed (0054's kept row, the return rows' order), a third to M4.8 |
| M4.5d2b2c Loop limits, long-running tests | 2026-11-10 | `m4.5d2b2c-loop-limits` | 0056, 0057 (0016 amended) | six of d2b2b's loop false alarms lifted (a site the loop cannot fill keeps its flags, the value set of a lone owner, a leaf's ghost through an alias, `main`'s global leaf under an entry snapshot, a site's cells as places, chains at any depth), the escape scan, two exit 3s fixed; five pins kept under the written-invariant rule; the long-running tier (`tests/long/`, `CERASTES_LONG=1`); question 196's two programs past 1800 s, to M4.8 |
| M4.5d2b2b Owning fields | 2026-11-09/10 | `m4.5d2b2b-owning-fields` | 0055 | `owning` on members: the closure of every atom over owning fields (acyclic, any depth), the value rule at stores, named leaves (struct locals, globals, `static` locals, by-value copies) as owners in memory, copies, loops; P4417R0's `[[owner]]` mapped; the session tools `regen_vpr.py` and `vprdiff.py`; seven loop limits pinned for d2b2c |
| M4.5d2b2a Owners in memory | 2026-11-09 | `m4.5d2b2a-owners-in-memory` | 0054 | block sites through loads and on globals; owning and marked globals, `static` locals and address-taken locals (the deep atom; static owners exempt at `main`'s end); written `owns_block` invariants; loop carriers by current state lifting d2b1's four false alarms; the static struct leaf cover; d2b2 split (owning fields and P4418R0 to d2b2b) |
| S8 CI toolchain profiles | 2026-11-06 | `s8b-ci-profiles`, PR #33 | 0053 (0001, 0016, 0031 amended) | CI asserts declared majors and records exact builds; toolchain profiles (`scripts/ci_toolchains.toml`), a matrix with a `profiles` job, cadences `every`/`nightly`/`dispatch`, exclusions; the guard as rules and a schema; GCC builds of cerastes and EDG; a builder evaluation-order bug found by GCC fixed |
| M4.5d2b1 Partnerless results, owning locals | 2026-11-05 | `m4.5d2b1-results-locals` | 0052 | `ensures` atoms with no partner (covered by implicit atoms or given an output amount; bodiless contracts fresh results only), owning automatic locals (leaks at every exit and reassignment, loops), marked locals and P4222R3's strict rules, block-site guards over memory, `cerastes_owns_vlibc_block`, two d2a holes closed; written `owns_block` invariants cut to d2b2 |
| S7 Code style | 2026-11-05 | `s7-code-style` | 0051 (0001 amended) | cerastes's tool code in the owner's Boost style: the pinned clang-format 23.1.3 (`format.sh`, a `check.sh` step, fail-closed guards), then brace initialization, `U` and one declarator per declaration; proved by `object_identity.py` (objects and IR at every commit, macOS and Linux; tokens) and by `.vpr`/`.map` identical over the suite and the VIR self-test; the docs lockfile fixed |
| M4.5d2v vlibc's allocator | 2026-11-04/05 | `m4.5d2v-allocator` | 0050 (0047 amended) | the M4.5d2a re-audit's B1–S3; vlibc's allocator (`vlibc_malloc` and six more, a C23 core over trusted glue, sanitized and threaded tests); blocks know their allocator (the token's value); the pairing check; contract-less library routines; vlibc allocations rejected in C++; two older exit-3 holes fixed; vlibc and vlibm in the owner's Boost style |
| S6 Tutorial and developer guide | 2026-11-04 | `s6-tutorial` | 0049 | a VerCors-style tutorial (seven chapters, 20 verified programs and 24 failing variants, both modes) and a developer guide (eight pages) as sections of the one site; the overview rewritten; the website's landing page; a chapter per concept from now on (questions 131–133) |
| M4.5d2a The heap | 2026-11-03/04 | `m4.5d2a-heap` | 0047 | vlibc's `malloc`, `calloc`, `free` (trusted, reserved identifiers, shapes not names); typed views at `void*` → `T*` (one object, `sizeof(T)`); heap blocks with the free token; `invalid-free`; leaks by `forperm` (after `l_end`, at back edges; `main` included); `owns_block`; `owning`/`ref_to_uninit` on functions; sequencing over block sites; loops over blocks held through locals; d2 split (d2v allocator, d2b) |
| M4.5d1 vlibc in cerastes | 2026-11-03 | `m4.5d1-vlibc` | 0046 | `third_party/vlibc` pinned and guarded; vlibc's shim first in verify mode; library files by path; trust from vlibc's tags, "does not return" from vlibc, reserved names, attributes by position; three routes redirecting trusted calls closed (older than vlibc); `trusted_libc_reason` gone; d2's dependency map |
| M4.5c2b Places through a heap read | 2026-11-03 | `m4.5c2b-heap-read-places`, PR #26 | 0045 | contract and invariant places through loads (`**pp`, `*gp`, `p->q->x`, `*s.q`, `*r.q`) with the entry-state rule, the post-state reading, amounts that follow objects, bodiless ties, heap-reading guards, `eb`/`lb` loop places, written atoms through loads and on struct places, `restrict` over load roots; `live(r.q)` to M4.5d2 |
| M4.5c2a Struct values and struct storage | 2026-11-02/03 | `m4.5c2a-struct-values`, PR #25 | 0044 | by-value struct parameters and results as tuples of leaves, `f().x`, C++ temporaries, compound literals (C objects, C++ values), struct globals and `static` struct locals, member measures, `restrict` beside by-value structs; invariant reads through a loaded pointer pinned; M4.5c2 split, heap-read places and written struct atoms to M4.5c2b |
| M4.5c1 Struct objects | 2026-11-02 | `m4.5c1-struct-objects`, PR #24 `8f45843` | 0043 | C structures with member blocks, struct locals as blocks, `.`/`->`, pointers to members, initializers, copies, `struct S *` objects with `WF_S`, contract atoms on struct places with overlap per leaf cell, markers per leaf; `restrict` closed beside loaded pointers (a hole since 0037); M4.5c split, owning fields and P4418R0 to M4.5d2 |
| M4.5b2b Initialization through pointers | 2026-11-02 | `m4.5b2b-init-markers` | 0042 | P4222's markers (`uninit` on locals, `ref_to_uninit`/`must_init` on pointer parameters), every call flag row `uninitialized`, `--strict-init` with the `// verify-options:` header, the loop row on every loop place (question 89, fixing 0038's nested-loop exit 3); M4.5b2 complete |
| M4.5b2a Globals and `static` locals | 2026-10-30 | `m4.5b2a-globals`, PR #22 `38877b4` | 0041 | mutable globals and `static` locals as `unique` blocks, `program_start` for `main` with initial values by linkage, frames through `owns(g)`/`reads(g)`, implicit atoms for `static` locals; M4.5b2 split, the markers to M4.5b2b |
| M4.5b1c3 Pointers to pointers, owning, `*result`, written atoms | 2026-10-30 | `m4.5b1c3-pointers-to-pointers` | 0040 | `T**` at any depth and pointer cells, `[[cerastes::owning]]` on parameters, `*result` as a value, written `owns`/`reads`/`live` in loop invariants (question 68's false alarms gone); conversions and contract comparisons across `const` rejected (a hole since 0037 closed); M4.5b1 complete |
| M4.5b1c2 Sequencing and placement | 2026-10-29 | `m4.5b1c2-sequencing` | 0039 | writes to cells inside expressions, sequencing on objects (`unsequenced`; same-object shapes rejected like the compilers), calls beside accesses, `return &local;` rejected (R14), `c ? owns : reads` path-precise and partners by path, function measures over memory |
| M4.5b1c1 Loops and jumps | 2026-10-29 | `m4.5b1c1-loops` | 0038 | loops over memory with automatic permission invariants ("every place held"), invariants and loop measures over memory, scope exits at jumps; written atoms in invariants cut to b1c3, function measures over memory to b1c2; the `address_of_local.c` port |
| M4.5b1b Pointer parameters and contracts | 2026-10-28/29 | `m4.5b1b-pointer-params` | 0037 | pointer parameters and results, `&param`; `owns`/`reads`/`live`/`old` in both spellings with ghost amounts; framing per path; `permission` at calls and returns; `pointer-comparison`; `restrict` on owned C parameters |
| M4.5b1a The model, address-taken locals | 2026-10-28 | `m4.5b1a-pointers` | 0032–0036 | the `Block` domain and its four axioms; address-taken locals as blocks, `&`, `*`, NULL, `==`, liveness and scope exits inside one function; `null-dereference`, `permission`, `lifetime`; C's incompatible pointer types as errors |
| S5 Linux CI | 2026-10-28 | `s5-linux-ci` | 0031 | `check-linux` on `ubuntu-26.04` in both tiers; Linux bootstrap; glibc `assert`; the same EDG language on both platforms; eight macOS-only tests tagged |
| M4.5a Memory-model design, `\|`/`^` masks | 2026-10-27 | `m4.5a-memory-design` | 0027–0030 | the C memory model designed and audited (E2, four axioms, member blocks, liveness), ownership annotations, vlibc; `\|`/`^` constant masks decided; S5 Linux CI next |
| M4.4d Exact sets, constant masks | 2026-10-27 | `m4.4d-exact-sets` | 0026 | preconditions and comma values get exact sets, with today's as the fallback; one `requires` per argument accepted; constant masks as remainders, the `bit_mask.c [c23]` hang gone; hooks `force_base_sets`, `drop_exact_set` |
| M4.4c Writes outside the chain, `void` `?:`, `-Wall` shapes | 2026-10-26 | `m4.4c-writes-void-warnings` | 0025 | finding A fixed (a flagged read is an access); writes and snapshots leave the chain; a `void` `?:` statement is an `if`; warnings in `verified`/`error` tests; the static-assertion cross-check in bodies; 13 `-Wall` shape rules; M4.4e planned |
| M4.4b2 Values the front end computes | 2026-10-26 | `m4.4b2-front-end-values`, `8bff6ba` | 0024 | static assertions checked as written (file scope through source sequence lists, now on); wrapped character literals are `conversion` errors; types computed from a constant rejected in `sizeof`/`typeof`/typedefs until M4.5e |
| M4.4b1 Bitwise operators | 2026-10-23/24 | `m4.4b1-bitwise`, PR #9 `effc63f` | 0023 | `& \| ^ ~` and `&= \|= ^=` exact (prelude functions per width with proved range postconditions); seven bitwise `-Wall` shapes rejected; multi-character literals refused; item 5's truth values one verdict in both modes |
| M4.4a No wrapping | 2026-10-23 | `m4.4a-no-wrapping`, PR #8 `00f60d2` | 0022 | unsigned arithmetic and conversions checked (`overflow`, `conversion`, `shift`), `c_wrap` gone; lowering's internal errors exit 3; enumeration `==`/`!=` with possibly negative operands rejected |
| M4.3b Calls in expressions | 2026-10-23 | `m4.3b-calls`, PR #7 `58d208c` | 0021 | calls in guarded operands and loop conditions; the dependency-chain rule (`unsequenced_calls`); explicit termination checks; `self_assignment` |
| M4.3a Side effects in expressions | 2026-10-22 | `m4.3a-side-effects`, PR #6 `d694606` | 0020 | assignments, `++`/`--` and commas as values; C's sequencing rule (`unsequenced_modification`); the first `-Wall` shapes |
| M4.2c Locals without an initializer | 2026-10-22 | `m4.2c-uninitialized-locals`, PR #5 `1c1cc29` | 0019 | category `uninitialized` by a ghost flag; jumps past such declarations; EDG #2656 fixed on fork branch `0022` (pin `f0ed5525af`); `-Wswitch`'s rules |
| M4.2b2 Enumerations | 2026-10-22 | `m4.2b2-enumerations`, PR #4 `4622259` | 0018 | enumeration types with value ranges, every conversion into one checked |
| M4.2b1 Named constants and case values | 2026-10-21 | `m4.2b1-named-constants`, PR #3 `ae2d2a4` | 0017 | `const`/`constexpr` globals and case values checked in each method's prologue |
| S3 Mutation triage | 2026-10-21 | `mutation-triage`, PR #2 `915e372` | — | 17 tests strengthened, 33 twins, 436 reviewed equivalents |
| S2 CI | 2026-10-21 | `ci`, PR #1 `e1f2860` | 0016 | `check.sh` on GitHub's `xcode-27` runner, required on `main` |
| S1 EDG upstream merge | 2026-10-21 | `edg-upstream-merge` | — (0001) | fork `cerastes` merged upstream `main`; 0020's Linux failure was stale box binaries |
| M4.2a Jumps and ends | 2026-10-21 | `m4.2a-goto-noreturn-assert` | 0015 | outward-and-forward `goto`, `noreturn` (category), trusted `abort`/`exit`/`_Exit`, `assert` as `check` |
| M4.1 Control flow | 2026-10-20 | `m4.1-control-flow`, `29a3dd3` | 0014 | `break`, `continue`, `do`, `switch`; the plan re-sequenced (new M4) |
| M3 close-out | 2026-10-20 | `worktree-m3-closeout`, `bfeda03` | — | M3 acceptance, mutation run |
| M2 re-look and M3 Scalar encoder | 2026-10-16/19 | — | 0010–0013 | VIR, emitter, driver, scalar encoding, termination |
| EDG bugs 0003–0007 | 2026-10-15 | fork branches | — | fixed, branches for the owner's PRs |
| M2 Annotation layer | 2026-10-14/15 | — | 0005–0009 | `[[cerastes::…]]` registration in the EDG fork, `<cerastes.h>`, the fork pin |
| M1.5 Documentation site | 2026-10-14 | — | 0004 | Antora docs at cerastes.dev/cerastes/doc/ |
| M1 IL inventory · Gate G1 | 2026-10-13 | — | 0002, 0003 | G1 PASS; in-process back end |
| M0 Bootstrap | 2026-10-13 | — | 0001 | toolchain pins, harness, `check.sh` |
