// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 9 (audit E3a finding 8 (e); the auditor's y01 and z05 programs): the program_start group of a static array of structures with two array members and a scalar between them (struct Q { int a[2]; int n; int b[2]; }): a list that sets part of the first element, and an array with no initializer: every listed cell has its value, every other cell of every element is zero
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the static control of negative/array_rows_struct_elem_start_unknown.c: g4 is the same type with internal linkage); time it first
#include <cerastes.h>

struct Q {
  int a[2];
  int n;
  int b[2];
};

static struct Q g3[3] = {{{1, 2}, 3, {4}}};
static struct Q g4[2];

int main(void)
{
  CERASTES_CHECK(g3[0].a[0] == 1 && g3[0].a[1] == 2 && g3[0].n == 3 && g3[0].b[0] == 4 && g3[0].b[1] == 0 && g3[1].a[0] == 0 && g3[1].b[1] == 0 && g3[2].n == 0 && g3[2].b[0] == 0);
  CERASTES_CHECK(g4[0].n == 0 && g4[1].a[0] == 0 && g4[1].b[1] == 0);
  return g3[2].n + g4[1].n;
}
