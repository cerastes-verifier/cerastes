# 0007 — Native macOS builds do not support `ext_vector_type` (or `vector_size`), which libc++ uses

- **Labels:** `bug`, `infrastructure`
- **Found:** M0 (2026-10-13). Narrowed in M1.5 (cause 2 of the libc++ failures in
  cerastes decision 0001); root cause found on upstream on 2026-10-15
- **Status:** open (the one-line fix below was tested locally, not committed)
- **Upstream issue:** not filed

## Summary

The front end implements clang's `ext_vector_type` and GNU's `vector_size`
(`apply_ext_vector_type_attr`, `ak_vector_size`), but only when
`GNU_VECTOR_TYPES_ALLOWED` is set.
- The CMake macro configuration for Linux sets it, in
  `cmake/macro-conf/support/platform/linux/base.cmakedef`.
- The macOS one does not, in
  `cmake/macro-conf/support/platform/macos/base.cmakedef`.

So in a native macOS build (`macos-arm-clang-debug`, and every other
`macos-*` preset), both attributes are "unknown attribute" warnings. The
alias stays a scalar type.

libc++ enables its vectorized algorithms for every clang-based compiler in
C++14 and later (`<__algorithm/simd_utils.h>`, line 28). It declares
`__simd_vector` as an alias template with `__ext_vector_type__` (line 91).
So, with the Command Line Tools' libc++, `<algorithm>`, `<string>`,
`<vector>` and `<iostream>` fail in `<__algorithm/find.h>`.

## Reproducer

```c++
// The vector extension on an alias declaration, plain and as an alias template.
using v4i __attribute__((__ext_vector_type__(4))) = int;
static_assert(sizeof(v4i) == 4 * sizeof(int), "plain alias is a vector");

template <class T, unsigned N>
using vec __attribute__((__ext_vector_type__(N))) = T;
static_assert(sizeof(vec<int, 4>) == 4 * sizeof(int), "alias template is a vector");
```

```sh
cpfe --clang_version=210000 --c++20 --no_wrap_diagnostics ext_vector.cpp
```

**Actual** (native `macos-arm-clang-debug` build):

```
"ext_vector.cpp", line 2: warning: unknown attribute "__ext_vector_type__"
"ext_vector.cpp", line 3: error: static assertion failed with "plain alias is a vector"
"ext_vector.cpp", line 3: note: the final comparison was 4 == 16
"ext_vector.cpp", line 6: warning: unknown attribute "__ext_vector_type__"
"ext_vector.cpp", line 7: error: static assertion failed with "alias template is a vector"
"ext_vector.cpp", line 7: note: the final comparison was 4 == 16
2 errors detected in the compilation of "ext_vector.cpp".
```

`typedef int v4si __attribute__((vector_size(16)));` gives the same "unknown
attribute" warning, and `sizeof(v4si)` is 4.

**Expected:** no diagnostics, as with Apple clang 21
(`clang++ -std=c++20 -fsyntax-only`).

With the SDK (Command Line Tools 27.0, SDK 27.0), `#include <algorithm>`
gives 7 errors in `<__algorithm/find.h>`, with notes in `simd_utils.h`. The
first is:

```
"…/c++/v1/__algorithm/find.h", line 76: error: const variable "__lhs" requires an initializer
        __vec __lhs[__unroll_count];
                                   ^
          detected during:
            instantiation of "_Tp *std::__1::__find_vectorized(_Tp *, _Tp *, _Up) [with _Tp=const char16_t, _Up=char16_t]" at line 140
```

`__vec` is the scalar `const char16_t`, so the comparisons give `bool`, and
`std::__any_of(bool)` and `std::__find_first_set(bool)` find no candidate.

## Confirmation

With `GNU_VECTOR_TYPES_ALLOWED=1` added to
`cmake/macro-conf/support/platform/macos/base.cmakedef` (rebuilt natively,
then reverted):
- the reproducer above and the `vector_size` example compile with no
  diagnostics;
- the 7 `find.h` errors of `#include <algorithm>` are gone.

The remaining 10 errors are those of
[0005](0005-macos-base-predefines-blocks.md) (8, `_stdlib.h`) and
[0006](0006-macos-base-missing-interference-size-macros.md) (2,
`interference_size.h`).

## Suggested fix

Add `GNU_VECTOR_TYPES_ALLOWED=1` to
`cmake/macro-conf/support/platform/macos/base.cmakedef`, as the Linux
platform configuration does. Only the test above was run with it; the
EDG test suites were not.

## Impact on cerastes

Together with 0005 and 0006, this is why cerastes cannot verify programs
that include `<algorithm>`, `<string>`, `<vector>` or `<iostream>` on macOS
(decision 0001).
