// expect: rejected cerastes-unsupported_declaration @ L11
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0063 item 14 (question 301): jmp_buf of the real <setjmp.h> is an array type reached through a typedef that no verified file declares (int[48] on macOS, an array of one structure on Linux): an object of it is rejected the same way on both hosts, at its declaration (the one untagged pin; no library-fixture header)
// Pre-registered from record 0063 item 14 (Part D), before any run: rejected
#include <cerastes.h>
#include <setjmp.h>

int f(void)
{
  jmp_buf b;  // <- here
  (void)b;
  return 0;
}
