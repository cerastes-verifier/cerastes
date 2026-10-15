# 0003 — A function parameter pack in an attribute argument is substituted as an empty pack

- **Labels:** `bug`
- **Found:** M2 (2026-10-14), audit CP2a issue 1, with attributes registered through the
  fork series; reproduced on upstream with `enable_if` on 2026-10-15
- **Status:** fixed for `enable_if`. Branch `0003` in the fork clone is commit
  `1419c40a6d`, stacked on branch `0004` (`35e10894c9`, on upstream `main`
  `d6f83174c7`): PR 0004 first. Merged into the fork's `cerastes` as `4ff94f065c`.
  The fork series still rejects such references in registered attributes (commit B)
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

## Fix

Branch `0003`, commit `1419c40a6d` "Expand function parameter packs named in
enable_if conditions with the elements of the instance.", stacked on
[0004](0004-attribute-parameter-after-pack-wrong-parameter.md):
- Root cause: `eval_enable_if_attributes` (`src/overload.c`) substitutes the
  condition with a fresh substitution state, so a reference to a function
  parameter pack finds no pack information and is expanded as an empty pack
  (`sizeof...` gives 0, folds give their identity).
- The fix builds the pack information from the instance's parameter list
  (each function parameter pack's first element, or none for an empty pack)
  for that substitution. Pack-element references are renumbered to the
  current element's position, using 0004's convention and guards.
- Tests: `tests/tests/changes/GH-enable-if-pack.sft.cpp` (C++17 to 23),
  `GH-enable-if-pack-cxx11.sft.cpp` (C++11 and 14),
  `GH-enable-if-pack-explicit-object.sft.cpp` (C++23) and
  `GH-enable-if-pack-nested.sft.cpp`. They fail with upstream and with 0004
  alone, and pass with 0003, natively and in Docker. Docker
  `changes,regressions` has no mismatch in either configuration. Reviewed by
  the Opus auditor; its blocking finding was fixed. Mutation-tested and
  checked against clang with the analysis probe matrix.
- Folds over two adjacent function packs also need
  [0016](0016-adjacent-function-parameter-packs-element-count.md) (fixed on its
  own branch).
- Known limitations (owner decision: not pursued):
  - a pack named inside a requires-expression in the condition is still
    empty;
  - in a rare shape, a nested function type's parameter named in an
    expansion of the outer pack takes the outer element's type;
  - a condition that names elements of a reference or forwarding pack
    (`Ts&&... args`) now rejects every call. Upstream accepted them only
    because the pack was empty. Reference parameters are not evaluable
    ([0015](0015-enable-if-reference-parameter-not-evaluated.md)).

## Impact on cerastes

A contract cannot mention a function parameter pack. In the fork series,
registered attribute arguments that name a function parameter pack, or
expand a pack, are errors (commit B). The Annotations page lists them among
the errors the front end reports. Without that, such a contract would
silently mean something else in every instance. The fix is for `enable_if`'s
path; registered attributes go through another one, so lifting those
rejections would be a separate change and a decision for the owner.
