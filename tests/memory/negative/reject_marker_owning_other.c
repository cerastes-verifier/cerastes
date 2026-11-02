// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/init_owning_must_init.c
// Pins: 0042 item 4: both markers beside owning (never lifted)
#include <cerastes.h>
#include <stddef.h>

void f(CERASTES_OWNING CERASTES_REF_TO_UNINIT CERASTES_MUST_INIT int *p)  // <- here
{
  if (p != NULL) {
    *p = 0;
  }
}
