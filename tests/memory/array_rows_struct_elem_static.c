// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 9 (audit E3a finding 8 (b); the auditor's l08c, l08d): a static local array of structures with an array member is known in main (the listed element's values, the rest zero) and written and read back in a function, where nothing is known about it on entry
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the right-value sibling of negative/array_rows_struct_elem_static_wrong.c: main's check of s[2].a[1] == 0; the checks outside main are in the twin and fail by design, the control being this main)
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

int write_read(void)
    CERASTES_ENSURES(r, r == 9)
{
  static struct P s[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  s[1].a[2] = 9;
  return s[1].a[2];
}

int main(void)
{
  static struct P s[3] = {{1, {2, 3, 4}}};
  CERASTES_CHECK(s[0].n == 1 && s[0].a[2] == 4 && s[2].a[1] == 0);
  return s[0].n - 1;
}
