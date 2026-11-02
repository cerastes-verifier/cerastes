# Plan: M4.5b2b — initialization through pointers (record 0042)

Approved by the owner on 2026-11-02 (planning in plan mode; questions 85-88 answered with the recommendations). Branch
`m4.5b2b-init-markers`, worktree `.claude/worktrees/m4.5b2b`; the baseline is taken in the detached worktree
`.claude/worktrees/m4.5b2b-base` (`38877b4`), not in the main checkout.

## Context

`origin/main` and the local `main` are both at `38877b4`, and the main checkout is clean. That includes M4.5b1c3 (PR #21)
and M4.5b2a, globals and `static` locals (PR #22, record 0041). The session worktree `.claude/worktrees/m4.2b1` no longer
exists.

The next row of `plan.md` is **M4.5b2b**: P4222's markers in the `cerastes::` spelling and the opt-in `--strict-init`. The
inputs are:
- records 0019 item 2, 0028 item 14 and 0029 item 2;
- P54;
- handoff §4.2, the b2b paragraph;
- owner questions 82 and 83.

No marker is registered today. `check_attributes` (`encode.cpp:511`) skips every registered `cerastes` attribute without a
word, so each marker is registered in the same commit that handles it.

Size estimate: about 600–900 changed lines plus about 50 tests. That fits one context (question 77).

**What lands:**
- **`[[cerastes::uninit]]`** on an automatic local without an initializer. It documents intent; 0019 already proves every
  read, so the Viper is unchanged.
- **`[[cerastes::ref_to_uninit]]` and `[[cerastes::must_init]]`** on pointer parameters:
  - on entry, the pointee's flag is not required;
  - `must_init` promises it on return;
  - `ref_to_uninit` keeps an initialized pointee initialized (question 87).
- **The call's flag row becomes `uninitialized`** for every atom (questions 82 and 85).
- **`--strict-init`:**
  - every local without an initializer must be marked `uninit` (`uninit_unmarked`);
  - an argument to a marked parameter must be `&x` of an `uninit` local, or the caller's own marked parameter passed
    unchanged (`uninit_ref_initialized`, question 83);
  - tests pass the option through a new `// verify-options:` header.

## Owner decisions

**From this planning (2026-11-02; each was the recommendation):**
- **85, which flag rows move.** Every call flag row becomes `uninitialized`, including global and `static`-local atoms;
  their message names the global. The pre-approved flips grow to cover `selftest_cases12.inc:113, :206, :283`.
- **86, contract reads of a marked pointee** are rejected statically (`unsupported_annotation`, each with its own
  message). This covers:
  - `*p` in a `requires`;
  - `old(*p)`;
  - `decreases(*p)`;
  - `*p` in the `ensures` of a `ref_to_uninit`-only parameter.

  `*p` in the `ensures` of a `must_init` parameter stays accepted.
- **87, after a `ref_to_uninit` call** an initialized pointee stays initialized: the `ensures` states `old(flag) ==> flag`.
  This amends 0028 item 14. The callee's row for it is `internal`, since no verified operation makes an object
  indeterminate again.
- **88, a marker needs an `owns(*p)` atom.** It must be in the `requires`, explicit or from `cerastes::owning`; `must_init`
  needs one in the `ensures` too. A marker with no atom, or beside `reads(*p)`, is rejected. Ownership and initialization
  stay orthogonal.

**During the session (2026-11-02, after audit round 2):** **89**, the loop row `held && flag ==> range` on every place a
loop does not reach, in every function (record 0042 item 5): it changes the Viper of existing programs with such loops
(added invariant lines, no verdict) and fixes 0038's exit 3 on nested loops; re-expecting a VIR self-test pin is
pre-approved only where its row count at a loop grows by exactly the added rows.

**Standing:**
- **82:** the call's flag row becomes `uninitialized`.
- **83:** strict mode's argument rule is static.
- **77:** a session fits one context.
- **69:** the EDG merge waits for the box session.

## Step 1 (first, before any code): consolidate the plan documents

The first commit is markdown only. It keeps one place for each live fact, and only what later sessions need.

1. **Worktree and branch.**
   - `git worktree add .claude/worktrees/m4.5b2b -b m4.5b2b-init-markers origin/main` (`38877b4`).
   - Submodules with `--reference` to the main checkout's module stores (memory `background-worktree-setup`).
   - Then `scripts/bootstrap.sh`.
2. **The plan.** Save it as `.claude/plans/m4-5b2b-init-markers.md`, with the "Inputs from the handoff" section below.
3. **`internal/handoff.md`**, consolidated:
   - **Header:** one pointer to the text before, `git show 38877b4:internal/handoff.md`.
   - **§1–§2 rewritten.** §1: everything through b2a is on `origin/main` `38877b4`; b2b is on its branch in the new
     worktree; the pin is unchanged. §2 lists no owner step until the session ends.
   - **§3 regrouped by the session that consumes each note**, one line per note with its origin (for example "b2a N3"):
     M4.5c, d1, d2, e1, f1, M4.6, M4.7, M4.8, M5.0/M5, the M8 warning sweep and over-strict listings, and "standing"
     (by design, messages, tooling, fail-closed corners).
     - Notes that b2b consumes move into the plan's Inputs.
     - The audit-round narratives are dropped; they are in `status.md` and the records.
   - **§4 rules:** 0042 is b2b's record and 0043 the next free one.
   - **§4.2:**
     - b2a becomes one "done" line;
     - b2b becomes a pointer to the plan with its flips;
     - d2 keeps `ref_to_uninit` on locals and functions;
     - M4.5c gains a note: struct copies must not clear the flag of a held cell (question 87's `internal` row relies on
       that).
   - **§6:** the worktree note now says each session creates `.claude/worktrees/<session>`, and the old `m4.2b1` is gone.
   - **§8:** questions 85–88 are answered; none is open.
4. **`plan.md`:**
   - the Done table gains the M4.5b2a row;
   - the M4.5 table loses the b2a row, and the b2b row names the plan file;
   - the header's "the last: M4.5b2a" becomes M4.5b2b.
5. **Commit 1 (planning):** local, with no Claude trailer (CLAUDE.md overrides the harness default).

## Inputs from the handoff (moved into the plan at step 1)

- **The registration rule** (0029 item 1, M4.5a note): register only what the session handles, in the commit that handles
  it.
- **P54 (`il-inventory.md:4787`).** What registration does:
  - `uninit` is registered `v:+a!`. EDG refuses it on a global or `static` local (#1862) and on a parameter, field or
    function (#1835).
  - `ref_to_uninit` and `must_init` are registered `p` with `RAF_NO_FLAGS`. `RAF_ENTITY_ATTACHED` with `p` alone aborts EDG
    at startup, as it did for `owning`.
  - Attributes accumulate over redeclarations, and none is must-match.
  - Registration accepts a non-pointer parameter and a duplicate; the encoder rejects them.
  - Both markers on one parameter are recorded by EDG; the encoder rejects that.
- **Question 82's pins** (the survey of this planning). Only call rows move. The loop and return pins (cases11 :232, :235,
  :287, :288, :380, :401, :423) count `written_checks` or `atom_returns` rows and stay as they are.

  | Pin | `permission` rows now | After the change |
  |---|---|---|
  | `selftest_cases8.inc:73` | 11, index 3 | 10, index 3 |
  | `selftest_cases11.inc:137` | 17, index 2 | 16, index 2 |
  | `selftest_cases11.inc:138` | 17, index 3 | 16, index 3 |
  | `selftest_cases12.inc:113` | 11, index 3 | 10, index 3 |
  | `selftest_cases12.inc:206` | 23, index 16 | 21, index 14 |
  | `selftest_cases12.inc:283` | 11, index 3 | 10, index 3 |
- **The `.map`:** the category and message of a row live only in the `.map` (`emit.cpp:93-113`, `posmap.cpp`). So
  question 82 changes one row per atom in the `.map` of every program that calls an `owns`/`reads` function, and never
  the `.vpr`. No `.map` is tracked in git.
- **No test expects the flag row:** this planning's scan found no test under `tests/` or `doc/` whose expected
  `permission` error comes from an uninitialized argument. So no `expect:` line moves.

## Setup (in parallel with step 1)

- **Baseline that agents cannot disturb.** In the detached worktree `.claude/worktrees/m4.5b2b-base` (`38877b4`; the main
  checkout is left alone), in the background:
  1. `bootstrap.sh`;
  2. `run_tests.py -j 8`;
  3. the VIR self-test (the generated `.vpr`/`.map` files);
  4. copy the results into the worktree's `build/m4.5b2b-baseline-{vpr,vir}`.
- **Worktree builds:** `lockf build/m4.5b2b/build.lock cmake --build build/cmake`.

## Record 0042 — initialization through pointers

The record is written before code (invariant 6), while P64 runs. Audit round 1 runs on it in the background.

1. **Scope table** (b2b | d2 | M4.6 | M7 | M8). It amends:
   - 0019 items 2–3: the strict mode;
   - 0028 item 14: question 87;
   - 0029 item 2: the targets, categories and messages fixed;
   - 0037 items 8 and 10: the flag rows at calls and returns;
   - 0040 item 3: `annotation_mismatch` names the attribute, and `owning` joins the protected macro names.
2. **Registration and macros.**
   - The registration rows (in `register_attributes.cpp:58`):
     - `{"uninit","","v:+a!",RAF_NO_FLAGS}`
     - `{"ref_to_uninit","","p",RAF_NO_FLAGS}`
     - `{"must_init","","p",RAF_NO_FLAGS}`
   - The kinds are added to `annotation_il.h:31/:44`, with `annotation_checks.cpp:722`'s skip.
   - `<cerastes.h>` defines `CERASTES_UNINIT`, `CERASTES_REF_TO_UNINIT` and `CERASTES_MUST_INIT`, each with an empty
     fallback.
   - `is_protected_macro_name` (`annotation_checks.cpp:1003`) gains `owning`, `uninit`, `ref_to_uninit` and `must_init`.
     For `owning` this closes a gap left by b1c3: a user `#define owning` would rewrite the attribute token.
3. **`uninit` on locals** (read in `declaration()`, `encode.cpp:3532`).
   - **Accepted** on the definition of an automatic local of an accepted type (scalar, enumeration, or a pointer at any
     depth) without an initializer, including in a `for` init.
   - **Rejected:**
     - with an initializer: `uninit_initialized`. C++'s `int x{}` and `= {}` count as initializers, and a leading marker
       over `int a, b = 1;` reaches `b`.
     - twice on one declaration: `unsupported_annotation`.
   - EDG itself refuses the other targets: a global, a `static` local, a parameter, a field and a function (pinned).
   - **It emits nothing.** The `.vpr` is identical with and without the marker, and a test pins that.
4. **The parameter markers** (read like `read_owning`, `encode.cpp:624-798`; stored as `vir::Param::init {None,
   RefToUninit, MustInit}` beside `is_restrict`, `vir.h:356`; `check_program` repeats the rules as a backstop).
   - **Rejected** (`unsupported_annotation`, each with its own message):
     - a non-pointer;
     - a pointer to a function;
     - `void *`;
     - a `const` pointee;
     - the marker twice;
     - both markers on one parameter;
     - no `owns(*p)` in the `requires`;
     - `must_init` with no `owns(*p)` in the `ensures`;
     - `reads(*p)` beside a marker;
     - the contract reads of question 86.
   - **Declarations must match:** a mismatch is `annotation_mismatch`. Its text names the attribute; `owning`'s message
     stays byte-identical.
   - **Combinations:**
     - Beside `owning`: the desugared or merged atom carries the marker.
     - A `T **pp` marker covers the pointer cell `*pp`, with `WF` in place of the range.
   - **Rejections:**
     - `must_init` on a `noreturn` function gets its own rejection, mirroring `ensures_on_noreturn` (`encode.cpp:977-981`),
       with the same rule in the `check_program` backstop (pattern at `check.cpp:474`). Otherwise the "no `ensures` atom"
       rule would ask for a clause the user cannot write. `ref_to_uninit` there needs nothing.
     - When `finish_atoms` or `apply_owning` already rejected the atom or the parameter, the marker's "no `owns(*p)`"
       error is not reported a second time.
     - "Cannot tell apart" (`encode.cpp:700`) is generalised from `owning` as well.
   - **Order** (design check):
     - The markers are read in `check_signature` beside `read_owning` (`:588`), into a side table.
     - `Param::init` is set in `function_of` after `add_param` (`:877-885`) and before `encode_contract` (`:891`), so a
       written `*p` is rejected while the contract is read.
     - The atom checks (an `owns` atom present, `must_init`'s `ensures` partner, no `reads`) run after `apply_owning` and
       `finish_atoms` (`:990-991`).
5. **The callee** (`lower.cpp`).
   - **The `requires` atom of a marked parameter** has the permissions but no `cell_flag`. Its range or `WF` is stated only
     under the flag, as `flag ==> range` (`atom()`, `:1376-1385`): an uninitialized cell's value is unconstrained, since
     `alloc_<t>` gives no range. A `bool` cell has no range row (`cell_range` returns null), so nothing is emitted for it.
   - **Reads through an alias** (design check). `contract_read` (`:1475-1489`) lets any atom of the field cover a read,
     such as `requires(q == p) requires(*q > 0)`, `ensures(r == p) ensures(*r == 1)`, `old(*q)` or `p == &g`. So a marked
     atom's site never covers a read:
     - for pre-state reads, always;
     - for post-state reads, when the parameter is `ref_to_uninit` only.

     Such a read then fails `f__wf`'s framing (`permission`, per path). The encoder's check on the spelling `*p` stays,
     for its clearer messages.
   - **The body:** a read checks the flag (`uninitialized`, path by path), and a write sets it. Loops need no change: the
     automatic places already snapshot each flag (`:4385-4435`).
   - **Returns** (`atom_returns`, `:3679-3755`):
     - `must_init`: the flag row becomes `postcondition`, "returns without initializing `*p`, which `cerastes::must_init`
       on `p` promises".
     - `ref_to_uninit`: the flag row becomes `(gp && old(flag)) ==> flag` (`internal`), and the range row `flag ==>
       range`.
     - `gp` is the guard of the `requires` partner. `old(p.d_T)` has permission only where `gp` holds, and `f__wf` reports
       an `ensures` guard that does not imply it (`:4999-5010`; 0037 item 7: Silicon then reports nothing else).
   - **The method's `ensures`** states the same facts.
   - **What the `internal` row relies on:** nothing clears the flag of a cell the function holds. Today `lint_flags`
     (`lint_viper.py:1301`) and the loop snapshots (`:4398`, `:4434`) guarantee that. M4.5c's struct copies must keep it
     true, and the handoff's M4.5c paragraph gets that note.
   - **Amended text:** 0028 item 14 ("not its value"), and the `maybe_init` comment in
     `tests/viper-smoke/memory/s25_d_init.vpr`. The comment is not an `expect:` line, and the record names the change.
6. **The caller** (`call_atoms`, `:2225-2384`).
   - **A marked parameter's atom:** there is no flag row, and its range row is `flag ==> range`.
   - **Every other cell atom:** the flag row (`:2286`) is `uninitialized` (questions 82 and 85). It needs its own
     `obl(…, meta("uninitialized", …))`, because `add` (`:2263-2265`) always writes `permission`. Its wording comes from
     `site_phrase` (`:1308`), so implicit atoms read right. The messages:
     - for a parameter: "passes `&x` to `f`, whose parameter `p` needs `*p` to have a value; it may not have one here";
     - for a global or `static` local: "calls `f`, which needs `g` to have a value".
   - **After the call:** the `ensures` is inhaled. An initialized pointee stays initialized after `ref_to_uninit`
     (question 87); after `must_init` it is initialized.
7. **`--strict-init`** (`VerifyConfig::strict_init`, `verify_config.h:25`; the option and usage in `verify_main.cpp:50-179`,
   `:389`).
   - Every local definition without an initializer must carry `uninit`, or it is rejected `uninit_unmarked`.
     - The check sits after the `static`-local `continue` in `declaration()` (`:3534-3537`).
     - EDG refuses `uninit` on a `static` local, so `static int s;` must not be asked for it.
   - Every argument to a marked parameter must be one of:
     - `&x` of an `uninit` local;
     - the caller's own marked parameter, passed as its name, when that parameter is never assigned and never
       address-taken.

     Anything else is rejected statically as `uninit_ref_initialized` (question 83). That includes a null constant and a
     pointer local holding `&x`; both are listed as over-strict.
   - **Where the check runs:** on the VIR argument in `call()` (`encode.cpp:3076-3088`), so C23 and C++20 look the same.
     - The accepted forms are an `AddrOf` of an `uninit` local, or `param_var(k)` of the caller's marked parameter.
     - "Never assigned" is not known at the call. Pending checks are settled after the body, next to
       `report_goto_problems` (`:5109`).
     - For that, lowering's `scan`/`scan_expr` (`lower.cpp:3783-3800`, which feeds `assigned_params`) moves into
       `vir.cpp`, so the encoder and lowering agree.
     - "Address-taken" is the parameter's `address_taken`.
8. **Categories:** none new.
   - `uninitialized` gains the argument message, and `postcondition` gains `must_init`'s.
   - New rejection tags: `uninit_initialized`, `uninit_unmarked` and `uninit_ref_initialized`.
9. **Listed, not changed:**
   - A written `owns(*p)` in a loop invariant still includes the flag. So `invariant(owns(*p))` before the first write is
     reported "`permission`: enters this loop without owning `*p`".
   - The flag rows of `written_checks` (`:4237`) and of unmarked returns stay `permission`; they cannot fail on a
     parameter's pointee.
   - `ref_to_uninit` on locals and functions (targets `v`, `r`) is M4.5d2's.
10. **Facts relied on:** P64 (P54 re-run on the real registration) and S32.
11. **Tests and documentation** (below).

## Team (disjoint files; three Sonnet agents and one Opus reviewer from `.claude/agents/`)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | **First, P64:** P54 re-run once the registration rows are in the worktree build, in both modes: positions (leading and trailing), accumulation and typedef'd function types, macros, EDG's refusals and their numbers, the `for` init, `int a, b = 1;`, C++ `{}` initializers, and the IL of `&x` arguments and of a parameter passed on unchanged. **Then S32:** test-mutator's hand-written fixtures through Silicon, covering the flag-free `requires`, `flag ==> range`, `old(flag) ==> flag`, `must_init`'s `postcondition` row, and a vacuity run. **Then** `src/vir/lower.cpp` and `src/viper/` (items 5–6, question 82's category), and the `.map` comparison against the baseline. |
| **edg-backend** (Sonnet) | **First:** `vir.h` (`Param::init`), the registration rows, `annotation_il.h`, `annotation_checks.cpp` and `<cerastes.h>`. Also lowering's `scan`/`scan_expr` moved into `vir.cpp`; il-analyst switches `lower.cpp` to it, with no behaviour change. **Then:** `check.cpp`'s backstop; `encode.cpp` (`read_markers` from `read_owning`, the marker on atoms after `apply_owning`, the rejections, `uninit` in `declaration()`, the strict checks at calls and declarations); `diagnostics.def`; `verify_config.h` and `verify_main.cpp`. |
| **test-mutator** (Sonnet) | **First:** the S32 fixtures (`tests/viper-smoke/memory/s32_*`), and the `// verify-options:` header (only `--strict-init` is allowed). The header touches `run_tests.py` (an `Expectation` field, `verify_command`), `mutate.py` (passes the options), `test_run_tests.py` self-tests that fail if the header is weakened, and `tests/README.md`. **Then:** the tests and twins below, written from the record before the code runs; the VIR self-test `selftest_cases13.inc` and the re-expected pins (Inputs), with `VIR_CASES_MIN`; any lint change with its self-test; the doc example files. |
| **soundness-auditor** (Opus, read-only) | **Round 1:** record 0042. **Round 2:** the code and the tests, especially the flag-free `requires`, `old(flag) ==> flag`, the call rows of marked parameters, the strict checks and the contract-read rejections. **One re-audit** of the blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Step 1; record 0042 and its amendments; `doc/`; `internal/encoding.md`; `il-inventory.md` (P64, S32); integration; serialized builds; the baseline; status and handoff. Stops and asks the owner. |

Any question from an agent, or of mine, stops that work and goes to the owner.

## Tests (`.c` in both modes where valid; each positive has a twin failing for one reason at one line)

| Positive | Twin (expected) |
|---|---|
| `init(int *p CERASTES_MUST_INIT)` with `owns(*p)` in both lists and `*p == 0` ensured; the caller `int x; init(&x); return x;` | a path of `init` that does not write → `postcondition` at its return |
| a `ref_to_uninit` callee that writes `*p` and then reads it | it reads `*p` first → `uninitialized` |
| an unmarked `owns(*p)` or `reads(*p)` callee given an initialized `&x` | `int x; set(&x);` → `uninitialized` at the argument (question 82) |
| `int x = 1; maybe(&x); return x;` with a `ref_to_uninit`-only callee (question 87) | `int x; maybe(&x); return x;` → `uninitialized` at `return` |
| `owning` plus `must_init`: `if (p) *p = 0;` | the write is missing → `postcondition` |
| `T**`: `make(int **pp CERASTES_MUST_INIT)` writes `*pp`; the caller uses `q` after `make(&q)` | the write is missing → `postcondition` |
| a `must_init` parameter passed on to another `must_init` callee | passed to an unmarked callee → `uninitialized` at the argument |
| a loop in a `must_init` callee, with the write before the loop | the write only inside a loop that may not run → `postcondition` |
| the C++ spellings, trailing position, and a prototype plus a definition both marked | the definition is unmarked → `rejected annotation_mismatch` |
| `CERASTES_UNINIT int x;` on paths (the same `.vpr` as unmarked); `for (CERASTES_UNINIT int i; …)`; a leading marker on a declaration statement never reaches `annotation_statement` | with an initializer → `rejected uninit_initialized`; twice → `rejected unsupported_annotation` |
| a `ref_to_uninit` callee whose contract has no read of the pointee | reads through an alias, `requires(q == p) requires(*q > 0)` and `ensures(r == p) ensures(*r == 1)` → `permission` in the contract (the marked site never covers a read) |
| `ref_to_uninit` on a `noreturn` function | `must_init` on a `noreturn` function → rejected |
| strict mode with a `static int s;` local (not asked for `uninit`) | — |
| `// verify-options: --strict-init`: every local marked, `init(&x)` of an `uninit` local, a marked parameter passed on | an unmarked local → `uninit_unmarked`; `int x = 0; init(&x);`, `init(q)` with `q = &x`, `init(NULL)` and a reassigned parameter → `uninit_ref_initialized`; the same programs without the option verify |
| — | rejected: a non-pointer, `void *`, a `const` pointee, a pointer to function, a marker twice, both markers, no `owns` atom, `must_init` without an `ensures` atom, `reads(*p)` beside a marker, `*p` in a `requires`, `old(*p)`, `decreases(*p)`, `*p` in a `ref_to_uninit` `ensures`; EDG's own refusals (`uninit` on a global, `static` local or parameter; `ref_to_uninit` and `must_init` on a local or function); `#define owning` / `#define must_init` → `annotation_macro_changed` |

**Flips** (pre-approved): only the six VIR self-test pins listed in Inputs (questions 82 and 85). No test's `expect:` line
changes.

**Viper against the baseline:**
- Every existing `.vpr` is byte-identical.
- Each `.map` differs only in the call flag rows' category (`permission` → `uninitialized`) and message. A comparison
  script, `build/m4.5b2b/mapdiff.py`, checks exactly that.
- The VIR self-test shows only added cases and the six re-expected pins.

## Docs (invariant 8; every example is a test)

- **Memory and ownership:**
  - a new section, "Initialization through pointers", after "Pointer parameters and contracts" (`memory.adoc:130`): the
    three markers, what is checked, the caller's view and the strict mode;
  - the not-yet list (`:453`): `ref_to_uninit` on locals and functions, M4.5d2.
- **Annotations:**
  - a section "Initialization markers" after "Owning parameters" (`annotations.adoc:262`), covering the markers and macros;
  - "Declarations that disagree" (`:853`) gains the markers;
  - "Changing the cerastes macro" (`:923`) gains the newly protected names.
- **Error messages:**
  - `uninitialized` (`:339`) gains the argument message, with an example;
  - `postcondition` gains `must_init`'s;
  - the new rejections.
- **Verifying a program:** the options table (`:42-68`) gains `--strict-init`; "Locals without an initializer"
  (`:271-303`) gains the marker and the mode.
- **`internal/encoding.md`:** the markers' encoding.

## Order of work

1. Step 1, the worktree and the baseline. In parallel:
   - the record draft, with audit round 1 in the background;
   - edg-backend's first block;
   - the S32 fixtures and the harness header;
   - P64, as soon as the registration builds.
2. S32. Then the encoder, lowering and question 82. Then the tests and the VIR self-test.
   - **Commit 2** (markers and question 82), together with the baseline comparison.
3. Strict mode, end to end. **Commit 3.**
4. The full regeneration against the baseline. Then audit round 2, the fixes and one re-audit.
5. Docs; `check.sh` (full, at most twice); status and handoff; the last commit.

## Stop and ask

- Any question from an agent.
- An existing test's outcome changing, any `.vpr` changing, or a `.map` change outside the call flag rows.
- P64 contradicting P54: the registration aborting EDG, a position the IL does not keep, or `&x` arguments not
  recognisable in one mode.
- S32: a vacuity run that passes, or Silicon failing first with its own reason.
- Exit 3 on a correct program, or a new test over 60 s.
- Context running short after commit 2: offer to move strict mode and its header to a session of its own.

## Acceptance (commands run at the end)

- `python3 scripts/run_tests.py tests/memory tests/scalar tests/viper-smoke/memory doc/modules/ROOT/examples/memory
  doc/modules/ROOT/examples/error-messages doc/modules/ROOT/examples/verifying doc/modules/ROOT/examples/annotations`:
  all pass.
- `build/m4.5b2b/mapdiff.py build/m4.5b2b-baseline-vpr build/vpr`: `.vpr` files identical, `.map` files differing only in
  call flag rows, the rest added files. The VIR self-test against `build/m4.5b2b-baseline-vir` shows only added cases and
  the six re-expected pins.
- `python3 scripts/test_run_tests.py`, `python3 scripts/test_lint_viper.py` and `python3 scripts/run_tests.py
  --check-headers`: green.
- `scripts/build_docs.sh` and `python3 scripts/check_doc_examples.py --html build/doc`: green.
- `scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): green. There is no Linux run; CI covers it, and any x86-64 run
  goes to `matt@clang4.cpp.al`.
- **Status entry:** record 0042, the flips, the audit outcome, the plan file, and the first step of the next session
  (M4.5c, planned from handoff §4.2). The b2a entry becomes a history row. Handoff §3 gets what was not taken, and §8 any
  new question.
- **Commits:** planning; markers and question 82; strict mode; status and handoff. Local only, on
  `m4.5b2b-init-markers`, no Claude trailer, never pushed. The handoff gives the owner the push and PR steps.
