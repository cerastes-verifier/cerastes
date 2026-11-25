# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, M4.5a's
`git show 72e30b3:internal/status.md`, S5's `git show 794e960:internal/status.md`, M4.5b1a's
`git show bdc6bd0:internal/status.md`, M4.5b1b's `git show 8003dca:internal/status.md`, M4.5b1c1's
`git show eab4058:internal/status.md`, M4.5b1c2's `git show a5e3e30:internal/status.md`, M4.5b1c3's
`git show 5ffb1a2:internal/status.md`, M4.5b2a's `git show 38877b4:internal/status.md`, M4.5b2b's
`git show 1bfa705:internal/status.md`, M4.5c1's `git show cbdcce5:internal/status.md`, and M4.5c2a's
`git show 7f11706:internal/status.md`, M4.5c2b's `git show d26a87c:internal/status.md`, and M4.5d1's
`git show 9d98fe8:internal/status.md`, M4.5d2a's `git show bb41d55:internal/status.md`, S6's `git show 9255b93:internal/status.md`, M4.5d2v's `git show c2bc48d:internal/status.md`, S7's `git show a660abc:internal/status.md`, M4.5d2b1's
`git show c1b691d:internal/status.md`, S8's `git show 15bcdc6:internal/status.md`, M4.5d2b2a's
`git show 19d977a:internal/status.md`, M4.5d2b2b's `git show e3f4b9f:internal/status.md`, M4.5d2b2c's
`git show 023ea50:internal/status.md`, M4.5d2b2d's `git show d88a457:internal/status.md`, M4.5e1a's `git show 685034b:internal/status.md`, M4.5e1b's `git show 7964d33:internal/status.md`, M4.5e1c's `git show c65f618:internal/status.md`, and M4.5e1d's `git show 4e28487:internal/status.md`.

## M4.5e1e1 — Arrays of scalars in structures and static storage (2026-11-25): done; on the branch, local only; the full check.sh not run (owner)

Plan: `.claude/plans/m4-5e1e1-arrays-of-scalars.md` (written at M4.5e1e's planning; executed without re-planning,
from its section 5). Record 0063 (amends 0012, 0024, 0028, 0041, 0043, 0044, 0059, 0060, 0061, 0062). Read
`internal/handoff.md` next: §1, §2, §3 "From M4.5e1e1", §8 (questions 301–308 settled with the advisor, 304 open).
**Branched from local `main` = `4e28487`** (the planning's commits were not on `origin`: the owner had not pushed).
Session from 2026-11-25 03:04 UTC; the owner went to bed at about 03:24 UTC giving the overnight rule ("if you have any
questions you may auto accept the recommended answer after your normal routine. Just document the decisions made in
the handoff documents"); every question went to the Fable advisor first and none had to be decided by that rule.

**Done** (commits on `m4.5e1e1-arrays-of-scalars`, local only):
- **Commit 1** `d9a51fc`: record 0063's draft revised by P82 and S47 (every row of both tables on its first branch; cuts
  3, 4 and 5 not needed), the chapter `## M4.5e1e1` of `internal/il-inventory.md`, 44 S47 fixtures (46 with audit
  round 1's two copy-implementation fixtures, question 307), nine staging pins, R4's lint defect fixed (a global beside
  a zero-filled local array was refused) with its test pair, the lint's rules of record item 15, `scripts/box_lane.sh`.
- **Commit 2** `3935296` (Parts A, B, D and audit round 2's fixes): VIR plumbing (three leaf views, the poison);
  global and `static` local arrays; array typedefs, `typeof`, `sizeof` of array types; array compound literals in C;
  round 2's B1/B1' (question 308).
- **Commit 3** `6aad43c` (Part C and audit round 3's fixes): array members in every storage of a structure.
- **Commit 4** (Part E and audit round 4's fixes): whole-structure copies of a structure with array members through the
  trusted `copy_arr_<t>` (S47 (g)'s clause set; the guard only where a place is reached through a pointer), by-value
  parameters, results, arguments and literal values up to **7** array elements (question 309), `f().a[i]` and contract
  `s.a[i]` as `ElemValue` with its `out-of-bounds` row; the pages, `internal/encoding.md`, the developer guide.

**Audits** (Opus; four instances, each fresh from the earlier reports — a deviation: the plan had one resumed
auditor; each instance's context filled in one round): round 1 on Commit 1 (no blocking; S1–S7 taken);
round 2 on A+B+D (**B1: a write inside a compound literal used as an object was invisible to the sequencing rules,
unsound since M4.5c2a for `&(T){…}` and `*&(int){…}`**; fixed fail closed, question 308; its re-read found B1', a
writing literal as a whole element of a braced list, fixed in the list rule; S3 a file-scope typedef bound naming a
constant, fixed); round 3 on Part C (no blocking; S1–S4 taken); round 4 in two halves — 4a on Commit 3 and the pages
(no blocking; S1 the lint and pointer-element array members, S2–S3 the pages, all taken) and 4b on Part E (no blocking; S1
five Part E tests already corrected, S2 a false message for a C++ temporary, S3 the copy's message and the pages, all
taken). Reports: `build/m4.5e1e1/auditor-report-{1,2,2b,2c,3,4a,4b}.md`.

**Advisor-settled questions** (handoff §8): 301 (the system-typedef pin: one untagged `jmp_buf` pin), 302 (an unused
block-scope array typedef stays rejected; Flips A → C for `reject_typedef_array_block.c`), 303 (C++ `{0}` over an
array member rejected), 305 (Part D over its test cap), 306 (heap-read contract places with an array member rejected,
M4.5e1e2), 307 (the copy's implementation fixtures), 308 (B1/B1'), 309 (the by-value limit 7, not 64: 64 timed out
at 600 s); **304 open for the owner** (C++ `{0}` over a
nested-structure member, accepted since M4.5c1 although clang++ warns at `-Wall`).

**Deviations from the plan, for confirmation:**
- Agents replaced as their contexts filled: edg-backend 1→4, il-analyst 1→4 (il-analyst-4 took Part C's round-3
  fixes and Part E, the plan's il-analyst-3), test-mutator 1→4, the auditor 1→4.
- Commit 2 was cut from an auxiliary worktree at the proved snapshot `j2` plus the round-2 fixes (Part C was already
  being built in the main worktree); Commit 3 was composed from the frozen Part C encoder (`refs/tmp/e1e1-srcC`),
  the VIR after round 3's fixes (`refs/tmp/e1e1-r3fix`) and named test files, while Part E was built in parallel.
- Part E's encoder half was started before split point 2's decision, on the advisor's projection (checkpoint 3: Part E
  fits whole only if Commit 3 lands by about 10:30 UTC; it landed at 09:13).
- Mac single-file probes during development (edg-backend's, il-analyst-1's P82, test-mutator-1's R2 before the box
  rerun); one extra S47 debug run (`s47_g_dbg`, deleted); S47 (a) re-runs after a fixture change.
- 68 Part C programs whose Viper changed from `j4` to `j5` only in a trusted contract's comment (round 3's N5), with
  byte-identical maps, were not rerun (rule 1 read as semantic identity).
- `struct_array_decay.c` (a Flips A positive) changed one line: its helper `h(int *p)` got a `live(p)` contract (an
  unconstrained pointer parameter's use is a `lifetime` row).

**Corrections of new tests after their first run** (question 160; each a test-design error, no verdict changed;
details in `build/m4.5e1e1/test-mutator-report-{6,12,16,19}.md`):
- Parts B/D (report 6): `static_array_implicit_atom.c` (`twice() - twice()` is `unsequenced` by 0041 item 12: two
  statements); EDG's #550 in `global_array_values.c`, `global_array_escape.cpp`, `global_array_pointer_element_wrong.c`,
  `global_array_reads_write.c` (a variable used only in a `check`); `frontend-warning` lines added to
  `array_literal_reject_return.c`, `global_array_reject_initializer_address.c`, `global_array_reject_long_list.c`;
  `global_array_reject_expressions.c` (a one-`return` pure function; `g == NULL` for `if (g)`).
- Part C (report 12): `struct_array_reject_annotations.c` (a caller of the declared `mk`); `struct_array_decay.c`
  (its helper's `live(p)` contract; listed above); the staging pins `struct_array_reject_by_value_param.c` (its member
  line dropped) and `struct_array_reject_copy.c` (`unsupported_declaration @ L10` → `unsupported_expression @ L18`),
  both then retired by Part E's Flips A.
- Part E (report 19): `struct_array_copy_pointers.c`, `negative/struct_array_copy_dest_reads.c`,
  `negative/struct_array_copy_self_wrong.c`, `struct_array_ptr_elems_owns.c` and its `_wrong` twin (contracts that
  state what the checks read; `ensures … live(x)`; a bound against an `overflow`).
- Before any run: Flips A → C for `reject_typedef_array_block.c` (question 302); the two literal pins for a `for` step
  and a `do` condition (one deleted, question 305); the doc example `array_global_index_error.c` (#550).
- `compound_literal_reject_list_effects.c` was edited after it passed and restored to Commit 2's text (advisor).

**Not done / limits** (handoff §3 "From M4.5e1e1"): no cut was taken (cuts 1–7 all stayed in). Record 0063's listed
false alarms: a store by name to any member of a structure local reaches every array place of that field (item 10,
0043 item 11); `f__wf` does not tie a member element's two states; a `static` structure local's implicit atom does
not cover a partnerless atom through an array leaf (item 7); a call passing several by-value structures near the
limit may time out (exit 3, item 13). Everything item 1's table leaves to M4.5e1e2, M4.5e2, M4.5f1, M4.5f3, M4.6,
M4.8 and M8.

**Checks** (every test run on the box, `~/cerastes-e1e1/{base,il,edg,work,audit}`; times in the agents' reports):
- **Neutrality at every part:** no pre-session program's `.vpr` or `.map` changed (6,403 of 6,403 byte-identical at
  each joined proof `j1`–`j7`; the last, `refs/tmp/e1e1-j7` = `7ac066a`: 6,683 programs regenerated, 0 missing, no
  exit 3); the 2,318 baseline VIR cases byte-identical; `rejections.py --base` changed no pre-session entry.
- **At `j7`** (edg-backend report 16): VIR self-test 2,448 lowering and 398 rejection cases, 0 problems (`VIR_CASES_MIN`
  2448); `lint_viper.py` clean over 6,683 programs and 2,448 cases; `test_lint_viper.py` 1,006 OK; `format.sh --check`
  91 files; `py_compile`. On the Mac: `build_docs.sh` exit 0, `check_doc_examples.py` source (26 pages, 0 problems) and
  `--html` (27 files, 0 problems); `test_run_tests.py` 1,287 OK (1 skipped) after the last harness change.
- **New tests, each run once on the box, timed** (reports 6, 12, 16, 19 and the last run): Parts B and D 82 files,
  Part C 51, round 3's 16, Part E 24 and round 4a's 3, the doc examples; all green after the corrections listed
  above; slowest 42.4 s (`struct_array_init.c`), the limit pair 17.7 s; the S47 fixtures 46 (5.6–42.5 s).
- **Not run, at the owner's request** (2026-11-25, about 10:40 UTC: "I'm about to leave and disconnect, so skip
  forward to the advisor check, closing commit and status and handoff updates. I'll handle the checks and runs later
  and so will CI"): the preflight's `run_tests.py --check-headers` and `mutate.py --validate` on the final tree (both
  ran green earlier in the session), and **`CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` on the box**.
- `src/` and `include/` against `4e28487`, measured: +4,944 −439 without the self-test cases (+7,596 −445 with them); the plan
  estimated 2,340–4,475 for Parts A–D and 600–1,050 more for Part E.

**First step for the next session:** the owner runs `CERASTES_LONG=1 CERASTES_JOBS=32 scripts/check.sh` on the box
(handoff §2), reviews handoff §8's questions 301–309 (304 open) and the deviations above, fast-forwards `main` to
`m4.5e1e1-arrays-of-scalars` and pushes. Then **M4.5e1e2**'s short planning (`.claude/plans/m4-5e1e1-arrays-of-scalars.md`
section 14 and handoff §3 "From M4.5e1e1"), or M4.5e1f / M4.5e2.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| M4.5e1e1 Arrays of scalars in structures and static storage | 2026-11-25 | `m4.5e1e1-arrays-of-scalars` (local) | 0063 (0012, 0024, 0028, 0041, 0043, 0044, 0059, 0060, 0061, 0062 amended) | global and `static` arrays, array members, array typedefs and `sizeof`, C array literals, structure copies and by-value passing up to 7 array elements; a compound-literal sequencing hole open since M4.5c2a closed; questions 301–309, no cut; the full check left to the owner |
| M4.5e1d Pure functions and the exact sum | 2026-11-24/25 | `m4.5e1d-pure-functions`, `68384d1` (fast-forward) | 0062 (0006, 0010, 0011, 0012, 0013, 0024, 0025, 0037, 0041, 0042, 0044, 0061 amended) | a `pure` function that is called or has a contract as a Viper function with its definition check; self-recursion with `decreases`; applications in checks, contracts, calls and loop invariants; the exact sum verifies; testing moved to the x86-64 box; Silicon's prover cap; questions 273–286, no cut |
| M4.5e1c Quantifiers | 2026-11-23/24 | `m4.5e1c-quantifiers` | 0061 (0006, 0011, 0028, 0029, 0037, 0038, 0041, 0059, 0060 amended) | `cerastes_forall` with an inlined `pure` body in contracts, loop invariants and checks; `f__wf` variant M; the fill loop, max, the carrays port, the tutorial chapter; S45 (r) for e1f; Part D cut to M4.5e2 (question 260); questions 253–263 (256–263 built overnight, for review) |
| M4.5e1b Ranges in contracts | 2026-11-11/12 | `m4.5e1b-ranges-in-contracts` | 0060 (0011, 0028, 0029, 0037, 0038, 0039, 0040, 0042, 0059 amended) | `owns_range`/`reads_range` in contracts, element reads in contracts, calls and returns over ranges, loops over parameters' ranges, markers over ranges, `owning(n)` on a parameter, `restrict` with a range, array parameters (two new tags), the carrays port; questions 238–246 |
| M4.5e1a Arrays inside one function | 2026-11-11 | `m4.5e1a-arrays-in-functions` | 0059 (0011, 0012, 0024, 0028, 0032, 0038, 0039, 0042, 0043, 0054 amended) | local arrays as blocks, `a[i]`, `&a[i]`, `p[i]`, the decay, initializers with C's zero fill, `out-of-bounds`, the marked range rows in the driver, loops carrying arrays, sequencing, `sizeof(a)`, `uninit` on arrays; #175, #1162, #138 as errors; array designators rejected (234); the `offset_of == 0` backstops (235) |
| M4.5d2b2d `not_invalidating`, `--strict-owner`, marked fields | 2026-11-10 | `m4.5d2b2d-invalidation-profile` | 0058 (0029, 0042, 0052, 0054, 0055 amended) | `cerastes::not_invalidating` checked at the definition (taken implies returned; `p->q == old(p->q)` spliced, a bodiless borrow's tie implied); `--strict-owner` (P4417R0's p4 and p6); `ref_to_uninit` on fields (marked closures keep flags only where tied) and on pointers to structures with owning fields (closing nothing); N1; two older exit 3s fixed (0054's kept row, the return rows' order), a third to M4.8 |
| M4.5d2b2c Loop limits, long-running tests | 2026-11-10 | `m4.5d2b2c-loop-limits` | 0056, 0057 (0016 amended) | six of d2b2b's loop false alarms lifted (a site the loop cannot fill keeps its flags, the value set of a lone owner, a leaf's ghost through an alias, `main`'s global leaf under an entry snapshot, a site's cells as places, chains at any depth), the escape scan, two exit 3s fixed; five pins kept under the written-invariant rule; the long-running tier (`tests/long/`, `CERASTES_LONG=1`); question 196's two programs past 1800 s, to M4.8 |
| M4.5d2b2b Owning fields | 2026-11-09/10 | `m4.5d2b2b-owning-fields` | 0055 | `owning` on members: the closure of every atom over owning fields (acyclic, any depth), the value rule at stores, named leaves (struct locals, globals, `static` locals, by-value copies) as owners in memory, copies, loops; P4417R0's `[[owner]]` mapped; the session tools `regen_vpr.py` and `vprdiff.py`; seven loop limits pinned for d2b2c |
| M4.5d2b2a Owners in memory | 2026-11-09 | `m4.5d2b2a-owners-in-memory` | 0054 | block sites through loads and on globals; owning and marked globals, `static` locals and address-taken locals (the deep atom; static owners exempt at `main`'s end); written `owns_block` invariants; loop carriers by current state lifting d2b1's four false alarms; the static struct leaf cover; d2b2 split (owning fields and P4418R0 to d2b2b) |
| S8 CI toolchain profiles | 2026-11-06 | `s8b-ci-profiles`, PR #33 | 0053 (0001, 0016, 0031 amended) | CI asserts declared majors and records exact builds; toolchain profiles (`scripts/ci_toolchains.toml`), a matrix with a `profiles` job, cadences `every`/`nightly`/`dispatch`, exclusions; the guard as rules and a schema; GCC builds of cerastes and EDG; a builder evaluation-order bug found by GCC fixed |
| M4.5d2b1 Partnerless results, owning locals | 2026-11-05 | `m4.5d2b1-results-locals` | 0052 | `ensures` atoms with no partner (covered by implicit atoms or given an output amount; bodiless contracts fresh results only), owning automatic locals (leaks at every exit and reassignment, loops), marked locals and P4222R3's strict rules, block-site guards over memory, `cerastes_owns_vlibc_block`, two d2a holes closed; written `owns_block` invariants cut to d2b2 |
| S7 Code style | 2026-11-05 | `s7-code-style` | 0051 (0001 amended) | cerastes's tool code in the owner's Boost style: the pinned clang-format 23.1.3 (`format.sh`, a `check.sh` step, fail-closed guards), then brace initialization, `U` and one declarator per declaration; proved by `object_identity.py` (objects and IR at every commit, macOS and Linux; tokens) and by `.vpr`/`.map` identical over the suite and the VIR self-test; the docs lockfile fixed |
| M4.5d2v vlibc's allocator | 2026-11-04/05 | `m4.5d2v-allocator` | 0050 (0047 amended) | the M4.5d2a re-audit's B1–S3; vlibc's allocator (`vlibc_malloc` and six more, a C23 core over trusted glue, sanitized and threaded tests); blocks know their allocator (the token's value); the pairing check; contract-less library routines; vlibc allocations rejected in C++; two older exit-3 holes fixed; vlibc and vlibm in the owner's Boost style |
| S6 Tutorial and developer guide | 2026-11-04 | `s6-tutorial` | 0049 | a VerCors-style tutorial (seven chapters, 20 verified programs and 24 failing variants, both modes) and a developer guide (eight pages) as sections of the one site; the overview rewritten; the website's landing page; a chapter per concept from now on (questions 131–133) |
| M4.5d2a The heap | 2026-11-03/04 | `m4.5d2a-heap` | 0047 | vlibc's `malloc`, `calloc`, `free` (trusted, reserved identifiers, shapes not names); typed views at `void*` → `T*` (one object, `sizeof(T)`); heap blocks with the free token; `invalid-free`; leaks by `forperm` (after `l_end`, at back edges; `main` included); `owns_block`; `owning`/`ref_to_uninit` on functions; sequencing over block sites; loops over blocks held through locals; d2 split (d2v allocator, d2b) |
| M4.5d1 vlibc in cerastes | 2026-11-03 | `m4.5d1-vlibc` | 0046 | `third_party/vlibc` pinned and guarded; vlibc's shim first in verify mode; library files by path; trust from vlibc's tags, "does not return" from vlibc, reserved names, attributes by position; three routes redirecting trusted calls closed (older than vlibc); `trusted_libc_reason` gone; d2's dependency map |
| M4.5c2b Places through a heap read | 2026-11-03 | `m4.5c2b-heap-read-places`, PR #26 | 0045 | contract and invariant places through loads (`**pp`, `*gp`, `p->q->x`, `*s.q`, `*r.q`) with the entry-state rule, the post-state reading, amounts that follow objects, bodiless ties, heap-reading guards, `eb`/`lb` loop places, written atoms through loads and on struct places, `restrict` over load roots; `live(r.q)` to M4.5d2 |
| M4.5c2a Struct values and struct storage | 2026-11-02/03 | `m4.5c2a-struct-values`, PR #25 | 0044 | by-value struct parameters and results as tuples of leaves, `f().x`, C++ temporaries, compound literals (C objects, C++ values), struct globals and `static` struct locals, member measures, `restrict` beside by-value structs; invariant reads through a loaded pointer pinned; M4.5c2 split, heap-read places and written struct atoms to M4.5c2b |
| M4.5c1 Struct objects | 2026-11-02 | `m4.5c1-struct-objects`, PR #24 `8f45843` | 0043 | C structures with member blocks, struct locals as blocks, `.`/`->`, pointers to members, initializers, copies, `struct S *` objects with `WF_S`, contract atoms on struct places with overlap per leaf cell, markers per leaf; `restrict` closed beside loaded pointers (a hole since 0037); M4.5c split, owning fields and P4418R0 to M4.5d2 |
| M4.5b2b Initialization through pointers | 2026-11-02 | `m4.5b2b-init-markers` | 0042 | P4222's markers (`uninit` on locals, `ref_to_uninit`/`must_init` on pointer parameters), every call flag row `uninitialized`, `--strict-init` with the `// verify-options:` header, the loop row on every loop place (question 89, fixing 0038's nested-loop exit 3); M4.5b2 complete |
| M4.5b2a Globals and `static` locals | 2026-10-30 | `m4.5b2a-globals`, PR #22 `38877b4` | 0041 | mutable globals and `static` locals as `unique` blocks, `program_start` for `main` with initial values by linkage, frames through `owns(g)`/`reads(g)`, implicit atoms for `static` locals; M4.5b2 split, the markers to M4.5b2b |
| M4.5b1c3 Pointers to pointers, owning, `*result`, written atoms | 2026-10-30 | `m4.5b1c3-pointers-to-pointers` | 0040 | `T**` at any depth and pointer cells, `[[cerastes::owning]]` on parameters, `*result` as a value, written `owns`/`reads`/`live` in loop invariants (question 68's false alarms gone); conversions and contract comparisons across `const` rejected (a hole since 0037 closed); M4.5b1 complete |
| M4.5b1c2 Sequencing and placement | 2026-10-29 | `m4.5b1c2-sequencing` | 0039 | writes to cells inside expressions, sequencing on objects (`unsequenced`; same-object shapes rejected like the compilers), calls beside accesses, `return &local;` rejected (R14), `c ? owns : reads` path-precise and partners by path, function measures over memory |
| M4.5b1c1 Loops and jumps | 2026-10-29 | `m4.5b1c1-loops` | 0038 | loops over memory with automatic permission invariants ("every place held"), invariants and loop measures over memory, scope exits at jumps; written atoms in invariants cut to b1c3, function measures over memory to b1c2; the `address_of_local.c` port |
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
