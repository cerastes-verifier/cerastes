# Plan: M4.5d2b2d — `not_invalidating`, `--strict-owner`, marked fields (the next free record)

**Written at M4.5d2b2c's planning (2026-11-10)**, from the same surveys, when the owner split the session in two
(question 198). It is approved with that plan. Its session starts with Step 0 below, which re-checks it against
what M4.5d2b2c landed, and needs no planning session of its own unless Step 0 finds a contradiction.

Branch `m4.5d2b2d-invalidation-profile`, worktree `.claude/worktrees/m4.5d2b2d`.

## Context

**Scope:** the rest of P4418R0's invalidation profile for C (question 91), after M4.5d2b2c and before M4.5e1.
- **Part N:** `cerastes::not_invalidating` on a pointer parameter, a checked attribute (question 178); N1 with it.
- **Part S:** `--strict-owner`, P4417R0's p4 and p6 as an opt-in (question 177).
- **Part F:** `ref_to_uninit` on a field (question 192), and on a pointer to a structure with an owning field
  (question 197).
- **Pins:** `not_invalidating` on a function; `no_dangling` (no `no_dangling` in C, question 179).
- C++'s parts (references, member functions, `std::no_dangling`) are M7's.

**Size** (insertions in `src/`, without self-tests): Part N 290–440, Part S 200–280, Part F about 250–450; in all
about 740–1,170. Beside it: VIR self-test cases, `lint_viper.py`, the harness allow-list.

**Survey refresh** (tree at `262edb3`; the Inputs' older numbers are replaced by these; `encode.cpp` is
`src/backend/encode.cpp`):
- **Registration:** `register_attributes.cpp` table 42–91; `owning` 74 (`"r|v|p|d"`), `ref_to_uninit` 86
  (`"r|v|p"`, `RAF_ENTITY_ATTACHED`), `must_init` 87 (`"p"`, `RAF_NO_FLAGS`); the prose at 61–84.
- **Names:** `annotation_il.h` enum 31–47 and `kNames` 64–70; `annotation_checks.cpp` early return 1147–1150 and
  protected names 1649–1652; `include/cerastes.h` 58–61 and 74–77 (the owner added `CERASTES_PRE`/`CERASTES_POST`
  at 48 and 50).
- **Markers in `encode.cpp`:** `check_signature` 1179–1282; `marker_name` 1305–1312 (**falls through to
  "owning"**); `read_marker` 1324–1697 (`const` branch 1548–1568; **1685–1694 files any kind that is not `owning`
  or `must_init` as `RefToUninit`**); `check_marker_pairs` 1700–1738; `check_marker_atoms` 1744–1854;
  `reject_marked_read` 1859–1891; `is_param_marker` 5552 with seven callers (1075, 1137, 5567, 5630, 10567, 10854,
  11933); `routine_has_contract` 11978 (12004).
- **Contract passes:** `encode_contract` 3299–3368 runs `apply_owning` 3363, `apply_owning_globals` 3364,
  `apply_owning_closure` 3365, `finish_atoms` 3366 (7354–7720); `check_marker_atoms` is called at 3179. The
  closure: `close_leaf` 2540–2555, `close_struct` 2557–2572, `closure_marked` 2696–2713,
  `reject_marked_owning_pointees` 2716–2762, `apply_owning_closure` 2969–3020.
- **Atoms:** `AtomSite` `vir.h` 930–944; `atom_place` `vir.cpp` 1268–1319; `atom_guard_shape` 1401–1430;
  `restrict_backed` 1593–1698; `vir::Param` 668–682.
- **`f__wf`:** `wf_method` `lower.cpp` 17146–17533; "returned implies taken" 17176–17228; the bodiless tie
  17230–17315 (guard row 17249–17263, tie 17264–17279); the atom tie in `atom()` 5499–5514; `block_consumes`
  4304–4318; `block_marker` 4268–4285, `BlockMode` 4374–4379, `block_mode` 4422–4438.
- **`--strict-init`:** `verify_main.cpp` 71–73, 87, 220–223, 630–633; `verify_config.h` 32–34; `diagnostics.def`
  110–119; in `encode.cpp` the `Ctx` fields 292–309, `call()` 7965–8127 (**heap routines return early at
  8024–8027**; strict sites 8084–8088, 8109–8118), `strict_init_arguments` 8129–8193, `settle_strict_init`
  8195–8218, `allow_strict_result` 8548–8567, `check_marked_source` 8573–8592, `check_marked_global_source`
  8602–8613, `check_marked_return` 8614–8638, `strict_value_uninit_ok` 8639–8675, `allocation_view` 11518–11638
  (strict 11622–11628), `reject_strict_uninit_result` 11640–11653, `heap_call` 11655–11739, `declaration`
  12294–12350, `initializer` 12386–12421, `assignment_parts` 13352– (13365–13382), `AssignParts` 13199–13208,
  `return_statement` 14210–14273.
- **Result and consumption:** the result site test is the `result_sites` lambda (1979–1991); there is no encoder
  helper for "this parameter's contract consumes its block".
- **Fields:** `struct_info` 18235–18596 (the `owning` reader 18343, rejections 18349–18457, cycle 18542–18561, the
  flag 18563–18566); the union rejection 18661–18684; `StructMember` `vir.h` 171–179; `leaf_is_owning` 5010.
- **Marked variables:** `read_variable_markers` 8404–8535 (the owning-field rejection 8489–8505).
- **Harness:** `VERIFY_OPTIONS_ALLOWED` `run_tests.py:309`; `test_run_tests.py` 13359 and the tag-coverage test
  15757–15768.
- **Docs:** `verifying.adoc` options 41–73, `[#strict-init]` 400–528; `annotations.adoc` 308–335, 420–447, 1058,
  1147–1148; `memory.adoc` `[#p4417-owners]` 1261–1290 and `[#not-yet]` 1357–1364, 1384–1390;
  `dev/architecture.adoc:123`; `tests/README.md` 99–101, 338–362.

## Owner decisions

- **Standing:** 175 (`owning` on a parameter stays a borrow); 177 (`--strict-owner`); 178
  (`cerastes::not_invalidating`, checked); 179 (no `no_dangling` in C); 192 (`ref_to_uninit` on a field here); 197
  (the collision pins stay as they are, and this session revisits `ref_to_uninit` beside an owning field); 198.
- **At M4.5d2b2c's planning (2026-11-10), each the recommendation:**
  - **202:** `ref_to_uninit` on a pointer (parameter, result, variable) to a structure with an owning field is
    accepted, the marked site closing nothing (record item 4). If the audit finds a hole, the rejections stay.
  - **203:** `not_invalidating` on a pointer to a `const` object is accepted.

## Step 0 and Step 1

1. M4.5d2b2c's box check is green and its branch is merged; branch from the base that holds it.
2. Re-read "Survey refresh" against the tree: `lower.cpp` moved with M4.5d2b2c, `encode.cpp` should not have. A
   contradiction with an item below stops the session.
3. Worktree, submodules with `--reference`, `scripts/bootstrap.sh`; the CI check of M4.5d2b2c's Step 1; the
   baseline (`regen_vpr.py`, the VIR self-test) into `build/m4.5d2b2d/baseline/`.

## The record (written before code; the next free number)

Amends 0029 items 1 and 2, 0042 items 4 and 8, 0052 item 4, 0054 item 2, 0055 items 3 and 11.

1. **Scope:** items 2–5. Fail closed on the rest (item 8).

2. **`cerastes::not_invalidating`** (question 178; P4417R0 9.13.u and [profiles.inval.verify]).
   - **Registration:** `{"not_invalidating", "", "p", RAF_NO_FLAGS}`, as `must_init`; macro
     `CERASTES_NOT_INVALIDATING`. It lands with its reader: `read_marker` gets its own branch before the
     registration, since today any unknown kind is filed as `ref_to_uninit`.
   - **Accepted** on a pointer parameter, in either position, a pointer to a `const` object included (question
     203: `read_marker`'s `const` branch does not apply to it); declarations must agree (`annotation_mismatch`).
   - **Rejected** (`unsupported_annotation`, one pin each): a non-pointer; `void *`; a pointer to a function
     (M4.7); twice; beside `ref_to_uninit` or `must_init` on the same parameter (`reject_marked_read` refuses the
     `old(*p)` it needs). On a function it is EDG's #1835 (pinned; a member function is M7's).
   - **The check on the contract,** with or without a body, after `finish_atoms`, for each marked `p`:
     - every `requires` atom rooted at `p` (any `via` and `path`) has an `ensures` atom with its amount;
     - every `requires` block site rooted at `p` has an `ensures` site on the same place;
     - **taken implies returned on every path.** Where both guards are absent or the same null check
       (`atom_guard_shape`) the rule is syntactic. Any other guard gets a new `f__wf` row
       `prior ==> (g_req ==> g_ens)` (`permission`).
   - **The implicit `ensures`,** built after that check so that it is framed in both states: for each atom or
     block site rooted at `p` whose place goes through a load (closures of owning fields included), each loaded
     pointer `E` on its path satisfies `E == old(E)`, under the site's guards. With a body it is a `postcondition`
     at the return. Without one it is 0054 item 2's tie, so a bodiless borrow of a structure with an owning field
     no longer writes `p->q == old(p->q)`.
   - **What the caller gains:** a pointer taken before the call into what `p` reaches (`int *t = p->q;`) is still
     valid after it. The marker promises nothing about values.
   - **N1** (d2b2b's audit): in `wf_method` the tie assert moves above the partner-guard assert, so an untied
     bodiless borrow gets one message (Flips).

3. **`--strict-owner`** (question 177; P4417R0 [profiles.inval.owner] p4 and p6). The pattern is
   `--strict-init`'s, end to end.
   - **p4.** A call whose contract gives its result's block (an `owning` function, `malloc`, `calloc`, vlibc's
     allocations) must be: the initializer of, or the value assigned to, an owner (an `owning` local, global,
     `static` local or field); the operand of a `return` in a function that gives its result's block; or the
     argument of a parameter whose contract takes the block (`free(make())`).
   - **p6.** The argument of a parameter whose contract takes a block and does not return it (`free`,
     `vlibc_free`, a user's destructor) must be: an owner by name (`v`, `g`, `s.q`, `p->q`); a parameter of the
     enclosing function whose own contract takes its block; or a p4 call.
   - **Two encoder-side facts are new:** "this function gives its result's block" (the `result_sites` test as a
     function) and "this parameter's contract consumes its block" (the encoder's twin of `block_consumes`).
     Heap routines leave `call()` early, so the rules are also applied in `heap_call` and `allocation_view`.
   - **Diagnostics:** two new tags in `diagnostics.def`, one per rule, each named by a test.
   - **What the static rule cannot see** is listed, as 0042 item 8 does: accepted although wrong (an owner by
     name that is already empty, which the default mode's `invalid-free` still reports) and rejected although
     harmless (`t = v; free(t);`, `free(c ? a : b)`, `free(*pp)` with `pp == &v`).
   - Without the option every program means the same; the option only rejects more. Both strict options may be
     given together.

4. **Marked fields** (questions 192, 197, 202).
   - **Registration:** `ref_to_uninit` becomes `"r|v|p|d"`, with its reader in `struct_info` and
     `vir::StructMember::ref_to_uninit`. `must_init` on a field stays EDG's #1835 (pinned).
   - **Accepted** on a pointer field; rejected like `owning` on a field (a non-pointer, `void *`, a function
     pointer, a pointer to `const`, twice, a bit-field, a union member).
   - **Default mode, non-owning field:** the marker documents intent and changes no Viper (0052 item 4).
   - **Owning and marked field:** its closure clause is a marked block (0047 item 8's mode, as for an owning and
     marked global in 0054 item 2): a `requires` site claims no flags, a borrowed `ensures` site keeps them
     (`old(flag) ==> flag`), a given site claims none. This lifts 0055 item 3's "the closure is plain": a
     structure whose buffer is allocated and not yet written can be passed to a borrower.
   - **`--strict-init`:** a store to a marked field follows 0052 item 4's rule for a marked local, and a
     `ref_to_uninit` call's result may be stored to it. The same store to an unmarked field stays rejected.
   - **A marked pointer to a structure with an owning field** (question 202): accepted, and the marked site
     closes nothing on either side, as 0055 item 3 already says of `malloc`'s view. A callee that leaves a block
     in the field is reported by its own end row; an initializer is `must_init`. The three rejections
     (`reject_marked_owning_pointees` for a parameter and a result, `read_variable_markers` for a variable) go,
     after S42 and the audit. If the audit finds a hole, they stay and their pins are re-filed.

5. **Pins:** `not_invalidating` on a function (#1835); `no_dangling` (an unknown name in the closed namespace:
   `unrecognized_attribute`, pre-registered from one probe); `must_init` on a field.

6. **The lint:** the new `f__wf` row; marked closure sites. **Categories:** none new; two new diagnostic tags.

7. **The `[[owner]]` table** in `memory.adoc` gains `[[not_invalidating]]` and the p4 and p6 rows.

8. **Still rejected:** `not_invalidating` on a function or member function (M7); `no_dangling`; `must_init` on a
   field; `ref_to_uninit` on a field that is not a pointer to an object.

9. **Facts:** P77 and S42.

## P77 and spike S42 (il-analyst, before code)

- **P77 (IL):** `not_invalidating` on a parameter in both positions and both modes, on several declarations; on a
  function (the refusal); `ref_to_uninit` on a field alone and beside `owning`, in both positions, in a C++
  aggregate; `no_dangling`'s diagnostic.
- **S42 (hand-written Viper):** the `g_req ==> g_ens` row with a twin; the implicit `ensures` on a borrower with a
  body (the twin frees and replaces `p->q`: `postcondition`) and without one (the caller keeps `t = p->q`); the
  marked closure in its three modes, with a twin that reads the unwritten buffer; question 202's shape: a caller
  whose struct local holds a block passes `&s` to a `ref_to_uninit` callee that overwrites the field (the caller's
  end row must report the block), and a callee that leaves a block in the field (its own end row must report it).

## Parts and commits

1. The record (draft, revised after P77, S42 and audit round 1); P77 and S42 in `il-inventory.md`.
2. **Part N**, then **Part S**, then **Part F**, each with its tests, its flips and its docs examples; each
   audited on a snapshot commit before its commit.
3. The audits' fixes; then docs, status, handoff, `plan.md`'s Done row, the record accepted.

**Pre-agreed cuts, in this order:** the marked pointer to a structure with an owning field (the three rejections
stay); then Part S (to M4.8, with its design). **Split point:** at Part N's commit, if the context will not carry
Parts S and F, stop and ask.

## Team

| Agent | Owns |
|---|---|
| **edg-backend** (Sonnet) | `register_attributes.cpp`, `annotation_il.h`, `annotation_checks.cpp`, `include/cerastes.h`, `encode.cpp`, `src/driver`, `verify_config.h`, `diagnostics.def`; the builds. |
| **il-analyst** (Sonnet) | P77, S42. Then `src/vir` (`Param::not_invalidating`, `StructMember::ref_to_uninit`, the `f__wf` row, N1, `block_marker`/`block_mode` for marked leaves, `check.cpp`'s backstops) and the VIR self-test cases. |
| **test-mutator** (Sonnet) | Tests and twins; exactly the Flips; the harness allow-list with its self-tests; `lint_viper.py` with its self-test; the doc example files. |
| **soundness-auditor** (Opus, read-only) | Round 1: the record draft with P77 and S42. Round 2: each part. One re-audit. |
| **Orchestrator** (Opus), **Advisor** (Fable, optional) | As in M4.5d2b2c's plan, with `build/m4.5d2b2d/`. |

The rules for every agent are M4.5d2b2c's.

## Tests (each positive has a twin failing for one reason at one line)

- **Part N:** a marked borrower with a body, whose caller uses `t = p->q` after the call. Twins: the same without
  the marker (the caller's use fails); a body that frees and replaces `p->q` (`postcondition`); a contract that
  consumes (`requires(owns_block(p))` alone); guards that differ (`permission`). A bodiless marked borrow with no
  written tie. `int **pp` and plain `int *p` forms. Every rejection of item 2.
- **Part S** (`// verify-options: --strict-owner`): every allowed position of p4 and p6, and each one's rejected
  neighbour (a plain local, a non-owning field, a borrowing parameter, an alias, a non-owning function's
  `return`). The "cannot see" lists, one test per entry. Both strict options on one line.
- **Part F:** a structure with an owning and marked buffer, allocated, passed unwritten to a filler. Twins: the
  filler reads first (`uninitialized`); the same without the marker (`uninitialized` at the call). Under
  `--strict-init`: `p->q = malloc(…)` into a marked field; `p->q = &x` rejected. A non-owning marked field. Every
  rejection of item 4. Question 202's shapes with their twins.
- **Byte identity:** `scripts/vprdiff.py` against the baseline; no existing program's Viper changes outside the
  flips.

## Flips (pre-approved; any other changed outcome stops the session)

- **Retired for a positive of the same shape, with a twin:** `tests/memory/negative/reject_ref_to_uninit_field.c`
  (`rejected wrong_entity_for_attribute @ L9`).
- **Retired likewise (question 202):** `reject_owning_field_ref_to_uninit_param.c`,
  `reject_owning_field_ref_to_uninit_param_second.c`, `reject_owning_field_marked_global.c`. If the audit keeps
  the rejections, the pins stay and only their messages name the new record.
- **Stays, re-messaged:** `reject_strict_init_malloc_member.c` (its field is unmarked; it becomes the twin of the
  new marked-field positive).
- **N1:** `negative/owning_field_bodiless_untied.c` (two `error permission @ L17`) and
  `negative/owning_field_bodiless_chain_untied.c` (two `@ L22`) each keep one; the second `expect:` line becomes a
  comment so that the line numbers stay.
- **Stay as written:** the other five `reject_strict_init_malloc_*` pins and
  `tests/vlibc/negative/reject_strict_init_vlibc_malloc_local.c`; `reject_owning_member_function.cpp` (M7).
- **Harness:** `test_the_allow_list_holds_only_strict_init` follows the allow-list.

## Docs (invariant 8; every example is a test)

- **`annotations.adoc`:** `not_invalidating` beside the initialization markers (308–335); "Owning members"
  (420–447, the sentence at 439); "Declarations that disagree" (1058); the protected names (1147–1148).
- **`verifying.adoc`:** the option table (41–73) and a section `[#strict-owner]` after the strict-init sections,
  with both "cannot see" lists.
- **`memory.adoc`:** the `[#p4417-owners]` table (1261–1290); `[#not-yet]` at 1357–1364 and 1384–1390.
- **`error-messages.adoc`:** the new messages and tags. The tutorial's heap chapter gains a paragraph on
  `not_invalidating` beside "A structure that owns its buffer".
- **`dev/architecture.adoc:123`**, **`tests/README.md`** (99–101, 338–362), the driver's usage text,
  `include/cerastes.h`'s comment (17–22), `internal/encoding.md`, `internal/il-inventory.md`.

## Stop and ask

Any agent question; any changed outcome outside Flips; any existing program's Viper changing; a spike that
contradicts items 2–4; a cut beyond the pre-agreed ones; the split point; an exit 3 or a timeout over 120 s not
fixed within its part.

## Acceptance

As M4.5d2b2c's, with `build/m4.5d2b2d/`: the lint and regeneration before the box run; `CERASTES_LONG=1
scripts/check.sh` once on the box, green; vprdiff; the VIR self-test, `test_lint_viper.py`, `test_run_tests.py`
(the tag-coverage test names both new tags); format; docs; status and handoff. Commits stay local.

## Inputs (from handoff §3 at M4.5d2b2c's planning)

Verbatim from `git show 262edb3:internal/handoff.md` §3, "M4.5d2b2c". Its line numbers are stale; "Survey
refresh" above replaces them.

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
- **Pins.** No P4418R0 attribute has a pin of its own, though 0054 item 10 promised one each. `not_invalidating` on
  a function, and `no_dangling` (an unknown name in the closed namespace; question 179), get theirs here.
  [d2b2b planning survey]
