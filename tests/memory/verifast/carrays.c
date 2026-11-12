// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pre-registered from record 0060 item 1 (the port of the plan's section 8), checked against the frozen Commit 3 build: verifies
// Port of VeriFast's examples/carrays.c (~/Documents/verifast at 0fb63506) to cerastes (record 0060, M4.5e1b).
// Adaptation: `//@ requires a[0..3] |-> ?vs` / `ensures a[0..3] |-> vs` is `cerastes_owns_range(a, 3)` in both (the contents `vs` are not named: a list of contents is M4.5e1c's); `array_test2`'s ghost count `?n` is a parameter `n` (`a[..?n] |-> ?vs &*& 1 < n` is `owns_range(a, n) && 1 < n`) and `ensures a[..n] |-> update(0, 10, vs)` is `a[0] == 10 && a[1] == cerastes_old(a[1])` (the two facts the function's own `assert`s check); `assert` is CERASTES_CHECK, and the locals that only an annotation reads (or that nothing reads) are written back to the element they came from (a `(void)` cast does not count as a use for EDG's C++ mode, which warns #550 on the file, and the ordinary compilers warn on an unused local: the file is `native-ok` and prints no warning); `to_verify`'s `arr[..10] |-> {0, 10, 20, 30, ...}` is `owns_range(arr, 10) && arr[3] == 30` before and `arr[3] == 31 && r == 30` after (the other nine elements are e1c's list).
// Left out: the predicate `p` (M4.5f1) and the two failing functions, which are the twins tests/memory/negative/carrays_vf_fail.c and carrays_vf_fail2.c.
// Line map (this file : VeriFast's):  10 : 1   11 : 2   12 : 3   14 : 5   15 : 6   16 : 7   22 : 10   23 : 11   24 : 12   27 : 14   28 : 15   29 : 16   30 : 17   31 : 18   32 : 19   38 : 22   39 : 23   40 : 24   42 : 26
#include <cerastes.h>

void array_test1(int *a)
    CERASTES_REQUIRES(cerastes_owns_range(a, 3))
    CERASTES_ENSURES(cerastes_owns_range(a, 3))
{
  int tmp0 = a[0];
  int tmp1 = a[1];
  int tmp2 = a[2];
  a[0] = tmp0;
  a[1] = tmp1;
  a[2] = tmp2;
}

void array_test2(int *a, int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 1 < n)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[0] == 10 && a[1] == cerastes_old(a[1]))
{
  (void)n;
  int a1_old = a[1];
  a[0] = 10;
  int tmp = a[0];
  CERASTES_CHECK(tmp == 10);
  int a1 = a[1];
  CERASTES_CHECK(a1_old == a1);
  a[0] = tmp;
  a[1] = a1_old;
  a[1] = a1;
}

int to_verify(int *arr)
    CERASTES_REQUIRES(cerastes_owns_range(arr, 10) && arr[3] == 30)
    CERASTES_ENSURES(r, cerastes_owns_range(arr, 10) && arr[3] == 31 && r == 30)
{
  return arr[3]++;
}
