// expect: error leak @ L32
// expect: error leak @ L33
// mode: c23, c++20
// twin: tests/memory/marked_pointee_param.c
// Pins: 0058 item 4 (question 202): a caller whose struct local holds a block in the owning field passes &s to a ref_to_uninit callee that overwrites the field: the block is dropped, the caller's end row reports it: leak at the caller's end
// Pre-registered from record 0058 item 4 (question 202): two leaks, the field's token at the return and the allocation at the closing brace (observed on the Part F build with question 202 applied)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void fill(struct buf *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->n = 0;
  p->data = NULL;
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf s = { NULL, 1 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data == NULL) {
    return -1;
  }
  fill(&s);
  return 0;  // <- here
}  // <- here
