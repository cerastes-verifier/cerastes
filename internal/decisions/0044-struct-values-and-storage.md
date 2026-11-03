# 0044 — Struct values and struct storage (M4.5c2a)

- **Status:** accepted for M4.5c2a (2026-11-02), written before code (invariant 6), revised after P67 (the IL: PASS, hazards
  H1–H9, items 4, 6–8), P68 (the compiler matrix: only `h2(s, s = t)` warns, item 5), P69 (the invariant heap-read gap: no
  wrong verdict, item 12) and audit round 1 (Opus, on the draft: B1 the elements of `&(T){…}` stay unsequenced and B2 a
  `const` literal may share its object, item 7; should-fix S1 the literal's block for `for`/`switch`/`else`, S2 literals used
  as lvalues, S3 element writes through aliases, S4 side-effect bases beyond arguments, S5 item 12's argument, S6 the
  rejections of item 15, S7 the lint's `Addr` leaf, S8 item 11's wording; notes N1–N7; all taken; `build/m4.5c2a/audit1.md`);
  and S34 (Silicon: PASS for (a)–(f), every vacuity clone failing; H8 needs no fact, item 8), and audit round 2 (on the code: no blocking item; S1 `volatile` and `_Atomic` literal values, item
  7; S2 and S3 pins; S4 the lint's `Addr` rule, item 13; S5 `u = mk().t;`, item 5; notes N6–N10 taken or in the handoff;
  `build/m4.5c2a/audit2.md`), and one re-audit (no blocking item; S1 the global-leaf write rule outside `main`, item 8;
  N1 a C++ initializer context, item 6; `build/m4.5c2a/audit3.md`). During the session the owner answered question 101 (two VIR self-test rejection cases retired for
  lowering cases, item 18) question 102 (the expectations of new tests corrected after their first run, each listed
  in the status entry) and question 103 (one more new VIR case's row index). Owner decisions (M4.5c2a
  planning, 2026-11-02; plan `.claude/plans/m4-5c2a-struct-values.md`; handoff questions 95–100, each the
  recommendation):
  - **95:** M4.5c2 is two sessions: this record's struct values and struct storage, and M4.5c2b's places reached through
    a heap read (with written struct atoms in loop invariants, question 99, and the `restrict` re-check).
  - **96:** compound literals are, in C, values and objects of their enclosing block (`&(T){…}`); in C++ values only.
  - **97:** passing a struct by value and returning one are copies (question 92): every member is read.
  - **98:** the flips of item 18.
  - **99:** written atoms on struct places in loop invariants move to M4.5c2b.
  - **100:** struct `?:`, comma and assignment values stay rejected, naming M8.
- **Milestone:** M4.5c2a (2026-11-02).
- **Implements:** 0043 item 1's M4.5c2 row, except the places reached through a heap read (M4.5c2b).
- **Amends:** 0012 item 2 (a by-value struct parameter's entry value, item 3), 0037 items 2 (`restrict`, item 11) and 3
  (result splits per pointer leaf, item 4), 0039 items 2–4 and 8 (struct-valued calls and stores, item 5; member
  measures, item 10), 0041 items 2–5, 7, 10–12 (struct globals and `static` struct locals, items 8 and 9), 0043 items 1,
  5–9 and 15 (the lifted rejections).

## Context

Record 0043 made C structures objects: struct locals are blocks (`alloc_struct_<n>_<tag>()`), members are blocks
`loc(member(r, k), 0)`, copies go leaf by leaf, contract atoms and values name struct places by root and member path.
Everything that is a struct *value* rather than an object stayed rejected, naming M4.5c2: by-value parameters and
results, `f().x`, temporaries (`S{…}`, `S()`, compound literals), and struct objects of static storage (globals and
`static` locals). VeriFast verifies all of them (`tests/call_in_struct_init.c`, `examples/struct_assignment.c`, whose
`test4`–`test6` pass structures and compound literals by value). Facts: P67, P68, P69 and S34 in
`internal/il-inventory.md`, "M4.5c2a".

## Decision

1. **Scope.**

   | Construct | Session |
   |---|---|
   | by-value struct parameters and results, `f().x`, struct-valued calls as arguments, initializers and copy sources; C++ `S{…}`, `S()` as values; compound literals (C: values and `&(T){…}`; C++: values); struct globals and `static` struct locals; `decreases(p->x)`, `decreases(g.x)`, `decreases(s.x)`; `restrict` beside by-value structs | M4.5c2a (this record) |
   | places reached through a heap read (`owns(**pp)`, `reads(**pp)`, `live(*pp)`, `**pp`, `*gp`, `owns(*gp)`, `p->q->x`, `*s->q`, an atom on `*q` where `q` is a cell); atoms and reads through a pointer member of a by-value struct (`owns(*s.q)`, `*s.q`, `s.q->x`, `*r.q`); written struct atoms in loop invariants (question 99); the `restrict` re-check | M4.5c2b |
   | atoms on result members (`owns(*r.q)`, an `ensures` atom with no `requires` partner) | M4.5d2 |
   | array literals, arrays of structs | M4.5e1 |
   | `const` by-value structs (`const struct S s`), `const` struct globals and `static const` struct locals, the address of a `const`-qualified compound literal | M4.6 |
   | pure functions and predicates taking or returning structures | M4.5f1 |
   | `S s(1, 2)`, `S(1, 2)` as a value, C++ `t = {1, 2};` (a call of `operator=`), and every C++ class feature | M7 |
   | struct `?:`, comma and assignment values (question 100); a struct store in a `for` step or a `?:` arm; a compound literal in a loop condition, a `for` step or a `do` condition, or used as an lvalue without `&`; a struct object read through a loaded pointer or a base with a side effect as an argument, a copy source or a destination; `&h.x` as a global initializer; a nested member initialized from a struct value (`{s, 1}`); in C++ a value aggregate with an effect beside another non-constant element, and a folded `S{…}.x` | M8 (listed, over-strict) |

2. **Struct values in VIR.** A struct value is a tuple of its leaves (0043 item 3's leaves, in `struct_leaves` order,
   depth first). It exists only between a few producers and consumers, and lowering expands it into one Viper value per
   leaf; struct objects keep 0043's encoding.
   - **Expressions:** `Field(a, k)` (member `k` of a struct value; nested members chain) and `Aggregate(T, args)` (one
     argument per leaf of `T`).
   - **Producers:** a `Call` of a struct-returning function; an `Aggregate`; a struct-typed `Field`; `Formal` and
     `Result` in contracts only, and only directly under a `Field` (a `Formal` also inside `Old`).
   - **Consumers:** a call argument of a struct parameter; `Return` of a struct-returning function; a struct `Store`
     whose value is a `Call`, an `Aggregate` or a `Field` chain over a `Call`; an expression statement of a `Call`; a
     `Field`.
   - `check_program` admits exactly these positions (every other keeps its error), with an `Aggregate`'s arity and leaf
     types, a `Field`'s index and a struct global's leaves checked; an `Aggregate` never appears in a contract (no
     temporary is a contract value). The encoder rejects every other position first, with a diagnostic at the C++ line
     (invariant 3); `check_program` and lowering are backstops, lowering making a struct value that reaches any other
     consumer an internal error (exit 3), never a silent truncation.
3. **By-value parameters** (amends 0012 item 2).
   - Each leaf of a struct parameter is a Viper formal, named with one `_m<k>` suffix per path element (`p_s__0_m0`,
     `p_s__0_m1_m0`); the leaves take the parameter's place among the formals, before the ghost amounts. Each integer
     leaf's range and each pointer leaf's `WF` (`WF_S` for a struct pointer) is a `requires` of the method and of
     `f__wf`.
   - **Entry:** the parameter's copy is a struct block, as every struct local is (0043 item 4): allocated by
     `alloc_struct_<n>_<tag>()`, each leaf written from its formal with its flag set (through a `Ref` temporary, so
     `lint_flags` is unchanged), and exited at every return and at the fall-through.
   - **Contracts:** `s.x`, `s.t.y` and a pointer leaf `s.q` read the formals, which hold the entry values (0012 item 2), in
     `requires`, `ensures` and `decreases`; `old(s.x)` is the same value. The whole parameter as a value (`s`, `old(s)`,
     `s` compared or passed in a contract) is rejected: C has no `==` on structures.
   - **Rejected:** `const struct S s` (a `const` object, M4.6); `owning`, `ref_to_uninit` and `must_init` on a by-value
     struct (`unsupported_annotation`: it is not a pointer); an atom on a member (`owns(s.x)`: the copy is the callee's);
     `*s.q`, `owns(*s.q)`, `reads(*s.q)`, `live(s.q)` and `s.q->x` (M4.5c2b).
4. **By-value results** (amends 0037 item 3).
   - The method returns one result per leaf (`res_m0`, …), each with its range or `WF`/`WF_S` as an `ensures`.
   - **`return`:** `return s;` (a struct object: a local, `*p`, `p->t`, `s.t`, a global) reads every leaf as a copy source
     does (question 97, 0043 item 7: the base's null check, then each leaf's permission and flag, and a pointer leaf's
     liveness), and assigns the results before the scope exits and the postconditions. A pointer leaf to the dying object
     (`n.next = &n; return n;`) is not R14's shape; it dangles at the caller, whose every use of it is checked (`lifetime`,
     0028 item 3a; pinned). `return f();`, `return (S){…};`, and C++'s `return S{…};`, `return S();` and `return {…};`
     assign the value's leaves; in C++ a class-typed return keeps its value in `return_dynamic_init`, not `expr` (P67 H1).
   - **Contracts:** `r.x`, `r.t.y` and `r.q` in an `ensures` read the result leaves. `old(r.x)` is rejected (a result has
     no entry value); atoms on result members (M4.5d2), `*r.q` and `live(r.q)` (M4.5c2b) are rejected. A leaf's range or
     `WF` among the `requires` and `ensures` is checked where the scalar parameter's and result's are, with the same
     categories.
   - **Callers:** a call of a struct-returning function is one call with one target per leaf (`t__n_m0, t__n_m1 :=
     f(…)`), and 0037 item 3's case splits after a pointer result run for every pointer leaf of the result.
5. **Struct values at calls, copies and statements** (amends 0039 items 2–4).
   - **Arguments:** a struct argument passes its leaves. An argument that is a struct object (`s`, `*p`, `p->t`, `s.t`,
     `g`) is read leaf by leaf (question 97: as 0043 item 7's copy source); its base's null check comes once, before any
     leaf. The base is evaluated once per leaf, so a base with a side effect or a call (`g(*p++)`, `g(*get())`) and a base
     read through a loaded pointer (`g(*w->q)`, `g(**pp)`) are rejected (M8, listed): the same rule holds for a `return`
     source (`return *get();`) and a struct `Store`'s destination (`get()->t = f();`, `(*pp)->t = f();`, where `f` could
     write `*pp`, which C leaves indeterminately sequenced with the call). C++'s `g({1, 2})` is a value (P67 H7).
   - **Results used:** `S s = f();`, `t = f();`, `*p = f();`, `p->t = f();`, `g(f())`, `return f();`, `f().x`, `f().t.y`,
     `u = f().t;` and `struct T u = f().t;` (a nested member of a result), and `f();` discarded. A struct `Store` writes every destination leaf after the value exists: the base's null check,
     then per leaf its permission at `write` (with `WF` for a pointer leaf) and its flag set.
   - **Sequencing:** a struct-valued call is one call (0021, 0039 item 4), its leaves one dependency set; the leaf reads of
     an object argument are reads of each leaf's key (0043 item 12); a struct `Store` writes every destination leaf's key;
     the loop havoc rule counts a struct `Store` as a write of every leaf field. So a shape that names one leaf twice with
     a write is rejected `unsequenced_modification` by 0039 item 3's static rule (question 70): `h2(s, s = t)` (clang and
     GCC warn, P68 Q4), and also `get(s) + (s.y = 1)`, `get(s) + s.x++` and `h(s, s.x = 1)`, which the compilers do not
     connect to the member (P68 Q1–Q3, Q5) but C leaves undefined (an unsequenced read and write of `s.y`, C23 6.5p2);
     shapes that depend on aliasing (`get(*p) + (q->y = 1)`) are checked path-precise (`unsequenced`).
   - **An assignment from a temporary.** In `t = (struct S){…}` (and every struct `Store` of an `Aggregate`) the elements'
     side effects are unsequenced with the store to the destination's leaves (C23 6.5.16p3: the store follows the
     operands' value computations, not their side effects). The elements write nothing of the destination: by name
     (`s = (struct S){s.x++, 0}`, `s = (struct S){.x = (s.y = 1)}`) 0039 item 3's static rule rejects them
     (`unsequenced_modification`; P68 L1, L2: no compiler warns, cerastes's own rule as above), through an alias (`*p =
     (struct S){.x = (q->x = 5)}` with `q == &p->x` possible) the path-precise check reports `unsequenced`. A call in an
     element is sequenced before the store (its value computation precedes it) and is accepted, as is the swap `s =
     (struct S){s.y, s.x}`. In C++ the temporary is fully initialized before the trivial `operator=` (P68: silent).
   - **Rejected:** a struct `?:`, comma or assignment used as a value (`t = c ? s : u;`, `t = (f(), s);`, `(t = f()).x`,
     `u = (t = f())`), and a struct store in a `for` step or a `?:` arm (question 100; M8, over-strict, as 0043 item 7's
     copy used as a value).
6. **C++ temporaries.** `S s = S{…}` and `S s = S()` are 0043 item 6's braced list and `{}` (P65: an aggregate with
   `explicit_cast_applied`); `S{…}`, `S{}` and `S()` are values wherever item 5 takes one, built as an `Aggregate` of
   their elements with zero leaves for the members they do not name (P67 H6: C++ writes zero fillers into designated
   lists, and `S()` as a value is `dik_zero` with no aggregate: every leaf zero; H7: as an initializer or a return the list
   is a plain aggregate, as an argument or an assignment source a `temp_init`). `S s(1, 2)` and `S(1, 2)` stay M7, and so
   does `t = {1, 2};` (a call of `operator=` with a braced list). A C++20 designated list that skips a nested struct
   member (`T{.z = 1}`: EDG fills the member with zeros without braces, P65 H1) stays rejected as 0043 item 6 rejects it
   in an initializer (M8), and so does a temporary with a nested struct member as an argument inside a variable's
   initializer (`int r = get(T{{1, 2}, 3});`: EDG folds it there and drops its braces flags, so it cannot be told from
   brace elision; M8, re-audit N1); as an assignment's or a statement's call it is accepted.
7. **Compound literals** (question 96).
   - **As values** (both modes): `(struct S){…}` used as a copy source, an argument, a return value or a member read is an
     `Aggregate` of its elements (0043 item 6's element rules: each an assignment conversion, overrides rejected, in C
     every element first) with zero leaves; `(int){5}` used as a value is its converted element. C++ folds a constant
     literal value (`(S){1, 2}.x` is the constant 1, `S s = (S){1, 2};` a plain aggregate; P67 H2), read as the folded
     constant with 0043 item 6's original-value rule.
   - **As objects in C:** `&(T){…}` and `&(T){…}.m` designate an unnamed object with automatic storage duration
     associated with the enclosing block (C23 6.5.2.5p5–6). The encoder declares a hidden address-taken local at the
     start of the innermost block that contains the literal, C's block for it (C23 6.8.4p3, 6.8.5p5: a selection or
     iteration statement is a block, and so is each substatement; EDG already wraps every C `if`, `while`, `do`, `for` and
     `switch` in a compiler-generated block, and a non-compound substatement too, P67 H4, so a literal in a `for` init, a
     `switch`'s controlling expression or an `else` arm lives in that block only). So it is allocated at block entry, has
     no value until the literal is evaluated, may be jumped over (0019 item 6; one directly in a `switch` body is
     allocated before the dispatch, as 0019 item 5 places flags), and is exited at every way out of the block (0038 item
     7). **The literal's evaluation follows 0043 item 6's rule** (audit B1): every element is evaluated first, as the
     unsequenced operands of one full expression (0039's and 0021's rules; `&(struct S){x++, x}` is rejected like
     `x++ + x`), then every leaf is stored (zero leaves included) as writes inside the expression (0039 item 2), then the
     address is the value. Each hidden object is initialized at most once per lifetime and is never read before (the
     rejections below). Messages name the literal by its source text, never the hidden local (invariant 7).
   - **Rejected:** a literal in a loop condition, a `for` step or a `do` condition (it is evaluated again without
     re-entering its block; M8, listed); `return &(T){…}` (0039 item 6's R14, `return_local_address`, in both modes: EDG
     warns only in C, P67 H3); `&(S){…}` and `&(int){5}` in C++ (a temporary: clang and GCC refuse it, P68; EDG accepts it
     silently, so the rejection is cerastes's, P67 H3); the address of a `const`-qualified literal (`&(const int){1}`:
     C23 6.5.2.5p7 lets such literals share one object, so two hidden locals would prove `p != q` where an implementation
     may merge them; a `const` object, M4.6; audit B2); a literal used as an lvalue without `&` (`(struct S){1, 2}.x = 3;`,
     `(int){1}++`, `(int){1} += 2`, `(struct S){0} = t;`; M8); file-scope literals (static storage: EDG makes `&(int){5}`
     a compiler-generated static variable and `struct S gs = (struct S){1, 2};` an aggregate flagged
     `is_compound_literal`, P67 H5; also as a `static` local's initializer); array literals (M4.5e1). C23 storage-class
     literals (`(static int){5}`, `(constexpr int){5}`) never reach cerastes: EDG refuses them (P67, P68). A `volatile` or
     `_Atomic` literal is rejected as a value too (M8, as 0043 item 2's qualified members; audit round 2, S1).
   - **Unevaluated operands** (`sizeof`, `typeof`, `_Generic`, `alignof`) create no hidden object and store nothing.
8. **Struct globals** (amends 0041 items 2–5, 7, 10, 11).
   - **Layout:** a struct global's block `GB_g()` holds the slot `loc(GB_g(), 0)` with its member blocks (0043 item 3).
   - **Facts:** the global facts (every method's first `requires`, and `program_start`'s first `ensures`) gain, for each
     struct global, the member lengths `len(member(X, k)) == 1` of every slot `X` of `loc(GB_g(), 0)` and the slot facts
     `X != Y` for every nested slot `Y` below a slot `X` (0043 items 4 and 8). The prelude has no `len >= 0`, so
     `WF(&g.x)` needs the lengths (0041 item 4's argument for a scalar global's `len`). They appear only in units with a
     struct global, so no program accepted before this record changes its Viper.
   - **`program_start`:** one row per struct global: for every leaf `L`, `acc(L.v_T) && acc(L.d_T) && L.d_T && <value>`,
     then exactly one `acc(lvloc(GB_g()).lv)`. The value is the leaf's initial value where 0041 item 3 knows it (a
     literal, `null`, `loc(GB_h(), 0)`), else its range, `WF` or `WF_S`.
   - **Initial values** by linkage (question 84) per leaf: internal linkage and C's external linkage with an initializer
     get the initializer's values, zero or null for a leaf it does not name; C's external tentative definition and C++'s
     external linkage get unknown values. **Initializers** are braced lists of constant elements (0043 item 6's forms and
     element rules, every element checked in the prologue as 0041 item 5 checks a scalar's, named `g.x`); a pointer leaf
     may be null or `&h` (0041 item 5, with its escape rule per leaf: C++'s `static S g{}; T t = {&g};` with `t` of
     unknown value makes `g`'s value unknown); `&h.x` is rejected (M8; P67 H9: told from `&h` by the constant's
     `subobject_path`, never by its offset); a C++ dynamic initializer, and an element that is a folded `constexpr` call,
     are rejected as 0041 item 2 rejects them. A `const` struct global is a `const` object (M4.6).
   - **Code:** `g.x`, `g.t.y`, `&g`, `&g.x`, copies to and from `g`, `h(g)` and `g = f()`. Messages name the place (`g.x`).
   - **Contracts:** `owns(g)`, `reads(g)` (every leaf, one liveness fraction), `owns(g.t)`, `owns(g.x)`, `reads(g.x)`,
     reads of `g.x`, `old(g.x)` and `decreases(g.x)`, with 0043 item 9's overlap rule per root (a global's root is its
     block) and 0041 item 7's rule of one atom of a kind per place.
   - **Loops:** a held struct global (every global in `main`) gives every leaf as an automatic place (0041 item 10, 0043
     item 11), one liveness row per root. A write by name to a member of a global (`g.x = …`, `g.t = …`: an address that
     is a member chain over the global's address, with no load) reaches exactly the leaves it names, as a write to a
     scalar global by name reaches only that global (amends 0038 item 3's rule that a write reaches every place of its
     field), **where the function holds those leaves of `g` as its own automatic places** (in `main`, or under a
     `requires` atom on `g` covering them): its write permission then keeps every other place apart (another leaf of `g`
     is another member block, another global another block, a local another block). Elsewhere — a function that reaches
     `g` through a parameter's atom (`owns(*s)` with `s == &g`) or a written atom on a reassigned parameter — the write
     stays field-wide (re-audit S1: those places may hold the leaf). A missed reach could only strengthen a row Silicon
     checks, never assume a value. A write through a pointer stays field-wide, and so do writes to the members of struct
     locals (0043 item 11, unchanged: more precise, but it would change the Viper of programs accepted before this
     record).
   - **Hazard H8, closed by S34 (d):** two globals' blocks are told apart without a new fact. If a member block of
     `g`'s slot were `GB_h()`, `member_lv` would give it `g`'s liveness cell `lvloc(GB_g()) == loc(GB_g(), -2)` (0041
     item 4's fact), while `h`'s fact gives `lvloc(GB_h()) == loc(GB_h(), -2)`; `loc_inv` then makes `GB_g() == GB_h()`,
     which the `unique` functions refute. S34's loop over a scalar and a struct global and its `&g.x != &h` comparison
     verify with today's facts plus item 8's member lengths. No rule assumes the distinctness anyway (the shortcuts that
     treat two globals as two blocks only skip checks and facts).
9. **`static` struct locals** (amends 0041 item 12). A `static` local of a verified structure is a struct global whose
   name only its function sees: its initializer is item 8's, run once before `main`, and question 80's implicit `owns`
   in its function's and every transitive caller's contract is a struct atom that expands per leaf. A `static const`
   struct local (M4.6) and one initialized from a compound literal (a GNU extension) are rejected.
10. **Function measures over members** (amends 0039 item 8). `decreases(p->x)` (a member leaf of a pointer parameter's
    object), `decreases(g.x)` and `decreases(s.x)` (a by-value parameter's leaf, a plain formal) are accepted where
    `decreases(*p)` is, through the same framing and termination rows.
11. **`restrict`** (amends 0037 item 2 and 0043 item 10). A by-value struct parameter whose leaves can hold a pointer from
    which a `T*` can be formed (an `int *` leaf, an `int **` leaf, a `struct T *` or `struct T **` leaf whose `T` has an
    `int` leaf, at any depth) is one of the places beside which a `restrict T*` parameter is rejected, since a pointer
    passed in it is not based on the `restrict` parameter; so is a struct global named by the contract (`owns(g)` with an
    `int *` leaf in `g`), which 0043 item 10's global rule already covers. The check also runs after the implicit atoms of
    `static` locals (item 9) are added.
12. **Loop invariants and measures that read through a loaded pointer** (P69; audit S5). Only `**pp` is rejected in a
    loop invariant or measure, by its spelling (0040 item 5); `p->q->x`, `*p->q` and `*gp` have always reached the loop's
    read chains (0038 item 6), unpinned, and a by-value parameter's `*s.q` joins them. They are accepted, by this argument:
    every read in an invariant or a measure, the inner loads included, is lowered as the guarded chain `ite(ptr == place
    && k > none, place.v, dummy)` over the automatic places and the written atoms (0038 item 6, 0040 item 5), and the
    framing assert asserts the disjunction of the guards for every recorded read at entry and at the back edge; the
    loaded pointer is a value of the chain, so a read through it is framed only when it equals a held place, and nothing
    is assumed (no inhale, no axiom). P69 confirms it end to end: in both modes every positive verifies and every twin fails
    for its reason (`invariant-entry`, `invariant-preserved`, `termination`, `permission`), including traps that move the
    loaded pointer inside the loop; the shapes are pinned. `**pp` stays rejected (M4.5c2b, with the contract places).
13. **The lint** (`lint_viper.py`).
    - `program_start`'s row of a struct global is pinned by its shape, as `alloc_struct_*`'s is (0043 item 13): every
      chain canonical and rooted at `loc(GB_g(), 0)`, each leaf once and no leaf's chain a prefix of another's, the value
      grammar per leaf, exactly one `lv` on `lvloc(GB_g())`, nothing else; an `Addr` value from a scalar pointer field
      only to a scalar global and from a `ptr_struct_*` field only to a struct global, and one field's targets all of one
      row shape. The `.vpr` names no struct type on a global, so a target of another struct type with the same shape
      lints clean (audit round 2, S4): the type match is the encoder's and `check_program`'s (the leaf's pointee type
      equals the target global's), and Viper's field typing makes a wrong target unusable.
    - The facts row is exactly the scalar facts, the member lengths of the unit's struct globals and their slot pairs (and
      nothing else; S34 (d) showed H8 needs no `parent_of` fact).
    - `lint_perm_locals` sees every target of a multi-target call (closing a standing gap).
14. **Categories:** none new. Messages name leaves in source terms, as 0043 item 7's copies do: a member without a value
    passed or returned is "`s.y` may be read here before it is given a value", reported at the call or the `return`;
    `r.x`, `g.t.y` likewise. (0043 item 7 and the Error messages page quoted "copies `s`, whose member `s.y` may not have
    a value", a text the tool never printed; the page is corrected to the printed message.)
15. **Still rejected** (fail closed; each pinned): item 1's c2b, d2, e1, M4.6, M4.5f1, M7 and M8 rows; item 3's and item
    4's rejected contract forms (whole struct values, `old(r.x)`, atoms on by-value members, `*s.q`, `live(s.q)`,
    `reads(*s.q)`, `live(r.q)`); item 5's values and bases; item 7's literals; item 8's initializers; item 9's `static`
    locals; C++ by-value copies the IL shows as a call of a copy constructor (none in P67: trivially copyable structures
    copy by value, 0043 item 7).
16. **Facts relied on** (`internal/il-inventory.md`, "M4.5c2a"): (P67) the IL of by-value parameters, arguments,
    results, temporaries, compound literals and static-storage struct initializers in both modes (PASS for all seven
    questions; hazards H1–H9 taken in items 4, 6–8); (P68) the compiler matrix (clang 21, GCC 16, both modes, three tiers:
    only `h2(s, s = t)` warns; literal assignments, partly initialized by-value structs and struct-valued calls are
    silent; C++ `&(S){…}` is an error in both compilers); (P69) the invariant heap-read gap (44 programs, 90 runs, no wrong
    verdict, no exit 3); (S34) Silicon on multi-result methods, the by-value entry copy, struct global facts and rows,
    H8, `static` struct locals and hidden literal objects (31 fixtures, PASS; two fixture expectations corrected by hand
    and reported: a twin's continuation errors, and the H8 control, which verified without the `parent_of` fact and is now
    a positive).
17. **Tests and documentation.** Must-verify and must-fail pairs for every accepted form in both modes (`.c` files run as
    C23 and C++20 where valid), including the ports of VeriFast's `tests/call_in_struct_init.c` and
    `examples/struct_assignment.c` (`test1`–`test6`); every rejection of item 15. Docs: Memory and ownership
    ("Structures", "Global variables", "`static` local variables", the not-yet list), Annotations, Error messages,
    Verifying a program; `internal/encoding.md`.
18. **Flips** (questions 98 and 99, pre-approved). Retired for positives of the same shape:
    `reject_struct_by_value_param.c`, `reject_struct_by_value_result.c`, `reject_struct_temporary_init.cpp`,
    `reject_struct_init_compound_literal.cpp`, `reject_struct_static_local.c`, `reject_struct_decreases_member.c`,
    `global_struct.c` (its `@ L13` line), `reject_struct_compound_literal.c` and `reject_compound_literal_address.c`
    (their positives are C only; each gains a C++ rejection pin); question 101: the VIR self-test rejection cases
    `struct_param_by_value` and `struct_result_by_value` (`selftest_cases14.inc`) retired for lowering cases of the same
    shapes. Re-messaged naming M4.5c2b, every `expect:` line kept:
    `reject_owns_double_deref.c`, `reject_read_double_deref.c`, `reject_read_double_deref_invariant.c`,
    `reject_atom_on_cell_pointer.c`, `global_pointer_global_deref_atom.c`, `reject_struct_member_chain_contract.c`,
    `reject_struct_atom_in_invariant.c`.

## Consequences

- C programs that pass and return structures, build them from compound literals and keep them in globals verify, with
  every member's value tracked across calls; VeriFast's `result.x` is `r.x`, its `(struct foo){…}` values are compound
  literals, and its `result == (struct foo){40, 50}` is written member by member.
- A struct value costs one Viper value per leaf at every call and return; struct globals add their member lengths to
  every method's first `requires` in their unit.
- Passing or returning a partly initialized structure is reported, stricter than C, as copying one is (question 92).
- Contracts that read through a pointer member, a pointer to a pointer or a pointer global wait for M4.5c2b.
