# Handoff after M4.5b1a — the memory model and address-taken locals inside one function (records 0032–0036) (2026-10-28)

The next session reads this file after `CLAUDE.md`, its own section in `plan.md`, `internal/status.md` and
`internal/decisions/`. `status.md` has the formal entry. This file adds what a cold start would otherwise have to
rediscover. It was trimmed at the end of M4.4c (owner, 2026-10-26) to what is live: the full text up to M4.4c, with the
finished sessions' plans, results and answered questions, is `git show 1fcc43b:internal/handoff.md` (M4.4c's and S4's
own sections: `git show fce46de:internal/handoff.md`; M4.4d's: `git show 9017105:internal/handoff.md`; S5's §1–§2 and
§7: `git show 794e960:internal/handoff.md`); earlier handoffs
are `git show 60d3fbe:internal/handoff.md` (M4.1) and `git show a4da52c:internal/handoff.md` (M3 close-out).

## 1. State

M4.5b1a is done on branch `m4.5b1a-pointers` (based on local `main` `794e960`, which merges S5; result in
`internal/status.md`):
- **Record 0032** (the slice of 0028 that needs no contract) and **records 0033–0036** (the four axioms). Pointers to
  integers, `bool` and enumerations in locals and expressions; address-taken locals as one-slot blocks allocated at
  block entry; `&`, `*`, statement-level writes, NULL, `==`/`!=`, truth tests, `?:`; `null-dereference`, `permission`,
  `lifetime`; scope exits at fall-through and `return`; C's incompatible pointer types as errors. Everything that needs a
  contract, a loop over memory or a jump out of a cell's block is rejected (record 0032 item 10) and pinned.
- **Audits:** round 1 on the records (one blocking item, `restrict` on the pointer itself; eight should-fix items, all
  taken), round 2 on the code (one blocking item, a discarded pointer value without its liveness check; three should-fix
  items, all taken), and one re-audit of the round-2 fixes. What was not taken is in §3.
- **S5's PR #14** was open at the start, its Linux job green and its macOS job red only on a 0027 mask timeout (§3).

The EDG pin is unchanged (`f0ed5525af`).

## 2. Owner steps after M4.5b1a, in this order

1. **S5** (PR #14), if not merged yet: merge it (its macOS failure is the known mask flake, §3).
2. **This branch.** Push it and open its PR: `git push origin m4.5b1a-pointers`. Both CI jobs run the PR tier; the mask
   flake may show again on macOS.
3. **The main checkout's `internal/handoff.md`** carries your answers to questions 59–63, which this branch committed
   (`c1f0448`): drop the local copy (`git checkout internal/handoff.md`) before pulling.
4. **Questions.** Answer §8 (question 64). Then M4.5b1b (§4.4).

## 3. Open audit notes (condensed; the full notes are in the commit named above)

**From M4.5b1a** (audit round 1 on records 0032–0036, round 2 on the code, a re-audit of round 2's fixes):
- *Not pinned:* a bare `p;` with `p` dangling reports `lifetime` (checked by hand), but no test holds it: EDG also warns
  #174 (`expr_has_no_effect`), and #550 in C++, which a test with an `error` line may not list
  (`VERIFY_WARNING_TAGS` is closed). The `(void)p;` twins pin the rule.
- *The scope-exit check cannot fail in b1a* (record 0032 item 8; round 2 agreed after trying fall-through, nested
  returns, switch bodies, `goto` inside the block, `abort`, `break` out of loops that hold no pointer): its must-fail
  twin, a callee that consumes the cell, is b1b's first test. M4.8's mutation run will count a mutant of that assert as
  a survivor until then; record it as explained.
- *Evidence not committed by M4.5a:* P52, P54 and P55 exist only as `build/m4.5a/P5{2,4,5}.md` in the worktree; record
  0029 cites them as in `il-inventory.md`. b1b needs P52 (the `owning` attribute's IL) and should add it; P54 goes with
  M4.5b2, P55 with M4.5d1. P51 was added in b1a, with a correction: the address of a static-storage object folds to an
  address constant in every position, not only in initializers.
- *Pointer `?:` as an `if`:* Silicon refuses a Viper `ite` of `Ref`s as a field receiver (insufficient permission even
  after `perm == write` asserts), so lowering assigns a temporary in an `if` (`internal/encoding.md` §3). Keep it when
  b1b passes pointers to calls.
- *For b1c:* a compound store now lowers its address once (round 2); b1c's writes inside expressions rely on it.
  Round 2 also pinned a cell that stays live across a loop that does not touch it (`cell_across_loop.c`): Viper keeps
  the cell's value across the loop because the loop's invariant does not mention it; b1c's automatic invariants must
  keep that frame.
- *Over-strict, fail closed (re-audit):* `*(g() ? &x : &y) += 1` is refused as "a call that is reachable twice" (the
  compound store's address holds a call) and `*(q = p) += 1` as unsequenced; both are defined C. Revisit with b1c's
  writes inside expressions; the wording is misleading for the first.
- *Messages:* the new rejections reuse `unsupported_type`, `unsupported_expression` and `unsupported_statement`, whose
  fixed texts cite "(decision 0012)" (the carried note on record citations below applies); the type message still says
  "only integer types, bool and the enumerations … are" although pointers to them are now accepted in locals.

**CI flake on `main` (M4.5b1 planning, 2026-10-28; owner: ignore for now).** Three of 0027's `^` round-trip
must-fail tests have each run to the 300 s timeout once in C23 on GitHub's macOS runner, one per run:
`mask_or_xor_round_trip_chain_three_post.c` (M4.5a's merge push, run `40686322936`), `..._chain_high_part_post.c` (the
nightly `40700798811`) and `..._chain_value_off_by_one_post.c` (S5's PR, `40739057963`; its Linux job was green). They
pass locally. Record 0027 item 4 measured false claims refuted in 2–5 s and noted that C's `((res == x) ? 1 : 0) != 0`
alone moves a case between 3 s and a timeout; the runner's 3 cores make it worse. A future session may reproduce it
under load (`-j 3`, parallel verifiers) before deciding.

**From S5** (one Opus round on the encoder diff: one blocking item, fixed and re-audited; record 0031 items 5–7):
- *Fixed:* glibc's C23 matcher first accepted any folded constant as the comma's left operand, so a hand-written
  `((void)(INT_MAX + 1), x >= 0) ? (void)0 : __assert_fail(…)` would have verified with its overflow unencoded; the
  constant's original expression must now be the `sizeof`.
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
- *Round 4's reopened item (P1) was fixed in record 0028 item 1 but not re-audited* (the cap): C's implicit conversions
  between incompatible pointer types (EDG warnings #42, #144, #167 and the assignment variant) must become errors in C's
  verify mode, and every pointer conversion node, compiler-generated or not, falls under item 1's `void*` rules; M4.5b1
  lands it with must-fail tests (`ps == px`, `int *px = ps;`, an incompatible argument, `int *q = v;`) and its audit
  re-checks it.
- *Registered attributes are skipped by `check_attributes`* (`encode.cpp:418`): register each target in the session that
  implements it (0029 item 1), never earlier.
- *P51 facts for M4.5b1:* the IL's `address_taken` flag on variables; no lvalue-to-rvalue node (`*p` is `indirect`, an
  lvalue only on the left of `=`); null is a folded constant in every spelling, `int *b = &G;` a `ck_address` constant
  with the tree in its `expr`; C and C++ differ in comparison result types, conditions (`bool_cast` in C++), assignment
  as an lvalue (C++) and implicit `void*` conversions (C only). Not probed: `register`, VLAs, compound literals,
  `restrict`, `sizeof(arr)` and `address_taken`.
- *P55 for M4.5d1:* the attribute allow-list rejects a system declaration's attributes (`[[const]]`, `[[alloc_size]]`,
  `[[__cold__]]`) once a user or vlibc file redeclares the function; check by the attribute's own position.
- *EDG, not chased* (memory: only bugs that block cerastes): in C23, `typeof(*cp)` for a `const int* cp` drops `const`
  (clang keeps it; P53, five shapes). Harmless for `cerastes_old`.
- *Costs to watch:* liveness checks at every pointer use cost about 1.5–1.7× time and text on S26's scale program; deep
  `^` mask chains (0027): range claims are fast to depth 32, identities only to the measured depth (record 0027 item 4).
- *Completeness gaps accepted by the records:* `&s.x < &s.y` reported (members are different blocks); signed/unsigned
  and enumeration variants through `void*` cannot be written; a list traversal's truth tests need liveness inside
  M4.5f2's predicates; `forperm` sees only top-level tokens.
- *The fixtures in `tests/viper-smoke/memory/`* are hand-written Viper of the chosen encoding (E2, four axioms, member
  blocks, liveness); M4.5b1 compares the encoder's output with them and changes them only with a record.

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
- *For M4.5b* (0026 item 15): step 1 places a write of a comma's value in C's order; step 2 keeps 0025 item 2's
  placement, whose caveat (callees do not see locals) applies to it in particular.

**From M4.4c** (audit rounds 1 and 2; items not taken):
- *Shapes accepted and listed* (record 0025 item 6, "still accepted"): `RED == RED` (the front end folds it to 1 without
  an expression); clang's `-Wparentheses-equality`, `-Wswitch-bool`, `-Wfor-loop-analysis`, `-Wmisleading-indentation`
  (clang and GCC), GCC's `-Wmultistatement-macros`, clang's `-Wempty-body` on loops; R10 with a named constant
  (`uc == DEF`, `const int DEF = 300;`, clang's C++ mode warns) or a constant expression (`uc == 256 + 44`); R7 does
  not look into a `?:` arm that is a comma ending in a constant. All are for the warning sweep, now in **M8** (§4.2).
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

**From S4** (closed in M4.4d): the `bit_mask.c [c23]` hang (one Z3 query on `c_uand_64` whose answer depended on the
prover's history) is gone with record 0026 part B; reproducible solver runs (`--numberOfParallelVerifiers 1`,
`--assertTimeout`, §8 question 48) are deferred. The investigation is in `git show fce46de:internal/handoff.md`.

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
that needs the x86-64 box is its own session. Record numbers are the next free ones when written (0027 is next).

### 4.1 M4.4d (record 0026) and M4.5a (records 0027–0030): done

Both done on 2026-10-27 (`internal/status.md`; plans `~/.claude/plans/using-plan-md-and-my-fuzzy-sky.md` and
`~/.claude/plans/using-plan-md-and-my-streamed-glacier.md`).

### 4.2 The warning sweep: deferred to M8 (owner, 2026-10-27, M4.5a planning)

"Since warnings will impact much more than just what we have now we can defer M4.4e until M8." The sweep is an M8 known
entry in `plan.md` (every default and `-Wall` warning of clang 21 and GCC 16 that can fire on the language cerastes
accepts, one rule per family or listed; 0025 item 6's list and §3's M4.4c notes first). Its record takes the next free
number when it runs; 0027 went to M4.5a. The plan it had is `git show 9017105:internal/handoff.md` §4.2.

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

### 4.4 S5 Linux CI, then M4.5 Pointers (sessions b1–f3; designed in M4.5a, records 0027–0030)

**What M4.5a decided** (details in the records; evidence in `il-inventory.md` "M4.5a"; the encoding pinned by the 33
fixtures in `tests/viper-smoke/memory/`):
- **0028, the memory model: E2** (owner, after S24). A pointer is a Viper `Ref`, NULL `null`; a `Block` domain with
  `loc`, `len`, `block_of`, `offset_of`, `member`, `lvloc` and **four axioms** (`loc_inv`, `loc_nonnull`, `member_inv`,
  `member_lv`), each to get its own record and lint tag in M4.5b1; one field per C object type plus 0019's flag per
  cell (`v_int`/`d_int`); `WF(p)` (with bounds) on pointer parameters, results and cells instead of a surjectivity
  axiom; quantified permissions over the absolute slot index, one quantifier per member over arrays of structs;
  globals as `unique` 0-ary domain functions with a trusted `program_start`, and calls to `main` rejected; **each struct
  member and inner array a block of its own** (owner, after audit round 2: bounds per sub-object); the heap token at
  slot −1; **a liveness cell per top-level block** at slot −2, checked before every arithmetic, comparison and truth
  test of a pointer, carried by `owns`/`reads`/`live(p)` (owner, after audit round 2: dangling pointers are reported);
  leaks by `forperm`; `reads` by ghost fractions (`rd := min(perm…)/(n+1)`); explicit checks before every operation and
  at every return, `a != b` asserted at entry per pair of owned places, and `insufficient.permission` accepted only on
  rows marked as range checks (owner, after S25); sequencing on cells by name or pointer, and calls against the cells
  they own (0028 items 12–13).
- **0029, the annotations:** `[[cerastes::owning]]` (an owning pointer; owner, renamed from `owns` the same day), P4222's
  markers as `[[cerastes::uninit]]`, `ref_to_uninit`, `must_init` with `--strict-init`; C's spec functions
  `cerastes_owns(*p)`, `cerastes_reads`, `cerastes_owns_range`, `cerastes_live(p)`, `cerastes_old(e)` (a `typeof` macro) and
  `cerastes_forall(f, free...)` with a named predicate or pure function; no EDG fork commit (P52, P53, P54).
- **0030, vlibc:** `cerastes-verifier/vlibc` (the owner creates it), contract headers and verified implementations;
  explicit `<vlibc/…>` and a transparent `<stdlib.h>` shim in verify mode; trust by path.
- **The VeriFast port inventory** is `build/m4.5a/ports.md` in the M4.5a worktree (gitignored; its conclusions are below
  per session). `malloc_block.c` holds no `malloc` (an address-taken struct local); `insertion_sort.c` is a linked list
  whose ghost machinery is not portable as written (an array insertion sort in M4.5e2 replaces it as a benchmark); 14 of
  the 22 files run VeriFast with `-disable_overflow_check`, so their ports need `requires` bounds; `functions.c`'s
  predicate families and constructors, consumed fractions, backward `goto` and `open_module` are M8 entries.

**S5 Linux CI: done (2026-10-28, record 0031; the plan as M4.5a wrote it follows, for the record).** (owner,
2026-10-27, during M4.5a: "the first sub phase should be adding Linux support to the CI. If MSVC
is easy add it, if not defer"; `ubuntu-24.04` x86-64 only; MSVC and arm64 Linux are M8 entries). ≤ 4 h; its own record
(the next free number). Work, from 0016 item 13 and M4.5a's survey:
- `bootstrap.sh` for Linux: the JDK 17 and Z3 4.16.0 x86-64 builds pinned by checksum in 0001; EDG built out of tree as
  on the box; the system include directories scraped from the Linux compilers (`tool_paths.h`, 0011 item 3); clang 21
  and GCC 16 for `native-ok` and for the scrape (whether `ubuntu-24.04` can install GCC 16 is the first thing to
  check: stop and ask if not); libstdc++ or libc++ for the C++ tests (EDG in Clang emulation over libstdc++ is the
  risk; libc++ matches macOS).
- `src/driver/silicon.cpp`: the clang-only `__builtin_available(macOS 26.0, *)` and `posix_spawn_file_actions_addchdir`
  get a glibc path (`posix_spawn_file_actions_addchdir_np`, glibc ≥ 2.29).
- The encoder: glibc's `assert` expansion (`__assert_fail`, in C and in C++), amending 0015 item 6 (32 test files
  include `<assert.h>`).
- `check_edg_config.sh`'s Linux `GNU_VECTOR_TYPES_ALLOWED`; `check_edg_options.sh`.
- The suite: of 2034 test and doc-example files that include a header, 77 include `<stdbool.h>`, 32 `<stdlib.h>`
  (`abort`/`exit`: glibc declares them with `__THROW`/`__attribute__`s), 32 `<assert.h>`, 8 `<limits.h>`, and about 30
  C++ files include `<concepts>`, `<type_traits>`, `<vector>`, `<array>`, `<variant>` and others. Every test must give
  the same outcome on both platforms; **any test whose outcome differs stops the session** (no per-platform expectation
  without an owner decision). Walker goldens that list system-header facts may need a platform-neutral form.
- `.github/workflows/ci.yml`: a second job in both tiers (0016 item 14), and VeriFast's Linux binary in M5c's Docker
  step can then run on this runner.
- P55's glibc half (how a vlibc shim meets glibc's `<stdlib.h>`) is measured here, ahead of M4.5d1.

**M4.5b1 is three sessions** (owner, 2026-10-28, M4.5b1 planning: a design pass put the session above at well over
27 agent-hours for `src/vir` and `src/viper` alone; plan `~/.claude/plans/using-plan-md-and-handoff-md-temporal-map.md`).
Each rejects what a later one makes precise (fail closed). **Pre-approved** (owner): b1b and b1c may retire or re-expect
exactly the b1a rejection tests listed for them in b1a's status entry, and no others; an existing test whose only point
was "pointers are rejected" is retired where its construct is accepted (listed).

**M4.5b1a The model and address-taken locals inside one function: done (2026-10-28; records 0032–0036, the status
entry). The plan as written follows.** The `Block`
domain with the four axioms and the lint tags (`// cerastes:domain-ok`, `// cerastes:axiom-ok`; `new` refused);
address-taken locals (the IL's flag, P51) as one-slot blocks from a trusted `alloc_T`, allocated at block entry (C23
6.2.4p6, amending 0028 item 4) and before a switch's dispatch; the scope exit at fall-through and `return` (`lifetime`);
`&x`, `*p` reads, statement-level writes, NULL in every spelling, `==`/`!=` (the one-past-the-end exception as an
internal row), truth tests, pointer locals, `?:`; explicit checks (`null-dereference`, `permission`, `lifetime`, the
flag); C's incompatible pointer types (#42, #144, #513, #167) as errors in verify mode. Rejected until later: pointer
parameters and results, `&param`, `T**`, `void*`, struct pointers, pointer casts, loops that hold a pointer node or a
cell, `return` in a loop while a cell is live, `goto`/`break` out of a cell's scope, writes to cells inside expressions.
The Memory and ownership page starts; Error messages gains three sections. P51 lands in `il-inventory.md`.

**M4.5b1b Pointer parameters and contracts.** Pre-approved flips (owner): `tests/scalar/negative/reject_pointer.c`,
`tests/memory/negative/reject_{address_of_param,implicit_function_pointer_argument,pointer_param,pointer_result}.c`.
Its first test is the consuming callee that makes b1a's scope-exit check fail (§3). `WF(p)` on pointer parameters and results; `owns`/`reads`/`live`/`old(*p)`
in both spellings (`<cerastes.h>`'s `cerastes_` forms and `cerastes::live`, 0029 item 3), with the ghost amounts, the
caller's choice of fractions and the case splits (0028 item 8); the framing check of contracts; the call's and the
return's permission clauses one by one; the entry separation; `&param`; `pointer-comparison` (the internal row of b1a
becomes the category); the consuming callee whose caller fails its scope-exit `lifetime` check (b1a's missing twin).
Range facts of a cell's value belong in `owns`/`reads` (design pass: without them Silicon's own range clauses fail
first). `cerastes_old(x)` of a by-value parameter stays a Viper local (pin it, 0029 item 3). The Annotations page's
spec-function table is rewritten.

**M4.5b1c Ownership, sequencing, loops, ports.** Pre-approved flips (owner): `tests/memory/negative/reject_{address_of_pointer,break_out_of_switch_with_cell,cell_in_invariant,cell_in_measure,cell_postfix_value,goto_out_of_cell_scope,loop_cell,loop_pointer,pointer_to_pointer,return_in_loop_with_cell,write_in_assignment_value}.c`. `[[cerastes::owning]]` on parameters and functions (registered then,
targets `p` and `r`: `check_attributes` skips registered attributes, so never earlier) and `owns_block`; sequencing on
objects (0028 item 12, `unsequenced`) and the placement rule (item 13); cells in loops with **hybrid permission
invariants** (owner, 2026-10-28: automatic for places reached through an address-taken local or a pointer parameter the
function never reassigns, `owns`/`reads` in `CERASTES_INVARIANT` for everything else; a missing one is reported at the
access); `T**`; `goto`/`break` out of a cell's scope; the aliasing trap. Ports: `address_of_local.c`'s `inc` (no
tutorial `swap`/`inc` file exists), `swap.c` L4-11, `address_of_local.c`'s scalar functions (its `should_fail` at L158,
L174, L185: a cell a callee consumed, reported at the closing brace). The Memory and ownership page completed. If it
will not fit 4 h, the ports move to M4.8.

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
leaks (`leak`), owning locals and globals (`[[cerastes::owning]]` target `v`). Ports: `account.c`, `deposit.c`,
`swap.c`, `globals.c`, `malloc_block.c`'s `free` twin.

**M4.5e1 Arrays.** Local arrays as blocks, `a[i]`, array parameters, `owns_range` as quantified permissions over the
absolute slot, the marked range rows in the driver (owner, after S25), `forall` in C (0029 item 4: its planning decides
between pure functions with contracts and moving M4.5f1 first), sum and max. P38: a braced initializer with more than
1000 elements keeps no `expr` for its elements (`REDUCE_BACKING_EXPRESSION_USE`), only an `orig_type`; read every
element through `constant_value`, which rejects that shape, never through `ck_aggregate`'s values directly, or a
wrapping element would pass unchecked. M4.4b2's types computed from a constant (`sizeof`, `typeof`, typedefs). Port:
`carrays.c` (lists → `forall`; its `should_fail` at L35 and L42: below and one past the end).

**M4.5e2 Pointer arithmetic.** `p + i`, `p - i`, `p - q`, relational comparisons within one object, one-past-the-end,
the `==` exception of 0028 item 3, `malloc(n * sizeof(T))`; `out-of-bounds`, `pointer-comparison`. Port: an array
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

### 4.5 M4.6–M4.8

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
(§8 question 49), the Memory and ownership page complete, M4's acceptance (plan.md), and the deferred list for M8
(unions, floating point, `volatile`/`_Atomic`/`_BitInt`/vector types, variadic functions and `printf`, several
translation units, concurrency).

### 4.6 M5.0 and M5 — type traits and generic templates (designed in the M4.1 session)

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

## 7. Acceptance of M4.5b1a

The list is the one in `internal/status.md` (M4.5b1a, "Acceptance"); S5's is in `git show 794e960:internal/status.md`.

## 8. Questions for the owner

Each has a recommended answer. Answer inline, as before. Answered questions (1–48) are recorded in the decision records,
in the commit named at the top and in `43f0f81` (questions 5, 10–12 and 43–48); 49–52 are answered below (`b231f32`) and
recorded in records 0027 and handoff §3–§4; 53–58 are answered below (`9ac7ca7`; 57 and 58 used by S5, record 0031);
59–63 are answered below (`c1f0448`, used by M4.5b1 planning). Open: 64.

**M4.4d, 2026-10-27 (answered):**

49. **Mutating the mask positives** (cut today): a mutated mask constant (`255` → `256`) lands on a shape Z3 does not
    decide, and each such mutant runs to the 300 s timeout. In M4.8's run over the M4 tests, run them with mutants of a
    mask constant counted as "undecided (0023 item 2)", reviewed like the safe-direction bucket? *Recommended: yes, in
    M4.8.*

Follow recommendation

50. **A note when the fallback is used:** an error reported in a full expression that took step 2 may be a false alarm
    (0026 item 6). Print a note with it ("checked with the smaller sets: C may not reach this")? *Recommended: not now;
    no test takes step 2 to be accepted; revisit with the ports in M4.8.*

Follow recommendation

51. **Masks passed as arguments** (§3, M4.4d): a mask that is a parameter of the callee still reaches Silicon's own
    precondition check as `c_uand_N` and may time out. *Recommended: leave it documented; revisit if a port needs it
    (M5's integral `T` will meet masks over a parameter type).*

Follow recommendation

52. **`|` and `^` with a constant mask** (`x | 0xFF`, `x ^ 0xFF`) keep their prelude functions, so symbolic claims about
    them stay undecided. Extend the rule (`x | (2^k − 1) = x − x % 2^k + 2^k − 1`, `x ^ (2^k − 1) = x − 2·(x % 2^k) +
    2^k − 1`)? *Recommended: not now; when a ported example (M4.6 chars, M4.8) needs it, with its own spike.*

Do it now since we know it needs handled

**M4.5a planning, 2026-10-27** (owner; plan `~/.claude/plans/using-plan-md-and-my-streamed-glacier.md`):
- **M4.4e is deferred to M8** (§4.2).
- **Q52 is folded into M4.5a** (record 0027, code in `src/vir`), as Q47 was into M4.4d.
- **`[[cerastes::owning]]` is an owning pointer** (spelled `owning`, owner's correction the same day: "instead of a
  pointer as owns make it owning e.g. `[[owning]] int* x;`"; `owns` stays the contract function) wherever it appears: on a parameter the callee borrows the pointee
  (`p == NULL || owns(*p)` on entry and exit); on a function, the caller receives its result's pointee; on a local or
  global, the holder frees it or hands it on before its lifetime ends (the leak check); on a struct field, owning the
  struct owns `*s->f`. Phased over M4.5b–f (record 0029). "Much like how P4222 allows `[[uninit]] int x;`", in
  addition to `owns(x)` in contracts.
- **vlibc** (`cerastes-verifier/vlibc`, created by the owner): contract headers and verified implementations; a user's
  `#include <vlibc/stdlib.h>` is used, and a bare `<stdlib.h>` is transparently replaced in verify mode ("combine all 3
  of those options"). Our ports of `malloc`/`free` live there, not in this repository as VeriFast's do (record 0030).

vlibc has been created and is available in ~/Documents/cerastes/vlibc. When we need to do math functions I have also created a vlibm much the same way.

- **P4222's markers** are spelled `[[cerastes::uninit]]`, `[[cerastes::ref_to_uninit]]`, `[[cerastes::must_init]]`,
  checked when written, with an opt-in strict mode (record 0029).
- **Linux CI first** (owner, during M4.5a: "the first sub phase should be adding Linux support to the CI. If MSVC is
  easy add it, if not defer"): a support session **S5 Linux CI** right after M4.5a and before M4.5b1, on `ubuntu-24.04`
  x86-64 only (arm64 an M8 entry); 0016 item 13 lists the work. **MSVC is deferred to M8:** not easy (a second EDG
  configuration besides the Clang emulation of 0002, a Windows bootstrap without bash, `posix_spawn` in the driver,
  Windows builds of the JDK 17/Z3 pins, an MSVC `native-ok` matrix). M4.5a writes S5's plan (§4).

**M4.5a, 2026-10-27:**

53. **`void*` until an address model** (record 0028 item 1, audit rounds 3–4): an object and its first member share an
    address in C but not in the encoding, so a `void*` may only be tested against null or passed on as an argument, a
    `T*` becomes `void*` only as a call argument, and a `void*` becomes `T*` only from an allocation's result. Generic
    `void*` code (callbacks with a context pointer, `qsort`-style interfaces) is rejected until M4.7/M8. *Recommended:
    accept; revisit with M4.7's function pointers.*

Follow recommendations.

54. **Leaks at `main`'s exit** (record 0028 item 6): VeriFast reports blocks `main` still holds at exit, with a `leak`
    command to waive one. *Recommended: report them too, with an explicit waiver annotation designed in M4.5d2.*

Follow recommendations.

55. **The cost of liveness** (record 0028 item 3a): checking every use of a pointer value costs about 1.5–1.7× time and
    text on S26's scale program. *Recommended: keep it (dangling pointers are reported); revisit only if a port's time
    is unacceptable.*

Follow recommendations. A port time of 10x is when to revisit.

56. **`static` locals** (record 0028 item 5): their permission must reach the function. *Recommended: an implicit
    `owns` of the function's `static` locals, propagated to its callers' contracts, and a rejection inside a recursive
    cycle; M4.5b2 decides the details.*

Follow recommendations.

57. **S5's toolchain**: GCC 16 and clang 21 may not be packaged for `ubuntu-24.04`. *Recommended: the toolchain PPA or
    apt.llvm.org first, then a pinned container image; stop and ask if neither works.*

ubuntu-26.04 is available with github actions. If this doesn't work, then use the newest toolchains availble in that container

58. **A rehearsal machine for S5**: the box runs EDG suites in `~/edg-cerastes` only (memory). *Recommended: allow a
    separate `~/cerastes-ci` checkout on the box to rehearse the Linux job; otherwise S5 iterates on GitHub's runner
    only (slower).*

The box is allowed to be used for any testing you need.

**S5, 2026-10-28:**

Answered during the session: **the eight macOS-only tests** (12 runs that fail on Linux because they name what only
the macOS SDK declares): "Platform tag" — `; platform: macos` on their `mode:`/`c-only:` line (record 0031 item 6,
`tests/README.md`).

59. **glibc twins for the macOS-only tests.** The glibc `assert` matcher's near misses (a `(void)1` success arm, a comma
    operand that is not the `sizeof`, `__assert_fail` in the success arm, another noreturn function) and P55's
    `quick_exit` redeclaration can only run against glibc. *Recommended: add them as `platform: linux` tests in M4.5d1,
    with P55's fix (vlibc's shim meets glibc's `<stdlib.h>` there anyway), and tighten the `sizeof` operand to
    `__assert_single_arg` then (§3).*

Follow recommendation. Tests should be portable to the greatest extent possible.

60. **GCC 16 on Linux is a snapshot** (`16.0.1 20260322`, Ubuntu 26.04's newest; 16.2.0 only from 26.10). *Recommended:
    keep it while the native-ok runs agree with the Mac's GCC 16.2.0 (they did in the rehearsal: 1078 runs each); move
    to 16.2.0 when 26.04 gets it or the image moves, as a pin change of 0001, 0031 and `ci-linux.sh` together.*

Keep it for now. In the future we need to support a few older compilers anyway.

61. **Required checks.** *Recommended: make `check (Linux, Ubuntu 26.04 x86-64)` a required check on `main` after its
    first two green runs (one full, by dispatch or nightly; one PR tier), like `check (macOS, …)`.*

I have made the check required.

62. **The box's Linux checkout.** The box did not answer for most of the session, so the rehearsal used Docker on the
    Mac (the PR tier took 43 min there under Rosetta); when it came back, the full check ran on it natively in 79 min
    at `-j 8`. Its checkout `~/cerastes-ci` (5.8 GB) and image `cerastes-s5:runner` (3.2 GB) are still there, on a
    disk 92% full (35 GB free). *Recommended: keep both until GitHub's Linux job is green twice (they reproduce a
    runner difference in minutes), then the next session that uses the box removes them.*

Instead of the box you may use ssh matt@clang4.cpp.al from here on out. The key is `~/.ssh/id_rsa`.

63. **The Linux job's time limit.** The job has the macOS job's `timeout-minutes: 220` (0016 item 9: max(150, 1.5 ×
    the longest full check seen), and the guard allows at most 220). The box ran the full check in 79 min on 16
    threads, but GitHub's runner has 4 vCPUs, and the macOS job already takes 145 min on 3 cores, so the Linux full
    check may exceed 220. *Recommended: run the first dispatch as is; if the Linux job takes more than 147 min (220 /
    1.5) or times out, a short session applies item 9's rule to that job alone (its own bound in the guard, below
    GitHub's 360-minute job limit) from the run's evidence; the PR tier is not affected (43 min under Rosetta).*

Follow recommendation.

**M4.5b1a, 2026-10-28:**

64. **`restrict` pointers** are rejected (record 0032 item 2; audit round 1: `int *restrict p = &x;` then `x = 2;
    return *p;` is undefined, C23 6.7.3.1, and would verify). `restrict` is common on library parameters
    (`memcpy(void *restrict, …)`), which vlibc's contracts will meet in M4.5d1/M4.6. *Recommended: keep rejecting it in
    user code until a session models it (an M8 inventory entry), and let vlibc's trusted declarations carry it, since a
    trusted contract states its own separation (`owns` of both ranges implies they do not overlap).*

