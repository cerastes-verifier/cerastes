// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 6 ("Stores ... by name") and audit round 2 (Parts C and D), probe g3: an address-taken owner that receives malloc's result inside a condition and frees it through the alias
// Pre-registered from derived from audit probe g3 (the program without the first allocation; verifies): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_OWNING int *v = NULL;
  int **q = &v;
  if ((v = (int *)malloc(sizeof(int))) == NULL) {
    return 1;
  }
  free(*q);
  return 0;
}

int main(void)
{
  return f();
}
