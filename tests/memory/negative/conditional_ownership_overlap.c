// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/conditional_ownership.c
// Pins: 0039 item 7: (c != 0 || owns(*p)) && reads(*p): the two atoms hold together when c == 0, so the contract both owns *p and reads it on one path: permission at the contract (the later atom)
#include <cerastes.h>
#include <stddef.h>

void both(int c, int *p)
    CERASTES_REQUIRES((c != 0 || cerastes_owns(*p)) && cerastes_reads(*p))
{
  (void)c;
  (void)p;
}
