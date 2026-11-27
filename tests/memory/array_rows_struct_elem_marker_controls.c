// expect: verified
// expect: native-ok
// mode: c23, c++20
// twin-exempt: the controls of the rejected pin negative/array_rows_struct_elem_reject_markers.c: nothing in a function that only marks and writes can fail; the must-fail forms are that pin's rejections
// Pins: 0064 item 13 and question 383 (edg-backend report 17 section C, probes pC/c_plain_ref.c, c_plain_must.c, c_one_ref.c, c_one_must.c; moved here from controls appended to the existing range_struct_marker.c, whose Viper the joined proof must keep as the baseline's): the neighbours of the rejected markers of negative/array_rows_struct_elem_reject_markers.c that stay accepted: ref_to_uninit and must_init on a range of structures WITHOUT an array member (struct Q { int x; int y; }), and on a pointer to ONE structure with an array member (struct A { int n; int a[3]; }, owns(*p)), each used on a local
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the controls named by report 17 section C: all four verified there; this file runs them together)
#include <cerastes.h>

struct Q {
  int x;
  int y;
};

struct A {
  int n;
  int a[3];
};

void touch_range(CERASTES_REF_TO_UNINIT struct Q *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n == 1)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)n;
  p[0].x = 1;
}

void fill_range(CERASTES_MUST_INIT struct Q *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n == 1)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)n;
  p[0].x = 1;
  p[0].y = 2;
}

void touch_one(CERASTES_REF_TO_UNINIT struct A *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->a[1] = 1;
}

void fill_one(CERASTES_MUST_INIT struct A *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->n = 1;
  p->a[0] = 1;
  p->a[1] = 1;
  p->a[2] = 1;
}

int use(void)
{
  struct Q q[1] = {{1, 2}};
  struct A v = {1, {2, 3, 4}};
  struct A w;
  touch_range(q, 1);
  fill_one(&w);
  touch_one(&v);
  return (q[0].x - q[0].x) + (v.a[2] - v.a[2]) + (w.a[2] - w.a[2]);
}

int use_fill(void)
{
  struct Q q[1];
  fill_range(q, 1);
  return q[0].y - q[0].y;
}
