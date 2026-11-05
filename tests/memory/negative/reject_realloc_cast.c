// expect: rejected cerastes-call_without_contract @ L12
// mode: c23, c++20
// twin: tests/memory/heap_malloc_free.c
// Pins: 0050 item 6 (audit round 2 S3): the platform's realloc in the cast form (int *)realloc(...) has no contract either: call_without_contract at the call (it was unsupported_type before)
// Pre-registered from record 0050 item 6: rejected call_without_contract at the call, in both modes

#include <stdlib.h>
#include <cerastes.h>

int f(void)
{
  int *p = (int *)realloc(NULL, sizeof(int));  // <- here
  return p == NULL ? 0 : 1;
}
