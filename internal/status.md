# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, M4.5a's
`git show 72e30b3:internal/status.md`, S5's `git show 794e960:internal/status.md`, M4.5b1a's
`git show bdc6bd0:internal/status.md`, M4.5b1b's `git show 8003dca:internal/status.md`, M4.5b1c1's
`git show eab4058:internal/status.md`, M4.5b1c2's `git show a5e3e30:internal/status.md`, M4.5b1c3's
`git show 5ffb1a2:internal/status.md`, M4.5b2a's `git show 38877b4:internal/status.md`, M4.5b2b's
`git show 1bfa705:internal/status.md`, M4.5c1's `git show cbdcce5:internal/status.md`, and M4.5c2a's
`git show 7f11706:internal/status.md`, M4.5c2b's `git show d26a87c:internal/status.md`, and M4.5d1's
`git show 9d98fe8:internal/status.md`, M4.5d2a's `git show bb41d55:internal/status.md`, S6's `git show 9255b93:internal/status.md`, and M4.5d2v's `git show c2bc48d:internal/status.md`.

## S7 — cerastes's tool code in the owner's Boost style (2026-11-05): done on branch `s7-code-style`

Plan: `.claude/plans/s7-code-style.md` (its acceptance amended in the session, as the proof stands). Record 0051; 0001
amended (the clang-format row). Read `internal/handoff.md` next: §2 (owner steps: push this branch, dispatch the full
check once, merge before M4.5d2b branches), §3 "From S7", §8 (questions 150 and 151).

**Owner decisions:** at planning (2026-11-05): 146 (a session of its own, proved behaviour-neutral), 147 (the full
sweep), 148 (PyPI's clang-format wheel 23.1.3 on both platforms), 149 (Allman lambdas). None asked during the session.

**Done** (the worktree `.claude/worktrees/s7`; three Sonnet agents, S1 tooling and backend/driver, S2 the proof tool and
vir, S3 fixups and viper; one read-only Opus reviewer, two rounds)
- `9cff0ad` (outside the plan): `doc/package-lock.json` deduplicated. Dependabot's Antora 3.2.2 merge (`aa488d8`) had
  nested six copies of `@antora/logger`, so `build_docs.sh --log-failure-level=warn` configured one copy while
  asciidoctor logged through another: a broken xref no longer failed the docs build, and `DocBuildWarningsTests` failed
  on `main` (`c2bc48d`). Versions and integrity hashes unchanged.
- **C1 `1da1763`, tooling:** the style's `.clang-format` (checked against int128 and decimal), the pinned clang-format
  (PyPI's wheel 23.1.3 per platform with its SHA-256; `bootstrap.sh`, `CERASTES_CLANG_FORMAT`, record 0001's row),
  `scripts/format.sh [--check]`, `scripts/object_identity.py` and its self-test (in `check.sh`), record 0051's draft.
- **C2 `8f2beff`, the layout:** `format.sh` over the 75 files, nothing else (74 changed; 7,992 inserted brace tokens and
  no other token change; every file the pinned formatter's output of its C1 version); `check.sh`'s lint step runs
  `format.sh --check`. **C3 `52a7294`, hand fixups:** about 261 trailing comments that forced a break moved above their
  code (24 on lambda openers where clang-format had given up), directives inside functions indented, the two long
  `#error` lines whole under `clang-format off`, the X-macro bodies of `diagnostics.cpp` under `clang-format off`,
  `register_attributes.h`'s stale count (S6's note); zero token changes.
- **Round 1's fixes `4a77088`:** fail-closed guards (the root `.clang-format` is `DisableFormat`; the style lives in
  `src/.clang-format` and a byte-identical `include/.clang-format`; `scripts/testdata/`'s fixtures and `internal/`'s
  probes had been unguarded), `.clang-format-ignore`'s `dir/**`, the binary's SHA-256 checked on every bootstrap; the
  tool's token pass, no `-DNDEBUG`, CMake inputs covered, normalisation for `assertion_failed` only, an allow-list for
  dependencies. **`5ce98fa`:** the one control statement InsertBraces could not reach (`walker.cpp`, an `#endif` between
  the `if` and its statement; `MICROSOFT_EXTENSIONS_ALLOWED` is 1 in cerastes's EDG configuration, so it is compiled).
- **The sweep** (S2's one-time helper, `build/s7/sweep/`: a clang tool over the real ASTs plus clang-tidy, record 0051
  item 5): C4a `e667747` (backend, driver), C4b `6c76572` (vir), C4c `b762747` (viper; `include/cerastes.h` needed
  nothing): 111 declarations split, 1,641 `U`, 7,028 brace initializations; four narrowing fixes with the conversion
  the copy-initialization made, `static_cast<T>` inside the braces (`annotation_checks.cpp`, `encode.cpp`,
  `vir/dec.h`, `vir/check.cpp`). Kept as written: 23 macro-spelled `U` sites (`register_attributes.cpp` 10,
  `verify_main.cpp` 5, `silicon.cpp` 3, `encode.cpp` 2, `annotation_checks.cpp`, `diagnostics.cpp`, `run_state.cpp`
  1 each), 657 lambdas, 792 range-`for` and 89 condition declarations (by rule), parenthesised direct-initialization.
  **C4d `5dd303f`** (round 2): 23 member declaration groups split by hand (30 declarations); the `for`-init with two
  declarators in `encode.cpp` is the one exception.
- **C5 `3e1dd4f`, `e09b0e5` and this commit:** `CLAUDE.md` (the format and identity commands, the style convention), the
  developer guide (contributing's "Code style", building's pin and variable, testing's format step and "Refactors that
  change nothing"), record 0051 accepted, `.git-blame-ignore-revs` (C2, C3, `5ce98fa`, C4a–C4d), `il-inventory.md`'s
  ~70 source citations pinned to `aa488d8`, the handoff's live citations by function name; round 2's tool items
  (`.clang-format` files skipped by the coverage check, the noise named, `third_party` pins and
  `scripts/vlibc_contracts.py` compared, `--formatted-by`, the `braces` limit documented; self-test 45 tests).

**Checks run**
- `check.sh` (full, `CERASTES_CHECK_BASE` unset), twice. Run 1 at C1 in a detached worktree (`.claude/worktrees/s7-base`,
  so the layout work went on beside it): all green in 1 h 17 min (harness self-test 1108; suite 10441 passed, 0 failed,
  10 skipped, native-ok Apple clang 1945, GCC 1945; lint 4037 files clean; VIR self-test 1561 passed; walker oracle 1924
  facts; docs OK). Run 2 at `3e1dd4f` (all of the sweep but C4d): all green in 1 h 17 min, the same counts. Run 2's
  first start stopped after 4 s at the build step (`doc/node_modules`' stamp, stale since the lockfile fix was installed
  by hand; `bootstrap.sh` refreshed it) before checking anything.
- **Output identity:** run 2's `build/vpr` (8,075 files) against run 1's: identical but the `MANIFEST` run-id line and
  350 `.map` files whose only difference is the worktree's name in absolute paths to vlibc's headers (`s7-base` against
  `s7`); `build/vir-selftest` identical.
- **Object identity** (`build/s7/evidence/`, commands and exit statuses in its `README.md`): C1/C1 30 identical;
  C1→C2, C2→C3, C3→C4a…C4d each 29 TUs byte-identical and `encode.cpp` identical modulo its two `assertion_failed`
  line immediates (IR compared, objects not), or 30/30 where `encode.cpp` did not move; all-decls noise 0 in the layout
  commits and 12 in the sweep, all one constexpr libc++ trait (`__can_be_converted_to_string_view_v`); tokens C1→C2
  braces only, C2→C3 none; `walker.cpp`'s brace the one explained token difference.
- **After run 2** (C4d and docs; memory `verify-delta-not-full-check`): C4d object-identical to `e09b0e5` (encode.cpp
  modulo lines; tokens only `,` → `;` plus the repeated type); rebuilt, the VIR self-test's 1561 cases written
  byte-identical to the baseline; `run_tests.py doc/modules/ROOT/examples` 853 passed, 0 failed, their Viper identical
  to the baseline's; `format.sh --check`; `test_object_identity.py` 45 OK; `build_docs.sh` and both
  `check_doc_examples.py` runs clean.
- **Linux** (the x86-64 box, container from `cerastes-m4.5d1:runner`, clang 21.1.8 over libc++ 21): at C3 and at
  `3e1dd4f`, `ci-linux.sh`/`bootstrap.sh --build-only` (the manylinux wheel's and binary's SHA-256), `format.sh --check`
  75 conform, the self-test (31) with no skip, `object_identity.py --before 1da1763` 29 identical + `encode.cpp` modulo
  2 lines with noise 0, `--tokens braces` C1→C3; `MICROSOFT_EXTENSIONS_ALLOWED` and `EXPENSIVE_CHECKING` 1. At the final source tip `5dd303f`, in a fresh
  container: `ci-linux.sh` green, `format.sh --check` 75 conform, the self-test 45 OK with no skip,
  `object_identity.py --before 1da1763 --after 5dd303f` with default options exit 0 (29 identical + `encode.cpp`
  modulo 2 lines, no noise, the `third_party` pins and `vlibc_contracts.py` agree), and `--formatted-by` with the
  Linux binary reproduces C2's 74 files exactly. The container and checkout were removed.
- Audits: round 1 (B1 hand edits invisible to object identity, S1–S9, N1–N9: `build/s7/audit1.md`), round 2 (no blocking
  item; S1–S6, N1–N9: `audit2.md`); every item taken; no re-audit (round 2 had no blocking item).

**Surprises**
- `encode.cpp` passes `__FILE__`/`__LINE__` to EDG's `assertion_failed` through EDG's checking macros, although `src/`
  uses neither itself: a layout change cannot leave that object byte-identical. The proof compares its IR with exactly
  those two immediates zeroed and says so.
- The reviewer's premise that the `MICROSOFT_EXTENSIONS_ALLOWED` branch is compiled by no build was wrong: the macro is 1
  in cerastes's configuration (`defines_macosx.h`'s 0 is overridden).
- `readability-isolate-declaration` never sees member declarations; 23 groups were left until round 2.
- `.clang-format-ignore`'s `tests/` pattern matches nothing in 23.1.3 (`tests/**` does).
- A Dependabot lockfile broke the docs' failure on warnings on `main` silently, caught only by the harness self-test.

**Decision records added:** 0051-code-style; 0001 amended (the clang-format row).

**First step for the next session.** The owner pushes `s7-code-style`, dispatches the full check once and merges it
(handoff §2). Then M4.5d2b's planning (handoff §4.2), on a branch from the merged `main`.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| S7 Code style | 2026-11-05 | `s7-code-style` | 0051 (0001 amended) | cerastes's tool code in the owner's Boost style: the pinned clang-format 23.1.3 (`format.sh`, a `check.sh` step, fail-closed guards), then brace initialization, `U` and one declarator per declaration; proved by `object_identity.py` (objects and IR at every commit, macOS and Linux; tokens) and by `.vpr`/`.map` identical over the suite and the VIR self-test; the docs lockfile fixed |
| M4.5d2v vlibc's allocator | 2026-11-04/05 | `m4.5d2v-allocator` | 0050 (0047 amended) | the M4.5d2a re-audit's B1–S3; vlibc's allocator (`vlibc_malloc` and six more, a C23 core over trusted glue, sanitized and threaded tests); blocks know their allocator (the token's value); the pairing check; contract-less library routines; vlibc allocations rejected in C++; two older exit-3 holes fixed; vlibc and vlibm in the owner's Boost style |
| S6 Tutorial and developer guide | 2026-11-04 | `s6-tutorial` | 0049 | a VerCors-style tutorial (seven chapters, 20 verified programs and 24 failing variants, both modes) and a developer guide (eight pages) as sections of the one site; the overview rewritten; the website's landing page; a chapter per concept from now on (questions 131–133) |
| M4.5d2a The heap | 2026-11-03/04 | `m4.5d2a-heap` | 0047 | vlibc's `malloc`, `calloc`, `free` (trusted, reserved identifiers, shapes not names); typed views at `void*` → `T*` (one object, `sizeof(T)`); heap blocks with the free token; `invalid-free`; leaks by `forperm` (after `l_end`, at back edges; `main` included); `owns_block`; `owning`/`ref_to_uninit` on functions; sequencing over block sites; loops over blocks held through locals; d2 split (d2v allocator, d2b) |
| M4.5d1 vlibc in cerastes | 2026-11-03 | `m4.5d1-vlibc` | 0046 | `third_party/vlibc` pinned and guarded; vlibc's shim first in verify mode; library files by path; trust from vlibc's tags, "does not return" from vlibc, reserved names, attributes by position; three routes redirecting trusted calls closed (older than vlibc); `trusted_libc_reason` gone; d2's dependency map |
| M4.5c2b Places through a heap read | 2026-11-03 | `m4.5c2b-heap-read-places`, PR #26 | 0045 | contract and invariant places through loads (`**pp`, `*gp`, `p->q->x`, `*s.q`, `*r.q`) with the entry-state rule, the post-state reading, amounts that follow objects, bodiless ties, heap-reading guards, `eb`/`lb` loop places, written atoms through loads and on struct places, `restrict` over load roots; `live(r.q)` to M4.5d2 |
| M4.5c2a Struct values and struct storage | 2026-11-02/03 | `m4.5c2a-struct-values`, PR #25 | 0044 | by-value struct parameters and results as tuples of leaves, `f().x`, C++ temporaries, compound literals (C objects, C++ values), struct globals and `static` struct locals, member measures, `restrict` beside by-value structs; invariant reads through a loaded pointer pinned; M4.5c2 split, heap-read places and written struct atoms to M4.5c2b |
| M4.5c1 Struct objects | 2026-11-02 | `m4.5c1-struct-objects`, PR #24 `8f45843` | 0043 | C structures with member blocks, struct locals as blocks, `.`/`->`, pointers to members, initializers, copies, `struct S *` objects with `WF_S`, contract atoms on struct places with overlap per leaf cell, markers per leaf; `restrict` closed beside loaded pointers (a hole since 0037); M4.5c split, owning fields and P4418R0 to M4.5d2 |
| M4.5b2b Initialization through pointers | 2026-11-02 | `m4.5b2b-init-markers` | 0042 | P4222's markers (`uninit` on locals, `ref_to_uninit`/`must_init` on pointer parameters), every call flag row `uninitialized`, `--strict-init` with the `// verify-options:` header, the loop row on every loop place (question 89, fixing 0038's nested-loop exit 3); M4.5b2 complete |
| M4.5b2a Globals and `static` locals | 2026-10-30 | `m4.5b2a-globals`, PR #22 `38877b4` | 0041 | mutable globals and `static` locals as `unique` blocks, `program_start` for `main` with initial values by linkage, frames through `owns(g)`/`reads(g)`, implicit atoms for `static` locals; M4.5b2 split, the markers to M4.5b2b |
| M4.5b1c3 Pointers to pointers, owning, `*result`, written atoms | 2026-10-30 | `m4.5b1c3-pointers-to-pointers` | 0040 | `T**` at any depth and pointer cells, `[[cerastes::owning]]` on parameters, `*result` as a value, written `owns`/`reads`/`live` in loop invariants (question 68's false alarms gone); conversions and contract comparisons across `const` rejected (a hole since 0037 closed); M4.5b1 complete |
| M4.5b1c2 Sequencing and placement | 2026-10-29 | `m4.5b1c2-sequencing` | 0039 | writes to cells inside expressions, sequencing on objects (`unsequenced`; same-object shapes rejected like the compilers), calls beside accesses, `return &local;` rejected (R14), `c ? owns : reads` path-precise and partners by path, function measures over memory |
| M4.5b1c1 Loops and jumps | 2026-10-29 | `m4.5b1c1-loops` | 0038 | loops over memory with automatic permission invariants ("every place held"), invariants and loop measures over memory, scope exits at jumps; written atoms in invariants cut to b1c3, function measures over memory to b1c2; the `address_of_local.c` port |
| M4.5b1b Pointer parameters and contracts | 2026-10-28/29 | `m4.5b1b-pointer-params` | 0037 | pointer parameters and results, `&param`; `owns`/`reads`/`live`/`old` in both spellings with ghost amounts; framing per path; `permission` at calls and returns; `pointer-comparison`; `restrict` on owned C parameters |
| M4.5b1a The model, address-taken locals | 2026-10-28 | `m4.5b1a-pointers` | 0032–0036 | the `Block` domain and its four axioms; address-taken locals as blocks, `&`, `*`, NULL, `==`, liveness and scope exits inside one function; `null-dereference`, `permission`, `lifetime`; C's incompatible pointer types as errors |
| S5 Linux CI | 2026-10-28 | `s5-linux-ci` | 0031 | `check-linux` on `ubuntu-26.04` in both tiers; Linux bootstrap; glibc `assert`; the same EDG language on both platforms; eight macOS-only tests tagged |
| M4.5a Memory-model design, `\|`/`^` masks | 2026-10-27 | `m4.5a-memory-design` | 0027–0030 | the C memory model designed and audited (E2, four axioms, member blocks, liveness), ownership annotations, vlibc; `\|`/`^` constant masks decided; S5 Linux CI next |
| M4.4d Exact sets, constant masks | 2026-10-27 | `m4.4d-exact-sets` | 0026 | preconditions and comma values get exact sets, with today's as the fallback; one `requires` per argument accepted; constant masks as remainders, the `bit_mask.c [c23]` hang gone; hooks `force_base_sets`, `drop_exact_set` |
| M4.4c Writes outside the chain, `void` `?:`, `-Wall` shapes | 2026-10-26 | `m4.4c-writes-void-warnings` | 0025 | finding A fixed (a flagged read is an access); writes and snapshots leave the chain; a `void` `?:` statement is an `if`; warnings in `verified`/`error` tests; the static-assertion cross-check in bodies; 13 `-Wall` shape rules; M4.4e planned |
| M4.4b2 Values the front end computes | 2026-10-26 | `m4.4b2-front-end-values`, `8bff6ba` | 0024 | static assertions checked as written (file scope through source sequence lists, now on); wrapped character literals are `conversion` errors; types computed from a constant rejected in `sizeof`/`typeof`/typedefs until M4.5e |
| M4.4b1 Bitwise operators | 2026-10-23/24 | `m4.4b1-bitwise`, PR #9 `effc63f` | 0023 | `& \| ^ ~` and `&= \|= ^=` exact (prelude functions per width with proved range postconditions); seven bitwise `-Wall` shapes rejected; multi-character literals refused; item 5's truth values one verdict in both modes |
| M4.4a No wrapping | 2026-10-23 | `m4.4a-no-wrapping`, PR #8 `00f60d2` | 0022 | unsigned arithmetic and conversions checked (`overflow`, `conversion`, `shift`), `c_wrap` gone; lowering's internal errors exit 3; enumeration `==`/`!=` with possibly negative operands rejected |
| M4.3b Calls in expressions | 2026-10-23 | `m4.3b-calls`, PR #7 `58d208c` | 0021 | calls in guarded operands and loop conditions; the dependency-chain rule (`unsequenced_calls`); explicit termination checks; `self_assignment` |
| M4.3a Side effects in expressions | 2026-10-22 | `m4.3a-side-effects`, PR #6 `d694606` | 0020 | assignments, `++`/`--` and commas as values; C's sequencing rule (`unsequenced_modification`); the first `-Wall` shapes |
| M4.2c Locals without an initializer | 2026-10-22 | `m4.2c-uninitialized-locals`, PR #5 `1c1cc29` | 0019 | category `uninitialized` by a ghost flag; jumps past such declarations; EDG #2656 fixed on fork branch `0022` (pin `f0ed5525af`); `-Wswitch`'s rules |
| M4.2b2 Enumerations | 2026-10-22 | `m4.2b2-enumerations`, PR #4 `4622259` | 0018 | enumeration types with value ranges, every conversion into one checked |
| M4.2b1 Named constants and case values | 2026-10-21 | `m4.2b1-named-constants`, PR #3 `ae2d2a4` | 0017 | `const`/`constexpr` globals and case values checked in each method's prologue |
| S3 Mutation triage | 2026-10-21 | `mutation-triage`, PR #2 `915e372` | — | 17 tests strengthened, 33 twins, 436 reviewed equivalents |
| S2 CI | 2026-10-21 | `ci`, PR #1 `e1f2860` | 0016 | `check.sh` on GitHub's `xcode-27` runner, required on `main` |
| S1 EDG upstream merge | 2026-10-21 | `edg-upstream-merge` | — (0001) | fork `cerastes` merged upstream `main`; 0020's Linux failure was stale box binaries |
| M4.2a Jumps and ends | 2026-10-21 | `m4.2a-goto-noreturn-assert` | 0015 | outward-and-forward `goto`, `noreturn` (category), trusted `abort`/`exit`/`_Exit`, `assert` as `check` |
| M4.1 Control flow | 2026-10-20 | `m4.1-control-flow`, `29a3dd3` | 0014 | `break`, `continue`, `do`, `switch`; the plan re-sequenced (new M4) |
| M3 close-out | 2026-10-20 | `worktree-m3-closeout`, `bfeda03` | — | M3 acceptance, mutation run |
| M2 re-look and M3 Scalar encoder | 2026-10-16/19 | — | 0010–0013 | VIR, emitter, driver, scalar encoding, termination |
| EDG bugs 0003–0007 | 2026-10-15 | fork branches | — | fixed, branches for the owner's PRs |
| M2 Annotation layer | 2026-10-14/15 | — | 0005–0009 | `[[cerastes::…]]` registration in the EDG fork, `<cerastes.h>`, the fork pin |
| M1.5 Documentation site | 2026-10-14 | — | 0004 | Antora docs at cerastes.dev/cerastes/doc/ |
| M1 IL inventory · Gate G1 | 2026-10-13 | — | 0002, 0003 | G1 PASS; in-process back end |
| M0 Bootstrap | 2026-10-13 | — | 0001 | toolchain pins, harness, `check.sh` |
