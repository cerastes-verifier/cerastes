# 0050 — vlibc's allocator: `vlibc_malloc` and its family, blocks that know their allocator (M4.5d2v)

- **Status:** draft (2026-11-04), written before code (invariant 6); revised after audit round 1 (Opus, on the draft:
  B1 — the value rows gated on vlibc allocations alone left `vlibc_free` without its value row in programs that allocate
  only with the platform (every C++ program), item 4; S1 — the loop value invariant replaces 0047's conjunct, item 5; S2 —
  the encoder's class fails closed, item 2; S3–S5 tests and the header's conditionals, items 2 and 11; N1–N14, taken in
  items 2–8 and the 0047 amendment; `build/m4.5d2v/audit1.md`). Owner decisions (M4.5d2v planning, 2026-11-04; plan
  `.claude/plans/m4-5d2v-allocator.md`; handoff questions 129, 130 and 134–142, each the recommendation): 129 (blocks know
  their allocator; `owns_block` keeps meaning the standard allocator), 130 (the four functions whose standard names have no
  contract are declared without one; cerastes skips such a library routine unless it is called), 136 (one session with a
  split point), 137 (one global mutex in the trusted glue), 138 and 139 (zero sizes, `realloc(p, 0)`, POSIX.1-2024's
  errno values), 140 (vlibc's allocations rejected in C++ until M7), 141 (the sized frees check and abort), 142 (macOS
  GCC's sanitizer fallback). Questions 134 and 135 amend 0047 item 10 (the re-audit's B1–S3).
- **Milestone:** M4.5d2v.
- **Amends:** 0047 items 2 (the shapes, read per allocator), 6 (the token row in a program that uses vlibc's
  allocator), 7 (unchanged in substance: `forperm` sees both allocators' tokens) and 10 (loops carry the token's value);
  0048 items 2 (the pairing check as built; the one-line `extern "C"` form refused) and 4 (the rule as built); 0046 items 4
  and 6 (a library routine declared without a contract, item 6 below).

## Context

Record 0048 (question 128) put vlibc's implementations under their own names, declared beside the standard names with the
same contracts, and left two recommendations for this session (questions 129, 130), both accepted:

- **Two heaps.** A program may call both `malloc` and `vlibc_malloc`. The platform's heap and vlibc's are different heaps:
  `free(vlibc_malloc(n))` and `vlibc_free(malloc(n))` are undefined behaviour. With identical contracts, 0047's model
  cannot tell the blocks apart, so it would verify both.
- **Four functions without a contract.** `vlibc_realloc`, `vlibc_aligned_alloc`, `vlibc_free_sized` and
  `vlibc_free_aligned_sized` exist before their standard names have contracts (`realloc`'s comes with M4.5e2; `free_sized`'s
  needs a deallocation shape with a size). Declared without a contract in a library file, each is a `void*` signature in a
  verified file, which `check_signature` rejects (`unsupported_type`) in every program that includes `<stdlib.h>`.
- **What 0047 already gives.** A heap block's token is the cell `loc(b, −1).tok`; an allocation view ensures full
  permission to it and its value `true`; a user's `owns_block` and `free`'s token row require both; leaks are
  `forperm t: Ref [t.tok] :: false`. The value has always been `true` and nothing reads it otherwise.
- **C++.** C++20 creates objects implicitly only for the operations it lists ([intro.object]/10–13): `std::malloc`,
  `calloc`, `realloc` and `aligned_alloc` ([c.malloc]/4), `operator new`, `memcpy`, `memmove`, the start of an array of bytes. 0047 item 2's
  typed views in C++ rest on that list. `vlibc_malloc` is not on it: its storage comes from `mmap` inside a C library.

## Decision

1. **vlibc's allocator** (in vlibc's repository; usable without cerastes; trusted contracts until cerastes verifies the
   core, after M4.6). Seven functions, C23 7.24.3: `vlibc_malloc`, `vlibc_calloc`, `vlibc_realloc`, `vlibc_free`,
   `vlibc_aligned_alloc`, `vlibc_free_sized`, `vlibc_free_aligned_sized`.
   - **Core and glue.** A portable C23 core (`src/alloc_core.c`) defines the seven functions: a 16-byte header before
     every block (the requested size, the offset to its storage, a size class, an alignment code, a live/freed state),
     power-of-two size classes from 32 B to 32 KiB carved from 1 MiB regions with a free list per class, and a mapping of
     its own for anything larger (unmapped when freed). The core has no lock, no atomics and no POSIX, and makes pointers
     only by `unsigned char *` arithmetic, so that cerastes can verify it later. The glue (`src/alloc_glue_posix.c`) holds
     what C23 cannot express and stays trusted for good: `mmap`/`munmap`, the page size, `errno`, and one global
     `pthread_mutex_t` that every core operation runs under (7.24.3p2's total order; question 137). No
     `pthread_atfork`: after `fork` in a threaded process POSIX allows only async-signal-safe calls.
   - **Choices C23 leaves open** (questions 138, 139, 141): every result is aligned to 16 (`max_align_t` is 8 on arm64
     macOS and 16 on x86-64); a zero size gives a unique block; `vlibc_realloc(p, 0)` returns NULL, sets `EINVAL` and frees
     nothing (C23 makes it undefined; POSIX.1-2024 allows NULL with `EINVAL`); `vlibc_aligned_alloc` accepts every power of
     two up to 2^30 and returns NULL with `EINVAL` for any other alignment; lack of memory is NULL with `ENOMEM`; the frees
     leave `errno` unchanged; `vlibc_free_sized` and `vlibc_free_aligned_sized` abort on a size or alignment that is not
     the block's, and the frees abort on a block they can tell is already free.
   - **Built as `libvlibc`** (a static library, C only; C++ callers reach it through the header's `extern "C"` block).
     Run tests in C and C++, through `<vlibc/stdlib.h>` and through the shim, a stress test against a shadow model and a
     threaded test with cross-thread frees, under ASan with UBSan and under TSan in vlibc's CI on both platforms with both
     compilers (macOS GCC: UBSan only where its ASan does not run, and no TSan; question 142).
2. **Declarations and the pairing check** (0048 item 2 as built). `<vlibc/stdlib.h>` declares, after `free` and inside
   `#ifdef __cplusplus extern "C" { #endif … }`, `vlibc_malloc`, `vlibc_calloc` and `vlibc_free` with `malloc`'s,
   `calloc`'s and `free`'s contracts token for token, each under its own tag, and the four functions of item 6 with no
   contract, untagged, with reserved parameter names. Lines 1–52 of the header are unchanged, so every existing `.map`
   row that points into it stays. `scripts/vlibc_contracts.py` refuses a tagged `vlibc_X` whose `X` is not tagged in the
   same header, whose declaration differs from `X`'s once the name is substituted, or that is not inside an `extern "C"`
   block, and refuses a tagged declaration written with a one-line `extern "C"` (its shape could not be read). The
   generated `vlibc_names.inc` gains a third field, the standard name a `vlibc_` entry pairs with, so the encoder's name
   class is exactly what the script checked (0048 item 4: no table in the encoder); the encoder fails closed (exit 3)
   unless a name starts with `vlibc_` exactly when its entry pairs it with the rest of the name (audit S2: a missing or
   empty field must never read as the platform's allocator). The script also refuses a tagged `vlibc_X` declared before
   `X`, and every preprocessor conditional except the include guard and `#ifdef __cplusplus` … `#endif` exactly around
   the `extern "C" {` and its `}` (audit S5: the script reads every branch as live, so a contract under `#if 0` would be
   listed as trusted). `tests/vlibc/trusted_contracts.txt`
   gains the three rows (a golden under invariant 1, read by the auditor).
3. **A block's allocator is its token's value.** `loc(b, −1).tok` is `true` for a block of the platform's allocator and
   `false` for a block of vlibc's. An allocation view of a `vlibc_` routine ensures `acc(loc(b, −1).tok) && !loc(b,
   −1).tok`; every other part of the view is 0047 item 2's.
   - **What is trusted:** the views' postconditions, as before, including the value by class (0048 items 1 and 4: vlibc's
     heap is a different heap, and a `vlibc_` allocation's block is vlibc's).
   - **What is checked:** a deallocation requires its own class's value, at an explicit row before the call
     (`invalid-free`); a user's `owns_block` keeps 0047 item 4's conjunct `loc(b, −1).tok`, so it means a block of the
     standard allocator — assumed at a function's entry, checked at its callers' calls and at its returns (question
     129: every existing program keeps its meaning); a held token's value is stable (it comes only from an allocation
     view, a `requires` block site or a callee's `ensures`, each with its value stated; the lint forbids any other
     assignment, inhale or exhale of `tok`); a loop carries each token place's value (item 5).
   - **Leaks** are unchanged: `forperm` ranges over the field, whatever its value, so a vlibc block still held at the end
     is a `leak`.
   - **Why the value and not a second field, a slot or a domain function:** the value needs no new field (one `forperm`),
     no change to the `Block` domain or its lint, and no change to any existing view or to `owns_block`'s text; a domain
     function would have needed a conjunct in every `owns_block` and every platform view, a second field or slot a second
     `forperm` and a second token place in every loop.
4. **`vlibc_heap_`.** A program whose VIR holds any function of vlibc's allocator — an allocation view or a
   deallocation view (`vlibc_free`) — is a vlibc heap program. Only there is the token row split (item 5) and the loop
   value carried; every other program's Viper and `.map` are byte-identical to what 0047 as amended at M4.5d2v emits
   (checked over the whole suite against the session's Part 1 baseline). In a program with no `vlibc_` view, every
   block and every deallocation is the platform's, so the omitted value rows hold trivially. (Audit round 1, B1: gating
   on allocation views alone left `vlibc_free` of a platform block — every C++ program's only vlibc call, item 7 — to
   Silicon's own precondition check, exit 3; or, had the view taken the platform's conjunct, verified.)
5. **Rows and loops in a vlibc heap program.**
   - **The token row** (0047 item 6, and the call and return rows of 0047 item 4) becomes two rows: the permission
     (today's category and message), then the value. At a deallocation the value row is `invalid-free`: "frees `p` with
     `free`, but it may be a block of vlibc's allocator" or "frees `p` with `vlibc_free`, but it may be a block of the
     platform's allocator" (the freeing routine's name is the callee's; the allocators are named by class, so no table
     of routine names is needed, audit N6); at a call to a function whose contract has `owns_block` it is `permission`
     ("… needs `p` to be a block of the platform's allocator (`owns_block`); it may be a block of vlibc's, which a
     contract cannot name yet"); at a return through `owns_block` or `owning` likewise. A failed value row ends its path
     (its assumption contradicts the held value).
   - **Loops** (0047 item 10): each token place gains a snapshot `ktv := loc(block_of(hb), −1).tok` taken when `kt >
     none` (`false` otherwise); the `internal` invariant `(g && kt > none) ==> loc(block_of(hb), −1).tok == ktv`
     **replaces** 0047's `(g && kt > none) ==> loc(block_of(hb), −1).tok` (kept beside it, it fails on entry for a vlibc
     block: audit S1); and its back-edge row becomes `g ==> !(perm(tok) < kt || (kt > none && tok != ktv))`, `permission` as before: a body that
     frees the block and allocates another in its variable fails that user-category row before any `internal` row can.
   - **A vlibc block at a call** to a function whose contract names its cells (`owns(*p)`, `reads(*p)`) works as any
     block does: the cells, the liveness fraction and the flags pass and come back. Only `owns_block` and `owning` need
     the platform's value. A spelling for a vlibc block in a contract is d2b's (with owning results).
6. **Library routines declared without a contract** (question 130; amends 0046 items 4 and 6). A bodiless routine whose
   declaration is in a library file and that carries no contract and no marker is read like a routine a system header
   declares: nothing about it is encoded unless it is called, and a call is `call_without_contract` at the caller's line
   (0011 item 6). This holds for the cast form too (`(int *)vlibc_realloc(p, 4)`, and the platform's `realloc`, which was
   `unsupported_type` in that form). A program's own declaration of such a routine with a contract stays under 0046
   item 6: `vlibc_realloc` is not a reserved name until it has a contract in vlibc, and its `void*` signature is then
   rejected at the program's line.
7. **C++** (question 140). A typed view of a `vlibc_` allocation routine is rejected in C++ (`unsupported_expression`
   at the call): C++20 creates no object in `vlibc_malloc`'s storage, so `int *p = static_cast<int *>(vlibc_malloc(4));
   *p = 1;` has no object to assign by the letter of the standard, although every compiler cerastes supports makes it
   work. The message names M7 (placement `new`; C++23's `std::start_lifetime_as` is not C++20). `vlibc_free` stays
   accepted in C++: a block it can meet there is the platform's, so a call is `invalid-free`. C programs are fully
   supported.
8. **Reserved names** (0048 item 5). `vlibc_malloc`, `vlibc_calloc` and `vlibc_free` are reserved like `malloc` for C
   language linkage: a program's contract on one or definition of one with C language linkage (in any namespace) is
   `annotation_reserved_library`, with a message saying it is vlibc's own function (not C23 7.1.3's reservation). 0046
   item 6's further C++ rule — any linkage in the global namespace — rests on [extern.names]/4, which reserves the
   standard library's names only, so it does not apply to `vlibc_` names: a C++ function `int vlibc_malloc(int)` with
   C++ linkage is another entity, an ordinary function (audit N8).
9. **VIR and the lint.** A view `Function` carries its allocator (`Platform` or `Vlibc`, from the routine's class);
   `check_program` accepts `Vlibc` only on a bodiless, trusted allocation or deallocation view. `lint_viper.py` pins the
   `vlibc_` views as it pins `malloc`'s (`!tok` in their token conjunct), reports a bodiless method tagged as a typed
   view that it does not recognise (it skipped such methods silently), and admits exactly the `ktv_` snapshot of item 5
   as a read of `tok`.
10. **Facts relied on:** P73 (`internal/il-inventory.md`: EDG and vlibc's `extern "C"` declarations, the secondary
    declaration of a routine declared once, the tag inside the block, `::vlibc_malloc` in C++, the platforms' attributes)
    and S37 (Silicon: the value rows, the loop value snapshots, the replace-in-body back edge, B1's splits, S3's
    invariant), each fixture with its failing twin and vacuity clone in `tests/viper-smoke/memory/s37_*`.
11. **Tests and documentation.** Must-verify and must-fail pairs for every accepted form (C only for vlibc's
    allocations; item 7's rejections in C++), a must-fail test for each new trusted contract (0030 item 5: a double
    `vlibc_free`, `vlibc_free` of a platform block, `free` of a vlibc block, a leak), item 6's four call rejections and a
    program that includes `<stdlib.h>` and calls none of them. Docs: "Memory and ownership" (vlibc's allocator, what is
    not supported yet), "Verifying a program" (vlibc's `vlibc_` names and `libvlibc`), "Error messages" (`invalid-free`'s
    mismatch), "Annotations", the developer guide's architecture and soundness pages, one paragraph in the tutorial's
    heap chapter; `internal/encoding.md`.

## Consequences

- A C program can use vlibc's allocator and have it checked like `malloc`: double and mismatched frees, uses after free
  and leaks, inside one function or through `owns(*p)` contracts.
- The platform's `malloc` keeps its trusted contract for good (0048 item 3), and every existing program's Viper is
  unchanged by this record (beyond 0047's M4.5d2v amendment, which changes heap programs with loops).
- A vlibc block cannot yet cross a call through `owns_block` or `owning`, and vlibc's allocations are rejected in C++,
  until d2b (a contract spelling) and M7 (C++'s object model).
- vlibc gains its first compiled library, run tests under sanitizers, and a core written to be verified after M4.6.
