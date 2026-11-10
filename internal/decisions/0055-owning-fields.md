# 0055 — Owning fields (M4.5d2b2b)

- **Status:** accepted (2026-11-10, M4.5d2b2b), written before code (invariant 6), revised after spike S40
  (`il-inventory.md`, chapter M4.5d2b2b): S40 (d) decided item 4 for the fallback, S40 (f) left chains in (no cut 3),
  and S40 added the bodiless initializer's rejection (item 3) and the two-field loop's cost (item 7). Revised after
  audit round 1 (Opus; no blocking item; should-fix S1–S7 and notes N1–N10 taken in items 3–7 and 11): the two open
  points are settled, item 3's bodiless fresh result by S2 (the rejection kept, its scope and reason restated) and item
  5's `&s.q` as a `T**` by question 3(b) (no row, sound). Audit round 2 per part, on snapshots: Part A (`4e2cdc2`; no
  blocking item; S1 tests, S2 a message, S3 a marked owning global rejected), Parts B and C (`d299fb9`; B1 leaf ghosts
  havocked by loops, S1 an unguarded entry read, S2 owner lists without leaves, S3 the cover's cost), and the re-audit
  with Part D (`7ba961f`; B1 the S3 change left a covered site exempt from the leak row — unsound, fixed; B2 a global
  leaf reached by type neither framed nor carried; S1 an assigned owner's closure; S2 chains in loops), then a focused
  review of those fixes. Owner decisions from M4.5d2b2b's planning, 2026-11-09 (plan
  `.claude/plans/m4-5d2b2b-owning-fields.md`), each the recommendation:
  - **188:** M4.5d2b2 is three sessions; this record is d2b2b's, owning fields complete with the `[[owner]]` mapping.
    `not_invalidating`, `--strict-owner` and `ref_to_uninit` on a field are d2b2c's.
  - **189:** the cost of two written load-sites goes to M4.8 with question 66.
  - **190:** a copy of a structure with an owning field stores to an owner; a by-value parameter or result is a value.
  - **191:** an owning field may point to a structure with owning fields, to any acyclic depth.
  - **192:** `ref_to_uninit` on a field stays EDG's refusal here, pinned (d2b2c).
  - **193:** the session tools are `scripts/vprdiff.py` and `scripts/regen_vpr.py`.
  - Standing: 174 (static owners exempt at `main`'s end), 175 (`owning` on a parameter stays a borrow).
- **Milestone:** M4.5d2b2b.
- **Amends:**
  - 0029 item 1: target `d`.
  - 0043 items 7 and 9: copies and member stores of an owning leaf.
  - 0044 items 3, 4 and 8: by-value structures and struct storage with owning leaves.
  - 0052 items 3 and 8: owners at named places, the still-rejected list.
  - 0054 items 2, 5, 6 and 10: the closure as a block site through a load, owners in memory keyed by leaf, the
    still-rejected list.

## Context

**What a user writes today.** Record 0054 item 2 accepts a block site through a load, so a borrow of a structure that
owns a buffer is already writable by hand:

```c
struct buf { int *data [[cerastes::owning]]; };   // rejected today: EDG #1835 (no target `d`)
void reset(struct buf *b)
    [[cerastes::requires(owns(*b) && (b->data == NULL || owns_block(b->data)))]]
    [[cerastes::ensures(owns(*b) && (b->data == NULL || owns_block(b->data)))]];
```

What is missing is the attribute on the field, and with it:
- the clause written by the encoder wherever the structure is owned (item 3), so that a contract says `owns(*b)` and
  means the buffer too, at any depth;
- checks at the stores to the field (item 4), instead of a later return row;
- owning leaves of named structures (struct locals, globals, `static` locals) as owners in memory (item 5);
- copies of such structures (item 6) and loops over them (item 7).

**P4417R0** [profiles.inval.owner] (owner questions 175 and 177–179): p3 an owner-to-owner assignment transfers and
leaves an alias, an owner-to-non-owner assignment aliases, an owner parameter takes the object, an owner result gives
it; p5 and Note 3 an automatic owner deletes or transfers on every path, a static owner need not; §8.2's drafting note
keeps raw storage out of the profile. cerastes checks raw storage (0047).

**Survey facts** (the planning survey, re-read at the session's start):
- **(a)** EDG accepts target `d` (`attribute.c:1659`); P74 (b) shows a field's attribute on its own
  `source_corresp.attributes` in both modes. `check_attributes` skips every registered attribute and `struct_info`
  reads none, so registration must land with its reader.
- **(b)** `apply_owning_globals` (0054 item 5's deep atom) joins only along a top-level `&&` chain and rejects a
  guarded `owns(g)`; a field's atom is usually guarded (`p == NULL || owns(*p)` from `apply_owning`).
- **(c)** 0054's block-site machinery iterates sites generically (`collect_derived`, `site_root`, `block_call_rows`,
  `leak_exemption`, `returned_owners`).
- **(d)** Owner identity in the lowering is a `Var*` or a `Global*` in about 25 functions and in the loop carriers.
- **(e)** A field reached through a pointer has no method ghost (there is none per heap object).
- **(f)** `free` reads no leaf value; `malloc`'s typed view leaves the flags unconstrained; a struct local's
  allocation says `!flag`. So every row that reads a field's old value is guarded by the field's flag.
- **(g)** `copy_struct` already expands a copy into leaf stores.
- **(h)** `check.cpp` accepts `owns_block(g.q)` but not `owns_block(s.q)` of a struct local in an invariant;
  `read_struct_global_init` lets a pointer leaf start as `&h`.

## Decision

1. **Scope:** owning fields in contracts (items 2 and 3), in bodies (items 4 and 5), in copies and values (item 6) and
   in loops (item 7), and the mapping of P4417R0's `[[owner]]` (item 8). Everything else that names an owning field
   fails closed (item 11).

2. **Registration and acceptance** (amends 0029 item 1).
   - `owning` becomes `{"owning", "?(X)", "r|v|p|d", RAF_ENTITY_ATTACHED}`, in the commit that adds the reader in
     `struct_info` and the flag `vir::StructMember::owning`. Before that commit the attribute on a field stays EDG's
     #1835.
   - **Accepted** on a non-static data member of a verified structure (0043 item 2), C structures and C++ aggregates
     alike, in either position (before the declarator or after it); each declarator carries its own (P74 (b)). Its
     type must be a pointer to an object type that is not `void`, a function, or `const`- or `volatile`-qualified.
   - **Rejected** (`unsupported_annotation`, one pin each, at the field): a non-pointer field; `void*`; a pointer to a
     function (M4.7); a pointer to a `const` or `volatile` type; a count (`owning(n)`, M4.5e1); twice on one field; a
     field of a union (deferred beyond M4); a bit-field.
   - **Acyclic** (question 191). The owning-field graph has an edge from a structure `S` to the pointee structure of
     each owning field of `S` and of each by-value member of `S`, at any depth. A cycle is rejected at the field that
     closes it (`unsupported_annotation`: "owning field `next` makes `struct node` own itself; recursive ownership is
     M4.5f2"): `struct node { struct node *next [[owning]]; }`, and two structures that own each other. `check.cpp`
     has a backstop modelled on `contains_by_value`, so a cycle that reaches the lowering is an internal error (exit 3),
     never a non-terminating closure.

3. **The closure of an atom** (generalizes 0054 item 5's deep atom).
   - **Which atoms close.** Every atom that gives `write` permission to an owning leaf's cell:
     - `owns` of a place that covers the leaf: `owns(*p)`, `owns(p->t)` for a by-value member `t` holding the leaf,
       the leaf itself `owns(p->q)`; the same on a struct global (`owns(g)`, `owns(g.t)`, `owns(g.q)`) and through
       loads (`owns(*pp->s)`, 0045);
     - the implicit `owns(s)` of a `static` struct local (0041 item 12);
     - `owning` on a `struct S *` parameter (`p == NULL || owns(*p)`, 0040);
     - a plain block site on a pointer to such a structure: `owns_block(p)`, an `owning` function's result, the
       deep atom of an owning `struct S *` global or local in memory (0054 item 5), a block site through a load whose
       pointee is such a structure.
   - **What follows it.** For each owning leaf `L` the atom covers, the clause `L == NULL || owns_block(L)`, placed
     right after the atom **under the atom's own guards**, in the same list. The clause is 0054 item 2's site through a
     load: its key, its mode (plain), its partners, its rows at calls and returns, the leak exemption and the bodiless
     tie are item 2's. Its reads of `L` are framed by the atom it follows. Only the clause is added, never an atom: a
     leaf atom `owns(p->q)` beside a covering `owns(*p)` is 0043 item 9's overlap rejection, and `owns(*r)` beside a
     block site `owns_block(r)` is 0047 item 4's "beside" rejection. So a block site closes as `r == NULL || r->q ==
     NULL || owns_block(r->q)`, its cells framed by the block (P76/S40 probes `q/vi2.c`, `q/iv.c`, `q/vi.c`).
   - **Every closed form is one a user can write today** (S40's table, `build/m4.5d2b2b/s40/q/`): the borrow (i), the
     consumed destructor site (ii), the initializer with `requires(owns(*p))` beside `must_init` (iii; 0042 item 4
     needs it), the disjoint leaf atom (iv2), a nested member (v), an `owning` result with a body (vi2) and a struct
     global and its leaf (vii) each verify with the clause written by hand on today's encoder. The pass adds no new
     Viper shape for contracts; what is new is that the user need not write it, and items 4–7.
   - **Recursively** (question 191). A clause on `L` whose pointee structure has owning fields is itself a plain block
     site of the first list, so it closes in turn. Item 2's acyclicity makes the closure finite. S40 (f) verifies a
     chain `a->b->x` with a struct-local caller in 4.5 s in both modes, so pre-agreed cut 3 is not taken.
   - **`reads` and `live` close nothing.** `reads(*p)` gives the field's value only; a borrower that reads `*p->q`
     writes `reads(*p->q)` (0045).
   - **Marked sites close nothing.** A cell without a value owns nothing: `malloc`'s typed view, a `ref_to_uninit`
     result, `free`'s parameter and the `requires` side of a `must_init` parameter carry no clause. For `free` this is
     also soundness (audit N4): a clause there would consume `p->q`'s token without freeing it, and no row would see
     the leak. A `must_init` callee given a structure whose field already holds a block leaves that block with the
     caller, whose end row reports it; the callee cannot read the old value (`uninitialized`). The `ensures` side
     of a `must_init` parameter carries it as a given site (0054 item 2): this is the initializer `void s_init(struct S
     *p CERASTES_MUST_INIT) [[cerastes::requires(owns(*p))]] [[cerastes::ensures(owns(*p))]]`, whose `requires(owns(*p))`
     0042 item 4 asks for. `ref_to_uninit` on a parameter or result whose pointee has an owning field is rejected
     (pinned; d2b2c's strict rules decide it).
   - **Allocation views close nothing.** Every zeroed view (`calloc`'s and `vlibc_calloc`'s, audit N6) is a plain
     block site whose cells are zero or `NULL`, so its owning fields hold nothing and the clause would be vacuous; it
     also keeps `calloc(1, sizeof(struct S))` from being a closure through a bodiless contract's result. Leaving a
     clause out on the given side only gives the caller less, which is sound.
   - **The closure is plain** (0054 item 2's mode; audit N2). A field's buffer must hold values wherever the
     structure is owned in a contract: a structure whose `q` points to a `malloc`ed buffer not yet written cannot be
     passed to a borrower (`uninitialized` at the call, "needs `*p->q` to have a value"). `ref_to_uninit` on a field
     (M4.5d2b2c) will lift it.
   - **Merging and allocators** (0054 item 5). The clause names the platform's allocator. A user who writes the clause
     beside the atom, in exactly this shape (`L == NULL || owns_block(L)`) or with `cerastes_owns_vlibc_block(L)`, has
     it merged: the written clause takes the closure's place. Any other block site on `L` beside an atom that covers it
     is rejected ("`owns(*p)` already includes the block of owning field `p->q`").
   - **Bodiless contracts.** 0054 item 2's tie applies to every clause: a bodiless borrow of a structure with an owning
     field writes `p->q == old(p->q)` (`permission` in `f__wf` otherwise; d2b2c's `not_invalidating` will imply it).
     The tie is per clause, so a chain needs one per level, under the partner's guard (audit N5): `a->b ==
     old(a->b) && (a->b == NULL || a->b->x == old(a->b->x))`.
     **A bodiless initializer** (a `must_init` parameter whose `ensures` closes, with no body in the unit) gives a
     block through a load with no `requires` partner, which 0052 item 2a and 0054 item 2 reject in a bodiless
     contract (`unsupported_annotation`, naming M6's contract manifest; S40 `c/`, the same message a user writing the
     clause by hand gets today). The pass reports it as such, naming the owning field. A bodied initializer verifies
     (S40 `b/`, `c/`).
     **A bodiless contract's fresh result** (audit S2). A token inhaled from a bodiless `ensures` is distinct from
     every block the caller holds (0052 item 2a's own argument). What no bodiless contract can state is the pairwise
     distinctness of two blocks it gives, which `f__wf` asserts (0045 item 5, 0054 item 2). So:
     - a result that is itself a block site (`owning`, or `owns_block(r)`) whose closure gives a block: two given
       blocks (`r`'s and `r->q`'s), **rejected** (`unsupported_annotation`, naming M6's contract manifest). Written by
       hand today the contract fails at its own head (`permission`: "the contract of `make` returns a block and `r->q`
       as owned blocks, but they may be the same block", S40 `q/vi4.c`), so the rejection names the cause at once;
     - a partnerless `owns(*r)` result (not a block site) whose closure gives exactly one block (`r->q`): **accepted**
       under 0052 item 2a (S40 `q/vi3.c` verifies in both modes with the clause by hand);
     - a closure under `owns(*r)` that gives two or more blocks (two owning fields, a chain): **rejected**, as the
       first case.
   - **Where it is built.** One pass after the atoms are read and before `finish_atoms`, beside `apply_owning_globals`,
     so that `finish_atoms`' duplicate and "beside" rules see the clauses. It handles guarded atoms (fact (b)). The
     clauses are ordinary atoms of the contract from there on, so **every consumer of a callee's atoms sees them**
     (audit S1; a consumer that read only the written atoms would let a leak or an unsequenced access verify): 0047
     item 7's heap-program test (a program that names a type with an owning field is a heap program, so `tok` is
     declared and the end row runs even with no `malloc` in the unit), the back-edge leak trigger ("the body can call a
     function with an `ensures` block site"), 0054 item 4's "the loop has a written site" (a clause added to a written
     invariant counts), the sequencing registration of 0047 item 9 and 0039 item 4, and 0038 item 3's havoc and touch
     rules. Must-fail tests written without the clause pin each: a loop that calls an initializer and drops the block
     at the back edge, a written `owns(*p)` invariant whose body drops `p->q`, and `int *t = p->q; int x = *t +
     refill(p);` (`unsequenced`).
   - **Never at `program_start` or `main`'s entry** (audit S7), as 0054 item 5 says of the deep atom: a struct global
     of unknown value (a C tentative definition, a C++ global with external linkage) gives no token at entry, so `main`
     assigns its owning leaves before a callee whose closure needs them ("calls `f`, which needs to own the whole
     allocation `g.q` points to"; pinned).
   - **Covered atoms** (audit S6; audit round 2 on Parts B and C, S3). An output atom that the implicit `owns(s)` of a
     `static` struct can cover (0052 item 2's `return &s;` with `ensures(r, owns(*r))`, 0054 item 7's leaf cover)
     carries **no** closure: the block of `s.q` travels with the implicit atom, and a closure beside it would hand it
     over twice. The first build put the closure under `!cov`, as the atom's value facts are; it was right, but two
     calls of such a function in one caller took Silicon past 170 s (`p = where(); p->n = 1; p2 = where();`, against
     10 s without the clause). So the closure is dropped where a cover is possible. This is sound: a function that
     returns a fresh structure on some path, whose owning field holds a block there, still holds that block at its
     return and is reported (a false alarm for that rare shape; recorded for the owner, handoff question 194). The
     caller of a function that returns `&s` neither receives nor frees `r->q`'s block: the static owner keeps it, and
     `main` may end holding it.

4. **Stores to an owning field through a pointer** (`p->q = x`, `p->t.q = x`, `(*pp)->q = x`, inside an expression
   too; amends 0043 item 9).
   - **The value rule** (`permission`, at the store): `x != NULL ==>` the function holds `x`'s block whole (0052 item
     3: `offset_of(x) == 0` and the token at `write`). Message: "owning field `p->q` receives a pointer that is not the
     start of a block the function holds". A null pointer constant needs no row.
   - **The flag** (fact (f)). A row that reads the field's old value reads it only under the leaf's flag.
   - **A dropped block** (fact (e); decided by S40 (d)). **No row** at a store through a pointer or at `free(p)`. The
     end-of-function leak row (0047 item 7) reports a block dropped through `p->q = …` or by `free(p)` while a field
     holds it, at the function's closing line ("`f` may return while it still owns an allocation that its `ensures`
     does not hand back"), and the docs say so. The preferred rows (a `leak` row at the store, "assigning to owning field `p->q` drops the block it
     holds", and at `free(p)`) were built by hand in S40 and raise false alarms on four correct programs, because
     nothing names the other owner of a block reached through a pointer (fact (e)):
     - the transfer `free(b->q); b->q = a->q; a->q = NULL;` (at `a->q = NULL`);
     - the reallocation `free(p->q); t = malloc(…); p->q = NULL; free(t);`;
     - taking the block into a local, `t = p->q; p->q = NULL; free(t);`;
     - `t = p->q; free(p); free(t);` for the `free(p)` row.

     The fixtures `tests/viper-smoke/memory/s40_d_*` keep the evidence. A row that names the other owner needs a
     ghost per heap object, which is M4.5f's predicate machinery, not a row. This is a design decision, not
     pre-agreed cut 2: nothing is deferred to d2b2c. Two positions the docs give (audit N1): a path that ends in a
     `noreturn` call is not checked for leaks (0047 item 7), and a drop under a current-value loop carrier is reported
     at the loop's back edge, not at the closing line.
   - **A store through an alias** (`int **pp = &p->q; *pp = x;`) gets no row, as for an owning global (0054 item 5):
     the closure's return row or the end leak row reports it.

5. **Owning leaves at named places** (amends 0044 items 3, 4 and 8; 0052 item 3; 0054 items 5 and 6): a struct local
   (a by-value parameter's copy and C's `&(T){…}` object included), a struct global, a `static` struct local.
   - **Owners in memory.** Each owning leaf is an owner in memory by 0054 items 5 and 6, keyed by its variable and
     member path.
   - **Re-keying first.** The owner identity of fact (d) becomes one type — the root (`Var*` or `Global*`), the leaf
     path, the held ghost's name, the read of the value — in a commit that changes no output (`vprdiff.py`: none
     differ).
   - **Ghost** `held_<var>_<path>`: false at a local's declaration; at method entry true exactly where a `requires`
     site gives the leaf's block (a clause of item 3 on the leaf, or a written site).
   - **Rows at stores by name** (`s.q = x`, `s.t.q = x`, `g.q = x`): the value rule of item 4 and 0052 item 3's
     reassignment row (`leak`: "assigning to owning field `s.q` still holds its block", 0052's wording), reading the old value under
     the leaf's flag. After every call and every `free`, the ghost is updated framed, as 0054 item 5 does for an owning
     global. **The two updates** (audit S3): after a callee whose `ensures` closes over the leaf (a borrow of `&s`, a
     call that takes `owns(g)`), the plain `held := L != NULL`, under the atom's guard; after every other call
     (including one through an alias of `&s`) and every `free`, 0054 item 5's framed form `held := held && perm(<L's
     cell>) > none && L != NULL && perm(tok(block_of(L))) > none`. So `free(s.q); s.q = malloc(…);` verifies, and a
     callee that consumed `s`'s cells clears `held` before the lifetime row reads `L`. A named leaf has its ghost, so
     here the row is 0052's and behaves as it does for an owning local
     (`t = s.q; s.q = NULL; free(t);` is the reassignment row's `leak`, as `t = v; v = NULL;` is for an owning local
     `v`, S40 (d) `own_local_take.c`; audit N3: named leaves follow P4417, pointer leaves memory safety; pinned). S40
     (c) verifies the hand-written ghost and lifetime row on a struct local passed by `&s` to an initializer, a filler
     and a clearer, and each row's twin fails at it (`tests/viper-smoke/memory/s40_c_*`). Its evidence is narrower than
     this item (audit S3): its rows are `perm(tok) == none` rather than 0052's owner list, its passing program never
     stores `s.q` by name, and its ghost update is the plain one only. The session's tests cover the rest.
   - **Lifetime end** of a struct local: 0052 item 3's ending row at each exit from its scope (`leak`, at the exit:
     "owning field `s.q` still holds its block", worded as 0052's "owning `p` still holds its block").
   - **Globals and `static` locals.** An owning leaf's initializer is absent or a null pointer constant
     (`unsupported_annotation` otherwise: "an owning field starts null; it owns only what it is given"; fact (h)).
     `main` starts with every leaf's `held` false (0054 item 5's `program_start` rule). `main`'s leak row exempts the
     leaves of struct globals and `static` struct locals under their `held` ghosts (question 174), by 0054 item 8's
     `program_start` tag extended with the member path: the term `held_<g>_<path> && t == loc(block_of(<leaf>), −1)`
     on the leaf's cell. S40 (h) verifies it and its twin (another block still leaked) fails
     (`tests/viper-smoke/memory/s40_h_*`). **Deeper owners** (audit S4; P4417 Note 3): a static owner owns what its
     owning fields own, so the exemption also covers the blocks reached through owning leaves from a static owner — a
     chain `g.q->r` and the owning field `gp->q` of an owning `struct S *` global — each term framed by the cells it
     reads, since `main` can hold a block's token without its cells: `held_g_q && g.q != NULL && perm(<g.q->r's cell>)
     > none && t == loc(block_of(g.q->r), −1)`. (Fallback if the framing cannot be proved: a documented limit, pinned.)
   - **`&s` to a callee:** the callee's contract decides, by its closure: an `owns(*p)` borrow takes and returns the
     leaf's block, and then `held := s.q != NULL` read framed; a consuming `owns_block(p)` cannot take a struct
     local's address (0047: not the start of a heap block).
   - **`&s.q` as a `T**`** (audit question 3(b), argued). No row at a store through it, as for a global (0054 item 5),
     and this is sound: a dropped block keeps its token and is not exempt, so the end row reports it; the exemptions
     (`main`'s term, the back-edge carriers) match on the current value and need the carried token; the lifetime row
     reads `s.q` only under `held`, which a callee that consumed `s`'s cells has cleared through the framed update.
     0054 item 6's guarded rows (`pp == &s.q`, provable while the function holds `s`'s cells) would add precision only.

6. **Copies and struct values** (question 190; amends 0043 item 7, 0044 items 3 and 4).
   - **A copy** (`t = s;`, `*p = *q;`, an initializer from an object) is its leaf stores (fact (g)). Each owning leaf
     of the destination is a store to an owner: item 5's rows at a named leaf, item 4's through a pointer. Nothing is
     cleared, so source and destination then hold one block, as two owning locals do after `b = a;` (0052 item 3).
     While both live, each one's exit row accepts the other where the other outlives the exit (two struct locals
     that end at the same `}` do not accept each other: 0052 item 3's owner list is the owners that outlive the
     exit). S40 (e) ran the base encoder, which has no ghosts, so the rows at copies are tested by the session's tests,
     not spiked. Where both are owned across a call or a return, two
     sites name one block and the row is `permission`; the program sets one to `NULL` first. A copy from a structure
     the function only reads (`struct S t = *p;` under `reads(*p)`) fails the value rule at the copy: it cannot make a
     second owner of a block it does not hold. A view of the buffer is a plain pointer, read like any pointer from
     memory: `int *v = p->q;` under `reads(*p)` needs `live(p->q)` (or `reads(*p->q)`) beside it, or the read is
     `lifetime` (0036), as it is today without an owning field.
   - **A by-value parameter or result** is the tuple of its leaves (0044) and owns nothing. It carries a block only
     where its contract says `owns_block(s.q)` or `owns_block(r.q)` (0054 item 2). The parameter's copy in the callee
     is a struct local whose leaf's `held` starts true exactly there. `f(s)` without such a site passes an alias.
   - C++ `S{…}` values and compound-literal values likewise own nothing.

7. **Loops** (question 189).
   - The closure's `requires` sites are automatic loop places (0054 item 2), entry-keyed. A loop that replaces a
     field's block writes the invariant.
   - A written `owns(*p)` invariant on a structure with owning fields carries the clause, as a written `owns_block`
     invariant (0054 item 4).
   - Named leaves get 0054 item 5's synthetic loop sites and 0054 item 3's carriers, on the re-keyed owner. `check.cpp`
     admits a block site rooted at a struct local's leaf in an invariant (fact (h)). Today such an invariant is
     rejected, and the same loop without it is a false `permission` ("writes `*s.q` without owning it", S40 (g)
     `g2_named*.c`); this item lifts both. (Pre-agreed cut 1: loops that write a named leaf are rejected at the loop,
     naming d2b2c.)
   - **As built (Part D).** A loop reaches a named leaf when it names its structure, stores through a pointer of the
     structure's type, or calls a function whose contract names cells of that type. A reached leaf gets a synthetic
     site `s.q == NULL || owns_block(s.q)` (a written site on it takes precedence) and its ghost is carried by the
     invariant `held == (s.q != NULL)`, with entry and back-edge rows (`permission`: "the loop needs owning field `s.q`
     to point to a block the function holds, or to be null"). An unreached leaf keeps its ghost in the loop's frame
     (audit round 2 on Parts B and C, B1: without it a loop with any call havocked the ghost, a false `leak` on an
     ordinary program and exit 3 on others).
   - **Owning locals and heap places of `struct S *`** (audit S5). An owning automatic local `struct S *v` (0052 item
     3) gets no closure at its declaration (its blocks are the function's, backstopped by the end row), but its loop
     carries the synthetic site `v == NULL || (v->q == NULL || owns_block(v->q) && …)`, so a loop that frees or
     replaces `v->q` verifies. **Limit:** an owning `struct S *` global's field in a loop (`gp->q` freed and replaced)
     is a false `permission` ("this loop invariant reads `gp->q`, which this loop does not hold"): the loop's places do
     not include the cells of an owning global pointer's block, and a written invariant fails the same way (pinned,
     `owning_field_loop_global_pointer_false_alarm.c`; handoff, for M4.5d2b2c or M4.8).
   - **Further limits, each a pinned false alarm** (the re-audit; none hides an error, since a carrier or a frame can
     only add checks):
     - a loop through a non-owning heap alias of a borrowed structure (`v = p;` then `free(v->q)`): 0047 item 10's
       heap place does not carry its pointee's closure (`owning_field_loop_alias_false_alarm.c`);
     - a call through an alias `ps = &s` in a loop (`refill(ps)`): the leaf's ghost is not refreshed
       (`owning_field_loop_alias_call_false_alarm.c`);
     - in `main`, a struct global's leaf that the loop reaches only by type is not carried, since `program_start` does
       not fix a global's value; outside `main` it is carried where the function holds it at entry
       (`owning_field_loop_main_holds_false_alarm.c`);
     - a leaf carried by type, or an owning local that may be null at the loop's entry, loses its contents' flag when
       the loop neither stores that type nor passes the owner on: a later read is `uninitialized`
       (`owning_field_loop_pointer_arg_false_alarm.c`, `owning_field_loop_owning_local_maybe_null_false_alarm.c`);
     - a chain of three owning levels in a loop: one level of closure is carried
       (`owning_field_loop_chain3_false_alarm.c`);
     - a loop that may replace the owner itself: an owning `struct S *v` assigned a non-null value (also inside an
       expression, or through a pointer of its type), a leaf stored by name with a non-null value, its address passed
       to a call, or, for an address-taken owning local, any call in the loop. No closure is carried across the
       replacement, so freeing the field inside such a loop is reported (`owning_field_loop_owner_replaced*` and
       `owning_field_loop_owner_renewed_false_alarm.c`; the focused review of the re-audit's fixes, whose probes
       showed that a closure carried across a replacement asserted a flag of the wrong object: exit 3).
   - **Fixes from the re-audit** (as built): a covered closure site has no leak exemption either (B1); a global leaf
     that a loop neither uses nor reaches through a callee stays in the frame, and one it reaches by type is carried
     outside `main` where the function holds it, with the invariant `known ==> (held == (L != NULL))`, `!known ==>
     !held` over an entry snapshot; every read of a leaf owner's value that rests on `held` is also guarded by
     `perm(cell) > none` (B2); an assigned owning local's synthetic closure stands only where its cell has a value
     (S1), and is not built at all where the loop may replace the owner (the focused review: a snapshot of the flag
     cannot follow a replaced object); a struct local's leaf whose pointee has owning fields carries one level of
     closure (S2).
   - **Cost** (S40 (g), loaded machine). One owning field replaced under the written invariant: 6.3 s; a loop that only
     reads through the field: 3.0 s. Two owning fields replaced under two written invariants: **218.5 s**, worse than
     0054's 33–148 s for two load sites. The tests pin one owning field per loop, and the two-field figure goes to the
     handoff for M4.8 (question 189).

8. **P4417R0's `[[owner]]` in cerastes** (question 175; docs only).

   | P4417R0 | cerastes |
   |---|---|
   | `[[owner]]` on a variable (automatic, static, global) | `cerastes::owning` (0052 item 3, 0054 items 5 and 6) |
   | `[[owner]]` on a field | `cerastes::owning` (this record) |
   | `[[owner]]` on a result | `cerastes::owning` on the function (0047) |
   | `[[owner]]` on a parameter (a transfer: the callee deletes) | `requires(p == NULL \|\| owns_block(p))` with no `ensures` partner |
   | (none: a borrow) | `cerastes::owning` on a parameter (0040; question 175) |
   | a static owner need not delete (p5 Note 3) | exempt at `main`'s end (question 174) |
   | raw storage outside the profile (§8.2) | checked: `malloc`/`free` blocks are owned like any other |

9. **The lint** (`lint_viper.py`): the leaf ghosts and their assignments (a leaf cell in place of `GB_x`'s); `main`'s
   exemption terms for struct leaves, admitted only for a leaf the `program_start` tag names (0054 item 8's tag
   extended with the member path); the rows of item 4, if built.

10. **Categories:** none new. `leak` and `permission` gain the messages of items 3–6.

11. **Still rejected** (fail closed; one pin each, naming the session):
    - recursive owning fields (M4.5f2);
    - a bodiless initializer of a structure with an owning field (item 3; M6);
    - `owning(n)` on a field (M4.5e1);
    - `ref_to_uninit` on a field (EDG's #1835; d2b2c);
    - `ref_to_uninit` on a pointer to a structure with an owning field (d2b2c);
    - a block site of another shape beside an atom that covers the leaf;
    - a closure through a bodiless contract's result that gives two or more blocks (item 3; M6);
    - a non-null initializer of an owning leaf of a struct global or `static` struct local;
    - an owning field of a union, a bit-field.

    **Every other context** (the session's survey of each reader of struct leaves and atoms in `encode.cpp`,
    `check.cpp` and the lowering). Ownership is accounted by token sites, not by pointer values: a value dropped or
    stored without a row leaves its block held, and the end-of-function leak row (0047 item 7) reports it, while a
    closure clause at a call or a return is checked as a site. Items 4–6's rows add precision and the named-leaf
    ghosts; they are not the only backstop. So:
    - **Already rejected, unchanged:** a union (now `unsupported_annotation` at an owning field, the union still
      rejected), an array member and an array of structures (M4.5e1 meets owning structures there), a `const` or
      `volatile` member, `pure` and `predicate` bodies (not encoded until M4.5f1), C++ classes with constructors,
      virtual functions, default member initializers or non-trivial special members (so an owning field in C++ is in an
      aggregate with a trivial copy).
    - **Sound with no rule, because they read or pass values and move no token:** member reads (`s.q`, `p->q`) and
      contract reads and `old` of a field; by-value tuples (parameters, results, `S{…}`, compound-literal values; item
      6); the leaf plumbing of the lowering, which sees an owning leaf as a pointer leaf; `restrict` (separation between
      parameters; a clause is one more site); `check.cpp`'s type checks of stores and values; sequencing keys.
    - **A struct local's allocation** states `!flag` on every leaf, which matches item 5's `held` starting false.
    - **A struct global of unknown initial value** (external linkage, no initializer) keeps its leaves `Unknown`
      (0041 item 3); `main` starts with each owning leaf's `held` false whatever the value (0054 item 5, audit B2).

12. **Facts relied on:** P74 (b), P76 and S40 (`internal/il-inventory.md`, chapter M4.5d2b2b; kept fixtures
    `tests/viper-smoke/memory/s40_*`).

## Consequences

- A structure can own its buffers: `owns(*p)` in a contract means the structure and every block its owning fields hold,
  at any acyclic depth, with checks at each store to a field.
- Struct locals, globals and `static` locals with owning fields are owners in memory, with 0052's and 0054's rows.
- P4417R0's `[[owner]]` positions each have a cerastes spelling (item 8).
- **Left for M4.5d2b2c:** `cerastes::not_invalidating`, `--strict-owner`, `ref_to_uninit` on a field, and this
  session's cuts 1 and 2 if taken. Recursive owning fields are M4.5f2's.
