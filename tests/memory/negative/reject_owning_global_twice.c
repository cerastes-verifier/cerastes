// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 5 (one pin per rejected shape; the encoder reports it at the declaration carrying the attribute): the attribute twice on one declaration (never lifted)
// Pre-registered from record 0054 item 5: rejected unsupported_annotation at the declaration
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *g CERASTES_OWNING = NULL;  // <- here

int main(void)
{
  return 0;
}
