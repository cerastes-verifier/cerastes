# 0005 — The macOS clang base predefines `__BLOCKS__`, but the front end cannot parse block syntax

- **Labels:** `bug`, `infrastructure`
- **Found:** M0 (2026-10-13). Narrowed in M1.5 (cause 1 of the libc++ failures in
  cerastes decision 0001); reproduced on upstream on 2026-10-15
- **Status:** open
- **Upstream issue:** not filed

## Summary

The native macOS bases predefine `__BLOCKS__` for clang mode:
- `bases/cmake-native/macos-arm/clang/lib/predefined_macros.txt` line 114;
- `bases/cmake-native/macos-x86_64/clang/lib/predefined_macros.txt` line 65.

```
clang no  __BLOCKS__ 1
```

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
- `__BLOCKS__` comes only from the two macOS bases' `predefined_macros.txt`.
- Apple clang 21 predefines `__BLOCKS__ 1`
  (`clang++ -dM -E -x c++ /dev/null`), and it supports blocks.

## Suggested fix

Remove `__BLOCKS__` from the two macOS bases' `predefined_macros.txt`. The
headers then take their non-blocks paths. Supporting the blocks extension
in clang mode is the larger alternative.

## Impact on cerastes

cerastes runs EDG natively on macOS with the SDK's headers. Until this is
fixed, a verified program cannot include any C++ standard header that
reaches `<stdlib.h>`. The tests use only `<cstdio>`, `<cstdint>`,
`<concepts>`, `<type_traits>` and `<limits>` (decision 0001).
