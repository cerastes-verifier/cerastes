# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, M4.5a's
`git show 72e30b3:internal/status.md`, S5's `git show 794e960:internal/status.md`, M4.5b1a's
`git show bdc6bd0:internal/status.md`, M4.5b1b's `git show 8003dca:internal/status.md`, M4.5b1c1's
`git show eab4058:internal/status.md`, and M4.5b1c2's `git show a5e3e30:internal/status.md`.

## M4.5b1c3 — pointers to pointers, owning parameters, `*result`, written atoms (2026-10-30): done on branch `m4.5b1c3-pointers-to-pointers`

Plan: `.claude/plans/m4-5b1c3-pointers-to-pointers.md`. Read `internal/handoff.md` next: §3 "From M4.5b1c3", §4.2 (M4.5b2),
§8 (questions 78 and 79 are open).

**Owner decisions (2026-10-30):** atoms in a `do … while (0)` invariant stay rejected (question 73); `owns(**pp)` and
reads of `**pp` in contracts move to M4.5c (74); `owning` beside an explicit `owns(*p)` or `p == NULL || owns(*p)` is
merged (75); pointers to pointers at any depth (76); sessions sized to one context, under about 12 h unless a reason is
given (77, was ≤ 4 h). Step 1 consolidated the handoff and `plan.md` (PR #20 had merged as `a5e3e30`, so the branch
starts there, not from the local `7dda635` with the same tree).

**Done**
- **Record 0040** (accepted; revised after P62's hazards H1, H2 and A8; audit round 1 on the design: four blocking items,
  B1 a pointer loaded from a cell without case splits, B2 qualification conversions at depth writing a `const` object,
  B3 written-atom amounts that may be `none` and an unguarded de-duplication, B4 written atoms on places reached through
  a heap read, seven should-fix items and five notes, all taken; audit round 2 on the code: one blocking item, B1, a
  `const` object written through a pointer a trusted contract relates to it, a wrong verdict whose `int *` shape had
  verified since record 0037, fixed fail closed; four should-fix items, S1–S3 taken, S4 a pinned false alarm; one
  re-audit: one blocking item with two more routes to a `const` write, a qualification-adding cast or a `?:` in a
  contract and a trusted `owns` of a `const` place (both also verified since 0037), fixed fail closed and checked by
  the tests and the auditor's probes, not re-audited: the cap; one should-fix taken), amending 0029, 0032, 0037, 0038 and 0039: address-taken pointer variables as cells
  (`v_ptr_<t>`, `WF` in place of a range, case splits after every load), `T**` at any depth; pointer conversions only
  by adding qualifiers at the pointee's top level, in both modes, written or not, and contract comparisons across
  qualifiers rejected; `[[cerastes::owning]]` on parameters (registered target `p`, `RAF_NO_FLAGS`; desugared to `p ==
  NULL || owns(*p)` first in both clause lists, merged with an explicit `owns`, declarations matched by position);
  `*result` as a value (framed per path, `f__wf` ghosts keyed by the result); written `owns`/`reads`/`live` in loop
  invariants (`NOTAUTO` by place and liveness cell, guarded de-duplication, positivity rows, entry, back-edge and
  separation checks, case splits, framing, values kept as for automatic places); `owns(**pp)`, `live(*pp)` and `**pp`
  in contracts and invariants rejected naming M4.5c; nested dereferences of one pointer variable in the static half of
  sequencing (GCC's `-Wsequence-point`).
- **Code:** `src/vir/vir.h`, `vir.cpp` (`loop_atoms`, `atom_guard_shape`), `check.cpp` (nested pointers, cells of pointer
  type, invariant atoms, `Deref(Result)`, `Sequencer::object_of` per dereference depth); `src/vir/lower.cpp` (pointer
  fields and allocations, `WF` rows, loaded-pointer splits, `ptr_use` with `WF` for stores into pointer cells, the
  result ghost, `WAtom`s with their rows, checks and splits); `src/backend/encode.cpp` (`vtype`, `address_of`,
  `pointer_conversion`, `comparison`'s contract rule, `contract_read`, `invariant_atom`, `read_owning`, `apply_owning`,
  `clause_bool` only for clauses holding an atom), `register_attributes.cpp`, `annotation_il.h`,
  `annotation_checks.cpp`, `diagnostics.def` (`annotation_mismatch`), `il_query.{h,cpp}` (secondary declarations);
  `src/driver/verify_main.cpp` (#144 and #167 errors in C++ too); `include/cerastes.h` (`CERASTES_OWNING`).
- **Tests:** `tests/memory/` 45 positives (three of them question 68's moved tests, and the new port
  `verifast/swap.c`) and 101 negatives (58 error twins, 43 rejections); the ports `tests/memory/verifast/swap.c` (`swap`, its `swap(&x, &x)` aliasing-trap twin) and
  `address_of_local.c`'s `address_of_local` and `address_of_local_uintptr_t` (appended; the three existing `expect:`
  lines unchanged); a `T**` aliasing trap; every audit pin; VIR self-test `selftest_cases11.inc` (37 lowering cases;
  `VIR_CASES_MIN` 1354); three doc examples.
- **Docs:** Memory and ownership ("Pointers to pointers", owning parameters, `*r`, `T**` contracts, contract comparisons
  across `const`, written ownership in loops, the not-yet list), Annotations (owning parameters, atoms in invariants,
  declarations that disagree), Error messages (`permission`), Verifying; `internal/encoding.md` §3 and §8;
  `il-inventory.md` P62, S30 and its addendum.

**Flips (pre-approved):** retired `tests/memory/negative/reject_{pointer_to_pointer,address_of_pointer,atom_in_loop_invariant}.c`
and `reject_atom_in_loop_invariant_cpp.cpp`, each replaced by a positive of the same shape (`ptrptr_null.c`,
`ptrptr_address_compare.c`, `loop_atom_automatic{,_cpp}`); `reject_result_value.c` replaced by
`negative/result_value_unframed.c` (`permission` at the `ensures` and, audit round 1 S5, `postcondition` at the
return; one header line more, so both lines moved by one); question 68's three tests moved from `tests/memory/negative/`
to `tests/memory/`, gaining the written atom and re-expected `verified`, each with a `_no_atom` twin keeping the old
text and expectation; VIR self-test `mem_rej_pointer_to_pointer`, `mem_rej_address_of_pointer_variable` and
`mem_rej_atom_in_loop_invariant` became lowering cases. `reject_atom_in_do_once_invariant.c` stays (question 73). No
other existing test's `expect:` line changed.

**Acceptance**
- The baseline (`main` `7dda635`, the tree of `a5e3e30`, in the main checkout): 6815 passed, 0 failed, 1 skipped; VIR
  self-test 1317.
- **Viper against the baseline:** every one of the 2526 kept `.vpr` and `.map` files regenerated with the final binary:
  2520 byte-identical except `tests/memory/verifast/address_of_local.c` (both modes; the port gained two functions), and
  six without a source (question 68's three tests, moved to `tests/memory/`). No existing program's Viper changed.
- `tests/memory`, the memory fixtures and the memory, error-messages, verifying and annotations doc examples: 1736
  passed, 0 failed before audit round 2 (native-ok 364 each); after the re-audit's fixes, `tests/memory` and every doc
  example: 1858 passed, 0 failed (the two files rewritten during that run re-run on their own); VIR self-test 1354
  lowering and 237 rejection cases, 0 problems; `build_docs.sh` and `check_doc_examples` (source and `--html`) green.
- `check.sh` (the full check, `CERASTES_CHECK_BASE` unset; twice: the first run stopped at the lint step on two
  `++…++` passthroughs in the new doc text, which `check_doc_examples`' source lint refuses, fixed by naming the
  attribute and `pass:[…]`): **all green** in about 55 min. Harness self-test 1024 tests (1 skip), 79, lint self-test
  307, 291; tests **7259 passed, 0 failed, 1 skipped** (native-ok runs: Apple clang 1391, GCC 1391); Viper lint 2718
  files clean; VIR self-test **1354** lowering and 237 rejection cases, 0 problems, all 1354 through Silicon; walker
  oracle 136 goldens, 1924 facts, 0 failed; docs green. `build/vpr` against the baseline: only added files, question
  68's three moved tests and the `address_of_local.c` port. No Linux run (CI covers it).

**Surprises**
- P62 H1/H2: EDG in this configuration accepts C++'s const-laundering `int**` → `const int**` with warnings, and C's
  `int**` → `const int* const*` with none; audit round 1 B2 then showed that an explicit cast, which no compiler warns
  about, would write a `const` object once field names ignore `const`, so the conversion rule stays by types.
- Audit round 2 B1 and its re-audit: a trusted contract equating pointers across `const` (directly, through a cast or a
  `?:`), or owning a `const` place, does what a conversion does; such shapes verified a write to a `const` object since
  record 0037. Contracts now reject them (fail closed) until M4.6 models `const` objects (question 78).
- `RAF_ENTITY_ATTACHED` makes EDG abort at startup for a registration whose only target is `p`.
- Written atoms needed 0038's value rule too (found end to end: the re-expected `loop_invariant_reads_unowned.c`).
- The front end's `clause_bool` change for atoms under C's `&&` changed two existing programs' C23 Viper (equivalent,
  not byte-identical); restricted to clauses holding an atom, they are byte-identical again.

**Decision records added:** 0040-pointers-to-pointers-owning-written-atoms.

**First step for the next session (M4.5b2).** Confirm the owner pushed this branch and merged its PR with both CI jobs
green, and answered questions 78–79; then plan M4.5b2 from handoff §4.2 (globals, `static` locals, initialization
through pointers), saving the plan as `.claude/plans/m4-5b2-<topic>.md`.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
