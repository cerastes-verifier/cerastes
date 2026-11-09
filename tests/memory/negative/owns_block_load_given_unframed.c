// expect: error permission @ L11
// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/owns_block_load_given.c
// Pins: 0054 item 2 (framing): the site reads *pp, and no atom before it in the ensures frames that load: permission at the clause (f__wf, as 0045 item 4); no caller, so no second error. Pre-registered from record 0054 item 2 ("When it is read"; S39 a1w): permission at the ensures clause; after the first run (question 183): also permission at the return row (L18), whose body cannot hold the unframed place either, as an unframed owns(**pp) does
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns_block(*pp))  // <- here
{
  *pp = (int *)malloc(sizeof(int));
  if (*pp == NULL) {
    abort();
  }
  **pp = 5;
}
