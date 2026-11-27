// expect: error postcondition @ L23
// expect: error postcondition @ L32
// expect: error postcondition @ L42
// expect: error postcondition @ L51
// expect: error postcondition @ L59
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_copy_between.c
// Pins: 0064 item 12 (audit E3a finding 8 (a); the auditor's k04, k05, k06, k07 programs, measured `postcondition`, both modes): each result states the cell the copy did not take (the right values 13, 8, 8, 9 and 8 are in the positive twin: between, by_pointer, alias_self, from_value, to_local)
// Pre-registered (test-mutator-10, Task 1/2, before any run): one `postcondition` at the return of each function; right-value siblings: the same functions in tests/memory/array_rows_struct_elem_copy_between.c
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

int wrong_between(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  struct P w[1] = {{0, {0, 0, 0}}};
  w[0] = v[1];
  return w[0].a[2] + w[0].n;  // <- here
}

int wrong_pointer(void)
    CERASTES_ENSURES(r, r == 4)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  struct P *p = &v[0];
  *p = v[1];
  return v[0].a[2];  // <- here
}

int wrong_alias(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  struct P *p = &v[1];
  *p = v[1];
  v[1] = *p;
  return v[1].a[2];  // <- here
}

int wrong_value(void)
    CERASTES_ENSURES(r, r == 4)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  struct P t = {0, {7, 8, 9}};
  v[0] = t;
  return v[0].a[2];  // <- here
}

int wrong_local(void)
    CERASTES_ENSURES(r, r == 4)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  struct P t = v[1];
  return t.a[2];  // <- here
}
