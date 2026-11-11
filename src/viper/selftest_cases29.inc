// (included by selftest.cpp) Arrays inside one function, M4.5e1a Part B's lowering (decision 0059 items 2, 3, 4, 6, 7 and 9): an automatic array local is one block
// (`alloc_arr_<t>(N)`, with `zero_arr_<t>(r, N)` at the declaration of an initializer that does not write every element), an element `a[i]` is `Deref(ElemAddr(AddrOf(a), i))`
// lowered to `loc(block_of(a), offset_of(a) + i)` with the bound row `out-of-bounds` (a place access: `0 <= offset + i < len`; an address as a value: `<= len`) before the
// permission and the flag rows, the scope exit is two marked range rows, the liveness row and three `exhale`s (item 6), and two elements accessed in one full expression, one of
// them written, get 0039 item 3's `unsequenced` row (item 9), as an element beside a `*p` into the same array does.
// Each rule has a case that verifies and a twin that fails for one reason (invariant 2); a failure's category and the VIR node whose position the record gives are the
// expectation (the element's read for `uninitialized` and for a read's bound, the store for a write's, the address for `&a[i]`, the block's closing brace for the scope exit,
// the store for `unsequenced`).  `fail_only(c, loc, n, i)`: n checks of that category at that line, only the i-th (in Viper line order) fails.
namespace {

void define_arrays_lowering()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    // an automatic array local of `count` elements of type `element`: a cell, declared without an initializer (`zero_filled`: the fill of a braced list that writes fewer elements)
    auto arr = [](Kit& k, Function* f, const char* name, Type element, unsigned long long count, bool zero_filled = false)
    {
        Var* x {f->new_local(name, k.prog.array_of(element, count), k.at())};
        f->mark_address_taken(x);
        x->zero_filled = zero_filled;
        return x;
    };
    // `&a[i]`, `a[i]` and the store `a[i] = v;`
    auto at_ = [](Kit& k, const Var* a, const Expr* i)
    {
        return k.b.elem_addr(k.addr(a), i, k.at());
    };
    auto rd = [=](Kit& k, const Var* a, const Expr* i)
    {
        return k.deref(at_(k, a, i));
    };
    auto wr = [=](Kit& k, const Var* a, const Expr* i, const Expr* v)
    {
        return k.store(at_(k, a, i), v);
    };
    // an index that is the parameter `i` of `f`, with its spelling
    auto ix = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.pv(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto pown = [=](Kit& k, Function* f, int i, int amount)
    {
        return k.b.atom(AtomKind::Owns, fx(k, f, i), amount, k.at());
    };

    // ---- item 3 and item 4: the block, a write, a read ----
    // void f() { int a[3]; a[0] = 1; a[2] = 3; check(a[0] + a[2] == [4|5]); }
    const auto rw_const = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        const Stmt* c {k.chk(k.eq_c(k.add(rd(k, a, N(k, "0")), rd(k, a, N(k, "2"))), k.i32, wrong ? "5" : "4"))};
        f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "1")), wr(k, a, N(k, "2"), N(k, "3")), c});
        k.done(f);
        return c;
    };
    lc("arr_rw_const", "a[0] = 1; a[2] = 3; a[0] + a[2] == 4: two elements of a three-element block, written then read",
       [=](Kit& k)
       {
           rw_const(k, false);
       });
    lc("arr_rw_const_wrong", "the same program claiming a[0] + a[2] == 5",
       [=](Kit& k)
       {
           k.fail("check", rw_const(k, true)->loc);
       });

    // void f() { int a[3]; a[0] = 1; int x = a[[0|1]]; }
    const auto read_elem = [=](Kit& k, const char* index) -> const Expr*
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        Var* x {k.local(f, "x", k.i32)};
        const Expr* r {rd(k, a, N(k, index))};
        f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "1")), k.decl(x, r)});
        k.done(f);
        return r;
    };
    lc("arr_read_written", "int a[3]; a[0] = 1; int x = a[0]: the element was written",
       [=](Kit& k)
       {
           read_elem(k, "0");
       });
    lc("arr_read_unwritten", "the same program reading a[1], which nothing wrote: its flag is false (0019, path-precise)",
       [=](Kit& k)
       {
           k.fail("uninitialized", read_elem(k, "1")->loc);
       });

    // ---- item 4: the bound, in place position ----
    // void f(int i) requires(0 <= i && i <= [2|3]) { int a[3]; a[0] = 0; a[1] = 0; a[2] = 0; a[i] = 7; }
    const auto write_index = [=](Kit& k, const char* hi) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd, {{"i", k.i32}})};
        k.req(f, k.land(k.ge_c(fx(k, f, 0), k.i32, "0"), k.le_c(fx(k, f, 0), k.i32, hi)));
        Var* a {arr(k, f, "a", k.i32, 3U)};
        const Stmt* w {wr(k, a, ix(k, f, 0), N(k, "7"))};
        f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "0")), wr(k, a, N(k, "1"), N(k, "0")),
                          wr(k, a, N(k, "2"), N(k, "0")), w});
        k.done(f);
        return w;
    };
    lc("arr_write_index", "a[i] = 7 for 0 <= i <= 2 of a three-element array: the bound row holds",
       [=](Kit& k)
       {
           write_index(k, "2");
       });
    lc("arr_write_index_end", "the same with 0 <= i <= 3: i == 3 is one past the end, never writable: out-of-bounds at the store",
       [=](Kit& k)
       {
           k.fail("out-of-bounds", write_index(k, "3")->loc);
       });
    // void f(int i) requires(-1 <= i && i <= 2) { int a[3]; a[0] = 0; a[1] = 0; a[2] = 0; int x = a[i]; } and with 0 <= i
    const auto read_index = [=](Kit& k, const char* lo) -> const Expr*
    {
        Function* f {k.fn("f", k.vd, {{"i", k.i32}})};
        k.req(f, k.land(k.ge_c(fx(k, f, 0), k.i32, lo), k.le_c(fx(k, f, 0), k.i32, "2")));
        Var* a {arr(k, f, "a", k.i32, 3U)};
        Var* x {k.local(f, "x", k.i32)};
        const Expr* r {rd(k, a, ix(k, f, 0))};
        f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "0")), wr(k, a, N(k, "1"), N(k, "0")),
                          wr(k, a, N(k, "2"), N(k, "0")), k.decl(x, r)});
        k.done(f);
        return r;
    };
    lc("arr_read_index", "int x = a[i] for 0 <= i <= 2: the bound row holds",
       [=](Kit& k)
       {
           read_index(k, "0");
       });
    lc("arr_read_index_negative", "the same with -1 <= i: a[-1] is outside (the value, not a conversion of it, is the offset): out-of-bounds at the read",
       [=](Kit& k)
       {
           k.fail("out-of-bounds", read_index(k, "-1")->loc);
       });

    // ---- item 3: the zero fill of a braced initializer ----
    // void f() { int a[4]; <fill>; a[0] = 1; check(a[3] == [0|1] && a[0] == 1); }
    lc("arr_zero_fill_int", "int a[4] = {1}: zero_arr_int fills a[1..3] with 0 and sets their flags, then the store of a[0]: a[3] == 0 and a[0] == 1",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 4U, true)};
           f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "1")),
                             k.chk(k.land(k.eq_c(rd(k, a, N(k, "3")), k.i32, "0"), k.eq_c(rd(k, a, N(k, "0")), k.i32, "1")))});
           k.done(f);
       });
    lc("arr_zero_fill_int_wrong", "the same program claiming a[3] == 1",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 4U, true)};
           const Stmt* c {k.chk(k.eq_c(rd(k, a, N(k, "3")), k.i32, "1"))};
           f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "1")), c});
           k.done(f);
           k.fail("check", c->loc);
       });
    // void f() { bool b[2]; <fill>; check([!]b[1]); }
    const auto fill_bool = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd)};
        Var* b {arr(k, f, "b", k.bl, 2U, true)};
        const Expr* v {rd(k, b, N(k, "1"))};
        const Stmt* c {k.chk(wrong ? v : k.lnot(v))};
        f->body = k.blkc({k.decl(b, nullptr), c});
        k.done(f);
        return c;
    };
    lc("arr_zero_fill_bool", "bool b[2] = {}: zero_arr_bool sets every element false and initialized: !b[1]",
       [=](Kit& k)
       {
           fill_bool(k, false);
       });
    lc("arr_zero_fill_bool_wrong", "the same program claiming b[1]",
       [=](Kit& k)
       {
           k.fail("check", fill_bool(k, true)->loc);
       });
    // void f() { int *a[2]; <fill>; check(a[1] [==|!=] NULL); }
    const auto fill_ptr = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", PI(k), 2U, true)};
        const Expr* v {rd(k, a, N(k, "1"))};
        const Stmt* c {k.chk(wrong ? k.ne(v, k.nul(PI(k))) : k.eq(v, k.nul(PI(k))))};
        f->body = k.blkc({k.decl(a, nullptr), c});
        k.done(f);
        return c;
    };
    lc("arr_zero_fill_ptr", "int *a[2] = {}: zero_arr_ptr_int sets every element null and initialized: a[1] == NULL",
       [=](Kit& k)
       {
           fill_ptr(k, false);
       });
    lc("arr_zero_fill_ptr_wrong", "the same program claiming a[1] != NULL",
       [=](Kit& k)
       {
           k.fail("check", fill_ptr(k, true)->loc);
       });

    // ---- item 6: the scope exit ----
    // void bump(int *p) requires(owns(*p) && *p < 100) ensures(owns(*p) && *p == old(*p) + 1) { *p += 1; }
    // void eat(int *p) requires(owns(*p))   (bodyless: the callee keeps what it takes)
    const auto make_bump = [=](Kit& k) -> Function*
    {
        Function* f {k.fn("bump", k.vd, {{"p", PI(k)}})};
        k.req(f, k.land(pown(k, f, 0, 0), k.lt(k.deref(fx(k, f, 0)), N(k, "100"))));
        k.ens(f, k.land(pown(k, f, 0, 0), k.eq(k.deref(fx(k, f, 0)), k.add(k.old(k.deref(fx(k, f, 0))), N(k, "1")))));
        f->body = k.blk({k.cstore(k.pv(f, 0), BinaryOp::Add, N(k, "1"))});
        k.done(f);
        return f;
    };
    const auto make_eat = [=](Kit& k) -> Function*
    {
        Function* f {k.fn("eat", k.vd, {{"p", PI(k)}})};
        k.req(f, pown(k, f, 0, 0));
        k.trusted(f);
        return f;
    };
    // void f() { int a[3]; a[1] = 5; bump(&a[1]);  check(a[1] == 6); }   |   void f() { int a[3]; a[1] = 5; eat(&a[1]); }
    lc("arr_scope_exit_returned", "a callee that owns *p and gives it back (bump(&a[1])): the scope exit finds every element's permission, and a[1] == 6",
       [=](Kit& k)
       {
           Function* bump {make_bump(k)};
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 3U)};
           f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "1"), N(k, "5")),
                             k.es(k.call(bump, {at_(k, a, N(k, "1"))})), k.chk(k.eq_c(rd(k, a, N(k, "1")), k.i32, "6"))});
           k.done(f);
       });
    lc("arr_scope_exit_kept", "a callee that takes the permission to a[1] and keeps it (eat(&a[1])): the marked range row at the closing brace fails, and only it (lifetime)",
       [=](Kit& k)
       {
           Function* eat {make_eat(k)};
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 3U)};
           const Stmt* body {k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "1"), N(k, "5")),
                                     k.es(k.call(eat, {at_(k, a, N(k, "1"))}))})};
           f->body = body;
           k.done(f);
           // two marked rows (the value, then the flag) and the liveness row stand at the brace; a failed marked row ends the path
           k.fail_only("lifetime", body->end_loc, 3U, 0U);
       });
    // int f() ensures(r == [7|8]) { int a[2]; a[0] = 3; { int b[2]; b[1] = 4; return a[0] + b[1]; } }
    const auto return_in_block = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* f {k.fn("f", k.i32)};
        k.ens(f, k.eq_c(k.R(f), k.i32, wrong ? "8" : "7"));
        Var* a {arr(k, f, "a", k.i32, 2U)};
        Var* b {arr(k, f, "b", k.i32, 2U)};
        const Stmt* r {k.ret(k.add(rd(k, a, N(k, "0")), rd(k, b, N(k, "1"))))};
        f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "3")),
                          k.blkc({k.decl(b, nullptr), wr(k, b, N(k, "1"), N(k, "4")), r})});
        k.done(f);
        return r;
    };
    lc("arr_return_in_block", "a return inside a nested block leaves both blocks, the inner one first: each array's rows and exhales, then the postcondition r == 7",
       [=](Kit& k)
       {
           return_in_block(k, false);
       });
    lc("arr_return_in_block_wrong", "the same program claiming r == 8",
       [=](Kit& k)
       {
           k.fail("postcondition", return_in_block(k, true)->loc);
       });

    // ---- item 4: an address as a value (one past the end is formed, never accessed) ----
    // void id(int *p);   void f(int i) requires(0 <= i && i <= [3|4]) { int a[3]; id(&a[i]); }
    const auto addr_value = [=](Kit& k, const char* hi) -> const Expr*
    {
        Function* id {k.fn("id", k.vd, {{"p", PI(k)}})};
        k.trusted(id);
        Function* f {k.fn("f", k.vd, {{"i", k.i32}})};
        k.req(f, k.land(k.ge_c(fx(k, f, 0), k.i32, "0"), k.le_c(fx(k, f, 0), k.i32, hi)));
        Var* a {arr(k, f, "a", k.i32, 3U)};
        const Expr* ea {at_(k, a, ix(k, f, 0))};
        f->body = k.blkc({k.decl(a, nullptr), k.es(k.call(id, {ea}))});
        k.done(f);
        return ea;
    };
    lc("arr_addr_value", "id(&a[i]) for 0 <= i <= 3: the address one past the end of the three-element array may be formed",
       [=](Kit& k)
       {
           addr_value(k, "3");
       });
    lc("arr_addr_value_beyond", "the same with 0 <= i <= 4: &a[4] is more than one past the end: out-of-bounds at the address",
       [=](Kit& k)
       {
           k.fail("out-of-bounds", addr_value(k, "4")->loc);
       });

    // ---- item 9: sequencing ----
    // void f(int i, int j) requires(0 <= i, j <= 2 [&& i != j]) { int a[3]; a[0] = 0; a[1] = 0; a[2] = 0; a[i] = a[j]++; }
    const auto seq_elems = [=](Kit& k, bool distinct) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd, {{"i", k.i32}, {"j", k.i32}})};
        const auto in3 = [&](int p)
        {
            return k.land(k.ge_c(fx(k, f, p), k.i32, "0"), k.le_c(fx(k, f, p), k.i32, "2"));
        };
        const Expr* pre {k.land(in3(0), in3(1))};
        k.req(f, distinct ? k.land(pre, k.ne(fx(k, f, 0), fx(k, f, 1))) : pre);
        Var* a {arr(k, f, "a", k.i32, 3U)};
        const Stmt* s {k.store(at_(k, a, ix(k, f, 0)), k.b.inc_dec_store_expr(at_(k, a, ix(k, f, 1)), true, true, k.at()))};
        f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "0")), wr(k, a, N(k, "1"), N(k, "0")),
                          wr(k, a, N(k, "2"), N(k, "0")), s});
        k.done(f);
        return s;
    };
    lc("arr_seq_distinct", "a[i] = a[j]++ with i != j: two elements of one array, the write and the increment are unsequenced and never one object",
       [=](Kit& k)
       {
           seq_elems(k, true);
       });
    lc("arr_seq_alias", "the same without i != j: they may be one element, so the two writes are unsequenced on one object (unsequenced, at the store)",
       [=](Kit& k)
       {
           k.fail("unsequenced", seq_elems(k, false)->loc);
       });
    // void f(int i) requires(0 <= i <= 2 [&& i != 1]) { int a[3]; a[0..2] = 0; int *p = &a[1]; *p = a[i]++; }
    const auto seq_pointer = [=](Kit& k, bool distinct) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd, {{"i", k.i32}})};
        const Expr* pre {k.land(k.ge_c(fx(k, f, 0), k.i32, "0"), k.le_c(fx(k, f, 0), k.i32, "2"))};
        k.req(f, distinct ? k.land(pre, k.ne(fx(k, f, 0), N(k, "1"))) : pre);
        Var* a {arr(k, f, "a", k.i32, 3U)};
        Var* p {k.local(f, "p", PI(k))};
        const Stmt* s {k.store(k.pn(p), k.b.inc_dec_store_expr(at_(k, a, ix(k, f, 0)), true, true, k.at()))};
        f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "0")), wr(k, a, N(k, "1"), N(k, "0")),
                          wr(k, a, N(k, "2"), N(k, "0")), k.decl(p, at_(k, a, N(k, "1"))), s});
        k.done(f);
        return s;
    };
    lc("arr_seq_pointer", "int *p = &a[1]; *p = a[i]++ with i != 1: a write through a pointer into the array beside an element's increment, never one object",
       [=](Kit& k)
       {
           seq_pointer(k, true);
       });
    lc("arr_seq_pointer_alias", "the same without i != 1: p may point at a[i] (unsequenced, at the store)",
       [=](Kit& k)
       {
           k.fail("unsequenced", seq_pointer(k, false)->loc);
       });

    // ---- items 4 and 9: compound assignment and increment share the element's address ----
    // void f([int i]) { int a[3]; a[1] = 5; a[1] += 2; a[1]++; check(a[1] == [8|9]); }
    const auto compound_const = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        const Stmt* c {k.chk(k.eq_c(rd(k, a, N(k, "1")), k.i32, wrong ? "9" : "8"))};
        f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "1"), N(k, "5")),
                          k.cstore(at_(k, a, N(k, "1")), BinaryOp::Add, N(k, "2")), k.idstore(at_(k, a, N(k, "1")), true), c});
        k.done(f);
        return c;
    };
    lc("arr_compound_const", "a[1] = 5; a[1] += 2; a[1]++: a[1] == 8 (the read and the write of a compound store are one element)",
       [=](Kit& k)
       {
           compound_const(k, false);
       });
    lc("arr_compound_const_wrong", "the same program claiming a[1] == 9",
       [=](Kit& k)
       {
           k.fail("check", compound_const(k, true)->loc);
       });
    // void f(int i) requires(0 <= i && i <= [2|3]) { int a[3]; a[0..2] = 0; a[i] += 2; }
    const auto compound_index = [=](Kit& k, const char* hi) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd, {{"i", k.i32}})};
        k.req(f, k.land(k.ge_c(fx(k, f, 0), k.i32, "0"), k.le_c(fx(k, f, 0), k.i32, hi)));
        Var* a {arr(k, f, "a", k.i32, 3U)};
        const Stmt* s {k.cstore(at_(k, a, ix(k, f, 0)), BinaryOp::Add, N(k, "2"))};
        f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "0")), wr(k, a, N(k, "1"), N(k, "0")),
                          wr(k, a, N(k, "2"), N(k, "0")), s});
        k.done(f);
        return s;
    };
    lc("arr_compound_index", "a[i] += 2 for 0 <= i <= 2: one bound row for the element's read and write",
       [=](Kit& k)
       {
           compound_index(k, "2");
       });
    lc("arr_compound_index_end", "the same with 0 <= i <= 3: out-of-bounds at the compound store, once",
       [=](Kit& k)
       {
           k.fail("out-of-bounds", compound_index(k, "3")->loc);
       });

    // ---- pointer elements: the fields are named by the pointee ----
    // int f() ensures(r == 3) { int x = 1; int *a[2]; a[0] = &x; *a[0] = 3; return x; }
    lc("arr_ptr_elems", "int *a[2]: a[0] = &x; *a[0] = 3: a pointer element is a cell of the pointee's field, and the pointer read back is &x",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32)};
           k.ens(f, k.eq_c(k.R(f), k.i32, "3"));
           Var* x {k.cell(f, "x", k.i32)};
           Var* a {arr(k, f, "a", PI(k), 2U)};
           f->body = k.blkc({k.decl(x, N(k, "1")), k.decl(a, nullptr), wr(k, a, N(k, "0"), k.addr(x)),
                             k.store(k.deref(at_(k, a, N(k, "0"))), N(k, "3")), k.ret(k.v(x))});
           k.done(f);
       });
    lc("arr_ptr_elems_end", "the same storing the pointer into a[2]: out-of-bounds at the store",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* x {k.cell(f, "x", k.i32)};
           Var* a {arr(k, f, "a", PI(k), 2U)};
           const Stmt* w {wr(k, a, N(k, "2"), k.addr(x))};
           f->body = k.blkc({k.decl(x, N(k, "1")), k.decl(a, nullptr), w});
           k.done(f);
           k.fail("out-of-bounds", w->loc);
       });
}

} // namespace
