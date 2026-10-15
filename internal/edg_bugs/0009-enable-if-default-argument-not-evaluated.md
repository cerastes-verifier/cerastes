# 0009 — A call that omits a defaulted parameter named in an `enable_if` condition is rejected

- **Labels:** `bug`
- **Found:** analysis of 0003 and 0004 (2026-10-15), while probing default
  arguments after a function parameter pack; reproduced on upstream
  `d6f83174c7` the same day
- **Status:** open
- **Upstream issue:** not filed

## Summary

With clang's `enable_if`, the condition is evaluated with the values of the
arguments of the call. For a parameter that the call does not supply, clang
uses the default argument. The front end evaluates the condition with the
supplied arguments only: a reference to a parameter that has no argument is
not evaluable, the condition fails, and the candidate is discarded, whatever
the default argument is. A call such as `h()` with
`int h(int x = 5) enable_if(x > 0)` is rejected.

## Reproducer

```c++
// A parameter with a default argument, named in an enable_if condition, when the call omits the argument.
int h(int x = 5) __attribute__((enable_if(x > 0, "x positive")));
int a = h();       // enabled: the default argument 5 is positive
int b = h(1);      // enabled
int c = h(-1);     // disabled
```

```sh
cpfe --clang_version=210000 --c++17 --no_wrap_diagnostics defarg.cpp
```

**Actual:** line 3 is rejected, as well as line 5:

```
"defarg.cpp", line 3: error: function "h" cannot be called with the given argument list
  int a = h();       // enabled: the default argument 5 is positive
          ^

"defarg.cpp", line 5: error: function "h" cannot be called with the given argument list
            argument types are: (int)
  int c = h(-1);     // disabled
          ^

2 errors detected in the compilation of "defarg.cpp".
```

**Expected:** only line 5 is rejected, as with clang:

```
defarg.cpp:5:9: error: no matching function for call to 'h'
defarg.cpp:2:5: note: candidate disabled: x positive
1 error generated.
```

## Confirmation

- Apple clang 21 (`clang++ -std=c++17 -fsyntax-only`) gives the expected result.
- Function templates behave the same: `template <class T> int ht(T x = 5)
  enable_if(x > 0)` rejects `ht<int>()`, and `template <class T> int hu(T t, int y = 5)
  enable_if(y > 0)` rejects `hu(1)`.
- A parameter that is not named is unaffected: `int h2(int x, int y = 1)
  enable_if(x > 0)` accepts `h2(1)` and rejects `h2(-1)`, as clang does.
- When the default argument makes the condition false (`int h3(int x = -5)
  enable_if(x > 0)`, `h3()`), the front end and clang agree, because the front
  end rejects every such call.
- Cause: `eval_enable_if_attributes` (`src/overload.c`, lines 5672-5731) pairs the
  arguments of the call with the parameters and fills `args[k]` for the
  supplied ones only; the array has one entry per argument plus one. The
  evaluator reads a parameter by number (`interpret.c`, `case enk_param_ref`,
  line 32566) and, for a number beyond the array, reports "cannot be
  interpreted" (`interpret.c`, about line 32590), so the function is discarded.
- It was noticed with a parameter after a function parameter pack
  (`int f(Ts... xs, int y = 5) enable_if(y > 0)`, `f<int, int>(1, 2)`), where
  upstream accepts the call only because the parameter number is read from the
  wrong slot (bug 0004); with the numbers fixed the call is rejected for this
  reason.

## Suggested fix

For a parameter with no corresponding argument and a default argument, put the
constant value of the default argument (converted to the parameter type, if it
is a constant expression) in the array, as is done for supplied arguments;
otherwise, an error constant, as is done now. The default argument expression
of the routine may not exist yet for a template (`has_default_arg` without
`default_arg_expr`, see the note in `overload.c` near line 5539), which has to be
handled.

## Impact on cerastes

None known. cerastes does not use `enable_if`.
