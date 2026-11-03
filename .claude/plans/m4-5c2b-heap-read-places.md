# Plan: M4.5c2b — places reached through a heap read (record 0045)

Approved by the owner on 2026-11-03 (planning in plan mode; questions 104–107 answered, each with the recommendation: 104 the post-state reading of an `ensures` place; 105 heap-read `requires` places as automatic loop places by a snapshot; 106 `live(r.q)` to M4.5d2; 107 the flips). Addendum the same day, after the design review (questions 108–110): 108 amounts follow objects now; 109 guards that read the heap accepted now; 110 both snapshots, method entry and loop entry, where they can differ (108 and 109 not the recommendation; 110 the recommendation).

- **Branch:** `m4.5c2b-heap-read-places`, from c2a's tip `7f11706` (pushed as `origin/m4.5c2a-struct-values`, PR #25
  open with CI pending), so c2b's PR stacks on PR #25 as c2a's stacked on PR #24.
- **Worktree:** `.claude/worktrees/m4.5c2b`; baseline in the detached `.claude/worktrees/m4.5c2b-base` at `7f11706`.
- **Saved at step 1** as `.claude/plans/m4-5c2b-heap-read-places.md`.

## Context

M4.5c2 was split at c2a's planning (question 95). c2a (record 0044) landed struct values and struct storage; every place
a contract or a loop invariant reaches **through a pointer loaded from memory** stayed rejected, naming M4.5c2b (11
pins). M4.5d2 needs this session: an owning field points through a heap read. Today:

- **Contracts** (`encode.cpp`: `contract_place_address` 3058–3128, `spec_atom` ~3180–3322, `old_call` ~2990–3020,
  `old_c` ~3440–3500, `contract_read` 3520–3570): `owns(**pp)`, `reads(**pp)`, `live(*pp)`, `**pp`, `old(**pp)`;
  `owns(*gp)`, `live(gp)`, `*gp`; `owns(*p->q)`, `live(p->q)`, `p->q->x`, `*p->q`; `owns(*s.q)`, `reads(*s.q)`,
  `live(s.q)`, `*s.q`, `s.q->x` of a by-value parameter; `*r.q` of a struct result (`live(r.q)` moves to M4.5d2,
  question 106).
- **Loop invariants** (`invariant_atom` 3326–3377, `indirect_read` 4587–4591): an atom on `*q` where `q` is a cell,
  `owns(**pp)`, `live(*pp)`, `owns(*p->q)`, `owns(*s.q)`; written atoms on struct places (`owns(*q)` for `struct S *q`,
  `owns(q->x)`; question 99); `**pp` read in an invariant or measure (by spelling only: `p->q->x`, `*p->q`, `*gp` are
  accepted and pinned since c2a, P69).
- **Why it is not a local change.** Every place-keyed mechanism in `lower.cpp` works on an `int` root key and builds
  the root with `place_of(env, k)` (1886: a formal, the result or `loc(GB_g(),0)`), which never depends on the state:
  `atom()` 1790, `contract_read` 1933 (old only wraps the last field read, 1994), `wf_method` 6119 (ghosts keyed by
  `GKey<int, path, post>` 1878), `call_atoms` 2944, `entry_checks` 4523, `atom_returns` 4577, `loop_places` 4922,
  `written_atoms` 5113, `result_splits` 4482. In VIR, `PlaceKey`/`atom_place`/`member_root` (vir.h 700, vir.cpp 839–860)
  strip only `MemberAddr`; `check.cpp` 1048–1127 allows only Formal/GlobalAddr/Result roots; `AtomCollector` refuses
  guards that read the heap. In the encoder, `finish_atoms` (3574), `reject_marked_read` (1031), `check_marker_atoms`
  (975), `apply_owning` (1055) and `restrict_hazard`/`restrict_backed` (vir.cpp 923–994) assume a Formal or global root:
  a heap-read root would **silently skip** the marker checks and the `restrict` hazard, so each is rewritten, not merely
  unblocked.
- **Consequence today:** a `T**` callee may store into `*pp` but cannot use the loaded pointer (`lifetime`,
  `ptrptr_callee_escape.c`), since no contract can say `live(*pp)` (record 0040, Consequences).
- **The `restrict` re-check** (handoff §3): once `live(*pp)` is writable, the alias path that only `lifetime` stopped
  (c1 audit 2 S1) must be re-checked; `restrict_hazard` today sees only GlobalAddr-rooted atoms.

Size: about 1,500–2,500 changed source lines (c2a: ~2,700; encoder ~300, VIR ~150, lowering the rest), one session.

## Owner decisions (2026-11-03, at planning; each the recommendation)

- **104, the `ensures` reading:** a place reached through a heap read in an `ensures` is the object the path designates
  **on return** (`*p` in an `ensures` is the value on return already); `old(**pp)` is the entry value of the entry
  object. A callee that moves `*pp` returns the new object; `*pp == old(*pp)` is the user's tie (VeriFast's `?s`
  idiom). Partners pair by syntactic place; the return checks verify what is promised.
- **105, loops:** places the `requires` reaches through a heap read are **automatic places** of every loop ("every
  place held", 0038 item 3), addressed by a ghost `Ref` snapshot of the place's address taken at the loop's entry where
  the function holds the inner cells, standing in for the formal. S35 (f) confirms the encoding first; if it fails
  (exit 3, a vacuity clone passing, or Silicon refusing a shape), stop and ask.
- **106, `live(r.q)`:** moves to **M4.5d2** with `live(r)`, `owns(*r.q)` and every `ensures` atom without a `requires`
  partner; `reject_struct_value_live_result.c` is re-messaged naming M4.5d2, its `expect:` line kept. `*r.q` as a value
  (framed like `*result`, 0040 item 4) lands here.
- **107, flips:** pre-approved as listed under Flips; nothing else may change.

**Addendum (2026-11-03, after the design review; questions 108–110):**
- **108, amounts follow objects** (not the recommendation, which was to pin the false alarm): an `ensures` atom whose
  place is reached through a load returns the ghost amount of the `requires` atom of its kind whose **entry** place
  equals its **return** place, `ite(q_post == q1_pre, a_1, ite(q_post == q2_pre, a_2, … a_partner))` over the `requires`
  atoms of the same kind and leaf field (the partner by key last). A callee that moves `*pp` to another object it was
  given therefore returns that object at the amount it was given, and `owns(**pp)` after a move verifies. Places
  without a load keep today's amounts and Viper. The return sums, the call's view (Viper inhales the `ensures` with the
  `ite` amount) and `f__wf`'s partner rows (0039 item 7) follow; S35 confirms first.
- **109, guards that read the heap** (not the recommendation, which was to keep them rejected): an atom's guard may
  read through a pointer (`*pp == NULL || owns(**pp)`, `p->q == NULL || owns(*p->q)`, `c ? owns(*p->q) : …` with a heap
  condition), in contracts and in loop invariants. The guard's loads are contract reads framed per path by the atoms
  before them (chain-guarded in the method, the call and the return; ghost values with framing asserts in `f__wf`;
  through the automatic places' chains in an invariant). `AtomCollector` and `site_guard` (today forced to `Wf` mode)
  change accordingly; `atom_guard_shape`'s `NullCheck` stays the `Formal == NULL` shape only (`owning`'s merge and
  `restrict_backed` unchanged). No existing pin rejects such a guard (checked at planning).
- **110, both snapshots where they can differ** (the recommendation): every `requires` place reached through a load gets
  a method-entry ghost `Ref` (`eb`, the object the `requires` named, as a formal is the entry value), which every loop
  uses as an automatic place and which also serves `result_splits` and the splits at calls; and, when the function may
  write a cell of a loaded field of the path (`ptr_T` for `**pp`: a store or a call reaching that field anywhere in the
  body, by 0038 item 3's havoc rule), a loop-entry ghost `Ref` (`lb`, what the path designates when the loop starts,
  taken under the inner cells being held, else `null`) as a second automatic place. Where nothing can write the inner
  cells the two are one object and only `eb` is emitted. Duplicate places are handled by the existing guards.
- **111, a call beside an access** (during the session, the recommendation): a call whose contract names a place
  reached through a load, in one expression with another access (`y = x + inc2(&p);`), is accepted: each such callee
  place is snapshotted before the call and 0039 item 4's checks compare against the snapshot.
- **112, post-run expectations** (during the session, the recommendation; as question 102 in c2a): nine new must-fail
  twins also report a second (one: third) legitimate `permission` error at their expected line (sibling liveness rows of
  places that may share a block); duplicate expect lines added, no line number changed.

**Standing:** 77 (a session fits one context), 69 (the EDG merge waits for the box session; no fork work here), 78
(`const` objects are M4.6's), 91 (owning fields are M4.5d2's), 99 (written struct atoms in invariants are this
session's), 100 (struct `?:`/comma/assignment values stay M8).

## Step 1 (first, before any code): consolidate the plan documents

The first commit is markdown only.

1. **Worktree:** `git worktree add .claude/worktrees/m4.5c2b -b m4.5c2b-heap-read-places 7f11706`; submodules cloned
   with `--reference` to the main checkout's stores (memory `background-worktree-setup`); `scripts/bootstrap.sh`.
2. **Save this plan** as `.claude/plans/m4-5c2b-heap-read-places.md`, with an "Approved" line and the Inputs below.
3. **`internal/handoff.md`:** header pointer `git show 7f11706:internal/handoff.md`; §1–2 owner steps (PR #25 after CI,
   then `git merge --ff-only origin/main` and remove `.claude/worktrees/m4.5c2a{,-base}`, and the older
   `m4.5b2b-base`, `m4.5c1`, `m4.5c1-base`; at the end c2b's PR); §3 "M4.5c2b" notes move into this plan's Inputs; §4
   record 0045 is this session's, 0046 the next free one; §4.2 the c2b paragraph points at this plan, and M4.5d2's
   paragraph gains `live(r.q)` (question 106); §8 questions 104–107 answered, none open.
4. **`plan.md`:** the header's "the last" becomes M4.5c2b; M4.5d2's row gains `live(r.q)` (question 106).
5. **Commit 1 (planning):** local, no Claude trailer.

## Inputs from the handoff (moved into the plan at step 1)

- §3 "M4.5c2b": the heap-read places; "a `T**` callee cannot use `*pp`'s pointer until then" [b1c3]; `p->q->x`,
  `*s->q` [c1]; atoms and reads through a by-value struct's pointer member [c2a planning]; written struct atoms in loop
  invariants with atoms on cell pointers (question 99) [c1, c2a]; the `restrict` re-check [c1 audit 2 S1]; `live(s.q)`
  and `return s.q;` [c2a audit 2 N10]; unify `**pp` in invariants with P69's accepted shapes [c2a].
- Stale comments to fix in passing: `encode.cpp:4055` and `:5296` name M4.5c2b for heap-read copy bases whose messages
  name M8 (record 0044 item 1's M8 row); `annotations.adoc:395–397` still says atoms are not accepted in invariants.
- Standing [c1 audit 2 N2]: `overlap_pairs`' one-type shortcut and `f__wf`'s equal-ghost skip for one root with two
  paths are complete only while types prevent `p == &q->t`; a loaded root makes `p == (*pp)->t`-style aliasing
  expressible, so the record re-states both for load roots (fail closed: a missed pair costs exit 3, never a fact).
- Process (§6): generators in `build/m4.5c2b/`, never rerun after a hand edit; twins whose failed read flows into an
  `ensures` get an `ensures` that holds for any value; single-file runs; builds serialized with
  `lockf build/m4.5c2b/build.lock cmake --build build/cmake`; no build while a directory-wide run is going.

## Setup (in parallel with step 1)

- **Baseline** in `.claude/worktrees/m4.5c2b-base` (detached at `7f11706`), in the background: `bootstrap.sh`;
  `run_tests.py -j 8`; the VIR self-test; copy into the session worktree's `build/m4.5c2b-baseline-{vpr,vir}`.
- Copy c2a's regeneration script (`.claude/worktrees/m4.5c2a/build/m4.5c2a/regen/`) into `build/m4.5c2b/regen/`.
- Probes run against the session worktree's first build (before any code change), which equals the baseline.

## Record 0045 — places reached through a heap read

Written before code (invariant 6) while P70 and S35 run; audit round 1 on the draft in the background.

1. **Scope table:** c2b | d2 | e1 | M4.6 | M8. **Amends:** 0037 items 5–10, 0038 items 3 and 6, 0039 items 4, 7 and 8,
   0040 items 2, 5, 6 and 9, 0041 item 7, 0042 item 6, 0043 items 9–11 and 15, 0044 items 3, 4, 12 and 15.
2. **Places as access paths.** A place is a root plus steps: a member index or a **load** (a dereference of a
   pointer-valued place). Roots: a formal, the result (`ensures`), a global, a by-value parameter's leaf formal
   (`s.q`), the result's leaf (`r.q`), and in an invariant a pointer variable (a cell's root is its address, followed
   by a load). `**pp` = pp·[load]; `p->q->x` = p·[q, load, x]; `*gp` = `&gp`·[load]; `(*ps)->data` = ps·[load,
   data]; any depth. `vir::PlaceKey` gains the steps; places without a load keep today's keys and code paths
   (byte-identical Viper). The key is **syntactic**: it decides duplicates and `requires`/`ensures` partners only.
   Overlap, separation, framing and splits are always decided by `Ref` equality in the evaluated state, never by key.
   A key prefix that crosses a load does not make two atoms overlap statically (`owns(*pp) && owns(**pp)` is legal);
   0043 item 9's static rejection applies only to prefixes before the first load, and the rest is per path.
3. **States** (question 104). A place's inner loads are contract reads, framed per path by earlier atoms (0037 item 7:
   `owns(**pp)` needs `owns(*pp)` or `reads(*pp)` before it; `owns(*gp)` needs `owns(gp)`/`reads(gp)`; `owns(*p->q)`
   needs an atom covering `p->q`). In a `requires` and under `old` the place is read in the entry state; in an `ensures`
   in the return state (the object the path designates on return); `old(**pp)` is entirely the entry state. Partners
   stay by key; the return checks (0037 item 10) verify what is promised, so a callee that moves `*pp` returns the new
   object, and `*pp == old(*pp)` is the user's tie.
4. **Method contracts.** The atom's receiver and every inner load are chain-guarded reads (0037 item 7's form); on a
   path where an inner load is unframed the atom is `false` there, and `f__wf` reports that path (`permission`, "the
   contract of `f` reads `*pp` without owning it"), as for any unframed read.
5. **`f__wf`.** Ghost values keyed by the full place and state; a load-rooted place's `Ref` is the ghost value of its
   inner read, with `WF`/`WF_S` as its `requires`. New: a framing assert for each atom's own inner loads. Facts: owns
   distinctness and equal-ghost facts by `Ref` equality across depths (the one-root shortcut of 6214 only for load-free
   paths); the `reads` post = pre fact only under `q_post == q_pre` for a load-rooted place (the audit checks the
   argument).
6. **Calls, entry, returns.** At a call each callee atom's place is evaluated at the arguments in the caller's current
   heap, in clause order, so every inner load is framed by an earlier row; separation, `lvloc` splits, amounts, sums and
   the splits against the caller's `reads` places compare `Ref`s; memory calls (0039 item 4) and the havoc rule cover
   every leaf field a heap-read atom owns. Entry splits and separation asserts, and the return rows, likewise.
   **Row order:** no row may read through a load before a row has checked the inner cell is held (else Silicon's own
   well-definedness check fails first: exit 3). Today 0037 item 10 puts the return's separation rows before the
   per-atom rows; for load-rooted places the separation rows move after the per-atom rows of their inner places (the
   rows of load-free places keep today's order and text), and the same rule orders the call rows (0037 item 8).
   **Body-side names.** Where the body names a `requires` place by its formal today (`site_var`: `result_splits`, the
   splits at calls against the caller's `reads` places, the entry splits), a load-rooted place has no formal: its
   address is snapshotted once at the method's entry into a ghost `Ref` (`pe_<site>`, assigned under the atom's guard
   from the `requires`-framed read, `null` otherwise) and used there; a `Ref` read of the heap in a split condition
   would not be well-defined where the inner cell is no longer held. Item 9's loop places reuse these entry snapshots, plus a
   loop-entry snapshot where they can differ (question 110).
   **Return order, pass 0** (design review): before the return's separation rows, for each `ensures` atom whose cells
   frame the base of a load-rooted `ensures` place, `g_j ==> perm(cell) > none` in clause order with that atom's message,
   emitted only when an `ensures` place has a load.
7. **Measures.** A function's `decreases(**pp)`, `decreases(*gp)`, `decreases(p->q->x)` through 0039 item 8 (framing in
   `f__wf`, the chain-guarded method measure, the explicit check at the caller).
8. **By-value leaves and results.** `owns(*s.q)`, `reads(*s.q)`, `live(s.q)`, `*s.q`, `s.q->x`, `old(*s.q)`: the root is
   the leaf formal (a fixed value, the entry value in an `ensures`, as `*p` of a pointer parameter); `*r.q` is framed
   as `*result` is (0040 item 4). `live(r.q)` and every atom on a result member stay M4.5d2's (question 106).
9. **Loops** (questions 105, 110). Each `requires` place reached through a heap read is an automatic place of every
   loop through its method-entry `eb` (item 6), and, where the function may write a loaded field of its path, also
   through a loop-entry `lb_<site>__<loop>` (`:= null`, then `if (<inner cells held>) { lb := <the place's address> }`
   right before the loop, after the existing case splits). `eb`/`lb` stand where a formal stands in `loop_places`
   (derived keys, as the design review proposed: per-function keys `kDerivedKeyBase + n` for each distinct (root, via)
   so the int-keyed consumers treat a base as one more root; load-free places never get one), `key_cells`, the guards
   (non-null, distinct from the earlier places of their field and from every cell), the splits, snapshots, rows and
   back-edge checks; nothing assigns them in the loop. The `Ref` locals get a lint rule (assigned once, from `null` or a
   guarded read, at method entry or right before their loop). Written atoms
   on heap-read places
   (`owns(**pp)`, an atom on `*q` of a cell, `owns(*p->q)`, `owns(*s.q)`, `owns(*gp)`), their `q` the place's `Ref`
   read at each iteration start through the automatic places' chains (in the invariant) and, for the body's splits
   (`active_w_` at calls and after loads, today a variable), a ghost `Ref` assigned from it at the top of each
   iteration; written struct atoms (question 99): 0040 item 5's
   rows per leaf (amounts, `NOTAUTO`, snapshots, checks), one liveness cell per root; `**pp` in invariants and measures
   by 0044 item 12's argument (the spelling rule lifted).
10. **Encoder rules over load roots:** the marker rules (0042 item 6: a read through a marked parameter's pointee is
    rejected, `ref_to_uninit` on `pp` with `owns(**pp)` in the `requires` included; `check_marker_atoms`,
    `reject_marked_read`, `apply_owning` walk the whole path); the `const` rule at the innermost level (0040 item 2:
    `owns` of a place whose own type is `const`-qualified, and every conversion changing qualifiers below the top);
    guards still read no heap (0037 item 5). **`restrict`:** `restrict_hazard` and `restrict_backed` see load roots;
    the hazard argument re-stated (a loaded pointer is never based on the `restrict` parameter; every root that can
    hold a `T*` is already a hazard by type, now pinned through each new atom). **Lint:** contract `acc`s are not
    pinned (no rule); one rule for item 9's snapshot `Ref` locals, with self-tests.
11. **Still rejected** (each pinned): `live(r.q)`, `owns(*r.q)`, `owns(**r)` (M4.5d2); `*old(*pp)` and `old` inside a
    place path (M8, listed); places through `void *`, unions, function pointers, arrays (M8, M4.7, M4.5e1); atoms in a
    measure, a `check` or a `do … while (0)` invariant (no session).
12. **Categories:** none new. Messages name the place in source terms (`**pp`, `(*ps)->data`, `*gp`, `p->q->x`).
13. **Facts relied on:** P70, S35. 14. **Tests and docs** (below). 15. **Flips** (below).

**S35** (test-mutator writes, il-analyst runs; `tests/viper-smoke/memory/s35_*`, each with a `_fail` twin and a
`vacuity_fail` clone whose `assert false` must fail, hand-written in the emitted style, from the record draft):
- (a) `pp_contract`: `owns(*pp) && owns(**pp)` on both sides, `ensures(**pp == old(**pp) + 1)` with the nested `old`;
- (b) `pp_guarded`: an `acc(ite(…).v_int)` receiver framed only on one path: `f__wf` fails, the method and the caller
  report no Silicon error of their own;
- (c) `pp_call`: the caller's rows in clause order; alias twin `q == *pp` with `owns(*q)` → the separation row;
- (d) `pp_return`: a callee that gives `*pp` away → the pass-0 row fails (and once without pass 0, showing the exit 3
  it prevents); `live(*pp)` at entry and return (the escape twin);
- (e) `pp_move`: a callee moving `*pp` to an object it was given, the `ite` amount of question 108; a swap of two
  targets; twin: a move to an object given only by `live`;
- (f) `pp_reads_fact`: `reads` at both levels, the guarded post = pre fact; twin: `owns(*pp)` moves `*pp`;
- (g) `heap_guard`: `*pp == NULL || owns(**pp)` (question 109) in a method, its `f__wf`, a call and a return; twin:
  the guard unframed;
- (h) `gp` and `member_chain`: `reads(gp) && owns(*gp)` from `main`; `owns(*p) && owns(p->next->x)` with `WF_S` of the
  loaded pointer; twin: `p->next == p`;
- (i) `byvalue_leaf` and `result_leaf`: `owns(*s.q)`, `live(s.q)`, `return s.q`; `*r.q` with the result-root fact;
- (j) `decreases_pp`: recursive `decreases(**pp)`;
- (k) `loop_eb_lb`: `eb` and `lb` places (question 110), `*pp = &x` before and in the loop, a callee keeping `**pp` →
  back-edge row, a `return` in the loop;
- (l) `written_pp` and `written_cellptr`: a chained `q`, the moved case, the unframed case (framing assert first);
- (m) `written_struct`: per-leaf rows; separation of `owns(*q)` and `owns(r->x)` when `r == q`.

## Team (disjoint files; three Sonnet agents and one Opus reviewer from `.claude/agents/`)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | **First, P70** (both modes): the IL of every new contract and invariant shape — `**pp`, `***ppp`, `owns(**pp)`, `live(*pp)`, `cerastes_old(**pp)`/`cerastes::old(**pp)` (C23 folds `&*`), `*gp`, `owns(*gp)`, `live(gp)`, `p->q->x`, `(*p->q).x`, `*p->q`, `(*pp)->x`, `gp->x`, `s.q->x`, `*s.q`, `live(s.q)`, `*r.q`, a written atom on `*q` of a cell, `owns(*q)`/`owns(q->x)` of `struct S *q` in an invariant; `decreases(**pp)`; the `const` qualifiers at each level; and today's verdict of `*s.q` in an invariant (P69's gap 6). **Then S35** through Silicon (test-mutator's fixtures, with vacuity runs). **Then** `src/vir/lower.cpp`, `lowered.h`, `src/viper/`: the state-dependent place roots in every mechanism (record items 3–7), and the `.vpr`/`.map` comparisons against the baseline. |
| **edg-backend** (Sonnet) | **First:** `vir.h`, `vir.cpp`, `check.cpp` — the place grammar with load steps (record item 2), `atom_place`/`member_root`/`AtomCollector` guards, `contract_atoms`/`loop_atoms`, `restrict_hazard`/`restrict_backed` seeing load roots; committed with no behaviour change, baseline compared. **Then** `encode.cpp`: the 17 rejection sites become encodings; `finish_atoms`, `reject_marked_read`, `check_marker_atoms`, `apply_owning` and the `const` rule (innermost level) generalized to load roots; `old` of a heap-read place; the rejections that stay (record item 11), re-messaged; the stale comments; `diagnostics.def` if needed. |
| **test-mutator** (Sonnet) | **First:** S35's fixtures in `tests/viper-smoke/memory/s35_*`, each with a `_fail` twin and a `vacuity_fail` clone. **Then:** the tests and twins below, written from the record before the code runs; the two ports; the flips; the VIR self-test `selftest_cases16.inc` with `VIR_CASES_MIN`; `lint_viper.py` (record item 10) with its self-tests; the doc example files. |
| **soundness-auditor** (Opus, read-only) | **Round 1:** record 0045 (in the background while P70/S35 run). **Round 2:** the code and the tests — the state of each root per mode, `f__wf`'s ghosts and facts over load roots, the call rows' order (every inner load framed by an earlier row), partners when the inner pointer moves, loops (automatic heap places or written atoms), the marker/`owning`/`const` rules over load roots, the `restrict` argument. **One re-audit** of the blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | step 1; record 0045 and its amendments; `doc/`; `internal/encoding.md` (an M4.5c2b paragraph after line 504; §8 "Since M4.5c2b" after 804, line 782's sentence updated); `il-inventory.md` (`## M4.5c2b`, P70, S35); integration, serialized builds, the baseline; status and handoff. |

Any question from an agent, or of mine, stops that work and goes to the owner.

## Tests (`tests/memory/`; `.c` in both modes unless marked; each positive has a twin failing for one reason at one line)

| Positive | Twin (expected) |
|---|---|
| `void set(int **pp)` `requires(owns(*pp) && owns(**pp))` `ensures(owns(*pp) && *pp == old(*pp) && owns(**pp) && **pp == 5)`, body `**pp = 5` | a wrong value → `postcondition`; `owns(**pp)` without `owns(*pp)` before it → `permission` at the contract ("reads `*pp` without owning it") |
| `live(*pp)` in `requires` and `ensures`; the callee compares and passes `*pp` | the old `ptrptr_callee_escape.c` shape with `ensures(live(*pp))` → `permission` at the return (the stored `&local` dies) |
| a callee that moves `*pp` to another owned object, `ensures(owns(**pp))` names the new one at the amount it was given (questions 104, 108); a swap of two `int **` targets | the caller relies on the old object after the call → `permission`; `ensures(**pp == old(**pp))` after a move → `postcondition`; a move to an object given only by `live(q)` with `ensures(owns(**pp))` → `permission` at the return |
| heap-reading guards (question 109): `requires(owns(*pp) && (*pp == NULL \|\| owns(**pp)))`, `p->q == NULL \|\| owns(*p->q)`, a `?:` arm under a heap condition, and the same in a loop invariant | the guard's `*pp` unframed → `permission` at the contract; a caller with `*pp != NULL` not owning `**pp` → `permission` at the call; a body writing `**pp` when `*pp` may be null → `null-dereference` |
| loops with a move before the loop (question 110): `int *old = *pp; *pp = q;` then a loop using both `*old` and `**pp` | a callee in the loop keeping either object → `permission` at the back edge |
| `reads(**pp)`, `old(**pp)`, `decreases(**pp)` recursion | a write under `reads` → `permission`; a measure that does not decrease → `termination` |
| the caller of a heap-read contract: `int x; int *p = &x; set(&p); check(x == 5);` | the caller does not own `x` → `permission` at the call; `set2(&p, &x)` with `owns(**pp) && owns(*q)` → `permission` (separation: one object twice) |
| `***ppp` at depth 3 | a wrong value → `postcondition` |
| `owns(gp) && owns(*gp)` (`static int *gp`), `*gp += 1`; in `main` with `gp = &h` | `owns(*gp)` without `owns(gp)` → `permission`; `*gp` aliasing an owned `*p` → `permission` at the call |
| `struct L { struct L *q; int x; }`: `owns(*p) && owns(p->q->x)`, `owns(*p->q)`, `live(p->q)`, `p->q->x` values | unframed `p->q->x` → `permission`; a wrong value → `postcondition`; a caller passing a self-loop (`s.q = &s`, so `p->q->x` is `p->x`) → `permission` at the call (one cell twice; the static overlap rule cannot see it) |
| by-value `struct P { int *q; }`: `owns(*s.q)`, `reads(*s.q)`, `live(s.q)`, `*s.q`, `s.q->x`, `return s.q;` with `live(s.q)` | without the atom `return *s.q;` → `permission`; `return s.q;` without `live(s.q)` → `lifetime` (the documented c2a shape) |
| `*r.q` of a struct result framed by `ensures(r.q == p && owns(*p) && *r.q == 1)` | unframed `*r.q` → `permission` |
| loops (question 105): `**pp += 1` in a loop with only `requires(owns(*pp) && owns(**pp))`; the same with `owns(*gp)`, `owns(p->q->x)`, `owns(*s.q)`; a loop that moves `*pp` between two owned objects | a callee in the body that keeps the object → `permission` at the back edge; a value claim the loop breaks → `invariant-preserved`; after the move, the entry object's value assumed → `check` |
| written atoms: `CERASTES_INVARIANT(owns(**pp))`, an atom on `*q` with `q` a cell, `owns(*p->q)`, `owns(*s.q)` | entry without the place → `permission`; an iteration that gives it away → `permission` at the back edge |
| written struct atoms (question 99): `owns(*q)` for `struct S *q` walking two struct objects, `owns(q->x)`, `reads(*q)` | a missing leaf at the back edge → `permission`; two `owns` that may be one object → `permission` (separation) |
| `**pp` in an invariant and a measure (P69's shapes extended) | value → `invariant-preserved`; unheld → `permission`; a trap that moves `*pp` → `invariant-preserved` |
| `restrict`: a positive beside heap-read atoms that cannot form a `T*` | **rejected:** `restrict int *p` beside `owns(**pp)`, `live(*pp)`, `owns(*gp)`, `owns(*s.q)`, `owns(p->q->x)` (each through `restrict_hazard`) |
| ports: VeriFast `tests/issue310.c` (`s_data **ps`, `owns(*ps) && owns((*ps)->data)`, `(*f)->data -= 1`) and the struct half of `examples/globals.c` (`static struct counter *c`, `owns(c) && owns(c->f)`; `malloc` in `main` replaced by a `static struct counter`, noted in the header; `leak` left out, M4.5d2) | issue310: `ensures` `d - 2` → `postcondition`; globals: `c->f` without `owns(c->f)` → `permission` |
| — | **rejected (pins):** `live(r.q)` (question 106: M4.5d2); `owns(*r.q)`, `owns(**r)` (M4.5d2); `*old(*pp)` and `old` inside a path (M8, listed); a marker or `owning` whose reads go through a load (`ref_to_uninit` on `pp` with `owns(**pp)` in the `requires`); `owns` of a `const` place at its own level (`owns(**pp)` with `const int **pp`, `owns(*pp)` with `int *const *pp`; `owns(**pp)` with `int *const *pp` is accepted, the `int` not being `const`: corrected during the session); an atom under `!`, inside `==` or as an argument (positions, unchanged); an atom through a `void *` or a function pointer |

Expected sizes, from c2a's: about 30 positives, 50 twins and 20 pins; S35 about 25 fixtures; VIR cases16 about 30.

## Flips (question 107, pre-approved; nothing else may change)

- **Retired for positives of the same shape** (each with a verified test in its place and a twin):
  `reject_owns_double_deref.c`, `reject_read_double_deref.c`, `reject_read_double_deref_invariant.c`,
  `reject_atom_on_cell_pointer.c`, `global_pointer_global_deref_atom.c`, `reject_struct_member_chain_contract.c`,
  `reject_struct_atom_in_invariant.c`, `reject_struct_value_deref_member.c`, `reject_struct_value_reads_member.c`,
  `reject_struct_value_live_member.c`.
- **Re-messaged naming M4.5d2** (question 106), `expect:` line kept: `reject_struct_value_live_result.c`.
- **`Pins:` comments only** (no `expect:` change): `global_pointer_global_unowned.c` (its "owns(*gp) is M4.5c"),
  `struct_init_pointers.c` ("M4.5c2"), `verifast/globals.c`'s "Left out" line (the struct half is now a port).
- `ptrptr_callee_escape.c`, `ptrptr_store_havocked_lifetime.c`, `struct_value_pass_dangling_member.c`,
  `reject_struct_value_owns_member.c` and `global_loop_written_atom.c` stay as they are (their contracts name no
  heap-read atom).

**Viper against the baseline:** every existing `.vpr` and `.map` byte-identical, the rest added files (no accepted
program has a heap-read atom; P69's accepted invariant shapes keep their Viper); the VIR self-test shows only added
cases.

## Docs (invariant 8; every example is a test)

- **Memory and ownership:** "Pointers to pointers" (`live(*pp)`, `owns(**pp)`, and why `*pp == old(*pp)` is written);
  "Pointer parameters and contracts" (places reached through a pointer read from memory, the post-state reading,
  `old(**pp)`); "Structures" (`p->q->x`, `owns(*s.q)`, `live(s.q)`, `*r.q`); "Global variables" (`owns(*gp)`); "Loops"
  (heap-read places and written struct atoms); "`restrict`" (the new hazards); the not-yet list (`memory.adoc:639–668`)
  loses c2b's items and gains `live(r.q)` (M4.5d2) and `*old(*pp)` (M8).
- New doc examples (each a test with `expect:` lines, test-mutator): `memory/pointer_to_pointer_contract.c`
  (`owns(**pp)`, `live(*pp)`), `memory/struct_pointer_member.c` (`p->q->x`, `owns(*s.q)`),
  `memory/pointer_global_target.c` (`owns(*gp)`), `memory/loop_written_struct.c` (a written struct atom).
- **Annotations:** the places a contract and an invariant may name; `annotations.adoc:395–397` corrected.
- **Error messages:** `permission`'s new messages (an unframed inner read, a call needing `**pp`, a return without
  `live(*pp)`).
- **Verifying a program:** the rejected list (`verifying.adoc:~880`).
- **`internal/encoding.md`:** the M4.5c2b paragraph (after 504), §8 "Since M4.5c2b" (after 804), line 782 updated.

## Order of work

1. **Step 1, the worktree and the baseline.** In parallel: the record draft (audit round 1 in the background); P70
   (il-analyst); the S35 fixtures (test-mutator); the VIR block (edg-backend).
2. **S35 through Silicon; the VIR changes** (place grammar, keys, checks, `restrict_hazard`/`restrict_backed` over load
   roots) with no behaviour change: the VIR self-test and a sample regeneration byte-identical. **Commit 2.**
3. **Contracts** (record items 2–8: places, states, `f__wf`, calls, entry, returns, measures; parameters, globals,
   member chains, by-value leaves, `*r.q`; the marker, `owning` and `const` rules over load roots; `restrict` pins):
   lowering, encoder, tests, cases16 part 1, the issue310 port. **Commit 3.**
4. **Loops** (record item 9: heap-read places under question 105's rule, written atoms on heap-read places and on struct
   places, `**pp` in invariants and measures), the `globals.c` struct-half port, cases16 part 2. **Commit 4.**
5. **Audit round 2.** First the full regeneration against the baseline, then audit round 2, the fixes and one re-audit.
6. **Close-out:** docs, `encoding.md`, `il-inventory.md`; `check.sh` (full, at most twice); status and handoff; the last
   commit.

**The pre-agreed cut**, if context runs short after commit 3: written struct atoms in loop invariants (question 99) move
to the next session, with `reject_struct_atom_in_invariant.c` re-messaged instead of retired (ask the owner first: it
changes a flip). Questions 108–110 add about 400–600 lines to the estimate; heap-reading guards in loop invariants are
the second cut candidate (ask first). The `restrict` re-check and the marker/`owning`/`const` rules over load roots are never cut.

## Stop and ask

- Any question from an agent.
- An existing test's outcome changing outside the flips, or any existing `.vpr`/`.map` changing.
- P70: a heap-read place whose IL loses a level (a folded `&*`, a missing `eok_indirect`) in either mode.
- S35: a vacuity clone that passes, Silicon failing first with its own reason (exit 3), or a shape it refuses.
- A wrong verdict today on any shape the session touches (an existing soundness hole).
- Exit 3 on a correct program, or a new test over 60 s.
- Context running short after commit 3 (the cut above).

## Acceptance (commands run at the end)

- `python3 scripts/run_tests.py tests/memory tests/scalar tests/viper-smoke/memory doc/modules/ROOT/examples/memory
  doc/modules/ROOT/examples/error-messages doc/modules/ROOT/examples/verifying doc/modules/ROOT/examples/annotations`:
  all pass.
- **Against the baseline:** every `.vpr`/`.map` in `build/vpr` that exists in `build/m4.5c2b-baseline-vpr` is
  byte-identical, and the VIR self-test shows only added cases.
- `python3 scripts/test_run_tests.py`, `python3 scripts/test_lint_viper.py`, `python3 scripts/run_tests.py
  --check-headers`: green.
- `scripts/build_docs.sh` and `python3 scripts/check_doc_examples.py --html build/doc`: green.
- `scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): green. No Linux run in the session (CI covers it; any
  x86-64 run goes to `matt@clang4.cpp.al`).
- **Status entry:** record 0045, the flips, the audit outcome, the plan file, the first step of the next session
  (M4.5d1 once the vlibc repository exists, else M4.5d2's planning). Handoff §3 gets what was not taken, §8 any new
  question.
- **Commits:** planning; VIR; contracts; loops; status and handoff. All local on `m4.5c2b-heap-read-places`, no Claude
  trailer, never pushed; the handoff gives the owner the push and PR steps after PR #25.
