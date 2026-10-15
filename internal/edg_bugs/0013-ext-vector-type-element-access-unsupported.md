# 0013 — `ext_vector_type` element access (`v.x`, `v.xy`, `v.lo`, `v.s0`) is unsupported, but `__has_attribute(ext_vector_type)` is 1

- **Labels:** `bug`
- **Found:** 0007 fix review (2026-10-15). Reproduced on upstream `d6f83174c7` the
  same day, in a Linux build hosted by clang
- **Status:** open
- **Upstream issue:** not filed

## Summary

The front end accepts clang's `ext_vector_type` attribute (`attribute.c`, line 305,
`{ "ext_vector_type", "(ci)", "lx", ak_ext_vector_type }`) and builds the vector
type, but it has no support for the element-access syntax that goes with the type:

- single and multiple components, with swizzles and repeats: `v.x`, `v.xy`, `v.wzyx`,
  `v.xxxx`;
- the numeric and halving forms: `v.s0`, `v.s3`, `v.lo`, `v.hi`, `v.even`, `v.odd`;
- the same as an lvalue: `v.xy = w;`.

Member selection on a vector type is an error: "expression must have class type but
it has type ..." in C++ and "... must have struct or union type ..." in C. Meanwhile
`__has_attribute(ext_vector_type)` and `__has_attribute(__ext_vector_type__)` are 1,
as for clang, so headers that test the attribute and then use element access take
the vector path and fail. The macOS SDK's `<simd/simd.h>` is the case that matters:
`simd/base.h` line 25 selects its vector implementation with
`__has_attribute(__ext_vector_type__) && __has_attribute(__overloadable__)`, and the
rest of `simd/` uses element access throughout. `<simd/simd.h>` is included, directly
or not, by the ARKit, ModelIO, MetalPerformanceShaders, Vision, SpriteKit, SceneKit,
MetalKit and AVFoundation headers (the reviewer's list; only `<simd/simd.h>` itself
was run here).

## Reproducer

```c++
typedef float float4 __attribute__((ext_vector_type(4)));
typedef float float2 __attribute__((ext_vector_type(2)));
float f(float4 v) { return v.x + v.w; }
float4 g(float4 v) { return v.wzyx; }
float4 h(float4 v) { float4 r = v; r.xy = v.zw; return r; }
float2 lo(float4 v) { return v.lo; }
float2 hi(float4 v) { return v.hi; }
float2 ev(float4 v) { return v.even; }
float2 od(float4 v) { return v.odd; }
float s0(float4 v) { return v.s0 + v.s3; }
```

```sh
# upstream d6f83174c7, linux-clang-debug build, in the Docker environment
cpfe --edg_base bases/docker/dev-env/clang --clang --c++17 --no_code_gen --no_wrap_diagnostics ext.cpp
```

**Actual:** one error for every access (11 in all), each of the form:

```
"ext.cpp", line 3: error: expression must have class type but it has type "float4" (aka "__attribute((ext_vector_type(4))) float")
```

**Expected:** no diagnostics, as with Apple clang 21
(`clang++ -std=c++17 -fsyntax-only ext.cpp` reports nothing). The test of the
attribute reports 1 in both (`#if __has_attribute(ext_vector_type)`), and does so
in EDG too:

```c++
#if __has_attribute(ext_vector_type) && __has_attribute(__ext_vector_type__)
int ok;
#endif
```

## Confirmation

- The Linux reproducer is the clang-hosted build of upstream `d6f83174c7`
  (`cmake --preset linux-clang-debug`; Linux enables `GNU_VECTOR_TYPES_ALLOWED`,
  `linux/base.cmakedef` line 60). The reproducer is the two typedefs and the lines
  that use them; the probe files are in
  `.tmp/agent-bugs/analysis/new/ext/` (`e1.cpp`, `e2.cpp`, `e3.cpp`, `simd.cpp`).
- The effect on `<simd/simd.h>`, natively on macOS (SDK 27.0, with the 0005 and 0006
  bases applied so that the headers parse otherwise), counted with the default error
  limit raised:

  | | `#include <simd/simd.h>` C++20 | C17 |
  |---|---|---|
  | upstream (`__has_attribute(ext_vector_type)` is 0 without `GNU_VECTOR_TYPES_ALLOWED`) | 0 errors | 0 errors |
  | with the 0007 fix (branch `0007`, `91a07dbc0d`) | 2568 errors | 2673 errors |

  The first of them is `simd/vector_make.h` line 84, and the most frequent message
  is `expression must have class type but it has type "simd_char4" (aka
  "__attribute((ext_vector_type(4))) char")` (27 on `simd_char4` among the first
  hundred, 19 on `simd_char32`, ...). So before the 0007 fix `<simd/simd.h>`
  compiled to nothing, silently; after it, it fails loudly.
- `<arm_neon.h>` goes the other way. It uses `neon_vector_type`, which needs vector
  types: without the 0007 fix it gives 3356 errors in C++20 and 2968 in C17
  (`builtin function is not available because vector types are not supported`, ...);
  with it, 0.
- The libc++ headers do not hit this: `__algorithm/simd_utils.h` declares
  `__simd_vector` with `__attribute__((__ext_vector_type__(_Np)))` but never selects
  components, and it is enabled by `_LIBCPP_COMPILER_CLANG_BASED`, not by
  `__has_attribute`.

## Suggested fix

Implement the element access of clang's `ext_vector_type` (member selection on a
vector type, rvalue and lvalue, with the component names above and the result type
a scalar or a vector of the selected size). That is the real fix and is a feature
of some size: a new case in member selection and the IL representation of the
selected elements.

A stopgap that makes `__has_attribute(ext_vector_type)` report 0 would restore
today's behaviour for `<simd/simd.h>` (it compiles to nothing again), and it is not
sound as a report: the attribute is accepted and the type works, so the answer is
false, and code that tests the attribute in order to *require* it gets an `#error` or
missing types instead of the real diagnostic. libc++ does not test `__has_attribute`
for it (see above), so it would not break libc++, but it would only hide this bug
behind a wrong answer. It is better kept as a macOS-base-only workaround, if at all,
until the element access exists.

## Impact on cerastes

Once the 0007 fix is pinned, a program that includes `<simd/simd.h>` or one of the
Apple frameworks that include it fails to compile, where before it compiled to
nothing. cerastes does not use these headers, and its tests use only standard
headers (decision 0001), which do not reach them. Element access on a vector in a
verified program is rejected by the front end with the error above, before cerastes
sees it.
