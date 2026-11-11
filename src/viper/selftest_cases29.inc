// (included by selftest.cpp) Arrays inside one function, M4.5e1a Part B's lowering (decision 0059 items 2, 3, 4, 6, 7 and 9): an automatic array local is one block
// (`alloc_arr_<t>(N)`, with `zero_arr_<t>(r, N)` at the declaration of an initializer that does not write every element), an element `a[i]` is `Deref(ElemAddr(AddrOf(a), i))`
// lowered to `loc(block_of(a), offset_of(a) + i)` with the bound row `out-of-bounds` (a place access: `0 <= offset + i < len`; an address as a value: `<= len`) before the
// permission and the flag rows, the scope exit is two marked range rows, the liveness row and three `exhale`s (item 6), and two elements accessed in one full expression, one of
// them written, get 0039 item 3's `unsequenced` row (item 9), as an element beside a `*p` into the same array does.
// Each rule has a case that verifies and a twin that fails for one reason (invariant 2); a failure's category and the VIR node whose position the record gives are the
// expectation (the element's read for `uninitialized` and for a read's bound, the store for a write's, the address for `&a[i]`, the block's closing brace for the scope exit,
// the store for `unsequenced`).  `fail_only(c, loc, n, i)`: n checks of that category at that line, only the i-th (in Viper line order) fails.
namespace {

void define_array_loops();
void define_array_pointers();

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
    auto arr =
        [](Kit& k, Function* f, const char* name, Type element, unsigned long long count, bool zero_filled = false)
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
    lc("arr_read_unwritten",
       "the same program reading a[1], which nothing wrote: its flag is false (0019, path-precise)",
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
    lc("arr_write_index_end",
       "the same with 0 <= i <= 3: i == 3 is one past the end, never writable: out-of-bounds at the store",
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
    lc("arr_read_index_negative",
       "the same with -1 <= i: a[-1] is outside (the value, not a conversion of it, is the offset): out-of-bounds at the read",
       [=](Kit& k)
       {
           k.fail("out-of-bounds", read_index(k, "-1")->loc);
       });

    // ---- item 3: the zero fill of a braced initializer ----
    // void f() { int a[4]; <fill>; a[0] = 1; check(a[3] == [0|1] && a[0] == 1); }
    lc("arr_zero_fill_int",
       "int a[4] = {1}: zero_arr_int fills a[1..3] with 0 and sets their flags, then the store of a[0]: a[3] == 0 and a[0] == 1",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 4U, true)};
           f->body = k.blkc(
               {k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "1")),
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
    lc("arr_scope_exit_returned",
       "a callee that owns *p and gives it back (bump(&a[1])): the scope exit finds every element's permission, and a[1] == 6",
       [=](Kit& k)
       {
           Function* bump {make_bump(k)};
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 3U)};
           f->body =
               k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "1"), N(k, "5")), k.es(k.call(bump, {at_(k, a, N(k, "1"))})),
                       k.chk(k.eq_c(rd(k, a, N(k, "1")), k.i32, "6"))});
           k.done(f);
       });
    lc("arr_scope_exit_kept",
       "a callee that takes the permission to a[1] and keeps it (eat(&a[1])): the marked range row at the closing brace fails, and only it (lifetime)",
       [=](Kit& k)
       {
           Function* eat {make_eat(k)};
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 3U)};
           const Stmt* body {
               k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "1"), N(k, "5")), k.es(k.call(eat, {at_(k, a, N(k, "1"))}))})};
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
    lc("arr_return_in_block",
       "a return inside a nested block leaves both blocks, the inner one first: each array's rows and exhales, then the postcondition r == 7",
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
    lc("arr_addr_value",
       "id(&a[i]) for 0 <= i <= 3: the address one past the end of the three-element array may be formed",
       [=](Kit& k)
       {
           addr_value(k, "3");
       });
    lc("arr_addr_value_beyond",
       "the same with 0 <= i <= 4: &a[4] is more than one past the end: out-of-bounds at the address",
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
        const Stmt* s {
            k.store(at_(k, a, ix(k, f, 0)), k.b.inc_dec_store_expr(at_(k, a, ix(k, f, 1)), true, true, k.at()))};
        f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "0")), wr(k, a, N(k, "1"), N(k, "0")),
                          wr(k, a, N(k, "2"), N(k, "0")), s});
        k.done(f);
        return s;
    };
    lc("arr_seq_distinct",
       "a[i] = a[j]++ with i != j: two elements of one array, the write and the increment are unsequenced and never one object",
       [=](Kit& k)
       {
           seq_elems(k, true);
       });
    lc("arr_seq_alias",
       "the same without i != j: they may be one element, so the two writes are unsequenced on one object (unsequenced, at the store)",
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
    lc("arr_seq_pointer",
       "int *p = &a[1]; *p = a[i]++ with i != 1: a write through a pointer into the array beside an element's increment, never one object",
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
                          k.cstore(at_(k, a, N(k, "1")), BinaryOp::Add, N(k, "2")),
                          k.idstore(at_(k, a, N(k, "1")), true), c});
        k.done(f);
        return c;
    };
    lc("arr_compound_const",
       "a[1] = 5; a[1] += 2; a[1]++: a[1] == 8 (the read and the write of a compound store are one element)",
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
    lc("arr_ptr_elems",
       "int *a[2]: a[0] = &x; *a[0] = 3: a pointer element is a cell of the pointee's field, and the pointer read back is &x",
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

    define_array_loops();
    define_array_pointers();
}

// ---- Part D: a pointer base (`p[i]`, `&p[i]`, the decay), decision 0059 items 4 and 10 ----
void define_array_pointers()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto arr =
        [](Kit& k, Function* f, const char* name, Type element, unsigned long long count, bool zero_filled = false)
    {
        Var* x {f->new_local(name, k.prog.array_of(element, count), k.at())};
        f->mark_address_taken(x);
        x->zero_filled = zero_filled;
        return x;
    };
    // `&a[i]` of a named array, `&p[i]` of a pointer expression, and the places they name
    auto at_ = [](Kit& k, const Var* a, const Expr* i)
    {
        return k.b.elem_addr(k.addr(a), i, k.at());
    };
    auto pat = [](Kit& k, const Expr* base, const Expr* i)
    {
        return k.b.elem_addr(base, i, k.at());
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto pown = [=](Kit& k, Function* f, int i, int amount)
    {
        return k.b.atom(AtomKind::Owns, fx(k, f, i), amount, k.at());
    };

    // int f() { int a[3] = {}; int *p = &a[<off>]; p[<i>] = 5; check(a[<off + i>] == 5); }   (the decay is off 0)
    const auto walk = [=](Kit& k, const char* off, const char* idx, const char* target) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U, true)};
        Var* p {k.local(f, "p", PI(k))};
        const Stmt* w {k.store(pat(k, k.pn(p), N(k, idx)), N(k, "5"))};
        f->body = k.blkc({k.decl(a, nullptr), k.decl(p, at_(k, a, N(k, off))), w,
                          k.chk(k.eq_c(k.deref(at_(k, a, N(k, target))), k.i32, "5"))});
        k.done(f);
        return w;
    };
    lc("arrp_walk",
       "int *p = &a[1]; p[1] = 5: the pointer base's offset is 1, so the store names a[2] (null, lifetime, bound, permission all hold)",
       [=](Kit& k)
       {
           walk(k, "1", "1", "2");
       });
    lc("arrp_walk_beyond",
       "the same with p[2]: offset 1 + 2 is the length of the block, so the store is outside (out-of-bounds at the store, and only there: the failed row is assumed, which ends this path)",
       [=](Kit& k)
       {
           const Stmt* w {walk(k, "1", "2", "0")};
           k.fail("out-of-bounds", w->loc);
       });
    // int g(int *p) requires(owns(*p)) ensures(owns(*p)) { int x = p[0]; }   and the twin p[1]
    const auto param_read = [=](Kit& k, const char* idx) -> const Expr*
    {
        Function* f {k.fn("g", k.vd, {{"p", PI(k)}})};
        k.req(f, pown(k, f, 0, 0));
        k.ens(f, pown(k, f, 0, 0));
        Var* x {k.local(f, "x", k.i32)};
        const Expr* r {k.deref(pat(k, k.pn(f->param_var(0)), N(k, idx)))};
        f->body = k.blk({k.decl(x, r)});
        k.done(f);
        return r;
    };
    lc("arrp_param_first",
       "p[0] of a parameter that owns *p: the atom carries offset_of(p) < len(block_of(p)), so the bound holds",
       [=](Kit& k)
       {
           param_read(k, "0");
       });
    lc("arrp_param_second",
       "p[1] of the same parameter: the object may end at p (out-of-bounds) and the callee holds no permission to p[1] (permission): two real cases, two errors at the read",
       [=](Kit& k)
       {
           const Expr* r {param_read(k, "1")};
           k.fail("out-of-bounds", r->loc);
           k.fail("permission", r->loc);
       });
    // void f(int *p) { int *q = &p[<i>]; }  p a pointer that may be null: the address of an element of a null pointer
    lc("arrp_null_base",
       "int *q = &p[0] for a pointer parameter with no contract: p may be null (null-dereference at the address) or dangle (lifetime)",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}})};
           Var* q {k.local(f, "q", PI(k))};
           const Expr* ea {pat(k, k.pn(f->param_var(0)), N(k, "0"))};
           f->body = k.blk({k.decl(q, ea)});
           k.done(f);
           k.fail("null-dereference", ea->loc);
           // a non-null p of no contract may also point at an object that ended: two lifetime rows stand at the address (p's own use, then the copied value's), the first fails
           k.fail_only("lifetime", ea->loc, 2U, 0U);
       });
    // int *q = &p[<i>] one past the end is formed, beyond is not
    const auto addr_of = [=](Kit& k, const char* idx) -> const Expr*
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U, true)};
        Var* p {k.local(f, "p", PI(k))};
        Var* q {k.local(f, "q", PI(k))};
        const Expr* ea {pat(k, k.pn(p), N(k, idx))};
        f->body = k.blkc({k.decl(a, nullptr), k.decl(p, at_(k, a, N(k, "1"))), k.decl(q, ea)});
        k.done(f);
        return ea;
    };
    lc("arrp_addr_one_past", "p = &a[1]; q = &p[2]: the address one past the end of the three-element array is formed",
       [=](Kit& k)
       {
           addr_of(k, "2");
       });
    lc("arrp_addr_beyond", "the same with &p[3]: more than one past the end (out-of-bounds, at the address)",
       [=](Kit& k)
       {
           k.fail("out-of-bounds", addr_of(k, "3")->loc);
       });
}

} // namespace

// Loops over arrays (defined here, called by define_arrays_lowering), M4.5e1a Part C's lowering (decision 0059 item 8): each array local of an open scope is a range place of a loop that carries its
// places: a marked entry row per field and the liveness snapshot, the label `lh_<loop>`, the conjuncts (full permission over `[0, N)` and the liveness, the flags kept through
// `old[lh]`, the type's range, and the values kept when no write in the loop can reach the array), a marked back-edge row per field, and reads of `a[e]` in invariants framed by the
// range place (an index outside it: `out-of-bounds`, the flag: `uninitialized`).  An array declared in a loop body is allocated per iteration.
// Each rule has a case that verifies and a twin that fails for one reason (invariant 2); the expectations name the category and the VIR node whose position the record gives (the
// loop for an entry row, the closing brace of the body for a back-edge row, the invariant clause for a conjunct the loop does not keep, the read for `uninitialized`).
namespace {

void define_array_loops()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto arr =
        [](Kit& k, Function* f, const char* name, Type element, unsigned long long count, bool zero_filled = false)
    {
        Var* x {f->new_local(name, k.prog.array_of(element, count), k.at())};
        f->mark_address_taken(x);
        x->zero_filled = zero_filled;
        return x;
    };
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
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto pown = [=](Kit& k, Function* f, int i, int amount)
    {
        return k.b.atom(AtomKind::Owns, fx(k, f, i), amount, k.at());
    };
    // `void eat(int *p) requires(owns(*p))`: bodyless, the callee keeps what it takes
    auto make_eat = [=](Kit& k) -> Function*
    {
        Function* f {k.fn("eat", k.vd, {{"p", PI(k)}})};
        k.req(f, pown(k, f, 0, 0));
        k.trusted(f);
        return f;
    };
    auto inc = [](Kit& k, Var* i)
    {
        return k.asg(i, k.add(k.v(i), k.num(k.i32, "1")));
    };

    // ---- the fill loop; the flags kept ----
    // void f() { int a[3] = {0}; | int a[3]; int i = 0; while (i < 3) inv 0 <= i <= 3 measure 3 - i { a[i] = i; i++; } int x = a[1]; }
    const auto fill = [=](Kit& k, bool zero) -> const Expr*
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U, zero)};
        Var* i {k.local(f, "i", k.i32)};
        Var* x {k.local(f, "x", k.i32)};
        const Stmt* body {k.blkc({wr(k, a, k.v(i), k.v(i)), inc(k, i)})};
        const Expr* r {rd(k, a, N(k, "1"))};
        f->body = k.blkc({k.decl(a, nullptr), k.decl(i, N(k, "0")),
                          k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                {k.cl(k.sub(N(k, "3"), k.v(i)))}, body),
                          k.decl(x, r)});
        k.done(f);
        return r;
    };
    lc("arrl_fill",
       "int a[3] = {}; a[i] = i in a loop: the flags set before the loop are kept (old[lh]), so a[1] is readable after it",
       [=](Kit& k)
       {
           fill(k, true);
       });
    lc("arrl_fill_unfilled",
       "the same loop over int a[3] with no initializer: a loop that fills the array cannot yet say which elements it wrote, so the read is uninitialized (pinned)",
       [=](Kit& k)
       {
           k.fail("uninitialized", fill(k, false)->loc);
       });

    // ---- the values kept, and the reach rule ----
    // `mode` 0: read-only; 1: the body also writes b[i] (another array: does not reach a); 2: the body writes *p = 1 for p = &a[1] (reaches a)
    // void f() { int a[3] = {7, 7, 7}; int b[3] = {}; int *p = &a[1]; long s = 0; int i = 0;
    //            while (i < 3) inv 0 <= i <= 3 && s == 7 * i { s = s + a[i]; [b[i] = 1;] [*p = 1;] i++; } check(s == 21); }
    const auto sum = [=](Kit& k, int mode) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U, false)};
        Var* b {arr(k, f, "b", k.i32, 3U, true)};
        Var* p {k.local(f, "p", PI(k))};
        Var* s {k.local(f, "s", k.i64)};
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> bs {k.asg(s, k.add(k.v(s), k.cv(k.i64, rd(k, a, k.v(i)))))};
        if (mode == 1)
        {
            bs.push_back(wr(k, b, k.v(i), N(k, "1")));
        }
        if (mode == 2)
        {
            bs.push_back(k.store(k.pn(p), N(k, "1")));
        }
        bs.push_back(inc(k, i));
        const Expr* bound {k.range(k.v(i), k.i32, "0", "3")};
        const Clause kept {k.cl(k.eq(k.v(s), k.mul(k.cv(k.i64, N(k, "7")), k.cv(k.i64, k.v(i)))))};
        const Stmt* c {k.chk(k.eq_c(k.v(s), k.i64, "21"))};
        f->body = k.blkc(
            {k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "7")), wr(k, a, N(k, "1"), N(k, "7")),
             wr(k, a, N(k, "2"), N(k, "7")), k.decl(b, nullptr), k.decl(p, at_(k, a, N(k, "1"))),
             k.decl(s, k.num(k.i64, "0")), k.decl(i, N(k, "0")),
             k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(bound), kept}, {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(bs)), c});
        k.done(f);
        if (mode == 2)
        {
            k.fail("invariant-preserved", kept.loc);
        }
        return c;
    };
    lc("arrl_sum_read_only",
       "a read-only loop over a[3] = {7, 7, 7}: the values are kept (forall k :: a[k] == old[lh](a[k])), so s == 7 * i holds and s == 21 after the loop",
       [=](Kit& k)
       {
           sum(k, 0);
       });
    lc("arrl_sum_other_array_written",
       "the body also writes b[i] of another array: it does not reach a (a store by name to another array), s == 7 * i still holds",
       [=](Kit& k)
       {
           sum(k, 1);
       });
    lc("arrl_sum_pointer_write",
       "the body also writes *p for p = &a[1]: a store through a pointer of the cell field reaches a, its values are not kept, s == 7 * i is not preserved",
       [=](Kit& k)
       {
           sum(k, 2);
       });

    // ---- reads of a[e] in an invariant: the bound ----
    // void f() { int a[3] = {7, 7, 7}; int i = 0; while (i < 3) inv 0 <= i <= 3 && [i < 3 ==> ]a[i] == 7 { i++; } }
    const auto inv_read = [=](Kit& k, bool guarded) -> const Expr*
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U, false)};
        Var* i {k.local(f, "i", k.i32)};
        const Expr* r {rd(k, a, k.pn(i))};
        const Expr* seven {k.eq_c(r, k.i32, "7")};
        const Expr* body_inv {guarded ? k.implies(k.lt(k.v(i), N(k, "3")), seven) : seven};
        f->body = k.blkc({k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "7")), wr(k, a, N(k, "1"), N(k, "7")),
                          wr(k, a, N(k, "2"), N(k, "7")), k.decl(i, N(k, "0")),
                          k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3")), k.cl(body_inv)},
                                {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({inc(k, i)}))});
        k.done(f);
        return r;
    };
    lc("arrl_invariant_read_guarded",
       "i < 3 ==> a[i] == 7 as a loop invariant: the read is framed by the range place and in bounds under its guard",
       [=](Kit& k)
       {
           inv_read(k, true);
       });
    lc("arrl_invariant_read_unguarded",
       "a[i] == 7 as a loop invariant while i reaches 3: the index is outside the array (out-of-bounds, at the read)",
       [=](Kit& k)
       {
           // the clause's two checks (entry, preserved) stand at the read; the entry holds (i == 0), the second fails
           k.fail_only("out-of-bounds", inv_read(k, false)->loc, 2U, 1U);
       });

    // ---- the entry row and the back-edge row ----
    // void f() { int a[3] = {}; [eat(&a[1]);] int i = 0; int x = 0; while (i < 3) inv 0 <= i <= 3 { x = a[0]; i++; } }
    lc("arrl_entry_kept",
       "an element kept by a callee before the loop: the loop begins without the permission to every element (the marked entry row, permission, at the loop)",
       [=](Kit& k)
       {
           Function* eat {make_eat(k)};
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 3U, true)};
           Var* i {k.local(f, "i", k.i32)};
           Var* x {k.local(f, "x", k.i32)};
           const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                 {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({k.asg(x, rd(k, a, N(k, "0"))), inc(k, i)}))};
           f->body = k.blkc({k.decl(a, nullptr), k.es(k.call(eat, {at_(k, a, N(k, "1"))})), k.decl(i, N(k, "0")),
                             k.decl(x, N(k, "0")), lp});
           k.done(f);
           // the two marked rows (the value field, then the flag field) stand at the loop; a failed one ends the path
           k.fail_only("permission", lp->loc, 2U, 0U);
       });
    lc("arrl_entry_kept_not", "the same loop with no call before it: the entry rows hold",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 3U, true)};
           Var* i {k.local(f, "i", k.i32)};
           Var* x {k.local(f, "x", k.i32)};
           f->body =
               k.blkc({k.decl(a, nullptr), k.decl(i, N(k, "0")), k.decl(x, N(k, "0")),
                       k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                             {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({k.asg(x, rd(k, a, N(k, "0"))), inc(k, i)}))});
           k.done(f);
       });
    // void f() { int a[3] = {}; int i = 0; while (i < 3) inv 0 <= i <= 3 { [eat(&a[i]);|touch(&a[i]);] i++; } }
    lc("arrl_back_edge_kept",
       "the body hands a[i] to a callee that keeps it: an iteration ends without the permission it began with (the marked back-edge row at the body's closing brace)",
       [=](Kit& k)
       {
           Function* eat {make_eat(k)};
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 3U, true)};
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* body {k.blkc({k.es(k.call(eat, {at_(k, a, k.v(i))})), inc(k, i)})};
           f->body = k.blkc({k.decl(a, nullptr), k.decl(i, N(k, "0")),
                             k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                   {k.cl(k.sub(N(k, "3"), k.v(i)))}, body)});
           k.done(f);
           // two marked rows and the liveness row stand at the brace; the first failed one ends the path
           k.fail_only("permission", body->end_loc, 3U, 0U);
       });
    lc("arrl_back_edge_returned",
       "the body hands a[i] to a callee (touch) that owns it and gives it back: every iteration ends with what it began with",
       [=](Kit& k)
       {
           // void touch(int *p) requires(owns(*p)) ensures(owns(*p)) { *p = 0; }
           Function* bump {k.fn("touch", k.vd, {{"p", PI(k)}})};
           k.req(bump, pown(k, bump, 0, 0));
           k.ens(bump, pown(k, bump, 0, 0));
           bump->body = k.blk({k.store(k.pv(bump, 0), N(k, "0"))});
           k.done(bump);
           Function* f {k.fn("f", k.vd)};
           Var* a {arr(k, f, "a", k.i32, 3U, true)};
           Var* i {k.local(f, "i", k.i32)};
           f->body = k.blkc(
               {k.decl(a, nullptr), k.decl(i, N(k, "0")),
                k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                      {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({k.es(k.call(bump, {at_(k, a, k.v(i))})), inc(k, i)}))});
           k.done(f);
       });

    // ---- an array declared in a loop body: allocated per iteration ----
    // void f() { int x = 0; int i = 0; while (i < 3) inv 0 <= i <= 3 { int b[2]; b[0] = i; [b[1] = 1;] x = b[[0|1]]; i++; } }
    const auto in_body = [=](Kit& k, bool read_unwritten) -> const Expr*
    {
        Function* f {k.fn("f", k.vd)};
        Var* i {k.local(f, "i", k.i32)};
        Var* x {k.local(f, "x", k.i32)};
        Var* b {arr(k, f, "b", k.i32, 2U)};
        const Expr* r {rd(k, b, N(k, read_unwritten ? "1" : "0"))};
        std::vector<const Stmt*> bs {k.decl(b, nullptr), wr(k, b, N(k, "0"), k.v(i))};
        if (!read_unwritten)
        {
            bs.push_back(wr(k, b, N(k, "1"), N(k, "1")));
        }
        bs.push_back(k.asg(x, r));
        bs.push_back(inc(k, i));
        f->body = k.blkc({k.decl(i, N(k, "0")), k.decl(x, N(k, "0")),
                          k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(bs))});
        k.done(f);
        return r;
    };
    lc("arrl_in_body",
       "int b[2] declared in the body, written and read in each iteration: allocated per iteration, exited at the body's end",
       [=](Kit& k)
       {
           in_body(k, false);
       });
    lc("arrl_in_body_fresh",
       "the body reads b[1], which it never writes: every iteration's array is a new block (the previous iteration's write is gone): uninitialized",
       [=](Kit& k)
       {
           k.fail("uninitialized", in_body(k, true)->loc);
       });

    // ---- a return from the loop leaves the array's scope ----
    // int f(int n) requires(0 <= n && n <= 3) ensures(r == [5|0]) { int a[3] = {}; int i = 0;
    //   while (i < n) inv 0 <= i <= n { if (i == 2) return a[0]; i++; } return 5; }
    const auto ret_in_loop = [=](Kit& k, bool wrong) -> const Stmt*
    {
        Function* f {k.fn("f", k.i32, {{"n", k.i32}})};
        k.req(f, k.land(k.ge_c(fx(k, f, 0), k.i32, "0"), k.le_c(fx(k, f, 0), k.i32, "3")));
        k.ens(f, wrong ? k.eq_c(k.R(f), k.i32, "5") : k.lor(k.eq_c(k.R(f), k.i32, "5"), k.eq_c(k.R(f), k.i32, "0")));
        Var* a {arr(k, f, "a", k.i32, 3U, true)};
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* early {k.ret(rd(k, a, N(k, "0")))};
        const Stmt* body {k.blkc({k.iff(k.eq(k.v(i), N(k, "2")), k.blk({early})), inc(k, i)})};
        f->body = k.blkc(
            {k.decl(a, nullptr), k.decl(i, N(k, "0")),
             k.whl(k.lt(k.v(i), k.pv(f, 0)), {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 0))))},
                   {k.cl(k.sub(k.pv(f, 0), k.v(i)))}, body),
             k.ret(N(k, "5"))});
        k.done(f);
        return early;
    };
    lc("arrl_return_in_loop",
       "a return inside the loop exits the array's scope with the permission the loop carries: r == 5 or r == 0",
       [=](Kit& k)
       {
           ret_in_loop(k, false);
       });
    lc("arrl_return_in_loop_wrong",
       "the same function claiming r == 5: the early return gives a[0] == 0 (postcondition, at that return)",
       [=](Kit& k)
       {
           k.fail("postcondition", ret_in_loop(k, true)->loc);
       });

    // ---- nested loops: the outer writes the array, the inner only reads it (0042 item 5) ----
    // void f() { int a[3] [= {}]; int s = 0; int i = 0; while (i < 3) inv 0 <= i <= 3 { a[i] = i; int j = 0;
    //            while (j < 3) inv 0 <= j <= 3 { s = a[j]; j++; } i++; } }
    const auto nested = [=](Kit& k, bool zero) -> const Expr*
    {
        Function* f {k.fn("f", k.vd)};
        Var* a {arr(k, f, "a", k.i32, 3U, zero)};
        Var* s {k.local(f, "s", k.i32)};
        Var* i {k.local(f, "i", k.i32)};
        Var* j {k.local(f, "j", k.i32)};
        const Expr* r {rd(k, a, k.v(j))};
        const Stmt* inner {k.whl(k.lt(k.v(j), N(k, "3")), {k.cl(k.range(k.v(j), k.i32, "0", "3"))},
                                 {k.cl(k.sub(N(k, "3"), k.v(j)))}, k.blkc({k.asg(s, r), inc(k, j)}))};
        const Stmt* outer {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                 {k.cl(k.sub(N(k, "3"), k.v(i)))},
                                 k.blkc({wr(k, a, k.v(i), k.v(i)), k.asg(j, N(k, "0")), inner, inc(k, i)}))};
        f->body = k.blkc({k.decl(a, nullptr), k.decl(s, N(k, "0")), k.decl(i, N(k, "0")), k.decl(j, N(k, "0")), outer});
        k.done(f);
        return r;
    };
    lc("arrl_nested",
       "the outer loop writes a[i], the inner loop only reads a[j]: the inner loop keeps the outer loop's flags (old[lh] of its own entry state)",
       [=](Kit& k)
       {
           nested(k, true);
       });
    lc("arrl_nested_unfilled",
       "the same without the zero fill: the inner read of a[j] is of an element nothing says was written (uninitialized)",
       [=](Kit& k)
       {
           k.fail("uninitialized", nested(k, false)->loc);
       });

    // ---- a loop before the array's declaration in its own block: the block allocated the array at its entry, so the loop carries it (flags false) ----
    // void f() { int i = 0; while (i < 3) inv 0 <= i <= 3 { i++; } int a[3]; a[0] = 1; int x = a[[0|1]]; }
    const auto before_decl = [=](Kit& k, bool read_unwritten) -> const Expr*
    {
        Function* f {k.fn("f", k.vd)};
        Var* i {k.local(f, "i", k.i32)};
        Var* x {k.local(f, "x", k.i32)};
        Var* a {arr(k, f, "a", k.i32, 3U)};
        const Expr* r {rd(k, a, N(k, read_unwritten ? "1" : "0"))};
        f->body = k.blkc({k.decl(i, N(k, "0")),
                          k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({inc(k, i)})),
                          k.decl(a, nullptr), wr(k, a, N(k, "0"), N(k, "1")), k.decl(x, r)});
        k.done(f);
        return r;
    };
    lc("arrl_before_declaration",
       "a loop that stands before the declaration of int a[3] in its block: the array is already allocated (unwritten) and the loop carries it; a[0] is readable after its write",
       [=](Kit& k)
       {
           before_decl(k, false);
       });
    lc("arrl_before_declaration_unwritten",
       "the same reading a[1] after the declaration: the loop kept the flags false, so the element is uninitialized",
       [=](Kit& k)
       {
           k.fail("uninitialized", before_decl(k, true)->loc);
       });

    // ---- a parameter whose atom can be false, beside an array in the loop (decision 0038 item 3, audit round 2 B1, for arrays) ----
    // void f(int c, int *p) requires(c == 0 || owns(*p)) ensures(c == 0 || owns(*p))
    // { int a[3] = {}; int i = 0; while (i < 3) inv 0 <= i <= 3 { a[i] = i; [if (c != 0) { *p = 5; }|*p = 5;] i++; } }
    const auto alias_guard = [=](Kit& k, bool guarded) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd, {{"c", k.i32}, {"p", PI(k)}})};
        const auto cond = [&]()
        {
            return k.lor(k.eq_c(fx(k, f, 0), k.i32, "0"), pown(k, f, 1, 0));
        };
        k.req(f, cond());
        k.ens(f, cond());
        Var* a {arr(k, f, "a", k.i32, 3U, true)};
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* w {k.store(k.pn(f->param_var(1)), N(k, "5"))};
        const Stmt* guarded_w {k.iff(k.ne(k.pv(f, 0), N(k, "0")), k.blk({w}))};
        f->body = k.blkc(
            {k.decl(a, nullptr), k.decl(i, N(k, "0")),
             k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))}, {k.cl(k.sub(N(k, "3"), k.v(i)))},
                   k.blkc({k.store(at_(k, a, k.v(i)), k.v(i)), guarded ? guarded_w : w, inc(k, i)}))});
        k.done(f);
        return w;
    };
    lc("arrl_param_alias_guarded",
       "an array in a loop beside a parameter that owns *p only when c != 0: the parameter's place is guarded against the array's block and liveness cell, so neither snapshot is taken twice, and *p = 5 under c != 0 holds",
       [=](Kit& k)
       {
           alias_guard(k, true);
       });
    lc("arrl_param_alias_unguarded",
       "the same with *p = 5 on every path: on the path with c == 0 the function holds nothing of p (permission, and null-dereference), never an internal row",
       [=](Kit& k)
       {
           const Stmt* w {alias_guard(k, false)};
           k.fail("permission", w->loc);
           k.fail("null-dereference", w->loc);
       });

    // ---- element types: a pointer element's range is WF (the spike S43 addendum) ----
    // void f() { int x = 1; int *a[3] = {}; int i = 0; while (i < 3) inv 0 <= i <= 3 { a[i] = &x; i++; } }   (twin: i <= 3 reaches a[3])
    const auto ptr_fill = [=](Kit& k, bool beyond) -> const Stmt*
    {
        Function* f {k.fn("f", k.vd)};
        Var* x {k.cell(f, "x", k.i32)};
        Var* a {arr(k, f, "a", PI(k), 3U, true)};
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* w {wr(k, a, k.v(i), k.addr(x))};
        f->body = k.blkc({k.decl(x, N(k, "1")), k.decl(a, nullptr), k.decl(i, N(k, "0")),
                          k.whl(k.le(k.v(i), N(k, beyond ? "3" : "2")), {k.cl(k.range(k.v(i), k.i32, "0", "4"))},
                                {k.cl(k.sub(N(k, "4"), k.v(i)))}, k.blkc({w, inc(k, i)}))});
        k.done(f);
        return w;
    };
    lc("arrl_ptr_fill",
       "a loop storing &x into every element of int *a[3]: the WF conjunct of a pointer element is kept",
       [=](Kit& k)
       {
           ptr_fill(k, false);
       });
    lc("arrl_ptr_fill_beyond",
       "the same loop running to i == 3: the store is outside the array (out-of-bounds, at the store)",
       [=](Kit& k)
       {
           k.fail("out-of-bounds", ptr_fill(k, true)->loc);
       });
}

} // namespace
