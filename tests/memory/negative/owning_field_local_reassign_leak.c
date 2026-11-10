// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owning_field_local_reassign.c
// Pins: 0055 item 5 (reassignment): the first block is not freed before the field takes a second one: "assigning to owning field `s.data` drops the block it holds", at the assignment (owning_local_overwrite.c)
// Pre-registered from record 0055 item 5 (named leaves: new, no base form): leak at the second store to s.data (the reassignment row); frozen base: leak at the closing brace
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data == NULL) {
    return -1;
  }
  *s.data = 1;
  s.data = (int *)malloc(sizeof(int));  // <- here
  if (s.data == NULL) {
    return -1;
  }
  *s.data = 2;
  free(s.data);
  return 0;
}
