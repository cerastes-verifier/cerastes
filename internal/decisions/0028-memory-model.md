# 0028 — The memory model for C (M4.5)

- **Status:** accepted for M4.5a (2026-10-27), finalized with S24, S25, S26, P51 and the owner's choices; revised after
  audit round 2 (Opus: five blocking items, eight should-fix items and six missing cases, all taken) and audit round 3
  (three blocking items: an object and its first member compared through `void*`, two calls ordered against each other,
  ownership transfer; and fourteen should-fix items, all taken), and its re-audit, round 4 (the first item reopened
  by C's implicit conversions between incompatible pointer types, closed in item 1; three should-fix items taken). Owner decisions:
  - **M4.5a planning** (2026-10-27): VerCors and its documentation as well as VeriFast and its documentation are the
    references (ideas only: VerCors is MPL-2.0; no file is copied); the M4.5 sessions may be split further than a–f;
    owning pointers, vlibc and P4222's markers are records 0029 and 0030.
  - **The encoding** (2026-10-27, after S24): **E2**, a pointer as a Viper `Ref` over a `Block` domain, chosen over E1
    (no domain, no arrays) and E3 (a Viper `adt`).
  - **Range-permission failures** (after S25): reported only on position-map rows that the encoder marks as permission
    checks over a range, which record the reason they expect; every other reason stays exit 3.
  - **Sub-objects** (after audit round 2, B1): each struct member and each inner array is a block of its own
    (`member(r, k)`, one more inverse axiom), so pointer arithmetic is bounded by the sub-object, as C requires; chosen
    over keeping a struct as consecutive slots and recording the deviation.
  - **Dangling pointers** (after audit round 2, B3): a **liveness token** per block, carried by `owns`, `reads` and a new
    spec function `live(p)`; every arithmetic, comparison and truth test of a pointer checks it. Chosen over recording
    the gap (VeriFast's behaviour) and over liveness without a new spec function.
  - **A member's liveness** (after S26): a fourth axiom, `member_lv`, makes a sub-object's liveness its enclosing
    object's (one liveness cell per top-level block), chosen over a liveness cell per member block.

  Facts marked (P*n*) and (S*n*) are probes and Silicon spikes, recorded with their evidence in
  `internal/il-inventory.md`, "M4.5a". The chosen encoding's battery is kept as `tests/viper-smoke/memory/`.
- **Milestone:** M4.5a (2026-10-27); implemented in M4.5b1 (pointers to scalars, address-taken locals and parameters),
  M4.5b2 (globals, `static` locals, initialization through pointers), M4.5c (structs), M4.5d2 (`malloc` and `free`),
  M4.5e1 and M4.5e2 (arrays and pointer arithmetic).

## Context

Today the encoder rejects every pointer type, `&`, `*`, `->`, `[]`, mutable globals and `static` locals (0012 item 12).
A value lives in a Viper local; there is no heap. M4.5 adds C's memory: objects whose address is taken, objects reached
through pointers, globals, structs, arrays and allocated storage.

**VeriFast** (the parity target, its own front end): a pointer is a provenance and an address; each C type has its own
points-to chunk (`integer(p, v)`, `p->f |-> v`), with a `_` twin for possibly uninitialized memory; a struct member
gets a sub-object provenance (`field_ptr`); arrays are recursive chunks over ghost lists; `malloc` produces the chunks
and a `malloc_block` token that `free` consumes; an address-taken local's chunk is consumed again at block exit; at each
function's end any remaining chunk is "Function leaks heap chunks". It checks pointer arithmetic only against the
provenance's limits, `p < q` and `p - q` not at all against the object, and nothing about a pointer whose object is
gone until it is dereferenced. One message, "No matching heap chunks", stands for a null dereference, a double free and
the free of a local.

**VerCors** (C through Viper): a pointer is `Option<pointer>`, a pointer a block and an offset in a Viper ADT prelude;
`*p` is `ptr_deref(ptr_add(p, 0))`, a `Ref` from `loc(block, offset)`, with one field per element type; `\pointer(p, n,
perm)` gives non-null, bounds and a quantified permission; a struct is an ADT of member pointers; `malloc` only as
`(T*)malloc(n * sizeof(T))`; `free` needs offset 0 and the whole block. It has no leak check, no scope-exit check
(`free(&local)` verifies, issue #1237), cannot form a one-past-the-end pointer, and shares one field between `int` and
`short` (issue #1357); two of its prelude axioms are false outside their triggers (an unguarded `stride > 0`, an
unguarded address bound). The Viper tutorial and Gobra use the same `loc` construction; Gobra guards its injectivity
axiom by the index range.

## Decision

### Part A: objects and pointers

1. **Blocks and cells.** Every object whose address is taken, or that is reached through a pointer, is a **block** of
   `len(b)` slots: an address-taken local or parameter, a mutable global, a `static` local, an allocation, and each
   struct member and inner array (item 16); later a string literal. Slot `i` is the Viper `Ref` `loc(b, i)`. A value
   lives in a **field per C object type, ignoring qualifiers** (`v_int` for `int`, `const int` and `volatile`-free
   variants alike; `v_uint`, `v_short`, `v_char`, `v_bool`, an enumeration by its own type, a pointer-typed object by its
   pointee type: `v_ptr_int`, …), and beside it 0019's flag as a ghost field of the same type (`d_int`, …; item 14).
   - **Different fields never alias.** Each slot holds permission in at most one value field at a time (an invariant
     every trusted method keeps: the allocations, `program_start`, vlibc's `malloc`, M4.5d2's typed view of allocated
     storage). So an access through another type finds no permission: C's effective-type rules (6.5p7) for distinct
     types, and unlike VerCors #1357. C also allows access through the signed or unsigned variant of the object's type
     and through an enumeration's compatible integer type; the casts that would express it between object pointer types
     are rejected, and so is the `void*` round trip (next point), so such an access cannot be written: no false error,
     no false proof. Access through a character type is rejected until M4.6, other casts until M7, a conversion between
     a pointer and an integer type until M8.
   - **`void*` is restricted** (audit round 3, B1). In C an object and its first member have the same address
     (6.7.2.1p17; `&a[1]` and `&a[1].x`, `&m[1]` and `&m[1][0]`), but item 16 puts them in different blocks, where
     `loc_inv` proves them unequal: a pointer that reaches `void*` from one pointee type and is compared with, or
     converted to, another would give a false proof. Until an address model (M8), a value of type `void*` may only be
     tested against null or passed on unchanged as an argument to a `void*` parameter; a `T*` is converted to `void*` only
     as a call argument (so a `free` or `memcpy` argument keeps its static pointee type for the callee's layout, item 6);
     and a `void*` is converted to `T*` only directly from the result of an allocation (`malloc`, `calloc`, `realloc`:
     M4.5d2's typed view; a `realloc` whose result is converted to a type other than its argument's is rejected,
     6.5p6). Any other conversion to or from `void*` (storing or returning a converted value, `void *m = p;`), any
     `free` or `owns_block` whose operand's static type is `void*` (outside vlibc's own contracts), and any `==`, `!=`,
     relational comparison or arithmetic with a `void*` operand other than a test against null, is rejected
     (`unsupported_expression`). Generic `void*` code is M4.7's and M8's.
   - **Every pointer conversion counts, written or not** (audit round 4). C lets EDG accept a comparison, an
     initialization, an assignment or an argument between incompatible object pointer types with only a warning (#42,
     #144, #167, and the assignment's), inserting a compiler-generated cast (`struct S *ps; int *px; ps == px`,
     `int *px = ps;`): the same false proof as above, with no `void*`. Item 1's rules apply to every pointer
     conversion node in the IL, compiler-generated or not, and C's verify mode makes those warnings errors
     (`--diag_error`, as GCC 14 makes `-Wincompatible-pointer-types` an error by default; M4.5b1), with must-fail tests
     for each shape and for C's implicit `int *q = v;` from a `void* v` (told apart from the allowed `int *p =
     malloc(n)` by its operand, a direct allocation call).
   - **A local or parameter whose address is never taken stays a Viper local:** every program accepted today keeps its
     Viper byte for byte (P51: how the IL tells an address-taken local). An address-taken parameter is copied into a
     fresh one-slot block at entry, like a local with an initializer.
2. **The encoding: E2** (S24, S26). A pointer is a `Ref`; NULL is Viper's `null`. The prelude holds one domain:

   ```
   domain Block {
     function loc(b: Block, i: Int): Ref
     function len(b: Block): Int
     function block_of(r: Ref): Block
     function offset_of(r: Ref): Int
     function member(r: Ref, k: Int): Block
     function parent_of(b: Block): Ref
     function index_of(b: Block): Int
     function lvloc(b: Block): Ref
     unique function GB_<global>(): Block          // one per mutable global or static local (item 5)
     axiom loc_inv    { forall b: Block, i: Int :: { loc(b, i) } block_of(loc(b, i)) == b && offset_of(loc(b, i)) == i }
     axiom loc_nonnull { forall b: Block, i: Int :: { loc(b, i) } loc(b, i) != null }
     axiom member_inv { forall r: Ref, k: Int :: { member(r, k) } parent_of(member(r, k)) == r && index_of(member(r, k)) == k }
     axiom member_lv  { forall r: Ref, k: Int :: { lvloc(member(r, k)) } lvloc(member(r, k)) == lvloc(block_of(r)) }
   }
   ```

   - **Four axioms, no more.** `loc_inv` makes `loc` injective: without it 10 of S24's 19 files change verdict
     (quantified-permission receivers must be injective, Silicon's `qp.not.injective`; a derived pointer's block and
     offset). `loc_nonnull` proves `&x != NULL` without holding `x`'s permission. `member_inv` makes sub-object blocks
     distinct and their receivers injective under quantifiers (S26: without it 7 of 13 files change verdict; arrays of
     structs). `member_lv` gives a sub-object the liveness cell of its enclosing object (item 3a): a member lives exactly
     as long as the object it belongs to. Each is triggered only by its own function applications, holds for every
     argument with no guard, and has a model: the free term algebra of `Ref` = `null` | `loc(Block, Int)` and `Block` =
     an allocated block | `member(Ref, Int)`, with `GB_` constants distinct allocated blocks, `lvloc(b) = loc(b, -2)` for
     an allocated block and `lvloc(member(r, k)) = lvloc(block_of(r))` by recursion on the term; `len` arbitrary. The
     trusted allocations and `program_start` state `lvloc(b) == loc(b, -2)` of the blocks they return or create. Every subset of the four axioms
     leaves a method that instantiates every term (the local allocation, `program_start` with `unique` globals,
     `malloc`, `forperm` included) and then asserts `false` failing with exactly that error, and an `assert false` at the
     start and end of every fixture method and after every trusted call fails too (S24, S26, the vacuity runs after
     audit rounds 2 and 3; "S26 final"). The argument is the model above; the runs check that no trusted postcondition
     contradicts it. The model is the encoding's, not C's addresses: an object and its first member are different
     there, which is why `void*` is restricted (item 1). **Each axiom gets its own record (the next free numbers when M4.5b1 writes them) and
     the prelude domain a lint tag** (`// cerastes:axiom-ok <record>` before each axiom, `// cerastes:domain-ok
     <record>` before the domain), implemented in M4.5b1; the lint keeps refusing any other `domain`, `axiom`, and
     Viper's `new` (whose results would not be `WF`).
   - **`unique` globals.** Viper's `unique` makes the `GB_` constants pairwise distinct with no user axiom, so `&g !=
     &h` is known without holding either (audit round 2: distinctness by permissions fails for two `reads`, for globals
     of different types, and for a function that holds neither).
   - **Valid pointers** enter a method through a contract clause, not an axiom: `WF(p)` = `p == null || (p ==
     loc(block_of(p), offset_of(p)) && 0 <= offset_of(p) && offset_of(p) <= len(block_of(p)))` is a `requires` of every
     pointer parameter, an `ensures` of every pointer result, and part of the permission to every pointer-typed cell
     (a store proves it of the stored value). A pointer the encoder builds with `loc` after its bounds check is `WF`.
     Forgetting the clause costs completeness, never soundness (`WF` is only ever a hypothesis proved elsewhere;
     `e2_load_ptr_nowf_fail.vpr`). A surjectivity axiom would do the same work and is not adopted.
   - **Arithmetic** (C 6.5.6). `p + i` and `p - i` are `loc(block_of(p), offset_of(p) ± i)` after explicit checks: `p !=
     null` (C; C++ allows `null + 0` and `null - null`, 7.6.6, so there `p == null ==> i == 0`), liveness (item 3a), and `0 <=
     offset_of(p) ± i <= len(block_of(p))` (`out-of-bounds`). A one-past-the-end pointer and a zero-length block are
     values, never dereferenceable, since no permission exists at `len`. `p - q` additionally needs one block and its
     result to fit `ptrdiff_t` (`overflow`). A quantified permission is written over the absolute slot (`forall k: Int
     :: { loc(block_of(a), k) } offset_of(a) <= k && k < offset_of(a) + n ==> acc(loc(block_of(a), k).v_int)`): Viper
     refuses `{ loc(block_of(a), offset_of(a) + i) }` as a trigger (S24). Over an array of structs there is one quantifier
     per member and field, each triggered by its own receiver (`{ loc(member(loc(block_of(a), k), j), 0) }`): one
     quantifier holding two members' permissions with one trigger fails Silicon's injectivity check (S26).
3. **Comparisons and truth tests.** `==` and `!=` compare `Ref`s: pointers to different objects are unequal, as C says.
   The one exception: C lets a pointer one past the end of one object compare equal to a pointer to another object that
   happens to follow it in memory (6.5.9p6), which the model never makes equal. So an `==` or `!=` whose operands are
   both non-null, in different blocks, with one of them one past the end of its block, is reported
   (`pointer-comparison`, an explicit check) rather than verified for one outcome only; `p == NULL` is never reported.
   `<`, `<=`, `>`, `>=` and `p - q` require both operands non-null and in the same block, else `pointer-comparison`
   (undefined in C, 6.5.8p5, 6.5.6p10; C++ leaves the relational result unspecified, which is reported the same way):
   stricter than VeriFast, which compares addresses. A pointer used as a truth value (`if (p)`, `!p`, `p && …`) is
   `p != null`.
   - **3a. Liveness** (owner, after audit round 2, B3). Using the value of a pointer whose object's lifetime has ended
     is undefined (6.2.4p2: the value becomes indeterminate), even `p == NULL` after `free(p)`. Each top-level block has a
     ghost liveness cell `lvloc(b)` = `loc(b, -2)` (field `lv`), created with full permission with the block and
     consumed by its scope exit (item 4) or `free` (item 6), which require full permission to it; a sub-object's
     liveness cell is its enclosing object's (`member_lv`). `owns`, `reads` and the new spec function `live(p)` (record
     0029 item 3) carry a read fraction of it in a contract, returned exactly (the ghost amount of item 8; two places of
     one block share one cell, so their fractions are summed: S26's `swap_adjacent`). Before every arithmetic,
     comparison or truth test, and before every other use of a pointer value (a copy, a store, an argument, a returned
     value: C J.2 makes any use of an indeterminate pointer undefined; audit round 3), lowering checks for each operand
     `p != null ==> perm(lvloc(block_of(p)).lv) > none` (`lifetime`: "uses `p` after the object it points to may have
     ended"). A global's block is alive for the whole program: `program_start` gives full permission to each `GB_`
     block's liveness cell, with `lvloc(GB_g()) == loc(GB_g(), -2)`, and `owns(g)`/`reads(g)` carry a fraction of it and
     that fact. S26: `free(p); p == q`, `free(p); p ==
     NULL` and a pointer to a local after its scope fail; a callee that keeps a fraction blocks the caller's `free`;
     the cost on the scale program is about 1.5–1.7 times the time and text. A
     function that compares or computes on pointers it does not access states `live(p)` in its `requires`. A fraction
     still held elsewhere at a scope exit or a `free` is that operation's error (`lifetime`, `invalid-free`).
4. **Address-taken locals and parameters.** At its declaration (for a parameter, at entry) such an object gets a fresh
   one-slot block from a trusted allocation method (`// cerastes:trusted-contract`, as 0015 item 5's methods), with full
   permission to its cell and its liveness cell, and 0019's flag "not initialized" unless it has an initializer. Each
   time its declaration is reached it gets a new block (C23 6.2.4p6). At every way out of its scope (fall-through,
   `break`, `continue`, `return`, `goto`) lowering checks that full permission to the cell and to the liveness cell is
   back, then consumes both: if a callee kept either, the error is `lifetime` ("the address of `x` may still be in use
   when its lifetime ends"; VeriFast's `dispose_local` fails the same way, VerCors misses it).
5. **Globals and `static` locals.** One block each, named by a `unique` 0-ary domain function (`GB_g(): Block`, the cell
   `loc(GB_g(), 0)`), with `len(GB_g()) == 1` part of `owns(g)` and `reads(g)` (a contract clause, so `&g + 1` is formed
   without a false alarm where `g` is owned). A trusted `program_start` gives `main` every mutable global of the unit
   with its initializer's value (0017's checked constants), as VeriFast's `open_module` does. **A call to `main` is
   rejected** (C allows it, C++ does not; with `program_start` inhaled on every entry a recursive `main` would assume
   the initial values again: audit round 2, B4). A function reads or writes a global only through its contract (`owns(g)`,
   `reads(g)`); one that does not name it cannot change it, since the caller keeps the permission across the call. A
   `static` local is a global whose name only its function sees; M4.5b2 decides how its permission reaches the function
   (an implicit `owns` propagated to callers, or a rejection inside a recursive cycle).
6. **Freeing and transfer** (implemented in M4.5d2). A heap block's free token is a ghost field at slot −1,
   `acc(loc(b, -1).tok)`, a slot no C pointer can name (a zero-length block has one too). **`owns_block(p)`** (a spec
   function, record 0029 item 3) is ownership of a whole allocation: `p` at offset 0 of a heap block, its token, full
   permission to its liveness cell and to every cell of the block, recursively through its sub-object blocks, and its
   `len`. It is what moves when ownership is transferred (audit round 3, B3): an owning result, field, local or global
   (record 0029 item 1) holds `owns_block`, and vlibc's `free(p)` requires `p == NULL || owns_block(p)` and consumes it.
   The layout comes from the static pointee type of `free`'s argument before its conversion to `void*` (item 1: no
   other `void*` reaches `free` typed otherwise). `free(&local)`, freeing a global, `free(p + 1)`
   and a double free fail (no token, or not at offset 0): `invalid-free` (S24: the token alone separates heap blocks
   from the others, so blocks need no kind). **Leaks:** at a function's end, `forperm t: Ref [t.tok] :: <t is a token
   its ensures returns>`, the exemption generated mechanically from the `ensures` (the tokens of the `owns_block`
   clauses it holds; never broader), or `false` when it returns none (S25(c)), reports every
   heap block the function still holds: `leak`. `forperm` sees only top-level chunks: a token under a quantified
   permission is pinned by a fixture (S26), and a token inside a folded predicate (a list node) is M4.5f2's to check
   through the predicate. Whether `main`'s blocks at exit are leaks (VeriFast says yes, with a `leak` command to waive
   one) is M4.5d2's record.

### Part B: permissions

7. **`owns` and `old`.** `owns(*p)` is `WF(p)`, `p != null`, `offset_of(p) < len(block_of(p))`, full permission to
   `*p`'s cell and its flag, a read fraction of the block's liveness cell, and the flag set (item 14): `acc(p.v_T) &&
   acc(p.d_T) && p.d_T && …`. `owns(g)` is the same for a global's cell, `owns(*s)` for every cell of a struct's
   sub-object blocks with their `len` facts (M4.5c), `owns_range(p, n)` the quantified form over `p[0..n)` (M4.5e1).
   `owns` never holds a block's token or its full liveness: it borrows a place; `owns_block` (item 6) owns an
   allocation. `old(*p)` in an `ensures` is the cell's value on entry, `old(p.v_T)`.
8. **`reads` and the ghost amounts.** A method whose contract has a `reads`, an `owns` (for its liveness fraction) or a
   `live` takes ghost permission amounts, `requires none < rd && rd < write`, and `reads(*p)` is `acc(p.v_T, rd) &&
   acc(p.d_T, rd) && p.d_T` (with the liveness fraction) in both its `requires` and its `ensures`. **The caller's choice**
   (audit round 2, S4): at a call, after an explicit check `perm(place_i) > none` for each of the n places the callee
   reads, lowering sets `rd := min(perm(place_1), …, perm(place_n)) / (n + 1)`, the minimum also ranging over the
   liveness cells the callee takes fractions of, each counted once per fraction taken from it (audit round 3), so the
   sum of the fractions is below what
   the caller holds even when two places are the same cell (`f(p, p)`) and when the caller itself holds only a fraction
   (both pinned by fixtures). The caller gets back exactly what it lent: after the call it can write again (S25(a)).
   `wildcard` is not used: a caller does not get its full permission back across a call. A write through a `reads` place
   fails its explicit check (`permission`). **A case split** ("S26 final"): Silicon cannot take a fraction computed as
   `perm(…)/(n + 1)` from two chunks whose receivers may be the same cell (`peek_two(p, q)` failed with
   `insufficient.permission` although the amounts were exact), so before the fractions are computed lowering emits a
   no-op `if (lvloc(block_of(p)) == lvloc(block_of(q))) { }` for every pair of liveness clauses at method entry and before
   each call, and the same on the cells for every pair of `reads` places before such a call; the split is on `lvloc`, not
   on `block_of`, since two blocks share a liveness cell when one is a member of the other.
9. **Frames come from permissions.** A callee changes only cells its contract gives it full permission to; everything
   else the caller holds keeps its value across the call, with no `modifies` clause.

### Part C: errors

10. **New categories** (each lands with its session's must-fail examples, in the five copies of 0011 item 10's table):
    `permission` ("reads `*p` without owning it", "writes `*p` with read permission only"), `null-dereference`,
    `out-of-bounds` (pointer arithmetic past the object, an index), `pointer-comparison`, `lifetime` (a scope exit with
    the address still in use; a use of a pointer whose object may have ended), `invalid-free`, `leak`, `unsequenced`
    (Part D). Messages name the C place in source terms (`c->val`, `a[i]`).
11. **Explicit checks, and what the driver accepts** (amends 0011 items 9–10). Before each access, arithmetic,
    comparison, call, scope exit, `return` and `free`, lowering writes every check the operation needs, so that the
    operation itself never fails first (S25(b): a bounds check followed by no permission check lets the raw read fail
    second): `assert p != null`, liveness, `assert perm(p.v_T) > none` (a read) or `== write` (a write), the flag, the
    bounds, at a call the callee's permission clauses one by one, and **at each `return` the function's own `ensures`
    permission clauses one by one** (audit round 2, B5: Silicon's check of the postcondition must never be the first to
    fail). Each fails as `assertion.false` at a line the position map gives a category and a message. At method entry
    lowering also asserts `a != null && b != null ==> a != b` for each pair of single places in the same field that the
    method owns with full permission (provable from its `requires`; none for ranges, whose checks are marked rows):
    without it a failed explicit return check about two places that may alias is followed by Silicon's own
    `postcondition:insufficient.permission` (S26's round-2 finding), which would be exit 3. **A permission
    check over a range** cannot use `perm` (Viper refuses `perm` under a quantifier); it is `assert forall k :: … ==>
    acc(…)`, which fails with `insufficient.permission`. Its map row records that it expects this reason, and the driver
    accepts `insufficient.permission` on exactly such rows, mapped to the row's category (owner, after S25). Such a row
    holds no heap read other than the checked permissions, so the reason cannot come from anything else on the line.
    Any other reason on any other row (`insufficient.permission` or `receiver.null` from an operation itself, or from
    Silicon's own check of a call's precondition or a postcondition) is a missing explicit check, an encoder bug: exit
    3, fail closed. As for scalar checks (0011), Silicon assumes a failed check and goes on, so a later error on the same
    path may be hidden; the verdict is still a failure.

### Part D: sequencing and placement

12. **Sequencing on objects** (amends 0020 item 4; audit round 2, B2). The rule on names stays for locals that are Viper
    locals. For two unsequenced accesses to cells, at least one a write, whether by name (an address-taken local, a
    global) or through a pointer (`*p = (*q)++`, `x++ + *p`, `g++ + *p`, `a[i] = a[j]++` once arrays come), lowering
    asserts that the two places differ (`p != q`, `&x != p`, or the indices differ), path-precise, category
    `unsequenced` (S25(e): separate ownership proves it). Places in different fields never alias (item 1), so no check is
    written for them.
13. **Calls and cell accesses** (amends 0020 item 8, 0021 item 2, 0025 item 2, 0026 item 15; audit round 2, B2). A call
    in a full expression is indeterminately sequenced with the accesses C does not order before or after it (`g + f()`,
    `*p + f(p)`): if the callee's contract gives it full permission to a cell, or any permission when the access writes,
    the access's place must differ from every such cell, checked path-precise like item 12 (a single cell `!= p`, a
    range `!(block_of(x) == block_of(a) && off in range)`), or the expression is rejected (`unsequenced_calls`, 0021) when
    the callee's places cannot be written so (M4.5f's predicates). **Two calls** in one full expression that C does not
    order (`set_g() + get_g()`) are checked the same way when one callee has full permission to a cell and the other any
    permission to it, a cell here including liveness and token cells (`(free(p), 0) + use_live(p)`, audit round 4): the places must differ on the path, or the expression is rejected (`unsequenced_calls`; audit
    round 3, B2). Writes **and snapshots** (reads 0025 moves) of a cell
    are never moved across such a call; writes to Viper locals keep today's placement. 0021's reliance on "no globals or
    pointers before M4.5" is replaced by this rule.

### Part E: initialization through pointers

14. **The flag follows the cell** (0019 item 13; S25(d)). A write sets `d_T`; a read checks it (`uninitialized`).
    **In a `requires`**, `owns(*p)` includes the flag (P4222's default: a pointer points to something initialized);
    `[[cerastes::ref_to_uninit]]` drops it. **In an `ensures`**, `owns(*p)` includes the flag too (the object is
    initialized when the function returns), except for a `ref_to_uninit` parameter, whose `ensures` keeps the flag's
    permission but not its value, unless `[[cerastes::must_init]]` adds it (record 0029 items 1–2; audit round 2, S7). A
    local that is not address-taken keeps 0019's Viper flag.
15. **C enumerator initializers** (closes handoff §3, M4.2b2 note 1). A pointer operation in a C enumerator's
    initializer is rejected, so pointer arithmetic never observes the front end's C++ typing of earlier enumerators.

### Both

16. **Structs and inner arrays** (M4.5c, M4.5e1; owner, after audit round 2, B1; S26). A struct object is a slot `r`;
    member `k` is the block `member(r, k)`, of length 1 for a scalar or pointer member, n for an array member, and itself
    holding a struct's slot for a nested struct; row `i` of `int a[2][3]` is `member(loc(b, i), 0)`, of length 3. So
    `&s.x + 1` is one past the end of `x`'s block (formed, never read), `&s.y` is `loc(member(s, 1), 0)`, and `a[0][3]`
    is out of bounds while `a[1][0]` is not (C 6.5.6p8 and J.2). A struct value (a copy, a by-value parameter or result) is
    the tuple of its members' values; `owns(*s)` gives every member cell and states each `len(member(s, k))`. S26:
    `&s.y`, `p->y`, nested structs and arrays of structs verify; `*(&s.x + 1)`, `&s.x + 1 == &s.y`, `a[0][3]` and
    `a[2][0]` fail at their explicit checks; the cost is a tie with consecutive slots. A struct is never a single `Ref`
    with a field per member (S24's S1): one layout everywhere. **Completeness costs** (audit round 3): `&s.x < &s.y`,
    which C defines (6.5.8p5: later members compare greater), is reported as `pointer-comparison`, since the members are
    different blocks; and since every use of a pointer checks liveness (item 3a), a truth test on a pointer read from a
    cell (`n->next != NULL` in a list traversal) needs the liveness of its target, which M4.5f2's list predicates must
    carry.
17. **No `inhale` or `assume`** is emitted outside a tagged trusted method (invariant 4). The trusted methods (the
    allocation of an address-taken local or parameter, `program_start`, and vlibc's contracts, record 0030) carry
    `// cerastes:trusted-contract <reason>`.
18. **Tests and documentation.** Each session lands its constructs with must-verify and must-fail pairs (invariant 2),
    including swap, an aliasing trap that must fail, a local whose address escapes, a pointer used after its object
    ended, a struct field update, a member pointer past its member, a linked list, array sum and max with a quantifier, a
    double free and a leak (plan.md's M4 acceptance); the "Memory and ownership" page from M4.5b1; each new category's
    section in "Error messages" with a must-fail example. The fixtures in `tests/viper-smoke/memory/` pin this record's
    encoding before the encoder emits it; a session that changes the encoding changes them with a record.
19. **Facts relied on** (`internal/il-inventory.md`, "M4.5a"): (S24) the encodings compared, the ablation, the
    consistency runs; (S25) permissions, error reporting, leaks, init flags, sequencing, owning pointers; (S26) member
    blocks, liveness, and the fixtures after audit round 2; (P51) the IL of the pointer constructs.

## Consequences

- Programs without pointers keep their Viper; the memory model costs nothing until a program takes an address.
- The prelude gains its first domain and its first four axioms, each with a record and a lint tag.
- cerastes reports a local whose address outlives it, a use of a pointer whose object has ended, arithmetic past a
  member, a free of a non-heap object and a cross-object comparison, which VerCors (and, for most of them, VeriFast)
  accept.
- Eight new error categories, each documented with a must-fail example when it lands; the driver accepts one Viper
  reason besides `assertion.false`, on marked rows only.
- `live(p)` and `owns_block(p)` are new spec functions (record 0029).
- `void*` is usable only as an argument, a null test and an allocation's result until M8's address model.

*2026-10-28, amended by 0032 (M4.5b1a): item 1 — the value and flag fields are one pair per VIR type (VIR interns integer types by width and signedness, so `long` and `long long` share one), sound because no accepted conversion lets a pointer to one reach an object of the other, decided on EDG's types (0032 items 3, 5); `restrict`, `volatile` and `_Atomic` pointers are rejected. Item 4 — an address-taken local is allocated when its block is entered (C23 6.2.4p6), and before a `switch`'s dispatch for one declared in its body; its declaration stores the initializer or leaves the flag false. Items 2 and 17 — the four axioms have their records, 0033–0036, and lint tags; M4.5b1 is split into b1a (0032), b1b and b1c.*

*2026-10-28, amended by 0037 (M4.5b1b): item 4 — an address-taken parameter is copied into its block at entry and leaves it at the function body's scope exits. Item 8 — one ghost amount per atom of the `requires` (an `ensures` atom uses its `requires` partner's), the caller choosing `a_k := (g_k ? m_k : write) / (N + 1)` with `m_k` over the liveness cell (and for `reads` the value and flag cells), and the return check summing the amounts of atoms that may name one cell; `s25_a_reads.vpr`'s shared `rd` stays a verified alternative. Item 11 — the explicit checks at calls and returns are category `permission`; the framing of a contract is an explicit check in its well-formedness method, and each heap read of the method's own contract is guarded by its framing disjuncts (0037 item 7).*

*2026-10-29, amended by 0038 (M4.5b1c1): item 4 — the scope exit is written at every way out, `break`, `continue` and `goto` included (0038 item 7). Items 8–9 — a loop that touches memory takes, through its automatic invariant, everything the function holds of every cell in scope and of the places of its unassigned pointer parameters (0038 item 3: snapshot amounts, case splits, values kept when no write in the loop can reach them), and gives it back at each iteration's end (`permission`, 0038 item 4).*
