// expect: rejected cerastes-unsupported_annotation @ L23
// expect: rejected cerastes-unsupported_annotation @ L31
// expect: rejected cerastes-unsupported_annotation @ L42
// expect: rejected cerastes-unsupported_annotation @ L50
// expect: rejected cerastes-unsupported_annotation @ L57
// expect: rejected cerastes-unsupported_annotation @ L67
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_marker_controls.c
// Pins: 0064 item 13 and question 383 (audit E3a finding 2 (b); edg-backend report 17 section C, probes pC/c_arr_*.c): ref_to_uninit and must_init on a parameter that has an owns_range or reads_range atom over structures whose element has an array member are rejected at the parameter's line (`unsupported_annotation`: a marker over such a range, M4.5e2): ref_to_uninit and must_init with owns_range, with an int result, with reads_range, beside a marked range of another structure (only the first parameter's row), and the synthesised range atom of owning(n); the controls that stay accepted are in tests/memory/array_rows_struct_elem_marker_controls.c (a marker over a range of structures without an array member, and Part D's marker on a pointer to ONE structure with an array member)
// Pre-registered (test-mutator-10, Task 1/2, before any run): rows registered from report 17 section C's table (`unsupported_annotation` at the parameter's line, six functions, both modes); the controls verify in tests/memory/array_rows_struct_elem_marker_controls.c
#include <cerastes.h>

struct Q {
  int x;
  int y;
};

struct P {
  int n;
  int a[3];
};

void touch(CERASTES_REF_TO_UNINIT struct P *p, int n)  // <- here
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n == 1)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)n;
  p[0].a[1] = 1;
}

void fill(CERASTES_MUST_INIT struct P *p, int n)  // <- here
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n == 1)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)n;
  p[0].n = 1;
  p[0].a[0] = 1;
  p[0].a[1] = 1;
  p[0].a[2] = 1;
}

int touch_result(CERASTES_REF_TO_UNINIT struct P *p, int n)  // <- here
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n == 1)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n))
{
  (void)n;
  return p[0].a[0] - p[0].a[0];
}

void touch_reads(CERASTES_REF_TO_UNINIT struct P *p, int n)  // <- here
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n == 1)
    CERASTES_ENSURES(cerastes_reads_range(p, n))
{
  (void)n;
}

void touch_two(CERASTES_REF_TO_UNINIT struct P *p, int n, CERASTES_MUST_INIT struct Q *q, int m)  // <- here
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m) && n == 1 && m == 1)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns_range(q, m))
{
  (void)n;
  (void)m;
  q[0].x = 1;
  q[0].y = 2;
}

void touch_owning(int n, CERASTES_OWNING_N(n) CERASTES_REF_TO_UNINIT struct P *p)  // <- here
    CERASTES_REQUIRES(n == 1)
{
  (void)n;
  (void)p;
}
