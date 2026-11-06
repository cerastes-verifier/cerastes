# 0051 — cerastes's tool code in the owner's Boost style

- **Status:** accepted (2026-11-05, S7), drafted before the layout pass (invariant 6). Reviewed in two rounds by the
  read-only soundness auditor (`build/s7/audit{1,2}.md` in the session's worktree): round 1, before the layout
  landed, found one blocking item (hand edits invisible to object identity: the token level of item 6 answers it)
  and nine should-fix items (fail-closed guards, the tool's coverage, its normalisation, wording); round 2 found no
  blocking item and six should-fix items (members left multi-declarator, the `braces` level's limit, overclaims).
  Every item was taken. Owner decisions: handoff questions
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
   sweep (item 5): brace initialization of what was copy-initialized (`T x = e;` becomes `T x {e};`, for-init
   included; lambdas keep `=`, as in decimal), `U` on literals that an unsigned type receives and on every shift count
   (`n == 0U` for an unsigned `n`, `x << 3U`; `1 << 20U` keeps the `int` it shifts), one declarator per declaration
   (members included; a `for`-init with two declarators is the one exception, since it cannot be split).
   Parenthesised direct-initialization (`std::vector<int> v(3, 0)`, `std::string s(n, c)`) keeps its parentheses:
   braces would select a container's `initializer_list` constructor and change the value.
2. **The configuration** is `src/.clang-format`, with a byte-identical copy in `include/.clang-format`
   (`scripts/format.sh` refuses to run when they differ); each non-default option is explained there, checked against
   the exemplars. The only option that changes code tokens is `InsertBraces`; three can change comment text, never
   code: `FixNamespaceComments` (`} // namespace x`), `SpacesInLineCommentPrefix` (`//x` becomes `// x`) and
   `ReflowComments: IndentOnly` (re-indents, never re-wraps). `SortIncludes: Never` (EDG's headers depend on include
   order), `SortUsingDeclarations: Never` (LLVM style sorts them) and `BreakStringLiterals: false` keep order and
   strings as written. `SpaceBeforeCpp11BracedList: true` gives `T x {e};`, as the exemplars' declarations have it;
   clang-format has one switch for declarations and temporaries, so a braced temporary is spelled `T {e}` too, where
   the exemplars mostly write `T{e}`: an accepted difference.
3. **What is formatted:** the tracked `src/**/*.{cpp,h,inc}` and `include/cerastes.h`, listed by `scripts/format.sh`
   from `git ls-files`. Excluded: `src/driver/edg_options.inc` (generated, compared byte for byte by
   `check_edg_options.sh`), `src/backend/diagnostics.def` (an X-macro table that `run_tests.py` reads by regular
   expression), `src/driver/tool_paths.h.in` (a CMake template) and the `CMakeLists.txt` files. **Fail-closed
   guards:** the root `.clang-format` is `DisableFormat: true`, so a C or C++ file anywhere but `src/` and `include/`
   (tests, doc examples, `scripts/testdata/`'s fixtures, `internal/`'s probes, EDG, vlibc, a directory added later) is
   never formatted by an editor or by `clang-format -i`: test lines are expectations (invariant 1). The root
   `.clang-format-ignore` lists `tests/**`, `doc/**` and `third_party/**` as a second guard (in 23.1.3 only the `/**`
   form matches a directory) and the three excluded files of `src/`.
4. **The pin** (148): PyPI's official `clang_format` wheel 23.1.3, one per platform (macOS arm64, manylinux x86-64),
   downloaded by `bootstrap.sh` with a SHA-256 like Z3 and unpacked to `build/tools/clang-format-23.1.3/`;
   `build/env.sh` exports `CERASTES_CLANG_FORMAT`. The unpacked binary's SHA-256 is pinned too and checked on every
   bootstrap run (the wheel is deleted after unpacking), with an anchored version check. Not Homebrew's llvm, which
   floats, and not Ubuntu's, which has only clang-format 21 in 26.04: the two CI jobs must format alike. Record 0001's
   table has the row with the four digests.
5. **The sweep** (147) ran once, by a helper that is not kept (`build/s7/sweep/`, described here so it can be
   rebuilt). A small clang tool (`sweep_scan.cpp`, built against Homebrew's LLVM) walks the AST of every TU with the
   commands of `compile_commands.json` and keeps only sites spelled in cerastes's own files outside any macro expansion
   or macro argument. It never visits a templated declaration (`Decl::isTemplated`), so uninstantiated and dependent
   code is not edited. Sites are merged by file offset across TUs and instantiations, and a site is edited only when
   every occurrence the tool recorded proposes the same edit; an instantiation in which a literal is *not* converted
   to an unsigned type records nothing, so it cannot veto (no template in `src/` has a literal in such a context, and
   every emitted instantiation is checked by the IR). The classes:
   - **declarators:** clang-tidy's `readability-isolate-declaration` fix-its, which see statement-level declarations
     only; member declarations with several declarators were split by hand after review round 2;
   - **`U`:** an unsuffixed integer literal directly under an implicit integral conversion to an unsigned type, or the
     right operand of a builtin `<<`/`>>`/`<<=`/`>>=`; a literal whose type `U` would change in width (a decimal
     literal of type `long`) is skipped;
   - **braces:** a variable or member declared with copy-initialization: `= e` becomes `{e}`, `= {…}` becomes `{…}`.
     Excluded: an initializer that is a lambda (after implicit conversions and constructor wrappers), structured
     bindings, parameters and default arguments, range-`for` declarations, declarations in a condition
     (`if (T x = e)`; an init-statement is braced), `auto x = {…}` (braces would change the deduced type), bit-fields,
     a comment around the `=`, and an initializer not followed by `;`, `,` or `)`. A narrowing error is fixed by hand
     with `static_cast<T>(e)` inside the braces, the conversion the copy-initialization made (four sites).
   Sites the helper skips (23 macro-spelled `U` sites) keep their spelling and are listed in S7's status entry.
6. **The proof** has three levels, at every commit of the restyle (`scripts/object_identity.py --before <rev>
   [--after <rev>] [--tokens none|braces]`, with its self-test `scripts/test_object_identity.py` in `check.sh`):
   - **Tokens** (`--tokens`): every file under `src/`, `include/` and `cmake/` and every `CMakeLists.txt` of the two
     sides, C and C++ files as token streams (comments, whitespace and backslash-newline splices dropped, each
     directive closed by an end marker, `>>` kept whole), the others byte for byte. `none` allows no difference (C3,
     the hand fixups); `braces` allows only inserted, balanced `{`/`}` after `)`, `else` or `do` and before nothing but
     `;` or `}` (C2, `InsertBraces`). This is the only level that sees code no build compiles (an inactive `#if`
     branch, `include/cerastes.h`) and code that leaves no trace in the IR (`static_assert`, `= delete`, `explicit`,
     an access specifier, an uninstantiated template). `braces` cannot tell *where* a pair closes: a pair that rebinds
     an `else` or swallows the next statement passes it. So it is evidence only beside the compile passes or, for a
     pure layout commit, beside the check that every changed file is the pinned clang-format's output of its before
     version (C2 was checked so). The sweep (item 5) changes tokens by design, so its commits rely on the next two
     levels and on review.
   - **Objects and IR:** every cerastes TU of `compile_commands.json` is compiled from an export of each side with
     its own command (`-O0`, as the build) plus `-g0`, and `-fmacro-prefix-map` spelling `__FILE__` alike on both
     sides, three ways, and all three must agree: the object file byte for byte; the LLVM IR, which is the stronger
     half (at `-O0` the machine code of `x - 1` and `x - 1U` on an `int` is the same instruction, and the IR's `nsw`
     tells them apart); and the IR with `-femit-all-decls`, which also emits inline functions no TU calls, compared
     entity by entity, where only libc++ entities present on one side are counted and named rather than failed. All of
     S7's are one constexpr `bool` trait, `std::__can_be_converted_to_string_view_v<char, char_traits<char>, X>`:
     `X = char[N]` or `char*` where `std::string s {"…"}` now goes through list-initialization's overload resolution
     (which considers an `explicit` template constructor whose constraint is false), and `X = unsigned` where
     `std::string(1U, c)` replaced `(1, c)`; the call sites themselves are compared strictly by the plain IR. No `-DNDEBUG`: the shipped build has none, and nothing in
     `src/` calls `assert`.
     **Line numbers.** `encode.cpp` passes `__FILE__` and `__LINE__` to EDG's `assertion_failed` (two sites, through
     EDG's checking macros under `EXPENSIVE_CHECKING`), so a restyle changes those immediates. For calls of
     `assertion_failed` whose first argument names a file of the tree, and only those, the tool zeroes the line
     argument in the IR and reports the TU as "identical modulo N line immediates": its IR is compared with them
     normalised, and its object bytes differ and are not compared. What changes is the line an EDG internal assertion
     would name, never a result. So "byte-identical objects" holds for every TU but `encode.cpp`, whose evidence is
     its IR.
     The tool fails with "proof invalid" when an argument still names the working tree's `src/` or `include/` after
     the rewrite; when a TU reads a header from anywhere but its own side's export, `third_party/edg`, `build/` or the
     toolchain (an allow-list over its dependency file); when a tracked file under `src/`, `include/` or `cmake/` or a
     `CMakeLists.txt` differs and is neither a compiled `.cpp` nor a header some TU reads (only `include/cerastes.h`
     is allowed, its tokens checked by the first level and its use by the third, and the formatter's own
     `.clang-format` files, which no compile reads); when the two sides pin different `third_party` commits or differ
     in `scripts/vlibc_contracts.py`, inputs both sides share from the working tree; when a tracked `.cpp` is missing
     from the compile database; or when the TU count changes. Only the running platform's code is compiled, so the
     Linux run is part of the proof, not a formality: the `U` class judged "unsigned" from macOS's types.
   - **Output** over the whole suite and the VIR self-test: `build/vpr` and `build/vir-selftest` of a full `check.sh`
     at the end of S7 are byte-identical to the baseline's (the `MANIFEST` run-id line aside).
   Code that only Linux compiles (the `#else` of `__APPLE__` in `silicon.cpp`) is covered by the same tool on the Linux
   box with clang 21. `MICROSOFT_EXTENSIONS_ALLOWED` is 1 in cerastes's EDG configuration, so both sides of
   `walker.cpp`'s `#if` on it are covered: the first by compiling, the second by the token level.
7. **Enforcement:** `scripts/format.sh --check` runs in `check.sh`'s lint step on both platforms. New code is written in
   the style. These rules are review rules, not checked by a tool: the three sweep rules (item 5), and preprocessor
   directives inside a function or a class indented to the code's level with a blank line around them where the
   formatter keeps one (`IndentPPDirectives: Leave` keeps what is written). `// clang-format off` is used only where
   the formatter cannot lay a construct out (a `#define` body inside a function, a `#error` line longer than the
   limit), each with its reason beside it.
8. **History:** `.git-blame-ignore-revs` lists the restyle's commits (GitHub reads it; locally
   `git config blame.ignoreRevsFile .git-blame-ignore-revs`). Line citations of cerastes source written before S7
   (`encode.cpp:1462`) refer to the layout of `aa488d8` and are kept as history: in decision records, in
   `internal/il-inventory.md` (M1's gate evidence, about 70 citations, which says so at its head) and in the handoff's
   older notes; the handoff's live citations name functions instead of lines.

## Consequences

- Every later branch starts from the restyled tree; a branch open across S7 would conflict on every line, which is why
  S7 merged before M4.5d2b branched.
- `src/backend/edg_il.h` changed layout, so `edg_tree.py`'s configuration digest changed once and `cerastes-fe` rebuilt
  (time only).
- The proof tool stays for later refactors: a change meant to be behaviour-neutral can be checked with
  `object_identity.py --before <base>` long before a full `check.sh`.
