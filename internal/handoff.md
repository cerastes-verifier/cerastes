# Handoff after M4.4c — writes outside the chain, `void` `?:` statements, `-Wall` shapes (2026-10-26) — and S4, CI tiers (2026-10-27)

The next session reads this file after `CLAUDE.md`, its own section in `plan.md`, `internal/status.md` and
`internal/decisions/`. `status.md` has the formal entry. This file adds what a cold start would otherwise have to
rediscover. It was trimmed at the end of M4.4c (owner, 2026-10-26) to what is live: the full text up to M4.4c, with the
finished sessions' plans, results and answered questions, is `git show 1fcc43b:internal/handoff.md`; earlier handoffs
are `git show 60d3fbe:internal/handoff.md` (M4.1) and `git show a4da52c:internal/handoff.md` (M3 close-out).

## 1. State

M4.4c is done on branch `m4.4c-writes-void-warnings` (based on local `main` `8bff6ba`, which merges M4.4b2; result in
`internal/status.md`). Record 0025:
- **Finding A** (planning review): an `uninitialized` check inside a guarded operand could be emitted after a later write
  of the same local (`int x; int y = (x = (g(f(a) > 0 && x > 0), 2));` verified). Fixed: the check's set is an access
  of the local.
- **Writes and snapshots leave the chain** (old §11 question 28): `(x = f(a)) + (y = g(b))` verifies.
- **A `void` `?:` in statement position is an `if`** (question 30).
- **`verified` and `error` tests may list front-end warnings** (question 31), from a closed list (#187 today).
- **The static-assertion cross-check covers function bodies** (question 40).
- **The `-Wall` shapes** (question 37, and five more families the owner added during the session): 13 rules, R1–R13.

The EDG pin is unchanged (`f0ed5525af`); the EDG box session (§4.3) is still pending. M4.4b2's PR #10 was merged on
GitHub during the session; this branch starts from local `main` `8bff6ba`, the same tree.

**S4 (2026-10-27), branch `s4-ci-tiers`** (from local `main` `cfceccb`, which merges M4.4c; decision 0016 item 14): CI
runs the full `check.sh` every night at 07:00 UTC and on `workflow_dispatch`, and a PR tier on pull requests and pushes
to `main` (`CERASTES_CHECK_BASE`; `run_tests.py --pr-base`). It started from M4.4c's PR #11, whose CI run failed on one
test: `tests/scalar/bit_mask.c [c23]` hit the driver's 300 s Silicon limit. That is a fragile Z3 query, not M4.4c (§3,
"From S4"; §8 question 47).

## 2. Owner steps after M4.4c, in this order

1. **Handoff answers.** Your old §11 answers 40–42 were uncommitted in the main checkout's `internal/handoff.md`; this
   branch carries them. Drop the main checkout's copy before merging:
   `git -C ~/Documents/cerastes/cerastes checkout internal/handoff.md`.
2. **M4.4b2** is merged (PR #10). If `main` on GitHub and local `main` differ only by that merge commit, fetch and
   merge before opening this branch's PR.
3. **This branch.** Push it and open its PR: `git push origin m4.4c-writes-void-warnings` (the session never pushes).
   CI runs `check.sh` as locally (the suite grew by about 230 files).
4. **Questions.** Answer §8 (questions 43–48). Then M4.4d (§4.1, record 0026), then M4.4e (§4.2).

**After S4:**
5. **PR #11's red run** (run `40434946397`) is the `bit_mask.c [c23]` hang of §3 ("From S4"): its Viper is
   byte-identical to the local one and to M4.4b1's, so M4.4c did not cause it. Decision 0016 item 10 leaves a rerun or
   a merge over it to you (§8 question 47).
6. **This branch** contains M4.4c (it starts from local `main`). Merge PR #11 first, then push and open the PR:
   `git push origin s4-ci-tiers`. That PR's run is the first PR-tier run (a pull request runs the workflow of its merge
   commit); its job time gives the measured number for 0016 item 9. The first full nightly runs at 07:00 UTC after the
   merge.

## 3. Open audit notes (condensed; the full notes are in the commit named above)

**From M4.4c** (audit rounds 1 and 2; items not taken):
- *Shapes accepted and listed* (record 0025 item 6, "still accepted"): `RED == RED` (the front end folds it to 1 without
  an expression); clang's `-Wparentheses-equality`, `-Wswitch-bool`, `-Wfor-loop-analysis`, `-Wmisleading-indentation`
  (clang and GCC), GCC's `-Wmultistatement-macros`, clang's `-Wempty-body` on loops; R10 with a named constant
  (`uc == DEF`, `const int DEF = 300;`, clang's C++ mode warns) or a constant expression (`uc == 256 + 44`); R7 does
  not look into a `?:` arm that is a comma ending in a constant. All are for **M4.4e** (§4.2).
- *Rejected more broadly than the compilers, listed:* R13's `;` on the line after the condition and an empty macro
  invocation as the body (`if (x > 0) LOG("positive");`), since the IL keeps no position for the `)`; R7's `*` through
  both `?:` arms in every conversion to `bool` (GCC's C mode only); R9 on a 0 or 1 right operand from a macro
  (`x && ONE`) and R11 on an operand from a macro under a written `&&` (`x > LIM && x < 3`), where clang is silent
  (the re-check after audit round 2).
- *Rules that skip annotations:* R10 and R11 apply in code only (wrong or vacuous contracts are reported by
  verification; error tests use them on purpose); R5 and R9 apply in annotations whatever the macros. §8 question 44.
- *Untested accepted shapes:* `else ;` and `do ; while (c);` (GCC's `-Wextra` warns, so no `native-ok` positive can
  hold them).
- *Mode differences, not meanings:* `x = ((void)(a > 0 && x > 0), 2)` reports `uninitialized` in C and is rejected in
  C++ ("comma … folded to a constant"); `x ? RED : GREEN` as a value is accepted in C and rejected in C++ (0018 item 8).
- *Fail-closed checks of item 2* now read the placement emission uses, and three VIR hooks reach them; a set computed
  too large (the unsound direction) is still invisible to them (M4.3b note 4): the pins guard it
  (`call_write_noreturn_divzero.c`, `never_returns_call_divzero.c`, …).
- *Planning's finding A* existed since M4.3a; no test had the shape. M4.5b must revisit item 2's placement (callees that
  see globals or address-taken locals) with item 1's kinds of access.
- *From the mutation review:* R1–R13 do not reach the body of a `pure` or `predicate` function (not verified before
  M4.5f; the Verifying page says so), and `static_assert_pure_body.c`'s listed entry goes stale then; the doc example
  `verifying/void_conditional.c` keeps a redundant requires conjunct `(c == 0 || n > 0)` (four listed equivalents; the
  `m > 0` pattern of `void_conditional_for.c` would make it needed); a body `p | q` against an exact `&` contract runs to
  the timeout (0023's documented limit), so `not_eq_and_bools` states its contract with `&&`.

**From S4** (the CI failure on PR #11, investigated on the owner's Mac; nothing changed yet):
- *The `bit_mask.c [c23]` hang.* One Z3 query in `f_low_word__1__wf` (the well-formedness method of `low_word`'s
  contract), the conversion bound `!((c_uand_64(p_v__0, 4294967295) < 0) || (4294967295 < c_uand_64(...)))`, never
  answers: `c_uand_64` unfolds 64 halvings, and whether Z3 gets through depends on what the same prover process checked
  before. Silicon's parallel verifiers make that order vary, so the hang is intermittent (CI run `40434946397`; 1 of 48
  local runs at 12 at once), and with `--numberOfParallelVerifiers 1` it is deterministic in c23 (10 of 10) and absent
  in c++20 (0 of 5: c23's `(cond ? 1 : 0) != 0` wrapping changes the history). Silicon's `assertTimeout` defaults to 0,
  so nothing stops the query; Z3 was busy, not deadlocked (jstack: the worker in `ProverStdIO.readUnsat`). Its twin
  `negative/bit_mask_post.c [c23]` has the same `low_word` and hangs the same way; a one-verifier sweep of the 89
  programs that use the bitwise functions found no other hang (the 128-bit tests are slow, 43–75 s, but finish).
- *Candidate fixes* (§8 question 47): (1) the encoder writes a mask by `2^k - 1` as `x % 2^k` and by `~(2^k - 1)` as
  `x - x % 2^k` (exact identities on unbounded integers, since Viper's `%` by a positive divisor is the non-negative
  floor remainder; for exactly those constants with k at most the width; the conversion checks unchanged): on scratch
  copies 5 of 5 pass with one verifier and 16 of 16 with random seeds (slowest 14.1 s against 42.7 s); it amends 0023
  and needs a must-verify/must-fail pair and an audit. (2) Silicon options: `--numberOfParallelVerifiers 1` makes every
  run reproducible (this test would fail every time instead of sometimes), and `--assertTimeout` bounds each query by a
  resource limit and fails closed (exit 3); both change solver behaviour for the whole suite. (3) Splitting the test
  shortens the history but leaves the query fragile.

**Carried from earlier sessions** (each still open; the session that wrote it in brackets):
- *Recognition by name or shape.* `assert` is recognised by its macOS expansion and `__assert_rtn` (S3 note 1: a user
  C++ function named `assert` would be bucketed by `mutate.py`). The character-literal trigger compares VIR type names
  (`int`, `char`, `wchar_t`); `int_kind` would be sturdier (M4.4b2 note 3).
- *Messages.* Rejection texts still end "(decision 0012)" where a later record decides them (M4.2a–M4.2c notes); a
  block-scope `extern const` naming an internal-linkage C++ constant is called "a static local variable" (M4.2b1 note 2);
  `c >= +GREEN` is accepted in C and rejected in C++ (the front end drops the `+` in C; M4.2c note 1); C's `case -1:`
  over an enumeration prints 4294967295 in `case_not_in_enumeration` (M4.4a note 7); for `d == (x ? (y, RED) : GREEN)`
  C reports one diagnostic and C++ two (M4.4b1 note 2); lowering's internal-inconsistency diagnostics have no
  position.
- *Fail-closed corners and unreached guards.* `names_object` returns false on an unknown node kind (M4.2b1 note 3); a C
  case value of an unknown shape is rejected (M4.2b2 note 4); `?:` of enumeration type is rejected in C++ while C reads
  `x ? RED : GREEN` as `int` (0018 item 8; EDG rejects it in C++, accepts it in C); defensive branches with no test:
  three in M4.1's encoder (an undetermined `break`/`continue` target, a non-integer scrutinee, a case value of another
  type), a declaration directly in an unflattened generated block, a `cerastes::` attribute on a label (M4.2a note 2),
  `comparison()`'s raw operands of two enumerations, "an enumerator whose enumeration this version cannot find" (M4.2b2
  note 3), `enum_case_value`'s constant without `expr` (M4.4a note 3), the exit-3 paths for emitter problems and
  "encoded twice" (M4.4a note 2: only `--selftest` sets the flag).
- *Who wrote a flag.* The noreturn trust checks a routine's first declaration and the attribute entries (M4.2a note 1);
  source sequence lists are on since M4.4b2, so the check can now be made exact (and the libc table's "declared only in
  system files", 0015 item 5).
- *Pointers will reopen two holes (M4.5).* C enumerator lists: pointer arithmetic in an enumerator value
  (`(int)(long)((typeof(N) *)0 + 1)`) observes EDG's C++ typing of earlier enumerators; keep pointers out of C enumerator
  initializers or apply 0018 item 10's node-type rule (M4.2b2 note 1). Sequencing: 0020 item 4 checks names, so
  `*p = (*q)++` needs a check on objects (old §11 question 26's answer: M4.5a). And 0025 item 2's placement of writes rests on
  callees not seeing locals (M4.5b must revisit it with item 1's kinds of access).
- *Accepted shapes worth a test* (fail closed or listed): `0 && (x = 1)`, `1 || x++` (EDG folds them; rejected),
  `assert((note(x), x >= 0))` (rejected by `check_program`), `decltype(x++)` (accepted, clang warns) (M4.3a notes 5, 7);
  `a && (y = x = 1);` and a discarded `a && (flag = x);` with a `bool` target are over-rejected (M4.3b note 5); a
  `_Noreturn int main` in C23 (EDG's #3271 also prints; only C++ tested; M4.2a note 5); a typedef whose first reason is
  not an array (`typedef unsigned _BitInt(8) fn(char a[3]);`) is rejected only at a use; `_Static_assert(1.5, "")` names
  its type in C but not in C++; no test pins an assertion in a `pure` body before M4.4c (M4.4b2 note 6: now
  `static_assert_pure_body.c`).
- *Tooling.* `mutate.py` does not mutate namespace-scope initializers (M4.2b1 note 7); a literal mutated past
  `9223372036854775807` changes type (S3 note 2); `--list` is slow under load (M4.2a note 6); the lint's
  "while without decreases" rule reads up to the first `{` (M4.2a note 7); `lint_flags` checks the flag on the line after
  the write only (M4.3a note 3); a mutation wrapper's cap must stay above the driver's `--timeout`, or Silicon processes
  are orphaned (M4.4b1 note 7; `run_tool` could send SIGTERM first); `drop-complement` on a C++ destructor and
  `swap-bitwise` on `(T)&x` give stillborn mutants (M4.4b1 note 4).
- *Walker oracle.* It cannot tell an orphan closure copy from the used one (audit B note 6); a phantom `on type
  <unnamed>` line could pass, since already-matched entries stay in the pool (M4.4b2 note 5); the walker lists neither a
  first declaration's own attribute after the parameter list nor one on a template definition.
- *Templates (M5).* Audit B (M2): a predicate called inside a `forall` lambda in a function template's contract is
  falsely rejected (`spec_call_in_code`, leftover closure copies); the `operator=` branch of
  `spec_marker_on_special_member` lost its test; P changes overload choice where constant evaluation is a substitution
  failure (two tests to add); 2–3 tests for the back end's constant-context pass in never-instantiated templates; a
  class-scope `static_assert(q(sizeof(T)))` in a never-instantiated class template is accepted (harmless). EDG at P
  leaves an error node without a diagnostic for a `decltype(p)` lambda parameter; `annotation_substitution_error` is
  load-bearing (M5b fork commit R). A closed 128-bit claim over a shifted constant other than 1 is undecided (M4.4b1
  note 6: folding `k * c_pow2(n)` would change existing Viper).
- *Pragmas.* The pragma rule rejects harmless `#pragma clang/GCC diagnostic` in user code; an allow-list of pragmas that
  cannot change meaning would lift it (M4.2b1 note 1).

## 4. Session plans

**Rules for every session** (memory `session-size-few-hours`): planned at ≤ 4 h with fixed scope and stop rules (the
owner may choose a longer one, as for M4.4c); owner decisions and pre-approved expectation changes asked up front; the
decision record before code (invariant 6); must-verify/must-fail pairs (invariant 2); docs in the same session
(invariant 8); three Sonnet agents with disjoint files (edg-backend: `src/backend`, `src/driver`, build; il-analyst:
probes, IL evidence, Silicon spikes, `src/vir` + `src/viper`; test-mutator: tests, harness, compiler matrices, `mutate.py`,
`lint_viper.py`) and the Opus soundness-auditor (one round per diff plus a re-audit of blocking fixes; should-fix items
not taken go to §3); single-file runs while working (`build/cerastes verify` directly: `run_tests.py` locks `build/vpr`);
builds serialized (`lockf build/<session>/build.lock cmake --build build/cmake`); `check.sh` at most twice; anything
that needs the x86-64 box is its own session. Record numbers are the next free ones when written (0026 is next).

### 4.1 M4.4d Exact sets (record 0026)

**M4.4d Exact sets** (record 0026; old §11 question 27).
- **Measure first** with a prototype: exact precondition sets also move preconditions in pre-M4.3b programs
  (`f(a, g(b))`), and any expectation change other than the approved one stops the work.
- **Two steps:** exact sets (a precondition gets PRE(c), and a comma's value carries its floor), else today's sets. A
  rejection reports step 2's conflict.
- **The mechanism** (decided in the record): dual sets through `LV`/`Obl`/`Effect`/`Full::pre`/`last_*`/`floor_`/`since`,
  or lowering twice with a rollback.
- **Tests:** `h((validate(y), y))`, `100 / (validate(y), y)`, `(nr(0), 1) && 100 / d`, and `h(f(x), y) + 100 / g(z)`
  (accepted by the fallback). `reject_unsequenced_split_requires.c` is retired (pre-approved).

### 4.2 M4.4e The warning sweep (owner, 2026-10-26, during M4.4c; record 0027)

M4.4c's audits kept finding default and `-Wall` warnings that no rule covers (record 0025 item 6, "still accepted"),
because no one had swept the compilers' complete lists. M4.4e does it once:
- **Inventory:** every warning clang 21 enables by default or under `-Wall` (`clang -Wall -###`, the diagnostic groups
  in its documentation, or `diagtool tree`) and every GCC 16 `-Wall` warning (`gcc -Q --help=warnings -Wall`), each
  marked "can fire on the language cerastes accepts" or not (floating point, pointers, classes, … are rejected anyway;
  record why per family).
- **Measure** the families that can fire, both modes, three tiers (§6), with near misses; the IL facts each needs
  (positions and columns for `-Wmisleading-indentation`, macro origins, loop bodies for `-Wfor-loop-analysis`).
- **Reject** what warns, one rule per family, a rejection test per shape and the near misses in
  `warn_accepted_neighbours.c`; list what the IL cannot see (an M8 entry each). Start from 0025 item 6's list and
  §3's M4.4c notes, and §8 questions 43–46.
- **Stop rules:** a family that needs an EDG fork commit goes to the box session; any existing test that flips stops
  the work.

### 4.3 EDG box session (pending since M4.2c)

**EDG box session after M4.2c** (owner, 2026-10-22). The owner merged `0022` into the fork's `cerastes`
(`f0ed5525af`, which also brings upstream `main` `ec2f12cffa`, 11 commits after `21065e551e`) and M4.2c moved the pin
there (`check.sh` green, every `.vpr` byte-identical, the three #2656 tests added). Still to do on the box: (1) the full
suites at `f0ed5525af` in both 64-bit configurations (`changes,regressions,imported,edg,cwg,stdlib`; known `cwg1835`,
`cwg2881`), and whether `0022`'s `.rto` needs `edg_x86_32` overrides; (2) old §11 question 16: reproduce the C
enumerator-list typing on unmodified upstream and draft its bug file (no fork fix). Clean builds every run (memory:
stale binaries after a failed link). (3) **`#if`** (owner, 2026-10-23, M4.4b planning; old §11 question 32): EDG folds a
preprocessing expression with no diagnostic for an unsigned wrap (`#if 0u - 1 > 0`), and in C23 its signed overflow is
#61, a warning verify mode suppresses for bodies; nothing reaches the IL (`preproc.c:460-491`). A fork commit adds a
discretionary diagnostic, distinct from #61, for a preprocessing expression whose arithmetic wraps or overflows, with
`GH-*` tests and the full box suites before the pin moves; the cerastes driver then makes it an error in verify mode.

### 4.4 M4.5 Pointers (sessions a–f)

**M4.5 Pointers** (one 4.x, sessions a–f).

VerCors summary (this session's study of `utwente-fmt/vercors` `dev`, 2026-10-20; ideas only — MPL-2.0, no files
copied): a C pointer is a value of a `pointer` ADT pairing an abstract `block` with an integer offset
(`res/universal/res/adt/pointer.pvl`); `ptr_deref(p)` = `loc(pointer_block(p), pointer_offset(p))` gives a Viper
`Ref`, and each element type has its own Viper field; a nullable pointer is `Option<pointer>` (every dereference
checks non-null); `ptr_add` is abstract with postconditions (a one-past-the-end pointer cannot be formed);
`\pointer(p, n, perm)` desugars to non-null, bounds and a quantified permission over `p[0..n)`; address-taken
locals become 1-element non-null blocks (`VariableToPointer`); structs are by-value classes whose fields are
pointers to field cells; `malloc` is accepted only as `(T*)malloc(n*sizeof(T))`, `free` exhales the block's
permissions; relational pointer comparison across blocks is false instead of an error. Caveats to avoid: the prelude
is axiom-heavy and three axioms look inconsistent outside their triggers (`pointer.pvl` stride axiom, `option.pvl`
`some(option_get(opt)) == opt` unguarded, `const_pointer.pvl` injectivity); `int` and `short` share one field (issue
#1357, effective types); `free(&local)` verifies (#1237); integer overflow unchecked by default; no `switch`, `do`,
enums, unions or string literals. VeriFast's C coverage (its own front end) is the parity target: points-to chunks,
`malloc_block`, `chars`, predicates with `open`/`close`, lemmas, `/*@truncating@*/` casts, string and `malloc`
contracts in `bin/*.h`; canonical examples to port are listed per session below.

- **M4.5a Design** (no encoder code): records for (1) the memory model — cells and typed value fields (one field per
  C type, never shared between `int` and `short`), the pointer representation (block + offset, or one ref per cell
  with an explicit block-length ghost), NULL, provenance and one-past-the-end, address-taken locals, how
  permissions map to `owns`/`reads`; (2) **the C spec syntax**: today `owns`, `reads`, `old`, `owns_range`, `forall`
  are C++ only (`include/cerastes.h:61-75`, 0008 item 6) and `forall` needs a lambda, which C lacks — options:
  declared functions taking `const void*` (the IL keeps the typed argument under the conversion), `_Generic`
  macros, or a binder-introducing attribute (EDG's result-name mechanism, 0007 item 6); (3) the new error categories
  (permission, null dereference, out of bounds, invalid free, …) and their messages in source terms. Silicon spikes
  for each candidate encoding, including the axioms each needs. If an EDG fork commit is needed, a box session is
  added before M4.5b. (4) **P4222R1's markers** (`[[uninit]]`, `[[ref_to_uninit]]`) and the stricter mode that asks for
  them (owner, 2026-10-22, old §11 question 21: "as early as possible", in M4.5; what is left goes to M7): how they attach
  (an attribute on a definition and on a pointer parameter), and how 0019's flag follows the cell. (5) With pointers,
  0020 item 4's sequencing rule must become a check on objects, not names (`*p = (*q)++` may alias): a path-precise
  category then (0020 item 4).
- **M4.5b Pointers to scalars and globals:** `&x`, `*p`, NULL, pointer parameters with `owns`/`reads`/`old(*p)`,
  pointer equality, an aliasing trap that must fail, mutable globals and `static` locals as cells with frames; ports
  of VeriFast's `examples/swap.c`, `examples/address_of_local.c`, `examples/globals.c`; keep pointer arithmetic out of C
  enumerator initializers or extend 0018 item 10's rule to it (§3, M4.2b2 note 1).
- **M4.5c Structs:** field cells, `.`/`->`, struct values (copies, initializers, by-value parameters and results),
  nested structs, pointers to fields; ports of `tutorial_solutions/account.c`, `deposit.c`.
- **M4.5d malloc and free:** trusted contracts for `malloc`, `calloc`, `realloc`, `free`, `abort`, `exit` (system
  files are used only through contracts, 0011 item 4), allocation blocks, ownership on free, leaks (record); ports of
  `tutorial_solutions/malloc_block.c`, `examples/cell.c`.
- **M4.5e Arrays and pointer arithmetic:** local arrays, `a[i]`, `p + i`, `p - q`, comparisons within one object,
  `owns_range` and quantified permissions, `forall` in C; ports of `examples/carrays.c`, `examples/insertion_sort.c`,
  array sum and max.
  **From P38 (M4.4a, record 0022 item 3):** a braced initializer with more than 1000 elements keeps no `expr` for its
  elements (`REDUCE_BACKING_EXPRESSION_USE`), only an `orig_type`. Read every element through `constant_value`, which
  rejects that shape, never through `ck_aggregate`'s values directly, or a wrapping element would pass unchecked.
- **M4.5f Predicates and pure functions:** predicate and pure bodies encoded (0011 item 4 lifted), `fold`/`unfold`,
  recursive predicates; a linked list (push, pop, length), `tutorial_solutions/stack.c`, `reverse`. Lemmas and ghost
  inductive data are open design points (`internal/upstream/attribute-registration.md` Appendix A): a record decides
  whether they are in M4.

### 4.5 M4.6–M4.8

**M4.6 Chars and strings:** string literals as read-only arrays, `char` buffers, `strlen`/`memcpy`/`memcmp`/`strcmp`
contracts, `main(argc, argv)`; ports of `tutorial_solutions/strlen.c`, `memcpy.c`, `memcmp.c`.

**M4.7 Function pointers:** contracts on function types, calls through pointers; ports of `tutorial_solutions/byref.c`,
`examples/functions/functions.c`.

**M4.8 C parity close-out:** the VeriFast C differential set (the ports above, run with VeriFast in Docker as M5c
sets up, or recorded outcomes), the full mutation run over the M4 tests, the Memory and ownership page complete,
M4's acceptance (plan.md), and the deferred list for M8 (unions, floating point, `volatile`/`_Atomic`/`_BitInt`/
vector types, variadic functions and `printf`, several translation units, concurrency).

### 4.6 M5.0 and M5 — type traits and generic templates (designed in the M4.1 session)

**M5.0 Type traits** (owner, 2026-10-26, old §11 question 39; `plan.md` "Before M5"). After M4.8 and before M5a. M4.4b2
skips a static assertion whose condition calls a function, uses a type trait (`enk_builtin_operation`, a variable
template such as `std::is_integral_v<T>`), a concept-id, or names an entity of a system header (`std::numeric_limits`).
M5.0 decides how cerastes reads such conditions (and type traits elsewhere: contracts, `if constexpr`), so that M5's
integral `T` and M6's requirement contracts rest on it. Planned at its own planning, ≤ 4 h.

**From P38 (M4.4a, record 0022 item 3):** a non-type template argument keeps no `expr` in this configuration
(`BACKING_EXPR_FOR_NONTYPE_TEMPL_ARG` is false), so `t<0u - 1u>()` reaches an instance as the plain constant
`4294967295U`, which cannot be told from a literal. Under 0022 (no wrapping) M5 must not trust such a constant: check a
template argument's conversion at the place where it is written (the template-id in the caller), or reject arguments
that are not literals, decided in M5a's record.

The M3 handoff §6 still holds: VIR's `IntType` needs a type form for `T`; templates in verified files are rejected
today (`encode.cpp:1462-1467` declarations, `:1495-1497` routines, `:926-929` calls); generic proofs read contracts
from the root template; contracts can use lambdas naming `T`. Latent hazard: a template instance whose body is in a
verified file takes `callee_for_call`'s has-body path (`encode.cpp:911-915`) and would become a bodyless VIR
function with no `trusted_reason`; only the declaration-level rejection keeps it unreachable today — M5e must handle
instances before that rejection is lifted.

**M5a Eligibility** (cerastes-only; verification results unchanged).
- `src/backend/generic_eligibility.{h,cpp}`: `check_generic_eligibility(template, prototype)` → verdict (eligible;
  per type parameter opaque or integral; for ineligible ones a tag, a text and the first offending position). A
  pure function: no diagnostics, no exit status can change.
- Integral marks: only `eok_land` conjuncts of the four constraint places (il-inventory A2–A8); `std::integral`
  identified by entity (concept template `integral` in namespace `std`, through inline namespaces), argument the
  implicit one or one `tak_type` `tptk_param` at the template's depth, typerefs skipped. Other concepts,
  disjunctions, `!`, requires-expressions are ignored (they only narrow the instances; the parameter stays opaque).
- Value classes for dependent nodes (non-dependent subtrees are left to the encoder's own allow-list): Lit (`int`
  literal 0..32767), Val(p), Prom(p) (`+ - * / %` of one parameter, at most one literal), Bool (comparisons of one
  parameter; `&& || !`; `==`/`!=` on bools), Opaque(p) (`==`/`!=` of one opaque parameter in annotations only).
  `ck_template_param`/`tpck_expression` constants are looked inside (gotcha 6: `(T)100`); other tpck kinds fail.
  Conversions to `T` must be explicit casts of Val/Prom/NonDep builtin integers; an implicit promoted → `T`
  conversion (`return v + 1;`, no conversion node in the IL) is `conversion`.
- Tags: `member`, `nontype_parameter`, `template_template_parameter`, `parameter_pack`, `unnamed_parameter`,
  `spec_function`, `constexpr`, `signature_type` (`T*`, `T&`, `T&&`, `auto`, `Box<T>` — revisit `T*` with an opaque
  pointee now that M4.5 brings pointers), `default_argument`, `dependent_call` (incl. `old`/`forall`/`owns` over
  `T`), `member_access`, `pointer_operation`, `sizeof`, `cast`, `bitwise_or_shift`, `opaque_operation`, `operands`,
  `unary_arithmetic`, `increment_or_compound_assignment`, `conversion`, `not_bool`, `local_initializer`,
  `unsupported_statement`, `measure`, `unsupported_operation`, `internal`.
- Completeness backstop: a second pass with EDG's `traverse_statement`/`traverse_expr` (template-parameter
  constants included) collects every dependent node; one the classifier did not visit makes the template `internal`;
  a hook `--cerastes_generic_selftest` proves it fires.
- Observation: `--cerastes_generic` (cerastes-fe) adds to each function-template prototype in the walker listing
  `generic eligible T=integral U=opaque` or `generic ineligible <tag> @ L<n>:<c>: <text>`; harness header
  `// golden-options: --cerastes_generic` (only with `expect: golden`); the walker oracle counts the lines as a
  derived category (not IL facts). The 49 existing prototype goldens do not change.
- Tests: `tests/templates/eligibility/` (each ineligible tag next to its eligible minimal pair; verdicts
  pre-registered in `// verdict:` comments before goldens exist) and the six ports' verdicts
  (`tests/templates/verifast/`).
- Timeline: il-analyst probes P1–P8 (35 min box: `T i = 0` vs `(T)0`, contract param refs and opaque `r == x`,
  non-dependent calls in a prototype and `old(v)`, `t.m`/`-v`/`v++`/`v += 1`/`(int)v`/`sizeof(T)`/lambdas, parameter
  kinds, loops with attributes and `return v + 1`, `r == (0 <= v && v < 10)`, `a_constant.expr` vs `variant.expr`);
  test-mutator harness header + oracle category; orchestrator the record; edg-backend the checker; goldens checked
  by il-analyst; one Opus audit. Stop rules: cut the oracle's structural checks, then tag coverage automation, then
  `main` in the ports; any rule family not done by 2:10 returns ineligible "not checked" (fail closed).

**M5b EDG (box).** Best just before M5g. Fork commit Q: a registration flag
refusing an accumulating attribute that first appears on a later declaration (check site
`attach_decl_attributes_to_entity`, `decls.c:806-812`, next to `reconcile_must_match_attributes`,
`attribute.c:4131-4230`; next free flag bit `0x8`, `attribute.h:401-437`; precedent
`ec_comparison_defaulted_in_class_must_be_first_decl`, `decls.c:10631`; `first_decl` is set on namespace-scope
paths — check member and template paths; GH tests in `tests/tests/changes/GH-*.sft.cpp` with an `-ok` companion,
`src/Changes`, `doc/source/ext_intf.rst`); cerastes registers it for `predicate`/`pure`
(`register_attributes.cpp:51-52`). Fork commit R: EDG diagnoses series L's silent error node for a `decltype(p)`
lambda parameter (§3, *Templates*); the `decltype` tests move from `cerastes-annotation_substitution_error` to EDG's diagnostic
at the same line (owner's rule). Plus the close-out audit B's missing tests (§3, *Templates*). Fail-before/pass-after,
Opus audit, the full box suites (baseline mismatches `cwg1835`, `cwg2881`), pin move, 0006/0007 and the upstream
draft amended. Stop rules: box unavailable → topic branch, pin moved in a short follow-up; new box mismatch → no pin
move.

**M5c Generic infrastructure and opaque `T`.** A generic Viper method per eligible template, verified even when
nothing instantiates it; calls of instances and ineligible templates stay rejected. Opaque `T`: one empty Viper
domain per type parameter, admitted by `lint_viper.py` only after `// cerastes:opaque-type <template> <param>
<reason>`, with an empty body and a `c_T_` name; `==`/`!=` between values of one opaque parameter. Soundness
(parametricity): no functions, axioms or quantifiers over the sort, so validity over every non-empty interpretation
covers each instance; instances restricted to M3's scalar types (integers and `bool`; floating point excluded, NaN).
VeriFast in Docker (`--platform linux/amd64`, image digest pinned in 0001, `~/Documents/verifast/bin` mounted
read-only, both provers as `verifast_both`), recorded outcomes of the six originals in
`tests/differential/verifast/recorded/` so `check.sh` never needs Docker. Stop rules: F11 (non-dependent calls in a
prototype) not resolving → reject those calls and record; Docker not working in 30 min → run it on the box in M5b.

**M5d Integral `T`.** Four ghost formals per integral parameter (`T_MIN`, `T_MAX`, `PT_MIN`, `PT_MAX`) with
VeriFast's linear constraints as method `requires` (`127 <= T_MAX <= 2^128-1`, `-T_MAX-1 <= T_MIN <= 0`,
`T_MAX <= PT_MAX`, `INT_MAX <= PT_MAX <= 2^128-1`, `-PT_MAX-1 <= PT_MIN <= T_MIN`; INT_MAX from EDG's target): no
domain, no axiom, no assume. Signedness unknown: every symbolic operation is checked as signed against symbolic
bounds (sound for unsigned instances: a result in range means the wrap is the identity); M3's shortcuts that assume
symmetric ranges (`lower.cpp:236` unary minus `a != lo`, `:301` division `!(a == lo && b == -1)`) are not reused —
the general range check on the result instead. `promoted(T) = PT`, `usual_arithmetic(PT, int) = PT` computed by
cerastes (the IL types dependent operations `tptk_unknown`). `cerastes --selftest` checks every target integer type
except `bool` satisfies the constraints; the `consistent` must-fail test (`check(false)`) shows they are not
vacuous. Port `integral_wrong_spec` here (no `main`, no instances), its expectations written from the record before
the first run. Risk: `v * v` against symbolic bounds slow in Z3. Stop rule: loops in generic bodies move to M5f.

**M5e Instances, call sites, fallback, reporting.** Callers call each instance's own method (the M3 encoder on the
instance IL, the contract from the root template, 0006). One Silicon run holds every generic method and every
instance method; the position map gains a `#method` table (first/last line, role, template, display name). Per
template: generic verified → covered (a covered instance that fails is exit 3, "discrepancy" — owner question);
generic failed and instances exist → the generic's errors hidden, the instances' results count (notes
`in the instantiation f<long>`); generic failed and no instance → the generic's errors reported. Coverage: integral
arguments builtin integers other than `bool`; opaque arguments M3 scalar types; not an explicit specialization; not
in a recursion cycle. Notes on stdout after the verdict: `<file>:<line>: note: per-instantiation|not-generic|
not-verified: …`, pinned by `// expect: note <kind> @ L<n>`; 0011 items 9–10, `tests/README.md` and
`parse_verify_output` amended. The count `n`: plain bodies + one per template verified once + one per instance
verified separately. Port `integral_fallback`.

**M5f Specializations, recursion, headers, correspondence.** Explicit specializations verified as plain functions
with the primary's contract (EDG enforces must-match), never covered; specializations of system templates stay
rejected; explicit instantiations are ordinary instances (differs from VeriFast). Call graph keyed by instance
routine; a template in a cycle is not covered (Viper checks a measure only within an SCC of the emitted graph).
Templates in user headers verified per including unit; system templates only through contracts.
`internal/soundness/prototype-correspondence.md` finished (each session adds the constructs it accepts) with an
il-analyst oracle comparing prototype and instance listings for every builtin integer type. Mutation run on
`tests/templates`.

**M5g Gate G2.** Ports in `tests/templates/verifast/` with adaptation headers and line maps to the originals;
`scripts/vf_mutants.py` applies each `mutate.py` edit to the original through the map; VeriFast on originals and
mutants; `internal/differential.md`. Agreement: the same verdict and, when both fail, the same failing lines (and
the same path, generic or per instance). Explained, non-blocking classes: the C meaning of contract arithmetic (0010
item 11), unsigned wrapping, termination (0013), the eligibility path when the verdict agrees, limits from the
target. Anything else stops the session and blocks M6. G2 scope: `integral` (with `pick(1, p)`, pointers from
M4.5), `integral_fallback`, `integral_per_instantiation` (with `low_bit`, bitwise from M4.4),
`integral_wrong_spec`, and `generic_body`'s `copy_through_locals` (adapted: `T z = y;`), `second`,
`declared_first`; **deferred to M7** (owner): `generic_body`'s `assign`/`same_ref` and
`function_template_general_contract`.

**Questions to settle before M5c** (ask then): a failing covered instance → exit 3 (recommended) or ignored as in
VeriFast; notes on stdout with `expect: note` lines; the agreement definition and its explained classes.

## 5. Known gaps carried

- Termination: a recursion cycle through another unit (0013 item 5) — the contract manifest, M6 (M7 if it does not fit).
- The `predicate`/`pure` marker on the first declaration — M5b (an EDG registration feature).
- Still rejected for `forall` in templates: a lambda depending on `T` only through `decltype` of a function parameter
  (above), function parameter packs (`attr_function_parameter_pack`), `sizeof...` of a template pack, a lambda that
  names the template it is in (`h` undefined). EDG bug 0021 does not arise: cerastes reads C++ in Clang emulation.
- Closed: the five compile-time uses of `constexpr` spec functions that left no trace (P, registered).
- Attributes in positions where EDG records nothing (0011 item 7); M3's rejected constructs (0012 item 12) for M4 and M8.
- From M4.2a (0015): a `goto` that enters a block, an `if` branch or a switch body (C allows it; Silicon's handling
  of a jump into an `if` branch is unproven, S9d); a label that is the whole branch of an `if` without braces; GNU
  computed `goto`; `quick_exit`, `__builtin_trap`, `__builtin_unreachable`/`unreachable()`, `std::terminate`; an
  `assert` in a `for` increment; glibc's `assert` expansion. Each is rejected with a diagnostic and an M8 entry.

## 6. Process notes

- **What keeps a session short:** fixed scope with stop rules; the owner's decisions and pre-approved expectation
  changes asked up front, each with a concrete default; the record written while the probes' results come in, audited in
  the background; three agents with disjoint files; single-file runs during the work.
- **Worktree.** Sessions run in the worktree `.claude/worktrees/m4.2b1` on their own branch (`git checkout -b <branch>
  main`); its submodules were cloned with `--reference` to the main checkout's module stores. A shell `cd` elsewhere may
  leave the worktree: name it in every command.
- **Builds and baselines.** Take the baseline (`run_tests.py -j 8`, `build/vpr` → `build/<session>-baseline-vpr`, the VIR
  self-test into `build/<session>-baseline-vir`) before any build; agents edit meanwhile but build only after it, with
  `lockf build/<session>/build.lock cmake --build build/cmake`, and nothing builds while `check.sh` runs. A session that
  moves the pin runs `check.sh` once early.
- **Compiler matrices** (P41, P48, P50): clang and GCC, both modes, three tiers (no flag, `-Wall`, `-Wall -Wextra`); the
  union rule of 0020 item 5 rejects what warns at the first two tiers. Measure the near misses too, natively: a positive
  that holds them must compile with `-Wall -Wextra -Werror`.
- **Mutation** runs once, after the last rebuild, alone on a quiet machine, with no external cap below the driver's
  `--timeout` (M4.4b1: a SIGKILLed driver orphans Silicon). `mutate.py` reports progress on stderr; check `date` against
  the budget before starting.
- **Commits.** `git add <paths> && git commit` commits everything staged, including an earlier `git rm`: delete a retired
  test in the working tree, and stage the deletion only in the commit that lands its feature. No Claude trailer; never
  push.
- **Doc pitfalls.** `[[noreturn]]` in text becomes an anchor even inside `+…+`; `__name` in backticks needs `+…+`; a
  possessive after a code span breaks the next span; `pass:[…]` holding `<` or `>` is refused by
  `check_doc_examples.py` (write `+…+`). `build_docs.sh` and `check_doc_examples.py --html build/doc` catch them.
- **A test that loses an `expect:` line** (pre-approved) keeps its line numbers with a comment in its place
  (M4.4c, the four `reject_assert_shape_*.c`).

## 7. Acceptance of M4.4c

| # | Command | Result |
|---|---|---|
| 1 | `scripts/check.sh` #2 (16:37–17:19, after audit round 2's fixes) | **all green**: self-tests harness 981 (skipped 1), oracle 79, lint 190, mutate 291; tests 5386 passed, 0 failed, 1 skipped (native-ok: Apple clang 1019, GCC 1019); 1820 `.vpr` lint clean; VIR self-test 686/686 (+ 203 rejection cases); walker oracle 135 goldens, 1922 facts, 0 failed; docs 0 problems; `mutate.py --validate` 325 tests, 11241 mutants |
| 2 | `scripts/check.sh` #1 (15:50–16:23) | 5378 passed, 6 failed: R10 fired in three error tests (contracts on narrow results, a named constant), which the agents' sweep had skipped; R10 became code-only, with literals and enumerators only |
| 3 | `diff -rq -x MANIFEST build/m4.4c-baseline-vpr build/vpr`; the VIR self-test against `build/m4.4c-baseline-vir` | all 1768 `.vpr` and maps of the baseline identical, none missing, 104 new; the 671 old VIR cases identical |
| 4 | `run_tests.py --check-headers`, `--count-error-twins`; `build/cerastes --selftest` | 2027/2027 (with the mutation twin); 323; 94 checks |
| 5 | `mutate.py -j 8` over the 10 new positives and 2 doc examples (no external cap; logs `build/m4.4c/mutate1.log`–`mutate4.log`) | run 1: 1619 mutants, 979 killed, 186 safe-direction, 149 stillborn, 303 survivors, 2 tool failures (Silicon timeouts of a body `p \| q` against an exact `&` contract; that contract now uses `&&`); three files strengthened in place and re-run (0 tool failures), one new twin; **284 reviewed equivalents, 0 unexplained** (Opus: 26 reasons corrected, 6 proposed entries killed by strengthening instead); `mutate.py --validate` 325 tests, 11277 mutants, 863 entries |
| 6 | audits (Opus) | round 1 (record, P46–P48): 0 blocking, every should-fix taken; round 2 (code, tests): 0 blocking, 3 should-fix (2 fixed, 1 listed: M4.4e, owner); re-check: 0 blocking, 1 should-fix (three shapes, listed for M4.4e) |
| 7 | wall-clock | 14:36–21:30 (planning before), about 7 h against the planned 5 h: the mutation run and its review took about 4 h of it |

## 8. Questions for the owner

Each has a recommended answer. Answer inline, as before. Answered questions (1–42) are recorded in the decision records
and in the commit named at the top.

**Open from earlier sessions:**

5. **A `goto` into a block or an `if` branch** stays rejected (0015 item 2; stricter than C). Lift it only when a ported
   example needs it, after a Silicon spike with must-fail twins? *Recommended: yes.*
10. **The verified doc examples that have never been mutated:** mutate them in M4.8's run, or in a short session before?
    *Recommended: M4.8, with the M4 tests.*
11. **Typed locals in the mutation classifier** (about 20 listed entries, e.g. `assert_holds.c`): teach it to read local
    declarations? *Recommended: not now; every new bucket rule needs its own audit.*
12. **A full mutation run in CI** (about 85 min locally, far more on the 3-core runner)? *Recommended: no; a per-session
    acceptance step, with `--validate` in `check.sh`.*

**M4.4c, 2026-10-26:**

43. **M4.4e, the warning sweep** (your answer during M4.4c). Its method: list every warning clang 21 enables by default
    or under `-Wall` and every GCC 16 `-Wall` warning (from the compilers' own flag lists), keep those that can fire on
    the language cerastes accepts, measure each on both modes, and reject the remaining shapes with one rule per family
    (record 0027), or list them. When: after M4.4d, or before it? *Recommended: right after M4.4d (planned, small), and
    before M4.5a, so pointers start from a complete list; with an M8 entry for what the IL cannot see (layout-only
    warnings such as misleading indentation, if P-probes show no column data).*
44. **R10 and R11 skip annotations** (record 0025 item 6): a contract with an out-of-range or contradictory comparison is
    a wrong contract that verification reports, and three error tests use one on purpose. Keep that (one exception to
    0023's "the rules apply in annotations"), or apply them in annotations too and change those tests' contracts in
    place? *Recommended: keep.*
45. **R13's broader shapes** (a `;` on the next line, an empty macro body) are rejected although clang is silent there,
    since the IL keeps no `)` position. Keep (the M4.4b rule for shapes the IL cannot match), or accept them until the
    sweep finds a layout source? *Recommended: keep.*
46. **One verdict for `x ? RED : GREEN` used as an `int`** (accepted in C, rejected in C++ by 0018 item 8): reject it in C
    too? *Recommended: yes, in M4.4e (it is the enumeration half of `-Wenum-compare`'s family), with a test per mode.*

**S4, 2026-10-27:**

47. **The `bit_mask.c [c23]` hang** (§3, "From S4"): fix it with the encoder's mask-as-remainder rule (candidate 1, an
    amendment of 0023 with its tests and an audit), in a short session of its own before M4.4d? And PR #11: merge it
    over the explained red run, or rerun it first? *Recommended: the encoder rule, in its own session before M4.4d; merge
    PR #11 as it is (the failure predates M4.4c and is explained; a rerun that passes would prove nothing).*
48. **Reproducible solver runs:** pass `--numberOfParallelVerifiers 1` (each program's prover history fixed, so an
    intermittent hang becomes a failure every time, at some cost in time) or an `--assertTimeout` (a bound per query that
    fails closed) to Silicon in the driver? Either changes every test's run and needs a full-suite measurement first.
    *Recommended: measure `--numberOfParallelVerifiers 1` on the whole suite in the bit_mask session and decide on the
    numbers; no `--assertTimeout` (a resource bound can turn a slow proof into a failure).*
