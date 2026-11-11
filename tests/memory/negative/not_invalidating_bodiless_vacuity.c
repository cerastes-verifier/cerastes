// expect: error check @ L35
// mode: c23, c++20
// twin: tests/memory/not_invalidating_bodiless.c
// Pins: 0058 item 2 (audit round 2, S1): the bodiless marked contract is consistent, not vacuous: the caller's field is calloc'd NULL and stays NULL (the implicit tie), so a false fact about it, b->data != NULL, fails its check; a mutant whose implicit clause contradicted the contract would make every call verify anything after it
// Pre-registered from record 0058 item 2 (audit round 2, S1): one check error at the check (the auditor's audit2/vac_twin.c fails at the check today)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

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
  CERASTES_CHECK(b->data != NULL);  // <- here
  buf_destroy(b);
  return 0;
}
