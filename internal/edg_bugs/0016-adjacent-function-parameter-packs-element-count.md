# 0016 — With two adjacent function parameter packs, a reference to one pack counts the elements of the other

- **Labels:** `bug`
- **Found:** 0003 fix (2026-10-15). Reproduced on upstream `d6f83174c7` the same day;
  no `enable_if` is involved
- **Status:** fixed. Branch `0016` in the fork clone is commit `46d897f6b0` on upstream
  `main` `d6f83174c7`; merged into the fork's `cerastes` as `b1403a3ae7`
- **Upstream issue:** not filed

## Summary

When a function template is instantiated, a reference to a function parameter pack
that is made while the instance's declaration is scanned (after the parameter list:
in a trailing return type or an exception specification) is expanded over the
elements of the pack. `find_parameter_for_pack` (`src/scope_stk.c`, line 12131)
finds the pack's first element and counts the elements with

```c
  for (; param_id != NULL && param_id->is_pack_element;
       param_id = param_id->next) {
    (*elements)++;
  }
```

It does not check that the elements belong to the same pack. If the next function
parameter pack follows immediately (`Ts... as, Us... bs`), its elements are counted as
elements of the first one. A fold over both packs then fails with "pack "bs" does not
have the same number of elements as "as"", although they do.

Its sibling for the parameters of a function that is being defined,
`find_variable_for_pack` (line 12213), stops counting at the first parameter with
another number; this one does not.
The function is called when a declaration is scanned in an instantiation
(`scope_stk.c`, line 12820, "When scanning from tokens, we look in the enclosing
function prototype scopes").

## Reproducer

```c++
// A fold over two adjacent function parameter packs in a trailing return type.
template <class... Ts, class... Us>
auto f(Ts... as, Us... bs) -> decltype(((as + bs) + ... + 0));
int x = f<int, int>(1, 2, 3, 4);
```

```c++
// The same in a noexcept specification.
template <class... Ts, class... Us>
void g(Ts... as, Us... bs) noexcept(noexcept(((as + bs), ...)));
void h() { g<int, int>(1, 2, 3, 4); }
```

```sh
cpfe --clang_version=210000 --c++17 --no_wrap_diagnostics adj1.cpp   # and adj2.cpp
```

**Actual:**

```
"adj1.cpp", line 3: error: pack "bs" does not have the same number of elements as "as"
  auto f(Ts... as, Us... bs) -> decltype(((as + bs) + ... + 0));
                                                ^
          detected during instantiation of "f" based on template arguments <int, int, int, int> at line 4
```

```
"adj2.cpp", line 3: error: pack "bs" does not have the same number of elements as "as"
  void g(Ts... as, Us... bs) noexcept(noexcept(((as + bs), ...)));
                                                      ^
          detected during instantiation of "void g(Ts..., Us...) [with Ts=<int, int>, Us=<int, int>]" at line 4
```

**Expected:** no diagnostics. Apple clang 21 (`clang++ -std=c++17 -fsyntax-only`) and
g++ 16 accept both files.

## Confirmation

- Upstream `d6f83174c7`, native `macos-arm-clang-debug` build.
- A single pack is unaffected (`auto f(Ts... as) -> decltype(((as + as) + ... + 0))`
  with `f<int, int>(1, 2)` is accepted), and so is `sizeof...(as)` of two adjacent
  packs, which is computed from the template arguments, not from the parameter list.
- With the change below, in a native build, both files are accepted.
- The probe files are in `.tmp/agent-bugs/analysis/new/adj/` (`a1.cpp` to `a4.cpp`),
  and the implementer's in `.tmp/agent-bugs/0003/probes/` (`g01_two_packs.cpp`,
  `g02_decl.cpp`, `g03_trailing_return.cpp`).

## Fix

Branch `0016`, commit `46d897f6b0` "Count only the elements of a function
parameter pack that belong to the pack.":
- `find_parameter_for_pack` counts only the elements that have the pack's
  number (`param_id->param_num == param_num` in the loop condition). A
  pack's elements are contiguous and share one number, so the count stops at
  the end of the pack. An empty pack counts 0, and a non-pack parameter
  between two packs already stopped the old loop.
- The other loops over pack elements in `scope_stk.c` and `templates.c`
  already check the number or are delimited by start-of-pack arguments. The
  back ends' name discriminators count across packs but include the
  parameter's name, so they cannot collide (review).
- Tests:
  - `tests/tests/changes/GH-adjacent-packs.sft.cpp` (C++11 to 23, accepted by
    clang 21 and g++ 16): a trailing return type and a `noexcept` naming two
    adjacent packs, empty, middle and member cases, folds from C++17, a class
    pack in a row with a member template's function pack, and three packs in
    a row.
  - `GH-adjacent-packs-mismatch.sft.cpp`: packs of different lengths are
    still an error.
  Both fail before and pass after, natively with `edgy` and in Docker
  (`edg_x86_64`, `edg_x86_64_cp`).
- Docker `changes,regressions` has no mismatch in either configuration.
  `src/Changes` has an entry. Reviewed by the Opus auditor (approved).

A member template using a class pack and two function packs in one expansion
is accepted by g++ 16 and rejected by clang 21, so the test leaves it out.
With an empty class pack, the same shapes fail for another reason
([0018](0018-empty-class-pack-and-member-pack-in-one-expansion.md)).

## Impact on cerastes

None known. cerastes does not instantiate such declarations, and contracts that
name a function parameter pack are rejected by the fork series (commit B).
