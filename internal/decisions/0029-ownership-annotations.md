# 0029 — Ownership annotations and specification functions in C and C++ (M4.5a)

- **Status:** accepted for M4.5a (2026-10-27), finalized with P52, P53 and P54. Owner decisions (M4.5a planning,
  2026-10-27):
  - **`[[cerastes::owning]]` on a declaration** ("Much like how P4222 allows `[[uninit]] int x;` I want to have an
    annotation for `[[owns]] char* x;` in addition to `owns(x)`"; then, the same day: "instead of a pointer as owns make
    it owning e.g. `[[owning]] int* x;`", so the declaration attribute is `owning` and `owns` stays the contract
    function): an **owning pointer** wherever it appears (the
    owner chose this over a parameter-only shorthand and over transfer-on-parameter). On a parameter the callee borrows
    the pointee; on a function the caller receives the result's pointee; on a local or global the holder frees the
    pointee or hands it on before the variable's lifetime ends; on a struct field, owning the struct owns the field's
    pointee. Phased over M4.5b–f.
  - **P4222's markers** are spelled in the `cerastes::` namespace (`[[cerastes::uninit]]`,
    `[[cerastes::ref_to_uninit]]`, `[[cerastes::must_init]]`), registered like every other annotation (no fork
    commit), checked when written, with an opt-in strict mode; the default stays 0019's path-precise check without
    markers (the owner chose this over the paper's unqualified spelling and over strict-by-default).

  Facts marked (P*n*) are probes, recorded with their evidence in `internal/il-inventory.md`, "M4.5a".
- **Milestone:** M4.5a (2026-10-27); implemented in M4.5b1 (spec functions in C; `owning` on parameters and functions),
  M4.5b2 (P4222's markers), M4.5c (non-recursive owning fields), M4.5d2 (owning locals and globals), M4.5e1 (`forall` in
  C), M4.5f2 (recursive owning fields).

## Context

`<cerastes.h>` declares `owns`, `reads`, `old`, `owns_range` and `forall` in `namespace cerastes`, for C++ only (0008
item 6: "A C form waits for the memory model (M4.5)"); `forall` takes a lambda (0006 item 9), which C lacks. Their
meaning was left to the memory model (record 0028). The registration table (`register_attributes.cpp`) has no
attribute on a variable, a parameter or a field; EDG's registration accepts those targets (`v`, `p`, `d`) but not a
type (`t`) (0007). The encoder skips registered `cerastes` attributes when it checks a declaration's attributes
(`check_attributes`, `encode.cpp`), so an attribute registered before the encoder reads it would be silently ignored:
each attribute below is registered in the session that gives it its meaning, never earlier.

P4222R2 ("An initialization profile", 2026-07-11) proposes `[[uninit]]` on a definition left uninitialized,
`[[ref_to_uninit]]` on a pointer, reference or smart pointer whose referent is uninitialized (including a function's
result, `void* malloc [[ref_to_uninit]] (size_t)`), and `[[must_init]]` on a parameter whose referent the callee must
initialize (it implies `[[ref_to_uninit]]`). By default a pointer points to something initialized; a pointer to an
`[[uninit]]` object may be passed only to a `[[ref_to_uninit]]` parameter, and a pointer to an initialized object may
not be passed to one (§4.2). Decision 0019 checks every read of an uninitialized local on each feasible path without
markers, and left "a stricter mode that also requires P4222's markers" to a later record (0019 item 2); its item 13
says the flag follows the cell once pointers come.

## Decision

1. **`[[cerastes::owning]]`, the owning pointer.** It appertains to a declared entity whose type is a pointer to an
   object type, in either position (leading `[[cerastes::owning]] char* x;`, or after the declarator-id `char* x
   [[cerastes::owning]];`), with the macro `CERASTES_OWNING` for other compilers. Two meanings, by position (audit round
   3, B3): a **borrow**, record 0028's `owns` of the pointee (one object, `*p`, with a read fraction of its liveness),
   and a **transfer**, `owns_block` of the allocation it points to (record 0028 item 6: offset 0, the free token, full
   liveness, every cell), which is what a caller needs to `free` it. By position:
   - **A parameter** (target `p`): the callee borrows. It is `requires(p == NULL || owns(*p))` and
     `ensures(p == NULL || owns(*p))`, with `p` its entry value in the `ensures` (0006 item 5). The callee must check
     `p` before dereferencing it, unless a `requires(p != NULL)` says so. A function that takes ownership (frees or
     keeps the allocation) states `requires(p == NULL || owns_block(p))` with no matching `ensures`, as vlibc's `free`
     does; there is no attribute for it (the owner chose borrow for parameters).
   - **A function** (target `r`): the caller receives the allocation: `ensures(result == NULL ||
     owns_block(result))`. A callee that returns a pointer into an object it does not own entirely (a global, a local,
     an interior pointer) fails that `ensures`.
   - **A local or a global** (target `v`): the variable holds `owns_block` of its pointee. When its lifetime ends (scope
     exit, and for a global the end of `main`) and it is not null, the allocation must have been freed or handed on
     (returned, stored into another owner, or passed to a function that requires `owns_block`); otherwise the error is
     record 0028's `leak`. Assigning a new value to it while it owns an allocation is the same error.
   - **A struct field** (target `d`): owning the struct holds `owns_block` of the field's pointee when it is not null.
     Ownership is **recursive** when the owning-field graph between struct types has a cycle (`struct node { owning
     struct node* next; }`, or two structs owning each other; audit round 3): it is an implicit recursive predicate and
     comes with M4.5f2; acyclic owning fields come with M4.5c.
   - `void*` is allowed only on a function (a transfer: vlibc's `malloc`, `calloc`, `realloc` return `owning void*`,
     M4.5d2's typed view decides the cells); a borrowed `void*` has no pointee to own.
   - **Registration follows the phasing** (audit round 3): `check_attributes` skips every registered `cerastes`
     attribute, so a target registered before the encoder handles it would be silently ignored. Each session registers
     only the targets it implements (`p`, `r` in M4.5b1; `d` in M4.5c; `v` in M4.5d2), or rejects, for the targets still
     registered ahead, the entity kinds it does not handle; a must-fail test pins each such rejection.
   - **Rejected** (`unsupported_annotation`, with its own message): a non-pointer entity, a pointer to function or a
     borrowed `void*`, the pointer-declarator position
     `char* [[cerastes::owning]] x` (a type attribute, which registration refuses, P52), and any position P52 shows the
     IL does not keep.
   - **With a count:** `[[cerastes::owning(n)]]`, where `n` names an earlier parameter (or, on a function, a parameter),
     owns `n` elements from the pointer (`owns_range(p, n)`), for buffers (`char* buf [[cerastes::owning(n)]]`); it
     lands with M4.5e1. `owning()` is a syntax error, a later parameter or an undeclared name the front end's error #20
     (P52).
   - **Where EDG keeps it (P52, both modes):** registered as `{"owning", "?(X)", "r|v|p|d", RAF_ENTITY_ATTACHED}`, every
     position works with no fork commit, leading and trailing alike. A parameter's attribute is on its
     `param_type.attributes` (a definition reads `variable.assoc_param_type.attributes`, the union over all declarations);
     a local's, global's, `static` local's and field's on `source_corresp.attributes`; a function's on the routine's and on
     each secondary declaration. Two declarators follow C and C++: a leading attribute covers both, a trailing one its own
     declarator. EDG itself refuses the type position (`char* [[cerastes::owning]] x`, #1835), a position inside the
     declaration specifiers and a `typedef` (#1866, #1835).
   - **Rejected by the encoder** (registration accepts them): a non-pointer entity, a function that does not return a
     pointer, a pointer to function, a borrowed `void*`, a duplicate, and **a mismatch between declarations**: attributes on
     parameters and variables accumulate over redeclarations (P52; EDG's `RAF_MUST_MATCH` covers only the function
     position), so the encoder requires every declaration of an entity to carry the same `owning` (and the same count),
     as a contract must match (0006 item 6), and rejects any other (`annotation_mismatch`, its message naming both
     declarations).
   - **Macros:** `CERASTES_OWNING` and `CERASTES_OWNING_N(n)` are defined only by `<cerastes.h>` (a user definition is
     `annotation_macro_changed`, P52). The attribute and the spec function `owns` do not clash.
2. **P4222's markers.** Registered with these targets (P54):
   - `[[cerastes::uninit]]` on a definition of a local object without an initializer (registered `v:+a!`, automatic
     variables only, so EDG itself refuses it on a global or a `static` local, #1862, and on a parameter, field or
     function, #1835; P54): it states that the object starts uninitialized. A definition with an initializer is
     rejected by the encoder (`uninit_initialized`).
   - `[[cerastes::ref_to_uninit]]` on a pointer parameter, a pointer local or a function (`p`, `v`, `r`): its pointee
     may be uninitialized. A parameter so marked drops "initialized" from the pointee's `owns` on entry (record 0028
     item 14); a function so marked returns a pointee that may be uninitialized (vlibc's `malloc`).
   - `[[cerastes::must_init]]` on a pointer parameter (`p`): `ref_to_uninit`, and the pointee is initialized on
     return (an `ensures`).

   `ref_to_uninit` is registered `p|v|r` with `RAF_ENTITY_ATTACHED`, `must_init` `p` (P54); on a non-pointer they are
   rejected by the encoder, and they must match across declarations as `owning` must.

   **What is checked when they are written** (path-precise, through 0019's flags that follow the cell, record 0028
   item 14): a `must_init` pointee not initialized on some return path (`postcondition`); a pointer whose pointee may
   be uninitialized passed to a parameter that is not `ref_to_uninit` (`uninitialized`, at the argument, naming the
   parameter). Category names and messages are fixed with M4.5b2's tests. Macros: `CERASTES_UNINIT`,
   `CERASTES_REF_TO_UNINIT`, `CERASTES_MUST_INIT`.

   **The strict mode** (an opt-in driver option, `--strict-init`): every definition of a local without an initializer
   must carry `[[cerastes::uninit]]` (rejected `uninit_unmarked` otherwise), and a pointer to an initialized object
   passed to a `ref_to_uninit` parameter is an error, as P4222 §4.2 says. The default mode keeps 0019's check, which
   already proves on every feasible path that nothing uninitialized is read; the markers then document intent.
3. **Spec functions in C.** `<cerastes.h>` declares, in both modes, functions with C names (C has no namespaces):
   `bool cerastes_owns(...)`, `bool cerastes_reads(...)`, `bool cerastes_owns_range(...)`, `bool
   cerastes_owns_block(...)` (and in C++ `cerastes::owns_block`: the whole allocation `p` points to the start of, record
   0028 item 6; audit round 3, B3), `bool cerastes_live(...)`
   (and in C++ `cerastes::live`: "the pointer points into an object whose lifetime has not ended", record 0028 item
   3a, for a function that compares or computes on pointers it does not access; owner, after M4.5a's audit round 2), using C23's variadic
   declaration without a named parameter, so that each takes a place (`cerastes_owns(*p)`, `cerastes_owns(g)`,
   `cerastes_owns(p->f)`) like the C++ forms; the encoder reads the place under the argument's conversions (P53).
   `old` is a macro over C23 `typeof`, `cerastes_old(e)` = `(*(typeof(e)*)cerastes_old_(&(e)))`, which takes a place
   (`cerastes_old(*p) + 1`, never `cerastes_old(*p + 1)`, which the front end refuses: not an lvalue). In C++ the
   namespace forms stay, and the `cerastes_` forms are declared too, so a header shared by C and C++ (vlibc, record
   0030) writes one spelling. Like the C++ forms they are never defined; a call that reaches run time fails to link.
   **What the IL keeps (P53, both modes).** The declarations parse in C23 (the attributes of the function using them
   follow its declarator). `cerastes_owns(*p)` keeps the place: an `eok_indirect` with an `orig_lvalue_type` over the
   parameter; a `char`, `_Bool` or `short` place is wrapped in a compiler-generated cast to `int` (the default argument
   promotion), which the encoder looks through; a struct place has no cast; `cerastes_owns(g)` is the variable,
   `cerastes_owns(s->x)` an `eok_points_to_field`. A pointer value (`cerastes_owns(p)`, and `cerastes_owns(&*p)`,
   which the front end folds to `p`) has no `orig_lvalue_type` and is rejected (`unsupported_expression`: "owns takes
   a place, `*p`, not a pointer"). `cerastes_old(e)` parses in C23 and C++20; its argument reaches the IL as an
   address (`p` itself for `*p`, the front end folding `&*p`; an `eok_address_of` for `s->x` and `p[0]`; a constant
   address for a global), which the encoder reads as the place it points to; `cerastes_old(*p + 1)` and
   `cerastes_old(5)` are refused by the front end ("expression must be an lvalue"); a bit-field, a `register` object
   and an array place, whose address the macro cannot take (or which C does not let `old` copy), are rejected by the
   encoder with a message of its own rather than the front end's (audit round 3). `cerastes_old(x)` of a by-value
   parameter does not make it address-taken (the IL's flag stays false in both modes, audit round 3's probe), so it
   stays a Viper local; M4.5b1 pins that with a test. Through `typeof`, EDG's C23 mode
   drops `const` from `typeof(*cp)` for a `const int* cp` (clang keeps it; P53, five shapes): harmless for `old`, which
   only reads, and not chased (handoff §3). The `_Generic` fallback is not needed.
4. **`forall` in C, without a fork commit.** `cerastes_forall(f, free...)`, where `f` names a `predicate` or `pure`
   function whose first parameter is the quantified variable and whose other parameters receive the trailing
   arguments in order: 0006 item 9's rule, with a named function in the lambda's place. C++ accepts the same form
   with `cerastes::forall` (its declaration deduces `F` as a pointer to the function). A typed binder
   (`forall(size_t i; …)`) would need an EDG fork commit (the result-name code of 0007 item 6 cannot type a variable)
   and is not needed: the IL keeps the function among the variadic arguments in C23 (`enk_routine`, or an
   `eok_address_of` for `&nonneg`) and as a constant address in C++20 (P53). It is encoded with M4.5e1, when predicate and pure bodies are still opaque; the function's
   body gives the quantifier's meaning only once M4.5f1 encodes such bodies, so M4.5e1 accepts it only where the
   function's contract (`ensures`) states the body's meaning, or schedules `forall` after M4.5f1 (M4.5e1's planning
   decides).
5. **Spellings.** `owns`, `reads`, `old`, `owns_range` and `forall` keep their C++ meaning (decision 0006) and gain
   the meaning of record 0028; `live` is new and takes a pointer value (`cerastes_live(p)`), where `owns` and `reads`
   take a place (`cerastes_owns(*p)`). No unqualified attribute is registered.
6. **Documentation.** A "Memory and ownership" page starts in M4.5b1 and gains each annotation in the session that
   lands it; the Annotations page's table of spec functions ("What it will mean") is rewritten then. Nothing
   user-visible changes in M4.5a.

## Consequences

- The memory annotations are attributes on entities (targets `p`, `r`, `v`, `d`), never on types; no EDG fork commit
  is needed for them or for C's spec functions (if P52 and P53 hold).
- C contracts read `cerastes_owns(*p)` where C++ reads `cerastes::owns(*p)`; vlibc's headers use the C spelling.
- P4222's markers are available to C and C++ code in the `cerastes::` spelling. A later move to the paper's unqualified
  spelling, if P4222 is adopted, is a registration change plus a fork commit.

*2026-10-28, amended by 0037 (M4.5b1b): item 3 — `<cerastes.h>` declares `cerastes_owns`, `cerastes_reads`, `cerastes_live`, `cerastes_old_` and `cerastes_old` (over `__typeof__`, which C++20 compilers accept where they refuse `typeof`); `cerastes_owns_range`, `cerastes_owns_block` and `cerastes_forall` come with the sessions that give them a meaning (M4.5e1, M4.5d2). P58: in C23 `cerastes_old(*p)` reaches the IL as the bare pointer (the front end folds `&*p`), in C++20 as `&*p` (`eok_address_of` of the `indirect`); `old(p)` and `old(x)` as `&p`, `&x`; none sets `address_taken`. Item 5 — `live(p)` implies `p != NULL` (as `owns(*p)` does): a function that accepts NULL writes `p == NULL || live(p)`.*

*2026-10-29, amended by 0038 (M4.5b1c1): item 1 — the registration phasing changes: target `p` (parameters, a borrow) in M4.5b1c3, target `r` (functions, a transfer by `owns_block`) in M4.5d2 together with `owns_block`, `owns(*result)` and an `ensures` atom without a `requires` partner (owner, 2026-10-29: nothing creates a heap block before `malloc`).*
