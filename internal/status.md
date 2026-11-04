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
`git show 7f11706:internal/status.md`, and M4.5c2b's `git show d26a87c:internal/status.md`.

## M4.5d1 — vlibc in cerastes (2026-11-03): done on branch `m4.5d1-vlibc`

Plan: `.claude/plans/m4-5d1-vlibc.md`. Read `internal/handoff.md` next: §2 (owner steps: PR #26 first, then this branch's
PR), §3 (notes by session; d2's new notes), §4.2 (M4.5d2: the dependency map and the d2a/d2b proposal), §8 (no question
open).

**Owner decisions (2026-11-03):** at planning, d1 alone with d2's preparation (question 113), reserved names generated from
the pinned vlibc headers (114), a contract on a reserved name outside vlibc rejected even when identical (115). After audit
round 1: an identical contract after vlibc's header accepted, the front end discarding it (116); definitions of reserved
names and contracts on them rejected for C linkage in any namespace and C++'s global namespace, four pins re-expected
(117). After the first run of the new tests: the two alias pins re-expected `unsupported_attribute` (118); after the first
Linux run, the three glibc `assert` near misses gain a `side_effect_not_evaluated` expect line at their line (119). The owner
accepted the Linux host's key during the session. The session ran in the worktree `.claude/worktrees/m4.5d1` (from
c2b's tip `d26a87c`, stacked on PR #26), its baseline in the detached `.claude/worktrees/m4.5d1-base`; vlibc's half was
already vlibc's `5962865` (pushed).

**Done**
- **Record 0046** (accepted; written before code, revised after P71, audit round 1 on the draft (B1 trusted calls
  redirected by aliases, asm labels and pragmas, a hole that predates vlibc; B2 question 116; S1–S9, N1–N12), audit
  round 2 on the code (B1 vlibc's contract trusted on a C++-linkage `::exit` after a guard-skipped header, B2 the
  redirection check skipped for functions created early; S1–S3, notes), and one re-audit (one more blocking item: a
  definition or variable taking a trusted routine's symbol by an asm label; a `static` function before the header);
  every blocking item reproduced natively and fixed): the pin, the include order, the library class by path, trust from
  vlibc's tags, "does not return" from vlibc, reserved names, attributes by position, redirections.
- **Code:** `third_party/vlibc` at `5962865` with `scripts/vlibc_tree.py` (the pin guard) and `CERASTES_VLIBC_SRC`;
  `scripts/vlibc_contracts.py` (the reserved `{name, header}` table generated at configure time; `check` against
  `tests/vlibc/trusted_contracts.txt`, the pin audit as a check; shim headers refused if they hold directives other than
  the guard, comments in trusted declarations or forms its scanner does not model); `scripts/check_vlibc_headers.sh`
  (vlibc's headers through `cerastes-fe` as non-system headers with the driver's own arguments, from the hidden
  `--cerastes_print_edg_command`); the driver (vlibc's directories first as `--sys_include`, `-I-` refused, `--version`
  names vlibc, the `library_fixture` self-test hook); the backend (`VerifyConfig::library_dirs`, `in_library_file`,
  `SrcPos::in_system` by path in verify mode; in `encode.cpp` `lib_trust`, `check_reserved_library`,
  `check_routine_attributes`, `reject_redirected_call`, `reject_asm_label_declaration`, the tag reader and the contract
  macro scan; `trusted_libc_reason` removed); the glibc `assert` matcher's `sizeof` operand (0031 item 10).
- **Tests:** `tests/vlibc` (positives and twins; pins for reserved names in every order and mode, definitions, a
  namespace's `extern "C"`, every redirection form, symbol stealing, `static` before the header, macros, the guard
  defined, the Linux items); `LibraryFixtureTests`, `VlibcTreeToolTests`, the check.sh wiring model and the real-driver
  `-I-` and guard-skip tests in `test_run_tests.py`; `test_vlibc_contracts.py`; 3 doc examples.
- **Docs:** Verifying a program ("The C library: vlibc", library files), Annotations (reserved names), Building annotated
  code with ordinary compilers (vlibc's include path), Installation; `internal/encoding.md`; `il-inventory.md` P71 (macOS
  and Linux; P55's `-I` reasoning corrected).
- **d2's preparation** (question 113): handoff §4.2's dependency map and d2a/d2b proposal; P71 (d), the allocation
  functions as both platforms declare them.

**Flips (questions 117, 118):** `noreturn_user_abort.cpp`, `noreturn_user_abort_after.cpp`, `noreturn_other_signature.cpp`,
`noreturn_other_signature_after.cpp` re-expected `rejected cerastes-annotation_reserved_library` (and their 17 entries
removed from `tests/mutants-equivalent.txt`); the new `reject_redirect_alias.c` and `reject_redirect_alias_cpp.cpp`
re-expected `unsupported_attribute`; the new `reject_assert_glibc_{callee,sizeof_operand,third}.c` gain a second expect
line (question 119). No other `expect:` line changed.

**Acceptance**
- The baseline (`d26a87c`, detached worktree): 9434 passed, 0 failed, 1 skipped; 3643 kept programs; VIR self-test 1493.
- **Viper against the baseline** (`build/m4.5d1/vprdiff.py`, after the trust change): 7170 files identical; 108 allowed
  (the 54 programs calling `abort`/`exit`/`_Exit`: the tag `libc:` → `vlibc:`, their methods' `.map` rows at vlibc's
  lines, one well-formedness method each); 0 unexpected; 8 missing (the four re-expected pins); 44 added. VIR self-test
  byte-identical (1493 cases).
- `check.sh` (the full check, `CERASTES_CHECK_BASE` unset), green on the first run at `31ee820`: vlibc tree clean at
  `5962865`; vlibc's headers through `cerastes-fe` as non-system headers clean (4 compilations); harness self-test 1108
  tests, 79, lint self-test 389, mutate self-test 292, vlibc contracts self-test 82; tests **9540 passed, 0 failed, 9
  skipped** (8 `platform: linux`; native-ok runs: Apple clang 1795, GCC 1795); Viper lint 3667 files clean; VIR self-test
  **1493** lowering and 253 rejection cases, 0 problems, all through Silicon and lint clean; walker oracle 136 goldens,
  1924 facts, 0 failed; docs green (215 code blocks); EDG tree clean at `f0ed5525`. The Linux test fixes after it
  (`0ce70e9`, test files only): the changed files re-run on macOS, 8 passed, `--check-headers` 3535 ok.
- **Linux** (`matt@clang4.cpp.al`, container `cerastes-m4.5d1-p71` from a clean `ubuntu:26.04` with S5's recipe: glibc
  2.43, clang 21.1.8 over libc++ 21, gcc-16; clean bootstrap at `45f655b`): P71's Linux half; at `45f655b`
  `tests/memory` 3229 passed, `tests/scalar` and the doc examples all passed; at `0ce70e9` `tests/vlibc` **111 passed,
  0 failed** (the `platform: linux` tests included), `check_vlibc_headers.sh` clean (#541 suppressed in C++),
  `vlibc_contracts.py check` clean, `LibraryFixtureTests` and `VlibcTreeToolTests` 53 OK. The first Linux run of the new
  tests (at `31ee820`) found 9 failures, all in the tests (a missing glibc `noexcept`, a wrong twin, question 119),
  fixed in `0ce70e9`.

**Surprises**
- P55's "only a leading `--sys_include` shadows `<stdlib.h>`" was the driver's own argument order; but a `-I` shim would
  be system in some C++ units and not others (EDG flags what a system header includes), so `--sys_include` stayed.
- In C++ EDG moves a routine's declaration position to a later non-defining redeclaration with a parameter (and keeps the
  later exception specification: glibc's `exit` loses `noexcept`), so trust is keyed on each contract attribute's own
  position.
- The audits found three ways to send a trusted call to code that returns (aliases and pragmas; an early-created
  function; a definition or variable taking the symbol by an asm label), all older than vlibc, all reproduced natively.
- EDG silently discards an identical later contract, so question 115 could not be enforced in that one order (116).

**Decision records added:** 0046-vlibc-in-cerastes.

**First step for the next session.** Confirm the owner merged PR #26 (M4.5c2b) and this branch's PR with both CI jobs
green; then plan M4.5d2 from handoff §4.2 (the dependency map and the d2a/d2b proposal, P71 (d)) and §3 "M4.5d2",
checking P4222R3 against records 0029 and 0042, saving the plan as `.claude/plans/m4-5d2-<topic>.md`.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
