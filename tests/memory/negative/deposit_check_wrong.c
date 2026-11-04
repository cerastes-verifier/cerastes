// expect: error check @ L55
// mode: c23, c++20
// twin: tests/memory/verifast/deposit.c
// Pins: 0047 item 4 and the port of VeriFast's deposit.c: the balance is 5 + 10, not 16 (VeriFast: `assert(b == 16)` fails)
// Pre-registered from record 0047 item 4: check at the CERASTES_CHECK
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

int account_get_balance(struct account *myAccount)
    CERASTES_REQUIRES(cerastes_owns(myAccount->balance))
    CERASTES_ENSURES(r, cerastes_owns(myAccount->balance) && myAccount->balance == cerastes_old(myAccount->balance) && r == myAccount->balance)
{
    return myAccount->balance;
}

void account_set_balance(struct account *myAccount, int newBalance)
    CERASTES_REQUIRES(cerastes_owns(myAccount->balance))
    CERASTES_ENSURES(cerastes_owns(myAccount->balance) && myAccount->balance == newBalance)
{
    myAccount->balance = newBalance;
}

void account_deposit(struct account *myAccount, int amount)
    CERASTES_REQUIRES(cerastes_owns(myAccount->balance) && 0 <= myAccount->balance && myAccount->balance <= 1000 && 0 <= amount && amount <= 1000)
    CERASTES_ENSURES(cerastes_owns(myAccount->balance) && myAccount->balance == cerastes_old(myAccount->balance) + amount)
{
    myAccount->balance += amount;
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
    account_deposit(myAccount, 10);
    int b = account_get_balance(myAccount);
    CERASTES_CHECK(b == 16);  // <- here
    account_dispose(myAccount);
    return b - 15;
}
