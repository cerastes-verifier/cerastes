// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 5 (question 174): main stores a block into the owning leaf of a struct global by name and returns holding it: exempt, as an owning global is (0054 item 5), under the ghost held_g_data and the program_start tag extended with the member path
// Pre-registered from record 0055 item 5 (named leaves: new, no base form): verifies in both modes (frozen base with the field unmarked: leak at main's closing brace, as S40 (h) h_main_holds.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int *view;
};

static struct buf g = { NULL, NULL };

int main(void)
{
  g.data = (int *)malloc(sizeof(int));
  if (g.data != NULL) {
    *g.data = 1;
  }
  return 0;
}
