# Plan: M4.5b2a — mutable globals and `static` locals (record 0041); M4.5b2b split off

## Context

M4.5b1 is complete (records 0032–0040). M4.5b1c3 is on branch `m4.5b1c3-pointers-to-pointers` (`5ffb1a2`, pushed;
PR #21 open with CI pending at planning). The main checkout's local `main` `774fde4` is a local merge of that branch.
Its working tree holds the owner's uncommitted answers to handoff questions 78–79.

`plan.md`'s M4.5b2 row has four parts:
- mutable globals and `static` locals as blocks with frames;
- `main` owning the globals;
- 0025 item 2's placement revisited;
- P4222's markers and `--strict-init`.

All four are rejected today, pinned by tests or rejection sites that name M4.5b2:
- mutable globals: `encode.cpp:1703`, pinned by `reject_global.c` and `reject_pointer_global.c`;
- `static` locals: `encode.cpp:3014`, pinned by `reject_static_local.c`;
- `owns(g)`, `old(g)` and reads of a global in contracts: `encode.cpp:2044, 2134, 2190, 2298, 2340`;
- the markers: nothing is registered yet.

**The split** (sizing is mine, per question 77). This planning's design check estimates the whole row at about
1,300–1,500 changed lines plus about 55 tests, which is more than b1c3 and close to b1b. That is too much for one context.
The markers are independent of the rest, so the row becomes two sessions:
- **M4.5b2a** (this session, record 0041, about 7–10 h): globals, `main`'s `program_start`, frames through contracts,
  `static` locals, and 0025 item 2's placement for globals. Ports: `global_points_to_syntax.c` and `globals.c`'s scalar
  part.
- **M4.5b2b** (the next session, record 0042): `[[cerastes::uninit]]`, `ref_to_uninit` and `must_init` on parameters,
  `--strict-init` with a `// verify-options:` harness header, and the move of the call's flag row. Its owner decisions
  (82, 83) are already taken and are recorded in the handoff at step 1.

**Owner decisions (2026-10-30, this planning):**
- **78, `const` objects:** follow the recommendation; they are modelled in M4.6. `&K` of a `const` global stays rejected,
  now naming M4.6.
- **79, written atoms on moved pointers:** follow the recommendation; fixed in M4.8 together with question 66.
- **80, `static` locals:** an implicit `owns(s)` in `f`'s `requires` and `ensures`, and in every function that reaches
  `f` in the call graph. `main` gets it from `program_start`.
  - There is no inhale, recursion works, and the value is unknown on each entry.
  - `f() + f()` is reported `unsequenced`, and so is `h() + k()` when both reach `f`, as the two-call rule of 0028
    item 13 says.
- **81, flips (pre-approved):**
  - retire `tests/scalar/negative/reject_global.c`, `tests/scalar/negative/reject_static_local.c` and
    `tests/memory/negative/reject_pointer_global.c`, each replaced by a positive of the same shape with a must-fail twin;
  - `reject_address_of_const_global.c` and `reject_case_const_address_taken.cpp` stay rejected, with only their message
    and `Pins:` changed to name M4.6;
  - `reject_owns_of_param_cell.c` stays rejected, with `Pins:` changed to "never" (a parameter's own copy).
- **82 (for b2b):** the call's flag row becomes `uninitialized`, naming the parameter. This changes the `.map` of every
  program that calls an `owns`/`reads` function. b2b's plan adds the VIR self-test row pins this moves
  (`selftest_cases8.inc:73`, `selftest_cases11.inc:137-138`, and possibly :232, :287, :380, :401, :423) to its
  pre-approved flips.
- **83 (for b2b):** under `--strict-init`, an argument to a `ref_to_uninit`/`must_init` parameter must be `&x` of a
  `[[cerastes::uninit]]` local, or the caller's own marked parameter passed unchanged. Anything else is rejected statically
  (`uninit_ref_initialized`).
- **84, initial values by linkage:**
  - internal linkage: the initializer's value in both languages;
  - external linkage in C: the initializer's value (6.9p5: a second definition is undefined; recorded);
  - external linkage in C++: an unknown value, initialized and in range (or `WF`), because another unit's dynamic
    initializer may write it before `main`.
- **Standing:** 69 (the EDG merge waits for the box session); 77 (one context per session, under about 12 h).

## Step 1 (first, before any code): reconcile and consolidate the plan documents

The first commit is markdown only, with one place per live fact.

1. **Branch.** In the worktree `.claude/worktrees/m4.2b1` (on `m4.5b1c3-pointers-to-pointers` at `5ffb1a2`), create
   `m4.5b2a-globals`:
   - from `origin/main` if PR #21 has merged (`gh pr view 21`);
   - otherwise from `5ffb1a2`, so that the b2a PR shows only its own commits once #21 merges;
   - never from the local merge `774fde4`.
2. **The plan.** Save this plan as `.claude/plans/m4-5b2a-globals.md`, with the "Inputs from the handoff" section below.
3. **`internal/handoff.md`**, consolidated:
   - **Header:** one pointer to the text before the trim, `git show 5ffb1a2:internal/handoff.md`.
   - **§1–§2** rewritten:
     - b1c3 is on PR #21;
     - owner steps: merge #21; reset the local `main` to `origin/main` and drop the main checkout's uncommitted
       `handoff.md` edit, whose answers this branch carries; at the end, b2a's branch and PR.
   - **§3:** the notes this session consumes move into the plan's "Inputs"; the rest stay.
   - **§4 rules:** 0041 is b2a's record and 0042 b2b's.
   - **§4.2:**
     - b2a: one line pointing at the plan, with question 81's flips.
     - **A new b2b paragraph:**
       - the markers on parameters (targets `p`, `RAF_NO_FLAGS`) and `uninit` (target `v`);
       - `--strict-init` and its harness header;
       - questions 82 and 83, with 82's self-test row pins as b2b's pre-approved flips;
       - design-check risk 10: a contract read of a `ref_to_uninit` pointee, and the markers beside `owning`;
       - P54.
     - d2's paragraph gains `ref_to_uninit` on locals and functions (targets `v`, `r`).
   - **§4.3:** M4.6 gains question 78's `const`-object model. M4.8 gains question 79 beside 66.
   - **§8:** one line records questions 78–84. No question stays open.
4. **`plan.md`:**
   - the b2 row splits into b2a (naming the plan file) and b2b;
   - the M4.6 row gains `const` objects as cells (78), and the M4.8 row the moved-pointer false alarm (79);
   - the header's "the last: M4.5b1c3" becomes M4.5b2a.
5. **Commit 1 (planning):** local, no Claude trailer (CLAUDE.md overrides the harness default). `status.md` waits for the
   session's end.

## Inputs from the handoff (moved into the plan at step 1)

- **0025 item 2's placement** (M4.4c note; 0028 item 13). b1c2 (0039 items 2–4) never moves the writes and snapshots of a
  cell across a memory call. What is left for globals:
  - a callee with a global atom is a memory call, even with no pointer argument;
  - the place is `loc(GB_g(),0)`, so `g + f()` with `f` owning `g` fails `unsequenced` (`X != X`);
  - Viper locals keep today's placement (0021 item 2).
- **The `check_attributes` registration rule** (`encode.cpp:492`). It is not touched in b2a: no attribute is
  registered. P54 moves to b2b's paragraph.
- **The ports** (`build/m4.5a/ports.md` in the worktree; originals in `~/Documents/verifast/examples/`):
  - **`global_points_to_syntax.c`:** both functions under one contract spelling.
    - Bounds where VeriFast ran with `-disable_overflow_check`: `global1 <= INT_MAX - 5`, and `global4 <= 122` for
      `(char)(v + 5)`.
    - `main` writes `check(...)` after each call, with no `leak`.
  - **`globals.c`:** the scalar half only.
    - `m` with `requires(owns(x) && x == 7)` and `ensures(owns(x) && x == 8)`.
    - `main` checks that `x == 0` (zero initialization).
    - `struct counter *c` waits for M4.5c and M4.5d2.
    - Twins: no `owns(x)`, `x == 9`, `check(x0 == 1)`.

## Setup (in parallel with step 1)

- **Worktree build:** `scripts/bootstrap.sh`, then `lockf build/m4.5b2a/build.lock cmake --build build/cmake`.
- **Baseline that agents cannot disturb**, run in the background in the main checkout (`774fde4`):
  1. `bootstrap.sh`;
  2. `run_tests.py -j 8`;
  3. the VIR self-test;
  4. copy the results into the worktree's `build/m4.5b2a-baseline-{vpr,vir}`.

## Record 0041 — mutable globals and `static` locals

The record is written before code (invariant 6), while P63 and S31 run. Audit round 1 runs on it in the background.

1. **Scope table** (b2a | b2b | M4.5c | M4.5d2 | M4.6 | M8). It amends:
   - 0017: mutable globals beside named constants;
   - 0028 item 5: `static` locals decided; linkage;
   - 0025 item 2: placement for globals;
   - 0037 items 5 and 15: the places an atom may name;
   - 0038 item 3: global places in loops;
   - 0039: keys.
2. **Which globals.** A file- or namespace-scope variable of a verified file that is neither `const` nor `constexpr`
   (those stay named constants, 0017).
   - **Accepted types:** `bool`, an integer, an enumeration, or a pointer at any depth to one of these.
   - **Rejected, each with a message:**
     - `volatile`, `_Atomic` and thread-local variables;
     - arrays (e1), structs and unions (c/M8), `void*` and struct pointees;
     - an `extern` declaration with no definition in the unit (M8);
     - a C++ initializer that is not a constant;
     - an initializer that is the address of a `const` object (M4.6), a string literal (M4.6), or `&h + 1`/`&f`.
   - **Initial values** (question 84): by linkage and language, as listed there.
3. **VIR.** A global is not a program-level `Var`: `Var::id` is unique per function, and `var_names`, `key_of_cell` and
   the Sequencer's keys rely on it. Instead:
   - VIR has a `vir::Global` (id, name, type, initial value or unknown, the owning function for a `static` local), and one
     new expression kind, `GlobalAddr`, which lowers to `loc(GB_g(),0)`;
   - a read is `Deref(GlobalAddr)`, a write `Store`/`StoreE`, and an atom `Atom(a = GlobalAddr)`;
   - `Program::entry` is the global-namespace `main`, set by the encoder (`is_main` matches `ns::main` too).
4. **The prelude and `program_start`.**
   - The prelude: `unique function GB_<name>__<id>(): Block` inside `domain Block`. With no globals, the prelude's text is
     unchanged byte for byte.
   - `program_start()` is a trusted bodyless method (`// cerastes:trusted-contract`). For each global and `static` local
     it ensures:
     - `acc` of the value and flag cells, with the flag `true`;
     - the value (question 84);
     - full `acc` of the liveness cell.
   - Each row is positioned at its global's declaration (the position map needs a location for every row).
   - `main` calls it first. A contract on `main` and a call of `main` stay rejected.
   - **The global facts:** every method of a unit with globals (`f__wf` included) takes the pure `requires`
     `len(GB_g()) == 1 && lvloc(GB_g()) == loc(GB_g(),-2)` for each global; `main` gets them from `program_start`.
     - Without them, `&g` passed to a callee's `WF` precondition fails Silicon's own check (exit 3), and `one_past_end`
       gives false alarms.
     - They are true in 0028 item 2's model, so they are hypotheses callers discharge from their own, never axioms.
   - **The lint:**
     - pins `program_start`'s contract shape as `lint_allocations` pins `alloc_<t>`;
     - admits `unique function GB_…(): Block` only inside `domain Block`;
     - S31 re-runs the vacuity check on the emitted shape (the fixtures used a separate `domain Globals`).
5. **Initializers.**
   - `bool`, integer and enumeration initializers are read and checked by 0017's reader and prologue checks, kept in a
     list of their own (`NamedConst` holds only `Bool` and `Int`).
   - A pointer initializer is read by a new reader:
     - `ck_address` of `abk_variable` at offset 0 whose target is an accepted global becomes `GlobalAddr`;
     - a null constant becomes `null`;
     - everything else is rejected.
   - 0017 item 9's `maybe_perturb` hook covers the new list, or excludes it explicitly.
6. **In code.**
   - `g` is a cell with 0032 item 6's checks. Without permission the error is `permission`, e.g. "reads `g` without owning
     it; its function needs `reads(g)` or `owns(g)` in its contract".
   - `&g` is `GlobalAddr`: no allocation and no scope exit.
   - **Liveness:** in a unit with globals, the liveness check gains the disjunct `block_of(p) == GB_g()` for each global.
     This is sound because no operation consumes a `GB_` liveness cell. It removes the false alarms on `&g == p` and on
     `if (gp)` in a function that does not hold the target. S31 measures its cost. The fallback, if it is slow, is an
     exemption for the operand `&g` only, with `gp` documented.
7. **Contracts.**
   - `owns(g)` and `reads(g)` in both spellings.
   - Reads of `g` framed by an earlier atom (0037 item 7); `old(g)`; `decreases(g)` where `decreases(*p)` is accepted.
     Globals are routed before `variable()`'s contract refusal (`encode.cpp:1557`).
   - `owns(*gp)`, a place reached through a heap read, stays rejected (M4.5c, question 74).
8. **Calls and returns.** A global atom is a site keyed by a positive key (for example `(1 << 20) + id`; `< 0` already means
   "result").
   - Its receiver is `loc(GB_g(),0)`, with no argument.
   - The explicit checks, ghost amounts, separation against `*p` atoms of its field (`f(&g)` with `owns(*p) && owns(g)` is
     `permission`), return checks and `f__wf` ghosts are 0037's, keyed by the site.
   - Two global atoms need no case split.
   - Messages spell the place `g`, not `*…`.
   - Atom pairing (`encode.cpp:2393, 2404`; `check.cpp:358-385`) is by site key, not by `a->index`.
9. **Frames and loops.**
   - A callee whose contract does not name `g` cannot change it (0028 item 9).
   - In loops, every global the function holds is an automatic place: from its `requires` atoms, and for `main` from
     `program_start`. Its value is kept unless a write in the loop can reach it.
   - A written `owns(g)`/`reads(g)` in an invariant is rejected (it is automatic).
10. **Sequencing** (0039; 0025 item 2 revisited).
    - The Sequencer (`check.cpp:1188-1228`) gets a third key range for globals, decoded to the name, with `*&g` read as
      `g`. Otherwise lowering's same-key skip (`lower.cpp:1556`) would let `g++ + g` through.
    - Same-object shapes are rejected like the compilers (P63's matrix).
    - `g++ + *p` is path-precise `unsequenced`.
    - A callee with a global atom is a memory call.
11. **`static` locals** (question 80).
    - A `GB_` block each, with a value from `program_start`.
    - The implicit `owns(s)` is appended after atom numbering (`amount = nreq + i`) to `f`, and by reverse reachability
      over the encoder's call edges (`edges_`, after the `encode_body` loop; no SCC needed) to every caller.
      - `main` is skipped.
      - So is the `ensures` of a `noreturn` function.
    - Messages name "the `static` local `s` of `f`".
    - `return &s;` is accepted, but the caller has no atom on `*r` (d2), so its uses fail `lifetime` unless the liveness
      disjunct covers them (item 6: it does).
    - Rejected:
      - a C++ dynamic initializer;
      - `thread_local`;
      - a `static` local in a `pure`/`predicate` function.
12. **Categories:** none new; `permission` and `unsequenced` gain messages.
13. **Still rejected:**
    - b2b's markers;
    - places through a pointer global (c);
    - `owning` on globals (d2);
    - `&K` of `const` objects (M4.6);
    - arrays and structs as globals (e1, c);
    - several translation units (M8).

## Team (disjoint files; three Sonnet agents and one Opus reviewer from `.claude/agents/`)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | **First, P63:** the IL in both modes of mutable globals (internal and external linkage, tentative, `extern` plus a later definition, namespace scope), pointer initializers (`&h` as `ck_address`/`abk_variable`, NULL), `&g`, `g`, `*&g` in code and in contracts, `cerastes_owns(g)`/`cerastes::owns(g)`, `old(g)`, and `static` locals (storage class, initializer kinds, C++ dynamic initialization). Plus the clang/GCC matrix of `g++ + g`-style shapes (both modes, three tiers). **Then S31** (Silicon, hand-written targets by test-mutator): (a) the emitted prelude with `unique` `GB_` functions, `program_start` and the global facts, with the vacuity run; (b) global atoms at calls, returns and `f__wf`, and separation against `*p`; (c) implicit `static` atoms through a recursive cycle; (d) global places in `main`'s loops; (e) the cost of the liveness disjunct. **Then `src/vir/lower.cpp`** (the critical path): first the site-key refactor with no behaviour change; then globals (places, sites, `program_start`, the facts, liveness, loops, keys) in `lower.cpp` and `src/viper/` (`prelude.cpp`, `emit.cpp`). Then the baseline Viper diff. |
| **edg-backend** (Sonnet) | **First, `src/vir/vir.h`** from the record (`Global`, `GlobalAddr`, `Program::entry`). Then `vir.cpp` and `check.cpp`: `AddrOf`/`Deref`/`Atom` of `GlobalAddr`, `atoms()` pairing, the Sequencer's key range, and an initializer check context allowing `Null`/`GlobalAddr`. Then `src/backend/encode.cpp`: `encode_named_variable`'s mutable branch, the address-constant reader, `variable`/`lvalue_var`/`address_of`/`assign_target`, `spec_atom`/`old`/`contract_read`/`invariant_atom`, pairing at :2393/:2404, `static` locals and their propagation over `edges_`, `Program::entry`. Also `diagnostics.def`, and the messages of the pins that stay rejected. |
| **test-mutator** (Sonnet) | **First,** the S31 targets. Then the tests and twins (below), written from the record before the code runs. Also: the flips; the two ports; the VIR self-test `selftest_cases12.inc` (`VIR_CASES_MIN` in `check.sh`); `lint_viper.py` (the `GB_` allow-list, the `program_start` pin) with its self-test; the doc example files. |
| **soundness-auditor** (Opus, read-only) | **Round 1:** record 0041. **Round 2:** the code diff and tests, especially `program_start` and the global facts (vacuity), the liveness disjunct, global sites at calls and returns, the Sequencer's keys, the `static` propagation, and question 84's linkage rule. **One re-audit** of the blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Step 1; record 0041 and its amendments; `doc/`; `internal/encoding.md`; `il-inventory.md` (P63, S31); integration; serialized builds; the baseline; status and handoff. Stops and asks the owner. |

Any question from an agent, or of mine, stops that work and goes to the owner.

**Code sites** (this planning's survey and design check):
- **The site-key refactor:**
  - `lower.cpp` `site_param` :1232. Sites that index formals or arguments: :1268, :1279, :1332-1336, :1354, :2105-2118,
    :2189, :2355-2359, :3487, :3500/:3521, :3549, :3824-3836, :4184-4202, :4890. Messages: :1317, :3552, :3585, :3833,
    :4812.
  - Key comparisons that stay as they are: :1277, :1312, :2159, :3486-3529, :3810, :4188, :4884, :4900.
- **Globals:**
  - `encode.cpp`: :1543-1586, :1673-1769 (the `ck_integer`-only reader at :1721, `constant_value` :1164-1187), :2044,
    :2098-2172, :2190, :2298, :2340, :2393, :2404, :2598-2633, :3120-3130, :3207-3224, :511, :6032;
  - `check.cpp`: :303, :358-385, :707, :779-814, :1188-1228;
  - `lower.cpp`: `Place` :700, :1085-1109, :1427-1442, :1556, :1574-1582, :3802-3877, :4528-4622, `alloc_methods` :501;
  - `prelude.cpp` :134-160; `emit.cpp` :226;
  - `lint_viper.py` :507-614, :901-907.
- **`static` locals:** `encode.cpp:3014-3020`, :3632, `edges_` :166/:2568/:6075-6103, the `encode_body` loop :6047,
  numbering :2391.

## Tests (`.c` in both modes where valid; each positive has a twin failing for one reason)

| Positive | Twin (expected) |
|---|---|
| `static int g = 3;` read and written by `main` through a callee that `owns(g)` (`check` after) | the callee without `owns(g)` → `permission`; `check(g == 3)` after the call → `check` |
| a `reads(g)` callee; a call that does not name `g` keeps it (frame) | a callee with `owns(g)` and no `ensures` about it → the caller's `check` fails |
| `old(g)` in an `ensures`; `requires(owns(g) && g < 100)`; `decreases(g)` | `ensures(g == old(g))` with `g++` → `postcondition`; a read of `g` before its atom → `permission` |
| `int *p = &g; *p = 5;` while owning `g`; `&g != &h` and `&g == p` with no permission (the liveness disjunct) | `f(&g)` with `owns(*p) && owns(g)` → `permission` (the aliasing trap) |
| a pointer global `static int *gp = &h;` (`v_ptr_int`, `WF`), `if (gp) *gp = 1` while owning `gp` and `h` | `owns(*gp)` in a contract → rejected (M4.5c); `*gp` without owning `h` → `permission` |
| a loop in `main` over a global (value kept); a loop with `*p = …` of its field and an invariant | the same with no invariant → `check` |
| `g++ + *p` with `p != &g` known | `p == &g` possible → `unsequenced`; `g + f()` with `f` owning `g` → `unsequenced`; `g++ + g` → `unsequenced_modification` |
| external linkage in C (`int g = 0;` read by `main`: known) | in C++ (`int g = 0;` external): `check(g == 0)` in `main` → `check` (question 84) |
| a `static` counter in `f` with a bound, called from `main` and from a recursive `r` | `count++` without a bound → `overflow`; `f() + f()` → `unsequenced`; `h() + k()` both reaching `f` → `unsequenced` |
| — | rejected: `volatile`, `_Thread_local`, array, `extern`-only and struct globals; `&K` of a `const` global (M4.6); a C++ dynamic initializer; a `static` local in a `pure` function; a written `owns(g)` in an invariant |
| ports: `tests/memory/verifast/global_points_to_syntax.c`, `tests/memory/verifast/globals.c` (scalar part) | a wrong `+ 5` → `postcondition`; no `owns(x)` → `permission`; `check(x0 == 1)` → `check` |

**Flips** (pre-approved, question 81): the three retirements and the three message/`Pins:` updates. No other existing
test's outcome may change.

**The Viper of every existing program may not change**, `.vpr` and `.map` alike. No program accepted today has a mutable
global or a `static` local, and the site-key refactor changes no behaviour. The only diffs allowed are the flipped pins and
added files.

## Docs (invariant 8; every example is a test)

- **Memory and ownership:**
  - "Global variables": blocks, `program_start`, contracts and frames, `&g`, linkage, loops, sequencing;
  - "`static` local variables": the implicit ownership, the unsequenced calls, `return &s`;
  - "What is not supported yet".
- **Annotations:** `owns(g)`/`reads(g)` in the spec-function table.
- **Error messages:** the new `permission` and `unsequenced` messages.
- **Verifying a program:** :620 and :786 (globals and `static` locals are no longer rejected).
- **Overview:** :63.
- **`internal/encoding.md`:** §3 gains a globals paragraph after :416; "Memory prelude" (:461) gains `GB_`,
  `program_start` and the global facts.

## Order of work

1. Step 1, setup and the baseline. In parallel: P63; the record draft (audit round 1 in the background); `vir.h`.
2. **Commit 2:** the site-key refactor alone, after every kept `.vpr`/`.map` is regenerated byte-identical to the
   baseline.
3. S31, then globals: lowering, encoder, checks, tests, ports, VIR self-test. **Commit 3** once they pass, together with the
   baseline diff.
4. `static` locals. **Commit 4.**
5. End to end, then the full regeneration against the baseline.
6. Audit round 2, fixes, one re-audit.
7. Docs; `check.sh` (full, at most twice); status and handoff; the last commit.

## Stop and ask

- Any question from an agent.
- The refactor commit changing any Viper byte.
- An existing test's outcome or Viper changing outside the flips.
- P63 contradicting the record. For example, `&g` or an `&h` initializer in a shape the reader cannot tell apart, or
  linkage not readable from the IL.
- S31 contradicting items 4–11 with no fix inside the record. This includes:
  - a vacuity run passing;
  - Silicon failing first with its own reason;
  - the liveness disjunct over 2× S28's times.
- Exit 3 on a correct program; a new test over 60 s.
- Context running short after commit 3: offer to leave `static` locals rejected and move them to b2b. Then
  `reject_static_local.c` keeps its `expect:` line.

## Acceptance (commands run at the end)

- `python3 scripts/run_tests.py tests/memory tests/scalar tests/viper-smoke/memory doc/modules/ROOT/examples/memory
  doc/modules/ROOT/examples/error-messages doc/modules/ROOT/examples/verifying doc/modules/ROOT/examples/annotations`:
  all pass.
- Every kept `.vpr`/`.map` regenerated with the final `build/cerastes` is byte-identical to `build/m4.5b2a-baseline-vpr`.
  After `check.sh`, `diff -r -x MANIFEST build/m4.5b2a-baseline-vpr build/vpr` shows only added files and the flipped
  pins. The VIR self-test against `build/m4.5b2a-baseline-vir` shows only added cases.
- `python3 scripts/test_run_tests.py`, `python3 scripts/test_lint_viper.py` and `python3 scripts/run_tests.py
  --check-headers`: green.
- `scripts/build_docs.sh` and `python3 scripts/check_doc_examples.py --html build/doc`: green.
- `scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): green. There is no Linux run; CI covers it, and any x86-64 run
  goes to `matt@clang4.cpp.al`.
- **Status entry:** record 0041, the flips and the audit outcome, the plan file, and the first step for M4.5b2b (plan it
  from handoff §4.2). The b1c3 entry moves to a history row. Handoff §3 gets what was not taken, and §8 any new question.
- **Commits:** planning; refactor; globals; `static` locals; status and handoff. Local only, no Claude trailer, never
  pushed.
