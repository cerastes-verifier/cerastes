// expect: frontend-ok
// M3 probe P1: integer promotions and the usual arithmetic conversions in plain code.
signed char sc1(signed char a, signed char b) { return a + b; }
unsigned char uc1(unsigned char a, unsigned char b) { return a * b; }
int us1(unsigned short a, unsigned short b) { return a * b; }
int sh1(short a, short b) { return a - b; }
unsigned mix_ui(int a, unsigned b) { return a + b; }
long mix_li(long a, int b) { return a + b; }
long mix_lu(long a, unsigned b) { return a + b; }
unsigned long mix_ul(unsigned long a, long b) { return a + b; }
long long mix_ll(long long a, unsigned long b) { return a + b; }
bool lt_mixed(int a, unsigned b) { return a < b; }
int ch1(char a, char b) { return a + b; }
