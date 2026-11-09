// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/owns_block_load_consume.c
// Pins: 0054 item 2 and 10: a block site whose operand is under old holds no permission in the state old reads and stays rejected ("a block site holds a permission in the state it is read in")
// Pre-registered from record 0054 item 2 ("Still rejected: a block site under old") and item 10: rejected unsupported_annotation at the clause (the in-progress encoder already says so)
#include <cerastes.h>

void f(int **pp) CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp)) CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(cerastes_old(*pp)));  // <- here
