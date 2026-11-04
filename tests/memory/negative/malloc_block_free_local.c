// expect: error invalid-free @ L18
// mode: c23, c++20
// twin: tests/memory/verifast/malloc_block.c
// Pins: 0047 item 6 and the port of VeriFast's tutorial_solutions/malloc_block.c: the commented `//free(myAccount);` of the original, uncommented: the pointer is the address of a local, which is no allocation (VeriFast: no malloc_block chunk)
// Pre-registered from record 0047 item 6: invalid-free at the free
#include <cerastes.h>
#include <stdlib.h>

struct account {
    int balance;
};

int main(void)
{
    struct account myAccountLocal;
    struct account *myAccount = &myAccountLocal;
    myAccount->balance = 5;
    free(myAccount);  // <- here
    return 0;
}
