// expect: verified
// expect: native-ok
// mode: c23, c++20
// Port of VeriFast's tutorial_solutions/malloc_block.c (~/Documents/verifast at 0fb63506) to cerastes (decision 0043, M4.5c1).
// Adaptation: `#include "stdlib.h"` is dropped (nothing of it is used once `free` is out); `int main()` is `int main(void)`; `//@ requires true; ensures true;` is the absent contract (`main` takes none); the struct, the local, the pointer to it and the member store are as written.
// Left out: the commented `//free(myAccount);`: the object is a local, not a `malloc` block (M4.5d2a added `malloc` and `free`; the twin that frees it is tests/memory/negative/malloc_block_free_local.c, `invalid-free`).  The aliasing of the pointer and the local is the one that tests/memory/negative/malloc_block_read_first.c reads through.
// Line map (this file : VeriFast's):  10 : 4   14 : 8   16 : 12   17 : 13   18 : 14   19 : 16
#include <cerastes.h>

struct account {
    int balance;
};

int main(void)
{
    struct account myAccountLocal;
    struct account *myAccount = &myAccountLocal;
    myAccount->balance = 5;
    return 0;
}
