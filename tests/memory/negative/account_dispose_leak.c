// expect: error leak @ L32
// mode: c23, c++20
// twin: tests/memory/verifast/account.c
// Pins: 0047 item 7 and the port of VeriFast's account.c: account_dispose takes the block (owns_block, no ensures) and does not free it (VeriFast: `leak check` at the function's end)
// Pre-registered from record 0047 item 7: leak at the closing brace of account_dispose; re-registered after audit round 1 (record 0047 item 7): unchanged: the closing brace of a void function
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
}  // <- here

int main(void)
{
    struct account *myAccount = create_account();
    account_set_balance(myAccount, 5);
    account_dispose(myAccount);
    return 0;
}
