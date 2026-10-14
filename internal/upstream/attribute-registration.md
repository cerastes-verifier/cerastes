# DRAFT for the owner to file (sessions never open upstream issues, PRs or Discussions: decision 0003 item 6)

CONTRIBUTING.md says feature requests start as a GitHub Discussion, and a
maintainer opens an issue after consensus. So this is written as a Discussion.
Sections 1 to 9 and Appendix A are the post. The "For the owner" part at the
end (upstream state, PR series, CONTRIBUTING checklist, probes) is not part of
the post. Nothing here is EDG code; no fork commit exists for it.

Evidence conventions: every command was run against EDG
`9847356d461d2bef846bf133e16821f0e8b69749` (`7.0-25-g9847356d46`, the cerastes
pin), built in the cerastes configuration (5.1). Line numbers are for that
commit. Probes are in `internal/upstream/attribute-registration-probes/`; the
commands below are run from the cerastes repository root. Source-line
references to VeriFast are to `mborland/verifast` branch `generic2`
(`0fb63506a51cae559ae92348eb54ba7f7be0bf6d`), read only.

Revision note: this version reflects two owner clarifications. (1) The scope is
not the first attributes cerastes needs: it is a general mechanism that can host
a complete deductive-verification specification language for C and C++, at
least at VeriFast's level and beyond. (2) The specification language is C++
itself, written inside standard attributes. That is the difference from
VeriFast, and it is why EDG needs no new expression grammar.

---

## 1. Title

**A way for an external back end to register attribute namespaces and
attributes, so that C++ specification languages can live in `[[ns::name(...)]]`
(placement on every entity and statement, typed arguments with scopes and
binders, template-aware IL, redeclaration merging)**

## 2. Summary

cerastes is a deductive verifier for C and C++ (EDG front end, then Viper and
Silicon). We want to replicate everything VeriFast can do, and more: generic
template verification, contracts on concept requirements, and a memory model
with ownership. VeriFast writes its specifications in comments, in a language
that is not C++ (`|->`, `&*&`, `?x`). We want specifications that an ordinary
C++ programmer can read, so cerastes writes them in C++, inside standard
attributes, using spec functions (`owns`, `old`, `forall`) declared in a header.

That choice means EDG does all the parsing, lookup, overload resolution and type
checking of specifications, and we ask for no new tokens, operators or
expression grammar. What we need is a small set of general facilities around
attributes:

1. register a namespace and attributes from outside `attribute.c`;
2. place them on any entity or statement, and keep them across redeclarations;
3. typed C++ arguments, with scopes and binders (parameters, `this`, the result);
4. dependent arguments in template prototypes, substituted ones in instances;
5. a way to mark a declaration as specification-only.

Much of this already exists inside EDG for individual built-in attributes (Clang's
`enable_if`, C++23 `[[assume]]`). Section 5 shows what does and does not work today,
with probes. Section 6 proposes a phased extension mechanism. Our first use is a
set of eight attributes; the same mechanism has to carry the rest of a
VeriFast-class language later. We would like feedback on the design before
writing any code.

## 3. The idea in one example

VeriFast:

```c
void bump(counter* c)
//@ requires c->val |-> ?v &*& v < INT_MAX;
//@ ensures c->val |-> v + 1;
{ c->val += 1; }
```

cerastes:

```cpp
void bump(counter* c)
  [[cerastes::requires(cerastes::owns(c->val) && c->val < INT_MAX)]]
  [[cerastes::ensures(cerastes::owns(c->val) && c->val == cerastes::old(c->val) + 1)]]
{
  c->val += 1;
}
```

Everything inside the attribute is C++. `cerastes::owns` and `cerastes::old` are
ordinary function templates declared in a header (`<cerastes.h>`). For an
ordinary compiler the attribute is ignored (and the header can make the macros
expand to nothing). For cerastes, EDG type-checks the expression like any other,
and the verifier gives meaning to the calls (`owns` is a permission, `&&` is
separating conjunction in that context, `old` is the pre-state).

**Design constraint: no new tokens, operators or expression grammar are asked of
EDG.** If the specification language needs a new construct, it becomes a
function in a header, an attribute, or a lambda, not new syntax.

## 4. What the front end has to provide

The VeriFast annotation language is a good measure of the breadth, because it is
the most complete one for C and C++ that we know. Appendix A inventories it by
category, with the VeriFast source for each feature and one possible C++ form. For EDG,
the breadth reduces to the following requirements, because the arguments are
plain C++.

| # | Requirement | Which features need it | What EDG offers today |
|---|---|---|---|
| R1 | An external tool registers its own namespace and attribute names, without editing `attribute.c` | all | Nothing: namespaces are a static list (`attribute.c:970-977`). Unregistered ones can be recorded as raw tokens behind a build macro, with a warning (5.2, 5.3) |
| R2 | Placement on every entity and statement kind a specification can attach to | contracts (function, constructor, destructor, method); invariants (loops); ghost statements (null statement); declarations of spec-only entities (function, class, field, variable, typedef, enum, namespace); base class; parameter; lambda | Mostly. 17 kinds recorded (5.4). Not accepted: attributes on template parameters (misparsed, 5.4), before or after the word `concept`, or inside a requires-expression (the grammar has no such position) |
| R3 | An attribute survives redeclaration, merged by a per-attribute policy (accumulate or must match) | contracts on a declaration plus its definition; templates; members defined outside the class | After the parameter list: the IL keeps one declaration's attribute, on the type, and drops the others (5.2). Prefix: accumulates. GNU-style postfix: accumulates (5.3c). Nothing is selectable per attribute |
| R4 | Arguments are C++ expressions that EDG looks up, overload-resolves and type-checks, with errors at the attribute's own line | every assertion, contract and invariant | Yes, for attributes with an expression signature (`enable_if`, `assume`): undeclared names and bad calls are diagnosed at the attribute line; `this`, lambdas and templates work (5.3a) |
| R5 | Scopes and binders: parameters, `this`, the result, pre-state, quantifier variables | `requires`/`ensures`, `old()`, `ensures(r, p)`, `forall` | Parameters, `this` and lambdas: yes (5.3a). Pre-state needs nothing (`old(e)` is a call). A binder for the result: no. Deferral until a deduced return type is known: not examined |
| R6 | Dependent arguments in a template prototype, substituted ones in each instance | generic verification (verify once), per-instantiation fallback, concept requirement contracts | Yes for expression-signature attributes: dependent in the prototype, concrete in the instance (5.3b). Raw tokens only for unrecognized ones |
| R7 | Spec-only entities: type-checked and in name lookup, flagged so a back end never emits them, and without storage | predicates, pure functions, lemmas, ghost fields and variables, spec-only types | No marker. A ghost field written as an attributed field still takes storage (5.4). Spec function templates are instantiated normally |
| R8 | Diagnostics for a misplaced attribute or an attribute on the wrong entity | all | The descriptor target string does this for built-in attributes (`known_attr_appl_table`, `attribute.c:702`). A registered namespace must keep it, or ill-placed attributes become silent |

The first use (cerastes milestone M2) needs R1, R2, R3, R4, R5 (including the
result binder), R6 and R8. R7 comes with later milestones (predicates, pure
functions, memory model).

## 5. What EDG offers today, and why it is not enough

### 5.1 Configuration

The cerastes configuration is the macros in our `edg_config` CMake target; the
stock presets do not set them:

```
$ build/cerastes-fe --dump_configuration 2>&1 | grep -E '#define (RECORD_UNRECOGNIZED_ATTRIBUTES|PROTOTYPE_INSTANTIATIONS_IN_IL|ALL_TEMPLATE_INFO_IN_IL|EXTRA_SOURCE_POSITIONS_IN_IL|INCLUDE_EDG_TEST_ATTRIBUTES|DO_IL_LOWERING|BACK_END_IS_C_GEN_BE) '
#define ALL_TEMPLATE_INFO_IN_IL 1
#define BACK_END_IS_C_GEN_BE 0
#define DO_IL_LOWERING 0
#define EXTRA_SOURCE_POSITIONS_IN_IL 1
#define INCLUDE_EDG_TEST_ATTRIBUTES 1
#define PROTOTYPE_INSTANTIATIONS_IN_IL 1
#define RECORD_UNRECOGNIZED_ATTRIBUTES 1
```

(`--dump_configuration` writes to stderr, hence `2>&1`.) The stock
`macos-arm-clang-debug` build (`build/edg/bin/cpfe --dump_configuration 2>&1 | grep ...`)
prints `RECORD_UNRECOGNIZED_ATTRIBUTES 0` and `ALL_TEMPLATE_INFO_IN_IL 0`, so it
drops them with a warning: for `[[cerastes::x]]` stock `cpfe` prints
`attribute namespace "cerastes" is unrecognized` (`prefix_ns.cpp`; the
`unknown attribute "cl::x"` warning in open question 10 is a different case). We
have NOT re-run the reproduction below on a stock preset rebuilt with only
`-DRECORD_UNRECOGNIZED_ATTRIBUTES=1`; to reproduce, set the macros above (open
question 6). `scripts/cerastes-fe.sh` runs that front end plus a back end that
prints a listing of the IL (`--cerastes_walk`); it adds `--clang_version=210000`
and the system include paths. `--il_display` is EDG's own dump.

### 5.2 Reduced reproduction (12 lines): of several declarations' contracts after the parameter list, the IL keeps one

`postfunc_redecl.cpp`:

```cpp
// Reduced reproduction: attributes written after a function's parameter list.
// Compile as C++20.
int g(int x) [[cerastes::req(x > 0)]];     // line 3: declaration
int g(int x) [[cerastes::req(x < 100)]];   // line 4: redeclaration
int g(int x) { return x; }

template <class T>
T f(T v) [[cerastes::req(v > 0)]];         // line 8: template declaration
template <class T>
T f(T v) [[cerastes::req(v < 100)]] { return v; }  // line 10: definition

int use() { return g(1) + f(2); }
```

```
$ scripts/cerastes-fe.sh --c++20 --cerastes_walk postfunc_redecl.cpp
...postfunc_redecl.cpp, line 3: warning: attribute namespace "cerastes" is unrecognized
...                       (the same warning for lines 4, 8 and 10)
routine g plain body @ L5:5
  returns int
  param 1 x: int
routine f prototype body @ L10:3
  returns T
  param 1 v: T
  attr [[cerastes::req(v > 0)]] on type @ L8:12
routine f instance body @ L10:3
  returns int
  param 1 v: int
  template f args <T=int>
  attr [[cerastes::req(v > 0)]] on type @ L8:12
routine use plain body @ L12:5
  returns int
end walk: 4 routines reported, 0 routines with bodies in non-system files outside the primary file
```

- Four attributes were written; two are in the IL. `g` has none, because its
  defining declaration (line 5) has no attribute and the earlier declarations'
  are not kept. `f` has only the first declaration's (line 8), on the function
  **type**, not on the routine. The line-10 attribute on the definition is gone.
- `--il_display` agrees: exactly two `attribute` entries, both
  `syntactic_location: post func` (`al_post_func`, `il_def.h:2889`),
  `namespace_name: "cerastes"`, `name: "req"`, `position.seq: 8`,
  `position.column: 12`; each hangs off a `kind: tk_routine` type's
  `source_corresp.attributes` (`il_def.h:3610`), not off the routine.
- The argument is recorded as raw tokens (`v`, `>`, `0`): no lookup, no types.
- Which declaration's attribute survives depends on the case. `postfunc_matrix.cpp`
  writes an attribute on each declaration (`d` first declaration, `r`
  redeclaration, `f` definition) and the walker shows what is kept:

  | Case | Written on | Kept |
  |---|---|---|
  | non-template, declaration then definition | `c1d`, `c1f` | `c1f` (the definition's) |
  | non-template, definition has none | `c2d` | none |
  | non-template, declaration, redeclaration, definition has none | `c3d`, `c3r` | none |
  | non-template, declaration, redeclaration, definition | `c4d`, `c4r`, `c4f` | `c4f` |
  | member declared in the class, defined outside | `m1d`, `m1f` | `m1f` |
  | template, declaration then definition | `t1d`, `t1f` | `t1d` (the first declaration's) |
  | template, first declaration has none | `t2f` | none |
  | template, declaration, redeclaration, definition | `t3d`, `t3r`, `t3f` | `t3d` |

  So the IL keeps at most one declaration's attribute, always on the type and not
  on the routine: the defining declaration's for a non-template function or an
  out-of-class member, the first declaration's for a template. Every other
  declaration's attribute is dropped. A contract split between a declaration
  (the usual place for a header's contract) and a definition cannot be read back.
- A prefix attribute on the same declarations is kept and accumulates across
  redeclarations (`tests/il/inventory/attr_prefix_redecl.cpp` in the cerastes
  repository), so only this placement is affected. Contracts are conventionally
  written after the parameter list, as in the example in section 3.

### 5.3 What already works, one built-in attribute at a time

**(a) Typed expression arguments with parameters, `this`, lambdas and spec
functions in scope.** Clang's `enable_if` takes an expression (`sig` `"(X,sn)"`,
`attribute.c:270`) and is applied to the function type (`apply_enable_if_attr`,
`attribute.c:5954`, with a deferred check at `attribute.c:5988`). Using it as a
stand-in for our attributes, with the example from section 3 (`spec_calls.cpp`,
12 lines):

```cpp
namespace cerastes { template <class T> bool owns(const T&); template <class T> T old(const T&); }
struct counter { int val; };
void bump(counter* c)
  __attribute__((enable_if(cerastes::owns(c->val) && c->val < INT_MAX, "pre")))
  __attribute__((enable_if(cerastes::owns(c->val) && c->val == cerastes::old(c->val) + 1, "post")));
```

```
$ scripts/cerastes-fe.sh --c++20 --cerastes_walk spec_calls.cpp     # exit status 0, no diagnostics
routine cerastes::owns prototype nobody @ L4:25
  returns bool
  param 1 _: const T &
routine cerastes::owns instance nobody @ L4:25
  returns bool
  param 1 _: const int &
  template cerastes::owns args <T=int>
routine cerastes::old prototype nobody @ L5:22
  returns T
  param 1 _: const T &
routine cerastes::old instance nobody @ L5:22
  returns int
  param 1 _: const int &
  template cerastes::old args <T=int>
routine counter::operator= plain implicit nobody @ L7:8
  returns counter &
  param 1 _: const counter &
routine counter::operator= plain implicit nobody @ L7:8
  returns counter &
  param 1 _: counter &&
routine bump plain nobody @ L9:6
  returns void
  param 1 c: counter *
  attr [[enable_if(<aak_expression> <aak_constant>)]] on type @ L10:18
  attr [[enable_if(<aak_expression> <aak_constant>)]] on type @ L11:18
routine bump plain body @ L12:6
  returns void
  param 1 c: counter *
end walk: 8 routines reported, 0 routines with bodies in non-system files outside the primary file
```

Both attributes are on the function type, and `cerastes::owns<int>` and
`cerastes::old<int>` were instantiated. `bump` appears as two routines (the
declaration at line 9 and the definition at line 12) because `enable_if` takes
part in overloading, so the definition without the attributes is a distinct
function.

Must-fail twins, in `spec_calls_fail.cpp` (same declarations of `cerastes::owns`
and `old`). EDG diagnoses them at their own lines and exits with status 2:

```
$ scripts/cerastes-fe.sh --c++20 --cerastes_walk spec_calls_fail.cpp     # exit status 2
"spec_calls_fail.cpp", line 8:
          error: namespace "cerastes" has no member "owns_all"
  void bad1(counter* c) __attribute__((enable_if(cerastes::owns_all(c->val), "undeclared")));
                                                           ^

"spec_calls_fail.cpp", line 9:
          error: no instance of function template "cerastes::owns" matches the
          argument list
            argument types are: (int, int)
  void bad2(counter* c) __attribute__((enable_if(cerastes::owns(c->val, 1), "wrong arity")));
                                                 ^
"spec_calls_fail.cpp", line 4:
          note: number of parameters of function template "cerastes::owns" does
          not match the call
  template <class T> bool owns(const T&);
                          ^

2 errors detected in the compilation of "spec_calls_fail.cpp".
```

(Paths in the real output are prefixed `internal/upstream/attribute-registration-probes/`.)

The same redeclaration behaviour shows for `enable_if` (`enable_if.cpp`): for
the non-template `g` the attribute of the defining declaration (line 2) is kept,
for the template `f` the first declaration's (line 4) is kept and the
definition's (line 6) is not.

`expr_args.cpp` shows `this->val > 0` in a member function and a lambda call
(`[](int i) { return i > 0; }(n)`) accepted in the argument, with no diagnostic.
The lambda becomes an ordinary closure type and routine in the IL
(`routine <lambda@L7:41>::operator() plain body @ L7:43`), so a back end has to
know it belongs to a specification.

**(b) Dependent arguments in the prototype, substituted ones in the instance.**
For a template (`expr_args.cpp`, `enable_if(v + 1 > 0, ...)`), `--il_display`
shows two different expression trees: the prototype's is `eok_bool_cast` with
`operation.type_kind: tk_template_param`, the instance's is `eok_gt` with
`operation.type_kind: tk_integer`. The same file's walker output lists the
attribute on both the prototype and the instance routine.

**(c) Entity-attached, accumulating post-declarator attributes.**
`declarator.c:98-118` moves a GNU attribute written after a function declarator
to the entity (`dps->id_attributes`, `syntactic_location` set to
`al_postfix`/`al_id_equivalent`) when `is_gcc_attribute(ap)` holds
(`attribute.h:112`: `af_gnu`, or a std-syntax attribute in the `gnu` namespace).
Probe `gnu_postfix.cpp` (same shape as 5.2, `__attribute__((cr_req(...)))`):

```
routine g plain body @ L3:5
  attr [[cr_req(x > 0)]] on routine @ L1:29
  attr [[cr_req(x < 100)]] on routine @ L2:29
routine f prototype body @ L7:3
  attr [[cr_req(v > 0)]] on routine @ L5:25
  attr [[cr_req(v < 100)]] on routine @ L7:25
routine f instance body @ L7:3
  attr [[cr_req(v > 0)]] on routine @ L5:25
  attr [[cr_req(v < 100)]] on routine @ L7:25
```

`[[gnu::cr_req(...)]]` (`gnu_std_postfix.cpp`) behaves the same, with no
diagnostics.

**(d) A standard-syntax expression attribute on a statement.** C++23
`[[assume(expr)]]` on a null statement inside a template shows
`stmt stmk_empty @ L5:3 attr [[assume(<aak_expression>)]]` in both the prototype
and the instance (`assume_stmt.cpp`, `--c++23`). It is the closest existing
analogue to a registered `check(p)`.

### 5.4 Where an attribute can be placed today, and what it cannot do

`placements.cpp` puts one unrecognized `[[cerastes::a_...]]` on each kind of
entity or statement. The walker records all of these (rc 0):

namespace-scope variable, typedef (see below), enum, enumerator, namespace, class
(after the class-key), derived class, base class, field, method, constructor,
destructor, static data member, parameter, local variable declaration, return
statement, lambda (on its type). Together with the placements in the cerastes IL
inventory (function declaration prefix, loop and null statements, class-template
members), this covers every placement in Appendix A, with the exceptions below.
None of them is a placement a specification language needs, except possibly the
concept ones (open question 9).

- **Typedefs.** `placements.cpp`'s form (`typedef int [[x]] t;`) is recorded as
  `on typedef <unnamed>`: it attaches to the attributed type. `typedef_forms.cpp`
  shows which spellings attach to the named typedef:

  ```
  $ scripts/cerastes-fe.sh --c++20 --cerastes_walk typedef_forms.cpp     # exit status 0
  attr [[cerastes::ty1]] on typedef t1 @ L2:3
  attr [[cerastes::ty2]] on typedef t2 @ L3:18
  attr [[cerastes::ty3]] on typedef <unnamed> @ L4:15
  attr [[cerastes::ty4]] on typedef t4 @ L5:12
  attr [[cerastes::ty5]] on typedef <unnamed> @ L6:18
  end walk: 0 routines reported, 0 routines with bodies in non-system files outside the primary file
  ```

  `[[x]] typedef int t1;` (prefix, ty1), `typedef int t2 [[x]];` (after the
  declarator-id, ty2) and `using t4 [[x]] = int;` (ty4) attach to the named
  typedef. `typedef int [[x]] t3;` (ty3) and `using t5 = int [[x]];` (ty5)
  attach to the type.
- **Template parameters.** `template_param_attrs.cpp` (`--c++20`, 13 errors, exit
  status 2). On a non-type parameter (`template <[[cerastes::np]] int N>`) EDG
  warns `attributes may not appear here` and drops the attribute, with no error.
  On a type parameter (`[[x]] class T`, `class [[x]] T`, `[[x]] typename T`) EDG
  warns or errors the same way and then misparses the parameter as a non-type
  parameter of class type:

  ```
  line 6: error: a template parameter of class type must be of structural class type
  line 6: error: function "f2<<unnamed>>(T)" returns incomplete type "T"
  line 10: error: a class or namespace qualified name is required
  ```

  (`class [[x]] T` gives `error: attributes are not allowed here` instead of the
  warning.) Apple clang also rejects them: `error: an attribute list cannot
  appear here` for the non-type form, `non-type template parameter has
  incomplete type 'class T'` for the two `class` forms, and `expected a
  qualified name after 'typename'` for the `typename` form.
- **Concepts.** `concept_placements_rejected.cpp`: before a concept's `template`
  header, after the word `concept`, on a simple requirement and on a compound
  requirement inside a requires-expression are rejected, 6 errors covering all 4
  placements (the grammar has no attribute position there);
- accepted: after the concept's name (`concept C3 [[cerastes::p2]] = true;`,
  recorded as `attr [[cerastes::p2]] on template C3`), and on a constrained
  function template, before or after the parameter list
  (`concept_placements.cpp`: `on routine` and `on type`).

Missing today, with evidence:

- **Spec-only fields.** `ghost_field_layout.cpp`:
  `struct S { int x; [[cerastes::ghost]] int y; };
  static_assert(sizeof(S) == sizeof(int));` fails with `the final comparison was 8 == 4`.
  An attributed field is an ordinary field.
- **Silent drop of an attribute that appertains to nothing.** `[[cerastes::x]]
  struct S {};` is not in the IL. Today only the unrecognized-namespace warning
  betrays it (cerastes inventory 5b-class). After registration nothing would,
  unless the descriptor's target check applies (R8).
- **A result binder** and **deferral** until the return type is known (R5).

## 6. Proposal: a phased extensibility design

The goal is one mechanism that scales from the first eight attributes to a
complete specification language. All of it is design for discussion; names are
placeholders. M2's attributes (`requires`, `ensures`, `invariant`, `check`,
`fold`, `unfold`, `predicate`, `pure`) are the first use, not the scope. The
phases are ordered so that each can be proposed, reviewed and tested on its own.

### Phase 1: registration (R1, R8)

A registration hook called once from `attribute_init` (before
`init_attr_name_map`, `attribute.c:1246`), in the style of the `BACK_END_*`
macros: a macro `ATTRIBUTE_REGISTRATION_FN`, default undefined, naming a function
the configuration supplies. It calls:

```c
void register_attribute_namespace(a_const_char *ns);
void register_attribute(const an_ext_attr_descr *d);
```

```c
typedef struct an_ext_attr_descr {
  a_const_char *ns;      /* namespace, e.g. "cerastes" */
  a_const_char *name;    /* e.g. "requires" */
  a_const_char *sig;     /* same grammar as an_attr_descr.sig (X, Xb, t, n, sn, ...) */
  a_const_char *target;  /* same grammar as an_attr_appl_descr, e.g. "r|c|d" */
  unsigned      flags;   /* phases 2 to 5 */
} an_ext_attr_descr;
```

Registered namespaces are added to `attribute_namespace_is_recognized`
(`attribute.c:1084`), so the unrecognized-namespace warning goes away for them.
The target string reuses the existing "wrong entity" diagnostics (R8). A command-line option
`--attribute_namespace=<ns>` that only calls `register_attribute_namespace`
would be a much smaller first step, for tools that need recognition only.

### Phase 2: placement and redeclaration merging (R2, R3)

A descriptor flag (`EAF_ENTITY_ATTACHED`, name TBD): a std-syntax attribute in a
registered namespace takes the path that 5.3c shows for GNU attributes. It is attached to the
routine (`source_corresp.attributes` of the routine, not of its type) and accumulates in
declaration order across redeclarations, including a template declaration plus
its definition and a member defined outside its class. A per-descriptor merge
policy: **accumulate** (default for contracts: a redeclaration's contract adds to
the earlier one) or **must match** (compare `compatible_enable_if_attributes`,
defined at `types.c:8831-8872`, called from `class_decl.c:13180-13182`,
`symbol_tbl.c:10974`, `types.c:9692, 15695` and `trans_corresp.c:6915`). Statement and loop placement already work (5.4) and only
gain argument parsing (phase 3).

### Phase 3: typed arguments, scopes and binders (R4, R5)

Reuse the `sig` codes: `"Xb"` is already an unevaluated expression contextually
converted to `bool`. Parameters, `this`, lambdas and templates are already in
scope (5.3a). What is missing is a **binder**: `ensures(r, p)` declares `r` as
the function's result, usable in `p`. Two options: a new `sig` code (say `B`: "an
identifier, declared as a variable of the function's return type in a scope that
encloses the following arguments"), or a custom `scan_args` callback on the
descriptor with helpers (`ext_attr_declare_binder(name, type)`,
`ext_attr_scan_expression(bool)`). Different arities (`ensures(p)` and
`ensures(r, p)`) need either two descriptors with one name selected by argument
count, or the callback. If the return type is `auto` and not yet deduced when the
attribute is scanned, parsing would be deferred to the end of the declaration,
the way `enable_if`'s check already is (`add_end_of_parse_action`,
`attribute.c:5988`).

Pre-state needs nothing: `old(e)` is an ordinary call. Quantifiers need nothing:
a lambda argument already works (5.3a).

### Phase 4: template-aware IL (R6)

For registered attributes, keep what `enable_if` already does (5.3b): arguments
dependent in the prototype instantiation, substituted in each instance. The
change is to make that a tested guarantee for registered attributes, including
when the attribute is entity-attached (phase 2), with per-argument positions.
This is what lets a template be verified once, against the prototype.

### Phase 5: spec-only declarations (R7)

A generic way to say "this declaration takes part in lookup and type checking,
but is specification only": a flag on the entity in the IL (and a descriptor flag
that sets it), so a back end (and EDG's own code generators, layout and
`sizeof`) skip it. For functions, classes and variables this is a flag only. For
a field it changes the layout, which is a larger change; if EDG would rather not
do that, we can model ghost state without ghost fields (open question 4).

### What stays out of EDG

No `cerastes` string anywhere in `third_party/edg`. EDG's own tests exercise the
mechanism with the existing `edg` test namespace (`INCLUDE_EDG_TEST_ATTRIBUTES`,
`attribute.c:433-441`), for example new `[[edg::t7(...)]]` descriptors, so the
tests need no external back end.

## 7. Alternatives considered

1. **Hard-code `cerastes` in `attribute.c`.** Smallest EDG change, but a
   tool-specific namespace in the tree, and every other tool would ask for the
   same. Our fallback if this Discussion finds no general design.
2. **Stay on the unrecognized path and do our own parsing.** We would re-scan
   raw tokens in the back end, with no lookup, no types and no template
   handling, and still lose all but one declaration's contract (5.2). It duplicates the front end.
3. **Specifications in comments with their own grammar, as VeriFast does.**
   This needs a new grammar and a second parser, and cannot reuse EDG's
   lookup, overload resolution or template handling. It is also not C++, which
   is the point we are trying to avoid.
4. **A dynamic plug-in interface.** More general and much larger; EDG has no
   plug-in mechanism today.
5. **Use `[[gnu::...]]`.** It attaches to the routine, but covers almost none of the scope: raw tokens only
   (no types, scopes or binders), no template-aware arguments, and another
   tool's meaning in GCC's namespace. Rejected.
6. **Use `clang::enable_if` and `diagnose_if`.** Only conditions with a fixed meaning; `enable_if` also
   takes part in overloading (the definition of `bump` in `spec_calls.cpp`
   shows up as a second routine); no postconditions or binders.
7. **Write the IL and process attributes outside (`il_read`).** Does not solve
   parsing or the redeclaration loss.

## 8. Open questions for the maintainers

1. Is a general registration mechanism something EDG wants, or would
   maintainers prefer a dedicated per-tool compile-time descriptor set under a
   macro, still upstream?
2. IL representation: extend `an_attribute_kind` (`il_def.h:3144`, and the
   `il_display.c:6397-6400` switch), or a flag plus a descriptor index on the
   existing record? What are the consequences for `il_write`/`il_read` and the
   IL version?
3. Is a macro-named hook in `attribute_init` acceptable, or is there a preferred
   initialization point? Our back end only gets control in `back_end()`, after
   parsing; cerastes compiles EDG's front-end sources itself with its own
   `main`, which calls `edg_main`.
4. Spec-only declarations (phase 5): is a flag on the IL entity, honored by
   layout, acceptable, or should tools model ghost state without ghost fields?
   (We can do the latter, at the cost of a less direct specification syntax.)
5. Binder injection (phase 3): new `sig` code or callback? Is there an existing
   facility for introducing a name into the attribute-argument scope?
6. Should EDG ship a CMake preset or option for "keep unrecognized attributes and
   prototype instantiations", so reproductions need no hand-set macros?
7. Merge policy across redeclarations: accumulate or must match, and who
   decides, the descriptor or the caller? How should an attribute on a
   declaration whose definition differs only in the attribute (as for
   `enable_if`, where the definition becomes a distinct overload) be handled?
8. Should an unevaluated argument (`X`) instantiate function templates it names?
   Today it does (`cerastes::owns<int>` appears as an instance in 5.3a). That
   is harmless for an external back end that filters it, but may not be intended.
9. Attributes on concepts and requirements (our M5 spike): only "after the
   concept name" is accepted (5.4). Do you foresee changing that, or is a
   separate declaration form (an attributed function declaring the requirement's
   contract) the only route?
10. Adjacent finding, possibly a separate issue: `attribute_namespace_is_recognized`
    (`attribute.c:1084-1105`) compares `strncmp(*np, name, length)` with `length`
    the length of the spelled name, so a name that is only a prefix of a known
    namespace is accepted. We observed no unrecognized-namespace warning for
    `[[cl::x]]`, `[[gn::x]]` and `[[g::x]]` (`prefix_ns.cpp`, `--no_code_gen`),
    while `[[cerastes::x]]` warns. Only the namespace check is bypassed: stock
    `cpfe` still warns `unknown attribute "cl::x"` (also for `gn::x` and `g::x`),
    and with `RECORD_UNRECOGNIZED_ATTRIBUTES` on the attribute is recorded
    silently. Not verified whether the prefix match is intended.

## 9. What we will do

Once there is agreement on the shape, the cerastes owner would implement it phase
by phase on the `cerastes` branch of `mborland/compiler`, with tests, following
CONTRIBUTING.md. Our pinned submodule never records a fork commit: we wait for
each change to land upstream (invariant 5 of our project).

---

## Appendix A: what a VeriFast-class specification language needs

This is a working inventory for the Discussion, not the full language-support
inventory (cerastes milestone M7). It is for judging the breadth of the
requirements, and is not meant to be exhaustive.

### A.1 Sources

All from `mborland/verifast` `generic2` `0fb63506`, read only: the grammar in
`src/frontend/parser.ml` and `src/frontend/ast.ml`; how annotations enter the
C++ front end, `src/cxx_frontend/annotation_parser.ml:11-45` (the parser's
interface lists what C++ needs: `parse_func_contract`, `parse_functype_contract`,
`parse_loop_spec`, `parse_decls`, `parse_stmt`, `parse_functype_ghost_params`,
`parse_struct_members`, `parse_include_directives`) and `decl_translator.ml:54,94,120,353-362`,
`stmt_translator.ml:76,119`; `src/cxx_frontend/Readme.md`; `intro-c.md`;
`soundness.md`; the tests in `tests/cxx/` (templates, instance_preds,
virtual_methods, references, ghost_field.cpp); the library headers
`bin/list.gh` and `bin/threading.h`; examples (`examples/concurrent_cell.c`,
`examples/barrier.c`, `examples/decreases.c`, `examples/chat.c`); the tutorial
outline (`examples/tutorial/tutorial.docx`, `ecoop-tutorial.pptx`). Facts about
VeriFast's C++ front end: annotations reach it as raw comment text; it has no
exceptions (no `throw`, `try` or `catch` in `src/cxx_frontend/`, although
`ast.ml:887-888` has `Throw` and `TryCatch` for Java) and no concurrency tests
in `tests/cxx/`.

"One possible C++ form" is illustrative only. It is not a cerastes design
decision unless the third column says plan.md fixes it (plan.md M2's attribute
table, the spec functions in `<cerastes.h>`, M5 and M6).

### A.2 Features by category

| Feature | VeriFast form (source) | One possible C++ form | Status |
|---|---|---|---|
| **Contracts** | | | |
| precondition, postcondition | `//@ requires A; ensures B;` (`parser.ml:1598-1606`) | `[[cerastes::requires(p)]]`, `[[cerastes::ensures(p)]]` after the parameter list | plan.md M2 |
| result name | `result` in `ensures` (`parser.ml:2251`) | `ensures(r, p)` | plan.md M2 |
| pre-state values | `?v` bound in the precondition, scope extends into the postcondition (`intro-c.md`) | `cerastes::old(e)` | plan.md M2 (`old`) |
| termination | `terminates;` (`parser.ml:1590`), `decreases e;` (`parser.ml:1074, 2112`) | `[[cerastes::terminates]]`, `[[cerastes::decreases(e)]]` | not in plan; open |
| restrict callers to non-ghost | `nonghost_callers_only` (`parser.ml:1584`) | n/a (ghost code is not a separate caller in the C++ form) | open |
| **Assertions** | | | |
| points-to | `e |-> v`, `e |-?-> v` (maybe uninitialized) (`parser.ml:2304`) | `cerastes::owns(e)`, `cerastes::reads(e)` | plan.md M2/M6 (`owns`, `reads`) |
| separating conjunction | `A &*& B` (`parser.ml:2293`) | `&&`, given separating meaning by the verifier | owner's example |
| pattern binders | `?x`, `_` (`parser.ml:2257, 2271`) | the value is `e` itself or `old(e)`; existential choice (`?x` matched against the footprint) has no obvious C++ form | **open design point for cerastes, not EDG** |
| fractions | `[f]p` (`parse_asn0`) | an extra spec-function argument | open |
| conditional, switch assertions | `c ? A : B`, `switch (xs) {case nil: ...}` over inductive values (`ast.ml` `WSwitchAsn`) | C++ `?:`; `switch` has no expression form, use a pure function | open |
| quantifiers | `forall_(T x; e)` (`parser.ml:2238`) | `cerastes::forall([&](T x) { return e; })` | plan.md M6 (lambda-bound `forall`) |
| `emp` | `emp` | `true` | open |
| **Ghost declarations** | | | |
| predicates with input/output parameters | `predicate nodes(struct node *n; list<...> nodes) = ...`; `;` splits inputs and outputs (`PredTypeExpr`, `ast.ml:417`) | `[[cerastes::predicate]]` function; the input/output split has no obvious C++ form | plan.md M6 (predicate); split is open |
| predicate families, constructors | `predicate_family_instance`, `predicate_ctor` (`parser.ml:991-1005`) | overloads or templates plus the attribute | open |
| instance predicates in classes | `InstancePredDecl` (`ast.ml:937`); `tests/cxx/instance_preds/Stack.cpp` | member function marked `[[cerastes::predicate]]` | plan.md M6 |
| fixpoint (pure) functions | `fixpoint` (`parser.ml:1066, 1498`) | `[[cerastes::pure]]` function | plan.md M2 (`pure`) |
| inductive data types | `inductive list<t> = nil \| cons(t, list<t>);` (`parser.ml:973`, `bin/list.gh:19`) | an ordinary C++ type; pattern matching has no obvious form | **open for cerastes** |
| abstract types | `abstract_type` (`parser.ml:1062`) | opaque struct or template parameter | open |
| lemmas, auto lemmas | `lemma`, `lemma_auto(trigger)` (`parser.ml:1014-1043`; `bin/list.gh:42, 57, 61`) | `[[cerastes::lemma]]` function, proof as an ordinary body with attributed statements | not in plan; open |
| ghost fields, variables, globals | `//@ int y;` in a struct (`tests/cxx/ghost_field.cpp:3`); `Global` (`ast.ml`) | declaration marked spec-only (R7); or ghost state modelled outside the object | open (question 4) |
| ghost includes | `//@ #include "list.gh"` (`examples/mergesort_and_binarysearch.c:3`) | ordinary `#include` of a header | fits `<cerastes.h>` |
| type predicates, `typeid` | `type_pred_decl` (`parser.ml:1150`), `typeid` (`parser.ml:2624`) | type traits and `typeid` | open |
| **Ghost statements** | | | |
| open, close | `open P(...)`, `close P(...)` (`parser.ml:1746, 1759`) | `[[cerastes::unfold(P(...))]]`, `[[cerastes::fold(P(...))]]` on a null statement | plan.md M2 |
| assert | `assert A;` | `[[cerastes::check(p)]]` on a null statement | plan.md M2 |
| leak | `leak A;` (`parser.ml:1742`) | `[[cerastes::leak(...)]]` | open |
| split, merge fractions | `split_fraction`, `merge_fractions` (`parser.ml:1772, 1786`) | spec-function calls | open |
| lemma calls | ordinary calls in ghost code | an attributed null statement holding a call | open |
| ghost assignment | `this->y = 6;` in ghost code (`tests/cxx/ghost_field.cpp:9`) | attributed null statement | open |
| function-pointer chunks | `produce_lemma_function_pointer_chunk`, `duplicate_...`, `produce_function_pointer_chunk` (`parser.ml:1829-1858`; `examples/barrier.c:256`) | open | open |
| shared boxes, handles, actions | `box_class`, `handle_predicate`, `perform_action`, `create_box` (`parser.ml:1030, 1194, 1170, 1986, 1804`) | none | outside plan.md (concurrency is a non-goal) |
| **Loops** | | | |
| invariant | `invariant A;` after the loop header (`parser.ml:2085-2105`) | `[[cerastes::invariant(p)]]` on the loop | plan.md M2 |
| loop requires/ensures, decreases | `requires`/`ensures` loop spec, `decreases e;` (`parser.ml:2095-2117`) | attributes on the loop | open |
| **Callbacks** | | | |
| function-pointer specifications | function type with contract, `: ft(args)` clause (`parser.ml:1042, 1596`); `typedef lemma void f(...)()` (`examples/concurrent_cell.c:69`) | attributes on a function type or typedef | open |
| **C++ specifics** | | | |
| constructors, destructors | contracts on `Foo()` and `~Foo()` (`tests/cxx/ghost_field.cpp`, `instance_preds/Account.cpp`) | the same attributes on a constructor or destructor, with `this` in scope | plan.md M6 |
| class invariants | an instance predicate plus open/close in each method | predicate member plus fold/unfold | plan.md M6 |
| virtual functions | spec override check, `A_vtype`, `this->valid(&typeid(A))()` (`tests/cxx/virtual_methods/spec_override_check.cpp`) | not designed | plan.md M6: rejected for now |
| references | `tests/cxx/references` | encoded as pointers | plan.md M6 |
| exceptions | none in VeriFast's C++ front end | not designed | plan.md M6: rejected for now |
| templates | generic function verified once; integral type parameters (`src/cxx_frontend/Readme.md`, `tests/cxx/templates`) | the same attributes on a template; dependent arguments | plan.md M4 |
| concurrency, global invariants | `bin/threading.h`: `mutex`, `lock`, `locked`, lock predicates; fractions | none | outside plan.md |
| **Beyond VeriFast (plan.md)** | | | |
| generic template verification | n/a | contracts on a template's prototype, checked once; integral `T` axioms | plan.md M4 |
| contracts on concept requirements | n/a | M5: three candidate syntaxes (attribute on a compound requirement, a requirement-spec declaration, an attribute on the concept); the spike in 5.4 shows only "after the concept name" is accepted | plan.md M5 |
| ownership memory model | n/a | `owns`, `reads`, `old`, `owns_range`, lambda-bound `forall` | plan.md M6 |

### A.3 What each category requires of the front end

These are the properties asked for in R1 to R8, per category.

| Category | Where it is written | Arguments | Names in scope | Declares ghost entities? | Templates | Redeclaration |
|---|---|---|---|---|---|---|
| contracts | after a function's parameter list (also constructor, destructor, method, function type) | C++ expressions | parameters, `this`, `result` (binder), `old(...)` is a call | no | dependent in the prototype, substituted in instances | accumulate or must match; contracts of a declaration and its definition |
| assertions | inside contract, invariant and `check` arguments | C++ expressions with calls to spec functions | as above; quantifier variables via lambda parameters | no | as above | n/a |
| ghost declarations | on a function, class, field, variable, typedef, enum or namespace declaration | usually none; sometimes expressions (e.g. a trigger) | as for the declaration | yes: entities that take part in lookup and type checking but are never emitted (R7) | dependent, and per instance | accumulate (a predicate or lemma may be declared, then defined) |
| ghost statements | on a null statement (and on a declaration statement for ghost variables) | C++ expressions, calls | locals and parameters at that point | possibly (ghost locals) | dependent in the prototype | n/a |
| loop specifications | on a loop statement | C++ expressions | locals and parameters at that point | no | dependent | n/a |
| function-pointer and callback specs | on a function type or typedef | C++ expressions | the function type's parameters | no | dependent | must match |
| class specifications | on class members, constructors, destructors | as for contracts | `this`, members | yes (instance predicates, ghost fields) | per instantiation | member defined outside its class |
| concept requirement contracts (M5) | on a requirement (syntax undecided) | C++ expressions | the requirement's parameters | no | dependent by construction | n/a |

---

## For the owner (not part of the post)

### O.1 State of upstream

Upstream `main` is now `034385ba` ("Fix Python 3.6 compatibility issues"), 8
commits past our pin `9847356d` (re-verified with
`https://api.github.com/repos/edgcpp/compiler/compare/9847356d...main`: 8 commits,
40 files). The first, `4e739c28` ("Use pinned docker images"), is from 18:50Z on
2026-10-13; the other seven were pushed between 21:29Z and 22:23Z:
`7a0533db`, `b971e940`, `0d49bfb9`, `d05f214f` ("Implement expectation
inheritance"), `996d67fa` ("Update expectations"), `435b7d16`, `034385ba`. None
touches `src/`: the files are agent skills, workflows, `dev_tools/`,
`doc/source/testing.rst`, `util/eccp.sh` and `tests/expectations/*`. So
`attribute.c` and every line number above are the same at `034385ba` (checked
through the compare's file list; the line numbers were not re-read at that
commit). The fork's `cerastes` branch is still at `90b66472`, 22 commits behind
the pin (status.md): fast-forward it to `9847356d` (or the new head) before any
commit.

### O.2 Outline of the PR series (after a Discussion and an issue, `[GH #?]`)

One PR per phase of section 6, in this order. Each has its own `[GH #?]` and its
own `src/Changes` entry.

1. **PR 1, registration (phase 1).** `register_attribute_namespace`,
   `register_attribute` with `sig` and `target`, the `ATTRIBUTE_REGISTRATION_FN`
   hook in `attribute_init`, optional `--attribute_namespace=`. Tests, using the
   `edg` namespace: a registered attribute gets no unrecognized-namespace warning,
   an unregistered namespace still does (must-fail twin), a wrong-entity attribute
   is diagnosed, an ill-typed argument is diagnosed at its own line.
2. **PR 2, IL representation.** The representation chosen in open question 2,
   `il_display.c` printing, `il_write`/`il_read` if needed. Tests: round trip.
3. **PR 3, entity-attached placement and merging (phase 2).** The flag, the
   accumulate and must-match policies, sharing the GNU path of
   `declarator.c:98-118`. Tests: declaration plus redeclaration, template
   declaration plus definition, member defined outside the class (the cases
   `postfunc_redecl.cpp` and `postfunc_matrix.cpp` lose today), each with a must-fail counterpart.
4. **PR 4, scopes and binders (phase 3).** Result binder (`B` code or callback),
   deferral for `auto` return types. Tests: binder used in a postcondition,
   shadowing a parameter diagnosed, `auto` return type, `this` in a member.
5. **PR 5, template guarantees (phase 4).** Tests that a registered attribute's
   arguments are dependent in the prototype and substituted in the instance,
   including entity-attached ones.
6. **PR 6, spec-only declarations (phase 5), only if the maintainers want it.**
   Flag, descriptor flag, and for fields the layout rule.
7. **PR 7, docs.** `doc/source` entry for the extension API.
8. **Adjacent**, separate issue: open question 10.

EDG test conventions: tests live in `tests/tests/<area>/`, regressions as
`gh-<N>*.sft.cpp` with `//type:rp` and `//options_all:` header lines (see
`tests/tests/regressions/gh-176.sft.cpp`). Expectations are in
`tests/expectations/<config>/<suite>.log`. Since `d05f214f` ("Implement
expectation inheritance") each configuration other than the default
(`edg_x86_64`) holds only its differences from the default's file
(`doc/source/testing.rst` at `034385ba`, section "Expectation Inheritance"; I
diffed it against the pinned copy: the only changes are that section and one
paragraph in the re-recording text). A PR therefore records the default
configuration first (`edgy -W <test>`) and adds explicit lines to another
configuration's file only where it differs. The file was fetched from
`raw.githubusercontent.com/edgcpp/compiler/034385ba.../doc/source/testing.rst`
into the session scratchpad, not into `third_party/edg`.

### O.3 CONTRIBUTING.md checklist (as far as it can be filled now)

- [ ] There is an open issue the PR addresses. (`[GH #?]`: the Discussion first,
      then a maintainer-opened issue.)
- [ ] Commits are tagged `[GH #?]`.
- [ ] Understand the code:
  - [x] High-level design explained: section 6.
  - [x] Alternatives noted: section 7.
  - [ ] No major performance or memory change: registered tables are consulted
        only when a namespace or name is not found in the static tables (to be
        measured with `benchmarks/`; claim NOT yet verified).
  - [ ] Changes localized: `attribute.c`, `attribute.h`, `declarator.c`,
        `il_display.c`, possibly `il_def.h`, `fe_init.c`/`cmd_line.c` (option),
        layout code for phase 5. Not yet written.
- [ ] Code quality:
  - [ ] EDG style.
  - [ ] Comments.
  - [ ] `src/Changes` entry per PR (format: `10/9/26  [GH #27]` heading, a title,
        a paragraph, an example).
- [ ] `edg-docker-test`:
  - [ ] Tests that FAIL before and PASS after.
  - [ ] No regressions; recorded-output diffs reviewed with `edg-test-run-diff`.
  - [ ] Recordings updated with `edg-docker-test -W` after review.

(None of the unchecked items has been done; no EDG code has been written.)

### O.4 Probes

`internal/upstream/attribute-registration-probes/` holds the probes. Re-run any of
them with `scripts/cerastes-fe.sh --c++20 --cerastes_walk <probe>` (or
`--il_display`; `--c++23` for `assume_stmt.cpp`).

| Probe | Used in | Result |
|---|---|---|
| `postfunc_redecl.cpp` | 5.2 | 2 of 4 attributes kept, on types |
| `postfunc_matrix.cpp` | 5.2 | one declaration's attribute kept, on the type: the definition's for non-templates and out-of-class members, the first declaration's for templates |
| `spec_calls.cpp` | 5.3a, 7 | the bump example type-checks, exit 0; the walker shows two `bump` routines (definition is a distinct overload) |
| `spec_calls_fail.cpp` | 5.3a | 2 must-fail twins, each diagnosed at its own line, exit 2 |
| `typedef_forms.cpp` | 5.4 | prefix, after-declarator-id and `using t [[x]] =` attach to the named typedef; the other two to the type |
| `template_param_attrs.cpp` | 5.4 | non-type parameter: warning and drop; type parameter: misparsed as a class-type non-type parameter, 13 errors |
| `expr_args.cpp` | 5.3a, 5.3b | `this`, a lambda and a dependent expression in `enable_if`; prototype `eok_bool_cast`/`tk_template_param`, instance `eok_gt`/`tk_integer` |
| `enable_if.cpp` | earlier draft | same redeclaration behaviour as 5.2 |
| `gnu_postfix.cpp`, `gnu_std_postfix.cpp` | 5.3c | accumulates on the routine, in prototype and instance |
| `assume_stmt.cpp` | 5.3d | `[[assume(expr)]]` on a null statement in a template, both prototype and instance |
| `placements.cpp` | 5.4 | 17 placements recorded; template-parameter attribute not accepted |
| `concept_placements.cpp`, `concept_placements_rejected.cpp` | 5.4, question 9 | only after the concept name is accepted; 6 errors on the other 4 placements |
| `ghost_field_layout.cpp` | 5.4, R7 | `sizeof(S)`: `8 == 4` |
| `prefix_ns.cpp` | question 10 | no warning for `cl`, `gn`, `g` |

They are evidence for this draft, not tests: the harness does not scan `internal/`.
If any should become a regression test, test-mutator writes it under `tests/`
with a reviewed golden (invariant 1).
