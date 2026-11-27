// expect: error uninitialized @ L25
// expect: error uninitialized @ L33
// expect: error uninitialized @ L43
// expect: error uninitialized @ L54
// expect: error uninitialized @ L66
// expect: error uninitialized @ L76
// expect: error uninitialized @ L88
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_copy_between.c
// Pins: 0064 item 12 and 0019 (audit E3a finding 8 (a); the auditor's c03 u1 to u5 and c02's u6 and u7, measured `uninitialized` at the copy, both modes): the source of a copy is read first, so a source whose array member or scalar leaf was not given a value, a source element that may be the unset one, and a copy of an element whose array cells were never written are `uninitialized` at the copy line
// Pre-registered (test-mutator-10, Task 1/2, before any run): `uninitialized` at the copy line of each function; control (the same copy from a fully written element verifies): copy_set in tests/memory/array_rows_struct_elem_copy_between.c
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

int no_cell(void)
{
  struct P v[2];
  v[0].n = 1;
  v[0].a[0] = 1;
  v[0].a[1] = 1;
  v[1] = v[0];  // <- here
  return 0;
}

int nothing(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && i == j)
{
  struct P v[2];
  v[i] = v[j];  // <- here
  return 0;
}

int no_scalar(void)
{
  struct P v[2];
  v[0].a[0] = 1;
  v[0].a[1] = 1;
  v[0].a[2] = 1;
  v[1] = v[0];  // <- here
  return 0;
}

int source_unset(void)
{
  struct P v[2];
  v[0].n = 1;
  v[0].a[0] = 1;
  v[0].a[1] = 1;
  v[0].a[2] = 1;
  v[0] = v[1];  // <- here
  return 0;
}

int by_index(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
{
  struct P v[2];
  v[0].n = 1;
  v[0].a[0] = 1;
  v[0].a[1] = 1;
  v[0].a[2] = 1;
  v[i] = v[i];  // <- here
  return 0;
}

int scalars_only(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && i == j)
{
  struct P v[2];
  v[0].n = 1;
  v[1].n = 2;
  v[i] = v[j];  // <- here
  return 0;
}

int one_element_set(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2)
{
  struct P v[2];
  v[0].n = 1;
  v[0].a[0] = 1;
  v[0].a[1] = 1;
  v[0].a[2] = 1;
  v[i] = v[j];  // <- here
  return 0;
}
