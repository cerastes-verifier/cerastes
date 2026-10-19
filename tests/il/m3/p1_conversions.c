// expect: frontend-ok
// mode: c23
// M3 probe P1 (C23): the same promotions and usual arithmetic conversions, and compound assignment.
signed char sc1(signed char a, signed char b) { return a + b; }
int us1(unsigned short a, unsigned short b) { return a * b; }
unsigned mix_ui(int a, unsigned b) { return a + b; }
long long mix_ll(long long a, unsigned long b) { return a + b; }
int lt_mixed(int a, unsigned b) { return a < b; }
void ca(signed char c, int i, unsigned u) { c += i; i += u; c <<= 1; ++c; c++; }
