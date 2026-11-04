# Plan: M4.5d1 — vlibc in cerastes (record 0046)

Approved by the owner on 2026-11-03 (planning in plan mode; questions 113–115 answered with the recommendations).
Branch `m4.5d1-vlibc`, from M4.5c2b's tip `d26a87c`. Its PR stacks on PR #26 the way c2b stacked on #25. Worktree
`.claude/worktrees/m4.5d1`; the baseline goes in the detached worktree `.claude/worktrees/m4.5d1-base` (`d26a87c`). The
first execution step saves this plan as `.claude/plans/m4-5d1-vlibc.md`.

## Context

**Where things stand.**
- `origin/main` is `37f6dda` (PR #25, M4.5c2a). M4.5c2b is PR #26 (`d26a87c`), still open; the local `main` `00fe5a2` merges
  the two.
- The EDG pin `f0ed5525af` is unchanged (question 69).

**vlibc's half of M4.5d1 is done** (`~/Documents/cerastes/vlibc` `5962865`, pushed to `cerastes-verifier/vlibc` `main`).
- It has the layout, CMake, the dual BSL-1.0 / Apache-2.0 WITH LLVM-exception license, and CI with GCC 16 and Clang 21 on
  ubuntu-26.04 and xcode-27.
- `include/vlibc/stdlib.h` is both `<vlibc/stdlib.h>` and the shim:
  - `#include_next <stdlib.h>`, then `#include <cerastes.h>`;
  - then `void abort(void) CERASTES_ENSURES(false);`, and the same for `exit` and `_Exit`;
  - each tagged `// cerastes:trusted-contract vlibc: <name> never returns (C23 7.24.4.n)`.

**This session does cerastes's half** (record 0030 items 3–5, 0031 items 9–10, handoff §4.2):
- the `third_party/vlibc` pin;
- the driver's include order;
- the library class (by path), with EDG's system-header flag audited;
- trust from vlibc's tags;
- "does not return" from vlibc;
- reserved names;
- attributes checked by where each is written (P55);
- P55's glibc half and question 59's glibc items;
- `trusted_libc_reason` retired.

The Viper of today's `abort`/`exit`/`_Exit` tests stays unchanged except for an explained tag text (`libc:` → `vlibc:`).

**What this planning's three surveys found:**
- **The `-I` question.** P55's "only a leading `--sys_include` shadows `<stdlib.h>`" came from the driver's argument order.
  EDG keeps `-I` and `--sys_include` in one ordered list (`edg/src/cmd_line.c:10840-10853`). But EDG also flags any file
  that a system header includes as system (`lexical.c:8159-8161`), and libc++'s `<cstdlib>`, `<array>` and `<exception>`
  include `<stdlib.h>`. So a `-I` shim would be system in some C++ units and not in others. **Keep 0030's
  `--sys_include`** (always system for EDG), and give cerastes a library rule by path. The record notes P55's correction.
- **One source for the flag.** Every cerastes use of EDG's flag goes through `SrcPos::in_system` (`render.cpp:130`), except
  `directive_in_system_file` (`annotation_checks.cpp:1043`, already path-checked in verify mode) and `encode.cpp:9041`.
  The users of `in_system` are:
  - `diagnostics.cpp:155`;
  - `spec_marker_in_system_file` (`annotation_checks.cpp:1126-1136`);
  - the initializer skip (`:1331`, `:1340`);
  - the unattributed-body skip (`:1591`);
  - `walker.cpp:718` (walk mode only).
- **The C++ declaration-position move.** In C++ (`gpp_mode`), EDG moves `decl_position` to a non-defining redeclaration of
  a function that has at least one parameter, has no body and is first declared in a system header
  (`decls.c:9802-9813`). So in C++, `exit` and `_Exit` are "declared here" in vlibc, while `abort(void)` and every C
  function keep the SDK's position. This is also why P55's attribute problem was C++-only.
  - Today, vlibc's `ensures(false)` on `abort` would be `ensures_on_noreturn` at the first call (`encode.cpp:1254`). In
    C++, `exit` would also go through the declaration checks (`:8905`, `:8940`).
  - Trust and checks must therefore be keyed on where the contract attributes are written, not on `decl_position`.
- **No extra methods.** The encoder emits nothing for vlibc's uncalled declarations: a routine first declared in a
  system file is skipped unless called (`encode.cpp:8869-8878`). In C++, `exit`/`_Exit` would reach the declaration
  checks but are encoded lazily. So programs that don't call the three keep byte-identical Viper.
- **Walk mode stays without vlibc.** Adding the shim to `cerastes-fe.sh` would add a "not reported" warning line per
  vlibc contract to every walker golden that reaches `<stdlib.h>` (`walker.cpp:830-841`). The shim stays verify-mode only
  (0030 item 3), so no golden changes.
- **The tests that change.**
  - 13 must-verify and 16 must-fail tests call the three. 54 kept `.vpr` files carry 62 `libc:` tags (42 `abort`, 12
    `exit`, 8 `_Exit`).
  - The lint accepts any non-empty reason (`lint_viper.py:125`, `:1527`).
  - The C++-linkage look-alikes (`noreturn_user_abort.cpp`, `noreturn_namespaced_exit.cpp`) pin "not the library's".
- **M4.5d2's size.** 14 work items and no heap code yet (no token, `owns_block`, `forperm`, `leak` or `invalid-free`).
  Comparable sessions changed 1,800–3,200 source lines. Its dependency map goes into handoff §4.2 for d2's planning.

**Size of d1:** about 600–1,000 changed lines of source and scripts, about 40 tests, and docs. One context.

## Owner decisions

**From this planning (2026-11-03; each was the recommendation):**
- **113, scope: d1 plus d2 preparation.** d2 is not rolled in. This session also measures the malloc family as each
  platform declares it (P71 (d)), and writes d2's dependency map and a proposed d2a/d2b split into handoff §4.2.
  - d2a would take the heap: vlibc's malloc family, heap blocks with the slot −1 token, `owns_block`, the free checks,
    leaks, and owning results as `owns_block`.
  - d2b would take `owns(*result)` with output ghost amounts, `live(r.q)`, owning fields, and P4418R0.
  - The P4222R3 check stays at d2's planning.
- **114, reserved names are generated from vlibc.** They are the names that the pinned headers declare with a contract,
  generated at configure time; in d1, `abort`, `exit` and `_Exit`. User contracts on other system functions keep
  today's rules.
- **115, a contract on a reserved name outside vlibc is always rejected,** even when it is identical to vlibc's:
  `annotation_reserved_library`. This amends 0030 item 4's "a user may redeclare with the same contract". A *different*
  contract with vlibc's header present is EDG's must-match error first, as today.

**During the session (2026-11-03, after audit round 1 on the record; both the recommendation):**
- **116:** an identical contract on a redeclaration *after* vlibc's header is accepted. The front end discards a later
  equivalent must-match list (`attribute.c:4199-4227`), so it leaves no trace (audit B2); question 115 holds in every
  other case.
- **117:** a definition of a reserved name, or a contract on any declaration of one, is rejected for C language linkage
  in any namespace and, in C++, for any linkage in the global namespace (C23 7.1.3, C++ `[extern.names]`/4). The four
  pins `noreturn_user_abort.cpp`, `noreturn_user_abort_after.cpp`, `noreturn_other_signature.cpp` and
  `noreturn_other_signature_after.cpp` are re-expected `rejected cerastes-annotation_reserved_library` (pre-approved).
- Audit B1 (taken, record item 8): a bodiless routine with a trusted contract is refused when its calls may reach other
  code (an asm label, `#pragma redefine_extname`, an alias, `ifunc`, `weak`/`weakref`). This hole predates vlibc.

**Standing:**
- 69: the EDG upstream merge waits for the box session.
- 77: one context per session.
- 0030 item 5: every pin move is audited.
- 0031 item 10: P55's glibc half, `reject_quick_exit_user_declared.c`'s glibc twin, question 59's near-miss twins and the
  tighter `sizeof` operand come with d1.

## Step 1 (first, markdown only): the plan documents

1. **Worktree.**
   - `git worktree add .claude/worktrees/m4.5d1 -b m4.5d1-vlibc d26a87c`.
   - Submodules with `--reference` (memory `background-worktree-setup`).
   - Then `scripts/bootstrap.sh`.
2. **Baseline worktree** `.claude/worktrees/m4.5d1-base`, detached at `d26a87c`. In the background:
   1. `bootstrap.sh`;
   2. `run_tests.py -j 8`;
   3. the VIR self-test;
   4. copy the results into the session worktree's `build/m4.5d1-baseline-{vpr,vir}`.
3. **Save the plan** as `.claude/plans/m4-5d1-vlibc.md`, with the Inputs below.
4. **`internal/handoff.md`:**
   - the header points at `git show d26a87c:internal/handoff.md`;
   - §1–§2: the state, and the owner steps (PR #26 first);
   - §3: the d1 notes move into the plan;
   - §4: 0046 is d1's record, 0047 the next free one;
   - §4.2: d2's paragraph gains the dependency map and the d2a/d2b proposal (question 113);
   - §8: questions 113–115 are answered, and none is open.
5. **`plan.md`:** the header becomes "the last: M4.5d1", and the M4.5d1 row names the plan file.
6. **Commit 1 (planning).** Local, with no Claude trailer.

## Inputs (from handoff §3, moved into the plan at step 1)

- **P55** (`il-inventory.md:4795`). The macOS half, measured only on macOS. Its `-I` reasoning is corrected by P71.
- **The attribute allow-list** holds a system declaration's attributes once a user or vlibc file redeclares the function
  (`[[const]]` on `abs`, `[[alloc_size(1)]]` on `malloc`, `[[__cold__]]` on `abort`). Check each attribute by where it is
  written [M4.5a].
- **0031 item 9's macro exemption** must count vlibc as a verified file or exclude it [S5]. Under the path rule,
  `directive_in_system_file` already excludes it in verify mode (`:1046`).
- **The marker rejection on a user redeclaration of a system function** reads only secondary declarations in verified
  files (`encode.cpp:2668`). vlibc's d1 headers carry no markers, so d2 revisits it [b2b].
- **0031 item 10:**
  - the glibc half of P55;
  - `reject_quick_exit_user_declared.c`'s glibc twin;
  - the glibc `assert` matcher's near-miss twins (`platform: linux`, question 59);
  - require the matcher's `sizeof` operand to be a call of `__assert_single_arg` (`encode.cpp:5980-6000`).

## Record 0046 — vlibc in cerastes

Written before code (invariant 6). It implements 0030 and amends:
- 0011 items 4–6;
- 0015 items 4–5;
- 0030 items 3–5;
- 0031 item 9.

Audit round 1 runs on the draft in the background while P71 runs.

1. **The pin.**
   - `third_party/vlibc` becomes a submodule at `5962865`, URL `https://github.com/cerastes-verifier/vlibc.git`.
   - Record 0001 gets a pins row and a pin-history entry.
   - **`scripts/vlibc_tree.py`** is modelled on `edg_tree.py`. It checks that the tree is clean, that `HEAD` equals the
     gitlink, and that the pin is in `origin/main` or a `local` remote; the last case is reported `NOT YET PUSHED`.
   - **`CERASTES_VLIBC_SRC`** builds against another vlibc checkout, as `CERASTES_EDG_SRC` does (0005). `check.sh`
     refuses it on `main`. d2 and M4.6 need it to edit both repositories together.
   - **CI.** `check.sh` runs the guard first. CI already checks out submodules recursively. If `ci.yml`, `ci.sh` or
     `ci-linux.sh` changes, `CiGuardTests`' canonical models change in the same commit.
2. **The include order** (0030 item 3 kept; P55's reasoning corrected). The driver passes, in this order:
   1. `--sys_include=<vlibc>/include/vlibc` (the shim);
   2. `--sys_include=<vlibc>/include` (`<vlibc/…>`);
   3. the platform's directories;
   4. `<cerastes>/include`;
   5. the user's `-I`s.

   Consequences:
   - `<vlibc/stdlib.h>` always resolves to the pinned copy, never to a user `-I` copy.
   - `CERASTES_TOOL_VLIBC_DIR` goes into `tool_paths.h.in` and `env.sh`.
   - `cerastes-fe.sh` stays as it is.
3. **The library class** (amends 0011 item 4). A file under the canonical `<vlibc>/include` is a library file.
   - It is kept out of `cfg.system_dirs` and listed in a new `cfg.library_dirs`, so it is a verified file.
   - `SrcPos::in_system` becomes "EDG's flag and not a library file" in verify mode (`render.cpp:130`). Through that one
     change:
     - `spec_marker_in_system_file` allows `predicate` and `pure` in vlibc;
     - initializers and bodies in vlibc are checked (`annotation_checks.cpp:1331`, `:1340`, `:1591`);
     - diagnostics in vlibc contracts point at the argument (`diagnostics.cpp:155`).
   - `directive_in_system_file` already uses the path in verify mode, so a predefined-macro redefinition in vlibc is
     `annotation_macro_changed`. `encode.cpp:9041` rejects `#line` and `#pragma GCC system_header` in vlibc.
   - **The audit table**, one row per site, comes from this planning's survey, with P71's facts.
   - **EDG's internal relaxations in system headers** are listed with their status: warnings dropped, discretionary
     errors downgraded (`error.c:3621`, `:3651`, `:3701`), and the first-declaration rules (`decls.c:2443`, `:4634`,
     `:9802`). Errors from `--diag_error` still fire (`error.c:5032`).
     - vlibc's CI compiles every header as a non-system `-I` header with `-Wall -Wextra -Werror` under GCC and Clang, in
       both languages.
     - cerastes's own checks apply by path.
     - The discretionary-error residual is re-examined when vlibc first holds a body (M4.6).
4. **Trust** (replaces 0015 item 5's table).
   - A bodiless routine whose `requires`/`ensures` attributes are written in a library file is trusted. The encoder finds
     those attributes by their own positions, not by `decl_position` (C++ moves it, C does not).
   - **The reason is vlibc's tag**, read from the comment line directly above the declaration that carries the contract.
     A library contract without a tag fails closed (exit 3). The emitted method repeats the tag, for example
     `// cerastes:trusted-contract vlibc: abort never returns (C23 7.24.4.1)`.
   - `trusted_libc_reason` and its use in `callee_for_call` are removed.
   - A system function that vlibc does not cover keeps today's rules.
5. **"Does not return" comes from vlibc.** For a routine whose contract comes from a library file, EDG's
   `does_not_return` flag (the SDK's `__dead2`, glibc's `__attribute__((__noreturn__))`) is ignored:
   - `f->noreturn` is false;
   - neither `ensures_on_noreturn` site fires (`:1254`, `:8940`);
   - the method's `ensures false` is vlibc's clause.

   `user_declared_noreturn` never trusts a reserved name. The expected difference is the tag text in the `.vpr`, and in
   the `.map` the position of the `ensures false` row, which now points at vlibc's line.
6. **Reserved names** (0030 item 4, amended by questions 114 and 115).
   - `scripts/vlibc_contracts.py` generates `vlibc_names.inc` at configure time from the pinned headers: `abort`, `exit`
     and `_Exit` in d1.
   - For a routine with C language linkage in the global namespace whose name is reserved, a contract (`requires`,
     `ensures`, `decreases`, a written `noreturn`/`_Noreturn`, `owning`, a marker) outside a library file is
     `annotation_reserved_library`. The message names vlibc's header.
   - C++-linkage look-alikes stay ordinary functions. A user definition with a body is verified as today; C 7.1.3's
     undefined behaviour is an M8 entry.
7. **Attributes by their own position** (P55). `check_attributes`, called on routines (`:8905`) and parameters (`:728`),
   skips an entry written in a system file, as the type path already does (`:8793`). Library and user attributes stay
   held to the allow-list.
   - **Why this is sound:** cerastes uses only a routine's contract and type, and a platform attribute is the platform's
     claim about its own function.
   - **What it requires of vlibc:** every vlibc contract implies the call preconditions of the platform's attributes
     (`nonnull` means `requires(p != NULL)`). The pin audit checks this, from d2 and M4.6 on.
8. **The pin audit as a check.**
   - `vlibc_contracts.py --check tests/vlibc/trusted_contracts.txt` lists every trusted vlibc declaration (name,
     declaration, contract, tag) and compares it with the committed list. It runs in `check.sh`.
   - A pin move that changes a trusted contract fails until the list changes. That change is a golden under invariant 1,
     and the soundness-auditor reads it.
   - Each trusted contract has its pair of tests.
9. **Listed, not changed:**
   - `_FORTIFY_SOURCE`'s `#undef`s (M4.6, `<string.h>`);
   - markers on vlibc declarations, and the b2b note (d2);
   - `quick_exit`, the `__builtin_*` functions and `<malloc/malloc.h>`;
   - vlibc must never declare `__assert_rtn`/`__assert_fail` first (`encode.cpp:5962`);
   - the self-test hook's exposure (item 10).
10. **Testing the library class.** A new self-test hook, `--cerastes_verify_selftest=library_fixture`, adds one fixed
    directory as a second library directory: `<cerastes>/scripts/testdata/library_fixture/include`, compiled in like
    `CERASTES_TOOL_INCLUDE_DIR`. No other directory can be given library trust. With it, `LibraryFixtureTests` in
    `test_run_tests.py` exercise what vlibc does not hold yet:
    - a `predicate` and a `pure` function in a library header, accepted;
    - an inline body there, verified, plus its must-fail twin;
    - a trusted contract, with its tag repeated in the Viper;
    - a contract without a tag → exit 3;
    - a `#define` of a predefined macro → `annotation_macro_changed`;
    - a `#line` → rejected.

    The hook is listed with the others (`verify_main.cpp:99`) and is not documented for users.

## P71 — include mechanics and IL facts (il-analyst, before code)

Run on macOS, then on `matt@clang4.cpp.al` (glibc 2.43, libc++ 21, clean build). Use a hand-built EDG command line with
the driver's flags and the planned order, in both modes, with vlibc at `5962865`.
- **(a)** The shim is reached by `<stdlib.h>`, `<cstdlib>`, `<array>` and `<exception>`. Its `#include_next` reaches
  libc++'s or the C library's header. `<vlibc/stdlib.h>` resolves to the pin, even with a user `-I` to another copy.
- **(b)** EDG accepts vlibc's redeclarations, in both modes and on both platforms:
  - after glibc's `__THROW` (`noexcept(true)` in C++) and `__attribute__((__noreturn__)) __COLD`;
  - after the SDK's `__cold __dead2`.
- **(c)** For each of the three:
  - where `decl_position` ends up in each mode (`decls.c:9802`);
  - each attribute entry and its position;
  - how the contract's declaration is found (secondary declarations or the attribute's position);
  - language linkage for `std::exit`, a user `extern "C"` declaration and a C++-linkage `void abort(void)`.
- **(d)** Also measure `malloc`, `calloc`, `realloc` and `free` as they are declared on both platforms, with their
  attributes, `noexcept` and `decl_position`. This is for d2's planning only; it costs a few lines.
- **(e)** EDG's flag on each vlibc file record, through each route, and `SrcPos::in_system` after item 3.

## Team (three Sonnet agents with disjoint files and one Opus reviewer, from `.claude/agents/`)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | **First, P71** (macOS, then the box). **Then:** `build/m4.5d1/vprdiff.py`, which compares every kept `.vpr`/`.map` with the baseline and allows only the tag and the `ensures false` row in programs that call the three; `il-inventory.md` P71 and P55's correction. Owns `src/vir`/`src/viper` only if lowering needs a change; none is expected. |
| **edg-backend** (Sonnet) | **First, the pin and the build:** `.gitmodules`, `bootstrap.sh`, `vlibc_tree.py`, `check.sh`, `src/driver/CMakeLists.txt`, `tool_paths.h.in`, `env.sh`, `CERASTES_VLIBC_SRC`, and the generated `vlibc_names.inc` (calling test-mutator's script). **Then:** `verify_main.cpp` (the order, the hook), `verify_config.*`, `il_query.*`, `render.cpp:130`, `annotation_checks.cpp`, `diagnostics.def`, and `encode.cpp`: trust, the tag reader, noreturn from vlibc, reserved names, attributes by position, `trusted_libc_reason` removed, and the `sizeof` operand. |
| **test-mutator** (Sonnet) | `scripts/vlibc_contracts.py` (the generator and `--check`) with self-tests; `tests/vlibc/trusted_contracts.txt`; the tests and twins below; `LibraryFixtureTests`; `vlibc_tree.py`'s self-tests; any `CiGuardTests` change; `driver/selftest.cpp`'s library-path cases; the doc example files. |
| **soundness-auditor** (Opus, read-only) | **Round 1:** record 0046's draft. **Round 2:** the code and tests, plus the pin audit of vlibc at `5962865` (each declaration, contract and tag against C23 7.24.4, and the platform attributes on both platforms). **One re-audit** of the blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Step 1, the baseline, the record, the docs, `internal/encoding.md`, integration, serialized builds (`lockf build/m4.5d1/build.lock`), status and handoff. Stops and asks. |

Any question from an agent, or of mine, stops that work and goes to the owner.

## Tests (`.c` in both modes where valid; each positive has a twin that fails for one reason at one line)

**Existing tests:** every `expect:` line is kept. The 29 tests that call the three now reach vlibc's contracts.

**New tests** (in `tests/vlibc/`):

| Positive | Twin (expected) |
|---|---|
| explicit `#include <vlibc/stdlib.h>`; `if (x < 0) exit(1); return x;` with `ensures(r >= 0)` | the call removed → `postcondition` |
| C++ `<cstdlib>` with `std::_Exit` and `std::abort` | the call removed → `postcondition` |
| C++ with only `<array>` (loads the shim), an unrelated function | — |
| the three through the shim on Linux (`platform: linux`, CI) | a path without the call → `check` |
| a user-declared `void abort(void);` with no `<stdlib.h>` and no contract | a call → `call_without_contract` (as today) |
| C++: `#include <stdlib.h>`, then a user redeclaration `int abs(int) CERASTES_ENSURES(...)`, accepted (the platform's `[[const]]` is skipped by position) | its contract violated at a call → `postcondition` |
| — | **reserved names** (question 115), each `rejected annotation_reserved_library`: `void exit(int) CERASTES_ENSURES(false);` without `<stdlib.h>` and after it (identical to vlibc's); `_Noreturn void _Exit(int);` (C); `extern "C" [[noreturn]] void abort();` (C++); `void exit(int) CERASTES_REQUIRES(status == 0);` without `<stdlib.h>`. With `<stdlib.h>`, a different contract is EDG's must-match error (pinned at its line). |
| — | the glibc twin of `reject_quick_exit_user_declared.c`; the glibc `assert` near misses and a `sizeof` operand that is not `__assert_single_arg(…)` → rejected (`platform: linux`) |

**Self-tests:**
- `vlibc_contracts.py`: a tag without a declaration, a declaration without a tag, a changed contract.
- `vlibc_tree.py`: dirty, `HEAD` not equal to the gitlink, not pushed, a wrong URL.
- `LibraryFixtureTests` (record item 10).
- `driver/selftest.cpp`'s library-path cases.

**Flips:** only the four pins of question 117 (pre-approved). Any other, stop and ask.

## Docs (invariant 8; every example is a test)

- **Verifying a program:**
  - a new section, "The C library: vlibc", after "What is verified": the shim in verify mode, `<vlibc/…>`, what is
    trusted and why, reserved names, and what is not covered;
  - library files in the definition of a system file (`:84-95`);
  - `:606-609` and `:871-874` updated.
- **Annotations:**
  - reserved names in "Declarations that repeat a contract" (`:184`) and "Declarations that disagree" (`:905`);
  - `:671-672` and `:874-877`.
- **Error messages:** `annotation_reserved_library`.
- **Building annotated code with ordinary compilers,** "The include path": vlibc's `include/` and the shim opt-in.
- **Installation:** the submodule (`:40`, `:19-21`).
- **`internal/encoding.md`:** `:305-307`, `:661-662` and `:717-721`.

## Order of work

1. **Step 1 and the baseline.** In parallel:
   - P71 on macOS;
   - edg-backend's pin and build;
   - the record draft, with audit round 1 in the background.
2. **P71 on the box.** Then the library class and the self-test hook, without the include order. **Commit 2:** the pin,
   the guard, the library class. Every `.vpr` must be byte-identical.
   - *Re-sequenced during the session:* the shim on the path alone would make vlibc's `ensures(false)` meet the SDK's
     `does_not_return` flag (`ensures_on_noreturn` at every call of `abort`). So the include order lands with the trust
     change in commit 3.
3. **The include order with trust, noreturn, reserved names and attributes by position**; `trusted_libc_reason` removed;
   the tests; `vprdiff.py`. **Commit 3.**
4. **The Linux items** and `LibraryFixtureTests`. Then audit round 2, the fixes and one re-audit. **Commit 4.**
5. **Docs; `check.sh`** (full, at most twice); a box run of the touched tests; status and handoff. **Last commit.**

## Stop and ask

- Any question from an agent.
- P71: EDG rejects vlibc's redeclarations in a mode or on a platform; the shim is not reached; or the contract's
  declaration cannot be found from the IL. Any of these would mean a vlibc change, which would be `NOT YET PUSHED`.
- A `.vpr` change beyond the allowed differences, or an existing test's outcome changing.
- Exit 3 on a correct program.

## Acceptance (commands run at the end)

- `python3 scripts/run_tests.py tests/scalar tests/vlibc tests/memory doc/modules/ROOT/examples`: all pass.
- `build/m4.5d1/vprdiff.py build/m4.5d1-baseline-vpr build/vpr`: only the allowed differences. The VIR self-test is
  byte-identical to `build/m4.5d1-baseline-vir`.
- Green:
  - `python3 scripts/vlibc_contracts.py --check tests/vlibc/trusted_contracts.txt`;
  - `python3 scripts/vlibc_tree.py check`;
  - `python3 scripts/test_run_tests.py`;
  - `python3 scripts/test_lint_viper.py`;
  - `scripts/build_docs.sh`;
  - `python3 scripts/check_doc_examples.py --html build/doc`.
- `scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): green.
- **On the box** (clean build): `bootstrap.sh`, then `run_tests.py tests/scalar tests/vlibc` and the `platform: linux`
  tests.
- **Status entry:** record 0046, the audit, the plan file, and the next step (M4.5d2's planning, with the d2a/d2b
  proposal). c2b's entry becomes a history row.
- **Commits:** planning; pin, include order and library class; trust; the Linux items and fixes; docs, status and
  handoff. Local, on `m4.5d1-vlibc`, no Claude trailer, never pushed. The handoff gives the push steps: PR #26 first,
  then this branch's PR. vlibc needs no push unless it changed.
