// expect: error invariant-entry @ L22
// expect: error invariant-preserved @ L35
// expect: error check @ L53
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_loop_global.c
// Pins: 0064 items 9 and 10 (audit E3a finding 8 (f); the auditor's x09w, x09d (in a function holding owns(g): a variant, a new program) and l07, measured `invariant-entry`, `invariant-preserved` and `check`, both modes): an invariant claiming g[1].a[2] == 9 does not hold on entry, one claiming 8 is not kept by a body that writes g[i].a[2], and in main a loop that writes g[i].a[0] with no invariant about g loses g[0].a[0] (the check after it fails: unknown by design; the same main without the write verifies)
// Pre-registered (test-mutator-10, Task 1/2, before any run): invariant-entry and invariant-preserved at the invariant lines, one `check` in main; controls: held_reads, held_owned and main in tests/memory/array_rows_struct_elem_loop_global.c
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

static struct P g[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};

int entry_wrong(void)
    CERASTES_REQUIRES(cerastes_reads(g) && g[1].a[2] == 8)
    CERASTES_ENSURES(r, cerastes_reads(g) && r == 8)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && g[1].a[2] == 9)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return g[1].a[2];
}

int not_kept(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[1].a[2] == 8)
    CERASTES_ENSURES(r, cerastes_owns(g))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && g[1].a[2] == 8)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    g[i].a[2] = 0;
    i = i + 1;
  }
  return 0;
}

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    g[i].a[0] = 1;
    i = i + 1;
  }
  CERASTES_CHECK(g[0].a[0] == 2 && g[1].a[2] == 8 && g[1].n == 5);  // <- here
  return g[1].a[2] - 8;
}
