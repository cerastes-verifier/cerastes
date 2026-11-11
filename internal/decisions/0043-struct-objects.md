# 0043 — Struct objects (M4.5c1)

- **Status:** accepted for M4.5c1 (2026-11-02), written before code (invariant 6), revised after P65 (items 2, 5, 6, 9),
  P66 (item 12), S33 (items 8, 11) and audit round 1 (Opus, on the draft: four blocking items, B1 overlapping atoms on one
  root, item 9; B2 `restrict` through a loaded pointer, a hole since 0037 that the auditor reproduced, item 10; B3 struct
  atoms and copies in the sequencing rules, items 7 and 12; B4 overriding designators, item 6; should-fix S1–S11 and notes
  N1–N5, all taken in items 2–13 and 17), and audit round 2 (on the code: B1 an initializer writing its own object,
item 6; S1 `restrict` through a pointer to a structure, item 10; S2 a struct-pointer global's row in `program_start`,
item 8; S3 `#pragma pack`, item 2; notes in the handoff; `build/m4.5c1/audit1.md`), and its re-audit (all closed; a
C++ compound-literal initializer, item 15, taken). During the session the owner answered question 94 (a test's
second error after a contract that fails framing is not expected, 0037 item 7). Owner decisions (M4.5c1 planning, 2026-11-02; plan `.claude/plans/m4-5c1-struct-objects.md`; handoff
  questions 90–93):
  - **90:** M4.5c is two sessions: this record's struct objects (with copies inside a function, the pre-agreed cut to
    M4.5c2 if the session runs short), and M4.5c2's struct values across calls, struct globals and places reached through
    a heap read.
  - **91:** owning fields (`[[cerastes::owning]]` target `d`) move to M4.5d2, which also covers the rest of P4418R0's
    invalidation profile.
  - **92:** a struct copy reads every member; an uninitialized member is `uninitialized` at the copy (item 7), stricter
    than C, which lets a struct with indeterminate members be copied.
  - **93:** the flips of item 18.
- **Milestone:** M4.5c1 (2026-11-02).
- **Implements:** 0028 item 16 (member blocks) for C structures in automatic storage and through pointers; 0028 item 7's
  `owns(*s)`; 0042's scope row "structs as marked locals".
- **Amends:** 0028 item 1 (struct locals are blocks whether or not their address is taken, item 4), 0028 item 16 (lengths
  of member blocks as part of `WF`, item 8), 0029 item 1 (owning fields to M4.5d2), 0037 items 2 (`restrict`, item 10), 5
  (places, item 9), 6 (amounts), 7 (framing), 8 and 10 (calls and returns by leaf), 0038 item 3 (loop places by leaf,
  item 11), 0039 item 3 (object keys with member paths, item 12), 0040 items 6 and 9 (rows renamed to M4.5c2 or M4.5d2),
  0041's scope row (struct globals to M4.5c2), 0042's scope row.

## Context

Until this record every class, struct and union in a verified file is rejected (`unsupported_declaration`, 0012 item 12),
and so is every pointer to one (0032 item 2, 0040 item 2), `.` and `->`, an aggregate initializer, and every contract
place other than `*p` of a pointer parameter, a global and `*result` (0037 item 5, 0040 item 6, 0041 item 7). Record 0028
item 16 fixed the layout: a struct object is a slot, each member a block of its own (`member(r, k)`), and S26 measured it
on Silicon (`tests/viper-smoke/memory/s26_member_*`), with the fourth axiom `member_lv` giving every member its enclosing
object's liveness cell. Records 0035 and 0036 hold the two member axioms, already in the emitted prelude. Facts: P51 (the
IL of `&s.f`, `p->f`, `(*p).f`, a struct copy), P65 (this session's IL), P66 (the compiler matrix) and S33 (Silicon), in
`internal/il-inventory.md`.

## Decision

1. **Scope.**

   | Construct | Session |
   |---|---|
   | C structures as types; struct locals; `.`, `->`; nested structs; pointers to members; braced initializers; copies inside a function; `struct S *` objects; contract atoms and values on struct places; P4222's markers on struct objects | M4.5c1 (this record) |
   | by-value struct parameters and results, `f().x`; struct globals and `static` struct locals; places reached through a heap read (`owns(**pp)`, `reads(**pp)`, `live(*pp)`, reads of `**pp` and `*gp`, `owns(*gp)`, `*s->q`, an atom on `*q` where `q` is a cell); compound literals | M4.5c2 |
   | owning fields (`[[cerastes::owning]]` target `d`) | M4.5d2 |
   | array members and arrays of structs; `&s.x + 1` | M4.5e1, M4.5e2 |
   | `const` members | M4.6 |
   | pointer-to-function members | M4.7 |
   | C++ class features (member functions, constructors, bases, access, `static` members, default member initializers, references) | M7 |
   | unions, bit-fields, `volatile`/`_Atomic` members, anonymous members, block-scope struct definitions, `restrict` on a struct pointer | M8 |

2. **Which structures.** A *verified structure* is a complete `struct` defined at file scope (namespace scope in C++) in
   a verified file, with at least one member, each member one of:
   - a `bool`, an integer or an enumeration that 0032 item 2 accepts for a local;
   - a pointer type 0040 item 2 accepts, the innermost pointee now allowed to be a verified structure (so a
     self-referential `struct node *next` is a pointer member);
   - a verified structure (a nested struct member).

   In C++20 the same aggregates are accepted when every special member is implicitly declared and trivial (P65 says how
   the IL shows it). Everything else is rejected with its own message naming its session (item 1): a union; a bit-field;
   an array or flexible array member; a `const`, `volatile` or `_Atomic` member; an anonymous struct or union member; a
   `void *` member (0028 item 1); a pointer-to-function member; an empty struct; a structure declared in a system header
   (`FILE`; as 0018 does for system enumerations); a struct definition at block scope; any C++ class feature of item 1;
   a template (M5); an incomplete struct type used as an object or dereferenced; a `packed` struct (M8), by the attribute
or by `#pragma pack`, that is any structure with a member whose offset is not a multiple of its type's alignment or
whose own alignment is below its largest member's (audit round 2, S3: a misaligned member's address is as unusable as
under the attribute); a C23 compatible
   redefinition of a tag (N3037; M8). In C++ a verified structure has only public non-static data members: friends, nested
   types and typedefs, `mutable` and `[[no_unique_address]]` members are rejected (M7). A pointer to a struct type that is
   not a verified structure, including a pointer to an incomplete struct, is rejected wherever it appears
   (`unsupported_type`). A `const`-qualified struct object or pointee is a `const` object (M4.6): a `const struct S` local
   is rejected, and `owns(*p)`, `owns(p->x)`, `owning` and the markers on a `const struct S *p` are rejected as 0040 item
   2 rejects them on a `const int *` (`reads` stays accepted).
   **How the IL tells them (P65):** `tk_union`, a field's `is_bit_field`, `tk_array` (a flexible member: the class's
   `contains_flexible_array_member`), a member's typeref qualifiers (`const`, `volatile`, `_Atomic`), a field's
   `is_anonymous_parent_object`, `void`/routine pointees, `is_empty_class`, `incomplete`, `is_packed`,
   `is_local_to_function`, a declaration position in a system include directory; in C++, every routine of the class scope
   `compiler_generated`, trivial and not deleted, no base classes, no virtual member, no `has_field_initializer`, no
   `static` member, every field public. A copy's triviality is decided by this class-level test, never by the expression
   (a class with a user destructor still copies by `eok_assign`, P65 H2). Member `k` is the position in the class's
   `field_list`. **Names:** a struct's cell and allocation names use its tag, else the typedef that names it (`typedef
   struct { … } P;`, nameless in C23 and named in C++20, P65 H5), else `anon`, with `<n>` the struct's index in the
   program, unique across C++ namespaces.
3. **Layout** (0028 item 16, S26). A struct object is a slot `r`. Member `k` (in declaration order) is the block
   `member(r, k)` of length 1: a scalar or pointer member is the **leaf cell** `loc(member(r, k), 0)`, with the value and
   flag fields of its type (0028 item 1, 0040 item 2); a nested struct member is the **slot** `loc(member(r, k), 0)`,
   whose own members are blocks of their own. A struct type has no value field: an object's value is the values of its
   leaves. Every leaf of an object shares the liveness cell of the object's root block (`member_lv`: `lvloc(member(r, k))
   == lvloc(block_of(r))`). The field of a pointer to a struct is named after the structure, like an enumeration's:
   `v_ptr_struct_<n>_<tag>`.
4. **Struct locals are blocks** (amends 0028 item 1 for structures). A local of a verified structure type is a block
   whether or not its address is taken: one layout everywhere (0028 item 16), the allocation, scope exit and loop rules
   of 0032, 0038 and 0042 reused as they are, and no program accepted before this record has a struct. At block entry it
   gets a fresh block from a trusted allocation `alloc_struct_<n>_<tag>()` (`// cerastes:trusted-contract`), whose
   `ensures` are the root facts of 0032 (`r != null`, `r == loc(block_of(r), 0)`, `len(block_of(r)) == 1`,
   `lvloc(block_of(r)) == loc(block_of(r), -2)`, full permission to the liveness cell) and, for each member block,
   `len(member(X, k)) == 1`, for each leaf `L` full permission to `L`'s value and flag with the flag false, and the
   **slot facts** `X != Y` for every nested slot `Y` (at any depth) below a slot `X` (the root `r` included; S33 (i):
   the four axioms admit a cyclic model `r == loc(member(r, 0), 0)`, in which `r.t.y` and `r.z` are one cell, and a nested
   struct held by fractions then fails at a loop's entry with Silicon's own reason; with the facts it verifies). At every
   way out of its scope (0038 item 7) every leaf and the liveness cell are checked at `write` (`lifetime`, "the address
   of `s` (or of `s.x`) may still be in use when its lifetime ends") and exhaled. Consistency: the allocation's facts hold
   in 0028 item 2's model for a fresh block (the member blocks are fresh terms, `len` is free), and the lint (item 13)
   admits each leaf exactly once, so no `ensures` asks for two full permissions to one cell.
5. **Member access in code.** `s.x`, `p->x`, `(*p).x`, `s.t.x` and `p->t.x` designate leaf places; their address is the
   member-address chain `loc(member(…loc(member(b, k1), 0)…, kn), 0)` over the base pointer `b` (`&s` for a local).
   - **A read or a write through a member** checks, each one explicit and atomic (0028 item 11): `b != null`
     (`null-dereference`, "dereferences `p`, which may be null", for `p->x`; never for a local), the leaf's permission
     (`permission`, "reads `p->x` without owning it", "writes `p->x` with read permission only"), and for a read the flag
     (`uninitialized`, naming `p->x`). The null check is on the base pointer: `loc(…)` is never null (`loc_nonnull`), so
     a check on the leaf's address would turn a null dereference into a permission error. Like `*p`, it is not a use of
     `p`'s value and needs no liveness check (0032 item 6): through a dangling `p` it fails `permission`.
   - **The address of a member** (`&s.x`, `&p->x`, `&s.t`, `&p->t`, and `&s` itself) is a pointer value. Forming `&p->x`
     evaluates `*p` as an lvalue (undefined for a null pointer and for one past the end, C J.2) and is arithmetic on `p`
     (0028 item 3a): it checks `p != null && offset_of(p) < len(block_of(p))` (`null-dereference`, "forms `&p->x`, but
     `p` may be null or point past the end of its object"), then `p`'s liveness (`lifetime`), then builds the chain. `return
     &s.x;` and `return &s.t;` of a struct local are R14's `return_local_address` (0039 item 6; clang warns at no flag).
   - **Not a member access** (P65): `s.x`, `p->x` and `(*p).x` without `&` do not set `s`'s IL `address_taken`; `&s`,
     `&s.x`, `&s.t.y` and `(&s)->x` do. Item 4 makes every struct local a block regardless.
   - **Writes:** `=`, compound assignment, `++`/`--`, and writes inside expressions (0039 item 2) work on a leaf as on any
     cell, the flag set after the value (`lint_flags`).
   - **Still rejected:** `&s.x + 1` and every other arithmetic on a member pointer (M4.5e2), relational comparisons of
     member pointers (M4.5e2; `&s.x < &s.y` will be reported `pointer-comparison` there, the accepted completeness gap of
     0028 item 16), every conversion between struct pointer types and between a struct pointer and another object pointer
     (0028 item 1; `(int *)&s` would make `&s` and `&s.x` equal in C and different in the model), and `sizeof`/`offsetof`
     forms that do not fold to a constant (P65: every `sizeof` and `offsetof` of a verified structure folds, with EDG's
     ABI values, which are not the model's member blocks: M4.5d2 sizes allocations by the model). A discarded whole-struct
     expression (`(void)s;`, `*p;`) is rejected (`unsupported_expression`).
6. **Initializers** of a struct local. A braced list — positional, designated (in C++20 in declaration order), nested or
   with brace elision, `{}` and `{0}`, and C++'s `S s{}`, `S s = {}` and `S s{…}` — stores every leaf: the listed value,
   or zero (null for a pointer, `false` for `bool`) for a leaf the list does not mention, as for an object with static
   storage, recursively through nested members (P65 N9: `{0}` over a first member that is a struct elides braces); each
   store sets its flag. **IL** (P65): `dik_constant` or `dik_nonconstant_aggregate` over a `ck_aggregate` in source order,
   `ck_designator` elements naming the field, trailing members absent, non-constant elements as `ck_dynamic_init` holding a
   `dik_expression`; C++20 designated lists carry compiler-generated zero fillers without a position (H1), which no
   duplicate check counts.
   - **Each element is an assignment conversion** to its leaf's type, checked like a scalar initializer (0022): EDG folds a
     constant element to the leaf's type (`{300}` for an `unsigned char` leaf is `(unsigned char)44U`, the original only in
     the constant's `orig_type` and `expr`; audit S7), so the encoder reads the original value under the fold, and `{300}`
     is a `conversion` error as `unsigned char c = 300;` is.
   - **A subobject initialized twice is rejected** in C (audit B4: C23 lets an overridden initializer go unevaluated, clang
     does not evaluate it, EDG keeps both; `{.y = bump(&g), .y = 2}`, and DR 413's `{.t = u, .t.x = 3}`); C++ refuses it
     itself. EDG warns #2904-D on out-of-order designators (H6); an out-of-order list without an override is accepted.
   - **Order.** In C the elements are indeterminately sequenced with one another and the object has no value until the
     whole list is evaluated: every element is evaluated (into temporaries, as one full expression's operands under 0039's
     and 0021's rules, shapes the compilers warn on rejected) before any leaf is stored, so `struct S s = {1, s.x};` reads
     an uninitialized `s.x`. In C++ the elements initialize their members in order ([dcl.init.aggr]), each a full
     expression stored before the next is evaluated.
   - **The object being initialized is not written** (audit round 2, B1): an element that writes a member of the object
     being initialized, or takes the address of one (`{.x = 1, .y = (s.x = 6)}`, `{.x = 1, .y = g(&t.x)}`, `{.x =
     g(&s.y)}`), is rejected in both modes (`unsupported_declaration`, "a local variable used in its own initializer"):
     in C the compilers do not follow "every element first" once an element writes the object (GCC and clang return 6
     where that order gives 1), and in pass:[C++] such a write precedes the member's lifetime. Reading a member (`{1,
     s.x}`) stays as above.
   - **Rejected:** C++'s parenthesized aggregate initialization `S s(1, 2)` (narrowing rules of its own; M7), and
     initializers that are temporaries, `S s = S{…}` and `S s = S()` (struct values: M4.5c2).
   - **Rejected like the compilers** (0020 item 5's union rule; the native-ok policy compiles every positive with `-Wall
     -Wextra -Werror`): brace elision over a nested struct (`struct T t = {1, 2, 3};`: clang's and GCC's `-Wmissing-braces`
     at `-Wall`), except C's universal zero initializer `{0}`, which no compiler warns on in C; and in C designators out of
     declaration order (`{.y = 1, .x = 2}`: valid C, but EDG warns #2904-D in both modes and C++ requires the order;
     over-strict in C, listed for M8's warning sweep). A positional list shorter than the struct (`{1}`) warns only at
     `-Wextra` (`-Wmissing-field-initializers`) and is accepted; C++'s `{0}` likewise.
   - An initializer that is another struct object (`struct S t = s;`, `= *p`) is a copy (item 7).
7. **Copies** (question 92). `t = s`, `*p = *q`, `*p = s`, `s = *p`, `p->t = s.t` and an initializer from an object copy a
   structure leaf by leaf. The source is an lvalue of the structure's type (`s`, `*p`, `p->t`, `s.t`); a `?:`, comma,
   assignment, call or compound literal source is rejected (struct values: M4.5c2). First the bases' checks (`p != null`
   for each base pointer, `null-dereference`, before any leaf: `loc_nonnull` would otherwise make them `permission` errors),
   then every source leaf is read (its permission and its flag: `uninitialized`, "copies `s`, whose member `s.y` may not have
   a value"; a pointer leaf is also a use of its value, with 0028 item 3a's liveness check, `lifetime`, stricter than C
   like the flag), into temporaries, then every destination leaf is written (its permission at `write`, the flag set). So an
   exact self-copy (`*p = *p`, which C allows) verifies, and partial overlap cannot arise (two struct places of one type
   are the same slot or disjoint). A copy never clears a flag, which keeps 0042 item 5's `old(flag) ==> flag` row and the
   loop row `held && flag ==> range` sound.
   - **Where:** a copy is an expression statement, a declaration's initializer (a `for` init included), or a discarded
     operand of a comma whose value is not used; anywhere else (`(t = s).x`, `u = (t = s)`, `(t = s, 0) + t.x`, and a `for`
     step, which cannot hold the temporaries; over-strict, listed) it is rejected (audit B3).
   - **Sequencing:** a copy writes every destination leaf and reads every source leaf, for 0039's keys, its static rule and
     its path-precise checks (item 12).
   - **C++:** a copy the IL shows as a call of a special member (`operator=`, a copy constructor) is rejected (M7); item 2's
     class test admits only trivially copyable structures, which the IL copies by `eok_assign` or a `dik_expression` of
     the source (P65).
8. **Struct pointers.** `struct S *` locals, parameters, results, and pointer cells (`struct S **`, 0040 item 2), with
   null, `==`, `!=`, truth tests, copies of the pointer, arguments and results, and the case splits of 0037 item 3 and
   0040 item 2 by the pointer's own field. **`WF` of a struct pointer** includes the lengths of its object's member blocks:
   `WF_S(p)` = `WF(p) && (p != null && offset_of(p) < len(block_of(p)) ==> len(member(p, k)) == 1` for every member `k`,
   recursively through nested members, `&&` item 4's slot facts `X != Y` for every nested slot `Y` below a slot `X` of
   `*p`, `p` included`)`. The slot facts hold in 0028 item 2's term model (a slot term is a proper subterm of every slot
   below it), and a caller proves `WF_S(&t.s)` from `t`'s own facts (the pairs below `t.s` are among `t`'s). Every struct object the program can reach is a slot whose member blocks have
   length 1 (its allocation says so; M4.5d2's typed view of allocated storage must too); the guard leaves a one-past-the-end
   pointer, which designates no object (audit S3), without the claim. So `WF_S` is an invariant of trusted methods like 0028
   item 1's "different fields never alias", and it makes `&p->x` provably `WF` wherever `p` points to an object (a
   struct-pointer global of unknown initial value gets the row `WF_S` in `program_start`, which the lint pins like the
   allocation's facts; audit round 2, S2) (S33 (g):
   without the lengths only `offset_of(q) <= len(block_of(q))` of the callee's `WF(q)` fails).
   By-value struct parameters and results are rejected, naming M4.5c2.
9. **Contracts** (amends 0037 items 5–8, 10).
   - **Places:** for a parameter `p` of type `struct S *`, `*p` (the whole object), `p->x`, `p->t`, `p->t.x` (and the
     same through `(*p).`); in an `ensures`, the same places of a pointer result (`r->x`), framed as 0040 item 4 frames
     `*r`. A place is a **root** (a formal, the result) and a **member path**.
   - **Atoms** (each also `p != null`, `WF_S(p)` and `offset_of(p) < len(block_of(p))` for its root `p`):
     - `owns(*p)`: for every leaf `L` of `*p` full
       permission to its value and flag, the flag set, and its range (or `WF`, or `WF_S` for a struct pointer leaf); one
       read fraction of the liveness cell.
     - `reads(*p)`: the same with the atom's ghost amount on every leaf's value and flag.
     - `live(p)`: as 0037 item 6, with `WF_S(p)`.
     - `owns(p->x)`, `reads(p->x)`: one leaf; `owns(p->t)`, `reads(p->t)`: the leaves of the nested object; each with one
       liveness fraction, summed with the others that share the root (S26's `swap_adjacent`).
     - **Amounts** (0037 items 6, 8): the caller's `m_k` for a `reads` atom is the minimum over every leaf's value and flag
       cell and the liveness cell; the sum checks at calls and the return checks are per leaf cell.
     - **Overlapping atoms on one root are rejected** (audit B1): in one clause list, two atoms whose places are on one
       root with one path a prefix of the other (`owns(*p) && owns(p->x)`, `owns(*p) && reads(p->x)`, `owns(p->t) &&
       reads(p->t.x)`) would ask twice for one cell, a `requires` nothing satisfies, and make `f__wf`'s distinctness fact
       `L != L` false; they are rejected (`unsupported_annotation`). Two atoms on one place keep 0037 item 5's rule and 0039
       item 7's `owns`/`reads` exclusion. `owning` on a struct pointer parameter beside any atom on its members, and a
       member atom beside a marker on its root, are rejected too.
     - `[[cerastes::owning]]` on a `struct S *` parameter is 0040 item 3's borrow `p == NULL || owns(*p)`.
     - Each atom has one ghost amount (0037 item 6); two atoms of one kind on one place (root and path) in one clause list
       are rejected; an `ensures` atom's partner is the `requires` atom of the same kind, root and path.
   - **Values:** `p->x`, `p->t.x`, and `old(p->x)` (`cerastes_old(p->x)`, `cerastes::old(p->x)`). A whole struct value in
     a contract (`old(*p)`, `*p` compared or passed) is rejected, and so is `old(*p).x` (P65: a member of the old whole
     object; the message points to `old(p->x)`). A function's `decreases(p->x)` is rejected (M4.5c2); a loop's invariants
     and measures may read member leaves, framed by the automatic places (0038 item 6), per leaf. `p->q->x` (through a
     pointer member) is a place reached through a heap read (M4.5c2). **`f__wf`'s ghost values** are keyed by root, path and
     state, and its equal-ghost and distinctness facts are per leaf (audit S1: keyed by root alone, `p->x` and `p->y` would
     share one ghost, and a contract comparing `old(p->x) == old(p->y)` would frame an unframed read).
   - **Overlap by slots, never by the atom's pointee type** (the soundness core). An atom on `struct S *s` and one on
     `int *q` share a cell when `q == &s->x`; two struct atoms share cells when one root is the other's nested slot.
     Every rule that today compares the atoms' pointee field (`exclusive_pair`, the amount sums of 0037 item 6, the entry
     separation of 0037 item 9, `f__wf`'s distinctness and equal-ghost facts, framing disjuncts, the case splits of 0037
     items 3 and 8, 0038 item 6 and 0040 item 2, the havoc rule, loop places, 0039's memory calls and keys) works per **leaf
     cell and its VIR field** (so `long` and `long long` leaves compare, N2). Two atoms overlap iff a slot or leaf of
     one equals a slot or leaf of the same type of the other: a disjunction of `Ref` equalities between the shorter
     member chains (`q == loc(member(s, 0), 0)`, `s == loc(member(t, 0), 0)`), never a comparison of field names. A read
     of a leaf is framed by an atom covering it with the disjunct `g && place == leaf`, one per covering leaf of that
     field. Comparing only slots of the same struct type is an optimization: a missed pair may cost Silicon's own error
     (exit 3), never an assumed distinctness fact.
   - **The markers** (0042) on a `struct S *` parameter apply to every leaf: `ref_to_uninit` drops every leaf's flag on
     entry (each range under its flag) and keeps each initialized leaf initialized (`(g && old(flag)) ==> flag`, per
     leaf); `must_init` promises every leaf on return (`postcondition`, "returns without initializing `p->y`, which
     `cerastes::must_init` on `p` promises"). `[[cerastes::uninit]]` is accepted on a struct local without an initializer
     and emits nothing; `--strict-init` asks for it on every struct local without an initializer (`uninit_unmarked`), and
     its argument rule (0042 item 8) accepts `&s` of an `uninit` struct local; `&s.x` and `&s.t` of one are
     `uninit_ref_initialized` (over-strict, listed). 0042 item 6 extends to member paths (audit S5): of a marked parameter,
     a read of `p->x` or `(*p).t.x` in a `requires`, `old(p->x)`, and a read of a member in a `ref_to_uninit` parameter's
     `ensures` are rejected, and a marked site frames no read of any of its leaves; a marker needs a whole `owns(*p)`.
10. **`restrict`** (amends 0037 item 2, whose argument "parameters of another pointee type never reach `p`'s cell" no
    longer holds: an `int *q` can point at `s->x`; and audit round 1, B2, found it already broken by pointers loaded from
    memory: `int f(int *restrict p, int **pp)` with `owns(*p) && owns(*pp) && *pp == p` and `*p = 1; **pp = 2; return
    *p;`, and the same through a pointer global `gp == p`, verify `result == 2` today, where clang and GCC at `-O2` return
    1). Until M8 models `restrict` (fail closed; no test uses these shapes):
    - `restrict` stays C-only and only on an owned parameter, and is rejected on a struct pointer parameter (M8);
    - a `restrict` parameter whose pointee has field `T` needs every other pointer parameter whose object may contain a
      cell of field `T` (the same pointee type, or a struct pointer with a leaf of field `T`, at any depth) to hold a whole
      `owns(*q)` or `reads(*q)` of its own (a member atom is not enough: a code test `p == &s->x` can make the alias hold);
    - it is rejected beside any parameter, atom or global of the function's contract whose object can hold a pointer
      from which a `T*` can be formed: a `T*` (a `T**` at any depth, a struct with a `T*` leaf, a `T*` global) or a
      pointer to a structure with a leaf of field `T` at any depth (`struct S **`, a struct with a `struct S *` leaf, a
      `struct S *` global; audit round 2, S1: `&(*pp)->x` is an `int *` not based on `p`), since a pointer loaded from
      there is not based on `p`.
11. **Loops** (amends 0038 item 3). Every leaf of every struct object in scope, and every leaf of a held struct place of a
    parameter, is an automatic place of its field, with its own value, flag and range rows. The liveness rows are written
    once per distinct liveness cell (one per root block, under 0038's guard `lvloc(block_of(a)) != lvloc(block_of(b))`
    against the earlier roots, not a comparison of the roots), never once per leaf: two rows over one cell at the snapshot
    amount ask for twice what the function holds (S33 (d)). The havoc rule (a write reaches the automatic places of its
    field) covers member leaves, so `*q = 1` with `int *q` reaches every `int` leaf, and a copy or a call with a struct
    atom reaches every leaf field it writes or owns. The case splits after a load, after a pointer result and on the
    pointers a loop assigns (0037 item 3, 0038 item 6, 0040 item 2) compare against leaves and nested slots of the
    pointer's field.
12. **Sequencing** (amends 0039 items 2–4). Object keys gain member paths: `s.x` and `s.y` are different objects, `s.x`
    named twice is one object, `s.t.x` and `s.t.y` differ. A shape that names one member object twice with a write is
    rejected `unsequenced_modification` (P66: GCC's `-Wsequence-point` warns on all of them at `-Wall` in both modes, clang
    on none; 0020 item 5's union rule): `s.x + (s.x = 1)`, `s.x++ + s.x`, `(s.x = 1) + (s.x = 2)`, `s.x = s.x++`, `p->x +
    (p->x = 1)`, `p->x = p->x++`, `s.t.x + (s.t.x = 1)`. Different members (`s.x + (s.y = 1)`) need no check; accesses
    through different bases (`p->x` and `q->x`, `p->x` and `*r`) are checked path-precise (`unsequenced`, "… which may be
    one object"). A call whose contract holds a struct atom is a memory call on every leaf field of the atom, with an
    obligation per leaf (0039 item 4; audit B3: as one field `struct_S`, `s.x + set_x(&s)` would match nothing and the
    read would move past the call), and a copy is a write of every destination leaf and a read of every source leaf.
13. **The lint** (`lint_viper.py`). The allocation `alloc_struct_*` is pinned by its shape, dispatched before the
    `alloc_<t>` rule (whose suffix pattern would match `struct_0_S`): the root facts as `alloc_<t>`'s; exactly one `lv`
    permission, on the literal `lvloc(block_of(r))` (`member_lv` makes every member's liveness the same cell, so a second
    one would make the allocation unsatisfiable); for each member block `len(X) == 1` with the literal 1; for each leaf
    exactly `acc(L.v_t) && acc(L.d_t) && !L.d_t` with one suffix `t`, declared; every chain in the canonical grammar
    `loc(member(<chain> | r, <non-negative literal>), 0)` (no `block_of`, `parent_of` or other offset inside a chain); every
    (receiver, field) pair and every member block once; item 4's slot facts `X != Y` exactly for the pairs where `Y`'s chain
    strictly extends `X`'s (true in the term model by construction); nothing else. A self-test refuses a duplicated leaf, also when
    spelled `loc(member(loc(block_of(r), 0), 0), 0)`, a missing `!d`, an extra conjunct and an unrooted chain. `lint_flags`
    is unchanged: lowering writes a member leaf through a `Ref` temporary, and a cell's flag is set only by `:= true` right
    after its value.
    **Cyclic models** (audit N1, S33 (i)): the slot facts of items 4 and 8 are what separates leaves at different depths
    under fractions.
14. **Categories:** none new. `permission`, `uninitialized`, `null-dereference`, `lifetime`, `postcondition` and
    `unsequenced` gain member messages; every message names the place in source terms (`s.t.x`, `p->y`).
15. **Still rejected** (fail closed; each pinned by a test): item 2's list; item 5's; items 6's and 7's; item 9's
    overlapping atoms, `old(*p).x`, `decreases(p->x)` and marked member reads; item 10's `restrict` shapes; by-value struct
    parameters and results, struct globals and `static` struct locals, compound literals, and the places of 0040 item 6
    and 0041 item 7 reached through a heap read (renamed to M4.5c2); `owning` on a field (EDG's own refusal, registration
    target `p`; M4.5d2); a whole struct value in a contract; a written atom in a loop invariant on a struct place
    (`owns(*q)` for a `struct S *q`, `owns(q->x)`; 0040 item 5's machinery is per cell; M4.5c2).
    **Diagnostics and positions** (fixed before the tests ran, so the pins are pre-registered): a rejection of a structure
    is `unsupported_declaration` at its definition's line — a union, an empty, packed (attribute or pragma), block-scope
    or parameter-list structure, a tag redefinition, and every C++ class feature (member functions, special members,
    bases, access, `static` members, friends, nested types, default member initializers) — and of a member kind at the
    member's line (a bit-field, an array or flexible array, a `const`, `volatile` or `_Atomic` member, a reference, a
    `void *` or function-pointer member; an anonymous member at its nested `struct {` line), a member's own rejection
    coming before the class-level one it causes (a `const` member deleting C++'s copy assignment); a by-value struct parameter or result `unsupported_type` at the parameter, or at the function's
    declarator; a struct global and a `static` struct local `unsupported_declaration`; C++'s `S s(1, 2)`, `S s = S{…}` and
    `S s = S()` `unsupported_declaration` at the declaration; brace elision, out-of-order C designators, an overriding
    designator, a copy used as a value and a discarded whole-struct expression `unsupported_expression`; the contract
    rejections (overlapping atoms, `old(*p).x` in both spellings, marked member reads, `decreases(p->x)`, `p->q->x`, a
    written struct atom in an invariant, `owns` on a `const` struct pointee) and item 10's `restrict` shapes
    `unsupported_annotation`.
16. **Facts relied on** (`internal/il-inventory.md`, "M4.5c1"): (P65) the IL of every construct of items 2–9 in both
    modes (PASS for all nine questions; hazards H1–H6 and notes N1–N9 taken above); (P66) the compiler matrix of member
    sequencing shapes and of uninitialized whole-struct uses (clang silent, GCC `-Wsequence-point` at `-Wall`; a copy of a
    partly initialized struct warns nowhere); (S33) Silicon on nested allocation, leaf overlap across pointee types and
    across nesting, loop liveness, copies, `must_init` over a struct, member-address `WF`, the vacuity runs, and (i)
    nested structs under fractions.
17. **Tests and documentation.** Must-verify and must-fail pairs for every accepted form in both modes (`tests/memory/`),
    including the leaf-overlap trap (`f(&s, &s.x)` with `owns(*s) && owns(*q)`), a struct field update (M4's acceptance),
    the ports of VeriFast's `tutorial_solutions/malloc_block.c` and `examples/swap.c`'s `point_mirror` with its aliasing
    twin; every rejection of item 15; audit round 1's pins: B1 (`owns(*p) && owns(p->x)` and `owns(*p) && reads(p->x)`,
    rejected), B2 (the `T**` and pointer-global `restrict` programs, and the struct one, rejected), B3 (`s.x + set_x(&s)`
    with `check(r == 1)` reported `unsequenced`, and `(t = s, 0) + t.x` rejected), B4 (`{.y = bump(&g), .y = 2}`, rejected),
    S1 (`old(p->x) == old(p->y) || *q > 0` with `q` possibly null: `permission` in the contract), S4 (a trusted `owns(p->x)
    ensures(p->x == 2)` on a `const struct S *`, rejected), S7 (`{300}` for `unsigned char` and `signed char`:
    `conversion`; `struct S s = {1, s.x};`: `uninitialized` in C, verified in C++), and the first-member pins (`(int
    *)&s`, C's implicit `int *q = &s;`, both modes). Docs: Memory and ownership ("Structures"), Annotations, Error messages, Verifying a
    program; `internal/encoding.md`.
18. **Flips** (question 93, pre-approved): `tests/scalar/negative/reject_struct.c` retired for a positive of the same
    shape; `tests/memory/negative/global_struct.c` loses its `@ L8` line (the struct definition is accepted; the global
    stays rejected, naming M4.5c2); `reject_struct_pointer.c`, `reject_compound_literal_address.c`,
    `reject_owns_double_deref.c`, `reject_read_double_deref.c`, `reject_read_double_deref_invariant.c`,
    `reject_atom_on_cell_pointer.c` and `global_pointer_global_deref_atom.c` re-messaged, their `expect:` lines kept.

## Consequences

- C programs whose objects are structures of scalars and pointers verify with member-precise ownership; VeriFast's
  `p->x |-> ?x` chunks are `owns(p->x)`, its struct predicates' fields `owns(*p)`.
- Separation is decided by the cells two atoms may share, so passing `&s` and `&s.x` to two owning parameters is an
  error whatever their pointee types.
- Every struct local costs a block and its permissions, address taken or not; no program accepted before this record
  changes its Viper.
- Struct values across calls, struct globals and contracts that read through a heap pointer wait for M4.5c2; owning
  fields for M4.5d2.

*2026-11-11, amended by 0059 (M4.5e1a): item 6 — EDG's #1162 (excess initializers) is an error in C verify mode, structure initializers included (0059 item 3).*
