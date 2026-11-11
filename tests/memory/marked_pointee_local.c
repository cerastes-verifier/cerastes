// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (question 202; read_variable_markers' rejection goes): a marked local pointer to a structure with an owning field is accepted: the marker documents intent (0052 item 4) and closes nothing
// Pre-registered from record 0058 item 4 (question 202): verifies in both modes (observed on the Part F build with question 202 applied)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct buf s = { NULL, 1 };
  CERASTES_REF_TO_UNINIT struct buf *q = &s;
  q->n = 0;
  return s.n;
}
