# 0006 — Annotation language v0: the eight M2 attributes

- **Status:** accepted for M2 (owner decisions 2026-10-14 on loop invariants and `forall`)
- **Milestone:** M2 (2026-10-14)

## Context

plan.md M2 defines the first slice of cerastes's specification language:
- `requires`, `ensures`, `invariant`, `check`, `fold`, `unfold`, `predicate`
  and `pure`;
- the spec functions `owns`, `reads`, `old`, `forall` and `owns_range`.

The owner's scope is VeriFast parity and beyond, with specifications written
as C++ expressions inside standard attributes, never in VeriFast's syntax
(2026-10-14).

M2 gives these annotations no verification meaning yet. M3 and later
milestones do that. This record fixes what M2 does give them:
- their spelling and where they may be written;
- what their arguments are, and which names are in scope;
- what the parameters mean in a postcondition (plan.md asks for this record);
- how redeclarations combine;
- how templates behave;
- which layer diagnoses what.

EDG facts it relies on are in 0007. The header and macros are in 0008.

Probe evidence (M2 Phase 1, il-analyst; probes in the session scratchpad,
summarized here):
- **F1.** `requires` is a C++20 keyword. It is accepted as an attribute name,
  because the standard treats a keyword in an attribute-token as an
  identifier, and EDG's scanner does too, in C++20 and C23.
- **F2.** A lambda body inside an attribute written after the parameter
  list may not name a function parameter, whatever its capture (a lambda's
  declarator may, in `decltype`; CP2a). EDG reports "a
  parameter is not allowed", and the same holds in a `requires` clause. Clang
  rejects `[&]` and `[=]` there ("non-local lambda expression cannot have a
  capture-default").

  What works in both tools is a lambda with no captures that receives every
  free variable as a trailing argument. In block-scope attributes (`invariant`,
  `check`), lambdas may capture as usual.
- **F3.** An error inside a macro argument is reported at the start of the
  macro invocation (its first line and column), not at the argument. See the
  Consequences section.

## Decision

1. **Namespace.** Everything is `[[cerastes::…]]`.
   - The namespace is closed: any name not listed here is an error, including
     `cerastes::assert`. We never use `assert`, because `<cassert>`
     macro-expands it (CLAUDE.md).
   - Users normally write the `CERASTES_*` macros from `<cerastes.h>` (0008).
     Each macro expands to exactly the attribute below.
   - There is deliberately **no `assume`**.
2. **Placement.** Anything else is an error.

   | Attribute | Arguments | Written |
   |---|---|---|
   | `requires(p)` | `p`: unevaluated, contextually `bool` | after a function's parameter list (and after cv/ref/`noexcept` qualifiers) |
   | `ensures(p)`, `ensures(r, p)` | optional result name `r`, then `p` | after a function's parameter list |
   | `invariant(p)` | `p` | before a `while`, `do`, `for` or range-`for` statement |
   | `check(p)` | `p` | on a null statement: `[[cerastes::check(p)]];` |
   | `fold(e)`, `unfold(e)` | `e`: unevaluated expression, a call to a `predicate` function | on a null statement |
   | `predicate`, `pure` | none | on a function declaration, before the declaration (prefix) or after the parameter list |

   - `requires`/`ensures` apply to functions, member functions, constructors
     and destructors. They are not allowed on function pointers, typedefs or
     lambdas.
   - Written as a prefix (`[[cerastes::requires(…)]] int f(int x);`) they are
     an error. EDG scans a prefix attribute before the parameters exist, and
     the conventional place is after the parameter list.
   - **Loop invariants are prefix-only** (owner, 2026-10-14). EDG scans the
     attribute before a `for` init-statement, so variables declared there are
     not in scope: declare the counter before the loop. A placement on the
     loop body may follow later.
   - Worse, such a name silently binds to an outer variable of the same name.
     In `int i = 5; [[cerastes::invariant(i <= n)]] for (int i = 0; …)` the
     invariant is about the outer `i` (CP3 audit). cerastes therefore rejects
     an `invariant` that names a variable with the same name as one declared
     in the loop's init-statement or range-`for` declaration.
3. **Names in scope.**
   - **Contracts:** the parameters, `this` in members (not in a class's
     members declared later: an attribute is not a complete-class context),
     names visible at the declaration, and the result name.
   - **Statement attributes:** everything in scope at that point of the body.
4. **The result name.** In `ensures(r, p)`, `r` names the function's result,
   with the declared return type. It is an error:
   - when it has the same name as a parameter;
   - on a `void` function, a constructor or a destructor;
   - when the return type is deduced (`auto`, `decltype(auto)`) or trailing.
     This is a restriction of this version, not a design limit.

   `ensures(p)` without a result name is always allowed. The first argument
   is a result name exactly when it is a lone identifier followed by a comma.
5. **Parameters in postconditions denote their values on entry.** C++ lets a
   function assign to its by-value parameters, but in `ensures`, `x` means the
   argument value that was passed. This is VeriFast's and Viper's reading.
   - `cerastes::old(e)` is needed only for state that the function can change
     and that the caller can see: the heap through a pointer or reference,
     fields, globals.
   - Writing `old(x)` for a by-value parameter `x` is allowed and means `x`.
   - A reference parameter `int& x` denotes the same object on entry and on
     exit, because a reference cannot be rebound. In `ensures`, `x` reads
     that object's value **on exit**, and `cerastes::old(x)` reads its value
     on entry. This is the same rule as for `*p` with a pointer parameter `p`.
6. **Redeclarations must match** (owner, 2026-10-14; VeriFast's rule). A
   function's contract is the list of `requires` and `ensures` written on
   one of its declarations. Declarations include:
   - a redeclaration and the definition;
   - a member's in-class declaration and its out-of-class definition;
   - a template's declaration and its definition.

   Any number of its declarations may carry contracts, but every declaration
   that carries one must carry the same one. "The same" means the same
   attributes in the same order, with equivalent arguments; parameters are
   compared by position, so renaming them is fine. A declaration without a
   contract has the function's contract. A mismatch is an error at the later
   declaration (EDG, 0007 item 4).
   - The usual style is to write the contract once, on the declaration in
     the header. Repeating it on the definition is allowed.
   - Within one declaration, several `requires` (or `ensures`) are conjoined
     in source order, and so are several `invariant`s on one loop.
   - **Why not accumulate** (the CP1 audit found this). Translation units are
     verified one at a time. If a definition could add `requires(x > 10)` to
     the header's `requires(x > 0)`, callers in other translation units would
     be checked against less than the body assumes. That is unsound.
   - **Across translation units,** EDG enforces the rule only within one
     unit, so consistency between units is an explicit assumption of a
     per-unit result.
     - Example of what it rules out: unit A defines `f` with
       `requires(x > 10)`, unit B declares `f` itself with `requires(x > 0)`
       and calls `f(5)`.
     - The shared header normally gives every unit the same contract.
     - A unit that declares a function without its contract cannot rely on
       one: M3 rejects a call to a function that has no visible contract (and
       no body to verify in the same unit).
     - M3's driver records each function's contract per unit (a contract
       manifest, as VeriFast keeps). A multi-unit verification fails when two
       units disagree. Until that check exists, a result for one unit says
       that it assumes this consistency.
7. **Templates.** Contracts and statement attributes in a template are
   type-checked in the template definition, with dependent arguments, as in
   any template code. They are checked again, with substituted types, in
   each instance; an error found only in an instance is reported with the
   instantiation context.
   - M4 decides when a template is verified once from its prototype.
   - The walker shows both forms (0007 item 5).
8. **`predicate` and `pure` functions** are ordinary C++ functions, marked as
   specification functions. They are never called by the program.
   - A `predicate` returns `bool`, cannot be `pure` as well, and has no
     `requires` or `ensures` (Viper predicates have no contracts).
   - A `pure` function returns a value (not `void`) and may have contracts.
   - Their bodies are C++ and may use the spec functions.
9. **Spec functions** (`namespace cerastes`, 0008) are declared and
   type-checked like any function. M2 gives them no meaning. The intended
   meaning, for later milestones:

   | Function | Meaning |
   |---|---|
   | `owns(place)` | full permission |
   | `reads(place)` | read permission |
   | `old(e)` | the value on entry; only meaningful in `ensures` |
   | `owns_range(p, n)` | permission to `p[0..n)` |
   | `forall(…)` | quantifier |

   - **`forall`** (owner, 2026-10-14): `cerastes::forall(lambda, free...)`. The
     lambda's first parameter is the quantified variable, and its remaining
     parameters receive the trailing arguments, in order:

     ```cpp
     cerastes::forall([](int i, const int* a, int n) { return !(0 <= i && i < n) || a[i] >= 0; }, a, n)
     ```

     The lambda must have no captures when it is used in a contract (F2). In
     `invariant` and `check` it may capture (`[&]`), but the same form works
     there too, and the docs use it everywhere for uniformity.

     M2 does not check that the lambda takes exactly one more parameter than
     there are trailing arguments: the quantified variable's type cannot be
     deduced. M6, which gives `forall` its meaning, must reject a mismatch.
   - `old` is meaningful only in `ensures`. Using it elsewhere is an error
     from cerastes.
10. **Who diagnoses what.**
    - **EDG** (0007): unknown name, wrong entity or statement kind, an
      attribute that appertains to nothing, wrong number of arguments,
      ill-typed or undeclared argument expressions, every result-name error,
      and references to function parameter packs or pack expansions in an
      argument (EDG would otherwise substitute them as empty packs).
    - **cerastes** (the back end, in EDG's diagnostic format, at the C++ line,
      exit status 2):
      - `requires`/`ensures` anywhere but directly after the parameter list
        (`syntactic_location` exactly `al_post_func`): as a prefix, or after
        the declarator-id (`int f [[…]] (int x)`, where `x` would bind to an
        outer declaration);
      - side-effecting operators anywhere in an annotation argument: the
        built-in assignment, compound assignment, `++`/`--`, `new`/`delete`
        and `throw`. This includes the bodies of lambdas written in the
        argument, so that `[&]{ x = 1; return true; }()` in a `check` is
        rejected, and reads or writes of `volatile` objects. GNU statement
        expressions (`({ … })`) are rejected outright in annotation
        arguments.

        Calls are left to M3. That covers explicit calls to functions that
        are not `pure`, `predicate` or spec functions, and implicit ones:
        overloaded operators, conversion functions, constructors and
        destructors. M3's encoder rejects them;
      - a registered attribute in a namespace other than `cerastes`;
      - an `invariant` that names a variable shadowed by the loop's own
        init-statement or range-`for` declaration (item 2);
      - a cerastes attribute on a lambda (`[] [[cerastes::requires(…)]] () {}`
        attaches to the closure's `operator()`) or on a deduction guide, since
        neither can carry a contract in v0;
      - C++20 modules (`import`, module units): contracts on imported entities
        are not carried through module files, so they are rejected until that
        is checked (invariant 3);
      - a `fold`/`unfold` argument that is not a call to a `predicate`
        function;
      - a `predicate` that does not return `bool`, or a `pure` function that
        returns `void`;
      - both `predicate` and `pure` on one function;
      - a contract on a `predicate`;
      - `old` outside `ensures`;
      - a call to a `predicate` or `pure` function from ordinary code (if M2
        has time; otherwise M3).

      Each diagnostic has one owner, either EDG or cerastes, and a must-fail
      test.
    - **Known gap.** Every cerastes-side check of a dependent construct is
      made in each instance: a `fold`/`unfold` callee, and a dependent
      assignment such as `v = 0` on a `T`, which is not an `eok_assign` in the
      prototype. In a template that is never instantiated, these constructs
      are not checked in M2. M4's eligibility checker must fail closed on
      them.
    - **Positions.** A cerastes diagnostic, like EDG's own under
      `FULLY_RESOLVED_MACRO_POSITIONS`, points at the original position of
      the offending text (`orig_seq`/`orig_column`) when that position lies
      in a user file. Otherwise it points at `seq`/`column`, the start of the
      macro invocation. Both layers then report the same line for the same
      problem. The walker prints `seq`/`column`, and says so.

## Consequences

- **Errors inside a macro argument.** In EDG's default configuration they
  point at the first line and column of the macro invocation (F3): a C++
  line (invariant 7), but not the exact one.
  - With `FULLY_RESOLVED_MACRO_POSITIONS=1` they point at the argument's own
    line and column. The probe showed identical output on all 42 existing test
    files, and only an addition (`orig_seq`, `orig_column`) to source
    positions in the IL.
  - cerastes adopts it in `edg_config` (decision 0002, M2 amendment).
  - The walker still prints the invocation position for attributes and their
    trees until the printer reads the original positions, so tests keep each
    `CERASTES_*` invocation on one line.
- **Contracts that quantify are wordier than with `[&]`.** The free-variable
  form is what C++ allows after a parameter list. It is also what Viper's
  quantifiers need: an explicit bound variable.
- **Contracts must match across declarations.** The header's contract is
  the contract; a definition may repeat it but cannot strengthen or weaken it.
