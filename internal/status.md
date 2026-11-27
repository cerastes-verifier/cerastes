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
the first session of M4.5e1e2's `git show ae622e5:internal/status.md`, the first session of its
continuation's `git show 493a923:internal/status.md`, and the second session of its continuation's
`git show 42880a1:internal/status.md`.

## M4.5e1e2, the continuation's third session — Part E: arrays of arrays, through two dimensions (2026-11-26/27): three commits; the full check green on the box at Commit 7 `0a4eb26` (18,190 tests, the long tier included); closed inside Part E: a lint step and three dimensions open the next session; Part F not begun; for the owner: three internal errors found after the first commit and fixed, a matrix of structures not built, five rejections and two lint lifts to review, a session of 21 hours

**What a reader needs first.** Arrays whose elements are arrays (`int m[2][3]`) are verified now: as
automatic locals, as parameters that take rows (`int a[][3]`, `int a[2][3]`, `int (*a)[3]` and the spellings
through a typedef), as globals and `static` locals, and as members of structures, with `m[i][j]` checked one
index at a time, ranges over rows in contracts, contract reads, loops and sequencing. An element structure
may have an array member of scalars (`v[i].a[j]`) in every storage, with whole-element copies. Array types
of arrays and of structures are accepted in `sizeof`, typedefs and static assertions wherever an object of
that type is. A structure with a matrix member can be passed, returned and copied as a whole
value, up to 7 cells. **Not built:** three dimensions (`int c[2][3][4]`): rejected with a pin, the next
session's step after its lint step, before Part F; a matrix of structures (question 363, the owner's); Part F
(quantifiers over members). Six audit readings (a fresh Fable auditor each) found no wrong program that
verifies. **Three internal errors of one family** (a row of a matrix asked for what only a structure or a
cell has; each fail closed: exit 3, "the program was not checked", never a wrong proof) were found in the
first commit's tree after it was cut: the second commit fixes two and the third commit fixes the third, so **a checkout of the first or of
the second commit alone still has them** (below). **The owner's attention is asked for** (handoff §8, in this order): question 363; the lint's
guard-shaped branch (question 374, with 385, 386 and 389) and its second site (391); three shapes that worked in the auditor's probes
and are rejected because the lint cannot check their Viper or the record did not name them (382, 383, 384);
two listed rejections (377, 381); a first step for the next session that the owner already agreed to (the
lint step: 369, 373, 379, 380 and the table collision); the three internal errors and how each was
handled (392); two older defects the last audit read met (393: a crash, with no diagnostic, on a by-value
structure of about 4,095 members; 394: a false alarm at a call); the macOS re-approval of rebuilt binaries; four deletions that agents were denied and
nobody made in their place; an uncommitted change of the owner's to `plan.md` in the main checkout, which
stops the fast-forward merge until it is committed or stashed; and the review of the questions settled
with the advisor (358 to 394).

Plan: `.claude/plans/m4-5e1e2b-arrays-of-structures.md` (the continuation plan), executed from Part E after
its Step 0 again (without P84 and R4), without re-planning; the owner's first message named the plan and
the part. Record 0064 (accepted for the items built: item 13 through two dimensions with this session).
**Base:** `42880a1` = local `main` at the start (the tip of `m4.5e1e2c-arrays-of-structures`; `86442ce`,
the commit the last full check passed on, is an ancestor and nothing under `src`, `include`, `tests`,
`scripts` or `doc` differs from it). **`origin/main` was `f07d80a` and `origin/m4.5e1e2c-arrays-of-structures`
was `42880a1` at the start; nothing was pushed by this session.** Branch `m4.5e1e2d-arrays-of-structures`,
worktree `.claude/worktrees/m4.5e1e2d`. Session from 2026-11-26 20:17 UTC to 2026-11-27 17:20 UTC (about 21 hours, against the plan's twelve: the owner lifted the limit at 03:38). The owner
gave no overnight rule; every question went to the Fable advisor first.

**The owner's decisions in the session.** (21:56) "I have approved the file access" (a macOS consent prompt
had blocked every build on the Mac for 45 minutes: below). (00:43, on the twelve-hour cap, which Part E in
full would pass) "Commit the current part and then move to the next slice here as well": Part E is
committed after its third slice and the fourth slice is built in the same session. (03:38) "You may go over
time as long as it's not going to be affected by overly full context. I'll follow your recommendation on
the issue with 369 for the next session": no time limit; the lint step of question 369 opens the next
session. (10:19, a push notification of mine) asked whether three dimensions should be built in this session
(about three hours more) or left; no answer came by the close, and the default stated in it (left for the
next session) was taken.

**How the part was built** (deviations from the plan, all on the advisor's word unless said). The plan
gives Part E one il-analyst, one joined proof and one audit round. Its lowering was instead cut into four
slices and each slice into proved sub-steps (a snapshot per sub-step: the Mac compile, the format check, the
VIR self-test with 0 problems, the part's cases through Silicon, every older output byte-identical to the
baseline, a mutation check), built by a chain of eight il-analyst instances as each ran out of context
(the last one resumed three times for the fixes below);
the encoder half started from last session's unproved draft, re-read function by function against the
record (three things it lacked were built); joined proofs of j1, j2, j3, j3b, j4a, j4, then of the trees
the commits were cut from (j4x and j4y for Commit 5, j5 for Commit 6, j6 and j6b for Commit 7); **six audit readings** (E1 on j2,
E2a on j3b, E3a on j4a, E3b on j4, a short one S4 on j5, a shorter one S5 on j6), since one auditor could not read the part in
full (round E2a used 720k tokens on a third of it). Six edg-backend and twelve test-mutator instances.
Every commit is the joined tree exactly (`git commit-tree` on a tree composed from the agents' snapshot
refs), never a `git add` of the working tree.

**Done** (commits on `m4.5e1e2d-arrays-of-structures`, local, no Claude trailer):
- **Commit 5 `670ae10`** (12:00 UTC; Part E through slice 3, record 0064 item 13 through two dimensions,
  with the fixes of the four audit rounds before it). Sizes against the base: `src/` without the
  self-test cases +4,978 −639 (`encode.cpp` +2,371 −189, `lower.cpp` +1,909 −375, `check.cpp` +412 −73,
  `vir.cpp` +187 −2, `vir.h` +65, `selftest.cpp` +34); the self-test cases +8,827 −28; the lint +2,569
  −158; 175 test files and three doc examples added, six test files deleted, two modified (the plan sized
  Part E's `src/` at 800–1,500 lines: the part came to more than three times its upper size).
  - **Local matrices** (slice 1): `m[i][j]` with one bound row per index (the second against the row's own
    length: `m[0][3]` of an `int m[2][3]` is `out-of-bounds`), nested initializers with C's zero fill, a row
    as a pointer, `&m[i][j]`, the trusted `alloc_arr_row_<t>_<M>` and `zero_arr_row_<t>_<M>`.
  - **Row parameters** (slice 2) in six spellings; `owns_range(a, n)` and `reads_range(a, n)` over rows with
    calls, returns and forwarding; contract reads `a[e][e2]`; the pairing and sequencing rows of a range
    over rows; loops; the tie of a row element's two states under `reads` (question 367).
  - **Global and `static` matrices, matrix members** (slices 3a, 3b): `program_start`'s row groups, `owns(g)`
    and `reads(g)`, loops in `main`; a member in a local structure, under `owns(*p)`, `owns(p->m)`,
    `reads(*p)`, in a global structure, through a chain of by-value members; contract reads of each; a
    global or member matrix passed whole to a row parameter. No tie of a member matrix's cell across a
    call (question 345's listed false alarm).
  - **`v[i].a[j]`** (slice 3c): an element structure with an array member of scalars as a local, under a
    parameter's range (with `p[e].a[e2]` in the contract), as a global and a `static` local, as a member;
    the ties of question 376 for the range and global forms; the whole-element copy with its guard only
    where the two places can be one object (slice 3d); the Sequencer's key for a cell of an element's
    array member (question 378).
  - **Computed types:** a typedef, `typeof`, `sizeof`, `_Alignof` or static assertion naming an array of
    arrays or of structures follows the rules of an object of that type.
  - **Rejections added on audit findings**, each by the encoder at its C line with a pin: `return m[i];` and
    its forms (`return_local_address`); a row tested or compared with null, and the folded element
    addresses of static-storage arrays in both modes (`array_address_null_test`); question 348's six
    positions and a seventh route for a structure whose array member holds arrays or structures (three
    structure types, three pin files and three companions); the nested element of 382; a marker over a
    range of structures with an array member (383); an atom under a guard on one of Part E's objects
    (384); `cerastes_old` of `g[e].a[e2]` and of a global structure's matrix member (381, 346).
  - The pages (`memory.adoc`: "Arrays of arrays", "Parameters that take a matrix", "Global, `static` and
    member matrices", the passage on an element's array members, the not-yet lists; `annotations.adoc`,
    `error-messages.adoc`, `overview.adoc`, `verifying.adoc`; the tutorial's matrix example in "Arrays in
    aggregates"), three doc examples, `internal/encoding.md`, amendment lines in records 0024, 0028, 0039,
    0059, 0060 and 0063.
- **Commit 6 `68146c6`** (14:34 UTC; slice 4: record 0064 item 12 for the two types of depth 2). `src/`
  without the self-test cases +149 −51 (`lower.cpp` +61 −10, `vir.cpp` +44 −2, `check.cpp` +19 −28, `vir.h`
  +20 −10, `encode.cpp` +2 −1, `selftest.cpp` +3); the cases +1,574 −29; the lint +11 −4; 30 test files
  added (one in `tests/long`), one modified.
  - **Structure values of depth 2:** a structure with a matrix member (`struct U { int a; int m[2][2]; int
    b[2]; int c; }`) or with a member array of structures that have an array member (`struct T { struct W
    v[2]; int k; }`) passed by value, returned, stored from a call's result and copied as a whole (`w = u;`,
    `u = u;`, `*p = *q;`, an initializer from an object), under the limit of 7 cells (a value counts every
    cell reached through an index; a whole copy counts only the cells of members that are arrays of
    aggregates). One enumeration of a value's cells (`vir::value_cells`) serves the encoder and the
    lowering. A contract read of `v.m[i][j]` of a by-value parameter and `f().m[1][2]` stay rejected
    (question 364).
  - **Two of the three internal errors fixed** (below), each with self-test cases, a mutation check and a
    C-level pair (`matrix_first_member_pair*`, `matrix_reads_pair*`).
  - The page (`memory.adoc`: whole values of a structure with a matrix member), record 0064's item 12 as
    built, with its costs.
- **Commit 7 `0a4eb26`** (15:54 UTC; the fix of the third internal error, cut as a commit of its own on the
  advisor's word, inside a time box of about forty minutes: it was proved in 25). `lower.cpp` +14 −2
  (`split_candidates`: a matrix member's row is split against the first cell of its first row, as a local
  matrix is; and, on the last audit's note, a structure value whose cell list reaches the enumeration's cut
  of 4,096 cells is stopped with an error), 16 self-test cases (+183 lines), the C-level pair
  `tests/memory/matrix_member_split.c` and `negative/matrix_member_split_wrong.c`, the long twin's header
  from `// timeout: 1800` to `3000`, `VIR_CASES_MIN` 3351. No existing program's Viper changes (7,435
  outputs identical to Commit 6's). The pair was probed and run only on builds with the fix; that the
  shapes ended in exit 3 before it rests on the audit's 34 runs on Commit 6's tree and the encoder owner's
  four on a build of Commit 5. The new stop is wider than its message (a by-value structure of 4,096 or
  more scalar cells with no array is stopped too, with the words "is cut there"), and just under it a
  4,095-cell by-value structure ends the tool in a segmentation fault, as both sizes do on the base's
  build (measured; question 393).
- **The three internal errors** (question 392; all fail closed: exit 3 and "the program was not checked",
  nothing is reported verified). Each asks a ROW of a matrix for something only a structure element or a
  cell has; the four audit rounds before Commit 5 did not meet them because their two-pointer probes used a
  structure whose matrix is not the first member, one `reads` atom at a call, and no pointer to the cell's
  type beside the structure.
  1. A structure whose FIRST member is a matrix (`struct U2 { int m[2][2]; int c; }`) under `owns(*p)`
     beside `reads(*q)` or `owns(*q)` (`overlap_pairs`): found by the lowering owner while it wrote slice
     4's cases; in Commit 5 (measured on its build); fixed in Commit 6 (5 lines).
  2. A CALL of a function whose contract has two `reads` atoms of a structure with a matrix member
     (`call_atoms`): found by the encoder owner's probes of the first fix; in Commit 5 (measured); fixed in
     Commit 6 (7 lines; two mutations survived the first case set and were closed with two more cases).
  3. A structure with a matrix member in a function that needs a pointer split, for example beside an
     `int *q` parameter (`split_candidates`): found by the short audit read S4; in Commit 5 and in Commit 6
     (measured on a build of Commit 5 and on j5). Fixed in Commit 7 (the split is the no-op branch of record 0037 item
     3: precision, no verdict; the lowering owner's table of the family, by reading, is in its report 10).
     With the fix the programs get their ordinary verdicts (the audit's first example is a `lifetime`
     error: it compares a pointer no atom covers). All 34 of the audit's exit-3 runs end in a verdict on
     the fix's build.

**Not done.** Three dimensions (record item 2 admits depth 3; its lowering was not begun: a new trusted
allocation, a third bound variable through every walker, lint rules and an audit round): rejected by the
encoder behind a closed constant with a staging pin; **the next session's step after its lint step, before Part F**
(a split inside Part E under the owner's context condition; not the plan's cut (3)). Six test drafts for
it are in the worktree, untracked and in no commit, with copies in `build/m4.5e1e2d/depth3-drafts/` (the
handoff names them).
A matrix of structures (question 363). Part F. The listed rejections of questions 377 (a loop invariant
that reads `p[e].a[e2]` under a range) and 381 to 384.

**Found on the way** (none a wrong proof in a committed tree; each fixed before its join unless listed):
- In the draft and the first slices: the draft accepted bare reads of a row parameter and lacked a matrix
  passed to a row parameter; `site_pointee` read a row parameter's atom as a range of `int`; `apply_frame`
  would have framed a scalar application by an interval over row slots; an unatomed row parameter ended in
  exit 3.
- By audit E1: `return m[i];` verified where `return a;` is rejected; and on the way `m[0] == 0` verified.
- By a sweep: with a constant index into a static-storage matrix the front end folds the address, and in
  C `if (g[1])` and its forms verified (so did the one-dimensional `&ga[1]` in a C++ `check` or invariant
  since Part R): closed in both modes, 166 forms probed.
- `cerastes_old(gu.m[1][1])` verified on an interim build where the record rejects it.
- By audit E2a: a structure with a matrix member under an atom ended in exit 3 on j3b (the sub-step that
  builds it landed after); the lint admitted a one-binder row over a matrix's block (tightened).
- Before audit E3a, by measurement: the element copy `v[i] = v[j]` was live on an interim build with no
  lowering case; `program_start`'s zero quantifier for elements a list does not reach had no case (the
  test-mutator saw that no text held it).
- By audit E3a: the nested element stopped at `check.cpp`'s backstop; a seventh route for 348 stopped
  there too; five accepted shapes whose Viper the lint refuses (two became rejections, one a lint rule,
  two are listed); question 374's lint branch admitted a row length contradicting the method's own row
  when a second structure type stood in the file.
- "Lint clean on the self-test's text" did not predict the C-level text of `struct T { struct W v[2]; int
  k; }` (85 refused lines): rules were written from regenerated text, and the auditor's 332 kept files of
  accepted programs became the lint's regression corpus.
- By audit E3b: record item 10 had a wrong sentence of mine (an invariant read of `p->v[e].a[e2]` with a
  symbolic index verifies; what is `permission` is a first index that may be outside `p->v`): corrected, and
  the pair no C-level test held was added; three more accepted shapes whose Viper the lint refuses (listed,
  question 390); the lint's table collision is one declaration away from every member-form test unit (two
  new test files hit it and were reshaped; the lint was not edited).
- After Commit 5: the three internal errors (above). By the short read S4: Rule A of the lint (question
  391) lifts more than the two copies it was written for and can admit a length that is not the type's own
  where another declared structure of the unit has a scalar array at that member index.
- In my own log and record: two log lines stated as fact what had not been run (one was true when
  measured, one named the wrong agent); my criterion for question 384 was meaningless for a type the lint
  had no rules for, and the encoder's rule was built wider than my record sentence (the record was widened
  after the code: a deviation from invariant 6's order).

**Audits** (Fable at `xhigh`, a fresh instance per round; reports
`build/m4.5e1e2d/auditor-report-{E1,E2a,E3a,E3b,S4,S5}.md`, probes under `audit-<round>/`):
- **E1** on j2 `277f58f` (the encoder's diff in full, slices 1 and 2a-2d, the lint): 453 probe sources, 958
  runs; no wrong program verified, no blocking finding; should-fix: `return m[i]`; a `pure` application
  beside a range over rows ended in exit 3 (fixed in a later snapshot, re-run); rules with no C-level
  test. By mutation of kept Viper: a pairing row made `true` still fails at Silicon's own precondition
  check; a SEQUENCING row made `true` lets two probes verify (nothing stands behind those rows, as the
  record says).
- **E2a** on j3b `a6ee8d7` (loops, the tie, global matrices, the first member form, the gates, the lint): 835
  sources, 1,658 runs, 72 lint mutants; no wrong program verified, no blocking soundness finding; 348 holds
  for a matrix member; ten findings and notes taken (above).
- **E3a** on j4a `0cfc232` (members under atoms, global structures, contract reads, `v[i].a[j]` as a local,
  a range and a global, 376's ties, the lint diff in full): 552 probes, about 1,090 runs; no wrong program
  verified, no blocking finding; 374 "stands with a change"; the live copy right in every probe; the three
  surviving mutations of the member sub-step fail closed (by reading).
- **E3b** on j4 `5606bb1` (the diff since j4a: the member form of `v[i].a[j]`, the copies, the four
  rejections, the lint): about 1,220 runs; no wrong program verified, no blocking finding; nothing a user
  can reach ended at a `check.cpp` backstop text or in an internal error; 374 (ii), 386 and both edits of
  389 stand; no lowering or encoder change asked; eleven findings and notes (above; costs below).
- **S4** on j5 `6c07457` (a short read in full of slice 4's diff and the two fixes): 708 runs; no wrong
  program verified, no blocking finding; the private cell count agrees with the one enumeration for every
  type; the enumeration against C, the whole copy, the limit of 7 and the two fixes right; everything that
  must stay rejected is rejected by the encoder; should-fix: the third internal error; Rule A "stands with
  a change" (391); a note for the owner's eye under invariant 1: four `expect:` lines of the staging pin
  `negative/matrix_reject_slice4_staging.c` were removed, which are the plan's Flips for staging pins.
- **S5** on j6 `8fdbec8` (a shorter read of Commit 7's fix; `src/` identical to the commit's): 322 runs; no
  wrong program verified, no blocking finding; established by reading and probes that the split's
  candidates reach only no-op branches, so no verdict rests on them; every aliasing call with the address
  of any matrix cell fails `permission`; the 34 runs of the earlier exit 3 all end in a right verdict; all
  24 call sites of `elem_leaves_ext` guarded. Should-fix, none taken into the commit (handoff §8, after
  394): the self-test's text pin for the candidate cannot tell the first cell from the first row's slot;
  the stop's message; the segmentation fault (393). Notes: a false alarm of the base in the amounts a call
  lends (394); no C-level failing twin for a matrix cell behind the unknown pointer; a header's prose.
  It did not re-derive the tables of `row_leaf`, `cell_suffix` and `elem_cell_suffix`, and did not
  reproduce the exit 3 before the fix, the byte-identity or the mutation run.
- **What no auditor read:** the self-test cases beyond names and samples (`selftest_cases37.inc`, 10,342
  lines); `scripts/test_lint_viper.py`; `encode.cpp` outside each round's diff; the unchanged
  bodies of the large lowering functions beyond their hunks (each round names them). Not reproduced by any
  auditor: the vprdiff neutrality runs, `rejections.py`, the agents' mutation checks and lint weakenings.

**Tests.** 207 test files and three doc examples added (the plan's cap of 160 for the milestone was
long passed: the excess is audit- and question-derived and listed by the test-mutators' reports 1 to 23);
six staging pins retired for positives (Flips A (E)). **Runs of the plan's budget:** the joined proofs
(above); the part's new tests once on j4 (65 files, both modes, one invocation: 222 passed, 0 failed), the
14 files that changed after it on j4x (44 passed, 4 failed: the `native-ok` compiles of one new positive
with unused parameters; corrected, re-joined as j4y and run again: 8 passed); slice 4's files on j5 (113
passed, 0 failed; the long twin by its own invocation); the fix's pair with the two earlier pairs of the family on j6 (34 passed, 0 failed) and the pair's
second cut on j6b (12 passed, 0 failed); the preflight once, on j5. The VIR self-test:
3,351 lowering and 554 rejection cases (`VIR_CASES_MIN` 3351).
**Corrections of new tests under question 160** (each a test-side defect, found at a first run or probe,
never an `expect:` line edited to fit a verdict): `array_rows_spellings.c`, `array_rows_alias.c`,
`array_rows_reject_forms.c` (twice: its 377 row moved to `negative/array_rows_reject_inv_range.c`),
`array_rows_reject_bare.c`, `matrix_lent_kept_lifetime.c`'s `lifetime` rows, `matrix_global.c`,
`matrix_sizeof.c`, `negative/matrix_global_wrong.c`, `negative/matrix_return_local_address.c`,
`negative/matrix_return_local_ok_use.c`, `negative/array_rows_struct_elem_global_wrong.c`,
`negative/matrix_loop_entry_exit.c`, `negative/matrix_member_reject_old.c`,
`negative/matrix_return_local_address_members.c`, `negative/matrix_typedef_wrong.c` (a second edit under
question 388), `array_rows_struct_elem_loop_wrong.c`, the audit-E2a batch (8 of 13 files before a run);
under question 277 `array_rows_forward_past.c`; five program-only corrections among test-mutator-10's 45
files and the 382 pin's five unlisted warnings; `matrix_value_limit_eight.c` split by mode (C++ reads an
initializer from a compound literal as a braced initialization and accepts it). test-mutator-12's four corrections of its own new pair before its cut.
**Flips S** (self-test cases re-pointed or removed as their refusals were lifted; the list per instance
is in `build/m4.5e1e2d/status-draft.md` and in il-analyst reports 1 to 9): older refusal cases re-pointed
to depth 3 or removed as their refusals were lifted in each slice, and in slice 4 six older rejection
cases rewritten (the limit at 8, the positions, question 364's two forms).

**Costs and limits listed** (each fail closed; record 0064 has each): no tie of a matrix member's cell or of
`p->v[e].a[e2]` across a call (345); a loop writing every `u.m[i][j]` loses `u.n` (item 10's reach); several
loops over global matrices in ONE function are slow (four: 147 s; question 387); `v[i] = v[j]` through two
pointers with a failing claim (361 s in the lowering's case; in C 414 s on the audited build, no verdict
under the driver's default 300 s); several whole copies of 9-cell structures with a false claim and `*p =
*q` under two `reads` get no verdict in 300 s in C; this session's long twin
(`tests/long/memory/negative/matrix_value_copy_ptr_two_wrong.c`) takes 380 s a mode on a quiet box and 859
s at a load of 37 (its header allows 3,000 s since Commit 7); four normal tests of 63 to 97 s under load (the harness's limit is 600 s);
the self-test case `e1e2e_rrcall_t_distinct` 117 s alone (limit 600 s); a first index that may be outside `p->v` in a loop invariant is
reported as `permission`, not `out-of-bounds` (a true error under another category's words); `sizeof(a[0])` of a row parameter in a contract is rejected
(over-strict); `int *q = &gu.m[1][0];` as an initializer is rejected in C only; message texts (a cell of an
element's array member prints without its element; a back-edge row that names `u.n`; the driver sorts
printed errors by text).

**The Viper lint** (`scripts/lint_viper.py` +2,569 −158). It is an allow-list, and every new shape of Part
E needed a rule tied to the trusted method's or the group's own counts, with refusing twins on real
emitted text; each lifted refusal is listed in the test-mutators' reports with its rule, message, Viper
lines and record item. Question 357's boundary (no amount, guard or kept-flag rule loosened) was held by
function-level diffs, with three exceptions that are the owner's to review: question 374's branch (a
guard-shaped lift made against the stop rule, then tightened twice by audit findings: 385, 386, 389),
question 375 (kept rows over a place that a new rule makes known) and question 391 (Rule A of the second
commit: a length fact at 374's site, tied by number; the audit's "stands with a change" is the next
session's type-keyed table). **Known gaps, for the lint step that
opens the next session:** 369 (`leaf_value` accepts any literal range for an unknown initial value), 373
(older rules Z1-Z3, A5), 379, 380, the table collision (two structure types with an array member at one
member index and different counts: every range over either is refused: 16 test units hold two such types
and 4 hit the collision without a range), accepted shapes whose Viper the lint refuses (a global matrix of
pointers to structures with an unknown value; `owns(p->n)` beside a contract read on a structure with a
member array of structures; guarded atoms on Part C2's structures), no completeness rule for a structure
local's scope exit.

**The environment.** macOS treats every rebuilt, ad hoc-signed `cerastes` or `cerastes-vir-selftest` as a
new application and asks for file access again; while the prompt waits, every open of `~/Documents` blocks
and with it every build (21:10 to 21:56). From then on agents only compiled on the Mac and ran binaries in
the box lanes (three self-test runs on the Mac happened against that rule). The lasting fixes are the
owner's (handoff §6). The box's own clock is one hour ahead of UTC. Four deletions by agents in ignored build or lane directories were refused by the permission system (an
`rm` of a broken copy, an `rm -f` with a glob, an auditor's `rm -rf` in its lane, an `rm -f` of a probe
directory that did not exist yet); each agent stopped or went on without the line, and nobody deleted
anything in its place.

**Questions settled with the advisor: 358 to 394** (handoff §8, each with its soundness statement, the
answer and the alternative). For the owner: 363; 374 (with 385, 386, 389, 391); 382, 383, 384; 377, 381;
392 (the three internal errors), 393 (a segmentation fault on a very large by-value structure, older
than this session), 394 (a false alarm of the base at a call). Test mechanics: 358, 360, 368, 370, 371, 387, 388. Lint: 369 (answered by
the owner), 372, 373, 375, 379, 380, 390.

**Runs outside the plan's budget** (single-file probes named as such in each report; the list per instance
is in `build/m4.5e1e2d/status-draft.md`): the agents' end-to-end probes of each sub-step's drafts in their
lanes; the sweeps (truth tests of a row: 332 runs; 348's positions per type; the seventh route: 200; the
nested element: 90; guarded atoms: about 100; the `old` table); `scripts/test_run_tests.py` twice on the Mac
and once in the lane; the orchestrator's own (seven console-text runs on j3; the tutorial's variant; four
scalar-cell probes on the frozen base; two large-structure files once on the base's build for question
393); after Commit 5: the encoder owner's tables for the first-member
matrix (on Commit 5's build and on the fix) and for two `reads` atoms at a call (48 files), its build of
Commit 5 for the third defect, one self-test case timed alone, the audit's 17 files on the fix's build, and
test-mutator-12's five probes of its pair. **The full check** ran once, after the last code change, on Commit 7 `0a4eb26` in
`~/cerastes-s8/p-ubuntu-26.04` on the box (`CERASTES_LONG=1 CERASTES_JOBS=32`): **green** in 65 minutes (`check.sh: all green`; 18,190 tests passed, 0 failed, 10 skipped: the nine
macOS-only tests of decision 0031 and the VeriFast originals' directory, as before; native-ok runs clang
3,429 and GCC 3,429; the long tier ran; the Viper lint 7,439 files clean; the VIR self-test 3,351 cases,
all through Silicon; the harness self-tests, the walker oracle and the docs with their example checks).
The last session's check had 17,488 tests. Log: `build/m4.5e1e2d/check-c7-0a4eb26.log`; on the box
`check-e1e2d-c7.{log,start,exit}` in the check lane, which sits detached at `0a4eb26`.

**The exact first step of the next session.** Start `claude --agent orchestrator --effort high` with the
first message `M4.5e1e2 continuation, from the lint step and three dimensions
(.claude/plans/m4-5e1e2b-arrays-of-structures.md)`. It makes the branch `m4.5e1e2e-arrays-of-structures` from
the tip of `m4.5e1e2d-arrays-of-structures` (or from `main` once the owner has merged), the worktree
`.claude/worktrees/m4.5e1e2e` and the session directory `build/m4.5e1e2e/`, runs the plan's Step 0 on the new
base (no P84), copies `build/m4.5e1e2d/` to its `prev/`, and then, in this order: the lint step (handoff §2
step 4 and §3; the test-mutator's, with one audit read), three dimensions (handoff §3: the lowering's first
step is in `il-analyst-report-7.md`, "4c"; its first two items are the last audit read's), Part F from the
plan's section 7.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| M4.5e1e2 (the continuation's third session) Arrays of arrays through two dimensions; `v[i].a[j]`; structure values that hold a matrix | 2026-11-26/27 | `m4.5e1e2d-arrays-of-structures` (not merged at the session's end; nothing pushed) | 0064, for the items built (0024, 0028, 0039, 0059, 0060, 0063 amended) | Part E of the continuation plan through two dimensions, in three commits: matrices as locals, array-of-rows parameters (three spellings and through a typedef), globals, `static` locals and members, with ranges over rows, contract reads, loops and sequencing; an element structure with an array member of scalars (`v[i].a[j]`) in every storage, with the whole-element copy; computed types of arrays of arrays and of structures; structure values that hold a matrix passed, returned and copied whole under the limit of 7 cells; three internal errors of one family found after the first commit and fixed in the second and third (fail closed, question 392); six audit readings, no wrong program verifies; the Viper lint grew by about 2,580 lines with listed gaps, so the next session opens with a lint step (369); three dimensions not built (the owner's context condition), a matrix of structures not built (363), Part F not begun; the full check green on the box at Commit 7 `0a4eb26` (18,190 tests, the long tier included) |
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
