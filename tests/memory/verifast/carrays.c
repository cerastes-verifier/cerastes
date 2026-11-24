// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pre-registered from record 0061 item 8 (the port, Flips C), before the code; it replaces record 0060 item 1's version of this file (same expectations: verified, native-ok, both modes): the contents of the arrays that VeriFast names with `vs` are now stated in full with cerastes_forall
// Port of VeriFast's examples/carrays.c (~/Documents/verifast at 0fb63506) to cerastes (records 0060, M4.5e1b, and 0061, M4.5e1c).
// Adaptation: `//@ requires a[0..3] |-> ?vs` / `ensures a[0..3] |-> vs` is `owns_range(a, 3)` in both and, for the contents, `cerastes_forall(kept, a, 3)`: every element keeps its entry value (`cerastes_old(a[i])` under the binder) (`kept` is on line 10 so that the line map stays); `array_test2`'s ghost count `?n` is a parameter `n` (`a[..?n] |-> ?vs &*& 1 < n` is `owns_range(a, n) && 1 < n`) and `ensures a[..n] |-> update(0, 10, vs)` is `a[0] == 10` and `cerastes_forall(kept_but_first, a, n)`: every other element keeps its entry value; `assert` is CERASTES_CHECK, and the locals that only an annotation reads (or that nothing reads) are written back to the element they came from (a `(void)` cast does not count as a use for EDG's C++ mode, which warns #550 on the file, and the ordinary compilers warn on an unused local: the file is `native-ok` and prints no warning); `to_verify`'s `arr[..10] |-> {0, 10, 20, 30, ...}` is `owns_range(arr, 10) && cerastes_forall(ten_times, arr, 10)` (`arr[i] == 10 * i` for every i) before and `arr[3] == 31 && r == 30 && cerastes_forall(ten_times_but_3, arr, 10)` after (every i except 3 still holds `10 * i`).
// Left out: the predicate `p` (M4.5f1) and the two failing functions, which are the twins tests/memory/negative/carrays_vf_fail.c and carrays_vf_fail2.c. Ported in full now: every element's value of the three functions (lists of contents are quantified, not named).
// Line map (this file : VeriFast's):  10 : 1   11 : 2   12 : 3   14 : 5   15 : 6   16 : 7   22 : 10   23 : 11   24 : 12   27 : 14   28 : 15   29 : 16   30 : 17   31 : 18   32 : 19   38 : 22   39 : 23   40 : 24   42 : 26
#include <cerastes.h>
CERASTES_PURE bool kept(int i, const int *a, int n) { return !(0 <= i && i < n) || a[i] == cerastes_old(a[i]); }
void array_test1(int *a)
    CERASTES_REQUIRES(cerastes_owns_range(a, 3))
    CERASTES_ENSURES(cerastes_owns_range(a, 3) && cerastes_forall(kept, a, 3))
{
  int tmp0 = a[0];
  int tmp1 = a[1];
  int tmp2 = a[2];
  a[0] = tmp0;
  a[1] = tmp1;
  a[2] = tmp2;
}
CERASTES_PURE bool kept_but_first(int i, const int *a, int n) { return !(0 <= i && i < n && i != 0) || a[i] == cerastes_old(a[i]); }
void array_test2(int *a, int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 1 < n)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[0] == 10 && cerastes_forall(kept_but_first, a, n))
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
CERASTES_PURE bool ten_times(int i, const int *a, int n) { return !(0 <= i && i < n) || a[i] == 10 * i; } CERASTES_PURE bool ten_times_but_3(int i, const int *a, int n) { return !(0 <= i && i < n && i != 3) || a[i] == 10 * i; }
int to_verify(int *arr)
    CERASTES_REQUIRES(cerastes_owns_range(arr, 10) && cerastes_forall(ten_times, arr, 10))
    CERASTES_ENSURES(r, cerastes_owns_range(arr, 10) && arr[3] == 31 && r == 30 && cerastes_forall(ten_times_but_3, arr, 10))
{
  return arr[3]++;
}
