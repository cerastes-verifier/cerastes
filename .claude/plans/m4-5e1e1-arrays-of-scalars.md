# Plan: M4.5e1e1 — Arrays of scalars in structures and static storage (record 0063)

**Written on 2026-11-25 at a session that only planned.** It supersedes section 7 of
`.claude/plans/m4-5e1-arrays.md` ("plan mode again after P82 and S47"): **e1e1 does not plan again.** Its probes
and spikes run first (section 6), each with a decision table that says what is built for each outcome.

**M4.5e1e is two sessions** (owner, 2026-11-25, question 290): the planning's surveys put all of e1e at 3,800–7,200
`src/` lines, against the 3,654 that e1d landed in 11.5 hours.
- **M4.5e1e1** (this plan, in full): arrays whose elements are scalars, in the places e1a left out — globals and
  `static` locals, members of structures, compound literals, and the array types the front end computes.
- **M4.5e1e2** (section 14: scope and inputs; a short planning of its own from what e1e1 built): arrays whose
  elements are structures or arrays.

**Written for this ensemble** (owner, 2026-11-25): an Opus orchestrator; three Sonnet implementers from
`.claude/agents/` (`edg-backend`, `il-analyst`, `test-mutator`); the Opus `soundness-auditor`; the Fable advisor.
Section 2 assigns every task; the orchestrator does not re-divide the work. **The session starts outside plan
mode**, from the opening prompt in `internal/handoff.md` §2, and reads the records this plan names, not all of
`internal/decisions/`.

**Two things differ from e1d's plan, both the owner's (2026-11-25):**
- **Questions go to the Fable advisor first** and to the owner only if the advisor cannot settle them (section 12).
- **A test budget** (section 3): the owner believes a wasteful amount of testing is occurring. Every test run this
  session makes is in section 3's table with its reason; a run that is not there is not made.

Branch `m4.5e1e1-arrays-of-scalars`, worktree `.claude/worktrees/m4.5e1e1`, `build/m4.5e1e1/`, record
`internal/decisions/0063-arrays-of-scalars-in-structures-and-storage.md` (the next free number: if M4.5e1f has run
first, the one after its record, and "0063" below reads as that number). The plan holds whether M4.5e1f lands
before or after: e1f changes the rows `call_atoms` writes for a range atom, which this session reuses and does not
change.

## 1. Context

**Why.** e1a made automatic local arrays of scalars objects, e1b gave ranges of them to contracts, e1c and e1d
quantifiers and `pure` functions over them. Every other array is still rejected with a diagnostic that names
M4.5e1e (28 messages in `src/backend/encode.cpp`, 22 pins in `tests/`): a global table, a buffer in a structure,
`typedef int vec3[3]`, `sizeof(int[3])`. Record 0028 item 16 fixed their layout in M4.5a (an array member is the
block `member(r, k)` of length n) and S26 measured it; nothing of it is built for arrays.

**Owner decisions.**
- **290** (2026-11-25): M4.5e1e is cut "scalars, then structures": e1e1 and e1e2 as above.
- **291** (2026-11-25): a structure with an array member is copied (`t = s;`, `struct S t = s;`) **and** passed and
  returned by value in M4.5e1e ("Copy and by-value now"; the recommendation was the copy as a first cut and
  by-value in M4.5f3). They are this plan's **Part E**, its last part: section 8's split point 2 decides whether
  Part E runs in e1e1 or opens e1e2 (either way it is in M4.5e1e).
- **226–233, 236, 237** (M4.5e1's planning): section 1 of `.claude/plans/m4-5e1-arrays.md`.
- **Standing:** 69 (no EDG work), 77 (one context, under about 12 h), 160 (a new test's expect line may be
  corrected after its first run, each listed in status), 176 (the owner handles CI), 285 (a test that is slow on
  the box only because of what it must refute goes to `tests/long/` with a short default-tier twin), 277 (a twin
  with several real errors lists every one); `check.sh` once, at the end, on the x86-64 box; every test run on
  the box; no test runs while a fix is queued; no Claude trailer; commits stay local.

**Scope.** Below, a **scalar** is `bool`, an integer type, an enumeration or a pointer to an object type (0059
item 1's element types); an **array** is one dimension of scalars with a constant bound of at least 1.

- **In:**
  - **(G) Global and `static` local arrays:** definitions with and without a braced initializer; `g[i]` read and
    written, `&g[i]`, the decay of `g`, `sizeof(g)`; initial values by linkage (0041 item 3); `owns(g)` and
    `reads(g)` (every element), element reads in contracts, `old`, `decreases`; the implicit atoms of a `static`
    local array; loops; `program_start`; `g` passed to a callee's `owns_range`/`reads_range`; a named global
    array as an argument of `cerastes_forall` and of a `pure` function in a loop invariant or a `check`.
  - **(M) Array members:** `struct S { int a[4]; int n; }` in every storage 0043 and 0044 give a structure (a
    struct local, a `struct S *` object, a struct global and `static` struct local, a nested member `s.t.a[i]`);
    `s.a[i]`, `p->a[i]`, `&s.a[i]`, the decay of `s.a` and `p->a`, `sizeof(s.a)`; braced initializers with a
    nested list for the member; `owns(*p)` and `reads(*p)` (the array's elements with the other leaves),
    `owns(p->a)` and `reads(p->a)` (the whole array member), element reads `p->a[i]` in contracts; calls,
    returns, `f__wf`, loops, scope exits, sequencing.
  - **(L) Array compound literals in C:** `(int[]){1, 2, 3}` as an object of its block.
  - **(T) Array types the front end computes** (0024 item 6, for arrays of scalars): typedefs and aliases;
    `typeof` and `decltype`; `sizeof` and `_Alignof` of such types; a bound that names a local constant if P82
    shows it can be read.
  - **(V) Structure values with array members** (question 291; Part E): the whole-structure copy inside a
    function; by-value parameters and results; `f().a[i]`; compound literals of such structures.
- **Not here** (rejected, each pinned, the message naming where it is lifted):

| Construct | Lifted by |
|---|---|
| an array of structures (`struct S v[3]`, `p[i].x`, a range over structures); an array of arrays (`int m[2][3]`, `int a[][3]`, `sizeof(int[2][3])`); an array member whose elements are structures or arrays | M4.5e1e2 |
| `cerastes_forall` and `pure` applications whose pointer argument is a member (`p->a`, `s.a`), or a global array in a contract; structure members under a binder | M4.5e1e2 |
| heap storage of a structure with an array member (`malloc(sizeof(struct S))`, `owns_block`, `free`) | M4.5e1e2 (question 292) |
| a structure with both an array member and an `owning` or marked member; `ref_to_uninit` and `must_init` on a pointer to a structure with an array member; a written atom in a loop invariant on a structure place with an array member | M4.5e1e2 (questions 293, 294) |
| a range on part of a global array or of an array member (`owns_range(g, n)`, `owns_range(p->a, n)`, `&g[2]` as a base); an atom on one element; `&g[i]` and `&p->a[i]` in a contract; a global initializer `&g[2]`; `g + i` | M4.5e2 |
| a structure value that holds more than 64 array elements, passed or returned by value (question 295; a copy has no limit) | M4.5f3 (array values as ghost sequences) |
| a read `pure` function over a structure or a global (0062 item 1, 0041 item 12) | M4.5f1 |
| `const` arrays in any storage (a `const` global table, `static const`); `char s[] = "abc"`, `sizeof("abc")`, `sizeof((char[4]){0})` | M4.6 |
| an array of owning pointers; `owning` or `ref_to_uninit` on an array | M4.8 |
| array designators (`{[2] = 5}`), VLAs, a zero-length array, flexible array members, a pointer to an array (`int (*p)[3]`, `sizeof(char(*)[4])`), `[static N]`, `thread_local` arrays, a block-scope `extern` array, `__datasizeof`, `_Alignof` of an expression, a typedef of a function type with an array parameter | M8 (function types: M4.7) |
| an array compound literal in C++ (a GNU extension), at file scope, `const`, or in a loop condition, a `for` step or a `do` condition (0044 item 7's list) | M8 |
| array types as arguments of type traits and variable templates (`__is_same(char[4], char[4])`); `std::array<char, 4>` | M5.0, M5 |

**Size** (`src/` and `include/` insertions without the VIR self-test cases; from the planning's three surveys of
the tree at `68384d1`, each compared with the commits of e1a, e1b, c1, c2a and b2a; e1c landed 60% over its
estimate and e1d inside its own, so read the tops as likely):

| Part | Content | `encode.cpp` | `lower.cpp`, `check.cpp`, `vir.*` | Sum |
|---|---|---|---|---|
| A | plumbing, behaviour-neutral | — | 200–350 | 200–350 |
| B | (G) global and `static` arrays | 300–550 | 570–1,040 | 870–1,590 |
| D | (T) computed array types; (L) array compound literals | 240–550 | 80–215 | 320–765 |
| C | (M) array members | 350–650 | 600–1,120 | 950–1,770 |
| E | (V) structure values with array members | 250–450 | 350–600 | 600–1,050 |

**2,340–4,475 for Parts A–D; 2,940–5,525 with Part E.** Beside it the lint: 500–900 lines with 600–1,000 in its
self-test. Parts A–D are an e1d-sized session; Part E is expected to run in e1e1 only if they land near the
bottom (section 8's split point 2).

## 2. The ensemble

| Role (model) | Owns | Never touches |
|---|---|---|
| **Orchestrator** (Opus) | Steps 0–1; record 0063; the vprdiff verdict per part; user pages in `doc/`; status, handoff, `plan.md`'s Done row; every commit; briefing and resuming the agents; `build/m4.5e1e1/orchestrator-state.md` (kept current, times from `date -u`) | `src/`, `tests/`, `scripts/` (it delegates) |
| **edg-backend** (Sonnet) | `src/backend/**`, `src/driver/**`, `include/cerastes.h`; **in Part A only** `src/vir/**` and `src/viper/**`; the build system, the box lanes and the baseline; **the joined build of each part** with `scripts/regen_vpr.py` and `scripts/vprdiff.py` on it; `internal/encoding.md`, the developer guide's paragraph | `tests/`, `scripts/lint_viper.py`, `src/vir` and `src/viper` after Part A |
| **il-analyst-1** (Sonnet) | P82 and S47; the chapter `## M4.5e1e1` of `internal/il-inventory.md`; the fixtures `tests/viper-smoke/memory/s47_*.vpr`. It ends when the chapter is committed | `src/` |
| **il-analyst-2** (Sonnet, fresh at Part B) | `src/vir/**` (`vir.h`, `vir.cpp`, `check.cpp`, `lower.cpp`, `lowered.*`), `src/viper/**`, the next free self-test case file (`src/viper/selftest_cases34.inc` unless M4.5e1f took it), for Parts B and D | `src/backend`, `tests/` |
| **il-analyst-3** (Sonnet, fresh at Part C) | the same files, for Parts C and E. Briefed with this plan, record 0063, the S47 chapter and Commit 2 | `src/backend`, `tests/` |
| **test-mutator** (Sonnet) | `tests/**` (tests, twins, pins, exactly the Flips), `tests/mutants-equivalent.txt`, the compiler matrix, `scripts/run_tests.py`, `scripts/lint_viper.py`, `scripts/rejections.py` and their self-tests, the example files under `doc/modules/ROOT/examples/` | `src/` |
| **soundness-auditor** (Opus, read-only) | four rounds (section 8), each on a snapshot commit in `.claude/worktrees/m4.5e1e1-audit` and the box's audit lane, each before its part's commit; resumed between rounds | everything (never edits) |
| **Advisor** (Fable) | every question first (section 12), and the five checkpoints below | — |

- **Five Sonnet agents over the session, at most three alive at once:** edg-backend, the current il-analyst,
  test-mutator. edg-backend is resumed between tasks (SendMessage); an il-analyst is replaced, not resumed, at
  the two hand-overs (`lower.cpp` is 23,269 lines); the test-mutator is replaced by a fresh one from its state
  file (`build/m4.5e1e1/test-mutator-state.md`, which it keeps current) when the orchestrator and the advisor
  judge its context low (e1d did so twice, near 600k tokens). This is the plan, not a deviation.
- **The advisor** is the `advisor` tool; if the orchestrator has none, an Agent call with `model: fable`, briefed
  with the question and the paths to read. Step 0 checks that one of the two answers.
- **Advisor checkpoints:** (1) the record draft with P82's and S47's outcomes mapped to section 6's tables,
  before audit round 1; (2) every question, and any outcome outside a table (section 12); (3) the split points
  at Commit 2 and at Commit 3; (4) any cut; (5) before the box run.
- **Every test run is on the x86-64 box** (owner, 2026-11-24), in a lane of its own per agent that builds:
  `~/cerastes-e1e1/{base,work,audit}` on `matt@clang4.cpp.al`, each a copy of the `ubuntu-26.04` checkout with
  its own container (`internal/handoff.md` §6 has the recipe; Step 0 sets them up or adopts `~/cerastes-e1d`'s).
  `base` holds the baseline at the session's base commit, `work` the joined builds and every test run, `audit`
  the auditor's snapshots. Work is shipped as a commit object in a git bundle. The Mac builds to compile and
  runs nothing but a single-file probe whose box round trip would cost more than the run (say so when choosing
  it). The final `check.sh` runs in `~/cerastes-s8/p-ubuntu-26.04`.
- **Builds:** every agent that edits `src/` builds its own change on the Mac through the lock (`lockf
  build/m4.5e1e1/build.lock cmake --build build/cmake`). The `work` lane has one builder at a time, the agent
  the orchestrator names; nothing builds there during a run.
- **The runner** (`scripts/run_tests.py`; it locks `build/vpr`) has one holder at a time in the `work` lane: the
  test-mutator, except while il-analyst-1 runs S47's fixtures (from R3's end until Commit 1). Everyone else uses
  `build/cerastes verify <file> --keep-vpr`.
- **`lower.cpp`, `vir.h`, `vir.cpp`, `check.cpp`, `lowered.*`, `emit.cpp` have one holder at a time:**
  edg-backend in Part A, il-analyst-2 in Parts B and D, il-analyst-3 in Parts C and E, with a commit between
  holders. An agent that needs a change in another's file asks the orchestrator, who passes it on.
- **Every agent** (the orchestrator puts this in each briefing):
  - reads `CLAUDE.md`, this plan, record 0063 and the files it is assigned; edits only its own files;
  - **any question stops that work and goes to the orchestrator, who takes it to the advisor first** (section
    12); the agent does not wait idle: it goes on with whatever the question does not block;
  - writes each report to `build/m4.5e1e1/<role>-report-<n>.md` as well as returning it (the soundness-auditor
    cannot write: the orchestrator cuts each snapshot commit and the audit worktree, ships the snapshot to the
    audit lane, and saves each audit report to `build/m4.5e1e1/auditor-report-<n>.md` itself);
  - follows the build, lane and runner rules above and section 3's budget: **it runs no test that section 3
    does not list**, and reports the wall time of every new test's first run;
  - a test generator lives in `build/m4.5e1e1/` and is never re-run over the tree;
  - tool code in the owner's style (`scripts/format.sh`; brace initialization, `U`, one declarator per
    declaration, no comment on a brace line); no Claude trailer.
- **A reader lands with what it reads** (P77): nothing is registered and no name is added to `<cerastes.h>` in
  this session; the encoder's acceptance of a construct lands in the part whose lowering gives it rows.
- **Fail closed between parts:** what a later part lifts is rejected with a diagnostic and a staging pin (section
  5's R2), written and retired inside the session; in `src/vir` the gate is `check.cpp`'s.
- **Session tools are committed** (question 193): anything Step 0 or a spike scripts goes to `scripts/` with a
  self-test, or its summary to `internal/il-inventory.md`.

## 3. The test budget

**Where e1d's time went** (its orchestrator's log, UTC; `build/m4.5e1d/` goes with its worktree, so the numbers
are kept here). 14:40 start. 17:19 Commit 1: 2 h 40 for Step 0, P81, S46 (93 fixtures), Part A and the lint.
18:17 audit round 1 back. 19:45 Commit 2. 20:51 Commit 3. 21:45 the last part built and proved. **01:10 the box
check started, 3 h 25 later:** a full `run_tests.py` on the Mac (15,982 runs, 21:45–23:34), which the box check
then repeated; the move of all testing to the box; audit round 4 (23:36–00:10); two tests found slow on the box
only at 23:21 (the Mac's Z3 is 3–6.5 times faster per hard query) and the two owner questions they cost; a third
validation round after the audit. 02:00 the box check green (50 min). **Of 11.5 h, 4 h 25 came after the last
part.** Before that, each part's run repeated the earlier parts' tests (333, then 436, then 483 runs) although
`vprdiff` had already shown their Viper byte-identical.

**The rules.**
1. **Byte identity replaces re-running.** After each part edg-backend regenerates every kept `.vpr` and `.map`
   (`scripts/regen_vpr.py`, about 25 s for 6,400 programs on the box) and compares with the baseline
   (`scripts/vprdiff.py`). This session expects **no existing program's Viper or map to change** (section 7 says
   why for each item): while that holds, no existing test is run before the box check. A test's last pass
   stands while its `.vpr` and `.map` are identical to that run's, `src/driver/**` and `scripts/run_tests.py`
   are unchanged since, and the host is the same.
2. **A new test runs once,** when its part is joined, and again only if `vprdiff` against the previous part's
   regeneration shows its Viper or map changed, or its expectation was corrected. A part's run never includes
   the earlier parts' tests.
3. **Every new test is timed at its first run, on the box.** Over 60 s: reported in the part's report. Over
   120 s: a smaller program, or `tests/long/` with a short default-tier twin (question 285's rule), settled with
   the advisor at that part, not at the end.
4. **Counts are capped:** at most 45 S47 fixtures (section 6 gives each row its count) and at most 150 test
   files (section 9 gives each part its count). One positive and one error twin per rule of the record, one pin
   per rejected row of section 1's table; more only where an audit asks for it.
5. **The VIR self-test:** `cerastes-vir-selftest` writes every case without Silicon (seconds) and the lint reads
   them; **only the session's new or changed cases go through Silicon**, by name.
6. **The lint:** each new rule has a refusing twin in `scripts/test_lint_viper.py`. No separate mutant sweeps of
   the lint or the harness.
7. **Front-end tests, goldens and the walker oracle** run only if `walker.cpp`, `register_attributes.cpp`,
   `annotation_checks.cpp`, `il_query.cpp` or `include/cerastes.h` changed, and then only `tests/annotations`
   and `tests/il`. This plan changes none of them.
8. **One validation pass after the last audit's fixes** (the preflight below), then the box check. A should-fix
   that touches no test becomes a listed limit, not a diagnosis task. An auditor's probe that runs past ten
   minutes is stopped and reported as a timeout.
9. **Never cut:** invariant 2's pairs, the audit of each `src` commit before it lands, the neutrality gate, the
   one full `check.sh`.

| Run | Who, where | When | Why | In place of (e1d) |
|---|---|---|---|---|
| baseline: `regen_vpr.py` and the VIR self-test output at the base commit | edg-backend, lane `base` | Step 0 | the reference for rule 1 | the same |
| R1–R3 and M (section 5) | test-mutator, lane `work`, frozen binaries | Step 0 | today's outcomes; the staging pins; `rejections.py`'s base | the same |
| S47's fixtures, one file per run | il-analyst-1, lane `work` | section 6 | the decision tables | 93 fixtures |
| the half's own proof: new VIR cases through Silicon; the build | each implementer | each part | a half is proved before it is joined | the same |
| joined proof: regen and `vprdiff` against the baseline and the previous part; `rejections.py --base` | edg-backend, lane `work` | each part, once | rule 1; the Flips | the same |
| the part's new tests, once, timed | test-mutator, lane `work` | each part, after the joined proof, builds held | invariant 2; rule 3 | the cumulative set at every part |
| the auditor's probes | soundness-auditor, lane `audit` | each round | its judgement, bounded by rule 8 | unbounded |
| **preflight**, once: `format.sh --check`; `py_compile`; `run_tests.py --check-headers`; `mutate.py --validate`; `test_run_tests.py`; `test_lint_viper.py`; regen and `lint_viper.py` over it; the VIR self-test directory, its lint, `VIR_CASES_MIN`; `rejections.py --base`; `build_docs.sh`; `check_doc_examples.py` (source and `--html`) | test-mutator and edg-backend, lane `work` | after the last audit's fixes | `check.sh`'s steps 4, 6, 8, 9 and 11 for what the session touched, so the 50-minute run cannot fail late on them | the same list **plus a full `run_tests.py` on the Mac (1 h 50) and a third round** |
| `CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` | orchestrator, `~/cerastes-s8/p-ubuntu-26.04` | once, after the last code change | the full check (invariant 8 of `plan.md`) | the same (50 min) |

**Not run:** a full `run_tests.py` outside `check.sh`; any existing test whose Viper is unchanged; `mutate.py`'s
mutation run (M4.8's); the self-tests of tools the session did not touch (`test_walker_oracle.py`,
`test_mutate.py`, `test_vlibc_contracts.py`, `test_object_identity.py`), which `check.sh` runs anyway.

**The target:** the box check starts within two hours of the last part's joined proof (audit round 4, its fixes
in one batch, the preflight), with the pages, the status entry and the handoff written while agents run.

## 4. Survey facts (tree at `68384d1`; read, not run, by three read-only surveys; names, with lines where they help)

`E` is `src/backend/encode.cpp` (24,010 lines), `L` is `src/vir/lower.cpp` (23,269), `C` is `src/vir/check.cpp`
(5,629), `V` is `src/vir/vir.h` and `vir.cpp`, `P` is `scripts/lint_viper.py` (6,049).

- **Nothing needs the EDG fork, and nothing is registered.**
- **The address arithmetic already composes.** `elem_addr` (`L` 9608–9665) returns `loc(block_of(b), offset_of(b) +
  i)` for any pointer `b`; over `b = loc(member(X, k), 0)` that is `loc(member(X, k), i)` by `loc_inv`, and over
  `loc(GB_g(), 0)` it is `loc(GB_g(), i)`. An array member and a global array need lengths and permissions, no
  new arithmetic. There is no decay node: the decay is `AddrOf(array)` of type `Ptr(Array)` under an `ElemAddr`,
  retyped by `Builder::elem_addr` (`V` cpp 857–874).
- **Where arrays are refused today.**
  - `E`: `vtype`'s `case tk_array` (1762–1766) for every array that is not an automatic local (reached through
    `global_vtype` 486–508 from `encode_named_variable` 7204 and `static_local_decl` 7966; **it runs before any
    `const` rule, so a `const int t[3]` global prints the e1e text today**); `struct_info`'s `case tk_array`
    (23252–23255: the structure becomes not ok and silent); `subscript_address` (858–1024) at 935 when the base
    is `eok_array_to_pointer` and `array_variable(base)` is null (`s.a[i]`, `p->a[i]`, a literal's subscript)
    and for a global through `rejected_globals_`; `describe_operation`'s `eok_array_to_pointer` (184, from
    `operation()` 14437–14454) for a decay of anything but a local array variable; `contract_element_address`
    (699–826) at 728 (an array decay as a base in a contract) and `array_decay` (1028–1055) at 1036;
    `temp_init_value` 13540 and `literal_address` 13612 (array literals); `reject_computed_sizeof_operand`
    (6442–6507; only `sizeof` of an array in `cx_.vars` is lifted, 6486–6494) and `scan_type` (6552–6560, in
    static assertions, whose message names no session); `declaration()` 16885–16892
    (`written_as_array_declarator` 629–640); `declaration_level_checks` 23413–23430 (a typedef whose
    `computed_type_reason` is "an array type", used or not; block-scope typedefs are collected too, by
    `il_query.cpp` 198–200); `array_vtype` 534 (a bound that names a local constant), 551 and 556 (arrays of
    arrays and of structures: e1e2's); `application` 5850 and `forall_site` 11208 (a named array that is not in
    `cx_.vars`).
  - **Untagged gates** (no "e1e" in their text): `constant_value` (6246–6406) accepts only `ck_integer` and
    `ck_address` (6262: no `ck_aggregate`); `read_global_init` (7551–7656) 7616 "an initializer this version
    cannot read"; `collect_init_elems` (17171–17321) 17311–17314 for a `ck_aggregate` element of a member that
    is not a structure; `address_target` (7422–7504: an offset, a subobject path, or a pointee type that differs
    from the variable's: so `int *gp = g;` is refused); `global_read` (7012–7032) has a guard for a structure and
    none for an array; `vir::member_root` (`V` cpp 1427–1440) peels `MemberAddr` only, so `in_own_initializer`
    (17112–17116) misses `s.a[0]` inside `s`'s own initializer.
  - `C`: `struct_type` 436–458 (a member that is not a structure, a pointer or a scalar); `global` 540–613 (575–577);
    `accepted_pointer` 339–354 (a `Ptr(Array)` passes only through `array_addr_ok_`, 3092–3099); `ElemAddr`
    3745–3815: **one site, 3787–3791, refuses every base that is not `AddrOf` of an array local or an accepted
    pointer to a scalar** (`s.a[i]`, `g[i]`), and in a contract the base must be a `Formal` (3758–3764); `declare`
    2497–2502 refuses `compound_literal` on an array local; the Sequencer's `object_of` (5125–5179) gives element
    keys only for `ElemAddr(AddrOf(var), …)`, and `object_of_root` (5202–5206) makes a global one object.
- **The local-array machinery to reuse** (`E`): `array_vtype` 512–573, `check_array_bound` 579–583 and
  `check_array_bound_constant` 606–625 (a bound with an `expr` is read by `constant_value` and emitted as a
  prologue-checked constant with `maybe_perturb`), `collect_array_elems` 1090–1166, `array_element_value`
  1168–1191, `element_store` 1194–1198, `array_initializer` 1203–1344, `array_decay`, `subscript_address`,
  `detect_array_address_test` 1065–1077. (`L`): `alloc_cell` 3767–3800 and `zero_fill` 3752–3764 with the
  generated trusted methods (`array_alloc_method` 1090–1172), `array_scope_exit` 3911–3931, `range_perm`
  3841–3849 and `emit_marked_row` 3857–3866, `range_quant` 3870 (**its pointer must be a variable**: S44 F-b1),
  `in_range` 3880, `range_shape` 3890, `ArrayPlace` 14653–14660 and `RangePlace` 14665–14680 (collected in
  `loop`, 16654–16740; conjuncts 17308–17369; back edge 17866–17888), `range_atom_value` 6649–6720,
  `range_marked_rows` 14716–14732, `call_atoms` (head 10046; range rows near 10540), `range_returns` 13933,
  `inv_elem` 4087–4122, `array_place_hit` 4139–4146, `inv_framing_any` 4295–4356.
- **Structures are a finite list of cells today.** `vir::struct_leaves` (`V` h 233; `collect_leaves`, cpp 32–55)
  makes every member that is not a structure one leaf, **an array member included, with the array's type**;
  `StructLeaf::path` is member indices only. `L` uses it 38 times and `leaf_cells` (2135–2146) 11 times;
  `member_ref` (2111) is `loc(member(base, k), 0)` and `chain_ref` (2117) chains it. `L` has two traps that
  already fail closed: `cell_suffix` on an Array type (1945–1950) and `use_cell_type` on a Struct (2003–2006)
  are internal errors. **There is no `WF_S` symbol:** `wf_expr` (5884–5899) appends `struct_lens` (2180–2204:
  `len(member(X, k)) == 1` and the slot facts) inline. Section 8's Part C has the site table.
- **`member_addr`** (`L` 9558–9601) passes the place mark only to an inner `MemberAddr`; `elem_addr` lowers its
  base as a value. So for `p->a[i]` today's code would lower `&p->a` as a value, with `member_addr`'s row
  "forms … but `p` may be null or point past the end" and a liveness row: wrong for a place access (0043 item
  5: a member access is not a use of `p`'s value). **`elem_addr` needs the place case for a `MemberAddr` base.**
- **Globals** (`L`): `global_block` 7076, `global_ref` 7080 (`loc(GB_g(), 0)`), `global_facts` 7094–7111 (`len(GB)
  == 1 && lvloc(GB) == loc(GB, -2)`, and `struct_lens` for a structure), `program_start_method` 1252–1371 (a
  scalar row 1345–1367, a structure row 1305–1343), `held_keys` 15195–15244, the global places of `loop_places`
  (15285–15462: one cell per scalar global), `note_global_member_write` 14819–14857, `key_of_addr` 8430–8433
  (one key per global). **`is_global_ref` (7084–7088) ignores the slot index, but its one use,
  `inv_place_disjunct` (4235–4243), also asks `same_expr(q, pl.recv)`:** the shortcut fires only for the
  identical term `loc(GB_g(), 0)`, and any other slot takes the general `q == pl.recv` disjunct against that one
  place. So the handoff's note ("treats any `loc(GB_…, i)` as one place") is overstated; what a global array
  needs is an interval hit, `block_of(q) == GB_g() && 0 <= offset_of(q) < N`, as `array_place_hit` has it.
  `elem_key` (4809–4821) assumes a `Formal` base.
- **`vir::Global`** (`V` h 691–736) holds one initial value (`init_kind`, `init`, `value`, `target`) or, for a
  structure, `leaves` (one `GlobalLeaf` per `struct_leaves` entry). `E` `read_struct_global_init` (7746–7913)
  fills them through `collect_init_elems`.
- **Structure values** (`L`): by-value parameters are one Viper formal per leaf (`param_vars` 20690,
  `p_s__k_m<i>`), results `res_m<path>` (`result_vars` 20709); a value is `LV::leaves`; `struct_store`
  (4577–4655), `struct_cell_write` (8935), `full_exprs` (12462), `ret` (13036), `contract_env` (12415). `E`
  expands a copy between objects into leaf stores (`copy_struct` 17326–17415, over `vir::struct_leaves`), and
  builds values in `object_value` 13401–13441 and `aggregate_value` 13704–13800.
- **The lint pins every shape** (`P`): `BLOCK_BASE_RE` (1360) asks for `block_of(<identifier>)` as the block of
  every `forall k` row (`parse_range_quantifier_full` 3379; `_range_acc` 3518; `_loop_conjunct` 3540;
  `range_atom_row_problem` 3461); `global_facts_atoms` (947–951) compares the facts as an exact set with `len(…)
  == 1`, at three places (1162, 1223, 2780); `lint_globals` (1076–1228) pins `program_start` row by row, every
  row unquantified at slot 0; `struct_allocation_problem` (783) pins `alloc_struct_*` with `len(member(X, k)) ==
  1` per member; `parse_struct_chain` (724) admits only `loc(member(<chain>, k), 0)`; `lint_flags` (5806) admits
  a flag write only through an identifier or `loc(GB_x(), 0)`. **A defect found by reading:** line 1220 exempts
  methods whose name starts `alloc_` from "first `requires` is the global facts"; `zero_arr_<t>` does not start
  so and carries no facts line (`array_zero_clauses` 3282), so **a program with a global and a zero-filled local
  array would be refused by the lint today**. No test or fixture combines the two (R4 measures it).
- **Pins** (22 test files name e1e; the planning's table of every array rejection with its expect line, mode and
  `Pins:` text is the base of section 10): `tests/memory/negative/global_array.c`,
  `array_reject_static_local.c`, `reject_struct_array_member.c`, `array_reject_compound_literal.c` (C only),
  the typedef, `typeof`, `decltype` and `sizeof` pins of `tests/scalar/negative` and `tests/memory/negative`.
- **Docs today:** `memory.adoc` 465–466 (array members not supported), 656–657, 728–741 (the not-yet list of the
  quantifier section), 847–849 and 1843 (global arrays); `verifying.adoc` 1107, 1242–1244, 1269–1275 (array
  types as operands; "So is a typedef of an array type"); `overview.adoc` 98–99; `annotations.adoc` 965–966,
  1035–1036. Anchors of `memory.adoc`: structures 354, arrays 479, globals 789, static-locals 851, not-yet 1825.
- **Self-tests:** the latest case file is `selftest_cases33.inc`; `VIR_CASES_MIN=2318` (`scripts/check.sh:104`).
  `scripts/test_lint_viper.py` has 9,293 lines and 828 tests.
- **Costs of a round trip:** the box regenerates 6,400 programs in 18–25 s; its full check took 50 minutes with
  Silicon capped at 16 provers.

## 5. Step 0 and Step 1

1. **Orchestrator.** `git fetch`; branch from `origin/main` when it holds this plan's commit (otherwise from
   local `main`, and say so in status); `git worktree add .claude/worktrees/m4.5e1e1 -b m4.5e1e1-arrays-of-scalars
   <base>`; submodules with `--reference` (edg, silicon, then silver); `scripts/bootstrap.sh`. If e1d's
   worktrees and branch are still there, leave them (the owner's step). `npm ls @antora/logger` in `doc/` shows
   one hoisted copy (**nested copies: the advisor, then the owner**). CI recorded only (`gh run list --workflow
   ci.yml --limit 5`). Whether M4.5e1f has landed (the record's number). One question to the advisor, to check
   that it answers.
2. **edg-backend.** The box lanes (`~/cerastes-e1e1/{base,work,audit}`: `internal/handoff.md` §6's recipe; if
   `~/cerastes-e1d` and its image are still there, copy from them), each at the base commit, built with
   `scripts/bootstrap.sh --build-only`. **`scripts/box_lane.sh`** (`setup`, `ship <ref>`, `build`, `run <command>`
   for a named lane; shellcheck clean; the commands e1d typed by hand): at most thirty minutes, otherwise the
   recipe by hand and a note in status. The baseline before any change to `src/`: `scripts/regen_vpr.py` and the
   VIR self-test into the `base` lane's `build/e1e1-base/`; frozen binaries kept in the `work` lane for R1–R4.
3. **test-mutator, on the frozen build** (lane `work`, both modes):
   - **R1** today's outcome of the pins section 10 lists (unchanged by this session unless section 10 says so).
   - **R2** today's diagnostic for each construct of section 1's "In" list and each row of its table, one small
     program each. **The test-mutator adds one staging pin per construct that a later part of this session
     lifts and that no existing pin covers** (`s.a[i]`, `p->a[i]`, the decay of `s.a`, `owns(p->a)`, `g[i]` in a
     contract, a `static` local array's use, a structure global with an array member, `t = s` and a by-value
     parameter of a structure with an array member), each retired by the part that lifts it (Flips A).
     `grep -n 'e1e' src/` (the survey found 28 messages in `E`, 3 in `C`, 1 comment in `L`).
   - **R3** `scripts/rejections.py build/m4.5e1e1/s0/rejected-base.txt`. After each part the same run with
     `--base` may differ only by section 10.
   - **R4** the lint defect of section 4: a program with a scalar global and `int a[4] = {1};` in a function,
     kept Viper through `lint_viper.py`. **If the lint refuses it** (expected): the test-mutator extends line
     1220's exemption to `zero_arr_`, with a self-test twin, and the program becomes a test
     (`tests/memory/global_beside_zero_filled_array.c` with its twin); it is a lint change only, recorded in
     0063 as a defect fixed. **If the lint accepts it:** record why; no change.
   - **M** is P82 (g)'s matrix (section 6).
   - R3 is the only `run_tests.py` run until S47's start.

## 6. P82 and S47 (il-analyst-1, before any encoder code; results in `internal/il-inventory.md`, `## M4.5e1e1`)

**P82 (IL, both modes, `build/m4.5e1e1/p82/`; every claim with its `--il_display` or `--cerastes_walk`
excerpt).** A row's "If not" says what is built instead; "the advisor" means section 12's route.

| Probe | Expected | If not |
|---|---|---|
| (a) **global and `static` arrays:** `int g[3];`, `int g[3] = {1, 2};`, `int g[] = {1, 2, 3};`, `static int t[2] = {5};` at file scope and in a function; an `extern` declaration with a later definition; elements `{300}` for `unsigned char` and `{65536u * 65536u}`; a list of 1,001 constants; `g[i]`, `&g[i]`, `f(g)`, `sizeof(g)`; `int *gp = g;` and `int *gq = &g[1];` | the variable's type is e1a's `tk_array` entry with the same bound fields (P78 (a)); the initializer is `initk_static` over a `ck_aggregate` with trailing zeros not materialized (as P78 (b) found for automatic arrays); each element constant keeps its `expr` and `orig_type` **up to 1,000 elements and loses them beyond** (P38); uses are P78 (c)'s shapes over an `enk_variable` of the file scope; `gp`'s constant is a `ck_address` of `g` with offset 0 and no subobject path, `gq`'s has an offset or a path | an initializer shape the reader cannot tell from these: **the advisor**. `expr` lost below 1,001 elements for some shape: that shape is rejected through `constant_value` (pinned), recorded. `gp = g` not telling itself from `&g[0] + k`: `int *gp = g;` stays rejected (M4.5e2), pinned |
| (b) **array members:** `struct S { int a[4]; int n; }`, a bound `[2 * 2]` and an enumerator bound, a nested `struct T { struct S s; int z; }`; `s.a[i]`, `p->a[i]`, `&s.a[i]`, `f(s.a)`, `sizeof(s.a)`, `t.s.a[i]`; the C++20 class tests of 0043 item 2 on such a structure | the field's type is a `tk_array` entry with e1a's bound fields; `s.a[i]` is `subscript(array_to_pointer(dot_field(var s, a)), i)` and `p->a[i]` the same over `points_to_field`; `sizeof(s.a)` folds to a constant that keeps its `expr`; the structure still passes 0043 item 2's tests in C++ (trivial special members, no packing) | another access shape in one mode: one reading rule in `subscript_address`, recorded. A C++ structure with an array member failing 0043 item 2's class tests: **the advisor** (the member is then accepted in C only, pinned in C++) |
| (c) **initializers of a structure with an array member:** `{{1, 2, 3, 4}, 5}`, `{.a = {1, 2}, .n = 3}`, `{0}`, `{}`, brace elision `{1, 2, 3, 4, 5}`, a partial inner list, non-constant elements, `{.a = {[1] = 2}}`, an excess inner element; the same for a structure global | the member's elements are a nested `ck_aggregate` at the member's position, with trailing zeros absent; elision is told from a nested list by the flags 0043 item 6 reads for nested structures; an array designator is a `ck_designator` with `array_element`; a global's list is constant with P38's limit | elision not told from a nested list: every list without the inner braces is rejected except `{0}` (as 0043 item 6 does for nested structures when it cannot tell), recorded |
| (d) **values:** `t = s;`, `struct S t = s;`, `*p = *q;`, `f(s)`, `return s;`, `f().a[1]`, `(struct S){{1, 2}, 3}` as an argument and under `&` | the copy is `eok_assign` (or a `dik_expression` of the source) as for any trivially copyable structure (P65); a by-value argument and result have 0044's shapes; `f().a[1]` is a subscript over a decay of a field of the call's value | a copy that the IL shows as a call or an element-wise expansion: **the advisor** (Part E's copy rule is then written on that shape) |
| (e) **array compound literals (C23):** `int *p = (int[]){1, 2, 3};`, as an argument, `(int[]){1, 2, 3}[i]`, `(int[2]){1}`, in a loop condition; the same in C++20 | the shapes of 0044 item 7's scalar and structure literals (P67): `is_compound_literal` on the initializer, the enclosing block as EDG wraps it (P67 H4), the elements in e1a's aggregate shape; C++ accepts it silently or with a diagnostic | C: a shape `literal_address` cannot place in a block: array literals stay rejected (**cut 4 without asking**, M4.5e1e2). C++: whatever EDG does, the encoder rejects (as today), recorded |
| (f) **computed types:** `typedef int vec3[3]; vec3 v = {1, 2, 3};` at file and block scope, `typedef char big[65536u * 65536u + 1];`, `using T3 = int[3];`, `typeof(a) b;`, `typeof(int[3]) b;`, `decltype(a) b;`, `sizeof(int[3])`, `sizeof(vec3)`, `sizeof(int[2 * 2])`, `_Alignof(int[3])`, `static_assert(sizeof(a) == 12)`; C++'s `const int N = 4; int a[N];` and a wrapped `N` | the typedef's `tk_array` entry holds the bound constant with its `expr` as a declarator's does (P78 (a)), and a variable declared through it reaches the same entry through a typeref; a type operand's array entry is reachable from the `sizeof` node with its bound's `expr`; the local-constant bound has no `expr`, and its `lerk_array_bound` referrer holds `var N` (P78 F1) | a bound's `expr` not reachable for a typedef or a type operand: that form stays rejected (its pin re-pointed to M8), recorded; no stop. The referrer of a local-constant bound not readable, or `N`'s own initializer without an `expr`: **cut 3 without asking** (the bound that names a local constant stays rejected, M8) |
| (g) **the compiler matrix M** (clang and GCC, both modes, no flag / `-Wall` / `-Wall -Wextra`) for the new source shapes: the initializer forms of (c); `g[3]` and `s.a[4]` with a constant index outside; `if (g)`, `if (s.a)`, `g == NULL`; `sizeof(g) / sizeof(g[0])`; a block-scope array typedef, used and unused; a by-value structure with an array member; a compound literal's decay | **rule, not a decision** (0020 item 5's union of the first two tiers): a shape that warns without a flag or at `-Wall` is rejected like the compilers, by the rule e1a and c1 already have for its local form (`array_address_null_test`, EDG's #175 as an error, `-Wmissing-braces`); every positive compiles with `-Wall -Wextra -Werror` | a new warning family with no existing rule: **the advisor** (reject like the compilers, with a pin, is the default) |

**S47 (hand-written Viper in the shapes of section 7; fixtures `tests/viper-smoke/memory/s47_*.vpr`; every row
expanded as lowering will write it, from the frozen build's kept Viper of e1a's, e1b's, c1's and b2a's tests;
one file at a time through `scripts/run_tests.py` on the box; times on a quiet lane).** A twin must fail with
**exactly one** error unless the table says otherwise. Every fixture method ends with an `assert false` twin
(vacuity). The count in brackets is the row's cap.

| Spike | Programs | Built when it passes | If it does not |
|---|---|---|---|
| **(a) the bound local** [5] | a structure local with an array member: `alloc_struct`'s `ensures` in the **block-term form** (`len(member(r, 1)) == 4`; `forall j: Int :: { loc(member(r, 1), j) } 0 <= j && j < 4 ==> acc(loc(member(r, 1), j).v_int)`, the flag field, the flags false); then **e1a's rows unchanged over a Viper local bound to the member's first slot** (`t := loc(member(s, 1), 0)`): the element access in `elem_addr`'s spelling, the marked rows and the `exhale` at the scope exit over `loc(block_of(t), k)`. Twins: an index outside (`out-of-bounds`); the scope exit after a callee kept one element (the marked row, `insufficient.permission`). The same over `t := loc(GB_g(), 0)` for a global array | section 7 item 5: **in code, every range row is today's shape over a bound local** (`BLOCK_BASE_RE` and every `forall k` rule of the lint are unchanged); the block-term form stands only where no local exists: contracts, `alloc_struct`'s `ensures`, `program_start`, the global facts and `struct_lens` | the bound-local rows fail where the block-term rows pass (Silicon does not carry the permission through `block_of(t)`): **every row takes the block-term form** (the lint's `forall k` rules gain the block grammar of item 15 in every shape; about 150–250 more lint lines and their twins). Recorded, no stop. Both forms failing: **the advisor, then the owner** (protected) |
| **(b) a global array** [8] | `program_start` with the row group of item 9 (the two quantified permissions, the flags, listed values for a prefix and one quantifier for the zero rest, the liveness cell) and the facts `len(GB_g()) == 4`; `main` with a loop that sums and one that fills `g` (e1a's conjuncts over the bound local); a callee with `requires owns(g)` in the block-term form called from `main` (the marked rows, the amount); `reads(g)` with a ghost amount; a callee with `owns_range(p, n)` called with `g`. Twins: `check(g[3] == 1)` against the zero rest (`check`); a write under `reads(g)` (`permission`); a loop after a callee kept `g` (the entry marked row); the unknown-value form of `program_start` with a `check` on a value (`check`); `assert false` after `program_start` | items 3, 7, 8, 9 and 10 for global arrays | a positive failing or any program over 30 s: **the advisor** (protected: the owner if no form passes) |
| **(c) an array member through a pointer** [7] | a method with `requires owns(*p)` in the block-term form (the root facts, `len(member(p, 0)) == 4 && len(member(p, 1)) == 1`, the quantified leaf, the cell leaf, one liveness fraction); the body reads and writes `p->a[i]` and `p->n`; a caller that holds a structure local and calls it (cell rows as today, marked rows for the array leaf over a bound local), then reads the element back; `reads(*p)` with a ghost amount. Twins: the caller passes `&s` and `&s.a[1]` to `owns(*p) && owns(*q)` (the sum row: `permission`); `WF_S` without the member's length (the bound row fails: the evidence that the length is needed); a callee that keeps the array leaf (`permission` at the caller's next read) | items 3, 6, 7, 8 for array members | a positive failing: **the advisor** (protected). The alias twin verifying: **stop, the owner** (the overlap rule of 0043 item 9 does not reach an array leaf) |
| **(d) loops over a member's array** [4] | a loop over `s.a` of a structure local (the array leaf as an array place over a bound local, with the structure's cell places beside it) and over `p->a` under `owns(*p)` (a guarded range place); values kept where no write reaches. Twins: a store through an `int *` in the body (the values-kept conjunct must be absent: with it, an `internal` row fails); the invariant reads `s.a[4]` (`out-of-bounds`) | item 10 | a positive failing: **the advisor** |
| **(e) depth** [2] | `t.s.a[i]` (a nested structure's array member) through a pointer to `T`: the chain `loc(member(loc(member(p, 0), 0), 0), i)`; one twin (the other member's cell is kept: `postcondition` when it is changed) | the chains of items 3 and 6 at any depth | **cut 5 without asking:** an array member only in a structure that is not itself a member (pinned, M4.5e1e2) |
| **(f) an array literal** [2] | a hidden array local of its block, `zero_arr` and the element stores inside an expression, the decay as the value; a twin reading past it | item 12 | **cut 4 without asking** |
| **(g) the trusted copy** [7] | `copy_arr_int(d, s, n, rd)` in item 13's shape; a copy between two structure locals; from `*p` held at a read fraction into a local; an exact self-copy under the guard `d != s`. Twins: one source element unwritten (the `uninitialized` row); the destination held at a fraction (`permission`); a wrong value read back after the copy (`check`); the self-copy without the guard (fails: the evidence for the guard); `assert false` after the call | item 13's copy by the trusted method | the method's `requires` refused by Silicon, a positive failing, or the vacuity twin verifying: **the copy is written element by element, as the value view's leaves, under question 295's limit of 64** (no trusted method; a copy of more elements is rejected, pinned, M4.5f3). Recorded for the owner (section 15), no stop |
| **(h) cost** [3] | a structure with two array members and a loop over each; a global array of 1,000 elements with 16 listed values; S43 (h)'s scale program at K = 16 with one array member | recorded | **over 60 s: the advisor** |
| **(x) for M4.5e1e2, recorded only** [6] | (x1) `owns_range(p, n)` over `struct S { int x; int y; }` as 0028 item 2 fixed it (one quantifier per leaf, each with its own trigger `{ loc(member(loc(block_of(p), k), j), 0) }`), with e1b's loop conjuncts per leaf and a loop that sums `p[i].x`; (x2) `int m[2][3]` with a permission over two bound variables, `{ loc(member(loc(block_of(m), i), 0), j) }`, and a nested loop; one failing twin each; times | nothing: the verdicts and times go to the chapter for e1e2's planning | nothing |

**Stop rules for all of S47:** a second Silicon error after a marked row; `qp.not.injective`; a native error in a
positive; an outcome no table names. Each goes to the advisor first.

## 7. Record 0063 — arrays of scalars in structures and static storage (the orchestrator writes it before code from this section; revised only by section 6's tables and audit round 1)

Amends 0024 item 6 (array types of scalars); 0028 items 5 and 16 (as built for arrays); 0041 items 2, 4, 5, 10 and
12; 0043 items 2, 3, 4, 6, 7, 8, 9, 11 and 13; 0044 items 2, 3, 4, 7 and 8; 0059 items 1, 2, 4, 8 and 11; 0060
items 1 and 9; 0061 item 1 and 0062 item 4 (a named global array as an argument); 0012 item 11 (the lint).

**No existing program's Viper or map changes in this session.** Every item below adds rows only for a program
that holds a construct rejected today; where an existing function is touched (`inv_place_disjunct`,
`global_facts`, `struct_lens`, the leaf enumerations), the text it writes for a program without the new
constructs is the same, and `vprdiff` shows it after every part.

1. **Scope:** section 1, with its table.
2. **Which arrays.** One dimension; the element a scalar that 0059 item 2 accepts (not `const`, `volatile` or
   `_Atomic`; a `const` array in any storage is M4.6's, and **the `const` rule is tested before the array rule**,
   so its message names M4.6); a constant bound of at least 1, or none with a braced initializer.
   - **The bound is read as written** (0059 item 2, `check_array_bound_constant`): a bound with an `expr` is a
     checked constant with its obligations at the bound's position and the `internal` guard that the value as
     written is EDG's (0017 item 4). For a global, a `static` local, a member and a file-scope typedef it is one
     of the unit's prologue constants (0041 item 5's list), named for the message ("the bound of `g`", "the bound
     of `S::a`", "the bound of `vec3`"); for a block-scope typedef and a `typeof` it is a constant of its
     function. A literal, a macro's literal and an enumerator keep no `expr` and need no check (P78 (a)).
   - **Rejected:** section 1's table; `owning`, `ref_to_uninit` and `uninit` on an array member or a global
     array (0059 item 2's rule for a local, `unsupported_annotation`).
3. **Layout** (0028 items 5 and 16, as decided in M4.5a).
   - A global or `static` local array `T g[N]` is the one block `GB_g()` of `len` N; element `k` is `loc(GB_g(),
     k)` in `T`'s value and flag fields. The global facts state `len(GB_g()) == N && lvloc(GB_g()) == loc(GB_g(),
     -2)`.
   - An array member `T a[N]` at index `k` of the structure slot `X` is the block `member(X, k)` of `len` N;
     element `i` is `loc(member(X, k), i)`. It shares its root's liveness cell (`member_lv`). `struct_lens`
     (0043 item 8's `WF_S`), `alloc_struct`'s `ensures` and a structure global's facts state `len(member(X, k))
     == N` where they state `== 1` for every other member.
   - Both hold in 0028 item 2's model (`len` is free; a member block is a fresh term), and they are hypotheses
     discharged by an allocation, by `program_start` or by a caller, never axioms.
4. **VIR.**
   - `Type::Array` is the type of a `Global` and of a `StructMember`, beside an automatic local's. An array
     still has no value: its address (`AddrOf` of an array local, `GlobalAddr` of an array global, `MemberAddr`
     of an array member) is accepted as the base of an `ElemAddr` and as the operand of a whole-array atom, and
     nowhere else.
   - **Three views of a structure's leaves.** The **object view** (`struct_object_leaves`): each leaf is a cell
     or an **array leaf** (the member's path, the element type, the count). The **value view**
     (`struct_value_leaves`, Part E): every element of an array member is a leaf of its own, in index order
     (`StructLeaf::elem`). **The old view, `struct_leaves`, is unchanged and keeps returning an array member as
     one leaf of the array's type: it is the poison.** A site that still treats that leaf as a cell is stopped:
     lowering's `leaf_cells` and every direct `struct_leaves` loop report an internal error on an array-typed
     leaf (exit 3; `cell_suffix` already does), `check.cpp` refuses a structure with an array member in every
     position a part has not lifted, and the encoder rejects it there first with a diagnostic at the C line
     (invariants 3 and 7). For a structure without an array member the three views are the same list, so every
     existing program is lowered as before.
   - `Global` gains the initial values of an array: the listed prefix (one `GlobalLeaf` per written element),
     the zero rest, or "unknown".
   - `Stmt::Kind::ArrayZero` (the address of an array member: C's zero fill of the elements an initializer does
     not write; e1a's `Var::zero_filled` stays for a local), `Stmt::Kind::ArrayCopy` (Part E) and
     `Expr::Kind::ElemValue` (element `i` of an array member of a structure **value**, Part E).
   - `check.cpp` backstops each rule of items 2–14 (positions, types, a count that matches the type, an
     `ElemAddr` base that is one of the three array addresses, `ArrayCopy` between two array members of one
     type).
5. **Rows in code.**
   - **The bound local** (S47 (a)). Where lowering writes a row over an array's slots in code (a scope exit, a
     loop's entry, conjuncts and back edge, a call's and a return's marked rows), it first binds the array's
     first slot to a Viper local (`t := loc(member(X, k), 0)`, `t := loc(GB_g(), 0)`) and writes **e1a's and
     e1b's rows over that local, unchanged**. A marked row's block is then still `block_of` of a Viper local
     (0059 item 7's argument that its reason is unambiguous holds as written). The block-term form (`loc(member(X,
     k), j)`, `loc(GB_g(), j)`, with `X` the canonical member chain of 0043 item 13) stands only in lines that
     have no local: a method's contract, `alloc_struct`'s `ensures`, `program_start`, the global facts,
     `struct_lens`. (If S47 (a) refuses the bound local, every row is in the block-term form.)
   - **An element access** `g[i]`, `s.a[i]`, `p->a[i]`, `t.s.a[i]` is `ElemAddr` over the array's address, with
     0059 item 4's rows in its order. A global array and an array member are **named** bases: no null or
     liveness row of their own (a global's block never ends; a member's is its structure's). For `p->a[i]` the
     root's row is 0043 item 5's (`p != null`, `null-dereference`), once, and **the access is not a use of
     `p`'s value** (no liveness row): `elem_addr` passes the place mark to a `MemberAddr` base. Then the bound,
     against the length item 3's facts give (`out-of-bounds`: "reads `s.a[i]`, whose index may be outside
     `s.a`"), `permission` and, for a read, `uninitialized`. The decay of `s.a` or `p->a` used as a value, and
     `&p->a[i]`, form `&p->a` first: 0043 item 5's row for the address of a member (`null-dereference`, then
     `p`'s liveness).
   - **Rejected like the compilers** (0059 item 4, extended to the new bases by P82 (g)): a constant index
     outside a global array or an array member (EDG's #175, already an error in verify mode); a truth test or a
     null comparison of such an array's address (`array_address_null_test`); a plain `char` index.
6. **Structure objects with an array member** (amends 0043 items 4, 6 and 8).
   - **Allocation:** `alloc_struct_<n>_<tag>()` ensures, for an array member's block, `len(member(X, k)) == N`
     and, each as its own quantifier with the trigger `{ loc(member(X, k), j) }` over `0 <= j && j < N`, full
     permission to the value field, to the flag field, and every flag false. Consistent as 0043 item 4's is:
     the member blocks are fresh terms, and each (receiver, field) pair is given once.
   - **Scope exit:** per array leaf, e1a's rows over the bound local (the two marked rows, `lifetime`: "the
     address of an element of `s.a` may still be in use when its lifetime ends"), with the cell leaves' rows,
     then one `exhale`.
   - **Initializers** (0043 item 6's rules for the list): the array member takes a nested braced list in e1a's
     element rules (each element an assignment conversion read through `init_element_value`; an array
     designator rejected, M8; a subobject initialized twice rejected in C; in C every element of the whole
     structure list is evaluated before the first store, in C++ in order). The elements the list does not write
     are zero: `ArrayZero` on the member, then the written elements' stores. A list that elides the member's
     braces is rejected like the compilers (`-Wmissing-braces`), except `{0}` and `{}`. An element that reads,
     writes or takes the address of the structure being initialized is rejected through an `ElemAddr` too.
   - **Without an initializer** every flag stays false; `cerastes::uninit` on the structure local is accepted
     and emits nothing (0043 item 9).
7. **Contracts** (amends 0041 item 7, 0043 item 9, 0060 items 1 and 9).
   - **Atoms.** `owns(g)` and `reads(g)` of an array global: every element (the two quantified permissions,
     full or at the atom's ghost amount, the flags, the element range, one liveness fraction), in the block-term
     form over `loc(GB_g(), k)`, `0 <= k && k < N`. `owns(*p)` and `reads(*p)` of a structure with an array
     member: 0043 item 9's lines for the cell leaves and, per array leaf, the same four lines over `loc(member(X,
     k), j)`. `owns(p->a)` and `reads(p->a)` (and `g.a` of a structure global): that array leaf alone, with one
     liveness fraction. **An atom never names part of an array here:** `owns_range` on a global or member base,
     an atom on one element and `&p->a[i]` in a contract stay M4.5e2's.
   - **Element reads:** `g[e]`, `p->a[e]`, `r->a[e]` of a pointer result and `g.a[e]` in `requires`, `ensures`, a
     function's `decreases` and under `old`, the index reading no memory (0060 item 5's rule). A read is framed
     by an atom that covers its array leaf: the disjunct is "the site's guard, the same block, the index inside
     `[0, N)`", and `f__wf` asserts the disjunction with the bound (`permission`: "the contract of `f` reads
     `p->a[i]` without owning it"; an index outside is the same row, as 0060 item 5 has no separate bound row).
     `f__wf`'s ghost value of a read is keyed by root, member path, index and state.
   - **Overlap is by slots** (0043 item 9, the soundness core). An array leaf of an atom is, for every rule that
     pairs two sites (the exclusion of `owns` and `reads`, the amount sums at calls and returns, the entry
     separation, `f__wf`'s distinctness and equal-ghost facts, framing disjuncts, the case splits, loop places),
     **a range of N slots at `loc(member(X, k), 0)` or `loc(GB_g(), 0)`**: it meets a cell `q` of its element
     field when `q` lies in that block within `[0, N)` (`in_range`'s test), and another array leaf or range when
     the intervals of one block meet. So `f(&s, &s.a[1])` with `owns(*p) && owns(*q)` fails its sum row
     (`permission`), as `f(&s, &s.x)` does today.
   - **Amounts** (0037 item 6, 0060 item 6): a `reads` atom's amount at a call is the minimum over its cell
     leaves and, for each array leaf, the leaf's first slot (0060 item 6's rule, with its listed limit for an
     uneven holding: question 238, M4.5e1f).
   - **Rejected** (`unsupported_annotation`): 0043 item 9's overlapping atoms on one root, now also `owns(*p) &&
     owns(p->a)`; a marker on a pointer to a structure with an array member (question 294); a written atom in a
     loop invariant on such a structure place.
8. **Calls and returns** (amends 0037 items 8 and 10, 0043 item 9). A call's and a return's rows for an atom are
   per leaf: the cell rows as today, and per array leaf e1b's marked rows (value, then flag) over the bound
   local, with the messages in source terms ("calls `f`, which needs every element of `s.a`"). The decay of a
   global array or of an array member passed to a callee's `owns_range(q, n)` or `reads_range(q, n)` is bound to
   a local first (S44 F-b1) and takes 0060 item 8's rows as any pointer does.
9. **`program_start`, initial values, `static` locals** (amends 0041 items 3–5 and 12, 0044 item 8).
   - **An array global's rows**, positioned at its declaration, in this order: the two quantified permissions
     over `[0, N)`; the flags, every one true; the values; `acc(lvloc(GB_g()).lv)`. The values, by 0041 item
     3's linkage rule: **known** — one conjunct per written element (`loc(GB_g(), i).v_T == c_i`; a pointer
     element `null` or `loc(GB_h(), 0)`), and one quantifier for the rest (`m <= k && k < N ==> … == 0`, `false`
     or `null`); **unknown** — the element range (or `WF`) as one quantifier.
   - **Initializers.** A braced list of constant elements, each read through `constant_value` (P38: an element
     of a list of more than 1,000 keeps no `expr`, and that element is rejected by `constant_value`'s own rule,
     never read from the aggregate), each with an `expr` checked in the prologue as 0041 item 5 checks a scalar
     global's. No designators (M8). `int *gp = g;` is the address of element 0 if P82 (a) can tell it; `&g[k]`
     is M4.5e2's.
   - **A `static` local array** is a global whose name its function sees (0041 item 12), with the implicit
     `owns` atom of item 7's array form in its function's and its callers' contracts.
   - **A structure global** with an array member: its row gains the array leaf's lines; its facts the length.
   - `program_start` stays a trusted contract pinned line by line (item 15); its rows give each (receiver,
     field) pair once.
10. **Loops** (amends 0038 item 3, 0041 item 10, 0043 item 11, 0059 item 8). A loop that touches memory takes,
    as **array places** with 0059 item 8's entry rows, conjuncts and back-edge rows over the bound local: each
    global array the function holds (every one in `main`; elsewhere those its `requires` atoms name, at `write`
    or at the atom's amount, under its guard), each array leaf of every structure object in scope, and each
    array leaf of a held structure place of a parameter (guarded and at its amount, as a range place is).
    - **Reach:** a store by name to an element of a global array the function holds as its own place reaches
      that array only (0044 item 8's rule); a store to an array member of a structure local, any store through
      a pointer of the element field, and a call whose `requires` owns a cell of that field reach every array
      place of the field (0043 item 11's rule for members, unchanged).
    - **A global's place is hit by an interval:** `inv_place_disjunct`'s shortcut for a global is the identical
      slot-0 term today; an array global's place is hit when the read's block is `GB_g()` and its offset is in
      `[0, N)`. For a scalar global the emitted text is unchanged.
    - Invariant and measure reads of `g[e]`, `s.a[e]` and `p->a[e]` are framed by these places, with 0059 item
      8's `out-of-bounds` and `uninitialized` obligations.
11. **Sequencing** (0039 item 3, 0059 item 9). Object keys gain the element of a global array and of an array
    member (`g[0]` and `g[1]` are different objects; a global array is no longer one object). Two unsequenced
    accesses to elements of one array, one a write, with indexes that are not both constants, get the
    path-precise `unsequenced` row; the same element named twice with a write is rejected like the compilers
    (`unsequenced_modification`).
12. **Array compound literals** (C; amends 0044 item 7). `(T[]){…}` and `(T[N]){…}` designate an unnamed array
    object of the enclosing block: a hidden array local, allocated at block entry as e1a's are, zero-filled and
    written inside the expression when the literal is evaluated (C's rule: every element first), exited with
    its block; its value is its decay. 0044 item 7's rejections hold for it (C++, file scope, `const`, a loop
    condition, a `for` step, a `do` condition, `return` of its address).
13. **Structure values with an array member** (owner, question 291; Part E; amends 0043 item 7 and 0044 items
    2–5).
    - **The copy** (`t = s`, `*p = *q`, `p->t = s.t`, an initializer from an object). The cell leaves as 0043
      item 7 has them. Per array leaf: the source's permission (a marked row, `permission`) and **every source
      flag** (`uninitialized`: "copies `s`, whose member `s.a` may hold an element without a value"; question
      92's rule, for elements: question 296), the destination's permission at `write` (a marked row); then,
      **unless source and destination are one object**, `copy_arr_<t>(d, s, N, rd)`. An exact self-copy is
      guarded out (`*p = *p` verifies, as today); partial overlap cannot arise (two structure places of one
      type are one slot or disjoint).
    - **`copy_arr_<t>`** is a trusted method (`// cerastes:trusted-contract`), one per element cell type, as
      `zero_arr_<t>` is: it takes and returns full permission to the destination's value and flag fields over
      `n` slots and the amount `rd` (`none < rd`) of the source's value field, and ensures that each destination
      value is the source's old value at the same index, each destination flag is set and each source value is
      unchanged. **It is the contract of the loop `d[k] = s[k]` over two disjoint ranges** (the record argues
      it, with the model of 0028 item 2: disjoint blocks or disjoint intervals; the guard is what makes the
      ranges disjoint); S47 (g) asserts `false` after it and it fails. If S47 (g) does not pass, the copy is
      written element by element under question 295's limit, with no trusted method.
    - **By value.** A structure value is the tuple of its value-view leaves: an array member contributes one
      Viper formal, result or temporary per element (`p_s__k_m<j>_e<i>`). Everything 0044 items 2–5 say of a
      leaf holds for an element leaf: a parameter's copy block is written from the formals at entry; an object
      argument and `return s;` read every element (the base's rows, then each element's permission and flag);
      an `Aggregate` carries one argument per element (zero for the elements a literal does not write).
      `f().a[i]`, and `s.a[i]` of a by-value parameter in a contract, are `ElemValue`: the element at `i` (an
      `ite` chain over the member's elements) with one `out-of-bounds` obligation.
    - **The limit** (question 295): a structure type whose value view holds more than 64 array elements is
      rejected as a by-value parameter, result, argument or literal value (`unsupported_type`, "… more than 64
      array elements passed by value (array values: M4.5f3)"). A copy has no limit.
14. **Computed array types** (amends 0024 item 6 for arrays of scalars).
    - A typedef or alias of an array type is accepted when the type is item 2's, with its bound checked as
      written at the typedef; an array declared through a typedef, `typeof` or `decltype` is the array its type
      names, and its bound was checked where it was written (the typedef, the named array, or the `typeof`'s own
      type operand).
    - As the type operand of `sizeof` and `_Alignof`, and as the type of the operand of `sizeof` when that
      operand names an array (`g`, `s.a`, `p->a`; a local as since e1a), an array type is accepted when **every
      bound in it is read as written** with its obligations at that position; the value is then EDG's (the
      `internal` guard ties the bound). `sizeof(char[65536u * 65536u + 1])` is an `overflow` error at the bound,
      not a rejection. The same rule in static assertions (`scan_type`) and C++ enumerators.
    - **Still rejected:** an array of arrays or of structures and a pointer to an array inside such a type
      (e1e2, M8); the operand a compound literal or a string (M4.6); `__datasizeof`; `_Alignof` of an
      expression; `_BitInt`, vectors, class templates with constant arguments; type traits' arguments (M5.0);
      C's enumerator rule (0018 item 10) first.
    - A bound that names a local constant (`const int N = 4; int a[N];`): the bound is `N`'s own checked
      constant when P82 (f) can read it; otherwise it stays rejected.
15. **The lint** (amends 0012 item 11, 0043 item 13, 0059 item 12).
    - **The block grammar:** where a line has no local (item 5), a quantified row's block is `GB_<g>()` of a
      declared array global or `member(<chain>, k)` with `<chain>` 0043 item 13's canonical chain rooted at a
      formal, `r` or `loc(GB_<g>(), 0)`; the bounds `0` and the literal count the facts state. Nothing else.
    - `alloc_struct_*`: per array member exactly `len(member(X, k)) == N` and the three quantifiers of item 6,
      each (receiver, field) once. `struct_lens`' conjunct: `== N` for an array member, with the count of the
      allocation of that structure. The global facts: `len(GB_g()) == N` for an array global, the same N in all
      three places the facts are pinned.
    - `program_start`: an array global's row group exactly as item 9 orders it (a duplicate group, a missing
      zero rest, a value conjunct for a slot at or past the listed prefix are refused); a structure global's
      array leaf likewise.
    - `copy_arr_<t>` and `zero_arr_<t>` are trusted methods pinned by their clause sets; with `alloc_*` they are
      exempt from "first `requires` is the global facts" (R4's defect).
    - A flag write to an element of a global array or of a member goes through a `Ref` temporary (`lint_flags`
      unchanged).
    - Each rule with a refusing twin.
16. **Categories and tags:** none new. **Facts:** P82, S47.

## 8. Parts, owners, order

| Part | Content | edg-backend | il-analyst | test-mutator |
|---|---|---|---|---|
| **A** plumbing, behaviour-neutral | item 4, nothing accepted yet | `Type::Array` on `StructMember` and `Global`; the three views (`struct_object_leaves`, `struct_value_leaves`, `StructLeaf::elem`; `struct_leaves` untouched); `Global`'s array values; `ArrayZero`, `ArrayCopy`, `ElemValue` and their builders; every kind switch (`same_pure_expr`, `reads_heap`, the sequencing walkers, the printers); `place_text`, `member_place` and `elem_place` for the new chains; `check.cpp` refusing all of it; **the explicit guard in `leaf_cells` and every direct `struct_leaves` loop of `lower.cpp`** (an array-typed leaf is an internal error); the stale comments of `vir.h` 84–86, 291–294, 433, 438–439 | il-analyst-1: P82, S47 | Step 0 (R1–R4, M, R2's staging pins); the lint rules of item 15 against S47's fixtures, with twins |
| **B** (G) global and `static` arrays | items 2, 3, 5, 7–11 for a global array | `vtype`/`global_vtype` (the `const` rule first, then `array_vtype`'s rules); `make_global`, `static_local_decl`, `add_static_local_atoms`; `read_global_init`'s array branch (elements through `constant_initializer`, the zero rest, linkage); the bound as a prologue constant; `subscript_address`, `array_decay`, `address_of`, `global_read` and `assign_target` over `b_.global_addr`; `sizeof(g)` (6486–6494); `contract_element_address` (724–730) and `spec_atom` (9554–9563) for an array global; `application` 5850 and `forall_site` 11208 for a named global array outside a contract; `detect_array_address_test`; `address_target` for `int *gp = g;` (P82 (a)) | **il-analyst-2**: `check.cpp` `global`, `ElemAddr` over `GlobalAddr` (3758–3764, 3772–3814), the named-array rules of quantifiers and applications (`forall_loop_base`, `array_decay_of`, `apply`), the Sequencer's element keys; `lower.cpp`: `elem_addr`'s named base and the wording of `deref`, `store`, `cell_write`; `global_facts`; `program_start_method`'s array rows; `site_cells`, `site_fields` and `atom` for an array global; `call_atoms`, `atom_returns`; `contract_read` (`elem_key`), `wf_place`; `held_keys` and an array place for a held global array; `inv_place_disjunct`; `writes_in` (`elem_array_var`), `key_of_addr`; `fq_loop_read`, `fq_check_rows`, `apply_inv`, `apply_check` for a named global array | Part B's tests; the lint's block grammar for `GB_g()`, the facts with N, `lint_globals`' array row group; Flips A for (G) |
| **D** (T) and (L) | items 12 and 14 | **alone, in parallel with Part B's lowering:** `declaration()` 16885–16892; `declaration_level_checks` 23413–23430; the walk of a type operand that checks each bound as written, in `reject_computed_sizeof_operand` (6495–6506), `scan_type` (6552–6560) and `encode_named_enumerator`; `array_vtype` 534 by P82 (f); `temp_init_value` 13537–13542, `literal_address` 13610–13614 and the literal's decay in `operation()` | il-analyst-2, after Part B's half: `check.cpp` `declare` (2497–2502) and `comma`/`literal_stores_` (4186–4193) for an array literal; `zero_fill` inside an expression | Part D's tests; Flips A and C for (T) and (L) |
| **C** (M) array members | items 2, 3, 5–11 for an array member | `struct_info`'s `case tk_array` (23252; the element rules; question 293's rejection); `collect_init_elems` 17311 (the nested list, elision, designators) and `struct_initializer` (`ArrayZero`, the element stores); `subscript_address` 931–937 and `array_decay` over `dot_field` and `points_to_field` (`b_.elem_addr(place_address(member), ix)`); `sizeof(s.a)`; `in_own_initializer` through an `ElemAddr`; `contract_element_address` and `contract_place_address` for `p->a[i]`; `spec_atom` for `owns(p->a)`; `read_struct_global_init`; **the seven `vir::struct_leaves` callers** (4038 `owning_leaves`, 7757, 13436 `object_value`, 13784 `aggregate_value`, 16001 `heap_view`, 17403 `copy_struct`, 17749): each migrated to the object view or rejecting with a diagnostic | **il-analyst-3**: `check.cpp` `struct_type` (436–458), `ElemAddr` and atoms over a `MemberAddr` of an array member; `lower.cpp`: **the site table below** | Part C's tests; the lint's `alloc_struct_*` and `struct_lens` rules for an array member, the chain grammar, a structure global's array leaf; Flips A for (M) |
| **E** (V) structure values | item 13 | `copy_struct` (`ArrayCopy` per array leaf); `object_value`, `aggregate_value`, `signature_vtype` (the limit); `value_member` and `contract_field_chain` for `ElemValue`; a compound literal of such a structure | il-analyst-3: `check.cpp` (the value view in every value position; `ArrayCopy`; `ElemValue`); `lower.cpp`: the value-view sites of the table, the copy's rows and `copy_arr_<t>` (in `array_alloc_method`'s pattern), `ElemValue` | Part E's tests; the lint's `copy_arr_*` rule; Flips A for (V) |

**Part C's site table** (`lower.cpp`; every site that enumerates a structure's leaves, from the planning's
survey; **audit round 3 gives a verdict per row**).

| Takes the object view in Part C (an array leaf gets its rows) | Takes the value view in Part E; refuses a structure with an array member until then | Never sees one (the encoder rejects the structure first); the guard stays |
|---|---|---|
| `alloc_struct_method` 1177–1247, `alloc_struct_site` 2015; `struct_lens` 2180–2204; `scope_exit` 3945–3965; `site_cells` 6035, `site_fields` 6068; `atom` 6723 (6910–6943); `cov_of` 6527, `leaf_cover_read` 6585; `contract_read` 7963–8270; `wf_place` 22469; `call_atoms` 10046; `atom_returns` 13483; `loop_places` 15249 (a structure local 15589–15602, a structure global in `main` 15433–15458); `program_start_method` 1305–1343 and `global_facts`; `prologue` 20543; `memory_fields` 8470, `mem_scan` 8582; `key_of_addr`; `note_global_member_write` 14819, `writes_in_expr` 14874; `split_candidates` 13093, `given_result_splits` 13141, `struct_ptr_uses` 11149 (an array of pointers is split against its first element only: e1a's listed limit) | `param_vars` 20690, `result_vars` 20709, `contract_env` 12415, `full_exprs` 12462, `ret` 13036, `struct_store` 4577, `struct_cell_write` 8935, the parameter copies of `function` (20927) | `has_owning_leaf` 2244, `leaf_owners_of` 2256, `owner_value` and `owner_cell` 2325–2353, `owner_text` 2356, `global_owners_of` 2374, `exempt_owned_fields` 14094, `named_leaf` (18575–18622) (question 293); `block_cells` 5351 (heap views: question 292); `written_atoms` 15891, `resolve_written` 16004; `struct_local_framed_in` 3209 |

**Staging pins:** R2's pins. After Commit 2 array members are still rejected (`reject_struct_array_member.c` and
R2's member pins stand until Part C), and a structure with an array member is still rejected as a value and in
a copy (R2's two value pins stand until Part E, or are re-pointed to M4.5e1e2 if Part E moves there).

**Order of work.**
1. Step 0 and Step 1. Then, in parallel: il-analyst-1 on P82 and S47; edg-backend on Part A; the test-mutator
   on R1–R4 and M, then the lint rules; the orchestrator on record 0063's draft.
2. **Commit 1:** record 0063 (draft, revised by section 6's tables), the `## M4.5e1e1` chapter, the S47
   fixtures, R2's pins, R4's lint fix, the lint rules. Advisor checkpoint 1, then audit round 1 on it.
   il-analyst-1 ends.
3. **Part A** may be built before round 1 returns (it accepts nothing). It is proved neutral (`vprdiff`: every
   `.vpr` and `.map` identical; the VIR self-test's output identical) and joins Part B's commit. **edg-backend's
   half of Part D** may be built during round 1 too (its rules are item 14's and reject what they cannot read).
4. **Part B's lowering starts only when audit round 1 has returned and the record holds its blocking fixes**
   (invariant 6). edg-backend and il-analyst-2 work in parallel against Part A's VIR; the test-mutator writes
   the tests from the record, expectations pre-registered. Then Part D's lowering half. Audit round 2 on the
   snapshot of A, B and D. **Commit 2. Split point 1.**
5. **Part C:** il-analyst-3 on the lowering (it may start from the audit snapshot while round 2 runs);
   edg-backend on the encoder half. Audit round 3 on the snapshot. **Commit 3. Split point 2.**
6. **Part E,** if split point 2 lets it run: il-analyst-3, then edg-backend's half. Audit round 4 on the
   snapshot, with the pages. **Commit 4.**
7. The doc examples (test-mutator); the user pages (orchestrator, drafted during the waits of steps 4–6);
   `internal/encoding.md` and the developer guide (edg-backend); the fixes of the last audit in one batch; the
   preflight; the box check; record 0063 accepted; status, handoff, `plan.md`'s Done row. **A session that ends
   at a split point still does this step for what it built**, and its last audit round is the round of its last
   commit with the pages (three rounds, not four, when Part E moves).

**Proofs, per part.**
- **Each half alone:** the il-analyst proves its half by the new VIR cases through `cerastes-vir-selftest` and
  Silicon (the new cases only, section 3 rule 5), which need no encoder; edg-backend proves its half by its
  build and by `rejections.py --base` against section 10. Both run `scripts/format.sh`.
- **The joined proof**, once both halves have reported: edg-backend ships the joined tree to the `work` lane,
  builds, runs `scripts/regen_vpr.py` into a directory of its own and `scripts/vprdiff.py` against the baseline
  (**no existing program's `.vpr` or `.map` differs**) and against the previous part's regeneration (the list of
  this session's tests whose Viper changed); then, with builds held, the test-mutator runs **the part's new
  tests and that list, once**, timed. A failure goes through the orchestrator to the half that owns the failing
  file; the joined proof is repeated once, after the last fix, for what that fix changed.
- The audit snapshot is the tree the joined proof passed on.

**Audits** (one resumed Opus reviewer; a snapshot commit, the local audit worktree and the box's audit lane):
- **Round 1, the record with P82 and S47:** reproduce the gates; **the trusted contracts** (`alloc_struct` with
  quantified leaves, `program_start`'s array rows, `copy_arr_<t>`: can any be unsatisfiable or give a pair
  twice; can `copy_arr` be called with overlapping ranges; what a caller learns that is not true of C);
  **overlap by slots for an array leaf** (a cell or a range that points into an array member or a global array
  and escapes the sum rows, the exclusion, the framing disjuncts or `f__wf`'s distinctness facts); the lengths
  (a path on which `len(member(X, k)) == N` is assumed and never discharged); the bound local (a marked row
  whose reason could come from something else on the line); the poison (a site that reads an array member as
  one cell without an internal error); initial values (an element whose constant is read from EDG's folded
  value; the zero rest over the wrong interval; linkage); 0024 item 6's lift (a bound that is not read as
  written; a type operand whose inner array is skipped); every row of section 1's table has a rejection.
- **Round 2, Parts A, B and D:** Part A's neutrality; a global array read, written or framed at a slot other
  than 0 without a row; `inv_place_disjunct` for a scalar global unchanged; an array global the function does
  not hold used in a loop; `program_start` against item 9 (and the lint's pin of it); a `static` local array's
  implicit atom in a caller; the Sequencer's keys (two elements treated as one object, or one as two); a
  `const` array reaching the array rule; computed types (a wrapped bound accepted; `sizeof` of an operand that
  is not a named array); an array literal read before it is written or after its block.
- **Round 3, Part C:** **the site table, a verdict per site**; a structure with an array member reaching a
  value-view or owner site without a rejection; `p->a[i]` with a liveness row, or without the root's null row;
  the alias trap `f(&s, &s.a[1])`; `owns(p->a)` beside `owns(*p)`; a loop that writes through an `int *` and
  keeps an array member's values; the scope exit of a structure local whose array member a callee kept;
  initializers (elision accepted, an element written twice, the zero fill skipped); a structure global's array
  leaf.
- **Round 4, Part E (with the pages):** the copy's rows against item 13 (a source element without a value
  copied; the guard; a destination held at a fraction); the value view against the object view (an element
  dropped or doubled between a formal and its cell); the limit; `ElemValue` outside the member; the pages'
  statements about what is checked and trusted. When Part E has moved, the last round's snapshot holds the
  pages instead.
- **Blocking fixes** are re-audited by the next round, or by a short read of the fix's diff when it is the last
  round's. Should-fix items not taken go to handoff §3.

**Pre-agreed cuts, in this order** (each is cut by its spike's table, by an audit finding, or at a split point
with the advisor; each leaves its pin, re-pointed to M4.5e1e2, and the pages say what landed): (1) Part E's
by-value half (the copy stays); (2) Part E whole; (3) a bound that names a local constant (M8); (4) array
compound literals; (5) an array member of a nested structure; (6) structure globals and `static` structure
locals with an array member; (7) a named global array as an argument of `cerastes_forall` or of a `pure`
function. **Not cut without the owner:** global and `static` arrays of scalars with their contracts, calls and
loops; array members in structure locals and through pointers with `owns(*p)`, `reads(*p)` and `owns(p->a)`;
typedefs and `sizeof` of array types.

**Split points.** At Commit 2 and Commit 3 the orchestrator reports `git diff --numstat <base> -- src/ include/`
without the self-test cases against section 1's sizes, the hours used against the twelve-hour cap (question 77)
and what is still owed, and takes the decision to the advisor (checkpoint 3).
- **Split point 1 (after Commit 2).** Part C runs unless more than 7 hours are used: then the session closes
  there and Part C opens M4.5e1e2's first session from this plan, with no second planning.
- **Split point 2 (after Commit 3).** **Part E runs only if Parts A–D are under 3,300 lines and under 7.5 hours
  are used.** Otherwise the session closes and Part E is the first part of M4.5e1e2 (cut 2; question 291 holds:
  it is in M4.5e1e). This needs no question to the owner.
- **The owner is asked** only if the remaining work would pass the twelve-hour cap, or if the orchestrator and
  the advisor judge that its context will not carry what is left of a part already begun.

## 9. Tests (`tests/memory/`, scalar-only ones in `tests/scalar/`, negatives in the area's `negative/`; `.c` in `c23, c++20`; each positive has a twin failing for one reason at one line)

**At most 150 test files in all** (e1a added 223, e1b 200, e1d 139): one positive and one error twin per rule,
one pin per rejected row; a rule that two shapes exercise the same way gets one of them. The part's cap is in
brackets. A test's expectation is pre-registered from the record before its first run (question 160 for
corrections, each listed in status).

- **Part B [40].** `global_array_*`, `static_array_*`. A global array read and written by index in `main`
  (twin: an index outside, `out-of-bounds`). Initial values: a full list, a partial list with the zero rest, no
  initializer with internal linkage, an external C tentative definition and a C++ external array as unknown
  (twin for each known form: a `check` on a wrong value; for the unknown form: a `check` that relies on a
  value). `owns(g)` in a callee that writes, `reads(g)` in one that reads, called from `main` (twins: a write
  under `reads`, `permission`; a caller that does not hold `g`, `permission` at the call). An element read in a
  `requires`, an `ensures` with `old`, a `decreases` (twin: unframed, `permission`). A loop that sums and one
  that fills a global array (twins: `invariant-preserved`; the entry row after a callee kept it). `g` passed
  to `owns_range(p, n)` and `reads_range(p, n)` (twin: `n` past the array, `permission` or the shape row as e1b
  has it). A `static` local array with its implicit atom through a caller (twin). `sizeof(g) / sizeof(g[0])`.
  Element types: `bool`, an enumeration, a pointer element pointing at another global. Sequencing: `g[0] =
  g[1]++` verifies; `g[i] = g[j]++` is `unsequenced`. A named global array in a `check` with `cerastes_forall`
  and with a `pure` application (twin each). Pins: section 1's rows for globals (a `const` table naming M4.6,
  `owns_range(g, n)`, `&g[2]` as an initializer, a list of 1,001 elements, a designator, `if (g)`, a constant
  index outside, a block-scope `extern` array).
- **Part D [30, the flips included].** A typedef of an array type at file and at block scope with a local, a
  global and a member declared through it (twin: an index outside); `typeof(a) b`, `typeof(int[3]) b` (C),
  `decltype(a) b` and `using T3 = int[3]` (C++); `sizeof(int[3])`, `sizeof(vec3)`, `sizeof(s.a)`,
  `_Alignof(int[3])`, a `static_assert` over `sizeof` of an array type (twins: a `check` on the wrong value; a
  false static assertion); a wrapped bound in a type operand (`overflow`, at the bound). A bound naming a local
  constant (unless cut 3). An array literal's decay in an initializer, as an argument and subscripted (twins: an
  index outside; a read through the pointer after the literal's block, `lifetime`). Pins for item 14's "still
  rejected" list and item 12's rejections.
- **Part C [50].** `struct_array_*`. A structure local with an array member: element writes and reads beside
  the other member (twin: an index outside; an unwritten element, `uninitialized`). Initializers: a nested list,
  a designated member with a list, a partial list with the zero fill, `{0}`, `{}`, non-constant elements (twin:
  a `check` on a wrong value; pins for elision, an array designator, a repeated member in C). Through a pointer:
  `owns(*p)` in a callee that writes `p->a[i]` and `p->n`, `reads(*p)`, `owns(p->a)` alone (twins: `p->n`
  written under `owns(p->a)`, `permission`; a null `p`, `null-dereference`; a callee that keeps the array). The
  alias trap `f(&s, &s.a[1])` (`permission`; the positive passes `&other`). Element reads in a contract with
  `old` (twin: `postcondition`). The decay `f(s.a, 4)` to `owns_range` and `reads_range` (twin). `&s.a[i]`
  stored and read back; `return &s.a[0]` rejected (R14). Loops: over `s.a`, over `p->a` under `owns(*p)` and
  under `reads(*p)` (twins: `invariant-preserved`; a write under `reads`). A nested structure's array member
  (unless cut 5). A structure global and a `static` structure local with an array member, with initial values
  and a loop in `main` (unless cut 6). A scope exit after a callee kept an element (`lifetime`). `sizeof(s.a)`.
  Sequencing between two elements of a member. C++ spellings in one `.cpp`. Pins: section 1's rows for members
  (an owning member beside an array member, a marker on such a pointer, `malloc(sizeof(struct S))` of such a
  structure, `owns_range(p->a, n)`, an array member of structures, a flexible member stays M8, a `const` array
  member M4.6).
- **Part E [30].** A copy between two structure locals, through pointers, and an initializer from an object
  (twins: an element of the source unwritten, `uninitialized`; the destination only readable, `permission`; a
  `check` on a wrong element after the copy); `*p = *p` verifies. A by-value parameter read and changed in the
  callee, with the caller's object unchanged (twin: a `check` that the caller's element changed); a by-value
  result; `f().a[1]` and `f().a[i]` (twin: `out-of-bounds`); `s.a[i]` of a by-value parameter in a `requires`
  (twin: `precondition`); a compound literal of such a structure as an argument and under `&`. The limit: 64
  elements accepted, 65 rejected.
- **VIR self-test:** the session's cases in the next free case file; `VIR_CASES_MIN` raised.
- **Byte identity:** `scripts/vprdiff.py` against the baseline after each part: no existing program's `.vpr` or
  `.map` changes in this session.

## 10. Flips (pre-approved with this plan; any other changed outcome goes to the owner)

The planning's survey listed every array rejection in `tests/` with its expect line. A flip below is made by
the part named, on the program as it stands (line numbers kept, a comment in place of a lost expect line).

- **A. Retired for a positive of the same program, with an error twin:**
  - Part B: `tests/memory/negative/global_array.c`, `array_reject_static_local.c`.
  - Part C: `tests/memory/negative/reject_struct_array_member.c`.
  - Part D: `tests/memory/negative/array_reject_compound_literal.c` (unless cut 4);
    `array_reject_typedef_local.c`, `array_reject_using_local.cpp`, `array_reject_typeof.c`,
    `array_reject_typeof_type.c`, `array_reject_decltype.cpp`; `array_reject_const_bound.cpp` (unless cut 3 or
    P82 (f)'s "if not": then it stays, re-pointed to M8); `tests/scalar/negative/reject_sizeof_array.c`,
    `reject_typedef_array_file.c`, `reject_typedef_array_block.c`, `reject_typedef_array_used.c` (both of its
    lines), `reject_alias_array.cpp`, `reject_alignof_array.c`, `reject_sizeof_typeof_array.c`,
    `reject_static_assert_sizeof_array.c`, `reject_enumerator_sizeof_array.cpp`.
  - Every staging pin R2 adds, by the part that lifts it.
- **B. Re-expected from a rejection to an error** (item 14: the bound is now checked as written):
  `tests/scalar/negative/reject_sizeof_array_huge.c` and `reject_contract_sizeof_array.c` become `error overflow`
  at the line of their wrapping bound (the exact line and count from the first run, question 160).
- **C. Stay rejected; only the message or the `Pins:` comment changes** (they name e1e today and name their
  real lifter after): `array_reject_struct_elements.c`, `array_reject_struct_subscript.c`,
  `range_reject_struct_pointee.c`, `array_reject_array_of_arrays.c`, `array_param_reject_array_of_arrays.c`
  (both of its lines), `tests/scalar/negative/reject_sizeof_2d_array.c` (M4.5e1e2);
  `range_reject_base_member.c`, `reject_owning_field_count.c` (M4.5e2 alone);
  `reject_pure_struct_pointer_parameter.c` (M4.5f1 alone); `array_reject_extern_local.c` (M8);
  `tests/scalar/negative/reject_typedef_function_array_param.c` (M4.7); `forall_reject_*` and `reject_pure_*`
  pins whose text says "a global array: M4.5e1e" for a contract position (M4.5e1e2).
- **D. Unchanged:** `reject_struct_flexible_member.c`, `array_reject_pointer_to_array.c`,
  `array_reject_thread_local_array.c`, `array_reject_const_array.c`, `reject_sizeof_pointer_to_array.c`,
  `reject_sizeof_array_reference.cpp`, `reject_sizeof_compound_literal.c`, `reject_sizeof_string.c`,
  `reject_datasizeof_array.cpp`, `reject_decltype_array.cpp`, `reject_sizeof_std_array.cpp`,
  `reject_enum_list_sizeof_array.c`, `reject_enum_list_typeof_array.c`, `reject_skipped_is_same_array.cpp`,
  `reject_skipped_builtin_is_same.cpp`, `array_reject_alignof_expr.c`, `array_reject_alignof_expr_cxx.cpp`,
  `array_reject_datasizeof_expr.cpp`, and every pin that names M4.5e2, M4.6, M4.8 or M8. **If R1 or a part's
  `rejections.py` run shows one of them changed, it goes to the owner.**
- **E. Lint self-tests** that pin "this shape is refused" for a shape item 15 now admits (`len(GB_g()) == N`,
  a quantified `program_start` row, an `alloc_struct` clause with a count): the test-mutator reports which
  before editing, and status lists them.
- **F. Front-end tests, goldens and existing Viper:** none (`include/cerastes.h` does not change; **if a golden
  changes, it goes to the owner**). Tag coverage: no tag loses its last test (the three `unsupported_*` tags
  keep hundreds).

## 11. Docs (invariant 8; every example is a test)

- **`memory.adoc`:** "Structures" (354–480): array members — the layout in a sentence, the accesses, the
  initializer forms, what `owns(*p)` and `owns(p->a)` give, copies and by-value passing as far as Part E landed;
  465–466's not-supported list. "Arrays" (480–790): a subsection "Arrays in structures and in static storage"
  that points both ways; 656–657; the quantifier section's not-yet list (728–741). "Global variables" (790–852):
  array globals — initial values (the listed elements and the zero rest, unknown values by linkage), `owns(g)`
  and `reads(g)` as every element, loops in `main`; 847–849. "`static` local variables" (852–877). "Compound
  literals" (449): array literals in C. "What is not supported yet" (1826–): section 1's table rows a user can
  meet, each with its session.
- **`verifying.adoc`:** 1242–1244 and 1269–1275 (which arrays and which array types are accepted now; a wrapped
  bound in a type operand is an `overflow` error; what stays rejected), 1107. **`overview.adoc`** 98–99.
  **`annotations.adoc`:** `owns` and `reads` of an array global and of an array member (near 965–966: a range
  still takes a pointer parameter), 1035–1036 (a named global array as a quantifier's argument, unless cut 7).
- **`error-messages.adoc`:** `out-of-bounds`, `permission`, `uninitialized` and `lifetime` gain the forms a
  member's and a global's element give them; the copy's `uninitialized` form (Part E).
- **Examples** (`doc/modules/ROOT/examples/memory/`): `array_member.c`, `array_global.c`, and
  `struct_array_copy.c` if Part E landed, each with a failing variant; `struct_global.c` and `array_local.c`
  unchanged.
- **The tutorial:** no new section (question 298: e1e2 completes "arrays in aggregates"); `tutorial/arrays.adoc`'s
  "What is next" and any sentence a part makes false.
- **`tests/README.md`** only if a rule of the harness changes (R4's lint fix does not). **`internal/encoding.md`**
  §2 (the array types of a global and a member, the three views, `ArrayZero`, `ArrayCopy`, `ElemValue`), §3
  "Arrays of scalars in structures and static storage (M4.5e1e1)", §4 (the row groups and names); the developer
  guide where it names the leaf enumeration and the trusted methods; one amendment line in each amended record.

## 12. Questions, stops, acceptance

**Every question goes to the advisor first** (owner, 2026-11-25: "If you or the subagents have questions first
consult the Fable advisor, and then if you still need an answer or guidance you may stop and ask me"). An agent's
question goes to the orchestrator; the orchestrator states it to the advisor with the facts and a recommended
answer; the agent goes on with what the question does not block.
- **The advisor settles:** a choice inside the record's stated rules; a choice between fail-closed
  alternatives (a rejection with a pin, a listed false alarm); test mechanics, message wording and a
  question-160 correction; a table's "the advisor" row; a cut on section 8's list and a split-point decision
  under its rules; whether a slow test is restructured or moved under question 285's rule.
- **Goes to the owner, and the work it blocks waits:** an `expect:` change outside section 10 and question 160
  (invariant 1); **any existing program's Viper or map changing**; a golden changing; a cut that section 8
  protects or that is not on its list; a soundness finding that has no fail-closed fix (a wrong program
  verified, a trusted contract the auditor does not accept); a protected spike row failing in every form; the
  twelve-hour cap; anything the advisor says is the owner's.
- **Each answer the advisor settles is logged** in `internal/handoff.md` §8 under "Settled with the advisor in
  M4.5e1e1, for the owner's review": numbered from 300, the soundness statement first (fail closed or not), the
  answer taken, the alternative. The status entry lists them.
- **While the owner is away,** a question that is the owner's waits; the session goes on with everything it does
  not block. The overnight rule (build the fail-closed recommendation and log it) applies only if the owner
  gives it for this session.

**Other stops:** an exit 3 or a time over 120 s not settled within its part (section 3 rule 3); a second
failure of a joined proof on the same cause.

**Acceptance.**
- The preflight of section 3, once, after the last audit's fixes. Advisor checkpoint 5.
- `CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` **once**, after the last code change, on the x86-64 box
  (the branch as a git bundle into `~/cerastes-s8/p-ubuntu-26.04`): green.
- The status entry (sizes measured, the hours per step against section 3's target, deviations, corrections
  under question 160, each cut taken, each advisor-settled question) and the handoff with its Questions section
  and what e1e1 leaves (section 15). Commits stay local.

## 13. Inputs (moved here from `internal/handoff.md` §3 and `.claude/plans/m4-5e1-arrays.md` sections 7 and 9 at this planning)

- **Decided in M4.5a** (0028 items 2, 5 and 16; S26): the member-block layout and `GB_g()` blocks; over an array
  of structures one quantifier per member and field, each with its own receiver as its trigger (e1e2's);
  `a[0][3]` and `a[2][0]` fail their bounds (e1e2's). S26's fixtures: `tests/viper-smoke/memory/s26_member_*.vpr`.
- **From e1a** (record 0059; P78): a braced initializer's IL (trailing zeros not materialized, designators as
  `ck_designator`, non-constant elements in written order); **P38 for static storage** (a list of more than
  1,000 elements keeps no `expr`: read every element through `constant_value`, which rejects that shape, never
  through `ck_aggregate`'s values); a bound that names a local constant (P78 F1); EDG's #175, #1162 and #138 as
  errors in verify mode; the escape scan (0056 item 11) — the survey found it already treats `s.a` used as a
  value as an escape of `s` when the address is an `ElemAddr`/`MemberAddr` chain over `AddrOf(s)`, and the
  access `s.a[i]` as none.
- **The handoff's note on `is_global_ref`** is corrected in section 4: it is one place for slot 0 only; an array
  global needs the interval hit.
- **From e1b** (record 0060): ranges over structures, array members and global arrays were left to e1e: **this
  plan takes the whole-array atoms (`owns(g)`, `owns(p->a)`) and leaves partial ranges on those bases to
  M4.5e2** (questions 236 and 237's neighbours; section 1's table), and ranges over structures to e1e2.
- **From e1c and e1d** (records 0061 item 1, 0062 items 1 and 4): structure members and global arrays under a
  binder, and a named global array as an argument: the named global array outside a contract is Part B's (cut
  7); the rest is e1e2's and M4.5f1's.
- **The survey's contradictions, settled:** `plan.md`'s M8 entry "`int a[][3]` is e1e's" is e1e2's; 0024's "until
  M4.5e" also names `_BitInt`, vectors and non-type template arguments, which stay rejected; "M4.5e" in 0022 and
  0047 for `n * sizeof(T)`, `calloc(n, …)` and `realloc` is M4.5e2; e1d's table row "structure members, array
  members in a read `pure` body: M4.5f1, e1e" is M4.5f1's.
- **Costs:** S43 (h): three arrays and two loops 3.4 s on the Mac; S26: an array of structures by member blocks
  3.7–4.6 s. The box's Z3 is 3–6.5 times slower than the Mac's per hard refutation (question 285's two tests):
  times are measured on the box.
- **Lessons that bind here:** an allow-list is never computed by `grep` before a build (this session expects no
  existing Viper to change); the audit worktree needs no bootstrap (a source-only `git worktree add --detach`
  plus the lane's build of exactly that source); proofs made while another agent builds are provisional; a twin
  that runs further than its positive needs an `ensures` that holds at the wider state; after a failed check
  Silicon goes on with the failing value unconstrained; before a question about a lowering change, run the
  end-to-end vprdiff and put every changed VIR case through Silicon; `tail -n 3`, not `tail -3`, on the box;
  containers are started with `--init`.

## 14. M4.5e1e2 — Arrays of structures and arrays of arrays (scope and inputs; its own short planning)

**It plans after e1e1,** from e1e1's as-built names and S47 (x)'s two verdicts: a short planning in e1c's form
(decision tables, the ensemble assigned), not a second survey of the whole tree. Estimate today: 1,700–3,100
`src/` lines without what e1e1 hands over (`encode.cpp` 500–950, lowering and checks 1,200–2,100).

- **Scope.** Arrays whose elements are structures: `struct S v[3]`, `v[i].x`, `p[i].x` on a `struct S *`,
  `&v[i]`; `owns_range` and `reads_range` over structures (one quantifier per leaf, 0028 item 2); their loops
  (an array place per leaf field); global arrays of structures; an array member of structures. Arrays of
  arrays: `int m[2][3]`, `m[i][j]`, `m[i]` as a value, `int a[][3]` parameters, `sizeof(int[2][3])`; rows are
  `member(loc(b, i), 0)` (0028 item 16), so a matrix's permission has two bound variables. `cerastes_forall`
  over `p[i].x`, over a member's array and over a global array in a contract.
- **Handed over by e1e1 if cut or moved:** Part E (structure values with array members: question 291 keeps it
  in M4.5e1e); cuts 3–7; heap storage of a structure with an array member (question 292); a structure with an
  array member beside an owning or marked member, and markers on pointers to such structures (questions 293,
  294).
- **The two unknowns** S47 (x) records: the member-tail receiver in the encoder's shapes with e1b's loop
  conjuncts per leaf, and a permission over two bound variables (one S26 fixture is all the evidence today). If
  the two-variable form is refused or slow, a matrix is the first cut (M8).
- **Survey facts already in hand** (the planning's surveys at `68384d1`): `check.cpp` 3787–3791 and
  `range_atom_operands` 1411–1416 refuse a structure pointee; `place_root` (`vir.cpp` 1447) stops at an
  `ElemAddr`, so the contract rules for `Deref` (3423–3432) and `MemberAddr` (3626–3637) refuse `p[i].x`;
  `member_addr` (`lower.cpp` 9577–9581) does not pass the place mark to an `ElemAddr` base; `member_place` and
  `elem_place` (`vir.cpp` 749–834) would spell `(&p[i])->x` and `(&m[i])[j]`; `fq_collect` and
  `fq_binder_class` (`lower.cpp` 7363–7423) know only `Deref(ElemAddr(ForallArg, i ± c))`; `site_fields` returns
  one suffix for a range; `escapes_in_expr` and `struct_local_framed_in` never record an array-of-structures
  local (safe: it is never framed out); `array_alloc_method`, `zero_fill` and `array_scope_exit` are per element
  cell type; `encode.cpp`: `array_vtype` 551 and 556, `collect_array_elems` 1151–1157 (a nested list),
  `place_address` 13907, `subscript_address` 951–959, `contract_element_address` 757–761, `spec_range_atom`
  9856, `owning_count` 3831, `read_forall_function` 10795–10804, `check_signature` 2297–2313 (which reports
  `int a[][3]` twice today, by two rules), `copy_struct` 17353 and `struct_value_node` 13393 for a `p[i]`
  source.
- **M4.5e2** needs e1e2 only for `malloc(n * sizeof(struct S))`; the rest of e2 can run after e1e1.

## 15. What e1e1 leaves, and the questions

- **For handoff §3 at the session's end:** M4.5e1e2 takes section 14 and whatever a cut or a split point moved;
  M4.5e2 takes partial ranges on member and global bases, `&g[2]` and `g + i`, atoms on one element, and the
  re-argument of the `offset_of == 0` backstops (unchanged by this session: no owner gains an interior pointer
  here); M4.5e1f takes the first-slot amount of an array leaf with question 238; M4.5f1 takes `pure` functions
  over structures and globals; M4.5f3 takes structure values of more than 64 array elements; M4.6 takes `const`
  arrays and string-initialized arrays; M4.8 takes arrays of owning pointers and e1a's first-element split of
  an array of pointers; M8 takes designators, pointers to arrays and the rest of section 1's last rows.
- **Answered by the owner at this planning (2026-11-25):**
  - **290** M4.5e1e is two sessions, cut "scalars, then structures" (the recommendation; the alternatives were
    `plan.md`'s "aggregates, then storage" and one session with split points).
  - **291** A structure with an array member is copied and passed by value in M4.5e1e: "Copy and by-value now"
    (the recommendation was the copy as a last part and first cut, by-value in M4.5f3). Built as item 13 and
    Part E.
- **Defaults this plan builds on, for the owner's review** (each fail closed: a rejection with a diagnostic and
  a pin, never a wrong proof; answer in `internal/handoff.md` §8 before the session, or leave them):
  - **292** Heap storage of a structure with an array member (`malloc(sizeof(struct S))`) is rejected in e1e1
    and lifted by M4.5e1e2. Alternative: a sixth part here (about 200–350 lines: the typed view, `free`'s and
    `owns_block`'s rows, the leak check).
  - **293** A structure with both an array member and an `owning` or marked member is rejected in e1e1
    (M4.5e1e2). Alternative: accepted now, with the eight owner sites of section 8's table audited for it.
  - **294** `ref_to_uninit` and `must_init` on a pointer to a structure with an array member are rejected in
    e1e1 (M4.5e1e2). Alternative: per-element flag rows now.
  - **295** A structure value of more than 64 array elements is not passed or returned by value (M4.5f3); a
    copy has no limit. Alternative: another number, or no limit (Silicon then decides by timing out, exit 3).
  - **296** A whole-structure copy needs every element of the source's array members initialized (question 92's
    rule for members, extended; stricter than C, which lets a partly written buffer be copied). Alternative:
    the flags follow the copy, element by element.
  - **297** If Silicon does not take the trusted `copy_arr_<t>` (S47 (g)), the copy is written element by
    element under 295's limit, with no trusted method, and a longer copy is rejected (M4.5f3).
  - **298** No new tutorial section in e1e1; M4.5e1e2 completes "arrays in aggregates" and writes it.
  - **299** Question 285's answer is the standing rule for a test that is slow on the box because of what it
    must refute: `tests/long/` with a short default-tier twin, settled with the advisor at its part.
- **Open questions: none.** A question that arises in the session goes to the advisor first (section 12); the
  session's questions start at 300.
