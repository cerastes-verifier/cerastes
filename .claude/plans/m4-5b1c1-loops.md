# Plan: M4.5b1c1 — loops and jumps over memory (record 0038)

## Context

M4.5b1b is done and merged into local `main` (`9782bd4`; PR #15, b1a, merged as `0293112`; `origin/main` is at `fca0cbc`,
so b1b's merge is not pushed yet). Records 0032–0037 give address-taken locals, pointer parameters and results, and
contracts with `owns`/`reads`/`live`/`old`. Every loop that touches memory, every jump out of a cell's scope and every
`return` inside such a loop is still rejected (0032 item 10). Handoff §4.4 gave b1c a scope larger than b1b's.

**Owner decisions (2026-10-29, this planning):**
- **b1c is three sessions.**
  - **b1c1 (this session): loops and jumps.** Cells and pointers in loops with automatic permission invariants;
    `owns`/`reads`/`live` in `CERASTES_INVARIANT`; cells, and memory reads in general, in invariants and measures,
    including a function's `decreases`; `goto`/`break`/`continue`/`return` out of a cell's scope; the
    `address_of_local.c` ports these make possible.
  - **b1c2: sequencing and placement.** Writes inside expressions, `unsequenced` (0028 item 12), 0028 item 13's call
    placement (lifts 0037 item 8), handoff question 65 (reject `return &local;` like the compilers) and question 67
    (make `c ? owns(*p) : reads(*p)` path-precise).
  - **b1c3: `T**`, owning parameters, `*result`, the rest.** `T**`; `[[cerastes::owning]]` on parameters (target
    `p`); `*result` as a value, framed by `result == p`; `swap.c` and the aliasing trap; `address_of_local.c`'s `int**`
    functions; the Memory and ownership page completed.
- **Owning results move to M4.5d2.** That is `[[cerastes::owning]]` on functions (target `r`), `owns_block`,
  `owns(*result)` and `ensures` atoms with no `requires` partner. `owns_block` needs the heap token and a whole-block
  quantified permission, and nothing can create such a block before `malloc`.
- **Loop frames: "every place held".** Every loop that touches memory gets an automatic invariant with every cell in
  scope and the place of every pointer parameter the function never reassigns, each with the permission held at loop
  entry. A place keeps its value only when no write in the loop can reach it: any write through a pointer of the same
  type, or any call whose contract owns a cell of that type, havocs every place of that type. Written
  `owns`/`reads`/`live` atoms in an invariant are needed only for pointers the function reassigns.
- **Handoff questions 65–67** (the owner's answers, uncommitted in the main checkout):
  - 65: follow the recommendation (b1c2);
  - 66: fix the cost of aliasing parameters in M4.8;
  - 67: make the rule path-precise (b1c2).

## Step 1 (first, before any code): reconcile and trim the plan documents

This is the session's first commit, markdown only.

1. **Branch.** In the worktree `.claude/worktrees/m4.2b1` (now on `m4.5b1b-pointer-params`), run
   `git checkout -b m4.5b1c1-loops main` (`9782bd4`).
2. **The plan.** Save this plan, as approved, as `.claude/plans/m4-5b1c1-loops.md`.
3. **`plan.md`:**
   - Drop the M4.5 table's b1b row (it is in "Done").
   - Replace the b1c row with three rows: b1c1, b1c2, b1c3, with the scopes above.
   - M4.5d2's row gains owning results: `owning` target `r`, `owns_block`, `owns(*result)`, and `ensures` atoms with
     no partner.
   - M4.8's row gains "the cost of aliasing parameters (handoff question 66)".
   - The M4 "Sessions" text names the split (owner, 2026-10-29).
4. **`internal/handoff.md`**, trimmed to what is live. The header names `git show 8003dca:internal/handoff.md` for
   everything removed.
   - **§1–§2**, rewritten for the start of b1c1:
     - PR #15 is merged; b1b is merged into local `main` but not pushed.
     - The owner's answers to 65–67 sit uncommitted in the main checkout's `internal/handoff.md`. They are recorded
       on this branch: discard them there (`git checkout -- internal/handoff.md`) before merging.
   - **§3**, drop what b1b closed:
     - P52/P54/P55 copied into `il-inventory.md`;
     - the scope-exit twin (`consume_owned_scope.c`);
     - "pointer `?:` as an `if`" (it is in `encoding.md` §3).
   - **§3**, re-target the open notes to their session:
     - `cell_across_loop.c`'s frame → b1c1;
     - re-read `call_atoms`' splits → b1c1's audit round 2;
     - compound stores, `*(g() ? &x : &y) += 1`, `*p = (*q)++`, and 0025 item 2's placement → b1c2;
     - #1056 and `c ? owns : reads` → b1c2 (questions 65, 67).
   - **§4 rules:** "(0037 is next)" becomes "(0038 is next)".
   - **§4.4:**
     - b1c becomes b1c1 (it points at the plan file), b1c2 and b1c3, each with its pre-approved flips (below).
     - M4.5d2 gains owning results and the pins `reject_{atom_on_result,ensures_atom_no_partner}.c`.
   - **§4.5:** M4.8 gains question 66.
   - **§7:** drop it (b1b's acceptance is in `status.md`).
   - **§8:** one line records 65–67 as answered, pointing at `8003dca`; no open questions.
5. **Commit 1 (planning)**: items 2–4, local only, with no Claude trailer.

## Setup (in parallel with step 1)

- **Build:** run `scripts/bootstrap.sh` in the worktree (it was last built for b1b), then `cmake --build build/cmake`.
- **Baseline that agents cannot disturb** (handoff §6). The main checkout is on `main` (`9782bd4`), which is b1b's code
  plus a CI workflow bump. Run the baseline in the background in the main checkout: `scripts/bootstrap.sh`, then
  `python3 scripts/run_tests.py -j 8` and the VIR self-test. Then copy its `build/vpr` to the worktree's
  `build/m4.5b1c1-baseline-vpr`, and its VIR self-test output to `build/m4.5b1c1-baseline-vir`. Nothing in the main
  checkout is edited.

## Record 0038 — loops and jumps over memory (written before code; audit round 1 in the background)

1. **Scope.** A table for b1c1, b1c2, b1c3 and d2. It amends 0029 item 1 (target `r` is registered in d2) and 0037
   item 15's "lifted by" column (placement → b1c2; `*result` as a value → b1c3; atoms on `*result` and `ensures` atoms
   with no partner → d2; a memory read in a `decreases` → b1c1).
2. **Loops over memory.** This lifts 0032 item 10's loop row.
   - Every loop form may use pointers, `&`, `*` and cells.
   - A cell declared in a loop body gets a new block on each iteration (0032 item 4), and its scope exit is at the
     body's closing brace.
   - `do … while (0)` stays straight-line code.
3. **The automatic invariant** (owner: "every place held"). Each loop gets one when its condition, body, step, invariant
   or measure names a cell, uses a pointer or `*`, or calls a function with an atom. A loop that touches no memory gets
   none, so the frame keeps everything and the Viper of every program accepted today stays the same.
   - **Places:**
     - every cell in scope that is declared outside the loop: address-taken locals and `&param` copies;
     - for each pointer parameter that the function never assigns, the places its `requires` atoms name: the value and
       flag cells for `owns`/`reads`, and the liveness cell for every atom.
   - **Amounts:** a ghost `Perm` snapshot per field, taken before the loop: `kv := perm(x.v_T)`, `kd`, `kl`. The
     invariant holds `acc(x.v_T, kv)` and the others.
   - **Parameter places that may alias** are counted once. The place of parameter i of one field gets the guard
     `AND_{j<i} p_i != p_j`, and a liveness cell the guard `lvloc(block_of(p_i)) != lvloc(block_of(p_j))`. Cells never
     alias each other or a parameter: they are fresh blocks.
   - **Values:**
     - `kv > none ==> x.v_T == vs` (`vs` is a snapshot) when no write in the loop can reach the place: no write to it
       by name, no write through a pointer of its field type, and no call of a function whose contract owns a cell of
       that type.
     - Otherwise only the type's range, `kv > none ==> lo <= x.v_T <= hi`.
     - The flag: `ds ==> x.d_T` (0019 item 4's snapshot, for cells).
     - All of these are `internal` rows (provable by construction; exit 3 if not).
4. **Iteration checks** (0028 item 11: Silicon's own invariant checks are never the first to fail).
   - Before the back edge (the body's end, after a `for` step, which every `continue` reaches), for each automatic
     place: `perm(x.v_T) >= kv` and the others. Category `permission`, "an iteration of this loop ends without the
     permission to `x` it began with". Example: a consuming callee in the body.
   - Establishment at entry is `internal`.
   - Silicon's `invariant.not.established` and `invariant.not.preserved` stay `internal` rows.
5. **Written atoms in `CERASTES_INVARIANT`.** `owns(*q)`, `reads(*q)` and `live(q)` (C and C++ spellings) for any
   pointer variable `q` in scope, in 0037 item 5's positions.
   - **Meaning:** `q != null && WF(q) && offset_of(q) < len(block_of(q))` (`live`: no bounds). The permissions are
     guarded by `NOTAUTO(q)`, which says `q` is none of the automatic places of that field. An automatic place is then
     never counted twice, and for one that is, its amount must be positive (`q == x_i ==> kv_i > none`).
   - **Amounts:** snapshots at entry (`owns`: write on the value and the flag; liveness and `reads`: what is held).
   - **Checks:** explicit, category `permission`:
     - at entry: "enters this loop without owning `*q`, which its invariant needs";
     - at the back edge: "an iteration of this loop ends without owning `*q` …".
   - A missing atom is reported at the access, by the access's own check (owner's rule).
6. **Memory in invariants, measures and conditions.**
   - Allowed: cells by name, `*q`, `&x`, pointer `==`/`!=` and NULL. In an invariant or a measure, `==` is identity
     in the model (as in contracts, 0037 item 12), with no liveness check. A loop condition is code and keeps its
     checks, as obligations.
   - **Framing:** each heap read of an invariant or measure must have an automatic place, or an earlier written atom,
     on its path. It is asserted explicitly at entry and before the back edge, as `permission`: "this loop invariant
     reads `*q` without owning it".
   - In the invariant itself, each read is guarded by its own ite chain over the framing disjuncts (0037 item 7's
     form), so Silicon's well-formedness check passes.
   - A loop measure that reads a cell is Silicon's loop `decreases` (S28 checks this).
7. **Jumps out of a cell's scope.** This lifts 0032 item 10's jump rows.
   - At a `break`, `continue`, outward `goto` (0015) or `return` inside a loop, the scope exit of 0032 item 8 is written
     for each block the jump leaves, innermost first. It is reported at the jump (`lifetime`).
   - A switch's `break` lands before the body's scope exit (as today), so it exits only the blocks nested inside the
     case.
8. **Function measures that read memory** (cut 1). This lifts 0037 item 5's `decreases` rejection.
   - `decreases(*p)` must be framed by the `requires`: it is part of `f__wf`'s pre-state.
   - At a recursive call, the explicit check (0021 item 4) compares the callee's measure, over the arguments in the
     current heap, with the caller's measure over its entry heap (`old`).
   - S28 P6 decides how Silicon's own termination check reads it.
9. **Lint.**
   - A `Perm` local is assigned only `perm(…)`, `write`, `none` or a ghost amount, never `wildcard`.
   - `acc` with such an amount is allowed in a loop invariant.
   - `inhale` and `assume` stay refused.
10. **Still rejected,** each with a diagnostic naming its session:
    - writes inside expressions and calls beside accesses (b1c2);
    - `T**` and `*result` (b1c3);
    - `owns(*result)` and `ensures` atoms with no partner (d2);
    - atoms in `CERASTES_CHECK` or in a measure.
11. **Amendments:** 0014 item 4 (scope exits on jump paths); 0019 item 4 (flag snapshots of cells); 0028 items 4 and 8
    (loops); 0029 item 1; 0032 items 4, 8 and 10; 0037 items 1, 5 and 15.

## Team (disjoint files; three Sonnet agents and one Opus reviewer, `.claude/agents/`)

| Agent | Owns |
|---|---|
| **edg-backend** (Sonnet) | `src/backend/encode.cpp`: lift `pointer_node_in`'s invariant and measure rejections (≈2488, 2522); `spec_atom` (≈1801) in `Annot::invariant` for places `*q` of any pointer variable; the `decreases` dereference (≈1906, cut 1); the remaining b1c pins' messages naming b1c2, b1c3 or d2. `src/vir/vir.h`, `vir.cpp` and `check.cpp`: an `Atom` whose place is a `Var`, invariant atoms in `check_program`; lift `MemoryWalker`'s loop, jump and return rules (check.cpp ≈1105–1245) and keep backstops for what stays rejected. **`vir.h` first, from the record.** |
| **il-analyst** (Sonnet) | First: probe **P59** (the IL of atoms in `CERASTES_INVARIANT` in both modes, `&x` and `*q` in invariants, cells in measures, `decreases(*p)`) and spike **S28** (P1: a cell in the frame across a `break`, kept by Silicon's out-edge merge; P2: snapshot amounts, value and flag invariants, explicit back-edge checks failing before Silicon's; P3: parameter de-duplication with 2–4 `reads`, timed on a quiet machine; P4: heap reads in the condition and the loop `decreases`; P5: chain-guarded invariant reads well-formed; P6: a method `decreases` that reads the heap; P7: scope exits at `break`, `continue` and `goto` out of a body; P8: `NOTAUTO`-guarded written atoms). Then `src/vir/lower.cpp`: `loop()` (≈2916, the automatic invariant, snapshots, checks, written atoms, framing), `jump()` (scope exits for the blocks a jump leaves, using `fn_.scopes`), the measures, and `lowered.h`/`emit.cpp` only if a node is missing. Then the baseline diff. |
| **test-mutator** (Sonnet) | First: the hand-written target `.vpr` for S28 (with il-analyst). Then `scripts/lint_viper.py` and its self-test; the tests and twins (below), written from the record before the code runs; the ports; the retired pins; the VIR self-test (`selftest_cases9.inc`, the flipped `mem_rej_*`, `VIR_CASES_MIN`); the doc example files. |
| **soundness-auditor** (Opus, read-only) | Round 1: record 0038, as soon as it is drafted. Round 2: the code diff and the tests, including a re-read of b1b's call-site splits (`call_atoms`, handoff §3). One re-audit of blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Step 1; record 0038 and its amendments; `doc/`; `internal/encoding.md`; `il-inventory.md` (P59, S28); integration; serialized builds (`lockf build/m4.5b1c1/build.lock cmake --build build/cmake`); status and handoff. Stops and asks the owner. |

Any question from an agent, or of mine, stops that work and goes to the owner.

## Tests (`tests/memory/`; `.c` in both modes; each positive has a twin that fails for one reason)

| Positive | Twin (expected) |
|---|---|
| A cell written by name in a loop, with `CERASTES_INVARIANT(x == i)`, used after it | wrong invariant → `invariant-preserved` |
| `int *p = &x; while (…) { *p += 1; … }` with the invariant on `x` | no value invariant → `overflow` at the write |
| A cell only read in the loop keeps its value (`ensures` on it) | a write through another `int *` in the loop havocs it → `postcondition` (the owner's rule, pinned) |
| A parameter with `owns(*p)` filled in a loop; one with `reads(*p)` summed | `reads` only, writing in the loop → `permission` at the write |
| A non-consuming callee called in a loop | a consuming callee on an outer cell → `permission` at the back edge |
| A cell declared in the body (one block per iteration), with `continue` | a consuming callee on it → `lifetime` at the body's `}` |
| `p = c ? &x : &y` in the loop, with `CERASTES_INVARIANT(p == &x \|\| p == &y)` | without that invariant → `lifetime`/`permission` at the use |
| A reassigned parameter's place reached through a pointer local, with `owns(*r)` in the invariant; a written atom on an automatic place (not counted twice) | without the atom → `permission` at the access; an unframed read `*r` in the invariant → `permission` at the invariant |
| A cell in an invariant (b1a's `cell_in_invariant` shape) and in a measure (`DECREASES(k)` with `*p -= 1`) | the measure does not decrease → `termination` |
| `break` / `continue` / `goto` / `return` out of a block with a cell, inside and outside loops; a switch `break` with case-local cells | a callee kept the cell → `lifetime` at the jump |
| `down(int *p)` with `decreases(*p)` (cut 1) | no decrease → `termination` at the call |

**Ports** (cut 3: they move to b1c3). `tests/memory/verifast/address_of_local.c` follows VeriFast's `examples/address_of_local.c`
(`~/Documents/verifast` at `0fb63506`), with an adaptation header and a line map. It takes every function except the
`int**` pair (b1c3) and the non-portable `destroy_half`/`looptrouble`:
- `inc`, `inc_uintptr_t`, `address_of_param`;
- `test_goto` 1–3, `test_break`, `test_break2`, `test_requires_ensures_loop`, `break_statement`;
- `destroy` (trusted, bodyless);
- VeriFast's three `should_fail` lines as `expect: error lifetime` at their closing braces: `dispose_local`, and
  `dispose_half_local` and `dispose_half_local2` with whole permissions.

Loops get measures; overflow needs `requires` bounds (0022).

**Flips (pre-approved by the owner; this plan assigns them to b1c1):**
- Retired, since their construct is accepted, each replaced by a positive of the same shape:
  `tests/memory/negative/reject_{break_out_of_switch_with_cell,cell_in_invariant,cell_in_measure,goto_out_of_cell_scope,loop_cell,loop_pointer,return_in_loop_with_cell}.c`,
  and `reject_heap_read_decreases.c` if cut 1 lands.
- VIR self-test: `mem_rej_{loop_with_a_pointer_node,loop_with_a_cell,loop_with_a_dereference,loop_with_an_address,return_in_a_loop_with_a_live_cell,goto_out_of_a_cell_block,break_out_of_a_switch_with_a_cell}`
  become lowering cases.
- Assigned to later sessions (expect lines unchanged; only messages and `Pins:` comments change):
  - b1c2: `reject_{cell_postfix_value,write_in_assignment_value,call_beside_access,call_compound_target,two_memory_calls}.c`;
  - b1c3: `reject_{address_of_pointer,pointer_to_pointer,result_value}.c`;
  - d2: `reject_{atom_on_result,ensures_atom_no_partner}.c`.

**No existing test's outcome or Viper changes** (a loop that touches no memory gets no automatic invariant). Any change
stops the session.

## Docs (invariant 8; every example is a test)

- **Memory and ownership:**
  - a "Loops" section: what a loop gets automatically, which values are kept, written atoms, invariants and measures
    over memory;
  - a "Leaving a block" section: `break`/`continue`/`goto`/`return`;
  - the "not supported yet" list updated.
- **Error messages:** `permission` gains the loop entry, iteration-end and invariant-framing messages; `lifetime` gains
  jumps.
- **Annotations:** atoms in `CERASTES_INVARIANT`.
- **Verifying a program:** the loop list.
- **`internal/encoding.md`:** loops and jumps.

## Order of work

1. Step 1, setup and the baseline. In parallel: P59, S28 with the target `.vpr`, record 0038 (audit round 1 in the
   background), `vir.h`.
2. `lower.cpp`'s loop and jump work (the critical path). In parallel: the encoder and VIR checker, the lint, tests and
   ports.
3. End to end with single-file runs (`build/cerastes verify --keep-vpr`), then the baseline diff.
4. Audit round 2, fixes, one re-audit.
5. Docs; `check.sh` (full, at most twice); status and handoff; commits.

## Stop rules

The session is planned at ≤ 4 h. If it runs long, cut in this order and record each cut in the handoff:
1. function measures that read memory → b1c2;
2. written atoms in invariants → b1c3 (fail closed: they stay rejected, and a place that is not automatic is reported
   at its access);
3. the ports → b1c3.

Stop and ask on:
- any agent question;
- an existing test whose outcome or Viper changes;
- S28 contradicting item 3 or 4 (Silicon dropping the frame at a `break`, or failing first at its own invariant check);
- Silicon's own reason before an explicit check that cannot be fixed;
- a new test over 60 s.

## Acceptance (commands run at the end)

- `python3 scripts/run_tests.py tests/memory tests/viper-smoke/memory doc/modules/ROOT/examples/memory doc/modules/ROOT/examples/error-messages`:
  all pass.
- `diff -r -x MANIFEST build/m4.5b1c1-baseline-vpr build/vpr`: only added files and the retired pins. The VIR self-test
  against `build/m4.5b1c1-baseline-vir`: only added cases and the flipped `mem_rej_*`.
- `python3 scripts/test_run_tests.py` and `python3 scripts/test_lint_viper.py`: green.
- `scripts/build_docs.sh` and `python3 scripts/check_doc_examples.py --html build/doc`: green.
- `scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): green.
- The status entry lists record 0038, the flips, the cuts if any, the audit outcome, the plan file's path and b1c2's
  first step.
- Commits: planning; feature (code, tests, docs, record); status and handoff. Local only, no Claude trailer.
