// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/owning_merged.c
// Pins: 0040 item 3 (question 75): the merged contract owns *p unconditionally, so a call with NULL fails the requires
#include <cerastes.h>
#include <stddef.h>

void set_cell(CERASTES_OWNING int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
{
  *p = 1;
}

int use_merged(void)
{
  set_cell(NULL);
  return 0;
}
