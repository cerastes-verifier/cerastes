# Plan: M4.5d2b2a — block sites through memory, owners in memory, block carriers in loops (record 0054)

**Approved by the owner on 2026-11-09**, with questions 173–179 answered with the recommendations (below). The owner asked for three Sonnet agents and one Opus agent from `.claude/agents/`, and for any question from me or an agent to stop the work and go to the owner.

Saved at step 1 as `.claude/plans/m4-5d2b2a-owners-in-memory.md`. Branch `m4.5d2b2a-owners-in-memory`, worktree
`.claude/worktrees/m4.5d2b2a`, both from `origin/main` `15bcdc6` (PR #33 merged).

## Context

`plan.md`'s next row is **M4.5d2b2**:
- written `owns_block` in loop invariants (cut from d2b1);
- block sites through loads and on globals;
- owning globals, `static` locals and address-taken locals;
- acyclic owning fields;
- the rest of P4418R0;
- d2b1's four pinned false alarms (handoff §3 "From M4.5d2b1").

**Size.** The planning surveys (two read-only surveys of the code, 2026-11-09) measured it at 3,000–4,600 `src/` lines plus self-tests:

| Item | Lines |
|---|---|
| Block sites through loads and on globals | 600–800 |
| Written invariants | 350–500 |
| Global, `static` and address-taken owners | 500–800 |
| The false alarms | 200–300 |
| Owning fields | 1,000–1,500 |
| P4418R0 | 400–700 |

d2b1 landed about 3,900 lines and needed a cut, so the owner split d2b2 in two (question 173):
- **d2b2a (this session):** everything except owning fields and P4418R0.
- **d2b2b (next):** owning fields and P4418R0.

**Key survey facts:**
- **(a) Block sites are keyed on a formal or the result.** `block_key` (`lower.cpp:3015`) returns `atom_place(...).root`, which `vir.cpp:1275` special-cases for `OwnsBlock`. About 12 consumers read a key ≥ 0 as a formal index and < 0 as the result: `e.args[block_key(s)]` (3399, 6978), `block_marker`, `block_mode`, `block_partner`/`block_consumes`, `returned_owners`, `with_atom`, the framing test (4619, 4631), and `restrict_backed`/`restrict_hazard`. Global keys (`kGlobalSiteBase`) and derived keys (`kDerivedKeyBase`) are positive, so they would be misread. This is the main coupling risk.
- **(b) 0045's machinery is reusable as is.** `PlaceKey{root, via, path}`, `collect_derived`, `site_root`, the `eb_`/`lb_` snapshots, `site_guard` and the encoder's `place_pointer`. `owns_block(g)` is `Deref(GlobalAddr g)` with one load step, so "through loads" and "on globals" are one feature.
- **(c) An owner that outlives a call cannot keep its ownership in a method ghost.** For a global or `static` owner, ownership must travel in the contract: `owns(g)` of an owning global includes `g != NULL ==> owns_block(g)`.
- **(d) The three owning-loop false alarms come from snapshot guards.** The carriers' "not an earlier owner's block" guards are loop-entry snapshots (`dup_`/`ad_`). Written invariants and owners in memory use the same carriers, so the redesign comes first.
- **(e) A dormant holder.** The `HeapHolder` for a formal with a block site (`lower.cpp` ~10944–10973) looks unreachable today (`fa.sites` never holds `OwnsBlock`). Turning it on changes existing Viper.

**The papers** (P4418R0, P4417R0, read again at planning):
- P4417 (2.3) and [profiles.inval.owner] p5 Note 3 exempt owners with static storage from deleting.
- p3: owner to owner transfers; owner to non-owner aliases.
- p4: an allocation's result goes to an owner in its full-expression.
- p6: only an owner is deleted.
- `not_invalidating` [profiles.inval.verify] is checked at the definition.
- `std::no_dangling` returns `std::forward<T>(x)` and waives escape and validity.
- §8.2: raw storage is outside the profile.

## Owner decisions (2026-11-09, handoff questions 173–179, each the recommendation)

- **173:** d2b2 is two sessions.
  - d2b2a: block sites through memory; owning and marked globals, `static` locals and address-taken locals; written `owns_block` invariants; the carrier redesign that lifts the four pinned false alarms.
  - d2b2b: acyclic owning fields and P4418R0's rest.
- **174:** static owners are exempt at `main`'s end (P4417). `main`'s leak row exempts the blocks of owning globals and owning `static` locals. A block held only through a non-owning global stays a leak (question 123).
- **175:** `owning` on a parameter stays a borrow (0029). P4417's transfer is the contract `requires(p == NULL || owns_block(p))` with no partner. d2b2b's docs map each `[[owner]]` position to cerastes's spelling.
- **176:** the nightly full check is cancelled at its 220-minute timeout on every profile since 2026-10-29, and the `profiles=all` dispatch has not run. **The owner handles CI; it is outside this plan.** S8's questions 169–172 stay open.
- **177 (d2b2b):** `--strict-owner`, P4417's p4 and p6 as an opt-in like `--strict-init`.
- **178 (d2b2b):** `cerastes::not_invalidating` on a pointer parameter, a checked attribute:
  - the contract gives back every place it takes through `p` and consumes no block through `p`;
  - implicit `ensures` keep every owning pointer reachable through `p`.
- **179 (d2b2b):** `no_dangling` is not provided in C. M7 reads `std::no_dangling` as a checked identity.

**During the session** (2026-11-09, after spike S39, each the recommendation):
- **180:** `owning_loop_alias_becomes_owned_false_alarm.c` cannot verify as written (S39 `d2a`: if the loop ended with
  `v` null, `w`'s block would leak; ordinary loop incompleteness). The pin is retired for a positive
  `owning_loop_alias_becomes_owned.c`: the same program with `i < 3 || v != NULL` added to its invariant (S39 `d2b`).
  Its twin is the program without the conjunct, expecting only `error leak @ L30`, with a comment that the loop needs
  the invariant. The docs say that an owner assigned on some iterations needs such an invariant.
- **181:** the flag limit after a handoff (S39 `d2d`: a read of `*w` after `v = w;` in a later iteration is
  `uninitialized`). Part B tries flag-kept clauses per block under each carrier's guard, spiked first. Pre-agreed cut:
  if no shape verifies within the part, it is pinned as a documented false alarm
  (`owning_loop_handoff_read_false_alarm.c`) for M4.8.

**Standing:**
- 69: the EDG merge waits for the box; no EDG work.
- 77: one context.
- 160: a new test's expect line may be corrected after its first run when the test fails for its designed reason at a neighbouring line or category. Each correction is reviewed in audit round 2 and listed in status.
- 0030 item 5 and 0046 item 9: twins for trusted contracts.
- S7's style, and no comment on a brace line.
- An agent's question stops that work and goes to the owner.

## Step 1 (markdown only; commit 1, the planning commit)

1. `git worktree add .claude/worktrees/m4.5d2b2a -b m4.5d2b2a-owners-in-memory origin/main`.
2. Clone the submodules with `--reference` (edg, silicon, then silver), then run `scripts/bootstrap.sh`.
3. Save this plan as `.claude/plans/m4-5d2b2a-owners-in-memory.md`.
4. Update `internal/handoff.md`:
   - header and §1: PR #33 merged, d2b2a in progress;
   - §2: the S8 dispatch and the nightly timeout belong to the owner (176);
   - §3: the d2b2 notes move into this plan's Inputs; the P4418 notes stay for d2b2b;
   - §4.2: the d2b2a and d2b2b paragraphs;
   - §8: 173–179 answered.
5. Update `plan.md`:
   - the header names d2b2a's planning;
   - the d2b2 row splits into **M4.5d2b2a** and **M4.5d2b2b**. The d2b2b row reads: owning fields with target `d`; `not_invalidating` (178); `--strict-owner` (177); the `[[owner]]` mapping (175); `no_dangling` not in C (179).
6. Commit locally, with no Claude trailer.

**Baseline.** A detached worktree `.claude/worktrees/m4.5d2b2a-base` at `15bcdc6` runs bootstrap, then `run_tests.py -j 8` and the VIR self-test. Copy its outputs to `build/m4.5d2b2a/baseline/{vpr,vir-selftest}`, then remove the base worktree.

## Record 0054 — block sites through memory, owners in memory (written before code)

Amends:
- 0029 item 1 (target `v` on globals, `static` locals and address-taken locals);
- 0041 items 7, 10 and 12;
- 0045 items 7 and 10;
- 0047 items 4, 7 and 10;
- 0052 items 2, 3, 4, 6(a) and 8.

1. **Scope:** question 173's d2b2a list. Fail closed on the rest (item 10).

2. **Block sites through memory** (0047 item 4; 0052 item 2).
   - **Forms.** `owns_block(E)` and `cerastes_owns_vlibc_block(E)`, where `E` is a pointer value reached from a formal, the result or a global through loads and member steps: `*pp`, `p->q`, `**ppp`, `g`, `*gp`, `r->q`, a by-value leaf `s.q`. Both clause lists.
   - **The place** is 0045's `PlaceKey` with derived keys. It is read in the entry state in a `requires` and in the post-state in an `ensures` (0045 item 3, question 104). It is framed by the atoms before it (`owns(*pp)` before `owns_block(*pp)`; `owns(g)` or the implicit atom before `owns_block(g)`).
   - **Partners** are matched by place: an `ensures` site whose post place is its `requires` site's entry place.
   - **The three cases:**
     - a `requires` site alone consumes; the caller's `*pp` keeps a dangling value, and its uses are `lifetime`;
     - an `ensures` site alone gives a whole block: the callee holds it, and there is no amount (0052 item 2);
     - both sites borrow.
   - **Bodiless contracts:** 0052 item 2a is unchanged. A partnerless site on a formal's place stays rejected, naming M6.
   - **Re-keying.** Every consumer of `block_key` takes the site's place key (survey fact (a)); no ≥0/<0 index trick is left.
   - **The leak exemption** at the end label reads the post-state place, framed by its atom.
   - **Loops.** `requires` block sites through loads become automatic loop places (0045 item 10's snapshots, 0047 item 10's token place). The dormant formal holder (fact (e)) is decided by spike S39 and the audit. If it changes any existing program's Viper, stop and ask.
   - **Still rejected:** a block site under `old` and in a measure.

3. **Carriers by current state** (lifts audit 3b's false alarms; 0052 item 3 Loops).
   - Today a loop carrier's guard "not the block of an earlier carrier" is a loop-entry snapshot.
   - It becomes loop-carried ghost state: which carrier holds which block, updated at owner writes, with an exclusivity invariant.
   - A block that becomes shared inside the loop (`b = a;`, `v = w;`) moves to one carrier. One that stops being shared (`a = NULL;`) stays with the other.
   - S39 (d) fixes the shape and its cost.

4. **Written `owns_block` in loop invariants** (0052 item 6(a), audit S10). This covers a local, a load or a global.
   - **The carrier.** A whole-block carrier keyed on the current value: the token and its value, liveness, every cell, the block facts and `flag ==> range`. It is shared with the owner carrier.
   - **Rows:** ending rows at entry and at the back edge (`permission` at the loop).
   - **Guards:** NOTAUTO guards against heap, parameter and cell places (0040 item 5).
   - **Exemption:** its term joins the back-edge leak exemption.
   - **Owning loop variables:** a written site on one merges with its carrier; there is one carrier per block.
   - `heap_loop_swap_false_alarm.c` (no invariant) stays. Its shape verifies once the invariant is written.

5. **Owning and marked globals and `static` locals** (target `v`; question 174).
   - **Accepted** on a mutable global or `static` local whose type is a pointer to an object type. It may not be `void*`, a pointer to `const` or `volatile`, or a pointer to a function. Every declaration must carry the same attribute (`annotation_mismatch` otherwise).
   - **Initializer.** The initializer must be absent or a null pointer constant; any other is `unsupported_annotation`. `program_start` gives `g == NULL`.
   - **The deep atom.** Every `owns(g)` of an owning global includes item 2's site `g != NULL ==> owns_block(g)`, in the `requires` and the `ensures`. That covers written atoms and the implicit `owns(s)` of `static` locals (question 80).
   - **`reads(g)`** gives the value only. Reading `*g` needs `reads(*g)` (0045).
   - **In the body.** Ghosts follow 0052 item 3: `held_g`, initialized at entry to `g != NULL`, and `tv_g`. Rows are written at every store to `g`:
     - by name;
     - through a `T**` that may equal `&g` (a guarded row, path-precise);
     - inside an expression.

     Two rows at each such store:
     - **value rule** (`permission`): `new != NULL ==>` the function holds `new`'s block whole;
     - **reassignment** (`leak`): the old value's block, if held, is still owned by some other owner.

     After every call and every `free`, `held_g` is updated. Owner lists gain the held owning globals by their current value.
   - **Returns and calls.** At a return, the `ensures` site of `owns(g)` checks `g != NULL ==>` a whole block (`permission`: "returns while owning global `g` holds no block it owns: set it to `NULL` after freeing it"). At a call, the callee's `owns(g)` takes the block with the cell and gives back what `g` holds afterwards.
   - **`main`** (question 174). Its leak row exempts `g != NULL && t == tok(block_of(g))` for each owning global and owning `static` local.
   - **Loops.** A held owning global is a loop place whose block is carried by current value (item 3).
   - **Marked globals and `static` locals** (`ref_to_uninit`). In the default mode the marker documents intent. Under `--strict-init`, stores to them follow 0052 item 4's rules for marked locals.

6. **Owning address-taken locals** (target `v`).
   - **Ghosts and rows:** 0052 item 3's ghosts and rows, with the value read from the cell (0032).
   - **Stores:** rows at stores by name (`write_cell`) and, guarded and path-precise, at stores through any `T**` that may equal `&v`.
   - **`&v` passed to a callee:** the callee's contract decides.
     - `owns(*pp)` alone lets the callee change `v`. After the call, `held_v := held_v && token held`, and `v`'s exit row checks its new value.
     - `owns(*pp) && (*pp == NULL || owns_block(*pp))` lends the block too, by item 2.
   - **Pre-agreed cut 1.**

7. **The static struct leaf cover** (0052 item 2, audit 2 N1). An implicit `owns(s)` of a `static` struct covers an `owns`, `reads` or `live` atom on a leaf (`return &s.x;`). `f__wf`'s per-leaf facts must stay well formed; the earlier attempt broke them.

8. **The lint** (`lint_viper.py`) covers:
   - token reads in block sites through loads;
   - the carrier ghosts and their rows;
   - `main`'s exemption terms, admitted only for owning globals and `static` locals.

9. **Categories:** none new. `leak` and `permission` gain the owner-in-memory messages.

10. **Still rejected** (fail closed; one pin each, naming the session):
    - owning fields (EDG's #1835 until d2b2b registers `d`);
    - P4418R0's attributes (unregistered);
    - a non-null initializer of an owning global;
    - `owns_block` under `old` or in a measure.

11. **Facts:** P75 and S39.

## P75 and spike S39 (il-analyst, before code)

**P75 (IL; `il-inventory.md`):**
- `owning` and `ref_to_uninit` on globals, in both modes: redeclared `extern`, with the union of lists of P74;
- on `static` locals, including C++ dynamic init, which stays rejected;
- on address-taken locals;
- an owning global's initializer shapes (P63 H4);
- `&g` and `&v` reaching `T**` stores.

**S39** (hand-written Viper in the target shapes, each with Silicon's verdict and time; fixtures in `build/m4.5d2b2a/s39/`, the kept ones in `tests/viper-smoke/memory/`):
- (a) A callee allocating into `*pp` with a partnerless `owns_block(*pp)`, then the caller's `free`.
- (a') A consuming `owns_block(p->q)`, then the caller's later use of `p->q` (`lifetime`).
- (b) An owning global `g` and `init()` with `owns(g)`, which frees the old block and allocates a new one, called twice by `main`. This covers `main`'s exemption, the reassignment row, and a return after `free` without `NULL`.
- (c) A written `owns_block` invariant in the swap shape, with timing.
- (d) The current-state carriers on the three audit 3b shapes and their twins, timed against today's.
- (e) The static struct leaf cover with `f__wf`.
- (f) An address-taken owner with a store through an alias, and `&v` passed to a callee.
- (g) An owning global in a loop.
- (h) The dormant formal holder (fact (e)): what activating it changes.

A failed spike changes the record before code. Audit round 1 reads the draft with S39.

## Parts and commits

1. **Commit 1:** planning (Step 1).
2. **Commit 2:**
   - record 0054 (draft, revised after S39 and audit round 1);
   - P75;
   - the S39 fixtures;
   - `build/m4.5d2b2a/vprdiff.py`. It compares outputs with the baseline under an allow-list. d2b1's copy was lost with its worktree, so it is re-created.
3. **Commit 3, Part A:** block sites through memory.
   - Re-keying comes first, proven neutral by vprdiff (no output changes).
   - Then loads and globals, and the loop places.
   - Its tests and flips.
4. **Commit 4, Part B:** loops.
   - The carrier redesign (item 3), with the three false alarms flipped.
   - Written `owns_block` invariants (item 4).
5. **Commit 5, Part C:** owners in memory. Owning and marked globals and `static` locals (item 5), then address-taken owners (item 6), on Part B's carriers.
6. **Commit 6, Part D:** the static struct leaf cover (item 7).
7. **Commit 7:** audit round 2 fixes and the re-audit's.
8. **Commit 8:**
   - docs, status and handoff (with its Questions section);
   - `plan.md`'s Done row;
   - record 0054 accepted.

The soundness-auditor reviews each part's diff before its commit (CLAUDE.md). Round 2 covers the whole set, followed by one re-audit of blocking fixes.

**Pre-agreed cuts, in this order, if the context runs short:**
1. Address-taken owners (item 6). `reject_owning_address_taken.c` stays, re-messaged to d2b2b.
2. The carrier redesign (item 3). The three false-alarm pins stay, re-messaged; written invariants then build on today's guards.

Stop and ask before any other cut.

## Team (from `.claude/agents/`: three Sonnet agents with disjoint files, one Opus reviewer)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | **First:** P75, S39 and `vprdiff.py`. **Then:** `src/vir` (`vir.h`, `vir.cpp`, `check.cpp`, `lower.cpp`, `lowered.h`) and `src/viper` (`emit.cpp`, the VIR self-test cases, `VIR_CASES_MIN`). A fresh il-analyst takes over if its context runs low (d2b1's lesson). |
| **edg-backend** (Sonnet) | `encode.cpp`: `spec_block_atom` and `finish_atoms` through loads and globals; `read_local_markers` and `reject_variable_markers`; `add_static_local_atoms` with the deep atom; the store hooks for owners in memory. Also `register_attributes.cpp` (comments only; no new target), `annotation_checks.cpp` and the builds. |
| **test-mutator** (Sonnet) | Tests and twins; exactly the Flips list; `lint_viper.py` with `test_lint_viper.py`; the doc example files. |
| **soundness-auditor** (Opus, read-only) | **Round 1:** the record draft and S39. **Round 2:** the code and tests. **One re-audit** of the blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Step 1; record 0054; docs prose; `encoding.md`; integration; serialized builds (`lockf build/m4.5d2b2a/build.lock`); status and handoff. |

**Rules for every agent:**
- Any question stops that work and goes to the owner.
- Agents hold builds before a directory-wide run, and only one agent runs `run_tests.py`.
- Single-file runs during the work use `build/cerastes verify`.
- Code style: `scripts/format.sh`, brace init, `U`, one declarator per declaration, and no comment on a brace line.
- A new `.cpp` updates `object_identity.py`'s TU constants.

## Tests (each positive has a twin failing for one reason at one line; `.c` in `c23, c++20` where valid; positives `native-ok`)

- **Part A:**
  - `owns_block(*pp)`: given by an allocating callee, consumed, borrowed;
  - `owns_block(p->q)`, `owns_block(g)`, `owns_block(*gp)`, a by-value leaf;
  - the vlibc spelling through a load;
  - a heap-reading guard beside one;
  - a loop over a `requires` site through a load;
  - the bodiless 2a rejections, which stay.
- **Part B:**
  - the swap shape with a written `owns_block` invariant;
  - an invariant through a load;
  - a written site merged with an owning loop variable;
  - the three former false-alarm programs, verified, each with a twin that drops the block at the back edge.
- **Part C:**
  - an owning global with init and cleanup;
  - `main` holding at exit: exempt, while a non-owning global's block is still a `leak`;
  - reassignment `leak`;
  - a return after `free` without `NULL` (`permission`);
  - `g = &x` (`permission`);
  - `reads(g)` cannot free;
  - an owning `static` cache;
  - a marked global under `--strict-init`;
  - a non-null initializer (rejected);
  - an owning global in a loop;
  - address-taken owners: `&v` to an allocating callee, and a store through an alias.
- **Part D:** `return &s.x;` covered, with its twin.
- **From audit round 1** (each a must-fail twin unless marked):
  - a bodiless borrowed site with `*pp == q` (the tie fails in `f__wf`), and a bodiless deep atom with `g == p`;
  - C `int *g` owning without an initializer, then `if (g) free(g);` in `main` (`invalid-free`);
  - a non-owning alias of an owner's block in a written invariant, and two written sites sharing a block at entry;
  - a consumer of `reads(g) && owns_block(g)` that returns without setting `g` to `NULL`, and a consumer of `owns(g)`
    (a `requires` with no partner) followed by `main`'s end;
  - `&v` passed to a callee that only gives a block, then a loop (a positive);
  - a leaf of another type in the static struct cover, and a covered atom with a user read after it
    (`ensures(r, owns(*r) && *r == 1)`, a positive);
  - `owning_loop_swap.c` and `owning_loop_replace.c` unchanged under item 3's carriers (existing tests).
- **Byte identity:** vprdiff over the whole suite against the baseline. Only allow-listed tests may change, and the VIR self-test changes only in its new and converted cases.

## Flips (pre-approved with this plan; any other changed outcome stops the session)

- **Retired for positives of the same shape**, each with a twin:
  - `reject_owning_global.c`, `reject_owning_static_local.c`, `reject_owning_address_taken.c`;
  - `reject_ref_to_uninit_global.c`, `reject_ref_to_uninit_static_local.c`;
  - `reject_owns_block_global.c`, `reject_owns_block_load.c`, `reject_owns_block_invariant.c`.

  New pins take the still-rejected rows of item 10.
- **Pinned false alarms retired for positives of the same program**, each with a twin:
  - `owning_loop_becomes_shared_false_alarm.c`;
  - `owning_loop_alias_becomes_owned_false_alarm.c` (question 180: its positive adds `i < 3 || v != NULL` to the
    invariant, and its twin, without the conjunct, expects only `error leak @ L30`);
  - `owning_loop_stops_shared_false_alarm.c`;
  - `static_struct_member_result_false_alarm.c`.
- **Doc examples whose `expect:` changes from `rejected` to `verified`**, with their prose: `examples/annotations/locals/owning_global.c`, `marked_global.c` and `owning_address_taken.c`.
- **Message and `Pins:` only:**
  - `reject_struct_owning_field.c` (names d2b2b; its stale "target p" fixed);
  - `heap_loop_swap_false_alarm.c`, `heap_loop_replace_false_alarm.c` and `vlibc_alloc_loop_replace_false_alarm.c` stay as written.
- **VIR self-test:**
  - `hb_rej_load`, `hb_rej_global` and `hb_rej_invariant` are converted to lowering cases;
  - `hb_rej_beside_live` stays.

## Docs (invariant 8; every example is a test)

- **`memory.adoc`:**
  - block sites through memory, rewriting "Allocated memory"'s not-yet lines;
  - a new section "Owning globals and `static` locals", with the static-owner exemption;
  - address-taken owners;
  - written `owns_block` invariants under "Loops";
  - the owning-loop shapes;
  - the not-yet list rewritten for d2b2b.
- **`annotations.adoc`:** `owning` and `ref_to_uninit` on globals, `static` locals and address-taken locals.
- **`error-messages.adoc`:** the new messages.
- **`verifying.adoc`:** marked globals under `--strict-init`.
- **The tutorial's heap chapter:** an owning global (record 0049 item 4).
- **`internal/encoding.md`:** record 0054.

## Stop and ask

- Any agent question.
- Any changed outcome outside Flips.
- Any existing program's Viper changing outside vprdiff's allow-list.
- A spike that contradicts items 2, 3 or 5.
- A cut beyond the pre-agreed ones.
- An exit 3 or a timeout over 120 s not fixed within its part.

## Acceptance (run at the end)

- `scripts/check.sh`, the full check with `CERASTES_CHECK_BASE` unset: green, run at most twice.
- `python3 build/m4.5d2b2a/vprdiff.py build/m4.5d2b2a/baseline/vpr build/vpr`: only allow-listed tests differ.
- The VIR self-test at the new `VIR_CASES_MIN`.
- `scripts/format.sh --check`; no new comment on a brace line in the diff of `src/`.
- `python3 scripts/run_tests.py tests/memory tests/vlibc doc/modules/ROOT/examples` on the x86-64 box, in S8's `ubuntu-26.04` container (`~/cerastes-s8/run.sh`), if reachable.
- The status entry:
  - done and not done;
  - the flips and corrections;
  - surprises;
  - record 0054;
  - the first step for d2b2b's planning.
- Commits stay local on the branch; the owner pushes.

## Inputs (moved from handoff §3 at step 1)

Verbatim from `git show 15bcdc6:internal/handoff.md` §3. Each is handled as the plan says: the cut (a) is record
0054 item 4, the four false alarms items 3 and 7, `alloc_<t>`'s note goes to M4.8 unless S39 (b) needs it; the P4417
wording notes stay in handoff §3 for d2b2b.

At planning, d2b1's worktree `.claude/worktrees/m4.5d2b` is gone (removed after PR #32's merge), and with it
`build/m4.5d2b/` (the audits, the S38 fixtures, `vprdiff.py` and the kept tests in `d2b2-tests/`): the written-invariant
tests are written again from record 0054 item 4, and `vprdiff.py` is re-created.

**From M4.5d2b1** (record 0052, audits `build/m4.5d2b/audit{1,2,3,3b}.md` in the worktree)
- **Cut to d2b2:** written `owns_block` in loop invariants (record 0052 item 6(a), the plan's pre-agreed cut (a); tests kept
  in `build/m4.5d2b/d2b2-tests/`; pin `reject_owns_block_invariant.c`); audit S10's points apply (NOTAUTO guards, one
  carrier with an owning loop variable, the written sites in the exemption). [d2b1]
- **Documented false alarms, pinned, for d2b2:** a covered atom on a leaf of a `static` struct (`return &s.x;`,
  `static_struct_member_result_false_alarm.c`; covering a leaf broke `f__wf`, audit 2 N1); owning loop variables whose
  block becomes shared inside the loop (`owning_loop_becomes_shared_false_alarm.c`,
  `owning_loop_alias_becomes_owned_false_alarm.c`: a `permission` row, then a consequential `leak`) or stops being shared
  (`owning_loop_stops_shared_false_alarm.c`), because the carriers' guards are loop-entry snapshots (audit 3b). [d2b1]
- **Limits stated in record 0052:** a given amount is an unknown fraction to the caller (a callee hands back what it took
  by returning the partner); written atoms never cover (question 159); a bodiless declaration nothing calls is not
  checked against rule 2a (checking it changed 30 tests' Viper; audit 2 N3); `owns(**pp)` of a block stored in `*pp`
  needs d2b2's `owns_block(*pp)`; an owner freed inside a loop must be reset to `NULL`. [d2b1]
- **Notes not taken:** two reports for one fault in a few loop shapes (audit 3 N2: an owner freed through an alias and left
  non-null; N3: a `NULL`-head loop without `v == NULL` in its invariant; the docs say how to avoid both); an owner
  entering a loop with the other allocator's block is reported at entry even if the loop frees it first (audit 3 N4,
  documented). [d2b1]
- **Process:** the first il-analyst ran out of context after Part A and the owning locals outside loops; a fresh one (same
  role) did the loops and the audit fixes. Only one agent runs `run_tests.py` (a second run empties `build/vpr`). [d2b1]

**From M4.5d2b2's notes:**
- `alloc_<t>` does not ensure `block_of(r) != GB_g()`, so `&x != &g` is not proved in a function that holds nothing of
  `g`; d2a's typed views do not state it either (one rule; adding it changes the lint-pinned text and every program with
  cells and globals) [b2a audit 1 N3; to M4.8 if d2b does not need it].
