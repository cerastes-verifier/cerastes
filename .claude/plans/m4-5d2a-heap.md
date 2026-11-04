# Plan: M4.5d2a — the heap (record 0047)

Approved by the owner on 2026-11-03 (planning in plan mode; questions 120–123 answered with the recommendations).
- **Branch and worktree:** branch `m4.5d2a-heap` from M4.5d1's tip `9d98fe8`, so the PR stacks on PR #27 as d1 stacked on #26. Worktree `.claude/worktrees/m4.5d2a`.
- **Baseline:** the existing `.claude/worktrees/m4.5d1`, which is clean at `9d98fe8`. It is used read-only; no new bootstrap.
- **First step:** save this plan as `.claude/plans/m4-5d2a-heap.md`.

## Context

**Why this session.** plan.md and handoff §4.2 put **M4.5d2 `malloc` and `free`** next. It has 14 work items and no heap code exists yet: no free token, no `owns_block`, no `forperm`, no `leak` or `invalid-free`. That is more than one context holds, so the owner split it (question 120):
- **d2a (this session): the heap.**
- **d2v next: vlibc's allocator.**
- **d2b after that:** owning locals, globals and fields, `ensures` atoms with no `requires` partner, P4418R0, and P4222R3's strict rules.

**The owner's directive.** vlibc holds complete implementations of the C library functions it covers, usable without cerastes and verified as cerastes becomes able to. This changes 0030 item 2, which made the malloc family trusted-only. Questions 121–122 settled it:
- **121, the model:** vlibc replaces the platform's allocator:
  - the standard names over a portable C23 core;
  - pages from `mmap`;
  - glibc's documented replacement set;
  - on macOS, a malloc zone made the default zone, so libSystem's own allocations use vlibc too;
  - thread-safe.
- **122, the timing:** a session of its own right after d2a (M4.5d2v).
- **Until cerastes verifies the core:** the contracts stay trusted. The page and zone glue stay trusted for good.
- **Why so much machinery:** C23 7.1.3 makes a library outside the implementation that defines `malloc` undefined behaviour. glibc documents how to replace it; on macOS a static `malloc` rebinds only the program's own calls, so freeing what libSystem allocated would corrupt the heap.

**What d2a does:**
- vlibc's contracts for `malloc`, `calloc` and `free`;
- `void*` only where record 0028 item 1 allows it;
- heap blocks with the slot −1 free token;
- one object per allocation, read through the type it is converted to (a "typed view");
- the free checks (`invalid-free`) and leaks by `forperm` (`leak`);
- `owns_block` in contracts;
- `[[cerastes::owning]]` and `[[cerastes::ref_to_uninit]]` on functions (target `r`);
- two calls in one expression ordered over the liveness and token cells;
- last and cuttable: blocks held through locals across loops.

**What the planning surveys found** (encoder map, tests and vlibc, the papers, and the d2a design; their details are carried into the record items below):
- **One place for leaks.** `postconditions()` (`src/vir/lower.cpp:5074`) is the single choke point for every normal exit, so the leak check goes there.
- **New Viper and VIR support needed.**
  - There is no `forperm` expression kind (`lowered.h:35`) and no `tok` field.
  - The driver accepts only `assertion.false` (`src/driver/report.cpp`), and a failing `forperm` assert is exactly that.
  - Marked range rows come only in M4.5e1, so d2a must avoid quantified permission checks. It does: one object per allocation, one row per cell.
- **`void*` is rejected in three layers.**
  - `vtype` at `encode.cpp:584-614`;
  - `pointer_conversion` at `:3781-3851`, struct-pointer conversions at `:3842`;
  - `check.cpp`'s `accepted_pointer` at `:185`.
  - `cell_suffix(void)` dereferences null (`lower.cpp:1073`).
- **Today's marker and trust code would break the moment vlibc declares the malloc family:**
  - **C++:** EDG moves `decl_position` to vlibc (P71 (d)), so `check_signature` rejects `void*` at vlibc's line in every unit that includes `<stdlib.h>`.
  - **C:** `reject_markers_on_system_redeclaration` (`encode.cpp:2716`) rejects vlibc's own marker.
  - **Both:** `ours` (`:708`) is keyed on `decl_position`, `lib_trust` (`:5144`) sees only `contracts_of`, and `read_marker`'s all-or-none rule (`:955`) and `void*` refusal (`:911`) all get in the way.
  - So the vlibc pin move and the routine diversion must land in one commit.
- **Registration.** EDG itself refuses `owning`/`ref_to_uninit` on functions today (registered `"p"`, `register_attributes.cpp:62-70`). They become `"r|p"` with `RAF_ENTITY_ATTACHED`. The locals and fields targets (`v`, `d`) stay refused until d2b.
- **Categories:** five copies (0011's table, `run_tests.py:332`, `test_run_tests.py:10647`, `tests/README.md:400`, `error-messages.adoc` with examples checked by `check_doc_examples.py:1273`).
- **Lint:** `lint_viper.py` pins `alloc_<t>`'s text exactly (`:593-600`, `:775-792`). Its unmapped `forperm` passes only if it is mapped.
- **vlibc's CI** pins cerastes `234ede3`, whose `<cerastes.h>` lacks `CERASTES_REF_TO_UNINIT`. `origin/main` `f59e4b2` has it.
- **P4222R3 and P4416R0** (checked at planning, the handoff's d2 item):
  - A local receiving `malloc`'s result must be marked `ref_to_uninit`.
  - A null argument to a `ref_to_uninit` parameter: R3 rejects it, P4416 accepts it.
  - A marked result "must refer to uninit".
  - `free` is unmarked, so it needs an exemption.
  - Analysis is over syntactic paths. cerastes checks feasible paths, so it accepts more.
  - P4417's `[[owner]]` *parameter* is a transfer, where 0029's `owning` parameter is a borrow.
  - All of this is d2b's to decide. d2a only fails closed under `--strict-init` (record item 8).
- **Size.** About 2,800 changed source lines without the loop rule, 3,250 with it; about 85 tests and about 30 Silicon fixtures. That is the top of past sessions, so loops are the first cut.

## Owner decisions

From this planning (2026-11-03), each the recommendation:
- **120, scope: d2a.** `realloc` moves to M4.5e2: it needs `n * sizeof(T)` and block contents. vlibc declares no contract for it in d2a, so a call is `call_without_contract`. Loops are the first cut.
- **121, the allocator model:** replacement, as above. Recorded in 0047; built in d2v.
- **122, timing:** M4.5d2v, vlibc's allocator, right after d2a in vlibc's repository. It brings run tests and CI on both platforms with both compilers. Verifying the core needs arrays, pointer arithmetic, predicates and char storage, so it comes after M4.6 (a plan.md note).
- **123, `main`'s blocks: a leak, like VeriFast.**
  - Every normal exit of every function is checked, `main`'s return included, with no waiver.
  - A path ending in `abort`, `exit`, `_Exit` or a user `noreturn` call is not checked, since it never returns.
  - That differs from C's rule that returning from `main` calls `exit`; the difference is deliberate and documented.
  - Blocks reachable from globals at `main`'s end are leaks until d2b's owning globals.

**Standing:**
- 69: the EDG merge waits for the box.
- 77: one context per session.
- 0030 item 5 and 0046 item 9: every pin move is audited, and each trusted contract has a must-fail twin that violates its `requires`.

**Flips, approved with this plan; any other change stops the session:**
- `tests/memory/negative/reject_owning_function.c`: becomes `error permission @ L8`. The function returns a borrowed `p` as an owning result.
- `tests/memory/negative/reject_ref_to_uninit_function.c`: becomes `rejected cerastes-unsupported_annotation @ L7`. The marker needs `owns_block(r)` in an `ensures`.
- Message and `Pins:` changes only (each names d2b; the `expect:` lines stay):
  - `reject_atom_on_result.c`, `reject_ensures_atom_no_partner.c`, `reject_struct_value_live_result.c`, `reject_owns_result_member_target.c`, `reject_owns_result_double_deref.c`;
  - `reject_owning_local.c`, `reject_ref_to_uninit_local.c`, `reject_must_init_local.c`, `reject_struct_owning_field.c`;
  - `static_local_return_address_use.c`;
  - `reject_void_pointer.c`, `reject_void_to_int_pointer.c`, `global_void_pointer.c`, `reject_struct_void_pointer_member.c`. These name M4.7 and M8.

## Step 1 (first, markdown only)

1. **Worktree:** `git worktree add .claude/worktrees/m4.5d2a -b m4.5d2a-heap 9d98fe8`, with submodules by `--reference`, then `scripts/bootstrap.sh`.
2. **Baseline:** in `.claude/worktrees/m4.5d1`, `run_tests.py -j 8` and the VIR self-test, copied into the session worktree as `build/m4.5d2a-baseline-{vpr,vir}`. No session agent builds until this is done.
3. **Save this plan** as `.claude/plans/m4-5d2a-heap.md`.
4. **`internal/handoff.md`:**
   - the header points at `git show 9d98fe8:internal/handoff.md`;
   - §1–2: state, and the owner steps (PR #27 first);
   - §3: d2's notes move into this plan; the P4222R3/P4416/P4417 findings go under "M4.5d2b";
   - §4: 0047 is d2a's record, 0048 is next;
   - §4.2: d2a, d2v, d2b;
   - §8: questions 120–123.
5. **`plan.md`:** the header says the last session was M4.5d2a. Rows in the M4.5 table:
   - d2a, naming this plan;
   - **M4.5d2v vlibc's allocator** (the replacement model above; needs d2a);
   - d2b, the rest of the old d2 row;
   - plus a note on when vlibc's implementations get verified.
6. **Commit 1** (planning), locally, with no Claude trailer.

## Inputs (handoff §3 "M4.5d2", how each is handled)

- vlibc's markers in C: `ours` and `lib_trust` → record item 3.
- The macro scan and parameter names → reserved names (item 1).
- Twins of trusted contracts must violate the `requires` → double free and the free-kind twins.
- The marker rejection on a system redeclaration (`encode.cpp:2716`) → library declarations are skipped (item 3).
- Register `owning`/`ref_to_uninit` target `r` here; `v` and `d` wait for d2b.
- P4222R3 → checked above; consequences for d2b.
- `malloc(sizeof(struct S))`'s member lengths and slot facts → the views reuse `struct_lens` (item 2).
- `alloc_<t>`'s missing `block_of(r) != GB_g()` → **not in d2a.**
  - Adding it would change the lint-pinned text and the Viper of every program with both cells and globals.
  - No d2a test needs it, and the views do not state it either (one rule).
  - The note stays for M4.8.
- `return &s;` of a `static` local → d2b (a partnerless `live`).
- 0042's `internal` row re-check → S36 (j).

## Record 0047 — the heap

Written before code (invariant 6). Audit round 1 runs on the draft together with S36's fixtures.

**What it amends:**
- 0028 items 1, 6 and 13, and item 18's fixtures;
- 0029 items 1–3 (targets `r`, `owns_block`);
- 0030 item 2 (the owner's directive);
- 0038 item 3 (if loops land);
- 0039 item 4 (liveness and token cells);
- 0042 item 8;
- 0046 items 4 and 9.

1. **vlibc's contracts.** vlibc's `<stdlib.h>` gains these:
   ```c
   // cerastes:trusted-contract vlibc: malloc returns NULL or the start of a new allocation of size bytes, its contents indeterminate (C23 7.24.3.6)
   void *malloc(size_t __size) CERASTES_REF_TO_UNINIT CERASTES_ENSURES(__r, __r == NULL || cerastes_owns_block(__r, __size));
   // cerastes:trusted-contract vlibc: calloc returns NULL or the start of a new allocation of nmemb * size bytes, all zero (C23 7.24.3.2)
   void *calloc(size_t __nmemb, size_t __size) CERASTES_ENSURES(__r, __r == NULL || (cerastes_owns_block(__r, __nmemb * __size) && cerastes_zeroed(__r)));
   // cerastes:trusted-contract vlibc: free ends the allocation ptr is the start of; free(NULL) does nothing (C23 7.24.3.3)
   void free(void *__ptr CERASTES_REF_TO_UNINIT) CERASTES_REQUIRES(__ptr == NULL || cerastes_owns_block(__ptr));
   ```
   - **Reserved identifiers only.** Every identifier in a shim contract is reserved (`__size`, `__nmemb`, `__ptr`, `__r`, `cerastes_*`) or one of `NULL`, `false`, `true`. A new `vlibc_contracts.py` rule enforces it. The macro scan of 0046 item 4 stays as it is: a program defining `__size` is undefined behaviour under C23 7.1.3 anyway.
   - **The reserved table** (generated) gains `malloc`, `calloc` and `free`. `tests/vlibc/trusted_contracts.txt` gains three lines: a golden change, audited.
   - **vlibc's own files:** its CI moves `CERASTES_COMMIT` to `f59e4b2`, and its `CLAUDE.md` records 121–122.
   - **The marker on `free`'s parameter.** Without it, P4222's default would make `free` of a never-written block `uninitialized`.
2. **Shapes, and typed views by conversion.** No table keyed by name.
   - **The two shapes.** A routine trusted from vlibc whose signature mentions `void*` must have one of two contracts; anything else is an internal error (exit 3). Each is checked once per routine, called or not, in the declared-routine loop, which then skips it before `check_signature` (`:9344-9420`).
     - **(A) Allocation:** a `void*` result, integer parameters, and `ensures(B == NULL || owns_block(B, S) [&& zeroed(B)])`.
     - **(D) Deallocation:** one `void*` parameter with `ref_to_uninit`, and `requires(P == NULL || owns_block(P))`.
   - **An (A) call** is accepted only as the direct operand of a `void* → T*` conversion. C's implicit conversion counts in an initializer, an assignment, a `return` or an argument; in C++ an explicit cast is needed.
     - `T` is a scalar, an enumeration, `bool`, an accepted pointer or a verified struct, with no `const` or `volatile`.
     - `S`, evaluated exactly from EDG's folded constants, must equal EDG's `sizeof(T)`. Otherwise the call is rejected `unsupported_expression`, naming M4.5e2.
     - The call becomes a cached **view function** for each (routine, `T`): bodiless, no parameters, result `Ptr(T)`, `ensures r == NULL || owns_block(r)`.
     - For calloc the view also has one value clause per leaf: integers 0, `false`, null pointers, an enumeration's 0. Every 0018 range contains 0. Null being all bits zero is a platform assumption the record states, true of arm64 macOS and x86-64 Linux.
     - The view carries `result_init = RefToUninit` for `malloc`, and its reason is vlibc's tag plus `; typed view: one <T> (<n> bytes), decision 0047`.
   - **A (D) call:** the argument must be a conversion from `T*`. The layout comes from `T` (0028 item 6). The view has one `Ptr(T)` parameter and a VIR flag `deallocates`.
   - **VIR never holds `void*`** (`mem_rej_pointer_to_void` stays). C++ is sound through implicit object creation ([intro.object]/10–13; cerastes structs are trivial). `std::malloc` is `::malloc`.
3. **Library markers.**
   - `lib_trust` counts markers written in library files, on the routine and on its parameters, so a marker-only declaration still needs its tag and passes the reserved-name and macro checks.
   - `read_marker` reads a library routine's markers by their library position, with no all-or-none rule and no `void*` refusal.
   - `reject_markers_on_system_redeclaration` skips library declarations.
4. **`owns_block`** (`cerastes_owns_block(p)`, `cerastes::owns_block(p)`; 0028 item 6, 0029 item 3) is a whole-block write permission. It holds:
   - offset 0;
   - the token `acc(loc(b, -1).tok) && tok`;
   - `lvloc(b) == loc(b, -2)` and the full liveness cell;
   - `len`, and every leaf cell (through `struct_lens` for a struct), with the flags unless `ref_to_uninit`.

   **No ghost amount, no partner needed:**
   - in a `requires` alone, the function takes the block;
   - in an `ensures` alone, on `Result`, it hands the block to the caller;
   - with both, on a formal, it borrows and returns the block.

   **In the VIR and lowering:**
   - `AtomKind::OwnsBlock`, kept in its own list in `FnAtoms`, so the 25 `nreq` loops and every existing program's Viper stay unchanged.
   - Its sites frame reads of its cells, so `ensures(r, owns_block(r) && r->balance == 0)` works.
   - Its cells join the entry distinctness asserts, and `lvloc` separation rows are added against the other sites.

   **Rejected,** each with its own message:
   - the two-argument form, `zeroed` or a `void*` operand in user code;
   - a place reached through a load, a global, `old`, a measure or an invariant (d2b);
   - beside `owns`/`reads`/`live` on the same root, or twice in one list.
5. **Targets `r`.** Registration becomes `owning` `"r|p"` and `ref_to_uninit` `"r|p"`, both `RAF_ENTITY_ATTACHED`.
   - `[[cerastes::owning]]` on a function desugars to `ensures(r == NULL || owns_block(r))`, merged with an identical explicit one.
   - `[[cerastes::ref_to_uninit]]` on a function requires such an `ensures`, and drops its flag rows.
   - On a function that returns no pointer: `unsupported_annotation`.
6. **Free checks: `invalid-free`, a new category.** At a (D) call, each row is guarded `p != null ==>`:
   - offset 0. This row cannot fail before e2's arithmetic; fixture `e2_double_free_fail.vpr` covers it.
   - token held at `write` and true: a double free, a local, a global or a member.
   - `lvloc` equation, an `internal` row.
   - full liveness: a callee may still hold a pointer.
   - `len` and member lengths, `internal` rows.
   - every leaf cell at `write`.

   **Differences from 0028 item 18's fixtures, recorded:**
   - views without parameters;
   - one row per cell instead of the range row;
   - `malloc`'s flags unknown, not `!d`. The verdicts are the same.
7. **Leaks: `leak`, a new category.**
   - **The check.** In every method of a heap program, as the last row of `postconditions()`: `assert forperm t: Ref [t.tok] :: <exemption>`. The exemption is the disjunction of `res != null && t == loc(block_of(res), -1)` and, for parameters, the block sites of the `ensures` (guarded as written), or `false` when there are none.
   - **A heap program** calls a view or a function with an `ensures` block site, or has a block site in a `requires`. Every other program's Viper is unchanged.
   - **`main`:** decided by question 123.
   - **New VIR and Viper pieces:** `LExpr::Kind::ForPerm`, `field tok: Bool` in heap programs only, and new lint rules (`tok` only inside the block conjunction; `forperm` only on a `leak` row).
   - **Why exit is sound:** a leak is a resource rule, and no verdict about undefined behaviour depends on it. The model argument stands: there is no new axiom, and every view's postcondition holds of a fresh block.
8. **Initialization.**
   - `malloc`: the flags are unknown (`uninitialized` on a read before a write).
   - `calloc`: flags set, values zero.
   - `owns_block` in an unmarked `ensures`: includes the flags.
   - **Under `--strict-init`:**
     - `free`'s marked parameter is exempt from 0042 item 8's argument rule.
     - A local receiving a `ref_to_uninit` result is rejected `unsupported_annotation`, naming d2b (target `v`, P4222R3/P4416).
     - The default mode is unchanged.
9. **Two calls** (0028 item 13, 0039 item 4). Only in a full expression holding a call with a block site in its `requires`, so existing Viper is unchanged:
   - calls register `Acc`s on `lv` (a block site at full, `owns`/`reads`/`live` at a fraction) and on `tok`;
   - pointer uses register reads of `lv`;
   - `register_access` then gives path-precise `unsequenced` for `(free(p), 0) + use_live(p)`, `+ (p != q)` and `+ *p`.
   - **Cut 3:** a fail-closed `unsequenced_calls` instead.
10. **Loops** (last commit, **cut 1**). Extends 0038 item 3, in heap functions only.
    - **New automatic places:** a memory-touching loop takes, as automatic places, the object each non-cell pointer variable in scope designates at entry: its cells, its liveness cell, and its **token cell** (a new place kind).
    - **The base:** the variable itself, or the loop-entry ghost `hb := q` when the loop assigns it (as 0045's `lb`).
    - **Guards and back edges:** the guards add `block_of(hb_i) != block_of(hb_j)`, and the back-edge row is `perm(tok) >= kt`.
    - **Also covered:** a `requires` block parameter's places gain the token.
    - **Written `owns_block` in an invariant:** rejected, naming d2b.
    - **If cut:** the false alarm is pinned by `negative/heap_loop_local_false_alarm.c`, the docs give the helper-function workaround, and d2b takes it.
11. **Still rejected,** each with a message naming its session:
    - a size other than `sizeof(T)` (`n*sizeof(T)`, 0, a non-constant): e2;
    - an (A) result not converted directly (`void *m = malloc(4)`, discarded, `if (malloc(4))`): M4.7/M8;
    - `void*` objects, parameters and results in user code, and other `void*` conversions: M4.7/M8;
    - `free(NULL)` with no static type, `free` through `const`/`volatile`;
    - `realloc`/`aligned_alloc`/`strdup` (`call_without_contract`);
    - the `owns_block` forms of item 4.
12. **Categories.** `invalid-free` and `leak` (20 categories) are added to all five copies, each with its Error messages section and example pair. The missing `annotation_reserved_library` section that `verifying.adoc:153` links to is added too.
13. **The P4222R3, P4416R0 and P4417R0 findings** are listed for d2b. Facts relied on: P72, S36.

## P72 (il-analyst; macOS, then the box's glibc container; C23 and C++20)

Before code:
- **(a) Allocation conversions.** The IL of `int *p = malloc(sizeof *p)`; assignment; `return malloc(…)`; `(struct S*)malloc(sizeof(struct S))`; `static_cast`, a C cast, `reinterpret_cast`. Record the conversion node, `compiler_generated`, parens and the folded `sizeof`.
- **(b) Constant sizes.** `calloc(1, sizeof *p)` and `malloc(4)`.
- **(c) Free arguments.** `free(p)`, `free(&s->y)`, `free(&x)`, `free((void*)p)` and `free(NULL)`: each argument's conversion and static type.
- **(d) vlibc's contracts:**
  - the `__r` binder;
  - `__r == NULL`;
  - `cerastes_owns_block(__r, __size)`'s variadic arguments;
  - `__nmemb * __size`;
  - where `CERASTES_REF_TO_UNINIT` lands on the routine and on `free`'s parameter.
- **(e) Registration `"r|p"`.** No startup abort; leading and trailing function positions; P52/P64's parameter positions unchanged; `v` and `d` still refused.
- **(f)** No platform macro named `__r`, `__size`, `__nmemb`, `__ptr` or `cerastes_*`.
- **(g)** glibc's attribute entries after the contracts; `std::malloc` from `<cstdlib>`.
- **(h)** Today's verdict for `r->balance` in an `ensures`.

## S36 (il-analyst; `tests/viper-smoke/memory/s36_*`, each with vacuity clones)

- (a) Views: `int`, `struct S`, `calloc` of `S` (with `int`, `bool`, pointer and enumeration leaves), and `free`.
- (b) `assert false` after every view call and at the start and end of every method: each must fail.
- (c) Leak shapes: `false`; the result; a parameter's `ensures`; two atoms; a guarded atom; after loops; `main` with `program_start`; after `exit`.
- (d) Call rows, one error per twin: double free; `free(&x)`, `free(&g)`, `free(&s->y)`; a borrowed parameter; a keeper, then free; a cell given away.
- (e) Return rows: `&local`, `&g`, a borrowed parameter, a freed block.
- (f) A read framed by `owns_block`, in the method, in `f__wf` and at the caller.
- (g) Consuming parameters and separation rows: aliasing arguments must never give exit 3.
- (h) Sequencing over `lv` and `tok`.
- (i) The loop rule.
- (j) 0042's kept-flag row with `keep(p); q = malloc(...); init(q)`.
- (k) Freshness against held places.
- (l) Timing with five blocks, on a quiet machine.

## Team

Three Sonnet agents with disjoint files, and one Opus reviewer, all from `.claude/agents/`.

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | **First:** P72 (macOS, then the box) and S36. **Then:** `src/vir` (`vir.h`, `vir.cpp`, `check.cpp`, `lower.cpp`) and `src/viper` (`lowered.h`, `emit.cpp`, `prelude.cpp`, `selftest_cases17.inc`, `VIR_CASES_MIN`); `build/m4.5d2a/vprdiff.py` (no allowed differences); `il-inventory.md` P72 and S36. |
| **edg-backend** (Sonnet) | `encode.cpp` (shapes, views, the `void*` conversions, library markers, the routine diversion, `OwnsBlock` spec atoms, targets `r`, strict-init); `register_attributes.cpp`; `il_query.cpp:33`; `annotation_checks.cpp:1007`; `include/cerastes.h` (`cerastes_owns_block`, `cerastes_zeroed`, `cerastes::owns_block`); the pin move (gitlink plus the `local` remote); builds. |
| **test-mutator** (Sonnet) | vlibc's commit (`include/vlibc/stdlib.h`, `test/stdlib_run.c` run cases, `CERASTES_COMMIT`, `CLAUDE.md`) in `~/Documents/cerastes/vlibc` on local `main`, unpushed; `vlibc_contracts.py` (the reserved-identifier rule) with self-tests; `trusted_contracts.txt`; the tests, twins and ports below; the categories in `run_tests.py`, `test_run_tests.py` and `tests/README.md`; `lint_viper.py` with `test_lint_viper.py`; doc example files. |
| **soundness-auditor** (Opus, read-only) | **Round 1:** record 0047's draft and S36. **Round 2:** the code and tests, plus the pin audit (vlibc's three contracts against C23 7.24.3 and against each platform's attributes: `malloc`, `alloc_size`, `warn_unused_result`, `nothrow`). **One re-audit** of the blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Step 1; the baseline; the record; the docs prose; `encoding.md`; the 0011 table; integration; serialized builds (`lockf build/m4.5d2a/build.lock`); status and handoff. |

**Rules for every agent:**
- Any question from an agent, or of mine, stops that work and goes to the owner.
- Agents hold builds before a directory-wide run.
- Single files are verified directly with `build/cerastes verify`.

## Tests

Location: `tests/memory` (heap), `tests/memory/negative`, `tests/memory/verifast`, `tests/vlibc`. Both modes unless marked. Each positive has a twin that fails for one reason at one line.

| Positive | Twins (category) |
|---|---|
| `heap_malloc_free.c` | no null test (`null-dereference`); read before write (`uninitialized`); double free (`invalid-free`); `*p` after free (`permission`); `p == q` after free (`lifetime`) |
| `heap_free_kinds.c` | `free(&x)`, `free(&g)`, `free(&s->y)`, a borrowed parameter (each `invalid-free`) |
| `heap_leak_paths.c`, `heap_exit_holds.c` (verified: `exit` while holding) | no free; one branch; an overwritten pointer; `main` returns holding (each `leak`) |
| `heap_struct.c` (+ `.cpp` with `static_cast`) | a callee keeps `&s->y`, then free (`invalid-free`) |
| `heap_calloc_int.c`, `heap_calloc_struct.c` | a wrong value (`check`) |
| `heap_owning_result.c` | returns `&static_s` (`permission`); the caller leaks (`leak`); an uninitialized leaf without the marker (`uninitialized`) |
| `heap_owns_block_value.c` | a wrong `r->x` (`postcondition`) |
| `heap_consume.c` | not freed (`leak`); the caller uses it after (`permission`); passes a borrowed parameter (`permission`) |
| `heap_borrow_then_free.c` | a keeper blocks free (`invalid-free`) |
| `heap_ref_to_uninit_result.c` | read first (`uninitialized`) |
| `heap_seq_distinct.c` | `(free(p),0)+use_live(p)`, `+(p!=q)`, `+*p` (each `unsequenced`) |
| `heap_loop_local.c` (if loops land) | the back edge loses the token (`permission`) |
| vlibc: `<cstdlib>` with `std::malloc`/`std::free` (C++); explicit `<vlibc/stdlib.h>` | a user contract on `free`, a definition of `malloc` (`annotation_reserved_library`) |
| ports: `account.c`, `deposit.c`, `swap.c`'s `main`, `globals.c`'s malloc half | `malloc_block.c`'s `free(&local)` (`invalid-free`) |
| one pin per row of item 11, with its own message | — |

**Self-tests:**
- the VIR self-test (`selftest_cases17.inc`; old cases byte-identical);
- the lint rules;
- `vlibc_contracts.py`: a non-reserved contract identifier, the shapes;
- the categories in every copy.

## Docs (invariant 8; every example is a test)

- **Memory and ownership:** a new section "The heap": `malloc`, `calloc`, `free`, typed views, `owns_block`, owning results, leaks and `main`, `invalid-free`. The "not supported yet" list is updated (`realloc` → e2; owning locals, fields and partnerless atoms → d2b).
- **Annotations:**
  - `owns_block` in the spec-function table;
  - owning on functions (`:279-289`);
  - `ref_to_uninit` on functions (`:321-324`);
  - vlibc's covered names (`:229-231`).
- **Error messages:** `invalid-free` and `leak` sections with example pairs; the `annotation_reserved_library` section; the count (`:12`).
- **Verifying a program:** the vlibc section (`:139-141`, malloc family and the typed view); the stale "eleven names" (`:171`); `void*` (`:935-941`).
- **`internal/encoding.md`.**

## Order of work and commits

1. **Step 1 and the baseline.** In parallel:
   - P72 and S36 (il-analyst);
   - the record draft, with audit round 1 in the background once S36 exists;
   - edg-backend reads the touch points.
2. **Commit 2.** No existing `.vpr` or `.map` changes. Together:
   - the vlibc commit, the pin through `local` (`NOT YET PUSHED`) and `<cerastes.h>`;
   - shapes, views, the `void*` conversions, library markers and the routine diversion;
   - `OwnsBlock` sites, `tok`, the `forperm` leak and the two categories (all five copies plus doc sections);
   - lint and the VIR self-test;
   - the malloc, free, leak and invalid-free tests.
3. **Commit 3.** User `owns_block` and targets `r` (framing, separation and entry rows, block parameters in loops); strict-init; ports `account.c` and `deposit.c`; the two flips and the re-messaged pins; Annotations.
4. **Commit 4.** `calloc` and `cerastes_zeroed`; sequencing; `main` and `exit`; `swap.c`'s `main`; `globals.c`; the `malloc_block` twin; the Memory and Verifying docs; `encoding.md`. Then audit round 2, the fixes and one re-audit.
5. **Commit 5 (cuttable).** Loops (item 10).
6. **Last.**
   - `check.sh` (full, at most twice);
   - the box: bootstrap; `tests/vlibc`, the heap tests and `check_vlibc_headers.sh` on glibc;
   - vlibc's `scripts/check.sh` on macOS, and in the container (`gcc-16`, `clang-21`);
   - status, handoff, and plan.md's Done row.

**Cut order if context runs short:**
1. Loops → d2b.
2. `calloc` → e2.
3. Path-precise `lv`/`tok` sequencing → fail-closed.
4. `ref_to_uninit` target `r` → d2b.
5. A parameter's `ensures owns_block` → d2b.

## Stop and ask

- Any question from an agent.
- An existing test's outcome changing outside the flips, or any existing `.vpr` or `.map` changing.
- P72:
  - EDG refuses `"r|p"` or vlibc's declarations in a mode or on a platform;
  - the IL loses the `void*` conversion or the folded `sizeof`;
  - a platform macro collides with a reserved name.
- S36: a vacuity clone that passes; Silicon failing first with its own reason (exit 3); a shape it refuses.
- A wrong verdict today on a shape the session touches.
- Exit 3 on a correct program, or a new test over 60 s.
- Context running short after commit 4: apply the cut order.

## Acceptance (commands run at the end)

- `python3 scripts/run_tests.py tests/scalar tests/vlibc tests/memory doc/modules/ROOT/examples`: all pass.
- `build/m4.5d2a/vprdiff.py build/m4.5d2a-baseline-vpr build/vpr`: every existing program byte-identical. The VIR self-test's old cases are byte-identical to `build/m4.5d2a-baseline-vir`; the new cases pass.
- Green:
  - `python3 scripts/vlibc_contracts.py check --list tests/vlibc/trusted_contracts.txt`;
  - `python3 scripts/vlibc_tree.py check` (`NOT YET PUSHED`);
  - `python3 scripts/test_run_tests.py`;
  - `python3 scripts/test_lint_viper.py`;
  - `scripts/build_docs.sh`;
  - `python3 scripts/check_doc_examples.py --html build/doc`.
- `scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): green.
- vlibc: `scripts/check.sh` green on macOS with both compilers, and in the Linux container with `gcc-16` and `clang-21` over libc++.
- **The box** (clean bootstrap in the container): `tests/vlibc`, the heap tests, `check_vlibc_headers.sh`.
- **Status entry:** record 0047, the audits, the plan file, and the next step (M4.5d2v). The d1 entry becomes a history row.
- **Commits:** planning; vlibc and the heap core; owning results; calloc, sequencing and ports; loops; docs, status and handoff. All local on `m4.5d2a-heap`, no Claude trailer, never pushed.
- **Handoff owner steps, in order:**
  1. PR #27 first.
  2. Push vlibc's `main`: the d2a commit.
  3. Then this branch's PR.
