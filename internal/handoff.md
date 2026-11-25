# Handoff after M4.5e1e2's first session (2026-11-25) — the continuation runs next, from Part C of the same plan

The next session reads this file after `CLAUDE.md` and its plan. `status.md` has the formal entry of the last finished
session (M4.5e1e2's first: Parts A and B of six). This file adds what a cold start would otherwise have to rediscover,
and holds only what is live (owner, 2026-10-26).

**What the next session needs here:** §1, §2, §3 "From M4.5e1e2's first session", the rules of §4 and §6, and §8
(four questions open for the owner, 326 first; 321–324, 327 and 328 settled with the advisor, for review).

It was consolidated at M4.5d2b2b's planning (2026-11-09), at M4.5d2b2c's (2026-11-10), at M4.5e1's and at
M4.5e1b's (both 2026-11-11), at M4.5e1c's (2026-11-23), at M4.5e1d's (2026-11-24), at M4.5e1e's and at M4.5e1e2's
(both 2026-11-25), each at the owner's request. **The text before M4.5e1e2's first session is `git show
f07d80a:internal/handoff.md`**; the ones before it are named there. Each note below keeps its origin in brackets.
Session plans approved from M4.5b1a on are in `.claude/plans/`; the notes a session consumes move into its plan's
"Inputs".

## 1. State

- **Wrong programs verify on `main` today (found by audit rounds 1 and 2 of M4.5e1e2; question 326).** C's
  `restrict` promises that an object modified through the parameter is reached only through pointers based on it;
  cerastes accepted `restrict` whenever the contract owned the object, and the function could still reach the object
  by another route: `int g; int f(int *restrict p)` with `owns(*p)`, whose body tests `p == &g`, writes `*p`, writes
  `g` and returns `*p`, verified with an `ensures` true only of the aliased value, while `gcc -O2` returns another
  (C23 6.7.3.1p4: undefined). Round 1 and two probe rounds found the routes through a global's name, a callee that
  owns the global, and a pointer a callee hands back; round 2 then found two more that the first fix missed (the
  global named inside a callee's own body; a range atom whose count can be 0, which excludes nothing). **The branch
  `m4.5e1e2-arrays-of-structures` rejects every form found up to its last audit read** (record 0064 item 19: fail
  closed, over-strict for several shapes C allows). **That last read found one more, which is NOT fixed and still
  verifies on the branch:** `int f(int *restrict p, int *q, int n)` with `requires(owns_range(p, n) &&
  owns_range(q, n) && owns(g) && p == &g && q == &g)`, body `p[0] = 1; q[0] = 2; return p[0];`, called with `n ==
  0`: both ranges are empty, the permission comes from `owns(g)`, which the body never names, and the function
  verifies `r == 2` where `gcc -O2` and `clang -O2` return 1 (item 19, "KNOWN, OPEN AND UNSOUND"; the advisor's
  stop rule for the session: no third fix cycle). **The rule is an enumeration and is not argued complete: three
  audit reads in one day each found a form the rule then in force missed.** `main` has all of them until the
  branch is merged. The complete rule is the owner's (§8, question 326).
- **M4.5e1e2's first session** (2026-11-25; record 0064, accepted for the items built) is on the branch
  `m4.5e1e2-arrays-of-structures` (worktree `.claude/worktrees/m4.5e1e2`), branched from `main` = `f07d80a` (which `origin/main` also holds since the owner's push during the session):
  Commit 1 `571fe59` (the record, the spike), Commit 2 `b9e1ab9` (Parts A and B, the `restrict` rule, the pages),
  `aebf158` (one self-test case corrected after the first run of the full check) and
  the closing commit. It landed **Part A** (leaf paths, the traps closed) and **Part B** (automatic local arrays of
  structures in code, and question 304's rejection as the compilers measure it). **The full check
  (`CERASTES_LONG=1`, on the box) is green at `aebf158`**, the session's last commit that touches anything but
  `internal/` and `plan.md`; its first run, at `b9e1ab9`, was red on one new self-test case, corrected in `aebf158`.
- **M4.5e1e2 is not finished: Parts C, D, E and F are the continuation's**, from the same plan
  (`.claude/plans/m4-5e1e2-arrays-of-structures.md`), with no second planning (question 310). The session closed at
  split point 1 under the 12-hour rule; nothing was cut. Every spike row has run (S48: 79 fixtures, every decision
  table on its first branch), so the continuation runs no P83 and no S48.
- **The box:** this session's lanes `~/cerastes-e1e2/{base,il,edg,work,audit}` (containers `cerastes-e1e2-<lane>`,
  image `cerastes-e1d:26.04`); `work` holds the final tree built. `scripts/box_lane.sh`'s defaults still name
  `cerastes-e1e1`: a session sets `CERASTES_BOX_DIR` and `CERASTES_BOX_PREFIX` (this session kept them in
  `build/m4.5e1e2/box.env`, sourced before every call). The box was under other users' load (20–140) for most of the
  session; times measured there are upper bounds.
- **CI:** unchanged (question 176 the owner's). **vlibc** `6e41fbe`, **vlibm** `1e6982e`, the website and **the EDG pin**
  `f0ed5525af` are unchanged.

## 2. Owner steps

1. **Merge the branch; it does not wait for question 326.** On `restrict` the branch is strictly safer than `main`:
   it rejects every unsound form found but one, and `main` accepts them all. Local `main` and
   `origin/main` are both at `f07d80a`, the branch's base, so `git merge --ff-only m4.5e1e2-arrays-of-structures` on
   `main` and a push are all it takes (never reset or rebase). Then decide 326 (§8). The full check was run on the box and is green at `aebf158` (the status entry
   has both runs).
2. **Answer or leave §8's open questions:** 320 (needed before Part E's array-of-rows parameters), 322's second
   half, 325, 326. Review the advisor's rulings 321–324, 327 and 328 and the orchestrator's settlements listed in
   `internal/status.md`.
3. **Start the continuation:** `claude --agent orchestrator --effort high`, first message `M4.5e1e2, from Part C`.
   Branch `m4.5e1e2b-arrays-of-structures`, worktree `.claude/worktrees/m4.5e1e2b`, `build/m4.5e1e2b/`, lanes
   `~/cerastes-e1e2` (reuse them: ship the new base to each lane and rebuild; take a new baseline). **Its base is
   the commit that holds this session's commits** (`main` after the merge of step 1; if `main` has not been merged
   yet, the tip of `m4.5e1e2-arrays-of-structures`): the plan's Step 0 says "the commit of `main` that holds this
   plan", which is `f07d80a` and has no Parts A and B, so the continuation checks that its base contains `aebf158`
   and **stops and says so if neither `main` nor the branch does**. It runs the plan's Step 0 again on that base
   (worktree, lanes, baseline, R1 and R3) and goes on from Part C's line in the plan's "Order of work"; audit
   rounds number on from 3. Questions start at 331.
4. **Remove, when convenient:** e1d's and e1e1's worktrees and merged branches (as before); this session's
   `.claude/worktrees/m4.5e1e2-audit`; the refs `refs/tmp/e1e2-*`. Keep `.claude/worktrees/m4.5e1e2/build/m4.5e1e2/`
   until the continuation has started: it holds every agent report, both audit reports, the auditors' probes
   (`audit1/`), the `restrict` probes (`s0/restrict/`, `restrict2/`), the compiler matrices (`s0/`) and the
   orchestrator's state log. On the box, `~/cerastes-e1d/` and `~/cerastes-e1e1/` may go.
5. **CI** (question 176): unchanged.

## 3. Open notes, by the session that consumes them

One line per note, its origin in brackets (session and audit item); the full wording is in the commit named above.
Notes that a session consumes move into its plan's "Inputs".

**From M4.5e1e2's first session** (record 0064; `build/m4.5e1e2/` in the session worktree has every report):
- **The continuation, before Part C:** read record 0064 as it stands (items 4, 5, 6, 7, 11, 12, 16 and 19 were
  revised after the plan was written; the plan's section 7 is no longer its text), `build/m4.5e1e2/
  il-analyst2-report-3.md` (Part B's lowering: `elem_leaves`, `leaf_slot`, `leaf_perm`, `array_place_hit`'s leaf
  hit, which Part A guards remain for Parts C and E) and `edg-backend-report-{4,5,6}.md` (the encoder: which
  rejections name "Part C" or "Part E" and must be lifted there).
- **Part C must build item 7's pairing rows as the record now writes them** (per pair of leaf paths, one
  `parent_of(block_of(·))` per member step; the slot row for two ranges, never the block row; a nested-slot site by
  its slot): the plan's section 7 and the fixtures `s48_c_formP*.vpr` show the superseded forms; `s48_c2_*.vpr`
  show the current ones. [audit round 1, issues 3 and 4]
- **Part C lifts three Part A guards** (`site_fields`, `element_range`, `elem_fkey`) and must then give the general
  path to the walkers they dominate: `entry_checks`, `result_splits`, `given_result_splits`, `ens_amount`,
  `exclusive_pair`, `wf_keep_facts`, `site_pointer_written`, `flag_follows`, `note_global_member_write`, `need_lb`
  (`lower.cpp`). [audit round 1, issue 9]
- **The `restrict` predicate's "reaches" is by a global's whole type** and shares `holds_cell` with
  `restrict_backed`: Part C's `g[i].x` and Part E's rows go through it; do not add a second walker. [item 19]
- **Part E:** the parameter spelling `T (*a)[M]` stays rejected until question 320 is answered (only a parameter
  whose written type is an array declarator is accepted; the front end keeps the written type in `declared_type`);
  typedef'd spellings (`row *a`, `row a[]`) must be probed before they are lifted; depth 3 is admitted (S48 (m));
  the depth-2 copy `v[i] = v[j]` took 35 s in the spike (its tests go under the budget's rule 3); the trusted
  methods' names must identify the element type exactly (`alloc_arr_row_int_3`); the single report for `int
  a[][3]` (today two rows, pinned by `array_param_reject_array_of_arrays.c`) comes with that pin's flip.
- **Tests for the continuation:** loop tests over arrays of structures use two-leaf structures and at most one or
  two arrays per function (question 328: 1 to 5 arrays of two-leaf structures in one function take 7, 11, 17, 36 and 65 s on the box, and five mixed ones timed out); several `assert false` in one vacuity twin only if
  no path passes two of them (Silicon stops a path at its first failing assert); `box_lane.sh ship` leaves
  untracked files in a lane (two retired pins survived one and cost a re-run): `git status --short` in the lane
  must be empty before a joined proof; a snapshot name must be fresh for every ship (a re-used name fails silently
  on a non-fast-forward fetch).
- **M4.8:** loop reach by field for arrays of structures (question 318; pinned
  `array_struct_loop_keep_false_alarm.c`); the cost of several arrays of structures in one function (question 328);
  an element structure with an `owning` or marked member; `uninit` on an array of aggregates (so that
  `--strict-init` can hold a local array of structures without an initializer); the three shapes the `restrict`
  rule rejects although C allows them, if the owner keeps the syntactic rule.
- **Should-fix items of the last audit read not taken:** issue 7, the encoder's diagnostic at the
  `restrict_backed` call site says the OTHER parameter's atom may be empty also where it is `p`'s own range that
  may be (`owns_range(p, n) && owns(*q)`; two ranges with counts 0 and 1): a wrong text on a correct rejection;
  `vir::restrict_pair_reason` has the right one and `check.cpp` prints it; moot if the owner takes question 326's
  option (c). In C++ an element that reads its own array (`struct P v[2] = {{1, 2}, {v[0].x, 3}};`) is rejected
  with item 16's brace-elision message although every brace is written (fail closed; the right message is "used
  in its own initializer", as C prints). Issue 9: these rules are pinned only by hand-built VIR cases, with no C
  test: item 19's clause (iv) for a structure value with a pointer leaf and for an application; the pair rule's
  rejecting readings (`n >= 0`, positivity under `||` or of another count, a constant-expression count,
  `owns_block`) and its accepted spellings `0 < n`, `n >= 1`, `1 <= n`. The `held` exemption admits any range
  atom rooted at a global without checking that its count is the array's length: unreachable today, and M4.5e2,
  which lifts ranges on part of a global array, must narrow it to the whole-array atom (note 11).
- **M8's warning sweep:** the scalar-array address tests (question 322) and the non-literal lone zero (question
  325), if the owner leaves them.
- **Should-fix items of audit round 2 not taken, with their reasons:** issue 7, the lint does not tie a scope
  exit's rows to the allocation (blanking every `exhale forall` of an array's scope exit leaves the file "clean"):
  record 0064 item 6 allows "the lint or a self-test case", the self-test pin
  (`e1e2b_scope_exits_give_what_the_allocation_gave`, a text check over the emitted Viper) stands, and the lint
  rule belongs with Part C's lint work (leaf rows in contracts and `bound_locals`), where the same grammar is
  extended; issue 8, the prefix match of the Sequencer's `struct_store_leaves` has no case with `t.v[0].x` inside
  a value stored to `t`: unreachable until Part C lifts array members of structures, which adds the case.
  Round 2's note 13 lists two more walkers Part C must give the general path (`check.cpp`'s structure-global
  leaves and `Aggregate`; the old view's users in `lower.cpp`, loud through `leaves_of`). Still without a fixture
  from item 7: a `reads` site in the nested pairings, a cell held by a fraction, a forwarded atom of the element's
  own type (the audit's probes decide them; Part C adds tests).

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
  M4.5e1e2 the plan also holds each agent's briefing, ready to send (its section 13; owner, 2026-11-25). At most
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
- **Records and plans:** a record takes the next free number when it is written (0063 is M4.5e1e1's; 0064 is the
  next, M4.5e1e2's unless M4.5e1f runs first). The approved plan is saved as `.claude/plans/<milestone>-<topic>.md` (plan.md, "Session
  protocol"); a sub-session that plans again after its spikes saves that plan the same way.
- **What a worktree takes with it** (owner, 2026-11-09, question 193): session tools and spike summaries are
  committed. `vprdiff.py` and `regen.py` were lost with d2b1's worktree and again with d2b2a's, and so was S39's
  summary. From M4.5d2b2b they are `scripts/vprdiff.py` and `scripts/regen_vpr.py`, and a spike's summary goes into
  `internal/il-inventory.md` beside its probes. e1b's rejection rerun (`r3.sh`, `widen.sh`) was lost the same way;
  from M4.5e1c it is `scripts/rejections.py`.

Finished sessions: `plan.md`'s Done table has a row each, `.claude/plans/` their plans (from M4.5b1a on),
`internal/status.md` their results and the commits it names. The warning sweep (was M4.4e) is an M8 entry in
`plan.md`.

**Pending side sessions** (none blocks M4.5e1e2):
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
(4,700–8,450 in six parts; its plan's section 1) is calibrated on those commits and on three read-only surveys of
the tree at `45d26bc`.

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
| **e1e2** (planned in full 2026-11-25, record 0064; two sessions likely, question 310) | arrays of structures in code, in contracts and in storage (`v[i].x`, `p[i].x`, ranges over structures, globals, members); the six leftovers of a structure with an array member (questions 292–294, 306, partnerless and written atoms); arrays of arrays (`int m[2][3]`, array-of-rows parameters, depth 2); quantifiers and `pure` applications over members and member arrays; question 304's rejection; the tutorial section | `.claude/plans/m4-5e1e2-arrays-of-structures.md`, complete: P83 and S48 with decision tables, a test budget, the briefings, the 12-hour rule with split points, no second planning |
| **e1f** | range call rows: the minimum read amount over a range (238), `owns_range`'s row order D (242) | section 10: planned after e1c, from its spike S45 (r); §3 "M4.5e1f" |

No sub-session plans again after its spikes any more: the owner had e1b, e1c, e1d, e1e1 and e1e2 each planned in
full (2026-11-11, 2026-11-23, 2026-11-24 and twice on 2026-11-25), each probe and spike with a table of what is
built per outcome. A plan too large for one session is run under the 12-hour rule (question 310): the parts not
begun run from the same plan in a continuation session.

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

**Four questions are open, from M4.5e1e2's first session: 326, 320, 322 (its second half) and 325; they are at the
end of this section, each with a recommended default.** Questions 310–312 were answered at M4.5e1e2's planning and
304 inline before it; 313–319 are the defaults M4.5e1e2's plan builds on and 301–309 the advisor's rulings in
M4.5e1e1, both still for the owner's review; 321–324, 327 and 328 are the advisor's rulings in M4.5e1e2's first
session, for review (at the end). An answer is written inline, as before. Answered
questions 1–246 are recorded in the decision records, the session plans, `internal/status.md` and the earlier
handoffs:
`git show 2430529:internal/handoff.md` §8 digests questions 65–187, `git show 262edb3:internal/handoff.md` has
188–193, `git show f57ee43:internal/handoff.md` §8 has 194–223 with the owner's words (224 and 225 only as
asked: their answers are below), `git show bdf3581:internal/handoff.md` §8 has 238–246 as asked, with the
owner's inline answers to 238–242, and `git show 9ab9a1e:internal/handoff.md` §8 has 247–263 as asked. Records
0055–0058 hold what each of 194–223 decided; the ones that steer M4.8 are re-filed in §3. Questions 264–272 are
M4.5e1d's planning's, 273–289 its session's, 290–300 M4.5e1e's planning's, 301–309 M4.5e1e1's and 310–319
M4.5e1e2's planning's, below.

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
265, 266 and 287 (M4.5e2, and M4.5f1 and M4.5f2 for 266), 290–309 (M4.5e1e1's plan and record 0063), 304 and 310–319 (M4.5e1e2's plan, sections 1 and 15), 226–228 and 233 (M4.5e1's plan), 232, 239, 247–252, 254,
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
  begun run from the same plan in a second session, with no second planning; nothing is cut to hold a session short.
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

**Open, for the owner:**
- **320** (fail closed either way: a rejection stays, nothing wrong verifies). `tests/memory/negative/
  array_reject_pointer_to_array.c` pins the **parameter** `int f(int (*p)[3])` as rejected (record 0059 item 1,
  "a pointer to an array: M8"). M4.5e1e2's plan lists that pin as unchanged (its section 10, Flips D) and, in the
  same plan, accepts that spelling as an array-of-rows parameter in Part E (its section 1, record 0064 item 13,
  question 315): the planning read the pin as a local. Flipping an existing `expect:` line outside the plan's
  Flips is the owner's (invariant 1), and dropping the spelling for good is a cut the plan does not list. **Until
  answered:** only a parameter whose written type is an array declarator (`int a[][3]`, `int a[2][3]`) is
  accepted; `int (*a)[3]` stays rejected with the pin untouched (the front end keeps the written type, so the
  two are told apart). **Recommended:** accept the spelling in Part E as the plan's scope says, retiring the pin
  for a positive of the same program with an error twin (the plan's Flips A treatment), and add a pin for a
  pointer to an array as a local, which stays M8's. Alternative: keep the pointer spelling rejected until M8.

- **326** (**unsound, and on `main` today**; found by audit round 1, not introduced by M4.5e1e2). C's `restrict`
  beside a global the body reaches by name verifies a program C leaves undefined:
  `int g; int f(int *restrict p)` with `owns(*p)`, whose body tests `p == &g`, writes `*p = 1`, then `g = 2`, and
  returns `*p`, verifies with `ensures r == 0 || r == 2`, while `gcc -O2` returns 1 (C23 6.7.3.1p4). Record 0037
  item 2's argument ("an access to `*p`'s object through a pointer not based on `p` cannot verify") covers other
  parameters only. M4.5e1e2 builds a fail-closed rejection for every form found but the last (its record 0064 and audit round 2
  have the rule and what it leaves); **the complete rule for `restrict` is the owner's** (see "Questions for the
  owner" at the session's close).
- **325** (not unsoundness). clang's C warns `-Wmissing-braces` at `-Wall` on a lone zero that is not an integer
  literal (`{(0)}`, `{'\0'}`, `{false}`, `{NULL}`, an enumerator) wherever its rule for `{0}` applies; cerastes
  accepts these by their value, as before. No existing test or doc example holds one. **Recommended:** leave them
  to M8's warning sweep (rejecting them needs the front end to tell a literal `0` from `(0)` or an enumerator).

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
  M4.5e1a and are left unchanged: whether to reject them is the owner's** (see "Questions for the owner" at the
  session's close, with the measured set and the count of existing files). Alternative: one rule now for every
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

The next questions start at 331.

### Questions for the owner (M4.5e1e2's first session)

Each is explained above under its number; this is the list to answer, most urgent first.

1. **326, `restrict` (unsound on `main`; on the branch every form found but the last is rejected, and that one still
   verifies).**
   Record 0064 item 19 rejects `restrict` on a parameter `p` when the function's body (i) names or takes the
   address of a global or `static` local the parameter could point into (or that holds a pointer to such an
   object), (ii) calls a function whose contract holds one, (iii) calls a function that can hand back such a
   pointer (a pointer result, a structure result with a pointer leaf, an out-parameter), or (iv) passes such a
   pointer, `p` included, to any function; (i) and (ii) are exempt when the function's own contract holds the
   whole object without a condition; and an atom counts as excluding `p`'s object only when it is sure to hold
   something (a range whose count may be 0 does not). It rejects several shapes C allows (a body that only reads
   the global; `same(p)`; `set(p, 3)` and every helper that takes the pointer; `malloc` in the body). **It is an
   enumeration of routes, and three audit reads in one day each found a form the rule then in force missed; the
   third's is NOT fixed and verifies on the branch today** (two ranges with the same count of 0 beside a global
   the contract holds and the body never names; §1 has the program; the auditor's candidate rule is to pair every
   contract atom on a reaching global with the parameter's backing atom whether or not the body names it). One
   more gap is known for the complete rule (an access based on `p` beyond `p`'s own atom, which M4.5e2's pointer
   arithmetic would open). **Decide:** (a) keep this syntactic rule until M8 models `restrict`; (b) replace
   it by a path-precise rule (every access based on `p` stays inside `p`'s atom, every other access outside it),
   which needs a design, a soundness argument in `internal/soundness/` and a session of its own; (c) **reject
   `restrict` altogether until then (recommended: it is the only option that does not rest on an enumeration;
   `restrict` is a promise to the optimizer that a verified program can drop without changing its meaning; it
   retires or re-expects the positives among the 59 test files that use it, which is the owner's under
   invariant 1)**. If (a) is kept, the argument for why the enumeration is complete is owed before M4.5e2.
2. **320, the parameter spelling `int (*a)[3]`** (fail closed either way; needed before Part E).
   `tests/memory/negative/array_reject_pointer_to_array.c` pins that parameter as rejected, and M4.5e1e2's plan lists
   the pin as unchanged while its scope accepts the spelling. **Recommended:** accept it in Part E as the plan's
   scope says, retiring the pin for a positive of the same program with an error twin, and add a pin for a pointer
   to an array as a local (M8). Alternative: keep the pointer spelling rejected until M8.
3. **322, second half: a truth test or null comparison of an element's address on an array of scalars** (not
   unsoundness). GCC warns `-Waddress` at `-Wall` on `if (&a[1])`, `&a[1] == NULL`, `&p[1] != NULL` (any index but a
   literal 0 on a pointer), `&s.a[i]`, `&p->a[i]`; clang is silent; cerastes has accepted them since M4.5e1a. This
   session rejects the same tests on arrays of structures (`array_address_null_test`). No existing test or doc
   example holds a scalar form (three passes over `tests/` and the examples). **Recommended:** reject them too, by
   the same tag, in the continuation's first part (one rule for every element address; zero existing files change).
   Alternative: leave them to M8's warning sweep.
4. **325, a lone zero that is not an integer literal** (not unsoundness). clang's C warns `-Wmissing-braces` at
   `-Wall` on `{(0)}`, `{'\0'}`, `{false}`, `{NULL}` and an enumerator wherever its rule for `{0}` applies; GCC is
   silent; cerastes accepts them by their value. No existing file holds one. **Recommended:** leave them to M8's
   warning sweep (rejecting them needs the front end to tell a literal `0` from `(0)` or an enumerator, which may
   need an EDG change: question 69). Alternative: reject every lone zero that is not the token `0`, if the IL can
   tell.
