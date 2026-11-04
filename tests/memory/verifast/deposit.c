// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pre-registered from record 0047 items 4, 6 (VeriFast's deposit.c): verifies
// Port of VeriFast's tutorial_solutions/deposit.c (~/Documents/verifast at 0fb63506) to cerastes (decision 0047, M4.5d2a).
// Adaptation: as for account.c; `account_balance(myAccount, ?theBalance)` ... `account_balance(myAccount, theBalance)` are `cerastes_owns(myAccount->balance)` and `myAccount->balance == cerastes_old(myAccount->balance)` (the ghost `theBalance` is the `old`); `account_balance(myAccount, theBalance + amount)` is `myAccount->balance == cerastes_old(myAccount->balance) + amount`, with `0 <= myAccount->balance <= 1000 && 0 <= amount <= 1000` added to its `requires` because cerastes checks `+=` for overflow; `assert(b == 15)` is CERASTES_CHECK and `return 0;` is `return b - 15;` (0) because the ordinary compilers warn about a local that only an annotation reads (the file is `native-ok`).
// Line map (this file : VeriFast's):  11 : 3   15 : 7   18 : 11   19 : 12   20 : 13   21 : 14   24 : 17   28 : 21   31 : 24   35 : 28   38 : 31   42 : 35   45 : 38   48 : 42   51 : 45   53 : 49   54 : 50   55 : 51   56 : 52   57 : 53   58 : 54   59 : 55
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
    CERASTES_CHECK(b == 15);
    account_dispose(myAccount);
    return b - 15;
}
