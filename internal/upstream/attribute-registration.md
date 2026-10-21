# DRAFT for the owner to file (sessions never open upstream issues, PRs or Discussions: decision 0003 item 6)

CONTRIBUTING.md says feature requests start as a GitHub Discussion, and a
maintainer opens an issue after consensus. So this is written as a Discussion,
now with a working implementation behind it: a series of 13 commits on the
`cerastes` branch of `mborland/compiler`, on top of upstream `034385ba`,
proposed for upstream (owner's direction, 2026-10-14: implement on the fork
first, then discuss). cerastes pins that branch (decision 0009), so the code has
been in daily use, but nothing here has been pushed or proposed yet.
Sections 1 to 12 and Appendices A and B are the post. The "For the owner" part
at the end (state of upstream, push order, CONTRIBUTING checklist, probes) is
not part of the post.

Evidence conventions. Statements about the series come from the commits
themselves: `git show <sha>`, the `src/Changes` entry, the section of
`doc/source/ext_intf.rst` and the tests of each commit, at the fork's tip
`23f55e3628` (commit SHAs are those of the local branch at that tip; cerastes
pins them, so the commits are no longer amended). Statements about the unmodified
front end are probes run against EDG `9847356d461d2bef846bf133e16821f0e8b69749`
(`7.0-25-g9847356d46`, the cerastes pin before the series), built in the
cerastes configuration; they are kept in Appendix B, and the probe files are in
`internal/upstream/attribute-registration-probes/`. Source-line references to
VeriFast are to `mborland/verifast` branch `generic2`
(`0fb63506a51cae559ae92348eb54ba7f7be0bf6d`), read only.

Revision notes.
(1) The scope is not the first attributes cerastes needs: it is a general
mechanism that can host a complete deductive-verification specification
language for C and C++, at least at VeriFast's level and beyond. (2) The
specification language is C++ itself, written inside standard attributes. That
is the difference from VeriFast, and it is why EDG needs no new expression
grammar. (3) This version replaces the proposal of the earlier draft
(a phased design, sections 5 and 6 there) with the series as built; the
earlier evidence on the unmodified front end is Appendix B.

---

## 1. Title

**A way for an embedding program to register attribute namespaces and
attributes, so that C++ specification languages can live in `[[ns::name(...)]]`
(placement on every entity and statement, typed arguments with scopes and
binders, template-aware IL, redeclaration matching): a working implementation,
proposed for upstream**

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

Items 1 to 4 are implemented, as 13 commits (28 files under `src/`, `doc/` and
`dev_tools/`, +3377 -81 lines; 44 new tests), described one by one in section 5.
Item 5 is not implemented and cerastes does not need it yet. We would like
feedback on the design and on the series, and a decision on how to land it
(section 12).

The series also makes the front end stricter for a registered attribute than
for the attributes it defines itself: a registered attribute is never ignored
silently (an error in every language mode if it is misplaced, applies to
nothing, or cannot be substituted), and the namespace can be closed so that a
misspelled name is an error. For a verifier, a contract that is silently
dropped is an unsoundness, so this is the property we care about most.

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
ordinary compiler the attribute is ignored (and the header's macros expand to
nothing). For cerastes, EDG type-checks the expression like any other, and the
verifier gives meaning to the calls (`owns` is a permission, `&&` is separating
conjunction in that context, `old` is the pre-state).

**Design constraint: no new tokens, operators or expression grammar are asked of
EDG.** If the specification language needs a new construct, it becomes a
function in a header, an attribute, or a lambda, not new syntax.

## 4. What the front end has to provide

The VeriFast annotation language is a good measure of the breadth, because it is
the most complete one for C and C++ that we know. Appendix A inventories it by
category, with the VeriFast source for each feature and one possible C++ form. For EDG,
the breadth reduces to the following requirements, because the arguments are
plain C++. The third column is what the unmodified front end offers (Appendix B
has the probes); the fourth is what the series provides.

| # | Requirement | What unmodified EDG offers (`9847356d`) | In the series |
|---|---|---|---|
| R1 | An external tool registers its own namespace and attribute names, without editing `attribute.c` | Nothing: namespaces are a static list (`attribute.c:970-977`). Unregistered ones can be recorded as raw tokens behind a build macro, with a warning (B.1, B.3) | **B**: `register_attribute_namespace`, kind `ak_registered`, closed namespaces |
| R2 | Placement on every entity and statement kind a specification can attach to (contracts, invariants, ghost statements, spec-only declarations, base class, parameter, lambda) | Mostly. 17 kinds recorded (B.4). Not accepted: attributes on template parameters (misparsed), before or after the word `concept`, or inside a requires-expression | **B** (target check, no silent drop), **C** (null and iteration statements), **D0**, **D** (after the parameter list, on the function). Template parameters, concepts and requirements: not changed. Classes and enumerations: not allowed as targets of registered attributes (section 7) |
| R3 | An attribute survives redeclaration, merged by a per-attribute policy (accumulate or must match) | After the parameter list: the IL keeps one declaration's attribute, on the type, and drops the others (B.2). Prefix: accumulates | **D**: `RAF_ENTITY_ATTACHED` (accumulate) and `RAF_MUST_MATCH`; **F**, **G**, **I** for class-template members and specializations |
| R4 | Arguments are C++ expressions that EDG looks up, overload-resolves and type-checks, with errors at the attribute's own line | Yes, for attributes with an expression signature (`enable_if`, `assume`) (B.3a) | **B**: any signature in the existing notation, including `(Xb)` and `(X)` |
| R5 | Scopes and binders: parameters, `this`, the result, pre-state, quantifier variables | Parameters, `this` and lambdas: yes. Pre-state needs nothing. A binder for the result: no | **E**: signature code `R`; **H**, **I**, **J** for member functions, lambdas and substitution |
| R6 | Dependent arguments in a template prototype, substituted ones in each instance | Yes for expression-signature attributes (B.3b) | **B**, **F** (class parameters), **J** (fails closed) |
| R7 | Spec-only entities: type-checked, in name lookup, flagged so a back end never emits them, without storage | No marker; an attributed field still takes storage (B.4) | **Not implemented.** cerastes marks predicates and pure functions with attributes and handles them itself |
| R8 | Diagnostics for a misplaced attribute or an attribute on the wrong entity | Descriptor target strings do this for built-in attributes, but in clang and GNU modes only as a warning, and some placements are dropped silently | **B**: always an error for a registered attribute; the attribute account |

The first use (cerastes milestone M2) needs R1, R2, R3, R4, R5 (including the
result binder), R6 and R8. R7 comes with later milestones (predicates, pure
functions, memory model).

## 5. The series as built

Thirteen commits on `mborland/compiler` branch `cerastes`, in this order, on
upstream `034385ba0a` (the fork's tip is `23f55e3628`). Each commit has its own
`src/Changes` entry, its documentation in `doc/source` where it changes the
interface, and its own tests under `tests/tests/changes/` with their recordings.
Every subject carries `[GH #?]` (FILT carries `[]`), to be replaced when the
issues exist. Each commit builds on those before it in this list, except A, FILT
and D0, which are independent. The "files" counts are `src/`, `doc/` and
`dev_tools/` only (insertions and deletions).

| Letter | SHA | Subject | Files, lines | New tests |
|---|---|---|---|---|
| A | `822c8f35f8` | Recognize an attribute namespace only on an exact name match | 2, +14 -1 | 1 |
| FILT | `86b5465978` | Normalize the addresses printed for non-file-scope pointers in test output | 1, +3 | 0 |
| B | `ae98d79a22` | Add attributes and attribute namespaces registered by the embedding program | 22, +1215 -24 | 9 |
| C | `0dfcd606d8` | Add statement properties to attribute target constraints | 5, +134 -6 | 4 |
| D0 | `5acfa7f720` | Skip attributes after the parameter list of an abbreviated function template | 2, +19 -1 | 1 |
| D | `5d53cc9a91` | Attach registered attributes after a function's parameter list to the function | 9, +428 -16 | 10 |
| F | `8b4d48ff56` | Substitute the template arguments of a class in the attributes of its members | 5, +152 -31 | 1 |
| E | `0687d72bb2` | Let a registered attribute name the result of a function | 7, +478 -21 | 5 |
| G | `f1d201885f` | Make explicit specializations match the registered attributes of their template | 8, +452 -6 | 2 (+1 changed) |
| H | `fa150e01c7` | Fix the result-name scope of an attribute of a member function defined outside its class | 5, +89 -5 | 2 (+1 changed) |
| I | `207da5dffd` | Reject explicit specializations of member templates with must-match attributes; pack, lambda and duplicate-error fixes | 14, +394 -96 | 5 (+2 changed) |
| J | `88916714e6` | Make the substitution of the arguments of registered attributes fail closed | 10, +172 -61 | 3 (+1 changed) |
| K | `23f55e3628` | Initialize the template symbol of an explicit specialization whose template is not known yet | 7, +43 -1 | 1 |
| P | `b9519388ea` | A registered attribute can forbid the evaluation of a function at compile time (`RAF_NO_CONSTANT_EVALUATION`) | see 5.14 | 2 (29 + accepted cases) |

(L `3b97dcf3eb` and M `45954e8d6d`, lambdas that name template parameters in the attributes of function templates and
of member templates of class templates, and N `2c423db33d`, tests for explicit specializations of members of
templates with attributes, come between K and P on the branch and sit on EDG bug 0020's fix; they are not described
in this draft yet.)

(F comes before E in the branch because E's scope work needed F's
substitution; the letters are our names from the audit, not an order.)

### 5.1 A `822c8f35f8`: exact namespace match

`attribute_namespace_is_recognized` compared only the first `length` characters of
each known namespace, so a namespace that is a prefix of a known one (`cl`, `g`,
`ms`, `_Cl`) was taken as known and got no "attribute namespace is unrecognized"
warning. The comparison now also requires the known name to end there (one
condition in `attribute.c`). This is the adjacent issue 1 of section 11; it is
in the series because a registered namespace must be recognized exactly.
Test: `GH-ns-exact.sft.cpp` (`[[cl::x]]`, `[[gn::y]]`, `[[g::z]]`, `[[ms::w]]`,
`[[_Cl::v]]` warn; `gnu`, `__gnu__`, `clang`, `msvc`, `_Clang` do not, checked
with a negative regular expression).

### 5.2 FILT `86b5465978`: test-output filter

`il_display` prints `**possible non-file-scope ptr** (0x...)` for pointers to
entities outside the file-scope region (the names of the parameters of a
function, for example). The address differs from run to run, so the recorded
output of any test that displays such an IL was never stable. The filter in
`dev_tools/pylibs/edgtest/filter/__init__.py` now replaces the address with
`0xXX`, like the other addresses it normalizes (3 lines). Needed by the tests
of B and later that use `--il_display`. No test of its own.

### 5.3 B `ae98d79a22`: registration

The core commit (22 files: `attribute.c` +815, `attribute.h` +91, `ext_intf.rst`
+123, `il.rst`, `templates.c`, `scope_stk.c`, `decls.c`, `class_decl.c` and
smaller edits).

- **API** (`attribute.h`, tested for with the macro `EDG_ATTRIBUTE_REGISTRATION`):
  `register_attribute_namespace(an_attr_namespace_registration_ptr)`. A
  registration is a namespace name, flags (`RANF_CLOSED`) and a table of
  `an_attr_registration` (name, signature `sig` and target `target` in the
  notations `attribute.c` already uses for the attributes it defines, and
  `RAF_...` flags, none yet). The tables belong to the caller. Registering
  allocates nothing, may happen any time before `edg_main` (and again if
  `edg_main` is called again), and registering the same entry twice has no
  effect.
- **Scanning and IL.** A registered attribute is a standard-syntax attribute
  (`[[ns::name(...)]]` or `[[using ns: name(...)]]`). Its arguments are scanned
  as typed expressions, types or constants as the signature says (instead of raw
  tokens), and it is recorded with the new kind `ak_registered` (`il_def.h`,
  `il_display.c`; documented in `il.rst`). An expression argument is
  dependent in a template declaration and substituted in each instance.
  `__has_cpp_attribute(ns::name)` is 1; the registered namespaces are listed by
  `--dump_configuration`; a registered namespace gets no "unrecognized" warning.
- **Closed namespaces.** With `RANF_CLOSED` an unregistered name in the
  namespace is the error `unknown attribute "ns::name"`.
- **Never dropped silently.** A registered attribute that is on the wrong kind of
  entity, on a class or a template, or on nothing (an empty declaration, in
  front of a linkage specification or `static_assert`, after a type specifier,
  on an alias, and other places) is an error in every dialect, clang and GNU
  modes included, and `-w` does not hide it. The attribute account
  (`attribute_trans_unit_wrapup`, called from `fe_wrapup.c` for C and after the
  instantiations for C++): every registered attribute
  scanned in the translation unit is accounted for once, however many times it
  is scanned (a template and each of its instances, say), as applied or
  diagnosed, and at the end of the translation unit, after the template
  instantiations, one that is neither is the error "attribute does not apply to
  any entity". In C the account is only a backstop: no construct reaches it.
  After an error in one argument, the others are skipped (no cascade).
- **Packs.** An attribute in a declaration cannot refer to a function parameter
  pack, or be a pack expansion, or refer to a parameter that follows a function
  parameter pack: the substitution of the attributes of a declaration does not
  substitute function parameter packs and the later parameters get other numbers
  in an instance. New errors `ec_attr_function_parameter_pack`,
  `ec_attr_pack_expansion`, `ec_attr_parameter_after_pack`. Adjacent issue 2 in
  section 11.
- **Validation.** Registrations are checked when the front end starts; an invalid
  one is an internal error in the embedding program: names that are not
  identifiers or are over 100 characters; the namespaces `gnu`, `__gnu__`,
  `clang`, `_Clang`, `msvc`; a duplicate namespace or attribute, or one the front
  end already defines in that namespace; a malformed signature; an empty target,
  the codes `T`, `0`, `t`, `c` or `e`, the `W` prefix, or a property switch
  without `!`. (The `t`, `c`, `e` codes are excluded because the attributes of a
  class template are kept one per attribute kind, so a second registered
  attribute would be lost.)
- **Test namespace.** Under `INCLUDE_EDG_TEST_ATTRIBUTES` the front end registers
  the closed namespace `edg` (`r_fn`, `r_stmt`, `r_expr` and `requires`), so the
  tests need no external program. Registered attributes of corresponding
  entities in several translation units are merged (kept, not compared), like
  the optional-matching attributes the front end defines.

Tests (9): `GH-reg.sft.cpp` (registered attributes, with `--il_display`),
`GH-reg-err.sft.cpp`, `GH-reg-err-c.sft.c`, `GH-reg-err-compat.sft.cpp` and
`GH-reg-err-compat-c.sft.c` (misplacement in clang and GNU modes),
`GH-reg-err-pack.sft.cpp` (no cascade), `GH-reg-err-after-pack.sft.cpp`,
`GH-reg-account.sft.cpp` (neither applied nor diagnosed) and
`GH-reg-account-ok.sft.cpp` (applied ones are not reported).

### 5.4 C `0dfcd606d8`: statement properties

The target of an attribute can say that a statement must be, or must not be,
a null statement (`s:+n`, `s:-n`) or an iteration statement (`s:+i`, `s:-i`:
`while`, `do`, `for` or range-based `for`). Like the other property switches, a
switch followed by `!` is an error when it is not met (`ec_attr_requires_null_statement`,
`ec_attr_disallows_null_statement`, `ec_attr_requires_iteration_statement`,
`ec_attr_disallows_iteration_statement`). In C99 and later an attribute in front
of an iteration or selection statement used to be attached to the block that the
front end generates around the statement; it is now attached to the statement
itself (`statements.c`, 8 lines), so the constraint sees the statement the
attribute was written on. The names in the arguments of an attribute in front of
a `for` statement are looked up before its init-statement is scanned, so they
never denote its variables (documented).
Tests (4): `GH-stmt.sft.cpp`, `GH-stmt-err.sft.cpp`, `GH-stmt-c.sft.c`,
`GH-stmt-err-c.sft.c`.

### 5.5 D0 `5acfa7f720`: abbreviated function templates

Independent of registration. When the declaration of a function with an `auto`
parameter is first parsed, the rest of the declarator is not scanned (a trailing
return type may refer to the `auto` parameters), and an attribute after the
parameter list was not skipped either: the front end took its `[[` for an array
declarator and gave the error "two consecutive left square brackets always
introduce an attribute list". The attribute is now skipped there and scanned when
the declaration is parsed again as a template (`declarator.c`, 5 lines).
Reproduces without any registration:
`void f(auto x) [[gnu::noinline]];` with `--c++20 --gnu_version=150000`.
Test: `GH-abbr-attr.sft.cpp`.

### 5.6 D `5d53cc9a91`: entity attachment and must-match

A registered attribute with the flag `RAF_ENTITY_ATTACHED` (its target must
include `r`), written after the parameter list of the top-level declarator of a
function declaration, is attached to the function, with syntactic location
`al_post_func`, instead of to the function type. The attributes of every
declaration are therefore kept, in order, and a function template gives them to
its instances, including instances that exist when a later declaration has them.
The parameters of the function are in scope, denoted by position (so a
redeclaration can rename them), and so is `this` in a member function. After any
other function declarator the attribute is an error: pointer to function,
typedef or alias, a returned function type, lambda, requires-expression
parameters, deduction guide, template argument, a block-scope declaration, and the
identifier list of an old-style (K&R) function definition. (Members of a class
defined in a function body are not block-scope declarations and get the
attribute.)

With `RAF_MUST_MATCH` as well, the attributes with the flag on a declaration form
an ordered list. The first declaration that has such a list gives it to the
function; every later list must be equivalent to it, element by element (same
attributes, same order, equivalent arguments, a parameter being its position),
or it is an error that names the earlier declaration
(`ec_attr_redeclaration_mismatch`); an identical list is not attached again, and
a declaration with none is fine. Other flagged attributes accumulate. The commit's
`Changes` entry also says an explicit specialization of a function template must
have the same attributes as the template, whichever is declared first (made
complete in G). Files: `attribute.c` +211, `declarator.c` +41, `templates.c`
+72, `decls.c`, `attribute.h`, `templates.h`, `ext_intf.rst`.
Tests (10): `GH-postfunc.sft.cpp`, `-inst`, `-gnu` (C language linkage declared
in two namespaces, GNU mode), `-c`, `-err`, `-err2`, `-err-c`, `-err-knr`,
`-err-after-pack`, `-account-ok`; it also adjusts `GH-reg-err-c.sft.c`.

### 5.7 F `8b4d48ff56`: class-template members

The arguments of an attribute of a member of a class template (a registered
attribute after the parameter list of a member function, say) were substituted
with the template parameters of the member only, so the parameters of the
enclosing class stayed in the expression of an instance. The substitution now
also does the parameters of the enclosing class (and its enclosing classes),
first, as it already did for the types of the attributes. A registered attribute
of the declaration of a member of a class template that is attached after an
instance of the class was created, or that is written on the definition outside
the class, now reaches the members of those instances, and an attribute the
member already has is not attached twice. This is the adjacent issue 3 of
section 11. Files: `templates.c` +55, `attribute.c` +91 (net), `decls.c`.
Test: `GH-postfunc-cls.sft.cpp` (with `--il_display`).

### 5.8 E `0687d72bb2`: naming the result

A signature that starts with the new code `R` (`"(R,Xb)"`, for a postcondition)
lets the attribute name the result of the function. The name is optional: when
the arguments start with an identifier followed by a comma, the identifier names
the result and is in scope in the arguments that follow, in which it denotes the
result (the type the function returns, a reference if it returns one). Otherwise
there is no name, and what follows is the condition (an identifier that is not
followed by a comma is the start of the condition), so `ensures(r, r > 0)` and
`ensures(x > 0)` use one descriptor. Such an attribute must be entity-attached.
A name for the result is allowed only after the parameter list of a function
declaration that is not in a function body, for a function whose return type is
neither `void` nor deduced; it is an error (`ec_attr_result_name_not_allowed`,
`..._void_function`, `..._deduced_type`, `..._no_result_type`) after any other
declarator, for a trailing return type, a constructor, a destructor or a
conversion function, when the name is that of a parameter, and when the type is
dependent and `void` in an instance. The name is the first argument (an
`aak_token`) and is not compared when must-match attributes are compared.
In the IL the result is a parameter reference (`enk_param_ref`) with
`param_num` 1 and `levels_up` 0, and the parameters of the function are one level
further out (`levels_up` 1), with or without a name; `this` is as without a
result name. The result type is substituted in instances like the parameter
types. Files: `attribute.c` +302, `declarator.c` +102, `attribute.h`, `ext_intf.rst`
+31. Once an error has been issued for a registered attribute that is then
ignored, no other error is issued for it.
Tests (5): `GH-result-name.sft.cpp` (with `--il_display`), `-unnamed`, `-modes`,
`-c`, `-err`.

### 5.9 G `f1d201885f`: explicit specializations

The must-match attributes of a member of a class template are those of any
declaration of the member, including a definition outside the class. They now
reach the member of every instance of the class when the member is created, not
only when its definition is instantiated, so a member that is only declared (an
explicit instantiation declaration) has them, and so does an explicit
specialization of the member that has none of its own. The attributes of an
explicit specialization of a function template or of a member of a class
template (a member function or a member function template) are compared with
the template's when the specialization is declared and again when the template
gets them later, whatever the order of the declarations, also when the template
has them only on an out-of-class definition. A specialization that has such an
attribute when the template has none at the end of the translation unit is an
error (`ec_attr_specialization_without_template_attribute`). Files:
`templates.c` +360, `decls.c`, `ext_intf.rst`.
Tests: `GH-postfunc-spec.sft.cpp`, `GH-postfunc-inherit.sft.cpp` (members of
instances that have the contract only on a definition); `GH-postfunc-cls.sft.cpp`
is extended.

### 5.10 H `fa150e01c7`: result-name scope for out-of-class members

The scope pushed while the arguments of an `R`-signature attribute are scanned
(whether it names the result or not) is a copy of the function prototype scope
of the declarator. Looking up `this` for a member function defined outside its
class, which is not yet known to be nonstatic, took it for the scope of the
declarator and failed an assertion (`variable_this_exists_full`) whenever the
arguments used a member of the class or `this` and the function had no
cv-qualifier. The scope is now marked and skipped by that lookup (`overload.c`,
`scope_stk.h`, `class_decl.c`). Also: a lambda that captures `this` (`[this]`,
`[*this]`) in an attribute after the parameter list of a member function was an
internal error in `make_field_for_lambda_capture`; it is now the error that
`this` cannot be captured. The error that an attribute cannot name the result of a
function that returns `void` is issued once when the declaration of a function
template is scanned again for an instance. Example from the `Changes` entry:
`struct counter { int val; int bump(); };
int counter::bump() [[mine::post(val >= 0)]] { return val; }`.
Tests: `GH-result-name-member-cases.sft.cpp`, `GH-result-name-member-err.sft.cpp`;
`GH-result-name-err.sft.cpp` is extended.

### 5.11 I `207da5dffd`: member-template specializations rejected, and fixes

An explicit specialization of a member template of a class template for one
instance of the class (`template <> template <class U> ... S<int>::g(U)`, and
`template <> template <> ... S<int>::g<long>(long)`) is an error
(`ec_attr_member_template_specialization`) when the member template has a
must-match attribute on any of its declarations, whether the specialization has
the attribute or not and whichever comes first. The attributes of such a
specialization and of the member template could not be compared reliably, and a
copy of the template's could not be inherited without breaking the instantiation
of the specialization (this replaces the approach of G for this one case).
Other fixes in the commit: a reference to a function parameter pack in an
attribute after a parameter list is one error (it was reported again for each
instance, and a reference to the pack was taken for a reference to a parameter
after a pack); a pack or parameter declared in the argument itself (the pack of
a generic lambda) is not one of the declaration; a registered attribute for which
the `void` result error was issued no longer also gets "does not apply" (a friend
defined in a class template); and a lambda in the attribute of a function template
whose parameter has the type of a template parameter could be copied to an instance
(J removes this again). Files: `templates.c` +242, `il.c`, `scope_stk.c`, `decls.c`.
Tests: `GH-postfunc-spec-membertemplate.sft.cpp`, `-pack-ok`, `-pack-once`,
`-lambda-t`, `GH-result-name-void-once.sft.cpp`; `GH-postfunc-spec` and
`GH-postfunc-inherit` are changed.

### 5.12 J `88916714e6`: substitution fails closed

An expression that is an argument of a registered attribute and has an error in
it after the substitution for an instance (a lambda in it is scanned again with
its diagnostics suppressed, and a failure left an error expression without the
error flag) is now the error `bad attribute argument substitution`, whatever the
cause. Before, the instance had an attribute with an error expression and no
diagnostic. Changes made to get there: the way I made a lambda with a
template-parameter-typed parameter work in the attribute of a function template is
removed (the attribute is an error, `T` is undefined, as before it) because it
caused internal errors and crashes for some generic lambdas; a generic lambda in
the expression of an attribute copied for an instance of a function template is an
error instead of a crash (this applies to attribute expressions only; a generic
lambda in any other substituted expression is handled as before, and a test checks
it); an `auto` parameter of a lambda scanned again outside the template parameters
is the error `ec_generic_lambda_not_scanned_again` instead of an assertion; an
expression without rescan information, in the attribute of a member of a class
template for an instance that could not scan it itself, is an error after another
error instead of an assertion (`missing default rescan info`); and the `void`
result error is remembered for every attribute that got it. The tests that need
IL lowering off run with `--no_il_lowering`. Files: `attribute.c`, `il.c`,
`expr.c`, `decl_spec.c`, `templates.c`, `il.h`, `ext_intf.rst`.
Tests: `GH-postfunc-subst-error.sft.cpp`, `GH-generic-lambda-substitution.sft.cpp`,
`GH-result-name-void-two.sft.cpp`; `GH-postfunc-lambda-t.sft.cpp` is rewritten.

### 5.13 K `23f55e3628`: a crash fix for G and I

`prototype_routine_of_template_instance` set its out parameters (the template's
symbol and supplement) only when it found the template. G records an explicit
specialization whose template is not known yet, and stored the uninitialized
symbol. That is an explicit specialization of a member template declared in its
class template (valid since CWG 727, and in Microsoft mode). I then read the
symbol. On x86-64 with AddressSanitizer the front end crashed on
`edg/cmeerw/gnats-23971*`, `edg/daveed/d00244a`, `edg/rma/ms_cpgen01` and
`EDGcpfe9529`, whether the class template was instantiated or not. Both out
parameters are now set to NULL when NULL is returned. The crash was found by the
full `imported,edg,cwg,stdlib` run, which the per-commit `changes,regressions`
runs had not covered. Files: `templates.c`, `Changes`. Test:
`GH-spec-in-class.sft.cpp` (four modes; it crashes the parent commit).

### 5.14 P `b9519388ea`: `RAF_NO_CONSTANT_EVALUATION`

What it does. A registered attribute with the flag `RAF_NO_CONSTANT_EVALUATION` (which also requires
`RAF_ENTITY_ATTACHED`) makes the front end's interpreter refuse to call a function that has the attribute on any of
its declarations (the instances of a template get the template's attributes). A call in a context that needs a
constant is then not a constant expression: error "expression must have a constant value", with the note "cannot call
function ... in a constant expression: an attribute of the function does not allow it" (new message
`ec_constexpr_call_of_function_with_attribute`). Nothing else changes: the function is called at run time, appears in
unevaluated operands (`decltype`, `sizeof`, `noexcept(...)`) as any function does, a constexpr function that calls it
is accepted while it is not evaluated at compile time, and a program that registers no such attribute is not
affected. A function that is defaulted cannot have the attribute (a defaulted special member can be trivial, and the
evaluation of a call of a trivial function calls nothing: the new error
`ec_attr_no_constant_evaluation_on_defaulted_function`). A call of a virtual function is refused if the function that
the call names has the attribute or the override it is dispatched to has it.

Why a tool needs it. An embedding program that gives an attribute the meaning "this function is a specification, not
code" (a verifier's predicate or pure function, a contract-language helper) must make sure that it never runs: not at
run time, which the tool controls, and not in the compiler's constant evaluation, which only the front end can stop.
A `constexpr` specification function used as an array bound, a template argument, a `static_assert`, an enumerator, a
`case` label, an `if constexpr`, a `noexcept` or `explicit` operand, an `alignas` or a bit-field width would
otherwise be evaluated by the front end, and the program would be accepted with a value that the tool's own model of
the function may not share. Several of those contexts leave no trace in the IL (the call is folded into a constant), so
the embedding program cannot find them by walking the IL; the interpreter is the one place that sees all of them.

Files: `attribute.c` (`routine_has_no_constant_evaluation_attribute`, `check_defaulted_function_attributes`), the
interpreter's callee checks, `error_msg.txt`, `ext_intf.rst`, `Changes`. Tests: `GH-no-constant-evaluation` (29
cases that must be errors, each anchored to the note or to the defaulted-function error) and
`GH-no-constant-evaluation-ok` (run-time calls, unevaluated operands, the address of the function, constexpr
functions that are not evaluated, and the same functions without the attribute or with a flagless attribute).

## 6. What an embedding program writes and what it gets

A program registers its namespace before calling `edg_main` (example from the
`Changes` entry of B; this is also what cerastes does, with its eight
attributes):

```c
static const an_attr_registration attrs[] = {
  { "pre", "(Xb)", "r", RAF_NO_FLAGS },
  { NULL, NULL, NULL, RAF_NO_FLAGS }
};
static an_attr_namespace_registration ns = { "mine", RANF_CLOSED, attrs, NULL };
register_attribute_namespace(&ns);   /* before calling edg_main */

[[mine::pre(true)]] void f();   // recorded as a registered attribute
[[mine::prex]] void g();        // error: unknown attribute "mine::prex"
```

The vocabulary, all of it in the notations `attribute.c` already had except the
flags, the code `R` and the statement properties:

| Piece | Values | From |
|---|---|---|
| `sig` | `""` (no arguments), `(Xb)` (an unevaluated expression converted to `bool`), `(X)`, `?(n)`, and the rest of the existing notation; `(R,Xb)`: an optional result name, then the condition | B, E |
| `target` | `r` routine, `v` variable, `s` statement, `r|v` either; `s:+n!` a null statement, `s:+i!` an iteration statement, `s:-n!`, `s:-i!` the others; no `t`, `c`, `e`, `T`, `0`, no `W` prefix, every switch ends in `!` | B, C |
| `flags` | `RAF_ENTITY_ATTACHED` (after a function's parameter list the attribute goes on the function, `al_post_func`), `RAF_MUST_MATCH` (also required: the declarations' lists must be equivalent), `RAF_NO_CONSTANT_EVALUATION` (the front end never evaluates a call of the function at compile time; requires `RAF_ENTITY_ATTACHED`), `RANF_CLOSED` on the namespace | D, P, B |

What the program sees in the IL:

- kind `ak_registered`, with `namespace_name` and `name`; arguments as the
  signature says (`aak_expression` entries with typed expression trees, `aak_token`
  for the result name);
- for an entity-attached attribute, the attribute on the routine
  (`source_corresp.attributes`) with `syntactic_location` `al_post_func`, once for a
  must-match one however many declarations repeat it, on the template, on every
  instance that exists and on instances created later, with the template
  arguments (also those of an enclosing class) substituted;
- the parameters of the function as `enk_param_ref` with their own `param_num` and
  `levels_up` 1 and the named result as `param_num` 1, `levels_up` 0 when the
  signature starts with `R`; `this` as `param_num` 0, `levels_up` 0;
- attributes on statements on the statement itself, also in C99 and later in front of
  `for`, `while` and `if`;
- every misplacement as an error, so a back end that is called at all (EDG calls the
  back end only when there were no errors) can read the attributes it finds as
  complete.

New diagnostics (all in `error_msg.txt`): `attr_function_parameter_pack`,
`attr_parameter_after_pack`, `attr_pack_expansion` (B);
`attr_requires_null_statement`, `attr_disallows_null_statement`,
`attr_requires_iteration_statement`, `attr_disallows_iteration_statement` (C);
`attr_redeclaration_mismatch` (D);
`attr_result_name_not_allowed`, `attr_result_name_void_function`,
`attr_result_name_deduced_type`, `attr_result_name_no_result_type` (E);
`attr_specialization_without_template_attribute` (G);
`attr_member_template_specialization` (I);
`generic_lambda_not_scanned_again` (J). It reuses `decl_hides_function_parameter`
for a result name equal to a parameter's, `unrecognized_attribute` (an error in a
closed namespace) and `bad_attribute_template_substitution`.

What stays out of EDG: no `cerastes` string anywhere in the source tree. EDG's own
tests use the `edg` test namespace. Nothing in the series changes the behaviour of
an attribute the front end already defines, except what A, C (the C99 block) and
D0 fix, which are bugs on their own.

## 7. Known limitations

From the documentation and `Changes` entries of the series and from cerastes's
decisions 0006 and 0007 (our audits). Each is either rejected with a diagnostic
or documented; none is silent unless it says so.

1. **Member templates of a class instance.** For a member template of a class
   template whose contract is written only on an out-of-class definition, the
   member-template prototype in an instance of the class that exists when the
   definition is seen (`S<long>::g`) does not get a copy; the instances of the
   member template get theirs from the definition when they are created. G's
   `ext_intf.rst` text documented this; I's rewrite of the paragraph removed the
   sentence, but the behaviour is what cerastes relies on a walker cross-check for
   (`warning: contract of <root> missing on <routine>`). Verify at the tip before filing.
2. **Explicit specializations of member templates** of class templates
   (`template <> template <class U> ... S<int>::g(U)` and the specialization of one
   instance of it) are an error whenever the member template has a must-match
   attribute on any declaration (I). Three earlier rounds of fixes did not make the
   comparison and the inheritance right. Specializations of function templates and
   of member functions of class templates are supported (G). Members of an explicit
   or partial specialization of a class template are members of another class and
   have attributes of their own.
3. **Lambdas in the contract of a function template** cannot name a template
   parameter (`[](int i, const T* p) ...`, `sizeof(T)` in the body, a `decltype(x)`
   parameter, the enclosing class's `T` in a member template): EDG reports
   `identifier "T" is undefined` when the template is instantiated (J removed the
   attempt in I that made it work, because of crashes), and generic lambdas
   (`auto` parameters, `[]<class V>`) in the attribute of a function template are an
   error. The lambda in the attribute of a member of a class template is not affected.
4. **Function parameter packs.** A reference to a function parameter pack, a pack
   expansion, or a reference to a parameter that follows a pack in an attribute
   outside a function body is an error (B). Inside a function body, such as on a
   statement, they are substituted because the body is scanned again.
5. **Must-match compares expressions with EDG's equivalence**, which gives false
   mismatches for two spellings of the same thing: `this->a > x` against `a > y`,
   and anything containing a lambda (lambdas compare by identity). Such an attribute
   has to be written once (our decision 0006).
6. **GNU emulation.** A function with C language linkage declared in two namespaces
   is one function; in GNU emulation the front end has a routine for each
   declaration and they are not compared (`ext_intf.rst`; test `GH-postfunc-gnu`).
   cerastes refuses GNU emulation because of this.
7. **Targets.** A registered attribute cannot appertain to a class, an enumeration
   or a type (`t`, `c`, `e` are rejected when registering), because the attributes
   of a class template are kept one per attribute kind. So contract-like attributes
   on classes (class invariants as attributes) are not possible with this series; a
   class specification would have to be on a function, a field or a variable.
8. **Only the post-list form is `al_post_func`.** A registered attribute whose target
   includes `r` that is written as a prefix, or after the declarator-id
   (`int f [[...]] (int x)`), is accepted and attached with another syntactic
   location. Its arguments are scanned before the parameters exist, so a prefix
   `requires(x > 0)` that names a parameter fails lookup. A back end that wants only
   the post-list form has to check `syntactic_location` (cerastes does).
9. **The result name** is not allowed for `void`, deduced and trailing return types,
   constructors, destructors and conversion functions; no deferral until the return
   type is known (listed as cut from the series).
10. **Loops.** The names in an attribute in front of a `for` are looked up before the
    init-statement, so they never denote its variables and an outer variable of the
    same name is found instead (documented; a back end can detect the shadowing).
11. **Modules.** Importing a module re-creates only `deprecated` attributes (the
    MSVC IFC reader, `ifc_modules_read.c`), so registered attributes on imported
    entities vanish. Not changed by the series.
12. **Not provided:** spec-only declarations (R7); printing registered attributes
    from `cp_gen_be` (it skips them); a separate IL version bump; per-descriptor
    merge policies beyond accumulate and must-match; a hook macro or
    `--attribute_namespace=` option for registration (the series uses a function the
    embedding program calls). Registered attributes of corresponding entities in
    several translation units are merged, not compared, so contracts written in
    different translation units are not checked against each other.

## 8. Alternatives considered

1. **Hard-code `cerastes` in `attribute.c`.** Smallest EDG change, but a
   tool-specific namespace in the tree, and every other tool would ask for the
   same. Our fallback if the maintainers do not want the general mechanism.
2. **Stay on the unrecognized path and do our own parsing.** We would re-scan
   raw tokens in the back end, with no lookup, no types and no template
   handling, and still lose all but one declaration's contract (B.2). It duplicates the front end.
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

## 9. Open questions for the maintainers

1. Is a general registration mechanism something EDG wants, as built (a function the
   embedding program calls before `edg_main`), or would maintainers prefer a dedicated
   per-tool compile-time descriptor set under a macro, still upstream? (A macro hook
   `ATTRIBUTE_REGISTRATION_FN` and a command-line option were considered and not built.)
2. IL representation: the series adds an attribute kind `ak_registered`, with the
   namespace and name in the existing fields. Is that acceptable, or would a flag plus a
   descriptor index be preferred? What are the consequences for `il_write`/`il_read` and
   the IL version (the series did not bump it, nor touch `il_write`)?
3. The `edg` test namespace under `INCLUDE_EDG_TEST_ATTRIBUTES` (`r_fn`, `r_stmt`,
   `r_expr`, `requires`) is how EDG's own tests exercise the
   interface. Is registering a closed namespace named `edg` acceptable, or should the tests
   use a second namespace? (No public EDG test used `edg::` before.)
4. Spec-only declarations (R7, not built): a flag on the IL entity honored by layout, or
   should tools model ghost state without ghost fields?
5. The result name is a signature code (`R`) for the first argument. Is that the right
   shape, or would a callback be preferred? Is there an existing facility for introducing
   a name into the attribute-argument scope that we missed?
6. Should EDG ship a CMake preset or option for "keep unrecognized attributes and
   prototype instantiations", so reproductions need no hand-set macros? (The series' tests
   need `INCLUDE_EDG_TEST_ATTRIBUTES` and, for the IL ones, `--no_il_lowering`.)
7. Merge policy across redeclarations: the series has two (accumulate, must match), chosen
   by descriptor flag. Is that the right granularity? Must-match uses EDG's own expression
   equivalence, which has false mismatches (limitation 5).
8. Should an unevaluated argument (`X`) instantiate function templates it names? Today it
   does (`cerastes::owns<int>` appears as an instance, B.3a). That is harmless for an
   external back end that filters it, but may not be intended.
9. Attributes on concepts and requirements (our M6 spike): only "after the concept name"
   is accepted (B.4). Do you foresee changing that?
10. Strictness: a registered attribute is an error where a built-in one is a warning or
    silently dropped (B), in every language mode, and `-w` does not hide it. Is that
    acceptable as a documented property of registered attributes only?
11. The substitution of function parameter packs in declaration attributes (adjacent issue
    2): the series rejects them for registered attributes. Would the maintainers rather fix
    the substitution?
12. Member-template explicit specializations (limitation 2) and lambdas that name template
    parameters in function-template attributes (limitation 3) are rejected after several
    rounds of fixes did not make them right. If the maintainers know how EDG intends the
    substitution of a lambda scanned again in an attribute to work, we would like to hear it.

## 10. Testing status

- **Tests.** 43 new `.sft` tests (35 C++, 8 C) under `tests/tests/changes/`, each with its
  recording (`.GH-*.rto`), and 31 new lines in
  `tests/expectations/edg_x86_64_cp/changes.log` (the only expectations file changed).
  Each commit's tests fail on its parent and pass on the commit (checked per commit).
  They are named `GH-<topic>`, to be renamed `gh-<N>-...` when the issues exist (For the
  owner, O.5).
- **Regressions.** Per commit, the `changes` and `regressions` suites, the imported C
  suite and an earlier tip of EDG were run under amd64 emulation. No regression was
  found.
- **Not yet done.** The owner's full `all` run on the x86-64 box with a debug preset.
  `INCLUDE_EDG_TEST_ATTRIBUTES` must be on; some IL tests need `--no_il_lowering`;
  `edg_x86_64_cp` showed load-dependent flaky differences that do not reproduce when the
  tests are run individually. Performance (`benchmarks/`) and memory have not been
  measured.
- **In use.** cerastes builds on the fork and runs its own tests against it: over 700
  harness tests (`tests/annotations`, the documentation examples, IL goldens), in C++20 and
  C23, and checks that Apple clang and GCC accept the same sources without the macro.

## 11. Adjacent issues found

Found while building the series. Draft texts for the owner; each says what is already
fixed by a commit in the series. They are separate from the Discussion. The filing-ready
versions are in `internal/edg_bugs/` (re-checked on upstream on 2026-10-15): issue 1 is
0001 (branch `0001`), issue 2 is 0003 and 0004, issue 6 is 0002 (branch `0002`); issues 3, 4
and 5 did not reproduce on upstream and are not filed (see that README).

### Issue 1: a namespace that only starts like a known one is accepted (fixed by A)

**Title:** `attribute_namespace_is_recognized` accepts a name that is a prefix of a known
namespace

`attribute_namespace_is_recognized` (`attribute.c`) compares `strncmp(*np, name, length)`
with `length` the length of the spelled name, so a name that is only a prefix of a known
namespace is accepted and gets no "attribute namespace is unrecognized" warning.
Reproduction (`--c++17`):

```cpp
[[cl::x]] int a;     // prefix of "clang": no namespace warning
[[gn::y]] int b;     // prefix of "gnu"
[[g::z]] int c;      // "gnu"
[[ms::w]] int d;     // "msvc"
[[_Cl::v]] int e;    // "_Clang"
```

Only the namespace check is bypassed: the front end still warns `unknown attribute`, and
with `RECORD_UNRECOGNIZED_ATTRIBUTES` the attribute is recorded silently. Fix: also
require the known name to end at `length`. Commit `822c8f35f8`, test `GH-ns-exact`.

### Issue 2: a function parameter pack in a declaration attribute is substituted as an empty pack

**Title:** Function parameter pack in an attribute argument on a function declaration is
substituted as an empty pack in template instances

```cpp
template <class... Ts> int f(Ts... args) [[ns::attr(sizeof...(args))]];
template <class... Ts> int g(Ts... args) [[ns::attr(((args > 0) && ...))]];
```

When the attributes of a declaration are copied to an instance, a reference to the
function parameter pack is substituted as an empty pack with no diagnostic: in an
instance `sizeof...(args)` becomes 0 and `((args > 0) && ...)` becomes `true`. (Inside a
function body the same expression is substituted correctly, because the body is scanned
again for each instance.) A reference to a parameter that follows a pack has the same
problem, since it has another number in an instance. The series avoids it for registered
attributes by rejecting these references and pack expansions (B); the general behaviour
for the attributes the front end defines is unchanged. Verified on upstream with clang's
`enable_if` on 2026-10-15 (`internal/edg_bugs/0003`; the parameter-after-pack case is 0004).

### Issue 3: out-of-class definitions of class-template members: attribute arguments are not substituted for the class parameters (fixed by F)

**Title:** Attributes with expression arguments on members of class templates keep the
class's template parameters in an instance

For a member of a class template, the arguments of an attribute (an expression argument
after the parameter list, say) were substituted with the template parameters of the
member only, so the parameters of the enclosing class stayed in the expression of an
instance (`attribute.c` passes no parent class to `substitute_attribute_expr`). An
attribute written on the definition outside the class, or after an instance was created,
did not reach the members of existing instances either. Commit `8b4d48ff56` substitutes
the enclosing class's parameters first and attaches such attributes to the members of
existing instances; test `GH-postfunc-cls`. The draft reproduction in the commit uses a
registered attribute. With the built-in `enable_if`, upstream substitutes the class's
parameters correctly (checked 2026-10-15), so this is not filed as an upstream bug.

### Issue 4: internal error for an expression attribute after the identifier list of an old-style definition

**Title:** Internal error "conv_glvalue_expr_to_prvalue: bad expr" for an attribute with an
expression argument after an identifier list of an old-style definition

```c
int f(x) [[ns::attr(x > 0)]] int x; { return x; }   // --c17 or --c23, clang or GNU mode
```

Correction (2026-10-15): this does **not** reproduce on the unmodified parent. With
`enable_if`, `diagnose_if` or an unknown attribute, upstream reports `identifier "x" is
undefined`, which is right; the internal error was in the series' registered-attribute path. The series makes the registered form an error before it gets
there (D: an entity-attached attribute after the identifier list of an old-style
definition; test `GH-postfunc-err-knr`, `--c17` and `--c23`, plain, clang and GNU modes).
The underlying crash for attributes the front end defines is not fixed.

### Issue 5: several clang and GNU mode paths drop an attribute with only a warning

**Title:** Attributes that appertain to nothing, or to the wrong entity, are dropped
silently or with a warning in clang and GNU modes

In clang and GNU modes an attribute of the front end's on the wrong kind of entity gets
only a warning, and an attribute that appertains to nothing is ignored or dropped: on an
empty declaration, in front of a linkage specification or a `static_assert`, after a type
specifier, on a class or an alias, on an alias template, on an inheriting constructor,
and in other places (the audits of the series found several; none of the list is
exhaustive). B closes this for registered attributes with an account of every attribute
scanned (an error at the end of the translation unit for one that was neither applied
nor diagnosed) and always-error placement; the same class of bug remains for the
attributes the front end defines. This is a general observation, not a single bug, and
may be better as a Discussion than an issue.

### Issue 6: `il_display` prints raw addresses for non-file-scope pointers (the FILT commit)

**Title:** Test output filter does not normalize `**possible non-file-scope ptr** (0x...)`

`il_display` prints the address of a pointer to an entity outside the file-scope region
(the names of the parameters of a function, for example), which differs between runs, so
the recorded output of a test that displays such an IL is never stable. Commit
`86b5465978` makes `edgtest`'s filter replace it with `0xXX`, like the other addresses.

## 12. What we propose

1. Open this Discussion with sections 1 to 12 (and the appendices) as the text.
2. After agreement on the shape (open questions 1, 2, 3 and 10 matter most), open the
   issues and land the series as pull requests in the order of section 5, one commit
   per pull request where they are independent (A, FILT, D0) and B, C, D, F, E, G, H, I,
   J as a chain, or squashed into fewer pull requests if the maintainers prefer. Each
   keeps its own `src/Changes` entry, documentation and tests.
3. Until then, cerastes runs on the fork branch (decision 0009), so nothing blocks on the
   outcome.

## Appendix A: what a VeriFast-class specification language needs

This is a working inventory for the Discussion, not the full language-support
inventory (cerastes milestone M8). It is for judging the breadth of the
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
table, the spec functions in `<cerastes.h>`, M4.5, M6 and M7).

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
| points-to | `e |-> v`, `e |-?-> v` (maybe uninitialized) (`parser.ml:2304`) | `cerastes::owns(e)`, `cerastes::reads(e)` | plan.md M2/M4.5 (`owns`, `reads`) |
| separating conjunction | `A &*& B` (`parser.ml:2293`) | `&&`, given separating meaning by the verifier | owner's example |
| pattern binders | `?x`, `_` (`parser.ml:2257, 2271`) | the value is `e` itself or `old(e)`; existential choice (`?x` matched against the footprint) has no obvious C++ form | **open design point for cerastes, not EDG** |
| fractions | `[f]p` (`parse_asn0`) | an extra spec-function argument | open |
| conditional, switch assertions | `c ? A : B`, `switch (xs) {case nil: ...}` over inductive values (`ast.ml` `WSwitchAsn`) | C++ `?:`; `switch` has no expression form, use a pure function | open |
| quantifiers | `forall_(T x; e)` (`parser.ml:2238`) | `cerastes::forall([&](T x) { return e; })` | plan.md M7 (lambda-bound `forall`; M4.5e designs a C form) |
| `emp` | `emp` | `true` | open |
| **Ghost declarations** | | | |
| predicates with input/output parameters | `predicate nodes(struct node *n; list<...> nodes) = ...`; `;` splits inputs and outputs (`PredTypeExpr`, `ast.ml:417`) | `[[cerastes::predicate]]` function; the input/output split has no obvious C++ form | plan.md M4.5f (predicate); split is open |
| predicate families, constructors | `predicate_family_instance`, `predicate_ctor` (`parser.ml:991-1005`) | overloads or templates plus the attribute | open |
| instance predicates in classes | `InstancePredDecl` (`ast.ml:937`); `tests/cxx/instance_preds/Stack.cpp` | member function marked `[[cerastes::predicate]]` | plan.md M7 |
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
| constructors, destructors | contracts on `Foo()` and `~Foo()` (`tests/cxx/ghost_field.cpp`, `instance_preds/Account.cpp`) | the same attributes on a constructor or destructor, with `this` in scope | plan.md M7 |
| class invariants | an instance predicate plus open/close in each method | predicate member plus fold/unfold | plan.md M7 |
| virtual functions | spec override check, `A_vtype`, `this->valid(&typeid(A))()` (`tests/cxx/virtual_methods/spec_override_check.cpp`) | not designed | plan.md M7: rejected for now |
| references | `tests/cxx/references` | encoded as pointers | plan.md M7 |
| exceptions | none in VeriFast's C++ front end | not designed | plan.md M7: rejected for now |
| templates | generic function verified once; integral type parameters (`src/cxx_frontend/Readme.md`, `tests/cxx/templates`) | the same attributes on a template; dependent arguments | plan.md M5 |
| concurrency, global invariants | `bin/threading.h`: `mutex`, `lock`, `locked`, lock predicates; fractions | none | outside plan.md |
| **Beyond VeriFast (plan.md)** | | | |
| generic template verification | n/a | contracts on a template's prototype, checked once; integral `T` axioms | plan.md M5 |
| contracts on concept requirements | n/a | M6: three candidate syntaxes (attribute on a compound requirement, a requirement-spec declaration, an attribute on the concept); the spike in B.4 shows only "after the concept name" is accepted | plan.md M5 |
| ownership memory model | n/a | `owns`, `reads`, `old`, `owns_range`, lambda-bound `forall` | plan.md M4.5 (C), M7 (C++) |

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
| concept requirement contracts (M6) | on a requirement (syntax undecided) | C++ expressions | the requirement's parameters | no | dependent by construction | n/a |

---

## Appendix B: what the unmodified front end does (evidence)

All of this is the behaviour of EDG `9847356d` (the cerastes pin before the series), built
in the cerastes configuration, and was the motivation for the series. For a registered
attribute much of it no longer applies: the attribute loss in B.2 is fixed by D (and F, G),
the lack of a result binder by E, the silent drops listed in B.4 by B, and the namespace
prefix match in B.1 by A. The probes themselves are kept as evidence and are marked in
O.4. The references `(B.x)` elsewhere in this document point here.

### B.1 Configuration

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
`unknown attribute "cl::x"` warning in adjacent issue 1 of section 11 is a different case). We
have NOT re-run the reproduction below on a stock preset rebuilt with only
`-DRECORD_UNRECOGNIZED_ATTRIBUTES=1`; to reproduce, set the macros above (question 6
of section 9). `scripts/cerastes-fe.sh` runs that front end plus a back end that
prints a listing of the IL (`--cerastes_walk`); it adds `--clang_version=210000`
and the system include paths. `--il_display` is EDG's own dump.

### B.2 Reduced reproduction (12 lines): of several declarations' contracts after the parameter list, the IL keeps one

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

### B.3 What already works, one built-in attribute at a time

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
Probe `gnu_postfix.cpp` (same shape as B.2, `__attribute__((cr_req(...)))`):

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

### B.4 Where an attribute can be placed today, and what it cannot do

`placements.cpp` puts one unrecognized `[[cerastes::a_...]]` on each kind of
entity or statement. The walker records all of these (rc 0):

namespace-scope variable, typedef (see below), enum, enumerator, namespace, class
(after the class-key), derived class, base class, field, method, constructor,
destructor, static data member, parameter, local variable declaration, return
statement, lambda (on its type). Together with the placements in the cerastes IL
inventory (function declaration prefix, loop and null statements, class-template
members), this covers every placement in Appendix A, with the exceptions below.
None of them is a placement a specification language needs, except possibly the
concept ones (question 9 of section 9).

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

---

## For the owner (not part of the post)

### O.1 State of the code and of upstream

- **The code** is 13 commits on the local clone's `cerastes` branch (`~/Documents/compiler`),
  on top of upstream `034385ba0a` ("Fix Python 3.6 compatibility issues"), tip
  `23f55e3628`. cerastes pins that tip (decision 0009). Until you push the branch, the pin
  names commits that exist only in the clone (remote `fork-local`), and `scripts/check.sh`
  prints "NOT YET PUSHED".
- **Upstream** was at `034385ba0a` when this was last checked (2026-10-14, by the compare
  of `9847356d...main`: 8 commits, none touching `src/`). Re-check before filing: the
  series changes `attribute.c`, `templates.c`, `decls.c`, `declarator.c` and others, and
  line numbers in Appendix B are for `9847356d`.
- **CONTRIBUTING.md** (read at the fork's tip) says feature requests start as a
  Discussion and a maintainer opens an issue after consensus, that AI-assisted
  contributions are accepted if you can explain and defend them, and that every commit
  is tagged `[GH #n]`. Every commit of the series has a `Co-Authored-By: Claude Sonnet 5.5`
  trailer; decide whether to keep it.

### O.2 Push order

1. Push the fork's `cerastes` branch (`mborland/compiler`) first.
2. Then push cerastes `main`, whose submodule pin names a commit of that branch.
   Otherwise a clean clone of cerastes cannot fetch its pin (decision 0009 item 3, which
   also says the handoff lists the pushes in this order).
3. Then open the Discussion with sections 1 to 12 and the appendices.
4. After consensus, a maintainer opens the issues, and the series is rebased with the
   issue numbers (O.5) and proposed as pull requests (section 12).

### O.3 CONTRIBUTING.md checklist, as far as it can be filled now

For each commit of the series, unless a line says otherwise:

- [ ] There is an open issue the pull request addresses. (`[GH #?]` in every subject but
      FILT's; the Discussion comes first.)
- [x] Commits are tagged `[GH #?]` (placeholder).
- [ ] You understand your code:
  - [x] The high-level design is explained: sections 5 and 6, with the alternatives in
        section 8. (Open for the owner: be ready to defend the design decisions.)
  - [ ] No major performance or memory change: NOT measured. Registered tables are
        consulted only when a name or namespace is not found in the static tables, and
        the attribute account adds a field per scanned registered attribute; check with
        `benchmarks/`.
  - [ ] Changes are localized: the files are `attribute.c`, `attribute.h`, `declarator.c`,
        `decls.c`, `templates.c`, `templates.h`, `il.c`, `il.h`, `expr.c`, `decl_spec.c`,
        `class_decl.c`, `scope_stk.c`, `scope_stk.h`, `symbol_tbl.c`, `symbol_tbl.h`,
        `statements.c`, `overload.c`, `il_def.h`, `il_display.c`, `cmd_line.c`,
        `cp_gen_be.c`, `fe_init.c`, `fe_wrapup.c`, `error_msg.txt`, plus `doc/` and one
        `dev_tools` file. That is wider than "localized" suggests; B is the large one.
- [ ] Code quality:
  - [ ] EDG style: followed by imitation of the surrounding code; not reviewed by a
        maintainer.
  - [x] Comments are present on the new functions and in `attribute.h`.
  - [x] A `src/Changes` entry exists for every commit except FILT (a `dev_tools` change,
        which does not need one). Format: a `10/14/26  [GH #?]` heading, a title, a
        paragraph, an example.
- [ ] `edg-docker-test`:
  - [x] Every commit has tests that FAIL before and PASS after (checked per commit, under
        amd64 emulation).
  - [ ] No regressions: the `changes` and `regressions` suites, the imported C suite and
        an earlier tip were run; the full `all` run on the x86-64 box with a debug
        preset is NOT done (section 10).
  - [ ] Recordings updated with `edg-docker-test -W` after reviewing the differences: the
        recordings are in the tree (`.GH-*.rto`, `changes.log`); the review of every
        diff with `edg-test-run-diff` was not done by hand.

### O.4 Probes

`internal/upstream/attribute-registration-probes/` holds the probes of Appendix B. They
describe the unmodified front end and are kept as evidence, not as tests (the harness
does not scan `internal/`). Re-run any of them with
`scripts/cerastes-fe.sh --c++20 --cerastes_walk <probe>` (or `--il_display`; `--c++23`
for `assume_stmt.cpp`); a registered attribute in them would behave as in section 5.

| Probe | Used in | Result on the unmodified front end | Status now |
|---|---|---|---|
| `postfunc_redecl.cpp` | B.2 | 2 of 4 attributes kept, on types | superseded by D for a registered attribute with `RAF_ENTITY_ATTACHED` (test `GH-postfunc`) |
| `postfunc_matrix.cpp` | B.2 | one declaration's attribute kept, on the type: the definition's for non-templates and out-of-class members, the first declaration's for templates | superseded by D, F, G |
| `spec_calls.cpp` | B.3a | the bump example type-checks, exit 0; two `bump` routines (the definition is a distinct overload) | still valid for `enable_if`; for a registered attribute the definition is not a distinct overload |
| `spec_calls_fail.cpp` | B.3a | 2 must-fail twins, each diagnosed at its own line, exit 2 | still valid |
| `typedef_forms.cpp` | B.4 | prefix, after-declarator-id and `using t [[x]] =` attach to the named typedef; the other two to the type | unchanged by the series |
| `template_param_attrs.cpp` | B.4 | non-type parameter: warning and drop; type parameter: misparsed, 13 errors | unchanged by the series |
| `expr_args.cpp` | B.3a, B.3b | `this`, a lambda and a dependent expression in `enable_if`; prototype `eok_bool_cast`/`tk_template_param`, instance `eok_gt`/`tk_integer` | still valid; J makes substitution failures errors for registered attributes |
| `enable_if.cpp` | earlier draft | same redeclaration behaviour as B.2 | superseded for registered attributes |
| `gnu_postfix.cpp`, `gnu_std_postfix.cpp` | B.3c | accumulates on the routine, in prototype and instance | the model for D |
| `assume_stmt.cpp` | B.3d | `[[assume(expr)]]` on a null statement in a template, both prototype and instance | the model for C |
| `placements.cpp` | B.4 | 17 placements recorded; template-parameter attribute not accepted | superseded in part: registered attributes on the wrong entity are errors (B) |
| `concept_placements.cpp`, `concept_placements_rejected.cpp` | B.4, question 9 | only after the concept name is accepted; 6 errors on the other 4 placements | unchanged by the series |
| `ghost_field_layout.cpp` | B.4, R7 | `sizeof(S)`: `8 == 4` | unchanged (R7 not implemented) |
| `prefix_ns.cpp` | adjacent issue 1 | no warning for `cl`, `gn`, `g` | fixed by A |

### O.5 Placeholders and renames to do before proposing

- **`[GH #?]`** appears in the subject of 11 commits (not FILT's), in the heading of 11
  `src/Changes` entries, and in the `//remark:` line of each of the 43 tests. Replace each
  with the number of the issue the maintainers open.
- **Test names.** The tests are `tests/tests/changes/GH-<topic>.sft.{cpp,c}` with recordings
  in `.GH-<topic>.rto/` and lines in `tests/expectations/edg_x86_64_cp/changes.log`. EDG's
  convention for a regression is `gh-<N>*.sft.cpp` (see
  `tests/tests/regressions/gh-176.sft.cpp`), so each test, its `.rto` directory and its
  recording lines are renamed when the numbers are known (and re-recorded, since the
  recordings contain the test name).
- **SHAs** in this document change when commits are reworded or reordered by that rebase.
- **Section 12** and open questions 1, 2, 3 and 10 should be re-read after the
  maintainers' first response.

*2026-10-20: cerastes milestone numbers follow the renumbering of that date (plan.md: old M4 → M5, M5 → M6, M6 → M4.5 for C and M7 for C++, M7 → M8).*
