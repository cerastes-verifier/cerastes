# 0019 — In C++23 and later, the parameter names of a function type are not found in its trailing return type when the type is a sizeof operand or a function parameter

- **Labels:** `bug`
- **Found:** 0003 review (2026-10-15), while testing nested function types in `enable_if`
  conditions. Reproduced on upstream `d6f83174c7` the same day. It does not involve
  `enable_if` or templates
- **Status:** open
- **Upstream issue:** not filed

## Summary

A function type with named parameters can use them in its trailing return type:
`auto (*)(int pa, int pc) -> decltype(pc)`. With `--c++23` (and `--c++2b`, `--c++26`)
the front end does not find those names in two places: when the type is the
operand of `sizeof` or `alignof`, and when it is the type of a function parameter.
In C++20 and earlier it does. Elsewhere in C++23 it works: in an alias declaration,
as a variable's declarator, as the operand of a `static_cast`, and in a function body.

## Reproducer

```c++
int a = sizeof(auto (*)(int pa, int pc) -> decltype(pa));
```

```c++
void f(auto (*)(int pa, int pc) -> decltype(pc));
```

```sh
cpfe --clang_version=210000 --c++23 --no_wrap_diagnostics sizeof23.cpp
```

**Actual:**

```
"sizeof23.cpp", line 1: error: identifier "pa" is undefined
  int a = sizeof(auto (*)(int pa, int pc) -> decltype(pa));
                                                      ^
```

The second file gives `identifier "pc" is undefined` for the same reason. Both are
accepted with `--c++20`.

**Expected:** no diagnostic. Apple clang 21 (`clang++ -std=c++23 -fsyntax-only`) and
g++ 16 accept both.

## Confirmation

- Upstream `d6f83174c7`, native `macos-arm-clang-debug` build.
- What works and what does not in C++23, with the same type
  `auto (*)(int pa, int pc) -> decltype(pc)` (probe files `p1`-`p7`, `n1`-`n8` in
  `.tmp/agent-bugs/analysis/new/c23/`; all accepted by clang and g++):

  | Context | C++23 |
  |---|---|
  | alias declaration `using F = auto (int pa, int pc) -> decltype(pc);` | accepted |
  | declarator `auto (*q)(int pa, int pc) -> decltype(pc);` and a member | accepted |
  | `static_cast<auto (*)(int pa, int pc) -> decltype(pc)>(nullptr)` | accepted |
  | inside a function body, `sizeof(auto (*)(T pa, T pc) -> decltype(pc))` | accepted |
  | `sizeof(...)` or `alignof(...)` at namespace scope or in a class (also in a `static_assert` of a class template and in `alignas(...)`) | **error** |
  | the type of a function parameter, `void f(auto (*)(int pa, int pc) -> decltype(pc))` | **error** |
  | the same in a `noexcept` of the nested type | accepted |
  | `... -> int`, without a use of the names | accepted |

- The `sizeof` forms also fail in an `enable_if` condition of a function template and
  of a non-template function (`nfa`/`nfz` cases): the condition is an expression at
  declaration scope. That is why the test of the fix for 0004 leaves C++23 out.
- I did not look for the cause in the front end. It looks like the lookup of the
  parameter names of a function declarator in a trailing return type, which depends on
  the context the type is scanned in; something differs for C++23 only
  (it passes in C++20 with the same text).

## Suggested fix

Not investigated. The parameter names of a function type must be in scope in its
trailing return type in every context the type can be scanned in, as in C++20 mode.

## Impact on cerastes

None known. cerastes does not use function types with named parameters in
expressions, and the source files are not C++23 only.
