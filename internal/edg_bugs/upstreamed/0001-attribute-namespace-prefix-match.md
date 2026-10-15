# 0001 — An attribute namespace that is only a prefix of a known one is accepted

- **Labels:** `bug`
- **Found:** M2 (2026-10-14), while registering the `cerastes` attribute namespace
- **Status:** fixed. Branch `0001` in the fork clone is commit `822c8f35f8` on upstream
  `main` `034385ba0a`; fork series commit A
- **Upstream issue:** Filed and a PR submitted

## Summary

`attribute_namespace_is_recognized` (`src/attribute.c`) compares each known
attribute namespace with the spelled name using `strncmp(*np, name, length)`,
where `length` is the length of the spelled name. So a name that is only a
prefix of a known namespace counts as known. `cl` matches `clang`, `g` and
`gn` match `gnu`, `ms` matches `msvc`, and `_Cl` matches `_Clang`.

Such a namespace gets no "attribute namespace is unrecognized" warning, only
"unknown attribute". With `RECORD_UNRECOGNIZED_ATTRIBUTES` the attribute is
recorded silently.

## Reproducer

```c++
[[zz::x]] int z;     // unknown namespace: warned
[[cl::x]] int a;     // prefix of "clang"
[[gn::y]] int b;     // prefix of "gnu"
[[g::z]] int c;      // prefix of "gnu"
[[ms::w]] int d;     // prefix of "msvc"
[[_Cl::v]] int e;    // prefix of "_Clang"
```

```sh
cpfe --clang_version=210000 --c++17 --no_wrap_diagnostics prefix.cpp
```

**Actual:** only `zz` is reported as an unrecognized namespace. The prefixes
get the warning for an unknown attribute in a known namespace:

```
"prefix.cpp", line 1: warning: attribute namespace "zz" is unrecognized
"prefix.cpp", line 2: warning: unknown attribute "cl::x"
"prefix.cpp", line 3: warning: unknown attribute "gn::y"
"prefix.cpp", line 4: warning: unknown attribute "g::z"
"prefix.cpp", line 5: warning: unknown attribute "ms::w"
"prefix.cpp", line 6: warning: unknown attribute "_Cl::v"
```

**Expected:** every line reports `attribute namespace "…" is unrecognized`,
as line 1 does.

## Confirmation

With the fix (branch `0001`, rebuilt natively), all six lines report the
unrecognized namespace. The real namespaces are still recognized:
- `[[gnu::unused]]` and `[[clang::noinline]]` give no diagnostic;
- `[[msvc::no_unique_address]]` gives only "unknown attribute".

## Fix

Branch `0001`, commit `822c8f35f8` "Recognize an attribute namespace only on
an exact name match":
- It also requires the known name to end at `length`:
  `strncmp(*np, name, length) == 0 && (*np)[length] == '\0'`.
- The test is `tests/tests/changes/GH-ns-exact.sft.cpp`. It checks each
  prefix, and checks that no real namespace (`gnu`, `__gnu__`, `clang`,
  `_Clang`, `msvc`) is reported as unrecognized.
- The commit includes a `src/Changes` entry.

## Impact on cerastes

A typo in an annotation, such as `[[ceraste::requires(…)]]` or `[[c::…]]`
once `cerastes` is registered, must not be taken for a known namespace. The
registration mechanism (fork series commit B) recognizes namespaces with
this function, so the series needed this fix first.
