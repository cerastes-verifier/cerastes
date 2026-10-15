# 0018 — A trailing return type that expands an empty class pack together with a member template's own pack is rejected

- **Labels:** `bug`
- **Found:** 0016 review (2026-10-15). Reproduced on upstream `d6f83174c7` the
  same day; it does not involve `enable_if`
- **Status:** open
- **Upstream issue:** not filed

## Summary

A member function template of a class template can have, in its parameter list, a
pack of the class (`Cs... cs`) and a pack of its own (`Ts... ts`). If the trailing
return type has one pack expansion that names both (`(cs + ts)...`), and the class is
instantiated with an empty `Cs`, the instantiation of the class is rejected with
`identifier "cs" is undefined`. The identifier is declared: its parameter pack is
empty in this instance. The expansion has to stay, because `ts` is not known until
the member template is used (it must then also be empty), but the front end scans
the pattern with `cs` not bound to anything.

## Reproducer

```c++
int g(...) { return 0; }
template <class... Cs> struct S {
  template <class... Ts> auto m(Cs... cs, Ts... ts) -> decltype(g((cs + ts)...)) { return 0; }
};
S<> se;
```

```sh
cpfe --clang_version=210000 --c++17 --no_wrap_diagnostics emptycls.cpp
```

**Actual:**

```
"emptycls.cpp", line 3: error: identifier "cs" is undefined
    template <class... Ts> auto m(Cs... cs, Ts... ts) -> decltype(g((cs + ts)...)) { return 0; }
                                                                     ^
          detected during instantiation of class "S<Cs...> [with Cs=<>]" at line 5

1 error detected in the compilation of "emptycls.cpp".
```

**Expected:** no diagnostic. Apple clang 21 (`clang++ -std=c++17 -fsyntax-only`) and
g++ 16 accept the file. The result is the same with `--c++11` and `--c++20`.

## Confirmation

- Upstream `d6f83174c7`, native `macos-arm-clang-debug` build. The 0016 review reports
  it unchanged by the fix for bug 0016 (not re-run here).
- The conditions that matter, from variations of the file (all accepted by clang 21;
  the main file also by g++ 16):
  - one expansion that names *both* packs: `decltype(g((cs + ts)...))`,
    `decltype(g((cs, ts)...))`, and with the member pack first
    (`m(Ts... ts, Cs... cs)`): all rejected;
  - the class pack alone, `decltype(g(cs...))`: accepted, also `decltype(sizeof...(cs))`
    and a member template with only a type parameter; so the empty class pack is
    fine when the expansion is dropped altogether;
  - a non-empty class pack (`S<int>`, `S<int, int>`): accepted;
  - the same expansion in a `noexcept` specification, or in the body
    (`return g((cs + ts)...);`): accepted;
  - an empty pack in a free function template (`template <class... Ts> auto f(Ts... ts)
    -> decltype(g(ts...))` with `f()`): accepted.
- Where it happens, from a debugger run on the reproducer (the first
  `ec_undefined_identifier` is issued here): `scan_expr_full` (`expr.c:46214`), the
  argument of the call `g(...)` in `scan_decltype_operator` (`expr.c:18373`), the
  trailing return type, `scan_trailing_return_type` (`declarator.c:2562`), in the
  scan of the member template's declaration at the instantiation of the class
  (`class_member_template_declaration` and `template_declaration` above it).
- Mechanism, as far as it was followed: for an empty class pack the front end
  creates a dummy parameter entry (`declarator.c`, around line 3349: "An empty pack
  expansion: Create a dummy parameter id entry in case nested prototype
  instantiations refer to it") and drops the tokens of an expansion that has no
  elements (`skip_pack_expansion_tokens`, `scope_stk.c`). Both are right for an
  expansion over the empty pack only. For an expansion that also contains a pack that
  is not expanded in this instantiation (`ts`, a pack of the member template), the
  expansion is neither dropped nor given a binding for `cs`, and the identifier
  lookup in its pattern fails. I did not find the exact place that should keep `cs`
  visible.

## Suggested fix

In the instantiation of the class, an expansion whose packs are empty (class pack)
and unexpanded (member template pack) has to be kept as a dependent expansion of the
member template, with `cs` resolved to the empty pack (so that its length, 0, is
checked against `ts` when the member template is instantiated), or dropped
only if all its packs are known to be empty.

## Impact on cerastes

None known. cerastes does not instantiate such declarations, and contracts that name a
pack are rejected by the fork series (commit B).
