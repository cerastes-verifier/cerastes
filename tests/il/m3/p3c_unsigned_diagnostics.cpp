// expect: frontend-warning integer_truncated @ L5
// expect: frontend-warning integer_sign_change @ L6
// expect: frontend-warning integer_truncated @ L7
// M3 probe P3c: EDG's truncation and sign-change diagnostics also fire for conversions to unsigned types.
unsigned char a() { unsigned char c = 300; return c; }
unsigned b() { unsigned u = -1; return u; }
unsigned short c() { return 70000; }
