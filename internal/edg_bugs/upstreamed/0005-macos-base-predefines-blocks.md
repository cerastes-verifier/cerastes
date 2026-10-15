# 0005 — The macOS clang base predefines `__BLOCKS__`, but the front end cannot parse block syntax

- **Labels:** `bug`, `infrastructure`
- **Found:** M0 (2026-10-13). Narrowed in M1.5 (cause 1 of the libc++ failures in
  cerastes decision 0001); reproduced on upstream on 2026-10-15
- **Status:** fixed. Branch `0005` is commit `7173149c6b` on upstream `main`
  `d6f83174c7`, pushed for the PR. The fork's `cerastes` merged the version
  before filing, `cca18c33c5` (the same fix; its test was then renamed
  `GH307`), as `8e1eb6c0a6`
- **Upstream issue:** filed as edgcpp/compiler#307, and a PR opened

## Summary

The native macOS bases predefine `__BLOCKS__` for clang mode:
- `bases/cmake-native/macos-arm/clang/lib/predefined_macros.txt` line 114;
- `bases/cmake-native/macos-x86_64/clang/lib/predefined_macros.txt` line 65;
- the byte-identical copies in each base's target-configuration directory
  (`lib_macos_arm64/`, `lib_macos_x86_64/`, read with `--target`).

```
clang no  __BLOCKS__ 1
```

The same four files predefine `__block`
(`__attribute__((__blocks__(byref)))`), which clang defines together with
`__BLOCKS__`, and only when blocks are enabled.

The front end has no support for Apple's blocks extension: no block pointer
type, and `^` is not accepted in a declarator. The macOS SDK declares
block-taking functions under `#ifdef __BLOCKS__`. So every translation unit
that includes `<stdlib.h>`, directly or through libc++ (`<utility>`,
`<string>`, `<vector>`, `<algorithm>`, `<iostream>`, …), fails to compile in
the documented macOS-native setup (`HACKING.md`).

## Reproducer

```c++
#if __BLOCKS__
int atexit_b(void (^)(void));
#endif
int main() { return 0; }
```

```sh
cpfe --clang_version=210000 --c++20 --no_wrap_diagnostics blocks.cpp
```

**Actual:**

```
"blocks.cpp", line 2: error: expected an expression
  int atexit_b(void (^)(void));
                     ^
"blocks.cpp", line 2: error: expression preceding parentheses of apparent call must have (pointer-to-) function type
  int atexit_b(void (^)(void));
               ^
"blocks.cpp", line 2: error: type name is not allowed
  int atexit_b(void (^)(void));
                        ^
3 errors detected in the compilation of "blocks.cpp".
```

**Expected:** no diagnostics. Either `__BLOCKS__` is not predefined, so the
declaration is skipped, or the declaration is accepted as Apple clang
accepts it (`clang++ -std=c++20 -fsyntax-only blocks.cpp` reports nothing).

With the SDK (Command Line Tools 27.0, SDK 27.0), `#include <utility>` alone
gives 8 errors in `usr/include/_stdlib.h`, at lines 298 (3), 308, 345, 352,
360 and 368, all on `^` declarators, for example:

```
"…/MacOSX27.0.sdk/usr/include/_stdlib.h", line 298: error: expected an expression
  int	 atexit_b(void (^ _Nonnull)(void)) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
     	                ^
```

## Confirmation

- `grep` finds no blocks support in `src/` (block pointer types,
  `__BLOCKS__`).
- `__BLOCKS__` comes only from the macOS bases' `predefined_macros.txt` (four
  copies). They are output of `util/make_predef_macro_table`, which keeps
  every macro the host clang predefines unless it is listed in
  `delete_unneeded_entries`.
- Apple clang 21 predefines `__BLOCKS__ 1`
  (`clang++ -dM -E -x c++ /dev/null`), and it supports blocks. With
  `-fno-blocks` it defines neither `__BLOCKS__` nor `__block`.
- The front end's own `__has_feature(blocks)` and `__has_extension(blocks)`
  are 0, which libc++ tests.
- With the fix (branch `0005`; the same upstream `cpfe` binary, `EDG_BASE`
  switched between the two bases):
  - the reproducer has no diagnostics, with the arm64 and the x86_64 base;
  - `#include <utility>` and `#include <cstdlib>` go from 8 errors to 0, and
    C17 `#include <stdlib.h>` from 6 to 0;
  - `<string>`, `<vector>`, `<algorithm>` and `<iostream>` go from 17 errors to
    9; the rest are [0006](../0006-macos-base-missing-interference-size-macros.md)
    and [0007](../0007-clang-ext-vector-type-unsupported.md);
  - run natively with `edgy` against its recording, in `edg_x86_64` and
    `edg_x86_64_cp`, the new test fails with the upstream base, fails with a
    base that keeps `__block`, and passes with the fixed base.

## Fix

Branch `0005`, commit `7173149c6b` "Do not define `__BLOCKS__` with macOS
Clang [GH #307]":
- It removes `__BLOCKS__` and `__block` from the four copies of
  `predefined_macros.txt`. The headers then take their non-blocks paths.
- It adds both macros to `delete_unneeded_entries` in
  `util/make_predef_macro_table` (clang branch), so a table regenerated on
  macOS does not bring them back. Run on Apple clang 21 before and after, the
  generated table differs in exactly those two lines. `util/Changes` has an
  entry.
- The test is `tests/tests/changes/GH307.sft.cpp`: the
  reproducer, plus checks that `__block` and `__has_feature(blocks)` are off.
  The Linux bases never define the macros, so in the Docker configurations it
  passes with or without the fix. It fails before and passes after only
  natively on macOS.
- No `src/Changes` entry: CONTRIBUTING.md exempts build-system changes.

Supporting the blocks extension in clang mode is the larger alternative, and
nothing needs it to parse the headers.

## Impact on cerastes

cerastes runs EDG natively on macOS with the SDK's headers. Before the fix,
a verified program could not include any C++ standard header that reaches
`<stdlib.h>`, so the tests used only `<cstdio>`, `<cstdint>`, `<concepts>`,
`<type_traits>` and `<limits>` (decision 0001). With the fork pinned at the
merge, `<utility>` and `<cstdlib>` parse.
