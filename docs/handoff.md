# Handoff after M1 (2026-10-13)

The next session reads this file after `CLAUDE.md`, its own milestone in
`plan.md`, `docs/status.md` and `docs/decisions/`. `status.md` has the formal
M1 entry. This file adds what a cold start would otherwise have to rediscover:
what to look at again, where the friction was, and what M1.5, M2 and M4
inherit.

## 1. State in one paragraph

Gate G1 passed (`docs/il-inventory.md`, "Gate G1 verdict"). EDG's prototype IL
carries the generic bodies, the instance links, the constraints and the
`[[cerastes::…]]` attributes the plan needs. cerastes compiles EDG's front end
itself, with its own configuration, and links an in-process `back_end()`
(decision 0002). It does this without touching `third_party/edg`.
`build/cerastes-fe` is that front end. `--cerastes_walk` prints a
deterministic listing of the IL, and `--il_display` prints EDG's own dump.
Every G1 fact is checked by golden tests in `tests/il/`. A pin move that
breaks one fails `check.sh`. `check.sh` is green in about 13 s.

## 2. Things to look at again

These held up for M1 but are compromises or open questions.

1. **Generated EDG headers come from the stock build.** `cmake_defines.h`,
   `err_codes.h` and `err_data.h` are read from `build/edg/src/includes`
   (0002 §3). That `cmake_defines.h` silently pre-empts `host_envir.h`'s
   derived defaults: it hard-codes `DEFAULT_INSTANTIATION_MODE tim_none` and
   `DEFAULT_REMOVE_UNNEEDED_ENTITIES 0`. Everything load-bearing is now
   pinned in `edg_config`, checked by `static_assert`s in `src/backend/edg_il.h` and
   checked again by `scripts/check_edg_config.sh`.
   - A new load-bearing macro must be added in all three places.
   - If the stock base configuration ever changes, consider generating the
     headers ourselves (`edg-cmakedef`, `mk_errinfo`).
2. **Shared-expression budget instead of a visited set.** EDG's `Ptr_set`
   allocates from the front-end region, which is dead once `back_end()`
   runs. So the walker caps the number of recorded expressions it follows (1,000,000) instead of
   tracking visited shared expressions, and prints a truncation warning if
   it is reached. It is never reached in the tests.
   - The walker reports each operation node at most once per routine, and
     each use of a template-parameter constant (the pair of constant and
     `enk_constant` node) at most once. It keeps its own per-routine sets,
     because a node can be reached both through a constant's backing
     expression and through the `ck_template_param` variant's expression.
     The dedupe hides double traversal. Audit pass 3c found duplicate
     `tconst` lines only by counting constants against `il_display`, so
     count, don't just diff, when the walker changes.
   - M3/M4 encoders that walk expressions need their own visited set
     (`std::unordered_set` in our code, not EDG's allocator).
3. **Lambda names come from `output_name`/`output_class_qualifier` hooks.**
   This replaces EDG's `lambda [] type at line <seq>…` (which prints a
   sequence number labelled as a line) with `<lambda@L:C>`. For every other
   entity the hook defers to EDG's own `form_type`/`form_name`. An earlier
   version that re-implemented EDG's tag-keyword logic broke C `enum` tags and
   the template arguments of unnamed types. If type rendering ever looks off,
   look here first (`src/backend/render.cpp`) and compare with `il_display`.
   Known limit: a closure that appears as a template argument inside another
   name keeps EDG's default spelling.
4. **Primary-file filter.** Only routines whose position is in the primary
   source file are reported. Code in user headers or after `#line` is hidden
   but counted: `end walk: … <U> routines with bodies in non-system files
   outside the primary file`, plus a warning for each `cerastes::` attribute
   there. A `#line` or GNU linemarker in the primary file gets its own
   warning. Code after a flag-3 ("system header") linemarker is neither
   reported nor counted, so that warning is its only trace. M3's driver needs
   a real policy for user headers, because verified code will live in them.
   M3 must also own the command line: `cerastes-fe` passes every EDG option
   through. It rejects `--remove_unneeded_entities` and its abbreviations, by
   checking EDG's runtime globals in `back_end()`, but options such as
   `--instantiate=none` would still change the IL.
5. **The goldens are tied to the toolchain.** Constraint lines print libc++'s
   inline namespace (`std::__1::integral`), and `il_to_str` has no option to
   hide it. Goldens also carry EDG's #2803 warnings in their stderr section.
   - Expect mass golden diffs when the macOS SDK or the EDG pin moves, and
     when M2 registers the `cerastes` namespace (the #2803 lines disappear).
   - Each regeneration needs a reviewed reason (invariant 1; `tests/README.md`).
6. **Facts `il_display` cannot show.** It never prints attribute entries that
   hang off statements, nor some constant entries (`tpck_unknown_function`
   callees, `tpck_member`). For those the walker is the only reader. The
   inventory records the differential method used instead (mutate the source,
   watch only the expected walker lines change). Do not conclude that
   something is missing from the IL by grepping `il_display`: one agent did
   that this session and reported a false loss of null-statement attributes.
7. **Default arguments EDG has not evaluated** have `default_arg_expr == NULL`
   and are not walked (noted in the walker's contract comment).
   **Folded constants.** EDG folds non-dependent constant expressions. The
   operations survive only as the folded constant's recorded expression
   (`constant->expr`). The walker follows two kinds of recorded expression:
   - those of `ck_template_param` constants;
   - those of unnamed constants whose expression lives in the routine's own
     memory region (`!in_file_scope(c->expr)`), i.e. folds written in the
     body, such as `3 + 4`, `1 << 4` and `sizeof(T) * 2`.

   It does not follow named entities' values, such as an enumerator's
   initializer, or file-scope expressions, such as another function's
   default argument used at a call site. Both would otherwise be printed as
   if they were in the routine. M4's eligibility checker needs the same
   distinction. Expressions embedded in types (array bounds, C VLA bounds,
   `Holder<sizeof(T) / 2>`) and `static_assert` expressions are not walked.
   A routine's own default arguments are walked with file-scope folds
   allowed. That currently also pulls in a callee's default argument nested
   inside one (`c = h() + 1`), which shows at the callee's position. The fix
   is in inventory "Walker gaps"; no golden exercises it. For M3 this means a
   non-overflowing constant expression such as `INT_MAX - 1 + 1` may reach
   the encoder already folded. An overflowing one (`2147483647 + 1`) keeps
   its `eok_add`, with a warning and exit status 0. `6 / 0` is only a warning.
   M3's integer-semantics decision record has to say which overflow checks
   happen on folded constants, and how.
8. **No helper headers next to tests.** Discovery treats every source-like
   file (`.h`, `.hpp`, `.cc`, …) under `tests/` as an error, so that a
   mis-suffixed test is never silently skipped. M3 and M6 tests that need a
   `#include "helper.h"` will need an escape hatch, for example an allowlisted
   `include/` directory. Add it with a self-test twin, as for the skip
   allowlist.
9. **Rendering of `type`.** A dependent operation's type is the
   `tptk_unknown` placeholder and prints as `<tptk_unknown>`. EDG itself calls
   it `<unnamed>`.

## 3. Friction (process and tooling)

- **New agent types load only at session start.** `edg-backend` and
  `il-analyst` were written into `.claude/agents/` during M1, so they could not
  be spawned by type. They ran as `general-purpose` agents (model `sonnet`)
  with their definition file as the first instruction. From M1.5 on they are
  real types. The team was:
  - `edg-backend`: build, configuration and walker;
  - `il-analyst`: evidence, inventory and golden checking;
  - `test-mutator`: harness, twins and contrast probes;
  - `soundness-auditor` (Opus, read-only): three passes (configuration, walker and
    harness; full review with the G1 verdict; pre-commit).

  File ownership was disjoint, so all four worked in one tree without
  conflicts.
- **The auditor paid for itself.** Pass 1 found two blocking walker defects
  that the goldens had already frozen:
  - lambda and local-class routines were shown their enclosing function's
    body;
  - operations inside template-parameter constants, such as the
    `tk_unknown` cast in `(T)100` and `++i`, were invisible.

  It also found the inherited `DEFAULT_REMOVE_UNNEEDED_ENTITIES` hazard.
  Keep the order "walker → independent golden check → audit" for anything
  that turns IL into facts.
- **Read compile flags from the right target.** I first told the build agent
  that EDG compiles as `gnu++14`. That came from the first `FLAGS` line in
  `build/edg/build.ninja`, which belongs to a `util/` tool. Stock cpfe uses
  `-std=c++14 -pedantic -Werror=shadow -Wconversion …`, and `edg_fe` now
  mirrors that exactly.
- **EDG cannot parse `<algorithm>`, `<utility>`, `<iostream>`, `<string>` or
  `<vector>` from the macOS SDK 27.0 libc++** (status M0). That kept probes
  away from `std::min` and the like. `<concepts>`, `<cstdint>`, `<limits>`
  and `<type_traits>` work. It is still worth an upstream EDG issue.
- **Each subagent has a large context.** Resuming agents with `SendMessage`
  (instead of fresh spawns) kept their findings. The il-analyst's comparator,
  which re-derives walker facts from `il_display` dumps, lives only in this
  session's scratchpad. If M2/M4 need it, have `il-analyst` rebuild it in
  `scripts/` as a tool.

## 4. For M1.5 (next by plan order)

M1.5 depends only on M0 and does not use anything M1 built. M1 added no
user-visible behaviour: `cerastes-fe` is a developer tool, so it gets no
`doc/` page. Its owner steps (create `cerastes-verifier/doc`, enable Pages,
add the dispatch token, add the website link) are outward-facing and stay
with the owner. M1.5 writes **0004-docs-site.md**, as the plan says. 0003 is
now the EDG fork-branch record: G1 passed, so the fallback record that plan.md
reserved that number for was never written. `check.sh` gains a docs step. Keep it
after the tests step, or make sure `run_tests.py`'s discovery (now with a skip
allowlist and orphan-golden detection) also covers
`doc/modules/ROOT/examples/` as plan.md asks.

## 5. For M2 (annotation layer)

- **Critical path: registering the attributes is an EDG source change.**
  Registering `cerastes::requires/ensures/invariant/check/…` in `attribute.c`'s
  descriptor table, and adding `cerastes` to the known namespaces
  (`attribute.c:970-980`), changes `third_party/edg`. Under invariant 5 that
  means (decision 0003):
  1. small commits with tests on branch `cerastes` of
     https://github.com/mborland/compiler, which the owner created on
     2026-10-13;
  2. an upstream issue, then a PR to `edgcpp/compiler`, following EDG's
     `CONTRIBUTING.md`;
  3. only then a pin move.

  **Before its first commit, the branch needs a fast-forward.** It was
  created from the fork's old `main` (`90b66472`), 22 commits behind the pin
  `9847356d` (see question 1 below).
  - Sessions may commit and push to the fork branch.
  - Sessions never open upstream issues, PRs or Discussions: the owner files
    them. Write the proposed issue and PR text into this file for them.
  - Prepare the EDG series at the very start of M2, so the owner can open the
    upstream issue early.
  - cerastes code cannot depend on the registration until it is upstream. Prefer a general
  mechanism that upstream will want, such as declaring extra attribute
  namespaces and descriptors from a back end or an option, over hard-coding
  `cerastes` into `attribute.c`.
- **Why the unrecognized-attribute path is not enough for contracts**
  (inventory check 5c). An attribute after the parameter list lands on the
  function type (`al_post_func`), not on the routine. It is lost when the
  function is redeclared. For a function template declared more than once,
  neither copy survives. Registered attributes are applied while the
  declaration is processed, which should fix this. That is an expectation,
  not a verified fact (inventory check 5c). M2 must test contracts on a
  declaration plus a redeclaration, on a template declaration plus its
  definition, and on a member defined outside its class.
- **Ill-formed placements become silent once the namespace is registered.**
  `[[cerastes::x]] struct S {};` (a leading attribute with no declarator)
  appertains to nothing and is not in the IL. Today only #2803 betrays it;
  after M2 registers `cerastes`, nothing will. Give it a diagnostic or a
  must-fail test.
- **What the unrecognized path does carry reliably:**
  - prefix attributes on function declarations (plain, function template
    prototype and instance, class-template members);
  - attributes on `while`/`for`/`if`/`switch`/`do`/compound/expression
    statements;
  - attributes on null statements in every position tried (first in a block,
    after a label, in C and C++).

  So `check(p)` on a null statement and `invariant(p)` on loops are safe
  placements.
- **Class-level annotations go after the class-key** (`struct [[x]] S`). A
  prefix `[[x]] struct S {…};` appertains to no entity and is not in the IL.
- **What M2's golden tests should compare.** plan.md asks for
  "`il_display` golden output". `il_display` prints addresses and omits
  statement attributes, so golden-compare walker output instead. Extend
  `--cerastes_walk` to print the typed expression tree of each `cerastes::`
  attribute argument once EDG parses them.

## 6. For M4 (generic templates)

From the inventory's gotchas; each is evidenced there.

- **`(T)<literal>` is not an `eok_cast`.** It is an `enk_constant` of
  `ck_template_param`/`tpck_expression` whose `expr` is an `eok_cast` with
  `type_kind tk_unknown`.
  - A naive fail-closed rule ("reject `tk_unknown` operations") would reject
    `clamp100`'s `(T)100` and `two_hundred`'s `(T)200`, which VeriFast treats
    as literal casts.
  - The eligibility checker must recognise this exact shape (cast of an
    integer literal to a template parameter) and still fail closed on every
    other `tk_unknown`.
- **`type_kind` is not a dependence test.**
  - Most dependent operations are `tk_template_param`.
  - `eok_question`, `eok_call` and casts inside constants are `tk_unknown`.
  - `eok_land`/`eok_lor` are `tk_integer` even in prototypes.

  Dependence needs a recursive type walk; there is no `isDependentType`
  flag.
- **Expressions hide inside `tpck_expression` constants:** `!(a<b)`, default
  arguments, `&r`. Walk `constant.expr`, as the walker now does.
- **Telling prototypes from instances.**
  - Both carry `assoc_template` and `template_arg_list`; the prototype's
    list holds the template's own parameters.
  - An instance is a routine *without* `is_prototype_instantiation`.
  - There is no per-template list of instances, so scan all routines.
  - `template_arg_list` is NULL for members of class-template instances.
- **Constraints.**
  - `std::integral` is `std::__1::integral`, so match the concept template
    entity, not a string.
  - For a type-constraint, `concept_id.args` is NULL; the constrained
    parameter is implicit.
  - The four locations are recorded in inventory check 4.
- **VeriFast originals and parity.** The originals are in
  `tests/differential/verifast/` (skipped by the harness; README gives the
  source commit and the `testsuite.mysh` lines). Annotation-free copies with
  goldens are in `tests/il/verifast-plain/` (line N there is line N−6 of the
  original). VeriFast still has to be built or recorded (0001 §VeriFast); it
  is a Linux binary here.

## 7. M1 acceptance (commands and results, 2026-10-13)

| # | Command | Result |
|---|---|---|
| 1 | `scripts/bootstrap.sh --build-only` | builds `build/cerastes-fe`; a re-run is a no-op (0.9 s) |
| 2 | `build/cerastes-fe --dump_configuration \| grep -E '#define (PROTOTYPE_INSTANTIATIONS_IN_IL\|ALL_TEMPLATE_INFO_IN_IL\|RECORD_UNRECOGNIZED_ATTRIBUTES\|EXTRA_SOURCE_POSITIONS_IN_IL\|BACK_END_IS_(C\|CP)_GEN_BE\|DO_IL_LOWERING) '` | 1, 1, 1, 1, 0/0, 0. `scripts/check_edg_config.sh` checks 15 values in `check.sh`, and it fails (exit 1, naming the macros) against a corrupted dump. |
| 3 | `scripts/cerastes-fe.sh --c++20 --il_display tests/il/verifast-plain/integral.cpp` | `Intermediate language for memory region 41 (function "uninstantiated")`; the walker shows `routine uninstantiated prototype body @ L102:3` |
| 4 | `python3 scripts/run_tests.py tests/il` | 39 golden tests pass (g1 5, verifast-plain 6, inventory 9, walker 2, contrast 17) |
| 5 | `python3 scripts/test_run_tests.py` (also with `CERASTES_REQUIRE_E2E=1`) | 140 tests OK, 0 skipped |
| 6 | `scripts/check.sh` | green in about 13 s: 44 passed, 0 failed, 1 skipped (`tests/differential/verifast`) |
| 7 | `git -C third_party/edg status --porcelain`; `git submodule status` | empty; EDG still at `9847356d`, Silicon at `6b540aa4` |
| 8 | clean clone of the M1 commit: `scripts/bootstrap.sh && scripts/check.sh` | green in 1 min 47 s for `b1e3d41`. The run fetched the submodules from GitHub; built EDG (stock), `build/cerastes-fe` and Silicon; downloaded Z3; and ran `check.sh`: 140 self-tests OK, 44 passed, 1 skipped. It left no `.ti` or other stray files, and the tree was clean afterwards. The Homebrew and sbt caches were already warm. |
| 9 | `docs/il-inventory.md` | evidence on every G1 check and table row, or the row is marked derived/UNCLEAR with the reason. G1 verdict recorded. The final soundness audit passes (3, 3b, 3c, 3d) found nothing blocking; the remaining notes are tracked in §2 and in the inventory's "Walker gaps". |

## 8. Questions for the owner

Every open decision from this session is here, with a recommended answer, so
they can all be answered at once. Future handoffs keep this section.

1. **The fork branch needs a fast-forward, and sessions can't push yet.**
   `cerastes` on https://github.com/mborland/compiler is at `90b66472`, 22
   commits behind the pin. Decision 0003 lets sessions push to it, but this
   session had no GitHub credentials: no SSH key, and no keychain entry
   visible to it. Please run:
   ```
   cd ~/Documents/compiler && git fetch edgcpp && git checkout cerastes
   git merge --ff-only 9847356d461d2bef846bf133e16821f0e8b69749 && git push origin cerastes
   ```
   How should future sessions push?
   - (a) Give the session credentials, e.g. a fine-grained token limited to
     `mborland/compiler` contents.
   - (b) Sessions commit in a local clone of the fork, outside this repo,
     and you push.

   *Recommended: (b) until you decide otherwise.*
2. **Can a topic branch of this repository temporarily record a fork
   submodule pointer?** *Recommended: no, as 0003 says.* Develop against fork
   commits by checking them out in the submodule working tree, without
   committing the pointer.
3. **Next milestone.** plan.md order is M1.5 (docs), then M2. M2's critical
   path is upstream review of the EDG attribute registration, which you
   file. *Recommended: run M1.5 next. Its session also drafts the text of
   the upstream Discussion or issue for attribute registration (design only,
   no EDG code), so you can file it while M1.5 runs.*
4. **M1.5 owner steps.** M1.5's acceptance (`curl -sI
   https://cerastes.dev/doc/` returns 200, and a docs-only commit appears
   with no manual step) needs four things done first:
   - create `cerastes-verifier/doc`;
   - enable Pages;
   - add the dispatch-token secret to this repo;
   - add the "Documentation" link on cerastes.dev.

   *Recommended: do them before the M1.5 session. Otherwise it ends with
   those acceptance items open.*
5. **What CI runs.** M1.5 adds CI that builds the docs. The IL goldens
   depend on the macOS SDK and libc++ (`std::__1::…`, which headers EDG can
   parse), so `check.sh` on a GitHub macOS runner would likely show golden
   diffs. *Recommended: CI runs only the docs build in M1.5. Running
   `check.sh` in CI needs a decision record on pinning the SDK first.*
6. **The libc++ headers EDG cannot parse** (`<algorithm>`, `<utility>`,
   `<iostream>`, `<string>`, `<vector>` from SDK 27.0). These will matter for
   M3+ tests. *Recommended: in M2, a session writes a reduced reproduction
   (under 100 lines, with `--dump_configuration`, as `CONTRIBUTING.md` asks)
   into this file for you to file upstream.*
7. **Committing in this repository.** Should sessions commit to local `main`
   at the end of the session, as plan.md's closing prompt says, and after
   each change you ask for, without asking first? Never pushing. *Recommended:
   yes.* This session committed its last change on that basis.
8. **VeriFast for M4** (0001): a native macOS build, a container for the
   Linux binary, or recording its outcomes once? Not urgent until M4.
   *Recommended: try the native build at the start of M4, and fall back to
   recorded outcomes.*
