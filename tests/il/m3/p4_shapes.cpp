// expect: frontend-ok
// M3 probe P4: shapes of <<, >>, %, unary -, unary +, !, comma, ++/--, ?: and && / || in plain code.
int sh(int a, int n, unsigned u, unsigned char uc, long l) { int r = a << n; r = a >> n; r = a << u; r = uc << 3; r = a >> l; return r; }
unsigned long shl(unsigned long a, int n) { return a << n; }
int rem(int a, int b, unsigned u) { int r = a % b; r = a / b; r = u % b; return r; }
int un(int a, unsigned char c, unsigned u, bool b) { int r = -a; r = +a; r = -c; r = +c; r = !a; r = -b; r = ~a; return r + (int)-u; }
bool lg(int a, int b) { return !(a < b) && (a == b || a > 3); }
int cond(int a, int b, bool c) { return a < b ? a : b + (c ? 1 : 2); }
int cond_mixed(int a, unsigned u, bool c) { return c ? a : u; }
int comma(int a, int b) { return (a++, b); }
void incdec_val(int a) { int r = a++ + ++a; r = a-- - --a; }
int logic_val(int a, int b) { int r = a && b; r = a || b; r = a < b; return r; }
