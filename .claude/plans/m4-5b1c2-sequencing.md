# Plan: M4.5b1c2 — sequencing and placement over memory (record 0039)

## Context

M4.5b1c1 is merged into local `main` (`eab4058`); `origin/main` (`234ede3`) has b1b (PR #17) and the Python pins
(PR #18) but not b1c1, and `origin/m4.5b1c1-loops` is pushed. Records 0032–0038 give cells, pointer parameters,
contracts and loops over memory. Today these are still rejected:
- a write to a cell inside an expression (`y = x++`, `*p = (*q)++`, `f(x = 1)`);
- a call whose contract holds an atom, when the same full expression has another cell access or a second such call
  (the `Placement` class, record 0037 item 8);
- a function's `decreases(*p)`;
- `c ? owns(*p) : reads(*p)`.

`return &local;` is accepted. It only prints EDG's #1056, and the caller later fails `lifetime`.

b1c2 implements:
- 0028 item 12: sequencing on objects, with the new category `unsequenced`;
- 0028 item 13: where calls go relative to cell accesses;
- handoff question 65: `return &local;` rejected like the compilers;
- handoff question 67: the `owns`+`reads` rule made path-precise;
- record 0038 item 8: function measures that read memory.

**Owner decisions (2026-10-29, this planning):**
- **The EDG pin stays at `f0ed5525af`.** Upstream `main` is 23 commits ahead of the fork's `cerastes`. Merging it is
  deferred to the pending box session (handoff §4.3), which runs the suites before the pin moves.
- **Same-object shapes are rejected like the compilers.** This covers unsequenced accesses that name one object in the
  source, such as `x++ + x` on a cell, and probably `*p = (*p)++`. When clang or GCC warns at the no-flag or `-Wall`
  tier (0020 item 5's union rule, measured by P61), the shape is rejected `unsequenced_modification`, as for plain
  locals. Shapes whose outcome depends on aliasing (`*p = (*q)++`, `x++ + *p`, `x + bump(p)`) get the path-precise
  `unsequenced` check.
- **No time cap and no cuts** (as in b1b). The full scope lands in this session.
- Earlier answers still apply:
  - 65: reject `return &local;` like the compilers;
  - 67: path-precise (b1c2);
  - 66: the cost of aliasing is M4.8's;
  - 68: the false alarms of reassigned parameters are b1c3's.

## Step 1 (first, before any code): reconcile and consolidate the plan documents

This is the session's first commit, markdown only. The goal is one place per live fact. Everything a later session no
longer needs is dropped, and `git show` pointers lead to it.

1. **Branch.** In the worktree `.claude/worktrees/m4.2b1` (now on `m4.5b1c1-loops`), run
   `git checkout -b m4.5b1c2-sequencing main` (`eab4058`).
2. **The plan.** Save this plan, as approved, as `.claude/plans/m4-5b1c2-sequencing.md`. Add a section "Inputs from
   the handoff", which holds the §3 notes this session consumes (item 4 below), so they live in one place.
3. **A check before dropping a note.** Probe `(int)(long)((typeof(N) *)0 + 1)` in a C enumerator. If it is rejected
   (pointer-to-integer casts are, `reject_pointer_to_int_cast.c`), the enumerator half of the carried note "Pointers
   will reopen two holes" is closed (0028 item 15). If it is not, the note stays.
4. **`internal/handoff.md`**, consolidated:
   - **Header:** one pointer, `git show eab4058:internal/handoff.md`, to the text before this trim; that text names the
     earlier versions. This replaces the chain of eight pointers.
   - **§1–§2**, rewritten for b1c2:
     - b1c1 is merged into local `main`, not pushed. Owner step: push `main`.
     - Drop the steps that are done: discarding the main checkout's answers (main is clean) and pushing b1c1's branch.
   - **§3, moved into the plan's "Inputs"** (b1c2 consumes them):
     - b1c1's "b1c2's function measures";
     - b1b's `c ? owns : reads` and #1056;
     - b1a's "compound store lowers its address once" and the over-strict `*(g() ? &x : &y) += 1` and `*(q = p) += 1`;
     - M4.4d's "For b1c2 (0026 item 15)";
     - the b1c2 half of M4.4c's "finding A" (the M4.5b2 half stays);
     - the sequencing half of the carried "Pointers will reopen two holes".
   - **§3, dropped as closed:**
     - S5's "Fixed: glibc's C23 matcher";
     - "From S4 (closed in M4.4d)";
     - the enumerator half of "two holes", if item 3 confirms it.
   - **§4:**
     - §4.1, and §4.4's b1a, b1b and b1c1 "done" paragraphs, become one line pointing at `status.md` and
       `.claude/plans/`.
     - The b1a pre-approval paragraph is condensed to the rule that is still live: each session flips only the pins
       listed for it.
     - b1c2's paragraph points at the plan file. b1c3 and later stay as they are.
   - **§4.2 (the warning sweep) is dropped.** It duplicates plan.md's M8 entry, which gains the old plan's pointer
     (`git show 9017105:internal/handoff.md` §4.2).
   - **§4.3** gains the upstream merge: 23 commits behind on 2026-10-29, deferred by the owner to the box session.
   - **§7:** a pointer to the plans and to `status.md`.
   - **§8:** one line records this planning's three answers as questions 69–71. No question is open.
5. **`plan.md`:**
   - the M4.5 table's b1c2 row names the plan file;
   - M8's warning-sweep entry gains the pointer from item 4;
   - the header's trim note is updated.

   `status.md` is not changed until the session ends.
6. **Commit 1 (planning):** items 2–5. Local only, with no Claude trailer (CLAUDE.md overrides the harness default).

## Inputs from the handoff (moved here from `internal/handoff.md` §3 at step 1; `git show eab4058:internal/handoff.md`)

- **Function measures that read memory** (from M4.5b1c1, record 0038 item 8, S28 P6, audit round 1 B5): no `old` in
  the emitted `decreases` (Silicon reads the caller's measure in its entry heap already); `f__wf`'s framing is the only
  well-formedness check, non-recursive functions included (Silicon checks none); the method's `decreases` reads are
  chain-guarded; 0021 item 4's explicit check comes after the call's permission checks and case splits.
- **`c ? owns(*p) : reads(*p)`** (from M4.5b1b) is rejected by the `owns`+`reads` rule although the guards exclude each
  other; question 67: make it path-precise.
- **`return &x;`** (from M4.5b1b): C's `return &x;` raises EDG warning #1056 (`returning_ptr_to_local_variable`), which
  `VERIFY_WARNING_TAGS` does not admit, so the dangling-result test returns through a local (`int *p = &x; return p;`,
  `tests/memory/negative/pick_ret_local.c`); question 65: reject `return &local;` like the compilers (clang
  `-Wreturn-stack-address`, GCC `-Wreturn-local-addr`, both on by default).
- **The compound store** (from M4.5b1a, audit round 2 and re-audit): a compound store lowers its address once; b1c2's
  writes inside expressions rely on it. Over-strict, fail closed: `*(g() ? &x : &y) += 1` is refused as "a call that is
  reachable twice" (the compound store's address holds a call; the wording misleads) and `*(q = p) += 1` as
  unsequenced; both are defined C.
- **0026 item 15** (from M4.4d): step 1 places a write of a comma's value in C's order; step 2 keeps 0025 item 2's
  placement, whose caveat (callees do not see locals) applies to it in particular.
- **0025 item 2 and finding A** (from M4.4c): item 2's placement (callees that see address-taken locals) is revisited
  with item 1's kinds of access (a flag read is an access) here; globals in M4.5b2.
- **Sequencing on objects** (carried since M4.2b2): 0020 item 4 checks names, so `*p = (*q)++` needs a check on objects
  (old §11 question 26; 0028 item 12); 0025 item 2's placement of writes rests on callees not seeing locals. The other
  half of that note (pointer arithmetic in a C enumerator observing EDG's C++ typing) is closed: step 1's probe showed
  `(int)(long)((typeof(N) *)0 + 1)` rejected ("a conversion to or from a pointer type") and `sizeof(*(typeof(N) *)0)`
  rejected (0018 item 10's rule).

## Setup (in parallel with step 1)

- **Worktree build.** Run `scripts/bootstrap.sh` in the worktree, then
  `lockf build/m4.5b1c2/build.lock cmake --build build/cmake`.
- **Baseline that agents cannot disturb** (handoff §6). It runs in the background in the main checkout, which is on
  `main` (`eab4058`) and is not edited:
  1. `scripts/bootstrap.sh`;
  2. `python3 scripts/run_tests.py -j 8`;
  3. the VIR self-test;
  4. copy `build/vpr` to the worktree's `build/m4.5b1c2-baseline-vpr`, and the VIR self-test output to
     `build/m4.5b1c2-baseline-vir`.

## Record 0039 — sequencing and placement over memory

The record is written before code. Audit round 1 runs on it in the background.

1. **Scope table** (b1c2, b1c3, d2). It amends:
   - 0037 item 15's and 0038 item 10's "lifted by" column;
   - 0011 item 10: `unsequenced`, making eighteen categories;
   - 0020 items 4 and 8; 0021 item 2; 0025 item 2; 0026 item 15;
   - 0028 items 12 and 13; 0032 items 9 and 10; 0037 items 5 and 8; 0038 item 8.
2. **Writes to cells inside expressions** (lifts 0032 item 9).
   - An assignment, `++`/`--` or compound assignment to a cell may stand anywhere code allows one: by name, or through
     a pointer (`*p`).
   - VIR gains a store used as a value. Either `AssignE` on an address-taken `Var`, or a new appended kind `StoreE`
     (`*addr := a`, sharing `addr` like `Stmt::Store`); `vir.h` decides.
   - Lowering makes each such store a **write effect**: null, liveness and `perm == write` checks, then `v_T :=`, then
     `d_T := true`.
   - **E3 for memory.** Every read of a place of field T, in a full expression that writes any place of T or calls a
     function whose `requires` owns a T cell, is a **snapshot effect** after its checks. Today a heap read creates no
     effect and is inlined at its consumer, which a write or a call could then overtake (`int r = x && bump(&x);`).
   - C++ lvalue uses (`++*p = 3`) stay rejected (0020 item 3).
3. **Sequencing on objects** (0028 item 12, with the owner's split).
   - The name rule (0020 item 4) is extended to cells and to `*p` with the same unassigned pointer variable, exactly for
     the shapes P61 shows a compiler warning on. Those are rejected `unsequenced_modification`.
   - Every other pair of unsequenced accesses to places of one field, at least one a write, gets a path-precise
     `assert g1 && g2 ==> place1 != place2` (`&x != p`, `p != q`). It is placed before both accesses, category
     `unsequenced`, "modifies `*p` and accesses `*q` in an order C leaves undefined, and they may be one object".
   - Places of different fields never alias, so they get no check.
4. **Calls beside cell accesses** (0028 item 13; lifts 0037 item 8's `Placement` rule).
   - **A call beside an access.** For each memory call `c` (a callee with an `owns` or `reads` atom), and each cell
     access `a` that C does not order with `c`:
     - for every callee `owns` atom (and every atom when `a` writes): `g_k ==> place(a) != arg_k`.
     - Category `unsequenced`, message "accesses `x`, which `bump` may change, in an order C leaves unspecified".
   - **Two memory calls.** For one with an `owns` atom and the other with any atom on the same field:
     `g1 && g2 ==> q1 != q2`, also `unsequenced`.
   - **No reordering across a memory call.** Writes and snapshots of cells never cross a memory call. A cell write's
     set holds every fixed point of its operands (the store follows their value computation, C23 6.5.17.1). This fixes
     `x = (bump(&x), 2)`, which 0025 item 2 would emit before the call (0026 item 15's caveat).
   - **Fail-closed memory-order check.** Per field, the emitted order of cell writes, snapshots and memory calls keeps
     C's order wherever C orders them. Exit 3 otherwise, with a self-test hook.
   - 0021 item 2's reliance on "no pointers before M4.5" is replaced.
5. **Compound stores** (handoff §3, b1a).
   - The shared address is one evaluation, both for the name rule and for "reachable once".
   - So `*(q = p) += 1` and `*(g() ? &x : &y) += 1` are accepted.
6. **`return &local;`** (question 65). A new `-Wall`-shape rule in 0025 item 6's family: tag `return_local_address`,
   "(-Wreturn-stack-address, -Wreturn-local-addr)".
   - It covers exactly the shapes P61 shows clang 21 or GCC 16 warning on by default: `&x`, `&param`, `?:` arms,
     parentheses, casts.
   - `int *p = &x; return p;` stays the caller's `lifetime` (`pick_ret_local.c`).
7. **`c ? owns(*p) : reads(*p)`** (question 67). The static `owns`+`reads` rule (`encode.cpp` 1999, `check.cpp` 375) is
   replaced by an `f__wf` assert, `prior ==> !(g_owns && g_reads)`, per clause list. Category `permission`: "the
   contract of `f` both owns and reads `*p` when …".
   - Calls already separate the pair under both guards (0037 item 8), and the return sums count it once.
   - Two atoms of one kind on one place stay rejected.
8. **Function measures that read memory** (record 0038 item 8, S28 P6, audit B5).
   - `decreases(*p)` is framed in `f__wf` over pre-state ghosts. It is the only well-formedness check, and covers
     non-recursive functions too.
   - The method's `decreases` is plain (no `old`), with each read chain-guarded (0037 item 7).
   - 0021 item 4's explicit check reads the callee's measure in the current heap, after the call's permission checks
     and case splits, and the caller's measure under `old(…)`.
   - Item 4 guarantees that no unsequenced memory call writes a cell the measure reads.
9. **Still rejected:**
   - b1c3: `T**`, written atoms in invariants, `*result`;
   - d2: `owns(*result)`, partnerless `ensures` atoms;
   - no session: atoms in measures, `check` or code.

## Team (disjoint files; three Sonnet agents and one Opus reviewer from `.claude/agents/`)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | **First, P60:** the IL in both modes of cell and `*p` writes inside expressions, the compound shapes of item 5, `return &x` shapes (#1056 and #1057 positions), `c ? owns : reads` guards (`AtomGuard` pointer identity) and `decreases(*p)`. **Then S29** (Silicon, with test-mutator's hand-written targets): (a) item 3's asserts and item 2's snapshots; (b) item 4: `x + bump(&x)`, `bump(&x) + bump(&y)`, and `x = (bump(&x), 2)` in both placements; (c) item 7: the callee, the caller's amounts and sums for both guard values, `f__wf`'s overlap assert; (d) item 8: the chain-guarded method `decreases` and the explicit check with `old` after an earlier unsequenced call. **Then `src/vir/lower.cpp`** (the critical path): memory effects and snapshots (keys per cell and per field); the item 3 and 4 checks; the cell write and snapshot sets; the memory-order check and its hook (`lower.h`); `wf_method` measure framing; the chain-guarded `decreases`; `explicit_termination`; item 7's assert. `lowered.h` and `emit.cpp` only if a node is missing. Then the baseline diff. |
| **edg-backend** (Sonnet) | **First, `src/vir/vir.h`** from the record: the store-as-value node. Then `vir.cpp` (Builder), and `check.cpp`: lift the `AssignE`-to-cell refusal (832); remove `Placement` (1103–1190), keeping backstops for what stays rejected; `measure(memory_ok)` for function measures (240); the item 3 extension of `sequence_problems` (1196–1356); drop the `owns`+`reads` twin (375). Then `src/backend/encode.cpp`: `assign_target` (2764) builds the new node; `contract_read`'s `decreases` lift (1919); `finish_atoms` (1999); `report_placement_problems` (2944) retired; `return_statement` (3171) and the new tag in `diagnostics.def`; remaining pins' messages; the compound-store "reachable twice". |
| **test-mutator** (Sonnet) | **First, P61:** the compiler matrix (Apple clang and GCC, both modes, three tiers) for item 3's same-object shapes and item 6's `return` shapes, plus the hand-written S29 targets. Then category `unsequenced` in `scripts/run_tests.py` `DRIVER_CATEGORIES` and the three pinned copies in `scripts/test_run_tests.py`, and in `tests/README.md`'s table. Then the tests and twins (below), written from the record before the code runs; the flips; the VIR self-test (`selftest_cases10.inc`, the two flipped `mem_rej_*`, `VIR_CASES_MIN` in `check.sh`); `lint_viper.py` only if a new shape needs it; the doc example files, including `error-messages/unsequenced_{error,fixed}.c`. |
| **soundness-auditor** (Opus, read-only) | **Round 1:** record 0039 as soon as it is drafted. **Round 2:** the code diff and tests, especially E3-for-memory, the cell write sets and the memory-order check. **One re-audit** of blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Step 1; record 0039 and its amendments (0011's table included); `doc/`; `internal/encoding.md`; `il-inventory.md` (P60, P61, S29); integration; serialized builds; the baseline; status and handoff. Stops and asks the owner. |

Any question from an agent, or of mine, stops that work and goes to the owner.

## Tests (`tests/memory/`; `.c` in both modes; each positive has a twin failing for one reason)

| Positive | Twin (expected) |
|---|---|
| `int y = x++;` on a cell, `f(x = 1)`, `y = (*p = 3) + 1` | `x` at `INT_MAX` → `overflow`; an uninitialized cell read in the value → `uninitialized` |
| `*p = (*q)++` with `p`, `q` to different cells | `q` may equal `p` on one path → `unsequenced` |
| `x++ + *p` with `p == &y` | `p` may be `&x` → `unsequenced` |
| `x + bump(&y)`; `x = bump(&x)` (ordered) | `x + bump(&x)`, `x += bump(&x)` → `unsequenced` (the flipped pins) |
| `bump(&x) + bump(&y)`; `bump(&x) + peek(&y)` | `bump(p) + peek(q)` with `p == q` possible → `unsequenced` |
| `x = (bump(&x), 2)` with `ensures(r == 2)`; `int r = x && bump(&x);` | the same with the wrong value → `postcondition` (pins the order and the snapshot) |
| `*(q = p) += 1`, `*(g() ? &x : &y) += 1` | the target's cell consumed earlier → `permission` |
| `c ? owns(*p) : reads(*p)` in `requires` and `ensures`, written under `c`; callers with both values | overlapping guards (`c \|\| owns(*p)` with `reads(*p)`) → `permission` at the contract; a write under `!c` → `permission` |
| `down(int *p)` with `decreases(*p)`; a non-recursive one | no decrease → `termination`; `decreases(*p)` with only `live(p)` → `permission` at the contract; the explicit check after an earlier unsequenced call |
| `return &x;`, `return c ? &x : &y;`, `return &param;` (per P61) | rejected `cerastes-return_local_address` in C and C++ |

**Flips (pre-approved: exactly b1c2's list in handoff §4.4):**
- **Retired, each replaced by a positive of the same shape:** `reject_cell_postfix_value.c`, `reject_two_memory_calls.c`
  and `reject_heap_read_decreases.c`.
- **Retired, each replaced by a negative named for its shape** (`error unsequenced` at the same line):
  `reject_call_beside_access.c` and `reject_call_compound_target.c`.
- **`reject_write_in_assignment_value.c` (`*p = (*p)++`):** if P61 shows a warning, it is re-expected in place as
  `rejected cerastes-unsequenced_modification @ L11`; otherwise it is replaced as above.
- **VIR self-test:** `mem_rej_cell_assignment_as_value` and `mem_rej_cell_postfix_increment_as_value` become lowering
  cases.
- **No other existing test's outcome or Viper may change.** No program accepted today has a cell write inside an
  expression, a memory call beside an access, a heap-reading function measure or a `?:` atom pair. Any change stops
  the session.

## Docs (invariant 8; every example is a test)

- **Memory and ownership:**
  - "Expressions": writes inside expressions, `unsequenced`, calls beside accesses;
  - conditional ownership;
  - function measures over memory;
  - `return &local;`;
  - the "not supported yet" list (`memory.adoc` 238–257).
- **Error messages:** a new `unsequenced` section ("eighteen"), plus new messages for `permission` and `termination`.
- **Verifying a program:**
  - `unsequenced_modification` on cells (`verifying.adoc` 335);
  - the `-Wall` rules gain `return_local_address`;
  - the not-supported list.
- **Annotations:** `decreases` over memory; atoms under `?:`.
- **`internal/encoding.md`:** §2 and §3 (side effects at 214, memory at 323–420) and §8 (634–671).

## Order of work

1. Step 1, setup and the baseline. In parallel: P60, P61, the record draft (audit round 1 in the background), and
   `vir.h` from the record.
2. S29, then `lower.cpp` (the critical path). In parallel: `check.cpp`/`encode.cpp`, the category copies, tests and
   twins, and the VIR self-test.
3. End to end with single-file runs (`build/cerastes verify --keep-vpr`). Then regenerate every kept `.vpr`/`.map`
   against the baseline (handoff §6, own output directory).
4. Audit round 2, fixes, one re-audit.
5. Docs; `check.sh` (full, at most twice); status and handoff; commits.

## Stop and ask

There are no cuts (owner). Stop and ask on:
- any agent question;
- an existing test whose outcome or Viper changes outside the flips;
- S29 contradicting items 2–4, 7 or 8 with no fix inside the record, including Silicon failing first with its own
  reason;
- P61 showing a compiler warning on a shape whose outcome depends on aliasing (the owner's split would need a rule);
- exit 3 on a correct program;
- a new test over 60 s.

## Acceptance (commands run at the end)

- `python3 scripts/run_tests.py tests/memory tests/viper-smoke/memory doc/modules/ROOT/examples/memory doc/modules/ROOT/examples/error-messages doc/modules/ROOT/examples/verifying`:
  all pass.
- **Viper unchanged outside the flips:**
  - every kept `.vpr`/`.map`, regenerated with the new `build/cerastes`, is byte-identical to
    `build/m4.5b1c2-baseline-vpr`, except the flipped pins;
  - after `check.sh`, `diff -r -x MANIFEST build/m4.5b1c2-baseline-vpr build/vpr` shows only added files and the
    flipped pins;
  - against `build/m4.5b1c2-baseline-vir`, the VIR self-test shows only added cases and the two flipped `mem_rej_*`.
- `python3 scripts/test_run_tests.py`, `python3 scripts/test_lint_viper.py` and
  `python3 scripts/run_tests.py --check-headers`: green.
- `scripts/build_docs.sh` and `python3 scripts/check_doc_examples.py --html build/doc`: green, with the `unsequenced`
  section and example present.
- `scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): green. There is no Linux run, as in b1c1: CI covers it. If
  an x86-64 run is ever needed, it goes to `matt@clang4.cpp.al`.
- **The status entry** lists record 0039, the flips, the audit outcome, the plan file's path and b1c3's first step. The
  b1c1 entry moves to a history row. The handoff §3 gets what was not taken, and §8 any new question with a default.
- **Commits:** planning; feature (code, tests, docs, record); status and handoff. Local only, no Claude trailer, never
  pushed.
