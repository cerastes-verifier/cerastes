// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 5 (question 322): the forms next to the rejected ones (negative/array_address_scalar_reject.c) stay accepted: a null test of a pointer variable, an address formed by an index compared with another pointer (not with NULL), an element read compared with 0
// Pre-registered from record 0064 item 5 (Part R), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

int null_pointer(int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(r, (p == NULL || cerastes_owns(*p)) && r == (p == NULL ? 0 : *p))
{
  if (p == NULL) {
    return 0;
  }
  return *p;
}

int same_element(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[4] = {5, 6, 7, 8};
  int *q = &a[1];
  return &a[1] == q;
}

int zero_element(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[4] = {5, 0, 7, 0};
  return a[1] == 0;
}
