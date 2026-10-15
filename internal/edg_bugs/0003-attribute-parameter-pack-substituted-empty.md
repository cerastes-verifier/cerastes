# 0003 — A function parameter pack in an attribute argument is substituted as an empty pack

- **Labels:** `bug`
- **Found:** M2 (2026-10-14), audit CP2a issue 1, with attributes registered through the
  fork series; reproduced on upstream with `enable_if` on 2026-10-15
- **Status:** open. The fork series rejects such references in registered attributes
  (commit B); the bug itself is not fixed
- **Upstream issue:** not filed

## Summary

An attribute argument on a function template can name the template's function
parameter pack. In an instance of the template, that reference is substituted
as an empty pack, with no diagnostic:
- `sizeof...(args)` becomes 0;
- the fold `((args > 0) && ...)` becomes `true`.

With clang's `enable_if` this changes overload resolution: a function that
should be enabled is rejected, and one that should be rejected is called.

Inside the function body the same expressions are substituted correctly,
because the body is scanned again for each instance.

## Reproducer

```c++
// A function parameter pack named in an enable_if condition.
template <class... Ts>
int two(Ts... args) __attribute__((enable_if(sizeof...(args) == 2, "exactly two arguments")))
{
  return 2;
}

template <class... Ts>
int positive(Ts... args) __attribute__((enable_if(((args > 0) && ...), "all positive")))
{
  return 1;
}

int a = two(1, 2);         // enabled: sizeof...(args) is 2
int b = positive(1, 2);    // enabled: both arguments are positive
int c = positive(-1, 2);   // disabled: -1 is not positive
```

```sh
cpfe --clang_version=210000 --c++17 --no_wrap_diagnostics pack.cpp
```

**Actual:** line 14 is rejected and line 16 is accepted:

```
"pack.cpp", line 14: error: no instance of function template "two" matches the argument list
            argument types are: (int, int)
  int a = two(1, 2);         // enabled: sizeof...(args) is 2
          ^
1 error detected in the compilation of "pack.cpp".
```

**Expected:** line 14 is accepted and line 16 is rejected, as with clang:

```
pack.cpp:16:9: error: no matching function for call to 'positive'
pack.cpp:9:5: note: candidate disabled: all positive
1 error generated.
```

## Confirmation

- Apple clang 21 (`clang++ -std=c++17 -fsyntax-only`) gives the expected
  result above.
- Each half reproduces on its own. In a file with only `two`, EDG rejects
  `two(1, 2)`. In a file with only `positive`, EDG accepts
  `positive(-1, 2)`.
- The same substitution was first seen with attributes registered by an
  embedding program (fork series, decision 0007 of cerastes):
  `[[ns::attr(sizeof...(args))]]` is 0 in every instance.

## Suggested fix

When the attribute arguments of a function template are substituted for an
instance, substitute references to the function parameter pack against the
instance's expanded parameter list, as the body is. That covers
`sizeof...`, folds and pack expansions.

If that is not feasible, a reference to a function parameter pack in an
attribute argument should be an error rather than an empty pack. We have not
located the path that `enable_if` takes; for registered attributes, the
fork series found this behaviour in the substitution of attribute arguments
in `attribute.c`.

## Impact on cerastes

A contract cannot mention a function parameter pack. In the fork series,
registered attribute arguments that name a function parameter pack, or
expand a pack, are errors (commit B). The Annotations page lists them among
the errors the front end reports. Without that, such a contract would
silently mean something else in every instance.
