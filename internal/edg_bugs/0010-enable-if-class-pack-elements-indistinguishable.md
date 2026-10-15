# 0010 — Elements of a class template parameter pack named in the `enable_if` of a member function are indistinguishable

- **Labels:** `bug`
- **Found:** analysis of 0003 and 0004 (2026-10-15); reproduced on upstream
  `d6f83174c7` the same day
- **Status:** open
- **Upstream issue:** not filed

## Summary

A member function of a class template can have a pack of the *class* in its
parameter list. In an instance of the class the pack is expanded, and an
`enable_if` condition that names the pack elements is expanded with it:
`((cs > 0) && ...)` becomes one comparison per element. Every `enk_param_ref`
of the expansion carries the same parameter number, the number of the pack,
because the IL numbers the parameters of an instance like the parameters of
the template, with all elements of a pack sharing one number. The condition is
evaluated from a table of argument values indexed by parameter number, so all
elements read the same argument (the first). The call is accepted or rejected
by its first argument alone, with no diagnostic.

This is the same family as 0003 and 0004, which concern the parameter packs of
a function template. It is separate because the expansion happens when the
class is instantiated, the condition is not substituted at overload time, and
the expanded nodes do not say which element they name, so it cannot be repaired
where the condition is evaluated.

## Reproducer

```c++
// A member function of a class template, with a class template parameter pack
// in its parameter list, names the elements of the pack in an enable_if condition.
template <class... Cs>
struct C {
  int k(Cs... cs) __attribute__((enable_if(((cs > 0) && ...), "all positive"))) { return 1; }
};
C<int, int> c;
int a = c.k(1, 2);    // enabled
int b = c.k(1, -2);   // disabled: the second argument is not positive
int d = c.k(-1, 2);   // disabled: the first argument is not positive
```

```sh
cpfe --clang_version=210000 --c++17 --no_wrap_diagnostics classpack.cpp
```

**Actual:** line 9 is accepted. Line 10 is rejected, because the first
argument is the one that is read:

```
"classpack.cpp", line 10: error: function "C<Cs...>::k [with Cs=<int, int>]" cannot be called with the given argument list
            argument types are: (int, int)
            object type is: C<int, int>
  int d = c.k(-1, 2);   // disabled: the first argument is not positive
            ^

1 error detected in the compilation of "classpack.cpp".
```

**Expected:** lines 9 and 10 are rejected, as with clang:

```
classpack.cpp:9:11: error: no matching member function for call to 'k'
classpack.cpp:5:7: note: candidate disabled: all positive
classpack.cpp:10:11: error: no matching member function for call to 'k'
classpack.cpp:5:7: note: candidate disabled: all positive
2 errors generated.
```

## Confirmation

- Apple clang 21 (`clang++ -std=c++17 -fsyntax-only`) gives the expected result.
- `--il_display` of the file without line 10 shows the cause. The instance's
  `enable_if` expression holds two `enk_param_ref` nodes with the same number,
  and the parameter list has two parameters named `cs` with the same number:

  ```
  kind:                    enk_param_ref
  param_ref.param_num:     1               (twice, one per element)
  param-type  name "cs"  is_pack_element: TRUE  param_num: 1     (twice)
  ```

  `il_def.h` (line 6508) documents this numbering of the parameters of an
  instance of a variadic template: "there can be missing or repeated values in
  the parameter list of the instantiation".
- `sizeof...(cs)` in the same position is right (the expansion is a constant
  2): `int n(Cs... cs) enable_if(sizeof...(cs) == 2)` agrees with clang. Parameters
  that are not elements of the pack are also addressed correctly once bug 0004
  is fixed.
- The condition is read by `eval_enable_if_attributes` (`src/overload.c`, line
  5587) and `interpret_clang_enable_if_opnd` (`src/interpret.c`, line 33710): the
  constants are stored by position of the argument and read with
  `(*params)[param_num]` (`interpret.c`, line 32566), so with repeated numbers the
  first element of the pack is the one that is read.

## Suggested fix

The elements need distinct identities where the instance's condition is
created, which is where the instance declaration is scanned (the parameter
numbering of `declarator.c`, line 3406, and `make_param_ref_operand` in
`expr.c`, line 40111), or the node needs a field for the element's index. Both change
the documented numbering or the IL layout and are relied on by the pack machinery
in `scope_stk.c`, so it is not a small change. The alternative, until then, is
to make a reference to an element of a pack of the class in an `enable_if`
condition an error, as it is evaluated wrongly now.

## Impact on cerastes

None known. cerastes does not use `enable_if`, and its own attributes are
registered by the driver and do not go through this path.
