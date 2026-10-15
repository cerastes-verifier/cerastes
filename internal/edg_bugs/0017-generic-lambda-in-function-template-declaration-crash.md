# 0017 — A generic lambda with more `auto` parameters than the function template has template parameters crashes the front end in the template's declaration

- **Labels:** `bug`
- **Found:** 0004 review (2026-10-15), in an `enable_if` condition; reproduced on
  upstream `d6f83174c7` the same day. It is not specific to `enable_if`
- **Status:** open
- **Upstream issue:** not filed

## Summary

A generic lambda (`[](auto v, auto w) { ... }`) written in the declaration of a
function template, in the trailing return type, in a requires-clause or in the
condition of a clang `enable_if` attribute, crashes the front end (segmentation
fault, exit 139, no diagnostic) when the declaration is substituted for a call,
if the lambda has more `auto` parameters than the function template has template
parameters. A lambda with at most as many `auto` parameters as the template has
template parameters does not crash. No pack and no `enable_if` is needed.

The invented template parameters of the lambda's call operator are numbered among
the template parameters of the function template (the same observation is in the
message of the fork commit `88916714e6`, for attributes). When the lambda is
substituted, its parameter types are looked up by those coordinates in the *function
template's* parameter and argument lists. With fewer function template parameters
than the position asked for, the lookup runs off the end of the list and
dereferences a null parameter.

## Reproducer

```c++
// A generic lambda with two auto parameters in the trailing return type of a
// function template with one template parameter.
template <class T> auto f(T x) -> decltype([](auto v, auto w) { return v; }(x, 0)) { return 1; }
int a = f(1);
```

```sh
cpfe --clang_version=210000 --c++20 --no_wrap_diagnostics lam.cpp
```

**Actual:** the process crashes (segmentation fault, exit status 139), no diagnostic.

**Expected:** no diagnostic, as with Apple clang 21
(`clang++ -std=c++20 -fsyntax-only lam.cpp` reports nothing).

The reviewer's case, in an `enable_if` condition (C++20, the lambda in an unevaluated
operand; a pack is not needed):

```c++
template <class T> int a3(T x)
  __attribute__((enable_if([](auto v, auto w) { return v + w; }(x, 0) > 0, "")));
int x = a3<int>(1);
```

crashes the same way (the reviewer's `template <class... Ts> int a3(Ts... xs, int y)`
with a trailing return type on the lambda crashes before and after the 0004 fix).

## Confirmation

- Upstream `d6f83174c7`, native `macos-arm-clang-debug` build. Clang 21 accepts
  the crashing trailing-return and `enable_if` files below (the probes are in
  `.tmp/agent-bugs/analysis/new/lam/`).
- The condition is the number of `auto` parameters against the number of template
  parameters of the function template:

  | Template parameters | `auto` parameters | Result |
  |---|---|---|
  | 1 (`T`) | 1 | accepted |
  | 1 | 2 | crash |
  | 2 (`T`, `U`) | 2 | accepted |
  | 2 | 3 | crash |

  for the trailing-return form (`t1`, `s2`, `t2`, `t3` in the probes) and for the
  `enable_if` form (`n1`, `n4`, `r1`, `r2`). It does not depend on the arguments of
  the lambda call (literals crash too), on a trailing return type on the lambda
  (`[](auto v, auto w) { return v + w; }` crashes), or on a parameter pack.
- A generic lambda in the body of a function template, in a default argument, in a
  `noexcept` specification, or in a non-template function is not affected.
- A requires-clause form (`int f(T x) requires ([](auto v, auto w) { return true; }(x, 0))`)
  crashes as well (clang gives errors of its own for that file, because `x` is not
  a constant there, so it is not a comparison).
- Stack, from a native debugger run (`lldb`) on the trailing-return reproducer:

  ```
  frame #0: get_template_arg_by_list_pos         templates.c:13268   if (tpp->is_pack) {   (tpp is NULL)
  frame #1: get_template_arg_for_coordinates     templates.c:15317
  frame #2: copy_type_with_substitution          templates.c:18048
  frame #3: copy_param_type_list_with_substitution templates.c:17463
  frame #4: copy_type_with_substitution          templates.c:18509
  frame #5: copy_template_param_lambda_expr      il.c:20528
  frame #6: copy_template_param_expr             il.c:21297
  frame #7: make_rescan_operand_full             exprutil.c:4945
  frame #8: make_call_rescan_operands            exprutil.c:5145
  frame #9: scan_function_call                   expr.c:8125
  frame #10: rescan_expr_with_substitution_internal  expr.c:55780
  ...
  ```

  `get_template_arg_by_list_pos` (`templates.c`, line 13265) steps `pos - 1` times
  along the template parameter and argument lists; the second invented parameter
  asks for position 2 of a list with one element.
- With a lambda of at most as many parameters as the template, the lookup finds a
  *template parameter of the function template* for the lambda's parameter. This
  was not examined further; it does not give a wrong result in the probes
  (`[](auto v) { return v; }(0.5) > 0` with `T = int` is accepted, as in clang).

## A second symptom: a generic lambda with a parameter pack of its own

The same substitution (a generic lambda in a function template's declaration) gives
an error, not a crash, when the lambda has a *parameter pack* of its own
(`auto... v`) (found by the review of the 0003 fix, `f11.cpp`, on upstream, the 0004 and the 0003 branches alike):

```c++
template <class T> auto k5(T x) -> decltype([](auto... v) { return sizeof...(v); }(1, 2)) { return 0; }
int a0 = k5(1);
```

**Actual:** `error: function "lambda []()->void" cannot be called with the given argument list`
(twice). **Expected:** no diagnostic; clang 21 accepts it. The closure's call operator
has no parameters in the substituted copy: the lambda's own pack is expanded to no
elements. The same happens in an `enable_if` condition (`k3.cpp`, and the reviewer's
`template <class... Ts> int f11(Ts... xs) enable_if([](auto... v) { return ((v > 0) && ...); }(xs...))`)
and with a function template that has a template pack. A non-template function
(`int k2(int x) enable_if([](auto... v) {...}(1, 2))`) and a generic lambda with a
single plain `auto` parameter are accepted. So it is the substitution of the lambda
for a function template, as in the crash, and I take it for the same cause (the
lambda's invented template parameters are looked up in the function template's
argument list, here a pack of the lambda ending up empty); that was not confirmed in the
code for this symptom. Probes: `.tmp/agent-bugs/analysis/new/f11/` (`f11`, `h2`, `k2`
to `k5`).

## Suggested fix

In `copy_template_param_lambda_expr` (or in the lookup by coordinates), do not
substitute the lambda's own invented template parameters from the function
template's argument list: they belong to the lambda's call operator and are
deduced when the lambda is called. At a minimum the coordinates lookup has to fail
(a substitution error) instead of dereferencing a null parameter.

## Impact on cerastes

None known. cerastes does not substitute generic lambdas in declarations. The fork
series (commit `88916714e6`) already rejects a generic lambda in the argument of a
registered attribute of a function template.
