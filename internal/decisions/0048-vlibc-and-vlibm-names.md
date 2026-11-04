# 0048 — vlibc's and vlibm's own names: implementations under `vlibc_`/`vlibm_`, contracts shared with the standard names

- **Status:** accepted (2026-11-04, after M4.5d2a), written before code (invariant 6). Owner decision (handoff question
  128), choosing between the two readings of "substitute `malloc` for `vlibc_malloc`" in the Context: "re-write the
  plan and decision record to put things into vlibc_ or vlibm_ depending on the library moving forward. meaning A is
  what I meant" — (a) programs keep the standard names and their trusted contracts, and vlibc's implementations carry
  the same contracts under vlibc's own names; not (b), a rename of the standard names onto vlibc's in the program.
  Items 2 (functions implemented before their standard name has a contract) and 4 (blocks know their allocator) hold
  recommendations that M4.5d2v's planning confirms or changes (questions 129, 130).
- **Milestone:** M4.5d2v (vlibc's allocator), and every later session that adds an implementation to vlibc (M4.6's
  string functions) or to vlibm.
- **Amends:** 0030 item 2 (where the implementations live and what a user links), 0046 items 4 and 6 (the `vlibc_`
  names among the reserved names; vlibc's own `src/`), 0047 question 121 (no replacement allocator under the standard
  names) and its last consequence, and 0047 item 2's shapes, read per allocator (item 4).

## Context

Question 121 (record 0047) had vlibc replace the platform's allocator under the standard names. What that costs
(handoff §3 at M4.5d2a's end):

- **Undefined behaviour.** C23 7.1.3 reserves the library's external names: a library outside the implementation that
  defines `malloc` — or `strlen`, which 0030 item 2's `libvlibc` would also have defined — has undefined behaviour,
  except under a platform's documented replacement. glibc documents one; macOS does not.
- **macOS.** ld(1)'s two-level namespace rebinds only the program's own calls, so libSystem's allocations (`strdup`,
  `getline`) need a malloc zone made the default zone, and Apple clang's `-ftyped-memory-operations` calls
  `malloc_type_malloc`/`malloc_type_calloc` (d2a audit 2, N4).
- **glibc.** The whole replacement set (twelve functions, `memalign`, `valloc`, `pvalloc`, `malloc_usable_size` among
  them), initial-exec TLS, `free` leaving `errno`.
- **One process, one heap.** Every allocation the platform makes and the program frees through the replacement (or the
  reverse) corrupts it; ASan, valgrind and the C++ runtime's `operator new` all meet the replacement; a bug in it takes
  down every process it is linked into, its own test harness included.
- **Compilers know `malloc`.** A zeroing loop inside `calloc` may become a call back into the replacement
  (`-fno-builtin`).
- **Verification.** Record 0046 item 6 rejects a definition of a reserved name, so vlibc's own `malloc` would have needed
  an exemption from the rule that protects user programs.

"Substitute `malloc` for `vlibc_malloc`" has two readings:

- **(a) The contract.** The program calls `malloc`; cerastes uses `malloc`'s trusted contract; `vlibc_malloc` has the
  same contract and is verified against it.
- **(b) The program.** A `#define malloc vlibc_malloc` (or the linker's `--wrap=malloc`) makes the program call vlibc's.
  That is undefined behaviour (7.1.3: a reserved identifier defined as a macro), it is what 0046 item 4 refuses (a
  program's macro over a trusted contract's text) and what vlibc's header rule forbids (no `#define` in a shim header),
  and it splits the heap silently: what the platform allocates (`strdup`, a system function with a user's contract)
  reaches vlibc's `free`, and both sides are an `owns_block` to cerastes.

The owner chose (a).

## Decision

1. **Names.** vlibc defines each function it implements as `vlibc_<name>`, and vlibm as `vlibm_<name>`, where `<name>`
   is the C standard library's name: `vlibc_malloc`, `vlibc_free`, `vlibc_strlen`, `vlibc_memcpy`; `vlibm_sqrt`,
   `vlibm_sqrtf`, `vlibm_sqrtl`. Each has the C standard's signature and semantics. Neither library defines, aliases,
   interposes or `#define`s a name of the standard library: no malloc zone, no replacement set, no `--wrap`, no
   `DYLD_INTERPOSE`. The prefixes are neither reserved nor potentially reserved in C23 (7.1.3, 7.33: they are not `str`,
   `mem`, `wcs`, `is` or `to` followed by a lowercase letter, nor `cr_`, `stdc_`, `atomic_`, …), so the implementations
   are ordinary C23. A function either library adds that the standard does not have (none is planned) takes its prefix
   too.
2. **Declarations: beside the standard name, with the same contract.** `vlibc_<name>` is declared in vlibc's header for
   the standard header that declares `<name>` (`vlibc_malloc` in `<vlibc/stdlib.h>`), after `<name>`'s redeclaration,
   with C language linkage in both languages: an `extern "C"` block under `__cplusplus`, since no platform declaration
   gives it one, so one `libvlibc` compiled as C serves C and C++ callers. Its contract is `<name>`'s, token for token
   apart from the name, under its own tag (`// cerastes:trusted-contract vlibc: vlibc_malloc <reason>`).
   `vlibc_contracts.py check` refuses a pair whose contracts differ, and a `vlibc_<name>` with a contract whose `<name>`
   has none in vlibc.
   - *Recommended (question 130):* a function implemented before its standard name has a contract — `vlibc_realloc`
     until M4.5e2; `vlibc_aligned_alloc`, `vlibc_free_sized` and `vlibc_free_aligned_sized` until a session gives their
     standard names contracts (`free_sized`'s needs a deallocation shape with a size, which 0047 item 2 does not read) —
     is declared without one, untagged. A call of it in a verified program is `call_without_contract`. d2v's planning
     checks that such a declaration leaves programs that do not call it unchanged: it has `void*` in its signature and
     its position is in a library file, where 0047's declared-routine loop looks.
   - vlibm's headers follow the same rule when vlibm has any (floating point is deferred beyond M4).
3. **What a program runs, and what is proved.**
   - A program that calls `malloc` runs the platform's allocator, trusted to meet `malloc`'s contract (0046 item 4).
     That trust is permanent, not "until vlibc's allocator is verified" (0047's last consequence).
   - A program that calls `vlibc_malloc` and links `libvlibc` runs vlibc's allocator, which cerastes verifies against the
     same contract when it can: the core after M4.6 (arrays, pointer arithmetic, predicates, char storage). The page
     glue over `mmap` stays trusted.
   - Verifying `vlibc_<name>` shows that real code meets the trusted contract, so a contract stronger than any
     implementation could meet would be caught. It says nothing about the platform's `<name>`. For an allocation
     contract, which allows `NULL`, it says little about the contract's strength: its value there is the verified
     allocator itself.
   - String functions (M4.6) are interchangeable: `strlen` and `vlibc_strlen` compute the same thing on the same
     memory. Allocators are not (item 4).
4. **Blocks know their allocator** (soundness; the rule's form and spelling are recommendations for d2v's planning,
   question 129). The platform's heap and vlibc's are different heaps: `free(vlibc_malloc(n))` and
   `vlibc_free(malloc(n))` are undefined behaviour, and with identical contracts (item 2) record 0047's model cannot tell
   the blocks apart.
   - The model records each heap block's allocator. An allocation (0047 item 2's shape (A)) sets it to the routine's —
     the platform's for a standard name, vlibc's for a `vlibc_` name — and a deallocation (shape (D)) requires its own;
     a mismatch is `invalid-free` (no new category: it is an invalid free). The shapes stay the only way an allocation
     is recognized; the allocator comes from the routine's name class, both classes read from vlibc's generated
     reserved-name list (item 5), not from a table in the encoder.
   - A vlibc function that allocates (a future `vlibc_strdup`) allocates with `vlibc_malloc`, so its results are freed
     with `vlibc_free`.
   - A user's contract: `owns_block(p)` keeps meaning a block of the standard allocator, so every program and test keeps
     its meaning (invariant 1); a block of vlibc's allocator gets a spelling of its own, decided with d2b's owning
     results.
   - The pin moves to a vlibc whose header declares the `vlibc_` allocation functions with contracts only together with
     this rule: before it, cerastes would accept `free(vlibc_malloc(n))`.
5. **Reserved names and vlibc's own sources** (amends 0046 item 6). A `vlibc_<name>` declared with a contract is a name
   vlibc declares with a contract, so `vlibc_contracts.py names` lists it and 0046 items 4, 6 and 8 apply to it
   unchanged: it is trusted from vlibc's tag, and a program's definition of it or contract on it is rejected
   `annotation_reserved_library`. vlibc's `src/` defines only `vlibc_` names, never a standard one, so the rule for
   verifying vlibc's own sources (a reserved name's definition, accepted there and checked against its header's
   contract; question 121's "a rule of its own") exempts nothing that protects `malloc`. The first session that
   verifies a vlibc implementation decides it (M4.6, whose string functions come first).
6. **M4.5d2v's scope** (replaces question 121's list).
   - **vlibc's half:** a complete allocator under item 1's names — `vlibc_malloc`, `vlibc_calloc`, `vlibc_realloc`,
     `vlibc_free`, `vlibc_aligned_alloc`, `vlibc_free_sized`, `vlibc_free_aligned_sized` (C23 7.24.3) — with
     `max_align_t` alignment and thread safety (7.24.3p2's total order), over a portable C23 core (free lists over
     regions) and pages from `mmap` (POSIX, on both platforms), built as `libvlibc` and usable from C and C++ without
     cerastes. Run tests, threaded ones included, in vlibc's CI on both platforms with both compilers, also under ASan
     and UBSan (which a replacement would have collided with). What POSIX adds to C23 (`errno` set to `ENOMEM`) is
     decided at planning.
   - **Dropped** with question 121's design: the malloc zone, the `malloc_type_*` entry points, glibc's replacement set
     with its TLS and `errno` rules, and `-fno-builtin` as a correctness requirement (the compilers' knowledge of
     `malloc` does not apply to `vlibc_malloc`, and a loop they turn into `memset` calls the platform's).
   - **cerastes's half:** the pin move (0046 item 1: `check.sh`, audited), item 2's pairing check, item 4's rule with its
     must-verify/must-fail pairs (invariant 2), a must-fail test for each new trusted contract (0030 item 5), and the
     docs: the `vlibc_` functions in "Verifying a program" and "Memory and ownership" (invariant 8).
   - Its planning may split it into vlibc's and cerastes's halves (question 77).
7. **No replacement of the standard allocator.** A program that wants vlibc's allocator calls `vlibc_malloc`.
   Replacing the platform's allocator under the standard names (question 121's design) becomes an entry of M8's
   inventory, not planned.

## Consequences

- vlibc and vlibm are ordinary C23 libraries: their names cause no undefined behaviour, they change nothing in the
  platform's library, and they can be tested in isolation and under sanitizers.
- Programs and every existing test keep their meaning: `malloc`, `calloc`, `free`, `abort`, `exit` and `_Exit` keep
  vlibc's trusted contracts on the standard names, and the trusted contracts' tags and lines are unchanged.
- The encoder gains one distinction by name class (a standard name or a `vlibc_` one) for a heap block's allocator;
  an allocation is still recognized by its contract's shape alone.
- M4.5d2v is smaller in vlibc than question 121's plan, and gains the allocator rule in cerastes.
