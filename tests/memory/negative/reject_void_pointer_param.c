// expect: rejected cerastes-unsupported_type @ L8
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0047 item 11: a void* parameter in user code: VIR never holds void*; only vlibc's two contract shapes use it (M4.7, M8)
// Pre-registered from record 0047 item 11: rejected unsupported_type at the marked line (tag guessed: the nearest existing analogue)
#include <cerastes.h>

void sink(void *p)  // <- here
{
  (void)p;
}
