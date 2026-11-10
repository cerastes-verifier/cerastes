// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 item 3 ("Bodiless contracts", the default; 0054 item 10): an owning function with no body whose structure has an owning field would give the caller a closure clause for a block that nothing proves distinct from every block the caller holds: rejected, naming M6's contract manifest
// Pre-registered from record 0055 item 3 and 11: rejected unsupported_annotation at the declaration (open point of the draft record: the default is a rejection)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING struct buf *make(void);  // <- here

int use(void)
{
  struct buf *b = make();
  if (b != NULL) {
    free(b->data);
    free(b);
  }
  return 0;
}

int ident(int a)
{
  return a;
}
