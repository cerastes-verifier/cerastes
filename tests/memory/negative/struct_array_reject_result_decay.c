// expect: rejected cerastes-unsupported_expression @ L22
// mode: c23, c++20
// twin: tests/memory/struct_array_value_elem.c
// Pins: 0063 item 13 (M4.5f3: M4.5e1e2 built the value view of a structure whose member is an array of structures, so this row is M4.5f3's alone): the array member of a function's value decays to a pointer into a temporary: rejected ("a struct place this version cannot read"); mk().a[i] is accepted
// Pre-registered from record 0063 item 13 (Part E), before any run: one rejection at the decay (the tag is the best reading, unsupported_expression; confirm at the first run)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

struct S mk(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  return s;
}

int first(void)
{
  int *p = mk().a;  // <- here
  return p == NULL;
}
