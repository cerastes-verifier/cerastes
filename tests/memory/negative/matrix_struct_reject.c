// expect: rejected cerastes-unsupported_type @ L16
// expect: rejected cerastes-unsupported_type @ L21
// mode: c23, c++20
// twin: tests/memory/matrix_local.c
// Pins: 0064 item 1 (question 363, settled with the advisor on 2026-11-26): a matrix of STRUCTURES (struct P m[2][2], a row parameter struct P a[][2]) is not built in this session (there is no spike fixture for a row whose elements are structures): the encoder rejects it at its C line, one row per storage
// Pre-registered from record 0064 item 1 (Part E), before any run: rejected (the tag, unsupported_type, is by analogy with the other element-type rows and is not stated by the record: to confirm at the first run, question 160)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int local(void)
{
  struct P m[2][2];  // <- here
  m[0][0].x = 1;
  return m[0][0].x;
}

int param(struct P a[][2])  // <- here
{
  return a[0][0].x * 0;
}
