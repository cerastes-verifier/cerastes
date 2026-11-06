# Handoff at M4.5d2b1's planning — partnerless results, owning locals, marked locals (record 0052) (2026-11-05)

The next session reads this file after `CLAUDE.md`, its own section in `plan.md`, `internal/status.md` and
`internal/decisions/`. `status.md` has the formal entry. This file adds what a cold start would otherwise have to
rediscover, and holds only what is live (owner, 2026-10-26; consolidated again at M4.5b2b's start, 2026-11-02: §3 is
regrouped by the session that consumes each note; updated at M4.5c1's, M4.5c2a's, M4.5c2b's, M4.5d1's and M4.5d2a's starts, and
by S6, a docs session that changed no code: its additions are marked "S6"; at M4.5d2v's start, 2026-11-04; at S7's
planning, 2026-11-05; at S7's end, the same day: §1, §2, §3's "From S7", §8; and at M4.5d2b1's planning, the same
day: §1, §2, §3, §4.2, §8). The text before M4.5d2b1 is `git show a660abc:internal/handoff.md`, before S7
`git show c2bc48d:internal/handoff.md`, before M4.5d2v is `git show 9255b93:internal/handoff.md`, before S6
`git show bb41d55:internal/handoff.md`, before M4.5d2a `git show 9d98fe8:internal/handoff.md`, before M4.5d1 `git show d26a87c:internal/handoff.md`, before M4.5c2b `git show 7f11706:internal/handoff.md`, before M4.5c2a `git show cbdcce5:internal/handoff.md`, before M4.5c1
`git show 1bfa705:internal/handoff.md`, and before M4.5b2b `git show 38877b4:internal/handoff.md`, whose header names
every earlier version (each note below keeps its origin in brackets, so its full wording can be found there). Session
plans approved from M4.5b1a on are in `.claude/plans/`; the notes a session consumes move into its plan.

## 1. State

- **M4.5d2b1** (2026-11-05; record 0052; plan `.claude/plans/m4-5d2b1-results-locals.md`, questions 150–161): done on
  branch `m4.5d2b1-results-locals`, from S7's tip `a660abc` (S7 is not merged yet and the owner was away: "make your next
  branch on top of the current one"), in the worktree `.claude/worktrees/m4.5d2b`; result in `internal/status.md`. M4.5d2b
  is split (question 152): d2b1 done, **d2b2 next** (§4.2, §3 "From M4.5d2b1" and "M4.5d2b2"). Planning surveys, spike
  S38, the audits (`audit{1,2,3,3b}.md`), the expect corrections and the Viper shapes are in `build/m4.5d2b/` in the
  worktree; the baseline is S7's (`build/m4.5d2b/baseline`).

- **S7** (2026-11-05; record 0051; plan `.claude/plans/s7-code-style.md`, questions 146–149): done on branch
  `s7-code-style` (from local `main` `c2bc48d`, the planning commit) in the worktree `.claude/worktrees/s7`; result in
  `internal/status.md`. cerastes's tool code is in the owner's Boost style (clang-format's layout with a pinned
  23.1.3, then the sweep: brace initialization, `U`, one declarator per declaration), proved behaviour-neutral (object
  files and IR identical at every commit on macOS and on the x86-64 box; `.vpr`/`.map` of the whole suite and the VIR
  self-test byte-identical to the baseline); `scripts/format.sh --check` keeps the layout in `check.sh`;
  `scripts/object_identity.py` stays for later refactors. The baseline ran in a second worktree (detached at C1),
  removed at the end; its outputs are in `build/s7/baseline/`. Audit reports `build/s7/audit{1,2}.md` and the evidence `build/s7/evidence/` in
  the worktree. M4.5d2b1 stacks on it (below).

- **M4.5d2v** (2026-11-04/05; record 0050, 0047 amended; plan `.claude/plans/m4-5d2v-allocator.md`, questions 129, 130,
  134–145): done on branch `m4.5d2v-allocator` (from `origin/main` `9255b93`) in the worktree `.claude/worktrees/m4.5d2v`;
  result in `internal/status.md`. vlibc's half is vlibc's local `main` `bba61ae..6e41fbe` (the allocator, its tests and CI,
  the Boost style), pushed by the owner (2026-11-05: the pin is in vlibc's `origin/main`); vlibm's `main` `1e6982e` (the
  code style in `CLAUDE.md`, NOT pushed). Audit reports `build/m4.5d2v/audit{1,2,3}.md` in the worktree (the probe worktrees
  were removed at the session's end; `m4.5d2v-allocator` is the one branch).

- **S6** (2026-11-04; record 0049; plan `.claude/plans/s6-tutorial.md`, questions 131–133): a tutorial
  (`doc/modules/ROOT/pages/tutorial/`, seven chapters) and a developer guide (`pages/dev/`, eight pages) as sections of
  the one docs site, the overview rewritten, and the website's landing page; done on branch `s6-tutorial` (from
  `origin/main` `bb41d55`, which holds PRs #27, #28, #29 and #30: M4.5d1, M4.5d2a and record 0048) in the worktree
  `.claude/worktrees/s6`. No code changed; the M4.5d2a follow-up below is still the next code work. The owner asked for
  the examples only, not the full suite (status has what ran). The website's commit is `cerastes-verifier.github.io`'s
  local `main` `45a3d2f` (§2 step 5).

- **On `origin/main`** (`f59e4b2`): everything through M4.5c2b (PR #26). **M4.5d1** (vlibc in cerastes; record 0046;
  plan `.claude/plans/m4-5d1-vlibc.md`) is PR #27 (`m4.5d1-vlibc`, tip `9d98fe8`), open at M4.5d2a's start; the local
  `main` `38eb115` is a local merge of it. vlibc's half of d1 is vlibc's `5962865` (pushed).
- **M4.5d2a** (the heap; record 0047; plan `.claude/plans/m4-5d2a-heap.md`, questions 120–127) is done on branch
  `m4.5d2a-heap` (from `9d98fe8`, so its PR stacks on PR #27) in the worktree `.claude/worktrees/m4.5d2a`; the loops were
  built on `m4.5d2a-loops` (worktree `.claude/worktrees/m4.5d2a-loops`, merged; removable). vlibc's half is vlibc's local
  `main` at `35e9b6f` (three commits on `5962865`), pinned through the submodule's `local` remote (`NOT YET PUSHED`).
  Result in `internal/status.md`. **The owner ended the session at 08:00 (travel): the second `check.sh` and the Linux
  rerun of the final tip were not run; CI decides (§2).** Audit reports: `build/m4.5d2a/audit{1,2}.md` in the worktree; a
  re-audit of the loops and of round 2's fixes (`build/m4.5d2a/audit3.md`) found no wrong verdict but one blocking item
  (B1, exit 3) and three should-fix items, not fixed before the session ended: §3 "M4.5d2a follow-up".
- **The Linux host** `matt@clang4.cpp.al` cannot bootstrap natively (no root; no libc++ 21, JDK 17, GCC 16 there): runs
  go in a container built from a clean `ubuntu:26.04` with the S5 recipe (`cerastes-m4.5d1:runner`, container
  `cerastes-m4.5d1-p71`, checkout `~/cerastes-m4.5d1/cerastes`; reused by d2a).

- **After the session** (2026-11-04): the owner chose vlibc's and vlibm's own names for their implementations
  (`vlibc_malloc`, `vlibc_strlen`, `vlibm_sqrt`), declared beside the standard names with the same contracts, instead of
  question 121's replacement allocator under the standard names (question 128, record 0048, accepted before d2v's code).
  §3's d2v notes, §4.2, §4.3's M4.6, `plan.md`'s d2v and M4.6 rows and vlibc's and vlibm's `CLAUDE.md` follow it; two
  recommendations of the record are open (§8, questions 129 and 130).

The EDG pin is unchanged (`f0ed5525af`); the fork's `cerastes` is 29 commits behind upstream `main` (§4.1).

## 2. Owner steps

The steps before M4.5d2v (PRs #27–#30, S6's website push) are in `git show 9255b93:internal/handoff.md` §2; the ones not
done yet still apply.

1. **vlibc** (done, 2026-11-05): `main` `6e41fbe` pushed; check its CI on both platforms (Linux needs
   `libclang-rt-21-dev` and the `vm.mmap_rnd_bits` step; the x86-64 box passed all six configurations in a container
   without it). **vlibm:** push its `main` (`1e6982e`, the code style in `CLAUDE.md`; independent of cerastes).
2. **M4.5d2v** (done, seen at S7's planning, 2026-11-05): merged into `origin/main` (`aa488d8`), and its worktree and
   branch are gone.
3. **S7** (`s7-code-style`, its planning commit `c2bc48d` included; `m4.5d2b1-results-locals` stacks on it: merge S7
   first): push the branch and open its PR. The PR tier
   selects no Silicon tests by `src/` changes, so dispatch the full check once (both jobs: the Linux job fetches the
   manylinux clang-format wheel for the first time). Merge it before M4.5d2b branches: its diff touches every line of
   `src/`. Then `git config blame.ignoreRevsFile .git-blame-ignore-revs` locally (GitHub reads the file on its own).
   The branch also holds `9cff0ad`, outside S7's plan: `doc/package-lock.json` deduplicated, since Dependabot's Antora
   3.2.2 merge (`aa488d8`) had nested six copies of `@antora/logger` and broken `build_docs.sh`'s failure on warnings
   (the harness self-test failed on `main`).
4. **M4.5d2b1** (`m4.5d2b1-results-locals`, stacked on S7): after S7 merges, push it and open its PR (it touches
   `src/`, so dispatch the full check once, both jobs). The Linux run on the x86-64 box was not done in the session: CI
   decides. Then remove the worktree `.claude/worktrees/m4.5d2b` (its `build/m4.5d2b/` holds the audits and the S38
   fixtures; copy anything wanted first).

## 3. Open notes, by the session that consumes them

One line per note, its origin in brackets (session and audit item); the full wording is in the commit named above.
Notes that a session consumes move into its plan's "Inputs" (M4.5b2b took the call's flag row, the registration rule for
P4222's markers, P54 and the b2b design points).


**From S7** (record 0051, audits `build/s7/audit{1,2}.md`)
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
- `include/cerastes.h`'s fail-closed `#error` (the cerastes front end without its attributes, record 0008 item 2) is
  one line again under `clang-format off`, now at line 42 (was 40); no test reaches it (reviewer round 1 S3). [S7]

*M4.5d1's notes (P55's allow-list, 0031 item 9's exemption) moved into its plan's Inputs. M4.5d2's notes moved into
M4.5d2a's plan's Inputs (`.claude/plans/m4-5d2a-heap.md`); those d2a hands on are below.*

*M4.5d2a's follow-up (B1, S1, S2, S3) moved into M4.5d2v's plan's Inputs (`.claude/plans/m4-5d2v-allocator.md`).*

**From M4.5d2v** (record 0050, audits `build/m4.5d2v/audit{1,2,3}.md`)
- *The d2b note (a vlibc block's spelling, the loop false alarms) moved into M4.5d2b1's plan's Inputs.*
- *The cerastes code style (question 146) moved into S7's plan (`.claude/plans/s7-code-style.md`).*
- **Costs to watch:** B1's splits against outside holders grow the paths exponentially with the number of holders (S37's
  adversarial fixture 93 s); a function with several pointer variables beside a heap loop may time out [audit 2 N10].
- **For M7:** vlibc's allocations are rejected in C++ ([intro.object]/10–13 create objects only for `std::malloc` and its
  family); placement `new` (and C++23's `start_lifetime_as`) lift it [0050 item 7].
- **For M4.5e2 / later:** `vlibc_realloc`, `vlibc_aligned_alloc`, `vlibc_free_sized`, `vlibc_free_aligned_sized` get
  contracts with their standard names'; `free_sized` needs a deallocation shape with a size [0050 item 6].
- **vlibc's core** is written to be verified after M4.6 (arrays, pointer arithmetic, char storage); the glue stays trusted.
  vlibc's run test of an allocating SIGABRT handler calls a function that is not async-signal-safe (test-only) [d2v].
- **S6's claims audit** is still not done (§3 "S6 notes").

**S6 notes** (record 0049)
- **The claims audit is not done** (owner, 2026-11-04: "I can have the soundness auditor run in a different session"):
  a read-only soundness-auditor pass over `bb41d55..s6-tutorial`'s pages (`pages/tutorial/`, `pages/dev/`,
  `overview.adoc`), the policy-list diff and the website's paragraph, for statements that are false or overclaim about
  behaviour, trust or a result's meaning; its fixes are doc edits plus a rerun of the changed examples. [S6]
- **Tutorial upkeep** (0049 item 4): M4.5e1 owes the arrays chapter (and quantifiers), M4.5e2 extends it with pointer
  arithmetic, M4.5f2 owes predicates and a linked list, M4.6 strings, M4.7 function pointers, M5 generic templates, M6
  requirement contracts, M7 references and classes. Every session that changes a message, a category or an accepted
  construct a chapter shows updates its prose and `console` blocks: the blocks are copied from a run in
  `doc/modules/ROOT/examples/tutorial/` (`../../../../../build/cerastes verify <file>`), and only review checks them. A
  check that reruns each shown command and compares the output is possible later work (0049, Consequences). [S6]
- **Stale facts found, not fixed** (no code changes in S6; S7's Part 3 fixes the first): `src/backend/register_attributes.h`'s header comment says
  "eight attributes" (it registers more: `decreases`, `owning`, `uninit`, `ref_to_uninit`, `must_init`, …); `plan.md` §0
  and the M5 plan name `internal/soundness/`, which does not exist (the developer guide says so; the arguments are in
  the decision records and `internal/encoding.md` §3); 0005 item 5 still describes the upstream-only submodule guard that
  0009 superseded. [S6]
- **`examples/dev/`** is in the policy lists but has no files: the developer guide quotes test headers in `text` blocks
  (0049 item 5). [S6]

*M4.5d2v's notes moved into its plan's Inputs (`.claude/plans/m4-5d2v-allocator.md`).*

**From M4.5d2b1** (record 0052, audits `build/m4.5d2b/audit{1,2,3,3b}.md` in the worktree)
- **Cut to d2b2:** written `owns_block` in loop invariants (record 0052 item 6(a), the plan's pre-agreed cut (a); tests kept
  in `build/m4.5d2b/d2b2-tests/`; pin `reject_owns_block_invariant.c`); audit S10's points apply (NOTAUTO guards, one
  carrier with an owning loop variable, the written sites in the exemption). [d2b1]
- **Documented false alarms, pinned, for d2b2:** a covered atom on a leaf of a `static` struct (`return &s.x;`,
  `static_struct_member_result_false_alarm.c`; covering a leaf broke `f__wf`, audit 2 N1); owning loop variables whose
  block becomes shared inside the loop (`owning_loop_becomes_shared_false_alarm.c`,
  `owning_loop_alias_becomes_owned_false_alarm.c`: a `permission` row, then a consequential `leak`) or stops being shared
  (`owning_loop_stops_shared_false_alarm.c`), because the carriers' guards are loop-entry snapshots (audit 3b). [d2b1]
- **Limits stated in record 0052:** a given amount is an unknown fraction to the caller (a callee hands back what it took
  by returning the partner); written atoms never cover (question 159); a bodiless declaration nothing calls is not
  checked against rule 2a (checking it changed 30 tests' Viper; audit 2 N3); `owns(**pp)` of a block stored in `*pp`
  needs d2b2's `owns_block(*pp)`; an owner freed inside a loop must be reset to `NULL`. [d2b1]
- **Notes not taken:** two reports for one fault in a few loop shapes (audit 3 N2: an owner freed through an alias and left
  non-null; N3: a `NULL`-head loop without `v == NULL` in its invariant; the docs say how to avoid both); an owner
  entering a loop with the other allocator's block is reported at entry even if the loop frees it first (audit 3 N4,
  documented). [d2b1]
- **Process:** the first il-analyst ran out of context after Part A and the owning locals outside loops; a fresh one (same
  role) did the loops and the audit fixes. Only one agent runs `run_tests.py` (a second run empties `build/vpr`). [d2b1]

**M4.5d2b2 owning globals, `static` locals and fields, P4418R0** (from M4.5d2a's planning; sources: P4222R3, P4416R0,
P4418R0, P4417R0, all 2026-10-22, read again at M4.5d2b1's planning; question 152 gives d2b2's planning the P4418
spellings, borrow against transfer on parameters and static owners at `main`'s end; d2b1's notes moved into its plan's
Inputs)
- P4417R0's `[[owner]]` on a parameter is a **transfer** (the callee must delete), where 0029 item 1 made
  `cerastes::owning` on a parameter a **borrow**; P4418 exempts static owners where 0029 item 1 has an owning global freed
  by the end of `main`. `[[not_invalidating]]` (a pointer parameter whose function does not invalidate, checked at the
  definition) and `no_dangling()` (a suppression, `std::forward` in P4417 8.4) are what d2b spells in `cerastes::`;
  P4417 keeps raw storage (`malloc`) out of the invalidation profile. C applies: raw pointers, structs with pointer
  members as containers, the escape rules, `[[owner]]` on variables, parameters, fields and results, `not_invalidating`
  on pointer parameters; references, iterators, smart pointers and `new`/`delete` are M7's.
- `alloc_<t>` does not ensure `block_of(r) != GB_g()`, so `&x != &g` is not proved in a function that holds nothing of
  `g`; d2a's typed views do not state it either (one rule; adding it changes the lint-pinned text and every program with
  cells and globals) [b2a audit 1 N3; to M4.8 if d2b does not need it].
- P4417R0's wording, read at d2b1's planning ([profiles.inval.owner]): p3 owner-to-owner transfers and leaves an alias,
  owner-to-non-owner aliases, an owner parameter takes the object, an owner result gives it; p4 an allocation's (there:
  `new`'s) or a call's owning result must initialize or be assigned to an owner, or be deleted, in its full-expression;
  p5 an automatic owner deletes or transfers on every path to its block's end (a static owner need not, Note 3); p6
  only an owner is deleted; p7 every alias of a transferred object is invalid. §8.2's drafting note keeps raw storage
  (`allocator::deallocate`) out of the profile. `not_invalidating` (9.13.u) appertains to a pointer parameter or a
  member function and is verified at the definition ([profiles.inval.verify]); `std::no_dangling(x)` returns
  `std::forward<T>(x)` and waives the escape rules for its result. d2b1 gives owning automatic locals 0029's meaning
  (a path-precise `leak`), which agrees with p5. [d2b1 planning]

**M4.5e1/e2 Arrays and pointer arithmetic:** P38's braced initializers (§4.2); signed/unsigned and enumeration variants
through `void*` cannot be written [M4.5a]; accepted completeness gap for e2's relational comparisons: `&s.x < &s.y` is
reported, since members are different blocks [M4.5a].

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

## 4. Session plans

**Rules for every session** (memory `session-size-few-hours`): sized to what fits in one context, under about 12 h
unless a reason is given (owner, 2026-10-30, question 77; was ≤ 4 h), with fixed scope and stop rules; owner decisions and pre-approved expectation changes asked up front; the
decision record before code (invariant 6); must-verify/must-fail pairs (invariant 2); docs in the same session
(invariant 8); three Sonnet agents with disjoint files (edg-backend: `src/backend`, `src/driver`, build; il-analyst:
probes, IL evidence, Silicon spikes, `src/vir` + `src/viper`; test-mutator: tests, harness, compiler matrices, `mutate.py`,
`lint_viper.py`) and the Opus soundness-auditor (one round per diff plus a re-audit of blocking fixes; should-fix items
not taken go to §3); single-file runs while working (`build/cerastes verify` directly: `run_tests.py` locks `build/vpr`);
builds serialized (`lockf build/<session>/build.lock cmake --build build/cmake`); `check.sh` at most twice; the EDG
suites on the x86-64 host are their own session, but any x86-64 test run a session needs goes to `matt@clang4.cpp.al`
automatically (owner, 2026-10-29: not Docker multi-arch on the Mac, which is only the fallback when the host is down).
Record numbers are the next free ones when written (0047 is M4.5d2a's, 0048 and 0049 are taken, 0050 is M4.5d2v's, 0051 is S7's, 0052 is M4.5d2b1's). The
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
session start): the fork's `cerastes` is 29 commits behind upstream `main` (`a0325e00e6`, fetched 2026-10-30 at
M4.5b1c3's start); merge it
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

**M4.5b1 and M4.5b1c** were split by the owner (2026-10-28 and 2026-10-29): b1a, b1b, b1c1 and b1c2 are done (records
0032–0039; plans in `.claude/plans/`); owning results (`owning` on functions, `owns_block`, `owns(*result)`, `ensures`
atoms with no partner) moved to M4.5d2. Each session rejects what a later one makes precise (fail closed).
**Pre-approved** (owner): a session may retire or re-expect exactly the rejection pins listed for it below (or in its plan, when the
paragraph points at one), and no others; a pin whose construct stays rejected keeps its `expect:` line, and only its message and `Pins:` comment name the
new session.

**M4.5b2 is two sessions** (split at M4.5b2a's planning, 2026-10-30). **M4.5b2a** (globals and `static` locals) is done
(record 0041; plan `.claude/plans/m4-5b2a-globals.md`; flips of question 81). **M4.5b2b** (P4222's markers
`[[cerastes::uninit]]`, `ref_to_uninit` and `must_init` on parameters, `--strict-init` with the `// verify-options:`
header, the call's flag row as `uninitialized`) is done (record 0042; plan `.claude/plans/m4-5b2b-init-markers.md`; the
six pre-approved self-test pins of questions 82 and 85; question 89's loop row in every function).

**M4.5c is two sessions** (owner, 2026-11-02, question 90). **M4.5c1** (struct objects; record 0043) is done (plan
`.claude/plans/m4-5c1-struct-objects.md`; flips of questions 93 and 94): struct types and member blocks (0028 item 16), struct locals as blocks, `.`,
`->`, nested structs, pointers to members, braced initializers, copies inside a function (each member read, question 92),
`struct S *` objects, contract atoms and values on struct places, P4222's markers on struct objects; its flips are
question 93's; ports `malloc_block.c` (its `free(&local)` twin waits for M4.5d2) and `swap.c`'s `point_mirror`.
**M4.5c2 is two sessions** (owner, 2026-11-02, question 95). **M4.5c2a** (struct values and struct storage; record 0044;
plan `.claude/plans/m4-5c2a-struct-values.md`) is done: by-value struct parameters and results (a struct value as the tuple of its
leaves), `f().x`, C++ `S{…}`/`S()` as values, compound literals (question 96), struct globals and `static` struct locals,
`decreases(p->x)`, `restrict` beside by-value structs; ports `call_in_struct_init.c` and `struct_assignment.c`.
**M4.5c2b** (places through a heap read; record 0045; plan `.claude/plans/m4-5c2b-heap-read-places.md`, questions
104–112) is done: `owns(**pp)`, `reads(**pp)`, `live(*pp)` and reads of `**pp`, `*gp` and `owns(*gp)`, `p->q->x` and `*p->q`
through a pointer member, an atom on `*q` where `q` is a cell, atoms and reads through a by-value struct's pointer member,
`*r.q`, written struct atoms in loop invariants (question 99), the `restrict` re-check; `live(r.q)` moved to M4.5d2
(question 106). M4.5d2 needs it (owning fields point through a heap read).

**M4.5d1 vlibc** (record 0046; plan `.claude/plans/m4-5d1-vlibc.md`, questions 113–118) is done: the pin, the
include order, the library class, trust from vlibc's tags, reserved names, attributes by position, the redirection
rules; d2 is not rolled in (question 113), its preparation is below.

**M4.5d2 is three sessions** (owner, 2026-11-03, questions 120–122). **M4.5d2a** (the heap; record 0047; plan
`.claude/plans/m4-5d2a-heap.md`) is done (questions 120–127; the final check is CI's, §2): vlibc's contracts for `malloc`, `calloc` and `free`, `void*` at an
allocation and at `free` only, one object per allocation (a typed view, size `sizeof(T)`), heap blocks with the slot −1
token, `invalid-free`, leaks by `forperm` (`leak`; `main`'s return included, question 123), `owns_block` in contracts,
`owning` and `ref_to_uninit` on functions (target `r`), two calls ordered over liveness and token cells, blocks held
through locals across loops (landed; one pinned false alarm, §3); `realloc` to M4.5e2. **M4.5d2v** (vlibc's allocator, in vlibc's repository,
right after d2a, with cerastes's half): the allocator under vlibc's own names of record 0048 (question 128; §3). **M4.5d2b** (after d2v): the rest of the paragraph
below that d2a does not take, **split in two** at its planning (owner, 2026-11-05, question 152; the surveys measured
4,500–7,000 changed lines). **M4.5d2b1** (record 0052; plan `.claude/plans/m4-5d2b1-results-locals.md`): `ensures`
atoms with no `requires` partner, covered by a returned atom or given with an output amount (`owns(*r)`, `live(r)`,
`live(r.q)`, `owns(**r)`, a block site on a formal); owning automatic locals (`leak` at lifetime end and on
reassignment; the loop false alarms lifted for owning variables); `ref_to_uninit` on automatic locals and P4222R3's
strict rules (questions 154, 155); the d2a/d2v leftovers (written `owns_block` in loop invariants, block-site guards that
read the heap, `owning` beside `owns_block`, `restrict` beside a block-only parameter, `two((free(p), 0), p)`);
`cerastes_owns_vlibc_block` (question 153). **M4.5d2b2** (after d2b1): block sites through loads and on globals, owning
globals, `static` locals and address-taken locals, acyclic owning fields (target `d`), and the rest of P4418R0 (its
planning decides the `cerastes::` spellings of `not_invalidating` and `no_dangling`, a strict profile mode, borrow
against transfer on parameters, and static owners at `main`'s end; §3). The ports this paragraph names were all done by
d2a. The original d2 paragraph and its dependency map follow.

**M4.5d2 `malloc` and `free`.** vlibc's contracts (`malloc` `ref_to_uninit`, `calloc` initialized, `realloc`, `free`),
heap blocks with the slot −1 token, the free checks (`invalid-free`), the leak rule by `forperm` and a record on `main`'s
leaks (`leak`), owning locals and globals (`[[cerastes::owning]]` target `v`), `[[cerastes::ref_to_uninit]]` on pointer
locals and functions (targets `v`, `r`: `malloc`'s result and a local receiving it; from M4.5b2b's planning), and
**owning results** (from b1c, owner
2026-10-29): `[[cerastes::owning]]` on functions (target `r`), `owns_block`, `owns(*result)` and `ensures` atoms with no
`requires` partner (a callee handing a place's permission to its caller: an output ghost amount), `live(r.q)` of a
struct result's pointer member included (from M4.5c2b, question 106; pin `reject_struct_value_live_result.c`). **Owning fields** (from M4.5c,
owner 2026-11-02, question 91): `[[cerastes::owning]]` on struct fields (target `d`), owning the struct holding
`owns_block` of the field's pointee when it is not null (0029 item 1; acyclic here, recursive in M4.5f2). **The rest of
P4418R0's invalidation profile** (owner, 2026-11-02, question 91; Stroustrup, "An invalidation profile", 2026-10-22,
https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4418r0, wording in P4417R0): its guarantees (a pointer is
dereferenced only if it points to an existing object; no pointer escapes beyond the lifetime of what it points to; a
non-static owner is deleted exactly once), `[[owner]]`'s rules (§8: an owner must be freed or passed on to another owner; a
non-owner must not be freed; owner to owner transfers the obligation and leaves an alias; owner to non-owner makes an
alias; an allocation's result is an owner), `[[not_invalidating]]` (a function that does not invalidate an argument,
checked at its definition) and `no_dangling()`; d2's planning decides their `cerastes::` spellings (as 0029 did for
P4222) and what each adds over contracts that return their permissions; C++'s parts (references, containers, iterators,
`new`/`delete`, smart pointers) are M7's. Flips:
`tests/memory/negative/reject_{atom_on_result,ensures_atom_no_partner}.c`. Ports: `account.c`, `deposit.c`, `swap.c`'s
`main`, `globals.c`, `malloc_block.c`'s `free` twin.

*d2's dependency map* (M4.5d1's planning survey, question 113; nothing heap-specific exists in the code yet: no token,
`owns_block`, `forperm`, `leak` or `invalid-free`; the Viper is pinned by 14 smoke fixtures in `tests/viper-smoke/memory/`,
`e2_malloc_free.vpr`, `s25_c_leaks*.vpr`, `s25_f_owning*.vpr`, `s26_*`): (1) vlibc's malloc-family contracts (needs d1);
(2) `void*` per 0028 item 1 (allocation results, `free`'s argument, re-expecting `reject_void_pointer.c`,
`reject_void_to_int_pointer.c`); (3) heap blocks, the slot −1 token and the typed view of allocated storage
(`malloc(sizeof(struct S))`'s member lengths and `WF_S`; `alloc_<t>`'s `block_of(r) != GB_g()`); (4) the free checks and
`invalid-free`; (5) `owns_block`; (6) leaks by `forperm`, `leak`, and a record on `main`'s blocks at exit; (7) `ensures`
atoms without a `requires` partner, an output ghost amount (no record designs it: it must fit 0039 item 7's partner
row, 0045 item 7's `ite` fallback and 0037 item 8's return sums); (8) `owning` target `r`; (9) `owning` target `v` (locals
and globals: `leak` at lifetime end and on reassignment); (10) `ref_to_uninit` targets `v` and `r` (and 0042's
`internal`-row re-check); (11) acyclic owning fields (target `d`); (12) two-call sequencing over liveness and token cells
(0028 item 13); (13) the rest of P4418R0 (and the P4222R3 check against 0029 and 0042); (14) ports and flips. Chains:
d1 → 1; 2 → 1 → 3; 3 → 4 → 12; 3 → 5 → 6, 8, 9, 11; 7 → 8 → 10; 6 → 9; 8, 9, 11 → 13; all → 14. Comparable sessions
changed 1,800–3,200 source lines. **Proposed split** (question 113, for d2's planning to confirm): **d2a, the heap**:
items 1–6, 12 and owning results as `owns_block` (an `owns_block` in an `ensures` is a whole-block `write` permission,
needing no ghost amount); **d2b, results and fields**: item 7 (`owns(*result)`, `live(r.q)`, `owns(**r)`), items 9–11 and
13. M4.5d1's P71 (d) records how each platform declares `malloc`, `calloc`, `realloc` and `free` (attributes, `noexcept`,
`decl_position`).

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

## 7. Acceptance

Each session's acceptance commands are in its plan (`.claude/plans/`); the results of finished sessions are in
`internal/status.md` (the latest entry) and the commits its history names.

## 8. Questions for the owner

Each has a recommended answer. Answer inline, as before. Answered questions 1–84 are recorded in the decision records,
the session plans and the handoffs reachable from the one named at the top. The latest: 65 (`return &local;` rejected
like the compilers), 66 (the cost of aliasing parameters, M4.8), 67 (the `owns`+`reads` rule path-precise), 68 (b1c3
may re-expect the three pinned false alarms of reassigned parameters, §4.2); at M4.5b1c2's planning (2026-10-29): 69
(the upstream EDG merge and the pin move wait for the box session, §4.1), 70 (unsequenced accesses that name one object
are rejected like the compilers, `unsequenced_modification`; the shapes that depend on aliasing are path-precise
`unsequenced`), 71 (b1c2 has no time cap and no cuts), 72 (after audit round 1's S7: an `ensures` atom is returned only
where its `requires` partner was taken, checked in `f__wf` for every contract, record 0039 item 7); at M4.5b1c3's
planning (2026-10-30): 73 (atoms in a `do … while (0)` invariant stay rejected, like atoms in a `check`), 74
(`owns(**pp)` and reads of `**pp` in a contract move to M4.5c), 75 (`owning` beside an explicit `owns(*p)` or `p == NULL
|| owns(*p)` on the same parameter is merged; `reads`, `live` or another guard beside it is rejected), 76 (pointers to
pointers at any depth), 77 (sessions sized to one context, under about 12 h unless a reason is given).

At M4.5b2a's planning (2026-10-30): 78 (`const` objects are modelled in M4.6, the recommendation; §4.3), 79 (written
atoms on moved pointers are fixed in M4.8 with question 66, the recommendation; §4.3), 80 (a `static` local's
permission reaches its function by an implicit `owns` in its contract and in every caller's, `main` from
`program_start`), 81 (M4.5b2a's flips, §4.2), 82 (the call's flag row becomes `uninitialized`, in M4.5b2b, §4.2), 83
(under `--strict-init` an initialized pointee passed to a `ref_to_uninit` parameter is a static rejection, M4.5b2b,
§4.2), 84 (initial values by linkage: internal linkage and C's external linkage get the initializer's value, C++'s
external linkage an unknown one; M4.5b2a's plan).

At M4.5b2b's planning (2026-11-02, each the recommendation; recorded in its plan and record 0042): 85 (every call flag
row becomes `uninitialized`, global and `static`-local atoms included; `selftest_cases12.inc:113, :206, :283` join
question 82's pre-approved re-expected pins), 86 (contract reads of a marked parameter's pointee are rejected statically,
`*p` in a `must_init` parameter's `ensures` excepted), 87 (after a `ref_to_uninit` call an initialized pointee stays
initialized: `old(flag) ==> flag`, amending 0028 item 14), 88 (a marker needs an `owns(*p)` atom, explicit or from
`owning`, and `must_init` one in the `ensures` too; `reads(*p)` beside a marker is rejected). During M4.5b2b, after its
audit round 2: 89 (the loop row `held && flag ==> range` on every place a loop does not reach, in every function: it
changes the Viper of existing programs with such loops, an added invariant line and no verdict, and fixes 0038's exit 3
on nested loops; re-expecting a VIR self-test pin pre-approved only where its row count at a loop grows by exactly the
added rows, which none needed).

At M4.5c1's planning (2026-11-02; 90, 92 and 93 the recommendations; recorded in its plan and record 0043): 90 (M4.5c is
two sessions: c1 struct objects with copies inside a function, c2 struct values, struct globals and places through a heap
read; copies are the pre-agreed cut to c2), 91 (owning fields move to M4.5d2, which also covers the rest of P4418R0's
invalidation profile), 92 (a struct copy reads every member: an uninitialized one is `uninitialized` at the copy), 93
(c1's flips: `reject_struct.c` retired, `global_struct.c`'s `@ L8` line dropped, seven pins re-messaged). During
M4.5c1: 94 (the new must-fail test `struct_bodiless_frame.c` drops its `check @ L22` line: after a call to a function
whose contract fails framing, the caller's state is not checked further, 0037 item 7).

At M4.5c2a's planning (2026-11-02, each the recommendation; recorded in its plan and record 0044): 95 (M4.5c2 is two
sessions: c2a struct values and struct storage now, c2b places through a heap read next), 96 (compound literals: in C
values and `&(T){…}` as a hidden local of the enclosing block, in C++ values only; storage-class and file-scope literals
rejected), 97 (passing and returning a struct by value read every member, like a copy: a missing member is
`uninitialized` at the call or the return), 98 (c2a's flips), 99 (written struct atoms in loop invariants move to c2b,
their pin re-messaged), 100 (struct `?:`, comma and assignment values stay rejected, naming M8). During M4.5c2a: 101 (the
VIR self-test rejection cases `struct_param_by_value` and `struct_result_by_value` retired for lowering cases), 102 (the
expectations of new tests corrected after their first run, listed in the status entry), 103 (the VIR case
`sglob_static_unheld`'s expected row index 0 → 3, and the VIR self-test step re-run alone instead of a third `check.sh`).

At M4.5c2b's planning (2026-11-03, each the recommendation; recorded in its plan and record 0045): 104 (a place reached
through a heap read in an `ensures` is the object the path designates on return; `old(**pp)` the entry value of the entry
object), 105 (places the `requires` reaches through a heap read are automatic places of every loop, addressed by a ghost
`Ref` snapshot), 106 (`live(r.q)` moves to M4.5d2 with every `ensures` atom without a `requires` partner), 107 (c2b's flips). The same
day, after the design review: 108 (amounts follow objects: an `ensures` atom through a load returns the amount of the
`requires` atom whose entry place equals its return place; not the recommendation), 109 (guards that read the heap are
accepted in c2b; not the recommendation), 110 (both snapshots, at method entry and at loop entry, where they can differ). During
the session: 111 (a call whose contract names a place reached through a load, beside another access in one expression,
is accepted with a pre-call snapshot of the place), 112 (nine new twins gain duplicate same-line `permission` expect
lines for sibling liveness rows).

At M4.5d1's planning (2026-11-03, each the recommendation; recorded in its plan and record 0046): 113 (d1 alone, with
d2's preparation: the malloc family's platform declarations measured and d2's dependency map with a d2a/d2b proposal in
§4.2; d2 not rolled in), 114 (reserved names are generated from the pinned vlibc headers: the names they declare with a
contract), 115 (a contract on a reserved name outside vlibc is always rejected, `annotation_reserved_library`, even when
identical to vlibc's; amends 0030 item 4). During the session, after audit round 1: 116 (an identical contract after
vlibc's header is accepted: the front end discards it), 117 (definitions of reserved names and contracts on them are
rejected for C linkage in any namespace and, in C++, any linkage in the global namespace; four pins of 0015 item 5
re-expected); after the first runs of the new tests: 118 (the two alias pins re-expected `unsupported_attribute`), 119
(the three glibc `assert` near misses also expect `side_effect_not_evaluated`).

At M4.5d2a's planning (2026-11-03, each the recommendation; recorded in its plan and record 0047): 120 (d2 is split: d2a
the heap now, `realloc` to M4.5e2, loops the first cut), 121 (vlibc replaces the platform's allocator: standard names, a
portable C23 core, pages from `mmap`, glibc's replacement set, a default malloc zone on macOS, thread-safe; the owner's
directive that vlibc holds complete implementations of the functions it covers, verified as cerastes becomes able to,
amending 0030 item 2), 122 (that allocator is M4.5d2v, a session of its own right after d2a), 123 (blocks still held when
`main` returns are leaks, like VeriFast; a path ending in a call that never returns is not checked). During the session: 124
(`reject_owning_function.c` expects `error lifetime @ L8`: its `p` has no contract, so returning it fails the liveness
check before the missing token; the plan had pre-approved `permission`), 125 (two new pins re-expected to what they can
reach: a function template `unsupported_declaration`, a lambda EDG's `wrong_entity_for_attribute`), 126 (the template pin
also expects `unsupported_annotation @ L8`; the discarded-`malloc` pin lists EDG's two `call_result_should_be_used`
warnings, its rejection at L13; the member-function pins `unsupported_declaration @ L7`), 127 (and `unsupported_annotation
@ L9`); and at 07:15 that the session ends at 08:00 with CI as the final check.

After M4.5d2a (2026-11-04; record 0048): 128 (vlibc's and vlibm's implementations are named `vlibc_<name>` and
`vlibm_<name>`, declared beside the standard names with the same contracts: reading (a), the contract shared, not (b), a
`#define` or link-time rename of the standard names; question 121's replacement allocator, malloc zone and replacement set
are dropped).

S6 (2026-11-04; record 0049): 131 (the docs session runs before the M4.5d2a re-audit's B1/S1 fixes; it touches no
encoder code), 132 (a minimal landing page on the website: one paragraph and links, no code), 133 (a tutorial chapter
lands with the session that completes its concept; every session keeps the existing chapters true). At planning the owner
also agreed to the layout (the tutorial and the developer guide in this repository's `doc/`, the website the front door)
and, during the session, asked for the examples only, not the full suite.

At M4.5d2v's planning (2026-11-04, each the recommendation; recorded in its plan and record 0050): 129 (blocks know
their allocator: set by the allocating routine's name class, required by the deallocating one, a mismatch `invalid-free`;
`owns_block` keeps meaning the standard allocator; the pin moves only with the rule), 130 (the four functions whose standard
names have no contract are declared contract-less and untagged; cerastes skips such a library routine unless called, a call
`call_without_contract`), 134 (the M4.5d2a re-audit's B1–S3 first), 135 (S3 fix A: an automatic `internal` loop invariant
keeps assigned pointer locals well-formed in heap programs, the free row's `WF` `internal` again;
`heap_loop_swap_false_alarm.c` loses exactly `error permission @ L38` and `@ L39`, pre-approved), 136 (one session, split
point: vlibc's run, thread and sanitizer tests not green once cerastes's half passed its audit → ask), 137 (one global
`pthread_mutex_t` in the trusted glue), 138 (`vlibc_malloc(0)` a unique block; `vlibc_realloc(p, 0)` NULL, `p` kept), 139
(errno as POSIX.1-2024: `ENOMEM`, `EINVAL` for an unsupported alignment and `realloc(p, 0)`, frees keep errno), 140 (vlibc
allocation views rejected in C++ until M7: [intro.object]/10–13 create objects only for `std::malloc` and its family), 141
(`vlibc_free_sized`/`vlibc_free_aligned_sized` always check and abort on a mismatch), 142 (macOS GCC: UBSan only if its
ASan does not run).

During M4.5d2v (2026-11-04/05): 143 (the two Part 1 twins redesigned to fail for one cause after their first run, the
expectations then set from the redesigned runs), 144 (the free-and-reallocate exit 3, older than d2v, fixed in the session:
ending back-edge rows and value rows; not the recommendation, which was to defer it), 145 (its correct program pinned as a
false alarm with the encoder's errors, plus the platform twin). Also, as directives: vlibc's and vlibm's code follows the
owner's Boost libraries' style (int128, decimal, charconv), and cerastes's own refactor "may need its own session" (146).

At S7's planning (2026-11-05; recorded in its plan `.claude/plans/s7-code-style.md` and, in the session, record 0051):
- **146**, the cerastes code-style session: "Follow recommendation". It is a session of its own (S7), after M4.5d2v
  merged and before M4.5d2b. It is proved behaviour-neutral by byte-identical `.vpr`/`.map` over the whole suite and the
  VIR self-test, a `check.sh` step keeps it, and `CLAUDE.md` and the developer guide are updated.
- **147** widened 146's "mechanical": the full style sweep. It adds brace initialization (lambdas keep `=`, as in
  decimal), `U` on unsigned literals and one declarator per declaration to clang-format's layout. Each commit is proved
  by byte-identical object files (`-g0 -DNDEBUG`). This was not the recommendation, which was the layout plus
  declarators, with `U` and braces for new code only.
- **148**: the pinned clang-format is PyPI's `clang_format` wheel 23.1.3 on both platforms, with a SHA-256 for each,
  not Homebrew's floating llvm. Ubuntu 26.04 has only clang-format-21, so the two CI jobs would disagree.
- **149**: lambda bodies are Allman.

During S7: no question was put to the owner. The reviewer's items and their dispositions are in
`build/s7/audit{1,2}.md`; the plan's acceptance was worded as the proof stands (`encode.cpp`'s objects differ by the
two `__LINE__` immediates EDG's assertions receive, so its IR is compared; no `-DNDEBUG`).

At M4.5d2b1's planning (2026-11-05, each the recommendation; recorded in its plan and record 0052): 150 (lines over
120 columns are re-wrapped as they are touched), 151 (the 132 `{ // comment` lines stay until touched; the owner added:
"in the future I don't want comments on the same lines as braces": no new or touched line puts a comment on a line with
a brace, `CLAUDE.md` and the developer guide's "Code style"), 152 (M4.5d2b is split: d2b1 partnerless results, owning
automatic locals, marked locals, P4222R3's strict rules, the d2a/d2v leftovers and the vlibc spelling; d2b2 block sites
through loads, owning globals, `static` locals and fields, and P4418R0, whose planning decides its spellings, borrow
against transfer and static owners), 153 (`cerastes_owns_vlibc_block(p)`, C only: a block of vlibc's allocator; `owns_block`
and `owning` keep naming the platform's), 154 (under `--strict-init` a `ref_to_uninit` function returns a marked local, a
`ref_to_uninit` call or a null pointer constant; anything else is rejected; null departs from P4416 p6), 155 (`must_init`
stays parameter-only; EDG keeps refusing it on a local). The owner also asked for the branch on top of S7's (not merged
yet), three Sonnet agents and one Opus agent from `.claude/agents/`, and a stop for any question.
During M4.5d2b1 (each the recommendation; recorded in its plan and record 0052): 156 (a covered atom's output amount is
`write`: it grants nothing), 157 (a covered `live` gives pointer use only; reads need `owns`/`reads`), 158 (a
partnerless atom in a contract without a body only on a fresh result, else rejected naming M6; closes a hole open since
d2a), 159 (only implicit atoms cover: `&&` between written atoms stays separating, as VeriFast's `&*&`), 160 (a new test's
expect line may be corrected after its first run when it fails for its designed reason at another line or a neighbouring
category, listed in the status entry), 161 (two flipped pins expect two real errors each).

**Open, for the next session:** none. M4.5d2b2's planning decides its own questions (P4418R0's spellings, a strict
profile mode, borrow against transfer on parameters, static owners at `main`'s end; §3).

