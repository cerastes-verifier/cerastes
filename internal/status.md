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
and the first session of M4.5e1e2's `git show ae622e5:internal/status.md`.

## M4.5e1e2, the continuation's first session — `restrict` rejected (Part R), ranges over structures in contracts (Part C1), arrays of structures in static storage and as members (Part C2) (2026-11-26): three parts landed in three commits, the full check green on the last, closed at the split point after Part C2; one question open for the owner (343, fail closed)

**What a reader needs first.** Every C `restrict` parameter is now rejected, so the wrong programs that
`restrict` let verify (on `main` every form the first session's audits found; on that session's branch one)
no longer verify. Three of the continuation plan's six parts are built: R, C1 and C2. Parts D, E and F run in
a further session from the same plan. **One question is open for the owner (343, fail closed), and the
owner's review is asked for the questions settled with the advisor (338 to 342 and 344 to 347).** None of
this session's four audit reads (rounds R, C1 and C2, and a short second read of Part C2's final snapshot)
found a wrong program that verifies; what the audits of Part C2 did not read is said under "Audits".

Plan: `.claude/plans/m4-5e1e2b-arrays-of-structures.md` (the continuation plan; executed from its section 5,
without re-planning). **A noted reading:** the owner's first message was only "M4.5e1e2", typed in the main
checkout, where `main` (`f07d80a`) holds only the first plan and the older handoff. The first plan could not
run as written (its branch and worktree existed, Parts A and B were built), and the newest handoff, on this
branch, said the continuation runs next from the continuation plan; the advisor agreed. Record 0064 (accepted
for the items built: 2 to 6 and 10 to 12 for local arrays of structures, 16, 17's rules through Part C2, 19
rewritten, 5 as amended, 7 to 9 and the range, global and member parts of 10 and 12; items 13, 14 and 15
"decided, built by the next session"). Read `internal/handoff.md` next: §1, §2, §8.
**Base:** `ae622e5`, the tip of `m4.5e1e2b-arrays-of-structures` at the start (it contains `aebf158`, the
commit the first session's full check passed on; `main` = `origin/main` = `f07d80a` are ancestors, so no merge
was needed). Session from 2026-11-26 02:25 UTC to about 11:15 UTC (8 h 50). The owner gave no overnight rule; every question
went to the Fable advisor first.

**The session closed at split point 2** (question 310's 12-hour rule; the advisor, checkpoint 3 at Commit 2):
after Commit 2 the sum for Part C2 (its lowering, encoder half and tests existed; left were the joined proofs, the
flips and first run, round C1's fixes, audit round C2 with its fixes, and the two-hour close-out) came to
about 10 hours, so Part C2 went on. Part D was not begun: its build, its audit round and the close-out after
Commit 3 (at 7 h 39) would have passed twelve hours. Parts D, E and F run from the same plan in a further
session (handoff §2, step 4).

**Done** (commits on `m4.5e1e2b-arrays-of-structures`, local, no Claude trailer):
- **Commit 1 `a8c65ff`** (04:08 UTC, 1 h 43; Part R). Record 0064 amended before any code (its header, items
  1, 5, 7, 12, 13, 16 and 19; item 19 rewritten). Every pointer parameter qualified `restrict` is rejected
  (`unsupported_annotation` at the function's head, naming each parameter; question 326): the encoder's
  `restrict_hazard`/`restrict_backed` pass and its two later passes are gone, `vir.cpp` lost its 781 lines of
  `restrict` rules, `check.cpp` refuses any parameter with the flag (three backstop self-test cases; 22
  lowering and 38 rejection self-test cases retired). The 17 test files the owner approved (8 positives
  became rejection pins under `tests/memory/negative/`, their 9 error twins retired), a new positive with the
  keyword dropped and its twin, pins for every spelling and placement. Question 322: a truth test or null
  comparison of any address formed by an index is rejected (`array_address_null_test`). Questions 325 and
  339: probe P84 took its middle branch, and in C a lone zero that elides braces must be an integer literal
  where the IL can tell (`'\0'`, an enumerator, `false`, `NULL`, `(int)0` rejected; `(0)` not told, stays).
  `scripts/box_lane.sh` refuses to run without its two lane variables (question 336).
- **Commit 2 `d3d86d0`** (07:17 UTC, 4 h 52; Part C1 and the fixes of audit round R). `owns_range(p, n)` and
  `reads_range(p, n)` on a pointer parameter to a verified structure: rows per leaf path, the length facts of
  the range's slots, calls and returns with the pairing rows per pair of leaf paths by slots, element reads
  `p[e].x` in `requires`, `ensures`, `decreases` and under `old`, loops with one range place per leaf path
  and `p[e].x` in invariants, `owning(n)`, `ref_to_uninit` and `must_init` over such a range. Round R's
  fixes: a function without a body is rejected when any of its declarations gives a parameter `restrict`
  (question 341); in C a truth test whose operand is an index address that the front end folded to a constant
  is rejected. The pages ("Ranges of structures"; the tutorial section "Arrays in aggregates", question 319).
- **Commit 3 `46f34fb`** (10:04 UTC, 7 h 39; Part C2 and the fixes of audit rounds C1 and C2). Arrays of structures as
  globals and `static` locals (`program_start`'s row groups per leaf path with listed values, the zero rest
  and the unknown form; `owns(g)` and `reads(g)` as whole-array atoms over every leaf; both escape
  directions; `struct P *pg = g;`), a member that is an array of structures in every storage of its structure
  (`owns(*p)`, `owns(p->v)`, `owns(gt.v)`; the alias `f(&t, &t.v[1])` reported at the call), the contract reads
  `g[e].x`, `p->v[e].x`, `gt.v[e].x` and `r->v[e].x` (question 344), loops, and the value view (whole-structure
  copies, by-value parameters and results, compound literals, up to 7 values). Round C1's fixes: `apply_frame`
  and `contract_read` skip every site whose pointee is a structure (a `pure` application or a quantifier
  beside a range over structures is framed by scalar sites only; no exit 3), the lint's scope-exit rule no
  longer switched off by an `assert false`, the pointer-to-structure leaf row admitted and tested. Round C2's
  fix: the length quantifier of a member array of structures takes a binder that is not free in its slot
  expression (record 0064 item 3; thirteen shapes that were exit 3 verify). The length facts of a global
  array of structures are clauses of their own. The pages (array members, global arrays, the not-yet lists).

**Not done** (the next session's, from the same plan; nothing was cut): Part D (the leftovers of a structure
with an array member: heap storage, an `owning` or marked member beside one, markers, heap-read contract
places, partnerless and written atoms: questions 292 to 294 and 306), Part E (arrays of arrays, array-of-rows
parameters in the three spellings and, by P84 (b), the typedef'd ones, `v[i].a[j]`, depth 3), Part F
(quantifiers and `pure` applications over members); the tutorial's matrix example (with Part E). Each is still
rejected with a diagnostic and a pin that names its part.

**What the probe decided** (record 0064's table): P84 (a), the lone zero's spelling: the middle branch, "the
IL tells some" (question 339). P84 (b), typedef'd row parameters: the first branch (one pointer over one
`tk_typeref` over the row's array entry, the bound as written): Part E accepts them. No cut was taken.

**Audits** (Fable at `xhigh`, a fresh instance per round; reports
`build/m4.5e1e2b/auditor-report-{R,C1,C2,C2b}.md`, probes copied to `build/m4.5e1e2b/audit-{R,C1,C2,C2b}/`):
- **Round R** on Part R's joined snapshot (`c263b457938f`; 46 minutes): no wrong program verifies, no blocking
  finding. Should-fix 1, a called function without a body whose declarations disagree on `restrict` was
  accepted (not unsound; fixed in Commit 2, question 341); 2, in C `if (&ga[2])` on a static-storage array was
  accepted (fixed in Commit 2, wider than the one form: every truth test whose operand is the folded
  address); 3, question 339's rule had no page statement (Commit 1) and one call site without a test row
  (Commit 2). Notes taken: the rule of question 322 is over-strict on macro-written tests and `(int *)0` in C
  (question 342, listed for M8); wording in the pages and the record. Not taken: the message's tail cites
  decision 0037 as well as 0064 (it is fixed per tag in `diagnostics.def` for every
  `unsupported_annotation`).
- **Round C1** on Part C1's joined snapshot (`0513b158e4e2`; 88 minutes, the box at load 80 to 99; about 575
  runs, a false `check` injected at 135 points of the positives): no wrong program verifies, no blocking
  finding; round R's fixes hold. Should-fix 1, **the sequencing rows of a call that holds a range over
  structures have no other check behind them** (with the rows made `assert true` in the kept Viper, Silicon
  verified `wr(v, 2) + wr(&v[1], 2)`); as built they are right in 23 probes; pinned at C level in Commit 2,
  and the record says these rows, unlike the pairing rows of a call, a return and a loop, are not checked
  again. 2, a `pure` application beside a range over structures was framed as an interval of scalar cells
  (`apply_frame`; fail closed, an exit 3 beside the right diagnostics; fixed in Commit 3). 3, a quantifier
  beside such a range was accepted with a body the lint refuses (fixed in Commit 3). 4, a range over a
  structure with a pointer-to-structure member was accepted and right but untested, the lint refusing its
  Viper (lint rule, cases and tests in Commit 3). 5, `f__wf`'s ghosts per leaf and slot had no must-fail
  test (pinned in Commit 2). 6, the lint's scope-exit rule was switched off by any `assert false` in the
  method (fixed in Commit 3). 7, an unlisted cost (listed). Notes: a contract that owns one object twice
  verifies vacuously (not of this diff, also on the base; listed).
- **Round C2** on Part C2's snapshot (`a044aff629b8`): 50 minutes of probing, 387 probes: no wrong program verifies,
  no blocking finding. **It rested on its probes and on the functions it names; it did not read most of the
  `lower.cpp` and `encode.cpp` diff, the consumers of `leaf_arrays`, the lint diff, or replay mutants of the
  sequencing rows.** Should-fix 1, an exit 3 on accepted programs: the length quantifier of a member array of
  structures reused the binder `k` under another quantifier, and only Silicon's duplicate-identifier check
  stood behind the capture (13 shapes; fixed in Commit 3, the rule written into record 0064 item 3 before
  the code). 2, the lint at that snapshot refused Part C2's rows (the lint round on the next joined build had
  already cleared it). 3, seven Part C2 rules had no C-level test (17 files written, in Commit 3). 4, two
  rejections the record did not list, one against its own words (question 346: listed and pinned). Notes: a
  list of about a thousand structure elements gets no verdict (Silicon's stack; exit 3; listed); costs
  (listed); three messages not in source terms (listed); three page statements (corrected). Seven of its
  probes got no verdict (timeouts and that crash): not checked, not passes.
- **Round C2b**, a second read on the final snapshot (`588967a05edf`), of what no auditor had read: the `src/`
  diff between the two snapshots (the clause split of the global facts' length quantifiers, made after round
  C2's snapshot; the binder fix), the consumers of `leaf_arrays` and the site round C2 named, probes of the
  thirteen newly accepted shapes, the lint diff if time allowed: 37 minutes, 78 runs:
  **no wrong program verifies.** The clause split states the same conjunction at its three callers, nothing
  dropped or doubled, discharged and not vacuous; the binder fix is structural and right (the nested row read
  by hand; `k2` unreachable today and without a case); the consumers of `leaf_arrays` and
  `exempt_owned_fields` pass by reading their branches; nothing in the lint lines it read admits a permission
  or a heap read. Its findings: 1, **which it labelled "blocking for the full check" (fail closed, not
  unsoundness):** round C2's probe q05 took 13 s at the first snapshot and gets no verdict at the final one,
  and seven probes of that family got no verdict in 560 s. The orchestrator's bisect put it on the clause
  split (question 347). What answers the label: every test and self-test case whose Viper the split changes
  had already run on a split build (15 test files, none over 60 s; 39 cases in 17 s), and the full check
  below ran on Commit 3. 2 and 3, `fp(gt.v[1])` is accepted in C++ where the record said rejected: right in
  twenty probes (question 346 amended; the pin split, a C++ pair added). 4, the lint still refuses four
  classes of accepted shapes that no suite test holds (handoff §3). 5 and 6, a stale name in a comment and
  the binder rule's unexercised second step (not acted on: Commit 3's `src/` is the snapshot it read).
- **What no auditor read:** the body of Part C2's diff against Commit 2 in `lower.cpp` and `encode.cpp`
  beyond the functions the two reports name (both rounds rested on probes there), the self-test cases, round
  C1's lint rules for its findings 4 and 6, the lint lines written after round C2b's snapshot, and mutants
  of the sequencing rows (the il-analyst's replay stands alone). Seven probes of round C2 and seven of round
  C2b got no verdict and are not checked.

**Questions** (handoff §8):
- **Open for the owner: 343** (fail closed: an internal error, exit 3, never "verified"). A structure pointer
  formed by indexing a pointer base outside the slots the function's atoms cover (`&p[n]` one past a range of
  structures, `&p[1]` under `owns(*p)`), passed to a function with no atom on that parameter or returned,
  stops at Silicon's own check. It predates this session (the first session's Part B accepted `&p[i]` on a
  structure pointer; the same exit 3 on Commit 1's build). The fix, one explicit `permission` row for
  structure-pointer arguments and results, changes existing programs' Viper, which is the owner's to approve.
  Listed in record 0064 item 7 and on the page; the planned pin `range_struct_past_end_false_alarm.c` did not
  land (the harness cannot pin an exit 3).
- **Settled with the advisor, for review:** 338 (a `restrict` declaration without a body is reported at its
  first call, or at the declaration when its signature is read early; an uncalled, unforced one is not
  reported); 339 (the lone zero: the spellings the IL tells are rejected in C, the parenthesized ones
  listed for M8); 340 (a staging pin's `unsupported_annotation @ L52` row retired one part early, in Part C1);
  341 (a function without a body is rejected if any declaration has the qualifier); 342 (question 322's rule
  also rejects macro-written tests and `&a[1] == (int *)0` in C, which no compiler flags: listed as
  over-strict for M8); 344 (the contract reads `g[e].x` and `p->v[e].x` are built in Part C2); 345 (`f__wf`
  has no tie between the pre- and post-state ghosts of a member array's element: record 0063 item 7's listed
  false alarm, extended to arrays of structures); 346 (two rejections
  Part C2's build makes that the record did not list, one against its own words: `cerastes_old(gt.v[1].x)`,
  and `fp(gt.v[1])`, an element of a global structure's member array passed by value, which is rejected in C
  only: C++ accepts it, found by the joined proof and probed twenty ways by round C2b; listed in record 0064
  items 7 and 12 and on the page, pinned, the C++ path with a test pair; which session lifts them is not
  decided); **347 (this session's own change cost a program its verdict:** the global facts' length
  quantifiers were given clauses of their own to fit the lint's shape, and a function other than `main` that
  holds `reads(gt)` of a global structure with a member array of structures, calls another and checks one
  leaf went from 17 s to no verdict in 200 s; the two-call chain over such a global had no verdict before
  the split either; the split was kept so that Commit 3's `src/` is the snapshot the second read took, and
  the cost is listed; the alternative for a later session is the fused form with a lint rule).
- **Settled by the orchestrator inside the plan's rules, listed for review:** which plan to take (above);
  the dropped fifth row of `negative/struct_member_array_struct_wf_division.c` (a cross-state row that cannot
  be told from question 345's false alarm until the tie exists: the advisor took the test-mutator's
  recommendation, a twin when M4.8 builds the tie);
  round R's and round C1's code fixes were made on top of the next part's work and landed one commit later
  (Commits 2 and 3), with the record and pages saying at each commit what its build did; the lowering was
  changed to give the length facts of a global array of structures clauses of their own (the spike fixture's
  shape) rather than widening the lint.

**Programs accepted before this session that are now rejected** (each fail closed; none held by an existing
test or doc example beyond the 17 approved files; record 0064 items 5, 16 and 19): every function with a
`restrict` parameter (drop the keyword); a truth test or null comparison of `&a[i]`, `&p[i]`, `&s.a[i]`,
`&p->a[i]` on arrays of scalars, `&p[0]` and macro-written forms included; in C, a lone zero that elides
braces and is not an integer literal (`{'\0'}`, `{false}`, `{NULL}`, an enumerator), where item 16's
predicate applies; in C, `if (&ga[2])` and `x && &ga[2]` on a static-storage array; a called function
without a body one of whose declarations has `restrict`.

**Deviations from the plan, for confirmation:**
- Briefings were files (`build/m4.5e1e2b/briefings/`: the plan's section 12 text, extracted verbatim, plus an
  "Additions" block per run: session facts, gates, which task of the role was live).
- Instances: edg-backend 1 to 5, il-analyst R, R2 to R6 (Part C1's lowering took five instances, one slice
  each), C2 (all of Part C2's lowering in one), C2b (round C1's code fixes and the clause split), C2c (round
  C2's binder fix); test-mutator 1 to 7; a fresh auditor per round.
- **A fourth audit read that the plan does not have, round C2b** (the advisor's condition for Commit 3): the
  clause split of the global facts was made after round C2's snapshot, and round C2 had said what it did not
  read, so a fresh auditor read the final snapshot's difference, with the binder fix, for 37 minutes before
  the commit. Commit 3's `src/` is exactly the snapshot it read.
- **The lint was changed after the last audit snapshot** (the nested-quantifier rule and three placements,
  test-mutator report 13b), and no auditor read those lines; a lint rule that admits too much removes a
  backstop, it cannot make the verifier accept a program. An agent was replaced when its context was largely used (200k to 670k tokens).
- **Interim joined proofs:** the plan has one joined proof per part. Part C1 had an interim one on slices 1 to
  4 (`refs/tmp/e1e2b-j2a`) while slice 5 was built, and Part C2 one before round C1's fixes
  (`refs/tmp/e1e2b-j3a`), so that the tests' first run, the flips and the lint round did not wait; each final
  joined proof repeated only what changed.
- Part C2's lowering was started when round C1's audit snapshot was cut (the plan's order allows it), before
  the split-point decision at Commit 2; the advisor's projection then held.
- Round C1's fixes were not made in a separate worktree (the advisor's first arrangement, for a blocking
  finding): nothing was blocking, so they were made on the working tree after Part C2's lowering was frozen.
- Runs on the Mac: P84's first leg (single-file front-end probes), then once in lane `il`: identical; the docs
  builds; the lint's and the harness's self-tests and `--check-headers` as the test-mutators wrote.
- **Runs outside the plan's test budget:** question 338's 20 single-file probes and the audit's two more
  spellings (lane `edg`); the compile-only measurement of clang's and GCC's cells for the lone zero (92
  shapes); one probe by the orchestrator for `owning(n)` beside the same range (lane `edg`) and eight for
  question 343 (lanes `work` and `audit`); edg-backend's probe sets for its fixes (254 runs before and after)
  and for the value view; the lint run early over slice 2b's output; the il-analysts' in-lane mutants and
  mutant replays of the sequencing rows; the auditors' probes (round C2b made 78 runs where its briefing
  said about 40); the il-analyst's rerun of the audit's 13 probes on its binder fix; **four runs by the
  orchestrator to bisect round C2b's timing finding** (two probes in lanes `edg` and `il`, question 347);
  three `cerastes verify` runs on the Mac by a test-mutator for a pin's row count; mutants of the lint's new
  rules by the test-mutators (12 and about 6; the plan's rule 6 has no separate mutant sweeps of the lint);
  the final lint over the whole regeneration (6,928 programs; it is the preflight's "regen and lint over
  it", run by the test-mutator on edg-backend's regeneration); docs builds beyond the preflight's one: after
  Part C1's pages, after Part C2's (two, the first stopped on an AsciiDoc warning), after round C2's page
  corrections, and the last, after round C2b's, which stands for the preflight's.
- One planned pin did not land (question 343). One planned twin was dropped to keep Part C2 at its cap
  (`struct_member_array_struct_owns_wrong_post.c`). Three of the audit's candidate probes were not made
  tests (d13, c15's compound-literal argument, f03's other rows).
- **Two findings of round C2b were not acted on in code**, so that Commit 3's `src/` stayed the audited
  snapshot: a stale function name in a comment (`lower.cpp` 8917, `global_fact_quantifiers` for
  `global_fact_clauses`), and the binder rule's second step (`k1` to `k2`), which no program can reach
  today and no case exercises. Both are in the handoff for the next session.
- Not done from the plan's section 10: `error-messages.adoc` has no text for the `restrict` rejection (no page
  of that file lists rejections; `memory.adoc` and `verifying.adoc` have it).
- **Pages that were stale at Commit 2 and corrected in Commit 3:** `memory.adoc`'s arrays not-yet list still
  said "a range of structures" and its last list "ranges in contracts", and `verifying.adoc` and
  `overview.adoc` said arrays of structures were supported only as locals.
- A stray untracked directory `--help/` at the worktree root (a self-test output written by an agent that
  passed `--help` to `cerastes-vir-selftest`) was removed by the orchestrator. A first attempt at composing
  the snapshot `e1e2b-j3d` failed and left a wrong ref, overwritten at once; every snapshot and commit tree
  was checked tree by tree before use.
- The state log's hand-written times between 05:20 and 05:37 UTC run up to five minutes ahead of the clock.

**Expectation corrections against the plan:** the accepted `restrict` self-test cases to retire were 22, not
23; `restrict2` of the first session's reports is under `s0/`; Part R's new test files were 10 against a cap
of 9 (the tenth by the advisor, for the keyword's spellings).

**Corrections of new tests after their first run** (question 160; no verdict changed; the test-mutators'
reports 4, 7, 8, 10 and 11): Part C1: `range_struct_forward_one_more.c` and
`range_struct_pairings_forwarded.c` (two `permission` rows at the call, the shape or place row and the leaf's
row, both the twin's reason: two rows registered); `range_struct_loop.c` and its two twins (EDG's #550 in
C++: a real use added); the false-alarm pin's helper; two folded-address files corrected before their run.
Part C2: three files of the first run (registration errors: B1 to B3 of test-mutator report 11); the control
`struct_member_array_struct_wf.c` reworded and its twin's rows re-registered, a fifth row dropped (question
345: the old control is now the pin of the false alarm); `range_struct_pure_beside.c` and its wrong twin (a
`(void)m;` in a `pure` body; rows L37, L46, L54 became L36, L45, L53); `range_struct_forall_beside_unframed.c`
(a use after the failing row drew `lifetime` rows; removed). One new self-test twin of the binder fix gained
its `lifetime` row after its first run. Round C2's tests: the 21 files
written from the auditor's probes passed at their first run; the question-346 pin was split after the joined
proof showed C++ accepts its second row (an unmeasured registration), and its remaining file gained a
front-end warning row (EDG's #550).

**VIR self-test cases re-expected** (the plan's Flips S; hand-built cases): Part R: 22 accepted and 38
rejected `restrict` cases and `check_restrict_reach` replaced by three backstop rejection cases. Part C1:
`range_rej_struct_pointee` became the lowering case `range_struct_pointee`;
`e1e2a_guard_range_over_structures` and `e1e2a_guard_range_over_structures_in_a_call` went from guard cases
to lowering cases. Part C2: `arr_store_rej_member_array_of_structs` removed;
`arr_rej_array_global_of_structs` became `..._missing_leaves`; `e1e2a_guard_object_view` and `_value_view`
became positives.

**Limits listed by this session** (record 0064): question 343's exit 3 (item 7); question 346's two
rejections (`cerastes_old(gt.v[e].x)`; in C, `fp(gt.v[1])` of a global structure); **question 347: this
session's own clause split of the global facts cost one measured program its verdict** (17 s before, none in
200 s after; item 12); an initializer list of about a thousand structure elements gets no verdict (Silicon's
stack; item 9); three messages not in source terms (item 9); four classes of accepted shapes the lint still
refuses, none held by a test (round C2b, finding 4); the sequencing rows of a
call over a range, a member array or a global array of structures have nothing behind them and are pinned by
tests (item 11); a `must_init` filler over a range of structures that fills with a loop is a false alarm
until Part F; `f__wf` does not tie a member array's element across states (question 345); a contract cannot
read an element of a structure passed by value (M4.5f3); costs, each a timeout risk and never a proof: two
`reads_range` over one structure type in a function with a loop, and each structure local with a member array
of structures (about 45 s more per local on the loaded box); loop reach by field (a loop writing `t.v[i].x`
loses `v[k].y` and the structure's own `int` cells); question 322's over-strict family (342); the one form of
`restrict` still accepted, the type of a cast or of a compound literal (M8); a contract that owns one object
twice verifies vacuously (not of this session).

**Checks** (every test run on the box, lanes `~/cerastes-e1e2/{base,il,edg,work,audit}`; the box was under
other users' load, 50 to 100, for most of the session, so times are upper bounds):
- **Baseline** at `ae622e5` (lane `base`): 6,750 programs regenerated, 0 missing; VIR self-test 2,512 lowering
  and 449 rejection cases.
- **Neutrality at every joined proof: no existing program's `.vpr` or `.map` changed, but for the 17 files of
  Flips R.** Part R (`refs/tmp/e1e2b-j1`): 6,733 of 6,750 identical, the other 17 exactly Flips R's, 10 new.
  Part C1 (`e1e2b-j2`): 6,743 of 6,743 identical to Part R's, 70 new. Part C2 (`e1e2b-j3d`): 6,813 of 6,813
  identical to Part C1's, 73 new. The final build (`e1e2b-j3e`, Commit 3's `src/`, with
  round C2's binder fix): 6,928 programs regenerated, 0 missing; the 6,813 of Commit 2 identical; against
  `e1e2b-j3d` 6,882 of 6,886 identical, the 4 others being two new tests whose sources were rewritten
  (question 345); 115 new since Commit 2.
- **The VIR self-test:** 2,490 lowering and 414 rejection cases after Part R; 2,616 and 422 after Part C1;
  2,730 and 423 after Part C2 (`VIR_CASES_MIN` 2739; after round C2's fix 2,739 and 423); 0 problems each time; every case shared with the
  previous part byte-identical.
- **`rejections.py --base`** at each joined proof: no existing file's tag or line moved (the 34 `restrict`
  pins row by row against Step 0's R4); only the flips the plan lists and question 340's one row.
- **New tests, each run once on the box:** Part R 18 files (10 new, 8 flipped): 37 passed. Part C1 and round
  R's pins: 46 files on the interim build: 134 passed after corrections; 6 more on the final build; 5 pins
  for round C1 on Commit 2's build. Part C2: 40 files on the interim build
  (`e1e2b-j3a`): 36 passed at once, three registration errors corrected, the wf control failed (question 345);
  29 files on the joined build `e1e2b-j3d` (the 15 whose Viper the clause split changed, the reworded control
  and its twins, 8 new pins): all pass after three corrections of new files, the slowest 56 s; 24 files on
  the final build `e1e2b-j3e` (round C2's): all pass, the slowest 47 s, one pin split (question 346). No new
  test over 60 s on a build that stands; Commit 2's `range_struct_seq_unsequenced.c` took 72 s for two modes
  at box load 80.
- **Test counts against the plan's cap of 160:** 129 new test and example files by `git diff` against the base
  (Commit 1: 10, and 8 of the approved files moved to `negative/`; Commit 2: 51, among them 4 doc examples, round
  R's 10 pins and round C1's 5; Commit 3: 68, of them 26 planned for Part C2, 18 pins for round C1's findings
  in Part C2's storages, 24 for round C2 and C2b). 13 files removed: Flips R's 9 error twins and four staging
  pins.
- **Preflight:** once, after round C2's fixes, on the final build (lane `work`; the docs on the Mac):
  `format.sh --check` (93 files conform); `py_compile`; `run_tests.py --check-headers` (6,421 of 6,421);
  `mutate.py --validate` (374 tests, 12,864 mutants generated: this is the validation of the mutant
  generator, not a mutation run); `test_run_tests.py` 1,287 OK; `test_lint_viper.py` 1,110 OK; the lint over
  the whole regeneration: 6,928 files clean; the VIR self-test: 2,739 + 423, 0 problems, its output clean in
  the lint; the session's `e1e2c*` cases through `run_tests.py --expect-passed 244`: 244 passed, 123 s (Part
  R's three backstop cases are rejection cases and have no Silicon run); `walker_oracle.py` (136 goldens, 0
  failed) and the tree checks; `rejections.py --base`: no existing row moved; `build_docs.sh` exit 0;
  `check_doc_examples.py` on the source (26 pages, 417 blocks, 387 examples, 0 problems) and `--html` (27
  files, 0 problems). The three files of the pin split ran after it and are in the check.
- **`CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` on the box** (`~/cerastes-s8/p-ubuntu-26.04`):
  **green, exit 0, run once**, on Commit 3 `46f34fb`, the last commit that touches anything but `internal/` and
  `plan.md`: 10:04:57 to 11:00:48 UTC (56 minutes; box load about 3). Tests: 17,172 passed, 0 failed, 10
  skipped; native-ok runs clang 3,203, GCC 3,203. The Viper lint clean; the VIR self-test 2,739 passed, 0
  failed, its output clean in the lint; the walker oracle 136 goldens, 0 failed; the docs and
  `check_doc_examples --html`; the EDG and vlibc trees clean; `check.sh: all green`, `long tier: ran`. The
  log is `build/m4.5e1e2b/check-c3-46f34fb.log` (and `check-e1e2b-c3.log` in the lane).
- `src/` and `include/` against the base, without the self-test cases: Part R +102 -965 (the plan: about 150
  added, 700 deleted); Part C1 with round R's fixes +1,348 -233 (the plan: 800 to 1,400); Part C2 with round
  C1's fixes +1,692 -341 (`encode.cpp` +736 -123, `lower.cpp` +811 -187, `check.cpp` +118 -31, `vir.cpp` +20, `vir.h` +7; it includes question 344's contract reads and both rounds' fixes) (the plan: 700 to 1,200).
- Hours per step: Step 0 done at 0 h 20; Commit 1 at 1 h 43; Part C1's lowering at 2 h 46; its final joined
  proof at 3 h 12; audit round C1 back at 4 h 32; Commit 2 at 4 h 52; Part C2's lowering at 4 h 18 (built
  beside the audit); its audit snapshot at 5 h 25; audit round C2 back at 6 h 17; the binder fix proved at
  6 h 42; the final joined snapshot at 6 h 43; round C2b back at 7 h 21; Commit 3 at 7 h 39; the box check
  started at 7 h 40; the check green at 8 h 36 (56 minutes on a quiet box); the status entry, the handoff and `plan.md` written while it ran.

**First step for the next session:** the owner merges and pushes (handoff §2, step 1: the branch
fast-forwards `main`, which still accepts the unsound `restrict` forms) and answers question 343 if convenient
(it blocks nothing). Then the next session: `claude --agent orchestrator --effort high`, first message
`M4.5e1e2 continuation, from Part D (.claude/plans/m4-5e1e2b-arrays-of-structures.md)`. It runs from the same
plan without a planning: a new branch `m4.5e1e2c-arrays-of-structures` from this branch's tip, the worktree
`.claude/worktrees/m4.5e1e2c`, `build/m4.5e1e2c/`, the same box lanes; the plan's Step 0 again on the new base
(the worktree's submodules and bootstrap, this session's reports copied from
`.claude/worktrees/m4.5e1e2b/build/m4.5e1e2b/`, the lanes, the baseline, R1 and R3; no P84, no P83, no S48);
then Part D's line in the plan's section 7, "Order of work". Its questions start at 348.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
