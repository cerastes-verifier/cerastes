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
`git show 9d98fe8:internal/status.md`, and M4.5d2a's `git show bb41d55:internal/status.md`.

## S6 — a tutorial and a developer guide (2026-11-04): done on branch `s6-tutorial`

Plan: `.claude/plans/s6-tutorial.md`. Record 0049. Read `internal/handoff.md` next: §2 (owner steps: this branch's PR, then
the website), §3 ("M4.5d2a follow-up" is still the next code work; "S6 notes"), §8 (questions 131–133).

**Owner decisions:** at planning (2026-11-04), the layout (the tutorial and the developer guide in this repository's
`doc/`, the website the front door: "I agree this is the right approach"), 131 (S6 runs before the M4.5d2a re-audit's
fixes), 132 (a minimal landing page on the website), 133 (a tutorial chapter lands with the session that completes its
concept); three Sonnet implementers. During the session: no full test suite, only the examples ("You don't need to run
the full testing suite. Just the examples so that you can insert their information").

**Done**
- **Record 0049** (amends 0004): one site, three sections of the one `ROOT` module (`pages/tutorial/`, the reference,
  `pages/dev/`), so the doc lint's `ROOT`-only rule and every 0004 item 7 check apply unchanged; the nav's three titled
  lists; the tutorial's form; the upkeep rule (question 133, item 4: the chapters owed by M4.5e1, M4.5f2, M4.6, M4.7, M5,
  M6, M7); the developer guide summarises and links; the website has links and no code.
- **Tutorial** (`doc/modules/ROOT/pages/tutorial/`, VerCors-style): an introduction and seven chapters (a first proof,
  integers, loops, calls, pointers, structures, the heap); 20 verified programs and 24 failing variants in
  `examples/tutorial/` (`<positive>_<what>_error.c`), every `.c` in both modes, the positives `native-ok`; the `console`
  blocks copied from runs at the tip (`build/s6/out/`, run in the examples directory; c23 and c++20 print the same).
- **Developer guide** (`pages/dev/`, eight pages): architecture, building, testing, soundness, adding a feature, the EDG
  fork, writing documentation; links to the canonical sources on GitHub; no `examples/dev/` files yet (the directory is in
  the policy lists for later).
- **Reference touch-ups:** `overview.adoc` rewritten (its status said pointer parameters were rejected and counted
  sixteen categories; now a summary without counts and "Where to start"); Installation links the tutorial; the nav.
- **Harness:** `examples/tutorial` and `examples/dev` in `DUAL_MODE_DIRS` and `NATIVE_POLICY_DIRS`, pinned in the
  self-test; `tests/README.md` names them.
- **Website** (`cerastes-verifier.github.io`, local `main` `45a3d2f`, not pushed): a paragraph on what cerastes does and
  three links (Tutorial, Reference, Developer guide) in place of "Under construction"; `npm run build` OK.
- `CLAUDE.md`'s user-docs line and `plan.md` (the S6 row, M4's Docs paragraph, the tutorial lines of M5, M6, M7).

**Checks run** (the owner asked for the examples only, so no `check.sh` and no full self-test):
`run_tests.py doc/modules/ROOT/examples/tutorial` 172 passed, 0 failed (native-ok: Apple clang 40, GCC 40);
`check_doc_examples.py` 0 problems (25 pages, 265 source blocks); `build_docs.sh` clean; `--html build/doc` 0 problems;
every xref fragment of the pages exists in the built HTML (a scratch check); `run_tests.py --check-headers` 3788 ok; the
self-test classes `NativePolicyTests`, `DualModeTests`, `DocBuildWarningsTests`, `RealDocTreeTests`,
`WalkerFingerprintTests` 35 OK (a full self-test run mid-session, before the pages included every example, failed only
in the doc-tree classes). **Not done:** the planned claims audit (started, stopped when the owner ended the session for a
flight; the owner runs it in another session, handoff §3 "S6 notes"). The lead reviewed the developer guide against the
code and fixed six claims (VIR's types, the native compilers, the trusted methods, two broken links, a dangling xref).

**Surprises**
- A contract is checked for every value it is about before the body: in an `ensures`, `r` is any value of its type, so
  `2 * r == n` and `r + b == a` overflow at the contract's line. The examples write `r == n / 2`, `r == a - b`; the
  integers chapter teaches it (`integers_unsigned_ensures_error.c`).
- Several variants report two errors: the contract's own `-x`, `a - b` or `a / b` beside the body's.
- `owns(*q)` dropped from `requires` alone leaves `ensures owns(*q)` rejected (`unsupported_annotation`), so the swap
  variant uses `reads(*q)`; a double `drop_cell(p)` is `permission` at the call (a direct double `free` is
  `invalid-free`); a dangling read is `permission`, not `lifetime`.
- `internal/soundness/`, which `plan.md` §0 and the M5 plan name, does not exist; `src/backend/register_attributes.h`'s
  header comment still says "eight attributes".

**Decision records added:** 0049-tutorial-and-developer-guide (amends 0004).

**First step for the next session.** The M4.5d2a follow-up, unchanged by S6: check CI on `main` (the nightly full check
after PR #28; the expected Linux red `reject_owning_system_redeclaration.c [c23]`, handoff §2), then fix the re-audit's B1
and S1 in `src/vir/lower.cpp` (and S2, S3; handoff §3 "M4.5d2a follow-up" holds them: `build/m4.5d2a/audit3.md` went with the removed worktree),
then M4.5d2v (record 0048; questions 129–130). M4.5d2v owes no tutorial chapter (record 0049 item 4); M4.5e1 owes the
arrays chapter.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
