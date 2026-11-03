// expect: rejected cerastes-return_local_address @ L15
// expect: frontend-warning returning_ptr_to_local_temp @ L15
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (R14): return &(struct S){1, 2}.y;
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int *f(void)
{
  return &(struct S){1, 2}.y;  // <- here
}
