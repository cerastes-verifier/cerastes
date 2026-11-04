# 0046 — vlibc in cerastes: the pin, the include order, the library class and trust (M4.5d1)

- **Status:** accepted for M4.5d1 (2026-11-03); written before code (invariant 6), revised after P71 (macOS; items 2–6)
  and audit round 1 (Opus, on the draft: two blocking items — B1 a trusted call redirected to other code by an alias,
  an asm label, `#pragma redefine_extname` or `#pragma weak`, reproduced in today's code and closed by item 8; B2 an
  identical contract after vlibc's header leaves no trace, question 116 — and S1–S9, N1–N12, taken in items 2–11 and
  the handoff; `build/m4.5d1/audit1.md`), and audit round 2 (on the code: two blocking items, both reproduced with a
  linked binary that returns — B1 vlibc's contract trusted on a C++-linkage `::exit` after a guard-skipped platform
  header, B2 item 8 skipped for a function the encoder created before its first call — and S1–S3, notes 1–14; all
  taken in items 3, 4, 6–9 and 11 or the handoff; `build/m4.5d1/audit2.md`). Owner decisions (M4.5d1 planning, 2026-11-03; plan
  `.claude/plans/m4-5d1-vlibc.md`; handoff questions 113–117, each the recommendation):
  - **113:** M4.5d1 alone, with M4.5d2's preparation (the allocation functions' platform declarations measured in P71,
    d2's dependency map and a d2a/d2b proposal in the handoff); d2 is not rolled in.
  - **114:** the reserved names are generated from the pinned vlibc headers: the names they declare with a contract
    (item 6), not a fixed list of every C23 library function.
  - **115:** a contract on a reserved name outside vlibc is rejected even when it repeats vlibc's (item 6; amends 0030
    item 4's "a user may redeclare a vlibc function only with the same contract") — with one exception, 116.
  - **116** (after audit round 1, B2): an identical contract on a redeclaration *after* vlibc's is accepted: the front end
    discards a later must-match list equivalent to the first (`edg/src/attribute.c:4199-4227`), so it leaves no trace
    and adds nothing.
  - **117** (after audit round 1, S8): defining a reserved name, and a contract on any declaration of one, is rejected
    for C language linkage in any namespace and, in C++, for any linkage in the global namespace (C23 7.1.3, C++
    `[extern.names]`/4); four pins of 0015 item 5 are re-expected (item 6).
  - **118** (after the first run of the new tests): the two alias pins `reject_redirect_alias.c` and
    `reject_redirect_alias_cpp.cpp`, written before the code, are re-expected `unsupported_attribute` (item 7 reports a
    user-written alias; item 8 then stays silent, one error per cause).
  - Earlier decisions this record carries out: 0030 (the repository, both ways in, trust by path, the pin), 0031 item 10
    (P55's glibc half, `reject_quick_exit_user_declared.c`'s glibc twin, question 59's near misses and the `sizeof`
    operand come with d1).
- **Milestone:** M4.5d1 (2026-11-03).
- **Implements:** 0030 items 3–5 (cerastes's half; vlibc's half is vlibc's `5962865`: layout, license, CI and
  `<vlibc/stdlib.h>` with `abort`, `exit` and `_Exit`).
- **Amends:** 0006 item 6 (question 116), 0011 items 4–6 (a third class of file: library files; trust needs the call to
  reach the declared function, item 8), 0011 item 13 (the `library_fixture` hook injects no fault, item 11), 0015 items
  4–5 (the table of trusted C library functions is gone; "does not return" for a C library function comes from vlibc;
  the four pins of question 117), 0030 items 3–5 (P55's `-I` reasoning corrected; reserved names generated; the pin
  audit as a check), 0031 item 9 (the macro exemption does not cover vlibc), 0042 item 4 (in C++ `exit` and `_Exit` are
  declared in vlibc, so a user redeclaration's markers are reserved-name rejections, item 6). Corrects P52 (an
  attribute's `position` is its use, `orig_seq` the macro's body; N1).

## Context

Record 0030 moved the C library's contracts out of the encoder into vlibc (`cerastes-verifier/vlibc`), a repository of
headers that ordinary compilers read with the contracts expanding to nothing. vlibc's half of M4.5d1 is done:
`include/vlibc/stdlib.h` is both `<vlibc/stdlib.h>` and the shim for a bare `<stdlib.h>`; it does `#include_next
<stdlib.h>` and `#include <cerastes.h>`, then redeclares

```c
// cerastes:trusted-contract vlibc: abort never returns (C23 7.24.4.1)
void abort(void) CERASTES_ENSURES(false);
```

and the same for `exit` (7.24.4.4) and `_Exit` (7.24.4.5). Until now the encoder trusted these three by a closed,
name-keyed table (`trusted_libc_reason`, 0015 item 5) keyed on the platform's first declaration and EDG's "does not
return" flag. This record decides how cerastes reads vlibc instead.

Three facts from this session's planning (the surveys behind the plan; P71 confirms them, `internal/il-inventory.md`):

- **Search order.** P55 concluded that only a leading `--sys_include` can shadow `<stdlib.h>`, since "`-I` is searched
  after the system directories". That was the driver's argument order: EDG keeps `--include_directory` and
  `--sys_include` in one list in command-line order (`edg/src/cmd_line.c:10840-10853`), and the driver appends the
  user's `-I` after every system directory (`verify_main.cpp:315-367`). But EDG flags as system every file that a system
  header includes (`edg/src/lexical.c:8159-8161`), and libc++'s `<cstdlib>`, `<array>` and `<exception>` include
  `<stdlib.h>`; a shim passed as `-I` would be a system file in some C++ units and not in others.
- **The C++ declaration position.** In C++ with clang emulation (`gpp_mode`), EDG moves a routine's `decl_position` to a
  non-defining redeclaration when the routine has a parameter, no body, and a first declaration in a system header
  (`edg/src/decls.c:9802-9813`). So in C++ `exit` and `_Exit` are "declared" in vlibc's header, while `abort(void)`, and
  every function in C, keep the platform's position. P55's attribute problem (the platform's `[[const]]` on `abs` held to
  the allow-list after a user redeclaration) was C++-only for this reason.
- **What reads EDG's system flag.** In cerastes every use goes through `SrcPos::in_system` (`render.cpp:130`) —
  `diagnostics.cpp:155`, `spec_marker_in_system_file` (`annotation_checks.cpp:1126-1136`), the initializer skip
  (`:1331`, `:1340`), the unattributed-body skip (`:1591`), `walker.cpp:718` — except `directive_in_system_file`
  (`annotation_checks.cpp:1043`, which in verify mode also requires the path rule) and `encode.cpp:9041` (which only tells
  a `#pragma GCC system_header` from a `#line` in a file that is not system by path).

## Decision

1. **The pin.** `third_party/vlibc` is a git submodule, URL `https://github.com/cerastes-verifier/vlibc.git`, pinned to
   `5962865`; record 0001 lists it beside the EDG fork and Silicon. `scripts/vlibc_tree.py check` is its guard (run by
   `check.sh` after the EDG tree): the submodule is clean, its `HEAD` is the gitlink, the pinned commit is on vlibc's
   `origin/main` or on a remote named `local` (an unpushed pin, reported `NOT YET PUSHED`, as the EDG fork's
   `fork-local`, 0009), and both URLs are vlibc's. `CERASTES_VLIBC_SRC` builds against another vlibc checkout, as
   `CERASTES_EDG_SRC` does (0005), and `check.sh` refuses it on `main`; `cerastes --version` names the vlibc commit
   (N10). A pin move runs `check.sh` and is audited (item 9).
2. **The include order** (0030 item 3 kept, its reason corrected by P71). In verify mode the driver passes, in this
   order: `--sys_include=<vlibc>/include/vlibc` (the shim), `--sys_include=<vlibc>/include` (`<vlibc/…>`), the platform's
   directories, `--sys_include=<cerastes>/include`, then the user's `-I`. EDG keeps `-I` and `--sys_include` in one
   ordered list; what shadows `<stdlib.h>` is a position before the platform's directories, and a `-I` shim would be
   flagged system by EDG only in units where a system header includes it first (P71 (e)), so `--sys_include` keeps the
   flag the same in every unit and cerastes decides by path (item 3). A bare `<stdlib.h>` — the user's, or a system
   header's (libc++'s `<cstdlib>`, `<array>`, `<exception>`) — reaches the shim, whose `#include_next` continues with
   libc++'s wrapper or the C library's header; `<vlibc/stdlib.h>` resolves to the pinned copy even when a user `-I`
   names another copy (under the old order it failed, #1696 at the shim's `#include_next`, P71). `-I-` (which makes
   every directory before it quote-only) and an `-I` value starting with `-` are refused (audit S6). Every unit that
   reaches the shim includes `<cerastes.h>` (N5). Walk mode (`scripts/cerastes-fe.sh`, the IL goldens) stays without
   vlibc: the shim is verify mode only (0030 item 3), and adding it there would add a "not reported" line per vlibc
   contract to every walker golden that reaches `<stdlib.h>`.
3. **The library class** (amends 0011 item 4). A file under the canonical `<vlibc>/include/vlibc` is a **library file**
   (the one directory `scripts/vlibc_contracts.py` reads; a subdirectory there, or a header elsewhere under
   `<vlibc>/include`, fails that script, audit S1). It is never a system file for cerastes (`VerifyConfig::library_dirs`;
   `system_directories` and `is_system_file` exclude it), so it is a verified file: a body in it (an `inline` helper, a
   `predicate` or `pure` function; none in vlibc yet) is verified like user code, and a call of such a function is
   checked against its contract like any other.
   - **One rule by path** (0030 item 4's audit, 0031 item 9; audit S4). In verify mode `SrcPos::in_system`, and
     `diag_position`'s macro rule (`diagnostics.cpp:155`, which read EDG's flag directly), mean "not a verified file" by
     path, not EDG's flag. So `spec_marker_in_system_file` accepts `predicate` and `pure` in vlibc, initializers and
     unattributed bodies there are checked (`annotation_checks.cpp:1332`, `:1341`, `:1591`), a diagnostic inside a vlibc
     contract points at the macro argument — and a user header that a system header includes (EDG flags it,
     `lexical.c:8159`) is checked by the annotation checks as the encoder already verifies it. `directive_in_system_file`
     already requires the path rule in verify mode, so 0031 item 9's exemption never covers vlibc (a predefined macro
     redefined there is `annotation_macro_changed`), and the `#line` pass (`encode.cpp:9026-9045`) rejects a `#line` or a
     linemarker in a library file (reported with the message of `#pragma GCC system_header`: in a library file both
     records carry EDG's flag). `#pragma GCC system_header` itself is a no-op there: the file is already a
     `--sys_include` file for EDG, so the pragma makes no record, and cerastes decides by path, so the code after it is
     verified as before (a fixture test pins that). Walk mode is unchanged.
   - **EDG's own relaxations in system files**, which a vlibc file keeps (EDG's flag is on): warnings and remarks are
     dropped (`edg/src/error.c:3621`), discretionary errors do not count as errors (`:3651`, `:3701`), `__STDC__` may be
     0 (`macro.c:6296`, off in this configuration), and rules keyed on a system file at `decls.c:2443`, `:2626`, `:4634`,
     `:9802`, `:12718`, `:19834`, `decl_spec.c:9920`, `:12001`, `expr.c:52952`, `lookup.c:3665`, `symbol_tbl.c:4921`,
     `attribute.c:3888`, `class_decl.c:34882`, `templates.c:25294`. Errors raised by `--diag_error` still fire
     (`error.c:5032`). Two checks close the residual: vlibc's CI compiles every header as an ordinary `-I` header with
     `-Wall -Wextra -Werror` under GCC 16 and Clang 21 in C23 and C++20 (0030 item 5), and `check.sh` compiles every
     vlibc header with `cerastes-fe` in the driver's configuration with the shim as a *non-system* directory ahead of
     the platform's, in both modes, requiring no diagnostic at all (audit S5; `scripts/check_vlibc_headers.sh`, which
     takes the driver's own EDG arguments from a hidden command, `cerastes --cerastes_print_edg_command <mode>`, so the
     two cannot drift apart). One EDG warning is suppressed there, in C++ only: #541 ("allowing all exceptions is
     incompatible with previous function"), which glibc's `__THROW` (`noexcept(true)`) declarations give vlibc's
     redeclarations without `noexcept` (P71, Linux). GCC 16 and Clang 21 accept them silently (vlibc's CI, `-Werror`),
     cerastes models no exceptions (a `noexcept` expression is rejected), and the exception specification EDG then keeps
     is item 10's M7 entry.
4. **Trust** (replaces 0015 item 5's table; amends 0011 items 5–6). A routine without a body is **trusted from vlibc**
   when all of these hold; it is fail closed otherwise:
   - every `requires`/`ensures` attribute of the routine is written (its `position`, the macro's use, N1) in a library
     file — the encoder never keys trust on `decl_position`, which C++ moves to vlibc's redeclaration of a function with
     a parameter, or to a later user redeclaration (`decls.c:9802-9813`, P71 (c)), and C does not;
   - its name is reserved (item 6), it has C language linkage, and it is not a class member or a template;
   - the tag vlibc writes on the physical line directly above the declaration that carries the contract (the secondary
     declaration whose `attributes` hold it; `\r` stripped), `// cerastes:trusted-contract vlibc: <name> <reason>`,
     names this routine;
   - no identifier in the contract's source text is a macro that a directive outside the system files and
     `<cerastes.h>` defines or undefines, or that `-D`/`-U` names (audit round 1 S1: `#define false 1` before
     `<stdlib.h>` would rewrite vlibc's contract; `#define NULL p` would empty M4.5d2's `free`). Library files count
     too (audit round 2 S1: a `#define false 1` inside vlibc's own header), and `vlibc_contracts.py` refuses in a shim
     header every `#define`/`#undef` but its include guard, every `#pragma` and `#line`, and a comment inside a
     trusted declaration, so such a header cannot be pinned. The scan reads only the contract's own identifiers: it
     does not follow a system macro (`NULL`), and any program macro of the same name is refused, wherever it is
     defined — strict, and to revisit when M4.5d2's contracts name parameters (audit round 2 note 5).

   A contract written in a library file on a routine that fails the first two conditions (a user's `#define abort sync`
   makes vlibc's line declare `sync`; a guard-skipped platform header leaves vlibc's declaration with C++ linkage) is
   rejected `annotation_reserved_library`; a missing or mismatched tag is an internal error (exit 3; vlibc's own
   `vlibc_contracts.py` check refuses such a header before it can be pinned); a user macro over the text is
   `annotation_macro_changed`. The reason is the tag's text, repeated by the emitted method (`// cerastes:trusted-contract
   vlibc: abort never returns (C23 7.24.4.1)`). `trusted_libc_reason` and its call in `callee_for_call` are removed. A
   system function vlibc does not cover keeps its rules (without a contract, `call_without_contract`; with a user's
   contract on a redeclaration, trusted with "declared in a system file", or "declared here, defined in another unit"
   when C++ moved its position to the user's file, N7) — `malloc` and `free` among them until M4.5d2 (N9).
5. **"Does not return" comes from vlibc** (0030 item 4; amends 0015 item 4). For a routine trusted from vlibc, EDG's
   `does_not_return` flag (the SDK's `__dead2`, glibc's `__attribute__((__noreturn__))`) is ignored: `f->noreturn` is
   false, neither `ensures_on_noreturn` check fires (`encode.cpp:1254`, `:8940`), and the method's `ensures false` is
   vlibc's own `ensures(false)` clause. Its location (the method's and its parameters') is the vlibc declaration that
   carries the contract, in both modes. The Viper of a program calling `abort`, `exit` or `_Exit` changes in three
   ways: the tag (`libc: …` becomes `vlibc: …`); in the `.map`, the position of those methods' rows (vlibc's line
   instead of the platform's declaration, P71 (f)); and one well-formedness method per called function
   (`f_abort__1__wf`, …), which every function with a contract has (0037) and which the old `noreturn` path, having no
   contract clause, did not emit. M4.5d2's contracts (`free`'s `requires`) need that method; for `ensures(false)` it
   holds trivially. Every other program's Viper is byte-identical: an uncalled routine first
   declared in a system file is never encoded (`encode.cpp:8869-8878`), and in C++ `exit` and `_Exit`, now declared in
   vlibc, reach the declaration checks but are encoded only when called (N2).
6. **Reserved names** (0030 item 4, as decided by questions 114–117). The names vlibc declares with a contract are
   **reserved**: `scripts/vlibc_contracts.py names` generates them at configure time from the pinned headers (in d1
   `abort`, `exit`, `_Exit`), so a pin move updates them. A routine is the library's when its name is reserved and it has
   C language linkage in any namespace (`namespace n { extern "C" void exit(int); }` is `::exit`, `[dcl.link]`/6; audit
   S2) or, in C++, it is declared in the global namespace whatever its linkage (`[extern.names]`/4, question 117). For
   such a routine, outside a library file:
   - a contract — `requires`, `ensures`, `decreases`, `owning`, `ref_to_uninit`, `must_init`, or a written `noreturn`
     attribute — is rejected `annotation_reserved_library`, with a message naming vlibc's header, whether or not the unit
     includes it (question 115), except an identical contract on a redeclaration after vlibc's, which the front end
     discards (question 116); a *different* one there is EDG's must-match error first (`attr_redeclaration_mismatch`);
   - a definition is rejected `annotation_reserved_library` (C23 7.1.3; question 117), with or without the header;
   - `_Noreturn`, which leaves no attribute entry (P21), is detected through the flag when no system or library
     declaration exists (the flag set and no platform declaration: rejected); beside the platform's declaration it
     cannot be told apart and is harmless, since the flag is ignored (item 5) (audit S9).

   A function in a namespace with C++ linkage (`n::exit`) is not the library's and stays an ordinary function
   (`noreturn_namespaced_exit.cpp`). The four pins of 0015 item 5 that accepted a C++-linkage global look-alike
   (`noreturn_user_abort.cpp`, `noreturn_other_signature.cpp` and their `_after` twins) are re-expected
   `rejected cerastes-annotation_reserved_library` (question 117). A plain declaration with no contract stays allowed
   (C 7.1.4p2), and a call of it without the header is `call_without_contract` as before. A `static` function named
   `exit` in C is not the library's (internal linkage; C23 7.1.3 allows it when the header is not included, and with
   the header EDG refuses the `static` redeclaration itself) and is an ordinary function (audit round 2 note 2).
7. **Attributes by their own position** (P55; audit S3). For every routine — declared here or not, called or not —
   every attribute entry of its merged list, of each secondary declaration and of its parameters is held to the
   allow-list unless it is written in a system file (as the type path already does, `encode.cpp:8793`). The filter is
   in `check_attributes` itself, so it applies the same way to the attributes of variables, fields and statements
   (audit round 2 note 10; none of those can come from a system file in verified code). Entries written
   in a library or user file are checked; the platform's own (`__cold__`, `const`, `alloc_size`, `nothrow`, `leaf`,
   `__noreturn__`) are not. Sound because cerastes uses only a routine's contract and its type, a call reaches the
   declared function (item 8), and a platform attribute is the platform's claim about its own function. It requires of
   vlibc that every contract is consistent with every attribute the platform's declaration gives the compiler to
   transform callers with (`const`/`pure`, `malloc`, `alloc_size`, `nonnull`, `noescape`): the pin audit (item 9) checks
   that from M4.5d2 on.
8. **A trusted contract needs the call to reach its function** (audit round 1, B1; amends 0011 items 5–6). A routine
   without a body whose contract is trusted, for any reason (vlibc's, or a user's on a function defined elsewhere), is
   rejected `unsupported_declaration` when its calls may reach other code: an asm label (`asm_name`, also set by
   `#pragma redefine_extname`), a GNU alias (`is_gnu_alias`; not EDG's `implicit_alias`), an `ifunc`, or `weak`/`weakref`
   (also from `#pragma weak`). The check is made at every call, also of a function the encoder created before its
   first call (one with a structure parameter or result, or a marker; audit round 2 B2). A redirection a
   program-written attribute shows (`alias`, `ifunc`, `weak`, `weakref`) is already `unsupported_attribute` by item 7
   and is not reported twice (question 118). A routine with a body in a library file is checked for `weak` like user
   code (audit round 2 S2). Before this record these were accepted, and a call of `exit` redirected to a function
   that returns was trusted not to return (reproduced in C23 and C++20). Fail closed: a platform declaration with an asm
   label (glibc's C23 `strtol` → `__isoc23_strtol`) makes that function untrustable until a later record tells the
   platform's labels from a user's; P71 checks that `abort`, `exit` and `_Exit` carry none on either platform.
9. **The pin audit as a check** (0030 item 5). `scripts/vlibc_contracts.py check` lists every trusted vlibc declaration
   (name, header, declaration with its contract, tag) and compares it with `tests/vlibc/trusted_contracts.txt`;
   `check.sh` runs it. A pin move that changes, adds or removes a trusted contract fails until the list is updated, and
   updating it is a golden change under invariant 1 that the soundness-auditor reads. Every trusted contract has a
   must-verify test that relies on it and a must-fail twin (for `ensures(false)`: the call removed fails the
   postcondition; from M4.5d2 on, twins that violate the `requires`, N8).
10. **Listed, not changed.** `_FORTIFY_SOURCE`'s `#undef`s (0030 item 5) come with `<string.h>` (M4.6); markers on
    vlibc declarations, and `reject_markers_on_system_redeclaration`, are M4.5d2's (`malloc`'s `ref_to_uninit` result);
    `quick_exit`, the `__builtin_` functions and `<malloc/malloc.h>` stay as they are; vlibc must never declare
    `__assert_rtn` or `__assert_fail` (the `assert` recognizer requires their first declaration outside verified files,
    `encode.cpp:5962`); in C++ on glibc, `exit` and `_Exit` lose glibc's `noexcept` after vlibc's redeclaration
    (`decls.c:9796-9807`; nothing accepted today observes it, `noexcept(…)` being rejected: an M7 entry, N3); vlibc's
    include guard wraps its `#include_next`, unlike libc++'s wrappers (no effect on today's SDK or glibc headers; a note
    for vlibc, N4).
11. **Testing the library class** (amends 0011 item 13). A self-test hook, `--cerastes_verify_selftest=library_fixture`,
    adds one fixed directory, `<cerastes>/scripts/testdata/library_fixture/include` (compiled in like
    `<cerastes>/include`), as a second library directory and a leading `--sys_include`; no other directory can be given
    library status. Unlike the other hooks it injects no fault, so a run under it may end `verified`; its success line
    names the hook (audit S7). The harness's `LibraryFixtureTests` use it for what vlibc does not hold yet: a `predicate`
    and a `pure` function in a library header, an `inline` body verified there with its must-fail twin, a trusted
    contract with its tag in the Viper, a contract without a tag or with another name's tag (exit 3), a library contract
    on a name vlibc does not reserve (rejected), a predefined macro redefined (`annotation_macro_changed`), a library body
    made `weak` (rejected), a `#line` (rejected) and a `#pragma GCC system_header` (no effect: the code after it is still verified); and they check that no fixture header is named like a standard header.
12. **Diagnostics and categories.** One new rejection tag, `annotation_reserved_library`. No new error category.
13. **Facts relied on** (`internal/il-inventory.md`): (P55) the macOS shim mechanics, its `-I` reasoning corrected here;
    (P71) the include routes and EDG's flag per route, the redeclarations accepted after the SDK's (and glibc's: the
    Linux half) declarations in both modes, `decl_position` per mode, each attribute's position, language linkage
    (`source_corresp.name_linkage`), the secondary declarations' own `attributes`, and the allocation functions'
    platform declarations (for M4.5d2); P52 corrected (N1).
14. **Tests and documentation.** Every existing `expect:` line is kept except the four of question 117. New tests in
    `tests/vlibc/` (both modes where valid; each positive with a twin failing for one reason at one line): the explicit
    `<vlibc/stdlib.h>`, `<cstdlib>`'s `std::_Exit` and `std::abort`, a C++ unit reaching the shim only through `<array>`,
    the three on Linux, an identical contract after the header (question 116), a user declaration with no contract
    (`call_without_contract`), a user redeclaration of `abs` with a contract in C++ (item 7), the reserved-name
    rejections (C linkage, a namespace's `extern "C"`, C++'s global namespace, a definition with and without the header),
    the redirections of item 8 in C and C++ (alias, asm label before and after the header, `#pragma redefine_extname`,
    `#pragma weak`, a user's trusted contract bound to another symbol), `#define false 1` before `<stdlib.h>`,
    `#define VLIBC_STDLIB_H` (the platform's flag alone is never trusted: `call_without_contract`), `-I-` refused, and the
    Linux items of 0031 item 10, and from audit round 2 a C++ guard-skipped platform header (`annotation_reserved_library`),
    an asm label on a trusted function with a structure parameter (`unsupported_declaration`) and a `static exit` in C
    (an ordinary function). Docs: "Verifying a program" (a section on vlibc; library files beside system files),
    "Annotations" (reserved names), "Building annotated code with ordinary compilers" (vlibc's include path and the shim
    opt-in), "Installation" (the submodule); `internal/encoding.md`.

## Consequences

- The encoder has no name-keyed trust table any more: trust comes from where a contract is written (a library file) and
  from vlibc's tag, and a weakened trusted contract cannot arrive through a pin move unseen (item 8).
- A C library function's contract has one source. A user can no longer give `exit` (or, from M4.5d2, `free`) a contract
  of their own, with or without including the header.
- vlibc headers are checked like user code by cerastes and by vlibc's CI; EDG's own system-header relaxations remain for
  them, listed in item 3.
- The emitted Viper of programs that call `abort`, `exit` or `_Exit` differs from M4.5c2b's in the tag text, the `.map`
  positions of those methods and one added well-formedness method per called function; every other program's Viper is
  unchanged.
- A trusted contract's calls must reach the declared function: aliases, asm labels and weak declarations are refused
  beside every trusted contract, closing a hole that predates vlibc (item 8).
- A user's own `abort` in the global namespace, or a definition of any reserved name, is refused (question 117).
