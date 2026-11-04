# 0047 — The heap: vlibc's allocation functions, typed views, free and leaks (M4.5d2a)

- **Status:** draft for M4.5d2a (2026-11-03), written before code (invariant 6); revised after audit round 1 (Opus, on
  the draft: three blocking items — B1 leaks missed inside loops, a block dropped at a back edge and a `return` inside a
  loop that sees only the invariant, reproduced with Silicon, item 7; B2 view calls skipping 0046 item 8's redirection
  check, item 2; B3 block sites invisible to the sequencing rules, item 9 — and S1–S8, N1–N14, taken in items 2–10;
  `build/m4.5d2a/audit1.md`); revised after P72 (PASS on macOS and glibc: EDG accepts vlibc's three declarations in both modes, the `void*` conversion and the folded `sizeof` survive in the IL, no platform macro collides with a reserved name, no platform asm label, weak, alias or ifunc on the three; items 2, 5) and S36 (PASS, 64 fixtures; findings in items 4, 6, 7, 9, 10); and audit round 2 (Opus, on `250a2af` and the vlibc pin `5962865` → `0f5de92`: no blocking item; S1 the
  lint's view pin in programs with globals, S2 a block site's guard reading the heap (exit 3), S3 places framed only by a
  block site (exit 3 and false alarms), S4 two missing pairs, S5 the shape check `vlibc_contracts.py` lacked; notes N1–N9;
  all taken in items 2, 4, 7 and 10 or the handoff; `build/m4.5d2a/audit2.md`; the pin audit found the three contracts
  correct against C23 7.24.3 and the platforms' attributes). During the session the owner answered questions 124
  (`reject_owning_function.c` expects `lifetime @ L8`, the liveness of its unframed `p`, instead of the plan's
  `permission`) and 125 (two new pins re-expected to what they can reach: a template, a lambda). Owner decisions (M4.5d2a planning, 2026-11-03; plan `.claude/plans/m4-5d2a-heap.md`;
  handoff questions 120–123, each the recommendation):
  - **120:** M4.5d2 is three sessions: d2a the heap (this record), d2v vlibc's allocator, d2b results, owning locals,
    globals and fields and P4418R0. `realloc` moves to M4.5e2 (it needs `n * sizeof(T)` and a block's contents); blocks
    held through locals across loops are d2a's first cut.
  - **121:** vlibc holds complete implementations of the C library functions it covers, usable without cerastes and
    verified as cerastes becomes able to (the owner's directive, amending 0030 item 2's "trusted contracts for what C
    cannot implement"). For the allocation functions: a replacement allocator under the standard names — a portable C23
    core over memory regions, pages from `mmap`, glibc's documented replacement set, on macOS a malloc zone made the
    default zone (so libSystem's own allocations use vlibc too), thread-safe. Its contracts stay trusted until cerastes
    verifies the core; the page and zone glue stay trusted.
  - **122:** that allocator is M4.5d2v, a session of its own in vlibc's repository right after d2a.
  - **123:** blocks still held when `main` returns are leaks, like VeriFast (item 7).
- **Milestone:** M4.5d2a (2026-11-03).
- **Implements:** 0028 item 6 (freeing, transfer, leaks), 0028 item 1's allocation conversions, 0029 item 1's target
  `r` and item 3's `owns_block`, 0030 item 2's malloc family (contracts), 0039 item 4's liveness and token cells.
- **Amends:** 0028 items 1, 6, 13 and 18 (the fixtures' shapes), 0029 items 1–3, 0030 item 2 (question 121), 0038 item
  3 (item 10, if it lands), 0039 item 4, 0042 item 8, 0046 items 4 and 9.

## Context

Record 0028 designed the heap: a heap block's free token at slot −1 (`acc(loc(b, -1).tok)`), `owns_block(p)` as
ownership of a whole allocation, `free` consuming it, `invalid-free` when it is not there, and leaks by `forperm` at a
function's end; `void*` restricted to an allocation's result converted to `T*`, an argument to a `void*` parameter and a
null test. Record 0029 gave `[[cerastes::owning]]` a transfer meaning on a function (target `r`). Record 0030 put the
contracts of `malloc`, `calloc`, `realloc` and `free` in vlibc, and record 0046 made vlibc's contracts the trusted ones
(trust from a library file's tag, reserved names, the macro scan, attributes by their own position). Nothing of the
heap exists in the code: no token, no `owns_block`, no `forperm`, no `leak` or `invalid-free`. The Viper is pinned by the
fixtures `e2_malloc_free.vpr`, `e2_double_free_fail.vpr`, `s25_c_leaks*.vpr`, `s25_f_owning*.vpr`, `s26_live_free_blocked*.vpr`
and `s26_member_malloc_struct*.vpr` in `tests/viper-smoke/memory/`.

Three facts shape this record (the planning surveys; P72 confirms them):

- **A contract over bytes cannot name a C object type.** `malloc` returns `void*` and its contract can say only "a new
  allocation of `size` bytes"; the model has no bytes (0028 item 1: a field per C object type, different fields never
  alias). So an allocation becomes cells only where C converts its result to `T*`: the **typed view** (0028 item 1,
  "M4.5d2's typed view"). VeriFast does the same by recognizing `malloc(sizeof(T))` in its symbolic execution; VerCors
  maps `malloc` to a built-in. Records 0030 and 0046 removed the encoder's last table keyed by a function's name, so the
  view is keyed by the *shape* of a trusted contract, not by `malloc`'s name.
- **vlibc's declarations break today's checks.** In C++ EDG moves the allocation functions' `decl_position` to vlibc's
  redeclaration (P71 (d)), so the declared-routine loop would reject `void*` at vlibc's line in every unit that includes
  `<stdlib.h>`; in C the rejection of a marker on a redeclaration of a system function (`encode.cpp:2716`) would reject
  vlibc's own `ref_to_uninit`. The routines must be diverted before those checks, in the commit that moves the pin.
- **Marked range rows do not exist yet** (0011's amendment: M4.5e1), so every permission check of this record is a
  `perm` row on a single cell: d2a allocates one object per allocation.

## Decision

1. **vlibc's contracts.** vlibc's `<stdlib.h>` declares, after `_Exit`:

   ```c
   // cerastes:trusted-contract vlibc: malloc returns NULL or the start of a new allocation of size bytes, its contents indeterminate (C23 7.24.3.6)
   void *malloc(size_t __size) CERASTES_REF_TO_UNINIT CERASTES_ENSURES(__r, __r == NULL || cerastes_owns_block(__r, __size));
   // cerastes:trusted-contract vlibc: calloc returns NULL or the start of a new allocation of nmemb * size bytes, all zero (C23 7.24.3.2)
   void *calloc(size_t __nmemb, size_t __size) CERASTES_ENSURES(__r, __r == NULL || (cerastes_owns_block(__r, __nmemb * __size) && cerastes_zeroed(__r)));
   // cerastes:trusted-contract vlibc: free ends the allocation ptr is the start of; free(NULL) does nothing (C23 7.24.3.3)
   void free(void *__ptr CERASTES_REF_TO_UNINIT) CERASTES_REQUIRES(__ptr == NULL || cerastes_owns_block(__ptr));
   ```

   - **Reserved identifiers only** (handoff note, d1 audit 2 note 5). Every identifier in a shim contract is reserved
     (`__size`, `__nmemb`, `__ptr`, the result binder `__r`, `cerastes_*`) or one of `NULL`, `false`, `true`;
     `scripts/vlibc_contracts.py` refuses any other. The macro scan of 0046 item 4 is unchanged: a program that defines
     `__size` as a macro is already undefined (C23 7.1.3), and the scan still refuses it.
   - **Shapes in vlibc's script** (audit round 2 S5): `vlibc_contracts.py` refuses a declaration with `void*` in its
     signature whose contract is neither of item 2's shapes, so such a header cannot be pinned.
   - **`calloc`'s product** `__nmemb * __size` is read as the exact product (audit round 2 N5): C23 7.24.3.2 returns
     null when the size cannot be represented, so a wrapping `size_t` product is never the allocation's size; the
     encoder computes it exactly and rejects a size that does not equal `sizeof(T)`.
   - **`free`'s parameter is `ref_to_uninit`.** Without it P4222's default ("a pointer points to something
     initialized", 0028 item 14) would make `free` of a block never written `uninitialized`; `free` takes the storage
     whatever it holds.
   - **The reserved table** (0046 item 6, generated) gains `malloc`, `calloc` and `free`; `tests/vlibc/trusted_contracts.txt`
     gains three lines (a golden change, audited, 0046 item 9). vlibc's CI moves `CERASTES_COMMIT` to `f59e4b2`, whose
     `<cerastes.h>` defines `CERASTES_REF_TO_UNINIT`.
   - **`realloc`**, `aligned_alloc`, `free_sized` and `free_aligned_sized` get no contract in d2a: a call is
     `call_without_contract` (`realloc` with M4.5e2).
2. **Two contract shapes, and typed views by conversion.** A routine trusted from vlibc (0046 item 4) whose signature
   mentions `void*` must have exactly one of two contracts; any other is an internal error (exit 3; vlibc's own
   `vlibc_contracts.py` refuses such a header before it can be pinned):
   - **(A) Allocation:** a `void*` result, integer parameters, no `requires`, and one `ensures` of the form
     `B == NULL || owns_block(B, S)` or `B == NULL || (owns_block(B, S) && zeroed(B))`, where `B` is the result binder
     and `S` a parameter or a product of parameters; the routine carries **exactly one** of `ref_to_uninit` and
     `zeroed` (audit S3: an allocation with neither would give set flags over unspecified values). `cerastes_owns_block`
     and `cerastes_zeroed` are recognized as entities (0037 item 5's test: the functions `<cerastes.h>` declares), not by
     name (audit N6).
   - **(D) Deallocation:** a `void` result, one `void*` parameter `P` marked `ref_to_uninit`, `requires(P == NULL ||
     owns_block(P))`, and no `ensures`.

   **What the IL holds (P72).** An (A) call (type `void *`) is the operand of an `eok_cast`: `compiler_generated` for C's
   implicit conversion, `is_static_cast`/`is_reinterpret_cast` in C++, no flag for a C cast; `T` is the cast's pointee
   type (qualifiers on it visible). A size argument is an `enk_constant` whose `constant.expr` keeps the original
   expression (`enk_sizeof`, a cast of `int 4`, an `eok_multiply` tree), which is what S1's checks encode. `free`'s
   argument is a `compiler_generated` cast of the `T*` value (none for an argument already `void *`; `free(NULL)` is an
   `enk_constant` of type `void *` with no pointee). In vlibc's contracts the binder is `param_ref{1,0}` and parameters
   are named by position; `free`'s marker is on the routine type's parameter entry (in C++ unnamed, `param #0`),
   `malloc`'s on the routine's attribute list before the `ensures`. `std::malloc` and `::malloc` are one entity.

   The shape is checked once per routine, called or not, in the declared-routine loop, which then skips the routine
   before `check_signature` and before its forced encoding. Nothing else about such a routine reaches VIR: **VIR never
   holds `void*`** (`check_program` keeps rejecting `Ptr(void)`).

   - **An (A) call** is accepted only as the direct operand of a conversion from `void*` to `T*` — an explicit cast in
     either language (a C cast; in C++ also `static_cast`), or in C the implicit conversion of an initializer, an
     assignment, a `return` or an argument (C++ has none) — where `T` is an integer, `bool`, an enumeration, an accepted pointer type or a verified structure, not
     qualified `const` or `volatile`. Each argument must be an integer constant the front end folded (`sizeof(T)`,
     `sizeof *p`, `4`) and is still encoded through its original expression as any folded constant is (`encode.cpp:1580`),
     so its own checks stay — 0022's no-wrap check (`sizeof(int) * 0x4000000000000001u` folds to 4 by wrapping and is an
     `overflow`), 0020 item 5's `side_effect_not_evaluated`, 0024 — and its value is then discarded (audit S1). `S` is
     evaluated exactly over the converted `size_t` values; it must equal the front end's `sizeof(T)`. Any other size (a
     variable, `n * sizeof(T)`, `0`, a constant other than `sizeof(T)`) is rejected `unsupported_expression`, naming
     M4.5e2. A size that equals `sizeof(T)` but is spelled `4` is accepted: it is the same allocation.
   - **The call's checks stay** (audit B2). A view call goes through every check `callee_for_call` makes of a trusted
     routine before its contract is used: `lib_trust`'s verdict, the reserved-name rejection, and 0046 item 8's
     redirection check (an asm label, `#pragma redefine_extname`, an alias, `ifunc`, `weak`/`weakref`, `#pragma weak`), so
     `#pragma redefine_extname malloc my_malloc` or `#pragma weak free` before `<stdlib.h>` makes the call
     `unsupported_declaration`, as for `exit`. P72 checks that neither platform's `malloc`, `calloc` or `free` carries an
     asm label or a weak attribute (audit N13).
   - **The view.** The call becomes a call of a **view function**, one per (routine, `T`), cached: bodiless, no
     parameters (the constant arguments have no effect and no obligation, so the view's contract is true of the C
     call), result `Ptr(T)`, `ensures r == NULL || owns_block(r)` (item 4); for `zeroed`, one value clause per leaf of
     `T` (an integer `0`, `false`, a null pointer, an enumeration's `0`) and the flags set; for a routine marked
     `ref_to_uninit`, the flags unknown (item 8). Its reason is vlibc's tag followed by `; typed view: one <T> (<n>
     bytes), decision 0047`, and its location is vlibc's declaration. **Every 0018 value range contains 0** (a fixed
     underlying type's range does; `[0, 2^M−1]` and `[−2^(M−1), 2^(M−1)−1]` do), so a zero enumeration leaf is a value of
     its type. **A null pointer is all bits zero** on the two supported platforms (arm64 macOS, x86-64 Linux); C23
     7.24.3.2 does not promise it, so it is a platform assumption of `calloc`'s view, recorded here and in record 0031's
     target facts.
   - **A (D) call:** the `void*` argument must be the conversion of a `T*` value (0028 item 6: the layout comes from the
     static pointee type before the conversion); `free(NULL)`, `free(0)` and an argument of static type `void*` are
     rejected (no layout), and so is a `T*` to `const` or `volatile` `T` (M4.6 for `const` objects). The call becomes a
     view `(Ptr(T)) -> void` with the same `requires` and a VIR flag `deallocates`, whose permission rows are category
     `invalid-free` (item 6).
   - **C++.** `std::malloc` and `std::free` are `::malloc` and `::free` (`<cstdlib>`). The typed view is C++'s too:
     `malloc` implicitly creates objects of implicit-lifetime types ([intro.object]/10–13, [c.malloc]/4), and every type
     cerastes accepts is one: a structure with a default member initializer, a base, a virtual function or a non-public
     member is rejected (`encode.cpp:9129-9168`; audit N5).
   - **Why it is sound.** A view's postcondition holds of the C call's result in the model: a fresh block of `len`
     1 whose one slot holds a `T` (an allocated object takes the effective type of what is stored, C23 6.5p6; the
     one-field rule, 0028 item 1, then forbids every other type's access, as C does). `malloc`'s result is suitably
     aligned for every type of fundamental alignment (C23 7.24.3p1), and every type cerastes accepts has one: `_Alignas`,
     `alignas` and `aligned` are rejected `unsupported_attribute` (audit N4). No axiom is added; the model of 0028 item 2
     interprets a fresh `loc(b, i)` block with `lvloc(b) = loc(b, −2)` and the token at `loc(b, −1)`.
   - **A member's address is not the allocation** (audit N2): `free(&s->first)` names the first member's block, which has
     no token, so it is `invalid-free` although C gives it `s`'s address — 0028 item 1's model, where an object and its
     first member are different blocks (and why `void*` is restricted).
3. **Library markers** (handoff notes, d1 audit 2 notes 3–4; amends 0046 item 4). `lib_trust` counts the registered
   markers written in library files — on the routine and on its parameters — together with `requires`/`ensures`, so a
   declaration whose only annotation is a marker still needs its tag and passes the reserved-name and macro checks.
   `read_marker` reads a library routine's markers by their library position (no all-or-none rule between the platform's
   unmarked declaration and vlibc's, no `void*` refusal there), and the rejection of a marker on a redeclaration of a
   system function (`encode.cpp:2716`) skips declarations in library files.
4. **`owns_block`** (0028 item 6, 0029 item 3). `cerastes_owns_block(p)` (C and C++) and `cerastes::owns_block(p)`
   (C++) take a pointer value, like `live`. In a contract it is **ownership of the whole allocation** `p` points to the
   start of. It is the **block part** — `p != null`, `WF(p)` (`WF_S` for a structure), `offset_of(p) == 0`, the token
   `acc(loc(b, −1).tok) && loc(b, −1).tok`, `lvloc(b) == loc(b, −2)`, full permission to the liveness cell
   `lvloc(b).lv`, `len(b) == 1` and, for a structure, 0043's member lengths and slot facts — and, **for each leaf, exactly
   what `owns` of that leaf gives except its liveness fraction** (audit S2): full permission to the value and flag cells,
   the flag set (unless the place is `ref_to_uninit`, item 8), and the facts `owns` states about the value under the flag
   (an integer's range, a pointer leaf's `WF`; 0042's `held && flag ==> range`). One definition serves the view's
   `ensures`, a contract's `requires` and `ensures`, the call and return rows (one explicit row per conjunct, 0028 item
   11) and item 6's free rows, so Silicon's own pre- or postcondition check never fails first. S36: the `flag ==> range`
   conjunct is pure and Silicon checks it at a call, so every place that may later meet a block site's `requires` (a loop
   place, 0042's `held && flag ==> range`; a bodiless `ref_to_uninit` callee's `ensures`, which promises `flag ==> range`
   beside 0042 item 5's `old(flag) ==> flag`) carries it, and the call, return and free rows include an `internal`
   range row per leaf. It carries **no ghost
   amount**: it is a write permission to everything in it.
   - **Where:** on a pointer formal (a parameter's entry value) in a `requires` and in an `ensures`, and on the result in
     an `ensures`. **A `requires` atom alone consumes** the block (the function frees it or hands it on); **an `ensures`
     atom on the result alone gives** the caller a block; **the same atom in both on a formal borrows** it and returns it.
     An `ensures` atom on a formal without its `requires` partner is rejected (the caller's pointer cannot come to own a
     block the call made: d2b's partnerless atoms).
   - **Reads it frames.** A contract read of a cell of the block (`r->balance == 0` after `owns_block(r)` in the same
     `ensures`) is framed by it, as by an `owns` atom (0037 item 7, 0040 item 4).
   - **Entry, calls and returns.** Its cells join the method-entry distinctness asserts, the separation rows at calls
     (0037 item 8) and the separation rows at returns (0037 item 10; audit S4: `requires(owns_block(p))
     ensures(owns_block(p)) ensures(r, owns_block(r)) { return p; }` must fail at a row, never at Silicon's own
     postcondition check): two block sites, or a block site and an `owns`/`reads`/`live` site, never name one block,
     which `lvloc` rows check.
   - **As an atom elsewhere** (audit N11): a block site on a `ref_to_uninit` (or `must_init`) parameter is that marker's
     atom (0042 item 4: a user `void my_free(int *p CERASTES_REF_TO_UNINIT) CERASTES_REQUIRES(p == NULL ||
     cerastes_owns_block(p))`); a `restrict` parameter whose only atom is a block site is rejected (0037 item 2's
     `restrict_backed` reads `owns` atoms only; d2b may lift it).
   - **Rejected,** each with its own message: the two-argument form, `cerastes_zeroed` and a `void*` operand outside a
     library file; an operand reached through a load (`owns_block(*pp)`, `owns_block(p->q)`), a global, under `old`, in
     a measure or in a loop invariant (d2b); beside `owns`/`reads`/`live` on the same root, or twice in one clause list;
     on a pointer to a `const` or `volatile` type (audit N12: 0040 item 2 rejects `owns` of a `const` place, and such a
     block could never be freed without a cast that drops the qualifier).
   - **A block site's own guard** may not read the heap, a global or an entry value (audit round 2 S2: `owns(*q) &&
     (*q > 0 ? owns_block(p) : true)`, `g > 0 ? owns_block(r) : r == NULL`): rejected `unsupported_annotation`, naming
     M4.5d2b; the leak row's exemption repeats the guard, which must be a pure fact of the formals and the result.
     `owning` on a function returning a pointer to a `const` or `volatile` type is rejected as the atom is (N1).
   - **Places framed by a block site** (audit round 2 S3): a place or guard reached through a load (`owns(*p->q)` after
     `owns_block(p)`) is framed by the block sites before it in evaluation order, as by the atoms before it, in the
     method, at calls, at returns and in the loop snapshots.
5. **Target `r`** (amends 0029 item 1's registration phasing). Registration becomes `owning` `"r|p"` and
   `ref_to_uninit` `"r|p"`, both `RAF_ENTITY_ATTACHED` (P72 (e)); `v` and `d` stay unregistered (EDG keeps refusing
   locals and fields until d2b).
   - **`[[cerastes::owning]]` on a function** is `ensures(r == NULL || owns_block(r))`, merged with an identical
     explicit clause (as 0040 item 3 merges a parameter's). A function that returns a pointer into an object it does not
     own entirely fails it at the return (`permission`: no token). On a function that returns no object pointer:
     `unsupported_annotation`.
   - **`[[cerastes::ref_to_uninit]]` on a function** requires an `ensures` `owns_block` on the result and drops that
     atom's flag rows (the caller gets cells that may hold no value). Without such an `ensures`: `unsupported_annotation`.
     0042 item 6 extends to it (audit N7): a read of the result's cells in that function's own `ensures` has no value to
     read and is rejected statically.
   - **Where target `r` is read** (audit S5; 0029 item 1's phasing and 0042 item 4's "never ignored"): on a function
     declared or defined in a verified file that is neither a class member, a template, a lambda nor a `predicate`/`pure`
     function. Every other routine entity that registration now lets the attribute reach is rejected
     `unsupported_annotation`, one pin each: a member function, a function template or its instance, a lambda's call
     operator, a `predicate`/`pure` function, and a C or C++ redeclaration of a function a system header declares first
     (`encode.cpp:2716`'s rule extended from parameter markers to routine markers); a function-pointer typedef never
     reaches a routine (EDG's #1835 on `typedef` stays, P52).
6. **The free checks: `invalid-free`** (a new category). Before a (D) call lowering writes, each guarded `p != null ==>`
   and in this order (after the separation rows of item 4): `WF(p)` (`internal`); `perm(loc(b, −1).tok) == write &&
   loc(b, −1).tok` ("… which may already be freed or not be an allocation: a local, a global or a member"); then
   `offset_of(p) == 0` ("frees `p`, which may not be the start of an allocation": written after the token row, it can
   fail only for a pointer into a block the function owns, which needs M4.5e2's arithmetic, so its C test comes with e2
   and `e2_double_free_fail.vpr` pins it; written first, it failed beside the token row for a borrowed `struct S *s`, two
   errors at one call);
   `lvloc(b) == loc(b, −2)` (`internal`, implied by the token); `perm(lvloc(b).lv) == write` ("… while a pointer to it
   may still be in use elsewhere"); `len(b) == 1` and the member lengths (`internal`); `perm(c) == write` for every leaf
   value and flag cell ("… without owning all of it"). Then the call consumes the block; any later use of `p` is
   `lifetime` (0028 item 3a) and any access `permission`.
   - **The argument's use check** (0028 item 3a: every pointer value passed as an argument is checked live first) is
     not written for a (D) call's argument when the argument is the pointer itself (a variable, a member read, a
     `T*`-typed expression without effects): the rows above check more (the token, then the whole liveness cell), so
     a double free is reported `invalid-free` ("… may already be freed"), not `lifetime`. Any other use inside the
     argument (`free(q = p)`, a call) keeps its own checks.
   - **Which row fails** (S36 (d)): `free(&s->y)`, `free(&x)`, `free(&g)`, a borrowed parameter and a double free all
     fail the token row; a block a callee keeps fails the liveness row; a leaf given away fails its leaf row. A failed
     row ends its path only when its assumption contradicts the permissions held (the token, `perm == write` and
     separation rows do; the leak, range and `lvloc` rows do not), which the twins follow.
   - **Fixture differences** (amends 0028 item 18): the views take no parameter (`malloc_int(n)` in `e2_malloc_free.vpr`),
     a block's cells are checked one row each (the fixtures' marked range row waits for M4.5e1), and `malloc`'s flags are
     unknown rather than `!d` (the verdicts agree: an unknown flag fails every read check that `false` fails).
7. **Leaks: `leak`** (a new category; 0028 item 6; question 123).
   - **A heap program** has a view call or a block site in any contract of any function, called or not (audit S8: an
     uncalled `CERASTES_OWNING int *f(void)` names `tok`, which must then be declared). Programs that are not heap
     programs keep their Viper byte for byte.
   - **At the function's end** (audit B1 (b)). Viper is affine, and a `return` inside a loop runs its postcondition rows
     inside the Viper loop body, which sees only the invariant, never the frame (reproduced: a `forperm … :: false` there
     passes while the method holds a block). So the leak row is written once per method, **after the end label** where
     every `return` and the fall-through land (Silicon merges the frame back at a jump out of a loop): `assert forperm t:
     Ref [t.tok] :: E`, where `E` is the disjunction, over the `ensures` block sites, of `g && t == loc(block_of(q), −1)`
     (`g` the site's guard as written, `q` its place: `res` for the result, the formal for a borrowed parameter, whose
     value never changes), or `false` when there is none. It is reported at the function's closing brace ("`f` may return
     while it still owns an allocation that its `ensures` does not hand back"), as VeriFast reports "Function leaks heap
     chunks" at a function's end: the row cannot tell which `return` leaked. Paths that end in a call that does not
     return never reach it.
   - **At every back edge** (audit B1 (a)). At the end of a loop body (every `continue` lands at the body's own
     continue label before it, so one row covers them) of a heap program whose body can call a function with an
     `ensures` block site (a view or a user's; nothing else can give the loop a token), before Viper exhales the invariant (which drops whatever it does not name), `assert forperm t: Ref [t.tok] :: E_L`, where
     `E_L` is the disjunction of the token places the loop's invariant carries (item 10: `t == loc(block_of(hb_i), −1) &&
     kt_i > none`), or `false` without them. Reported at the loop body's closing brace ("an iteration
     of the loop may end while it still owns an allocation that the loop does not carry to the next iteration"). A block
     allocated in an iteration and kept for later (stored in a pointer declared outside the loop) is reported there until
     d2b's owning variables: fail closed.
   - **The lint** (audit S7): `field tok: Bool` only in heap programs; `tok` only in a view's or a block site's
     conjunction, an explicit row, a loop place's snapshot or invariant, and a `forperm` header; no `exhale` of a `tok`
     place and no assignment to `tok`; `forperm` only as a `leak` row's assertion; the view methods' text pinned exactly,
     as `alloc_<t>` and `program_start` are (a view with an extra clause would be an unchecked assumption); a view in a
     program with globals carries 0041's global-facts `requires` first, pinned equal to the other methods' (audit round 2
     S1); a bodyless method with vlibc's allocation tag but no `typed view:` reason is refused (N3).
   - **`main`** (question 123) is checked like every function: blocks it still holds when it returns are leaks. A path
     that ends in a call that does not return (`abort`, `exit`, `_Exit`, a user `noreturn` function) has no exit and is
     not checked: `exit()` is C's explicit way to end with blocks allocated. That differs from C23 5.1.2.3.4 (returning
     from `main` is `exit`) on purpose. Blocks reachable from globals at `main`'s return are leaks until d2b's owning
     globals.
   - **Sound because** a leak is a resource rule: no verdict about undefined behaviour depends on it, and a token can only
     come from a view, a callee's `ensures` or the method's own `requires`.
   - **New pieces:** `LExpr::Kind::ForPerm`; `field tok: Bool`, declared only in heap programs; the lint rules above.
8. **Initialization** (amends 0042 item 8).
   - `malloc`'s view (marked `ref_to_uninit`): the cells' flags are unknown, so a read before a write is `uninitialized`.
     `calloc`'s view: flags set, values zero. An `owns_block` atom in an unmarked `ensures` includes the flags (P4222's
     default); in a `requires`, it includes them unless the parameter is `ref_to_uninit` (`free`).
   - **Under `--strict-init`:** `free`'s marked parameter is exempt from 0042 item 8's argument rule (`free` takes storage
     whatever it holds), and **every call of a function whose result is `ref_to_uninit`** (`malloc`'s view, a user's
     marked function) is rejected `unsupported_annotation`, naming d2b, wherever it stands — a local's initializer, an
     assignment to any place (`*pp = malloc(…)`, `s->q = malloc(…)`), a `return`, an argument (audit S6). P4222R3 and
     P4416R0 require the receiving pointer to be marked (target `v`, a marked result, a marked parameter), which d2b
     decides. `calloc` (unmarked) stays accepted. The default mode is unchanged.
   - The `internal` row of 0042 item 5 (`(g_r && old(flag)) ==> flag`) is re-checked against a fresh heap chunk (S36 (j)).
9. **Calls beside accesses, and two calls** (0028 item 13, 0039 item 4: "liveness and token cells wait for M4.5d2"; audit
   B3).
   - **A block site is a full-permission atom on every leaf cell of its block** for every rule that reads a callee's
     atoms: 0039 item 4's call/access and call/call pair checks (`memory_fields` and the `Acc` registration, which read
     `atoms_of(callee).sites` and so would miss a block site in its own list), 0028 item 13's snapshot and placement rule
     (writes and snapshots never move across such a call), and 0038 item 3's "touches memory" and havoc rule. So
     `*p + (free(p), 0)` and `g(*p, (free(p), 0))` are `unsequenced` in either evaluation order (a dereference has no
     liveness use of its own: its permission row stands for it, `lower.cpp:1418`).
   - **Liveness cells.** In a full expression that holds a call with a consuming block site (a `requires` site
     without an `ensures` partner) or a (D) view, that call registers a write of the block's liveness cell (the token
     cell needs no access of its own: only a consumer reads it, and a consumer also writes the liveness cell),
     and every use of a pointer value beside it a read of its liveness cell; 0039 item 4's pair checks then give
     path-precise `unsequenced` for `(free(p), 0) + use_live(p)` and `(free(p), 0) + (p != q)`, reported at the
     pointer operand whose use may come after the free (`p`). A borrowing site (both
     clauses) does not end a lifetime and registers its liveness cell as `owns` does, at a fraction (audit N8: otherwise
     `borrow(p) + (p != q)` would be a false `unsequenced`). Full expressions without such a call keep their Viper.
   - (Cut 3: a fail-closed `unsequenced_calls` for any full expression holding a block call beside another memory call
     or a pointer use.)
10. **Loops** (amends 0038 item 3; heap programs only; landed). A loop that touches memory takes, as
    automatic places, the object each pointer variable that is not a cell designates at the loop's entry, through its
    base `hb` — the variable itself when the loop does not assign it, else a loop-entry ghost `hb := q` (0045 item 10's
    `lb`) — : its cells (with 0042's `held && flag ==> range` row), its liveness cell and its token cell (a new place
    kind: snapshot `kt := perm(loc(block_of(hb), −1).tok)`, conjunct `hb != null ==> acc(loc(block_of(hb), −1).tok, kt)`,
    value `kt > none ==> loc(block_of(hb), −1).tok`, back-edge `permission` row `perm(loc(block_of(hb), −1).tok) >= kt`;
    audit N9). Guards add `block_of(hb_i) != block_of(hb_j)` for token cells. A `requires` block site's places gain the
    token too. Item 7's back-edge leak row exempts exactly these token places. A written `owns_block` in an invariant is
    rejected, naming d2b. The pointer variable must be declared before the loop and not be a cell. Two more rules came
    with it: a call whose `requires` has a block site writes every leaf field of that block (0038 item 3's havoc rule;
    without it a loop calling a borrowing function exited 3), and a block row's `WF` check at a call is a `permission`
    row, so a pointer a loop reassigns gives a user error, never exit 3. **A pinned false alarm:** two block pointers
    swapped inside a loop (`negative/heap_loop_swap_false_alarm.c`): the places are the blocks the variables held at
    entry, and after the loop nothing says which variable holds which block, until written `owns_block` atoms in
    invariants (M4.5d2b).
11. **Still rejected** (fail closed; one pin each, the message naming the session): an allocation size other than
    `sizeof(T)` (M4.5e2); an (A) call not converted directly to `T*` (`void *m = malloc(4)`, a discarded or tested
    result, `free(malloc(4))`; M4.7, M8); `void*` objects, parameters and results in user code and every other `void*`
    conversion or comparison (0028 item 1; M4.7, M8); `free(NULL)`, `free` of a `void*`-typed or qualified operand;
    `realloc`, `aligned_alloc`, `strdup` (`call_without_contract`); the `owns_block` forms of item 4; `owning` and the
    markers on locals and fields (EDG's own refusal, d2b); `ensures` atoms with no `requires` partner other than a
    result's `owns_block` (d2b).
12. **Categories.** `invalid-free` and `leak` join the table of 0011 item 10 (twenty categories) in its five copies
    (this table's record, `run_tests.py`, `test_run_tests.py`, `tests/README.md`, "Error messages" with an example
    pair each). No new rejection tag.
13. **Facts relied on:** P72 (`internal/il-inventory.md` `### P72`; the IL of allocation conversions, constant sizes, `free`'s argument, vlibc's contracts and
    markers, registration `"r|p"`; macOS and glibc), S36 (the views, the free rows, leaks, `owns_block` sites, calls and
    returns, sequencing, loops, the 0042 row, freshness, cost; `### S36`, 64 fixtures in `tests/viper-smoke/memory/s36_*`;
    two contrast fixtures expect Silicon's own reason on purpose, showing the separation rows are needed). **Cost:** the
    leak row is exact, so a function with several allocations frees the others on each failure path; five live blocks
    verify in about 2.5 s, five without that cleanup took 34 s. The papers checked at planning (P4222R3, P4416R0,
    P4418R0, P4417R0) are d2b's; their findings are in the handoff.
14. **Tests and documentation.** Must-verify and must-fail pairs for every accepted form in both modes where valid
    (plan, Tests), twins of the trusted contracts violating `free`'s `requires` (0046 item 9: a double free, a local, a
    global, a member, a borrowed block, a block a callee keeps); the ports `account.c`, `deposit.c`, `swap.c`'s `main`,
    `globals.c`'s allocation half and `malloc_block.c`'s `free(&local)` twin. Docs: "Memory and ownership" (the heap),
    "Annotations" (`owns_block`, target `r`), "Error messages" (`invalid-free`, `leak`), "Verifying a program" (vlibc's
    allocation functions); `internal/encoding.md`.

## Consequences

- cerastes verifies C programs that allocate single objects with `malloc` and `calloc` and free them, reporting double
  and invalid frees, uses after free, frees while a callee keeps a pointer, and leaks — `main`'s included.
- The encoder still has no table keyed by a function's name: an allocation is a trusted contract of a known shape, and
  its meaning at a call comes from the conversion C writes.
- `void*` appears only at an allocation's conversion and at `free`'s argument; VIR never holds it.
- A heap program's Viper gains `field tok` and `forperm` rows; every other program's Viper is unchanged.
- vlibc's allocation contracts are trusted until M4.5d2v's allocator is verified (after M4.6); the platform's allocator
  stands behind them until a user links vlibc's.
