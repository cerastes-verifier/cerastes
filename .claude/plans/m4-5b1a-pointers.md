# Plan: M4.5b1a — the memory model and address-taken locals inside one function (records 0032–0036)

## Context

`plan.md` and handoff §4.4 put **M4.5b1** next. It is the first session in which the encoder emits record 0028's
memory model (E2):
- a pointer is a Viper `Ref` over a `Block` domain with four axioms;
- every block has a liveness cell;
- every operation is preceded by explicit checks.

Today none of that exists:
- VIR has no heap (`Type::Kind` is `{Void, Bool, Int}`, `vir.h:61`).
- Every pointer type is rejected (`vtype()`, `encode.cpp:395`).
- Lowering has no hook at scope exits.

A design pass estimated handoff §4.4's M4.5b1 at well over 27 agent-hours for `src/vir` and `src/viper` alone.

**Your answers today:**
- **Three sessions.**
  - **b1a**, this session: the model, plus address-taken locals inside one function.
  - **b1b**: pointer parameters and contracts.
  - **b1c**: the refinements.
  - Each session rejects what a later one makes precise (fail closed, invariant 3).
- **CI flake on `main`: ignored for now.** The M4.5a merge push and the 2026-10-28 nightly each lost one 0027 mask
  must-fail test, in C23, to the 300 s timeout (`mask_or_xor_round_trip_chain_three_post.c`, then
  `..._chain_high_part_post.c`). A handoff §3 note.
- **Old rejection tests.** An existing test whose only point was "pointers are rejected" is retired in the feature
  commit and listed. Any other existing test whose outcome or Viper changes stops the session.
- **b1b's and b1c's flips are pre-approved.** Each fail-closed rejection test of b1a is listed in handoff §4.4 as
  "re-expected by b1b" or "by b1c". Exactly those tests may be retired or re-expected later.
- **Loop permissions (b1c) are hybrid.**
  - Automatic for places reached through an address-taken local or a pointer parameter the function never reassigns.
  - For everything else, you write `owns`/`reads` in `CERASTES_INVARIANT`.
- **Handoff questions 59–63** (uncommitted in the main checkout) go into the planning commit:
  - 59, 60 and 63 as recommended;
  - 61: the Linux check is already required;
  - 62: from now on, `ssh matt@clang4.cpp.al` (key `~/.ssh/id_rsa`) replaces the box. The memory file
    `x86-box-edg-suites.md` is updated to match.

| | **b1a (this session)** | **b1b** | **b1c** |
|---|---|---|---|
| Model | VIR heap, the `Block` domain, `v_T`/`d_T`/`lv` fields, a trusted `alloc_T`, lint tags, the four axiom records | — | — |
| Objects | Address-taken locals as one-slot blocks, allocated at block entry; the scope exit at fall-through and `return` | `&param` | Pointer-typed cells (`T**`); `goto`/`break` out of a cell's scope; cells in loops (hybrid invariants) |
| Operations | `&x`; reading `*p` and an address-taken `x`; statement-level writes; NULL in every spelling; `==`/`!=`; truth tests; pointer locals; `?:` of pointer type | — | Writes inside expressions (`*p = (*q)++`, `x++ + *p`) |
| Contracts | none (pointer parameters and results rejected) | `WF`; `owns`/`reads`/`live`/`old` in C++ and C spellings (`<cerastes.h>`); ghost amounts; case splits; framing; call and return permission checks | `[[cerastes::owning]]` (`p`, `r`), `owns_block` |
| Categories | `null-dereference`, `permission`, `lifetime` | `pointer-comparison`; the consuming callee's `lifetime` at scope exit | `unsequenced`, path-precise (0028 item 12); call placement (item 13) |
| C front end | Incompatible pointer types are errors in C verify mode (audit round 4's reopened item) | — | — |
| Docs, ports | The Memory and ownership page starts; three Error messages sections | The page gains contracts; Annotations table rewritten | Ports (`swap`, `inc`, the scalar functions of `address_of_local.c`); the page completed |

## What planning found

**Encoder** (`src/backend/encode.cpp`)
- Pointers are rejected in five places:
  - types: `vtype()` :334-398 and `check_signature` :484-499;
  - operators: `operation()` :1771;
  - assignment targets: `lvalue_var` :2076;
  - constants: `constant_value` :783-790;
  - casts: `conversion()` :1597.
- Calls of `main` are already rejected (`callee_for_call` :1833).
- `address_taken` is read nowhere.
- The 0020 `Sequencer` (`check.cpp:885`) is keyed by `Var::id`, so it cannot see writes through `*p`.

**VIR, lowering, emit**
- The lowered IR is Int/Bool only.
- Obligations are built by `meta()` and `obl()` (`lower.cpp:600-629`) and placed by `flush` (:1415).
- `ret()` (:1846) asserts the `ensures` explicitly at each return.
- `jump()` (:1738) knows only a label name.
- Block lowering (:1716) handles entry only.
- Trusted bodyless methods already print their tag (`emit.cpp:165`).

**Lint** (`scripts/lint_viper.py`)
- It refuses any `domain` (:467) and any `axiom` (:479).
- It does **not** refuse `new`, although 0028 item 2 says it "keeps refusing" it.
- `FLAG_ANYWHERE_RE` (:656) would refuse a field-flag write `p.d_int := true`.
- Every clause line needs a map row with a C position (:499-505, `posmap.cpp:18`).

**Driver**
- Only `assertion.false` is accepted (`report.cpp:122-130`). That is all b1a needs.
- The category table lives in five places:
  - 0011 item 10;
  - `DRIVER_CATEGORIES` (`run_tests.py:316`);
  - `tests/README.md:368`;
  - `error-messages.adoc` ("There are thirteen", rule h of `check_doc_examples.py`);
  - `test_run_tests.py` :1985 and :10149.

**Design-pass findings taken into b1a**
- **Allocate at block entry** (C23 6.2.4p6), and before a switch's dispatch. A `goto` past a declaration, or a case
  label after one (0019 item 6), then finds the block allocated. This amends 0028 item 4's "at its declaration"; the
  two agree wherever nothing jumps past the declaration.
- **Loops: a pointer node fails its liveness check.** A loop body sees only its invariant, so any pointer node fails
  there, not only a dereference. b1a therefore rejects any loop that holds:
  - a pointer-typed node, a cell, `&` or `*`;
  - a `return` from inside the loop while a cell of an enclosing scope is live.
- **A jump that leaves a cell's scope** (`break` out of a switch body with a cell, a `goto` outward) is rejected in
  b1a (b1c).
- **Every new map row needs a C position.**
  - A scope exit is reported at the block's closing brace (`end_loc`, from EDG's `final_position`, as
    `encode.cpp:523` does for function bodies).
  - `alloc_T`'s rows are reported at the first address-taken declaration of `T`.
- **The fixtures disagree on `member_lv`'s trigger.** 43 use `{ lvloc(member(r, k)) }`, which is 0028's text; 12 use
  `{ member(r, k) }`. The member_lv record fixes it as 0028 states, and the 12 fixtures are changed and rerun with it.

**Evidence and other facts**
- **P51 is not in `il-inventory.md`.** It exists only in the worktree as `build/m4.5a/P51.md`, although M4.5a's commit
  message says it was added. b1a adds it.
- **P56 (measured in planning).** In C23, EDG gives these as warnings:
  - #42: `pi == pl`;
  - #144: `long *pl = pi;`, and also `int *c = pc;`, which drops `const`;
  - #513: `pl = pi;`;
  - #167: an incompatible argument.

  `int *d = v;` from a `void*` gives no diagnostic at all.
- **CI.**
  - S5's PR #14 is open; its run `40739057963` was in progress during planning.
  - Local `main` (`794e960`) merges S5 and is 5 commits ahead of `origin/main`.
  - The worktree `.claude/worktrees/m4.2b1` is on the merged `m4.5a-memory-design`.

## Record 0032 — pointers inside one function (b1a), written before code

1. **Scope and the three-way split** (the table above). Everything outside b1a is rejected with a diagnostic naming
   the session that lifts it.
2. **Pointer types accepted.** A pointer type is accepted when:
   - its pointee is an integer type, `bool` or an accepted enumeration, `const`-qualified or not;
   - it is the type of a local or of an expression.

   Rejected until a later session:
   - pointer parameters, pointer results and pointer-typed cells (`int **`);
   - `void*`, which is also stricter than 0028 item 1: nothing in b1a needs it before M4.5d2;
   - struct, union and function pointers;
   - `volatile` pointees.
3. **Conversions.** Every pointer conversion node, compiler-generated or not, is rejected except two:
   - a null pointer constant becoming `T*`;
   - a conversion that only adds qualifiers (`T*` → `const T*`).

   In C verify mode, `--diag_error=42,144,167,513` turns P56's warnings into errors. This closes audit round 4's
   reopened item, with must-fail tests for each shape and for `int *q = v;`.
4. **Address-taken locals.** EDG's `address_taken` flag marks them. They are one-slot blocks from a trusted `alloc_T()`,
   allocated at the entry of their block (item 1's amendment of 0028 item 4) and initialized at the declaration.
   - Fail closed: a `&x` on a variable without the flag is an internal error (exit 3).
   - `cerastes_old`-style uses come with b1b.
5. **The explicit checks** (0028 item 11) for each operation:

   | Operation | Checks, in order (category) |
   |---|---|
   | `*p` read | `p != null` (`null-dereference`); `perm(p.v_T) > none` (`permission`, "reads `*p` without owning it"); the flag (`uninitialized`) |
   | `*p` write | `p != null` (`null-dereference`); `perm == write` for the value and the flag (`permission`) |
   | a cell read by name | permission (`internal`, provable); the flag (`uninitialized`) |
   | any other use of a pointer value: `==`, `!=`, truth test, copy | `p != null ==> perm(lvloc(block_of(p)).lv) > none` (`lifetime`, "uses `p` after the object it points to may have ended") |
   | `==`/`!=` | 0028 item 3's one-past-the-end exception as an `internal` row: provable in b1a, where every pointer is NULL or offset 0 of a one-slot block; b1b makes it `pointer-comparison` |
   | scope exit | full permission to the cell and to its liveness cell is back (`lifetime`), then `exhale` |

   The scope-exit check cannot fail in b1a, because no function can keep a cell. Its must-fail twin (a callee that
   consumes the cell) is listed as b1b's first test.
6. **Fail-closed rules until b1b/b1c** (diagnostics in `diagnostics.def`, each pinned by a test listed in the handoff):
   - a write to a cell anywhere but as an expression statement's top-level operation;
   - a loop holding a pointer node, a cell access, `&` or `*`;
   - a `return` inside a loop while a cell of an enclosing scope is live;
   - a `goto` or `break` that leaves a cell's scope.
7. **Amendments** (in the amended records' tails):
   - 0011 items 9–10: three categories now;
   - 0012 item 12: pointer locals accepted;
   - 0019 item 13: the flag is a field of the cell;
   - 0020 item 4: cells are written at statement level only, until b1c;
   - 0028 item 4: allocation at block entry.

**Records 0033–0036** (0028 item 2: "each axiom gets its own record"): `loc_inv`, `loc_nonnull`, `member_inv`,
`member_lv`. Each states:
- the axiom, its trigger, why it is needed (S24/S26 ablations), the model argument and the vacuity runs;
- its lint tag `// cerastes:axiom-ok <record>`. The domain itself carries `// cerastes:domain-ok 0028`.

The whole domain (eight functions, four axioms) is emitted whenever a program uses memory, and nothing is emitted
otherwise, so existing programs keep their Viper byte for byte. 0036 fixes the 12 fixtures' trigger.

## Setup and the planning commit

1. **CI.** `gh run list` for PR #14:
   - green, or red only on the known mask timeouts: go on;
   - red on anything else (the Linux job's environment, a test outcome): stop and ask.
2. **Branch.** In the worktree `.claude/worktrees/m4.2b1`: `git checkout -b m4.5b1a-pointers main` (`794e960`), then
   `cmake --build build/cmake`.
3. **Commit 1 (planning, markdown only):**
   - your handoff answers 59–63;
   - `plan.md`: the M4.5b1 row becomes b1a/b1b/b1c;
   - handoff §4.4: the three plans, with b1b/b1c's pre-approved flips and the hybrid loop rule;
   - handoff §3: the CI flake note.

   Your copy in the main checkout is then yours to drop, as in M4.4c.
4. **Memory.** Update `x86-box-edg-suites.md` for `matt@clang4.cpp.al` and its index line.
5. **Baseline** (test-mutator, in the background at once):
   - `run_tests.py -j 8`;
   - copy `build/vpr` → `build/m4.5b1a-baseline-vpr`;
   - copy the VIR self-test output → `build/m4.5b1a-baseline-vir`.

   Nothing builds until it finishes. Evidence goes to `build/m4.5b1a/`.

## Team (disjoint files; the standing split, shifted to balance the load)

| Agent | Owns |
|---|---|
| **edg-backend** (Sonnet) | `src/backend` (`encode.cpp`, `diagnostics.def`): pointer types; `address_taken`; `&`, `*`, NULL, `==`, truth tests, `?:`; the conversion allow-list; block `end_loc`; item 6's rejections. `src/driver/verify_main.cpp`: C's `--diag_error`. `src/vir/vir.h`, `vir.cpp`, `check.cpp`: `Ptr` type, `Null`, `AddrOf`, `Deref`, `Store`, `Var::address_taken`, `Block::end_loc`, typing rules, and item 6's backstop. **`vir.h` lands first (≈30 min), from the record.** |
| **il-analyst** (Sonnet) | First: P51 into `il-inventory.md`, and the P57 probes (the `address_taken` flag on a `const` local, on `&x` in `sizeof`, on `*&x`; the C++ and C23 `nullptr` shapes). Then `src/vir/lowered.{h,cpp}`, `lower.cpp`, `src/viper/emit.cpp`, `prelude.cpp`: typed `LVar` (`Ref`); `Null`, `Field`, `PermOf`, `Write`/`NoPerm`, `Acc`; `FieldAssign`, `Exhale`; cells at block entry; the checks of item 5; the scope stack (fall-through and `return`); the memory prelude and `alloc_T` as trusted LMethods. Then the comparison with the fixtures (`il-inventory.md` "M4.5b1a") and the byte-identity listing against the baseline. |
| **test-mutator** (Sonnet) | The baseline. `scripts/lint_viper.py` and its self-test: the domain/axiom tags, `new` refused, the field-flag form, `null` as a `Ref`'s dummy. `run_tests.py` `DRIVER_CATEGORIES` (+3), `DUAL_MODE_DIRS` (+`tests/memory`), `test_run_tests.py`'s pins. Every new test and twin in `tests/memory/`, with expectations written from the record before the code runs. The VIR self-test cases (`src/viper/selftest.cpp` Kit helpers and a new `selftest_cases7.inc`, `VIR_CASES_MIN` in `check.sh`), kept independent of the implementer. The doc-example files. The retirement of `reject_pointer.c` (`git rm` in the feature commit only). The 12 fixture triggers. |
| **soundness-auditor** (Opus, read-only) | Round 1 (background, as soon as they are drafted): records 0032–0036 and P51/P57. Round 2: the code diff (`src/backend`, `src/vir`, `src/viper`, lint, driver) and the tests, including audit round 4's reopened item. One re-audit of blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Records 0032–0036 and the amendments; `doc/`; `internal/encoding.md`; `plan.md`, status, handoff; integration and serialized builds (`lockf build/m4.5b1a/build.lock cmake --build build/cmake`); stop-and-ask. |

**Any question from an agent, or from me, stops that work and comes to you** (your instruction).

## Tests (`tests/memory/`, `.c` in both modes where valid; each positive with a twin that fails for one reason)

| Positive | Twin (expected error) |
|---|---|
| `int x = 1; int *p = &x; *p = 2; check(x == 2);` | `check(x == 1)` → `check` |
| `*p += k`, `(*p)++` with a guard | at `INT_MAX` → `overflow` |
| write through `p`, then read `x` | read before any write → `uninitialized` |
| `p = c ? &x : NULL; if (p) *p = 1;` | the unguarded `*p` → `null-dereference` |
| a pointer to an inner block's local used inside the block | `*p` after the block → `permission` |
| the same, compared inside the block | `p == &y` or `q = p` after the block → `lifetime` |
| `&x != &y`, `p == &x`, NULL in every spelling (`NULL`, `0`, `nullptr` in C23 and C++), `if (p)`, `!p` | `check(&x == &y)` → `check` |
| `return` from nested blocks with cells; fall-through exits | a path that returns a cell it never wrote → `uninitialized` |
| a `goto` past an address-taken declaration in the same block, then write and read | read before the write → `uninitialized` |

Rejection pins (each listed in the handoff for b1b or b1c):
- pointer parameter, pointer result, `&param`, `int **`, `void *`, a struct pointer, a cast between pointer types;
- a loop touching a cell, `return` in a loop with a live cell, `break` out of a switch with a cell, `goto` out of a
  cell's scope;
- `x++` as a value on a cell, `*p = (*p)++`;
- C's four diagnostics (#42, #144, #513, #167) and `int *q = v;`.

Existing tests: `reject_pointer.c` is retired (pre-approved). `reject_nullptr.cpp` (`nullptr == nullptr`, a
`nullptr_t` comparison) and `reject_function_pointer.c` are expected unchanged.

## Docs (invariant 8; every example a test)

- **New page "Memory and ownership"** (nav after Integer semantics):
  - pointers to scalars inside a function;
  - address-taken locals and their lifetime;
  - `&`, `*`, NULL, comparisons and truth tests;
  - dangling pointers;
  - what is rejected today, and which session lifts it.
- **Error messages:** "thirteen" → "sixteen"; sections `null-dereference`, `permission`, `lifetime`, each with an
  `_error.c`/`_fixed.c` pair.
- **Verifying a program:** "What is not supported yet" updated (pointers).
- **`internal/encoding.md`:** VIR §2, lowering §3, Viper shape §4.

## Timeline (≤ 4 h)

| Time | Work |
|---|---|
| 0:00–0:20 | Setup, planning commit, baseline started |
| 0:15–0:50 | Records 0032–0036 drafted (auditor round 1 in the background); il-analyst P51/P57; edg-backend `vir.h`; test-mutator lint and tests from the record |
| 0:50–2:45 | Implementation in parallel; builds only after the baseline finishes |
| 2:45–3:15 | Integration: single-file runs, fixture comparison, baseline diff |
| 3:00–3:35 | Audit round 2, fixes, re-audit |
| 3:35–4:00 | `check.sh` (at most twice), status, handoff, commits |

## Stop rules and cuts

**Stop and ask:**
- any question from an agent;
- PR #14 red on something other than the mask timeouts;
- any existing test whose outcome or Viper changes, other than the pre-approved retirement;
- a probe that contradicts P51, 0028 or 0029 (for example a shape that takes an address without the flag);
- Silicon's own error (any reason other than `assertion.false`, or a failing `internal` row) before an explicit check,
  on any test, that cannot be fixed in the session;
- a new test whose Silicon run takes more than 60 s.

**Cuts, in order:**
1. `?:` of pointer type moves to b1b.
2. The `goto`-past-declaration test moves to b1b (block-entry allocation stays).
3. The mutation run over `tests/memory` moves to b1b (only if time is left anyway).

## Acceptance (commands run at the end)

- `python3 scripts/run_tests.py tests/memory tests/viper-smoke/memory doc/modules/ROOT/examples/memory doc/modules/ROOT/examples/error-messages`:
  all pass.
- `diff -r -x MANIFEST build/m4.5b1a-baseline-vpr build/vpr`: only added files (new tests and doc examples) and the
  retired test. The same holds for the VIR self-test output against `build/m4.5b1a-baseline-vir`.
- `python3 scripts/test_run_tests.py` and `python3 scripts/test_lint_viper.py`: green.
- `scripts/build_docs.sh` and `python3 scripts/check_doc_examples.py --html build/doc`: green.
- `scripts/check.sh` (the full check, `CERASTES_CHECK_BASE` unset): green.
- The status entry lists:
  - the records added;
  - the retired test;
  - the fail-closed pins for b1b/b1c;
  - the audit outcome;
  - the exact first step of M4.5b1b.
- Commits:
  - planning;
  - feature: code, tests, docs and records;
  - status and handoff.

  Local only, with no Claude trailer.
