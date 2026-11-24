# Handoff after M4.5e1b (2026-11-12) and M4.5e1c's planning (2026-11-23) — M4.5e1c (quantifiers) next

The next session reads this file after `CLAUDE.md` and its plan, **`.claude/plans/m4-5e1c-quantifiers.md`**, which
is complete: its probes and spikes have decision tables, and it assigns every task (no second planning).
`status.md` has the formal entry of the last finished session (M4.5e1b). This file adds what a cold start would
otherwise have to rediscover, and holds only what is live (owner, 2026-10-26).

**What M4.5e1c's orchestrator needs here:** §1, §2, the rules of §4, §6 and §8. Its notes from §3 moved into its
plan's section 12; §3, §4.1, §4.3, §4.4 and §5 are other sessions'.

It was consolidated at M4.5d2b2b's planning (2026-11-09), at M4.5d2b2c's (2026-11-10), at M4.5e1's and at
M4.5e1b's (both 2026-11-11) and at M4.5e1c's (2026-11-23), each at the owner's request. The history of finished
sessions, the notes that had moved into session plans and the digests of answered questions were cut, and the open
notes were re-filed under the session that consumes each. **The text before this consolidation is `git show
bdf3581:internal/handoff.md`** (with the owner's answers to 238–242 as written inline); the ones before it are
`git show 825a9ee:internal/handoff.md`, `git show f57ee43:internal/handoff.md`, `git show
262edb3:internal/handoff.md` and `git show 2430529:internal/handoff.md`, whose header names every earlier
version. Each note below keeps its origin in brackets, so its full wording can be found there. Session plans
approved from M4.5b1a on are in `.claude/plans/`; the notes a session consumes move into its plan's "Inputs".

## 1. State

- **M4.5e1b** (2026-11-12; record 0060) is done and merged: `main` = `origin/main` = `bdf3581` at this planning,
  with the owner's inline answers to questions 238–242 (`d615630`) and Dependabot's PR #35 after it. Its result
  is in `internal/status.md`. **Its branch and worktree are gone, and `build/m4.5e1b/` with them** (the agents'
  and the four audit reports, the audit scratch, `s0/r3.sh`, `s0/widen.sh`, the frozen binaries): the three
  probes the notes name are inline in §3, and M4.5e1c's Step 0 rebuilds the rejection rerun as a committed
  `scripts/rejections.py`.
- **This planning's commits** are on local `main`, not pushed: `fabcd25` (the docs lockfile, below) and the
  planning commit after it (the plan, the trimmed umbrella plan, this file, `plan.md`, `status.md`).
- **M4.5e1c** quantifiers is next: `.claude/plans/m4-5e1c-quantifiers.md` (approved 2026-11-23), planned in full
  for the ensemble, as e1b was. It also runs the spike for **M4.5e1f** (range call rows: questions 238 and 242),
  a short session of its own after it.
- **CI:** `main` went red with PR #35's merge (2026-11-23, run 45240777868): the bump nested six copies of
  `@antora/logger` in `doc/package-lock.json` again, so a docs warning no longer failed the build and
  `DocBuildWarningsTests` failed on both jobs. `fabcd25` deduplicates the lockfile (`npm dedupe`; one hoisted
  copy, `build_docs.sh` and the six tests green on the Mac). The nightlies before it were cancelled at 6 h
  (question 176 the owner's); no `profiles=all` dispatch has run (question 172).
- **The Linux host** `matt@clang4.cpp.al`: container `cerastes-s8-ubuntu-26.04`, checkout `/work/cerastes`
  (`~/cerastes-s8/p-ubuntu-26.04`), detached at e1b's box run (`c58060e`) with `e1b-check.log`; the bundle
  `/tmp/e1b.bundle` can go.
- **vlibc** `6e41fbe`, **vlibm** `1e6982e`, the website `45a3d2f` and **the EDG pin** `f0ed5525af` are unchanged.

## 2. Owner steps

1. **Push** `main` (`fabcd25` and this planning's commit), so that M4.5e1c branches from `origin/main` and the
   push's CI run shows `main` green again.
2. **Questions 247–252** (§8): each is built as its recommendation unless answered inline.
3. **CI** (question 176): unchanged (the nightly does not fit 360 minutes; the `profiles=all` dispatch is
   pending, question 172).
4. **Optional.** On the box, `/tmp/e1b.bundle`.
5. **Start M4.5e1c** with this opening prompt. It replaces `plan.md`'s general one for this session: no plan mode
   (the plan is approved and complete), and only the records the plan rests on (reading all of
   `internal/decisions/` would spend the context that the split points depend on):

   ```
   You are the orchestrator of M4.5e1c (quantifiers). Do not use plan mode: the plan is approved and complete.
   Read CLAUDE.md, .claude/plans/m4-5e1c-quantifiers.md (all of it), the M4.5e1b entry of internal/status.md,
   internal/handoff.md §1, §2, the rules of §4, §6 and §8, and decision records 0060, 0059, 0037, 0038, 0029,
   0028 (item 2), 0011 (items 4, 7 and 10), 0006 (items 8 and 9) and 0041 (item 12) (not the rest of
   internal/decisions/). Then execute the plan from its section 4. Use the ensemble exactly as its section 2 assigns it: edg-backend, il-analyst and test-mutator
   (Sonnet) and the soundness-auditor (Opus) from .claude/agents/, and the Fable advisor at the plan's five
   checkpoints. Stop and ask me at every stop rule of the plan's sections 4, 5, 7 and 11, and for any question an
   agent raises.
   ```

## 3. Open notes, by the session that consumes them

One line per note, its origin in brackets (session and audit item); the full wording is in the commit named above.
Notes that a session consumes move into its plan's "Inputs".

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
  not a pointer parameter (results, globals, loaded pointers; members: e1e); counts and guards of range atoms that read
  memory; partnerless `ensures` ranges and `ensures` ranges with another count; `owning(n)` beyond parameters; `&p[e]`
  and `p + i` in contracts. [record 0060 item 1]
- e1c: moved into its plan's section 12 (the `must_init` loop-filler pin, the full contracts of `carrays.c`, the
  search loop's progress clause).
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
- Process: e1b's session tools were not committed (a deviation from §4's rule, question 193) and are gone with its
  worktree. M4.5e1c's Step 0 writes the rejection rerun again, as `scripts/rejections.py` with a self-test, in its
  first commit.

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

**From M4.5e1a, for later sessions:**
- e1c: moved into its plan's section 12 (the fill loop and the self-write loop, pinned; a quantified invariant
  gives each a verifying sibling, and the pinned programs, which write none, fail as before).
- e1d: `lint_viper.py`'s `HEAP_READ_RE` treats a marked row's `B`, `lo`, `hi` and amount as heap-free when they hold
  no `.`; a heap-reading `pure` function application would pass. Tighten it with `pure` functions. [round 2, item 6]
- e1e: `is_global_ref` and `inv_place_disjunct` treat any `loc(GB_…, i)` as one place (wrong once a global has several
  slots); an array member decaying to a pointer is an escape of its struct local (0056 item 11). [e1 planning]
- M4.8: a pointer local the loop reassigns has no loop place (`*pk` fails as for scalars, and `pk[i]` adds `lifetime` and
  `out-of-bounds`); three rows share the message "holding something other than what it began with" and no test isolates
  `flag_follows`' row (`lower.cpp` near 17943 at `ba3034b`); `split_candidates` splits an array cell against its first
  element only. [e1a; round 3 notes 7, 8]
- M8 (the warning sweep): `_Alignof` of a scalar expression is accepted although clang warns by default
  (`-Wgnu-alignof-expression`), as before e1a; `register` without `&` stays accepted. Array designators wait for an EDG
  change that keeps the designator's expression (question 234). [round 3, item 1; round 1, B1]

**M4.5e2 Pointer arithmetic** (needs e1a and e1b; e1e for arrays of structures on the heap)
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
- M4.5e1d takes the slice an exact array sum needs (question 227): `pure` bodies as Viper functions, calls of `pure`
  functions in annotations, recursion with `decreases`. M4.5f1 keeps predicates, `fold`/`unfold` and pure functions
  over predicates; the next bullet's notes on `pure` bodies are met first by e1c (the `pure` body reader: it reads
  only a body a `forall` names, as one annotation expression, record 0061 item 4) and e1d. [e1 planning]
- Found at e1c's planning: a `requires`, `ensures` or `decreases` written on a `pure` function is neither encoded
  nor rejected today (`Encoder::run` skips every spec routine before `function_of`); e1c rejects it where a
  `forall` names the function, and e1d gives such contracts their meaning (question 252). [e1c planning]
- `pure` bodies now reject globals and `static` locals (0041 item 12) [b2a]; R1–R13 do not reach `pure`/`predicate`
  bodies, and `static_assert_pure_body.c`'s listed entry goes stale when bodies are encoded [M4.4c].
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
  `mutate.py`, `lint_viper.py`) and the Opus soundness-auditor (one round per diff plus a re-audit of blocking
  fixes; should-fix items not taken go to §3). The orchestrator is Opus and may consult a Fable advisor (owner,
  2026-11-10). **A session's plan assigns the work** (owner, 2026-11-11: the orchestrator does not decide the
  ensemble): section 2 of M4.5e1b's and of M4.5e1c's plan is the model. At most three Sonnet agents are alive at
  once. edg-backend and the test-mutator are resumed between tasks (SendMessage); the il-analyst is replaced by a
  fresh one after the spikes and wherever the plan says (`lower.cpp` is 20,479 lines), so a session may spawn five. `lower.cpp`
  and the VIR files have one holder at a time, with a commit or a snapshot between; an agent that needs the file
  while another holds it asks the orchestrator. Each report also goes to `build/<session>/<role>-report-<n>.md`.
- **While working:** single-file runs (`build/cerastes verify` directly: `run_tests.py` locks `build/vpr`); builds
  serialized (`lockf build/<session>/build.lock cmake --build build/cmake`).
- **`check.sh` once, at the end, after the last code change** (owner, 2026-11-09: "Since each run of check.sh now
  takes hours we should change our future work to run it less"; targeted runs and the neutrality regeneration
  before it), **on the x86-64 box only, never the Mac** (owner, 2026-11-09: "limit to just the linux because it's
  much faster, and doesn't lock up the local machine"): the branch as a git bundle into S8's `ubuntu-26.04`
  container, `~/cerastes-s8/p-ubuntu-26.04`, then `scripts/check.sh` there. The Mac keeps its targeted runs.
- **x86-64:** the EDG suites on the x86-64 host are their own session, but any x86-64 test run a session needs goes
  to `matt@clang4.cpp.al` automatically (owner, 2026-10-29: not Docker multi-arch on the Mac, which is only the
  fallback when the host is down).
- **Records and plans:** a record takes the next free number when it is written (0060 is M4.5e1b's; 0061 is the
  next, M4.5e1c's). The approved plan is saved as `.claude/plans/<milestone>-<topic>.md` (plan.md, "Session
  protocol"); a sub-session that plans again after its spikes saves that plan the same way.
- **What a worktree takes with it** (owner, 2026-11-09, question 193): session tools and spike summaries are
  committed. `vprdiff.py` and `regen.py` were lost with d2b1's worktree and again with d2b2a's, and so was S39's
  summary. From M4.5d2b2b they are `scripts/vprdiff.py` and `scripts/regen_vpr.py`, and a spike's summary goes into
  `internal/il-inventory.md` beside its probes. e1b's rejection rerun (`r3.sh`, `widen.sh`) was lost the same way;
  from M4.5e1c it is `scripts/rejections.py`.

Finished sessions: `plan.md`'s Done table has a row each, `.claude/plans/` their plans (from M4.5b1a on),
`internal/status.md` their results and the commits it names. The warning sweep (was M4.4e) is an M8 entry in
`plan.md`.

**Pending side sessions** (none blocks M4.5e1c):
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

**Finished: b1a to e1b** (records 0032–0060). `plan.md`'s Done table has a row each and `.claude/plans/` their
plans; the narrative of their splits is in `git show 2430529:internal/handoff.md` §4.2. Each session rejects what a
later one makes precise (fail closed). **Sizes against estimates**, for the next plans: M4.5d2b2b landed about 2,750
`src/` lines against 1,700–3,300; M4.5d2b2c about 900 lines of `lower.cpp` against 295–495 (the owner's in-session
answers, questions 206–214); M4.5d2b2d about 1,960 against 740–1,170 (the audits' fixes); M4.5e1a +1,943/−93 without self-tests (+3,457 with
them) against 2,200–3,450; M4.5e1b +2,754/−155 (+6,289 with them) against 2,050–3,050.

**Pre-approved** (owner): a session may retire or re-expect exactly the rejection pins its plan lists (its "Flips"),
and no others; a pin whose construct stays rejected keeps its `expect:` line, and only its message and `Pins:`
comment name the new session.

**M4.5e1 Arrays** (planned 2026-11-11; `.claude/plans/m4-5e1-arrays.md`; questions 226–228). Six sub-sessions
(e1f added at e1c's planning, 2026-11-23), each a session of its own with its record, twins, docs, audits and one
box `check.sh`:

| Sub-session | Scope | Plan |
|---|---|---|
| **e1a** (done 2026-11-11, record 0059) | local arrays as blocks, `a[i]`, `&a[i]`, `p[i]`, the decay, initializers, scope exits, the marked range rows, `out-of-bounds`, loops over arrays, sequencing, the `offset_of == 0` backstops, `sizeof(a)` | section 3; `internal/status.md` |
| **e1b** (done 2026-11-12, record 0060) | `owns_range`, `reads_range`, element reads in contracts, array parameters, calls and returns over ranges, `f__wf`, loops over parameter ranges, markers over ranges, `owning(n)` on a parameter, the `carrays.c` port | `.claude/plans/m4-5e1b-ranges-in-contracts.md`, complete: P79 and S44 with decision tables, no second planning |
| **e1c** (next) | `cerastes_forall` with the `pure` function's body inlined, in contracts, loop invariants and checks; max, the fill loop, `cerastes_old` in loop invariants, the full `carrays.c` port, the tutorial chapter; the spike for e1f | `.claude/plans/m4-5e1c-quantifiers.md`, complete: P80 and S45 with decision tables, no second planning |
| **e1d** | `pure` bodies as Viper functions, recursion, the exact sum | section 6: P81 and S46, then plan again |
| **e1e** | array members, arrays of structures, 2-D arrays, global and `static` arrays, array compound literals, 0024 item 6 lifted for array types | section 7: P82 and S47, then plan again |
| **e1f** | range call rows: the minimum read amount over a range (238), `owns_range`'s row order D (242) | section 10: planned after e1c, from its spike S45 (r); §3 "M4.5e1f" |

A sub-session marked "plan again" starts outside plan mode with its Step 0, Step 1, probes and spikes as its section
gives them (they change no `src/`), commits their results to `internal/il-inventory.md`, then plans the encoding
steps from them and from the as-built facts of the sub-sessions before it. e1b and e1c are not among them: the
owner had each planned in full before its spikes (2026-11-11, 2026-11-23), each probe and spike with a table of
what is built per outcome.

**M4.5e2 Pointer arithmetic** (needs e1a and e1b; may run before e1c–e1e). `p + i`, `p - i`, `p - q`, relational
comparisons within one object, `malloc(n * sizeof(T))`, `calloc(n, …)`, `realloc`; relational comparisons and
`p - q` under `pointer-comparison` (the category and the `==` exception of 0028 item 3 landed in M4.5b1b);
`out-of-bounds` and a one-past-the-end `&a[n]` are already e1a's. Port: an array insertion sort (in place of
`insertion_sort.c`). §3 "M4.5e2" has what e1's plan leaves it.

**M4.5f1 Predicates and pure functions.** Predicate bodies encoded (0011 item 4 lifted), `fold`/`unfold`,
non-recursive predicates, pure functions over predicates; e1d has already encoded `pure` bodies and their calls
(question 227). Port: `cell.c`. §3's notes on R1–R13 not reaching `pure`/`predicate` bodies and
`static_assert_pure_body.c` go stale with e1d (for `pure`) and here (for `predicate`).

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
  by il-analyst; one Opus audit. Stop rules: cut the oracle's structural checks, then tag coverage automation, then
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
Opus audit, the full box suites (baseline mismatches `cwg1835`, `cwg2881`), pin move, 0006/0007 and the upstream
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

- **What keeps a session short:** fixed scope with stop rules; the owner's decisions and pre-approved expectation
  changes asked up front, each with a concrete default; the record written while the probes' results come in, audited in
  the background; three agents with disjoint files; single-file runs during the work.
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
- **Tutorial upkeep** (0049 item 4): M4.5e1c owes the arrays chapter (and quantifiers), M4.5e1d extends it with the
  exact sum and M4.5e2 with pointer arithmetic, M4.5f2 owes predicates and a linked list, M4.6 strings, M4.7 function pointers, M5 generic templates, M6
  requirement contracts, M7 references and classes. Every session that changes a message, a category or an accepted
  construct a chapter shows updates its prose and `console` blocks: the blocks are copied from a run in
  `doc/modules/ROOT/examples/tutorial/` (`../../../../../build/cerastes verify <file>`), and only review checks them. A
  check that reruns each shown command and compares the output is possible later work (0049, Consequences). [S6]

## 7. Acceptance

Each session's acceptance commands are in its plan (`.claude/plans/`); the results of finished sessions are in
`internal/status.md` (the latest entry) and the commits its history names.

## 8. Questions for the owner

Each open question has a recommended answer. Answer inline, as before. Answered questions 1–246 are recorded in the
decision records, the session plans, `internal/status.md` and the earlier handoffs:
`git show 2430529:internal/handoff.md` §8 digests questions 65–187, `git show 262edb3:internal/handoff.md` has
188–193, `git show f57ee43:internal/handoff.md` §8 has 194–223 with the owner's words (224 and 225 only as
asked: their answers are below), and `git show bdf3581:internal/handoff.md` §8 has 238–246 as asked, with the
owner's inline answers to 238–242. Records 0055–0058 hold what each of 194–223 decided; the ones that steer M4.8
are re-filed in §3.

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

Other answers that steer a later session are in that session's notes (§3, §4) or its plan: 66, 79, 189, 194, 196,
212, 215, 216, 222, 225 and 246 (M4.8), 78 (M4.6), 120, 229, 236 and 237 (M4.5e2), 226–228 and 233 (M4.5e1's
plan), 232 and 239 (M4.5e1c's plan), 238 and 242 (M4.5e1f), 230, 234 and 241 (M8).

**Answered 224–237** (2026-11-11: before and at M4.5e1's planning, around M4.5e1a, and at M4.5e1b's planning and
in its session): recorded in the headers of records 0059 and 0060 and in section 1 of
`.claude/plans/m4-5e1-arrays.md` and of `.claude/plans/m4-5e1b-ranges-in-contracts.md`; the digests that stood here
are in `git show bdf3581:internal/handoff.md` §8. M4.5e1a's and M4.5e1b's listed deviations (their status
entries) stand: the owner answered the questions beside them and wrote nothing about them.

**Answered after M4.5e1b (2026-11-12, inline; the owner's words; the questions as asked are in `git show
bdf3581:internal/handoff.md` §8):**
- **238** (`reads_range`'s ghost amount from the range's first cell: a pinned false `permission` for a caller whose
  holding over the range is uneven): "This sounds like unsoundness, so we need to relook this case and how we can
  fix it." Relooked at M4.5e1c's planning (§3, "M4.5e1f"): it is a false alarm, fail closed, not unsoundness; the
  owner then placed the fix (below).
- **239** (the `must_init` loop filler pinned until e1c): "Follow recommendation."
- **240** (array-parameter shapes the compilers warn about rejected, by the matrix): "Follow recommendation."
- **241** (VLA parameters on a bodiless declaration rejected until M8): "Follow recommendation."
- **242** (the built row order at a call; order D not built): "Follow recommendation, but add where we are going to
  address this, because it seems incorrect." Placed at M4.5e1c's planning (below).
- **243–246** (built overnight under the owner's instruction of 2026-11-11: Part E kept whole; a local array's place
  frames `q[i]` read through a pointer in an invariant; the back-edge rows of read range places in the sum form;
  audit round 4's S1 listed for M4.8): the owner answered 238–242 beside them and wrote nothing about them, so
  they stand unless the owner says otherwise.

**Answered at M4.5e1c's planning (2026-11-23), each the recommendation:**
- **238, where the fix is built:** "Spike in e1c, fix next": M4.5e1c's first il-analyst runs the spike (its plan's
  S45 (r)) and records it; the fix is a short session of its own right after e1c, **M4.5e1f**, with its record,
  lint rule and audit, so that e1c keeps its "no existing Viper changes" proof.
- **242, where order D is built:** "With 238's fix": in M4.5e1f (the same call rows, one set of Viper flips).
- **`cerastes_old` in a loop invariant** (the function's entry state; cells and elements, also under `forall`, for
  places a `requires` atom frames): "In e1c, last part, first cut" (its plan's Part D; cut to M4.5e2 if the session
  runs long).
- **`main` red after Dependabot's PR #35:** "I fix it now, own commit": `fabcd25` (§1).
- The ensemble for M4.5e1c is e1b's. The umbrella plan `.claude/plans/m4-5e1-arrays.md` was trimmed again: its
  section 5 (e1c's sketch) is a pointer to `git show bdf3581:`, and its new section 10 is e1f.

**Open** (from M4.5e1c's plan, section 13; each is built as its recommendation unless answered inline; the next
handoff's questions start at 253):
- **247** An obligation inside the `pure` function a `forall` names (an overflow, an unowned read) is reported at
  the `forall` site, with a note at the operation's line in the function. **Recommended:** the site (one function
  serves many sites, and a contract's errors are reported at the contract).

Follow recommendation

- **248** A trailing argument of `forall` reads no memory; the read is written inside the function.
  **Recommended:** accept as e1c's rule (M4.5e2 lifts it with the other argument forms).

Follow recommendation

- **249** A `forall` stands only in a positive position (a conjunct, either side of `||`, an arm of `?:`): a nested
  `forall`, `!forall` and an `exists` are rejected until M4.8 reviews them. **Recommended:** accept (neighbour
  sortedness, max and the fill loop need none).

Keep rejected until M4.8 and document that this needs to be handled in M4.8

- **250** The three false-alarm pins (`array_loop_fill_then_read_false_alarm.c`,
  `array_loop_self_write_false_alarm.c`, `range_marker_pin_loop_filler.c`) keep their names and expectations: their
  programs write no quantified invariant and fail as before; their comments name their verifying siblings.
  **Recommended:** accept (a rename would move twin references for no change of outcome).

Accept

- **251** Under a binder `old` is written `cerastes_old` in both modes; `cerastes::old` in a `pure` body stays the
  front end's `old_outside_ensures` until M7. **Recommended:** accept (no front-end expectation changes).

Accept

- **252** A contract on a `pure` function is neither read nor rejected today; e1c rejects it only where a `forall`
  names the function. **Recommended:** accept; e1d gives such contracts their meaning.

Accept

**Answered in M4.5e1c (2026-11-23/24, in session, the owner's choices):**
- **253** (S45 (b), "recorded, not gating": a multi-class quantifier is instantiated only where the program holds a term
  of every class, so `requires p[i] == q[i] && q[i] >= 0` with a body that reads only `p[2]` fails to prove `p[2] >= 0`;
  a false alarm, fail closed): "M4.5e2 lifts it" (the recommendation was M4.8). Pinned; a read of `q[2]` at the same
  slot avoids it (record 0061 item 6).
- **254** (S45 (c), outside the one-error rule: the fill loop's skipped-write twin fails at the flag line and at the
  user's clause, two real errors): "Expect both, add 1-error twin (Recommended)" (item 9).
- **255** (raised by il-analyst-2 in Part B): f__wf's ghost functions are method-qualified, `hvq_<method>__<formal>_<state>`
  ("Method-qualified (Recommended)"); four fail-closed narrowings are listed limits, a `forall` on the left of `||`
  (M4.8) and, in a multi-class `E`, a memory-reading guard, arithmetic combining two classes' reads and a converted
  index (M4.5e2) ("Accept; (1) to M4.8, (2)(3) M4.5e2 (Recommended)") (items 1, 3, 7).
- **The owner's instruction for the rest of M4.5e1c** (2026-11-24, about 02:30 UTC): "I'm going to bed so for the rest
  of the session accept what the recommended answer on a question would be after following your normal question and
  answer generation process and then add to the handoff so I can see if these need relooked tomorrow". Every question
  after it is below, each built as its recommendation; a recommendation is always the fail-closed option (reject, pin
  or cut), and a stop with no fail-closed option is parked for the owner instead of chosen.

**Built overnight as recommended, for the owner's review (M4.5e1c, from 2026-11-24 02:30 UTC):**
- **257** (Part B, il-analyst-2's VIR case `fq_both_states_pre_only`; a false alarm, fail closed, not unsoundness): in
  f__wf a read class read in both states (`a[i]` and `old(a[i])`) triggered its quantified clause and rows on its first
  read's ghost only, so over `owns_range` (no keep fact bridges the states) a later obligation naming only `old(a[0])`
  was never instantiated. **Recommended and built:** one trigger set per state for such a class (`{ hvq_m__a_pre(q0) } {
  hvq_m__a_post(q0) }`; more triggers only instantiate assumed facts that hold, and single ghost applications cannot
  loop); the lint admits exactly that shape in `__wf` methods. The alternative was a pin lifted by M4.5e2 with question
  253's family. Record 0061 item 7.
- **256** (split point 1, the plan's section 7; a size and context question, not soundness): Parts A and B measured
  +2,483/−80 `src/` and `include/` lines without the VIR self-test cases (+3,836 with them; `encode.cpp` +902,
  `lower.cpp` +891, `check.cpp` +450), against the plan's 1,050–1,500 and its stop at 1,600, and past the Parts A–C limit
  of 2,200. The overrun is round 1's additions (argument rows, typed ghosts, the forms of `f`, cut 3 in every position,
  marked sites) and question 255's encoder mirrors, most of it rejection paths (fail closed). **Recommended and built:**
  continue to Part C (its fill loop, max and tutorial chapter are not cut without asking) with a ceiling of about 700
  lines for Part C's own delta (its estimate 300–450, at Part B's overrun); split point 2 at Commit 3 takes cut 1 (Part
  D to M4.5e2) unless Part D, audit round 4, the port, the docs, the re-audit and the box run (84 minutes in e1b) fit
  before the session's twelve hours end (about 10:35 UTC; 03:01 UTC at this point). The alternative was to stop after
  Commit 2 and run Parts C and D as a session of their own. Advisor checkpoint 3 agreed.
- **258** (Part C, raised by il-analyst-3; a lint question, not soundness): the guard (`prior`) of a loop's obligation
  line and of its framing `assert` holds the earlier quantified invariant clauses, which the lint refused (52 lines;
  Silicon accepts them). **Recommended and built:** the lint admits user quantifiers of item 11's shape in exactly those
  guards (0037 item 7's evaluation-order prior; each earlier clause is a checked invariant of its own). The alternative
  was to drop quantified clauses from the prior (fail closed, but ordinary loops with two quantified invariants would
  then report false alarms). Audit round 3 reviews it. Record 0061 items 9, 11.
- **259** (Part C, il-analyst-3's lowering; fail closed): in a `check`, a read the binder does not index (`a[r]`) and
  pointer elements under the binder; in a loop invariant or a `check`, `*p` of an array argument and a pointer argument
  used as a value. **Recommended and built:** listed limits lifted by M4.5e2, each an encoder diagnostic at the `forall`
  with a pin. The alternative was to lower them in e1c (more rows under the binder, beyond Part C's ceiling). Record 0061
  item 1.
- **260** (split point 2, the plan's section 7; size, time and context, not soundness): Parts A–C measured +3,245/−237
  `src/` and `include/` lines without the VIR self-test cases (Part C +671/−66, inside question 256's ceiling of about
  700; the plan's stop at 2,200 for A–C). Re-summed at 04:05 UTC with what is owed either way (audit round 3 and its
  fixes, plan §11's pre-box list, advisor checkpoint 5, the box run, status and handoff), Part D would end near the
  twelve-hour cap (10:35 UTC) with no room for a second box run, and its round-4 questions are the only soundness
  questions left with no one to ask overnight. **Recommended and built: cut 1** — `cerastes_old` in loop invariants
  (Part D, record 0061 item 12) moves to M4.5e2, with audit round 1's N17 (an address-taken parameter excluded) and
  Step 0 R2's five further shapes; R2's three pins stay, re-pointed; Part D's two tutorial examples are kept below
  (§3, M4.5e2). Advisor checkpoint 3 recommended the cut. The alternative was Part D in this session. The cut is
  reversible: item 12, P80 (f), S45 (g) and the pins make it a short session.
- **254, a second case** (Part C's run): `forall_invariant_unframed_read.c` fails at its invariant line twice,
  `permission` (the framing) and `uninitialized` (the flag line of the unframed read), as question 254's skipped write
  does: the tests expect both (record 0061 item 9). It is question 254's rule, not a question 160 correction.
- **261** (audit round 3, its blocking item; fail closed, not unsoundness: an exit 3 on wrong programs and a 300 s
  timeout on a wrong loop): in a loop invariant or a `check`, a multi-class `E` with arithmetic on a read of a class other
  than the first (`a[k] < a[k + 1] + 1`) put that row in the first class's view, reading the other class's slot before
  its rows (exit 3) or as a computed `loc` term (the lint; T1's matching loop). **Recommended and built:** rejected there
  (M4.5e2), contracts unchanged (their rows are routed by ghost applications). The alternative was to route each
  obligation to the classes its operands read, in loop and check modes, which is lowering work beyond Part C's ceiling.
- **262** (audit round 3, should-fix; the lint refused correct programs' Viper, fail closed): a `forall` under `||` or in
  a `?:` arm of a loop invariant or a `check`, and `A && forall` in a `check`, were emitted unsplit. **Recommended and
  built:** a `check` is split per top-level conjunct as an invariant is; under `||` and in a `?:` arm of a loop
  invariant or a `check` a `forall` is rejected (M4.8, with question 249's positions); contracts keep both. The
  alternative was to lower those positions as contract clauses are lowered.
- **263** (question 256's ceiling for Part C; size, not soundness): Part C's own delta reached about 900 `src/` lines
  (`check.cpp` +203, `lower.cpp` +352 since Commit 2, `encode.cpp` about +300) against the ceiling of about 700, all of
  the excess from audit round 3's fixes (questions 261 and 262's refusals, the `check` split, the flag line guarded by
  the framing). **Recommended and built:** accept; nothing else enters Part C, and cut 1 (question 260) already took
  Part D. The alternative was to revert round 3's fixes to refusals only, which would leave its lint findings open.
