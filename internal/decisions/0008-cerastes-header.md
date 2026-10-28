# 0008 — `<cerastes.h>`: macros, spec functions, ordinary compilers

- **Status:** accepted for M2 (owner chose the `CERASTES_*` names, 2026-10-14)
- **Milestone:** M2 (2026-10-14)

## Context

plan.md M2 asks for:
- a header that makes annotated source build cleanly with ordinary compilers;
- the spec functions `owns`, `reads`, `old`, `forall` and `owns_range` in
  `namespace cerastes`;
- an answer to whether spec functions are declared only or trivially defined.

Either way, `predicate`/`pure` functions must never be emitted or called at
run time. The acceptance criterion: the same test files compile with Apple
clang (and GCC if installed) using `-std=c++20 -Wall -Wextra -Werror`,
without `__CERASTES__`, with zero warnings.

Probe facts (M2 Phase 1):
- **F1.** Spelled out, `[[cerastes::requires(…)]]` can never be warning-free
  under `-Werror`:
  - Apple clang 21 warns `unknown attribute 'cerastes::requires' ignored
    [-Wunknown-attributes]`.
  - GCC 16 warns `scoped attribute directive ignored [-Wattributes]`, and for
    statement attributes also `attributes at the beginning of statement are
    ignored`.

  So the macros must expand to nothing for ordinary compilers.
- **F3.** Diagnostics for a macro-expanded attribute appear at the macro's use
  site in the user's file, the same with `--sys_include` and `-I`. A real
  error inside the header is reported either way. `--sys_include` hides only
  the header's own warnings. `-I` and `--sys_include` give byte-identical
  walker output.
- EDG cannot parse several libc++ headers on macOS SDK 27 (0001), so the
  header must include no standard headers.

## Decision

1. **Names.** `CERASTES_REQUIRES(...)`, `CERASTES_ENSURES(...)`,
   `CERASTES_INVARIANT(...)`, `CERASTES_CHECK(...)`, `CERASTES_FOLD(...)`,
   `CERASTES_UNFOLD(...)`, `CERASTES_PREDICATE`, `CERASTES_PURE` and (M3, decision 0013)
   `CERASTES_DECREASES(...)`. They
   replace plan.md's `VF_*` (owner, 2026-10-14).
   - The macros are variadic, so commas inside lambdas and `forall` calls need
     no extra parentheses.
2. **Expansion.**
   - **What decides** (amended after the CP6 audit): the header expands the
     attributes when `__has_cpp_attribute(cerastes::requires)` (C++) or
     `__has_c_attribute(cerastes::requires)` (C23) is non-zero. That is true
     only in the cerastes front end, where the namespace is registered, and
     0 in clang and GCC.
     - An `#undef __CERASTES__` therefore cannot drop contracts.
     - The front end still defines `__CERASTES__` for user code, and
       reports any source-level change of it (`cerastes_macro_changed`,
       0006 item 10).
     - **The header fails closed under the cerastes front end** (amended
       2026-10-16 after the M3-3 audit, which showed that `-U__cplusplus`, or
       `#undef __cplusplus` before the include, made every macro expand to
       nothing and a false postcondition verify): when `__CERASTES__` is
       defined but the attributes are not available, the header stops the
       run with `#error`. M2 had avoided an `#error` so that the back end
       could report a positioned diagnostic; stopping the run is the safer
       choice, and the `#error` names the header line and the reason.
       `__CERASTES__` itself cannot be removed unnoticed (item 3).
     - **Changing the names the annotations depend on is rejected**, on the
       command line by the drivers (0011 item 2) and in the source by the
       back end (`annotation_macro_changed`, M3 re-look R4c): a `#define` or
       `#undef` anywhere in the unit, outside this header, of `cerastes`, an
       attribute name (`requires`, `ensures`, `invariant`, `check`, `fold`,
       `unfold`, `predicate`, `pure`, `decreases`), a spec-function name (`old`, `owns`,
       `reads`, `forall`, `owns_range`), `CERASTES_H`, a `CERASTES_*` macro,
       `__has_cpp_attribute`, `__has_c_attribute`, or a predefined macro
       (`#define ensures requires` turned postconditions into preconditions).
       The detection uses EDG's own record of the directives
       (`RECORD_MACROS_IN_IL`), so string literals, comments and inactive
       groups do not count. `__has_cpp_attribute` and `__has_c_attribute`
       are protected as predefined macros, not by name: `<sys/cdefs.h>`
       defines the one of the other language under `#ifndef`, which is
       allowed. A directive that changes no macro (an `#undef` of an
       undefined name, an identical redefinition) is not recorded by EDG and
       is harmless. The diagnostic is reported at the directive.
   - **When the attributes are available:** each macro expands to exactly its
     attribute (0006). `CERASTES_PREDICATE` expands to
     `[[cerastes::predicate]] inline` and `CERASTES_PURE` to
     `[[cerastes::pure]] inline`.
   - **Otherwise:** the contract and statement macros expand to nothing, and
     `CERASTES_PREDICATE`/`CERASTES_PURE` expand to `inline`.
   - Rule for users: `CERASTES_PREDICATE`/`CERASTES_PURE` come first in the
     declaration, before `static`, `constexpr` and the return type. An
     attribute-specifier cannot appear in the middle of a decl-specifier
     sequence.
3. **`__CERASTES__` is defined by the cerastes front end itself** (the
   driver, not the wrapper script), so a direct run of the binary cannot
   silently drop every contract. The driver rejects attempts to undefine it.
   Ordinary compilers never define it.
4. **Spec functions are declared only, never defined:**
   ```cpp
   namespace cerastes {
   template <class T> bool owns(const T& place) noexcept;
   template <class T> bool reads(const T& place) noexcept;
   template <class T> const T& old(const T& value) noexcept;
   template <class F, class... Free> bool forall(F body, const Free&... free) noexcept;
   template <class T> bool owns_range(const T* first, decltype(sizeof 0) count) noexcept;
   }
   ```
   - **Why declared only.** A use at run time, or an emitted predicate that
     calls one, becomes a **link error** naming a `cerastes::` symbol. That
     enforces "never called at run time" with ordinary compilers. Trivial
     definitions (`return true;`) would let such calls succeed silently.
   - The functions are not `constexpr` and not `inline`, which avoids clang's
     `-Wundefined-inline`.
   - In attribute arguments they are unevaluated (0007), so EDG creates only
     declaration instances.
   - `forall`'s shape is the owner's choice (0006 item 9).
5. **`predicate`/`pure` functions are `inline` in both builds.** They have the
   same linkage in both builds and are not emitted unless odr-used. An
   unused `inline` function draws no warning from clang or GCC, whereas
   `static` would draw `-Wunused-function`. Their bodies may call spec
   functions. If a runtime call reaches one, the link fails.
6. **C.** In C23 mode the macros work, because `[[ns::attr]]` is valid C23,
   after a function declarator too. The spec functions are C++ only in M2. A
   C form waits for the memory model (M4.5).
7. **Where the header lives, and how tools find it.**
   - It is `include/cerastes.h` in this repository.
   - `scripts/cerastes-fe.sh` adds `--sys_include=$CERASTES_INCLUDE_DIR` (F3:
     use-site diagnostics are unaffected, and the header's own warnings are
     not repeated in every test).
   - Ordinary compilers use `-I include`.
   - The header includes nothing.
8. **Testing the acceptance criterion** (`native-ok`, `tests/README.md`):
   - Every positive annotation test, and every test that includes
     `<cerastes.h>`, also compiles with Apple clang, and with the
     highest-numbered GCC on `PATH` when one is installed.
   - Flags: `-std=c++20` (C: `-std=c23`) `-Wall -Wextra -Werror
     -U__CERASTES__ -I include -c`.
   - GCC is used when present, and its absence is reported. If `build/env.sh`
     names a GCC and it disappears, the test fails.
   - The GCC version is recorded in 0001, not pinned (Homebrew floats).

## Consequences

- **Pitfalls with ordinary compilers,** which the docs list and the tests
  show:
  - a parameter used only in a contract draws `-Wunused-parameter`, because
    the contract vanishes;
  - a local used only in `CERASTES_CHECK` draws `-Wunused-variable`. Use
    `[[maybe_unused]]` for such locals;
  - `if (c) CERASTES_CHECK(p);` draws `-Wempty-body`.
- **A link error naming `cerastes::owns` and the like** means that
  specification code is reachable at run time. The docs say so.
- **Multi-line macro invocations.** Errors in a multi-line `CERASTES_*`
  invocation are reported at the argument's own line and column once
  `FULLY_RESOLVED_MACRO_POSITIONS` is on (0006, Consequences). The walker prints
  original positions too. Tests keep invocations on one line for readability.

*2026-10-20: milestone numbers follow the renumbering of 2026-10-20 (plan.md: old M4 → M5, M5 → M6, M6 → M4.5 for C and M7 for C++, M7 → M8).*

*2026-10-27, amended by 0029 (M4.5a, effective from M4.5b1): item 6 — C gets the spec functions as `cerastes_owns`, `cerastes_reads`, `cerastes_owns_range`, `cerastes_owns_block`, `cerastes_live` (C23 variadic declarations), `cerastes_old` (a `typeof` macro) and `cerastes_forall` with a named predicate or pure function, declared in both modes (0029 items 3–4).*
