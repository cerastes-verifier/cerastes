# 0032 — Pointers to scalars inside one function: the memory model's first slice (M4.5b1a)

- **Status:** accepted for M4.5b1a (2026-10-28), revised after audit round 1 (Opus: one blocking item, `restrict` on the
  pointer itself, and eight should-fix items, all taken) and audit round 2 (one blocking item, a discarded pointer
  value without its liveness check, and three should-fix items, all taken). Owner decisions (M4.5b1 planning, 2026-10-28):
  - **M4.5b1 is three sessions** (a design pass put handoff §4.4's single session at well over 27 agent-hours for
    `src/vir` and `src/viper` alone): **b1a**, this record, the model and address-taken locals inside one function;
    **b1b**, pointer parameters and contracts; **b1c**, owning pointers, sequencing on objects, loops, ports. Each
    session rejects what a later one makes precise (invariant 3).
  - **Old rejection tests:** an existing test whose only point was "pointers are rejected" is retired where its
    construct is now accepted (listed in the status entry); any other change to an existing test's outcome or Viper
    stops the session.
  - **Later flips pre-approved:** b1b and b1c may retire or re-expect exactly the rejection tests of item 10 that the
    status entry lists for them.
  - **Loops (for b1c):** permission invariants are hybrid: automatic for places reached through an address-taken local
    or a pointer parameter the function never reassigns, written (`owns`/`reads` in `CERASTES_INVARIANT`) for
    everything else.
- **Milestone:** M4.5b1a (2026-10-28).
- **Implements:** record 0028 items 1–4, 11 and 14 for locals; records 0033–0036 (the four axioms).
- **Amends:** 0028 item 1 (a field pair per VIR type, item 5), 0028 item 4 (allocation at block entry), 0011 items 9–10 (three categories), 0012 item 12 (pointer
  locals accepted), 0019 item 13 (the flag is a field of the cell), 0020 item 4 (cells are written at statement level
  only, until b1c).

## Context

Record 0028 designed C's memory for cerastes and pinned the encoding with hand-written Viper
(`tests/viper-smoke/memory/`). Until this record the encoder rejected every pointer type (0012 item 12), VIR had no
heap (`Type::Kind` was `{Void, Bool, Int}`), and lowering had no notion of leaving a scope. This record is the slice of
0028 that needs no contract: objects whose address is taken, pointers to them held in locals, and the checks that keep
every access defined. A function cannot yet receive or return a pointer, so no permission ever crosses a call.

Facts (`internal/il-inventory.md`, "M4.5a" P51 and "M4.5b1a" P56, P57): the IL marks a variable whose address is taken
anywhere with `address_taken`; there is no lvalue-to-rvalue node (`*p` is `indirect`, an lvalue only as an assignment's
target); every spelling of a null pointer constant folds to an integer constant of pointer type; C compares with an
`int` result and tests a pointer bare, C++ with `bool` and a `bool_cast`; in C, EDG only warns about incompatible
object pointer types (#42, #144, #513, #167), and `int *q = v;` from a `void*` is silent.

## Decision

1. **Scope.**

   | | b1a (this record) | b1b | b1c |
   |---|---|---|---|
   | Objects | address-taken locals, allocated at block entry; scope exit at fall-through and `return` | `&param` | `T**`; `goto`/`break` out of a cell's scope; cells in loops |
   | Operations | `&x`; reading `*p` and an address-taken `x`; writes at statement level; NULL; `==`/`!=`; truth tests; pointer locals; `?:` | — | writes inside expressions |
   | Contracts | none: pointer parameters and results rejected | `WF`, `owns`/`reads`/`live`/`old`, ghost amounts, call and return checks | `[[cerastes::owning]]`, `owns_block` |
   | Categories | `null-dereference`, `permission`, `lifetime` | `pointer-comparison` | `unsequenced` |

   Everything outside b1a is rejected with a diagnostic (`unsupported_type`, `unsupported_expression`,
   `unsupported_statement`, whose fixed texts cite 0012, the record whose item 12 this one amends); item 10's table
   and each test's `Pins:` line say which session lifts it.

2. **Pointer types.** A pointer type is accepted when its pointee is an integer type, `bool` or an accepted enumeration
   (0018), with or without `const`, and only as the type of a local variable that is not itself address-taken, or of an
   expression. Rejected: pointer parameters and results (b1b); a pointer whose pointee is a pointer, and a pointer local
   whose address is taken (`T**`, b1c); `void*` (stricter than 0028 item 1, whose uses all need a call or an
   allocation: M4.5d2); pointers to structs, unions (M4.5c) and functions (M4.7); a `volatile` pointee (M8); and a
   pointer type that is itself `restrict`, `volatile` or `_Atomic` (audit round 1: `int *restrict p = &x;` then `x = 2;
   return *p;` is undefined, C23 6.7.3.1, and would verify; `restrict` waits for a model of it). A
   `nullptr_t` value that is not converted to a pointer type stays rejected (`reject_nullptr.cpp`).

3. **Conversions** (closes audit round 4's reopened item, handoff §3). Every conversion node whose operand or result
   has pointer type is rejected (`unsupported_expression`), compiler-generated or written, except:
   - a null pointer constant becoming `T*` (folded: P51);
   - a conversion between object pointer types whose pointees are the same C or C++ type up to qualifiers (EDG's type
     identity, never VIR's, which identifies `long` with `long long` and `char` with `signed char`), where the result
     only adds qualifiers (`int*` → `const int*`). `(long long *)&l` for a `long l` and C's silent `unsigned *pu = &e;`
     for an enumeration `e` are rejected here.

   **A pointer value is one of** (audit round 1; everything else is rejected): `&x` of a cell, a null pointer constant
   (an integer constant of pointer type whose value is 0: `(int*)4` folds to the same constant kind and is rejected,
   P57), a pointer local, a `?:` of pointer values, a pointer assignment used as a value, and an allowed conversion
   of one of them. So the address of a global, which folds to an address constant in every position (P51 item 3 as
   corrected), a string literal and a compound literal are rejected; with item 4 this gives item 7's guarantee.

   In C verify mode the driver makes EDG's warnings for incompatible object pointer types errors:
   `--diag_error=42,144,167,513` (P56: `pi == pl`, `long *pl = pi;` and `int *c = pc;` from a `const int*`, `pl = pi;`,
   an incompatible argument). C++ already refuses them. `int *q = v;` from a `void *v` is rejected by item 2 (the
   `void*` local) and by this item (the conversion).

4. **Address-taken locals** (0028 items 1, 4).
   - A local whose IL variable has `address_taken` set (P51, P57) is a **cell**: a one-slot block from a trusted
     method `alloc_T()` per cell type (0028 item 17: `// cerastes:trusted-contract`, no body), which returns `r` with
     `r != null`, `r == loc(block_of(r), 0)`, `len(block_of(r)) == 1`, `lvloc(block_of(r)) == loc(block_of(r), -2)`,
     and full permission to `r.v_T`, `r.d_T` (with `!r.d_T`) and `lvloc(block_of(r)).lv`.
   - **Allocation at block entry** (amends 0028 item 4's "at its declaration"; C23 6.2.4p6: an automatic object's
     lifetime starts when its block is entered): every cell declared directly in a block is allocated when the block
     is entered, and before a `switch`'s dispatch for cells declared in its body. The declaration then stores its
     initializer, or leaves the flag false (0019): a `goto` past a declaration without an initializer, or a `case` label
     after one, finds the object allocated and uninitialized, as in C; a jump past a declaration with an initializer
     stays rejected (0014 item 1, 0019 item 6). Each entry of the block allocates anew (each loop iteration would,
     once b1c allows cells in loops).
   - A local without the flag stays a Viper local: every program accepted before keeps its Viper byte for byte.
   - **Fail closed:** `&x` on a variable without the flag is an internal inconsistency (exit 3). A `const` local whose
     address is taken is a cell like any other: writes through a `const T*` are refused by the front end and
     qualifier-dropping conversions by item 3.
   - Address-taken parameters (`&param`) are b1b's (rejected). A local constant that a `case` value names (0017) and
     whose address is taken is rejected, so that it is never both a named constant and a cell.
   - **Annotations** (audit round 1): a `CERASTES_CHECK` or `assert` condition is evaluated like code at its place, so
     it may read a cell by name, dereference a pointer and take an address, each with item 6's checks
     (`check(x == 2)`, `check(*p == 2)`, `check(&x != &y)`); a pointer in a contract, an invariant or a measure is
     rejected until b1b (`unsupported_expression`).
   - **Blocks without a position:** a cell declared in a compiler-generated block (C++'s `if (c) int x = 5, *p = &x;`)
     takes the enclosing statement's end as its scope exit's position, or is rejected (`unsupported_statement`) when
     there is none: never exit 3.

5. **The Viper shape** (0028 items 1–2, 14).
   - **Fields:** one pair per cell type, `v_<t>` for the value and `d_<t>` (Bool) for 0019's flag, `<t>` named after the
     VIR type (VIR interns integer types by width and signedness, so `long` and `long long` share a pair; no accepted
     conversion lets a pointer of one reach an object of the other, item 3); an enumeration has its own pair; `field
     lv: Bool` is the liveness cell. Exact names are in `internal/encoding.md`.
   - **The `Block` domain** of 0028 item 2, all eight functions and the four axioms (records 0033–0036), each axiom
     preceded by `// cerastes:axiom-ok <record>` and the domain by `// cerastes:domain-ok 0028`. It, the fields and
     `alloc_T` are emitted only when the program has a pointer type or a cell; otherwise nothing changes.
   - Expanded inline: no Viper `define`.

6. **Explicit checks** (0028 item 11): before each operation lowering writes every check it needs, so that Silicon's
   own reason (`insufficient.permission`, `receiver.null`) never comes first; such a reason is exit 3, as before (the
   driver accepts nothing new in b1a).

   | Operation | Checks, in order (category, message) |
   |---|---|
   | read `*p` | `p != null` (`null-dereference`, "dereferences `p`, which may be null"); `perm(p.v_T) > none` (`permission`, "reads `*p` without owning it: the object it points to may have ended"); the flag (`uninitialized`) |
   | write `*p` (`*p = e;`, `*p op= e;`, `++*p;`, `(*p)--;`) | `p != null` (`null-dereference`); `perm(p.v_T) == write && perm(p.d_T) == write` (`permission`, "writes `*p` without owning it: …"); for a compound write first the read's checks |
   | read an address-taken `x` by name | `perm(x.v_T) > none` (`internal`: the function holds its own cells until their scope ends); the flag (`uninitialized`) |
   | write an address-taken `x` by name | `perm == write` on both fields (`internal`) |
   | any other use of a pointer value: `==`, `!=`, a truth test, a copy (initializer, assignment, `?:` arm), and a value evaluated and discarded (`p;`, `(void)p;`, a discarded `?:` arm; audit round 2) | `p != null ==> perm(lvloc(block_of(p)).lv) > none` (`lifetime`, "uses `p` after the object it points to may have ended") |
   | `p == q`, `p != q` | after the liveness checks, 0028 item 3's one-past-the-end exception as an `internal` row (item 7) |
   | scope exit | `perm(x.v_T) == write && perm(x.d_T) == write` and `perm(lvloc(block_of(x)).lv) == write` (`lifetime`, "the address of `x` may still be in use when its lifetime ends"); then `exhale` of the three |

   A dereference's permission check is the liveness check of that use (0028 item 3a): a cell's permission exists only
   while its block lives, so a dangling `*p` fails as `permission`, once, not also as `lifetime`. A pointer that is
   syntactically `&x` or a null constant needs no liveness check. An asserted condition is one atomic formula (0012):
   `!(a || b)`, never `a && b`.

7. **Comparisons and truth tests** (0028 item 3). `==`/`!=` compare `Ref`s. The one-past-the-end exception is written
   as an `internal` row: in b1a every pointer value is null or offset 0 of a one-slot block (`alloc_T`'s postcondition),
   so it is provable; were it not, exit 3 (fail closed). b1b, where a parameter may point one past the end, makes it the
   category `pointer-comparison`. A pointer as a truth value is `p != null`, in C and in C++ (`bool_cast`). `?:` with
   pointer arms is accepted; its result is a pointer value like any other.

8. **Scope exits.** At every way out of a block holding cells that b1a accepts (fall-through, reported at the block's
   closing brace; `return`, reported at the `return`), innermost scope first and cells in reverse declaration order,
   lowering writes item 6's scope-exit check and exhales the cell, its flag and its liveness cell. A `return` computes
   its value first, then leaves the scopes, then checks the postconditions (0012 item 9). In b1a no function can keep a
   cell, so the check cannot fail; its must-fail twin (a callee that consumes the cell) is b1b's first test.

9. **Writes and sequencing** (amends 0020 item 4 until b1c). A cell is written only as the top-level operation of an
   expression statement (`x = e;`, `*p = e;`, `x += e;`, `(*p)++;`), with the address and the value lowered in one full
   expression, and the value containing no write to a cell. The address and the value of a store are unsequenced
   operands for 0020's rule on names (C's rule in both modes; C++17 sequences the value first, the stricter C reading
   is taken), so `*p = (p = &y, 1)` and `*(p = &y) = *p` are rejected (`unsequenced_modification`). No function can reach a cell (none takes a pointer), so a
   call's effect never touches a cell and 0025 item 2's placement is unaffected. A full expression that writes a cell
   anywhere else is rejected (item 10).

10. **Rejected until b1b or b1c** (fail closed; each pinned by a test the status entry lists for its session):

    | Construct | Diagnostic | Lifted by |
    |---|---|---|
    | a pointer parameter or result | `unsupported_type` | b1b |
    | `&param` | `unsupported_expression` | b1b |
    | a pointer to a pointer; the address of a pointer local | `unsupported_type`, `unsupported_expression` | b1c |
    | a write to a cell inside an expression (`y = x++`, `*p = (*q)++`, `f(x = 1)`) | `unsupported_expression` | b1c |
    | a loop (its condition, body or step) holding a pointer-typed node, a cell, `&` or `*` | `unsupported_statement` | b1c |
    | a `return` inside a loop while a cell of an enclosing block is live | `unsupported_statement` | b1c |
    | a `goto`, `break` or `continue` that leaves a block holding a cell (one that declares a cell anywhere: with allocation at block entry, a cell declared after the jump is live too) | `unsupported_statement` | b1c |

    "A loop" is every loop form, `do … while (0)` included. A pointer copy used as a value (`if ((p = q))`) is a
    write to a Viper local, not to a cell, and is accepted with its liveness check. A loop body sees only its invariant in Viper, so a pointer used there would fail its liveness check and a cell its
    permission check: loops wait for b1c's permission invariants. `check_program` repeats the rules as a backstop
    (exit 3).

11. **The lint** (0028 item 2): `lint_viper.py` accepts a `domain` only after `// cerastes:domain-ok <record>` and an
    `axiom` only after `// cerastes:axiom-ok <record>`, and pins them: the domain is `Block` with `domain-ok 0028`, the
    axioms exactly the four of records 0033–0036, each with its own record and its exact text (audit round 1); it pins
    `alloc_<t>`'s two `ensures` to item 4's text (audit round 2); it refuses Viper's `new` (its results would
    not be `WF`), and accepts a field flag write `r.d_<t> := true` standing alone on its line, directly after the value
    write `r.v_<t> := …` of the same receiver and type (any other value of a field flag is refused).

12. **Categories** (amends 0011 item 10, in its five copies): `null-dereference`, `permission`, `lifetime`. The other
    five of 0028 item 10 land with their sessions.

13. **Tests and docs.** `tests/memory/` (dual-mode `.c` where the code is C and C++), each rule with a must-verify test
    and a must-fail twin (invariant 2), except item 8's scope-exit check, which cannot fail in b1a (item 8). The
    "Memory and ownership" page starts; "Error messages" gains three sections. The emitted Viper is compared with the
    fixtures' shapes (`il-inventory.md`, "M4.5b1a").

## Consequences

- The first programs with pointers verify: an object whose address is taken, read and written through a pointer, and
  dangling pointers and null dereferences reported, all inside one function.
- The prelude gains its first domain, axioms and fields, only for programs that use them.
- Pointer parameters, loops over memory and several jumps stay rejected until b1b and b1c.
- C's incompatible pointer conversions are errors in verify mode.

*2026-10-28, amended by 0037 (M4.5b1b): item 2 — pointer parameters and results are accepted (0037 items 2–3); `restrict` on a C parameter whose contract owns its pointee. Item 4 — `&param` is accepted (0037 item 4). Item 6 — the checks of a cell read or written by name are category `permission`, since a callee can now keep a cell. Item 7 — the one-past-the-end row is category `pointer-comparison`. Item 10 — the table's first two rows (a pointer parameter or result; `&param`) are lifted by 0037; the others stay b1c's, and 0037 item 15 adds its own rejections.*

*2026-10-29, amended by 0038 (M4.5b1c1): item 4 — a `CERASTES_CHECK`, `assert` or loop invariant may take `&x` only of a variable the code takes the address of: the front end does not flag a use in an annotation (P59), so `check(&x != &y)` holds only when the code takes both addresses; otherwise `unsupported_expression`. Items 8 and 10 — the loop, `return`-in-loop and jump rows of item 10 are lifted: loops over memory get 0038 item 3's automatic invariant and jumps write the scope exits of the blocks they leave (0038 item 7); the `T**` and writes-inside-expressions rows stay, lifted by b1c3 and b1c2.*

*2026-10-29, amended by 0039 (M4.5b1c2): item 9 — a cell may be written inside an expression (`AssignE` on a cell, `StoreE`; 0039 item 2); item 10 — the row "a write to a cell inside an expression" is lifted.*
