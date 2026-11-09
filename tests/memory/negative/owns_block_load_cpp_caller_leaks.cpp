// expect: error leak @ L26
// twin: tests/memory/owns_block_load_cpp.cpp
// Pins: 0054 item 2: the C++ spelling gives the block to the caller, who leaks it
// Pre-registered from record 0054 item 2 (given): leak at the closing brace
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes::owns(*pp))
    CERASTES_ENSURES(cerastes::owns(*pp) && cerastes::owns_block(*pp) && **pp == 5)
{
  *pp = static_cast<int *>(malloc(sizeof(int)));
  if (*pp == nullptr) {
    abort();
  }
  **pp = 5;
}

int use_make()
    CERASTES_ENSURES(r, r == 5)
{
  int *p = nullptr;
  make(&p);
  int v = *p;
  return v;
}  // <- here
