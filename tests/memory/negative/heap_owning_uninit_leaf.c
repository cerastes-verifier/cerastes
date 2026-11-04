// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/memory/heap_owning_result.c
// Pins: 0047 items 5, 8: owns_block in an unmarked ensures includes the flags (P4222's default), and malloc left balance unwritten: without ref_to_uninit the return row fails
// Pre-registered from record 0047 items 5, 8: uninitialized at the return
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

CERASTES_OWNING struct account *create_raw(void)
{
  struct account *a = (struct account *)malloc(sizeof(struct account));
  return a;  // <- here
}
