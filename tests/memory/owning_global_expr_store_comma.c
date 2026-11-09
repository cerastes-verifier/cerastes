// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 ("Stores ... including one inside an expression") and audit round 2 (Parts C and D), probe g (comma): a block p holds whole is handed to the owning global inside a comma expression (x = (g = p, 0)); main ends with the global holding it
// Pre-registered from derived from the record (not observed): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  *p = 1;
  int x = (g = p, 0);
  return x + (g == NULL);
}
