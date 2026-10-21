# 0020 — A lambda that names a template parameter in a function template's declaration cannot be substituted: "identifier is undefined"

- **Labels:** `bug`
- **Found:** owner's M2 handoff question 5 (2026-10-16: lambdas in the contracts of function templates must be
  able to name `T`). Reproduced on upstream `307002eae6` (and `d6f83174c7`) the same day. Upstream's own suite
  already records it: `imported/gnu/cpp/cpp2a/lambda-uneval7` and `lambda-targ4` are expected failures with
  this message
- **Status:** fixed. Branch `0020` in the fork clone is commit `c539de4101` on upstream `main` `307002eae6`, merged
  into the fork's `cerastes` with `--no-ff` (`9068279c12`, M3 close-out); its Linux failure is explained below
  (2026-10-21), so the branch is ready for the owner's upstream PR
- **Upstream issue:** not filed

## Summary

A lambda in the declaration of a function template (in the type of a parameter, in a default template argument,
in the condition of Clang's `enable_if` attribute) is scanned again from its cached tokens when template arguments
are substituted (`copy_template_param_lambda_expr`, `src/il.c` line 20503). The rescan makes the template
parameters visible only when the first parameter of the list has a declaration-info
(`template_param_list->cache.decl_info`, line 20558). That field is set only for a template template parameter
(`templates.c` line 29443) and for a non-type parameter with a dependent type (line 29061). For every other
function template, the instantiation scope pushed for the rescan (`push_instantiation_scope_for_rescan`,
`scope_stk.c` line 6023) binds no arguments, so `update_template_param_symbols(list, NULL)` (line 2081) marks every
parameter not visible: the lambda cannot name `T`, and the template cannot be used. Clang and GCC accept the code.

For Clang's `enable_if`, `eval_enable_if_attributes` (`overload.c` line 5587, the copy at line 5644) also
substituted the condition with no instantiation scope at all.

## Reproducer

```c++
template <class T>
void f(char (*)[sizeof([](T y) { return y; }(T{}))]) {}
void g() { char a[sizeof(int)]; f<int>(&a); }
```

Command: `cpfe --c++20 repro.cpp` (with the native base and the scraped system include directories).

**Actual:**

```
"repro.cpp", line 2: error: identifier "T" is undefined
  void f(char (*)[sizeof([](T y) { return y; }(T{}))]) {}
                            ^
"repro.cpp", line 3: error: no instance of function template "f" matches the argument list
            argument types are: (char (*)[4])
```

Other shapes with the same error: `template <int N> void g(A<[]{return N;}()>) {}` called as `g<1>({})`
(`imported/gnu/cpp/cpp2a/lambda-uneval7`, with `--gnu_version`); a lambda in a default template argument of a
member template of a class template (`lambda-targ4`); a lambda in a Clang `enable_if` condition.

**Expected:** no diagnostic, as with Apple clang 21 and GCC 16.

## Confirmation

Native macOS arm64, `macos-arm-clang-debug`, upstream `307002eae6`: the reproducer fails as shown, and
`lambda-uneval7` and `lambda-targ4` are expected failures with exactly this message. Apple clang 21 and GCC 16
accept every reproducer.

## Fix

Branch `0020`, commit `c539de4101`. It went through three reviews; each earlier version is replaced, never merged:
`20da6ab129` left a parameter without a value (partial argument list) not visible, so lookup fell through to an
outer entity of the same name; `d53f8f55cc` counted any token spelled like a parameter, so `::T`, `.T`, `NS::U`
and `G::template T<int>` made the copy fail. In both cases a valid program silently selected a different overload
than Clang and GCC. The fix as merged:
- When a lambda is not generic, has no template parameter list of its own, names a parameter of the list (an
  identifier token of its cached tokens) and names no pack, and an instantiation scope for the same parameter list
  is on the scope stack (compared as a pointer, so the rescan of another template is never taken for it), the
  parameter symbols are bound to the template arguments for the rescan, and restored exactly afterwards
  (visibility, type, constant, template-template argument). The path covers function templates, member templates
  and class-template partial specializations.
- The argument list can be incomplete (arguments not yet deduced): only parameters that have a value are bound
  (`update_template_param_symbols` with a partial argument list; binding all of them stored a null constant and
  crashed the front end on `imported/clang/cpp/SemaCXX/lambda-unevaluated`, which now reports its call as
  ambiguous, as the test's `expected-error` says). A parameter the lambda names that has no value makes the copy
  fail; lookup never falls through to an outer entity.
- An identifier counts as naming a parameter only if it is not preceded by `::`, `.` or `->` (also with the
  `template` keyword in between: `G::template T<int>`).
- Clang `enable_if` conditions are substituted inside `push_instantiation_scope_for_rescan` too.
- Tests `changes/GH-lambda-template-param` (and `-gnu`, `-enable-if`, `-mismatch`, `-incomplete`, `-generic`,
  `-outer`, `-overload`, `-limits`, `-qualified`); `lambda-uneval7` and `lambda-targ4` are no longer expected failures;
  `lambda-unevaluated` is re-recorded.
- Limitations, pinned by `-limits`: a nested deduction of the same template inside the lambda resets the binding
  (fails closed); a lambda that depends on a parameter only through `decltype` of a function parameter is still
  rejected (valid C++); packs named in a lambda keep the old behaviour.
- Not fixed: a lambda *called* in a constant expression in a template argument or array bound outside GNU mode
  ("template instantiation resulted in unexpected function type", the message of `lambda-uneval3/8/11/13`; bug
  0021); generic lambdas.

## The Linux failure of branch `0020` alone (explained 2026-10-21)

The M3 handoff recorded that the box run `0020d` (2026-10-17, `c539de4101`, `changes,regressions,imported`) failed
`changes/GH-lambda-template-param-qualified` cases 1 and 2 (`FAIL--OUTPUT MISMATCH`) in both 64-bit configurations,
while the test passed at the chain tip and natively. **It was not a bug in 0020: the box ran stale binaries.**
- The box tree is refreshed by `rsync -a` from the Mac (file times kept) and `edg-docker-test` builds incrementally
  in the surviving `build/gcc`. The checkout of `c539de4101` over the previous box build (the fork series) gave fresh
  times only to `il.c`, `lexical.c` and `overload.c` (`lexical.c` by a file-time effect: neither 0020 nor the series
  changes it), so ninja recompiled only those (`run.log`: steps 1–8 of 10).
- The link then failed: the stale `attribute.c.o` of the fork series references `set_attribute_expr_substitution` and
  `set_attribute_expr_lambda_deferral` (series M), which 0020's `il.c` alone does not define (`mold: error: undefined
  symbol`, "ninja: build stopped: subcommand failed").
- `edg-docker-test` went on ("Running tests...") with the binaries of the previous box build, and the run exited 0.
  That build (run directory `2026.10.16-22.50.50`) started 24 s before the last amend of `0020` (`999856d488` →
  `c539de4101`, 22:51:14), which added only the `template`-keyword rule (`G::template T<int>`, `.template T<int>()`)
  and the test's cases f5 and f6. So the stale binaries carried the previous revision of 0020 inside the fork chain:
  every other 0020 test and the imported expectations it changes passed, and exactly f5 and f6 failed ("the first
  f5 is viable and better … the final comparison was 8 == 1", `build/s1/0020d/x64/cases/1/1/change.diff`).
- **Confirmation:** run `0020e` (2026-10-21) at `c539de4101` with `build/gcc` removed first: a full build (660
  steps), then `changes` matches the expectations in `edg_x86_64` and in the `edg_x86_64_cp` overlay (91 expected
  non-PASS, 91 got, 0 unexpected). The box runner `~/edg-cerastes-runs/run.sh` now removes `build/gcc` before every
  run and reports `BUILD FAILED` when ninja stops. Evidence copied to `build/s1/0020d/` (gitignored).

Branch `0020` is ready for the owner to file upstream as it is.

## Impact on cerastes

Lambdas in `[[cerastes::…]]` contracts of function templates (the `forall` form over arrays of `T`) could not
name `T`: M2 documented it as a limitation (decision 0006 item 9). With this fix merged and the series commits L
and M, they can, in function templates and in member templates of class templates; the forms that still cannot
work fail closed.
