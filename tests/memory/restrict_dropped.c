// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 19 (question 326): the functions of restrict_owned.c and range_restrict.c with the keyword dropped, as the rejection's message tells a user to write them: a getter and a cell copy that own their pointees, a range setter unguarded and guarded, and two ranges. Dropping restrict removes only undefined behaviour, so every contract verifies as it did
// Pre-registered from record 0064 item 19 (Part R), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

int get(int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(r, (p == NULL || cerastes_owns(*p)) && r == (p == NULL ? 0 : *p))
{
  if (p == NULL) {
    return 0;
  }
  return *p;
}

void copy_cell(int *dst, int *src)
    CERASTES_REQUIRES(cerastes_owns(*dst) && cerastes_owns(*src))
    CERASTES_ENSURES(cerastes_owns(*dst) && cerastes_owns(*src) && *dst == *src && *src == cerastes_old(*src))
{
  *dst = *src;
}

void set0(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == 0)
{
  (void)n;
  p[0] = 0;
}

void set0_guarded(int *p, int n)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_range(p, n))
    CERASTES_ENSURES(p == NULL || cerastes_owns_range(p, n))
{
  if (p != NULL && n > 0) {
    p[0] = 0;
  }
}

void two(int *dst, int *src, int n)
    CERASTES_REQUIRES(cerastes_owns_range(dst, n) && cerastes_owns_range(src, n))
    CERASTES_ENSURES(cerastes_owns_range(dst, n) && cerastes_owns_range(src, n))
{
  (void)dst;
  (void)src;
  (void)n;
}
