// expect: error permission @ L17
// N1 of record 0058 leaves one message here; the line was: expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/owning_field_bodiless.c
// Pins: 0055 item 3 ("Bodiless contracts"): without p->data == old(p->data) a bodiless borrow could return a block of the field that the caller already holds a second token for; touch__wf fails at the ensures clause with the closure
// Pre-registered from record 0055 item 3: two permission errors at the ensures clause (the line is a best reading, as owns_block_load_bodiless_untied.c): "returns the block of p->data on a path where its requires did not take it" and "without saying it is the one its requires took (write p->data == old(p->data))", both from the missing tie, as observed on the hand-written form; the call site's check is dropped from the twin: after an untied call the field is unknown
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));  // <- here

void buf_destroy(struct buf *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_block(p))
{
  if (p != NULL) {
    free(p->data);
    free(p);
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  touch(b);
  buf_destroy(b);
  return 0;
}
