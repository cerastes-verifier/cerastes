# 0015 — An `enable_if` condition that names a reference parameter always fails, and crashes after a function parameter pack

- **Labels:** `bug`
- **Found:** Opus review of the 0004 fix (2026-10-15). Reproduced on upstream
  `d6f83174c7` the same day
- **Status:** open. The crash is fixed by the `0004` branch once it is merged; the
  rejection is not
- **Upstream issue:** not filed

## Summary

With clang's `enable_if`, the condition is evaluated with the values of the arguments
of the call. For a parameter of reference type (`const int &y`, `int &&y`) the front end
cannot produce that value, so a condition that names the parameter is never
satisfied and every call is rejected. That is the same for a function and for a
function template, with or without a parameter pack.

In one case it is worse. When a reference parameter follows a function parameter
pack (bug 0004), the evaluator reads the argument of another parameter, an `int`
constant, in the reference parameter's place, and the front end crashes (exit 139,
segmentation fault).

## Reproducer

```c++
// A reference parameter named in an enable_if condition.
int r(int x, const int &y) __attribute__((enable_if(y > 0, "y positive")));
int a = r(5, 3);       // enabled
int b = r(5, -3);      // disabled
int c = r(-5, 3);      // enabled: only y is named
```

```sh
cpfe --clang_version=210000 --c++17 --no_wrap_diagnostics refarg.cpp
```

**Actual:** all three calls are rejected:

```
"refarg.cpp", line 3: error: function "r" cannot be called with the given argument list
            argument types are: (int, int)
  int a = r(5, 3);       // enabled
          ^
"refarg.cpp", line 4: error: function "r" cannot be called with the given argument list
  ...
"refarg.cpp", line 5: error: function "r" cannot be called with the given argument list
  ...
3 errors detected in the compilation of "refarg.cpp".
```

**Expected:** only line 4, as with clang:

```
refarg.cpp:4:9: error: no matching function for call to 'r'
refarg.cpp:2:5: note: candidate disabled: y positive
1 error generated.
```

The same with `template <class T> int r(T x, const int &y)` (`r(5, 3)` rejected) and
with an rvalue reference (`int &&y`). A condition that names only the non-reference
parameter (`enable_if(x > 0)` on the same function) is evaluated correctly.

The crash:

```c++
// A reference parameter after a function parameter pack.
template <class... Ts>
int p8(Ts... xs, const int &y) __attribute__((enable_if(y > 0, "y positive"))) { return y; }
int h = p8<int, int>(5, 6, -3);
```

**Actual:** the process crashes with a segmentation fault (exit status 139), no
diagnostic. **Expected:** the call is rejected (`y` is -3), as with clang:
`refcrash.cpp:4:9: error: no matching function for call to 'p8'`.

## Confirmation

- Apple clang 21 gives the expected results above.
- Mechanism of the rejection, instrumented in `eval_enable_if_attributes`
  (`src/overload.c`, lines 5672-5731): for each argument the constant is taken and
  converted to the parameter type with `prep_argument`. For a parameter of
  reference type the result is not a constant but an expression (a reference bound
  to a temporary holding the argument), and `fold_constexpr_expr` on it does not
  give a value, so the slot of that parameter is set to an error constant
  (`set_error_constant`). The condition then fails when it reads `y`: the evaluator
  (`interpret.c`, `enk_param_ref`, line 32566) copies the slot, an error constant.
  Observed values: for `r(5, 3)`, `after prep_argument: conv_failed=0 is_constant=0
  is_expression=1 ... ref_param=1`, then slot 2 `kind=ck_error`.
- Mechanism of the crash: with the pack, the number 2 of `y` is read against the
  second *argument* (bug 0004); the `enk_param_ref` node has a reference type
  (`is_any_reference_type` is true) and, in the instrumented run, the slot it
  reads holds a `ck_integer` (the second element of the pack, 6). The evaluator
  hands that integer to `extract_value_from_constant` as if it were the value of a
  reference, and the front end crashes. Without the pack, or with the pack of one
  element where the slot is `y`'s own, the slot is the error constant and there is no
  crash (`p8<int>(5, -3)` is rejected normally; with `int y` instead of `const int &y`
  there is no crash). I could not get a stack trace in this environment (the
  debugger does not run here), so the call that faults is inferred from this.
- With the 0004 branch the crash does not happen (`crash.cpp`: exit 0, the call is
  rejected, as the reference parameter is now read from its own slot, an error
  constant); the rejection of `r(5, 3)` remains.
- The probes are in `.tmp/agent-bugs/analysis/new/` (`refarg.cpp`, `refcrash.cpp`)
  and the reviewer's in `.tmp/agent-bugs/review/0004/` (`ref.cpp`, `crash.cpp`).

## Suggested fix

For a parameter of reference type, take the constant of the argument before binding
(the value the condition would read through the reference) when the argument is a
constant, instead of folding the reference-binding expression; for a reference that
cannot be a constant (an lvalue argument that is not a constant), keep the error
constant. In `interpret.c`, the `enk_param_ref` case could also check that the slot
has the node's value kind (an error constant, not a value, for a reference-typed node)
and fail instead of using it.

## Impact on cerastes

None known. cerastes does not use `enable_if`.
