// expect: frontend-warning integer_truncated @ L5
// expect: frontend-warning integer_sign_change @ L6
// M3 probe P3b: converted constants in default arguments (file-scope constants): is `expr` kept?
int ok_default(long c = 7, unsigned u = 3, signed char s = 100);
int bad_default1(signed char s = 300);
int bad_default2(unsigned u = -1);
int use(int x) { return ok_default() + ok_default(1) + bad_default1() + bad_default2() + x; }
