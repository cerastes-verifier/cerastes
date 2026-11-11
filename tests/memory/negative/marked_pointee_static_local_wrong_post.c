// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/memory/marked_pointee_static_local.c
// Pins: 0058 item 4 (audit S5): the same function with an ensures that the allocation failure path breaks
// Pre-registered from record 0058 item 4 (audit S5, question 202): one postcondition error at the return of the failure path
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int next(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING CERASTES_REF_TO_UNINIT static struct buf *cache = NULL;
  if (cache == NULL) {
    cache = (struct buf *)malloc(sizeof(struct buf));
    if (cache == NULL) {
      return -1;  // <- here
    }
    cache->data = NULL;
  }
  cache->n = 1;
  return 0;
}
