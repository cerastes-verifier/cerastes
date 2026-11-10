# Plan: M4.5d2b2b — owning fields (record 0055)

**Approved by the owner on 2026-11-09**, at a session that only planned (its commit also consolidated
`internal/handoff.md` and `plan.md`). Questions 188–193 were answered there, each the recommendation (below).

Branch `m4.5d2b2b-owning-fields`, worktree `.claude/worktrees/m4.5d2b2b`, both from `origin/main`.

## Context

**Scope** (question 188): acyclic owning fields, complete: in contracts, in bodies, in copies and in loops, with
the docs table that maps P4417R0's `[[owner]]` positions (question 175). `not_invalidating`, `--strict-owner` and
`ref_to_uninit` on a field are M4.5d2b2c's ("Left for M4.5d2b2c", at the end).

**What d2b2a left to build on** (record 0054):
- block sites through loads (`owns_block(p->q)`, a by-value leaf `s.q`), with partners, the bodiless tie and loop
  places;
- the deep atom of an owning global, `owns(g) && (g == NULL || owns_block(g))`;
- owners in memory: the ghost `held_g`, value and reassignment rows at stores by name, framed updates after calls
  and frees, `main`'s exemption for static owners, synthetic loop sites;
- carriers keyed by the current state, and written `owns_block` invariants.

An owning field is these on a field place: owning a structure is `owns(*p)` plus `p->q == NULL || owns_block(p->q)`
for each owning field.

**Size** (the planning survey of `src/`, insertions without self-tests):

| Item | Lines |
|---|---|
| The closure in contracts: registration, the field reader, the pass, messages | 300–500 |
| Acyclicity | 60–100 |
| Named leaves: struct locals, struct globals, `static` struct locals | 450–800 |
| Stores through pointers, copies, by-value | 200–450 |
| `free(p)` while a field holds a block | 50–100 |
| Loops | 200–450 |
| **Sum** | **1,260–2,400** |

d2b2a landed 3,245 lines against an estimate of 1,650–2,400, so plan for 1,700–3,300. Beside it: `lint_viper.py`
120–300, its self-test 200–450, VIR self-test cases 300–700.

**Key survey facts:**
- **(a) Registration is data only, but it must land with its reader.** EDG accepts target `d`
  (`attribute.c:1659`), and P74 (b) shows a field's attribute on its own `source_corresp.attributes` in both modes.
  But `check_attributes` (`encode.cpp:949`) skips every registered attribute and `struct_info` (17447–17729;
  `add_member` at 17699) reads none. The row alone would accept `owning` on a field and ignore it.
  `vir::StructMember` (`vir.h:171`) has no flag.
- **(b) The closure is new code.** Its model, `apply_owning_globals` (`encode.cpp:2146–2271`), rejects a guarded
  `owns(g)` (2182) and joins only along a top-level `&&` chain (2237). A field's atom is usually guarded:
  `apply_owning` builds `p == NULL || owns(*p)` (2383).
- **(c) d2b2a's Part A is reusable as is.** `collect_derived`, `site_root`, `block_call_rows`, `leak_exemption` and
  `returned_owners` iterate sites generically.
- **(d) Owner identity is a `Var*` or a `Global*`** in about 25 lowering functions (`owners_of`, `scope_of`,
  `held_name`, `owner_value`, `held_updates`, `owners_exit`, `owning_rows`, `global_owner_rows`, `owning_global_of`,
  `global_owner_entry`, `leak_exemption`'s `gowners` term, the `program_start` tag) and in the carriers (`LoopOwner`,
  `loop_owners`, `carrier_guard`, `owner_rows`). Only `holds_token`, `ending_row`, `returned_owners`, the
  written-block machinery and `owner_loop_sites`' builder take an expression.
- **(e) A field reached through a pointer can have no ghost,** since there is no method ghost per heap object. A row
  on tokens alone false-alarms on `free(p->q); t = malloc(…); p->q = NULL; free(t);` (the reallocation case, 0052's
  audit S5).
- **(f) Flags.** `free` reads no leaf value (`block_rows`, `lower.cpp:3919–4016`). `malloc`'s typed view leaves the
  flags unconstrained (`block_mode`, 3837); a struct local's allocation says `!flag` (1076). So every row that reads
  a field's old value is guarded by the field's flag.
- **(g) `copy_struct`** (`encode.cpp:11913–12002`) already expands a copy into leaf stores.
- **(h) Two places refuse or admit too much today.** `check.cpp` (2471–2510) accepts `owns_block(g.q)` but not
  `owns_block(s.q)` of a struct local in an invariant. `read_struct_global_init` (`encode.cpp:4347`) lets a pointer
  leaf start as `&h`.
- **(i) The lint** names `GB_` functions only in `owner_assignment_ok` and the owning-global tag
  (`lint_viper.py:1815–1917`).

**The papers** (P4418R0; wording in P4417R0 [profiles.inval.owner]):
- p3: owner to owner transfers and leaves an alias; owner to non-owner aliases; an owner parameter takes the object;
  an owner result gives it.
- p5 and Note 3: an automatic owner deletes or transfers on every path; a static owner need not.
- §8.2: raw storage is outside the profile.

## Owner decisions

**At this planning (2026-11-09):**
- **188:** two sessions; this one is owning fields, complete, with the `[[owner]]` mapping.
- **189:** the cost of two written load-sites goes to M4.8 with question 66. The spike times a two-field loop, the
  tests pin the one-field shape, and a one-field loop over 120 s stops the session.
- **190:** a copy stores to an owner (item 6). A by-value parameter or result is a value.
- **191:** any acyclic depth. Chains are a pre-agreed cut; if cut they are rejected, naming M4.5f2.
- **192:** `ref_to_uninit` on a field is d2b2c's. Here it stays EDG's refusal, pinned.
- **193:** the two session tools are committed under `scripts/`, and S40's summary into `il-inventory.md`.

**Standing:**
- 174 (static owners are exempt at `main`'s end); 175 (`owning` on a parameter stays a borrow; the docs map each
  `[[owner]]` position).
- 69 (no EDG work); 77 (one context); 160 (a new test's expect line may be corrected after its first run when it
  fails for its designed reason at a neighbouring line or category; each correction is audited and listed in
  status).
- `check.sh` once, at the end, after the last code change, on the x86-64 box only.
- No Claude trailer; commits stay local; an agent's question stops that work and goes to the owner.

## Step 0 — before anything

1. **The box's full check on `b6b3298`:** `cat ~/cerastes-s8/p-ubuntu-26.04/d2b2a-check.exit` (0 is green) and the
   summary of `d2b2a-check.log`. If it is red, fix that first with targeted checks, and say so in status.
2. `git fetch`. Everything below is committed on the session branch, never on `main`.

## Step 1 — worktree, CI, tools, baseline (no encoder code)

1. `git worktree add .claude/worktrees/m4.5d2b2b -b m4.5d2b2b-owning-fields origin/main`; submodules with
   `--reference` (edg, silicon, then silver); `scripts/bootstrap.sh`.
2. **CI, one small commit on the branch** (questions 169 and 172):
   - `gh run list --workflow ci.yml --event workflow_dispatch`. If the owner's `profiles=all` dispatch ran and
     `macos-26` is green, promote it to `nightly` in `scripts/ci_toolchains.toml` with the guard self-test's change.
     If it ran red, record what differs in the handoff. If it has not run, leave it and say so.
   - The comment at `scripts/ci_toolchains.toml:133` takes question 169's answer (`ubuntu-24.04-clang18` stays at
     `dispatch`; revisited at M8).
3. **The tools** (question 193): `scripts/vprdiff.py` (compare two `.vpr`/`.map` trees under an allow-list) and
   `scripts/regen_vpr.py` (regenerate every kept `.vpr` and `.map` with `build/cerastes`, no Silicon), with a
   paragraph in the developer guide's Testing page.
4. **Baseline** in a detached worktree `.claude/worktrees/m4.5d2b2b-base` at the starting commit:
   `regen_vpr.py` (about 30 minutes) and the VIR self-test, copied to `build/m4.5d2b2b/baseline/`; then remove the
   base worktree. The baseline's outcomes are the box's check of Step 0.

## Record 0055 — owning fields (written before code)

Amends 0029 item 1 (target `d`), 0043 items 7 and 9, 0044 items 3, 4 and 8, 0052 items 3 and 8, 0054 items 2, 5, 6
and 10.

1. **Scope:** items 2–8. Fail closed on the rest (item 11).

2. **Registration and acceptance.**
   - `owning` becomes `"r|v|p|d"`, in the commit that adds the reader in `struct_info` and `StructMember::owning`.
   - **Accepted** on a field of a verified structure (0043 item 2) whose type is a pointer to an object type, in
     either position; each declarator carries its own (P74 (b)). C structures and C++ aggregates alike.
   - **Rejected** (`unsupported_annotation`, one pin each): a non-pointer field; a pointer to a `const` or
     `volatile` type; a count (`owning(n)`, M4.5e1); twice on one field.
   - **Acyclic.** The owning-field graph has an edge from a structure to the pointee structure of each of its
     owning fields, nested struct members included. A cycle (`struct node { struct node *next [[owning]]; }`, or
     two structures that own each other) is rejected at the field, naming M4.5f2. `check.cpp` has a backstop
     modelled on `contains_by_value`.

3. **The closure of an atom** (the deep atom, generalizing 0054 item 5).
   - **Which atoms.** Any atom that gives `write` permission to an owning leaf's cell:
     - `owns` of a place that covers it: `owns(*p)`, `owns(p->t)`, the leaf itself `owns(p->q)`; the same on a
       struct global (`owns(g)`, `owns(g.q)`); the implicit `owns(s)` of a `static` struct local;
     - `owning` on a `struct S *` parameter (`p == NULL || owns(*p)`);
     - a plain block site on a pointer to such a structure: `owns_block(p)`, an `owning` function's result, the
       deep atom of an owning `struct S *` global.
   - **What follows it.** For each owning leaf `L` it covers, the clause `L == NULL || owns_block(L)`, right after
     the atom, **under the atom's own guards**, in the list the atom is in. The clause is 0054 item 2's site through
     a load, so its partners, its rows at calls and returns, the leak exemption and the bodiless tie are item 2's.
   - **Recursively** (question 191). The block site the clause adds is itself an atom of the first list when `L`
     points to a structure with owning fields. Item 2's acyclicity makes the closure finite.
   - **`reads` and `live` get none.** `reads(*p)` gives the field's value only.
   - **Marked sites get none.** A cell without a value owns nothing. So `malloc`'s typed view, a `ref_to_uninit`
     result, `free`'s parameter and the `requires` side of a `must_init` parameter carry no clause. `must_init`'s
     `ensures` side carries it, as a given site: this is the initializer `void s_init(struct S *p
     CERASTES_MUST_INIT)`. `ref_to_uninit` on a parameter whose pointee has an owning field is rejected (pinned).
   - **Allocation views get none.** `calloc`'s view is a plain block site, but it states every leaf zero or
     `NULL`, so its owning fields hold nothing and the clause would be vacuous. Without this rule
     `calloc(1, sizeof(struct S))` would be a closure through a bodiless contract's result (below).
   - **Merging and allocators** (0054 item 5). The clause names the platform's allocator. A user who writes the
     clause beside the atom, in this exact shape or with `cerastes_owns_vlibc_block`, has it merged. A block site
     on an owning leaf in any other shape, beside an atom that covers the leaf, is rejected ("`owns` of the
     structure already includes it").
   - **Bodiless contracts.** 0054 item 2's tie applies: a bodiless borrow of a structure with an owning field
     writes `p->q == old(p->q)`. (`not_invalidating` will say it; d2b2c.) A clause through a bodiless contract's
     fresh result is decided by the record with the audit; the default is a rejection naming M6, since nothing
     proves the field's block fresh.

4. **Stores to an owning field through a pointer** (`p->q = x`, `p->t.q = x`, inside an expression too).
   - **The value rule** (`permission`, at the store): `x != NULL ==>` the function holds `x`'s block whole. "owning
     field `p->q` receives a pointer that is not the start of a block the function holds".
   - **The flag.** The old value is read only under the leaf's flag (fact (f)).
   - **A dropped block** (fact (e)). S40 (d) decides between two shapes:
     - **preferred:** a `leak` row at the store ("assigning to owning field `p->q` drops the block it holds") and at
       `free(p)` ("frees `*p` while its owning field `q` still holds a block"). It is built only if it raises no
       false alarm on a transfer between two structures (`b->q = a->q; a->q = NULL;`) and on the reallocation case;
     - **fallback:** no row. The block is reported by the end-of-function leak row (0047 item 7), which is sound
       today, and the docs say where the error appears.
   - **A store through an alias** (`int **pp = &p->q; *pp = x;`) gets no row, as for an owning global (0054 item
     5). The closure's return row or the end leak row reports it.

5. **Owning leaves at named places:** a struct local (a by-value parameter's copy and C's `&(T){…}` object
   included), a struct global, a `static` struct local. Each owning leaf is an owner in memory by 0054 items 5 and
   6, keyed by variable and member path.
   - **Re-keying first.** The owner identity of fact (d) becomes one type (root, leaf path, the held ghost's name,
     the value's read) in a commit that changes no output (`vprdiff.py`: none differ), as d2b2a re-keyed block
     sites.
   - **Ghost** `held_<var>_<path>`: false at a local's declaration; at entry true exactly where a `requires` site
     gives the leaf's block.
   - **Rows** at stores by name: the value rule and the reassignment row (`leak`). Updates after calls and frees
     are framed as in 0054 item 5.
   - **Lifetime end** of a struct local: `leak` at the exit, "owning field `s.q` still holds its block when `s`'s
     lifetime ends".
   - **Globals.** An owning leaf's initializer is absent or null (`&h` is refused, fact (h)). `main`'s leak row
     exempts the leaves of struct globals and `static` struct locals (question 174), under their `held` ghosts.
   - **`&s` to a callee:** its contract's closure takes and returns the leaf's block; then `held := leaf != NULL`.
   - **`&s.q` as a `T**`:** no row at a store through it, as for a global; the record decides with the audit
     whether a struct local's leaf needs 0054 item 6's guarded rows instead.

6. **Copies and struct values** (question 190).
   - **A copy** (`t = s;`, `*p = *q;`, an initializer from an object) is its leaf stores (fact (g)). Each owning
     leaf of the destination is a store to an owner: item 5's rows at a named leaf, item 4's through a pointer.
     Nothing is cleared, so source and destination then hold one block, as two owning locals do after `b = a;`
     (0052 item 3). While both live, each one's exit row accepts the other. Where both are owned across a call or
     a return, two sites name one block and the row is `permission`; the program sets one to `NULL` first.
   - **A by-value parameter or result** is the tuple of its leaves (0044) and owns nothing. It carries a block only
     where the contract says `owns_block(s.q)` or `owns_block(r.q)` (0054 item 2). The parameter's copy in the
     callee is a struct local, whose leaf's `held` starts true exactly there. `f(s)` without such a site passes an
     alias.
   - C++ `S{…}` values and compound-literal values likewise own nothing.

7. **Loops.**
   - The closure's `requires` sites are automatic loop places (0054 item 2), entry-keyed. A loop that replaces a
     field's block writes the invariant.
   - A written `owns(*p)` invariant on a structure with owning fields carries the clause as a written `owns_block`
     invariant (0054 item 4).
   - Named leaves get 0054 item 5's synthetic loop sites and the carriers of 0054 item 3, on the re-keyed owner.
     `check.cpp` admits a block site rooted at a struct local's leaf in an invariant (fact (h)).
   - **Cost** (question 189). A structure with two owning fields under a written invariant is 0054's expensive
     shape (33–148 s). S40 (g) times it; the tests pin one owning field per loop; the two-field time goes to the
     record and the handoff for M4.8.

8. **The `[[owner]]` mapping** (question 175; docs only). Variable, field and result: `cerastes::owning`. Parameter
   (a transfer): `requires(p == NULL || owns_block(p))` with no partner. `cerastes::owning` on a parameter is a
   borrow, which P4417 has no attribute for. Static owners: exempt. Raw storage: P4417 leaves it out; cerastes
   checks it.

9. **The lint** (`lint_viper.py`): the leaf ghosts and their assignments (a leaf cell in place of `GB_x`'s);
   `main`'s exemption terms for struct leaves, by the `program_start` tag of 0054 item 8 extended with the member
   path; the rows of item 4 if built.

10. **Categories:** none new. `leak` and `permission` gain the messages of items 3–6.

11. **Still rejected** (fail closed; one pin each, naming the session): recursive owning fields (M4.5f2);
    `owning(n)` on a field (M4.5e1); `ref_to_uninit` on a field (EDG's #1835; d2b2c); `ref_to_uninit` on a pointer
    to a structure with an owning field; a block site of another shape beside an atom that covers the leaf; a
    closure through a bodiless contract's result (if the record keeps the default); a non-null initializer of an
    owning leaf of a struct global.

12. **Facts:** P74 (b), P76, S40.

## P76 and spike S40 (il-analyst, before code)

**P76 (IL; `il-inventory.md`).** Only what P74 (b) left:
- an owning field in a braced, designated and nested initializer, in a compound literal, in a C++ `S{…}` value;
- stores to `s.q`, `p->q`, `p->t.q` inside expressions; `&s.q` and `&p->q` as `T**`;
- struct copies and by-value passing of a structure with an owning field;
- `must_init` and `owning` on a `struct S *` parameter whose structure has an owning field;
- an owning field in a C++ aggregate, and in a structure named only by a typedef.

**S40** (hand-written Viper in the target shapes, each with Silicon's verdict and its time on a quiet machine;
fixtures in `build/m4.5d2b2b/s40/`, the kept ones in `tests/viper-smoke/memory/s40_*`, **the summary in
`il-inventory.md`**):
- (a) a borrowed `owns(*p)` with its closure; the callee frees and replaces `p->q`; twins for the return row;
- (b) the initializer (`must_init`, the given site), the destructor (`owns_block(p)` consumed with its closure) and
  `main` between them; the same structure from `malloc` (fields then written) and from `calloc` (fields `NULL`);
- (c) a struct local with an owning leaf: the ghost, the exit row, `&s` to a callee;
- (d) item 4's dropped-block row on the transfer and the reallocation shapes; the `free(p)` row;
- (e) item 6's copies: `t = s;` then one side set to `NULL`, and the twin that leaves both;
- (f) a chain of two owning fields (`a->b->x`), timed;
- (g) a loop that replaces a field's block under the written invariant; a loop over a named leaf; the two-field
  loop, timed;
- (h) a struct global with an owning leaf: `program_start`, `main`'s exemption.

A failed spike changes the record before code. Audit round 1 reads the draft with S40.

## Parts and commits

1. **Commit 1:** Step 1's CI commit, if any.
2. **Commit 2:** the two scripts; record 0055 (draft, revised after S40 and audit round 1); P76; S40's fixtures and
   summary.
3. **Commit 3, Part A — fields in contracts:** registration with the reader; the rejections and acyclicity; the
   closure (item 3); the value rule and the flag (item 4); the flip.
4. **Commit 4, Part B — named leaves:** the neutral re-keying, proved by `vprdiff.py`; then item 5.
5. **Commit 5, Part C — copies, values and dropped blocks:** item 6; item 4's rows as S40 (d) decided.
6. **Commit 6, Part D — loops:** item 7.
7. **Commit 7:** audit round 2's fixes and the re-audit's.
8. **Commit 8:** docs, status, handoff (with its Questions section), `plan.md`'s Done row, record 0055 accepted.

The soundness-auditor reviews each part's diff on a snapshot commit in a second worktree (d2b2a's method), before
that part's commit. One re-audit of the blocking fixes.

**Pre-agreed cuts, in this order.** Cuts 1 and 2 go to d2b2c; cut 3 names M4.5f2 (question 191).
1. **Loops over named leaves** (the second half of Part D). A loop that writes an owning leaf of a struct local or
   global, or whose callee takes that leaf's closure, is then **rejected** at the loop (`unsupported_statement`:
   "a loop that writes the owning field `s.q`", naming d2b2c), with one pin per shape. It is a rejection, not a
   pinned false alarm (invariant 3).
2. **The dropped-block rows through pointers** (Part C). The end-of-function leak row reports the block meanwhile,
   which is sound; the docs say where the error appears, and a test pins that position, naming d2b2c.
3. **Chains.** An owning field whose pointee structure has an owning field is then rejected at the field
   (`unsupported_annotation`, naming M4.5f2). Taken at S40 (f) if the spike fails or a two-level program passes
   60 s, or later if the context runs short.

**Split point.** At Part C's commit, if the context will not carry Part D with its audit and the docs, stop and
ask; the pre-agreed answer is cut 1. Stop and ask before any other cut.

## Team (from `.claude/agents/`: three Sonnet agents with disjoint files, one Opus reviewer)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | **First:** `scripts/vprdiff.py`, `scripts/regen_vpr.py`, P76, S40. **Then:** `src/vir` (`StructMember::owning` first, then the re-keying, the leaf owners, rows, loop sites, `check.cpp`'s backstops) and `src/viper` (the VIR self-test cases, `VIR_CASES_MIN`). A fresh il-analyst takes over if its context runs low. |
| **edg-backend** (Sonnet) | `register_attributes.cpp`; `encode.cpp` (the field reader in `struct_info`, acyclicity, the closure pass beside `apply_owning_globals`, `finish_atoms`' messages, `read_struct_global_init`, the named-leaf marks, `copy_struct`); the builds. |
| **test-mutator** (Sonnet) | Tests and twins; exactly the Flips; `lint_viper.py` with its self-test; the doc example files. |
| **soundness-auditor** (Opus, read-only) | Round 1: the record draft and S40. Round 2: each part's code and tests. One re-audit. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Steps 0 and 1 (the CI commit included); record 0055; docs prose; `encoding.md`; integration; serialized builds (`lockf build/m4.5d2b2b/build.lock`); status and handoff. |

**Rules for every agent:**
- Any question stops that work and goes to the owner.
- Agents hold builds before a directory-wide run, and only one agent runs `run_tests.py`.
- Single-file runs use `build/cerastes verify`.
- Code style: `scripts/format.sh`, brace initialization, `U`, one declarator per declaration, no comment on a brace
  line. A new `.cpp` updates `object_identity.py`'s TU constants.
- A test generator lives in `build/m4.5d2b2b/` and is never re-run over files in the tree.

## Tests (each positive has a twin failing for one reason at one line; `.c` in `c23, c++20` where valid; positives `native-ok`)

- **Part A:**
  - a structure with an owning field: an initializer, a destructor and `main`;
  - the structure allocated by `malloc` and by `calloc`, filled, then destroyed; the twin frees it with a field
    still holding a block;
  - a borrower that reads `*p->q`; one that frees and replaces `p->q`. Twins: a return after `free` without `NULL`
    (`permission`), a store of `&x` (`permission`);
  - `reads(*p)` cannot free `p->q`;
  - a nested member `p->t.q`; a leaf atom `owns(p->q)`; the `owning` parameter; `owns_block(p)` consumed; an
    `owning` function that returns such a structure;
  - the vlibc spelling merged; a block site of another shape beside the atom (rejected);
  - a chain `a->b->x`;
  - a bodiless borrow with `p->q == old(p->q)`, and without it (the tie fails);
  - every rejection of items 2 and 11, the two recursive shapes among them.
- **Part B:**
  - a struct local: freed; left at scope exit (`leak`); reassigned (`leak`); `&s` to an initializer and to a
    destructor;
  - a struct global: `main` ends holding it (exempt), while a non-owning field's block is a `leak`; a `static`
    struct local as a cache.
- **Part C:**
  - `t = s;` with one side set to `NULL`; the twin that returns both (`permission`);
  - a by-value parameter with and without `owns_block(s.q)`; a by-value result that gives `r.q`'s block;
  - a block dropped through `p->q = …` and by `free(p)`, at the position S40 (d) chose.
- **Part D:** a loop that replaces a field's block under a written invariant, with a twin that drops it at the back
  edge; a loop that reads through a borrowed field; a loop over a struct local's leaf.
- **Byte identity:** `scripts/vprdiff.py` over the whole suite against the baseline. No existing program's Viper
  changes, the flipped pin aside. The VIR self-test changes only in its new cases.

## Flips (pre-approved with this plan; any other changed outcome stops the session)

The planning survey searched `tests/`, `doc/` and `src/` for every pin that names this session or its constructs.
There is exactly one.

- **Retired for a positive of the same shape, with a twin:** `tests/memory/negative/reject_struct_owning_field.c`
  (`rejected wrong_entity_for_attribute @ L13`).
- **New pins** take item 11's rows.
- **Stay as written:** the six `reject_strict_init_malloc_*` pins and `reject_strict_init_vlibc_malloc_local.c`
  (d2b2c's, question 192); `reject_owning_member_function.cpp` (M7); `reject_struct_value_marker_owning.c`; the
  four `*_false_alarm.c` pins.
- No VIR self-test case is converted.

## Docs (invariant 8; every example is a test)

- **`memory.adoc`:**
  - a new section "Owning fields" after `[#owning-globals]` (line 1102), replacing the sentence at 1099–1100;
  - a table "P4417R0's owners in cerastes" (item 8);
  - `[#not-yet]` (1192–1253): the owning-member bullet (1215–1217) rewritten to what is left; the
    invalidation-profile bullet (1220–1226) stays, since `not_invalidating` and `--strict-owner` are still to come.
- **`annotations.adoc`:** `owning` on a member, under `[#marked-locals]` (296–298, 411–412).
- **`error-messages.adoc`:** the new messages.
- **The tutorial:** `tutorial/heap.adoc` gains "A structure that owns its buffer" before "What you have learned"
  (162); its closing paragraph (174–182) is brought up to date (record 0049 item 4).
- **The developer guide's Testing page:** the two scripts.
- **`internal/encoding.md`:** record 0055; lines 602 and 617 still say "M4.5d2b2" for what d2b2a built.
- **`internal/il-inventory.md`:** a chapter `## M4.5d2b2b` with P76 and S40's summary.
- **`src/backend/register_attributes.cpp`:** the comment at 61–71.

## Stop and ask

- Any agent question.
- Any changed outcome outside Flips.
- Any existing program's Viper changing outside `vprdiff.py`'s allow-list.
- A spike that contradicts items 3, 5 or 6.
- A cut beyond the pre-agreed ones; the split point.
- An exit 3, or a timeout over 120 s, not fixed within its part.

## Acceptance (run at the end)

- `scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): **once, after the last code change, on the x86-64 box.**
  The branch goes as a git bundle into `~/cerastes-s8/p-ubuntu-26.04`, then `scripts/check.sh` in its container.
  Green. The Mac runs only targeted suites.
- `python3 scripts/vprdiff.py build/m4.5d2b2b/baseline/vpr build/vpr`: only allow-listed tests differ.
- The VIR self-test at the new `VIR_CASES_MIN` (1669 today); `test_lint_viper.py`; `test_run_tests.py`.
- `scripts/format.sh --check`; no new comment on a brace line in the diff of `src/`.
- `scripts/build_docs.sh`.
- The status entry (done and not done, the flips and corrections, surprises, record 0055, the first step for
  d2b2c's planning) and the handoff with its Questions section.
- Commits stay local on the branch; the owner pushes.

## Left for M4.5d2b2c (its design notes are in handoff §3, "M4.5d2b2c")

`cerastes::not_invalidating` (question 178), `--strict-owner` (question 177), `ref_to_uninit` on a field (question
192), and this session's cuts 1 and 2 if taken. Cut 3 (chains) is M4.5f2's.

## Inputs (from handoff §3 at this planning)

Verbatim from `git show 2430529:internal/handoff.md` §3. The first block is copied, since M4.5d2b2c needs it too
and it stays in the handoff under "M4.5d2b2c". The notes of "From M4.5d2b2a" are copied as well: the handoff
re-files them under M4.8, Standing and §6. Each is handled as the plan says: the cost note is item 7 and question
189; the documented limits are record 0054's, and field owners inherit them (items 3–7 add none); the lint note is
item 9; the process note is the rules for every agent.

**M4.5d2b2b owning fields and P4418R0** (d2b2a's notes for it: owning fields reuse Part A's block sites through
loads (`owns_block(p->q)`), Part B's carriers and Part C's deep atom: owning a structure is `owns(*p)` plus `p->q ==
NULL || owns_block(p->q)` per owning field, as `owns(g)` of an owning global is; `not_invalidating` adds implicit
`ensures(p->q == old(p->q))` per owning field reachable through `p`; from M4.5d2a's planning; sources: P4222R3, P4416R0, P4418R0, P4417R0, all
2026-10-22, read again at M4.5d2b1's and M4.5d2b2a's planning; d2b2a's planning decided the spellings: questions 174
(static owners exempt at `main`'s end, P4417 (2.3), built in d2b2a), 175 (`owning` on a parameter stays a borrow), 177
(`--strict-owner`), 178 (`cerastes::not_invalidating`, checked), 179 (no `no_dangling` in C); d2b1's notes moved into
d2b2a's plan's Inputs)
- P4417R0's `[[owner]]` on a parameter is a **transfer** (the callee must delete), where 0029 item 1 made
  `cerastes::owning` on a parameter a **borrow**; P4418 exempts static owners where 0029 item 1 has an owning global freed
  by the end of `main`. `[[not_invalidating]]` (a pointer parameter whose function does not invalidate, checked at the
  definition) and `no_dangling()` (a suppression, `std::forward` in P4417 8.4) are what d2b spells in `cerastes::`;
  P4417 keeps raw storage (`malloc`) out of the invalidation profile. C applies: raw pointers, structs with pointer
  members as containers, the escape rules, `[[owner]]` on variables, parameters, fields and results, `not_invalidating`
  on pointer parameters; references, iterators, smart pointers and `new`/`delete` are M7's.
- P4417R0's wording, read at d2b1's planning ([profiles.inval.owner]): p3 owner-to-owner transfers and leaves an alias,
  owner-to-non-owner aliases, an owner parameter takes the object, an owner result gives it; p4 an allocation's (there:
  `new`'s) or a call's owning result must initialize or be assigned to an owner, or be deleted, in its full-expression;
  p5 an automatic owner deletes or transfers on every path to its block's end (a static owner need not, Note 3); p6
  only an owner is deleted; p7 every alias of a transferred object is invalid. §8.2's drafting note keeps raw storage
  (`allocator::deallocate`) out of the profile. `not_invalidating` (9.13.u) appertains to a pointer parameter or a
  member function and is verified at the definition ([profiles.inval.verify]); `std::no_dangling(x)` returns
  `std::forward<T>(x)` and waives the escape rules for its result. d2b1 gives owning automatic locals 0029's meaning
  (a path-precise `leak`), which agrees with p5. [d2b1 planning]

**From M4.5d2b2a** (record 0054)
- **Costs to watch:** two written `owns_block` sites through loads over parameter places cost about 33 s for the loop
  alone and about 148 s with one drop in the body, quiet. The tests pin cheaper shapes (question 186). The `?:`
  framing chains repeat in every pair row and every `forperm` instantiation; the reads are hoisted, and the next step
  is the pair and exclusivity structure. This belongs to M4.8 with question 66. [d2b2a]
- **Documented limits** (record 0054 items 2–5):
  - a loop that calls a borrower tied on `*pp` needs `*pp == keep` in its invariant;
  - `--strict-init` does not check a store to a marked global through an alias;
  - a fresh result beside a tied load in a bodiless contract cannot be proved distinct;
  - the syntactic sharing test misses `int *t = a; b = t;`;
  - the becomes-shared body nested in an outer loop is a false alarm;
  - a block shared between an owning global and an owning local inside a loop is a false alarm;
  - a synthetic site's `wef` heuristic gives `uninitialized` to a read of `*g` after a loop that stores nothing;
  - an address-taken owner reached in a loop only through a hidden alias stays in the frame;
  - a vlibc block stored into an owning `static` local is reported at the return, and in `main` not at all;
  - a loop over an owner in memory that holds a vlibc block is a false alarm: the synthetic site assumes the platform's
    allocator (the re-audit's s4/v2–v5; conservative);
  - an owning global reached in a loop only through an alias invariant (`*pg`, `pg == &g`) whose callee frees it, with
    a `break`, is a false `leak` at `main`'s end: the global is neither carried nor framed, so `held_g` is havocked at the
    loop head (the re-audit's t5; the cost of the N2 fix, which closed an exit 3). A loop carry for owners reached
    through alias invariants would lift it;
  - `owning_loop_stops_shared_drop.c`'s first line is a false alarm pinned by question 185. [d2b2a]
- **Lint:** `owner_assignment_ok` admits `held_X := <x> != null` anywhere. This is harmless (audit round 2 on C/D,
  N2), but it is a place where a later rule could be tightened. [d2b2a]
- **Process:** a test generator re-run wiped three hand-added expect lines. The audit caught it, and §6's rule (never
  re-run a generator over files in the tree) held after that. The audits ran on snapshot commits made with a
  temporary index (`GIT_INDEX_FILE=… git add -A; git write-tree; git commit-tree`) in a second worktree with its own
  build. So round 2 audited each part while the next part was built, with no mixed working tree. [d2b2a]
