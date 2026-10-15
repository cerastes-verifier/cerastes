# 0004 — A parameter after a function parameter pack, named in an attribute argument, is the wrong parameter in an instance

- **Labels:** `bug`
- **Found:** M2 (2026-10-14), audit CP3 part 5, with attributes registered through the
  fork series; reproduced on upstream with `enable_if` on 2026-10-15
- **Status:** fixed for `enable_if`. Branch `0004` in the fork clone is commit
  `35e10894c9` on upstream `main` `d6f83174c7`; merged into the fork's `cerastes` as
  `4c22965b6c`. Branch `0003` is stacked on it. The fork series still rejects such
  references in registered attributes (commit B)
- **Upstream issue:** not filed

## Summary

A function template can have a parameter after its function parameter pack:
`template <class... Ts> int f(Ts... xs, int y)`. An attribute argument that
names `y` keeps the parameter's position from the prototype, where the pack
is one parameter. In an instance the pack is expanded, so that position is
an element of the pack. With `Ts = int, int`, `y` is parameter 2 in the
prototype; in the instance parameter 2 is the second element of `xs`, and
`y` is parameter 3.

With clang's `enable_if`, the condition tests the wrong argument, so overload
resolution enables and disables the wrong calls. There is no diagnostic.

This is related to [0003](0003-attribute-parameter-pack-substituted-empty.md),
the pack itself substituted as empty. It is a different defect: the
parameter numbering, not the pack.

## Reproducer

```c++
// A parameter after a function parameter pack, named in an enable_if condition.
template <class... Ts>
int last_positive(Ts... xs, int y) __attribute__((enable_if(y > 0, "last argument positive")))
{
  return y;
}

int a = last_positive<int, int>(5, -6, 3);   // y is 3: enabled
int b = last_positive<int, int>(5, 6, -3);   // y is -3: must be rejected
```

```sh
cpfe --clang_version=210000 --c++17 --no_wrap_diagnostics after_pack.cpp
```

**Actual:** the call with `y == 3` (line 8) is rejected, because the
condition reads `-6`. The call with `y == -3` (line 9) is accepted, because it
reads `6`.

```
"after_pack.cpp", line 8: error: no instance of function template "last_positive" matches the argument list
            argument types are: (int, int, int)
  int a = last_positive<int, int>(5, -6, 3);   // y is 3: enabled
          ^
1 error detected in the compilation of "after_pack.cpp".
```

**Expected:** line 8 is accepted and line 9 is rejected, as with clang:

```
after_pack.cpp:9:9: error: no matching function for call to 'last_positive'
after_pack.cpp:3:5: note: candidate disabled: last argument positive
1 error generated.
```

## Confirmation

- Apple clang 21 gives the expected result above.
- With a one-element pack (`last_positive<int>(-5, 3)`), EDG agrees with
  clang, because the prototype's numbering and the instance's coincide. That
  is consistent with the numbering explanation.
- The same was first seen with registered attributes in the fork series,
  in `m(T... xs, int y) [[requires(y >= 0)]]` with `T = int, long`: the
  instance's attribute named the `long` element (cerastes decision 0007,
  item 5).
- Root cause (analysis 2026-10-15): `eval_enable_if_attributes`
  (`src/overload.c`) evaluates the condition for a candidate. It puts the
  call's argument constants in an array indexed by position in the
  instance's parameter list, but `enk_param_ref` keeps the template's
  number, in which a pack is one parameter (`il_def.h`: "missing or repeated
  values"). A member of a class template with a function parameter pack,
  whose condition is not substituted, has the same mismatch.
- With the fix (branch `0004`), the reproducer gives clang's result. The
  tests below fail before and pass after, natively with `edgy` and in Docker
  (`edg_x86_64`, `edg_x86_64_cp`). Docker `changes,regressions` matches the
  expectations with no mismatch in either configuration. The 573 tests that
  mention `enable_if` show only a stack-address difference that upstream
  shows too.

## Fix

Branch `0004`, commit `35e10894c9` "Evaluate enable_if conditions with the
arguments of the parameters after a function parameter pack.":
- Positions are the evaluator's convention, so references are renumbered to
  them. A new opt-in in `a_ctws_state`, `instance_param_types`, is set by
  `eval_enable_if_attributes` for its one substitution, and
  `make_operand_for_rescanned_identifier` renumbers each `enk_param_ref` to
  the position of the first instance parameter with its number
  (`renumber_param_ref_for_instance`). For a pack, that is its first
  element; the other elements of a class pack stay indistinguishable
  ([0010](0010-enable-if-class-pack-elements-indistinguishable.md)).
- The renumbering is limited to the function's own parameters, because the
  substitution state is shared with everything substituted for the
  condition. The first version renumbered the parameters of
  requires-expressions too, rejected valid code and poisoned cached concept
  satisfaction (found by review). Two guards prevent it:
  - `requires_expr_satisfied_full` clears the opt-in for the whole
    requires-expression;
  - only references made at the function's own `routine_type_levels` are
    renumbered, so a nested function type's references to its own
    parameters are left alone.
  A mutant without the second guard rejects valid calls with nested function
  types.
- For a condition that is not substituted (a non-template member of a class
  template with a pack), the evaluator maps parameter numbers to positions
  (`interpret_enable_if_by_param_num`), including after an empty class pack.
- It also fixes a segfault on a reference parameter after a pack. Calls that
  name a reference parameter are still rejected, with or without a pack
  ([0015](0015-enable-if-reference-parameter-not-evaluated.md)).
- Behaviour change: a call that omits a defaulted parameter after a pack,
  named in the condition, was accepted by accident (another argument was
  read). It is now rejected, as it already was without a pack
  ([0009](0009-enable-if-default-argument-not-evaluated.md)).
- Tests: `tests/tests/changes/GH-enable-if-after-pack.sft.cpp` (C++11 to 23),
  `GH-enable-if-after-pack-requires.sft.cpp` (C++20 and 23: requires-expressions,
  concepts, lambdas, and a concept-cache canary) and
  `GH-enable-if-after-pack-nested.sft.cpp` (C++11 to 20: nested function
  types). Every rejected call has its own anchored regex, and a final regex
  forbids errors on other lines. `src/Changes` has an entry.
- Reviewed four times by the Opus auditor, mutation-tested, and checked
  against clang with the analysis probe matrix at C++11 to 23.

Registered attributes (the fork series) take another path
(`copy_of_attributes_with_substitution`), which this does not change.

## Impact on cerastes

In the fork series, a reference in a registered attribute argument to any
function parameter that follows a function parameter pack is an error,
already in the prototype (commit B). cerastes also rejects a contract on a
function with a parameter after a pack (`cerastes-parameter_after_pack`).
The fix is for `enable_if`'s path. Lifting those rejections would need the
same renumbering in the substitution of registered attribute arguments, which
is a decision for the owner.
