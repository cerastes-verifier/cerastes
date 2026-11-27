// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (audit E3a finding 8 (a); the auditor's k04, k05, k06, k07, q29, q29b, c03's ok): the copy between two named arrays, from an element to a local structure, from a local structure into an element, through a pointer that points at an element (*p = v[1] with p == &v[0] and with p == &v[1]), and from an element whose cells were all written one by one: the copy is exact whichever object the other side is
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the right-value siblings of the wrong twins in negative/array_rows_struct_elem_copy_between_wrong.c and the control of the uninitialized twins in negative/array_rows_struct_elem_copy_uninit.c: copy_set); time it first
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

int between(void)
    CERASTES_ENSURES(r, r == 13)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  struct P w[1] = {{0, {0, 0, 0}}};
  w[0] = v[1];
  return w[0].a[2] + w[0].n;
}

int by_pointer(void)
    CERASTES_ENSURES(r, r == 8)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  struct P *p = &v[0];
  *p = v[1];
  return v[0].a[2];
}

int alias_self(void)
    CERASTES_ENSURES(r, r == 8)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  struct P *p = &v[1];
  *p = v[1];
  v[1] = *p;
  return v[1].a[2];
}

int from_value(void)
    CERASTES_ENSURES(r, r == 9)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  struct P t = {0, {7, 8, 9}};
  v[0] = t;
  return v[0].a[2];
}

int to_local(void)
    CERASTES_ENSURES(r, r == 8)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  struct P t = v[1];
  return t.a[2];
}

int copy_set(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct P v[2];
  v[0].n = 1;
  v[0].a[0] = 1;
  v[0].a[1] = 2;
  v[0].a[2] = 3;
  v[1] = v[0];
  return v[1].a[2];
}
