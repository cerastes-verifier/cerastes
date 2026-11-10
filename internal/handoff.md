# Handoff at M4.5d2b2b's planning — owning fields planned (2026-11-09); M4.5d2b2b's Step 0 next

The next session reads this file after `CLAUDE.md`, its own section in `plan.md`, `internal/status.md` and
`internal/decisions/`. `status.md` has the formal entry. This file adds what a cold start would otherwise have to
rediscover, and holds only what is live (owner, 2026-10-26).

It was consolidated at M4.5d2b2b's planning (2026-11-09, at the owner's request). The history of finished sessions,
the notes that had moved into session plans and the digests of answered questions were cut, and the open notes were
re-filed under the session that consumes each. **The text before this consolidation is
`git show 2430529:internal/handoff.md`**, whose header names every earlier version. Each note below keeps its origin
in brackets, so its full wording can be found there. Session plans approved from M4.5b1a on are in `.claude/plans/`;
the notes a session consumes move into its plan's "Inputs".

## 1. State

- **M4.5d2b2a** (2026-11-09; record 0054; plan `.claude/plans/m4-5d2b2a-owners-in-memory.md`) is on `origin/main`
  (`8302d24`; pushed without a PR). Its result is in `internal/status.md`. Its worktree and `build/m4.5d2b2a/` are
  gone, and with them `vprdiff.py`, `regen.py` and S39's summary (`build/m4.5d2b2a/s39/README.md`, which record 0054
  cites). The kept S39 fixtures are `tests/viper-smoke/memory/s39_*`.
- **Its one full `check.sh` is on the x86-64 box** and had not finished at this planning. It started 2026-11-10 00:13
  UTC in S8's `cerastes-s8-ubuntu-26.04` container, on `b6b3298`, which differs from `origin/main` only in
  `scripts/test_run_tests.py` and `tests/README.md` (the 360-minute timeout text). At 01:07 UTC its log had 11,779
  `PASS` lines and no `FAIL` line, and no `.exit` file yet. The result is
  `~/cerastes-s8/p-ubuntu-26.04/d2b2a-check.{exit,log}` (§2).
- **M4.5d2b2b is planned** (2026-11-09, a session that only planned; plan
  `.claude/plans/m4-5d2b2b-owning-fields.md`; questions 188–193): owning fields, complete, with the `[[owner]]`
  mapping. **Its Step 0 is next.** M4.5d2b2 is three sessions now (question 188): d2b2a is done, d2b2b is owning
  fields, and **d2b2c** takes `not_invalidating`, `--strict-owner`, marked fields and d2b2b's cuts (§3 "M4.5d2b2c").
- **CI** runs toolchain profiles (`scripts/ci_toolchains.toml`; record 0053): the references `macos-27` and
  `ubuntu-26.04` on every event, other toolchains nightly or on dispatch. The nightly full check was cancelled at its
  220-minute timeout every night from 2026-10-29 to 2026-11-09. The owner raised both full-check jobs to
  `timeout-minutes: 360` (`99b587e`; the guard's bound and record 0016 item 9 follow in `8302d24`). The push run on
  `86b9b88` failed in both `check` jobs on the guard self-test, which still expected 220; `8302d24`'s run was past
  that point and still going at this planning. The `profiles=all` dispatch has never run (§2).
- **The Linux host** `matt@clang4.cpp.al` cannot bootstrap natively (no root; no libc++ 21, JDK 17, GCC 16 there).
  Runs go in containers built from clean `ubuntu` images: `~/cerastes-s8` holds a base checkout, one checkout and one
  container `cerastes-s8-<profile>` per profile, and `run.sh setup|update|check <profile>` (§6). Its
  `p-ubuntu-26.04` checkout is detached at `b6b3298` (from `~/cerastes-d2b2a.bundle`).
- **vlibc** `6e41fbe` (the pin), **vlibm** `1e6982e` and the website (`cerastes-verifier.github.io` `45a3d2f`) are
  level with their `origin/main`.
- **The EDG pin** is unchanged (`f0ed5525af`); the fork's `cerastes` is 29 commits behind upstream `main` (§4.1).

## 2. Owner steps

Only what is pending. The steps of earlier sessions that are done (the d2b2a push, PRs #27–#33, vlibc's, vlibm's and
the website's pushes) were checked at M4.5d2b2b's planning and dropped.

1. **Push `main`**: this planning's two commits (`2430529`, the owner's answers 169–172 as written, and the planning
   commit). Markdown only.
2. **Read the box's full `check.sh` on `b6b3298`**: `cat ~/cerastes-s8/p-ubuntu-26.04/d2b2a-check.exit` (0 is green)
   and the summary lines of `d2b2a-check.log` beside it. If it is red, M4.5d2b2b fixes it first, with targeted checks
   (its Step 0).
3. **Remove the audit worktree** `.claude/worktrees/m4.5d2b2a-auditA` (`git worktree remove --force`). Its snapshot
   commits are not on any branch, so nothing is lost.
4. **CI** (question 176: the owner handles CI).
   - `8302d24`'s push run should be green in both `check` jobs.
   - The first nightly under the 360-minute limit shows whether the full check fits in six hours.
   - Dispatch the workflow once with `profiles` = `all`. Every Linux profile was rehearsed on the box, but `macos-26`
     has never run. M4.5d2b2b promotes it to `nightly` if its job is green, or records what differs (question 172).
5. **Your Mac's Command Line Tools** (question 171): 27.0 at S8, the image's 27.2 since `20261104.0337`. Nothing in
   the repository changes when you update; the Mac's next baseline is then on 27.2.
6. **Optional.** `git config blame.ignoreRevsFile .git-blame-ignore-revs` locally (GitHub reads the file on its
   own). On the box, `~/cerastes-m4.5d1` and its container `cerastes-m4.5d1-p71` (idle since 2026-11-04) can go
   (`docker rm -f`, `rm -rf`); `~/cerastes-s8` stays, since it runs each session's `check.sh`.

## 3. Open notes, by the session that consumes them

One line per note, its origin in brackets (session and audit item); the full wording is in the commit named above.
Notes that a session consumes move into its plan's "Inputs".

**M4.5d2b2b owning fields:** its notes are in its plan's Inputs (`.claude/plans/m4-5d2b2b-owning-fields.md`).

**M4.5d2b2c `not_invalidating`, `--strict-owner`, marked fields** (question 188; after d2b2b, before M4.5e1; the
spellings were decided at d2b2a's planning, questions 175 and 177–179; sources: P4222R3, P4416R0, P4418R0, P4417R0,
all 2026-10-22; Stroustrup, "An invalidation profile", https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4418r0,
wording in P4417R0)
- P4417R0's `[[owner]]` on a parameter is a **transfer** (the callee must delete), where 0029 item 1 made
  `cerastes::owning` on a parameter a **borrow**; P4418 exempts static owners where 0029 item 1 has an owning global freed
  by the end of `main`. `[[not_invalidating]]` (a pointer parameter whose function does not invalidate, checked at the
  definition) and `no_dangling()` (a suppression, `std::forward` in P4417 8.4) are what d2b spells in `cerastes::`;
  P4417 keeps raw storage (`malloc`) out of the invalidation profile. C applies: raw pointers, structs with pointer
  members as containers, the escape rules, `[[owner]]` on variables, parameters, fields and results, `not_invalidating`
  on pointer parameters; references, iterators, smart pointers and `new`/`delete` are M7's.
- P4417R0's wording, read at d2b1's planning ([profiles.inval.owner]): p3 owner-to-owner transfers and leaves an alias,
  owner-to-non-owner aliases, an owner parameter takes the object, an owner result gives it; p4 an allocation's (there:
  `new`'s) or a call's owning result must initialize or be assigned to an owner, or be deleted, in its full-expression;
  p5 an automatic owner deletes or transfers on every path to its block's end (a static owner need not, Note 3); p6
  only an owner is deleted; p7 every alias of a transferred object is invalid. §8.2's drafting note keeps raw storage
  (`allocator::deallocate`) out of the profile. `not_invalidating` (9.13.u) appertains to a pointer parameter or a
  member function and is verified at the definition ([profiles.inval.verify]); `std::no_dangling(x)` returns
  `std::forward<T>(x)` and waives the escape rules for its result. d2b1 gives owning automatic locals 0029's meaning
  (a path-precise `leak`), which agrees with p5. [d2b1 planning]
- **`cerastes::not_invalidating`** (question 178; the planning survey measured 290–440 `src/` lines).
  - **Registration:** `{"not_invalidating", "", "p", RAF_NO_FLAGS}`, as `must_init` is. Target `p` alone with
    `RAF_ENTITY_ATTACHED` aborts EDG at startup (`register_attributes.cpp:76`); P74 (c) probed `r|p`. Macro
    `CERASTES_NOT_INVALIDATING` (`include/cerastes.h` 54–59 and 68–73).
  - **The name lists to extend:** `annotation_il.h` 31–47 and 64–70; `annotation_checks.cpp` 1141–1150 (the early
    return for markers) and 1632–1661 (the protected macro names); `encode.cpp`'s `marker_name` (1299),
    `is_param_marker` (4847), `check_signature` (1226–1238) and `read_marker` (1318–1691; its `const` branch at
    1545–1560); `annotations.adoc` 305–335 and 1120–1122.
  - **The check on the contract,** with or without a body, after `finish_atoms` (`encode.cpp` 6649–6995), for each
    marked `p`: every `requires` atom rooted at `p` (`atom_place(…).root`, any `via` and `path`) has an `ensures`
    atom with its `amount`; every `requires` block site rooted at `p` has an `ensures` site on the same place. The
    nearest existing static check per parameter is `restrict_backed` (`vir.cpp` 1593–1698).
  - **"Taken implies returned on every path" is new.** `f__wf` (`wf_method`, `lower.cpp` 15870–16040) asserts only
    "returned implies taken" (15902–15952). Where a pair's guards are the same null-check shape
    (`vir::atom_guard_shape`) the rule is syntactic; any other guard needs a new row `prior ==> (g_req ==> g_ens)`.
  - **The implicit `ensures`,** built after that check so that it is framed in both states: for each block site
    rooted at `p` whose place loads (the closures of owning fields included, record 0055 item 3), `E == old(E)` for
    the site's pointer `E`, under the site's guards. With a body it is a `postcondition` at the return. Without one
    it is 0054 item 2's tie (`lower.cpp` 15988–16002 asks the user to "write `base == old(base)`"), so a bodiless
    borrow of a structure with an owning field no longer writes it.
  - **Rejected:** beside `ref_to_uninit` or `must_init` on the same parameter (`reject_marked_read`, `encode.cpp`
    1853–1885, refuses the `old(*p)` it needs); on a function or member function (M7). [d2b2b planning survey]
- **`--strict-owner`** (question 177; the survey measured 200–280 lines, almost all in `encode.cpp`).
  - **The pattern is `--strict-init`'s,** end to end: the driver (`verify_main.cpp` 71–73, 87, 220–223, 633),
    `verify_config.h` 32–34, `diagnostics.def` 110–119, and in `encode.cpp` the rule bodies at 7404–7493 and
    7801–7928 (`allow_strict_result` 7803, `check_marked_return` 7870, `settle_strict_init` 7471), `call()`
    7384–7393, `allocation_view` 10875–10881 and `reject_strict_uninit_result` 10893–10906.
  - **p4.** A call whose contract gives its result's block (an `owning` function, `malloc`, `calloc`, vlibc's
    allocations) must be the initializer of, or the value assigned to, an owner (an `owning` local, global, `static`
    local or field); the operand of a `return` in a function that gives its result's block; or the argument of a
    parameter whose contract takes the block (`free(make())`).
  - **p6.** The argument of a parameter whose contract takes a block and does not return it (`free`, `vlibc_free`, a
    user's destructor) must be an owner by name (`v`, `g`, `s.q`, `p->q`); a parameter of the enclosing function
    whose own contract takes its block (P4417's owner parameter, question 175); or a p4 call.
  - **Two encoder-side marks do not exist yet.** "This function gives its result's block": `vir::Function` has only
    `result_init` and `deallocates`, and today the fact is an `ensures` `OwnsBlock` at `kResultSiteKey` (`encode.cpp`
    1973–1985). "This parameter's contract consumes its block": `block_consumes` (`lower.cpp` 3712) is lowering's.
  - **The record lists what the static rule cannot see,** as 0042 item 8 does.
  - **Harness and docs:** `VERIFY_OPTIONS_ALLOWED` (`run_tests.py:309`) changes with
    `test_the_allow_list_holds_only_strict_init` (`test_run_tests.py` ~13359); every new tag in `diagnostics.def`
    needs a test that names it (the tag-coverage self-test, ~15574); `verifying.adoc` 62–65 and beside
    `[#strict-init]` (400), `dev/architecture.adoc:123`, `tests/README.md` 98–101 and 338–361, `memory.adoc`
    1220–1226. [d2b2b planning survey]
- **`ref_to_uninit` on a field** (question 192): target `d` for `ref_to_uninit`. A marked field's closure is a
  marked block (flags unknown, as an owning and marked global's deep atom is in 0054 item 2, "The mode"). Under
  `--strict-init` a store to a marked field follows 0052 item 4's rule for a marked local, and the pin
  `tests/memory/negative/reject_strict_init_malloc_member.c` flips. d2b2b pins EDG's #1835 on such a field, naming
  d2b2c. [d2b2b planning]
- **d2b2b's cuts 1 and 2, if taken** (its plan, "Pre-agreed cuts"): loops over the owning leaves of struct locals and
  globals; the dropped-block rows for an owning field reached through a pointer. Cut 3 (chains) names M4.5f2.
  [d2b2b planning]
- **Pins.** No P4418R0 attribute has a pin of its own, though 0054 item 10 promised one each. `not_invalidating` on
  a function, and `no_dangling` (an unknown name in the closed namespace; question 179), get theirs here.
  [d2b2b planning survey]

**M4.5e1/e2 Arrays and pointer arithmetic:** P38's braced initializers (§4.2); signed/unsigned and enumeration variants
through `void*` cannot be written [M4.5a]; accepted completeness gap for e2's relational comparisons: `&s.x < &s.y` is
reported, since members are different blocks [M4.5a].
- **For M4.5e1 (arrays):** the `held_v`/`held_g` updates and `main`'s exemption need `offset_of == 0` once interior
  pointers exist (audit round 1, note 20), and the flag clause of item 3(e) is restated on `hb_w`'s cell. [d2b2a]
- **For M4.5e2 / later:** `vlibc_realloc`, `vlibc_aligned_alloc`, `vlibc_free_sized`, `vlibc_free_aligned_sized` get
  contracts with their standard names'; `free_sized` needs a deallocation shape with a size [0050 item 6].

**M4.5f1/f2 Predicates**
- `pure` bodies now reject globals and `static` locals (0041 item 12) [b2a]; R1–R13 do not reach `pure`/`predicate`
  bodies, and `static_assert_pure_body.c`'s listed entry goes stale when bodies are encoded [M4.4c].
- A list traversal's truth tests need liveness inside f2's predicates; `forperm` sees only top-level tokens [M4.5a].

**M4.6 Chars and strings**
- The SDK's `__DARWIN_ALIAS(sym)` asm labels (`_stdlib.h:189`, `_string.h:95`, `strtod`, `strerror`) and glibc's C23
  `__isoc23_*` redirects: record 0046 item 8 refuses every trusted contract on such a function; a record must tell the
  platform's own labels from a user's before vlibc covers them [d1 audit 2 note 7]. vlibc's first body in a header
  re-examines EDG's discretionary errors in system files (record 0046 item 3).
- `main(int, char **)` in a unit with globals is not the entry and fails closed, but its message asks for `reads(g)` in a
  contract on `main`, which is rejected; M4.6 makes `main(argc, argv)` the entry [b2a audit 2 N3].
- `const` objects as cells (question 78, §4.3); `c | 0x20` case bits (0027).
- **vlibc's core** is written to be verified after M4.6 (arrays, pointer arithmetic, char storage); the glue stays trusted.
  vlibc's run test of an allocating SIGABRT handler calls a function that is not async-signal-safe (test-only) [d2v].

**M4.7 Function pointers:** keep `&main` rejected, so `program_start` is reached once [b2a audit 1 N10]; a marker in a
function-pointer typedef (`typedef void (*fp)(int *p CERASTES_MUST_INIT);`) reaches no function today and must be read
(or rejected) when calls through pointers arrive [b2b audit 2 S4].

**M4.8 C parity close-out**
- Exit 3 beside a correct error, pre-existing (reproduced on the baseline `7f11706`): in a function WITH a body, an `ensures`
  atom whose guard differs from its `requires` partner's (`requires(owns(*p) && (c == 0 || owns(*q)))`,
  `ensures(owns(*p) && (c != 0 || owns(*q)))`) is reported by `f__wf`'s partner row (`permission`), and the return's
  internal row "a pointer parameter points into its block" also fails (exit 3); guard that row by the partner's path, or
  make it a `permission` row [c2b test-mutator; the c2b pins use bodiless functions].
- The cost of aliasing `reads`/`live` parameters (question 66): the cell splits at calls were checked by timing, not
  re-audited (four and five parameters 5.7 s and 22 s, six time out with exit 3, documented) [b1b]; entry case splits
  multiply loop verification (one guarded parameter and one cell 2.7 s, two and three 37 s; four aliasing `reads` with a
  loop about 50 s under load); tests stay at three or fewer [b1c1]; the slowest b1c2 tests take 9–14 s under load 150–175
  [b1c2].
- Written atoms on moved pointers (question 79): a written atom whose pointer the loop moves keeps its entry liveness
  amount, a pinned false alarm (`tests/memory/negative/loop_written_moved_owns_false_alarm.c`) [b1c3 round 2 S4].
- A loop over `reads(*p)` and a global written by name may need intermediate asserts for Silicon to keep `*p`'s value
  (prover incompleteness, not the encoding) [b2a audit 2 N2].
- Exit-3 hazard, no case known: Silicon continuing past a failed explicit check and tripping over its own check on a
  store through an unknown pointer (found once in `main`, fixed by splitting against every held global in
  `result_splits`; no general guard) [b2a].
- Mutation: the mask positives were never mutated (question 49), nor the verified doc examples (question 10) [M4.4d];
  the markers' expected mutants are a dropped marker and `must_init`/`ref_to_uninit` swapped; the `g_r` guard of
  `ref_to_uninit`'s kept-flag row is never exercised by a test (`f__wf` reports the shapes that would need it first)
  [b2b audit 1 N8, audit 2 N5].
- From the mutation review: the doc example `verifying/void_conditional.c` keeps a redundant `requires` conjunct (four
  listed equivalents); a body `p | q` against an exact `&` contract runs to the timeout (0023's limit), so
  `not_eq_and_bools` states its contract with `&&` [M4.4c].
- Masks that are constants only through a call argument (`f(v, 4294967295u)` with `requires((v & m) == v)`): Silicon's own
  precondition check may run to the timeout (exit 3; question 51); the exact-set fallback keeps today's false alarms
  (question 50) [M4.4d].
- **Costs to watch:** two written `owns_block` sites through loads over parameter places cost about 33 s for the loop
  alone and about 148 s with one drop in the body, quiet. The tests pin cheaper shapes (question 186). The `?:`
  framing chains repeat in every pair row and every `forperm` instantiation; the reads are hoisted, and the next step
  is the pair and exclusivity structure. This belongs to M4.8 with question 66. [d2b2a]
- **Record 0054's documented limits** (its items 2–5; the twelve bullets are in
  `git show 2430529:internal/handoff.md` §3 "From M4.5d2b2a" and in M4.5d2b2b's plan's Inputs): candidates for the
  close-out's review of false alarms. [d2b2a]
- **Question 189** (owner, 2026-11-09): the cost of two written load-sites is fixed here with question 66. Owning
  fields make the shape more common (a structure with two owning fields under a written loop invariant); M4.5d2b2b's
  spike S40 (g) times it and records the time. [d2b2b planning]
- **Costs to watch:** B1's splits against outside holders grow the paths exponentially with the number of holders (S37's
  adversarial fixture 93 s); a function with several pointer variables beside a heap loop may time out [audit 2 N10].

**M5.0 and M5** (also §4.4)
- `mutate.py`'s classifier does not read local declarations (about 20 listed equivalents rest on a reviewer's reading),
  before M5a; a user C++ function named `assert` would be bucketed by `mutate.py` [S3 note 1].
- Templates [M2 audit B]: a predicate called inside a `forall` lambda in a function template's contract is falsely
  rejected (`spec_call_in_code`); the `operator=` branch of `spec_marker_on_special_member` lost its test; P changes
  overload choice where constant evaluation is a substitution failure (two tests to add); 2–3 tests for the back end's
  constant-context pass in never-instantiated templates; a class-scope `static_assert(q(sizeof(T)))` in a
  never-instantiated class template is accepted (harmless); EDG at P leaves an error node without a diagnostic for a
  `decltype(p)` lambda parameter (`annotation_substitution_error` is load-bearing, M5b fork commit R).
- A closed 128-bit claim over a shifted constant other than 1 is undecided (folding `k * c_pow2(n)` would change
  existing Viper) [M4.4b1 note 6].

**M7 C++ memory and classes**
- **For M7:** vlibc's allocations are rejected in C++ ([intro.object]/10–13 create objects only for `std::malloc` and its
  family); placement `new` (and C++23's `start_lifetime_as`) lift it [0050 item 7].

**M8 Inventory** (the warning sweep is an M8 entry in `plan.md`)
- From M4.5c2a's planning (question 100): struct `?:`, comma and assignment values (`t = c ? s : u;`, `t = (f(), s);`,
  `(t = f()).x`) stay rejected, over-strict [c2a planning].
- From M4.5c2a (record 0044's M8 row): rejected or over-strict — a struct object argument, copy source or destination
  through a loaded pointer or a base with a side effect (`g(*w->q)`, `g(*p++)`, `return *get();`, `(*pp)->t = f();`), a
  compound literal in a loop condition, a `for` step or a `do` condition, a literal used as an lvalue without `&`,
  `volatile`/`_Atomic` literals, `&h.x` as a global initializer, a nested member initialized from a struct value
  (`{s, 1}`), in C++ a value aggregate with an effect beside another non-constant element and a folded `S{…}.x`, C++'s
  `s = S{s.x++, 0}` (defined in C++17, rejected `unsequenced_modification` with a message saying so), a C++20
  designated list skipping a nested member (`T{.z = 1}`), and a temporary with a nested struct member as an argument
  inside a variable's initializer (`int r = get(T{{1, 2}, 3});`: EDG drops its braces there) [c2a, audits].
- From M4.5c1 (record 0043): rejected or over-strict — out-of-order C designators (valid C; EDG #2904-D, C++ requires the
  order), brace elision over a nested struct (`-Wmissing-braces`), a nested braced element whose braces flag EDG drops
  (C++ `S s8{1, {s8.a, 0}}`, rejected as elision), a struct copy in a `for` step, `&s.x`/`&s.t` of an `uninit` struct
  local under `--strict-init`, `{.y = (*&s).x}` in a struct's own initializer (it forms `&s`), `&p->x` with only `live(p)`
  (`null-dereference`: `p` may be one past the end); not measured — member self-assignment `s.x = s.x` and
  self-comparison `s.x == s.x`; accepted with no native positive possible (`-Wextra` only) — a positional list shorter
  than the struct and C++'s `{0}`; positions — own-initializer rejections at the declaration's line [c1].
- Warnings: EDG's #550 on a global that code only writes and only contracts read (clang and GCC are silent; tests read
  such globals in code) [b2a]; record 0025 item 6's "still accepted" shapes (`RED == RED`, clang's
  `-Wparentheses-equality`, `-Wswitch-bool`, `-Wfor-loop-analysis`, `-Wmisleading-indentation`, GCC's
  `-Wmultistatement-macros`, clang's `-Wempty-body` on loops, R10 with a named constant or a constant expression, R7 not
  looking into a `?:` arm that is a comma ending in a constant); untested shapes `else ;` and `do ; while (c);` [M4.4c].
- Rejected more broadly than the compilers [M4.4c]: R13's `;` on the line after the condition and an empty macro
  invocation as the body; R7's `*` through both `?:` arms in every conversion to `bool`; R9 on a 0 or 1 right operand from
  a macro and R11 on an operand from a macro under a written `&&`. R10 and R11 apply in code only, R5 and R9 in
  annotations too (question 44).
- Over-strict, listed: `bool b = &g`, `!&g`, `&g == NULL`, `static int b = (&g == &h)`, C++ `static int *p = &a` of a
  `static` local [b2a audit 2 N6]; C++'s multi-level qualification conversion (`int**` → `const int* const*`), a
  definition that must repeat its prototype's `owning`, two indistinguishable declarations with `owning` [b1c3];
  `(*p = f(a)) + (*q = g(b))` stays `unsequenced_calls`, a caller breaking an exclusion resting on a value `requires` is
  reported `permission` before `precondition` [b1c2]; `restrict` beside a same-typed parameter with only `live` [b1b];
  `a && (y = x = 1);` and a discarded `a && (flag = x);` with a `bool` target [M4.3b note 5]; the pragma rule rejects
  harmless `#pragma clang/GCC diagnostic` (an allow-list would lift it) [M4.2b1 note 1].
- C++'s `p == NULL ? d : *p` with a `const` pointee stays rejected (an lvalue `?:`), with a readable message [b1b].
- `--strict-init` reads declarations, not paths (record 0042 item 8): it accepts an `uninit` local that was already
  given a value, a second `init(&x)` and a marked parameter passed on after a write, and rejects a null argument, a
  pointer local holding `&x`, a reassigned marked parameter and `init(c ? &x : &y)`; C++'s `&*q` is rejected in code
  (since 0032) where C folds it to `q` [b2b].
- An uncalled `owning` declaration is not checked (`owning` beside `reads(*p)` is reported only once the function is
  called or defined); uncalled `must_init`/`ref_to_uninit` declarations are [b2b audit 2 S4].
- Mode differences, not meanings: `x = ((void)(a > 0 && x > 0), 2)` is `uninitialized` in C and rejected in C++; `x ? RED :
  GREEN` as a value accepted in C and rejected in C++ (0018 item 8) [M4.4c]; `c >= +GREEN` accepted in C, rejected in C++
  [M4.2c note 1].
- Not on an allow-list, no wrong verdict: asm labels on globals; a user `environ`/`__progname` without the system header
  (fine on macOS, Linux unchecked) [b2a audit 2 N5].

**Standing** (no session; act when the code is next touched)
- *From M4.5c2b:* call-site messages for a load-rooted callee place use the callee's spelling (`**pp`), not the
  argument's (`*p` for `&p`); `need_lb` is conservative (any by-name cell write gives every loading place a loop-entry
  snapshot: more case splits, no verdict); a loop moving a pointer between two parameters' objects through a global root
  (`gp`) hits 0040 item 5's swap false alarm like a local pointer does (question 79, M4.8); the existing `.map` texts keep
  `(p->q)->x` for invariant reads through a pointer member (P69's pins), while new load-rooted contract places are spelled
  as written (`Expr::user_spelling`); three S35 contrast fixtures expect `verified` of an unsound form on purpose
  (`s35_n_bodiless_retarget_nocheck`, `s35_o_old_chain_state_postdisjunct`, `s35_f_reads_fact_unguarded`), each beside its
  must-fail twin; `site_guard` reached in a loop annotation with a heap-reading guard is an internal error (unreachable:
  `resolve_written` lowers those guards) [c2b audit 2 N4–N6].
- *From M4.5c2a:* a write by name to a member of a struct LOCAL or a by-value parameter's copy in a loop still reaches
  every automatic place of its field (0043 item 11); globals got the precise rule (record 0044 item 8), locals would change
  existing programs' Viper (completeness only) [c2a]. The lint's `Addr` rule for struct-global rows cannot tell two struct
  types of one shape apart (the `.vpr` names no type on a global; the encoder and `check_program` match the types) [c2a
  audit 2 S4]. A hidden compound-literal local is named by its type's spelling (`(struct S){...}`), not its source text;
  a returned struct's dangling pointer leaf is reported without naming the leaf (`a.q`) [c2a audit 2 N9]. The Error
  messages page and record 0043 item 7 had quoted a copy message ("copies `s`, whose member …") the tool never printed;
  the page now quotes the printed one [c2a].
- *Fail-closed corners from M4.5c1:* `overlap_pairs` compares only the two addresses when both pointee types are equal,
  the `one_type` path filter skips reads-sum terms, `f__wf`'s equal-ghost skip for one root with two paths: complete only
  while types prevent `p == &q->t`, a miss costing Silicon's own error (exit 3), never an assumed fact (record 0043 item
  9) [c1 audit 2 N2]; storing `&p->t` into a cell skips `ptr_use`'s early `WF_S` row, asserted where it is assumed
  [c1 audit 2 N3].
- *By design:* C++17 sequences `g += f()` and `*p = (**pp = 5)` (the right operand first), but 0039's C rule stays in both
  modes [b2a N9, b1c3 N4]; `return p = &x;` and `int *r = &x; return r;` stay accepted (the caller's first use fails
  `lifetime`; questions 65, 70) and EDG's #1056 prints beside an R14 rejection [b1c2]; in an invariant `==` is identity
  with no liveness check [b1c1 note 7].
- *Messages:* `-Dfalse=1` (a program macro over vlibc's contract text) is reported three times, at the platform's
  declarations of `abort`/`exit`/`_Exit`, as "cannot be defined here"; it should point at the command line [d1 audit 2
  note 6]; a `#line` in a vlibc header is reported with the `#pragma GCC system_header` message (record 0046 item 3) [d1].
  `int *volatile gp` is named "volatile int *" [b2a N7]; an `annotation_mismatch` between declarations written
  by one macro names the parameter by position [b1c3]; `cerastes_old(*p)` in a loop invariant is rejected in C as "a
  conversion to or from a pointer type", in C++ as `old_outside_ensures` [b1c1]; rejection texts end "(decision 0012)" or
  "(decision 0037)" where a later record decides them, and the type message still says "only integer types, bool and the
  enumerations … are" [M4.2a–M4.2c, b1a, b1c1]; a block-scope `extern const` naming an internal-linkage C++ constant is
  called "a static local variable" [M4.2b1 note 2]; C's `case -1:` over an enumeration prints 4294967295 [M4.4a note 7];
  `d == (x ? (y, RED) : GREEN)` gives one diagnostic in C, two in C++ [M4.4b1 note 2]; lowering's
  internal-inconsistency diagnostics have no position; two failing `requires` clauses at one call print the same
  message twice (tests choose contracts that fail once) [M4.4d]; prototype `must_init` and definition `ref_to_uninit` give
  two `annotation_mismatch` errors (one per attribute), and two marker errors with one tag on one line collapse to one
  (`reject()`'s de-duplication); the markers' rejections end "(decision 0037)" like every `unsupported_annotation`; an
  unframed post-state read through an alias of a `ref_to_uninit` parameter (`ensures(*q == 1)`, `q == p`) is
  `permission` in `f__wf` and also `postcondition` at the return [b2b].
- *Positions:* with a `continue` in the body, a written atom's back-edge error is reported at the loop statement
  [b1c3 N3]; a loop body without braces likewise (no test) [b1c1 S5]; in C++20 a loop's IL position is its first
  attribute's (P59).
- *Fail-closed checks and their limits:* the memory-order check cannot see obligations that read cells, inline reads, or
  a set that is too large (0039 item 4, 0026 item 4; pinned by the audits' probes) [b1c2]; the termination-set fix was
  not re-audited (smaller sets are the safe direction; `noreturn_call_in_expression_termination.c`) [b1c2]; item 2's
  checks of M4.4c read the placement emission uses, a set computed too large is still invisible (pins
  `call_write_noreturn_divzero.c`, `never_returns_call_divzero.c`) [M4.4c]; `Acc::owner` can point at a dead stack `Full`
  of a then-arm while the else-arm is lowered (safe today; a heap-stable id would be sturdier) [b1c2 note 2].
- *Trusted reasoning checked by counterexample, not proof:* `f__wf`'s added facts (aliasing values equal, a `reads`
  place's post value equals its pre value, `owns` pairs distinct in one clause list) [b1b]; by-name cell checks are
  `permission`, so an encoder bug dropping a cell's permission would read as a user error (a VIR hook could pin it) [b1b].
- *Not re-audited* (the cap): b1c3's second B1 fix (in a contract, every pointer conversion changing pointee
  qualifiers, and `owns`/`owning` of a `const` place, rejected; pinned by `reject_contract_qualifier_*`,
  `reject_owns_const_*`, `reject_owning_const_int.c`) [b1c3].
- *Self-test pins:* the VIR self-test's written-atom cases pin failing rows by index (`fail_only(category, loc, count,
  index)`), so a change in row order breaks them, not a verdict [b1c3]; the VIR mask cases pass with either encoding by
  design [M4.4d round 2 note 10]; `calls_drop_exact_set_hook_off` duplicates `calls_fixed_order_hook_off` (harmless)
  [M4.4d].
- *Lint:* a `Perm` local as a non-first target of a multi-target call is not seen, an invariant's `acc` amount is not
  required to be a `Perm` local, and the "while without decreases" rule reads the first line holding `{` as the body
  [b1c1 note 8, M4.2a note 7]; the lint of `%` and `\` reads only the divisor and exempts the prelude bodies by name (the
  guarantee is structural: `low::mod_pow2`/`div_pow2`) [M4.4d]; `lint_flags` checks the flag on the line after the write
  only [M4.3a note 3].
- *Tooling:* `build/cerastes verify --keep-vpr` with no path writes `<stem>.vpr` into the current directory (P60) [b1c2];
  `mutate.py` does not mutate namespace-scope initializers, a literal mutated past `9223372036854775807` changes type,
  `--list` is slow under load, a mutation wrapper's cap must stay above the driver's `--timeout` (or Silicon processes are
  orphaned), `drop-complement` on a C++ destructor and `swap-bitwise` on `(T)&x` give stillborn mutants [M4.2b1 note 7,
  S3 note 2, M4.2a note 6, M4.4b1 notes 4, 7]; the walker oracle cannot tell an orphan closure copy from the used one, a
  phantom `on type <unnamed>` line could pass, and it lists neither a first declaration's own attribute after the
  parameter list nor one on a template definition [M2 audit B note 6, M4.4b2 note 5].
- *Driver and platforms* [S5]: `posix_spawn_file_actions_addchdir(_np)`'s result is not checked and Silicon is given the
  relative `program.vpr` (passing the absolute path removes it); `LC_ALL=en_US.UTF-8` may not exist on a bare Ubuntu;
  C++'s `assert((bool)x)` loses the user's cast on Linux, `assert(E::b)` with a scoped enumeration compiles only under
  glibc; `__thread` and the x86 GNU attributes are accepted on Linux only (0031 item 7); a hand-written
  `((void)sizeof(x++), x) ? (void)0 : __assert_fail(…)` on Linux matches and verifies (sound; require the `sizeof`'s
  operand to be a call of `__assert_single_arg`); the glibc matcher's near misses can only run against glibc
  (question 59).
- *Recognition by name or shape:* `assert` by its macOS expansion and `__assert_rtn`; the character-literal trigger
  compares VIR type names (`int_kind` would be sturdier) [S3, M4.4b2 note 3]; the noreturn trust checks a routine's first
  declaration (source sequence lists, on since M4.4b2, can now make it exact, and the libc table's "declared only in
  system files") [M4.2a note 1].
- *Fail-closed corners and unreached guards:* `names_object` false on an unknown node kind; a C case value of an unknown
  shape; defensive branches with no test (M4.1's undetermined `break`/`continue` target, a non-integer scrutinee, a case
  value of another type; a declaration directly in an unflattened generated block; a `cerastes::` attribute on a label;
  `comparison()`'s raw operands of two enumerations; "an enumerator whose enumeration this version cannot find";
  `enum_case_value`'s constant without `expr`; the exit-3 paths for emitter problems and "encoded twice", only
  `--selftest` sets the flag) [M4.1–M4.4a].
- *Accepted shapes worth a test:* `0 && (x = 1)`, `1 || x++` (folded, rejected); `assert((note(x), x >= 0))` (rejected by
  `check_program`); `decltype(x++)` (accepted, clang warns); a `_Noreturn int main` in C23 (only C++ tested); a typedef
  whose first reason is not an array is rejected only at a use; `_Static_assert(1.5, "")` names its type in C but not in
  C++ [M4.3a, M4.2a, M4.4b2]. Not pinned: a bare `p;` with `p` dangling reports `lifetime` (EDG's #174, and #550 in C++,
  keep it out of `error` tests; the `(void)p;` twins pin the rule) [b1a].
- *Costs to watch:* liveness checks at every pointer use cost about 1.5–1.7× time and text on S26's scale program; deep
  `^` mask chains are fast to depth 32 for range claims, identities only to the measured depth (0027 item 4) [M4.5a].
- *EDG, not chased:* in C23, `typeof(*cp)` for a `const int* cp` drops `const` (P53; harmless for `cerastes_old`) [M4.5a].
- **Lint:** `owner_assignment_ok` admits `held_X := <x> != null` anywhere. This is harmless (audit round 2 on C/D,
  N2), but it is a place where a later rule could be tightened. [d2b2a]
- **Toolchain-dependent facts found:** EDG's CMake takes `EDG_BASE` from the building compiler (bootstrap names the clang
  base); EDG's clang-18 emulation differs on eight tests (record 0053 item 11); with clang 18 and 20 vlibc's `<stdlib.h>`
  shim meets glibc's declarations with an incompatible linkage through libc++'s `<coroutine>` (question 170); glibc
  2.39 has no `__assert_single_arg`, the C23 one-argument `assert` the encoder matches (0031 item 5); ShellCheck 0.9.0
  (Ubuntu 24.04) adds SC2015. [S8]
- **Argument order:** GCC evaluates a call's arguments in another order than clang; `f(g(x), std::move(x))` lost
  `x` in the GCC build (`vir.cpp`'s builder, fixed). The GCC-built profile now catches such code, and clang-tidy's
  `bugprone-use-after-move` finds it on any compiler (the audit ran Homebrew's clang-tidy 23.1.1 over all 29 TUs of
  `build/cmake/compile_commands.json`: only the two fixed lines); a lint step would need a pinned clang-tidy. [S8 audit]
- **The position map accepts a row without a file** (`src/viper/posmap.cpp:25` stops the run, exit 3, only for a row
  without a line or column): adding `e.file.empty()` (and an empty `note_file` when there is a note) would make a lost
  file fail safe on every compiler, must-verify tests included (invariant 7). Not done in S8: the VIR self-test's
  synthetic programs may have rows without a file, so it needs a full-suite run. [S8 audit, should-fix 2]
- `include/cerastes.h`'s fail-closed `#error` (the cerastes front end without its attributes, record 0008 item 2) is
  one line again under `clang-format off`, now at line 42 (was 40); no test reaches it (reviewer round 1 S3). [S7]
- **Stale facts found, not fixed** (no code changes in S6; S7's Part 3 fixes the first): `src/backend/register_attributes.h`'s header comment says
  "eight attributes" (it registers more: `decreases`, `owning`, `uninit`, `ref_to_uninit`, `must_init`, …); `plan.md` §0
  and the M5 plan name `internal/soundness/`, which does not exist (the developer guide says so; the arguments are in
  the decision records and `internal/encoding.md` §3); 0005 item 5 still describes the upstream-only submodule guard that
  0009 superseded. [S6]
- **`examples/dev/`** is in the policy lists but has no files: the developer guide quotes test headers in `text` blocks
  (0049 item 5). [S6]

## 4. Session plans

**Rules for every session** (memory `session-size-few-hours`):
- **Size:** what fits in one context, under about 12 h unless a reason is given (owner, 2026-10-30, question 77; was
  ≤ 4 h), with fixed scope and stop rules.
- **Up front:** owner decisions and pre-approved expectation changes are asked before the work.
- **Invariants:** the decision record before code (6); must-verify and must-fail pairs (2); docs in the same session
  (8).
- **Team:** three Sonnet agents with disjoint files (edg-backend: `src/backend`, `src/driver`, build; il-analyst:
  probes, IL evidence, Silicon spikes, `src/vir` + `src/viper`; test-mutator: tests, harness, compiler matrices,
  `mutate.py`, `lint_viper.py`) and the Opus soundness-auditor (one round per diff plus a re-audit of blocking
  fixes; should-fix items not taken go to §3).
- **While working:** single-file runs (`build/cerastes verify` directly: `run_tests.py` locks `build/vpr`); builds
  serialized (`lockf build/<session>/build.lock cmake --build build/cmake`).
- **`check.sh` once, at the end, after the last code change** (owner, 2026-11-09: "Since each run of check.sh now
  takes hours we should change our future work to run it less"; targeted runs and the neutrality regeneration
  before it), **on the x86-64 box only, never the Mac** (owner, 2026-11-09: "limit to just the linux because it's
  much faster, and doesn't lock up the local machine"): the branch as a git bundle into S8's `ubuntu-26.04`
  container, `~/cerastes-s8/p-ubuntu-26.04`, then `scripts/check.sh` there. The Mac keeps its targeted runs.
- **x86-64:** the EDG suites on the x86-64 host are their own session, but any x86-64 test run a session needs goes
  to `matt@clang4.cpp.al` automatically (owner, 2026-10-29: not Docker multi-arch on the Mac, which is only the
  fallback when the host is down).
- **Records and plans:** a record takes the next free number when it is written (0055 is M4.5d2b2b's). The approved
  plan is saved as `.claude/plans/<milestone>-<topic>.md` (plan.md, "Session protocol").
- **What a worktree takes with it** (owner, 2026-11-09, question 193): session tools and spike summaries are
  committed. `vprdiff.py` and `regen.py` were lost with d2b1's worktree and again with d2b2a's, and so was S39's
  summary. From M4.5d2b2b they are `scripts/vprdiff.py` and `scripts/regen_vpr.py`, and a spike's summary goes into
  `internal/il-inventory.md` beside its probes.

Finished sessions: `plan.md`'s Done table has a row each, `.claude/plans/` their plans (from M4.5b1a on),
`internal/status.md` their results and the commits it names. The warning sweep (was M4.4e) is an M8 entry in
`plan.md`.

**Pending side sessions** (none blocks M4.5d2b2b):
- **The EDG box session** (§4.1, pending since M4.2c).
- **The vlibc `<stdlib.h>` shim session** (owner, 2026-11-09, question 170: "Follow recommendation using a vlibc
  session"). With clang 20 and 18, vlibc's `<stdlib.h>` shim meets glibc's declarations with an incompatible linkage
  through libc++'s `<coroutine>` (`reject_coroutine.cpp`'s `incompatible_linkage_specifier`; record 0053). A vlibc
  session finds the cause and fixes the shim; then the clang-20 profiles drop their exclusion
  (`scripts/ci_toolchains.toml`).
- **The claims audit is not done** (owner, 2026-11-04: "I can have the soundness auditor run in a different session"):
  a read-only soundness-auditor pass over `bb41d55..s6-tutorial`'s pages (`pages/tutorial/`, `pages/dev/`,
  `overview.adoc`), the policy-list diff and the website's paragraph, for statements that are false or overclaim about
  behaviour, trust or a result's meaning; its fixes are doc edits plus a rerun of the changed examples. [S6]

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
session start): the fork's `cerastes` is 29 commits behind upstream `main` (`a0325e00e6`, fetched 2026-10-30 at
M4.5b1c3's start); merge it
(never rebase), run the full suites on the box, then move the pin with `check.sh` green and the goldens' diffs reviewed.

### 4.2 M4.5 Pointers (sessions b1–f3; designed in M4.5a)

**The design** is records 0028 (the memory model: E2, the `Block` domain, liveness, explicit checks), 0029 (the
annotations and C's spec functions), 0030 (vlibc), 0032 (b1a's slice) and 0033–0036 (the four axioms); evidence in
`il-inventory.md` "M4.5a" and "M4.5b1a"; the encoding pinned by the fixtures in `tests/viper-smoke/memory/`.

**The VeriFast port inventory** was `build/m4.5a/ports.md` in M4.5a's worktree (gitignored, and gone with it); its
conclusions are below per session. `malloc_block.c` holds no `malloc` (an address-taken struct local);
`insertion_sort.c` is a linked list whose ghost machinery is not portable as written (an array insertion sort in
M4.5e2 replaces it as a benchmark); 14 of the 22 files run VeriFast with `-disable_overflow_check`, so their ports
need `requires` bounds; `functions.c`'s predicate families and constructors, consumed fractions, backward `goto` and
`open_module` are M8 entries.

**Finished: b1a to d2b2a** (records 0032–0054). `plan.md`'s Done table has a row each and `.claude/plans/` their
plans. The narrative of their splits, the original d2 paragraph and d2's dependency map are in
`git show 2430529:internal/handoff.md` §4.2. Each session rejects what a later one makes precise (fail closed).

**Pre-approved** (owner): a session may retire or re-expect exactly the rejection pins its plan lists (its "Flips"),
and no others; a pin whose construct stays rejected keeps its `expect:` line, and only its message and `Pins:`
comment name the new session.

**M4.5d2b2b Owning fields** (planned; plan `.claude/plans/m4-5d2b2b-owning-fields.md`; record 0055; questions
188–193). Acyclic owning fields (target `d`), complete: owning a structure is `owns(*p)` plus `p->q == NULL ||
owns_block(p->q)` per owning field (the closure, under the atom's own guards, to any acyclic depth); owning leaves of
struct locals, struct globals and `static` struct locals as owners in memory; copies as stores to an owner; loops; the
docs table that maps P4417R0's `[[owner]]` positions. Its one flip is `reject_struct_owning_field.c`. The planning
survey measured 1,260–2,400 `src/` lines (plan for 1,700–3,300).

**M4.5d2b2c `not_invalidating`, `--strict-owner`, marked fields** (after d2b2b; question 188).
`cerastes::not_invalidating` on a pointer parameter, checked (178); `--strict-owner`, P4417R0's p4 and p6 as an
opt-in (177); `ref_to_uninit` on a field (192); d2b2b's cuts 1 and 2 if taken. No `no_dangling` in C (179). Its design
notes are §3 "M4.5d2b2c"; the survey measured 290–440 and 200–280 lines.

**M4.5e1 Arrays.** Local arrays as blocks, `a[i]`, array parameters, `owns_range` as quantified permissions over the
absolute slot, the marked range rows in the driver (owner, after S25), `forall` in C (0029 item 4: its planning decides
between pure functions with contracts and moving M4.5f1 first), sum and max. P38: a braced initializer with more than
1000 elements keeps no `expr` for its elements (`REDUCE_BACKING_EXPRESSION_USE`), only an `orig_type`; read every
element through `constant_value`, which rejects that shape, never through `ck_aggregate`'s values directly, or a
wrapping element would pass unchecked. M4.4b2's types computed from a constant (`sizeof`, `typeof`, typedefs). Inner arrays and arrays of
structs use M4.5c1's member blocks (0028 item 16). Port:
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
`memcpy`, `memcmp`, `strcmp`: contracts on the standard names, and implementations `vlibc_strlen`, `vlibc_memcpy`,
`vlibc_memcmp`, `vlibc_strcmp` verified in vlibc's CI, record 0048; M4.6 decides the rule that verifies vlibc's `src/`,
0048 item 5), `main(argc, argv)`; character
access to other objects stays rejected (0028 item 1); `c | 0x20`-style case bits are decided (0027). Ports:
`strlen.c`, `memcpy.c`, `memcmp.c`. Open point: a zero-terminated buffer of unknown length needs a recursive predicate or a
built-in notion (M4.5f3 decides). **`const` objects in the model** (owner, 2026-10-30, question 78, the recommendation):
field names ignore `const`, so record 0040 item 2 rejects, in a contract, every pointer conversion that changes pointee
qualifiers, every comparison across them and every `owns`/`owning` of a `const` place (fail closed, after M4.5b1c3's
audit round 2 B1 and its re-audit); `char *strchr(const char *s, int c)` with `ensures(r == NULL || r points into s)`
needs them. M4.6 models a `const` object as a cell the function holds below `write` after its initializer (record 0028
amended: allocation, scope exit and `owns` of a `const` place), then lifts those rules; `&K` of a `const` global and an
address-taken `const` local (M4.5b2a's pins `reject_address_of_const_global.c`, `reject_case_const_address_taken.cpp`)
come with it.

**M4.7 Function pointers:** contracts on function types, calls through pointers; ports of `tutorial_solutions/byref.c`,
`examples/functions/functions.c` (without its predicate families, an M8 entry).

**M4.8 C parity close-out:** the VeriFast C differential set (the ports above, run with VeriFast in Docker as M5c
sets up, or recorded outcomes), the full mutation run over the M4 tests and the verified doc examples never mutated
before (owner, 2026-10-27, §8 question 10), the mask positives with mutants of a mask constant counted as "undecided"
(§8 question 49), the cost of aliasing `reads`/`live` parameters (§8 question 66: fix it here, e.g. Silicon's joins
or a per-call split order; record 0037, Consequences), with it the false alarm of written atoms on moved pointers (owner,
2026-10-30, question 79, the recommendation: per-place liveness amounts at the loop head, or the share named by the
parameter's ghost amount; pinned by `tests/memory/negative/loop_written_moved_owns_false_alarm.c`, §3), the Memory and
ownership page complete, M4's acceptance (plan.md),
and the deferred list for M8
(unions, floating point, `volatile`/`_Atomic`/`_BitInt`/vector types, variadic functions and `printf`, several
translation units, concurrency).

### 4.4 M5.0 and M5 — type traits and generic templates (designed in the M4.1 session)

**M5.0 Type traits** (owner, 2026-10-26, old §11 question 39; `plan.md` "Before M5"). After M4.8 and before M5a. M4.4b2
skips a static assertion whose condition calls a function, uses a type trait (`enk_builtin_operation`, a variable
template such as `std::is_integral_v<T>`), a concept-id, or names an entity of a system header (`std::numeric_limits`).
M5.0 decides how cerastes reads such conditions (and type traits elsewhere: contracts, `if constexpr`), so that M5's
integral `T` and M6's requirement contracts rest on it. Planned at its own planning.

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
today (in `encode.cpp`: declarations in `Encoder::declaration_level_checks`, routines in `Encoder::run` through
`template_declared_here`, calls in `callee_for_call`; S7 replaced the line numbers, record 0051 item 8); generic proofs read contracts
from the root template; contracts can use lambdas naming `T`. Latent hazard: a template instance whose body is in a
verified file takes `callee_for_call`'s has-body path (`encode.cpp`) and would become a bodyless VIR
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
(the `predicate` and `pure` rows of `register_attributes.cpp`'s table). Fork commit R: EDG diagnoses series L's silent error node for a `decltype(p)`
lambda parameter (§3, "M5.0 and M5"); the `decltype` tests move from `cerastes-annotation_substitution_error` to EDG's diagnostic
at the same line (owner's rule). Plus the close-out audit B's missing tests (§3, "M5.0 and M5"). Fail-before/pass-after,
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
symmetric ranges (in `lower.cpp`, `unary`'s `Neg` obligation `a != lo` and `binary`'s division
`!(a == lo && b == -1)`) are not reused —
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
- **Worktree.** Each session creates its own worktree, `.claude/worktrees/<session>` on its branch from `origin/main`
  (`git worktree add .claude/worktrees/m4.5b2b -b <branch> origin/main`; the old `.claude/worktrees/m4.2b1` is gone since
  M4.5b2b), with its submodules cloned with `--reference` to the main checkout's module stores (`third_party/edg`,
  `third_party/silicon`, then `silver` inside it), then `scripts/bootstrap.sh`. A shell `cd` elsewhere may leave the
  worktree: name it in every command.
- **Builds and baselines.** Take the baseline (`run_tests.py -j 8`, `build/vpr` → `build/<session>-baseline-vpr`, the VIR
  self-test into `build/<session>-baseline-vir`) before any build; agents edit meanwhile but build only after it, with
  `lockf build/<session>/build.lock cmake --build build/cmake`, and nothing builds while `check.sh` runs. A session that
  moves the pin runs `check.sh` once early.
- **A baseline that agents cannot disturb** (M4.5b1a; from M4.5b2b in a detached worktree of its own,
  `.claude/worktrees/<session>-base` at the starting commit, so the main checkout is never touched): its own `build/` and
  `build/vpr`, copied into the session worktree's `build/<session>-baseline-vpr`. Then the
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
- **Doc pitfalls.** `[[cerastes::owning]]` in prose becomes an anchor (with `::`, even `+…+` and `{empty}` leave one, and
  `check_doc_examples`' source lint refuses `++…++`): name the attribute (`cerastes::owning`) and let the example show
  the brackets; a code span holding `**` needs `+…+`, or `pass:[…]` when it also holds ` + `; a backtick next to a double quote
  (`` `*q`" ``) is a curved quote and breaks the span (M4.5b1c3). `[[noreturn]]` in text becomes an anchor even inside `+…+`; `__name` in backticks needs `+…+`; a
  possessive after a code span breaks the next span; `pass:[…]` holding `<` or `>` is refused by
  `check_doc_examples.py` (write `+…+`). `build_docs.sh` and `check_doc_examples.py --html build/doc` catch them.
- **Test generators** (M4.5c1): a script that writes many tests must live in `build/<session>/`, not `/tmp`, and must not
  be rerun after any test it wrote was edited by hand: a regeneration in M4.5c1 silently restored an owner-approved
  `expect:` change and failed `check.sh`. After the first full run the files in the tree are the source of truth.
- **A test that loses an `expect:` line** (pre-approved) keeps its line numbers with a comment in its place
  (M4.4c, the four `reject_assert_shape_*.c`).
- **Builds during a suite run** (M4.5c2a): `lockf` serializes builds, not a build against a running `run_tests.py`; an
  agent's rebuild replaced `build/cerastes` as the first suite run started. Tell every agent to hold builds before a
  directory-wide run, and treat a run that overlapped a build as provisional.
- **Generators and comment fixes** (M4.5c2a): a comment-only fix made by re-running a generator still re-ran it; the
  agent then compared every generated file's `expect:` lines with its earlier single-run record (0 differed). Fix comments
  by hand instead.
- **Process:** a test generator re-run wiped three hand-added expect lines. The audit caught it, and §6's rule (never
  re-run a generator over files in the tree) held after that. The audits ran on snapshot commits made with a
  temporary index (`GIT_INDEX_FILE=… git add -A; git write-tree; git commit-tree`) in a second worktree with its own
  build. So round 2 audited each part while the next part was built, with no mixed working tree. [d2b2a]
- **Every session that changes CI:** a new toolchain is one `[[profile]]` in `scripts/ci_toolchains.toml` at `dispatch`,
  promoted to `nightly` after a green full check; the developer guide's "Adding a toolchain" has the fields. The guard
  pins every assertion line of `ci.sh`/`ci-linux.sh` literally (messages included) and `ci.yml` as a pattern tree, so a
  changed assertion or step is a self-test change too; the two references are pinned in `CI_REFERENCE_PROFILES`. [S8]
- **Linux rehearsals on the box:** `~/cerastes-s8/run.sh setup|update|check <profile>` (a fresh copy of `base`, a
  container per profile from a clean `ubuntu` image without compilers, `ci-linux.sh <profile>` then `check.sh`). Ship
  commits as a git bundle (`git bundle create … <branch>`, then `run.sh update`). Never pipe a macOS `tar` into the
  trees (it adds AppleDouble `._*` files, and `scripts/._*.sh` breaks `check.sh`'s lint), and never clean them with
  `find -name "._*"`: EDG's test tree has directories named `._…`. [S8]
- **Every session:** new tool code in the style (`CLAUDE.md`'s conventions, the developer guide's "Code style");
  `scripts/format.sh` before committing; brace initialization, `U` and one declarator per declaration are review rules.
  A new `.cpp` under `src/` changes `object_identity.py`'s TU-count constants (`EXPECTED_TU_ENTRIES` 30,
  `EXPECTED_TU_FILES` 29): update them in the same commit, or the tool refuses. [S7]
- **Refactors:** `python3 scripts/object_identity.py --before <base>` (and `--tokens none|braces`, `--formatted-by`)
  proves a behaviour-neutral change long before `check.sh`; it compiles only the running platform's code, so a refactor
  of platform code also runs it on the x86-64 box. [S7]
- **Dependabot's npm bumps:** after one, check that `doc/package-lock.json` has one hoisted `@antora/logger` (`npm ls
  @antora/logger`); nested copies silently stop `build_docs.sh` from failing on warnings (`DocBuildWarningsTests`
  catches it). [S7, `9cff0ad`]
- **Not done by S7** (questions 150, 151, answered 2026-11-05 with the recommendations): about 2,500 lines of `src/`
  over 120 columns (almost all whole-line comments and long string literals) are re-wrapped as they are touched; the
  132 `{ // comment` brace lines stay until touched, and **no new or touched line puts a comment on a line with a
  brace** (`{ // …`, `} // …`; owner: "in the future I don't want comments on the same lines as braces"; `CLAUDE.md`,
  the developer guide's "Code style"). A `for`-init with two declarators (`encode.cpp`, the line-scanning loop) is the
  declarator rule's one exception. [S7]
- **Tutorial upkeep** (0049 item 4): M4.5e1 owes the arrays chapter (and quantifiers), M4.5e2 extends it with pointer
  arithmetic, M4.5f2 owes predicates and a linked list, M4.6 strings, M4.7 function pointers, M5 generic templates, M6
  requirement contracts, M7 references and classes. Every session that changes a message, a category or an accepted
  construct a chapter shows updates its prose and `console` blocks: the blocks are copied from a run in
  `doc/modules/ROOT/examples/tutorial/` (`../../../../../build/cerastes verify <file>`), and only review checks them. A
  check that reruns each shown command and compares the output is possible later work (0049, Consequences). [S6]

## 7. Acceptance

Each session's acceptance commands are in its plan (`.claude/plans/`); the results of finished sessions are in
`internal/status.md` (the latest entry) and the commits its history names.

## 8. Questions for the owner

Each open question has a recommended answer. Answer inline, as before. Answered questions 1–187 are recorded in the
decision records, the session plans, `internal/status.md` and the handoffs reachable from
`git show 2430529:internal/handoff.md`, whose §8 digests questions 65–187.

**Standing answers that later sessions act on:**
- **69:** the upstream EDG merge and the pin move wait for the box session (§4.1).
- **77:** sessions are sized to one context, under about 12 h unless a reason is given.
- **160:** a new test's expect line may be corrected after its first run when it fails for its designed reason at
  another line or a neighbouring category; each correction is listed in the status entry.
- **176:** the owner handles CI (the nightly timeout, the `profiles=all` dispatch).

Other answers that steer a later session are in that session's notes (§3, §4): 66, 79 and 189 (M4.8), 78 (M4.6),
120 (M4.5e2), 174, 175 and 177–179 (M4.5d2b2b and d2b2c).

**Answered in the handoff after M4.5d2b2a (2026-11-09; S8's open questions):**
- **169** `ubuntu-24.04-clang18` (EDG emulating clang 18 differs on eight tests, record 0053 item 11): "Follow
  recommendation". It stays at `dispatch` as the record of what the clang-18 emulation changes, and is revisited at
  M8's inventory (`plan.md`, M8's known entries). M4.5d2b2b's Step 1 updates the comment in
  `scripts/ci_toolchains.toml`.
- **170** vlibc's `<stdlib.h>` shim under libc++ 20 and 18: "Follow recommendation using a vlibc session" (§4,
  pending side sessions).
- **171** the Mac's Command Line Tools: "I will handle the update tonight." (§2).
- **172** `macos-26`: "Follow recommendation." After the owner's `profiles=all` dispatch, the next session
  (M4.5d2b2b, its Step 1) promotes it to `nightly` if its job is green, or records what differs.

**At M4.5d2b2b's planning (2026-11-09, each the recommendation; recorded in its plan):**
- **188** M4.5d2b2b's size: two sessions. d2b2b takes owning fields, complete, with the `[[owner]]` mapping; d2b2c
  takes `not_invalidating`, `--strict-owner` and d2b2b's cuts. The surveys measured 1,750–3,100 `src/` lines in all
  (owning fields 1,260–2,400, where d2b1's survey had 1,000–1,500), and d2b2a had landed 3,245 against 1,650–2,400.
- **189** the cost of two written load-sites: M4.8, with question 66.
- **190** a copy of a structure with an owning field stores to an owner (source and destination then share the
  block, as two owning locals do after `b = a;`); a by-value parameter or result is a value and carries a block only
  where the contract says `owns_block(s.q)`.
- **191** an owning field may point to a structure with an owning field, to any acyclic depth; chains are a
  pre-agreed cut (then rejected, naming M4.5f2).
- **192** `ref_to_uninit` on a field: in d2b2c, with the strict rules.
- **193** the session tools are committed under `scripts/`, and spike summaries into `internal/il-inventory.md`.

**Open:** none.
