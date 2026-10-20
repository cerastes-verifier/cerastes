# 0007 — Attribute registration in EDG: what cerastes relies on

- **Status:** accepted for M2. The EDG side is a local commit series on the
  fork, not yet upstream (decisions 0003, 0005).
- **Milestone:** M2 (2026-10-14)

## Context

M1 showed that EDG's unrecognized-attribute path cannot carry contracts:
- arguments are raw tokens, with no lookup and no types;
- written after a parameter list, the IL keeps at most one declaration's
  attribute, on the function type, and drops the others
  (`internal/upstream/attribute-registration.md` §5.2).

So M2 changes EDG. The proposal in the upstream draft (R1–R8, phases 1–5) is
sized for a VeriFast-class specification language written as C++ inside
`[[cerastes::…]]`, not just for M2's eight attributes (owner, 2026-10-14).

**Owner decisions (2026-10-14):**
- Don't file the upstream Discussion yet. First make the fork's `cerastes`
  branch do what cerastes needs, as a **local commit series**, one commit per
  phase. The owner pushes it and takes it to the EDG maintainers.
- cerastes builds against it through `CERASTES_EDG_SRC` on the topic branch
  `m2-annotations` (decision 0005).

This record says what cerastes relies on, so that a change in upstream review
shows up as a change to this record and to the goldens.

plan.md M2 said to register the attributes "in `attribute.c`'s descriptor
table under the `cerastes` namespace, one commit per attribute". That would
put a tool-specific namespace into EDG, against 0003 item 7, which prefers
general mechanisms. The general mechanism below replaces it. There is no
`cerastes` string anywhere in EDG, and EDG's own tests use its `edg` test
namespace.

## Decision

**The series** (fork branch `cerastes`, based on `034385ba`). Each commit has
EDG tests that fail on its parent and pass on it, a `src/Changes` entry and a
`[GH #?]` tag:

| Commit | What it gives |
|---|---|
| A | `attribute_namespace_is_recognized` compares names by exact length. Today a prefix of a known namespace (`cl`, `gn`, `g`) counts as recognized. |
| B | `register_attribute_namespace()` and new kind `ak_registered`. |
| C | Statement properties in target strings: `s:+n` (null statement), `s:+i` (iteration statement). |
| D | Flags `RAF_ENTITY_ATTACHED` (after a parameter list, the attribute goes on the routine, not the type) and `RAF_MUST_MATCH` (the contracts of a function's declarations must match: item 4). Unflagged attributes accumulate. |
| E | Signature code `R`: an optional leading result name. |
| F | Substitution of attribute arguments on out-of-class definitions of class-template members and member templates (CP2a issue 3); may be merged into D if small. |

**Commit B in detail.**
- `register_attribute_namespace()` can be called by the embedding program
  before `edg_main`. It takes caller-owned static tables: a namespace with
  flags, plus attributes, each with
  - a `name`;
  - a `sig` in EDG's existing argument-signature grammar;
  - a `target` in EDG's existing appertainment grammar;
  - `flags`.
- A feature macro lets cerastes refuse to build against an EDG without the
  API.

**What cerastes relies on:**

1. **A closed namespace.** cerastes registers `cerastes` with `RANF_CLOSED`.
   - An unregistered name in it, for example `[[cerastes::assert(x)]]`, is an
     **error**: EDG's `ec_unrecognized_attribute` raised to error severity.
     It is never silently recorded.
   - Registered attributes produce no "namespace unrecognized" warning
     (#2803).
2. **Misplacement is always an error,** even in Clang-compatible mode, where
   EDG downgrades wrong-entity diagnostics for its own attributes to
   warnings. That covers:
   - the wrong entity;
   - an attribute that appertains to nothing (`[[cerastes::predicate]]
     struct S {};`);
   - the wrong statement kind.
3. **Typed arguments.** `Xb` arguments are unevaluated expressions,
   contextually converted to `bool`; `X` arguments are unevaluated
   expressions. EDG looks them up, resolves overloads and type-checks them.
   Errors are reported at the attribute's own line.
   - Because the arguments are unevaluated, spec function templates named in
     them get declaration instances only.
   - Parameters may be used, which [dcl.fct] allows only in unevaluated
     operands.
4. **Entity attachment and must-match redeclarations (commit D).**
   - An entity-attached attribute written after a function's parameter list
     goes on the routine (for a template, on its routine) with
     `syntactic_location al_post_func`.
   - Descriptors flagged `RAF_MUST_MATCH` (cerastes's `requires` and
     `ensures`) follow the owner's must-match rule (0006 item 6, 2026-10-14).
     The flagged attributes of a declaration form a list. The first
     declaration that has a non-empty list attaches it to the routine.
   - Each later declaration with a non-empty list is compared with it
     element by element, and the two lists must have the same length: the
     same name, and equivalent arguments, with parameters by position. EDG's own equivalence of expressions is used,
     which also handles dependent expressions in templates.
     - Equal: the later copies are not attached, so there are no duplicates.
     - Different: an error at the later declaration.
     - A declaration with an empty list changes nothing.
   - This covers a template declaration plus its definition, and a member
     declared in its class and defined outside it.
   - Attributes without the flag (for example `predicate`, `pure`)
     accumulate in declaration order.
   - A redeclaration written after an instance exists also reaches that
     instance. So does the definition's contract when the instance exists
     only as a declaration (for example one created by `decltype(k(1))`
     before the definition). Under the must-match rule this matters when the
     first declaration that carries a contract comes after the instance was
     created.
   - Only the top-level declarator of the declared function qualifies. Each
     of these keeps the attribute on the type, where the strict target check
     rejects it:
     - a declarator nested inside it, such as `int (*f(int x))(double d)`,
       whose `(double d)` belongs to the returned function type (CP2a issue
       2);
     - a typedef;
     - a lambda;
     - requires-expression parameters;
     - a deduction guide.

     Without this gate, the GNU path that commit D reuses drops the attribute
     silently or attaches it to the wrong parameters (CP2a issues 2 and 4).
   - Prefix attributes (`[[cerastes::requires(…)]] int f(int x);`) are scanned
     before the parameters exist. Detecting them is cerastes's job.
5. **Templates.**
   - In a function template's prototype instantiation, arguments are
     dependent: `tk_template_param` operations, `<tptk_unknown>` types.
   - In each instance they are substituted, through
     `copy_of_attributes_with_substitution`.
   - Statement attributes inside a template body are re-parsed per instance.
   - **Out-of-class definitions of class-template members** and of member
     templates are substituted too. Today EDG passes no parent class to
     `substitute_attribute_expr` (`attribute.c:4376`), so the instance keeps
     the dependent expression (CP2a issue 3). The series fixes that for every
     attribute, in its own commit. If the fix proves infeasible, registered
     attributes there are an error instead, never left unsubstituted.
   - **Parameter packs.** EDG substitutes a reference to a function parameter
     pack in an attribute argument as an empty pack, with no diagnostic: in an
     instance, `((args > 0) && ...)` becomes `true` (CP2a issue 1). In
     registered attribute arguments, a reference to a function parameter pack
     and a pack expansion (of the argument or of the attribute) are therefore
     errors. The general EDG bug is drafted as a separate upstream issue.
   - **Parameters after a pack** (CP3 part 5). An instance keeps the
     prototype's parameter numbers, but its parameter list is expanded. In
     `m(T... xs, int y) [[requires(y >= 0)]]` with `T = int, long`, the
     instance's contract names the `long` element, not `y`. So a reference to
     any function parameter that follows a function parameter pack is an
     error, already in the prototype. cerastes also fails closed on routines
     whose prototype has a pack that is not the last parameter.
6. **The result name (commit E).**
   - `ensures(r, p)` and `ensures(p)` share one descriptor with signature
     `(R,Xb)`. A leading identifier followed by a comma is the result name.
     This is unambiguous because an attribute expression argument cannot
     contain a top-level comma.
   - In the IL, the name is an `aak_token` argument. Uses of it are
     `enk_param_ref` nodes of a nested prototype scope:

     | Use | `param_num` | `levels_up` |
     |---|---|---|
     | the result | 1 | 0 |
     | function parameter k | k | 1 |
     | `this` | 0 | 0 |

     A signature with `R` always pushes the extra scope, so in `ensures` the
     parameters are `{k, 1}` with or without a result name, and `{1, 0}` is
     the result only when one is named. `this` is always `{0, 0}`
     (`overload.c:13497`).

     Levels are relative to nesting. Inside a requires-expression or a lambda
     declarator within the argument, its own parameters take level 0 and the
     function's move up a level. `annotation_il.h` fails closed on a
     parameter reference that sits inside a nested prototype scope (lambda
     declarator, requires-expression, nested function type). cerastes reads
     the levels through `src/backend/annotation_il.h` only.
   - The result's type is the declared return type. Each of these is an error
     in this version (no deferral):
     - a `void` return;
     - a constructor or destructor;
     - a deduced (`auto`, `decltype(auto)`) or trailing return type;
     - a result name outside a top-level function declarator.
   - A result name equal to a parameter's name is an error
     (`ec_decl_hides_function_parameter` at error severity).
7. **Statement placement (commit C).**
   - `s:+i` accepts `while`, `do`, `for` and range-`for`.
   - `s:+n` accepts only the null statement.
   - A loop attribute is scanned before a `for` init-statement, so variables
     declared there are not in scope, and their names are looked up outside
     the loop: an outer variable of the same name is found instead. The owner
     chose to keep invariants prefix-only in M2 and document this
     (2026-10-14). cerastes rejects the shadowing case (0006 item 10).
   - **No silent drops anywhere.** EDG keeps an account of every registered
     attribute scanned in the translation unit. At the end it errors on any
     that was neither applied nor already diagnosed. The CP3 audit found
     several Clang/GNU-mode paths that dropped attributes with only a warning:
     decl-specifier positions, `static_assert`, alias templates, inheriting
     constructors. The account closes that class of bug, not just those
     sites.
8. **What EDG leaves to cerastes:**
   - the meaning of every attribute;
   - that a `requires`/`ensures` has `syntactic_location == al_post_func`
     exactly. It is not enough that it isn't a prefix:
     `int f [[cerastes::requires(x > 0)]] (int x);` binds `x` to an outer
     declaration;
   - that a registered attribute is in namespace `cerastes`. EDG's own test
     attributes (`edg::…`) must not reach the back end. The cerastes
     configuration also pins `INCLUDE_EDG_TEST_ATTRIBUTES=0` (decision 0002),
     because the debug base configuration turns it on;
   - side effects in contracts: unevaluated operands still accept `x++`,
     assignments and calls. M2 rejects the side-effecting operators, and M3's
     encoder rejects calls to anything that is not `pure`, `predicate` or a
     spec function;
   - `fold`/`unfold` arguments being calls to predicates;
   - conjoining the several `requires` (or `ensures`) of the one contract
     that EDG attached;
   - contract consistency across translation units (0006 item 6), which EDG
     cannot see;
   - choosing between prototype and instance (M4). Explicit specializations
     of function templates and of class-template members follow must-match
     with the primary (0006 item 6), which EDG enforces in any declaration
     order, also when the primary's contract is written only on an
     out-of-class definition. Members of explicit or partial class
     specializations carry their own contracts (M4 fails closed on them).
     Explicit specializations of member templates of class templates are an
     error when the primary has a must-match attribute (0006 item 6). The
     member-template prototype of a class instance carries no copy of the
     contract (EDG limitation), so cerastes reads contracts from the root
     template;
   - the language mode. cerastes runs C++ in Clang emulation and C as C23.
     GNU emulation (`--gnu_version`) is refused by the driver, because there
     `extern "C"` redeclarations in different namespaces can become separate
     routines that escape must-match (CP3), unless EDG fixes that. Pre-C23 C
     is refused too: identifier-list declarators could bind a contract's
     names to globals;
   - near-miss namespaces. `[[ceraste::ensures(…)]]` is only an EDG warning
     (#2803), and the back end still runs. The M3 driver makes unrecognized
     attribute namespaces and names errors in verify mode. M2's walker shows
     them.

**The cerastes registration table** (`src/backend/register_attributes.cpp`).
The namespace is `cerastes`, closed.

| Attribute | `sig` | `target` | Flags |
|---|---|---|---|
| `requires` | `(Xb)` | `r` | entity-attached, must-match |
| `ensures` | `(R,Xb)` | `r` | entity-attached, must-match |
| `invariant` | `(Xb)` | `s:+i!` | |
| `check` | `(Xb)` | `s:+n!` | |
| `fold`, `unfold` | `(X)` | `s:+n!` | |
| `predicate`, `pure` | `""` | `r` | entity-attached (they accumulate) |

**Cut from the proposal for this series:**
- per-descriptor policies other than accumulate and must-match;
- deferral for deduced and trailing return types (they are diagnosed
  instead);
- an IL version bump, and a separate IL-representation PR;
- `ATTRIBUTE_REGISTRATION_FN`, `--attribute_namespace=`, and scan or apply
  callbacks;
- `cp_gen_be` printing of registered attributes (it skips them);
- the scope of variables declared in a `for` init-statement;
- registered attributes on class templates;
- spec-only entities (phase 5, R7), which are needed from M6 on.

**Registration is validated strictly.** Each of these is an internal error
in the embedding program:
- an empty target (which would mean "no constraint") or a `0` target;
- a property switch without `!` (which would degrade a misplacement to a
  warning);
- a `c`, `t` or `e` target, until class-template attributes stop being
  de-duplicated by kind (with a single `ak_registered` kind, a second
  attribute would be dropped silently).

**EDG's own tests** register the namespace `edg` as closed, under
`INCLUDE_EDG_TEST_ATTRIBUTES`. No public EDG test uses `edg::` today. If the
maintainers object, a second test namespace is the fallback (a question for
the owner to raise with them).

## Series commits pinned at the M3 close-out (2026-10-20)

The pin moved from `0bcd9bf28a` to P `b9519388ea` (the fork's `cerastes` fast-forwarded; each commit was built,
audited and run on the x86-64 box on a temporary branch first). The five commits it adds, in order, and what each
gives cerastes:

| Commit | SHA | What it gives |
|---|---|---|
| 0020 (EDG bug 0020, merged as `9068279c12`) | `c539de4101` | The template parameters are bound to the template arguments for the rescan of a non-generic lambda that names them in the declaration of a function template. Before it, a template whose declaration had such a lambda could not be called ("identifier is undefined"). The branch `0020` stays for the owner's PR. |
| L | `3b97dcf3eb` | A lambda in the argument of a registered attribute of a function template (or of a member template of a class that is not a template) can name the template parameters, in the types of its parameters and in its body. So `forall` over a `const T *` works in a function template's contract (the former limit of 0006 item 9 is lifted; tests `forall_function_template.cpp`). Also closes a crash: a generic lambda in a substituted attribute of a template whose first parameter is a template template parameter is now the fail-closed error, not a segmentation fault. |
| M | `45954e8d6d` | The same for a member template of a class template: the lambda can name both the class's parameters and the member template's own (test `forall_member_template.cpp`). |
| N | `2c423db33d` | Tests for the explicit specializations of members of templates that have attributes, and NULL checks in the lookup of a template's prototype (robustness for the specializations of G and I; no behavior cerastes depends on changes). |
| P | `b9519388ea` | The flag `RAF_NO_CONSTANT_EVALUATION` (below). |

What stays rejected after L and M (tests in `tests/annotations/negative/`): a lambda that depends on `T` only through
`decltype` of a parameter (the substituted tree is erroneous: `cerastes-annotation_substitution_error`, then
`cerastes-unsupported_annotation_expression`), and a function parameter pack named in an attribute
(`attr_function_parameter_pack`, in a function template's, a member template's and a variadic class template's
contract).

**`RAF_NO_CONSTANT_EVALUATION` is registered for `predicate` and `pure`** (`src/backend/register_attributes.cpp`:
`RAF_ENTITY_ATTACHED | RAF_NO_CONSTANT_EVALUATION`). The flag needs `RAF_ENTITY_ATTACHED`. EDG's interpreter refuses
to call a routine with such an attribute on any declaration (the instances of a template get the template's), so a
constexpr or consteval use of a specification function in a context that needs a constant is the EDG error
`expr_not_constant` (#28), with the note "cannot call function ... in a constant expression: an attribute of the
function does not allow it" (#3615-D). A defaulted function cannot carry the marker
(`attr_no_constant_evaluation_on_defaulted_function`, #3616). Effects for cerastes:
- A specification function whose first declaration carries the marker is not run at compile time: the
  constant-evaluated contexts of the R4a list (array bound, template argument, `static_assert`, enumerator, constexpr variable, case label,
  `if constexpr`, `noexcept` specifier, `explicit(bool)`, bit-field width, `alignas`) are refused by the front end, at
  the line of the context, including the five that leave no trace in the IL (a global template argument and
  `static_assert`, a `static_assert` in a class, a local template argument, an explicit template argument of a
  function). The unevaluated contexts (`decltype`, `sizeof`, the `noexcept` operator, a local array bound that EDG
  treats as a variable-length array, a constexpr or consteval function body that is not called at compile time)
  are not refused by EDG; the cerastes-side R4a pass still reports them (`cerastes-spec_call_in_code`).
  **Known gap (first-declaration marker, M4):** the interpreter reads the attributes the routine has when it is
  called. If the marker is only on a later redeclaration (or on the definition), a use in a constant context that
  comes before it is evaluated by EDG as before and is not refused; the cerastes-side pass sees such a use only if it
  leaves a trace in the IL. Closing it needs the first-declaration marker rule (M4, handoff section 9 Q2).
- EDG calls `back_end()` only when the front end reported no error (cfe.c). In a file where EDG refuses a
  compile-time use, the cerastes-side diagnostics are not produced; EDG's errors replace them (same line, same
  cause). The cerastes-side checks in `annotation_checks.cpp` stay (defense in depth). The back-end constant-context
  pass is still reached: by the contexts EDG does not refuse (tests `constant_context_c1b`, `c8`, `c11`, `c12`,
  `c13`, `c16`, `c17`), and by the dependent forms in templates that are never instantiated, which EDG never
  evaluates. What no test reaches any more are the traces of the contexts EDG now refuses first (array bound at
  namespace scope, bit-field width, `noexcept` specifier, `explicit(bool)`, enumerator, constexpr variable, case label,
  `if constexpr`, `alignas`, and the five no-trace contexts), because the back end does not run after an EDG error.
- A marker on an `= default` special member is now refused by EDG (the cerastes check
  `spec_marker_on_special_member` still covers the non-defaulted special members).

## Consequences

- **Goldens.** Registering `cerastes` removes warning #2803 for it. Every
  `cerastes::` attribute becomes either registered (and printed as a typed
  tree) or an error. That is why the M1 probes that use made-up names were
  moved to the unregistered namespace `unreg_ns` (owner, 2026-10-14). Their
  output is otherwise byte-identical.
- **Upstream review may change the API or the IL** (for example the
  `enk_param_ref` levels, or the tags of the new diagnostics). Only
  `src/backend/register_attributes.cpp` and `src/backend/annotation_il.h`
  know the fork's additions. Any change is a reviewed golden diff and an
  amendment to this record.
- **Testing the series.** The targeted EDG suites (`changes`, `regressions`,
  `edg`) are run under amd64 emulation for each commit. The full `all` run
  on the x86-64 box is the owner's step before going upstream.
- **Merging.** *(Superseded by decision 0009.)* cerastes pins the fork's
  `cerastes` branch, so code that depends on this series lands on `main` once
  the pin includes the series.
