// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0050 item 6 (question 130): the four contract-less vlibc_ declarations in vlibc's <stdlib.h> change nothing for a program that includes the shim <stdlib.h> and calls malloc and free only (they are skipped unless called)
// Pre-registered from record 0050 item 6: verifies (the same Viper as before the declarations)
#include <stdlib.h>
#include <cerastes.h>

int roundtrip(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  int v = *p;
  free(p);
  return v;
}
