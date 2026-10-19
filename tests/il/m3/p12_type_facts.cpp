// expect: frontend-ok
// mode: c++20
// M3 probe P12: size and signedness of every integer type, at run time from EDG.
bool f_bool(bool x) { return x; }
char f_char(char x) { return x; }
signed char f_sc(signed char x) { return x; }
unsigned char f_uc(unsigned char x) { return x; }
short f_s(short x) { return x; }
unsigned short f_us(unsigned short x) { return x; }
int f_i(int x) { return x; }
unsigned f_u(unsigned x) { return x; }
long f_l(long x) { return x; }
unsigned long f_ul(unsigned long x) { return x; }
long long f_ll(long long x) { return x; }
unsigned long long f_ull(unsigned long long x) { return x; }
wchar_t f_wc(wchar_t x) { return x; }
char8_t f_c8(char8_t x) { return x; }
char16_t f_c16(char16_t x) { return x; }
char32_t f_c32(char32_t x) { return x; }
__int128 f_i128(__int128 x) { return x; }
unsigned __int128 f_u128(unsigned __int128 x) { return x; }
_BitInt(24) f_bi(_BitInt(24) x) { return x; }
unsigned _BitInt(7) f_ubi(unsigned _BitInt(7) x) { return x; }
