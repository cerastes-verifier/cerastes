# Handoff after M4.1 — control flow, and the plan re-sequenced (2026-10-20)

The next session reads this file after `CLAUDE.md`, its own section in `plan.md`, `internal/status.md` and
`internal/decisions/`. `status.md` has the formal entry. This file adds what a cold start would otherwise have to
rediscover. The M3 close-out handoff is `git show a4da52c:internal/handoff.md`; the M3 handoff (with the owner's
answers) `git show 80b9e35:internal/handoff.md`.

## 1. State in one paragraph

The milestones are renumbered (owner, 2026-10-20; `plan.md`, top): the new **M4 is the rest of C** (M4.1–M4.8,
pointers in M4.5a–f, VerCors as the reference), generic templates are **M5** (M5a–M5g), concepts M6, **C++ memory
and classes M7**, the inventory M8, evaluation M9; every record is renumbered and notes it. **M4.1 is done** on branch
`m4.1-control-flow`: `break`, `continue`, `do … while` (and `do … while (0)` without a measure) and `switch` verify,
with decision 0014, 86 new tests, VIR self-test 204 cases and the docs. The pin is unchanged (P `b9519388ea`).
Open from M4.1: 70 surviving mutants of the new tests go to the mutation triage session (§5). Nothing is pushed.
Planned at ≤ 4 h, the session used about 4 h (18:53–22:50), most of it the test-mutator's mutation runs.

## 2. Owner steps, in this order

1. **Bring the branch into `main`.** The work is on `m4.1-control-flow` in the worktree `.claude/worktrees/m3-closeout`
   (based on `main` `bfeda03`):
   ```
   cd ~/Documents/cerastes/cerastes
   git merge --ff-only m4.1-control-flow
   scripts/bootstrap.sh --build-only
   ```
   The old branch `worktree-m3-closeout` is fully merged; the worktree can be removed when no longer needed
   (`git worktree remove .claude/worktrees/m3-closeout`).
2. **Push the fork's `cerastes`** (`git -C ~/Documents/compiler push origin cerastes`): GitHub still has `0bcd9bf28a`;
   the pin P `b9519388ea` is 6 commits ahead. `check.sh` reports `NOT YET PUSHED` until then.
3. **Push cerastes `main`.**
4. Unchanged from the close-out: branch `0020` (`c539de4101`) kept for the upstream PR, its Linux failure unexplained
   (§4); the fork's `.tmp/agent-*` evidence directories.

## 3. What the session did

| Item | Outcome |
|---|---|
| Planning (owner's question: plan M4) | Exploring M4 (generic templates) showed G2 needs memory; the owner re-sequenced the plan (§1); this session designed and wrote down every M4 and M5 session (§8), with the VerCors study for M4.5 |
| Renumbering | `plan.md` (table at the top), the handoff, `encoding.md`, `il-inventory.md`, `tests/README.md`, records 0001/0006/0007/0008/0010–0013, the upstream draft, two code comments; mapped by meaning (old M6 → M4.5 for C, M7 for C++) |
| Decision 0014 | control flow: structured VIR jumps, invariants and measure on `continue` paths, `do` invariants at the start of each iteration, `do … while (0)` as straight-line code with an internal guard, `switch` dispatch, case values by their original expression, the rejections, #546 suppressed; amends 0010–0013 |
| Code | `encode.cpp` (target stack with label-shape checks, `do`, `switch`, `step`, `fallthrough` allowed, case values through the constant path, label attributes checked), `src/vir` (forms, `check_program` rules incl. innermost targets, lowering), `verify_main.cpp` (#546) |
| Tooling | `mutate.py` deletes `break;`/`continue;` and case bodies; `lint_viper.py` rejects backward gotos, labels with invariants, duplicate labels |
| Tests | 86 new files in `tests/scalar` (31 positives, 55 twins and rejections, both modes where C++ accepts them); `reject_{break,continue,do_while,switch}.c` retired (owner); VIR self-test 155 → 204 lowering cases (+ 75 rejection cases) |
| Docs | Verifying a program (control-flow section with `control_flow.c`; `rejected_double.cpp` replaces `rejected_switch.cpp`; rejected list), Annotations (`do` invariants, `decreases` before `do`, the `do … while (0)` exemption), Overview |
| Audits (Opus) | 0014: 1 blocking (EDG folds `case 1 << 40` to 0 with a suppressed warning: case values now encoded by their original expression), 5 should-fix, all taken; code: 0 blocking, 4 should-fix taken (`for (int k…)` in a case, `case k`, stale text, an `__int128` overflow in the checker), notes in §6 |

## 4. The CI session (after M4.1; owner 2026-10-16 and 2026-10-20)

Its first two steps need the x86-64 box (memory `x86-box-edg-suites`; the box sleeps, ask the owner to wake it):

1. **Merge upstream EDG `main`** into the fork's `cerastes` (`--no-ff`, never rebase; decision 0009). Upstream was
   `ed0a9efb06` on 2026-10-20, 8 commits after `307002eae6`:
   - `52a125e154` is our 0005 (`__BLOCKS__`), re-committed by the owner: expect it to merge as a duplicate of the
     fork's 0005 merge, or to conflict;
   - `4545bf930c` (#229, `if constexpr` with non-brace statements) changes front-end behaviour;
   - `ed0a9efb06` (#309) moves the position of cleanup actions to the closing brace: goldens and position maps may
     move;
   - `5bfce0ac36` (#302, base-class initializer order), and CI/docs commits.
   Then the full box suites (`changes,regressions,imported,edg,cwg,stdlib`, both 64-bit configurations) before the
   pin moves; known baseline mismatches `cwg1835`, `cwg2881`.
2. **Branch `0020` alone fails its own test on Linux** (M3 handoff §4): `GH-lambda-template-param-qualified.sft.cpp`
   cases 1 and 2, `FAIL--OUTPUT MISMATCH`, passes at the chain tip and natively. Explain it before the owner files 0020.
3. Then CI itself (a decision record; macOS runner first, x64 Docker if the goldens drift).

## 5. The mutation triage session (short; after the CI session, before M4.2)

The classifier now buckets a contract mutant only when the grid models it (signed builtin integer parameters,
linear, no `==`/`!=` flip) and, for a stronger `requires` or a weaker `ensures`, the function has no use in the
file. 297 → 223 candidates in `tests/scalar`: 74 leave, none join (31 caller, 12 caller + unmodelled, 29 unmodelled,
2 bodyless). Recompute the list statically: generate with `git show e04ce6e:scripts/mutate.py` (old) and the current
one, and compare `Subject.safe`. Per file: `call_contract_only.c` 3, `call_in_expression.c` 6,
`call_precondition.c` 1, `call_result_conversion.c` 6, `check_stmt.c` 6, `compound_mixed.c` 6,
`compound_ushort.c` 4, `contract_wf.c` 3, `default_argument.cpp` 8, `default_argument_expr.cpp` 4,
`measure_function_expression.c` 4, `measure_lexicographic.c` 2, `measure_mutual.c` 6, `measure_not_needed.c` 2,
`recursion.c` 2, `shift_promoted.c` 5, `ushort_scale.c` 5.

Steps: run `python3 scripts/mutate.py tests/scalar -j 8`; each surviving leaver is killed (a new test, never an
edited expect line) or listed in `tests/mutants-equivalent.txt` with a reason, reviewed by the Opus auditor. Audit A
expects 7 of the 8 caller-ensures leavers to be killed already; `call_result_conversion.c:9:40 100->101` is safe.
Fold in audit A's should-fix items (§6) at the same time. Until then M3's "0 unexplained survivors" holds for the
old bucket only.

**M4.1's survivors (added 2026-10-20).** The mutation run on the 31 new positives (`python3 scripts/mutate.py -j 8
<files>`; 1403 mutants: 991 killed, 299 in the safe-direction bucket, 27 stillborn) left **70 survivors**, which the
session's stop rule moved here. test-mutator's proposals are in `build/m4-staging/equivalents-proposed.txt` (id, tab,
reason; untracked — copy it before cleaning `build/`). 34 of them are in `continue_cond_div.c` and
`do_continue_cond_div.c`: test weakness, not equivalence (deleting the `continue;` survives) — strengthen those two
positives (an exact `ensures` on a result the continue branch changes; the twins may keep a weaker invariant so that
the reported error stays `division-by-zero`, said in their headers). The other 36 look like genuine equivalents or
safe-direction mutants (the last `break` of a switch, dead initialisers, measures of loops that never repeat, slack
bounds, `while (1)` → `while (2)`, weaker `ensures` on unsigned parameters, which the classifier does not model);
each needs the auditor's review before it enters `tests/mutants-equivalent.txt`.

## 6. Should-fix items and known gaps

**Audit A (tooling), deferred to the triage session** (none affects the 223 current candidates):
1. Python's meaning of chained comparisons and of `not` differs from C's (`x < r < 1`, `!r < 1`, `r == x > 0` can be
   bucketed wrongly): allow `!` only on a parenthesized operand, one comparison per operand, or parse with C precedence.
2. A literal coefficient of `*` is not monotone (`3 * x + y < 10` → `4 * x …` incomparable but bucketed): make it
   `unknown`, or drop `*`.
3. Contract well-definedness after a dropped guard (0010 item 11): a drop-conjunct/swap-logic mutant can remove the
   guard of arithmetic in the clause or in a later `ensures`.
4. `has_caller` misses uses inside a `#define` body and in a default argument.
5. The termination lint is line-based; a clause split over lines (`decreases` / `*` on the next line) passes. The
   emitter writes one line per clause, so this is defence in depth.

**Audit B (pin, registration, tests).** Its blocking item is fixed and re-audited: a substituted lambda whose own
signature holds an error type (a `decltype(p)` parameter the body does not use) was accepted; now
`annotation_substitution_error`. The records it found inaccurate (0006, 0007, the Annotations page) are corrected.
Deferred (probes in the job's scratch directory are gone; the shapes are given here):
1. **False `spec_call_in_code` from leftover closure copies:** a predicate called inside a `forall` lambda in a
   *function template's* contract is rejected, even when the lambda does not name `T` (accepted in a non-template
   function and in a class-template member). Series L leaves one closure copy per instance (two for member
   templates) that no attribute reaches, so its call node is not exempt. Fails closed (a false rejection); fix with
   the EDG follow-up of question 2 or by exempting closures reachable only from a `lambda` record of a contract.
2. **The `operator=` branch of `spec_marker_on_special_member`** lost its only test (`spec_marker_on_assignment.cpp`
   now pins EDG's defaulted-function error): add a twin with a user-provided `operator=` marked `CERASTES_PURE`.
3. **P changes overload choice where constant evaluation is a substitution failure:** e.g. a return type
   `decltype(S<p(sizeof(T))>{}, 1)` or `enable_if_t<q(sizeof(T))>` makes EDG choose another overload than clang/GCC.
   Today each is rejected at its line, only because the back end's pass over all routine nodes finds the trace in
   the prototype: add two tests so narrowing that pass cannot make it silent.
4. **Tests for the back end's constant-context pass in never-instantiated templates** (bit-field width, enumerator,
   `noexcept` specifier, `explicit`, `alignas`, a block `static_assert` with a dependent operand): 2–3 tests.
5. **Existing gaps (not from this diff):** a class-scope `static_assert(q(sizeof(T)))` in a never-instantiated class
   template is accepted (never evaluated; harmless); a `CERASTES_PURE` override of a non-spec virtual function,
   called through a base reference at run time, is accepted (M7, virtual dispatch).
6. **The walker oracle** cannot tell an orphan closure copy from the used one (same name, same body in normal cases).

**Found this session:**
- **EDG at P leaves an error node without a diagnostic** in the substituted contract of a function template whose
  `forall` lambda takes `decltype(p)` of a function parameter (series L's rescan cannot substitute it). The back-end
  check `annotation_substitution_error` rejects it, so it is safe, but that check is now load-bearing, not only
  defence in depth. A fork follow-up should make EDG diagnose it (question 2).
- **Unreached cerastes-side checks:** the R4a constant-context IL traces in `src/backend/annotation_checks.cpp` (array
  bounds at namespace scope, bit-fields, `noexcept`/`explicit(bool)` specifiers, template arguments, `static_assert`,
  `case`, enumerators, `constexpr` variables, `if constexpr`, `alignas`) are no longer reached by a test: EDG errors
  first and the back end does not run. They stay as defence in depth (0006 says so).
- **Accepted, correct:** in a member of a class template, a `forall` lambda parameter written `decltype(member)` is
  accepted and substituted correctly (il-analyst, `--il_display` check); `sizeof(T)` in a lambda body is accepted.

**M4.1 code audit notes (2026-10-20, not blocking):**
1. `lint_viper.py` does not flag a forward `goto` into a loop body; only VIR's nesting rule (and Silicon's exception,
   S9c, exit 3) guard it. Add a token-level loop-nesting check to the lint.
2. Three encoder rejection branches look unreachable and have no test (a `break`/`continue` whose target cannot be
   determined, a non-integer scrutinee, a case value of another type); they fail closed.
3. The probe/spike evidence of M4.1 is under `build/m4.1/` (gitignored; the facts are in `il-inventory.md`);
   two spike headers (S1b, S7f) state stale expectations, the record's claims are right.

## 7. Known gaps carried

- Termination: a recursion cycle through another unit (0013 item 5) — the contract manifest, M6 (M7 if it does not fit).
- The `predicate`/`pure` marker on the first declaration — M5b (an EDG registration feature).
- Still rejected for `forall` in templates: a lambda depending on `T` only through `decltype` of a function parameter
  (above), function parameter packs (`attr_function_parameter_pack`), `sizeof...` of a template pack, a lambda that
  names the template it is in (`h` undefined). EDG bug 0021 does not arise: cerastes reads C++ in Clang emulation.
- Closed: the five compile-time uses of `constexpr` spec functions that left no trace (P, registered).
- Attributes in positions where EDG records nothing (0011 item 7); M3's rejected constructs (0012 item 12) for M4 and M8.

## 8. Session plans: M4 (the rest of C) and M5 (generic templates)

**Re-sequencing (owner, 2026-10-20).** Planning M4 (generic templates) showed that Gate G2's six VeriFast tests use
pointers, references, structs and constructors, which M3's scalar fragment cannot express. The owner renumbered the
plan (`plan.md`, "Milestone renumbering"): the **new M4 is the rest of C** (strictly C, sub-milestones M4.1–M4.8,
pointers in M4.5a–f, VerCors as the reference); generic templates are **M5** (M5a–M5g); concepts M6; **C++ memory
and classes M7** (references, `new`/`delete`, classes; G2's parts that need them are deferred there); inventory M8;
evaluation M9. Session order: M4.1 (the session that wrote this) → CI (§4) → mutation triage (§5) → M4.2 … M4.8 →
M5a … M5g → M6.

**Rules for every session below** (memory `session-size-few-hours`): planned at ≤ 4 h with fixed scope and stop
rules; owner decisions and pre-approved expectation changes asked up front; the decision record before code
(invariant 6); must-verify/must-fail pairs (invariant 2); docs in the same session (invariant 8); three Sonnet agents
with disjoint files (edg-backend: `src/backend`, `src/driver`, build; il-analyst: probes, IL evidence, Silicon
spikes, `src/vir` + `src/viper`; test-mutator: tests, harness, `mutate.py`, `lint_viper.py`) and the Opus
soundness-auditor (one round per diff plus a re-audit of blocking fixes; should-fix items to the handoff);
single-file runs while working (`build/cerastes verify` directly: `run_tests.py` locks `build/vpr`); `check.sh` at
most twice; anything that needs the x86-64 box is its own session. Record numbers are the next free ones when written
(0014 is M4.1's).

### 8.1 M4 — the rest of C

**M4.1 Control flow** (done in the session that wrote this; see `internal/status.md`).

**M4.2 Scalar declarations — split into M4.2a, M4.2b, M4.2c** (owner, 2026-10-21: planning found about three
sessions of work; a Plan-agent review found the cross-mode traps listed below). Owner decisions for all three:
retirements pre-approved (`reject_goto.c`, `reject_noreturn.c` in M4.2a; `reject_case_variable.cpp`, `reject_enum.c`
in M4.2b; `reject_uninit_local.c` in M4.2c — each deleted in the commit that lands its feature, replaced by
positives, error twins and rejection tests of what stays rejected; `reject_global.c`, a mutable global, stays); a new
category `noreturn`; a failing `assert(e)` is `check`; "verified" means returns satisfying `ensures` or ends the
program through a noreturn call (0013 item 6 amended).

**M4.2a Jumps and ends** (run 2026-10-21; record 0015). Forward `goto` and labels, "outward and forward" (the label
is a later statement of a block enclosing the goto, after the encoder's flattening of generated blocks; never into a
block, branch, switch body or loop; never past a declaration); `[[noreturn]]`/`_Noreturn` (`ensures false`, every
reachable return asserts false, category `noreturn`); trusted libc `abort`/`exit`/`_Exit`; `<assert.h>` `assert`
recognised by the macOS expansion's shape and checked as `check`. See `internal/status.md` for what landed.

**M4.2b Named constants and enumerations** (record 0016).
- `const`/`constexpr` globals, constant-initialized, defined in a verified file: uses take EDG's folded value as a
  literal; a `globals__wf` method encodes each initializer by its original expression (used or not), asserts its
  obligations at the initializer, then asserts equality with EDG's value as `internal` (as `DoOnce` does). Also: C's
  `const int g;` (tentative, zero), const globals in contracts and annotations (`variable()` rejects them in
  contracts today, `encode.cpp:642`), `const int g = g + 1` (C++), a self-test hook perturbing the literal (exit 3).
  Rejected: `extern const` without a definition here, dynamic initialization, system-file globals, `volatile`,
  `static const` locals (M4.5b with the other statics).
- **Cross-mode trap (review, blocking):** a const global in a `do … while` condition is a folded constant 0 in C++
  (`DoOnce`, no measure) but an `enk_variable` in C (a measured loop): a global in the condition must never make a
  `DoOnce`, in either mode (0014 item 5's argument relies on it).
- `case k` (const local), `case N` (const global), `case RED` (enumerator): `vir::Var::is_const`, `check_program`
  rejects an `Assign` to a const local and accepts only const locals in case values (`check.cpp:317`), an `internal`
  guard per case value against EDG's `case_value`. Case-value obligations are path-dependent today (asserted before
  the dispatch, `lower.cpp:563-579`; an `if (0) switch` skips `case 1 << 40`, a C 6.6p4 constraint violation): assert
  them path-independently.
- Enumerations, one meaning (the stricter rule in both modes): an enum type's values are its C++ value range
  ([dcl.enum]/8, computed by cerastes; the underlying type's range when fixed), every conversion to an enum type is
  checked against it (`conversion`); enumerators are `int` constants (with a fixed underlying type, the enum's type,
  in C23 and C++); enumerator values not fitting `int` rejected; implicit promotion of a non-constant enum operand in
  arithmetic or ordering comparisons rejected (C promotes `unsigned int`-compatible enums to `unsigned int`, C++ to
  `int`), `==`/`!=`/`switch`/assignment/explicit casts allowed (mode-independent while every enum value is in its
  value range); compound assignment and `++`/`--` on enums rejected (they bypass EDG's conversion nodes in C, 0010
  item 6); a `bool` underlying type rejected; `enum class` (C++) accepted. Enumerators without an initializer have no
  `expr`.
- Probe P25: EDG's compatible type for C enums with and without negative enumerators (clang/GCC are not evidence),
  enumerator types per mode, fixed underlying types, the conversions EDG inserts; fix il-inventory's contradiction
  (L1084 "`tk_enum`" vs L1344 "`tk_integer` with `enum_type`").

**M4.2c Locals without an initializer** (record 0017). Recommended (review): definite assignment over VIR — one
forward pass suffices because every jump is forward (intersection at joins and labels; a loop head's state is its
entry state); no new category; the rejection names the read. Alternative: a ghost "defined" flag per local with
category `uninitialized` (flags false at method entry, since Viper locals start arbitrary, `emit.cpp:181`; reads
include compound assignment, `++`/`--`, arguments, returns and annotations; a monotone loop invariant for locals
declared outside the loop; the automatic range invariant, `lower.cpp:676-691`, must not fail on an unassigned local);
decide by spike. Small: may share a session with M4.3's start.

**M4.3 Expressions and sequencing.** The comma operator; assignments as values; `x++` inside expressions; calls in
guarded operands (`&&`, `||`, `?:`) and loop conditions (lowered as branches and re-evaluation); unsequenced calls
(C's sequencing rules: an unsequenced modification of one object is undefined behaviour and must be rejected or
detected). Record (sequencing), amending 0012 items 4 and 6. Mutation operators for the new forms.

**M4.4 Bitwise operators.** `& | ^ ~` and `&= |= ^=` on signed and unsigned types (C23 two's complement). Spike first
(Silicon `v.26.08`): total prelude functions with bodies over `Int` (bounded recursion on bits) vs an SMT bit-vector
interpretation domain (needs a lint rule and a record: invariant 4). Record (bitwise), amending 0010 item 9. Note for
M5: integral `T` still rejects bitwise operators; per-instance fallback now verifies them (`low_bit`).

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
  added before M4.5b.
- **M4.5b Pointers to scalars and globals:** `&x`, `*p`, NULL, pointer parameters with `owns`/`reads`/`old(*p)`,
  pointer equality, an aliasing trap that must fail, mutable globals and `static` locals as cells with frames; ports
  of VeriFast's `examples/swap.c`, `examples/address_of_local.c`, `examples/globals.c`.
- **M4.5c Structs:** field cells, `.`/`->`, struct values (copies, initializers, by-value parameters and results),
  nested structs, pointers to fields; ports of `tutorial_solutions/account.c`, `deposit.c`.
- **M4.5d malloc and free:** trusted contracts for `malloc`, `calloc`, `realloc`, `free`, `abort`, `exit` (system
  files are used only through contracts, 0011 item 4), allocation blocks, ownership on free, leaks (record); ports of
  `tutorial_solutions/malloc_block.c`, `examples/cell.c`.
- **M4.5e Arrays and pointer arithmetic:** local arrays, `a[i]`, `p + i`, `p - q`, comparisons within one object,
  `owns_range` and quantified permissions, `forall` in C; ports of `examples/carrays.c`, `examples/insertion_sort.c`,
  array sum and max.
- **M4.5f Predicates and pure functions:** predicate and pure bodies encoded (0011 item 4 lifted), `fold`/`unfold`,
  recursive predicates; a linked list (push, pop, length), `tutorial_solutions/stack.c`, `reverse`. Lemmas and ghost
  inductive data are open design points (`internal/upstream/attribute-registration.md` Appendix A): a record decides
  whether they are in M4.

**M4.6 Chars and strings:** string literals as read-only arrays, `char` buffers, `strlen`/`memcpy`/`memcmp`/`strcmp`
contracts, `main(argc, argv)`; ports of `tutorial_solutions/strlen.c`, `memcpy.c`, `memcmp.c`.

**M4.7 Function pointers:** contracts on function types, calls through pointers; ports of `tutorial_solutions/byref.c`,
`examples/functions/functions.c`.

**M4.8 C parity close-out:** the VeriFast C differential set (the ports above, run with VeriFast in Docker as M5c
sets up, or recorded outcomes), the full mutation run over the M4 tests, the Memory and ownership page complete,
M4's acceptance (plan.md), and the deferred list for M8 (unions, floating point, `volatile`/`_Atomic`/`_BitInt`/
vector types, variadic functions and `printf`, several translation units, concurrency).

### 8.2 M5 — generic templates (designed in the M4.1 session)

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

**M5b EDG (box).** After the CI session's upstream merge; best just before M5g. Fork commit Q: a registration flag
refusing an accumulating attribute that first appears on a later declaration (check site
`attach_decl_attributes_to_entity`, `decls.c:806-812`, next to `reconcile_must_match_attributes`,
`attribute.c:4131-4230`; next free flag bit `0x8`, `attribute.h:401-437`; precedent
`ec_comparison_defaulted_in_class_must_be_first_decl`, `decls.c:10631`; `first_decl` is set on namespace-scope
paths — check member and template paths; GH tests in `tests/tests/changes/GH-*.sft.cpp` with an `-ok` companion,
`src/Changes`, `doc/source/ext_intf.rst`); cerastes registers it for `predicate`/`pure`
(`register_attributes.cpp:51-52`). Fork commit R: EDG diagnoses series L's silent error node for a `decltype(p)`
lambda parameter (§6); the `decltype` tests move from `cerastes-annotation_substitution_error` to EDG's diagnostic
at the same line (owner's rule). Plus the close-out audit B's missing tests (§6 items 2–4). Fail-before/pass-after,
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

## 9. Process notes

- **What kept it short:** fixed scope with stop rules, owner decisions asked up front (upstream merge deferred, the
  r4a rule pre-approved), one audit round per diff plus one re-audit of blocking fixes, single-file harness runs
  during the work, three Sonnet agents with disjoint files.
- **Worktree isolation:** the session ran in a git worktree; its submodules were cloned with `--reference` to the main
  checkout's module stores (`git submodule update --init --reference …`), and bootstrap took a few minutes.
- **check.sh found two harness consequences** of the pin (a stale self-test allowlist entry, the oracle's closure
  rule): run it once early in a session that moves the pin.
- **The approved r4a rule was too narrow** for a file whose other errors stop the back end; asking up front with a
  concrete fallback would have saved a round trip.
- **M4.1 (2026-10-20):** the implementation (encoder, VIR, 86 tests) was done in about 1 h 15 min; the
  test-mutator's mutation runs on the new positives (two runs, one discarded after a rebuild) took most of the rest.
  Run mutation on new tests once, after the last rebuild, with a time box; the session's clock readings mid-run were
  misleading, so check `date` against the budget before starting any long run.
- **Commits:** `git add <paths> && git commit` commits everything staged, including an earlier `git rm`: stage
  deletions only in the commit that lands their feature (one record commit picked them up and was redone).

## 10. Acceptance (commands and results)

| # | Command | Result |
|---|---|---|
| 1 | `scripts/check.sh` | all green, 14 min 31 s: harness self-test 931, oracle self-test 74, lint self-test 125, mutate self-test 154; tests 2350 passed, 0 failed, 1 skipped (native-ok: Apple clang 586, GCC 586); 555 `.vpr` lint clean; VIR self-test 204 lowering + 75 rejection cases, 204 passed; oracle 134 goldens, 1915 facts, 0 failed; docs 0 problems; `NOT YET PUSHED` for the pin, as expected |
| 2 | `python3 scripts/run_tests.py -j 8 tests/scalar doc/modules/ROOT/examples` (before check.sh) | 1320 passed, 0 failed (native-ok: Apple clang 279, GCC 279); baseline before the session 1034 |
| 3 | `python3 scripts/run_tests.py --count-error-twins` | 113 (82 + 31 new positives) |
| 4 | `python3 scripts/run_tests.py --check-headers` | 861 / 861 ok |
| 5 | VIR self-test (`build/cerastes-vir-selftest build/vir-selftest`, run_tests, lint) | 204 lowering + 75 rejection cases, 0 problems; 204 passed; lint clean |
| 6 | `diff -r -x MANIFEST build/m4-baseline-vpr build/vpr` (M3 tests' Viper vs the session's baseline) | all 412 `.vpr` files of the M3 tests and doc examples byte-identical; none missing |
| 7 | `python3 scripts/mutate.py -j 8 <31 new positives>` | 1403 mutants: 991 killed, 299 bucket, 27 stillborn, **70 survivors → §5** |
| 8 | `python3 scripts/test_mutate.py`, `test_lint_viper.py` | 154 and 125 tests OK |
| 9 | audits (Opus) | 0014: 1 blocking fixed; code: 0 blocking; should-fix items taken; notes §6 |

## 11. Questions for the owner

Each has a recommended answer. Answer inline, as before.

1. **The M4 and M5 session plans** (§8, `plan.md`): do the sizes and the order fit? In particular M4.5 as six
   sessions (design first), and M5 as seven (M5b with the box just before the gate). *Recommended: yes; re-plan M4.5b–f
   after M4.5a's records.*
2. **The 70 M4.1 survivors** (§5): in the triage session, after the CI session? *Recommended: yes, with the two weak
   continue tests strengthened first.*
3. **`case k` with a `const` local** (C++) is rejected (0014 item 6): lift it in M4.2 with `const` globals?
   *Recommended: yes, M4.2.*
