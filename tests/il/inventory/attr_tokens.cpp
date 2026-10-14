// expect: golden
// Inventory probe for check 5: how EDG spells the raw tokens of an
// unrecognized attribute. PASS means the golden records the spelling EDG
// prints; the source spells 0x1F, 9u, 1.50, 1E3 and 'a' as written here.
[[unreg_ns::t1(0x1F, 9u, 1.50, 1E3, 'a', "s", 07, 10UL)]] int tok(int x);
int use() { return tok(1); }
