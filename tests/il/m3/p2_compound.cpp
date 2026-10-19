// expect: frontend-ok
// M3 probe P2: compound assignment and ++/-- on narrow types and bool.
void ca_sc(signed char c, int i) { c += 1; c -= i; c *= 2; c /= 3; c %= 5; c <<= 1; c >>= 1; }
void ca_uc(unsigned char c, unsigned u) { c += 1; c += u; }
void ca_sh(short s, long l) { s += l; s *= s; }
void ca_int(int i, unsigned u, long long ll) { i += u; i += ll; i <<= 3; i >>= u; }
void incdec(signed char c, unsigned char uc, short s, int i) {
  ++c; c++; --c; c--; ++uc; uc++; ++s; s--; ++i; i++; --i; i--;
}
void ca_bool(bool b, int i) { b += i; b |= true; }
void use_val(signed char c, int i) { int a = c++; int d = ++i; a = d; }
