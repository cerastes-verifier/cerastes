// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3, 9 and 10 (audit E3a finding 8 (f); the auditor's l00, l04, l05c, l06b): a loop that writes nothing keeps the cells of a local array of structures with an array member (and a loop that calls a function holding only reads_range of the array), an invariant may read v[1].a[i] with a symbolic index and v[0].a[2], and a loop over a parameter's range that does not write keeps p[0].a[0]
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the controls of the twins in negative/array_rows_struct_elem_loop_wrong.c: reach is the program of loop_writes_through_a_pointer, loop_writes_elements and loop_calls_a_writer minus their write; invariant_reads that of the invariant rows); time it first
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

int rdv(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == 0);

int reach(void)
    CERASTES_ENSURES(r, r == 8)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = i + 1;
  }
  return v[1].a[2];
}

int reads_call(void)
    CERASTES_ENSURES(r, r == 8)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = i + 1 + rdv(v, 2);
  }
  return v[1].a[2];
}

int invariant_reads(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && v[1].a[i] >= 6 && v[0].a[2] == 4)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return v[0].a[2] - 4;
}

void range_kept(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0].a[0] == cerastes_old(p[0].a[0]))
{
  (void)p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i = i + 1;
  }
}
