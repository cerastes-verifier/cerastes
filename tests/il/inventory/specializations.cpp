// expect: golden
// Inventory probe for rows F2 and G6: explicit specialization and explicit
// instantiation versus an implicit instance.
// PASS: the implicit instance (use of ident<int>) has is_specialized and
// explicit_instantiation FALSE; ident<char> (explicit specialization) and
// twice<long> (explicit instantiation) have the corresponding flag TRUE.
template <typename T> T ident(T x) { return x; }
template <> char ident<char>(char x) { return x; }
template <typename T> T twice(T x) { return x + x; }
template long twice<long>(long);
int use() { return ident(1) + ident('a') + twice(1); }
