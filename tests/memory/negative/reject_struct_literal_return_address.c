// expect: rejected cerastes-return_local_address @ L10
// expect: frontend-warning returning_ptr_to_local_temp @ L10
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (0039 item 6, R14; P67 H3: EDG warns only in C): return &(int){5}; returns the address of a block-scope object
#include <cerastes.h>

int *f(void)
{
  return &(int){5};  // <- here
}
