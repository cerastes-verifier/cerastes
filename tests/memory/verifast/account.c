// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pre-registered from record 0047 items 4, 6, 7 (VeriFast's account.c: create, set, dispose): verifies
// Port of VeriFast's tutorial_solutions/account.c (~/Documents/verifast at 0fb63506) to cerastes (decision 0047, M4.5d2a).
// Adaptation: `#include "stdlib.h"` is `<stdlib.h>` (vlibc's contracts, decision 0047); `struct account *create_account()` is `(void)`; `malloc(sizeof(struct account))` is converted by a C cast (C++ needs one) and `myAccount == 0` is `myAccount == NULL`; `//@ ensures account_balance(result, 0) &*& malloc_block_account(result)` is `cerastes_owns_block(r) && r->balance == 0` (owns_block is malloc_block_account and the cells, 0047 item 4); `account_balance(myAccount, _)` / `account_balance(myAccount, newBalance)` are `cerastes_owns(myAccount->balance)` and `myAccount->balance == newBalance`; `account_balance(myAccount, _) &*& malloc_block_account(myAccount)` is `cerastes_owns_block(myAccount)`; `int main()` is `int main(void)`.
// Left out: `//@ requires true; ensures true;` on `create_account`'s and `main`'s absent contracts (none is written); `//@ ensures true` of `account_dispose` is the absent `ensures` (the block is consumed).
// Line map (this file : VeriFast's):  12 : 3   13 : 4   16 : 7   19 : 11   20 : 12   21 : 13   22 : 14   25 : 17   29 : 21   32 : 24   35 : 28   38 : 31   40 : 35   41 : 36   42 : 37   43 : 38
#include <cerastes.h>
#include <stdlib.h>

struct account {
    int balance;
};

struct account *create_account(void)
    CERASTES_ENSURES(r, cerastes_owns_block(r) && r->balance == 0)
{
    struct account *myAccount = (struct account *)malloc(sizeof(struct account));
    if (myAccount == NULL) { abort(); }
    myAccount->balance = 0;
    return myAccount;
}

void account_set_balance(struct account *myAccount, int newBalance)
    CERASTES_REQUIRES(cerastes_owns(myAccount->balance))
    CERASTES_ENSURES(cerastes_owns(myAccount->balance) && myAccount->balance == newBalance)
{
    myAccount->balance = newBalance;
}

void account_dispose(struct account *myAccount)
    CERASTES_REQUIRES(cerastes_owns_block(myAccount))
{
    free(myAccount);
}

int main(void)
{
    struct account *myAccount = create_account();
    account_set_balance(myAccount, 5);
    account_dispose(myAccount);
    return 0;
}
