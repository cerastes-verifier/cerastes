# 0045 — Places reached through a heap read (M4.5c2b)

- **Status:** accepted for M4.5c2b (2026-11-03), written before code (invariant 6); revised after P70 (the IL: PASS, hazards H1–H16,
  items 2, 3, 13) and audit round 1 (Opus, on the draft: two blocking items, B1 a contract without a body returning a
  permission it never took, item 5, and B2 the state of a `requires` atom's guard and place read outside the entry
  state, item 3; should-fix S3 pass 0 over guard cells, item 6; S4 the `ite` amount, item 7; S5 the measures and the
  by-value leaves, items 8 and 9; S6 the snapshot order, item 10; S7 disjoint key ranges, item 2; notes N8–N12 taken;
  `build/m4.5c2b/audit1.md`), and after S35 (PASS; its findings in items 6, 7, 10). During the session the owner answered
  question 111 (a call with a load-rooted contract beside another access in one expression is accepted, item 6) and
  question 112 (nine new twins carry a second legitimate same-line `permission` error from sibling liveness rows).
  Audit round 2 on the code (no blocking item; should-fix 1 the `implied_by` shortcut under `old` (item 3: skipped when
  the outer `old` is set and the disjunct reads the heap), 2 `moved` missing a by-name write of a global root (item 12),
  3 pins for pass 0's two triggers and the heap-guard partner row; notes 4 `site_guard` in a loop annotation an internal
  error, 5 load-rooted member places spelled as written, 7 a positive for item 7's `old(g_i)`; all taken;
  `build/m4.5c2b/audit2.md`), and one re-audit of those fixes (nothing blocking; `build/m4.5c2b/audit3.md`). Owner decisions (M4.5c2b planning, 2026-11-03; plan `.claude/plans/m4-5c2b-heap-read-places.md`;
  handoff questions 104–110):
  - **104:** a place reached through a heap read in an `ensures` is the object the path designates on return;
    `old(**pp)` is the entry value of the entry object.
  - **105, 110:** places the `requires` reaches through a heap read are automatic places of every loop, through a
    method-entry snapshot of their address and, where the function may write a loaded field of the path, also a
    loop-entry snapshot.
  - **106:** `live(r.q)` moves to M4.5d2 with every `ensures` atom that has no `requires` partner.
  - **107:** the flips of item 15.
  - **108:** amounts follow objects: an `ensures` atom reached through a load returns the amount of the `requires` atom
    whose entry place equals its return place (not the recommendation, which pinned the false alarm).
  - **109:** guards that read the heap are accepted (not the recommendation, which kept them rejected).
- **Milestone:** M4.5c2b (2026-11-03).
- **Implements:** 0040 item 6's and 0041 item 7's places "reached through a heap read" (question 74), 0043 item 15's
  and 0044 item 1's M4.5c2b rows.
- **Amends:** 0037 items 5–10 (places, guards, framing, calls, entry, returns), 0038 items 3 and 6 (automatic places,
  invariant reads), 0039 items 4, 7 and 8 (memory calls, partners, measures), 0040 items 2, 5, 6 and 9, 0041 item 7,
  0042 item 6, 0043 items 9–11 and 15, 0044 items 3, 4, 12 and 15.

## Context

Since 0037 a contract names a place by a **root** that never changes between entry and return — a pointer parameter
(its entry value, 0012 item 2), a global, the result — and, since 0043, a member path from it. Every mechanism of the
lowering keys a place by an integer root and rebuilds the root from it (`place_of`), in every state. A place reached
through a pointer **loaded from memory** (`**pp`, `*gp`, `p->q->x`) has a root whose value depends on the state: the
pointer stored in `*pp` on entry need not be the one stored there on return. Records 0040, 0041, 0043 and 0044 therefore
rejected every such place, naming this session (11 pins). Without them a `T**` callee cannot use the pointer it loads
(`live(*pp)` cannot be written), a pointer member of a struct parameter cannot be followed, and M4.5d2's owning fields
(a pointer member owning its target) have nothing to stand on. Facts: P70 (the IL) and S35 (Silicon), in
`internal/il-inventory.md`, "M4.5c2b".

## Decision

1. **Scope.**

   | Construct | Session |
   |---|---|
   | `owns`/`reads`/`live` and reads of places reached through loads at any depth: `**pp`, `***ppp`, `*gp`, `gp->x`, `p->q->x`, `*p->q`, `(*pp)->x`; places through a by-value parameter's pointer leaf (`*s.q`, `s.q->x`, `live(s.q)`); `*r.q` of a struct result; `old(**pp)`; function measures over them; guards that read the heap (question 109); written atoms on such places and on struct places in loop invariants (question 99); `**pp` in invariants and measures; the `restrict` re-check | M4.5c2b (this record) |
   | `live(r.q)`, `owns(*r.q)`, `owns(**r)`, every `ensures` atom with no `requires` partner (question 106) | M4.5d2 |
   | places through an array element | M4.5e1 |
   | `owns` of a `const` place at any level (0040 item 2, kept) | M4.6 |
   | places through `void *`, unions, function pointers | M8, M4.7 |
   | `*old(*pp)` (the object `*pp` designated on entry, read on return) and `old` inside a place's path | M8 (listed) |

2. **Places as access paths.** A place is a **root**, the **steps to its base pointer** (`via`) and a **member tail**
   (`path`). A root is a formal, the result (in an `ensures`), a global, or, in a loop invariant, a pointer variable. A
   step is a member `k` (MemberAddr), a **load** (a Deref of a pointer-valued place) or a by-value field `k` (a Field of
   a by-value struct parameter or result, 0044 items 3–4). Examples: `**pp` = (pp, [load], []); `p->q->x` = (p, [q,
   load], [x]); `*s.q` = (s, [field q], []); `*gp` = (gp, [load], []); `(*ps)->data` = (ps, [load], [data]); `*p` and
   `p->t.x` keep `via = []`, today's key. Places without a load or a field step keep today's keys, code paths and
   Viper (byte-identical).
   - The **key** (root, via, path) is syntactic. It decides duplicates in one clause list and the partner of an
     `ensures` atom (0039 item 7) and nothing else. Overlap, separation, framing and splits are decided by `Ref`
     equality in the state where the place is evaluated, never by key.
   - 0043 item 9's static rejection of overlapping atoms (one place's path a prefix of the other's) applies only when
     both have the same root **and** the same `via`: `owns(*pp) && owns(**pp)` names a pointer cell and the object it
     points to, which never share a cell (different fields, 0028 item 1). Overlap across loads is per path (item 5).
   - In the lowering each distinct (root, via) with a load or a field step gets a per-function **derived key**, so the
     integer-keyed consumers (sums, splits, separation, loop places, ghosts) treat a base as one more root; one derived
     key is one syntactic base, hence one `Ref` in one state, so today's "one root" shortcuts stay correct for it.
     Derived keys are never globals (no `unique`-block shortcut). The key ranges are disjoint (audit S7: a local or derived
     key above `kGlobalSiteBase` must never pass a `>= kGlobalSiteBase` test): global keys in `[kGlobalSiteBase,
     kLocalSiteBase)`, invariant locals in `[kLocalSiteBase, kDerivedKeyBase)`, derived keys from `kDerivedKeyBase`, and
     every "is a global" test bounded on both sides; `site_key` and `same_place` compare the derived key (root and `via`).
   - Places are recognised by node shape, never by the IL's lvalue flag (P70 H4); `(*p->q).x` and `p->q->x`, `(**pp).x`
     and `(*pp)->x` are one place (H3); the C spelling's integer-promotion cast above a narrow place is peeled at every
     depth (H12); a cell pointer in an invariant atom is told by its variable's `address_taken` (H6).

3. **States** (question 104). A place's loads are **contract reads**: each is framed per path by the atoms before it in
   evaluation order (0037 item 7). `owns(**pp)` needs `owns(*pp)` or `reads(*pp)` earlier on its path; `owns(*gp)`
   needs `owns(gp)` or `reads(gp)`; `owns(*p->q)` an atom covering `p->q`. A `requires` place is evaluated in the entry
   state; an `ensures` place in the return state (the object the path designates on return); `old(**pp)` entirely in
   the entry state. A callee may therefore move `*pp`: `ensures(owns(**pp))` then promises the new object, and
   `*pp == old(*pp)` is the user's tie when it does not.
   - **Entry-state rule** (audit B2). Everything a `requires` atom contributes — its guard, its place, and the framing
     disjuncts built from it — is read in the **entry** state wherever it is used outside the `requires`: the chain of an
     `old(…)` read is one `old(…)` over the whole chain, disjuncts included (never `old` around the last field only:
     `ite(D_post, old(read), 0)` with a post-state disjunct gave a wrong verdict, audit B2's fixture); item 7's `q_ipre`
     and `old(g_i)`; the partner row of 0039 item 7 (the partner's guard over pre-state ghosts, the `ensures` guard over
     post-state ones); 0042's kept rows; `f__wf`'s `first_reads` (guards of `ensures` atoms over post-state ghosts, every
     ghost collected before the formals are emitted); the caller's measure in 0039 item 8's explicit check (`old`). An
     `ensures` atom's guard and place are read in the return state. P70 H1: C23 folds `cerastes_old(**pp)` to
     `*old_(*pp)`; the encoder reads `old_`'s argument `X` as the place `*X` (or `Y` for `address_of(Y)`), and the whole
     place is under the one `old`.

4. **The method's contract.** The atom's receiver and every load in it are chain-guarded reads (0037 item 7's
   `ite(D_1, read, … dummy)`, the dummy `null` for a pointer load). On a path where a load is unframed the atom is
   `false` there (its `root != null` fails); `f__wf` reports that path (`permission`, "the contract of `f` reads `*pp`
   without owning it") and Silicon's well-formedness never fails (no exit 3). A load is written plainly only where an
   earlier unconditional atom names the same inner place (never "implied by the guard": every copy of a base must be
   well-formed on its own, since splits and entry snapshots reuse it).

5. **`f__wf`.** A load's value is a ghost formal keyed by the full place and state, with `WF` (or `WF_S`) as its
   `requires`; a load-rooted place's `Ref` is built on that ghost. New: the **atom's own loads** get framing asserts (a
   place read for an atom never existed before). Facts (`f__wf`'s `requires`, each a fact Silicon derives from the
   method's contract):
   - equal ghosts (`q_a == q_b ==> h_a == h_b` in one state) by `Ref` equality across depths; the shortcut "one root,
     two member paths, two cells" only between places of one derived key (one base);
   - `owns` distinctness in one clause list, unchanged in form, over the ghost-built `Ref`s;
   - a `reads` place's post-state value equals its pre-state value only under `q_post == q_pre` when the place loads
     (a moved `*pp` names another object); the result-versus-`reads` fact (0040 item 4) likewise, for `*r.q`.
   - Partners by path (0039 item 7, `g_ens ==> g_partner`) keep their row, with item 3's states.
   - **A contract without a body** (audit B1): nothing proves its `ensures`, so a load-rooted `ensures` atom could name an
     object its `requires` never took (a trusted `retarget` with `ensures(live(*pp))` after `*pp = q`, or `owns(**pp) &&
     owns(**qq)` returned on one object, whose distinctness is only a fact here). For a function without a body, `f__wf`
     asserts for each `ensures` atom whose place loads `prior ==> (g_ens ==> q_post == q_partner_pre)` (`permission`,
     "the contract of `f` returns `**pp` without saying it is the object its `requires` took (write `*pp ==
     old(*pp)`)"), and for each pair of `ensures` atoms of one kind and field of which one loads, `prior ==> (g_a && g_b
     ==> q_a != q_b)` as an assert, not a fact. A function with a body keeps item 7: its return rows prove what it
     returns. Pinned by a must-fail test (the auditor's `retarget`).

6. **Calls, entry, returns.**
   - **Calls** (0037 item 8): each callee atom's place is evaluated at the arguments in the caller's current heap,
     chain-guarded over the callee's atoms at the arguments (audit N8: a contract framed through a value `requires`,
     `owns(*qq) && pp == qq && owns(**pp)`, would otherwise read `a.v_ptr_T` before its row; the chain gives `permission`,
     never exit 3). The per-atom rows are emitted in evaluation order, so each load is read only after the row that checks
     its cell is held (`perm(a.v_ptr_T) == write` before `perm(a.v_ptr_T.v_T) == write`). 0039 item 4's obligations
     evaluate the callee's load-rooted places in the heap after every argument is evaluated: each such place is
     snapshotted into a ghost `Ref` before the call (question 111), and the pair checks (a call beside an access, two
     calls) compare against the snapshot, the object the callee can touch. The caller also makes the pairwise `lvloc`
     splits over every callee atom place, loaded ones included (S35 finding 1). Separation, `lvloc` splits, amounts,
     sums and the splits against the caller's `reads` places compare `Ref`s and come after every per-atom row.
     Memory calls (0039 item 4) and the havoc rule (0038 item 3) cover every leaf field a load-rooted atom owns.
   - **Entry:** right after the `requires`, before 0037 item 9's checks, each load-rooted `requires` place's base is
     snapshotted into a ghost `Ref` (`eb`, item 8), from the `requires`-framed chain; the entry splits and separation
     asserts use it.
   - **Returns** (0037 item 10): **pass 0** — before the separation rows, for each `ensures` atom whose cells frame the
     base of a load-rooted `ensures` place **or a cell an `ensures` guard reads** (audit S3, the guards of pass-0 rows
     included), `g_j ==> perm(cell) > none` in clause order, with that atom's message ("returns without owning `*pp`,
     which its contract promises"); then today's rows. Emitted only when an `ensures` place loads or an `ensures` guard
     reads the heap, so no existing program changes.
   - **Body-side names:** where the body names a `requires` place by its formal (`result_splits`, the splits at calls
     against the caller's `reads` places), a load-rooted place uses its `eb` (a heap read in a split condition would not
     be well-defined where the cell is no longer held).

7. **Amounts follow objects** (question 108; amends 0037 item 6). An `ensures` atom whose place loads returns
   `ite(old(g_p) && q_post == q_ppre, a_p, ite(old(g_1) && q_post == q_1pre, a_1, … a_p))` over the `requires` atoms of
   its kind and field: its partner `p` by key **first** and last (the fallback), the others between (audit S4: an atom
   whose guard was false on entry took nothing); `q_ipre` is each one's entry place and `old(g_i)` its entry guard (item
   3). A struct atom matches by the slot of one struct type, else the partner; a `live` atom by the liveness cell
   `lvloc(block_of(…))`. The return rows and **every term of the return sums** use these `ite` amounts (today's sums use
   the amount names directly). A callee that moves `*pp`
   to an object it was given returns it at the amount it was given, so `owns(**pp)` after a move verifies; the return
   rows check `perm >=` the `ite` amount, and the caller's view is Viper's call (no inhale). Places without a load keep
   the partner's amount (today's Viper). A move to an object the callee did not receive by an atom of that kind
   returns the partner's amount and fails the return check (`permission`), which is right: the callee cannot give what
   it was not given.

8. **Measures** (amends 0039 item 8; audit S5). A function's `decreases(**pp)`, `decreases(*gp)`, `decreases(p->q->x)`,
   `decreases(*s.q)`: framed in `f__wf` over the `requires` atoms (their loads too), the method's `decreases`
   chain-guarded in the entry heap Silicon evaluates it in, and 0021 item 4's explicit check at the caller reading the
   callee's measure over the arguments in the current heap and the caller's under one `old(…)` (item 3).

9. **By-value leaves and results** (0044 items 3–4; audit S5). `owns(*s.q)`, `reads(*s.q)`, `live(s.q)`, `*s.q`,
   `s.q->x`, `old(*s.q)`: the place's base is the leaf formal (`p_s__k_m<i>`), a fixed value (the entry value in an
   `ensures`, as `*p` of a pointer parameter), so it needs no snapshot and no item 7 `ite`: a field step without a load is
   a fixed root. Such a place is an automatic loop place directly through its leaf formal (whatever the body does with
   the copy `s.q`). `*r.q` (a result leaf in an `ensures`) is framed as `*result` is (0040 item 4: `D_i = g_i && q_i ==
   r.q`, the result-versus-`reads` fact per result leaf). `live(r.q)` is M4.5d2's (question 106).

10. **Loops** (questions 105, 110; amends 0038 item 3). Each `requires` place reached through a load is an automatic
   place of every loop, through its **method-entry** snapshot `eb` (the object the `requires` named, as a formal is the
   entry value). Where the function may write a cell of a loaded field of the path (`ptr_T` for `**pp`; a store or a
   call reaching that field anywhere in the body, by 0038 item 3's havoc rule), it is also an automatic place through a
   **loop-entry** snapshot `lb` (`:= null`, then `if (<inner cells held>) { lb := <the place's address> }` right before
   the loop): what the path designates when the loop starts. Order at the loop's entry (audit S6; S28 P3): `lb` is
   assigned first, then the case splits of `eb`/`lb` against the cells, the formals' places, the globals, the other
   snapshots and the liveness cells, then the `perm` snapshots. Where nothing can write the inner
   cells the two are one object and only `eb` is emitted. `eb`/`lb` stand where a formal stands (guards non-null and
   distinct from the earlier places of the field and from every cell, splits, snapshots, rows, back-edge checks);
   nothing assigns them in the loop. A loop that writes `*pp` keeps both objects; `**pp` in it reads what the chains
   frame.

11. **Guards that read the heap** (question 109; amends 0037 item 5). An atom's guard (the left operand of `||`, the
   condition of `?:`) may read through pointers: `*pp == NULL || owns(**pp)`, `p->q == NULL || owns(*p->q)`. The
   guard's reads are contract reads framed per path by the atoms before it (chain-guarded in the method, the call and
   the return; ghosts with framing asserts in `f__wf`; through the automatic places' chains in an invariant). An atom
   under `!`, inside `==`, as an argument or on the left of `||` stays rejected: a guard may read the heap but holds no
   atom (audit N12; `check_program` reports any atom the collector does not list). `owning`'s merge (0040 item 3) and
   `restrict_backed` recognize only today's `p == NULL` shape.

12. **Written atoms** (amends 0040 item 5; question 99).
    - **On places reached through loads:** `owns(**pp)`, an atom on `*q` where `q` is a cell, `owns(*p->q)`,
      `owns(*s.q)`, `owns(*gp)` in `CERASTES_INVARIANT`. The atom's `q` is the place's `Ref` read in the invariant
      through the automatic places' chains (0038 item 6) and, for the body's splits, a ghost `Ref` assigned from it at
      the top of each iteration. Its loads get framing asserts before its entry and back-edge rows. The atom is
      **moved** (no value snapshot) when the loop assigns the root variable, writes a cell of the path by name, or
      writes a loaded field. A written atom and a `requires` place with the same spelling have different keys (they
      name the place in different states; audit N10).
    - **On struct places:** `owns(*q)`, `reads(*q)` for `struct S *q`, `owns(q->x)`, `owns(q->t)`: 0040 item 5's rows
      per leaf (amounts, `NOTAUTO`, value snapshots, entry and back-edge checks, the "on an automatic place" rows, splits
      against the places of each leaf's field), one liveness amount per atom on `live_cell(q)` (`member_lv`), `WF_S(q)`
      and the member lengths in the basic row; separation by `overlap_pairs`; each leaf a framing place.
    - `**pp` and every read through a load in an invariant or a loop measure are accepted by 0044 item 12's argument
      (the spelling rule of 0040 item 5 lifted).

13. **Encoder rules over loads.**
    - **Markers** (0042 item 6): every load of a place is a contract read, so a read through a marked parameter's
      pointee is rejected wherever it occurs (`owns(**pp)` with `ref_to_uninit` on `pp` in the `requires`: "the contract
      reads `*pp`, which `cerastes::ref_to_uninit` says may hold no value on entry"). `check_marker_atoms`, `apply_owning`
      and `reject_marked_read` walk the whole path to its root (today a load-rooted atom would skip them).
    - **`const`** (0040 item 2): `owns` of a place whose own type is `const`-qualified is rejected at the innermost level
      (`owns(**pp)` for `const int **pp`); a `const` cell further up the path does not make the place `const`: with
      `int *const *pp`, `**pp` is a plain `int` that C lets the function write, so `owns(**pp)` is accepted and
      `owns(*pp)` rejected (P70 Q2's table). The qualifier is read below the C++ binding's `lvalue_adjust` and from
      `orig_lvalue_type` in C (P70 H2, H5). Written atoms in loop invariants are checked, not trusted, and keep no
      `const` rule (0040 item 2). The conversion and comparison rules are unchanged.
    - **`restrict`** (0037 item 2, 0043 item 10, 0044 item 11): `restrict_hazard` walks every atom's and every contract
      read's path to its innermost root (an atom on `*gp` is a `T*` global's; on `**pp` a `T**` parameter's; on `*s.q` a
      by-value leaf's). The argument is unchanged: a pointer loaded from memory is never based on the `restrict`
      parameter, and every root that can hold a `T*` is already a hazard by its type; `live(*pp)` removes the `lifetime`
      stop that c1's audit-2 S1 alias path relied on, and `restrict_hazard`'s parameter loop rejects it from `pp`'s type
      alone, before any atom (audit N9; VIR types carry no qualifiers, so `const int **` is a hazard too); the walk over
      atom places matters for globals and the result. `restrict_backed` still needs `owns(*p)` on the formal itself.

14. **The lint** (`lint_viper.py`): contract `acc`s are not pinned (no rule). The ghost `Ref` locals of items 6, 10 and
    12 are assigned once, from `null` or a framed read, at method entry, right before their loop or at the top of an
    iteration; a rule pins that.

15. **Categories:** none new. Messages name places in source terms (`**pp`, `(*ps)->data`, `*gp`, `p->q->x`, `*s.q`).

16. **Still rejected** (fail closed; each pinned): item 1's M4.5d2, M4.5e1, M4.6, M8 and M4.7 rows; atoms in a measure, a
    `check` or a `do … while (0)` invariant (no session); an atom on a global itself in an invariant (`owns(g)`, 0041 item
    10, unchanged; `owns(*gp)` on its target is accepted, item 12); `old(…)` in a loop invariant (unchanged: a loop states
    a tie through a local copied before the loop).

17. **Flips** (question 107): retired for positives of the same shape `reject_owns_double_deref.c`,
    `reject_read_double_deref.c`, `reject_read_double_deref_invariant.c`, `reject_atom_on_cell_pointer.c`,
    `global_pointer_global_deref_atom.c`, `reject_struct_member_chain_contract.c`, `reject_struct_atom_in_invariant.c`,
    `reject_struct_value_deref_member.c`, `reject_struct_value_reads_member.c`, `reject_struct_value_live_member.c`;
    re-messaged naming M4.5d2 `reject_struct_value_live_result.c`; `Pins:` comments only in
    `global_pointer_global_unowned.c`, `struct_init_pointers.c`, `verifast/globals.c`.

18. **Facts relied on:** P70 (PASS; H1–H16), S35 (PASS for (a)–(o), 61 fixtures; findings 1–9 taken in items 6, 7 and 10).

19. **Tests and documentation:** must-verify and must-fail pairs for every accepted form in both modes (plan, Tests);
    ports of VeriFast's `tests/issue310.c` and the struct half of `examples/globals.c`. Docs: Memory and ownership,
    Annotations, Error messages, Verifying a program; `internal/encoding.md`.

## Consequences

- A `T**` callee can say that the pointer it stores or finds in `*pp` is live, use it, and own what it points to; a
  struct parameter's pointer member can be followed in contracts; M4.5d2's owning fields have a place to point.
- An `ensures` place through a load names the object on return; a callee that moves a pointer it owns returns the
  object at the amount it received.
- Programs accepted before c2b keep their Viper: none has a place through a load in a contract or a written atom, P69's
  invariant shapes keep their chains, and every new row is emitted only for load-rooted places.
