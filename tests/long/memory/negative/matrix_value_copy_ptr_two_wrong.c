// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/memory/matrix_value_copy_ptr.c
// Pins: 0064 item 12 (question 353; question 285's rule: this is the long twin, with tests/memory/negative/matrix_value_copy_ptr_wrong.c as the short one in the normal tier; moved to tests/long/ after its first run took 379 s in C23 on the box (the driver's limit is 300 s), the lowering's own case having taken about 360 s): the copy *p = *q writes the cell p->m[0][1] from q->m[0][1], so a postcondition that it holds q->m[1][0] (what a transposed copy would write) fails at the closing brace; its right-value sibling is copy_ptr in tests/memory/matrix_value_copy_ptr.c
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run (test-mutator-11, 2026-11-27): the one postcondition error at the closing brace; the lowering's own failing two-pointer case took about 360 s, so the first probe is made with the driver's longer limit and the file moves to tests/long/memory/negative/ with a `// timeout:` line if it needs more than 120 s; moved to tests/long/memory/negative/ with `// timeout: 1800` after the first probe (379 s in C23 at load 2; the row matched: postcondition at the closing brace, L20, in the same lines as registered; the rows and the program are unchanged)
// timeout: 3000
#include <cerastes.h>

struct U {
  int a;
  int m[2][2];
  int b[2];
  int c;
};

void copy_ptr_wrong(struct U *p, struct U *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_reads(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_reads(*q) && p->m[0][1] == q->m[1][0])
{
  *p = *q;
}  // <- here
