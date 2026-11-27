// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 9 and 10 (audit E3a finding 8 (f); the auditor's x09, x09b, l07b; held_owned is a variant of x09 under owns(g), a new program): a loop invariant may read g[1].a[2] of a global array of structures with an array member, in a function that holds the array (reads(g) or owns(g)) and in main (where the program_start values are known), and a loop that writes nothing keeps the global's cells
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the controls of negative/array_rows_struct_elem_loop_global_wrong.c: the same loops with a claim that holds, and the same main without the write); time it first
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

static struct P g[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};

int held_reads(void)
    CERASTES_REQUIRES(cerastes_reads(g) && g[1].a[2] == 8)
    CERASTES_ENSURES(r, cerastes_reads(g) && r == 8)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && g[1].a[2] == 8)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return g[1].a[2];
}

int held_owned(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[1].a[2] == 8)
    CERASTES_ENSURES(r, cerastes_owns(g) && r == 8)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && g[1].a[2] == 8)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return g[1].a[2];
}

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && g[1].a[2] == 8)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  CERASTES_CHECK(g[0].a[0] == 2 && g[1].a[2] == 8 && g[1].n == 5);
  return g[1].a[2] - 8;
}
