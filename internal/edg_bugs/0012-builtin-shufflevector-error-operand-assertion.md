# 0012 — `__builtin_shufflevector` with an operand that becomes an error operand is an internal error

- **Labels:** `bug`
- **Found:** 0007 fix (2026-10-15), in the clang-imported test
  `Sema/constant-builtins-vector.sft.cpp`. Reproduced on upstream `d6f83174c7` the
  same day, in a Linux build hosted by clang
- **Status:** open
- **Upstream issue:** not filed

## Summary

`scan_builtin_shuffle` (`src/expr.c`) checks that the first operand is a vector
(`check_operand_is_vector`, line 17456) and later transforms the operands
(`do_operand_transformations`, lines 17591-17606). When the operand is a
`constexpr` variable whose initializer was erroneous, the transformation turns the
reference into an error operand, whose type is the error type. The code that makes
the result type then asserts that the operand still has a vector type:

```c
/* src/expr.c, lines 17626-17627 */
        check_assertion(!op1_is_dependent && !op2_is_dependent &&
                        is_vector_type(p_op1->type));
```

`op1_is_vector` was computed before the transformation, so the check passed and
`result_type` is still NULL. Instrumented values at the assertion:
`op1_is_vector=1 op1_dep=0 op2_dep=0 op2_is_vector=1 is_vector_type(op1)=0 is_error_type(op1)=1`.
So a second error in a shufflevector that uses a variable that was already in
error is a crash, where the first error should be the only diagnostic. It requires
integer arguments (`int_args != 0`), the `__builtin_shufflevector` form.

The repository's own recordings contain it: `tests/tests/imported/clang/cpp/Sema/.constant-builtins-vector.rto/default.{1..6}.1.txt`
each record `line 115: internal error: assertion failed at: "expr.c", line <suppressed> in scan_builtin_shuffle`
for the first `__builtin_shufflevector` of that test. The error before it in the
test is the front end's lack of constant evaluation of `__builtin_convertvector`
(`expression cannot be interpreted`), which is valid code in clang.

## Reproducer

```c++
// A constexpr vector whose initializer has an error, used as an operand of __builtin_shufflevector.
typedef char v4c __attribute__((vector_size(4)));
constexpr v4c c = nonexistent;
v4c d = __builtin_shufflevector(c, c, 0, 1, 2, 3);
```

```sh
# upstream d6f83174c7, linux-clang-debug build, in the Docker environment
cpfe --edg_base bases/docker/dev-env/clang --clang --c++20 --no_code_gen --no_wrap_diagnostics shuf_err.cpp
```

**Actual:**

```
".tmp-probes/shuf_err.cpp", line 3: error: identifier "nonexistent" is undefined
  constexpr v4c c = nonexistent;
                    ^
".tmp-probes/shuf_err.cpp", line 4: internal error: assertion failed at: "expr.c", line 17627 in scan_builtin_shuffle
  v4c d = __builtin_shufflevector(c, c, 0, 1, 2, 3);
                                                   ^
1 error and 1 catastrophic error detected in the compilation of ".tmp-probes/shuf_err.cpp".
Compilation aborted.
```

**Expected:** only the first error, as Apple clang 21 gives:

```
shuf_err.cpp:3:19: error: use of undeclared identifier 'nonexistent'
1 error generated.
```

A valid-code variant, from the imported test, where the first error is the lack
of constant evaluation of `__builtin_convertvector` (clang accepts the file):

```c++
typedef int v4i __attribute__((vector_size(16)));
typedef char v4c __attribute__((vector_size(4)));
constexpr v4i a = {0, 1, 2, 3};
constexpr v4c c = __builtin_convertvector(a, v4c);   // error: expression cannot be interpreted
v4c d = __builtin_shufflevector(c, c, 0, 1, 2, 3);   // internal error
```

Expected: no diagnostic (clang accepts it); actual: the "expression must have a
constant value" error on line 4, then the same internal error on line 5.

## Confirmation

- The same file crashes in the native macOS build with the 0007 fix (branch `0007`,
  `91a07dbc0d`; the front end has no vector types without it), so it is not a Linux
  effect, and `--no_code_gen` keeps the C-generating back end out of it
  (`BACK_END_IS_C_GEN_BE` plays no part: the assertion is in the front end).
- A variable with a failed initializer is needed. With a non-constexpr
  `v4c c = nonexistent;`, or with the undefined identifier in the call
  (`__builtin_shufflevector(nonexistent, nonexistent, 0, 1, 2, 3)`), the front end
  gives the one error without the assertion; a valid call
  (`v4i f(v4i a) { return __builtin_shufflevector(a, a, 0, 1, 2, 3); }`) is
  accepted.
- Instrumentation (a `fprintf` before the assertion, Linux clang build) gave the
  values quoted above; the value `is_error_type(op1) = 1` is from there.

## Suggested fix

Test the operand after `do_operand_transformations`, or set `result_type = error_type()`
when `is_error_type(p_op1->type)` after the transformations; that is what the code
before them already does for an operand that is an error when checked.

## Impact on cerastes

Reachable. The assertion is in the front end that cerastes links, once vector types
are enabled for macOS (the 0007 fix). It needs a source with an error already, or the
use of `__builtin_shufflevector` on a `constexpr` vector that is initialized with
`__builtin_convertvector`. It is not reached by the libc++ headers: `__algorithm/simd_utils.h`
(used by `std::find`, `std::count`, ...) uses `__builtin_convertvector`,
`__builtin_reduce_*` and `__builtin_bit_cast` but never `__builtin_shufflevector`.
`<algorithm>`, `<vector>` and `<string>` with `std::find`, `std::count` and `std::any_of`
instantiated, on the 0007 fix together with the 0005 and 0006 bases, are accepted
without a diagnostic. `__builtin_shufflevector` is used by the SDK's `usr/include/simd/`
headers and by clang's own resource-directory headers on the same include path
(`arm_neon.h`, with about 10,000 uses, and the x86 intrinsic headers). Those uses are
valid and are accepted: `<arm_neon.h>` gives 0 errors with the 0007 fix. The assertion
needs an operand that is already in error, so valid headers do not reach it.
