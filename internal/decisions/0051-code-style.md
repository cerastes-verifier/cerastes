# 0051 — cerastes's tool code in the owner's Boost style

- **Status:** draft (2026-11-05, S7), written before the layout pass (invariant 6). Owner decisions: handoff questions
  146 (a code-style session of its own, after M4.5d2v and before M4.5d2b, proved behaviour-neutral), 147 (the full
  sweep: brace initialization, `U` on unsigned literals and one declarator per declaration as well as clang-format's
  layout), 148 (the pinned clang-format is PyPI's `clang_format` wheel 23.1.3 on both platforms) and 149 (lambda bodies
  are Allman).
- **Milestone:** S7; every later session writes new tool code in this style, and `check.sh` keeps the layout.
- **Amends:** 0001 (a pin row for clang-format).

## Context

vlibc and vlibm follow the owner's Boost libraries' style (`boost/libs/int128`, `decimal`, `charconv`; vlibc's
`CLAUDE.md` §"Code style"). cerastes's own tool code did not: about 44,700 lines in `src/` and `include/cerastes.h`,
written in two-space K&R with unbraced one-line `if`s and `T *p`. The owner asked for the same style in cerastes
(2026-11-04, during M4.5d2v: "Cerastes needs to be this way too but that refactoring may need its own session").

A restyle touches every line of `src/`, so it conflicts with any branch open beside it, and it must not change what
cerastes does: no verdict, no message, no line of emitted Viper. A layout change alone is whitespace, but the sweep the
owner chose (147) changes tokens: braces inserted around single statements, `= e` rewritten as `{e}`, `U` appended to
literals, declarations split. Each of those can change meaning in C++ (an `explicit` constructor that brace
initialization admits, an `initializer_list` constructor that braces prefer, an `int` operand turned unsigned, an
`else` rebound by an inserted brace). The proof has to see such a change wherever it could happen.

## Decision

1. **The style** (vlibc's text, with the exemplars as tie-breakers): Allman braces everywhere, lambda bodies included
   (149); braces on every control statement; `else`/`else if`/`catch` on their own line; 4 spaces, no tabs; `case`
   labels indented one level; `public:` at the class's column; namespaces attached (`namespace cerastes {`) and closed
   with `} // namespace x`; `*` and `&` on the type (`T* p`, `T& r`); about 120 columns; `if (`/`for (`/`while (`/
   `switch (` with a space and none after a function's name. Three rules clang-format cannot apply, written by the
   sweep (item 5): brace initialization (`T x {e};`, `T x {};`, for-init included; lambdas keep `=`, as in decimal),
   `U` on unsigned literals, one declarator per declaration.
2. **The configuration** is `.clang-format` at the root (S7 settles its options against the exemplars; each non-default
   option is explained there). The only option that changes tokens is `InsertBraces`; `SortIncludes: Never` (EDG's
   headers depend on include order), `SortUsingDeclarations: Never`, `BreakStringLiterals: false` and
   `ReflowComments: IndentOnly` keep strings, comments and order as written.
3. **What is formatted:** the tracked `src/**/*.{cpp,h,inc}` and `include/cerastes.h`, listed by `scripts/format.sh`
   from `git ls-files`. Excluded: `src/driver/edg_options.inc` (generated, compared byte for byte by
   `check_edg_options.sh`), `src/backend/diagnostics.def` (an X-macro table that `run_tests.py` reads by regular
   expression), `src/driver/tool_paths.h.in` (a CMake template) and the `CMakeLists.txt` files. **Never formatted:**
   `tests/`, `doc/` and `third_party/`, guarded by a `.clang-format` with `DisableFormat: true` in each and by
   `.clang-format-ignore`, so formatting a test, a doc example or an EDG or vlibc file from any editor changes nothing
   (test lines are expectations, invariant 1).
4. **The pin** (148): PyPI's official `clang_format` wheel 23.1.3, one per platform (macOS arm64, manylinux x86-64),
   downloaded by `bootstrap.sh` with a SHA-256 like Z3 and unpacked to `build/tools/clang-format-23.1.3/`;
   `build/env.sh` exports `CERASTES_CLANG_FORMAT`. Not Homebrew's llvm, which floats, and not Ubuntu's, which has only
   clang-format 21 in 26.04: the two CI jobs must format alike. Record 0001's table has the row.
5. **The sweep** (147) ran once, by a helper that is not kept (`build/s7/sweep/`, described here so it can be
   rebuilt). It reads clang's AST of every TU (the commands of `compile_commands.json`) and keeps only sites spelled in
   cerastes's own files outside any macro expansion or macro argument, deduplicated by file offset and edited only when
   every TU and template instantiation agrees:
   - **declarators:** clang-tidy's `readability-isolate-declaration` fix-its;
   - **`U`:** an unsuffixed integer literal directly under an implicit integral conversion to an unsigned type, or the
     right operand of `<<`/`>>`;
   - **braces:** a variable or member declared with copy-initialization: `= e` becomes `{e}`, `= {…}` becomes `{…}`.
     Excluded: an initializer that is a lambda, structured bindings, default arguments, range-`for` declarations and
     declarations in a condition (`if (T x = e)`; an init-statement is braced). A narrowing error is fixed with
     `static_cast<T>(e)` inside the braces, the conversion the copy-initialization made.
   Sites the helper skips (macros, disagreeing instantiations) keep their spelling and are listed in S7's status entry.
6. **The proof** has two levels, at every commit of the restyle:
   - **Objects and IR** (`scripts/object_identity.py --before <rev> [--after <rev>]`): every cerastes TU of
     `compile_commands.json` is compiled from an export of each side with its own command plus `-g0 -DNDEBUG`
     (`-fmacro-prefix-map` spelling `__FILE__` alike on both sides) three ways, and all three must agree:
     - the object file, byte for byte;
     - the `-O0` LLVM IR (its module-name lines dropped). The IR is the stronger half: at `-O0` the machine code of
       `x - 1` and `x - 1U` on an `int` is the same instruction, and the IR's `nsw` tells them apart;
     - the IR with `-femit-all-decls`, which also emits inline functions no TU calls, compared entity by entity. Only
       libc++ entities present on one side are counted rather than failed: `std::string(1U, c)` instantiates a libc++
       helper for `unsigned` where `(1, c)` did for `int`, and the call sites themselves are compared strictly by the
       plain IR.
     **Line numbers.** `encode.cpp` passes `__FILE__` and `__LINE__` to EDG's assertion routine (through EDG's
     checking macros), so a restyle changes those immediates. The tool zeroes, in the IR only, the line argument of a
     call whose first argument is a path inside `src/` or `include/`, compares objects built from that IR, and reports
     the TU as "identical modulo lines". What changes is the line an EDG internal assertion would name, never a result.
     The tool fails with "proof invalid" when an argument still names the working tree's `src/` or `include/` after the
     rewrite, when a TU reads a cerastes header from outside its own side's export (from its dependency file), when a
     tracked header no TU reads differs (only `include/cerastes.h` is allowed, see below), when a tracked `.cpp` is
     missing from the compile database, or when the TU count changes. Its self-test (`scripts/test_object_identity.py`,
     in `check.sh`) has the must-pass and must-fail cases of S7's plan and says which pass catches each.
   - **Output** over the whole suite and the VIR self-test: `build/vpr` and `build/vir-selftest` of a full `check.sh`
     at the end of S7 are byte-identical to the baseline's (the `MANIFEST` run-id line aside).
   Code that no TU compiles on macOS (`!__APPLE__` in `silicon.cpp`) is covered by the same tool run on the Linux box
   with clang 21. `include/cerastes.h` is read only by verified programs, so the output identity covers it.
7. **Enforcement:** `scripts/format.sh --check` runs in `check.sh`'s lint step on both platforms. New code is written in
   the style; the three sweep rules are review rules, not checked by a tool.
8. **History:** `.git-blame-ignore-revs` lists the restyle's commits (GitHub reads it; locally
   `git config blame.ignoreRevsFile .git-blame-ignore-revs`). Line citations of cerastes source in decision records
   written before S7 (`encode.cpp:1462`) refer to the layout before S7 and are kept as history; the live citations in
   `internal/handoff.md` were updated.

## Consequences

- Every later branch starts from the restyled tree; a branch open across S7 would conflict on every line, which is why
  S7 merged before M4.5d2b branched.
- `src/backend/edg_il.h` changed layout, so `edg_tree.py`'s configuration digest changed once and `cerastes-fe` rebuilt
  (time only).
- The proof tool stays for later refactors: a change meant to be behaviour-neutral can be checked with
  `object_identity.py --before <base>` long before a full `check.sh`.
