# Handoff after the M3 close-out (2026-10-20)

The next session reads this file after `CLAUDE.md`, its own section in `plan.md`, `internal/status.md` and
`internal/decisions/`. `status.md` has the formal entry. This file adds what a cold start would otherwise have to
rediscover. The M3 handoff (with the owner's answers) is `git show 80b9e35:internal/handoff.md`; the M2 one is
`git show e8830f9:internal/handoff.md`.

## 1. State in one paragraph

The M3 close-out is done on branch `worktree-m3-closeout` (§2 step 1 brings it to `main`). The pin is the fork's P
`b9519388ea` (EDG bug 0020 + series L, M, N, P): a lambda in a contract can name a template parameter, in function
templates and member templates too, and `predicate`/`pure` carry `RAF_NO_CONSTANT_EVALUATION`, so the front end
refuses to evaluate a spec function at compile time. The tooling should-fix items of the M3 audit are done; the
tightened safe-direction classifier moved 74 mutants out of the bucket, and their triage is its own short session
(§5). Planned at ≤ 4 h, the session ran about 1 h of wall-clock time (16:24 to 17:25), plus the planning step.
Nothing is pushed.

## 2. Owner steps, in this order

1. **Bring the session branch into `main`.** The work is on `worktree-m3-closeout` (worktree
   `.claude/worktrees/m3-closeout`). Your main checkout still has your uncommitted `plan.md` and
   `internal/handoff.md` edits; commit `80b9e35` holds them verbatim (`plan.md` plus the folded answers), so
   discard them and fast-forward:
   ```
   cd ~/Documents/cerastes/cerastes
   git checkout -- plan.md internal/handoff.md
   git merge --ff-only worktree-m3-closeout
   git -C third_party/edg fetch fork-local && git submodule update third_party/edg
   scripts/bootstrap.sh --build-only
   git worktree remove .claude/worktrees/m3-closeout   # when you no longer need it
   ```
2. **Push the fork's `cerastes`** (`git -C ~/Documents/compiler push origin cerastes`): it is `b9519388ea`, 6 commits
   ahead of GitHub (0020's merge, L, M, N, P). `check.sh` reports `NOT YET PUSHED` until then.
3. **Push cerastes `main`.**
4. **Branch `0020`** (`c539de4101`) is kept for the upstream PR, but its Linux failure is still unexplained (§4).
5. **Agent evidence:** the fork's `.tmp/agent-*` directories are still there (M3 handoff §2 step 5). The chain
   worktrees `build/wt-0020`, `wt-0020c`, `wt-L`, `wt-M`, `wt-N`, `wt-P` and the branches `tmp-*` are removed.

## 3. What the session did

| Item | Outcome |
|---|---|
| Fork | `cerastes` fast-forwarded `0bcd9bf28a` → `b9519388ea`; nothing new in EDG |
| Pin | `third_party/edg` → `b9519388ea`; at P, before the registration, exactly the two forall-over-`T` tests changed (1220 pass, 2 fail) |
| R1 moves (owner, 2026-10-16) | the negative test and the doc error example became positives with goldens; a member-template positive; five twins (a `decltype` parameter used, unused, of a plain `int`; function parameter packs in a member template and a variadic class template); goldens checked against `--il_display` by a second agent |
| Registration | `predicate`/`pure` get `RAF_NO_CONSTANT_EVALUATION`; 13 tests and 1 doc example move from `cerastes-spec_call_in_code` / `cerastes-spec_marker_on_special_member` to EDG's `expr_not_constant` / `attr_no_constant_evaluation_on_defaulted_function` at the same line (owner rule, 2026-10-20) |
| r4a | outside the rule; owner (2026-10-20): 18 contexts are EDG errors at their own lines (the five no-trace ones included), the 6 EDG accepts are covered by `constant_context_c12, c13, c17, c1b, c8, c16` |
| Tooling | `mutate.py --list`; lint rejects `decreases *` and conditional `decreases`; the classifier (§5) |
| Harness | `annotation_substitution_error` left the self-test-only list (it has a source test now, §6); the walker oracle recognises the closure copies series L makes for instances |
| Records | 0006 (forall, R4a), 0007 (L, M, N, P, the flag), 0013 item 5 (lint), the upstream draft (P, §5.14), plan.md (owner answers, CI session section) |
| Docs | Annotations page: forall over `T` works, what is still rejected, the constexpr passage |

## 4. The CI session (next; owner 2026-10-16 and 2026-10-20)

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

## 5. The mutation triage session (new, short; before M4)

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
   called through a base reference at run time, is accepted (M6, virtual dispatch).
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

## 7. Known gaps carried

- Termination: a recursion cycle through another unit (0013 item 5) — the contract manifest, M5 (M6 if it does not fit).
- The `predicate`/`pure` marker on the first declaration — M4 (an EDG registration feature).
- Still rejected for `forall` in templates: a lambda depending on `T` only through `decltype` of a function parameter
  (above), function parameter packs (`attr_function_parameter_pack`), `sizeof...` of a template pack, a lambda that
  names the template it is in (`h` undefined). EDG bug 0021 does not arise: cerastes reads C++ in Clang emulation.
- Closed: the five compile-time uses of `constexpr` spec functions that left no trace (P, registered).
- Attributes in positions where EDG records nothing (0011 item 7); M3's rejected constructs (0012 item 12) for M7.

## 8. For M4

The M3 handoff §6 still holds (VIR `IntType` needs a type form for `T`; templates in verified files are rejected
today; generic proofs read contracts from the root template, and contracts can now use lambdas naming `T`).

**Sizing.** The last two sessions took 24–36 h; this one about 1 h. M4 as written is several sessions at that size.
Proposed split, about 3–4 h each (question 1):
- **M4a:** the eligibility checker on prototype IL (fail-closed), and the first-declaration marker EDG feature.
- **M4b:** opaque `T` and integral `T` models in VIR and Viper.
- **M4c:** call sites, fallback per instantiation, `internal/soundness/prototype-correspondence.md`, the Generic
  templates page.
- **M4d:** Gate G2: the six ports, their mutants, `internal/differential.md`.

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

## 10. Acceptance (commands and results)

| # | Command | Result |
|---|---|---|
| 1 | `scripts/check.sh` (at `05da953`) | all green, 10 min 19 s: harness self-test 931, oracle self-test 74, lint self-test 113, mutate self-test 142; tests 2064 passed, 0 failed, 1 skipped (native-ok: Apple clang 523, GCC 523); 412 `.vpr` lint clean; VIR self-test 155/155; oracle 134 goldens, 1915 facts, 0 failed; docs 0 problems; `NOT YET PUSHED` for the pin, as expected |
| 2 | `python3 scripts/run_tests.py --count-error-twins` | 82 |
| 3 | `python3 scripts/mutate.py --validate` | 82 tests, 1314 mutants, 125 equivalents, 223 bucket candidates |
| 4 | `python3 scripts/mutate.py --list tests/scalar` | 1314 ids in 0.6 s, no run |
| 5 | full mutation run | not repeated: the 74 leavers go to the triage session (§5) |
| 6 | pins | `third_party/edg` and the fork's `cerastes` at `b9519388ea`; branch `0020` kept; chain worktrees gone |
| 7 | audits (Opus) | A (tooling): 1 blocking, fixed, re-audit closed; B (pin, registration, tests, oracle): 1 blocking, fixed, re-audit closed; should-fix items in §6 |

## 11. Questions for the owner

Each has a recommended answer. Answer inline, as before.

1. **Split M4 into M4a–M4d** (§8) in `plan.md`? *Recommended: yes, so each session is planned at ≤ 4 h.*
2. **EDG's silent error node** for a `decltype(p)` lambda parameter in a function template (§6): a fork follow-up
   commit (diagnose it in the rescan)? *Recommended: in M4a, as a small series commit with a `GH-` test; the cerastes
   check keeps it safe until then.*
3. **Order of the next sessions.** *Recommended: CI session (with the upstream merge and 0020), then the mutation
   triage session (about 2 h), then M4a.*
4. **Audit A's should-fix items 1–5** (§6): *Recommended: in the triage session, since they change which mutants are
   bucketed.*
