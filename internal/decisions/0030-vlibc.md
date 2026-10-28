# 0030 — vlibc: C library contracts and verified implementations (M4.5a)

- **Status:** accepted for M4.5a (2026-10-27), finalized with P55; revised after audit round 3 (four should-fix items
  taken: the trust rule restated for the shim's placement, EDG's system-header flag, `noreturn`, reserved names). Owner decisions (M4.5a planning, 2026-10-27):
  - **A repository of its own.** "If we need to make our own ports of malloc and free (as shown in 4.5d) those need to
    go into a new repo in the cerastes organization named vlibc as opposed to having them in this repository like
    verifast does. That way users can write their code with vlibc in addition to us using them for verification
    work." The repository is `cerastes-verifier/vlibc`; the owner creates it.
  - **All three ways at once.** vlibc holds contract headers and verified implementations; a user's
    `#include <vlibc/stdlib.h>` is used, and a bare `#include <stdlib.h>` is transparently replaced in verify mode:
    "Basically combine all 3 of those options."

  Facts marked (P*n*) are probes, recorded with their evidence in `internal/il-inventory.md`, "M4.5a".
- **Milestone:** M4.5a (2026-10-27), implemented from M4.5d1.

## Context

A call reaches a function of another file only through its contract (0011 item 5). A system file's functions have
none, so a call to one is `call_without_contract`, except for `abort`, `exit` and `_Exit`, which a closed, name-keyed
table in the encoder trusts (`trusted_libc_reason`, `encode.cpp`; 0015 item 5, which left "whether it moves to a
header" to M4.5a). M4.5 needs many more: `malloc`, `calloc`, `realloc` and `free` (M4.5d2), and the string functions of
M4.6. Their contracts need permissions (record 0028), which a built-in table would have to spell in Viper; and users
want the same contracts in their own builds.

VeriFast keeps its C library contracts in its own `bin/*.h` (`malloc.h`, `stdlib.h`, `string.h`), which replace the
system headers when VeriFast reads a program; the program's real build uses the system headers. VerCors maps
`malloc`/`free` to built-ins (`res/c/stdlib.h`). Neither lets a user's ordinary build see the contracts, or verifies
any implementation.

## Decision

1. **The repository.** `cerastes-verifier/vlibc`, Apache-2.0 WITH LLVM-exception (as cerastes). Its headers are valid
   C23 and C++20, declare the C library's functions with the same signatures as the C standard, and state contracts
   with `<cerastes.h>`'s macros and C spec functions (record 0029 items 3–4), so ordinary compilers read them with
   the contracts expanding to nothing. `<cerastes.h>` is a prerequisite on the include path, not a copy in vlibc
   (one header, decision 0008).
2. **Contents.**
   - **Trusted contracts** for what C cannot implement in C or what cerastes cannot verify: `malloc`, `calloc`,
     `realloc`, `free` (M4.5d2), `abort`, `exit`, `_Exit` (moved from the encoder's table in M4.5d1). Each carries a
     reason tag in the header (as `// cerastes:trusted-contract` does in Viper, 0015 item 5) that the emitted method
     repeats.
   - **Verified implementations** of what can be written in C: `strlen`, `memcpy`, `memcmp`, `strcmp` and their
     neighbours (M4.6), in vlibc's `src/`, verified by a pinned cerastes in vlibc's CI and built as a library
     (`libvlibc`) that users may link. A user who does not link it uses the platform's implementation under the same
     contract (the single-unit assumption, 0011 item 6).
3. **Two ways in.**
   - **Explicit:** `#include <vlibc/stdlib.h>` (and `<vlibc/string.h>`, …) in user code, in every build. With an
     ordinary compiler the header forwards to the platform's declarations; with cerastes the contracts are read.
   - **Transparent, in verify mode only:** the driver puts vlibc's shim directory ahead of the system include path,
     so `#include <stdlib.h>` reaches a shim that includes the same contracts. Ordinary builds see the system headers
     unchanged (a user may opt in by adding the shim directory with `-I`).
   - **How a shim meets the system header (P55, macOS SDK and libc++, both modes).** The driver passes the shim
     directory as the first `--sys_include` (a `-I` directory is searched after the system directories for `<…>`, so it
     cannot replace `<stdlib.h>`). The shim starts with its own include guard and `#include_next <stdlib.h>` (which works
     in both modes; in C++ through libc++'s wrapper), then redeclares the functions with their contracts. Declaring them
     *after* the system header keeps the system declaration first, needs no `extern "C"` in C++ (before it, EDG's #337),
     and is accepted: a declaration without a contract followed by one with a contract is not a mismatch, and
     `cerastes verify` applies the later declaration's contract (P55 measured `abs` and `exit`). `<vlibc/stdlib.h>` is the
     same header reached through `-I`. Every C++ program that includes a standard header (`<vector>`, `<string>`, …)
     loads the shim, since libc++ includes `<stdlib.h>`; `<string.h>`, `<stdio.h>` and the other C headers do not.
     Not covered by a shim: `<malloc/malloc.h>`, `alloca`, and the `__builtin_` functions (no declaration); they stay
     rejected or `call_without_contract`. glibc declares `malloc` in `<stdlib.h>` itself with its own attribute macros:
     S5 checks the shim there.
   - **The attribute allow-list** (P55): once a user file (or a vlibc header) redeclares a system function, the encoder
     holds the system declaration's attributes to the allow-list of verified code, so in C++ `abs`'s `[[const]]` and
     `malloc`'s `[[alloc_size(1)]]` are rejected (`unsupported_attribute`), and `abort`'s `[[__cold__]]` when it is
     redeclared before the system header. M4.5d1 checks each attribute by where *it* is written (a system file's
     attributes are not ours), not by the routine.
4. **Trust** (amends 0011 item 4 and 0015 item 5). vlibc's directories (the shim directory and `<vlibc/…>`) are a
   **library** class, decided by canonical path as 0011 item 4 decides system files: first on the include path like a
   system directory, but **not** a system directory for cerastes's rules. A function body in a vlibc header (an `inline`
   helper, a `predicate` or `pure` function) is verified like user code, and the `spec_marker_in_system_file` check uses
   the path rule, not EDG's system-header flag (P55: the flag would refuse vlibc's predicates). Passing the shim with
   `--sys_include` makes EDG treat vlibc headers as system headers (suppressed warnings, relaxed rules): M4.5d1 audits
   every use of EDG's system-header flag in cerastes and in the cerastes configuration (audit round 3), so that nothing
   in a vlibc header is checked less than user code. A bodyless declaration
   with a contract is trusted exactly as any declaration with a contract already is (0011 items 5–6: the single-unit
   assumption, marked in Viper by `// cerastes:trusted-contract`), with its own reason (`vlibc: <name>`). A user may
   redeclare a vlibc function only with the same contract (must-match). Any other system function stays
   `call_without_contract`. The table of 0015 item 5 moves to vlibc's `<stdlib.h>` contracts in M4.5d1. Its rule
   ("declared only in system files", C linkage, the exact signature) becomes (audit round 3: with the shim the system
   declaration stays first, so "first declared in vlibc" would never hold): **a contract of a C library function comes
   from a vlibc declaration, and every other declaration of it has no contract or the same one**. The emitted Viper of
   today's `abort`/`exit` tests stays byte for byte the same or each difference is explained.
   - **`noreturn` from vlibc, not the SDK** (audit round 3): the encoder today accepts a `noreturn` attribute from a
     system declaration (`encode.cpp`, the noreturn trust); once the table moves, "this call does not return" for a
     C library function must come from vlibc's declaration (its `ensures(false)` or `[[noreturn]]`), never from the
     SDK's `__dead2` alone.
   - **Reserved names** (audit round 3): when the shim is not on the path, a user may declare a C library function
     (7.1.4p2), and 0011 would trust its contract (`void free(void*)` with `requires(true)` would switch the double-free
     check off). The names vlibc covers are reserved: their contract must come from vlibc, and a contract on another
     declaration of such a name is rejected (`annotation_reserved_library`, message naming vlibc's header).
5. **The pin.** `third_party/vlibc` is a git submodule pinned to a vlibc commit, recorded in 0001 and `bootstrap.sh`
   like the EDG fork (0009). The session that moves the pin runs `check.sh`. vlibc's CI compiles every header with
   clang and GCC in C23 and C++20 (`-Wall -Wextra -Werror`), and, once it has implementations, verifies them with the
   cerastes commit it names. Trusted contracts now arrive through the pin, where `check.sh` cannot see a weakened one:
   every trusted vlibc contract has a must-fail test in cerastes (a caller that violates it fails), and each pin move
   is audited (the soundness-auditor reads the contract diff) (audit round 3). `_FORTIFY_SOURCE` forms of the library
   functions (`__builtin___memcpy_chk` and the like) bypass the shim: the shim `#undef`s the macros that select them, and
   the `__builtin_` functions stay rejected.
6. **What vlibc never holds:** contracts for functions cerastes rejects for other reasons (variadic functions such as
   `printf`, decision 0012; floating point; locales), until the milestone that lifts the rejection.

## Consequences

- The encoder loses its only name-keyed trust table; trust comes from where a declaration is, as for system files.
- A user's ordinary build and cerastes read the same contracts when the user writes `<vlibc/…>`.
- M4.5d1 is the first session that needs the repository; it can run any time after M4.5a.
