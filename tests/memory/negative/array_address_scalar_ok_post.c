// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/memory/array_address_scalar_ok.c
// Pins: 0064 item 5: null_pointer promises 1 for NULL where the body returns 0; the other two functions are the positive's
// Pre-registered from record 0064 item 5 (Part R), before any run: postcondition at the return of the NULL path
#include <cerastes.h>
#include <stddef.h>

int null_pointer(int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(r, (p == NULL || cerastes_owns(*p)) && r == (p == NULL ? 1 : *p))
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
