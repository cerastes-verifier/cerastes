// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/written_struct_reads.c
// Pins: 0045 item 12: a write under reads(*q): the permission is a fraction (permission at the write)
#include <cerastes.h>
#include <stddef.h>
struct S {
  int a;
  int b;
};

int sum(struct S *q, int n)
    CERASTES_REQUIRES(cerastes_reads(*q) && 0 <= n && n <= 100)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_reads(*q))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    q->a = i;  // <- here
    i += 1;
  }
  return 0;
}
