# 0011 — The C-generating back end asserts on a vector operation when the generated-code target is not GCC

- **Labels:** `bug`
- **Found:** 0007 fix (2026-10-15). Enabling GNU vector types for the native macOS
  builds made 107 of the 108 tests that now abort (in the `edg_x86_64` configuration)
  abort in the C-generating back end on one of the sites below.
  Reproduced on upstream `d6f83174c7` the same day, in a Linux build hosted by clang
- **Status:** open
- **Upstream issue:** not filed

## Summary

The C-generating back end, when it writes a vector operation, asserts that the
generated code is for GCC:

```c
/* src/c_gen_be.c, dump_expr, line 5937 */
        case eok_vector_fill:
          ...
            check_assertion(gcc_is_generated_code_target &&
                            is_vector_type(expr_type) &&
                            !is_vector_type(operand_1->type));
```

(`eok_vector_fill` is the node that promotes a scalar to a vector in an operation
such as `v + 1`.) The same `gcc_is_generated_code_target` assertion guards the
other constructs that the back end writes in GCC's vector syntax:

| Site | Construct | Line | Aborts in the clang-hosted build |
|---|---|---|---|
| `dump_expr`, `eok_vector_fill` | scalar promoted to a vector: `v + 1`, `v <<= 2` | 5937-5939 | yes (below) |
| `dump_expr`, `eok_vector_not` | `!v`: `V4 f(V4 v) { return !v; }` | 5919-5920 | yes, `line 5920` |
| `dump_expr`, `eok_vector_land` | `v && w` | 6491 | yes, `line 6491` |
| `dump_expr`, `eok_vector_lor` | `v \|\| w` | 6529 | yes, `line 6529` |
| `write_vector_constant` | a constant of vector type | 1193 | not reproduced (`V4 k = {1,2,3,4};`, `(V4){...}`, a `constexpr` vector) |
| `dump_expr`, `eok_vector_question` | vector `a ? b : c`: `V4 f(V4 a, V4 b, V4 c) { return a ? b : c; }` | 6637-6639 | yes, `line 6639` (clang and g++ 16 accept it) |

(A seventh use, `dump_routine_decl` line 11162, is the `ifunc` attribute and is not
about vectors.) The gcc-hosted build accepts every one of the files above (exit 0).
The reviewer of the 0007 fix found 1193 and 5920 by reading the code and reproduced
5920 with `!v`; the `eok_vector_not`, `eok_vector_land`, `eok_vector_lor` and
`eok_vector_question` rows were reproduced here on the clang-hosted Linux build, and
`write_vector_constant` was not reproduced by either.

Which compiler the generated C is
for is chosen at build time from the compiler that builds the front end
(`src/targ_def.h`, lines 3487-3550: `__clang__` gives
`CLANG_IS_GENERATED_CODE_TARGET`, `__GNUC__` gives `GCC_IS_GENERATED_CODE_TARGET`)
unless the configuration sets it. In a clang-hosted build the target is clang, the
assertion is false, and any translation unit with a vector operation aborts. A
gcc-hosted build (the Docker `default` configuration) works.

Vector types are allowed in every Linux configuration
(`cmake/macro-conf/support/platform/linux/base.cmakedef`, line 60) and, since the
fix for bug 0007, in the macOS ones. The macOS native build is hosted by Apple
clang, so it is the first place the combination is reachable by default.

## Reproducer

```c++
typedef int V4 __attribute__((vector_size(16)));
V4 f(V4 v) { return v + 1; }
```

```sh
# upstream d6f83174c7, linux-clang-debug build (cmake --preset linux-clang-debug), in the Docker environment
cpfe --edg_base bases/docker/dev-env/clang --clang --c++17 v1.cpp
```

**Actual:**

```
"v1.cpp", line 2 (col. 14): internal error: assertion failed at:
          "c_gen_be.c", line 5939 in dump_expr

1 catastrophic error detected in the compilation of "v1.cpp".
Compilation aborted.
```

The same with `--g++`, and for a compound assignment on a vector
(`typedef int V4 __attribute__((vector_size(16))); void g(V4 *p) { *p <<= 2; }`,
the test `changes/EDGcpfe22503.sft.cpp`).

**Expected:** no diagnostic and a translation unit written. Both Apple clang 21
(`clang++ -std=c++17 -fsyntax-only v1.cpp`) and g++ 16 accept both files.

## Confirmation

- Same source built two ways from upstream `d6f83174c7`, run on the reproducer in the
  Docker environment:
  - `linux-clang-debug` (`CLANG_IS_GENERATED_CODE_TARGET 1`): the abort above, exit 134;
  - `linux-gcc-debug` (the `default` configuration): no output, exit 0.
- `--no_code_gen` makes the clang-hosted build accept the file (exit 0): the failure
  is in the back end, not the front end.
- Natively on macOS, with the 0007 fix (branch `0007`, `91a07dbc0d`) and the same
  reproducer: the same assertion. Without that fix the front end does not know
  vector types, which is why no native run reached it before.
- Scale (the reviewer's re-run of the suite with the assertion line numbers not
  suppressed, native `0007` cpfe, `edg_x86_64`; script and output in
  `.tmp/agent-bugs/review/0007c/rerun.sh` and `sites.txt`). The recordings suppress
  the line numbers, so the figures cannot be read from them. 108 tests abort per
  configuration:

  | Abort | Construct | Tests (cases) |
  |---|---|---|
  | `c_gen_be.c:5939` | `eok_vector_fill` | 76 (77) |
  | `c_gen_be.c:6639` | `eok_vector_question` | 27 (27) |
  | `c_gen_be.c:6529` | `eok_vector_lor` | 3 (3) |
  | `c_gen_be.c:5920` | `eok_vector_not` | 1 (1) |
  | `expr.c:17627` | bug 0012 | 1 (6) |

  Examples are `changes/EDGcpfe22503` and `EDGcpfe24511` (fill), `changes/EDGcpfe26258`
  (question), `edg/daveed/g23082a`, and the GNU C vector tests under
  `imported/gnu/c/` (`fold-minus-1`, `pr100239`, ...).

## Suggested fix

Either have the C-generating back end produce vector operations for the clang
target too (clang accepts `__attribute__((vector_size))` and the compound-literal
form that `eok_vector_fill` writes), or reject the construct with a diagnostic
instead of an internal error when the target is not GCC. The sites of the
table need the same decision.
For a clang-hosted configuration the macro-conf could also set
`GCC_IS_GENERATED_CODE_TARGET` explicitly, but then the generated C is for the wrong
compiler.

## Impact on cerastes

None. The cerastes configuration has no C-generating back end
(`BACK_END_IS_C_GEN_BE` is 0; the front end is linked to cerastes' own back end), so
`dump_expr` never runs. It concerns only the stock `cpfe` that cerastes builds for
the `edg-smoke` tests, and only for vector operations written with code generation on
(`scripts/cpfe.sh` tests use `--no_code_gen`).
