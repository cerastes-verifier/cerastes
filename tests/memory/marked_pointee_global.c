// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (question 202; flip of reject_owning_field_marked_global.c): ref_to_uninit on an owning global pointer to a structure with an owning field is accepted; the program is the declaration and a function that does not touch it
// Pre-registered from record 0058 item 4 (question 202): verifies in both modes (observed on the Part F build with question 202 applied)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *gp CERASTES_OWNING CERASTES_REF_TO_UNINIT;

int f(void)
    CERASTES_ENSURES(r, r == 1)
{
  return 1;
}

int main(void)
{
  return 0;
}
