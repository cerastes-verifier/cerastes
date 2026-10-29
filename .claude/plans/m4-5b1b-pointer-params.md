# Plan: M4.5b1b — pointer parameters, results and contracts (record 0037)

## Context

M4.5b1a is done on `m4.5b1a-pointers` (PR #15 open, CI pending at `bdc6bd0`; records 0032–0036). It gave the memory
model inside one function: address-taken locals as one-slot blocks, `&`, `*`, NULL, `==`, liveness, scope exits. It
rejects pointer parameters, results, `&param` and pointers in contracts. **M4.5b1b** (handoff §4.4) lets permissions
cross calls.

**Owner decisions (2026-10-28, this planning):**
- **No cuts; run until complete.** This session has no time cap. Pointer results and `&param` stay in b1b, as handoff
  §4.4 had them, with their pre-approved flips. The process rules stay: one audit round per diff plus one re-audit,
  `check.sh` at most twice.
- **Framing is path-precise, decided by Silicon** (not a syntactic rejection). The design check found a way that needs
  no new driver reason, which is what this plan uses (record item 7). The framing condition is an explicit assert in
  `f__wf`, decided per path. The method's own contract guards each heap-reading conjunct by the same condition, so
  Silicon's own well-formedness check stays as the backstop (exit 3 only on an encoder bug).
- **A missing permission at a call or a return is category `permission`.**
- **Handoff question 64:** `T *restrict p` is accepted on a parameter whose contract gives `owns(*p)`, and rejected
  everywhere else.
- **Approved plans go in the repository** as `.claude/plans/<milestone>-<topic>.md` from now on, named after the milestone (the branch name with `.` as `-`, e.g. `m4-5b1b-pointer-params.md`), so that it can be referenced.

A design check (Plan agent, read-only) validated this design against `lower.cpp`, `report.cpp`, the lint and the
fixtures. Its findings are folded in below: ghost values by place and state, one amount per atom, the
compound-assignment hole in the placement rule, by-name cell checks as `permission`, and explicit asserts for every
part of an atom.

## Step 1 (first, before any code): reconcile and trim the plan documents

This is the session's first commit (markdown only), because the planner could not edit in plan mode.

1. **Branch.** In the worktree `.claude/worktrees/m4.2b1`: `git checkout -b m4.5b1b-pointer-params m4.5b1a-pointers`
   (`bdc6bd0`). PR #15 is not merged yet: merge any later commits of it into this branch, never rebase.
2. **`.claude/plans/m4-5b1b-pointer-params.md`**: this plan, as approved (and b1a's plan, `~/.claude/plans/using-plan-md-and-handoff-md-temporal-map.md`, as `.claude/plans/m4-5b1a-pointers.md`).
3. **`CLAUDE.md`**: one line after "Plan:": `Session plans: .claude/plans/<session>.md (the approved plan of each
   session, from M4.5b1b)`.
4. **`plan.md`**:
   - The M4.5 table's b1b row names what b1b holds:
     - pointer parameters and results, `restrict` only with `owns`, `&param`;
     - `WF`; `owns`/`reads`/`live`/`old(*p)` in both spellings;
     - ghost amounts per atom and case splits;
     - framing decided per path;
     - call, return and entry permission checks;
     - `pointer-comparison`.
   - The e2 row: relational comparisons and `p - q` use `pointer-comparison`, which lands in b1b.
   - "Session protocol" gains one line: the approved plan of each session is saved in `.claude/plans/` and named in
     its status entry.
5. **`internal/handoff.md`**, trimmed to what is live. The header names `git show bdc6bd0:internal/handoff.md` for
   everything removed.
   - §1–§2: rewritten for the start of b1b (PR #15's state, this branch).
   - §3:
     - Drop the closed "CI flake on `main`" paragraph; record 0027's amendment holds it.
     - Drop three "From M4.5a" items that b1a closed: round 4's P1 (0032 item 3), "P51 facts" (now in
       `il-inventory.md`), and "the fixtures … M4.5b1 compares" (il-inventory "M4.5b1a").
     - The P52/P54/P55 note becomes "copied in b1b" (task il-1).
   - §4 rules: "(0027 is next)" becomes "(0037 is next)"; plans live in `.claude/plans/`.
   - §4.1 and §4.2: one line each, pointing at `status.md` and at `plan.md`'s M8 entry.
   - §4.4:
     - Drop the S5 plan ("done … for the record") and b1a's "plan as written"; record 0032 and
       `.claude/plans/m4-5b1a-pointers.md` hold them.
     - Condense "What M4.5a decided" to a pointer at records 0028–0036, keeping the VeriFast port-inventory paragraph.
     - b1b's paragraph points at the plan file.
     - e2 loses "the `==` exception of 0028 item 3".
   - §8: drop answered questions 49–63 (pointer to `bdc6bd0`); record question 64 as answered (above).
6. **Memory:** a feedback file `approved-plans-in-repo.md` ("save each approved plan as `.claude/plans/<name>.md` in
   the repository and name it in the status entry; owner, 2026-10-28"), plus its `MEMORY.md` line.
7. **Commit 1** (planning): items 2–5. Local, no Claude trailer.

## Setup (in parallel with step 1)

- **CI:** run `gh pr checks 15`.
  - Green, or red only on the known 0027 mask timeouts: go on.
  - Red on anything else: stop and ask.
- **Build:** `scripts/bootstrap.sh` in the worktree (idempotent), then `cmake --build build/cmake`.
- **A baseline that agents cannot disturb** (handoff §6). The main checkout is on `main`, not on b1a's code. So the
  baseline runs in a temporary detached worktree `.claude/worktrees/b1b-base` at `bdc6bd0`:
  - set it up with submodules by `--reference` (memory `background-worktree-setup`) and bootstrap it;
  - run `run_tests.py -j 8` and the VIR self-test there, in the background;
  - copy its `build/vpr` to `build/m4.5b1b-baseline-vpr` in the session worktree, and the VIR self-test output to
    `build/m4.5b1b-baseline-vir`;
  - remove the temporary worktree at the end of the session.

  Fallback: run the baseline in the session worktree before any build. Agents may edit meanwhile, but nobody builds
  until it finishes.

## Record 0037 — pointer parameters, results and contracts (written before code)

1. **Scope.** The b1b/b1c table; b1c keeps `owning`, `owns_block`, `T**`, jumps out of cell scopes, loops, sequencing
   on objects, 0028 item 13's call placement, the aliasing trap and the ports. Everything outside b1b stays rejected,
   with a diagnostic naming the session that lifts it.
2. **Pointer parameters.**
   - Types as in 0032 item 2.
   - A parameter is a `Ref` formal with `requires WF(p)` (0028 item 2), and its local copy is a `Ref` Viper local.
   - **`restrict`** (question 64) is accepted when the `requires` holds `owns(*p)` (unconditional or
     `p == NULL || owns(*p)`) and the `ensures` returns it. This is sound:
     - every access in the callee needs permission;
     - full permission excludes every other holder;
     - a caller cannot lend two full permissions, or a full permission and a fraction, of one cell.

     Rejected otherwise: on locals, on results, and on `reads`/`live`-only parameters.
3. **Pointer results.**
   - `ensures WF(result)`.
   - A contract may relate `result` to parameters and NULL (`result == p || result == q`).
   - An atom on `*result` is rejected until owning results (b1c, d2).
   - The returned value is a use, so it gets a liveness check at the `return`.
   - A caller's use of a result whose object has ended fails `lifetime`; the twin returns `&local`.
4. **`&param`** (0028 items 1 and 4).
   - The parameter is copied at entry into a fresh one-slot block (`alloc_<t>`, then a store of the formal).
   - The block gets the scope exit of the function body.
   - Contracts keep reading the formal, the entry value (0006 item 5).
   - This lifts 0032's checker refusal of an address-taken parameter copy.
5. **Spec functions in contracts.**
   - The atoms are `owns(*p)`, `reads(*p)` and `live(p)`. Values are `*p`, `old(*p)`, and `old(p)` of a pointer
     parameter (which is `p`), where `p` is a pointer parameter.
   - Spellings:
     - C++: `cerastes::owns`, `reads`, `live`, `old`.
     - C, and also C++: `cerastes_owns`, `cerastes_reads`, `cerastes_live` (variadic), and the macro
       `cerastes_old(e)` over `typeof` (P53).
     - All are declared by `<cerastes.h>` (amends 0008).
     - `is_spec_name` and `is_spec_routine` gain `live` and the C names, and `spec_call_in_code` covers them.
     - Atoms are recognized before encode.cpp:1781's `call_in_annotation`.
   - `live(p)` implies `p != NULL`; a function that accepts NULL writes `p == NULL || live(p)`, as for `owns`.
   - Where atoms may appear:
     - a top-level conjunct;
     - the right of `||` whose left reads no heap, lowered as `!left ==> atom` (Viper refuses `acc` under `||`);
     - an arm of `?:` whose condition reads no heap.

     They are rejected under `!`, inside `==`, and on the left of `||`.
   - Still rejected: `owns_range`, `owns_block`, `forall` over pointers, and places other than `*param` (M4.5e1, d2,
     c, b2).
   - `cerastes_old(x)` of a by-value parameter stays a Viper local, pinned by a test.
6. **Viper meaning and ghost amounts** (0028 items 7, 8, 3a and 14; amends item 8).
   - `owns(*p)` = `p != null && WF(p) && offset_of(p) < len(block_of(p)) && acc(p.v_T) && acc(p.d_T) && p.d_T &&
     <range of T over p.v_T> && acc(lvloc(block_of(p)).lv, a_k)`.
   - `reads(*p)` is the same with `a_k` on the value and the flag.
   - `live(p)` = `p != null && WF(p) && acc(lvloc(block_of(p)).lv, a_k)`.
   - **One `Perm` formal per atom.** Each has `requires none < a_k && a_k < write`, placed before the user's clauses.
   - An `ensures` atom reuses the amount of the `requires` atom on the same place and of the same kind. An `ensures`
     atom with no such partner is rejected.
   - **The caller** sets `a_k := (g_k ? m_k : write) / (N + 1)`, where:
     - `m_k` is `perm(L_k)` for `owns`/`live`, and `min(perm(C_k), perm(L_k))` for `reads`;
     - `g_k` is the atom's guard;
     - `N` is the number of atoms.
   - **The return check** is `g_i ==> perm(X_i) >= a_i + Σ_{j≠i} (g_j && X_j == X_i ? a_j : none)`.
   - `s25_a_reads.vpr`, the shared-`rd` alternative, stays as a verified fixture, and the record says so.
7. **Framing, decided per path** (owner).
   - `f__wf`'s heap reads become ghost value formals, keyed by place and state, each with its type range:
     - pre-state: reads in the `requires` and `old` reads;
     - post-state: plain reads in the `ensures`.
   - Each heap read gets `assert prior && guard ==> OR_i (g_i && q_i == p)`, taken over the earlier atoms of the same
     field, in evaluation order:
     - the `requires` frame reads in the `requires` and `old` reads;
     - earlier `ensures` conjuncts frame plain `ensures` reads.
   - Category `permission`, message "the contract of `f` reads `*p` without owning it".
   - `prior` adds `g_i && g_j ==> q_i != q_j` for two full atoms of one field, and `q_i == q_j ==> h_i == h_j`. Without
     them `f__wf` would be stricter than Silicon.
   - In contract mode `f__wf` has no flag obligation for a dereference and no liveness checks (there is no heap). It
     keeps the one-past-the-end obligation as `pointer-comparison`.
   - **The method's own contract** emits each heap-reading pure conjunct as `F ==> conjunct`, where F is the framing
     formula, simplified to `true` when it is trivial. Once `f__wf` passes, F follows from the earlier clauses, so the
     meaning is unchanged.
   - Silicon's own well-formedness check stays an `internal` row (exit 3 on an encoder bug). The callers need no extra
     read checks and no `call.failed` rows. There is no driver change.
8. **Calls** (0028 items 8 and 11).
   - Before the call come explicit asserts for every part of every callee atom, each atomic: non-null, `WF`,
     `offset < len`, the permissions with flag and range, and the amount bounds. Category `permission`, message "calls
     `f`, which needs to own `*p`, without owning it".
   - Pairwise separation for `owns`/`owns` and `owns`/`reads` on one field, so `swap(&x, &x)` fails `permission`.
   - Then the `lvloc` case splits, the amounts, the value preconditions (`precondition`), and the call with the ghost
     arguments.
   - These are items created with PRE(c), before the value preconditions.
   - Pointer arguments are uses (liveness).
   - The callee's `ensures` come back through Viper's call: no `inhale`.
   - **Placement until b1c (fail closed).** A full expression that holds a call to a callee with a permission atom may
     have no other cell access, except inside that call's arguments and the statement's top-level store target. The
     read of a compound assignment's target counts as an access (`x += f(&x)` and `*p += f(p)` are rejected). There
     may be no second such call. Otherwise `unsupported_expression`, naming b1c.
9. **Entry:** the `lvloc` case splits for each pair of liveness atoms, and
   `assert a != null && b != null ==> a != b` for each pair of fully owned places of one field (`internal`; provable).
10. **Returns.** After the value and the scope exits come each `ensures` permission clause, one by one, with the sums
    of item 6 (`permission`, "returns without owning `*p`, which its contract promises"). Then the value `ensures`
    (`postcondition`). Silicon's own postcondition check stays `internal`.
11. **By-name cell checks** (`read_cell`/`write_cell`, lower.cpp:806, :817) become category `permission`. A callee can
    now consume a cell, so `keep(&x); return x;` is a user error, not exit 3. This changes the category column of b1a's
    memory maps; no expectation changes.
12. **Comparisons.** b1a's one-past-the-end row becomes `pointer-comparison` ("compares `p` and `q`, which may point
    into different objects, one of them one past its end"), in code and in contracts. It is reachable now, because
    `WF` allows a parameter at `offset == len`. `p == NULL` is never checked.
13. **The consuming callee.** A callee whose `requires` takes `owns(*p)` and whose `ensures` does not return it keeps
    the cell. The caller's scope exit then fails `lifetime` (b1a's missing twin), and a later read fails `permission`.
14. **Lint:**
    - ghost `Perm` formals only with `requires none < a && a < write`;
    - `wildcard` refused;
    - the case-split `if (…) { }` lines accepted;
    - `inhale`/`assume` still refused.
15. **Rejected until b1c or later**, each pinned by a test:
    - `restrict` without `owns`;
    - atoms in other positions, or on `*result`, a local or a global;
    - an `ensures` atom with no partner;
    - item 8's placement.
16. **Amendments:**
    - 0008 (the header);
    - 0011 items 9–10 (`pointer-comparison`; `permission` at calls and returns);
    - 0012 item 12;
    - 0028 items 4 (`&param`), 8 (amounts) and 11;
    - 0029 items 3 and 5 (`live` and NULL);
    - 0032 items 2, 4, 6, 7 and 10.

## Team (disjoint files; three Sonnet agents, one Opus reviewer)

| Agent | Owns |
|---|---|
| **edg-backend** (Sonnet) | `src/backend` (`encode.cpp`, `annotation_checks.cpp`, `il_query.cpp`, `annotation_il.h`, `diagnostics.def`): pointer parameters and results, `restrict` with `owns`, `&param`, the spec functions in both spellings, places through P53's casts and `old`'s address forms, the position rule, the placement rule. `include/cerastes.h`. `src/vir/vir.h`, `vir.cpp`, `check.cpp`: `Owns`, `Reads` and `Live` nodes; `Deref`/`Old` of a `Formal` in contracts; the contract/invariant split of `no_ptr_` (check.cpp:336, :605); the backstops. **`vir.h` first (≈30 min), from the record.** Then `src/viper/emit.cpp` and `prelude.cpp`, once `lowered.h` is frozen. |
| **il-analyst** (Sonnet) | First: probe P58 (the IL of pointer parameters and results, `restrict`, `&param`, C++ `cerastes::old(*p)`, and C `cerastes_owns`/`cerastes_live`/`cerastes_old(*p)`/`cerastes_old(x)` with the real header in both modes) and spike S27 (the design check's Silicon probes P1–P5: wf of `requires` and `ensures`, `call.failed` at callers, item 7's guarded form well-formed, the order of the amount bounds, per-atom amounts in `f(p, p)` with a guard-false amount). Task il-1: copy P52/P54/P55 from `build/m4.5a/` into `il-inventory.md`. Then `lowered.h`/`.cpp` (a `Perm` type, `Old`, `Acc` with an amount, `Perm` `/` and `?:`), frozen with edg-backend, and `src/vir/lower.cpp`: contract mode, `f__wf` framing, atoms, entry, calls, returns, `&param`, categories. Then the fixture comparison and the baseline diff. |
| **test-mutator** (Sonnet) | The baseline worktree and run. **First: hand-written target `.vpr` for five shapes** (owns plus `old` across a call; reads with `f(p, p)`; `live` plus `pointer-comparison`; the consuming callee; a guarded framed contract with its `f__wf`), run through Silicon. They are il-analyst's targets and part of S27. Then: `scripts/lint_viper.py` and its self-test; `pointer-comparison` in the category copies (`run_tests.py`, `test_run_tests.py`, `tests/README.md`); every new test and twin in `tests/memory/`, with expectations written from the record before the code runs; the flips; the VIR self-test (`selftest_cases8.inc`, `VIR_CASES_MIN`); the doc-example files. |
| **soundness-auditor** (Opus, read-only) | Round 1: record 0037, in the background as soon as it is drafted. Round 2: the code diff and the tests. One re-audit of blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Step 1; record 0037 and the amendments; `doc/`; `internal/encoding.md`; integration; serialized builds (`lockf build/m4.5b1b/build.lock cmake --build build/cmake`); status and handoff; stop and ask. |

Any question from an agent comes to the owner: that work stops and the question is asked.

## Tests (`tests/memory/`, `.c` in both modes with the C spellings; each positive with a twin that fails for one reason)

| Positive | Twin (expected) |
|---|---|
| `inc(int *p)`: `requires(owns(*p) && *p < INT_MAX)`, `ensures(owns(*p) && *p == old(*p) + 1)`, on `&x` | wrong `ensures` → `postcondition`; a caller holding a dangling `p` → `permission` at the call |
| `swap(int *a, int *b)` with `owns` both | `swap(&x, &x)` → `permission` at the call |
| `peek(const int *p)` with `reads(*p)`, called twice, then the caller writes; `f(&x, &x)` with two `reads` | a `reads` callee that writes → `permission`; one that returns less → `permission` at its return |
| `p == NULL \|\| owns(*p)` with a guarded body | an unguarded dereference → `null-dereference` |
| the consuming callee | the caller's scope exit → `lifetime`; a later read → `permission` |
| `same(int *p, int *q)` with `owns` both, `return p == q` | with `live` only → `pointer-comparison` |
| `pick(int c, int *p, int *q)` with `ensures(result == p \|\| result == q)` | a callee returning `&local` → `lifetime` at the caller's use |
| `&param` (`int *q = &a; *q = 1; return a;`) | a read of `*q` after the function's block → cannot be written; twin: the read before the write → `uninitialized` |
| `restrict` parameter with `owns` | `restrict` with `reads` only → rejected |
| framed contracts, unconditional and guarded (`p == NULL \|\| owns(*p)`, then `p == NULL \|\| *p > 0`) | `requires(*p > 0)` alone → `permission`; a mismatched guard → `permission` on that path only |
| `cerastes_old(x)` of a by-value parameter (pin: a Viper local) | — (pin) |

Rejection pins for b1c or later:
- an atom on `*result`;
- `restrict` without `owns`;
- an atom under `!` or on the left of `||`;
- `owns(*local)`;
- `owns_range`/`owns_block` over a pointer;
- an `ensures` atom with no partner;
- `x + f(&x)`, `x += f(&x)`, and two memory calls in one full expression.

**Flips (pre-approved):**
- `tests/scalar/negative/reject_pointer.c` and `tests/memory/negative/reject_{pointer_param,pointer_result,address_of_param}.c`
  are retired, since they are accepted now.
- `reject_implicit_function_pointer_argument.c` is re-expected only if its diagnostic changes.
- The VIR self-test cases `mem_rej_pointer_parameter`, `_pointer_result`, `_pointer_in_contract` and
  `_address_of_parameter` become lowering cases.

**Also approved with this plan:** b1a's `tests/memory` and memory/error-messages doc examples change only in two
rows' category and message (record items 11 and 12). The new pins that b1c lifts are pre-approved for b1c in the
status entry.

## Docs (invariant 8; every example is a test)

- **Memory and ownership:** a "Pointer parameters and contracts" section covering `owns`, `reads`, `live`, `old`, NULL
  guards, results, `&param`, `restrict`, both spellings and the consuming callee. The "not supported yet" list is
  updated.
- **Annotations:** the spec-function table is rewritten as "What it means", with `live` and the C spellings.
- **Error messages:** "sixteen" becomes "seventeen", with a `pointer-comparison` section and its `_error.c`/`_fixed.c`
  pair. `permission` gains the call, return and contract-framing messages.
- **Verifying a program:** the pointer list is updated.
- **`internal/encoding.md`:** §2–§4 for b1b.

## Order of work (no time cap; the critical path first)

1. **Step 1, setup and the baseline.** P58 and S27; record 0037 (auditor round 1 in the background); `vir.h`; the
   target `.vpr` shapes.
2. **Freeze `lowered.h`.** Then lower.cpp's contract mode and `f__wf` framing (the longest item), then the call,
   return and entry checks, then `&param` and results. In parallel: the encoder (atoms, spellings, placement, results,
   `restrict`), emit/prelude, lint, tests.
3. **End to end on Silicon** with single-file runs (`build/cerastes verify`); the fixture comparison; the baseline diff.
4. **Audit round 2**, fixes, one re-audit.
5. **`check.sh`** (full, at most twice); docs; status, handoff; commits.

## Stop rules (correctness only; no time cuts)

Stop and ask:
- any agent question;
- PR #15 red on something other than the mask timeouts;
- any existing test whose outcome or Viper changes, other than the pre-approved flips and item 11/12's category rows;
- S27 contradicting item 7, where `f__wf` and Silicon's own check disagree;
- Silicon's own reason before an explicit check that cannot be fixed;
- a new test over 60 s.

## Acceptance (commands run at the end)

- `python3 scripts/run_tests.py tests/memory tests/viper-smoke/memory doc/modules/ROOT/examples/memory doc/modules/ROOT/examples/error-messages`:
  all pass.
- `diff -r -x MANIFEST build/m4.5b1b-baseline-vpr build/vpr`: only added files, the retired tests, and b1a's memory
  rows of items 11 and 12 (listed). The VIR self-test output against `build/m4.5b1b-baseline-vir`: only added cases and
  the four flipped `mem_rej_*`.
- `python3 scripts/test_run_tests.py` and `python3 scripts/test_lint_viper.py`: green.
- `scripts/build_docs.sh` and `python3 scripts/check_doc_examples.py --html build/doc`: green.
- `scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): green.
- The status entry lists the records, the flips, the new b1c pins, the audit outcome, the plan file's path and b1c's
  first step.
- Commits: planning; feature (code, tests, docs, records); status and handoff. Local only, no Claude trailer; the
  temporary baseline worktree is removed.
