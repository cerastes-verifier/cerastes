# 0007 — Native macOS builds do not support `ext_vector_type` (or `vector_size`), which libc++ uses

- **Labels:** `bug`, `infrastructure`
- **Found:** M0 (2026-10-13). Narrowed in M1.5 (cause 2 of the libc++ failures in
  cerastes decision 0001); root cause found on upstream on 2026-10-15
- **Status:** fixed. Branch `0007` in the fork clone is commit `b49d77f810` on upstream
  `main` `d6f83174c7`, plus the test follow-up `3f91f945f9` (squash them for
  the PR); merged into the fork's `cerastes` as `d3cae7714b` and `c1cb2bb47b`
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
`cmake/macro-conf/support/platform/macos/base.cmakedef` (branch `0007`,
rebuilt natively):
- the reproducer above and the `vector_size` example compile with no
  diagnostics;
- with the bases of [0005](upstreamed/0005-macos-base-predefines-blocks.md)
  and [0006](upstreamed/0006-macos-base-missing-interference-size-macros.md), the
  libc++ headers that failed have no errors left in `--c++20`: `<algorithm>`,
  `<string>`, `<vector>`, `<iostream>`, `<map>`, `<unordered_map>`, `<set>`,
  `<functional>`, `<memory>`, `<array>`, `<span>`, `<ranges>`, `<format>`,
  `<numeric>`, `<optional>`, `<sstream>`, `<fstream>`, `<thread>`, `<mutex>`,
  `<chrono>`, `<random>`, `<regex>`, `<filesystem>` and `<string_view>`;
- `std::find`, `std::ranges::find`, `std::mismatch` and `std::equal` on
  `vector<char>`, `u16string`, `string_view` and the other element types that
  libc++ vectorizes are accepted, also in constant evaluation, with and without
  the C-generating back end (`__find_vectorized` and `__mismatch_vectorized`
  are instantiated);
- the Docker configurations are unaffected: the Linux configurations do not
  import the macOS files, and `--dump_configuration` differs only in this
  macro;
- run natively with `edgy` against its recording, in `edg_x86_64` and
  `edg_x86_64_cp`, the new test fails with the front end built before the
  change and passes with the one built after it.

Vector code that native macOS builds used to ignore now reaches code paths
with problems of their own; none is caused by the change (the same inputs
fail the same way in a Linux build hosted by clang):
- [0011](0011-c-gen-be-vector-fill-needs-gcc-target.md): the C-generating back
  end asserts on vector operations when it is built with clang
  (`eok_vector_fill`, `eok_vector_question`, `eok_vector_lor`,
  `eok_vector_not`). Natively, 107 tests per configuration that mention
  vector types now abort there.
- [0012](0012-builtin-shufflevector-error-operand-assertion.md):
  `__builtin_shufflevector` with an error operand (12 cases, already recorded
  in Docker).
- [0013](0013-ext-vector-type-element-access-unsupported.md): `v.x` and the
  other element accesses are unsupported, so the SDK's `<simd/simd.h>`, which
  was silently empty, now fails. `<arm_neon.h>` goes the other way, from
  errors to none.
- Bool vectors and `__builtin_reduce_*`, which libc++'s `__any_of`,
  `__all_of` and `__find_first_set` use, need clang 15 or later emulated
  (`--clang_version`); with a bare `--clang` (clang 9) they are errors. The
  cerastes scripts pass the installed clang's version, so libc++ is fine.
- 16 x86-only GCC tests go from FAIL to CATASTROPHE and 17 tests newly FAIL,
  from running x86 or GCC-mode tests with the arm64 clang base.

## Fix

Branch `0007`, commit `b49d77f810` "Allow GNU and clang vector types in the
macOS macro configurations." and its follow-up `3f91f945f9` "Test bool vectors
and __builtin_reduce_* in the macOS vector-type test.":
- It sets `GNU_VECTOR_TYPES_ALLOWED=1` in the common macOS platform file,
  which the arm64 and x86_64 presets import. `GNU_EXTENSIONS_ALLOWED`, which it
  requires, is already set there; the neon vector types use the same macro.
- The legacy non-CMake macOS settings (`src/defines_macosx.h`) have the same
  gap and are left alone.
- The test is `tests/tests/changes/GH-macos-vector-types.sft.cpp`, run with
  clang 21 emulated (`--clang_v 210000`). It checks what libc++ relies on:
  vector sizes for `ext_vector_type` aliases and alias templates and a
  `vector_size` typedef, element-wise arithmetic and comparison,
  `__builtin_convertvector`, `__builtin_bit_cast`, subscripting, braced
  initialization from a pack expansion, and the bool-vector operations of
  libc++'s `__any_of`, `__all_of` and `__find_first_set`
  (`__builtin_reduce_or`/`_and`, a bit cast to a mask, `__builtin_ctzg`). In
  the Docker configurations it passes with or without the fix.
- The message of `b49d77f810` wrongly says the front end lacks bool vectors
  and `__builtin_reduce_*` (they need clang 15 or later emulated), and its list
  of C-generating back-end sites misses `eok_vector_question`; the follow-up's
  message corrects both.
- No `src/Changes` entry: CONTRIBUTING.md exempts build-system changes.

## Impact on cerastes

Together with 0005 and 0006, this was why cerastes could not verify programs
that include `<algorithm>`, `<string>`, `<vector>` or `<iostream>` on macOS
(decision 0001). The macro changes the IL layout, so cerastes requires it
(`scripts/check_edg_config.sh`, `src/backend/edg_il.h`).
