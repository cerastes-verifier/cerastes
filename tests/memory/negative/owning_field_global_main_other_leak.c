// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owning_field_global_main.c
// Pins: 0055 item 5 (question 174): the block goes to the plain field g.view of the same structure: only the owning leaf is exempt at main's end, so main's end row reports the block (owning_global_main_nonowning_leak.c)
// Pre-registered from record 0055 item 5 (named leaves: new, no base form): leak at main's closing brace (the end-of-function row; the exemption names only the leaf's token, S40 (h) h_main_other_fail)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int *view;
};

static struct buf g = { NULL, NULL };

int main(void)
{
  g.view = (int *)malloc(sizeof(int));
  if (g.view != NULL) {
    *g.view = 1;
  }
  return 0;
}  // <- here
