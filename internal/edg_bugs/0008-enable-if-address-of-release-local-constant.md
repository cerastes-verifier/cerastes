# 0008 — Taking the address of a function with an `enable_if` whose condition is a constant is an internal error

- **Labels:** `bug`
- **Found:** analysis of 0003 and 0004 (2026-10-15), while probing the address-of
  path of `enable_if`; reproduced on upstream `d6f83174c7` the same day
- **Status:** open
- **Upstream issue:** not filed

## Summary

`enable_if_cond_is_true_constant` (`src/exprutil.c`, line 17785) decides whether
the address of a function with a clang `enable_if` attribute may be taken. When
the condition folds to a constant it records the constant in the attribute with
`move_local_constant_to_il(&cp)`, which sets `cp` to NULL, and then, at line
17817, calls `release_local_constant(&cp)` on that NULL pointer:

```c
      if (interpret_expr(cond, /*is_constant_evaluated=*/TRUE,
                         /*force_prvalue=*/TRUE, cp, &diag_list)) {
        ...
        il_cp = move_local_constant_to_il(&cp);      /* sets cp to NULL */
        ...
      }  /* if */
      discard_more_info_list(&diag_list);
      release_local_constant(&cp);                   /* line 17817: *cpp == NULL */
```

`release_local_constant` asserts `*cpp != NULL` (`il_alloc.c`, line 1071), so
every use of the address of such a function is an internal error. It happens
whether the constant is true or false. A condition that names a parameter does
not fold, `cp` is still valid, and the front end issues its normal diagnostic.

## Reproducer

```c++
// Taking the address of a function whose enable_if condition is a constant.
int f(int x) __attribute__((enable_if(1, "always")));
int (*p)(int) = f;
```

```sh
cpfe --clang_version=210000 --c++17 --no_wrap_diagnostics addr_true.cpp
```

**Actual:**

```
"addr_true.cpp", line 3: internal error: assertion failed at: "il_alloc.c", line 1071 in release_local_constant
  int (*p)(int) = f;
                  ^
1 catastrophic error detected in the compilation of "addr_true.cpp".
Compilation aborted.
```

(The process then aborts with signal 6.) The same with `enable_if(0, "never")`,
`enable_if(sizeof(T) == 4, ...)` on a function template (`&one<int>`), and a
member function (`&S<int>::f`).

**Expected:** no diagnostic for the true condition, as with clang
(`clang++ -std=c++17 -fsyntax-only addr_true.cpp` reports nothing). For the
false condition the front end's own diagnostic:

```
int g(int x) __attribute__((enable_if(0, "never")));
int (*q)(int) = g;
```

```
"addr_false.cpp", line 2: error: cannot take the address of a function with an "enable_if" attribute whose condition is not unconditionally true
```

clang says: `error: cannot take address of function 'g' because it has one or
more non-tautological enable_if conditions`.

## Confirmation

- Upstream `d6f83174c7`, native `macos-arm-clang-debug` build, run as
  `HACKING.md` describes. Apple clang 21 gives the expected results above.
- A condition that names a parameter is unaffected (it does not fold):
  `int h(int x) __attribute__((enable_if(x > 0, "positive"))); int (*r)(int) = h;`
  gives the diagnostic above, as clang does.
- It is not specific to this build. `CHECKING` is 1 in every EDG
  configuration (`cmake/macro-conf/support/simple-conf/{debug,release}/base.cmakedef`),
  and the repository's own recordings contain the internal error:
  `tests/tests/imported/clang/cpp/SemaCXX/.enable_if.rto/{default.1.1,default.2.1,edg_x86_32_cp.*,edg_x86_64_cp.*}.txt`
  record `line 288: internal error: assertion failed at: "il_alloc.c", line <suppressed> in release_local_constant`
  for `auto B = (VoidFooTy)foo;` in `SemaCXX/enable_if.sft.cpp`. So the
  recordings of that test have been recording the crash, and the lines after
  it in that test (289 onward) have never been checked. With assertions
  compiled out the call would dereference the NULL pointer instead
  (`(*cpp)->next`), so there is no configuration in which it works. (That last
  point is from reading `release_local_constant`; it was not run.)
- With the one-line change below, in a native build, `addr_true.cpp` is
  accepted and `addr_false.cpp` and the parameter case give the
  "not unconditionally true" diagnostic.

## Suggested fix

Release the local constant only if it was not moved:

```c
      if (cp != NULL) release_local_constant(&cp);
```

(or release it in the `else` of the `if (interpret_expr(...))`). Fixing it
exposes the rest of `SemaCXX/enable_if.sft.cpp` after line 288, whose
recordings then change (about 100 diagnostics, one error limit reached); they
need to be reviewed and re-recorded with the fix.

## Impact on cerastes

None known. cerastes does not take the address of functions with `enable_if`.
The 0003/0004 analysis found it when probing, and the fix commits for those
bugs must not include this change.
