// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23
// c-only: [static N] in a parameter is C only (a front-end error in C++)
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1: an int a[static N] parameter is rejected: C makes a shorter argument undefined and no contract states it (question 230; accepted as int * with the bound ignored before this session) (lifted by M8; record 0060 item 15 keeps it rejected, flip B: only the message's pointer to this session changes)
// Pre-registered from record 0059 item 1
#include <cerastes.h>

int first(int a[static 3])
{
  return 0;
}
