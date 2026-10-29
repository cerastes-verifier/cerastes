// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 item 5: live(p) implies p != NULL; a function that accepts NULL writes p == NULL || live(p); p == NULL is never checked for pointer-comparison
#include <cerastes.h>
#include <stddef.h>

int is_null(int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_live(p))
    CERASTES_ENSURES(r, (p == NULL || cerastes_live(p)) && r == (p == NULL))
{
  return p == NULL;
}

int use_is_null(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 1;
  int a = is_null(&x);
  int b = is_null(NULL);
  return a + b;
}
