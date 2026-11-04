// expect: error leak @ L16
// mode: c23, c++20
// twin: tests/memory/heap_consume.c
// Pins: 0047 items 4, 7: a function that takes a block and neither frees it nor hands it back leaks it (the closing brace of a void function)
// Pre-registered from record 0047 items 4, 7: leak at the closing brace; re-registered after audit round 1 (record 0047 item 7): unchanged: a void function's closing brace is where the one row is reported
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

void dispose(struct account *a)
    CERASTES_REQUIRES(cerastes_owns_block(a))
{
}  // <- here
