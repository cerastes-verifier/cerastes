# Plan: M4.5d2v — vlibc's allocator, with the M4.5d2a re-audit follow-up (record 0050)

The owner approved this plan on 2026-11-04, in plan mode. Their answers are recorded below.
- **Branch:** `m4.5d2v-allocator`, from `origin/main` `9255b93` (S6 merged).
- **Worktrees:** the session works in `.claude/worktrees/m4.5d2v`; the baseline is the detached worktree `.claude/worktrees/m4.5d2v-base` at `9255b93`.
- **vlibc:** its half is committed in the working clone `~/Documents/cerastes/vlibc` on local `main` (now `1439a7a`), never pushed.
- **First execution step:** save this plan as `.claude/plans/m4-5d2v-allocator.md`.

## Context

M4.5d2a (record 0047) gave the standard names `malloc`, `calloc` and `free` trusted contracts in vlibc. Record 0048 (question 128) then decided where implementations go:
- vlibc's own implementations are named `vlibc_<name>`;
- each is declared beside its standard name, with the same contract;
- nothing replaces the platform's allocator.

M4.5d2v builds that allocator. It is complete and thread-safe, and works without cerastes. cerastes gets the matching half:
- a pairing check between each `vlibc_` name and its standard name;
- heap blocks that know their allocator, so `free(vlibc_malloc(n))` is `invalid-free`;
- a rule for the four `vlibc_` functions whose standard names have no contract yet;
- the pin move;
- tests and docs.

The handoff puts the M4.5d2a re-audit's open items (B1, S1, S2, S3) before d2v, and the owner confirmed that.

**Where things stand:**
- `main` = `origin/main` = `9255b93`.
- The EDG pin `f0ed5525af` is unchanged (question 69).
- vlibc's submodule is at `35e9b6f`. The clone's `1439a7a` changes only `CLAUDE.md`, and that commit holds the current rules.
- vlibc has no `src/` yet, and its `vlibc` target is INTERFACE-only.
- The re-audit report `audit3.md` is gone. Its repros and the fixed `.vpr` files survive in `/private/tmp/claude-501/-Users-mattborland-Documents-cerastes-cerastes/10e100ed-0977-4a3f-bd2b-28d149d810d5/scratchpad/a3/`.

**What the planning surveys found:**
- **Untagged declarations are rejected today.** A contract-less `void *vlibc_realloc(void *, size_t);` in vlibc's header would be rejected `unsupported_type` at vlibc's line in every program that includes `<stdlib.h>`. The cause: library files are verified files, so `Encoder::run`'s declared-routine loop (`encode.cpp:10101-10200`) reaches `check_signature`. Part 3 adds a rule for this.
- **The lint would skip vlibc views silently.** `lint_viper.py` recognises typed views by the literal names `malloc|calloc|free` (`:1556-1557`), and `lint_views` skips any view it does not match (`:1795-1800`). A `vlibc_malloc` view would therefore go unpinned. Part 2 closes this.
- **The token's value can carry the allocator.** It is `true` today (`lower.cpp:1814`, `:1868`, `:6679`).
  - A user's `owns_block` and `free`'s token row already require `true`.
  - So "a vlibc block has `tok == false`" gives 0048 item 4's meaning with no new field, no domain change and the same `forperm`.
  - Programs that call no `vlibc_` allocation routine keep byte-identical Viper.
- **`vlibc_contracts.py` already treats `extern "C" { }` blocks as transparent** (`:85`, `:331`). It has no pairing logic yet.
- **The pin guard needs a `local` remote.** `vlibc_tree.py` accepts an unpushed pin through the submodule's `local` remote (`NOT YET PUSHED`); the submodule has no such remote yet.

**Size:** about 2,100–2,600 changed lines in total:

| Part | Size |
|---|---|
| Part 1 (the re-audit fixes) | about 80 lines of `lower.cpp` and 14 tests |
| cerastes's half | about 650 source lines, about 30 tests, docs |
| vlibc's half | about 420 lines of C, 540 of tests, 100 of build and CI |

That is comparable to M4.5d2a, so it is one session with a split point (below).

## Owner decisions (2026-11-04, all the recommendations)

- **Order: B1, S1, S2 and S3 first.**
- **S3, fix A:** an automatic `internal` loop invariant keeps the pointer locals a loop assigns well-formed (heap programs only), and the free row's `WF` goes back to `internal`.
  - **Pre-approved re-expectation:** `tests/memory/negative/heap_loop_swap_false_alarm.c` loses exactly `error permission @ L38` and `error permission @ L39`.
  - Any other changed expectation still stops the work for the owner.
- **Question 129, blocks know their allocator.**
  - The allocator is set by the allocating routine's name class: a standard name, or a `vlibc_` name. The deallocating routine requires the same class, and a mismatch is `invalid-free`.
  - A user's `owns_block` keeps meaning a block of the standard allocator. A spelling for vlibc blocks comes in d2b.
  - The pin moves only together with this rule.
- **Question 130, contract-less library declarations.** `vlibc_realloc`, `vlibc_aligned_alloc`, `vlibc_free_sized` and `vlibc_free_aligned_sized` are declared without a contract and untagged. cerastes skips such a routine unless it is called, and a call is `call_without_contract` at the caller's line.
- **Session size:** one session. **Split point:** if vlibc's run, thread and sanitizer tests are not green by the time cerastes's half has passed its audit, stop and ask whether to finish the allocator in a session of its own.
- **Lock:** one global `pthread_mutex_t` in the trusted POSIX glue. The portable C23 core has no lock and no atomics.
- **Edge cases.**
  - `vlibc_malloc(0)`, and `vlibc_calloc` with a zero product, return a unique minimum-size block.
  - `vlibc_realloc(p, 0)` returns NULL and frees nothing: `p` stays valid and unchanged.
  - **errno follows POSIX.1-2024:**
    - `ENOMEM` when out of memory;
    - `EINVAL` for an unsupported alignment and for `realloc(p, 0)`;
    - the frees leave errno unchanged.
- **C++.** A `vlibc_malloc`/`vlibc_calloc` typed view is rejected in C++ (`unsupported_expression`, naming M7: placement new).
  - The reason: [intro.object]/10–13 and [c.malloc]/4 create objects only for `std::malloc` and its family.
  - C is fully supported. `vlibc_free` stays accepted in C++.
- **`vlibc_free_sized` / `vlibc_free_aligned_sized`:** the size and alignment are always checked, and a mismatch aborts. So do a double free the allocator can see, and an `aligned_alloc` block passed to `free_sized`.
- **Sanitizers.** If Homebrew GCC 16's ASan does not run on arm64 macOS, that configuration falls back to UBSan only.
- **Standing:**
  - 69: no EDG merge;
  - 77: one context per session;
  - 0030 item 5: every pin move is audited;
  - 0048 item 4: the pin moves only with the allocator rule.

## Step 1 (markdown only, then commit 1)

1. **Worktree.** `git worktree add .claude/worktrees/m4.5d2v -b m4.5d2v-allocator origin/main`, with the submodules cloned `--reference` (memory `background-worktree-setup`), then `scripts/bootstrap.sh`. In that worktree's `third_party/vlibc`, run `git remote add local ~/Documents/cerastes/vlibc`.
2. **Baseline.** In `.claude/worktrees/m4.5d2v-base`, detached at `9255b93`, in the background:
   1. `bootstrap.sh`;
   2. `run_tests.py -j 8`;
   3. the VIR self-test.

   Copy the results into the session worktree's `build/m4.5d2v-baseline-{vpr,vir}`.
3. **Preserve the a3 scratch.** Copy the a3 directory into `build/m4.5d2v/a3/`; `/private/tmp` may be cleared.
4. **Save the plan** as `.claude/plans/m4-5d2v-allocator.md`.
5. **`internal/handoff.md`:**
   - the header points at `git show 9255b93:internal/handoff.md`;
   - §3's "M4.5d2a follow-up" and "M4.5d2v" notes move into the plan;
   - §8: questions 129 and 130 are answered, and this planning's other answers are recorded as 134 onward, in the order of "Owner decisions";
   - §4: 0050 is d2v's record.
6. **`plan.md`:** the header becomes "the last: M4.5d2v", and the d2v row names the plan.
7. **Commit 1 (planning).** Local, with no Claude trailer (CLAUDE.md; this overrides the attribution reminder).

## Records (invariant 6: written before code; audit round 1 reads them)

- **Amendments to 0047, dated:**
  - item 10: B1's catch-alls and splits, S1's skipped parameter copies, S3's invariant;
  - items 6 and 10's `WF` category, `internal` again;
  - note N2's wording.
- **`internal/decisions/0050-vlibc-allocator.md`** (amends 0047 items 2, 6, 7 and 10, 0048 items 2 and 4, 0046 item 6):
  1. **The pairing check, and the class field in `vlibc_names.inc`.** The field is the paired standard name, so the encoder's name class is exactly what the script checked.
  2. **The allocator as the token's value.**
     - `loc(b, −1).tok` is `true` for a platform block and `false` for a vlibc block.
     - **What is trusted:** the views' postconditions, as before, plus the value by class.
     - **What is checked:** every deallocation's value row; every user `owns_block` at entry, at calls and at returns; held values are stable (the lint forbids writes, inhales and exhales of `tok`); loops carry the value with an `internal` invariant, and a body that replaces the block fails a user-category back-edge row first.
     - Leaks use `forperm` over the field, so they do not depend on the value.
  3. **`vlibc_heap_`.** The value rows, the split token rows and the loop value snapshots are emitted only in a program that calls a `vlibc_` allocation routine. Every other program keeps byte-identical Viper and `.map`.
  4. **Messages.** The allocator names in them are taken from the program's views, not from a table.
  5. **Question 130's rule**, and `call_without_contract` for the cast form `(T *)vlibc_realloc(...)` (and the platform `realloc`'s).
  6. **The C++ rejection** of vlibc allocation views, and its reason.
  7. **The `vlibc_` messages** of `check_reserved_library` (0048 item 5).
  8. **vlibc's allocator as built:**
     - the design;
     - the C23 choices above;
     - the glue as trusted for good;
     - no `pthread_atfork`;
     - the platforms' `max_align_t` (8 on arm64 macOS, 16 on x86-64; vlibc uses 16 on both).

## Spikes (il-analyst, before code)

- **P73 (IL facts).** On macOS, then in the glibc box container; both modes. vlibc's draft header (V1 below) is read through `CERASTES_VLIBC_SRC`.
  1. EDG accepts the declarations inside `#ifdef __cplusplus extern "C" {`.
  2. `vlibc_malloc`'s `decl_position` is at vlibc's line, with C linkage in C++.
  3. **Load-bearing:** a routine declared only once still has a `src_seq_secondary_decl` that carries its attributes. If not, `lib_trust` exits 3 at `encode.cpp:5694-5714`.
  4. `find_tag` finds the tag inside the block.
  5. `CERASTES_REF_TO_UNINIT` is placed as P72 found it.
  6. No asm label or weak attribute on any `vlibc_` name on glibc.
  7. `#pragma redefine_extname vlibc_malloc x` sets `asm_name`.
  8. `<cstdlib>` gives `::vlibc_malloc`.
  9. The contract-less routines' declarations are all in the library file.
- **S37 (Silicon facts).** Hand-written fixtures in `tests/viper-smoke/memory/s37_*`, in s36's style. Every positive verifies, every twin fails once at its marked row, and every vacuity clone fails once per method.
  - **(a) Allocators, mixed and mismatched.** Each mismatch's value row ends its path. Plus the double `vlibc_free`, the leak, a vlibc block lent to an `owns(*p)` callee, and one passed to an `owns_block` consumer.
  - **(b) Loop value snapshots.** `ktv`, the `internal` value invariant, the back-edge row, the replace-in-body twin (a user-category row, never exit 3), and `||`'s well-definedness.
  - **(c) B1.** The repros t11, t16, t19, t21–t25, reduced, with catch-alls and `block_of` token splits. Plus an adversarial pair of holders through member addresses, which decides whether B1's outside-holder splits are needed beyond the catch-alls. Plus the t28 timing with and without S1.
  - **(d) S3's `flag ==> WF(v)` invariant.** Plus the `free(l->q)` twin on an unwritten member: exactly one `uninitialized`.
- Results go in `internal/il-inventory.md` as `### P73` and `### S37`.

## Part 1: the M4.5d2a follow-up (`src/vir/lower.cpp`; commit 2)

**S1** (in `loop_places`, after `:6042`): skip the copy of a pointer parameter that has an atom place (`atom_params`) and is never assigned (`fn_.assigned_params`).

**B1:**
1. A catch-all `if (k > none) { }` after each heap snapshot (`kl`, `kt`, `kv`, `kd`; `:6641-6668`; one between `kv` and `kd`).
2. A `block_of(hb_i) == block_of(hb_j)` split between token places, in the split loop at `:6625-6630`.
3. Splits against outside holders. A new `heap_holders_` holds the pointer locals and parameter copies declared before the loop and not used in it, and the formals with a `requires` block site. Each gets a `block_of`, an `lvloc` and (for scalars with the same suffix) a recv split. Item 3 is kept unless S37 (c) shows the catch-alls alone suffice; the reason goes in the amendment.

**S3:**
- After the flag-snapshot invariants (`:6539-6557`), heap programs only: for every pointer local the loop assigns and that is declared outside it, an `internal` invariant, `flag ==> WF(v)` (or `WF(v)`).
- `:1885`'s row goes back to `internal`, with its comment.

**S2:** a pin only, no code.

**Tests** (`.c`, both modes; positives `native-ok`; each positive has a twin that fails for one reason at one line):

| Item | Positive | Twin |
|---|---|---|
| t22 | `heap_loop_member_alias.c` | double free → `invalid-free` |
| t16 | `heap_loop_member_param.c` | no `free(s)` → `leak` |
| t19 | `heap_loop_param_cursor.c` | `invalid-free` |
| t23 | `heap_loop_param_reset.c` | `leak` |
| t25 | `heap_loop_param_two_blocks.c` | `leak` |
| S2 (t34) | `heap_loop_carry_and_free.c` | `negative/heap_loop_carry_and_leak.c` → `leak` |
| S3 | `heap_loop_reassigned_free.c` | `negative/heap_free_uninit_member.c` → exactly `uninitialized` |
| S1 | `heap_loop_param_atoms.c` (three `owns` parameters and a loop, in a file that calls `malloc`; under 60 s) | — |

**Also in commit 2:**
- **VIR self-test:** Part 1 cases in `selftest_cases17.inc`. The changed old heap-loop cases are reviewed.
- **The pre-approved re-expectation** of `heap_loop_swap_false_alarm.c`.
- **`internal/encoding.md`:** the loop paragraph (`:541-542`).

**After commit 2:** take a second baseline (`build/m4.5d2v-part1-vpr`) for Parts 2–4.

## vlibc's half (in `~/Documents/cerastes/vlibc`, local `main`, no Claude trailer)

**Design (from the planning's design pass):**
- **Small blocks.**
  - A 16-byte header sits before every result. It holds the requested size, the offset to the storage, the class, an alignment code and a LIVE/FREED state.
  - Eleven power-of-two classes, from 32 B to 32 KiB.
  - Slots are carved from 1 MiB regions obtained from `mmap`, and each class keeps a LIFO free list.
- **Large blocks.** Anything bigger gets its own mapping, which is unmapped when the block is freed.
- **`vlibc_aligned_alloc`** over-allocates by the alignment. It accepts powers of two up to 2^30; anything else returns NULL with `EINVAL`.
- **`vlibc_realloc`** resizes in place within a 2× band, and otherwise moves the block.
- **`vlibc_calloc`** checks `n > SIZE_MAX / m`, then zeroes the block after unlocking. A fresh mapping is already zero, so large blocks skip the zeroing.
- **The core (`src/alloc_core.c`)** defines the seven public functions. It is single-threaded C23, its invariants are written as comments, and it has no POSIX and no atomics.
- **The glue (`src/alloc_glue.h`, `src/alloc_glue_posix.c`)** provides lock and unlock, errno, the page size, map and unmap. It uses `_DEFAULT_SOURCE`/`_DARWIN_C_SOURCE` and `MAP_ANONYMOUS` on both platforms.
- **Commits:**
  1. **V1** `<stdlib.h>`: an `#ifdef __cplusplus extern "C" {` block after line 52 (lines 1–52 stay byte-identical). It holds `vlibc_malloc`, `vlibc_calloc` and `vlibc_free` with the contracts of `malloc`, `calloc` and `free` token for token, each under its own tag, and the four contract-less declarations (reserved parameter names, no attributes).
  2. **V2** the allocator, `libvlibc` as a STATIC library (`vlibc` becomes the real target, linked with `Threads::Threads`; `vlibc::shim` unchanged), and the `vlibc_` cases in `stdlib_run.c`. These cover every function in C and C++, explicit and through the shim: boundary sizes, alignment, uniqueness, overflow → NULL with errno, the `realloc` paths, aligned sizes, errno kept by the frees, and four expected aborts.
  3. **V3** `test/stdlib_stress.c`: a single-thread stress run with a shadow model, plus 8 threads with cross-thread frees through a mailbox.
  4. **V4** sanitizers: `VLIBC_SANITIZE`, and `check.sh` configs `<cc>`, `<cc>-asan` (address and undefined, RelWithDebInfo) and `<cc>-tsan`, plus `all`. `Darwin/gcc-tsan` is refused, and `Darwin/gcc-asan` follows the owner's fallback.
  5. **V5** CI: the matrix gains configs, `libclang-rt-21-dev`, the `vm.mmap_rnd_bits` sysctl on Linux and a timeout of 45 minutes; `CERASTES_COMMIT` moves to `9255b93`.
  6. **V6** `CLAUDE.md`: status, commands, layout (private `vlibc_glue_*` names) and design.
- **Gates.**
  - cerastes's half starts from V1 through `CERASTES_VLIBC_SRC`.
  - The pin moves to the final vlibc commit, whose `include/` must equal V1's (`git diff V1 HEAD -- include/`). If cerastes forces a header change, a V1' commit comes first.
- **Cuts,** in this order: Darwin `gcc-asan` (the agreed fallback), Linux `gcc-tsan`, macOS TSan, the RelWithDebInfo sanitizer builds, the stress program's C++ build.
- **Never cut:** the seven functions, the header, the run tests, the threaded test, and ASan+UBSan with clang on both platforms and with GCC on Linux.

## cerastes's half (commits 3–5)

**Part 4, `scripts/vlibc_contracts.py`** (can land before the pin):
- **`read_dir`'s pairing:**
  - `vlibc_X` needs a trusted `X` in the same header;
  - its declaration must equal `X`'s once the name is substituted;
  - it must sit inside an `extern "C"` block, tracked in `split_statements`;
  - `paired = "X"`.
- **The one-line form is refused explicitly:** a trusted declaration starting with `extern`.
- **`names_fragment`** emits `{"name", "header", "paired"}`.
- **Self-tests:** a `Pairing` class, and `RealHeader` goes from six names to nine.
- **`tests/vlibc/trusted_contracts.txt`** gains three rows. This is a golden change, and the auditor reads it.

**Part 2, `encode.cpp`:**
- `VlibcName.paired` and `vlibc_own_name`.
- `HeapShape.allocator` is set in `recognize_heap_shape`.
- `heap_view` sets `f->allocator`.
- The `vlibc_` messages in `check_reserved_library` and `lib_trust`.
- The C++ rejection of vlibc allocation views in `allocation_view`.

**Part 2, `vir.h` and `check.cpp`:**
- `vir.h`: `enum class Allocator { Platform, Vlibc }` and `Function::allocator`.
- `check.cpp` accepts `Vlibc` only on a bodiless, trusted deallocation or allocation view.

**Part 2, `lower.cpp`:**
- `vlibc_heap_` beside `heap_`.
- `tok_value(p, alloc)`, used by `block_conj`, `token_held` and `block_rows` (an `Allocator` parameter).
  - Without `vlibc_heap_`, the atomic row stays exactly as it is.
  - With `vlibc_heap_`, a perm row and then a value row. The value row is `invalid-free` when deallocating, `permission` at calls and returns.
- `Place::ktv`: the snapshot, the value invariant and the back-edge row, under `vlibc_heap_` only.

**Part 2, `lint_viper.py`:**
- The view patterns become `(?:vlibc_)?(malloc|calloc|free)`.
- `view_problem` uses the base name and gives a vlibc view `!tok`.
- `lint_views` reports an unmatched `; typed view:` instead of skipping it.
- `lint_heap_token` allows exactly the `ktv_` snapshot.
- `test_lint_viper.py` gains the views' fixtures and mutations.

**Part 3, `encode.cpp`:**
- **In `Encoder::run`, after the `void*` diversion:** a bodiless routine declared in a library file, with no contract and no marker, is skipped like a system-declared one. `routine_has_contract` is factored out of `callee_for_call`.
- **In `pointer_conversion`:** a contract-less `void*` callee reaches `callee_for_call`, giving `call_without_contract`.

**The pin move** (commit 5, together with Part 2):
- The gitlink moves to vlibc's final commit through the `local` remote, reported `NOT YET PUSHED`.
- `vlibc_tree.py check` passes.
- `vprdiff`: zero differences against the Part 1 baseline outside programs that call a `vlibc_` allocation routine.

**VIR self-test:**
- New `selftest_cases18.inc` (`define_heap_allocators`) with these cases:
  - `ha_vlibc_life`, `ha_vlibc_calloc`, `ha_vlibc_borrow`, `ha_vlibc_loop`;
  - the rejection cases `ha_rej_vlibc_with_body` and `ha_rej_vlibc_not_a_view`.
- `VIR_CASES_MIN` in `check.sh:80` rises to the observed count.

## Team (three Sonnet agents with disjoint files, one Opus reviewer)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | P73 and S37 first. Then `src/vir` (`vir.h`, `check.cpp`, `lower.cpp`: Part 1 and Part 2's lowering), `src/viper` (`selftest_cases17.inc`, the new `selftest_cases18.inc`, `selftest.cpp`), `build/m4.5d2v/vprdiff.py` (recreated), and `il-inventory.md` P73/S37. |
| **edg-backend** (Sonnet) | cerastes: `encode.cpp` (Parts 2 and 3, the C++ rejection, messages), the `src/backend/CMakeLists.txt` comment, the pin move and the `local` remote, builds. vlibc: `src/` (core and glue), the top-level `CMakeLists.txt`, `scripts/check.sh` (sanitizer configs), `.github/workflows/ci.yml`. |
| **test-mutator** (Sonnet) | cerastes: every test and twin above and below, `vlibc_contracts.py` with its self-tests, `trusted_contracts.txt`, `lint_viper.py` with `test_lint_viper.py`, `tests/README.md`, the doc example files. vlibc: V1's header text, `test/stdlib_run.c`, `test/stdlib_stress.c`, `test/CMakeLists.txt`. |
| **soundness-auditor** (Opus, read-only) | **Round 1:** the 0047 amendments, record 0050's draft, P73/S37. **Round 2:** the cerastes diff and tests; the pin audit (the three `vlibc_` contracts token for token, the platform attributes); and vlibc's allocator (C23 UB, alignment, overflow, the lock's coverage, errno, the abort paths). **One re-audit** of the blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Step 1, the baselines, the records, the docs prose, `encoding.md`, vlibc's `CLAUDE.md`, integration, serialized builds (`lockf build/m4.5d2v/build.lock cmake --build build/cmake`), status and handoff, commits. |

**Rules for every agent:**
- **Questions:** any question from an agent, or of mine, stops that work and goes to the owner.
- **Builds:** agents hold builds while a directory-wide run is in progress.
- **Single files:** verified directly with `build/cerastes verify --keep-vpr`.

## Tests (cerastes, `tests/vlibc/`; each positive has a twin failing for one reason at one line)

The vlibc allocation tests are C-only (`// mode: c23`, `// c-only: vlibc_malloc creates no C++ objects, record 0050`), and include `<vlibc/stdlib.h>`.

| Positive (verified) | Twin(s) in `negative/` |
|---|---|
| `vlibc_alloc_roundtrip.c` | `vlibc_free_of_platform.c` and `free_of_vlibc.c` (`invalid-free` @ the call); `vlibc_double_free.c` |
| `vlibc_calloc_zero.c` (an `int` and a struct) | `vlibc_calloc_wrong_value.c` (`check`) |
| `vlibc_alloc_leak_free.c` | `vlibc_alloc_leak.c` (`leak`) |
| `vlibc_alloc_loop_after.c` | `vlibc_alloc_loop_free_after.c` (`invalid-free`) |
| `vlibc_alloc_loop_inside.c` | `vlibc_alloc_loop_inside_wrong_free.c` |
| `vlibc_alloc_mixed_loop.c` | `vlibc_alloc_loop_replace.c` (a user category at the back edge, no exit 3) |
| `vlibc_alloc_borrow_owns.c` (an `owns(*p)` callee) | `vlibc_alloc_to_owns_block.c` (`permission` @ the call); `vlibc_alloc_owning_wrapper.c` (`permission` @ the return) |
| — | C++ rejections: `reject_vlibc_malloc_cxx.cpp`, `reject_vlibc_calloc_cxx.cpp` (`unsupported_expression`); C++ `vlibc_free_of_platform.cpp` (`invalid-free`) |
| — | `reject_reserved_vlibc_malloc_definition.c`, `reject_reserved_vlibc_free_requires.c` (`annotation_reserved_library`), `reject_redirect_vlibc_malloc_extname.c` (`unsupported_declaration`) |
| — | Part 3: `reject_vlibc_realloc.c`, `reject_vlibc_aligned_alloc.c`, `reject_vlibc_free_sized.c`, `reject_vlibc_free_aligned_sized.c` (`call_without_contract` @ the caller's line) |

**Byte-identity:** a program that includes `<stdlib.h>` and calls none of the `vlibc_` functions keeps its Viper. The whole-suite `vprdiff` shows it, and the guard-skipped harness test still expects its five rejections.

**Flips:** only S3's two pre-approved lines. Any other changed outcome: stop and ask.

## Docs (invariant 8; every example is a test)

- **`memory.adoc` `#heap`:** "vlibc's allocator", with an example and its failing twin:
  - free with the allocator's own routine;
  - mixing the two is `invalid-free`;
  - `owns_block` names `malloc`/`calloc` blocks only, while `owns` works for both;
  - C only.
- **`memory.adoc` "What is not supported yet":** the four contract-less functions, vlibc blocks in contracts, and C++.
- **`verifying.adoc` `#vlibc`** (`:108-165`; `:144-148` has been stale since 0048):
  - the platform's `malloc` is trusted for good;
  - the `vlibc_` names and `libvlibc`;
  - the reserved `vlibc_` names.
- **`error-messages.adoc` `invalid-free`:** the mismatch bullet, with a console line copied from a run.
- **Other pages:**
  - `annotations.adoc:229-238`;
  - `ordinary-compilers.adoc` (linking `libvlibc`);
  - `dev/architecture.adoc:137-147`;
  - `dev/soundness.adoc:56`, `:68-73`;
  - one paragraph in `tutorial/heap.adoc` (0049 item 4; d2v owes no chapter).
- **`internal/encoding.md`:** the token's value.

## Order of work and commits

1. **Step 1 and the baseline.** In parallel: P73 and S37 (il-analyst), V1 (test-mutator), Part 4 (test-mutator), and the records drafted by the orchestrator, with audit round 1 in the background. **Commit 1:** planning.
2. **Part 1:** code, pins and self-test. The Part 1 baseline follows. **Commit 2.** Meanwhile edg-backend writes vlibc's core and glue (V2).
3. **Part 4 and the records:** **commit 3.**
4. **Parts 2 and 3 with the pin move,** with the tests, the lint and `selftest_cases18.inc`. `vprdiff` must be clean. **Commit 4.** vlibc's V3–V5 run alongside.
5. **Audit round 2** (cerastes and vlibc), the fixes and one re-audit. **Commit 5.**
   - **The split point:** if vlibc's run, thread and sanitizer tests are not green at this point, stop and ask.
6. **Docs, `check.sh` and the rest.**
   - Docs and V6.
   - `check.sh` (full, at most twice).
   - On the box (container `cerastes-m4.5d1:runner`, clean build): the touched tests, the `platform: linux` tests, and vlibc's `check.sh all` with `gcc-16` and `clang-21`.
   - Status, handoff and `plan.md`'s Done row. **Last commit.**

## Stop and ask

- **Questions:** any question from an agent.
- **P73's IL facts:** item 3 fails (vlibc's lone declarations carry no secondary entry), or EDG refuses the `extern "C"` block.
- **Viper changes:**
  - a `.vpr`/`.map` difference outside programs that call a `vlibc_` allocation routine (after Part 1's reviewed baseline);
  - Part 1 changing any outcome other than S3's two lines.
- **Time:** Part 1 making any `heap_loop_*` test slower than 2×, or over 60 s.
- **Exit 3** on a correct program.
- **vlibc:** a header change after V1 that cerastes's pin depends on.
- **The split point** above.

## Acceptance (commands run at the end)

**cerastes:**
- `python3 scripts/run_tests.py tests/memory tests/vlibc tests/viper-smoke/memory doc/modules/ROOT/examples`: all pass.
- `build/m4.5d2v/vprdiff.py build/m4.5d2v-part1-vpr build/vpr`: differences only in programs that call a `vlibc_` allocation routine. The VIR self-test's old cases are byte-identical to the Part 1 baseline.
- Green:
  - `python3 scripts/vlibc_contracts.py check --include-dir third_party/vlibc/include/vlibc --list tests/vlibc/trusted_contracts.txt`;
  - `python3 scripts/vlibc_tree.py check` (reports `NOT YET PUSHED`);
  - `python3 scripts/test_vlibc_contracts.py`;
  - `python3 scripts/test_lint_viper.py`;
  - `python3 scripts/test_run_tests.py`;
  - `scripts/build_docs.sh`;
  - `python3 scripts/check_doc_examples.py --html build/doc`.
- `scripts/check.sh` (full, with `CERASTES_CHECK_BASE` unset): green.

**vlibc:**
- `scripts/check.sh all` on macOS: `gcc`, `clang`, `clang-asan`, `clang-tsan`, plus `gcc-asan` or its fallback.
- On the box: `gcc`, `gcc-asan`, `gcc-tsan`, `clang`, `clang-asan`, `clang-tsan`.
- `git diff <V1> HEAD -- include/`: empty.

**Status entry:**
- record 0050 and the 0047 amendments;
- the audits;
- the plan file;
- the owner steps, in this order: push vlibc `main`, then the cerastes PR;
- the first step for the next session: M4.5d2b's planning.

**Commits:**
- cerastes: planning; Part 1; Part 4 and the records; Parts 2–3 with the pin; fixes; docs, status and handoff.
- vlibc: V1–V6.

Every commit is local, carries no Claude trailer, and is never pushed.

## Inputs (moved from handoff §3 at step 1, verbatim)

**M4.5d2a follow-up** (the re-audit, `build/m4.5d2a/audit3.md`; the first work of the next session)
- **B1 (blocking, exit 3):** heap-place snapshots in loops (record 0047 item 10; `src/vir/lower.cpp:6036-6093`, splits
  `:6618-6631`) lack case splits Silicon needs: `block_of(hb_i) == block_of(hb_j)` between token places, and splits (cell,
  `lvloc`, `block_of`) against every pointer local and formal declared before the loop (used or not) and every `requires`
  block-site formal; plus a catch-all `if (k > none) { }` after each heap snapshot for holders reached through loads. Repros
  t22 (`calloc`'d `s`, `int *x = &s->b;`, a loop writing `*x`, then `free(s)`), t16, t11, t19, t23/t24, t25, t21 in the
  auditor's scratch (`/private/tmp/claude-501/-Users-mattborland-Documents-cerastes-cerastes/10e100ed-0977-4a3f-bd2b-28d149d810d5/scratchpad/a3/`,
  may be gone: rebuild from the report); the confirmed fixes are listed there. Pin t22, t16, t19, t23, t25.
- **S1:** a pointer parameter's local copy becomes a heap place that is always guarded off (`v_a__0 != p_a__0`), so a file
  that calls `malloc` anywhere multiplies the splits of unrelated functions (30 → 100 in t28; 186 s → timeout): skip the
  copy of a parameter with an atom place that the function never assigns (`lower.cpp:6039`).
- **S2:** pin the back-edge exemption's precision (t34: carries `p`, leaks `q = malloc(…)` each iteration → `leak`).
- **S3:** the block row's `WF` check at a call became `permission` (`lower.cpp:1885`), turning an encoder precision loss
  into a user message; add an automatic `internal` invariant `flag ==> WF(v)` for pointer locals a loop assigns and restore
  `internal`, or fix record 0047 item 6 and the docs. Notes N1–N6 in the report (a must-fail for `writes_in_expr`, record
  item 10's wording, two code tidiness items, `Call` in `heap_guard` with M4.5f1, the textual `void *` check).

**M4.5d2v vlibc's allocator** (questions 121, 122 and 128; record 0048, which replaces question 121's design)
- vlibc's allocator is named `vlibc_malloc`, `vlibc_calloc`, `vlibc_realloc`, `vlibc_free`, `vlibc_aligned_alloc`,
  `vlibc_free_sized`, `vlibc_free_aligned_sized` (C23 7.24.3), complete and usable without cerastes: a portable C23 core
  (free lists over regions), pages from `mmap` on both platforms (trusted glue), `max_align_t` alignment, thread-safe
  (7.24.3p2's total order; the lock is chosen at planning), built as `libvlibc` and declared in `<vlibc/stdlib.h>` after
  the standard names, inside `extern "C"` for C++, each with its standard name's contract token for token and its own
  tag (0048 items 1, 2, 6). Run tests in vlibc's CI on both platforms with both compilers, with threads and under ASan
  and UBSan (`test/` links nothing today; cerastes's native runs compile only). What POSIX adds (`errno` = `ENOMEM`) and
  `realloc(p, 0)` are decided at planning. [0048; M4.5d2a planning]
- Nothing replaces the standard names: no malloc zone, no `malloc_type_*` entry points (macOS's `_MALLOC_TYPED`, d2a
  audit 2 N4, concerns only `malloc` itself, which stays the platform's), no glibc replacement set, no `-fno-builtin`
  requirement (0048 item 6). Replacing the platform's allocator is an M8 inventory entry (0048 item 7). [0048]
- cerastes's half (0048 items 2, 4, 6): `vlibc_contracts.py` learns the `extern "C"` block and the pairing check (a
  `vlibc_<name>` whose contract differs from `<name>`'s is refused); heap blocks know their allocator, a mismatch is
  `invalid-free` (question 129); the pin moves only with that rule; the four functions whose standard names have no
  contract yet are declared without one (question 130), checked to leave programs that do not call them unchanged;
  must-verify/must-fail pairs, a must-fail per new trusted contract, the docs. The planning may split the halves
  (question 77). [0048]
- d2a's contracts are trusted; the allocator's own verification comes after M4.6 (plan.md's d2v row), under the rule for
  vlibc's `src/` that M4.6 decides (0048 item 5). vlibc's `<stdlib.h>` already includes `<stddef.h>` for `size_t`/`NULL`
  (`35e9b6f`) [d2a].
