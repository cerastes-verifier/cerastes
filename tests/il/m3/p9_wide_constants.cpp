// expect: frontend-warning integer_overflow @ L11
// M3 probe P9: reading full-width constants.
unsigned long long ull_max() { return 18446744073709551615ULL; }
long long ll_min() { return -9223372036854775807LL - 1; }
long long ll_max() { return 9223372036854775807LL; }
unsigned long ul_big() { return 0x8000000000000000UL; }
__int128 i128_big() { return (__int128)1 << 100; }
unsigned __int128 u128_max() { return ~(unsigned __int128)0; }
int i_min() { return -2147483647 - 1; }
unsigned u_max() { return 4294967295u; }
__int128 i128_mul() { return (__int128)9223372036854775807LL * 4; }
unsigned __int128 u128_mul() { return (unsigned __int128)18446744073709551615ULL * 18446744073709551615ULL; }
__int128 i128_min() { return -((__int128)1 << 126) - ((__int128)1 << 126) - ((__int128)1 << 126) * 0; }
