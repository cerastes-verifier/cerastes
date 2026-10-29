# Handoff during M4.5b1c2 — sequencing and placement over memory (record 0039); M4.5b1c1 before it (record 0038)

The next session reads this file after `CLAUDE.md`, its own section in `plan.md`, `internal/status.md` and
`internal/decisions/`. `status.md` has the formal entry. This file adds what a cold start would otherwise have to
rediscover, and holds only what is live (owner, 2026-10-26; consolidated again at M4.5b1c2's start, 2026-10-29): the
text before this consolidation is `git show eab4058:internal/handoff.md`, whose header names every earlier version.
Session plans approved from M4.5b1a on are in `.claude/plans/`; the notes a session consumes move into its plan.

## 1. State

- **On `origin/main`** (`234ede3`): M4.5b1a (PR #15), M4.5b1b (PR #17) and the Python pins (PR #18).
- **M4.5b1c1** (record 0038) is merged into the local `main` (`eab4058`), not yet pushed; its branch
  `m4.5b1c1-loops` is pushed.
- **M4.5b1c2** is in progress on branch `m4.5b1c2-sequencing` (from `eab4058`; plan
  `.claude/plans/m4-5b1c2-sequencing.md`): record 0039.

The EDG pin is unchanged (`f0ed5525af`); the fork's `cerastes` is 23 commits behind upstream `main` (§4.1).

## 2. Owner steps

1. **`main`**: push it (b1c1's merge, `eab4058`).
2. After M4.5b1c2: push its branch and open its PR; both CI jobs run the PR tier.

## 3. Open audit notes (condensed; the full notes are in the commit named above)

**From M4.5b1c1** (audit round 1 on record 0038, round 2 on the code, one re-audit whose blocking item was fixed and
checked by the tests and the auditor's repros, not re-audited: the cap; items not taken and open points):
- *Cut part and what it needs* (record 0038 item 5, from S28 and the audits; b1c2's function measures moved into its
  plan): **b1c3's written atoms** in
  `CERASTES_INVARIANT`: `NOTAUTO(q)` by `Ref` for value and flag and by liveness cell for liveness (round 1 S1), the
  atom's own flag and range reads chain-guarded (S2), case splits of `q` against every automatic place at the loop head
  (S28 P8), amounts and checks as a call's and a return's with `WF` and bounds as `permission` rows at the back edge
  (S4), separation of two written atoms, and P8's tie (`q0 := q`, `(q0 == r) == (q == r)`) so a reassigned `q` gives its
  permission back after the loop.
- *Pinned false alarms* (record 0038, Consequences): a parameter the function reassigns keeps its place out of every
  loop, so `loop_reassigned_param_access.c`, `loop_reassigned_param_return.c` and `loop_invariant_reads_unowned.c`
  (all `tests/memory/negative/`) expect `permission` errors on correct programs; b1c3's written atoms let the user say
  it (§4.2, b1c3's flips; question 68).
- *Cost:* the entry case splits multiply loop verification: one guarded parameter and one cell 2.7 s, two and one 8.3 s,
  two and three 37 s, one and six 10 s (re-audit, load 20-50); four aliasing `reads` parameters with a loop about 50 s
  under load (S28 P3). Tests stay at three or fewer. For M4.8 with question 66.
- *Not pinned:* a loop body without braces has no closing position, so its back-edge error is reported at the loop
  statement (round 1 S5; lowering's fallback, no test); in C++20 a loop's IL position is its first attribute's (P59).
- *Messages:* `cerastes_old(*p)` in a loop invariant is rejected, but in C as "a conversion to or from a pointer type"
  with a `const void *` type error and in C++ as `old_outside_ensures` (round 2 note 5); the new rejections end with
  their diagnostic's fixed record ("(decision 0012)", "(decision 0037)") though 0038 decides them (re-audit note 4; the
  carried note on record citations).
- *By design:* in an invariant `==` is identity with no liveness check, so an invariant may compare a dangling pointer
  (round 2 note 7); every use in code keeps its check.
- *Lint* (round 2 note 8): a `Perm` local as a non-first target of a multi-target call (`a, kv := m()`) is not seen (the
  emitter never writes it); an invariant's `acc` amount is not required to be a `Perm` local. The lint's
  "while without decreases" rule reads the first line holding `{` as the body, so a trigger set in an invariant is a
  false positive (pre-existing; the test filters it).
- *Corrected:* `CERASTES_CHECK(&x != &y)` exited 3 when the code never takes `&x` (the front end does not flag a use in
  an annotation, P59); it is `unsupported_expression` now (0032 item 4 amended).

**From M4.5b1b** (audit round 1 on record 0037, round 2 on the code, one re-audit; items not taken and open points):
- *The cell splits at calls were checked by timing, not re-audited* (the cap): moving the `reads` pairs' case splits
  from the entry to the calls (record 0037 items 8–9) restored four- and five-parameter times (5.7 s, 22 s); six still
  time out (exit 3, documented). b1c1's audit round 2 re-read `call_atoms`' splits, also inside loops, and found no exit
  3 (a `reads` call through a pointer the loop reassigns, 2.3 s; two aliasing `reads` and an `owns` parameter, 6 s).
  The cost itself is fixed in M4.8 (§8 question 66).
- *Over-strict, fail closed:* `restrict` beside a same-typed parameter with only `live` is rejected even when the
  function never dereferences it. (`c ? owns(*p) : reads(*p)` and `return &local;` are b1c2's: its plan.)
- *By-name cell checks are now `permission`* (record 0037 item 11): an encoder bug that dropped a cell's permission
  would now read as a user error instead of exit 3; a VIR self-test hook could pin it (round 1 note).
- *`f__wf`'s added facts* (aliasing values equal, a `reads` place's post value equals its pre value, `owns` pairs
  distinct within one clause list) are trusted reasoning about the model; the audit rounds checked them by
  counterexample search, not by proof.
- *C++'s `p == NULL ? d : *p` with a `const` pointee* stays rejected (an lvalue `?:`, already in b1a), now with a
  readable message.

**From M4.5b1a** (audit round 1 on records 0032–0036, round 2 on the code, a re-audit of round 2's fixes):
- *Not pinned:* a bare `p;` with `p` dangling reports `lifetime` (checked by hand), but no test holds it: EDG also warns
  #174 (`expr_has_no_effect`), and #550 in C++, which a test with an `error` line may not list
  (`VERIFY_WARNING_TAGS` is closed). The `(void)p;` twins pin the rule.
- *Evidence for later sessions* (in `il-inventory.md` since b1b): P52 is b1c3's (`owning`), P54 M4.5b2's, P55 M4.5d1's.
  (The compound store's notes are b1c2's: its plan.)
- *Messages:* the new rejections reuse `unsupported_type`, `unsupported_expression` and `unsupported_statement`, whose
  fixed texts cite "(decision 0012)" (the carried note on record citations below applies); the type message still says
  "only integer types, bool and the enumerations … are" although pointers to them are now accepted in locals.

**From S5** (one Opus round on the encoder diff: one blocking item, fixed and re-audited; record 0031 items 5–7):
- *Should fix, not taken:* a hand-written `((void)sizeof(x++), x) ? (void)0 : __assert_fail(…)` on Linux matches and
  verifies; the general path would reject it as `side_effect_not_evaluated` (0020 item 5). Sound (the `sizeof`'s
  operand is never evaluated), but more permissive than the rest of the encoder: require the `sizeof`'s operand to be
  a call of the system `__assert_single_arg`.
- *Tests:* the glibc matcher's near misses (a `(void)1` success arm, a non-`sizeof` comma operand, `__assert_fail` in
  the success arm, another noreturn function) can only run against glibc (§8 question 59).
- *Driver, pre-existing:* `posix_spawn_file_actions_addchdir(_np)`'s result is not checked and Silicon is given the
  relative `program.vpr`; if adding the action failed (ENOMEM), Silicon would run in the caller's directory. Passing
  the absolute path removes it. `LC_ALL=en_US.UTF-8` may not exist on a bare Ubuntu; no `.vpr` holds non-ASCII bytes.
- *Per-platform `.vpr`, no test today:* C++'s `assert((bool)x)` loses the user's cast on Linux; `assert(E::b)` with a
  scoped enumeration compiles only under glibc (record 0031 item 5).
- *Target differences that stay* (record 0031 item 7): `__thread` and the x86 GNU attributes are accepted on Linux only.
- *Second round, on the macro exemption (record 0031 item 9; two blocking items, fixed and re-audited):* system files by
  EDG's flag alone (a user file that a system header includes got the exemption and verified), and a pair across two
  inclusions; also exact text instead of tokens. **For M4.5d1:** vlibc's headers come through `--sys_include` (0030
  item 4), so EDG flags them as system; its library class must count as a verified file for this exemption, or be
  excluded from it, before vlibc lands (add it to 0030 item 4's audit list).

**From M4.5a** (four Opus rounds on records 0028–0030: round 2 five blocking, round 3 three blocking, round 4 one
reopened, all fixed in the records; every should-fix item taken in the text; the code of 0027: round 4, no blocking item).
What the next sessions must carry out, beyond the records' text:
- *Registered attributes are skipped by `check_attributes`* (`encode.cpp:418`): register each target in the session that
  implements it (0029 item 1), never earlier.
- *P55 for M4.5d1:* the attribute allow-list rejects a system declaration's attributes (`[[const]]`, `[[alloc_size]]`,
  `[[__cold__]]`) once a user or vlibc file redeclares the function; check by the attribute's own position.
- *EDG, not chased* (memory: only bugs that block cerastes): in C23, `typeof(*cp)` for a `const int* cp` drops `const`
  (clang keeps it; P53, five shapes). Harmless for `cerastes_old`.
- *Costs to watch:* liveness checks at every pointer use cost about 1.5–1.7× time and text on S26's scale program; deep
  `^` mask chains (0027): range claims are fast to depth 32, identities only to the measured depth (record 0027 item 4).
- *Completeness gaps accepted by the records:* `&s.x < &s.y` reported (members are different blocks); signed/unsigned
  and enumeration variants through `void*` cannot be written; a list traversal's truth tests need liveness inside
  M4.5f2's predicates; `forperm` sees only top-level tokens.
**From M4.4d** (audit rounds 1 and 2 and the re-audit; items not taken):
- *Masks that are constants only through a call argument* (round 2 note 4): `f(v, 4294967295u)` with
  `requires((v & m) == v)` rewrites the explicit check lowering writes at the call, but Silicon's own check of the
  callee's precondition still reads `c_uand_64(p_v, p_m)` and may run to the timeout (exit 3, never a verdict; a probe
  took 90 s). Record 0026 item 9 and the Integer semantics page say only a mask written as a constant is decided. §8
  question 51.
- *The fallback keeps today's false alarms* where step 2 is used (0026 item 6); no existing test is accepted by step 2
  (M1), so none shows one. §8 question 50.
- *The mask positives were not mutated* (the plan's cut 1: a mutated constant lands on an undecided shape and runs to
  the 300 s timeout). M4.8's run over the M4 tests takes them (§8 question 49).
- *The lint of `%` and `\`* (round 2 note 9) reads only the divisor: it cannot tell a C `%` by a power of two emitted as
  Viper's `%`, and it exempts the prelude bodies by name (`c_rem`, `c_urem`; `c_div`, `c_udiv`, `c_shr`). The guarantee
  is structural: `low::mod_pow2`/`div_pow2` are the only constructors, and user functions are named `f_<name>__<id>`.
- *The VIR mask cases pass with either encoding by design* (round 2 note 10); `test_a_constant_mask_is_emitted_as_a_
  remainder_not_as_a_bit_function` checks the emitted text.
- *Two failing `requires` clauses at one call print the same message twice* (seen on the retired test under exact sets;
  0011 item 9's harness rule fails such a test): tests choose contracts that fail once (`call_split_requires_pre.c`).
- *A duplicate VIR case:* `calls_drop_exact_set_hook_off` is the program and expectation of `calls_fixed_order_hook_off`
  (harmless).

**From M4.4c** (audit rounds 1 and 2; items not taken):
- *Shapes accepted and listed* (record 0025 item 6, "still accepted"): `RED == RED` (the front end folds it to 1 without
  an expression); clang's `-Wparentheses-equality`, `-Wswitch-bool`, `-Wfor-loop-analysis`, `-Wmisleading-indentation`
  (clang and GCC), GCC's `-Wmultistatement-macros`, clang's `-Wempty-body` on loops; R10 with a named constant
  (`uc == DEF`, `const int DEF = 300;`, clang's C++ mode warns) or a constant expression (`uc == 256 + 44`); R7 does
  not look into a `?:` arm that is a comma ending in a constant. All are for the warning sweep, now an **M8** entry (`plan.md`).
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
- *Planning's finding A* existed since M4.3a; no test had the shape. Item 2's placement is revisited with item 1's kinds
  of access for globals in M4.5b2 (address-taken locals: b1c2's plan).
- *From the mutation review:* R1–R13 do not reach the body of a `pure` or `predicate` function (not verified before
  M4.5f; the Verifying page says so), and `static_assert_pure_body.c`'s listed entry goes stale then; the doc example
  `verifying/void_conditional.c` keeps a redundant requires conjunct `(c == 0 || n > 0)` (four listed equivalents; the
  `m > 0` pattern of `void_conditional_for.c` would make it needed); a body `p | q` against an exact `&` contract runs to
  the timeout (0023's documented limit), so `not_eq_and_bools` states its contract with `&&`.

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
builds serialized (`lockf build/<session>/build.lock cmake --build build/cmake`); `check.sh` at most twice; the EDG
suites on the x86-64 host are their own session, but any x86-64 test run a session needs goes to `matt@clang4.cpp.al`
automatically (owner, 2026-10-29: not Docker multi-arch on the Mac, which is only the fallback when the host is down).
Record numbers are the next free ones when written (0039 is next). The
approved plan is saved as `.claude/plans/<milestone>-<topic>.md` (plan.md, "Session protocol").

Finished sessions: their plans are in `.claude/plans/` (from M4.5b1a on), their results in `internal/status.md` and the
commits it names. The warning sweep (was M4.4e) is an M8 entry in `plan.md`.

### 4.1 EDG box session (pending since M4.2c)

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
(4) **The upstream merge** (owner, 2026-10-29, M4.5b1c2 planning: deferred to this session rather than done at a
session start): the fork's `cerastes` is 23 commits behind upstream `main` (`5e001111a4` at the last fetch); merge it
(never rebase), run the full suites on the box, then move the pin with `check.sh` green and the goldens' diffs reviewed.

### 4.2 M4.5 Pointers (sessions b1–f3; designed in M4.5a)

**The design** is records 0028 (the memory model: E2, the `Block` domain, liveness, explicit checks), 0029 (the
annotations and C's spec functions), 0030 (vlibc), 0032 (b1a's slice) and 0033–0036 (the four axioms); evidence in
`il-inventory.md` "M4.5a" and "M4.5b1a"; the encoding pinned by the fixtures in `tests/viper-smoke/memory/`.

**The VeriFast port inventory** is `build/m4.5a/ports.md` in the worktree (gitignored; its conclusions are below per
session). `malloc_block.c` holds no `malloc` (an address-taken struct local); `insertion_sort.c` is a linked list whose
ghost machinery is not portable as written (an array insertion sort in M4.5e2 replaces it as a benchmark); 14 of the 22
files run VeriFast with `-disable_overflow_check`, so their ports need `requires` bounds; `functions.c`'s predicate
families and constructors, consumed fractions, backward `goto` and `open_module` are M8 entries.

**M4.5b1 and M4.5b1c** were split by the owner (2026-10-28 and 2026-10-29): b1a, b1b and b1c1 are done (records
0032–0038; plans in `.claude/plans/`); owning results (`owning` on functions, `owns_block`, `owns(*result)`, `ensures`
atoms with no partner) moved to M4.5d2. Each session rejects what a later one makes precise (fail closed).
**Pre-approved** (owner): a session may retire or re-expect exactly the rejection pins listed for it below, and no
others; a pin whose construct stays rejected keeps its `expect:` line, and only its message and `Pins:` comment name the
new session.

**M4.5b1c2 Sequencing and placement: in progress** (plan `.claude/plans/m4-5b1c2-sequencing.md`, with the notes of
§3 it consumes; record 0039). Flips:
`tests/memory/negative/reject_{cell_postfix_value,write_in_assignment_value,call_beside_access,call_compound_target,two_memory_calls,heap_read_decreases}.c`.

**M4.5b1c3 `T**`, owning parameters, `*result`.** `T**` and the address of a pointer local; `[[cerastes::owning]]` on
parameters (registered then, target `p` only: `check_attributes` skips registered attributes, so never earlier; P52),
`requires(p == NULL || owns(*p))` and its `ensures`; `*result` as a value, framed by `result == p` through record 0037
item 7; the aliasing trap; `swap.c` L4-11 (its `swap(&x, &x)` twin); `address_of_local.c`'s `address_of_local` and
`address_of_local_uintptr_t`; **from b1c1 (cut 2):** `owns`/`reads`/`live` in `CERASTES_INVARIANT` (record 0038 item 5,
§3), which lifts the pinned false alarms of reassigned parameters; the Memory and ownership page
completed. Flips: `tests/memory/negative/reject_{address_of_pointer,pointer_to_pointer,result_value,atom_in_loop_invariant,atom_in_do_once_invariant}.c`
and `reject_atom_in_loop_invariant_cpp.cpp`; and (owner, 2026-10-29, question 68) re-expecting
`tests/memory/negative/loop_reassigned_param_{access,return}.c` and `loop_invariant_reads_unowned.c` to `verified`, each
with a new must-fail twin, where the test gains the written atom (a test left without one keeps its false alarm).

**M4.5b2 Globals, `static` locals, initialization through pointers.** Mutable globals as `GB_g()` blocks, `program_start`
for `main`, frames through contracts (`owns(g)`); `static` locals (0028 item 5: the implicit `owns` or a rejection in a
recursive cycle); 0025 item 2's placement revisited with globals; 0019's flag per cell; `[[cerastes::uninit]]`,
`ref_to_uninit`, `must_init` (registered then) and `--strict-init` (0029 item 2). Ports: `global_points_to_syntax.c`
(adapted: no `leak`), `globals.c`'s scalar part.

**M4.5c Structs.** Members as blocks of their own (0028 item 16), `.` and `->`, struct values (copies, initializers,
by-value parameters and results), nested structs, pointers to members, `owns(*s)`, non-recursive owning fields
(`[[cerastes::owning]]` target `d`). Ports: `malloc_block.c` (the address-taken struct local; its `free(&local)` twin
waits for M4.5d2), the struct half of `swap.c`.

**M4.5d1 vlibc** (needs the repository; any time after M4.5a, best after S5). The repository's layout
(`include/vlibc/*.h`, the shim directory, `src/`, CMake, `LICENSE`, CI compiling every header with clang and GCC in both
modes), `third_party/vlibc` pinned (0001), the driver's include order and the vlibc-file trust rule (0030 items 3–4,
P55), `abort`/`exit`/`_Exit` moved out of `trusted_libc_reason` with their tests' Viper unchanged.

**M4.5d2 `malloc` and `free`.** vlibc's contracts (`malloc` `ref_to_uninit`, `calloc` initialized, `realloc`, `free`),
heap blocks with the slot −1 token, the free checks (`invalid-free`), the leak rule by `forperm` and a record on `main`'s
leaks (`leak`), owning locals and globals (`[[cerastes::owning]]` target `v`), and **owning results** (from b1c, owner
2026-10-29): `[[cerastes::owning]]` on functions (target `r`), `owns_block`, `owns(*result)` and `ensures` atoms with no
`requires` partner (a callee handing a place's permission to its caller: an output ghost amount). Flips:
`tests/memory/negative/reject_{atom_on_result,ensures_atom_no_partner}.c`. Ports: `account.c`, `deposit.c`, `swap.c`'s
`main`, `globals.c`, `malloc_block.c`'s `free` twin.

**M4.5e1 Arrays.** Local arrays as blocks, `a[i]`, array parameters, `owns_range` as quantified permissions over the
absolute slot, the marked range rows in the driver (owner, after S25), `forall` in C (0029 item 4: its planning decides
between pure functions with contracts and moving M4.5f1 first), sum and max. P38: a braced initializer with more than
1000 elements keeps no `expr` for its elements (`REDUCE_BACKING_EXPRESSION_USE`), only an `orig_type`; read every
element through `constant_value`, which rejects that shape, never through `ck_aggregate`'s values directly, or a
wrapping element would pass unchecked. M4.4b2's types computed from a constant (`sizeof`, `typeof`, typedefs). Port:
`carrays.c` (lists → `forall`; its `should_fail` at L35 and L42: below and one past the end).

**M4.5e2 Pointer arithmetic.** `p + i`, `p - i`, `p - q`, relational comparisons within one object, one-past-the-end,
`malloc(n * sizeof(T))`; `out-of-bounds`; relational comparisons and `p - q` under `pointer-comparison` (the category and
the `==` exception of 0028 item 3 land in M4.5b1b). Port: an array
insertion sort (in place of `insertion_sort.c`).

**M4.5f1 Predicates and pure functions.** Bodies encoded (0011 item 4 lifted), `fold`/`unfold`, non-recursive
predicates. Port: `cell.c`. §3's notes on R1–R13 not reaching `pure`/`predicate` bodies and `static_assert_pure_body.c`
go stale here.

**M4.5f2 Recursive predicates.** A linked list (push, pop, length), recursive owning fields (`next`), leaks inside
predicates (0028 item 6). Ports: `stack.c`, `sum.c`, `popn.c`.

**M4.5f3 Lemmas and ghost sequences.** Its record decides whether lemma functions and ghost sequences are in M4 (they
are needed for content specifications: `strlen`'s zero-terminated string, `memcpy`'s and `memcmp`'s contents).
`internal/upstream/attribute-registration.md` Appendix A lists the open design points. Ports: `reverse.c`, `sum_full.c`,
`push_all.c`.

### 4.3 M4.6–M4.8

**M4.6 Chars and strings:** string literals as read-only arrays, `char` buffers, vlibc's `<string.h>` (`strlen`,
`memcpy`, `memcmp`, `strcmp`: contracts, and implementations verified in vlibc's CI), `main(argc, argv)`; character
access to other objects stays rejected (0028 item 1); `c | 0x20`-style case bits are decided (0027). Ports:
`strlen.c`, `memcpy.c`, `memcmp.c`. Open point: a zero-terminated buffer of unknown length needs a recursive predicate or a
built-in notion (M4.5f3 decides).

**M4.7 Function pointers:** contracts on function types, calls through pointers; ports of `tutorial_solutions/byref.c`,
`examples/functions/functions.c` (without its predicate families, an M8 entry).

**M4.8 C parity close-out:** the VeriFast C differential set (the ports above, run with VeriFast in Docker as M5c
sets up, or recorded outcomes), the full mutation run over the M4 tests and the verified doc examples never mutated
before (owner, 2026-10-27, §8 question 10), the mask positives with mutants of a mask constant counted as "undecided"
(§8 question 49), the cost of aliasing `reads`/`live` parameters (§8 question 66: fix it here, e.g. Silicon's joins
or a per-call split order; record 0037, Consequences), the Memory and ownership page complete, M4's acceptance (plan.md),
and the deferred list for M8
(unions, floating point, `volatile`/`_Atomic`/`_BitInt`/vector types, variadic functions and `printf`, several
translation units, concurrency).

### 4.4 M5.0 and M5 — type traits and generic templates (designed in the M4.1 session)

**M5.0 Type traits** (owner, 2026-10-26, old §11 question 39; `plan.md` "Before M5"). After M4.8 and before M5a. M4.4b2
skips a static assertion whose condition calls a function, uses a type trait (`enk_builtin_operation`, a variable
template such as `std::is_integral_v<T>`), a concept-id, or names an entity of a system header (`std::numeric_limits`).
M5.0 decides how cerastes reads such conditions (and type traits elsewhere: contracts, `if constexpr`), so that M5's
integral `T` and M6's requirement contracts rest on it. Planned at its own planning, ≤ 4 h.

**Before M5: typed locals in the mutation classifier** (owner, 2026-10-27, §8 question 11: "Not now, but annotate that
this needs to be addressed before M5"). `mutate.py`'s classifier does not read local declarations, so about 20 listed
equivalents (e.g. `assert_holds.c`) rest on a reviewer's reading; teach it to read them, with its own audit, before M5a
(in M5.0 or a session of its own).

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
  `assert` in a `for` increment. Each is rejected with a diagnostic and an M8 entry (glibc's `assert` expansion is
  recognised since S5, record 0031 item 5).
  A `goto` into a block or an `if` branch is lifted only when a ported example needs it, after a Silicon spike with
  must-fail twins (owner, 2026-10-27, §8 question 5).

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
- **A baseline that agents cannot disturb** (M4.5b1a): run it from the main checkout's own build (`main` there, its own
  `build/` and `build/vpr`), copying its `build/vpr` into the worktree's `build/<session>-baseline-vpr`. Then the
  worktree's build is free for the implementers from the start; in M4.5b1a an agent's build relinked `build/cerastes`
  under a baseline that shared it. Run `scripts/bootstrap.sh` in the worktree first when it was last built for another
  branch (a stale EDG stamp stops `run_tests.py`).
- **Dependabot and the CI guard** (M4.5b1c1): `scripts/test_run_tests.py`'s `CiGuardTests` holds a model of
  `.github/workflows/ci.yml` with every action's SHA, and record 0001 lists them; a Dependabot bump changes only
  `ci.yml`, so `main` failed the harness self-test from PR #16's merge (`fca0cbc`, `actions/setup-python` v7.1.0) until
  this branch updated both. Each later bump needs the same two edits (or a change to the guard).
- **Viper against the baseline without Silicon** (M4.5b1c1): regenerating every kept `.vpr` and `.map` with
  `build/cerastes` and comparing them byte for byte with the baseline's (`build/m4.5b1c1/lower/vpr_one.sh`, about 30 min
  under load) answers "did any existing program's Viper change" long before `check.sh`; give each concurrent run its
  own output directory (two runs sharing one raced and printed false `MISSING` lines).
- **Parallel Silicon runs saturate the Mac** (M4.5b1b: load averages up to 215 while an agent ran a test directory and
  another timed programs): time single programs only on a quiet machine, and say so with each timing.
- **Twins and Silicon's continuation:** after a failed check Silicon goes on with the failing value unconstrained, so a
  twin whose failed read flows into an `ensures` fails twice; give such twins an `ensures` that holds for any value.
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

## 7. Acceptance

Each session's acceptance commands are in its plan (`.claude/plans/`); the results of finished sessions are in
`internal/status.md` (the latest entry) and the commits its history names.

## 8. Questions for the owner

Each has a recommended answer. Answer inline, as before. Answered questions 1–71 are recorded in the decision records,
the session plans and the handoffs reachable from the one named at the top. The latest: 65 (`return &local;` rejected
like the compilers), 66 (the cost of aliasing parameters, M4.8), 67 (the `owns`+`reads` rule path-precise), 68 (b1c3
may re-expect the three pinned false alarms of reassigned parameters, §4.2); at M4.5b1c2's planning (2026-10-29): 69
(the upstream EDG merge and the pin move wait for the box session, §4.1), 70 (unsequenced accesses that name one object
are rejected like the compilers, `unsequenced_modification`; the shapes that depend on aliasing are path-precise
`unsequenced`), 71 (b1c2 has no time cap and no cuts). None is open.
