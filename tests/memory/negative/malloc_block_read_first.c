// expect: error uninitialized @ L15
// mode: c23, c++20
// twin: tests/memory/verifast/malloc_block.c
// Pins: 0043 items 4, 5 and the port of VeriFast's tutorial_solutions/malloc_block.c: reading myAccount->balance before the store of 5 reads a leaf that was never written (VeriFast: no matching heap chunk, `balance |-> _` is not `balance |-> ?v`)
#include <cerastes.h>

struct account {
    int balance;
};

int main(void)
{
    struct account myAccountLocal;
    struct account *myAccount = &myAccountLocal;
    int b = myAccount->balance;  // <- here
    myAccount->balance = 5;
    return b;
}
