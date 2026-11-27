// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (audit E3a finding 8 (a); the auditor's c04 and d08): p[i] = p[j] under owns_range(p, n) copies the element's cells (the old values of the source, the first element kept), the same at symbolic indexes, and *d = s[1] reads its source under reads_range(s, 2)
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the right-value siblings of the wrong twins in negative/array_rows_struct_elem_copy_range_wrong.c: cp's p[1].a[1] == old(p[0].a[1]), cpsym's old(p[j].a[2]), from_reads' s[1].a[2])
#include <cerastes.h>

struct W {
  int a[3];
  int n;
};

void cp(struct W *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n >= 2)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[1].a[0] == cerastes_old(p[0].a[0]) && p[1].a[1] == cerastes_old(p[0].a[1]) && p[1].a[2] == cerastes_old(p[0].a[2]) && p[1].n == cerastes_old(p[0].n) && p[0].a[1] == cerastes_old(p[0].a[1]))
{
  (void)n;
  p[1] = p[0];
}

void cpsym(struct W *p, int n, int i, int j)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n && 0 <= j && j < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[i].a[2] == cerastes_old(p[j].a[2]) && p[i].n == cerastes_old(p[j].n))
{
  (void)n;
  p[i] = p[j];
}

void from_reads(struct W *d, struct W *s)
    CERASTES_REQUIRES(cerastes_owns(*d) && cerastes_reads_range(s, 2))
    CERASTES_ENSURES(cerastes_owns(*d) && cerastes_reads_range(s, 2) && d->a[2] == s[1].a[2] && d->n == s[1].n)
{
  *d = s[1];
}
