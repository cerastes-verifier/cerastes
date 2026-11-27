// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 9 (audit E3a finding 8 (e); the auditor's z01 and z03 programs; the structure Q of the earlier form of this file is in array_rows_struct_elem_start_two_members.c, because the Viper lint refuses a unit that declares both structures, audit finding 2 (e)): the program_start group of a static array of structures with an array member: a list that stops after element 0, a partial list in an element's array member: every listed cell has its value, every cell the list does not reach is zero, the same at the first and last element
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the right-value siblings of the wrong twins: negative/array_rows_struct_elem_start_beyond.c (g1) and _start_partial.c (g2)); time it first; the first form of the file (four globals of two structures) verified in lane edg and its Viper was refused by the lint (the table collision), so it was split before any harness run
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

static struct P g1[3] = {{7, {1, 2, 3}}};
static struct P g2[3] = {{7, {1}}, {8, {4, 5}}};

int main(void)
{
  CERASTES_CHECK(g1[0].n == 7 && g1[0].a[0] == 1 && g1[0].a[1] == 2 && g1[0].a[2] == 3 && g1[1].n == 0 && g1[1].a[0] == 0 && g1[1].a[2] == 0 && g1[2].n == 0 && g1[2].a[1] == 0);
  CERASTES_CHECK(g2[0].n == 7 && g2[0].a[0] == 1 && g2[0].a[1] == 0 && g2[0].a[2] == 0 && g2[1].n == 8 && g2[1].a[0] == 4 && g2[1].a[1] == 5 && g2[1].a[2] == 0 && g2[2].n == 0 && g2[2].a[1] == 0);
  return (g1[0].n - 7) + g2[2].n;
}
