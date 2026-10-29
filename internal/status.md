# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, M4.5a's
`git show 72e30b3:internal/status.md`, S5's `git show 794e960:internal/status.md`, and M4.5b1a's
`git show bdc6bd0:internal/status.md`.

## M4.5b1b — pointer parameters, results and contracts (2026-10-28/29): done on branch `m4.5b1b-pointer-params`

Plan: `.claude/plans/m4-5b1b-pointer-params.md` (from now on each approved plan is saved there, named by milestone).
Read `internal/handoff.md` next: §3 "From M4.5b1b", §4.4 (M4.5b1c, with its pre-approved flips), §8.

**Owner decisions (2026-10-28/29):** no cuts and no time cap (pointer results and `&param` stayed in b1b); the framing of
contracts decided per path by Silicon; a missing permission at a call or a return is `permission`; handoff question
64: `restrict` accepted only on a parameter the contract owns (tightened after audit round 2: every other pointer
parameter of the same type needs an atom of its own); approved plans in `.claude/plans/<milestone>-<topic>.md`; the
exponential cost of aliasing `reads`/`live` parameters documented, not redesigned; `tests/scalar/negative/
reject_header_pointer.c` (and its header `bad_pointer.h`) retired, its construct now accepted.

**Done**
- **Record 0037** (accepted; revised after audit round 1 on the design, S27, audit round 2 on the code and its
  re-audit), amending 0008, 0011, 0012, 0028, 0029 and 0032: pointer parameters and results with `WF`; `&param`;
  `owns(*p)`, `reads(*p)`, `live(p)`, `old(*p)`, `*p` in contracts in C++ (`cerastes::`) and C (`cerastes_owns`,
  `cerastes_reads`, `cerastes_live`, the `cerastes_old` macro over `__typeof__`); one ghost `Perm` amount per atom, the
  caller's amounts `(g ? m : write)/(N + 1)`, sums at returns and calls; framing checked per path in `f__wf` with each
  contract heap read guarded by its own ite chain; explicit `permission` checks at calls and returns, separation,
  `lvloc` and cell case splits (at calls, after pointer-returning calls, at entry); `pointer-comparison` (code only;
  contract `==` is the model's identity); by-name cell checks as `permission`; `restrict` on owned C parameters; the
  call placement rule until b1c.
- **Code:** `include/cerastes.h` (C spellings, `cerastes::live`); `src/backend` (encode.cpp: signatures, atoms,
  `finish_atoms`, `contract_truth`, placement, `restrict`, `old` forms, `measure_rejected_`; `unsupported_annotation`);
  `src/vir` (`Atom`, `contract_atoms`, `restrict_backed`, `placement_problems`, the `no_ptr_` split; lowered `Perm`,
  `Old`, `Acc` with an amount, `PermDiv`, `CaseSplit`; lower.cpp's contract modes); `src/viper/emit.cpp`.
- **Lint:** ghost amounts pinned (`none < a && a < write` before any use), `wildcard` refused.
- **Harness:** `pointer-comparison` in the category copies; README.
- **Tests:** `tests/memory/` 16 positives and 45 negatives and pins (both modes; `restrict` C-only); the VIR self-test's
  `selftest_cases8.inc` (45 lowering, 7 rejection cases), the four `mem_rej_*` flipped, `VIR_CASES_MIN` 1204.
- **Docs:** Memory and ownership ("Pointer parameters and contracts", the limits), Annotations (the spec-function
  table), Error messages ("seventeen", `pointer-comparison`, `permission`'s new places), Verifying; `internal/encoding.md`
  §2–§3; `il-inventory.md`: P52, P54, P55 (copied from M4.5a's evidence), P58, S27.

**Flips (pre-approved):** retired `tests/scalar/negative/reject_pointer.c`,
`tests/memory/negative/reject_{pointer_param,pointer_result,address_of_param}.c`; by owner decision during the session,
`tests/scalar/negative/reject_header_pointer.c` and `tests/scalar/headers/bad_pointer.h`.
`reject_implicit_function_pointer_argument.c` unchanged. b1a's memory maps changed only in the two rows' category
(0037 items 11–12).

**Pre-approved for b1c** (new b1b pins it lifts): `tests/memory/negative/reject_{atom_on_result,call_beside_access,call_compound_target,ensures_atom_no_partner,result_value,two_memory_calls,heap_read_decreases}.c`,
plus b1a's list (handoff §4.4).

**Acceptance**
- The baseline (`bdc6bd0`, a temporary worktree `.claude/worktrees/b1b-base`): 6105 passed, 0 failed, 1 skipped; VIR
  self-test 1159.
- `check.sh` (the full check, once, `CERASTES_CHECK_BASE` unset, 49 min): **all green**. Harness self-test 1024 tests
  (1 skip), lint self-test 285, mutate self-test 291; tests **6298 passed, 0 failed, 1 skipped** (native-ok runs: Apple
  clang 1183, GCC 1183); VIR self-test **1204** lowering and 232 rejection cases, 0 problems, lint clean; walker oracle
  136 goldens, 1924 facts, 0 failed; docs and `check_doc_examples --html` green.
- **Viper against the baseline:** `diff -r` of `build/vpr`: no `.vpr` differs and none is missing; 192 files added (the
  new tests and doc examples); 122 `.map` files of b1a's memory tests and doc examples differ only in two row kinds,
  308 by-name cell rows `internal` → `permission` and 36 one-past-the-end rows `internal` → `pointer-comparison`
  (record 0037 items 11–12). The VIR self-test output: 90 files added (45 `ptr_*` cases), 35 `mem_*` maps differ only in
  the same two row kinds (113 rows), no `.vpr` differs.
- `tests/memory`, the scalar negatives and the memory and error-messages doc examples: 2847 passed, 0 failed, run
  alone before `check.sh`. The Linux job was not run in the session (no host work); the PR's CI runs it.

**Surprises**
- Silicon refuses a heap read under a disjunctive guard (S27 P6): one guard per disjunct.
- Audit round 1 found two false proofs in the first framing design (a whole conjunct guarded; a separation fact across
  a `requires` atom and its partner); audit round 2 found two exit-3 shapes and an unsound `restrict` (permissions
  follow cells, not provenance); its re-audit found the entry cell splits' path explosion, moved to the calls.
- A pointer result known only by a disjunction (`r == p || r == q`) needs case splits before Silicon can read through
  it; contract comparisons with a result had to become model identity.
- Silicon's time grows exponentially with aliasing `reads`/`live` parameters of one type (six time out).
- Heavy machine load (averages up to 215) from parallel Silicon runs distorted timings; quiet re-measurements are in
  record 0037.

**Decision records added:** 0037-pointer-parameters-and-contracts.

**First step for the next session (M4.5b1c).** Confirm the owner merged PR #15 and pushed this branch with both CI
jobs green on its PR; then plan b1c from handoff §4.4 (its planning decides whether it splits), saving the plan as
`.claude/plans/m4-5b1c-<topic>.md`.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
