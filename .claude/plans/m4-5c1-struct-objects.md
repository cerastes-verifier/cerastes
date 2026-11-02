# Plan: M4.5c1 — struct objects (record 0043); M4.5c2 split off

Approved by the owner on 2026-11-02 (planning in plan mode; questions 90–93 answered: 90, 92, 93 with the recommendations; 91 moves owning fields to M4.5d2 together with the rest of P4418R0's invalidation profile).

Planned on 2026-11-02 in plan mode. The owner answered questions 90–93 (below). Branch `m4.5c1-struct-objects`,
worktree `.claude/worktrees/m4.5c1`; the baseline is taken in the detached worktree `.claude/worktrees/m4.5c1-base`, never
in the main checkout. Saved at step 1 as `.claude/plans/m4-5c1-struct-objects.md`.

## Context

**State.** `origin/main` is `38877b4` (through M4.5b2a). M4.5b2b is pushed as `m4.5b2b-init-markers` (`1bfa705`), but its
PR has not merged yet. The local `main` `853fccd` is a local merge of it. The old worktrees `.claude/worktrees/m4.5b2b` and
`-base` stay as they are: removing them is the owner's step. The EDG pin is unchanged, and this session needs no fork
work.

**What is rejected today.** `plan.md`'s M4.5c row is all rejected:
- every class, struct or union declaration (`encode.cpp:6778`);
- every struct type and pointer to one (`vtype`, `encode.cpp:459-492`);
- `.` and `->` (`operation()`'s default, `:3518`);
- `&s.x` (`address_of`, `:3366`);
- member assignment (`lvalue_var`, `:3910`);
- aggregate initializers (`constant_value`, `:1320`);
- contract places other than `*param`, a global or `*result` (`spec_atom`, `contract_read`, `old_call`, `old_c`: `:2649-3049`).

VIR's `Type` is `Void|Bool|Int|Ptr` (`vir.h:66`). Every cell, atom, loop `Place`, `Acc` and scope exit in `lower.cpp` holds
exactly one value/flag field pair, and every separation and framing filter compares `cell_suffix` of the atom's pointee
(`exclusive_pair`, `lower.cpp:1342`). The prelude already declares `member`, `parent_of` and `index_of` with their axioms
`member_inv` and `member_lv` (records 0035 and 0036, `prelude.cpp:148-159`), and `lint_viper.py` pins them. The target
encoding is record 0028 item 16, which the fixtures `tests/viper-smoke/memory/s26_member_*.vpr` pin.

**Size.** The whole row is about 3,000 or more changed lines, which does not fit one context (question 77). It is split in
two (question 90):
- **M4.5c1** (this session): struct objects in memory.
- **M4.5c2** (next): struct values across calls, struct globals, and places reached through a heap read.

The c1 estimate is about 1,800–2,400 changed lines and about 90 tests. Copies are the pre-agreed cut point.

**What M4.5c1 lands:**
- C structures as types;
- struct locals as blocks;
- `.`, `->` and `(*p).x`, with nested structs;
- pointers to members (`&s.x`, `&p->t`);
- braced initializers;
- copies inside a function;
- `struct S *` locals, parameters, results and pointer cells;
- contract atoms and values on struct places (`owns(*p)`, `reads(*p)`, `live(p)`, `owns(p->x)`, `p->x`, `old(p->x)`);
- P4222's markers on struct objects (record 0042's scope row);
- loops, scope exits, calls, returns and sequencing over member cells;
- the ports `malloc_block.c` and `swap.c`'s `point_mirror`.

## Owner decisions

**From this planning (2026-11-02):**
- **90, the split:** c1 is struct objects, c2 is struct values; this was the recommendation.
  - **c1:** everything listed above, with copies inside a function.
  - **c2:** by-value struct parameters and results (a struct value as the tuple of its members), `f().x`, struct globals
    and `static` struct locals, and places reached through a heap read. Those places are `owns(**pp)`, `reads(**pp)`,
    `live(*pp)`, reads of `**pp` and `*gp`, `owns(*gp)`, `*s->q` of a pointer member, and an atom on `*q` where `q` is a cell.
  - **The cut:** if context runs short after commit 2, copies move to c2.
- **91, owning fields:** they move to **M4.5d2**, beside `owns_block`, owning locals, globals and results. d2 also covers
  **the rest of P4418R0's invalidation profile** (Stroustrup, 2026-10-22,
  https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4418r0):
  - its three guarantees: a dereference only of an existing object; no pointer outliving its object; a non-static owner
    freed exactly once;
  - `[[owner]]`'s transfer rules (§8): owner to owner moves the obligation, owner to non-owner makes an alias, and an
    allocation's result is an owner;
  - `[[not_invalidating]]`;
  - `no_dangling()`;
  - C++'s parts (references, containers, iterators, `new`/`delete`, smart pointers) are M7's.
  - d2's planning decides the `cerastes::` spellings (as 0029 did for P4222) and what each adds over contracts that return
    their permissions. Its record cites P4418R0 and its wording paper P4417R0.
- **92, copies:** a copy reads every member, so an uninitialized one is `uninitialized` at the copy, path by path; this was
  the recommendation. That is stricter than C. It keeps record 0042's `old(flag) ==> flag` row, `lint_flags`, and the loop
  row `held && flag ==> range` as they are.
- **93, flips:** (a)–(c) are pre-approved, listed under Flips below. Nothing else may change.

**Standing:**
- 77: a session fits one context.
- 69: the EDG merge waits for the box session.
- 74: places through a heap read are now c2's.
- 78: `const` objects are M4.6's.
- 82–89: record 0042.

## Step 1 (first, before any code): consolidate the plan documents

The first commit is markdown only.

1. **Worktree:** `git worktree add .claude/worktrees/m4.5c1 -b m4.5c1-struct-objects m4.5b2b-init-markers`.
   - It branches from `1bfa705`, b2b's pushed tip, not from the local merge `853fccd`. So its PR stacks on b2b's PR and
     targets `main` once b2b merges.
   - Submodules are cloned with `--reference` to the main checkout's stores (memory `background-worktree-setup`).
   - Then `scripts/bootstrap.sh`.
2. **The plan:** save this file as `.claude/plans/m4-5c1-struct-objects.md`, with the "Approved" line and the Inputs below.
3. **`internal/handoff.md`** (header: one pointer to `git show 1bfa705:internal/handoff.md`):
   - **§1–2:** b2b is on its PR and c1 is on its branch. There is no owner step until the end, apart from b2b's PR.
   - **§3:**
     - The M4.5c notes split. The copies' flag note and the loop-row note go into this plan's Inputs.
     - The heap-read note stays, retitled M4.5c2.
     - `owning`'s `d` moves to M4.5d2, with question 91.
     - The `&s.x < &s.y` completeness gap moves to M4.5e2, where relational comparisons land.
     - New M4.5d2 note: 0029 and 0042 cite P4222R2, and P4418R0 cites P4222R3 (2026-10-22). Check R3 against them at
       d2's planning.
   - **§4:**
     - 0043 is this session's record, and 0044 the next free one.
     - §4.2: the M4.5c paragraph becomes c1 (a pointer to this plan) and c2 (its scope from question 90). d2's paragraph
       gains owning fields and P4418R0 (question 91). e1 notes that arrays of structs need c1's member blocks.
   - **§8:** questions 90–93 are answered; none is open.
4. **`plan.md`:**
   - the M4.5 table's c row becomes two rows, c1 and c2;
   - the d2 row gains owning fields (target `d`) and the rest of P4418R0, with needs "M4.5c2, M4.5d1";
   - f1's needs become "M4.5c2, M4.5d2";
   - the header's "the last" becomes M4.5c1.
5. **Commit 1 (planning):** local, with no Claude trailer.

## Inputs from the handoff (moved into the plan at step 1)

- **Struct copies must not clear a held cell's flag.** 0042's `internal` row `(g_r && old(flag)) ==> flag` relies on it
  (0042 item 5, "Why"). With question 92 a copy only ever sets flags.
- **Copies and member writes keep 0042 item 5's loop row inductive.** That row is `held && flag ==> range` on every loop
  place (question 89).
- **The registration rule** (0029 item 1): nothing new is registered in c1. `owning`'s `d` waits for d2, so EDG's own
  refusal of `owning` on a field stays, and a new pin names d2.
- **P51** (`il-inventory.md:4655, 4706-4718`):
  - `&s.f` is `address_of(dot_field(var s, field f))`;
  - `p->f` is `points_to_field`;
  - `(*p).f` is `dot_field(indirect(var p))`;
  - a struct copy is one `assign` node with no memcpy;
  - `x.i` under `&` marks `x` as address-taken, and `p->i` does not.
  P53 and P58 add that `cerastes_owns(s->x)` is a `points_to_field` and `cerastes_old(s->x)` is
  `address_of(points_to_field)`. There is no probe yet of initializer shapes, C++ copies, or `.` without `&`.
- **0037 item 2's `restrict` argument** ("parameters of another pointee type never reach `p`'s cell") stops holding: an
  `int *q` can point at `s->x`. Item 10 below decides what follows.

## Setup (in parallel with step 1)

- **Baseline:** in `.claude/worktrees/m4.5c1-base`, detached at `1bfa705`, in the background:
  1. `bootstrap.sh`;
  2. `run_tests.py -j 8`;
  3. the VIR self-test;
  4. copy the results into the session worktree's `build/m4.5c1-baseline-{vpr,vir}`.
- **Builds:** `lockf build/m4.5c1/build.lock cmake --build build/cmake`, never while `check.sh` runs.

## Record 0043 — struct objects

The record is written before code (invariant 6), while P65 and P66 run. Audit round 1 runs on it in the background.

1. **Scope table** (c1 | c2 | d2 | e1 | M4.6 | M4.7 | M7 | M8). It amends:
   - 0028 items 1 and 16 (struct locals always blocks; item 4);
   - 0029 item 1 (owning fields to d2, question 91);
   - 0037 items 2, 5, 7, 8 and 10;
   - 0038 item 3;
   - 0039 item 3;
   - 0040 items 6 and 9 (rows renamed to c2 or d2);
   - 0041's scope row (struct globals to c2);
   - 0042's scope row (structs as marked objects: this record).
2. **Which structures.** Accepted: a complete `struct` defined at file scope of a verified file. Each member must be one
   of:
   - a `bool`, an integer or an enumeration that 0032 accepts;
   - a pointer that 0040 accepts, with "innermost pointee" widened to an accepted struct, so a self-referential
     `struct node *next` is a pointer cell;
   - a nested accepted struct.

   In C++20 the same C-like aggregates are accepted, with only implicitly declared trivial special members.

   Rejected, each with its own message naming its session:
   - unions (M8);
   - bit-fields (M8);
   - array and flexible array members (M4.5e1);
   - `const` members (M4.6);
   - `volatile` and `_Atomic` members (M8);
   - anonymous struct and union members (M8);
   - `void *` members (0028 item 1);
   - pointer-to-function members (M4.7);
   - empty structs;
   - structures from system headers (`FILE`, as 0018 treats system enumerations);
   - a block-scope struct definition (M8);
   - every C++ class feature: member functions, constructors, bases, access specifiers, `static` members, default member
     initializers, references (M7);
   - templates (M5);
   - an incomplete struct dereferenced or used as an object.
3. **Layout** (0028 item 16, S26):
   - A struct object is a slot `r`. Member `k` is the block `member(r, k)`, of length 1.
   - A scalar or pointer member is the leaf cell `loc(member(r, k), 0)`, with the fields of its leaf type.
   - A nested struct member is the slot `loc(member(r, k), 0)`, whose members are blocks of their own.
   - A struct type has no value field. Every leaf shares the root's liveness cell (`member_lv`).
   - Pointer-to-struct fields are named like enumerations: `ptr_struct_<n>_<tag>`, from `cell_suffix`.
4. **Struct locals are always blocks**, whether or not `&s` is written. This amends 0028 item 1 for structs only.
   - **Why:** one layout (item 16). It also reuses the allocation, scope-exit and loop machinery. EDG marks `s` on `&s.x`
     anyway (P51), and no existing program's Viper changes.
   - **Allocation:** each is allocated at block entry by a trusted `alloc_struct_<n>_<tag>()`. It gives the root facts,
     `len(member(…, k)) == 1` for every member block, and full permission to every leaf cell with its flag false. It also
     gives the liveness cell.
   - **Scope exit:** checks every leaf cell and the liveness cell at `write` (`lifetime`), then exhales them.
5. **Member access in code.**
   - `s.x`, `p->x`, `(*p).x`, `s.t.x` and `p->t.x` are leaf places.
   - `p->x` checks, in order, each with its own category:
     - `p != null` (`null-dereference`);
     - the leaf's permission (`permission`, "reads `p->x` without owning it");
     - for a read, the flag (`uninitialized`).
   - Like `*p`, a member access is not a use of `p`'s value, so it needs no liveness check (0032 item 6).
   - **`&s.x`, `&p->x`, `&s.t`, `&s`** are pointer values. Forming `&p->x` is arithmetic on `p`, so it checks `p != null`
     and `p`'s liveness (0028 item 3a).
   - **Writes:** `=`, compound assignment, `++`/`--` and `StoreE` inside expressions work as they do for cells (0039 item 2).
   - **Still rejected:**
     - `&s.x + 1` and relational comparisons of member pointers (M4.5e2);
     - conversions between struct pointer types, and from a struct pointer to another object pointer (0028 item 1);
     - `offsetof` and `sizeof` of a struct where they do not fold to a constant (P65 decides).
6. **Initializers** of struct locals:
   - **Accepted forms:** positional and designated braced lists (C++20 designators in order), nested braces, `{}` and
     `{0}`, and C++ `S s{}`.
   - **Values:** unmentioned members are zero or null, as for an object with static storage. Each leaf store sets its
     flag.
   - **From another object:** `struct S t = s;` and `= *p` are copies (item 7).
   - **Evaluation order:** P65 settles EDG's element order in both modes. In C the elements are indeterminately sequenced
     (C's initialization rules), so they take 0039's and 0021's rules as one full expression's operands. In C++ they are sequenced left
     to right.
7. **Copies** (question 92):
   - **Forms:** `t = s`, `*p = *q`, `*p = s`, `s = *p`, `p->t = s.t`, and the initializers of item 6.
   - **Order:** first every source leaf is read, with permission and flag (`uninitialized`, naming the member). Then every
     destination leaf is written with its flag set. So an exact self-copy (`*p = *p`, which C's simple assignment allows) works, and partial
     overlap cannot arise between member blocks.
   - **Rejected:**
     - a copy used as a value (`(t = s).x`);
     - a C++ copy that the IL shows as a non-trivial call (M7).
8. **Struct pointers:**
   - **Where:** `struct S *` locals, parameters (`requires WF(p)`), results (`ensures WF(result)`) and pointer cells
     (`struct S **`, from 0040).
   - **Operations:** null, `==`/`!=`, truth tests, passing, and result case splits (0037 item 3; 0040 item 2) by the
     pointer's own field.
   - By-value struct parameters and results are rejected, naming M4.5c2.
9. **Contracts** (amends 0037 items 5–7, 10).
   - **Places:** `*p` of a `struct S *p` (the whole struct), `p->x`, `p->t` and `p->t.x` (also as `(*p).x`), and in an
     `ensures` `r->x` framed as 0040 item 4.
   - **Atoms:**
     - `owns(*p)`: every leaf of `*p`, recursively. For each it gives value and flag permission, the flag set and the range
       or `WF`. It also gives `len(member(…)) == 1` for every member block, `offset_of(p) < len(block_of(p))`, and one
       liveness fraction.
     - `reads(*p)`: the same with fractions.
     - `live(p)`.
     - `owns(p->x)`, `reads(p->x)` and `owns(p->t)` (a nested struct's leaves): one liveness fraction each, summed when
       several atoms share the root (S26's `swap_adjacent`).
     - `owning` on a `struct S *` parameter is 0040's borrow `p == NULL || owns(*p)`.
   - **Values:** `p->x`, and `old(p->x)` (`cerastes_old(p->x)`). `old(*p)` as a struct value is rejected, and so is
     `old(*p).x` unless P65 shows the IL makes it the same place.
   - **Site keys** are a root (formal, result) and a member path. Partners and "two atoms of one kind on one place" are
     decided by key and path.
   - **Soundness core: overlap by slots, never by the atom's pointee type.** An atom on `struct S *s` and one on `int *q`
     share a cell when `q == &s->x`. Two struct atoms share cells when one root is a nested slot of the other. So every
     rule now keyed by `cell_suffix` of the atom's pointee is keyed by each leaf's field. That covers:
     - call and return separation (`exclusive_pair`);
     - the sums (0037 item 6);
     - entry separation (item 9);
     - `f__wf`'s distinctness and equal-ghost facts;
     - framing disjuncts;
     - case splits;
     - loop places.

     Two atoms overlap iff a slot of one equals a slot of the same type of the other: a disjunction of `Ref` equalities.
     Framing of a leaf read by a struct atom is the disjunct `g && read_place == leaf`, one per covering leaf.
   - **The markers** (0042) on a `struct S *` parameter: `ref_to_uninit` drops every leaf's flag, and `must_init`
     promises every leaf (`postcondition`, naming the first missing member). `[[cerastes::uninit]]` works on struct
     locals, and `--strict-init` asks for it on a struct local without an initializer.
10. **`restrict`** (amends 0037 item 2):
    - It stays C-only and owned-only.
    - It is rejected on a struct pointer parameter (M8).
    - A `restrict` parameter of field `T` beside any pointer parameter whose pointee has a leaf of field `T` (a struct
      pointer included) must have its own atom, by the same rule as a same-typed parameter.
11. **Loops** (amends 0038 item 3):
    - Every leaf of every struct object in scope, and of every held struct place of a parameter, is an automatic place.
    - The liveness row is written once per distinct liveness cell (one per root block), never once per leaf.
    - The havoc rule (writes reach places by field) covers member leaves.
12. **Sequencing** (amends 0039 item 3):
    - Object keys gain member paths. `s.x` and `s.y` are different objects, and `s.x` twice is one object.
    - Shapes the compilers warn on are rejected `unsequenced_modification` (P66's matrix, 0020 item 5's union rule).
    - `p->x` against `q->x`, or against `*r`, is checked path-precise (`unsequenced`).
13. **Lint:**
    - `alloc_struct_*`'s shape is pinned structurally: the root facts, then for each leaf exactly `len(member(X, k)) == 1
      && acc(L.v_t) && acc(L.d_t) && !L.d_t` with `L = loc(member(X, k), 0)`.
    - Every receiver is rooted at `r`, every (receiver, field) pair appears once, and no other conjunct is allowed. A
      duplicated leaf would make the allocation inconsistent, which is a vacuity hazard.
    - `lint_flags` accepts member receivers, or lowering names them by `Ref` temporaries. Either way a flag is set only by
      `:= true` right after its value.
14. **Categories:** none new. Messages name the member place in source terms (`s.t.x`, `p->y`), as M4's acceptance asks.
15. **Still rejected**, each pinned:
    - c2's constructs, the rows renamed to M4.5c2: by-value struct parameters and results, struct globals and `static`
      struct locals, `owns(**pp)`, `reads(**pp)`, `live(*pp)`, `**pp`/`*gp` in contracts, invariants and measures,
      `owns(*gp)`, an atom on a cell pointer, compound literals;
    - `owning` on a field (EDG's refusal; M4.5d2);
    - item 2's list.
16. **Facts relied on:** P65, P66 and S33.
17. **Tests and documentation** (below).

## Team (disjoint files; three Sonnet agents and one Opus reviewer from `.claude/agents/`)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | **First, P65:** the IL in both modes of item 2's types and every rejected member kind; `.` with and without `&` (`address_taken`); every initializer shape of item 6 and its element order; C and C++ copies (`assign` against an implicit `operator=`); `sizeof` and `offsetof`; nested access; contract places (`cerastes_owns(*s)`, `(s->x)`, `(s->t)`, `old(s->x)`, `old(*s).x`); the markers on `struct S *`; `uninit` on a struct local. **Then S33:** test-mutator's fixtures through Silicon, with vacuity runs. **Then:** `src/vir/lower.cpp` and `src/viper/` (layout, allocation, scope exits, member places, atom expansion, leaf-keyed separation, framing, sums, splits, loops, sequencing keys), and the `.vpr`/`.map` comparison against the baseline. |
| **edg-backend** (Sonnet) | **First:** `vir.h` and `vir.cpp` (struct types interned in `Program`, `Type::Struct`, a member-address expression, site keys with paths) and `check.cpp`'s backstops; il-analyst then switches `lower.cpp`. **Then:** `encode.cpp` (`vtype` and the declaration checks of item 2, member access, `&` of members, assignment targets, initializers, copies, struct pointers, the contract places and atoms of item 9, `restrict` of item 10, markers on struct pointees, `uninit` and `--strict-init` on struct locals, the re-messaged c2 and d2 rejections); `diagnostics.def`; `<cerastes.h>` if needed. |
| **test-mutator** (Sonnet) | **First:** S33's fixtures (`tests/viper-smoke/memory/s33_*`: nested allocation, the leaf-overlap trap `f(&s, &s.x)`, loop liveness dedup, copies and self-copy, `must_init` on a struct, vacuity), and P66, the compiler matrix (clang 21, GCC 16, both modes, three tiers) of member sequencing shapes and `-Wuninitialized` on whole-struct copies. **Then:** the tests and twins below, written from the record before the code runs; the ports; the VIR self-test `selftest_cases14.inc` with `VIR_CASES_MIN`; the lint changes with their self-tests; the doc example files; the flips. |
| **soundness-auditor** (Opus, read-only) | **Round 1:** record 0043. **Round 2:** the code and the tests, especially leaf-keyed overlap, the allocation's lint pin, copies and flags, the `restrict` rule, loop liveness dedup and member framing. **One re-audit** of the blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Step 1; record 0043 and its amendments; `doc/`; `internal/encoding.md` (an M4.5c1 paragraph after `:450`, and §8's lists); `il-inventory.md` (P65, P66, S33); integration; serialized builds; the baseline; status and handoff. Stops and asks the owner. |

Any question from an agent, or of mine, stops that work and goes to the owner.

## Tests (`tests/memory/`, `.c` in both modes; each positive has a twin failing for one reason at one line)

| Positive | Twin (expected) |
|---|---|
| a struct local: write `s.x`, `s.y`, return `s.x + s.y` | read `s.y` before it is written → `uninitialized` |
| `{1, 2}`, `{.y = 2}` (then `check(s.x == 0)`), `{}`, `{0}`, nested braces, C++ `S s{}` | `check(s.x == 1)` after `{.y = 2}` → `check` |
| nested `s.t.x` writes and reads; `&s.t` passed to a `struct T *` callee | a wrong nested value → `check` |
| `inc(&s.x)` with `owns(*p)`, then `check(s.x == 1)` | `check(s.y == 1)` → `uninitialized` |
| `void bump(struct S *p)` with `owns(*p)`, `p->x++`, `ensures(p->x == old(p->x) + 1 && p->y == old(p->y))` | no atom → `permission`; `reads(*p)` and a write → `permission`; a wrong `ensures` → `postcondition` |
| `owns(p->x)` only, and the function touches only `p->x` | writes `p->y` → `permission`; `requires(p->y > 0)` → `permission` in the contract |
| port `malloc_block.c` (`struct account` local, `myAccount->balance = 5`) | reads `balance` first → `uninitialized` |
| port `point_mirror` into `tests/memory/verifast/swap.c` (`swap(&p->x, &p->y)`) | `swap(&p->x, &p->x)` → `permission` (separation) |
| **the leaf-overlap trap**: `f(struct S *s, int *q)` with `owns(*s) && owns(*q)`, called with `&s, &t.x` | `f(&s, &s.x)` → `permission` (separation across pointee types) |
| a `struct T` atom beside a `struct S` atom where `T` nests `S` | `g(&t, &t.s)` → `permission` |
| a pointer to a member, written through and read by name | the member's scope ends before the use → `lifetime`; a callee keeping `owns(p->x)` → `lifetime` at scope exit |
| `p->x` with `p == NULL` excluded by `requires` | `p` may be null → `null-dereference`; `&p->x` of a dangling `p` → `lifetime` |
| a loop summing into `s.x` with an invariant | a missing invariant clause → `invariant-preserved` or `check` |
| copies `t = s`, `*p = *q`, `*p = *p`, `struct S t = *p;` | `s.y` uninitialized → `uninitialized` at the copy |
| `must_init` on `struct S *p` writing every member; `ref_to_uninit`; `uninit` on a struct local; strict mode | one member missing → `postcondition`; unmarked under `--strict-init` → `uninit_unmarked` |
| `owning` on `struct S *p`; `live(p)` with `p == q` compared | the owning callee dereferences without the null test → `null-dereference` |
| sequencing: `s.x + (s.y = 1)`, `p->x + (q->x = 1)` with separate atoms | `p->x + (q->x = 1)` with `p == q` possible → `unsequenced`; `s.x + (s.x = 1)` → `rejected unsequenced_modification` (if P66 shows a warning) |
| a self-referential `struct node *next` read in code | `n->next->v` through an unowned `next` → `permission` |
| — | rejected: a union, a bit-field, array and flexible members, `const`/`volatile` members, an anonymous member, a `void *` member, a function-pointer member, an empty struct, `FILE *` (re-messaged), a block-scope struct, C++ member functions, constructors, bases and default member initializers, by-value parameters and results (c2), a struct global (c2), `owning` on a field (EDG, d2), `old(*p)`, `&s.x + 1`, `(int *)&s`, `restrict` on `struct S *`, `restrict int *` beside an atom-less `struct S *` |

**Flips** (question 93, pre-approved; nothing else may change):
- (a) `tests/scalar/negative/reject_struct.c` is retired for a positive of the same shape (a struct declaration beside a
  scalar function).
- (b) `tests/memory/negative/global_struct.c` loses its `@ L8` line, with a comment in its place. The `@ L13` global stays,
  its message naming M4.5c2.
- (c) re-messaged with every `expect:` line kept:
  - `reject_struct_pointer.c`;
  - `reject_compound_literal_address.c`;
  - `reject_owns_double_deref.c`, `reject_read_double_deref.c`, `reject_read_double_deref_invariant.c`,
    `reject_atom_on_cell_pointer.c` and `global_pointer_global_deref_atom.c`, now naming M4.5c2;
  - each with its `Pins:` comment.

**Viper against the baseline:** every existing `.vpr` and `.map` is byte-identical, and the rest are added files. No
program accepted today has a struct. The VIR self-test shows only added cases.

## Docs (invariant 8; every example is a test)

- **Memory and ownership:** a "Structures" section with member places, initializers, copies, pointers to members,
  `owns(*p)` and `owns(p->x)` with examples, the overlap trap and the markers. The not-yet list (`memory.adoc:514-536`)
  gains c2's constructs and d2's owning fields, and the globals text at `:362` changes.
- **Annotations:** the spec-function places gain members.
- **Error messages:** the member messages of `permission`, `uninitialized`, `null-dereference` and `lifetime`, with examples.
- **Verifying a program:** the rejected list (`:852-878`) becomes "C structures are accepted; unions and C++ class features
  are not".
- **`internal/encoding.md`:** the M4.5c1 paragraph.

## Order of work

1. Step 1, the worktree and the baseline. In parallel:
   - the record draft, with audit round 1 in the background;
   - P65 and P66;
   - the S33 fixtures;
   - edg-backend's VIR block.
2. S33, then the encoder and lowering for objects, members, initializers and contracts. Then the tests and the VIR
   self-test.
   - **Commit 2** (struct objects), together with the baseline comparison.
3. Copies and the markers on structs (the cut point). **Commit 3.**
4. The full regeneration against the baseline. Then audit round 2, the fixes and one re-audit.
5. Docs; `check.sh` (full, at most twice); status and handoff; the last commit.

## Stop and ask

- Any question from an agent.
- An existing test's outcome changing outside the flips, or any existing `.vpr` or `.map` changing.
- P65 contradicting P51: a member kind the IL does not keep, an initializer whose elements are lost, or a C++ copy that
  is not trivial in the IL.
- S33: a vacuity run that passes, or Silicon failing first with its own reason (exit 3).
- Exit 3 on a correct program, or a new test over 60 s.
- Context running short after commit 2: copies (and, if needed, the markers on structs) move to M4.5c2. This is the
  pre-agreed point of question 90.

## Acceptance (commands run at the end)

- `python3 scripts/run_tests.py tests/memory tests/scalar tests/viper-smoke/memory doc/modules/ROOT/examples/memory
  doc/modules/ROOT/examples/error-messages doc/modules/ROOT/examples/verifying doc/modules/ROOT/examples/annotations`:
  all pass.
- **Against the baseline:** every `.vpr`/`.map` in `build/vpr` that exists in `build/m4.5c1-baseline-vpr` is
  byte-identical; the VIR self-test shows only added cases.
- `python3 scripts/test_run_tests.py`, `python3 scripts/test_lint_viper.py` and `python3 scripts/run_tests.py
  --check-headers`: green.
- `scripts/build_docs.sh` and `python3 scripts/check_doc_examples.py --html build/doc`: green.
- `scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): green. There is no Linux run in the session; CI covers it, and
  any x86-64 run goes to `matt@clang4.cpp.al`.
- **Status entry:** record 0043, the flips, the audit outcome, the plan file, and the first step of M4.5c2 (planned from
  handoff §4.2). Handoff §3 gets what was not taken, and §8 any new question.
- **Commits:** planning; struct objects; copies and markers; status and handoff. Local only, on `m4.5c1-struct-objects`,
  no Claude trailer, never pushed. The handoff gives the owner the push and PR steps (after b2b's PR).
