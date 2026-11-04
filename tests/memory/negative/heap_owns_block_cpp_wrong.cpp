// expect: error postcondition @ L20
// twin: tests/memory/heap_owns_block_cpp.cpp
// Pins: 0047 item 4 (C++): the value clause after cerastes::owns_block fails
// Pre-registered from record 0047 item 4: postcondition at the return
#include <cerastes.h>
#include <stdlib.h>

struct account {
  int balance;
};

account *create_account()
    CERASTES_ENSURES(r, cerastes::owns_block(r) && r->balance == 0)
{
  account *a = static_cast<account *>(malloc(sizeof(account)));
  if (a == nullptr) {
    abort();
  }
  a->balance = 1;
  return a;  // <- here
}
