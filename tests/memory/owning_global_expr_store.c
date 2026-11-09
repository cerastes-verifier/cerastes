// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 ("Stores ... including one inside an expression") and audit round 2 (Parts C and D), probe g2: an owning global that receives malloc's result inside a condition
// Pre-registered from audit probe g2 (verifies today): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

int main(void)
{
  if ((g = (int *)malloc(sizeof(int))) == NULL) {
    return 1;
  }
  *g = 1;
  return 0;
}
