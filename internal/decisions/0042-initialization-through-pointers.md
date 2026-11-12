# 0042 — Initialization through pointers: P4222's markers and `--strict-init` (M4.5b2b)

- **Status:** accepted for M4.5b2b (2026-11-02); written before code (invariant 6), revised after P64 (items 4 and 8) and
  audit round 1 (Opus, on this record: no blocking item; S1 the loops' `flag ==> range`, item 5; S2 what strict mode
  accepts, item 8; S3 the one failing global flag row, item 7; S4 tests, item 12; S5 `pure`/`predicate` functions and
  system redeclarations, item 4; S6 row placement and guarded atoms, item 5; S7 the amends list; notes N1–N8 taken in
  items 5, 8, 10 and the handoff; `build/m4.5b2b/audit1.md`), S32 (the fixtures `s32_*`), and audit round 2 (on the code:
  no unsound verdict; S1 the loop row untested and S2 its gate, decided by question 89, item 5; S3 pins of `owning`'s new
  rejections; S4 uncalled marked declarations, item 4; notes taken in item 12 and the handoff; `build/m4.5b2b/audit2.md`). Owner decisions (M4.5b2b planning, 2026-11-02; plan
  `.claude/plans/m4-5b2b-init-markers.md`; handoff questions 85–88, each the recommendation):
  - **85:** every call's flag row is `uninitialized`, global and `static`-local atoms included (item 7), not only the
    rows of atoms on a pointer argument.
  - **86:** a contract that reads a marked parameter's pointee where its flag is not promised is rejected statically
    (item 6), not checked by a path-precise row (which would need flags in `f__wf`).
  - **87:** after a call to a `ref_to_uninit` parameter, a pointee that was initialized stays initialized (item 5:
    `old(flag) ==> flag`), not "no flag value" as 0028 item 14 said.
  - **88:** a marker needs an `owns(*p)` atom on its parameter (item 4), rather than implying one as `owning` does.
  - **89** (during the session, after audit round 2): the loop row `held && flag ==> range` goes on every place a loop
    does not reach, in every function (item 5), rather than only in functions with markers; it changes the Viper of
    existing programs with such loops (an added invariant line, no verdict) and fixes 0038's exit 3 on nested loops.
  - Earlier answers this record carries out: **82** (the call's flag row becomes `uninitialized`, naming the parameter),
    **83** (strict mode's argument rule is a static rejection, `uninit_ref_initialized`).
- **Milestone:** M4.5b2b (2026-11-02).
- **Implements:** 0029 item 2 (P4222's markers, their checks and the strict mode), 0019 item 2's stricter mode, 0028 item
  14 for parameters.
- **Amends:** 0019 items 2–3 (the strict mode is `--strict-init`), 0028 item 14 (question 87), 0029 item 2 (targets,
  categories and messages fixed; what strict mode rejects, item 8), 0037 items 7, 8 and 10 (framing of contract reads, the
  flag rows at calls and returns), 0038 item 3 (a range row for every loop place in a function with markers, item 5),
  0039 item 8 (measures read through framing), 0040 item 3 (`annotation_mismatch` and "cannot tell apart" name the
  attribute; `owning` is a protected macro name), 0041 items 8 and 12 (the flag rows of global and `static`-local atoms at
  calls). S32 supersedes the fixture `tests/viper-smoke/memory/s25_d_init.vpr`, whose `maybe_init` encodes the "permission,
  not value" `ensures` (0028 item 18).

## Context

Record 0019 checks every read of a local without an initializer on each feasible path, with a ghost flag, and needs no
marker; its item 2 left "a stricter mode that also requires P4222's markers" to a later record. Record 0028 item 14 moved
the flag into each cell (`d_<t>`, beside `v_<t>`) and fixed its meaning in contracts: `owns(*p)` includes the flag (a
pointer points to something initialized, P4222's default) in the `requires` and the `ensures`. Record 0029 item 2 chose
P4222's three markers in the `cerastes::` spelling — `uninit` on a local, `ref_to_uninit` on a pointer whose pointee may
be uninitialized, `must_init` on a parameter whose pointee the callee initializes — and an opt-in strict mode, and P54
found where EDG keeps them. Nothing is registered yet: `check_attributes` (`encode.cpp:511`) skips every registered
`cerastes` attribute without a word, so each is registered in the commit that handles it (0029 item 1).

Today a call to a function whose contract holds `owns(*p)` or `reads(*p)` checks the argument's flag in a row of
category `permission` (`call_atoms`, `lower.cpp:2286`; 0037 item 8), so passing `&x` of an uninitialized `x` is reported
as a missing permission, and no function can receive an uninitialized object to fill. Facts: P54 (M4.5a), P64 (this
session's re-run on the real registration) and S32, in `internal/il-inventory.md`.

## Decision

1. **Scope.**

   | Construct | Session |
   |---|---|
   | `[[cerastes::uninit]]` on automatic locals; `ref_to_uninit`, `must_init` on pointer parameters; the call's flag row; `--strict-init` | M4.5b2b (this record) |
   | `ref_to_uninit` on pointer locals and functions (targets `v`, `r`: `malloc`'s result and a local receiving it) | M4.5d2 |
   | `const` objects as cells | M4.6 |
   | P4222 on references and members | M7 |
   | arrays and structs as marked locals (rejected as locals today) | M4.5e1, M4.5c |

2. **Registration and macros.**
   - `{"uninit", "", "v:+a!", RAF_NO_FLAGS}`, `{"ref_to_uninit", "", "p", RAF_NO_FLAGS}`, `{"must_init", "", "p",
     RAF_NO_FLAGS}` (`register_attributes.cpp`), each in the commit that handles it. `RAF_ENTITY_ATTACHED` with target `p`
     alone aborts EDG at startup (0040 item 3), so the parameter markers take no flags, as `owning` does. EDG itself then
     refuses `uninit` on a global or a `static` local (#1862, "requires an automatic variable") and on a parameter, a
     field or a function, and `ref_to_uninit`/`must_init` on anything but a parameter (`wrong_entity_for_attribute`;
     P54, P64). Each refusal is pinned by a test; M4.5d2 re-expects the `ref_to_uninit` ones when it adds `v` and `r`.
   - `<cerastes.h>` defines `CERASTES_UNINIT`, `CERASTES_REF_TO_UNINIT` and `CERASTES_MUST_INIT` (empty for other
     compilers, like `CERASTES_OWNING`).
   - `owning`, `uninit`, `ref_to_uninit` and `must_init` join the protected macro names (`is_protected_macro_name`,
     0008): a user `#define must_init …` would rewrite the attribute's token, so it is `annotation_macro_changed`. For
     `owning` this closes a gap left by 0040.
3. **`[[cerastes::uninit]]` on a local.**
   - **Accepted** on the definition of an automatic local of an accepted type (an integer, `bool`, an enumeration, a
     pointer at any depth) with no initializer, in either position, in a `for` loop's init-statement too. A leading
     marker covers every declarator of its declaration.
   - **Meaning:** it states that the object starts uninitialized. 0019 already proves on every path that it is not read
     before it is given a value, so the marker changes nothing in VIR or in the emitted Viper: a program verifies or
     fails exactly as without it (a test compares the two `.vpr` files).
   - **Rejected:** on a definition with an initializer (`uninit_initialized`; C++'s `int x{}` and `int x = {}` are
     initializers, and a leading marker over `int a, b = 1;` reaches `b`); twice on one declaration
     (`unsupported_annotation`). A `const` local without an initializer stays rejected (0019 item 1).
4. **The parameter markers.** `ref_to_uninit` on a pointer parameter `p`: the callee may receive `*p` uninitialized and
   promises nothing about it. `must_init`: the same on entry, and `*p` is initialized when the function returns (P4222:
   `must_init` implies `ref_to_uninit`). VIR carries them as `Param::init` (`None`, `RefToUninit`, `MustInit`); lowering
   reads the marker of the parameter an atom's site names, so global and implicit atoms are never marked.
   - **Reading** (as 0040 item 3 reads `owning`): the union over the function's declarations (P54: attributes accumulate,
     no must-match), the declarations told apart by position, a typedef'd function type's attribute counted for the
     declarations it types. A declaration that differs is `annotation_mismatch`, naming the attribute and both
     declarations; two declarations cerastes cannot tell apart (one macro invocation) with a marker are rejected
     (`unsupported_annotation`). Both messages now name the attribute; `owning`'s text is unchanged.
   - **Needs an atom** (question 88): the parameter's `requires` holds an `owns(*p)` atom (top-level, guarded, or the
     clause `cerastes::owning` stands for, which carries the marker when merged or desugared); with `must_init`, its
     `ensures` holds one too. Ownership and initialization stay orthogonal: without `owns` the callee could not write
     `*p`, so the marker would mean nothing.
   - **Rejected** (`unsupported_annotation`, each with its own message): on a non-pointer, a pointer to a function, `void
     *`, a pointer to a `const` object (nothing could initialize it); the marker twice; both markers on one parameter
     (redundant: `must_init` implies `ref_to_uninit`; P54 records both); no `owns(*p)` in the `requires`; `must_init`
     without `owns(*p)` in the `ensures`; `reads(*p)` beside a marker (a borrowed read of an object that may hold no
     value); `must_init` on a function declared not to return (it has no `ensures`, 0015 item 4; its own message, like
     `ensures_on_noreturn`); the contract reads of item 6. When `finish_atoms` or `owning` already rejected that atom or
     parameter, the marker's "needs an atom" error is not reported again. `check_program` repeats these rules as a
     backstop.
   - A marker on `T **pp` covers the pointer cell `*pp` (0040 item 2), its range being `WF`. `int *const p` and a leading
     marker on an array parameter `int p[]` (a pointer parameter) are marked parameters; a trailing marker after `p[]` is
     EDG's #1835 (P64).
   - Shapes registration accepts that stay rejected for their own reasons (P64 A5): a C++ reference parameter
     (`unsupported_type`, and the marker "not a pointer"), a member function, a template, a variadic function
     (`unsupported_declaration`); a marker inside a function-pointer type's own parameter list reaches no `vir::Param`
     (the pointer to a function is `unsupported_type`, M4.7; an unused typedef carrying one is inert).
   - A marker is not a contract by itself (`callee_for_call` unchanged): it needs an `owns` atom, which is.
   - **Checked when nothing calls it** (audit round 2, S4): the encoder reads a function's contract lazily, at its first
     call or definition, so a marked declaration that nothing calls is checked on its own (the rules above), as a
     marked parameter must mean something wherever it is written. A marker in a function-pointer typedef reaches no
     function before M4.7 (a handoff note).
   - **Never ignored** (audit round 1, S5): a marker on a parameter of a `pure` or `predicate` function, `uninit` on a local
     of such a body (both skipped by the encoder before `check_signature`, `encode.cpp:6573`), and a marker on a user
     redeclaration of a function first declared in a system header (`ours`, `:558`) are rejected
     (`unsupported_annotation`), as 0041 item 12 rejects `static` locals in specification bodies. M4.5f1 decides markers in
     specification functions when it encodes their bodies. The same two rules now cover `cerastes::owning`, which had the
     same gap since 0040 (amends 0040 item 3; no existing test held such a shape). In C++ a user redeclaration of a system
     function already reaches the signature reader (P55's path), so a marker there gets that path's errors instead. M4.5d1
     revisits the system-redeclaration rule when vlibc's headers carry markers (a handoff note).
5. **In the callee** (amends 0028 item 14, 0037 item 10).
   - **Entry.** The `requires` atom of a marked parameter holds the value and flag permissions and the liveness
     fraction, but not the flag; the value's range (or `WF`) is stated only under the flag, `p.d_T ==> range`: an
     uninitialized cell's value is unconstrained (`alloc_<t>` states no range), and a write states it (0012 item 8). A
     `bool` cell has no range row.
   - **The body** is unchanged: a read checks the flag (`uninitialized`, path-precise, 0019 item 3), a write sets it.
     Loops (question 89; amends 0038 item 3): the automatic places snapshot each held cell's flag and keep `ds ==> flag`,
     and a place the loop reaches keeps `held && flag ==> range`; a place it did not reach kept only its value, so after
     the loop a cell whose flag was false on entry could hold `d` with an out-of-range value. That made the new rows
     `p.d_T ==> range` (item 7's call row, the return rows below) fail on correct programs (audit round 1, S1), and it
     already made 0038's loops exit 3 on a correct program without markers: an outer loop that writes `x` and an inner
     loop that only reads `*q` lose the outer loop's row across the inner one (audit round 2, S2; the baseline too). So
     **every value place of every loop**, reached or not, gets the invariant `held && flag ==> range` (inductive: a write
     is range-checked, and an uninitialized cell satisfies it vacuously; a `bool` cell has no range row). Existing
     programs with such loops gain that invariant line and nothing else. A written `owns(*p)` in an invariant still
     includes the flag (item 10).
   - **Returns** (each `ensures` atom of the parameter, under its guard; the other rows of 0037 item 10 unchanged):
     - `must_init`: the flag row has category **`postcondition`**: "returns without initializing `*p`, which
       `cerastes::must_init` on `p` promises". The range row becomes `p.d_T ==> range`, so a missing write is reported
       once; the method's `ensures` states the flag and the range.
     - `ref_to_uninit` (question 87): the flag row becomes `(g_r && old(p.d_T)) ==> p.d_T`, category `internal`, where
       `g_r` is the guard of the `requires` partner (`old(p.d_T)` has permission only where it holds; `f__wf` reports an
       `ensures` guard that does not imply it, 0039 item 7, and the row stays well-defined there). The range row becomes
       `p.d_T ==> range`.
     - The method's own `ensures` states the same facts.
     - **Placement** (audit round 1, S6): each new row takes the place of the row it replaces, after the flag-permission
       row (`call_atoms`, `atom_returns`), so `old(p.d_T)` and `p.d_T` are read with permission. `must_init`'s row carries
       the note "this is the postcondition" at the `ensures` atom, as the other return rows do.
     - **A guarded `ensures` atom** (`c ? owns(*p) : live(p)`, `p == NULL || owns(*p)`): `must_init` promises the pointee
       initialized only where that atom is returned, under its guard.
   - **Why `old(flag) ==> flag` holds** (the `internal` row): nothing clears the flag of a cell a function holds. A
     write sets it; `lint_flags` admits no other assignment to a cell's flag (the allocation gives `false` to a new
     cell); the loop snapshots keep it; a call returns it through an `ensures` atom of the same kind, `true` for an
     unmarked parameter, kept for `ref_to_uninit`, `true` for `must_init`. In C an object becomes indeterminate again
     only by copying an indeterminate value into it, which needs a read that 0019 reports. M4.5c's struct copies must
     keep this (a handoff note). A pointer cell's value also becomes indeterminate when its pointee's lifetime ends (C23
     6.2.4p2): that is liveness (0028 item 3), checked at every use of the pointer, not the flag (audit round 1, N3).
   - **Trust:** for a function without a body, `old(flag) ==> flag` is a promise added to its `ensures`, trusted as the rest
     of a trusted contract is; C++ lets such a function store an indeterminate `unsigned char`, which would break it. The
     Verifying page lists it with the trust assumptions (audit round 1, N2).
   - **For M4.5d2** (audit round 1, N1): Silicon goes on past a failed assertion with it assumed (it stops a path only when
     the assumption makes the path infeasible), so the `internal` row could fail after a real `permission` error on the
     same path, but only where another chunk may hold the same place, which nothing creates today; once a cell can be
     reached through a fresh heap chunk (`malloc` after `keep(p)`), M4.5d2 re-checks it (a handoff note).
6. **Contract reads of a marked pointee** (question 86). Where its flag is not promised, the pointee has no value to
   read: in the `requires` of either marker and in `old(…)` and `decreases`, and in the `ensures` of `ref_to_uninit`.
   `f__wf` has no flags, so it cannot check such a read on a path.
   - **The spelling** `*p` there — `*p` in a `requires`, `old(*p)` (`cerastes_old(*p)` in C), `decreases(*p)`, `*p` in a
     `ref_to_uninit` parameter's `ensures` — is rejected (`unsupported_annotation`): "the contract reads `*p`, which
     `cerastes::ref_to_uninit` says may hold no value on entry; …". `*p` in a `must_init` parameter's `ensures` is
     accepted: the flag is promised there.
   - **Reads through an alias.** 0037 item 7 frames a contract read of a place by any atom of its field, so `requires(q
     == p) requires(*q > 0)`, `ensures(r == p) ensures(*r == 1)`, `old(*q)` and a global `g` with `p == &g` read a marked
     pointee without the spelling `*p`. A marked atom's site never frames a read: for pre-state reads always, for
     post-state reads when the parameter is `ref_to_uninit` without `must_init`. Such a read is then reported by
     `f__wf`'s framing check (`permission`, on its path), and the guarded read stays well-formed; as for any unframed
     read in an `ensures` (0037 item 7), the guarded clause may then also fail as `postcondition` at the return
     (`ensures(*q == 1)` gives both; the tests use `*q == *q`). `*cerastes_old(p)` and `*cerastes::old(p)` never reach
     this rule: a dereference of `old` is rejected in a contract before it (in C as a read through a heap read, M4.5c; in
     C++ as a dereference of something other than a parameter or the result).
7. **At the caller** (questions 82, 85; amends 0037 item 8).
   - **A marked parameter's atom:** no flag row, and its range row becomes `p.d_T ==> range`.
   - **Every other cell atom** (an unmarked parameter's, a global's, a `static` local's implicit atom): its range row stays
     `permission`: wherever the flag holds the value is in range (item 5's loop row included), so after a failed flag row,
     which Silicon assumes and goes on from, the range row holds and each argument gives one error (audit round 2, S2:
     before question 89 a loop could make it a second error), and the flag row is
     category **`uninitialized`**, with a message from the site: for a parameter, "passes `&x` to `f`, whose parameter
     `p` needs `*p` to have a value; it may not have one here" (the argument as written); for a global or `static`
     local, "calls `f`, which needs `g` to have a value". A global's flag is set by `program_start` and kept by writes, so the
     row fails in one corner only (audit round 1, S3): a `ref_to_uninit` function with `requires(p == &g)` holds `g`'s cell
     with an unknown flag, and a call there to `h()`, which needs `owns(g)`, is reported "calls `h`, which needs `g` to
     have a value" (the must-fail test of this message).
   - **Hidden by order** (audit round 1, N5, listed): `g(&x, &x)` with `x` uninitialized reports `uninitialized`, not the
     separation error that 0037 item 8 checks later at the same call.
   - **After the call** the `ensures` is inhaled as Silicon does today: after `ref_to_uninit`, an object initialized
     before the call is still initialized (item 5) and one that was not may now be; after `must_init`, it is
     initialized.
   - A program without a marker keeps its `.vpr` apart from item 5's loop row; its `.map` changes in the category and
     message of each call flag row (`emit.cpp`, `posmap.cpp`: a row's category lives only in the map). Measured on the
     baseline's 2878 kept programs: 286 maps differ in 586 call flag rows, 92 programs gain 130 loop rows, nothing else
     changes (`build/m4.5b2b/vprdiff-regen2.txt`).
8. **`--strict-init`** (0019 item 2, 0029 item 2; a driver option through `VerifyConfig`; documented with the options).
   - **Unmarked locals:** a definition of an automatic local without an initializer and without `uninit` is rejected
     (`uninit_unmarked`), naming the variable and the marker to write. A `static` local is not asked (EDG refuses
     `uninit` there; 0041 gives it a value).
   - **Initialized objects passed to a marked parameter** (question 83, P4222 §4.2): every argument of a marked
     parameter must be `&x` of an `uninit` local, or the caller's own marked parameter passed as its name when that
     parameter is never assigned and never address-taken in the body; anything else is rejected
     (`uninit_ref_initialized`). The rule reads the VIR argument (an `AddrOf` of a marked local, or the parameter's
     copy), so C and C++ look alike (P64), and "never assigned" is settled after the body with lowering's own scan of
     assigned parameters (moved to `vir.cpp`, so both agree).
   - **The scan** counts every assignment of the parameter's copy, inside a store's address and value too (`*(p = q) =
     1`), and an `&*q` argument: C23 folds it to `q`, which is the parameter passed on; C++ rejects `&*p` in code
     everywhere since 0032 (`unsupported_expression`), so the two modes differ on `&*q` itself, not on this rule (P64).
   - **What the static rule cannot see** (audit round 1, S2; amends 0029 item 2, which promised to reject "a pointer to an
     initialized object"): the marker is checked, not the flag, so these are **accepted although the object is
     initialized** — `CERASTES_UNINIT int x; x = 1; init(&x);`, a second `init(&x)`, and a marked parameter passed on
     after `*p = v`. Rejected although harmless (**over-strict**): a null constant, a pointer local holding `&x`, a marked
     parameter that is reassigned, and `init(c ? &x : &y)`. Making the rule path-precise would need the flag's value at
     the argument; the default mode's checks are path-precise. Both lists are on the Verifying page.
   - **Where it applies:** the verified bodies. `pure` and `predicate` bodies are not verified before M4.5f1 (0011 item 4),
     and a marker there is rejected (item 4), so strict mode asks nothing of them yet.
   - Without the option every program means the same; the option only rejects more.
9. **Categories:** none new. `uninitialized` gains the argument and global messages (item 7), `postcondition` gains
   `must_init`'s (item 5). New rejection tags: `uninit_initialized`, `uninit_unmarked`, `uninit_ref_initialized`.
10. **Listed, not changed:** a written `owns(*p)` in a loop invariant includes the flag, so `invariant(owns(*p))` before
    the first write is "`permission`: enters this loop without owning `*p`"; the flag rows of written invariant atoms
    (`written_checks`) and of an unmarked parameter's `ensures` atom stay `permission` (the latter cannot fail: the flag
    was required on entry and nothing clears it); `ref_to_uninit` on locals and functions is M4.5d2's. A completeness cost
    (audit round 1, N7; as 0038 says for locals): a `must_init` function that writes `*p` only inside a loop that always
    runs fails `postcondition`, since the loop keeps only `ds ==> flag`; a write before the loop is the workaround (a
    written `owns(*p)` in the invariant needs the flag at the loop's entry).
11. **Facts relied on** (`internal/il-inventory.md`): (P54) where the markers land and what EDG refuses; (P64) the same on
    the registration this record uses, both modes, and the IL of the strict rule's arguments; (S32) Silicon on the new
    shapes: the flag-free `requires` and `flag ==> range`, `old(flag) ==> flag`, `must_init`'s row, a caller passing a
    fresh cell, a loop, and the vacuity run (`tests/viper-smoke/memory/s32_*`).
12. **Tests and documentation.** Must-verify and must-fail pairs for every accepted form in both modes where valid
    (`tests/memory/`), a rejection pin for each rejected form, the strict mode through the `// verify-options:` header
    (`tests/README.md`); VIR self-test `selftest_cases13.inc`; the six re-expected pins of questions 82 and 85
    (`selftest_cases8.inc:73`, `selftest_cases11.inc:137-138`, `selftest_cases12.inc:113, :206, :283`), pre-approved,
    and no other existing expectation changes. Audit round 1's S4 adds: the value not kept after `ref_to_uninit`
    (`int x = 1; maybe(&x); check(x == 1);` fails); every alias branch of item 6 (`old(*q)` with `q == p`, `g` and
    `old(g)` with `p == &g`, `decreases(*q)`); both `old` spellings in both modes; markers through a typedef'd function type and "cannot tell apart"; EDG's type-position refusals (`int
    *[[cerastes::uninit]] x`, `int *[[cerastes::must_init]] p`); the global row of item 7. Expected mutants for M4.8's run
    (N8): a dropped marker, `must_init` and `ref_to_uninit` swapped. Docs: Memory and ownership ("Initialization through pointers"),
    Annotations (the markers, "Declarations that disagree", the protected names), Error messages (`uninitialized`,
    `postcondition`), Verifying a program (`--strict-init`, "Locals without an initializer").

## Consequences

- A function can receive an object to initialize (`init(&x)` with `int x;`), the shape of every output parameter, and
  `malloc`'s uninitialized result (M4.5d2) has the marker it needs.
- An uninitialized argument to an unmarked parameter now reads as `uninitialized`, not as a missing permission.
- `--strict-init` gives P4222's profile in its static form; the default stays 0019's path-precise check.
- Existing programs change only in the call flag rows of their `.map` and, where a loop holds a place it does not
  write, one invariant line per such place (question 89); no verdict changes.

*2026-11-11, amended by 0059 (M4.5e1a): items 3 and 5 — `cerastes::uninit` applies to an array local without an initializer, and `--strict-init` asks for it (0059 item 11); a loop's flag and range rows hold over every element of every array it carries (0059 item 8).*

*2026-11-12, amended by 0060 (M4.5e1b): items 4–8 — a marker's atom may be `owns_range(p, n)`: `ref_to_uninit` drops the flags on entry and keeps `old(flag) ==> flag` per element, `must_init` promises every element's flag (`postcondition`), a marked range has no flags row at the call, and `--strict-init` reads an `uninit` array argument as `&x` (0060 item 12).*
