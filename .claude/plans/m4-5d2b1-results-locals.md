# Plan: M4.5d2b1 — partnerless results, owning locals, marked locals (record 0052)

**Approved by the owner on 2026-11-05.** Questions 152–155 were answered with the recommendations, and questions 150–151 with the recommendations plus the brace rule below.

**Owner instructions (2026-11-05):**
- Branch on top of S7's `s7-code-style` (`a660abc`), since S7 is not merged yet and the owner is away. The branch is `m4.5d2b1-results-locals`. Its worktree is `.claude/worktrees/m4.5d2b`.
- Use three Sonnet agents and one Opus agent from `.claude/agents/`.
- Questions 150 and 151 take the recommendations:
  - lines over 120 columns are re-wrapped as they are touched;
  - the 132 existing `{ // comment` lines stay;
  - **from now on, no comment on a line with a brace** (`{ // …` or `} // …`), in new or touched code.
- Any question from me or an agent stops the work and goes to the owner.

**Baseline:** `build/m4.5d2b/baseline/{vpr,vir-selftest}`, copied from S7's `build/s7/baseline`. That is the suite's output at `1da1763`, which S7 proved byte-identical through `a660abc`.

## Context

**Why this session.** plan.md's next row is M4.5d2b:
- `ensures` atoms with no `requires` partner;
- owning locals, globals and fields;
- `ref_to_uninit` on locals and P4222R3's strict rules;
- the rest of P4418R0;
- the leftovers from d2a and d2v (handoff §3).

**What the planning surveys found** (`build/m4.5d2b/survey-{encoder,tests}.md`, `p74.md`):
- **Size.** About 4,500–7,000 changed `src/` lines:

  | Item | Lines |
  |---|---|
  | Partnerless atoms | 1,100–1,500 |
  | Owning locals | 600–800 |
  | Owning globals | +500–900 |
  | Marked locals | 250–350 |
  | Owning fields | 1,000–1,500 |
  | P4418R0 | 400–700 |
  | Leftovers | 600–900 |

  That is beyond one context (question 77), hence question 152's split.
- **Registration (P74).** No fork commit is needed:
  - EDG keeps `owning` (`"?(X)" "r|v|p|d"`), `ref_to_uninit` (`"r|v|p"`) and a candidate `not_invalidating` on the variable's or field's `source_corresp.attributes`, in both positions and both modes.
  - A redeclared global's list is the union of its declarations. Each secondary declaration keeps its own list, and EDG raises no mismatch, so the encoder checks.
  - `void*` lands like `int*`.
  - A field is found from `p->q` through its `enk_field` operand. Cycles are the encoder's to find.
- **The leak rows.** There are two: `leak_row` (the `forperm` after the end label) and the loop back-edge row. No scope-exit leak exists, and `Fn::Scope` holds only address-taken cells.
- **Block sites are keyed on a formal or the result only** (`block_key`). Owning globals and fields need block sites through loads; that is the main cost of d2b2.
- **Partnerless atoms and amounts.** Numbering output amounts from `nreq` skips every partner path, since every `->amount` use already guards `amount < nreq`.
  - `LMethod::results` already takes a `Perm` result.
  - The caller's after-call splits (`result_splits`) must cover the new atoms, or Silicon exits 3.
  - **The design problem:** a callee can give only what it holds beyond its partnered returns, and `where()` returning `&s` holds only its implicit `owns(s)`'s amount. Record item 2's "covered or given" rule solves this.
- **Ports:** `account.c`, `deposit.c`, `swap.c`'s `main`, `malloc_block.c`'s twin and `globals.c` were all ported by d2a and pass (42 tests). This session adds the shapes they could not take, for example `account.c` with VeriFast's contracts, `owns(r->balance)` after `owns_block(r)`.
- **The papers** (P4418R0, P4417R0, read at planning):
  - P4417 [profiles.inval.owner] makes `[[owner]]` on a parameter a transfer. 0029's `owning` on a parameter is a borrow (67 test files use `owning`).
  - P4417 exempts owners with static storage from deleting.
  - It requires an allocation's result to initialize an owner or be deleted in the same full-expression.
  - It keeps raw storage out of the profile (§8.2's drafting note).
  - These are d2b2's (question 152).

## Owner decisions (questions 152–160, 2026-11-05: each the recommendation)

- **152, the split**. **d2b1 (this session):**
  - partnerless `ensures` atoms;
  - owning **automatic** locals;
  - `ref_to_uninit` on automatic locals with P4222R3's strict rules;
  - the d2a/d2v leftovers;
  - the vlibc spelling.

  **d2b2 (next):**
  - block sites through loads;
  - owning globals and `static` locals;
  - owning address-taken locals;
  - acyclic owning fields;
  - P4418R0's rest. d2b2's planning decides its spellings (`not_invalidating`, `no_dangling`, a strict profile mode), borrow against transfer on parameters, and static owners at `main`'s end.
- **153, a vlibc block in contracts**: `cerastes_owns_vlibc_block(p)` (C only; vlibc is rejected in C++ until M7). It means `owns_block` with the token's value `false` (record 0050 item 3); `owns_block` and `owning` keep meaning the platform's allocator.
- **154, a `ref_to_uninit` function's returns under `--strict-init`**: a return must be one of:
  - a marked local;
  - a call to a `ref_to_uninit` function;
  - a null pointer constant.

  Anything else is rejected. This differs from P4416 p6, which counts null as "initialized" and so would reject every allocation wrapper's `return NULL;`.
- **155, `must_init` on a local**: it stays refused (P4222 defines `must_init` for parameters only). The pin keeps EDG's #1835; only its `Pins:` comment stops naming d2b.
- **156, 157 (during the session, spike S38 (a)):** a covered atom's output amount is assigned `write` (it grants nothing); a covered `live(r)` gives pointer use only, and reads of `*r` need `owns(*r)`/`reads(*r)`, which a returned atom covers by place equality.
- **158, 159 (during the session, audit round 1 B3, B4; `build/m4.5d2b/audit1.md`):** a partnerless atom in a contract without a body is accepted only on a fresh result (no equation of the result, or a pointer read through it, with anything but NULL), else rejected naming M6, closing a hole open since d2a (`reject_owns_block_ensures_no_partner.c` stays rejected, re-messaged, instead of its planned flip); only implicit atoms cover (a static local's `owns(s)`, an `owning` parameter's borrow), so `&&` between written atoms stays separating.
- **160 (during the session):** a new, uncommitted test whose first run fails for its designed reason at a different line or a neighbouring category may have that expect line corrected, reviewed by the auditor in round 2 and listed in the status entry (as question 102).

**Standing:**
- 69: the EDG merge waits for the box.
- 77: one context per session.
- 0030 item 5, 0046 item 9: every trusted contract has a twin that violates its `requires`.
- The S7 style: `scripts/format.sh`, brace init, `U`, one declarator, and **no comment on a brace line**.

## Step 1 (markdown only; commit 1, the planning commit)

1. Rename the branch to `m4.5d2b1-results-locals` (done) and save this plan with the owner's answers.
2. **`internal/handoff.md`:**
   - header: points at `git show a660abc:internal/handoff.md`;
   - §1 state: S7 is unmerged and this branch stacks on it;
   - §2 owner steps: S7 first, then this branch;
   - §3: the d2b notes move into this plan's Inputs; d2b2's part stays;
   - §4.2: the d2b1 and d2b2 paragraphs;
   - §8: 150 and 151 answered with the brace rule, and 152–155.
3. **`plan.md`:** the header names d2b1's planning. The d2b row is split into **M4.5d2b1** and **M4.5d2b2**.
4. **The brace rule:**
   - `CLAUDE.md` (Conventions, beside the style line);
   - the developer guide's "Code style" section (`doc/modules/ROOT/pages/dev/`);
   - the memory note (done).
5. Commit 1 locally, with no Claude trailer.

## Inputs (from handoff §3, how each is handled)

- `owns_block`/`owning` name the platform's allocator only → item 7 (question 153).
- The false alarms of a block allocated in an iteration and kept in a variable declared outside the loop:
  - lifted for an **owning** variable (item 3);
  - `heap_loop_replace_false_alarm.c` and `vlibc_alloc_loop_replace_false_alarm.c` (no annotation) stay as written, their messages naming `owning`;
  - verified twins with `[[cerastes::owning]]` are added.
- `heap_loop_swap_false_alarm.c` → a written `owns_block` invariant (item 6a) verifies the shape with an invariant; the pin (no invariant) stays.
- A block site whose own guard reads the heap, a global or `old` → item 6b.
- `owning` beside `owns_block` in both lists → item 6c.
- `restrict` beside a block-only parameter → item 6d.
- `two((free(p), 0), p)` → item 6e.
- Strict-init rejects every `ref_to_uninit` result call → item 4.
- `return &s;` of a `static` local → item 2 (covered).
- `alloc_<t>`'s `block_of(r) != GB_g()` → still not needed (M4.8).
- B1's split costs → watched (item 3 adds no holder for an unannotated program).

## Record 0052 — results, owning locals, marked locals

Written before code (invariant 6). Audit round 1 reads the draft together with S38.

**Amends:**
- 0029 items 1 (target `v` for automatic locals) and 2 (`ref_to_uninit` target `v`);
- 0037 items 6, 7 and 15;
- 0038 item 3;
- 0042 item 8;
- 0045 item 7;
- 0047 items 4, 5, 10 and 11;
- 0050 item 3.

1. **Scope:** question 152's d2b1 list. Fail closed on the rest (item 8).

2. **Partnerless `ensures` atoms: covered or given.** An `ensures` atom with no `requires` partner of its kind and key is accepted wherever 0045 accepts an `ensures` place. That covers `*r`, `r->x`, `r.q` and `*r.q`, `**r`, a formal's pointee, `**pp`, and a block site on a formal.
   - **The rule:** at each return, for each such atom `k`, `cov_k` is true when a returned atom `j` whose guard holds names the same place in the post-state and its kind covers `k`:
     - `owns` covers `owns`, `reads` and `live`;
     - `reads` covers `reads` and `live`;
     - `live` covers `live` by the liveness cell;
     - a block site covers every atom on its cells.
   - **Covered:** `k` gives the caller only the fact that the places are equal. The caller already gets the permission through `j`.
   - **Not covered:** `k` gives an **output amount**: a ghost `Perm` result `o_k` of the method, `ensures none < o_k && o_k <= write` before every user clause.
     - At the return, `o_k := (perm(L_k) − R_k) / M`, where `R_k` is the sum of the partnered returns naming that cell and `M` is the number of partnerless atoms.
     - The return row is `cov_k || o_k > none` (`permission`: "returns `*r` without holding it").
     - `owns` also needs its value and flag cells at `write`, which the existing return rows check.
     - A block site on a formal has no amount: the callee must hold the whole block.
   - **The method's postcondition:** `!cov_k ==> (none < o_k && <atom at o_k>)`, `cov_k`'s equalities included. No inhale.
   - **The caller:** a multi-target call receives `o_k`. `result_splits` covers the new atoms, or Silicon would exit 3.
   - **Framing:** a partnerless atom frames reads after it (0037 item 7, 0047 item 4). 0039 item 7's partner-by-path check does not apply. Its exclusivity rows do.
   - **The static local:** `ensures(r, live(r))` on `where()` returning `&s` is covered by the implicit `owns(s)` (question 80). The caller learns that `r` is `s`'s liveness cell and uses its own implicit atom on `s`.
   - `static_local_return_address_use.c` (no contract) keeps its `lifetime` error. Its message names the `ensures`.
   - Pins become `permission`: see Flips.

3. **Owning automatic locals** (target `v`).
   - **Registration:** `owning` becomes `"?(X)" "r|v|p"`. `d` stays unregistered (EDG keeps refusing fields until d2b2).
   - **Rejected,** `unsupported_annotation`:
     - a global, a `static` local or an address-taken local (each naming d2b2);
     - a non-pointer;
     - a `void*`;
     - a pointer to `const` or `volatile` (0047 N1);
     - a count (`owning(n)`, M4.5e1).
   - **Owners:** the owning locals in scope at a point.
   - **The value rule:** at every initialization or assignment of an owning `v`, `new != null ==>` the function holds `new`'s block whole (offset 0, the token). Otherwise `permission`: "owning `v` receives a pointer that is not a block it holds".
   - **Lifetime end:**
     - Where: at each scope exit of `v` (block end, `break`/`continue`/`goto` out of it, `return`).
     - The row: `assert v != null && perm(tok(block_of(v))) > none ==> v == o_1 || … || v == result`, over the owners that remain and the returned value.
     - Its failure is `leak` at the exit: "owning `v` still holds its block when its lifetime ends".
   - **Reassignment:** the same row on the old value, before the write, with the new value counted. `v = v` passes.
   - **Unchanged:** the end-of-function `forperm` catches what has no owner. Silicon stops a path at its first failure, so the same token is not reported twice.
   - **Loops** (amends 0038 item 3 and 0047 item 10): an owning local declared outside a loop that the loop assigns has as its loop place its **current** value, not the entry ghost `hb`. This means `reach` and no `vs` equality. The back-edge leak row exempts the blocks of owning locals in scope. Unannotated programs keep their Viper byte for byte.

4. **Marked locals** (`ref_to_uninit` target `v`, automatic only; amends 0042 item 8 and 0047 item 8).
   - **Registration:** `"r|v|p"`.
   - **Default mode:** the marker documents intent. No Viper change. It is rejected on a non-pointer, a `void*`, a global or a `static` local (the last two naming d2b2).
   - **`--strict-init`** (P4222R3, P4416):
     - a local initialized or assigned from a `ref_to_uninit` call's result must be marked; an unmarked one keeps today's rejection with P4222R3's message;
     - a marked local passed to a parameter follows 0042 item 8 (a `ref_to_uninit` parameter, or the call's flag row);
     - a marked function's returns follow question 154;
     - a null argument to a `ref_to_uninit` parameter stays rejected (0042 item 8, R3);
     - `free` is exempt (P4416), and so is `vlibc_free`.
   - `must_init` on a local: question 155.

5. **The lint** (`lint_viper.py`):
   - a method's ghost `Perm` *results* each have exactly `ensures none < o && o <= write` first, beside 0037 item 14's rule for formals;
   - the owner rows read `tok` inside an `assert`, which is lint-legal.

6. **Leftovers:**
   - (a) **written `owns_block` in loop invariants** (0047 items 10 and 11): a whole-block place of the loop, with the token and every cell;
   - (b) **block-site guards that read the heap, a global or `old`** (0047 item 4, audit 2 S2): evaluated like 0045 item 11's atom guards, with the leak exemption repeating the guard over snapshots;
   - (c) **`owning` beside `owns_block`** on one parameter in both lists: merged into the block site (`apply_owning` sees block sites);
   - (d) **`restrict` beside a block-only parameter:** `restrict_backed` counts block sites;
   - (e) **`two((free(p), 0), p)`:** `free`'s effect is registered on the object, so it is `unsequenced`.

7. **vlibc blocks in contracts** (question 153): `cerastes_owns_vlibc_block(p)`.
   - It is a block site whose token rows read `!tok` (0050 item 3); everything else is as `owns_block`.
   - It is rejected in C++ (0050 item 7).
   - `vlibc_free` requires it.

8. **Still rejected** (fail closed; one pin each, the message naming the session):
   - owning and marked globals, `static` locals and address-taken locals;
   - owning fields (EDG #1835 until d2b2);
   - block sites through loads or on globals;
   - P4418R0's attributes (unregistered);
   - `must_init` on locals (permanently, with the reason);
   - `owns(**pp)` of a block allocated in the callee without `owns_block(*pp)` (a `leak` at the callee's end, which is right: d2b2's derived block sites give the caller the block).

9. **Categories:** none new.

10. **Facts relied on:** P74 (`il-inventory.md`); S38.

11. **Tests and documentation:** see below.

## Spike S38 (il-analyst, before code; fixtures in `build/m4.5d2b/s38/`, kept ones in `tests/viper-smoke/memory/`)

Hand-written Viper in the target shapes, each with Silicon's verdict and time:
- (a) a covered `live(r)` of a static local, with the caller's use of `p` (question 157: not `*p`); (a') a covered `owns(*r)` of the static local, with the caller's `*p`;
- (b) an uncovered `owns(*r)` returning a held global's cell with an output amount, and the caller's use;
- (c) the same when not held, failing at the return row;
- (d) a covered `owns(*r)` with `r == &p->x` beside a returned `owns(p->x)`;
- (e) an owning local's scope-exit row and reassignment row;
- (f) the loop with an owning variable's current-value place, the `heap_loop_replace` shape, with timing;
- (g) a written `owns_block` invariant, the swap shape;
- (h) a block-site guard reading the heap;
- (i) `cerastes_owns_vlibc_block` in a borrow.

A failed spike changes the record before code. Audit round 1 reads the spike with the draft.

## Parts and commits

1. **Commit 1:** planning (Step 1).
2. **Commit 2:**
   - record 0052 (draft);
   - P74 into `il-inventory.md`;
   - the S38 fixtures;
   - `build/m4.5d2b/vprdiff.py`, which compares outputs against the baseline with an allow-list of tests that use a new construct (no other output may change).
3. **Commit 3, Part A:** partnerless atoms (`encode.cpp` `spec_atom`/`finish_atoms`; `vir.cpp` `atom_place`; `check.cpp`; `lower.cpp` amounts, return rows, `result_splits`; emit; lint), with its tests and the flipped pins.
4. **Commit 4, Part B:** owning locals (registration, reading, `Fn::Scope` owners, `scope_exit`/`leave_to`/return/assignment rows, loop places, the back-edge exemption).
5. **Commit 5, Part C:** marked locals and the strict rules.
6. **Commit 6, Part D:** leftovers (a)–(e) and item 7.
7. **Commit 7:** audit round 2 fixes and the re-audit's.
8. **Commit 8:** docs, status, handoff, plan.md's Done row, record 0052 accepted.

**Pre-agreed cuts, in this order, if the context runs short:** (e), then (a) and (b), each to d2b2 with its pin kept. Stop and ask before any other cut.

## Team (three Sonnet agents with disjoint files, one Opus reviewer, all from `.claude/agents/`)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | **First:** S38 and P74's `il-inventory.md` text. **Then:** `src/vir` (`vir.h`, `vir.cpp`, `check.cpp`, `lower.cpp`), `src/viper` (`lowered.h`, `emit.cpp`, the VIR self-test cases and `VIR_CASES_MIN`) and `build/m4.5d2b/vprdiff.py`. |
| **edg-backend** (Sonnet) | `encode.cpp` (atoms, partners, `apply_owning`, local markers, strict-init, block-site guards, the vlibc spelling), `register_attributes.cpp`, `annotation_checks.cpp`, `include/cerastes.h` (`cerastes_owns_vlibc_block`) and the builds. |
| **test-mutator** (Sonnet) | The tests and twins below; the flipped and re-messaged pins (exactly the Flips list); `lint_viper.py` with `test_lint_viper.py`; `run_tests.py` if needed; the doc example files. |
| **soundness-auditor** (Opus, read-only) | **Round 1:** the record draft and S38. **Round 2:** the code and tests. **One re-audit** of the blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Step 1; the record; the docs prose; `encoding.md`; integration; serialized builds (`lockf build/m4.5d2b/build.lock`); status and handoff. |

**Rules for every agent:**
- Any question stops that work and goes to the owner.
- Agents hold builds before a directory-wide run.
- Code style: `scripts/format.sh`, brace init, `U`, one declarator per declaration, and no comment on a line with a brace.
- A new `.cpp` updates `object_identity.py`'s TU constants.

## Tests (each positive has a twin failing for one reason at one line; `.c` in `c23, c++20` where valid; positives `native-ok`)

- **Part A:**
  - `owns(*r)` given (a held global's cell) and covered (`&p->x`), each with a twin that does not hold the place;
  - `reads(*r)` and `live(r)`;
  - `live(r.q)` and `owns(*r.q)`;
  - `owns(**r)` after `owns(*r)`;
  - a formal's pointee given;
  - the static local's `ensures(r, live(r))` with the caller's use;
  - a block site on a formal without a partner;
  - `account.c` with VeriFast's contracts (`owns_block(r) && r->balance == 0` already passes; the new shape is a callee returning `owns(*r)` of a cell it holds);
  - two partnerless atoms on one cell;
  - a trusted bodyless callee giving an amount (the lint row).
- **Part B:**
  - an owning local freed, returned (with `owning` on the function), passed to a consuming callee, or handed to another owning local;
  - twins: a leak at block end, at `break`, at `goto` out of the scope, at `return` of another value, and at reassignment;
  - `owning v = &x;` (`permission`);
  - the owning loop shapes (verified twins of the two loop false alarms);
  - a pointer copied to a non-owner and the owner's scope ending (`leak`).
- **Part C:**
  - default mode, a marked local is accepted with the same verdicts as unmarked;
  - strict, a marked local receiving `malloc`, `calloc` and `vlibc_malloc`;
  - a marked function returning a marked local, `NULL` or a call; twins: an unmarked local, and a return of an initialized pointee.
- **Part D:**
  - each of (a)–(e) and item 7, with twins;
  - `vlibc_block_contract.c`'s shape.
- **Byte identity:** `vprdiff.py` over the whole suite against the baseline. Only tests that use a new construct may change output, and the VIR self-test changes only in its new and converted cases.

## Flips (approved with this plan; any other changed outcome stops the session)

- **Become `error permission` at the return or at the atom.** One `expect:` line each, at the line the first run shows. Each is reviewed against record item 2 and listed in the status entry.
  - `reject_atom_on_result.c`
  - `reject_ensures_atom_no_partner.c`
  - `reject_struct_value_live_result.c`
  - `reject_owns_result_member_target.c`
  - `reject_owns_result_double_deref.c`: `owns(**r)` with no atom on `*r` is a framing failure at the atom
- **Retired for positives of the same shape:**
  - `reject_owns_block_invariant.c`
  - `reject_owns_block_guard_heap_requires.c`
  - `reject_owns_block_guard_heap_ensures.c`
  - `reject_owns_block_guard_global_ensures.c`
  - `reject_owns_block_restrict.c`
  - `reject_owning_local.c` (its shape is accepted; a new pin `reject_owning_global.c` takes its place)
  - `reject_ref_to_uninit_local.c` (likewise, `reject_ref_to_uninit_global.c`)
- **Message and `Pins:` only:**
  - `reject_owns_block_ensures_no_partner.c` (question 158: bodiless, stays rejected);
  - `reject_must_init_local.c` and `reject_struct_owning_field.c` (EDG's #1835 stays);
  - `reject_owns_block_load.c`, `reject_owns_block_global.c`;
  - `reject_strict_init_malloc_{local,argument,member,return,store}.c` and `reject_strict_init_vlibc_malloc_local.c` (a pin whose exact shape item 4 accepts is retired for a positive instead, and listed);
  - `static_local_return_address_use.c`;
  - `heap_loop_swap_false_alarm.c`, `heap_loop_replace_false_alarm.c`, `vlibc_alloc_loop_replace_false_alarm.c`.
- **VIR self-test:**
  - converted to lowering cases: `ptr_rej_ensures_atom_without_partner`, `hr_rej_result_member_atom`, `hr_rej_result_double_deref`, `hb_rej_invariant`, `hb_rej_ensures_without_requires`, `hb_rej_beside_owns`;
  - `hb_rej_beside_live`, `hb_rej_load` and `hb_rej_global` stay.

## Docs (invariant 8; every example is a test)

- **`memory.adoc`:**
  - partnerless atoms ("Results");
  - owning locals;
  - marked locals;
  - written `owns_block` invariants;
  - guards;
  - vlibc blocks;
  - the not-yet list rewritten for d2b2.
- **`verifying.adoc`:** `--strict-init`'s rules.
- **`annotations.adoc`:** `owning` and `ref_to_uninit` on locals.
- **`error-messages.adoc`:** the new `leak` and `permission` messages.
- **The tutorial's heap chapter:** an owning local (it completes the concept of an owner variable); the static local's lifetime note.
- **The developer guide's "Code style":** the brace rule (Step 1).
- **`internal/encoding.md`:** record items 2–4.

## Order of work

1. Step 1, then commit 1.
2. The il-analyst on S38 and P74 while I draft record 0052.
3. Audit round 1 on the draft and S38. Fix, then commit 2.
4. Parts A, B, C and D, with edg-backend and il-analyst in parallel on disjoint files and test-mutator writing tests from the record. Builds go one at a time.
5. After each part: its tests, `vprdiff.py` and the soundness auditor on the diff before the commit (CLAUDE.md: every commit touching `src/vir`, `src/viper` or `src/backend`). Round 2 covers the whole set; one re-audit follows.
6. `check.sh` twice at most. The delta is checked after run 2.
7. Docs, status, handoff, and the last commit.

## Stop and ask

- Any agent question.
- Any changed outcome outside Flips.
- A spike that contradicts record item 2 or 3.
- A cut beyond the pre-agreed ones.
- An exit 3 or a timeout over 120 s that is not fixed within the part.

## Acceptance (commands run at the end)

- `scripts/check.sh` (the full check, `CERASTES_CHECK_BASE` unset): green.
- `python3 build/m4.5d2b/vprdiff.py build/m4.5d2b/baseline/vpr build/vpr`: only allow-listed tests differ.
- The VIR self-test (in `check.sh`) at the new `VIR_CASES_MIN`.
- `scripts/format.sh --check`, and `grep -nE '^\s*[{}].*//' ` over the session's diff of `src/` finding nothing new.
- `python3 scripts/run_tests.py tests/memory tests/vlibc doc/modules/ROOT/examples`, on the x86-64 box in the S5 container if it is reachable (otherwise CI decides).
- **The status entry:** what is done and not done, the flips with their lines, the surprises, record 0052, the first step for d2b2 (its planning).
- **Commits:** local, on this branch; the owner pushes S7 first, then this branch.

## Inputs (moved from handoff §3 at step 1, verbatim)

**From M4.5d2a's planning ("M4.5d2b results, owning locals, globals and fields, P4418R0"):**
- P4222R3 and P4416R0 require a local receiving `malloc`'s result to be marked `ref_to_uninit` (`int *s
  [[ref_to_uninit]] = buffer(4); // OK`, an unmarked one an error): d2a rejects that shape under `--strict-init`, naming
  d2b (target `v`); d2b decides the strict rules for marked locals, returns (P4416 `[profiles.init.ref]` p6: a marked
  function's returns "shall refer to uninitialized objects", null "refers to initialized objects"), the null argument to a
  `ref_to_uninit` parameter (R3 rejects it, P4416 (2.3) accepts it; 0042 item 8's strict rejection agrees with R3), `free`
  (unmarked in P4416: an exemption), and `void*` markers (R3 allows them; 0042 item 4 rejects them on parameters). The
  marker stays on the pointer until `now_init` in the papers; cerastes keeps the flag on the cell (more programs accepted,
  path-precise). P4416 treats `x = 1; x = 2;` as a double initialization (a drafting slip: not copied) and analyses
  syntactic paths (cerastes: feasible paths, so `--strict-init` is P4222's static form only up to that).
- `return &s;` of a `static` local leaves the caller only `WF(r)`, so its uses are `lifetime`
  (`static_local_return_address_use.c`) until partnerless `ensures` atoms land (`live(r)`) [b2a].
- From M4.5d2a (record 0047): a block site whose own guard reads memory, a global or `old` is rejected (audit 2 S2), and
  written `owns_block` in loop invariants is rejected: both d2b's; the pinned false alarm of two block pointers swapped
  in a loop (`tests/memory/negative/heap_loop_swap_false_alarm.c`, about 40 s per mode) needs written `owns_block` atoms;
  a block allocated in an iteration and kept in a pointer declared outside the loop is a `leak` at the back edge until
  owning variables (target `v`); an `owning` parameter beside `owns_block` in both lists is rejected as "beside" although
  `encode.cpp`'s merge treats a block site as `owns` (dead merge or an over-strict rule), and `restrict` beside a block-only
  parameter says "does not own or read `*q`" [audit 2 N7]; `two((free(p), 0), p)` is reported `permission`, not
  `unsequenced` [audit 2 N8]; strict-init rejects every `ref_to_uninit` result call (record 0047 item 8) until marked locals.

**From M4.5d2v:**
- **For M4.5d2b:** a spelling for a vlibc block in contracts (`owns_block` and `owning` name the platform's allocator
  only; a vlibc block handed over through them is `permission`); the false alarm of a block allocated in an iteration and
  kept in a variable declared outside the loop, now reported in user terms (`heap_loop_replace_false_alarm.c`,
  `vlibc_alloc_loop_replace_false_alarm.c`), lifted by owning variables. [d2v]
