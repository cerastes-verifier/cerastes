# 0004 — A parameter after a function parameter pack, named in an attribute argument, is the wrong parameter in an instance

- **Labels:** `bug`
- **Found:** M2 (2026-10-14), audit CP3 part 5, with attributes registered through the
  fork series; reproduced on upstream with `enable_if` on 2026-10-15
- **Status:** open. The fork series rejects such references in registered attributes
  (commit B); the bug itself is not fixed
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

## Suggested fix

When the attribute arguments of a function template are substituted for an
instance, map a reference to a parameter that follows a function parameter
pack to that parameter's position in the instance, as the body does. Failing
that, make such a reference an error.

## Impact on cerastes

In the fork series, a reference in a registered attribute argument to any
function parameter that follows a function parameter pack is an error,
already in the prototype (commit B). cerastes also rejects a contract on a
function with a parameter after a pack (`cerastes-parameter_after_pack`).
