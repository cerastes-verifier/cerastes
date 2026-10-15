# 0014 — The size of an `ext_vector_type` is not deduced into a template parameter that is not an `int`

- **Labels:** `bug`
- **Found:** 0007 follow-up (2026-10-15), in the test-mutator's probe `ded.cpp`.
  Reproduced on upstream `d6f83174c7` the same day, in a Linux build hosted by clang
- **Status:** open
- **Upstream issue:** not filed

## Summary

Clang deduces the template arguments of an alias template that carries an
`ext_vector_type` attribute, including the size, whatever integral type the size
parameter has. The front end has the deduction (`matches_template_type`, case
`tk_vector`, `templates.c` line 15178: "Clang does appear to support deduction for
vk_ext vectors"), but it succeeds only when the size parameter is an `int`. With
`unsigned` or `unsigned long` the deduction fails and the call has no viable
function.

The vector size of a type written `__attribute__((ext_vector_type(4)))` is the
`int` constant 4. `matches_template_constant` (`templates.c`) compares it with the
template parameter constant, which has the type of the parameter, and has no
conversion in clang mode (the one that is there is in a `microsoft_mode` block).
So `unsigned N` does not match. (Instrumented in a Linux build:
`size match=0 cp.kind=1 templ_cp.kind=13` for `unsigned`, `size match=1` for `int`; the
argument constant is a `ck_integer`, the parameter's a `ck_template_param`. The type
comparison itself was not read from the debugger: the `int`/`unsigned` split is what
the probes below show.) A size that came from a template argument of the
parameter's type, as in `sz(vec<int, 4>{})`, matches, because that constant already
has the parameter's type.

## Reproducer

```c++
template <class T, unsigned N> using vec __attribute__((__ext_vector_type__(N))) = T;
using v4p __attribute__((__ext_vector_type__(4))) = int;
template <class T, unsigned N> int sz(vec<T, N> v) { return N; }
int a = sz(v4p{});
int b = sz(vec<int, 4>{});
```

```sh
# upstream d6f83174c7, linux-clang-debug build, in the Docker environment
cpfe --edg_base bases/docker/dev-env/clang --clang --c++20 --no_code_gen --no_wrap_diagnostics d1.cpp
```

**Actual:**

```
"d1.cpp", line 4: error: no instance of function template "sz" matches the argument list
            argument types are: (v4p)
  int a = sz(v4p{});
          ^
"d1.cpp", line 3: note: candidate function template "sz" failed deduction
  template <class T, unsigned N> int sz(vec<T, N> v) { return N; }
                                     ^
```

(Line 5 is accepted.)

**Expected:** no diagnostic, as with Apple clang 21
(`clang++ -std=c++20 -fsyntax-only d1.cpp` reports nothing).

## Confirmation

- Only the type of `N` matters. With `template <class T, int N> using vec ... ; template <class T, int N> int sz(vec<T, N> v)` the same call `sz(v4p{})` is accepted; with
  `unsigned long N` it is rejected. Clang accepts all three.
- The failure is not because the types differ: `__is_same(v4p, vec<int, 4>)` holds,
  and `int f(vec<int, 4> v)` accepts a `v4p` argument.
- It is also not about deducing both parameters. With the size fixed
  (`template <class T> int sz(vec<T, 4> v)`) or the element fixed
  (`template <unsigned N> int sn(vec<int, N> v)`), the call still fails for
  `unsigned`, and works in clang.
- The attribute cannot be written in the function parameter itself instead
  (`T __attribute__((ext_vector_type(N))) v`: "the "ext_vector_type" attribute may
  appear only in a typedef"; clang accepts it), so the alias template is the only way
  to write such a parameter, and the alias template is what libc++ does
  (`__algorithm/simd_utils.h`, `__simd_vector`, `size_t _Np`).
- libc++ is not affected: its use deduces the size in a *partial specialization of a
  variable template* (`__simd_vector_size_v<__simd_vector<_Tp, _Np>> = _Np`), not in
  a function template, and `static_assert(std::__simd_vector_size_v<std::__simd_vector<char, 16>> == 16)`
  is accepted by the native build with the 0007 fix.
- The probes are in `.tmp/agent-bugs/analysis/new/ext/` (`d1.cpp`, `d3.cpp`, `d4.cpp`,
  `d5.cpp`, `libcxx_simd.cpp`).

## Suggested fix

In the `tk_vector` case, deduce the size as the array bound is deduced: accept an
integral template parameter of any type and convert the constant to the parameter's
type for the match (the conversion `matches_template_constant` does in Microsoft
mode, `convert_constant_for_deduction`, is the model).

## Impact on cerastes

None known. cerastes does not deduce vector types, and the standard headers it uses do
not reach this.
