// expect: frontend-ok
// mode: c++20
// M3 probe P5 (C++20 contrast): the same shapes in C++.
int cmp(int a, int b) { return a < b; }
int logic(int a, int b) { return a && b; }
int lnot(int a) { return !a; }
bool as_bool(int a) { return a; }
bool as_bool2(int a, int b) { bool r = a < b; return r; }
int from_bool(bool b) { return b + 1; }
bool consts() { return true; }
bool ff() { return false; }
int cond_c(int a, int b) { if (a) return 1; if (a < b) return 2; while (a && b) a = a - 1; return 0; }
