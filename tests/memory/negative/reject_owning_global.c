// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/owning_local_free.c
// Pins: 0052 items 3 and 8 (amends 0029 item 1): cerastes::owning on a global: the registration accepts target v, the encoder rejects a global; owning globals are M4.5d2b2 (leak at main's end, static owners, blocks through loads)
// Pre-registered from record 0052 item 8: rejected unsupported_annotation at the declaration (tag and line guessed: the declaration carrying the attribute; the message names d2b2)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *g = NULL;  // <- here

int main(void)
{
  return 0;
}
