# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, M4.5a's
`git show 72e30b3:internal/status.md`, and S5's `git show 794e960:internal/status.md`.

## M4.5b1a — the memory model and address-taken locals inside one function (2026-10-28): done on branch `m4.5b1a-pointers`

Read `internal/handoff.md` next: §3 "From M4.5b1a", §4.4 (M4.5b1b and b1c, with the pre-approved flips), §8 question 64.

**Owner decisions (2026-10-28, M4.5b1 planning):** M4.5b1 is three sessions (b1a, b1b, b1c; a design pass put the single
session at well over 27 agent-hours for `src/vir` and `src/viper` alone); b1b and b1c may retire or re-expect exactly
the b1a rejection tests listed below for them; an existing test whose only point was "pointers are rejected" is retired
where its construct is accepted (none was: `reject_pointer.c` is a pointer parameter, still rejected, now b1b's); loops
over memory in b1c get hybrid permission invariants; the 0027 mask timeouts on GitHub's macOS runner are ignored for
now (handoff §3); handoff questions 59–63 as recommended, with `ssh matt@clang4.cpp.al` replacing the box.

**Done**
- **Record 0032** (amends 0011 items 9–10, 0012 item 12, 0019 item 13, 0020 item 4, 0028 items 1 and 4), revised after
  two Opus audit rounds: pointers to integers, `bool` and enumerations as the types of locals and expressions;
  address-taken locals (the IL's `address_taken`, P51/P57) as one-slot blocks from a trusted `alloc_<t>`, allocated at
  block entry (C23 6.2.4p6) and before a switch's dispatch; `&x`, `*p`, statement-level writes (`*p = e;`, `*p op= e;`,
  `(*p)++;`), NULL in every spelling, `==`/`!=`, truth tests, pointer locals, `?:`; `CERASTES_CHECK`/`assert` may use
  them; explicit checks `null-dereference`, `permission`, `lifetime` (every use of a pointer value, a discarded one
  included), the flag (`uninitialized`); scope exits at fall-through and `return`; the one-past-the-end comparison row
  as `internal`; the conversion allow-list on EDG's types (null to `T*`, adding qualifiers) and C's #42/#144/#167/#513
  as errors in verify mode (audit round 4 of M4.5a's reopened item); rejected until b1b/b1c/later: pointer parameters,
  results and globals, `&param`, `T**`, `void*`, struct/union/function pointers, `restrict`/`volatile`/`_Atomic`
  pointers, pointer casts, address constants, string and compound literals, loops touching memory, `return` in a loop
  with a live cell, jumps out of a cell's block, cell writes inside expressions, pointers in contracts, invariants and
  measures.
- **Records 0033–0036:** the `Block` domain's four axioms (`loc_inv`, `loc_nonnull`, `member_inv`, `member_lv`), each
  with its lint tag; the 12 `s25_*` fixtures moved to 0028's `member_lv` trigger (55 fixtures pass, no outcome changed).
- **Code:** VIR `Ptr`, `Null`, `AddrOf`, `Deref`, `Store`, `Var::address_taken`, `Block::end_loc`, `memory_problems`
  (`vir.h`, `check.cpp`); the encoder (`encode.cpp`); lowering of cells, checks, liveness and scope exits with a typed
  `LVar` (`Ref`), `Field`, `PermOf`, `Acc`, `FieldAssign`, `Exhale` (`lower.cpp`, `lowered.h`); the memory prelude
  (fields, the domain with tags, `alloc_<t>`) emitted only when used (`emit.cpp`, `prelude.cpp`); a pointer `?:` is an
  `if` (Silicon refuses an `ite` field receiver); C's `--diag_error=42,144,167,513` (`verify_main.cpp`).
- **Lint:** the domain pinned to `Block`/`domain-ok 0028`, exactly the four axioms with their records and texts,
  `alloc_<t>`'s contract pinned, `new` refused, field flags.
- **Harness:** three categories in the five copies; `tests/memory` an area (dual mode, native policy, twins);
  `MemoryAreaTests`.
- **Tests:** `tests/memory/` (23 positives, 69 negatives and pins); the VIR self-test's `selftest_cases7.inc` (36
  lowering, 20 rejection cases; `VIR_CASES_MIN` 1159).
- **Docs:** the new page "Memory and ownership"; Error messages: `null-dereference`, `permission`, `lifetime`
  ("sixteen"); Verifying's list of what is not supported; `internal/encoding.md` §2–§4; P51 (never committed by M4.5a),
  P56, P57 and the fixture comparison in `il-inventory.md`.

**Pre-approved flips** (owner): **b1b** may retire or re-expect `tests/scalar/negative/reject_pointer.c` and
`tests/memory/negative/reject_{address_of_param,implicit_function_pointer_argument,pointer_param,pointer_result}.c`;
**b1c** `tests/memory/negative/reject_{address_of_pointer,break_out_of_switch_with_cell,cell_in_invariant,cell_in_measure,cell_postfix_value,goto_out_of_cell_scope,loop_cell,loop_pointer,pointer_to_pointer,return_in_loop_with_cell,write_in_assignment_value}.c`.
The other pins name later milestones (M4.5b2, M4.5c, M4.5d2, M8) and are not pre-approved.

**Acceptance**
- The baseline (`main` `794e960`, from the main checkout's own build): 5766 passed, 0 failed, 1 skipped.
- `check.sh` (the full check, once, after both audit rounds' fixes): **all green**. Harness self-test 1024 tests (1
  skip), lint self-test green; tests **6081 passed, 0 failed, 1 skipped** (native-ok runs: Apple clang 1136, GCC 1136);
  VIR self-test **1159** cases (229 rejection cases); walker oracle 136 goldens, 1924 facts, 0 failed; docs green.
- **Viper unchanged for every existing program:** `diff -r` of `build/vpr` against the baseline: no file differs; the
  only additions are `tests/memory/` and the new doc examples (error-messages, memory). The VIR self-test output: no
  case differs; 36 `mem_*` cases added.
- `tests/memory/`: every test gives exactly its `expect:` lines in both modes (also run one by one by il-analyst,
  edg-backend and the auditor); `tests/viper-smoke/memory`: 55 passed after the trigger change.
- The Linux job was not run in the session (no host work); the PR's CI runs it.

**Surprises**
- The first baseline in the worktree was invalidated: a stale EDG stamp (bootstrap refreshed it), then an agent's
  build relinked `build/cerastes` mid-run. The baseline was rerun from the main checkout's build, which also freed the
  worktree for the implementers.
- P51 was never committed by M4.5a (only `build/m4.5a/P51.md`); P52, P54 and P55 are still only there (handoff §3).
- Silicon refuses a Viper `ite` of `Ref`s as a field receiver, hence the `if` for a pointer `?:`.
- Two audit rounds found blocking holes: `restrict` on the pointer itself, and a discarded pointer value (`(void)p;`)
  without its liveness check; both fixed, the second re-audited.
- Two orphaned Z3 processes from earlier sessions (20–22 h old) were stopped.

**Decision records added:** 0032-pointers-inside-one-function, 0033-axiom-loc-inv, 0034-axiom-loc-nonnull,
0035-axiom-member-inv, 0036-axiom-member-lv.

**First step for the next session (M4.5b1b).** Confirm the owner pushed this branch and both CI jobs are green on its PR
(the 0027 mask timeouts on macOS are the known flake, handoff §3); then handoff §4.4, "M4.5b1b".

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
