# 0021 — A lambda called in a constant template argument of a function template is not folded in std mode: "template instantiation resulted in unexpected function type"

- **Labels:** `bug`
- **Found:** while fixing 0020 (2026-10-16). Reproduced on upstream `307002eae6` the same day, with a reproducer that
  does not depend on 0020 (its first template parameter is a template template parameter, so the template
  parameters are visible to the rescan even without 0020's fix)
- **Status:** open, not fixed (owner: fix only EDG bugs that block cerastes; cerastes contracts do not call lambdas
  in constant template arguments). Upstream's own suite shows the same message for `lambda-uneval3`, `8`, `11` and
  `13`, which fail in GNU mode too
- **Upstream issue:** not filed

## Summary

In C++ modes without GNU, Clang or Microsoft emulation (`--c++17`, `--c++20`, `--c++23`, and `--gcc`), when a
function template has a lambda that is *called* in a non-type template argument (or an array bound) of its
signature, every instance is rejected: "template instantiation resulted in unexpected function type of … (the
meaning of a name may have changed since the template declaration …)". The type that
`verify_routine_type_matches_template` (`templates.c` line 21814) builds by substitution keeps the call of the
rescanned lambda unfolded (`void (A<(<closure>::operator()(<expression>)))`), while the instance has `A<1>`. In
std mode the lambda copy goes through `scan_field_selection_operator` → `rescan_selector_of_call` →
`make_selection_rescan_operands`; in the GNU, Clang and Microsoft modes it goes through `make_call_rescan_operands`,
and the call folds.

## Reproducer

```c++
template <int N> struct A { };
template <template <class> class TT, class T>
void f(A<[]{ return sizeof(TT<T>); }()>) { }
template <class U> struct W { char c; };
void use() { f<W, int>({}); }
```

Command: `cpfe --c++20 --no_wrap_diagnostics fold5.cpp` (native base, scraped system include directories).

**Actual:**

```
error: template instantiation resulted in unexpected function type of "void (A<1>)" (the meaning of a name may
have changed since the template declaration -- the type of the template is "void (A<<expression>>)")
```

With `f<W, int>(A<1>{})` instead: `no instance of function template "f" matches the argument list`.

**Expected:** no diagnostic, as with Apple clang 21 and GCC 16.

## Confirmation

Upstream `307002eae6`, native macOS arm64 `macos-arm-clang-debug`: `--c++17`, `--c++20`, `--c++23` and
`--gcc --c++20` give the error; `--gnu_version=999999`, `--g++`, `--clang` (with `--clang_version=210000` or
`170000`) and `--microsoft` accept the program. Apple clang 21 and GCC 16 accept it.

## Suggested fix

Make the std-mode rescan of a lambda call in a constant template argument fold the call like the GNU-mode path
(`make_call_rescan_operands`), or have `verify_routine_type_matches_template` compare after folding. Not pursued.

## Impact on cerastes

None today: cerastes contracts do not call lambdas in constant template arguments. The tests of 0020's fix use
`--gnu_version` for the shapes that call a lambda in a template argument.
