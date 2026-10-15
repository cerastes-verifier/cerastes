# 0006 — The macOS clang base lacks `__GCC_DESTRUCTIVE_SIZE` and `__GCC_CONSTRUCTIVE_SIZE`

- **Labels:** `bug`, `infrastructure`
- **Found:** M1.5 (2026-10-14), cause 3 of the libc++ failures in cerastes decision
  0001; reproduced on upstream on 2026-10-15
- **Status:** open
- **Upstream issue:** not filed

## Summary

Clang 19 and later predefine `__GCC_DESTRUCTIVE_SIZE` and
`__GCC_CONSTRUCTIVE_SIZE`. libc++ uses them unconditionally from C++17 on, in
`<__new/interference_size.h>`, to define
`std::hardware_destructive_interference_size` and
`std::hardware_constructive_interference_size`.

The native macOS base for clang mode
(`bases/cmake-native/macos-arm/clang/lib/predefined_macros.txt`) does not
define them. The Linux docker bases do (for example
`bases/docker/dev-env/clang/lib_linux_riscv64/predefined_macros.txt`).

The macOS file appears to be a snapshot of Apple clang 15: it sets
`__apple_build_version__ 15000309`. That stays so even when the front end
emulates a newer clang with `--clang_version`, as `HACKING.md` says to do
with the scraped version. The current Command Line Tools ship Apple clang 21
and its libc++, so every C++17 or later translation unit that reaches `<new>`
fails.

## Reproducer

```c++
#ifndef __GCC_DESTRUCTIVE_SIZE
#error "__GCC_DESTRUCTIVE_SIZE is not defined"
#endif
#ifndef __GCC_CONSTRUCTIVE_SIZE
#error "__GCC_CONSTRUCTIVE_SIZE is not defined"
#endif
int x = __GCC_DESTRUCTIVE_SIZE + __GCC_CONSTRUCTIVE_SIZE;
```

```sh
cpfe --clang_version=210000 --c++20 --no_wrap_diagnostics interference.cpp
```

**Actual:**

```
"interference.cpp", line 2: catastrophic error: #error directive: "__GCC_DESTRUCTIVE_SIZE is not defined"
  #error "__GCC_DESTRUCTIVE_SIZE is not defined"
   ^
1 catastrophic error detected in the compilation of "interference.cpp".
```

**Expected:** no diagnostics, as with Apple clang 21. On arm64 it defines
`__GCC_CONSTRUCTIVE_SIZE 64` and `__GCC_DESTRUCTIVE_SIZE 256`.

With the SDK (Command Line Tools 27.0, SDK 27.0), `#include <algorithm>`
gives, among others:

```
"…/c++/v1/__new/interference_size.h", line 23: error: identifier "__GCC_DESTRUCTIVE_SIZE" is undefined
  inline constexpr size_t hardware_destructive_interference_size  = __GCC_DESTRUCTIVE_SIZE;
                                                                    ^
"…/c++/v1/__new/interference_size.h", line 24: error: identifier "__GCC_CONSTRUCTIVE_SIZE" is undefined
  inline constexpr size_t hardware_constructive_interference_size = __GCC_CONSTRUCTIVE_SIZE;
                                                                    ^
```

## Confirmation

- `clang++ -dM -E -x c++ /dev/null` (Apple clang 21) prints
  `__GCC_CONSTRUCTIVE_SIZE 64`, `__GCC_DESTRUCTIVE_SIZE 256` and
  `__apple_build_version__ 21000334`.
- With `--clang_version=210000`, the front end has `__clang_major__ == 21`
  but `__apple_build_version__ == 15000309`. Both were checked with
  `static_assert`.
- Neither interference-size macro appears in either macOS base.

## Suggested fix

Add the two macros to the macOS clang bases. Apple clang 21 on arm64 uses 64
and 256; the x86_64 values should be scraped from an x86_64 host.

More generally, the macOS-native setup could generate `predefined_macros.txt`
from the host compiler (`clang -dM -E`), as it already scrapes the include
directories and the version. Then the macros would follow the installed
clang rather than a fixed Apple clang 15 snapshot. That would also fix
`__apple_build_version__`.

## Impact on cerastes

As with [0005](0005-macos-base-predefines-blocks.md), cerastes cannot verify
a program that includes `<new>`, or anything that reaches it (`<string>`,
`<vector>`, `<algorithm>`, `<iostream>`), in C++17 or later with the current
Command Line Tools.
