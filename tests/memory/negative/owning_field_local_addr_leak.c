// expect: error leak @ L47
// mode: c23, c++20
// twin: tests/memory/owning_field_local_addr.c
// Pins: 0055 item 5 (lifetime end, after calls through &s): the filler gave the leaf a block and nothing clears it: reported at the return statement
// Pre-registered from record 0055 item 5 (named leaves: new, no base form): leak at the return statement (held_s_data := s.data != NULL after buf_fill; the lifetime-end row); frozen base: leak at the closing brace
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void buf_init(struct buf *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->data = NULL;
  p->n = 0;
}

void buf_fill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  free(p->data);
  p->data = (int *)malloc(sizeof(int));
  if (p->data != NULL) {
    *p->data = 7;
  }
  p->n = 1;
}

void buf_clear(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->data == NULL)
{
  free(p->data);
  p->data = NULL;
}

int use(void)
{
  struct buf s;
  buf_init(&s);
  buf_fill(&s);
  return 0;  // <- here
}
