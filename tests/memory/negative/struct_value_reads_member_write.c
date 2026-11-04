// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/struct_value_reads_member.c
// Pins: 0045 item 9: a write under reads(*s.q): permission at the write
#include <cerastes.h>
#include <stddef.h>
struct P {
  int *q;
};

void f(struct P s)
    CERASTES_REQUIRES(cerastes_reads(*s.q))
{
  *s.q = 1;  // <- here
}
