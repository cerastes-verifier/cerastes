# Handoff after M4.2a — forward goto, functions that do not return, assert (2026-10-21)

The next session reads this file after `CLAUDE.md`, its own section in `plan.md`, `internal/status.md` and
`internal/decisions/`. `status.md` has the formal entry. This file adds what a cold start would otherwise have to
rediscover. The M4.1 handoff is `git show 60d3fbe:internal/handoff.md`; the M3 close-out handoff
`git show a4da52c:internal/handoff.md`.

**Planning update (2026-10-21, after M4.2a).** The owner split the CI session: three sessions run before M4.2b,
**S1** the EDG upstream merge and pin move (box, §4.1), **S2** CI (§4.2) and **S3** the mutation triage (§5), each
planned at ≤ 4 h with at most three Sonnet agents (edg-backend, il-analyst, test-mutator) and one read-only Opus
soundness-auditor. If the orchestrator or any agent has a question, the session stops that work and asks the owner.
§4 and §5 hold the facts found while planning.

**S1 is done (2026-10-21, branch `edg-upstream-merge`; result in §4.1).** The fork's `cerastes` merges upstream
`main` `21065e551e` (`97c2141be9`, not pushed); cerastes pins it; box suites, tests, `.vpr` identity and `check.sh`
green; 0020's Linux failure explained (stale box binaries). **Owner steps after S1, in this order:**
1. Push the fork's `cerastes`: `git -C ~/Documents/compiler push origin cerastes` (GitHub has P `b9519388ea`; the pin
   `97c2141be9` is one merge ahead; `check.sh` reports `NOT YET PUSHED` until then, and S2's CI cannot fetch it).
2. Bring the branch into `main` and rebuild:
   ```
   cd ~/Documents/cerastes/cerastes
   git merge --ff-only edg-upstream-merge
   git submodule update third_party/edg
   scripts/bootstrap.sh --build-only
   ```
3. Push cerastes `main`. Then S2 (§4.2) can start.
*(Done by the owner, 2026-10-21.)*

**S2 is done (2026-10-21, branch `ci`, PR #1; result in §4.2).** `.github/workflows/ci.yml` runs `scripts/ci.sh`
(the owner's Command Line Tools asserted exactly, GCC 16.2.0, full bootstrap) and the full `scripts/check.sh` on
GitHub's `xcode-27` runner (decision 0016). **Owner steps after S2, in this order:**
1. Merge PR #1 into `main` (it carries your `59e1bf3` and the S2 commits; or locally `git merge --ff-only ci` in
   the main checkout and push `main`). The push to `main` starts the first CI run there (its own group; it is never
   cancelled).
2. After the first green run on `main`: make the `CI` job `check (macOS, Command Line Tools 27.0)` a required check
   on `main` (§11 question 6, answered yes).
Then S3 (§5). *(PR #1 merged, `e1f2860`.)*

**S3 is done (2026-10-21, branch `mutation-triage`; result in §5).** The mutation triage: `mutate.py` and
`lint_viper.py` take audit A's items, the owner's "verifier-checked" rule replaces a guard rule in the bucket, 17 weak
tests are strengthened in place with 33 new must-fail twins, and `tests/mutants-equivalent.txt` grows from 125 to 436
reviewed entries. The full run over `tests/scalar` and the four doc examples the list names exits 0. **Owner steps
after S3:**
1. Push the branch and open the PR: `git push origin mutation-triage` (the session never pushes). CI runs
   `check.sh` as locally.
2. Merge it. Then M4.2b (§8.1).

**M4.2b1 is done (2026-10-21, branch `m4.2b1-named-constants`, based on S3's tip `426f23d`; result in
`internal/status.md`).** Named constants (`const`/`constexpr` globals) and case values (C++ `case k`/`case N`) are
checked in a prologue of every method (record 0017); M4.2b was split into M4.2b1 and M4.2b2 (owner). **Owner steps
after M4.2b1:**
1. Merge S3's PR first (`mutation-triage`): this branch contains it.
2. Push this branch and open its PR: `git push origin m4.2b1-named-constants` (the session never pushes). CI runs
   `check.sh` as locally.
3. Answer §11 questions 14 (strict C for `const` objects in constant expressions) and 15 (C23 `constexpr`). Then
   M4.2b2 (§8.1). *(Answered 2026-10-22: keep the strict rule; an M8 entry.)*

**M4.2b2 is done (2026-10-22, branch `m4.2b2-enumerations`, based on M4.2b1's tip `6b5f7d9`; result in
`internal/status.md`).** Enumeration types and enumerators verify in both modes (record 0018): each enumeration is its
own VIR type with its value range, every conversion into it is checked (`conversion`, never a wrap), implicit
conversions out of it are accepted only where C and C++ agree, enumerator initializers are checked in the 0017 prologue,
and a C switch over an enumeration is encoded in the promoted type. **Owner steps after M4.2b2:**
1. Merge M4.2b1's PR (`m4.2b1-named-constants`) first: this branch contains it.
2. Push this branch and open its PR: `git push origin m4.2b2-enumerations` (the session never pushes). CI runs
   `check.sh` as locally.
3. Answer §11 questions 16–19. Then M4.2c (§8.1; the next free record, 0019). *(Answered 2026-10-22, M4.2c planning:
   16 as recommended; 18 and 19 done in M4.2c; 17 done in M4.2c with the fork fix branch `0022`.)*

**M4.2c is done (2026-10-22, branch `m4.2c-uninitialized-locals`, based on M4.2b2's tip `971c17c`; result in
`internal/status.md`).** Locals without an initializer verify in both modes (record 0019): a read that may see an
indeterminate value is the new error category `uninitialized` (P4222R1), checked on each path by a ghost flag; a `goto`
or `case` may jump past such a declaration. With it, §11 questions 17–19: EDG's internal error #2656 is fixed on the fork
branch `0022` (no driver change; the owner merged it into the fork's `cerastes`, `f0ed5525af`, and the pin moved there at
the end of the session), ordering of one unscoped enumeration verifies, and a switch on an enumeration value is held to
`-Wswitch`'s two rules (g++'s reading). **Owner steps after M4.2c, in this order:**
1. Merge M4.2b2's PR (#4, `m4.2b2-enumerations`) first: this branch contains it.
2. Push this branch and open its PR: `git push origin m4.2c-uninitialized-locals` (the session never pushes). CI runs
   `check.sh` as locally.
3. The fork branch `0022` (`3b05dd5c43`, in `~/Documents/compiler`, on upstream `main` `ec2f12cffa`; merged into
   `cerastes` as `f0ed5525af`, which is pushed and pinned) is ready for the upstream issue/PR: file
   `internal/edg_bugs/0022-suppressed-note-internal-error.md`, then replace `[GH #?]` in the commit's Changes entry with
   the number (the Changes check needs it) and push the branch.
4. Plan the next **EDG box session** (§8.1, "EDG box session after M4.2c"): the full box suites at the pin
   `f0ed5525af` (upstream `ec2f12cffa` and `0022`, merged without a box run), and §11 question 16's bug draft.
5. Answer §11 questions 20–23. Then M4.3 (§8.1). *(Answered 2026-10-22, M4.3 planning; M4.3 split into M4.3a/M4.3b.)*

**M4.3a is done (2026-10-22, branch `m4.3a-side-effects`, based on `main` `be44583`, the merged M4.2c; result in
`internal/status.md`).** Assignments, compound assignments and `++`/`--` used as values, chained assignment, the comma
operator (calls ordered by it too), and side effects in guarded operands and loop conditions verify in both modes (record
0020); an unsequenced modification is rejected by C's rules in both modes; the shapes the compilers' `-Wall` warns about
are rejected like the compilers (the union of clang and GCC, P35). With it, the CI fix the owner asked for (a race in
`EdgTreeToolTests`' `copytree`). **Owner steps after M4.3a, in this order:**
1. Your §11 answers 20–23 were uncommitted in the main checkout's `internal/handoff.md`; this branch carries them, so drop
   the main checkout's copy before merging: `git -C ~/Documents/cerastes/cerastes checkout internal/handoff.md`.
2. Push the branch and open its PR: `git push origin m4.3a-side-effects` (the session never pushes). CI runs `check.sh`
   as locally.
3. Answer §11 questions 24–26. Then M4.3b (§8.1; the next free record, 0021).

**M4.3b is done (2026-10-23, branch `m4.3b-calls`, based on local `main` `ba90e75`, the merged M4.3a; result in
`internal/status.md`).** Calls in the right operand of `&&`/`||` and the arms of `?:`, in `while`/`for`/`do` conditions,
and in any arrangement whose checks form a chain verify in both modes (record 0021: the dependency-chain rule, decided
exactly in lowering); an expression whose checks depend on different unsequenced calls is rejected (`unsequenced_calls`);
where another call may precede a recursive call, cerastes asserts that call's termination itself (Silicon's own relation,
S19); clang's `-Wself-assign` shapes are rejected (`self_assignment`, §11 question 24); and the M4.3a detector for a
discarded `&&`/`||` now follows GCC's rule (P37). With it, the CI timeout goes from 240 to 150 minutes (§11 question 8).
**Owner steps after M4.3b, in this order:**
1. Merge M4.3a's PR (#6, `m4.3a-side-effects`) first: this branch contains it (local `main` `ba90e75` is that merge).
2. Your §11 answers 24–26 were uncommitted in the main checkout's `internal/handoff.md`; this branch carries them, so drop
   the main checkout's copy before merging: `git -C ~/Documents/cerastes/cerastes checkout internal/handoff.md`.
3. Push the branch and open its PR: `git push origin m4.3b-calls` (the session never pushes). CI runs `check.sh` under the
   new 150-minute limit.
4. Answer §11 questions 27–31. For the next session, §11 question 13 (unsigned wrapping, in its own short session before
   M4.4) is still the recommendation; the EDG box session after M4.2c (§8.1) is also pending.

**M4.4 planning (2026-10-23, after M4.3b).** The owner answered §11 questions 27–31 and question 13 (no wrapping), and
split M4.4 into four sessions (§8.1): **M4.4a** no wrapping and exit 3 for internal inconsistencies, **M4.4b** bitwise
operators, **M4.4c** writes outside F, `void` `?:` arms and warnings in verified tests, **M4.4d** exact sets. Each is
planned at ≤ 4 h on its own branch, with the pre-approved expectation changes listed in §8.1.

**M4.4a is done (2026-10-23, branch `m4.4a-no-wrapping`, based on local `main` `b395a2b`, the merged M4.3b; result in
`internal/status.md`).**
- **No wrapping** (record 0022; §11 question 13). Unsigned arithmetic, `-u`, `u << n` and every conversion into an
  unsigned type are checked like the signed forms (`overflow`, `shift`, `conversion`), and `c_wrap` is gone.
- **Exit 3** (question 29). Lowering's internal errors, the emitter's problems and the "encoded twice" guard are tool
  failures, reported as `internal_inconsistency`.
- **Enumerations in C** (owner, during the session). C compares an enumeration without negative values as `unsigned
  int`, and both compilers warn `-Wsign-compare`. So `==`/`!=` between such an enumeration and a signed operand that
  may be negative is rejected in both modes, and so is `==`/`!=` between two different enumerations (`-Wenum-compare`;
  audit round 2's blocking item).

**Owner steps after M4.4a, in this order:**
1. **Handoff answers.** Your §11 answers 27–31 were uncommitted in the main checkout's `internal/handoff.md`; this
   branch carries them, together with question 13's answer. Drop the main checkout's copy before merging:
   `git -C ~/Documents/cerastes/cerastes checkout internal/handoff.md`.
2. **M4.3b.** Local `main` has M4.3b merged (`b395a2b`); `origin/main` as last fetched here is `d694606` (M4.3a's PR).
   If GitHub does not have M4.3b yet, push `main`, or open and merge M4.3b's PR (`m4.3b-calls`), first.
3. **This branch.** Push it and open its PR: `git push origin m4.4a-no-wrapping` (the session never pushes). CI runs
   `check.sh` as locally.
4. **Questions.** Answer §11 questions 32–36. Then M4.4b (§8.1, record 0023, probes P40–P41). *(Answered 2026-10-23,
   M4.4b planning.)*

**M4.4b planning (2026-10-23, after M4.4a).** The owner answered §11 questions 32–36 and split M4.4b (§8.1, §11 "M4.4b
planning"): **M4.4b1** bitwise operators with Q33 (multi-character literals) and Q36 (item 5's mode differences), record
0023; **M4.4b2** Q32's values the front end computes (`_Static_assert`, character literals, array types in types; `#if`
in the EDG box session), record 0024. M4.4c and M4.4d take records 0025 and 0026.

**M4.4b1 is done (2026-10-24, branch `m4.4b1-bitwise`, based on local `main` `4cc849f`, the merged M4.4a; result in
`internal/status.md`).**
- **Bitwise operators** (record 0023). `& | ^ ~` and `&= |= ^=` verify in both modes with their exact two's complement
  meaning. `& | ^` are prelude functions per width (32, 64, 128) that carry a range postcondition, which Silicon proves
  (owner, 2026-10-24: 0012 item 11 amended). `~` is inline. The compound forms convert back with a check.
- **Warning shapes.** The compilers' bitwise `-Wall` shapes are rejected: seven rules, two extensions of 0020's rules,
  and GCC's promoted-complement comparison (audit round 1).
- **Q33.** Multi-character and truncated character literals are refused in verify mode.
- **Q36.** A `?:` or comma of truth values counts as a truth value. In C, a `?:` of enumerators and a comma ending in
  an enumerator get C++'s verdict in `==`/`!=`.
- **Limit (owner: keep it, documented).** Relational and symbolic bit identities (`(u | 4) & 4 == 4`, the XOR swap,
  `low_bit`) are not decided: such a run ends at `--timeout` with exit 3.

**Owner steps after M4.4b1, in this order:**
1. **Handoff answers.** Your §11 answers 32–36 were uncommitted in the main checkout's `internal/handoff.md`; this
   branch carries them. Drop the main checkout's copy before merging:
   `git -C ~/Documents/cerastes/cerastes checkout internal/handoff.md`.
2. **M4.4a.** Local `main` `4cc849f` merges M4.4a. If GitHub does not have it yet, push `main` (or open and merge
   M4.4a's PR, `m4.4a-no-wrapping`) first.
3. **This branch.** Push it and open its PR: `git push origin m4.4b1-bitwise` (the session never pushes). CI runs
   `check.sh` as locally (about 33 min here; the bitwise tests add a few minutes).
4. **Questions.** Answer §11 questions 37–39. Then M4.4b2 (§8.1, record 0024). *(Answered 2026-10-26, M4.4b2
   planning.)*

**M4.4b2 planning (2026-10-26, after M4.4b1).** The owner answered §11 questions 37–39 (37 and 38 as recommended; 39:
skip a static assertion cerastes cannot encode, and add a type-traits session before M5, **M5.0**) and chose to reject
array types inside types until M4.5e (§11 "M4.4b2 planning"). The approved plan is
`~/.claude/plans/using-plan-md-and-handoff-md-soft-sloth.md`; §8.1 has the essentials.

**M4.4b2 is done (2026-10-26, branch `m4.4b2-front-end-values`, based on local `main` `10e7905`; result in
`internal/status.md`).**
- **Static assertions** (record 0024) are encoded as written and checked in 0017's prologue at every scope; file scope
  through EDG's source sequence lists, now on (the walker keeps its listing; no golden changed). A condition with a call
  of a system function, a type trait, a concept-id or a system name is skipped (§11 question 39), but still held to the
  type rule.
- **Wrapped character literals** (`'\xff'`) are `conversion` errors at the literal.
- **Types computed from a constant** (arrays, `_BitInt`, vectors, class instances with a non-type argument) are rejected
  inside `sizeof`/`_Alignof`/`typeof`/`decltype`, in array typedefs, and as declared types reached through an alias
  with a non-type argument, until M4.5e.

**Owner steps after M4.4b2, in this order:**
1. **Handoff answers.** Your §11 answers 37–39 were uncommitted in the main checkout's `internal/handoff.md`; this
   branch carries them. Drop the main checkout's copy before merging:
   `git -C ~/Documents/cerastes/cerastes checkout internal/handoff.md`.
2. **This branch.** Push it and open its PR: `git push origin m4.4b2-front-end-values` (the session never pushes). CI
   runs `check.sh` as locally; the EDG configuration changed (source sequence lists), so CI rebuilds EDG's front end.
3. **Questions.** Answer §11 questions 40–42. Then M4.4c (§8.1, record 0025). *(Answered 2026-10-26, M4.4c planning.)*

**M4.4c planning (2026-10-26, after M4.4b2).** The owner answered §11 questions 40–42 (as recommended) and chose one
session for M4.4c with Q37 last and a cut order; the planning review found a placement hole for `uninitialized` checks
(finding A), fixed first (§11 "M4.4c planning"). The approved plan is
`~/.claude/plans/using-plan-md-and-handoff-md-wild-rainbow.md`.

**M4.4c is done (2026-10-26, branch `m4.4c-writes-void-warnings`, based on local `main` `8bff6ba`; result in
`internal/status.md`).** Record 0025: finding A fixed (an `uninitialized` check in a guarded operand is an access of its
local); writes and snapshots leave F (Q28); a `void` `?:` in statement position is an `if` (Q30); `verified` and
`error` tests may list front-end warnings (Q31); the static-assertion cross-check covers function bodies (Q40); 13
`-Wall` shape rules R1–R13 (Q37, and five more families the owner added during the session). A new session **M4.4e**
sweeps every default and `-Wall` warning of clang and GCC (owner, during the session; record 0027).

**Owner steps after M4.4c, in this order:**
1. **Handoff answers.** Your §11 answers 40–42 were uncommitted in the main checkout's `internal/handoff.md`; this
   branch carries them. Drop the main checkout's copy before merging:
   `git -C ~/Documents/cerastes/cerastes checkout internal/handoff.md`.
2. **This branch.** Push it and open its PR: `git push origin m4.4c-writes-void-warnings` (the session never pushes).
3. **Questions.** Answer §11 questions 43–46. Then M4.4d (§8.1, record 0026), then M4.4e (record 0027).

## 1. State in one paragraph

The owner split M4.2 into **M4.2a** (forward `goto`, functions that do not return, `assert`), **M4.2b** (named
constants and enumerations) and **M4.2c** (locals without an initializer) (2026-10-21; `plan.md`, §8.1). **M4.2a is
done** on branch `m4.2a-goto-noreturn-assert` (based on `main` `518e0de`): user gotos that jump outward and forward,
`[[noreturn]]`/`_Noreturn` functions (new category `noreturn`), the trusted C library functions `abort`/`exit`/
`_Exit`, and `assert` from `<assert.h>` checked as `check`, with decision 0015, about 90 new tests, the VIR self-test at
230 cases and the docs. The branch's first commit moves the pin back to P `b9519388ea`: `main`'s `518e0de` had pinned
the pushed `0bcd9bf28a`, which lacks `RAF_NO_CONSTANT_EVALUATION`, so `main` did not build. The CI and mutation triage
sessions (§4, §5) have still not run; M4.2a went first at the owner's request. *(2026-10-21: the owner pushed the
fork's `cerastes` (P) and cerastes `main` (`243cf57`, the M4.2a branch tip); §2 is done.)*

## 2. Owner steps, in this order (all done, 2026-10-21)

1. **Push the fork's `cerastes`** (`git -C ~/Documents/compiler push origin cerastes`): GitHub has `0bcd9bf28a`; the
   pin P `b9519388ea` is 6 commits ahead. `check.sh` reports `NOT YET PUSHED` until then, and a fresh clone of
   cerastes cannot fetch the pin.
2. **Bring the branch into `main`.** The work is on `m4.2a-goto-noreturn-assert` in the worktree
   `.claude/worktrees/m3-closeout`:
   ```
   cd ~/Documents/cerastes/cerastes
   git merge --ff-only m4.2a-goto-noreturn-assert
   git submodule update third_party/edg      # the main checkout's EDG is at 0bcd9bf; the branch pins P
   scripts/bootstrap.sh --build-only
   ```
   `518e0de` "Update edg version" is what moved the pin back (probably a commit that picked up a stale submodule
   checkout). If you meant to pin `0bcd9bf`, say so: cerastes needs P.
3. **Push cerastes `main`.**
4. Unchanged: branch `0020` (`c539de4101`) kept for the upstream PR, its Linux failure unexplained (§4); the fork's
   `.tmp/agent-*` evidence directories.

## 3. What the session did

| Item | Outcome |
|---|---|
| Planning | M4.2 sized at about three sessions by exploration and a Plan-agent review; the owner split it (M4.2a/b/c) and pre-approved the five retirements, the `noreturn` category, `assert` as `check`, and the amended meaning of "verified" |
| Pin | restored to P on the branch (`48e4b45`) |
| Plan | `plan.md` M4 table and §8.1 here: M4.2a/b/c, with the review's findings for M4.2b and M4.2c |
| Decision 0015 | outward-and-forward `goto` (over VIR blocks after the encoder's flattening, with guards for generated blocks); noreturn (`Function::noreturn`, `ensures false`, `assert false` with `noreturn` at each reachable return and at the end; a user's `[[noreturn]]` is a trusted user contract only when no system header wrote the flag); the trusted libc table (`abort`/`exit`/`_Exit`: name, system file, C linkage, signature, flag); `assert` by the macOS expansion's exact shape (through folded constants); amends 0011–0014 |
| Probes and spikes | P19–P22, S11–S13, all confirmed (`il-inventory.md`, "M4.2a") |
| Code | `src/vir` (Goto/Label, `goto_problems`, noreturn lowering, assert-origin `Check`), `src/backend/encode.cpp` (user gotos and labels, label attributes on the entity, noreturn, the libc table, the assert matcher, `ensures_on_noreturn`), `verify_main.cpp` (#1305 suppressed) |
| Tooling | `lint_viper.py`: every `goto` outward and forward at token level; `mutate.py`: deletes `goto L;`, never a label, buckets a deleted call-free `assert(...)`; `noreturn` in the harness |
| Tests | 90 new files in `tests/scalar` (27 positives, 63 twins and rejections); `reject_goto.c`, `reject_noreturn.c` retired (owner); VIR self-test 204 → 230 lowering cases (75 → 87 rejection cases) |
| Docs | Verifying a program (goto, functions that do not return, `assert`, the meaning of a result, the rejected list), Error messages (`noreturn`), Annotations (`check` vs `assert`), Overview |
| Audits (Opus) | 0015: 1 blocking (a lint closed list for `ensures false` that rejected a user's trusted `ensures(false)`: dropped), 9 should-fix taken; code: 0 blocking, 2 should-fix taken (a noreturn flag written by a system header was trusted as the user's claim; missing tests); tooling re-audit: 0 blocking, 3 should-fix taken (the assert bucket with calls, `mutate.py --validate` on a new test, closed-list leftovers) |

## 4. The EDG session (S1) and the CI session (S2) (owner 2026-10-16, 2026-10-20, split 2026-10-21)

### 4.1 S1 — EDG upstream merge and pin move (box; about 2–2.5 h; branch `edg-upstream-merge`)

Needs the x86-64 box (memory `x86-box-edg-suites`; the box sleeps, ask the owner to wake it). Agents: edg-backend
(the fork merge, the box runs, the pin move), il-analyst (0020 from the box outputs), soundness-auditor (the merge,
the evidence).

1. **Merge upstream EDG `main`** into the fork's `cerastes` (`--no-ff`, never rebase; decision 0009). Upstream is
   `dedc820c91` (2026-10-21), 9 commits after `307002eae6`, all fetched in `~/Documents/compiler` as `edgcpp/main`:
   - `52a125e154` is our 0005 (`__BLOCKS__`), re-committed by the owner: `git range-diff` shows it identical to the
     fork's `7173149c6b`, so it merges as a duplicate;
   - `4545bf930c` (#229, `if constexpr` with non-brace statements, `lexical.c`): cerastes's `if constexpr` tests all
     use braces;
   - `ed0a9efb06` (#309) moves the position of cleanup actions to the closing brace, in `lower_il.c`: cerastes runs
     with no lowering (`src/backend/edg_il.h:35`), so positions should not move (the `.vpr` identity check proves it);
   - `dedc820c91` (#218, new since the M4.2a handoff) suppresses `noreturn_function_does_return` when a noreturn
     constructor or destructor is called (`statements.c`; the fork's hunk there is far away): the four warnings
     `tests/il/m3/p15b_noreturn.cpp` lists involve neither;
   - `5bfce0ac36` (#302, base-class initializer order); `3c62b61f23` moves the box's Docker images to
     `ghcr.io/edgcpp`; CI, docs and issue-template commits.
   Expect one textual conflict, `src/Changes` (keep both sides); any other conflict: stop and ask. Then the full box
   suites (`changes,regressions,imported,edg,cwg,stdlib`, both 64-bit configurations) before the pin moves; known
   baseline mismatches `cwg1835`, `cwg2881`. **Every box run starts from a clean build** (`rm -rf
   ~/edg-cerastes/build/gcc`): see item 2.
2. **Branch `0020` alone fails its own test on Linux** (M3 handoff §4): `GH-lambda-template-param-qualified.sft.cpp`
   cases 1 and 2, `FAIL--OUTPUT MISMATCH`, passes at the chain tip and natively. Explain it before the owner files 0020.
   **Hypothesis (planning, 2026-10-21): a stale incremental build on the box.** `edg-docker-test` reconfigures only
   when `build/gcc` is missing and otherwise rebuilds files newer than their objects; the box tree is rsynced with the
   Mac's file times. Run `0020d` checked out `c539de4101` over a Mac checkout of `0006`, so only `il.c`, `lexical.c`
   and `overload.c` got fresh times, while about 17 other files that differ from the previous box build (the fork
   series: `scope_stk.h`, `symbol_tbl.h`, `il_def.h`, `attribute.c`, …, which change struct layouts) kept stale
   objects. The test and its `.rto` expectations are identical at `0020` and at the tip, and no later commit touches
   the check. Confirm on the box: `~/edg-cerastes-runs/0020d/run.log` should compile only those three files; then a
   clean-build `run.sh 0020e c539de4101 changes` should pass. If it still fails: stop and ask (a real 0020 bug).
3. **Pin move**, locally while the box runs: keep `build/vpr` as the baseline, `git -C third_party/edg fetch
   fork-local` and `fetch origin` (its `origin/cerastes` is stale at `0bcd9bf28a`), check out the merge, `git add
   third_party/edg`, `scripts/bootstrap.sh --build-only`, `run_tests.py -j 8`, `diff -r -x MANIFEST <baseline>
   build/vpr` (byte-identical expected; any golden, `expect:` or `.vpr` difference: stop and ask). Then `check.sh`,
   decision 0001's EDG row, the 0020 bug file, an Opus audit of the merge and the evidence.

**Acceptance:** 0020 PASS on a clean build; the full suites at the merge with only the two known mismatches; every
test's `.vpr` byte-identical; `check.sh` green; no open blocking audit item.

**Result (S1, 2026-10-21, about 35 min of work plus box time).**
- **Merge:** upstream had moved again that morning to `21065e551e` ("Fix filter command escaping and script name",
  tests only: 285 files under `tests/tests/edg/cmeerw`), so the fork merges that tip: `97c2141be9`. Conflicts:
  `src/Changes` (both sides kept, the fork's first) and, unexpectedly, `util/Changes` and
  `util/make_predef_macro_table`: the fork carries the session's original 0005, upstream the owner's re-commit (#307);
  upstream's text taken (it drops the fork's comment). The Opus audit reproduced both as exact unions/upstream text,
  P→merge under `src/` equal to upstream's own delta, the fork series intact, no header changed.
- **Box (`run.sh` now builds from scratch and reports `BUILD FAILED`):** `U-full` at `97c2141be9`, all six suites,
  both configurations: match except the known `cwg1835` (`PASS--OUTPUT MISMATCH`) and `cwg2881`
  (`ABORT--OUTPUT MISMATCH`); changes 154/154, regressions 5/5, imported 9075/9075, edg 238/238, stdlib 3/3 (cp
  overlay).
- **0020:** run `0020d`'s incremental build linked stale fork-series objects against 0020's `il.c`; the link failed
  and `edg-docker-test` ran the previous box binaries (0020's previous revision, which lacks the `template`-keyword
  rule), so exactly cases f5 and f6 failed. A clean build of `c539de4101` matches `changes` (`0020e`) and
  `changes,regressions,imported` (`0020f`, which also covers the imported expectations 0020 changes) in both
  configurations. Written up in `internal/edg_bugs/0020-*.md`; the owner files it.
- **cerastes at the pin:** 2624 passed, 0 failed, 1 skipped (native-ok 642/642), as at M4.2a; 667 `.vpr` and maps
  byte-identical to P's (checked after the test run and after `check.sh`); `check.sh` green (15 min 43 s).
- **Audit (Opus):** 0 blocking. Taken: the 0020 write-up's mechanism (above), a clean Linux run of 0020's imported
  expectations (`0020f`), 0001's pin history. Notes: the fork keeps two tests for the same `__BLOCKS__` fix (its
  original `GH-no-blocks-predefine` and upstream's `GH307`; owner: leave it until a fork commit touches
  `tests/changes`); #229 matters once M5 encodes template instances (add a test with an unbraced discarded
  `if constexpr` branch then); `--dump_configuration`'s build date does not identify the pin.

### 4.2 S2 — CI (about 3 h; branch `ci`; needs S1 pushed)

Owner decisions (2026-10-21): GitHub's standard **`xcode-27`** runner (GA 2026-10-15: macOS 27.0.1, Xcode 27.0
`27A266a`, clang 21.0.0, SDKs 27.0 and 27.2, JDK 17, gcc-13/14/15, no sbt or shellcheck), the **full `check.sh`** on
pull requests, pushes to `main` and `workflow_dispatch`; the repository is public, so the runner is free. **The owner
pushes** the branch at checkpoints; the session watches the runs read-only (`gh run watch`, `gh run view
--log-failed`). Agents: edg-backend (`scripts/ci.sh`, the workflow), test-mutator (harness and self-tests),
il-analyst (drift analysis), soundness-auditor.

1. **Decision 0016-ci** first (the next free number; M4.2b's and M4.2c's records become 0017 and 0018): runner,
   triggers, per-ref concurrency (pull requests cancel, `main` never), `contents: read`; the toolchain match and its
   known difference (Xcode 27.0's clang against the owner's Command Line Tools `clang-2100.3.34.2`; SDK 27.0 selected
   and asserted, not 27.2; `EDG_CLANG_VER` 210000); Homebrew `gcc` with major 16 asserted (the owner's `g++-16`);
   caches for inputs only (sbt and Coursier, Silicon's `target` and stamp keyed on the silicon and silver gitlinks and
   the JDK, Z3 by asset and SHA, npm), never EDG or cerastes build outputs; drift policy: stop and ask, never edit an
   expectation to match CI; the x64 Docker fallback recorded, not built (glibc's `assert` shape `__assert_fail`, the
   macOS-only `posix_spawn_file_actions_addchdir` in `src/driver/silicon.cpp`, the Darwin-only `bootstrap.sh`).
   Amend 0004 ("CI runs only the docs") and 0001 (the runner, the new action SHAs).
2. **What CI must work around** (planning facts): a shallow submodule checkout lacks `refs/remotes/origin/cerastes`,
   so `edg_tree.py check` fails (fetch `+refs/heads/cerastes:refs/remotes/origin/cerastes`, blobless, enough history
   for `--is-ancestor`); `run_tests.py`'s default `-j` is cpu/2 (1 on 3 cores) and the `check.sh` lines for
   `run_tests.py` and `mutate.py --validate` are pinned by self-tests (`test_run_tests.py:3151`, `3165`), so the job
   count comes from an environment variable `CERASTES_JOBS` read by `run_tests.py` and `mutate.py` (with a self-test
   twin); `bootstrap.sh` needs Homebrew's `openjdk@17` and installs sbt and shellcheck; the EDG checkout is 3.7 GB
   (`tests/` 2.6 GB: a sparse checkout is the fallback if the disk is tight); 7 GB of RAM may need `-j 2`.
3. **`scripts/ci.sh`** (prints `uname`, `df -h`, the SDK, clang and gcc versions; sets `DEVELOPER_DIR`; asserts SDK
   27.0 and gcc 16; fetches the fork ref; `bootstrap.sh`; asserts a GCC in `build/env.sh`; `check.sh`), and
   **`.github/workflows/ci.yml`** (pinned `checkout` with `submodules: recursive` and `persist-credentials: false`,
   `setup-node`, `actions/cache`, `upload-artifact` for logs on failure; `timeout-minutes: 150`). Self-test guards in
   the style of docs.yml's (`test_run_tests.py` 3114, 3235–3270), each with a twin: `ci.yml` runs `scripts/ci.sh` with
   no `|| true`, `; true`, `set +e`, `continue-on-error` or `if:`; no `--update-golden`, no `CERASTES_EDG_SRC`;
   `ci.sh` runs `check.sh` unconditionally.
4. **Rehearsal** (= `check.sh` run #1): a fresh clone of the branch under `build/ci-rehearsal` (submodules with
   `--reference`) running `CERASTES_JOBS=3 scripts/ci.sh`. Then the owner's push, the run (60–90 min), at most two
   infrastructure fix rounds; drift on the runner: il-analyst diffs it, the session stops and asks.

**Acceptance:** a green `ci.yml` run on GitHub (URL recorded; the log shows SDK 27.0 and gcc-16; test counts equal to
the local ones, native-ok with both compilers); self-tests and `check.sh` green; no open blocking audit item.

**Result (S2, 2026-10-21, 14:39–16:35; branch `ci`, PR #1).**
- **Planning changed four things** (owner, decision 0016):
  - CI selects the runner's **Command Line Tools**, not Xcode 27.0. The `xcode-27` image (a public preview, not GA)
    ships CLT 27.0.0.0.1788430756, the owner's exact package.
  - The first runs come from a pull request.
  - `check.sh`'s VIR line drops `-j 8`.
  - The owner reinstalled Homebrew `gcc`, whose macOS 26 bottle had compiled the native-ok runs against SDK 26.5.
- **The workflow differs from this section's plan:**
  - `fetch-depth: 0` instead of fetching the fork ref;
  - `ci.sh` and `check.sh` are separate steps;
  - only npm and the Silicon jar are cached (not Z3, sbt or Coursier);
  - `timeout-minutes: 240`;
  - Python 3.14 is set up.
- **Acceptance run** [39664543570](https://github.com/cerastes-verifier/cerastes/actions/runs/39664543570) at
  `c959f59`: **green**, 44 min (checkout 2.5 min, `ci.sh` 1 min 45 s with the jar cached, `check.sh` 41 min, of which
  tests 24 min 41 s at `-j 3` and VIR 6 min 49 s).
  - **Toolchain report:** image `macos27 20261012.0260.1`; CLT 27.0.0.0.1788430756; clang-2100.3.34.2; SDK 27.0
    `26A425`; g++-16 16.2.0, sysroot `MacOSX27.sdk`; JDK 17.0.20.1; Z3 4.16.0; Python 3.14.8; bash 3.2.57; CMake
    4.4.4; Node 26.12.1.
  - **Disk:** 35 GB free of 145 GB.
  - **Same counts as run #2 below** (`compare_ci.py`, every item):
    - self-tests 951 (skipped=2), 74 (expected failures=1), 142 and 180;
    - tests 2624 passed, 0 failed, 1 skipped (native-ok: Apple clang 642, GCC 642);
    - VIR 230/230 (+ 87 rejection cases);
    - headers 954/954;
    - `mutate --validate` 3522 mutants;
    - oracle 134 goldens, 0 failed;
    - docs green.
  - **Emitted Viper:** the uploaded `build/vpr` (1335 files) is byte-identical to the rehearsal's.
- **Canary** 39658634464 at `6bfb1c3`: setup green; cancelled by the next push after the four self-tests (identical)
  and 1762 test PASS lines, with no FAIL line.
- **Local `check.sh`, both in fresh clones with the runner's environment:**
  - run #1 at `6bfb1c3`: 21 min;
  - run #2 at `c80d0d9`: 20 min.

  Both green, with the counts above (the harness self-test had 941 tests at `6bfb1c3`). The final `c959f59` changes
  only self-test code and 0016; the full harness self-test there gave 951 OK (skipped=1) in the worktree.
- **Local baseline with the reinstalled GCC:** 2624/0/1, native-ok 642/642; `.vpr` byte-identical to S1's.
- **Audits (Opus), three rounds, 0 blocking:**
  1. The GCC checks would have passed a macOS 26 bottle, and 0016 claimed guards that did not exist yet. Fixed:
     - GCC 16.2.0 exactly, with its sysroot equal to the asserted SDK;
     - SDK build `26A425`;
     - `/usr/bin` shims for `clang++`, `cc` and `c++`;
     - 21 environment variables that must be unset;
     - the upload with `always()`, logs included.
  2. The guards accepted additions: an extra step or checkout key, a line in the cache-key script, `break` in a loop,
     a `function` override, a reassignment. Fixed: they pin the whole active text.
  3. An unclosed quoted step name, and non-ASCII blanks before `#`. Fixed: names are normalised only as plain
     scalars, and printable ASCII only.
- **Notes kept:**
  - The guard does not tie each SHA to 0001 (owner: not now, §11 question 9).
  - `check.sh`'s own guard still misses a line wrapped in `if false; then … fi`. This predates S2.
  - `CERASTES_JOBS` has no upper bound, and `-j 0` crashes; both predate S2.
  - `walker_oracle.py` keeps `min(8, cores)`.
  - A fresh clone's harness self-test skips one test more (0016 item 11).
- **Owner answers** (§11 questions 6–9):
  - a required check on `main`: yes;
  - when the image's tools change: move the assertion and 0001 together after a rehearsal;
  - tighten the timeout after two green runs on `main`;
  - SHAs tied to 0001: not now.

## 5. The mutation triage session (S3; done 2026-10-21, result at the end; branch `mutation-triage`)

**Planning facts and owner decisions (2026-10-21).** Independent of the box. Agents: test-mutator A (tooling:
`mutate.py`, `lint_viper.py`, their self-tests, `tests/README.md`), test-mutator B (strengthened tests, twins, list
proposals), soundness-auditor (the tooling diff, every new list entry); il-analyst only if a survivor needs IL
evidence.
- **Weak tests are strengthened in place** (owner): a surviving mutant of a file can only be killed by that file's own
  contract, so the file's `ensures`/`check` are tightened; its `expect:` lines are untouched, it must verify in every
  mode it declares, and each new clause gets a must-fail twin. Doc examples change their pages (`build_docs.sh`,
  `check_doc_examples.py --html build/doc`).
- **Size now:** `tests/scalar` has 140 positives, **3522 mutants** (`--list`, 90 s), 738 in the bucket (`--validate`);
  a full run at `-j 8` is about 45–90 min. Run it once, alone, after the tooling and the strengthening, capped at
  100 min; nothing else but text review meanwhile.
- **Trap 1:** a run over a subset reports every list entry whose file is not selected as "no such mutant is
  generated" and exits 1 (`m4.2a-mutate.log` has 125 such lines): check staleness only for selected files.
- **Trap 2:** a `doc/…` entry would fail `check.sh`'s `mutate.py --validate` (default `tests/scalar`; the line and
  `DEFAULT_PATHS` are pinned by self-tests): make `--validate` also generate every file the list names.
- **Corrections to the counts below:** the per-file leaver list adds up to 73, the reason breakdown to 74 (the static
  recompute settles it; import both modules, because `e04ce6e`'s `--list` alone starts a full run). The two
  `cond_div` files have **22** M4.1 proposals, not 34; only 10 are test weakness (the literals `3→2/4`, `1→0/2` and
  the deleted `d += 1`, five per file); deleting the `continue;` is a true equivalent (the last statement of the loop
  body), as are the measure, invariant, `20→21` and `n <= 6` mutants.
- **Tooling items** (with self-test twins): progress on stderr with `flush` via `submit`/`as_completed` in `run_all`
  (stdout stays as it is: the self-tests read it); traps 1 and 2; `assert(e)` modelled as a CHECK clause (0015) before
  the 12 "weaker assert" proposals are listed; audit A items 1–5 (§6); compile each clause once (`_holds`;
  `switch_nested.c` alone takes 60 s to generate); the duplicated test name in `test_mutate.py` (377 and 451: the
  case-label test never runs). Proposals in `m4.2a-survivors-proposed.txt` have three columns; the list takes two.
- **Stop rule:** more than about 10 new weak tests beyond those named here go to the handoff.
- **Acceptance:** the full run exits 0 (0 unexplained survivors, 0 tool failures, no `NO KILLED MUTANT`);
  `--validate` clean; the mutate and lint self-tests green; docs 0 problems; `check.sh` green; no open blocking audit
  item.

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
session's stop rule moved here. test-mutator's proposals are in `internal/triage/m4.1-survivors-proposed.txt` (id, tab,
reason; copied from `build/m4-staging/` in M4.2a). 34 of them are in `continue_cond_div.c` and
`do_continue_cond_div.c`: test weakness, not equivalence (deleting the `continue;` survives) — strengthen those two
positives (an exact `ensures` on a result the continue branch changes; the twins may keep a weaker invariant so that
the reported error stays `division-by-zero`, said in their headers). The other 36 look like genuine equivalents or
safe-direction mutants (the last `break` of a switch, dead initialisers, measures of loops that never repeat, slack
bounds, `while (1)` → `while (2)`, weaker `ensures` on unsigned parameters, which the classifier does not model);
each needs the auditor's review before it enters `tests/mutants-equivalent.txt`.

**M4.2a's survivors (added 2026-10-21).** `python3 scripts/mutate.py -j 8` over the 27 new positives and 4 doc
examples (`internal/triage/m4.2a-mutate.log`): 913 mutants, 476 killed, 220 safe-direction, 13 stillborn, 200
survivors and 4 tool failures. The 15 survivors and 4 tool failures of `verifying/goto_exit.c` are gone: Silicon timed
out on its nonlinear contract, so the example was replaced by a linear one with an exact contract (30 mutants, 0
survivors). test-mutator classified the other 185 read-only, by rule per file, in
`internal/triage/m4.2a-survivors-proposed.txt` (id, class, reason): 139 equivalent, 31 safe-direction the
classifier does not model (asserts, `||` ensures, a bodyless callee's ensures), 15 weak tests, 0 suspicious. Most
equivalents are guards that make a function end the program on more inputs: the amended meaning of "verified" (0015
item 4) cannot see them. The weak tests are in `assert.c` (doc: pin `r == (a + b) / 2`), `noreturn_fixed.c` (doc: add
`ensures r > 0`), `goto_in_loop_body.c`, `goto_cleanup.c`, `goto_out_of_loops.c`, `exit_main.c`. Each entry needs the
auditor's review before it enters `tests/mutants-equivalent.txt`. M4.1's 70 proposals are now tracked too:
`internal/triage/m4.1-survivors-proposed.txt`.

**Result (S3, 2026-10-21, 16:47–20:15; branch `mutation-triage`).** Three test-mutator agents and one Opus auditor:
A for the tooling, B for the named weak tests and the doc examples, C for the M3 leavers.
- **The leaver count** is 74 (`call_contract_only.c` has 4, not 3). Recomputed statically by importing
  `e04ce6e`'s `mutate.py` and HEAD's.
- **Stop rule hit.** Audit A item 3 as written (bucket a drop-conjunct or swap-logic mutant only if no possibly
  undefined arithmetic remains in its clause or a later one) moved 145 mutants out of the bucket. The same reasoning
  would also apply to 242 relop and literal mutants.
  - **Owner's answer, "verifier-checked":** no guard rule. A surviving mutant has passed cerastes' own contract
    well-definedness check (0010 item 11: the `__wf` method checks each `requires` under the earlier ones, and each
    `ensures` under the `requires` and the earlier `ensures`; checks, asserts, invariants and measures are checked
    where they are evaluated).
  - That check is pinned per clause kind by `tests/scalar/guard_kept.c` and `negative/guard_dropped_requires.c`,
    `guard_dropped_ensures_later.c`, `guard_dropped_ensures_later_overflow.c`, `guard_dropped_overflow.c`,
    `guard_swapped_requires_overflow.c`, `guard_swapped_check.c`, `guard_swapped_assert.c`,
    `guard_changed_invariant.c`, plus the existing `contract_wf_divzero.c` and `contract_wf_overflow.c`.
  - The auditor confirmed the argument against `src/vir/lower.cpp`.
- **The bucket after the tooling:** 30 mutants leave (any `*` is `unknown`) and 2 join (asserts of `assert_ndebug.c`).
- **Tooling** (round-1 audit: 1 blocking, a logic group inside two brackets read with Python's meaning; fixed):
  - progress lines on stderr;
  - traps 1 and 2;
  - `assert(e);` as a CHECK clause;
  - the C-exact reading;
  - `has_caller` with `#define` bodies and default arguments;
  - the split-`decreases` lint;
  - speed: `--list`/`--validate` 2 min → 3 s, the mutate self-test 98 s → 15 s;
  - the duplicate test and its check;
  - the event-ordered progress test.
- **Tests**, 17 positives strengthened in place:
  - B: 6 named M4.2a/M4.1 tests, 2 doc examples, `do_zero_expr.c` (round 2: its "equivalents" were stronger
    `ensures` on a slack bound) and `switch_call.c`;
  - C: 7 M3 files, with callers and bounds appended so that no listed id moved.
  - 33 new twins; no `expect:` line of an existing test changed.
- **The list:** 436 entries. Of the 125 M3 entries, all are kept, with ids unchanged. 311 are new, under 120
  distinct reasons, reviewed by the auditor in two rounds plus a re-audit. The header records it.
- **`internal/triage/`** (the proposal files above) is removed: every entry is resolved, killed or listed;
  `git show 50f07ca:internal/triage/<file>` keeps them.
- **Acceptance:** **Full mutation run**, in three shards balanced by verifier runs (`-j 10`, 18:13–19:55, 1 h 41 min), over `tests/scalar` and the four doc examples: each shard exits 0. Together: 145 tests, 3880 mutants: 2652 killed, 436 equivalent (reviewed; every list entry survived), 733 safe-direction, **0 unexplained survivors**, 59 stillborn, **0 tool failures**; `check.sh` green (14 min 7 s, at the final tests and list): self-tests 951 + 74 + 152 + 225; tests 2694 passed, 0 failed, 1 skipped (native-ok: Apple clang 644, GCC 644); 733 `.vpr` lint clean; VIR self-test 230/230; walker oracle 134 goldens, 0 failed; docs 0 problems. Run #1 (14 min 37 s, before the audit fixes): green, 2678 tests.
- **Process.** A, B and C ran their file-level runs at `-j 4` alongside the owner's IDE and a VM (load up to 96). The
  full run waited for a quiet machine and was split into three shards balanced by verifier runs. Each shard is a
  complete run of its files: trap 1's fix makes shards add up to one run.

## 6. Should-fix items and known gaps

**S3 audit notes (2026-10-21, not blocking):**
1. `assert` is recognised by name (`_assert_calls`), not as the `<assert.h>` macro: a user-defined C++ function named
   `assert` would be bucketed like a check. None exists in the corpus.
2. A literal mutated past `9223372036854775807` changes its C type to unsigned. No test has such a literal.
3. `test_the_guard_rule_is_gone` (`test_mutate.py`) is a `hasattr` check; its behavioural twin does the real work.
4. Silicon checks an invariant's well-definedness at entry and preservation, not in the arbitrary loop-head state.
   This is sound (every reachable state is checked, and invariants are never bucketed); `guard_changed_invariant.c`
   pins it. A loop measure's range obligation is an invariant clause too: checked at every evaluation of the loop
   head (`measure_loop_overflow_later.c`).
5. A run checks list entries only for the files it selects, so an acceptance run must name every file the list names
   (the doc examples too). `tests/README.md` says so.

**Audit A (tooling), deferred to the triage session** (*all five closed in S3*):
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
1. *Closed in M4.2a:* `lint_viper.py` now checks every `goto` outward and forward at token level.
2. Three encoder rejection branches look unreachable and have no test (a `break`/`continue` whose target cannot be
   determined, a non-integer scrutinee, a case value of another type); they fail closed.
3. The probe/spike evidence of M4.1 is under `build/m4.1/` (gitignored; the facts are in `il-inventory.md`);
   two spike headers (S1b, S7f) state stale expectations, the record's claims are right.

**M4.2a audit notes (2026-10-21, not blocking; every should-fix item was taken):**
1. **Who wrote the noreturn flag.** The IL keeps only a routine's first declaration position and no list of its
   declarations (`GENERATE_SOURCE_SEQUENCE_LISTS` is off). The trust "declared noreturn here" therefore checks the first
   declaration and that no `noreturn` attribute entry sits outside a verified file. `_Noreturn` leaves no entry, so a
   `_Noreturn` written in a system header would not be seen. None exists in the pinned SDK (grep). The sound check is
   EDG's source sequence lists, a configuration change (decision 0002) for a later session. The same limit makes the
   libc table check "declared only in system files" on the first declaration only (0015 item 5; harmless, since
   must-match stops a user redeclaration from adding a contract).
2. **Unreached guards** (defence in depth, commented in `encode.cpp`): a declaration directly in a compiler-generated
   block that is not flattened (P20 found no such shape), and a `cerastes::` attribute on a label (EDG rejects it
   first, #1835).
3. **A compiler-generated goto or label without break/continue flags** is now treated as a user one, and
   `goto_problems` fails closed on it. Before M4.2a it was rejected; no program is known to produce one.
4. **Diagnostics.** Goto rejections use the generic `unsupported_statement` text, which ends "(decision 0012)", not
   0015. A misplaced GNU `__attribute__((noreturn))` is reported as `unrecognized_attribute` and spelled
   `[[noreturn]]` (EDG keeps it as unrecognized).
5. **Untested.** `_Noreturn int main` in C23 is rejected, but EDG's warning #3271 (a noreturn function with a non-void
   return type) also prints, which a `rejected` test cannot list; only the C++ variant is tested. The near-miss tests
   of a user `abort`/`exit` are C++ only: C compilers treat `abort`/`exit` as builtins, so they cannot be `native-ok`.
6. **`mutate.py --list`** took about 3 minutes under load (0.6 s at the M3 close-out on an idle machine).
7. The lint's existing "while without decreases" rule reads up to the first `{`, so a quantifier trigger inside an
   invariant would be misread as the loop body. The emitter writes no trigger today (test-mutator).

**M4.2b1 audit notes (2026-10-21, not blocking; every blocking and should-fix item was taken):**
1. **Static assertions are not encoded** (0010 item 13): `_Static_assert((1 << 40) == 0, "")` verifies with #63
   suppressed, which clang refuses. (A C `_Static_assert` naming a constant is refused through #1901, and EDG's
   diagnostic pragmas, which could turn that back into a warning, are now rejected in verified files.) The pragma
   rule also rejects common harmless pragmas in user code (`#pragma clang diagnostic`, `#pragma GCC diagnostic`):
   an allow-list of pragmas that cannot change meaning would lift that.
2. **Messages.** A block-scope `extern const int A;` naming an internal-linkage C++ constant is reported as "a static
   local variable"; in C, a block-scope `extern` followed by a file-scope `static` tentative definition as "a local
   variable without an initializer" (both fail closed). A weak function written with the attribute gets two
   diagnostics (`unsupported_attribute` and `unsupported_declaration`). The rejection texts still end "(decision
   0012)" (as M4.2a's do).
3. **`names_object` returns false on a node kind it does not know** (a `decltype(k){}` constant): the cross-mode rule
   fails open there; still sound, since a `DoOnce` keeps its `internal` guard.
4. **Verify output noise:** EDG warning #177 ("declared but never referenced") prints for an unused C++ named
   constant; consider suppressing it in verify mode.
5. **`perturb_folded`** can make two adjacent case values equal, which `check_program` rejects (exit 2, not 3): a
   self-test fixture must keep its case values apart (done).
6. **Several units:** a C++ internal-linkage constant named in a trusted contract may have another value in the
   defining unit (documented on Verifying a program; the contract manifest is M6).
7. **`mutate.py` does not mutate namespace-scope initializers** (pinned by `test_mutate.py`), so a constant's own
   value is never mutated; the planned extension (P2) was cut. Mutating constant initializers would test how the
   prologue's checks and the uses depend on the value.

**M4.2b2 audit notes (2026-10-22, not blocking; every blocking and should-fix item was taken):**
1. **Pointers will reopen the C enumerator-list hole (for M4.5).** EDG types an earlier enumerator inside a C
   enumerator list the C++ way; cerastes rejects the routes that observe it (a reference whose node type differs,
   `sizeof`/`_Alignof` of anything but a plain arithmetic type, `typeof`; 0018 item 10). Pointer arithmetic is a further
   route the front end accepts in an enumerator value (`(int)(long)((typeof(N) *)0 + 1)` folds to 8, C gives 4; also
   `&((char (*)[e])0)[1]`), closed today only because pointer types are rejected. When M4.5 accepts pointers, keep them
   out of C enumerator initializers or apply the node-type rule there.
2. **The C rule is deliberately conservative:** in a C enumerator initializer it also rejects `sizeof` of any expression,
   arrays, typedefs of arrays and typedefs of `typeof`, even when they do not depend on the list (fails closed; 0018 item
   10 says so).
3. **Untested defence-in-depth branches** (no IL shape reaches them): in `comparison()`, raw enumeration operands of
   different types and an order on an unscoped enumeration; "an enumerator whose enumeration this version cannot find";
   "a scoped enumeration whose underlying type is not int"; "values do not fit its underlying type".
4. **Fail-closed corners:** in C++ `x == (color)2`-style comparisons were rejected until the folded-operand rule (now
   accepted); a C case value of a shape `enum_case_value` does not know (neither the generated cast, a written cast, an
   enumerator nor a copy) is rejected ("a case value this version cannot read"); `?:` of enumeration type is rejected in
   C++ while C reads `x ? RED : GREEN` as `int` (0018 item 8).
5. **The flag `is_named_constant_definition`** is also set by EDG on predeclared SVE/RISC-V enumerations (a `#pragma GCC
   aarch64 "arm_sve.h"` position), module enumerators and non-standard member constants; cerastes requires the
   enumerator's enumeration to have passed `enum_info`, so none of them is trusted (0018 item 9).
6. **A C23 tag redefinition** gives a second type entry for one enumeration (harmless: uses refer to the first; both are
   checked).
7. **Messages:** an out-of-range conversion names the enumeration as each mode spells it (`enum color` in C, `color` in
   C++); rejection texts still end "(decision 0012)" as before.
8. **`#ifdef __CERASTES__` in a native-ok positive** (`enum_case_promoted.c`) is a hand-reviewed convention
   (`tests/README.md`), not a harness rule (§11 question 19). *(M4.2c: the owner forbade the pattern; the file is
   retired and `tests/README.md` says so.)*

**M4.2c audit notes (2026-10-22, not blocking; every should-fix item was taken):**
1. **`c >= +GREEN`** is accepted in C (the front end drops a unary `+` on an enumerator, so it is the IL of `c >= GREEN`)
   and rejected in C++: a difference in what the modes accept, never in meaning (0019 item 10).
2. **Rejection texts** still end "(decision 0012)", also for the new "a const local variable without an initializer"
   (0019 item 1).
3. **`switch_enumeration`** also accepts `?:` arms that are enumeration values (not only enumerators); 0018 item 8
   rejects those shapes first, so nothing changes.
4. **Duplicate display of an overridden note** (`--diag_remark`/`--diag_warning`/`--diag_error` on a note's number):
   a separate, pre-existing EDG display bug, recorded in bug 0022's text, not fixed by `0022` (memory: EDG bugs scoped
   to cerastes).
5. **Incidental:** `constexpr int z = (-2147483647 - 1) / -1;` (and `% -1`) gives no diagnostic in this EDG (P30); cerastes
   encodes initializers as written (0017), so it does not rely on the fold.

**M4.3a audit notes (2026-10-22; round 1 had one blocking item, fixed and re-audited in round 2; every should-fix item
of both rounds was taken):**
1. **Round 1's blocking item** (fixed): the first lowering kept every effect in one left-to-right order, so in
   `never_returns(0) + 100 / y++` the checks of `100 / y++` followed a call that does not return ("verified" although C
   may divide first). Calls are now the only fixed points (0020 item 7 E1–E2); four must-fail self-test cases and
   `tests/scalar/negative/never_returns_unsequenced_divzero.c`, `never_returns_comma_divzero.c` pin it.
2. **`check_variable_order`** (`src/vir/lower.cpp`) compares the emitted order of each variable's snapshots and writes
   with their creation order, which holds by construction, so it cannot fire today and no hook shows that it can. That
   creation order is C's order rests on the sequencing rule (0020 item 4) alone.
3. **`lint_flags`** (`scripts/lint_viper.py`) checks that a flag is set on the line right after the write. A write and its
   flag hoisted above a read C performs first would pass the lint; only lowering's placement rule guards that.
4. **The comma floor is conservative in the sound direction** (0020 E6): an item that uses a comma's value, or a sibling
   sequenced after it, may be placed before the comma's call, so `(never_returns(0), 1) && 100 / d` reports a false
   `division-by-zero`.
5. **Unpinned safe rejections:** `0 && (x = 1)` and `1 || x++` (EDG folds them; rejected with a message that now says
   so), `assert((note(x), x >= 0))` (rejected by `check_program`). No tests.
6. **Warnings outside P35:** clang's `-Wself-assign` on `x = x;` and on the new `y = (x = x)` (accepted; §11 question
   24); `_Generic(x++, …)` is rejected already (the encoder does not accept `_Generic`).
7. **`decltype(x++)`** is accepted with no test (clang warns, so no native-ok positive can hold it); `typeof(x++)` is
   pinned by the C-only `typeof_effect.c` (0020 item 5).

**M4.3b audit notes (2026-10-23; round 1 on the record had one blocking item, fixed in the record and in the code; round 2
on the code and on the tests had none; every should-fix item of both rounds was taken):**
1. **Round 1's blocking item** (fixed): the family F held PRE(c) only for the termination checks that end up printed, which
   depends on the order F determines; in `nr(k) + g(h(x))` with `g` recursive, g's explicit check would have been placed
   after `nr`. F now holds PRE(c) for every call in the caller's recursion cycle (0021 item 3); a VIR case pins it.
2. **Smaller sets are sound but cost false alarms** (round 2, recorded in 0021 item 10): a precondition's set is its
   arguments' values and the floor, and a comma's value does not carry its left operand's floor, so
   `h((validate(y), y))` (with `validate` ending the program unless `y > 0`) and `100 / (validate(y), y)` report errors C
   cannot reach. Exact sets would change the Viper of M4.3a-era programs (§11 question 27).
3. **Writes and snapshots are in F** (0021 item 3): `(x = f(a)) + (y = g(b))` is rejected although neither write can fail
   (§11 question 28).
4. **The fail-closed checks check the plan, not the emitted text** (round 2): `check_fixed_order` and
   `check_variable_order` cannot see a set computed too large (the only unsound direction). The pins guard that, and the
   auditor traced each to verify if its mechanism were removed: `never_returns_call_divzero.c` (the chain order),
   `branch_after_noreturn_divzero.c` (D(B) in F), `noreturn_recursive_pair_same.c`, `recursive_after_noreturn_same.c` and
   the `call_term_*` twins (the explicit termination check), `call_term_arg_value_same.c` (the argument's value), and the
   `_pre` twins of the loop conditions (a call made at every evaluation).
5. **Over-rejections** (sound): `a && (y = x = 1);` (every compiler is silent; the constant test looks one assignment
   deep); a discarded `a && (flag = x);` with a `bool` target and a non-constant value (GCC is silent; untested); a `?:`
   with `void` arms (`unsupported_type`, §11 question 30).
6. **Lowering's internal errors exit 2, not 3** (predates M4.3b): they are reported as `unsupported_expression` ("an
   internal inconsistency: …"), and `verify_main.cpp` maps only EDG's own internal errors to 3 (§11 question 29). Since
   M4.3b they are reported before lowering's rejections, so a rejection never hides one.
7. **Accepted shapes that print EDG's #187 cannot be pinned by a verified test** (`a && (y = 2);`, `a && (flag = 0);`
   with a `bool` target): the harness fails a `verified` run on any warning. `discarded_and_assign_const.c` pins the nearest
   warning-free shape (`a ? (y = 2) : 0;`), and `build/m4.3b/edg/disc.py` the probes (§11 question 31).
8. **Some twins pin nothing specific to 0021** (round 2, tests): the loops' `_termination`/`_preserved` twins,
   `call_reorder_divzero.c` and `call_comma_floor_divzero.c` fail for their stated reason at their line, as they would have
   before M4.3b; the positives and the pins carry the rule.

**M4.4a audit notes (2026-10-23).** Round 1, on the record and P38, had no blocking item; its should-fix items were taken
into 0022. Round 2, on the code and the tests, had one blocking item: two different enumerations in `==`/`!=`, now
rejected in both modes. Its should-fix items were taken, and a re-audit checked the blocking fix. Notes not taken:
1. **What EDG still computes itself, and wraps** (0022 item 2): `_Static_assert`, `#if`, array bounds and other
   expressions in types, template arguments, and character literals (`'\xff'` is -1, `L'\xffffffff'` an `int` -1;
   `'abcde'` is truncated with only warning #1654, which verify mode does not suppress). These are C's own values, so
   nothing is unsound, but they are outside the real-integer model (§11 questions 32–33).
2. **The exit-3 paths for emitter problems and "encoded twice"** have no test; no program reaches them without a hook.
   `--selftest` sets the flag itself rather than running the encoder's path, while `RealDriverTests` covers the
   lowering path end to end (`perturb_fixed_order`).
3. **The defensive rejection in `enum_case_value`** (a constant with an `orig_type` and no `expr`) is unreachable and
   untested; `constant_value` already rejects that shape.
4. **`same_fixed` in the doc example `enum_compare_signed.c`** is shown on the page but never verified, since the file is
   a rejection test. The accepted shapes are pinned by `tests/scalar/enum_eq_signed_ok.c` and `enum_eq_truth_ok.c`.
5. **Item 5 is stricter than the compilers** for `x == (enum color)2` and `c == (x > 0 ? 1 : 2)`, and stays as strict as
   clang for `c == (int)b` (GCC is silent there). See §11 question 34.
6. **A rejection test matches only the tag and the line.** That holds for the `reject_enum_eq_*` tests and the doc
   example. Each shape was accepted before item 5, so removing the rule fails them.
7. **Re-audit 2b's notes.** The blocking item is closed: no `==`/`!=` with an enumeration gives opposite meanings in the
   two modes. Its should-fix items were taken: the record's wording, and four rejection tests (`c == (int)b`, `RED ==
   NB`, `f == n`, `f == NA`). The rest:
   - a few shapes are rejected in one mode and accepted in the other with the same values (0022 item 5, "Verdicts in
     the two modes"; §11 question 36);
   - `switch (c) { case NB: … }` over `enum color` is accepted while `c == NB` is rejected: the same values, different
     policies (0019 rule 1 against 0022 item 5);
   - C's `case -1:` over an enumeration prints the wrapped value 4294967295 in the `case_not_in_enumeration` message,
     where C++ prints -1 (a message only; predates M4.4a);
   - lowering's internal-inconsistency diagnostics have no source position (acceptable for a tool failure).
8. **The in-place changes under the rule** (0022 status). The body changed; no `expect:` line did. Each change is
   listed with its new twin, which pins the wrap:

   | Tests changed in place | Change | New twin |
   |---|---|---|
   | `assign_value_unsigned.c` and its `_post` | `requires 0 <= a <= 255` | `negative/assign_value_unsigned_conversion.c` |
   | `incdec_value_unsigned.c` and its `_post` | `requires c <= 254` | `negative/incdec_value_unsigned_conversion.c` |
   | `uchar_promote.c` and its `_post` | `a == 254` | `negative/uchar_promote_conversion.c` |
   | `uninit_compound.c` | `requires a <= 4294967292u` | `negative/uninit_compound_overflow.c` |
   | `const_global_init.c` and `negative/const_global_init_{over,shift,conversion,post}.c` | `U = 65535u * 65537u` | `negative/const_unsigned_negative.c` |
   | `enum_list_type.cpp` and its `_post` | `V = U * 1073741824 > 0` | `negative/enum_list_type_wrap.cpp` |
   | `negative/assert_cast_kept.c` | `requires 0 <= x <= 255` | `negative/assert_cast_conversion.c` |
   | `negative/compound_ushort_over.c` | `a == 65535 && b == 65535` | `negative/compound_ushort_conversion.c` |
   | `negative/compound_mixed_negative.c` | `requires i >= 0 && …` | `negative/compound_mixed_neg_operand.c` |

   The doc examples `integer-semantics/{narrowing,constants,shifts,promotion_ok,promotion_overflow,usual_conversions}`
   were changed the same way, with `unsigned_checked.cpp`, `unsigned_overflow.cpp`, `modular_explicit.cpp`,
   `constant_unsigned_error.cpp` and `usual_conversions_error.c` new.

**M4.4b1 audit notes (2026-10-24).** Round 1 (the record and P40/P41/S21) and round 2 (the code and the tests) had no
blocking item. Their should-fix items were taken into record 0023 items 2, 4, 5, 6, 7 and 8, and into three tests
(`bit_accepted_shapes.c`, `bit_enumerator.c`, `bit_int128.c`'s high half). Notes not taken:
1. **`~` has no lowering-side guard** (`src/vir/lower.cpp`, `unary`). The binary operators raise an internal error on
   an enumeration or a width other than 32/64/128; `BitNot` does neither. It is unreachable: `check_program` rejects
   enumeration operands of unary operators, and the front end always promotes the operand. Add the same guard when
   `lower.cpp` is next touched.
2. **One message differs between the modes, with the same verdict.** For `d == (x ? (y, RED) : GREEN)`, C reports only
   the enumeration-`?:` rejection. C++ also reports `comma_without_effect`, since C returns before the arms are encoded.
3. **Unevaluated operands are not scanned by the detectors** (`sizeof(char[2 ^ 8])` verifies, with EDG's value; record
   0023 item 5 lists it). M4.4b2 inspects the types inside such operands and may scan them as
   `unevaluated_side_effect` does.
4. **`mutate.py`:** `drop-complement` on a C++ destructor `~T`, and `swap-bitwise` on `(T)&x`, give stillborn mutants
   (harmless).
6. **A closed 128-bit claim over a shifted constant other than 1 is undecided** (the mutation review: `and_wide(-1,
   (__int128)2 << 100)` ends at `--timeout`, exit 3, where `(__int128)1 << 101` is refuted in 39 s and `2L << 40` in
   14 s). Lowering writes the shift as `2 * c_pow2(100)`. Folding a constant `k * c_pow2(n)` in lowering would decide
   it, but it changes the Viper of every existing constant shift, so it is left for a session that may change that
   output; record 0023 item 2 and the Integer semantics page say so.
7. **The mutation wrapper's cap was below the driver's `--timeout`.** The harness SIGKILLed `cerastes verify` and its
   Silicon process groups were orphaned (19 JVMs, load average about 800, cleaned up). A cap must stay above the
   driver's `--timeout`, or the driver must be given a lower one; `run_tests.py`'s `run_tool` could send SIGTERM and
   wait before killing (a harness item for a later session).
5. **Item 2's evidence.** S21's symbolic claims (and its hierarchical form) were luck-dependent: they passed S21's
   battery and hung on the encoder's methods. The session's measurements of the real output are what item 2 rests on.
   Silicon's start is 1.3 s, and the bitwise tests take 1.4–6.3 s per file at 32/64 bits and 11–20 s at 128 bits.

**M4.4b2 audit notes (2026-10-26).** Round 1 (the record and P42–P45) had three blocking items, all fixed in the record
and the code and pinned by tests: character-literal identification by value alone misread enumeration-typed copies
(census 2); the record hid the spike's golden changes and the new rejection of attributes on redeclarations; and the
skip pre-scan let a user call inside `sizeof` switch off a whole assertion. Round 2 (the code and the tests) had no
blocking item; its should-fix items were taken (the walker's rule narrowed to redeclarations, recursion guards that
reject, an enumeration's underlying type through an alias, one report per rejected declared type, the `perturb_folded`
harness cases, a C++ float condition test). Notes not taken:
1. **The cross-check covers the file scope only** (round 2 item 4; record 0024 item 2): inside functions the statement
   walk of every encoded body reaches each assertion, and bodies that are not encoded (rejected functions, `predicate`
   and `pure` bodies) hold unchecked ones; no test reaches the "does not check" rejection (§11 question 40).
2. **The walker listing** (round 1 preferred listing a redeclaration's attributes, which changes two goldens; the owner
   chose to keep the listing). The encoder checks those attributes either way.
3. **The character-literal trigger compares VIR type names** (`int`, `char`, `wchar_t`); `int_kind` would be sturdier.
   `Program::int_type` keeps the first name it sees for a shape, so after the helper registers `char` a later C
   `signed char` message without its own spelling may say `char` (message text only; Viper has no type names).
4. **A skipped assertion is silent** (round 1 note 13; §11 question 41).
5. **The walker oracle's new check** (last review): an `on type <unnamed>` line must match an unrecognized attribute
   entry by position, name and tokens, but the candidates include entries already matched by routine lines, so a phantom
   line for an attribute the walker lists under its routine, or for a redeclaration's suppressed attribute, would pass
   the oracle; only the golden comparison catches it. Removing already-matched entries from the pool would close it.
   Also: the walker lists neither a first declaration's own attribute after the parameter list nor one on a template
   definition (both predate this session, round 2 note 10).
6. **Minor:** a typedef whose first reason is not an array (`typedef unsigned _BitInt(8) fn(char a[3]);`) is not rejected
   at the typedef (any use is); `_Static_assert(1.5, "")` names its type in C (`unsupported_type`) but not in C++
   (`unsupported_expression`); the M4.5f gap of assertions in a `pure` body has no pinning test.

**M4.4c audit notes (2026-10-26)** (audit rounds 1 and 2 and the re-check; items not taken):
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

## 7. Known gaps carried

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

## 8. Session plans: M4 (the rest of C) and M5 (generic templates)

**Re-sequencing (owner, 2026-10-20).** Planning M4 (generic templates) showed that Gate G2's six VeriFast tests use
pointers, references, structs and constructors, which M3's scalar fragment cannot express. The owner renumbered the
plan (`plan.md`, "Milestone renumbering"): the **new M4 is the rest of C** (strictly C, sub-milestones M4.1–M4.8,
pointers in M4.5a–f, VerCors as the reference); generic templates are **M5** (M5a–M5g); concepts M6; **C++ memory
and classes M7** (references, `new`/`delete`, classes; G2's parts that need them are deferred there); inventory M8;
evaluation M9. Session order: M4.1 → M4.2a (owner, 2026-10-21: before CI) → EDG upstream merge (§4.1) → CI (§4.2) →
mutation triage (§5; done 2026-10-21) → M4.2b1 → M4.2b2 → M4.2c → M4.3a → M4.3b (owner, 2026-10-22: M4.3 split) → M4.4a → M4.4b1 → M4.4b2 → M4.4c → M4.4d (owner, 2026-10-23: M4.4 split, then M4.4b) → M4.5a … M4.8 → M5.0 (owner, 2026-10-26: type traits, §11 question 39) → M5a … M5g → M6.

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

**M4.1 Control flow** (done 2026-10-20; see `internal/status.md`).

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

**M4.2b split into M4.2b1 and M4.2b2** (owner, 2026-10-21, M4.2b planning: constants, case values and
enumerations did not fit one ≤ 4 h session).

**M4.2b1 Named constants and case values** (record 0017; run 2026-10-21). `const`/`constexpr` globals defined in a
verified file and constant-initialized; uses take EDG's folded value as a literal (bodies, contracts, annotations,
case values); each initializer is encoded by its original expression and checked in a **prologue at the entry of
every method** (owner, replacing the planned separate `globals__wf` method), with an `internal` guard against EDG's
value; case values (`case k` with a const local, `case N` with a const global) checked in the same prologue, so
path-independently; a `do … while` condition that names a variable is a measured loop in both modes (the cross-mode
trap); hook `perturb_folded`. See `internal/status.md` for what landed.

**M4.2b2 Enumerations** (record 0018; designed in the M4.2b planning, 2026-10-21; owner answers below; **run
2026-10-22**, see `internal/status.md` for what landed and how it departs from this design: C's enumerator-list typing,
the C switch in the promoted type, the copies, #188). Retirement pre-approved: `reject_enum.c` (retired).
- **Owner (2026-10-21):** an out-of-range conversion to an enum with a fixed unsigned underlying type
  (`enum E : unsigned char`, `(enum E)300`) is a `conversion` error, like every conversion into an enum. **Note to
  relook:** wrapping like unsigned in general (0010's modular unsigned arithmetic): *integer semantics should model
  real integers, not how C and C++ support them* (§11 question 13).
- **VIR:** `IntType` gains `is_enum`, `is_scoped`, `lo`, `hi`; `Program::enum_type(...)` always makes a new type in
  its own store (never through `int_type()`'s interning by shape, whose "first name wins" could return an enum for
  `unsigned int`); the encoder caches one type per `skip_typerefs(t)`; `Type` equality for enums is identity;
  `dec::range_of` returns the value range (so formals, results, `__wf`, the automatic loop invariants, Const range
  checks and `convert`'s containment test follow); `convert` into an enum: `range_obligation`, category `conversion`
  ("the value may not be one of the values of `enum color` (0 to 3)"), never `c_wrap`; `check_program` rejects
  arithmetic and unary operators on an enum operand and `BoolToInt` to an enum; `compound_assign`/`inc_dec` on an enum
  build VIR that `check_program` rejects (they compute in `unsigned` today). Emitted Viper of existing tests must stay
  byte-identical.
- **Value range** (`enum_range()` in the encoder): fixed (`has_explicit_enum_base` or `is_scoped_enum`: a scoped enum
  without a base has underlying type `int`, [dcl.enum]/5) → the underlying type's range; unfixed → [dcl.enum]/8 over
  `enum_info.constant_list` ({0} and an empty C++ enum give [0, 1]; {0..2} [0, 3]; {−1, 0, 1} [−2, 1]), computed in
  `dec` strings; C gets the same (stricter than C). Rejected at the declaration: an enumerator outside `int`, a `bool`
  base, an incomplete unfixed type; block-scope enums stay rejected; enums from system headers rejected (M8).
- **Accepted:** enum parameters, results, locals, const globals, `old`, `decreases`; every `eok_cast` into an enum
  (checked); casts to `bool`; explicit casts from an enum; an implicit enum → integer cast only (i) as a direct
  operand of `==`/`!=`, (ii) as the switch scrutinee, (iii) as a whole initializer, assignment right-hand side,
  argument or return value, (iv) when its operand is an enumerator reference; enumerator references become literals
  with EDG's value, intercepted at the top of `constant_value` (before `c->expr` is read, so the initializer is not
  encoded at each use); enumerator initializers checked in the M4.2b1 prologue; scoped enums: switch, `==`/`!=`, and
  ordering of two raw operands of one scoped enum.
- **Rejected** (`unsupported_expression` at the cast unless noted): any other implicit enum → integer cast
  (arithmetic, ordering, unary, shift operand or count, `?:` arms), `?:` of enum type, compound assignment and
  `++`/`--` on an enum or with an enum right-hand side; `bool` base, an enumerator outside `int`
  (`unsupported_declaration`); system enums (`unsupported_type`).
- **One meaning:** every enum value lies in its value range (checked conversion, enumerator, `requires`, `ensures`),
  which lies in `int` (and in `unsigned` when non-negative), so `==`/`!=`/switch, conversions out of an enum and
  enumerator arithmetic agree in C and C++. `.cpp` only: `enum class`, `E::A`, `static_cast`, empty enums. c23 only:
  implicit int → enum, `e++`, `e += 1` (rejections), an enumerator initializer C++ refuses.
- **Probe first** (il-analyst, both modes; P-numbers continue after M4.2b1's): the type entries (`int_kind`, `size`,
  `base_type`, `constant_list`/`assoc_scope`), C's compatible type with and without negative enumerators, the type of
  an enumerator constant per mode (`affiliated_type` in C), how a use is linked to its enumerator (pointer equality
  with the `constant_list` entry or another flag), every conversion node EDG inserts (`compiler_generated`, target),
  C++ promotion of `A + 1` and `e + 1`, the scoped switch's scrutinee and case types, EDG's enum warnings. Fix
  il-inventory L1084 (`tk_enum` = `tk_integer`, `il_def.h:5583`; done in M4.2b1). **Stop rules:** enumerator uses
  that cannot be identified by pointer or flag → stop and ask (no value or position heuristic); C++ promoting an
  unfixed enumerator to anything but `int` → stop; C leaving enum operands without conversion nodes → amend the
  record before code (the encoder inserts the conversion only in the four allowed places).
- **Tests** (twins in `negative/`): `enum_switch.c` (`case RED`), `enum_conv_cast.c` (`(enum color)x`, 3 is in range
  but not an enumerator) and its `conversion` twin, a negative-enumerator range, `enum_to_int.c`, `enum_eq.c`,
  enumerator arithmetic, `enum_contract.c` (precondition and postcondition twins), `enum_fixed.c` (`: unsigned char`,
  `conversion`, not a wrap), `enum_class.cpp`, `enum_loop.c`, `enum_bool_conv.c`, an enumerator initializer checked
  (`(signed char)300` → `conversion` at the enum line), unnamed and empty enums; rejections for each rejected shape.
  `native-ok`: switches cover every enumerator or have a `default`; out-of-range values non-constant.
- **Docs:** integer-semantics (an Enumerations section with examples; the Conversions table), verifying (the
  not-supported list; a trusted function returning an enum is assumed to return one of its values), overview,
  error-messages (`conversion` on enum targets).
- **Estimate** about 1 h 45 min of work. **Cut order** (each stays rejected with a diagnostic): scoped ordering
  comparisons, `enum class`, fixed bases, empty enums, enum-typed const globals. Never cut: the conversion check, the
  promotion rule, enumerator recognition, the initializer check.

**M4.2c Locals without an initializer** (record 0019; **run 2026-10-22**, see `internal/status.md`: the owner chose a
new category `uninitialized` checked path-precisely by a ghost flag, not definite assignment, and lifted jumps past such
declarations; §11 questions 17–19 ran in the same session). The planning text follows. Recommended (review): definite assignment over VIR — one
forward pass suffices because every jump is forward (intersection at joins and labels; a loop head's state is its
entry state); no new category; the rejection names the read. Alternative: a ghost "defined" flag per local with
category `uninitialized` (flags false at method entry, since Viper locals start arbitrary, `emit.cpp:181`; reads
include compound assignment, `++`/`--`, arguments, returns and annotations; a monotone loop invariant for locals
declared outside the loop; the automatic range invariant, `lower.cpp:676-691`, must not fail on an unassigned local);
decide by spike. Small: may share a session with M4.3's start.

**EDG box session after M4.2c** (owner, 2026-10-22). The owner merged `0022` into the fork's `cerastes`
(`f0ed5525af`, which also brings upstream `main` `ec2f12cffa`, 11 commits after `21065e551e`) and M4.2c moved the pin
there (`check.sh` green, every `.vpr` byte-identical, the three #2656 tests added). Still to do on the box: (1) the full
suites at `f0ed5525af` in both 64-bit configurations (`changes,regressions,imported,edg,cwg,stdlib`; known `cwg1835`,
`cwg2881`), and whether `0022`'s `.rto` needs `edg_x86_32` overrides; (2) §11 question 16: reproduce the C
enumerator-list typing on unmodified upstream and draft its bug file (no fork fix). Clean builds every run (memory:
stale binaries after a failed link). (3) **`#if`** (owner, 2026-10-23, M4.4b planning; §11 question 32): EDG folds a
preprocessing expression with no diagnostic for an unsigned wrap (`#if 0u - 1 > 0`), and in C23 its signed overflow is
#61, a warning verify mode suppresses for bodies; nothing reaches the IL (`preproc.c:460-491`). A fork commit adds a
discretionary diagnostic, distinct from #61, for a preprocessing expression whose arithmetic wraps or overflows, with
`GH-*` tests and the full box suites before the pin moves; the cerastes driver then makes it an error in verify mode.

**M4.3 Expressions and sequencing — split into M4.3a and M4.3b** (owner, 2026-10-22, M4.3 planning).

**M4.3a Side effects in expressions** (record 0020; **run 2026-10-22**, see `internal/status.md`). Assignments, compound
assignments and `++`/`--` used as values, chained assignment, the comma operator (and calls ordered by it), side effects
in guarded operands (lowered as branches) and loop conditions (the top-test loop shape); unsequenced modifications
rejected by C's rules in both modes; the `-Wall` shapes rejected like the compilers (the union of P35); the 0019 flag at
the write. Lowering: calls are the only fixed points, every other item is placed after the calls it depends on (audit
round 1's blocking finding).

**M4.3b Calls in expressions** (record 0021; **run 2026-10-23**, see `internal/status.md`; with it, cerastes's own termination check of a recursive call that another call may precede, `self_assignment`, and P37's correction of a discarded `&&`/`||`). The planning text follows (2026-10-22). Scope:
- calls in the right operand of `&&`/`||` and the arms of `?:`: inside M4.3a's branches (E5), each call a fixed point
  inside its branch; the branch is then a fixed point of the enclosing full expression;
- calls in loop conditions: M4.3a's top-test shape (a `while`/`for` condition lowered inside `while (true)`), and `do`
  conditions (already lowered in the body); 0014 item 5's "no call in `c`" lifted;
- **unsequenced calls by the dependency-chain rule** (owner, 2026-10-22): each check's set is the calls that must have
  returned before C evaluates it (the results it uses, its guard, M4.3a's floor, and, for an item that touches a local,
  the calls the earlier accesses to it depend on). When the sets of a full expression form a chain under ⊆, the calls
  are ordered by it and each item goes right after its set, so no callee that ends the program hides a check C may
  evaluate first; otherwise the expression is rejected (`100 / f(x) + 100 / g(y)`: each check depends on a different
  call). This generalizes M4.3a's placement (`src/vir/lower.cpp`, calls as fixed points) from one call order to the
  order the chain gives; `check_program`'s `CallState` rule (nested, or ordered by `,`) becomes the chain test.
- Ports: `fib(n - 1) + fib(n - 2)`, `max(f(a), f(b))`, `while (next(i) < n)`, `a > 0 && pos(a) > 1`.
- Retirements to approve at its planning: `reject_call_guarded.c`, `reject_call_cond.c`, `reject_call_loop_condition.c`,
  `reject_do_call_condition.c`, `reject_unsequenced_calls.c`, `reject_effect_call_guarded.c`,
  `reject_effect_call_loop_condition.c` (M4.3a's), and the VIR rejection cases `call_in_loop_condition`,
  `call_in_guarded_operand`, `call_in_conditional_branch`, `two_unnested_calls`, `sibling_calls_in_arguments`,
  `cf_call_in_do_condition` (`cf_call_in_do_once_condition` stays while `DoOnce` refuses calls).
- Mutation operators for calls: swap the operands of `&&`/`||` around a call; swap the arms of `?:`.
- Facts to probe first: none new in the IL (P33 covers the shapes); Silicon spikes of a call inside a branch and in a
  top-test condition (precondition and termination checks at the right line, the callee's `ensures` known after it).

**M4.4 — split into M4.4a, M4.4b, M4.4c, M4.4d** (owner, 2026-10-23, M4.4 planning). The owner answered §11 question 13
(no wrapping) and questions 27–31 together with M4.4. A Plan-agent review sized the work at four sessions of ≤ 4 h each,
each on its own branch, and the owner split it. The approved plan is `~/.claude/plans/using-plan-md-and-handoff-md-unified-honey.md`;
the essentials follow. Team in every session: edg-backend, il-analyst, test-mutator, and the read-only Opus auditor.

*Owner decisions for all four (2026-10-23):*
- **No wrapping.** It uses the existing categories (`overflow`, `conversion`, `shift`) and has no opt-in; the explicit
  way is a reduction in a wider type (`%`, then masks). In C, `==`/`!=` with an enumeration compares mathematically.
- **Expectation changes.** In M4.4a, any test that flips only because of a new `overflow`, `conversion` or `shift` error
  at a wrapping site is pre-approved:
  - a test whose point was the wrap is retired and replaced (a positive in range and an error twin at the old line);
  - an error twin, or a test sharing a wrapping fixture, has the wrap removed in its body in place (never an `expect:`
    line), with a new twin for the wrap.

  The retirements listed below are pre-approved too. Any other flip stops the work and the session asks.

**M4.4a No wrapping, and exit 3 for internal inconsistencies** (record 0022; §11 questions 13 and 29).
- **Probe P38 first** (the review's blocking item). EDG must keep `a_constant.expr` for every folded unsigned constant:
  - the constants `-1u`, `0u - 1u`, `4294967295u + 1u`, `65536u * 65536u`, `unsigned u = -1;`, and nested ones;
  - in bodies, contracts, named-constant initializers, enumerator initializers, case labels (C only for `case -1:`
    over `unsigned`; C++ refuses it, #2362) and default arguments.

  A place with no `expr` is rejected. The trust-EDG sites are `encode.cpp:783-789` and `:1046`.
- **Lowering.** The four wrap sites (`lower.cpp:489, 542, 584, 611`) become obligations; `wrap()` and `c_wrap` go. A
  conversion of an integer literal whose value fits needs no obligation.
- **Amendments.**
  - 0010 items 4, 5, 6, 8, 10, 11, 12, 14;
  - 0012 item 11; 0014 item 6; 0017; 0018 items 5–6; 0020 item 2;
  - 0021 item 3: unsigned parameters now create obligations, so a new `unsequenced_calls` is possible; that is a
    stop-and-ask flip;
  - 0011 items 8 and 13, for Q29.
- **Q29.** The back end sets an internal-inconsistency flag for lowering's errors, emit's problems and the
  "encoded twice" guard. `verify_main.cpp` checks it before line 424 (`edg_status != 0 → 2`) and returns 3. The hook
  `perturb_fixed_order` proves it end to end, with `--selftest` checks.
- **Known flips** (the review's reading):

  | Group | Files |
  |---|---|
  | 15 positives | `assign_value_unsigned`, `compare_mixed`, `const_global_init`, `conv_unsigned`, `incdec_uchar`, `incdec_value_unsigned`, `shift_unsigned`, `switch_unsigned_case`, `uchar_promote`, `uninit_compound`, `unsigned_mul`, `unsigned_neg`, `unsigned_wrap`, `usual_conversions`, `enum_list_type.cpp` |
  | 17 error twins | the `_post` twins of those, `assert_cast_kept`, `compound_ushort_over`, `const_global_init_conversion`, `const_global_init_post` |
  | 7 doc examples | `unsigned_wraps`, `constants`, `narrowing`, `shifts`, `promotion_ok`, `promotion_overflow`, `usual_conversions.c` |
  | VIR self-test | about 15 cases |

  The stop rule is more than 10 flips outside this list.
- **Byte identity.** A baseline `.vpr` without `c_wrap` is identical; the rest is compared ignoring the prelude.

**M4.4b — split into M4.4b1 and M4.4b2** (owner, 2026-10-23, M4.4b planning, with §11 questions 32–36). The approved
plan is `~/.claude/plans/using-plan-md-and-handoff-md-swift-hinton.md`; the essentials follow.

**M4.4b1 Bitwise operators, with §11 Q33 and Q36** (record 0023).
- **Probes and spike.**
  - P40, the IL (edg-backend): operand conversions of `& | ^ ~` and `&= |= ^=` (char, short, bool, mixed signedness,
    `__int128`), C's bare enumeration operand of `~`, folded bitwise constants and their `expr` in every place the
    encoder reads, where `is_parenthesized` sits after folding, and the C shapes of Q36.
  - P41, the warning matrix (test-mutator; clang and GCC, `-Wall -Wextra`, both modes, and EDG): the union of the warned
    shapes is rejected, at most 8 detector rules; the unrelated `-Wall` shapes are measured for a §11 question.
  - S21, the closed form (il-analyst): floor-division bits, `c_and_N(a, b) = Σₖ<N 2ᵏ·[bitₖ(a) ∧ bitₖ(b)] − (a < 0 ∧ b
    < 0 ? 2ᴺ : 0)` with `bitₖ(a) = (a \ 2ᵏ) % 2`, exact on [−2ᴺ, 2ᴺ) for signed and unsigned operands; `|` and `^`
    as prelude functions calling it; `~` inline and linear. Variants: independent bits, a let-chain, flat and nested.
    Budget: median < 2 s, maximum < 10 s. Bounded recursion is out (Silicon's limited unfolding would leave 0017's closed
    asserts undecided); 64-bit must be completed (owner); 128-bit may be cut.
- **Semantics.** Exact two's complement on mathematical integers, no obligations; `&= |= ^=` convert back with a check
  (`sc |= 0x80`, `uc = ~uc`, `u & -8`, `u &= ~FLAG` are `conversion`; unsigned `x & -x` is `overflow`, from the unary
  minus); enumeration operands rejected (0018's arithmetic rule); a `bool` target of a compound form stays rejected.
- **Code traps.** `assignment_parts`' operator chain ends in a bare `else` meaning `Shr` (`encode.cpp:1814-1820`): make
  it an exhaustive `switch`. `eval_const` treats unknown unary operators as the identity (`check.cpp:66-73`).
  `lower.cpp:495` returns the operand of an unhandled unary operator. `~` must not be built from `Neg` (`~INT_MIN` is
  `INT_MAX`). New VIR operators are appended at the end of their enumerations.
- **Q36** (amends 0022 item 5): a `truth_value` helper (comparisons, `&&`, `||`, `!`, `bool_cast`, `bool` nodes, a `?:`
  of truth values, a comma whose right operand is one; never an explicit cast or an int constant); for `==`/`!=` in C, a
  `?:` of enumerators of one enumeration is rejected with 0018 item 8's message (owner: the symmetric form).
- **Q33** (amends 0011 item 7): `--diag_error=1422,1654,26,3255`, with a sweep of every header the tests include.
- **Retired** (pre-approved): `reject_bit_and.c`, `reject_bit_or.c`, `reject_bit_xor.c`, `reject_bit_not.c`,
  `reject_bit_and_assign.c`.
- **Stop rules:** no closed form at 64 bits (only a bit-vector domain left, which needs the owner); a folded bitwise
  constant without `expr`; more than 8 detector rules; any other verdict change; an unexpected byte difference.
- **M5 note:** integral `T` still rejects bitwise operators; the per-instance fallback verifies them (`low_bit`).

**M4.4b2 Values the front end computes** (record 0024; §11 Q32, owner: "fix it now, and any other known instances";
**run 2026-10-26**, see `internal/status.md` for what landed: the walker change, census 2's enumeration-typed copies, the
skip's type rule, declared types through aliases, attributes on redeclarations). The planning text follows.
- **Spike first:** `GENERATE_SOURCE_SEQUENCE_LISTS` TRUE in the cerastes configuration (`cmake/EdgFrontEnd.cmake`, the
  `src/backend/edg_il.h` requirement, `scripts/check_edg_config.sh`; amends 0002). The walker goldens and every `.vpr`
  must stay byte-identical, or the session stops and asks. It also makes M4.2a audit note 1's check exact (§6).
- **`_Static_assert`/`static_assert`.** Block scope: the `stmk_decl` entity `iek_static_assertion`, skipped today
  (`encode.cpp:1574`); file and namespace scope: the source sequence entries of the primary file and user headers
  (`decls.c:15562-15595` records them only then). The condition keeps its `expr` (a bare literal has none); it is
  encoded as written and checked in 0017's prologue: the obligations at their lines, then an `internal` guard that it
  holds. A condition cerastes cannot encode is **skipped** (owner, 2026-10-26, §11 question 39): a closed list of
  triggers found by a pre-scan (a call, `enk_builtin_operation`, a concept-id, an entity of a system file); anything
  else goes through the encoder and its rejections. Block-scope assertions copy the case-value path (`case_value` flag,
  the C `named_var` and `variable_read_by` rejections) and are encoded by `expr` then `clause_bool`. Existing:
  `static_assert_ok.c` and `static_assert_overflow.c` keep their verdicts (their Viper changes).
- **Wrapped character literals** (`'\xff'` is -1, `L'\xffffffff'` an `int` -1; EDG gives no diagnostic, and a plain
  literal has no IL marker: `chk_char` is `chk_default`). Probe P42 first (plain, `L`, `u`, `U`, `u8`; in macros, case
  values, enumerator initializers, named constants, default arguments, contracts). A constant without `expr`, not of an
  enumeration type, with a negative value is a wrapped literal: encoded as the conversion of `v + 2ᴺ` into its character
  type, a path-precise `conversion` at the literal. The rule is by value, since a C character literal is `int`; N comes
  from `character_kind`. The enumerator gaps: `encode.cpp:1040` takes a literal initializer's value without a check, and
  `:1053` a C++ top-level `sizeof`. Truncated escapes (`'\x100'`) are discretionary EDG errors left as warnings in
  clang mode: `--diag_error` for them.
- **Array types in types.** `sizeof`/`_Alignof` of a type never inspects it (`sizeof(char[65536u * 65536u + 1])` is
  EDG's 1), nor do the types inside a `sizeof` expression; array typedefs are skipped silently (`encode.cpp:1576`,
  `3436-3440`). EDG keeps each bound's expression (`array.bound_constant`, il_def.h:10355). **Rejected until M4.5e**
  (owner, 2026-10-26), with `_BitInt(N)`, vector types and class-template instances with a non-type argument inside
  such operands (planning review; Q32's "other known instances").
- **Warning shapes in static assertions** (0023 item 5, audit round 1): once their conditions are encoded, the bitwise
  detectors run on them too (P41 found `~true` and `2 ^ 8` warned there).
- **Template arguments:** closed (P38: no `expr`, rejected); the M5 note stands. **`#if`:** the EDG box session.

**M4.4c Writes outside F, `void` `?:` arms, warnings in verified tests, `-Wall` shapes** (record 0025, was 0024; §11
questions 28, 30, 31, 37, 40; owner, 2026-10-26: one session, Q37 last with a cut order; finding A of the planning review
fixed first; §11 "M4.4c planning").
- **Q28.** Only writes and snapshots leave F; call-free branches hold checks and stay in F, slotted by prefix. A write
  goes right after the member of its set that comes last in π, with ties broken by creation order. That means changes
  to `lower.cpp` lines 1084-1093, 1117, 1128, 1187-1188 and 1225, and to `origin_of`. `diagnostics.def` drops "or
  assigns".
- **Q30.** A `void` `?:` in statement position becomes an `If`, built directly (so no `assignment_as_condition`), with
  each arm through `expression_statement`. That covers expression statements, discarded commas, and `for` steps
  (`check.cpp` `step_shape` widened to an `If` of steps).
- **Q31.** `frontend-warning` lines are allowed with `verified` and `error` outcomes. The pin is
  `discarded_and_assign_warn.c` (`a && (y = 2);`), with its twin.
- **Retired** (pre-approved): `reject_unsequenced_assignments.c`, `calls_rej_two_writes`,
  `reject_void_conditional_arms.c`, and the `unsupported_type` line of the four `reject_assert_shape_*.c` tests as
  measured.

**M4.4d Exact sets** (record 0026, was 0025; §11 question 27).
- **Measure first** with a prototype: exact precondition sets also move preconditions in pre-M4.3b programs
  (`f(a, g(b))`), and any expectation change other than the approved one stops the work.
- **Two steps:** exact sets (a precondition gets PRE(c), and a comma's value carries its floor), else today's sets. A
  rejection reports step 2's conflict.
- **The mechanism** (decided in the record): dual sets through `LV`/`Obl`/`Effect`/`Full::pre`/`last_*`/`floor_`/`since`,
  or lowering twice with a rollback.
- **Tests:** `h((validate(y), y))`, `100 / (validate(y), y)`, `(nr(0), 1) && 100 / d`, and `h(f(x), y) + 100 / g(z)`
  (accepted by the fallback). `reject_unsequenced_split_requires.c` is retired (pre-approved).

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
  them (owner, 2026-10-22, §11 question 21: "as early as possible", in M4.5; what is left goes to M7): how they attach
  (an attribute on a definition and on a pointer parameter), and how 0019's flag follows the cell. (5) With pointers,
  0020 item 4's sequencing rule must become a check on objects, not names (`*p = (*q)++` may alias): a path-precise
  category then (0020 item 4).
- **M4.5b Pointers to scalars and globals:** `&x`, `*p`, NULL, pointer parameters with `owns`/`reads`/`old(*p)`,
  pointer equality, an aliasing trap that must fail, mutable globals and `static` locals as cells with frames; ports
  of VeriFast's `examples/swap.c`, `examples/address_of_local.c`, `examples/globals.c`; keep pointer arithmetic out of C
  enumerator initializers or extend 0018 item 10's rule to it (§6, M4.2b2 note 1).
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

**M4.6 Chars and strings:** string literals as read-only arrays, `char` buffers, `strlen`/`memcpy`/`memcmp`/`strcmp`
contracts, `main(argc, argv)`; ports of `tutorial_solutions/strlen.c`, `memcpy.c`, `memcmp.c`.

**M4.7 Function pointers:** contracts on function types, calls through pointers; ports of `tutorial_solutions/byref.c`,
`examples/functions/functions.c`.

**M4.8 C parity close-out:** the VeriFast C differential set (the ports above, run with VeriFast in Docker as M5c
sets up, or recorded outcomes), the full mutation run over the M4 tests, the Memory and ownership page complete,
M4's acceptance (plan.md), and the deferred list for M8 (unions, floating point, `volatile`/`_Atomic`/`_BitInt`/
vector types, variadic functions and `printf`, several translation units, concurrency).

### 8.2 M5 — generic templates (designed in the M4.1 session)

**M5.0 Type traits** (owner, 2026-10-26, §11 question 39; `plan.md` "Before M5"). After M4.8 and before M5a. M4.4b2
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

**M5b EDG (box).** After the S1 upstream merge (§4.1); best just before M5g. Fork commit Q: a registration flag
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
  It happened again in M4.2a (the tooling commit took two staged retirements; redone at once): run `git rm` just
  before the commit that lands the feature, or commit with explicit paths.
- **M4.2a (2026-10-21):** asking the owner up front (pin, split, pre-approvals, categories, the meaning of "verified")
  and a Plan-agent review before execution cut the scope to what fits; the record's audit ran in parallel with the
  probes, and the implementation (VIR, encoder, 90 tests) took about 30 min. The first mutation run did not finish
  in its 20-minute cap: it overlapped full test runs and two auditors (load average 30), and `mutate.py` prints
  nothing until the end, so a capped run yields nothing. Run mutation alone on a quiet machine, and consider making
  `mutate.py` report as it goes (triage session).
- **Doc pitfalls (M4.2a):** `[[noreturn]]` in text becomes an AsciiDoc anchor even inside `+…+`, so it clashes with
  a section id `noreturn`; `__name` in backticks needs `+…+`; a possessive after a code span (`` `main`'s ``) breaks
  the next span. `build_docs.sh` and `check_doc_examples.py --html build/doc` catch all three.

## 10. Acceptance (commands and results)

| # | Command | Result |
|---|---|---|
| 1 | `scripts/check.sh` (at `610e7e1`, one run) | all green, 17 min 5 s: harness self-test 931 (1 skipped), oracle self-test 74, lint self-test 142, mutate self-test 174; tests 2624 passed, 0 failed, 1 skipped (native-ok: Apple clang 642, GCC 642); 667 `.vpr` lint clean; VIR self-test 230 lowering + 87 rejection cases, 0 problems, 230 passed, lint clean; oracle 134 goldens, 1915 facts, 0 failed; docs 0 problems (source and `--html`); `NOT YET PUSHED` for the pin, as expected |
| 2 | `python3 scripts/run_tests.py -j 8 tests/scalar doc/modules/ROOT/examples` (before check.sh) | 1594 passed, 0 failed (native-ok: Apple clang 335, GCC 335); baseline at the session start 1320 |
| 3 | `python3 scripts/run_tests.py --count-error-twins` | 140 (113 + 27 new positives) |
| 4 | `python3 scripts/run_tests.py --check-headers` | 954 / 954 ok |
| 5 | `diff -r -x MANIFEST build/m4.2a-baseline-vpr build/vpr` (the existing tests' Viper vs the session's baseline) | all 555 `.vpr` files (and maps) of the M3/M4.1 tests and doc examples byte-identical; none missing; also reproduced independently by the code auditor |
| 6 | `python3 scripts/mutate.py -j 8 <27 new positives + 4 doc examples>` (26 min, quiet machine) | 913 mutants: 476 killed, 220 bucket, 13 stillborn, 200 survivors, 4 tool failures (all in the replaced `goto_exit.c`; its successor: 30 mutants, 0 survivors); **185 survivors → §5**, classified, 0 suspicious |
| 7 | `python3 scripts/mutate.py --list tests/scalar` before/after the tooling change | the 2717 existing ids unchanged |
| 8 | audits (Opus) | 0015: 1 blocking (dropped from the design), 9 should-fix taken; code: 0 blocking, 2 should-fix taken; tooling re-audit: 0 blocking, 3 should-fix taken; notes §6 |
| 9 | wall-clock | 08:19–about 11:00 (planned ≤ 4 h) |

## 11. Questions for the owner

Each has a recommended answer. Answer inline, as before.

1. **The EDG pin.** `main`'s `518e0de` moved it to `0bcd9bf28a`; this branch moves it back to P `b9519388ea` (§2).
   Is P the pin you want (push the fork first)? *Recommended: yes.* **Answered (owner, 2026-10-21): yes; the fork's
   `cerastes` (P) and `main` are pushed.**
2. **Order after M4.2a:** CI (§4), then the mutation triage (§5), then M4.2b — or M4.2b first? *Recommended: CI first:
   it settles the upstream EDG merge and the pin before more code depends on it.* **Answered (owner, 2026-10-21): CI
   first, split into three sessions: EDG upstream merge (§4.1), CI (§4.2), mutation triage (§5), then M4.2b.**
3. **M4.2a's surviving mutants** (§5) go to the triage session with M4.1's, and `mutate.py` reports as it goes?
   *Recommended: yes.* **Answered (owner, 2026-10-21): yes; weak tests are strengthened in place (§5).**
4. **EDG's source sequence lists** (`GENERATE_SOURCE_SEQUENCE_LISTS`) would make "who wrote the noreturn flag" and
   "declared only in system files" exact (§6, M4.2a audit note 1). It is an EDG configuration change (decision 0002;
   IL layout and goldens). Evaluate it in M4.2b's planning (named constants also care where a declaration is)?
   *Recommended: yes, as a spike there; until then the attribute-entry check stands.* **Answered (owner, 2026-10-21,
   M4.2b planning): not in M4.2b** (named constants do not need it: a global first declared in a system file is
   rejected, and an initializer is checked by its original expression wherever it is); revisit for M4.5b or M5b.
5. **A `goto` into a block or an `if` branch** stays rejected (0015 item 2; stricter than C). Lift it only when a
   ported example needs it, after a Silicon spike with must-fail twins? *Recommended: yes.*

**S2 (CI), 2026-10-21:**

6. **Make CI a required check on `main`** (branch protection or a ruleset requiring `check (macOS, Command Line
   Tools 27.0)`)? *Recommended: yes, after the first green run on `main`; Dependabot's weekly pull requests then
   cannot merge red.* If you also enable a merge queue, `ci.yml` needs a `merge_group` trigger (and the guard's pinned
   `on:` with it). **Answered (owner, 2026-10-21): yes.**
7. **When the preview image changes its tools** (a new Command Line Tools, SDK build or Homebrew `gcc`), `ci.sh`
   fails by design. Then: move the assertion and 0001 together after checking the change on your Mac (same
   package), or wait for GitHub? *Recommended: move both, in a short session with a rehearsal; never loosen the
   assertion alone.* **Answered (owner, 2026-10-21): yes** (0016 item 2).
8. **Tighten `timeout-minutes`** (240 now) once two runs on `main` are green: max(150, 1.5 × measured)?
   *Recommended: yes; S2's runs give the first numbers (§4.2).* **Answered (owner, 2026-10-21): yes** (0016 item 9;
   the first session that sees two green runs on `main` does it, with the guard's bound).
9. **Tie each action SHA to 0001** in the guard (today any 40-hex commit passes; a commit that exists only in a fork
   resolves too, so review and Dependabot are the check)? *Recommended: not now; revisit if anyone but you and
   Dependabot edits the workflows.* **Answered (owner, 2026-10-21): not now.**

**S3 (mutation triage), 2026-10-21:**

10. **The 27 verified doc examples that have never been mutated** (all but the M4.2a four): mutate them in M4.8's
    mutation run, or in a short session before? *Recommended: M4.8, with the M4 tests; their survivors are listed or
    their pages' examples strengthened then.*
11. **Typed locals in the classifier:** a weaker `check` or `assert` of a local declared with a signed builtin type
    is listed today (about 20 entries, e.g. `assert_holds.c`, `check_stmt.c`, `goto_cleanup.c`). Teach the classifier
    to read local declarations so they are bucketed? *Recommended: not now; the entries are reviewed and cheap, and
    every new rule of the bucket needs its own audit.*
12. **A full mutation run in CI:** about 85 min at `-j 10` locally, far more on the 3-core runner. Keep it a
    per-session acceptance step, with `--validate` in `check.sh`? *Recommended: yes, keep it out of CI.*

**M4.2b planning, 2026-10-21:**

13. **Relook unsigned wrapping** (owner note, 2026-10-21, with the M4.2b2 answer that a conversion into a fixed
    unsigned enum is checked, not wrapped): *model integer semantics on real integers, not on how C and C++ support
    them.* Today 0010 makes unsigned arithmetic and conversions to unsigned types modular. A later session (M4.4's
    bitwise record, or its own) decides whether wrapping becomes an error by default, with an explicit way to ask for
    modular arithmetic. *Recommended: decide it in its own short session before M4.4, since bitwise operators on
    unsigned types depend on it.* **Answered (owner, 2026-10-23, M4.4 planning): "Wrapping should be prohibited."**
    Every integer operation whose mathematical result does not fit the type it is computed in is an error, unsigned
    too, in the existing categories:
    - unsigned `+ - *`, unary `-`, and `++`/`--` in `unsigned int` or wider give `overflow`;
    - a conversion that does not fit gives `conversion`, including `uc++` at 255, which is computed in `int`;
    - `u << n` losing bits gives `shift`.

    There is no modular opt-in: a reduction is written in a wider type (`%` now, masks from M4.4b), and a truncating
    marker is an M8 entry. In C, `==`/`!=` between an enumeration and a possibly negative `int` compares the values
    mathematically, which is exactly C's result, so both modes agree. It is done first, in **M4.4a** (record 0022).

**M4.2b1, 2026-10-21:**

14. **A `const` object in a C constant expression** (0017 item 2; the audit's S1, decided by the orchestrator to keep
    the session going). EDG accepts it with the discretionary warning #1901; M4.2b1 makes #1901 an error in C
    (`--diag_error=1901`), so C programs naming a `const` in a case label, a `_Static_assert` or another file-scope
    constant's initializer are refused (strict ISO C; clang and GCC accept `const int B = A + 1;` at file scope as an
    extension, GCC refuses the static assertion). The alternative keeps the warning suppressed and has cerastes reject
    only the contexts it cares about (case labels; initializers would then be accepted). Keep the strict rule?
    *Recommended: yes — one rule, portable C, and C programmers write such constants as enumerators (M4.2b2) or
    macros.* **Answered (owner, 2026-10-22): keep it strict** (so `enum { A = K };` naming a `const` object is refused
    in C too, 0018 item 2).
15. **C23 `constexpr` objects do not parse** in this EDG configuration (P23: warning #77, errors #65/#20), although
    C23 has them; with them, `case N:` would be valid C. Record it as an M8 inventory entry and check whether upstream
    EDG supports C23 `constexpr` (an option, or a later version), or treat it as an EDG issue to draft? *Recommended:
    an M8 entry now; check upstream's C23 status when the next upstream merge runs.* **Answered (owner, 2026-10-22): an
    M8 entry now** (`plan.md` M8, "Known entries").

**M4.2b2, 2026-10-22:**

16. **EDG's C enumerator-list typing.** Inside an enumerator list EDG's C front end types a reference to an earlier
    enumerator by its initializer's type (the C++ rule, `decl_spec.c:5930` under `gcc_mode`), while C23 6.7.3.3 makes it
    `int` when the value fits: `enum { U = 2u, V = U > -1 };` folds `V` to 0 (C gives 1; stock `cpfe --c23` fails
    `_Static_assert`). cerastes now rejects such a reference in C (0018 item 10). Reproduce it on unmodified upstream
    `main` and draft `internal/edg_bugs/NNNN-*.md` for you to file? *Recommended: yes, in the next EDG session (box);
    no fork fix, since it does not block cerastes (memory: EDG bugs scoped to cerastes).* **Answered (owner,
    2026-10-22): as recommended** (the next EDG box session, §8.1).
17. **EDG internal error #2656** (found by P27): in C++, an error #28 whose note verify mode suppresses makes the front
    end assert (`determine_severity_code: bad severity`): `enum { A = 1 << 32 };`, `constexpr int z = 1 << 32;`, so
    `cerastes verify` exits 3 (a tool failure, never a verdict). Investigate which suppressed number is the note and stop
    suppressing it (or suppress differently), and draft the EDG bug? *Recommended: a short item in the next session that
    touches the driver; tests avoid the shape until then.* **Answered (owner, 2026-10-22): done in M4.2c, with a fork
    fix branch.** The notes are #39, #61, #62 and #63 (P30). Making them remarks prints the error twice (a second EDG
    display bug), so the owner chose no driver change: the fix is branch `0022`, which the owner merged into the
    fork's `cerastes` and M4.2c pinned (`f0ed5525af`; 0019 item 9; `internal/edg_bugs/0022-suppressed-note-internal-error.md`).
18. **Ordering of unscoped enumeration values** (`c < GREEN`, both operands of one enumeration) is rejected (0018 item
    7) although it means the same in both languages for values in range. Lift it when a ported example needs it?
    *Recommended: yes, then; one rule and one test pair.* **Answered (owner, 2026-10-22): do it now** (it diverges
    from language conformance): two values of one unscoped enumeration, or a value and one of its enumerators, are
    ordered in M4.2c; an enumeration value against another integer stays rejected (0019 item 10).
19. **`#ifdef __CERASTES__` in a native-ok positive** (`tests/scalar/enum_case_promoted.c`): clang's and GCC's `-Wswitch`
    (in `-Wall`) refuse `case 4:` over an enumeration, so the never-taken cases are hidden from the native compilers and
    the header says why. Accept this pattern (and add a sentence to `tests/README.md`), or forbid it and keep such shapes
    out of native-ok positives? *Recommended: accept it, only for code the compilers refuse as a warning, with the reason
    in the header.* **Answered (owner, 2026-10-22): reject it like the compilers** ("it may be legal, but it is
    wrong"): both halves of `-Wswitch` are rejections, the strictest reading (g++'s) in both modes, and the
    `#ifdef __CERASTES__` pattern is forbidden (0019 item 11; `enum_case_promoted.c` and four twins retired).

**M4.2c, 2026-10-22:**

20. **Warnings #550 ("set but never used") and #177 ("declared but never referenced")** print in verify mode for natural
    code with a local declared without an initializer (`int x; x = 1; return n;`) and fail a test (tests are
    warning-free). Suppress them in verify mode? *Recommended: not now; they are true and useful to a user, and tests
    read their variables.* **Answered (owner, 2026-10-22): follow the recommendation** (#550 and #177 stay).
21. **P4222's markers.** cerastes asks for no `[[uninit]]` on a definition left uninitialized (0019 item 3). Add a
    stricter mode that also requires P4222R1's markers (`[[uninit]]`, `[[ref_to_uninit]]`)? *Recommended: an M8
    inventory entry; revisit with M4.5 (pointers to uninitialized objects) and M7 (member initialization).*
    **Answered (owner, 2026-10-22): "We should handle these as early as possible so in M4.5 and then whatever is left add
    to M7."** (plan.md M4.5a and M7, §8.1.)
22. **A jump past a declaration *with* an initializer in C** (allowed in C, the object is indeterminate at the label;
    C++ refuses it): with 0019's flag it could be accepted in C. Lift it? *Recommended: no, until a ported example needs
    it; one program for both modes.* **Answered (owner, 2026-10-22): "This should be refused in both modes."** (0019
    item 6, amended by 0020.)
23. **EDG's double display of an overridden note** (`--diag_remark`/`--diag_warning`/`--diag_error` on a note's number
    prints the whole error twice; described in bug 0022). Draft it as its own bug? *Recommended: no (EDG bugs scoped to
    cerastes); 0022 removes cerastes's need for an override.* **Answered (owner, 2026-10-22): no.**


**M4.3a, 2026-10-22:**

24. **`-Wself-assign`** (clang, both modes; GCC is silent) warns on `x = x;`, which cerastes has accepted since M3 (0012
    item 3), and on the new value form `y = (x = x)` (accepted; audit round 2). The union rule of 0020 item 5 would reject
    both. Reject them (a small rule; no existing test uses the shape), and when? *Recommended: yes, in M4.3b, with a
    rejection test per form.* **Answered (owner, 2026-10-23): yes, in M4.3b, with a rejection test per form.**
25. **The `&&`/`||` asymmetry of rule C** (0020 item 5). By the union, a discarded `a && (x = 1);` is accepted (all four
    configurations are silent; EDG prints its #187) while `a && (x = b);` is rejected (gcc C warns: it folds `(x = b) != 0`
    away only for a constant). Keep the union exactly, or reject every discarded `&&`/`||` whose right operand has a side
    effect (one rule, stricter than every compiler on the constant form; `if (a) x = 1;` says the same)? *Recommended:
    keep the union; it is your rule, and the constant form already prints EDG's warning.* **Answered (owner, 2026-10-23):
    keep the union.**
26. **Sequencing with pointers.** 0020 item 4 checks names, which is exact while every modified object is a named local.
    With pointers (M4.5), `*p = (*q)++` is undefined only when `p` and `q` alias, so the check must become a check on
    objects, decided per path: a new category (`unsequenced`) in the manner of `uninitialized`. *Recommended: yes, as
    part of M4.5a's memory-model record.* **Answered (owner, 2026-10-23): yes, as part of M4.5a's memory model** (§8.1
    M4.5a item (5)).

**M4.3b, 2026-10-23:**

27. **Exact sets for preconditions and commas** (§6 M4.3b note 2). The sets are smaller than "what C completes first",
    which is sound but gives false alarms on `h((validate(y), y))`, a C idiom. Make them exact (the Viper of M4.3a-era
    programs with these shapes changes; no expectation does)? *Recommended: not now; when a ported example meets it.*
    **Answered (owner, 2026-10-23): "Do it now since we know it is correct."** The M4.4 planning found that exact sets
    also reject shapes accepted today (`h(f(x), y) + 100 / g(z)`, where `h`'s `requires` reads only `y`), so the owner
    chose two steps: exact sets first, today's (sound) sets if F is not a chain, a rejection only if both fail. It is
    **M4.4d** (record 0025; `reject_unsequenced_split_requires.c` retired, pre-approved).
28. **Writes in the chain** (§6 note 3). Let writes and snapshots, which cannot fail, be placed after the last fixed
    point of their set instead of entering F, so `(x = f(a)) + (y = g(b))` is accepted? *Recommended: not now; one rule
    is simpler to audit; revisit when a port needs it.* **Answered (owner, 2026-10-23): "Do it now since we know it is
    correct."** It is **M4.4c** (record 0024). Only writes and snapshots leave F; call-free branches hold checks and
    stay. `reject_unsequenced_assignments.c` and the VIR case `calls_rej_two_writes` are retired (pre-approved).
29. **Exit status of lowering's internal errors** (§6 note 6): route them to exit 3, decision 0011's "tool failure, never
    a verdict"? *Recommended: yes, a short item in the next session that touches the driver (its own report path and a
    `--selftest` check).* **Answered (owner, 2026-10-23): follow the recommendation.** It is in **M4.4a** (0011 items 8
    and 13 amended).
30. **`void` arms of `?:`** (`c ? v(x) : w(y);`, accepted by every compiler, rejected as `unsupported_type`): an M8
    entry, or support now? *Recommended: an M8 entry; `if (c) v(x); else w(y);` says the same.* **Answered (owner,
    2026-10-23): "Do it now since we know it is correct."** It is **M4.4c** (record 0024): a `void` `?:` in statement
    position is C's `if`. `reject_void_conditional_arms.c` is retired, and the `unsupported_type` line of the four
    `reject_assert_shape_*.c` tests changes as measured (pre-approved). A `void` `?:` as the left operand of a comma
    whose value is used, and C++'s `return c ? v() : w();`, stay rejected (M8).
31. **Warnings in verified tests** (§6 note 7): let a `verified` test list its expected front-end warnings
    (`frontend-warning`, as a `rejected` test may), so accepted shapes that print EDG's #187 can be pinned?
    *Recommended: yes, a small harness item with twins in the next session that touches the harness.* **Answered
    (owner, 2026-10-23): follow the recommendation.** It is in **M4.4c**: `error` tests may list the same warnings,
    since a positive's twin prints them too.

**M4.4a, 2026-10-23:**

32. **`_Static_assert` and other values EDG computes alone still wrap** (§6 M4.4a note 1):
    `_Static_assert(65536u * 65536u == 0u)` is accepted. A closed `_Static_assert` condition could be encoded as written
    and checked in the 0017 prologue, like a named constant's initializer. Do it, and when? *Recommended: yes, a small
    item in M4.4b (its record already touches constants); `#if` and array bounds stay EDG's (an M8 entry).*
    **Answered (owner, 2026-10-23): "Fix it now, and any other known instances of the same bad behavior."** The M4.4b
    planning split M4.4b (owner): this is **M4.4b2** (record 0024): `_Static_assert` at block and file scope (file scope
    through `GENERATE_SOURCE_SEQUENCE_LISTS`, owner, after a spike), wrapped character literals, array types inside
    `sizeof` and typedefs; `#if` needs an EDG fork commit, drafted for the EDG box session (owner); template arguments are
    closed already (P38). §8.1 has the plan.
33. **Multi-character literals** (`'abcde'`) are truncated by EDG with only warning #1654, which verify mode does not
    suppress, so a verified test cannot hold one. Make #1654 an error in verify mode (`--diag_error=1654`), as C23
    makes them implementation-defined? *Recommended: yes, a one-line driver item with a test, in the next session that
    touches the driver.*
    **Answered (owner, 2026-10-23): "Yes, and do it now."** In the M4.4b planning the owner extended it to the same
    loss of value in EDG's other character-literal warnings: `--diag_error=1422,1654,26,3255` (`'ab'`, `'abcde'`, `L'ab'`,
    a truncated UTF-16 value). It is in **M4.4b1** (record 0023).
34. **Item 5 rejects two shapes the compilers accept**: `x == (enum color)2`, which in C would otherwise be a
    `conversion` error in C only, and `c == (x > 0 ? 1 : 2)`. It also rejects `c == (int)b`, where only clang warns.
    Keep the rule as it is (one meaning in both modes, the strictest reading)? *Recommended: keep it; each has an
    explicit form that verifies (`(int)`).*
    **Answered (owner, 2026-10-23): keep it.**
35. **Sign-compare shapes without enumerations.** `n == 0u`, `i < u` and `-1 < 0u` are path-precise `conversion` errors
    now (0022 item 1), where the compilers warn `-Wsign-compare`, in C++ `-Wall` and in C `-Wextra`, on some of them. Keep
    the path-precise check, or reject the warned shapes like the compilers as for enumerations? *Recommended: keep the
    path-precise check; it reports only what can actually go wrong, and a warned shape that is in range verifies.*
    **Answered (owner, 2026-10-23): keep the path-precise check.**
36. **Item 5's remaining mode differences** (re-audit 2b; sound, but not one verdict in both modes). A `?:` or comma
    whose result is a truth value is typed `int` in C and `bool` in C++, so `c == (x ? true : false)` is rejected in C
    only. A `?:` of enumerators is typed `int` in C and the enumeration in C++, so `(x ? RED : GREEN) == y` is rejected
    in C++ only. Extend the rule, so that a `?:` whose arms are truth values, or a comma whose right operand is one,
    counts as a truth value, and in C a `?:` of enumerators of one enumeration counts as a value of it? *Recommended:
    yes, as a small item in M4.4b, with a test pair per shape.*
    **Answered (owner, 2026-10-23): follow the recommendation.** In the M4.4b planning the owner chose the symmetric
    form for C (a Plan-agent review found that counting a C `?:` of enumerators as a value of its enumeration would newly
    accept `c == (x ? RED : GREEN)` in C while C++ rejects it, 0018 item 8): for `==`/`!=`, a C `?:` whose arms are
    enumerators of one enumeration is rejected with 0018 item 8's message whatever the other operand is. In **M4.4b1**
    (record 0023).

**M4.4b planning, 2026-10-23** (owner, with the answers above):
- **Split.** M4.4b is two sessions: **M4.4b1** bitwise operators with Q33 and Q36 (record 0023), then **M4.4b2** Q32's
  values the front end computes (record 0024). M4.4c and M4.4d become records **0025** and **0026**.
- **File-scope `_Static_assert`** (no IL record without source sequence lists): turn on `GENERATE_SOURCE_SEQUENCE_LISTS`
  in the cerastes configuration, after a spike (M4.4b2).
- **`#if`** (no IL record, no EDG diagnostic for an unsigned wrap; C23's #61 there is suppressed): an EDG fork commit,
  drafted for the EDG box session (§8.1).
- **64-bit bitwise** is not a cut: if the first closed forms miss the time budget, M4.4b1 keeps working until 64-bit is
  complete (128-bit may still be cut, with a test and an M8 entry).
- **Warning shapes the IL cannot match exactly** (a literal's spelling, as in clang's `2 ^ 32`): reject the broader shape,
  stricter than the compilers, each one listed.
- **Other `-Wall` shapes** met by the P41 matrix (`a && b || c`, `!x == y`, `x == y == z`, …): measured in M4.4b1, then
  a §11 question.

**M4.4b1, 2026-10-24:**

37. **Other `-Wall` shapes cerastes accepts** (P41 group 11 and audit round 1; of these only `(p ^ q) == 2` involves a
    bitwise operator):
    - all four compilers warn: `a && b || c`, `a || b && c`, `a + b << c`, `a << b + c`, `!x == y`, `x == x`, `x != x`,
      `x > x`, `if (a << b)`, `a << b ? 1 : 2`;
    - clang warns by default on comparing a truth value with a constant outside 0–1 (`(a > b) == 2`, `(p ^ q) == 2`);
      `x < y < z` is a clang error by default;
    - GCC only (`-Wall`): `x == y == z`, `a < b == 1`, `if (a * b)`;
    - `RED + NA` (two enumerations in arithmetic; C++20's default `-Wdeprecated-enum-enum-conversion`; record 0023
      rejects the bitwise form);
    - GCC's `-Wextra`-only `-Wtype-limits`: `u >= 0`, `u < 0`, `(uc & vc) == 300` (EDG warns #186 on the first two).

    Reject the `-Wall` ones like the compilers (0020 item 5's union), and when? *Recommended: yes, in M4.4c (which
    already touches the warning rules' harness, question 31), a rejection test per shape; the `-Wextra`-only
    `-Wtype-limits` shapes stay accepted, outside the `-Wall` union.*
    **Answered (owner, 2026-10-26): "Yes, follow recommendation."** In **M4.4c** (§8.1), a rejection test per shape.
38. **Gate G2's `low_bit` and symbolic bit identities.** The prelude decides ranges, same-term results, constant masks
    and constants, but not `x & -x`'s bounds, `(u & m) <= m` for a symbolic `m`, the XOR swap or popcount (record 0023
    item 2); such a run ends at `--timeout` with exit 3. G2 (M5) needs `low_bit`. Options: more postconditions proved by
    Silicon on the prelude functions (for example `0 <= b ==> c_and(a, b) <= b`), measured as in this session; Silicon's
    `--assertTimeout` (errors instead of timeouts, incomplete); an SMT bit-vector domain (axioms, a lint tag). *Recommended:
    the first, spiked in M5d's planning (S22), since a postcondition is proved and needs no new trust.*
    **Answered (owner, 2026-10-26): "Follow recommendation."** Spike S22 in M5d's planning (§8.2).
39. **M4.4b2: a static assertion whose condition cerastes cannot encode** (a call of a `constexpr` function, a type
    trait, a name from a system header): reject it (fail closed, invariant 3), or skip it (EDG checked it, and nothing
    at run time depends on it)? *Recommended: reject it in verified files, as named-constant initializers are (0017),
    with an M8 entry for C++ type traits.*
    **Answered (owner, 2026-10-26): "Skip it for now, but we need to address type traits before we get into concepts.
    Add before M5."** M4.4b2 skips such an assertion (record 0024, a closed list of triggers); a new session **M5.0
    Type traits** comes before M5a (`plan.md`, §8.2).

**M4.4b2 planning, 2026-10-26** (owner, with the answers above):
- **Array types inside types** (`sizeof`/`_Alignof`/`typeof`/`decltype` operands, typedefs) are **rejected until
  M4.5e**, not checked in the prologue (record 0024). The planning review found further layouts EDG computes from a
  constant inside such operands (`_BitInt(N)`, vector types, `std::array<char, N>`), taken under Q32's "any other
  known instances".

**M4.4b2, 2026-10-26:**

40. **The static-assertion cross-check inside functions** (§6 M4.4b2 note 1). Today the file scope is cross-checked, and
    inside functions the statement walk of each encoded body is relied on. Compare the assertions EDG's walk finds in
    function regions with those the encoder saw, with a self-test hook to reach the rejection? *Recommended: yes, a small
    item in M4.4c (it touches `encode.cpp` anyway); it adds a check, not a meaning.*
    **Answered (owner, 2026-10-26): follow the recommendation.** In **M4.4c** (record 0025).
41. **A note for a skipped static assertion.** Q39's skip is silent; a user may believe a skipped condition was checked.
    Print a note (`cerastes verify` has no warning channel today; a verified test fails on any warning until M4.4c's
    question 31 lands), or stay silent until M5.0? *Recommended: silent until M5.0, which removes most skips; the
    Verifying page documents them.*
    **Answered (owner, 2026-10-26): follow the recommendation.**
42. **`sizeof("abc")` and other string operands** are rejected until M4.6 by the array rule (record 0024 item 6): their
    size comes from the string, not from a wrapped constant. Lift them in M4.6 with strings, or exempt an array type
    without a bound constant now? *Recommended: M4.6; one rule is simpler to audit, and no test uses them.*
    **Answered (owner, 2026-10-26): follow the recommendation.**

**M4.4c planning, 2026-10-26** (owner, with the answers above; the approved plan is
`~/.claude/plans/using-plan-md-and-handoff-md-wild-rainbow.md`):
- **One session**, with Q37 last and a cut order (a cut rule stays accepted and becomes a §11 question). Q40 joins it.
- **Finding A** (the planning review): an `uninitialized` check inside a guarded operand could be placed after a later
  write of the same local (`int x; int y = (x = (g(f(a) > 0 && x > 0), 2));` verified). Fixed in M4.4c, after a probe.
- **Q28's Viper:** measured changes are allowed for programs with a write or snapshot of a call result (the order π may
  change), listed in the status entry; any verdict or error-line change stops the work.
- **A flagged snapshot's `uninitialized` check stays in F** (it is a check): `int x, y; … (x = f(a), x) + (y = g(b), y)`
  stays rejected, pinned by a test.
- **`x ? RED : NA`** (two enumerations as the arms of `?:`, warned by default by both C++ compilers) is rejected with
  Q37's shapes.
- **The cleanup:** at the end of M4.4c, `internal/status.md`, this file and `plan.md` are trimmed to what is live; a line
  in each names the commit that holds the full text.

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
