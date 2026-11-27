// expect: error check @ L18
// expect: error check @ L25
// expect: error check @ L32
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_static.c
// Pins: 0064 item 9 (audit E3a finding 8 (b); the auditor's l08, l08b, l08e): a static local outside main holds an unknown value on entry (the initializer says what it starts as, but the function may be called after another call changed it), so a check of the listed value fails, and in main the element the list does not reach is zero, so a check of 4 fails
// Pre-registered (test-mutator-10, Task 1/2, before any run): three `check` rows (both modes); controls: main in tests/memory/array_rows_struct_elem_static.c checks the same objects (s[2].a[1] == 0 verifies there); the first two are unknown by design
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

int unknown_scalar(void)
{
  static struct P s[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  CERASTES_CHECK(s[0].n == 1);  // <- here
  return s[0].n;
}

int unknown_cell(void)
{
  static struct P s[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  CERASTES_CHECK(s[1].a[2] == 8);  // <- here
  return s[1].a[2];
}

int main(void)
{
  static struct P s[3] = {{1, {2, 3, 4}}};
  CERASTES_CHECK(s[2].a[1] == 4);  // <- here
  return s[0].n - 1;
}
