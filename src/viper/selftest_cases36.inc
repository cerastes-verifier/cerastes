// (included by selftest.cpp) M4.5e1e2 continuation, Part R (record 0064 item 19, question 326): cerastes does not model `restrict`.  The encoder refuses a function that has a `restrict` parameter at the
// C line, so a program never reaches check_program with the flag set; check_program refuses any parameter that carries it, as a backstop.  Each case is one of the shapes the retired rules accepted
// (a cell atom, a range atom, a declaration with no body) and must now be refused, the diagnostic naming the parameter.
namespace {

void define_restrict_backstop_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    // int f(int *restrict p) requires(owns(*p)) ensures(owns(*p)) { return 0; }: backed as the retired rule wanted, and refused
    rc("e1e2r_backstop_restrict_backed_by_a_cell", "parameter 1 of 'f' is `restrict`, which cerastes does not model",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32, {{"p", PI(k)}})};
           f->params[0U].is_restrict = true;
           k.req(f, k.b.atom(AtomKind::Owns, fx(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fx(k, f, 0), 0, k.at()));
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // int f(int *restrict p, int n) requires(owns_range(p, n) && n > 0) ensures(owns_range(p, n)) { return 0; }
    rc("e1e2r_backstop_restrict_backed_by_a_range", "parameter 1 of 'f' is `restrict`, which cerastes does not model",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.i32, {{"p", PI(k)}, {"n", k.i32}})};
           f->params[0U].is_restrict = true;
           k.req(f, k.land(k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()),
                           k.gt(fx(k, f, 1), N(k, "0"))));
           k.ens(f, k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()));
           f->body = k.blk({k.ret(N(k, "0"))});
           k.done(f);
       });
    // int f(int *p, int *restrict q) with no contract and no body (a declaration the unit defines elsewhere): the second parameter, not the first, is named
    rc("e1e2r_backstop_restrict_on_a_second_parameter_of_a_declaration",
       "parameter 2 of 'f' is `restrict`, which cerastes does not model",
       [=](Kit& k)
       {
           Function* f {k.fn("f", k.vd, {{"p", PI(k)}, {"q", PI(k)}})};
           f->params[1U].is_restrict = true;
           k.trusted(f);
           k.done(f);
       });
}

// ---- Part C1 (record 0064 items 7, 8 and 10): ranges over structures ----
// Slice 1: the contract of one function, `owns_range` and `reads_range` over a verified structure, rows per leaf path; a call of such a function and a loop in it are refused by
// check_program until the call rows and the range places are lifted.
void define_struct_range_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    // struct P { int x; int y; }
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"x", k.i32}, {"y", k.i32}});
    };
    auto RO = [=](Kit& k, Function* f, int amount)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), amount, k.at());
    };
    auto RR = [=](Kit& k, Function* f, int amount)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 0), fx(k, f, 1), amount, k.at());
    };
    auto pplace = [](Kit& k, const Expr* base, const char* i, int m)
    {
        return k.b.member_addr(k.b.elem_addr(base, k.num(k.i32, i), k.at()), m, k.at());
    };
    // int f(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && r == 7) { p[<idx>].x = 7; return p[<idx>].x; }
    const auto write_read = [=](Kit& k, const char* idx) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
        k.ens(f, k.land(RO(k, f, 0), k.eq(k.R(f), N(k, "7"))));
        const Stmt* st {k.store(pplace(k, k.pv(f, 0), idx, 0), N(k, "7"))};
        f->body = k.blkc({st, k.ret(k.deref(pplace(k, k.pv(f, 0), idx, 0)))});
        k.done(f);
        return st->loc;
    };
    // the twins: an index the count does not reach, a write under a read permission, a wrong postcondition
    lc("e1e2c1_range_beyond_the_count_fail",
       "p[2].x with n > 1 only: the bound row fails first (out-of-bounds), and the cell is held by no atom, so the write row fails too (permission), both at the store",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, RO(k, f, 0));
           const Stmt* st {k.store(pplace(k, k.pv(f, 0), "2", 0), N(k, "7"))};
           f->body = k.blkc({st});
           k.done(f);
           k.fail("out-of-bounds", st->loc);
           k.fail("permission", st->loc);
       });
    lc("e1e2c1_range_write_under_reads_fail", "reads_range(p, n): p[1].x written: the write row (permission)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, RR(k, f, 0));
           const Stmt* st {k.store(pplace(k, k.pv(f, 0), "1", 1), N(k, "7"))};
           f->body = k.blkc({st});
           k.done(f);
           k.fail("permission", st->loc);
       });
    lc("e1e2c1_range_wrong_postcondition_fail",
       "owns_range(p, n): p[1].x = 7 returned, ensures r == 8: the postcondition at the return",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           const Expr* wrong {k.eq(k.R(f), N(k, "8"))};
           k.ens(f, k.land(RO(k, f, 0), wrong));
           const Stmt* ret {k.ret(k.deref(pplace(k, k.pv(f, 0), "1", 0)))};
           f->body = k.blkc({k.store(pplace(k, k.pv(f, 0), "1", 0), N(k, "7")), ret});
           k.done(f);
           k.fail("postcondition", ret->loc);
       });
    // ---- more shapes of the contract ----
    // struct Q { int a; struct T { int b; int c; } t; }: a leaf of depth 2 (`p[1].t.c`), the receiver `loc(member(loc(member(loc(block_of(p), k), 1), 0), 1), 0)`
    lc("e1e2c1_range_nested_leaf",
       "owns_range(p, n) over struct Q with a nested struct member: p[1].t.c written and read back through its own leaf path",
       [=](Kit& k)
       {
           StructType* T {k.strukt("T", {{"b", k.i32}, {"c", k.i32}})};
           StructType* Q {k.strukt("Q", {{"a", k.i32}, {"t", k.st(T)}})};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(Q))}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, k.land(RO(k, f, 0), k.eq(k.R(f), N(k, "9"))));
           auto leaf = [&]()
           {
               return k.b.member_addr(k.b.member_addr(k.b.elem_addr(k.pv(f, 0), N(k, "1"), k.at()), 1, k.at()), 1,
                                      k.at());
           };
           f->body = k.blkc({k.store(leaf(), N(k, "9")), k.ret(k.deref(leaf()))});
           k.done(f);
       });
    // struct R { int *q; int y; }: a pointer leaf (its range row is the pointer's well-formedness)
    lc("e1e2c1_range_pointer_leaf",
       "owns_range(p, n) over struct R { int *q; int y; }: p[1].y written, the pointer leaf's range is a conjunct of the atom",
       [=](Kit& k)
       {
           StructType* R {k.strukt("R", {{"q", k.ptr(k.i32)}, {"y", k.i32}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, RO(k, f, 0));
           f->body = k.blkc({k.store(pplace(k, k.pv(f, 0), "1", 1), N(k, "3"))});
           k.done(f);
       });
    // requires(p == NULL || owns_range(p, n)) ensures(the same): the atom under its null guard, nothing is accessed
    lc("e1e2c1_range_null_guard", "p == NULL || owns_range(p, n) in both lists, an empty body",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.lor(k.eq(fx(k, f, 0), k.nul(k.ptr(k.st(P)))), RO(k, f, 0)));
           k.ens(f, k.lor(k.eq(fx(k, f, 0), k.nul(k.ptr(k.st(P)))), RO(k, f, 0)));
           f->body = k.blkc({});
           k.done(f);
       });
    // two reads_range of one structure type (the exception of the checker's neighbour rule): both read
    lc("e1e2c1_range_two_reads_of_one_type",
       "reads_range(p, n) && reads_range(q, m) over struct P, n > 0 && m > 0: q[0].y evaluated and p[0].x returned",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.st(P))}, {"m", k.i32}})};
           auto rr = [&](int pi, int ni, int amount)
           {
               return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, pi), fx(k, f, ni), amount, k.at());
           };
           k.req(f, k.land(k.land(rr(0, 1, 0), rr(2, 3, 1)),
                           k.land(k.gt(fx(k, f, 1), N(k, "0")), k.gt(fx(k, f, 3), N(k, "0")))));
           k.ens(f, k.land(rr(0, 1, 0), rr(2, 3, 1)));
           f->body =
               k.blkc({k.es(k.deref(pplace(k, k.pv(f, 2), "0", 1))), k.ret(k.deref(pplace(k, k.pv(f, 0), "0", 0)))});
           k.done(f);
       });
    // two owned ranges over structures with no leaf field in common (int and long): both written
    lc("e1e2c1_range_two_owns_no_common_field",
       "owns_range(p, n) over struct P { int x; int y; } && owns_range(q, m) over struct L { long l; }: p[0].x and q[0].l written",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           StructType* L {k.strukt("L", {{"l", k.i64}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.st(L))}, {"m", k.i32}})};
           auto ro = [&](int pi, int ni, int amount)
           {
               return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, pi), fx(k, f, ni), amount, k.at());
           };
           k.req(f, k.land(k.land(ro(0, 1, 0), ro(2, 3, 1)),
                           k.land(k.gt(fx(k, f, 1), N(k, "0")), k.gt(fx(k, f, 3), N(k, "0")))));
           k.ens(f, k.land(ro(0, 1, 0), ro(2, 3, 1)));
           f->body = k.blkc(
               {k.store(pplace(k, k.pv(f, 0), "0", 0), N(k, "1")),
                k.store(k.b.member_addr(k.b.elem_addr(k.pv(f, 2), N(k, "0"), k.at()), 0, k.at()), k.num(k.i64, "2"))});
           k.done(f);
       });
    // ---- what check_program keeps refusing until the call rows, the range places and the pairing rows are lifted ----
    // ---- slice 2a: calls and returns (record 0064 item 8; the call rows per leaf path; no pairing yet) ----
    // struct P { int x; int y; }; int g(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && r == 7) { p[1].x = 7; return p[1].x; }
    auto mk_g = [=](Kit& k, StructType* P, bool owns, int gcount) -> Function*
    {
        Function* g {k.fn("g", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        const AtomKind kind {owns ? AtomKind::OwnsRange : AtomKind::ReadsRange};
        auto atom = [&](int amount)
        {
            return k.b.range_atom(kind, fx(k, g, 0), fx(k, g, 1), amount, k.at());
        };
        k.req(g, k.land(atom(0), k.gt(fx(k, g, 1), N(k, std::to_string(gcount - 1).c_str()))));
        k.ens(g, k.land(atom(0), k.eq(k.R(g), N(k, "7"))));
        if (owns)
        {
            g->body = k.blkc(
                {k.store(pplace(k, k.pv(g, 0), "1", 0), N(k, "7")), k.ret(k.deref(pplace(k, k.pv(g, 0), "1", 0)))});
        }
        else
        {
            g->body = k.blkc({k.ret(N(k, "7"))});
        }
        k.done(g);
        return g;
    };
    // int f(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && r == <want>) { return g(p, n); }
    const auto forward = [=](Kit& k, const char* want, bool owns_in, bool owns_callee) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {mk_g(k, P, owns_callee, 2)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        const AtomKind kind {owns_in ? AtomKind::OwnsRange : AtomKind::ReadsRange};
        auto atom = [&](int amount)
        {
            return k.b.range_atom(kind, fx(k, f, 0), fx(k, f, 1), amount, k.at());
        };
        k.req(f, k.land(atom(0), k.gt(fx(k, f, 1), N(k, "1"))));
        k.ens(f, k.land(atom(0), k.eq(k.R(f), N(k, want))));
        const Expr* c {k.call(g, {k.pv(f, 0), k.pv(f, 1)})};
        const Stmt* ret {k.ret(c)};
        f->body = k.blkc({ret});
        k.done(f);
        return std::string(want) == "7" ? c->loc : ret->loc;
    };
    lc("e1e2c1_call_forward",
       "f(p, n) forwards its owns_range(p, n) over struct P to g, whose contract writes p[1].x: the call rows per leaf path, the callee's postcondition r == 7 reaches f's",
       [=](Kit& k)
       {
           forward(k, "7", true, true);
       });
    lc("e1e2c1_call_forward_wrong_postcondition_fail",
       "the same, f's ensures claims r == 8: the postcondition at f's return",
       [=](Kit& k)
       {
           k.fail("postcondition", forward(k, "8", true, true));
       });
    lc("e1e2c1_call_reads_callee_reads",
       "f holds reads_range(p, n) over struct P and calls g, which reads it: the amounts are fractions of the caller's own",
       [=](Kit& k)
       {
           forward(k, "7", false, false);
       });
    lc("e1e2c1_call_owns_needed_reads_held_fail",
       "f holds reads_range(p, n) and calls g, which needs to own it: the marked rows at write permission fail (permission), the call's row",
       [=](Kit& k)
       {
           const SrcLoc at {forward(k, "7", false, true)};
           // a failed marked row ends its path (S43 G2): Silicon reports the first, the value permission of the member `x`
           k.fail_only("permission", at, 12U, 5U);
       });
    // void f(void) { struct P v[<len>] = {0} or no initializer; int r = g(&v[0], <n>); check(r == 7); }
    const auto local_call = [=](Kit& k, unsigned len, const char* n, bool zero) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {mk_g(k, P, true, 2)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), len), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = zero;
        auto first = [&]()
        {
            return k.b.elem_addr(k.addr(v), N(k, "0"), k.at());
        };
        const Expr* c {k.call(g, {first(), N(k, n)})};
        Var* r {k.local(f, "r", k.i32)};
        const Stmt* chk {k.chk(k.eq(k.v(r), N(k, "7")))};
        f->body = k.blkc({k.decl(v, nullptr), k.decl(r, c), chk});
        k.done(f);
        return c->loc;
    };
    lc("e1e2c1_call_local_array",
       "struct P v[3] = {0}; g(&v[0], 3): the range of the whole local array, then the result r == 7 is known",
       [=](Kit& k)
       {
           local_call(k, 3U, "3", true);
       });
    lc("e1e2c1_call_local_array_too_long_fail",
       "struct P v[3] = {0}; g(&v[0], 4): the shape row of the range (permission at the call)",
       [=](Kit& k)
       {
           k.fail_only("permission", local_call(k, 3U, "4", true), 12U, 0U);
       });
    lc("e1e2c1_call_local_array_unset_fail",
       "struct P v[3] with no initializer; g(&v[0], 3): no element has a value, the flag row of each leaf fails (uninitialized, x and y)",
       [=](Kit& k)
       {
           k.fail("uninitialized", local_call(k, 3U, "3", false), 2U);
       });
    lc("e1e2c1_call_owns_to_reads",
       "f owns the range, g only reads it: the callee's marked rows are fractions of the caller's write permission",
       [=](Kit& k)
       {
           // the caller holds the range (owns) and calls a reading callee (a fraction of write)
           StructType* P {mk_p(k)};
           Function* g {mk_g(k, P, false, 2)};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, k.land(RO(k, f, 0), k.eq(k.R(f), N(k, "7"))));
           f->body = k.blkc({k.ret(k.call(g, {k.pv(f, 0), k.pv(f, 1)}))});
           k.done(f);
       });
    // int h(struct P *p, int n) requires(owns_range(p, n) && n > 0) ensures(owns_range(p, n) && r == 1) { p[0].x = 1; return 1; }
    auto mk_h = [=](Kit& k, StructType* P) -> Function*
    {
        Function* h {k.fn("h", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(h, k.land(RO(k, h, 0), k.gt(fx(k, h, 1), N(k, "0"))));
        k.ens(h, k.land(RO(k, h, 0), k.eq(k.R(h), N(k, "1"))));
        h->body = k.blkc({k.store(pplace(k, k.pv(h, 0), "0", 0), N(k, "1")), k.ret(N(k, "1"))});
        k.done(h);
        return h;
    };
    // void f(void) { struct P v[3] = {0}; h(&v[0], <n>) == v[1].x; }: the read of v[1].x and the call are unsequenced; the call's range is [0, n)
    const auto seq_read = [=](Kit& k, const char* n) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* h {mk_h(k, P)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        auto first = [&]()
        {
            return k.b.elem_addr(k.addr(v), N(k, "0"), k.at());
        };
        const Expr* rd {k.deref(pplace(k, first(), "1", 0))};
        const Expr* c {k.call(h, {first(), N(k, n)})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(k.eq(c, rd))});
        k.done(f);
        return rd->loc;
    };
    lc("e1e2c1_seq_call_and_read_disjoint",
       "h(&v[0], 1) == v[1].x: the call's range is [0, 1), the read is element 1: the leaf's row passes",
       [=](Kit& k)
       {
           seq_read(k, "1");
       });
    lc("e1e2c1_seq_call_and_read_overlap_fail",
       "h(&v[0], 2) == v[1].x: the read is a leaf the call's range holds, in an order C leaves unspecified (unsequenced at the read)",
       [=](Kit& k)
       {
           // the read is of the leaf x: the row of the atom of leaf x fails, the row of leaf y holds
           k.fail_only("unsequenced", seq_read(k, "2"), 2U, 0U);
       });
    // void f(void) { struct P v[3] = {0}; h(&v[0], <n>) == h(&v[<m>], 1); }: two calls over one array
    const auto seq_calls = [=](Kit& k, const char* n, const char* at2) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* h {mk_h(k, P)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        auto at = [&](const char* i)
        {
            return k.b.elem_addr(k.addr(v), N(k, i), k.at());
        };
        const Expr* c1 {k.call(h, {at("0"), N(k, n)})};
        const Expr* c2 {k.call(h, {at(at2), N(k, "1")})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(k.eq(c1, c2))});
        k.done(f);
        return c2->loc;
    };
    lc("e1e2c1_seq_two_calls_disjoint",
       "h(&v[0], 1) == h(&v[2], 1): two ranges of one element type that share no slot: the slot row passes",
       [=](Kit& k)
       {
           seq_calls(k, "1", "2");
       });
    lc("e1e2c1_seq_two_calls_overlap_fail",
       "h(&v[0], 2) == h(&v[1], 1): the ranges share element 1: unsequenced at the later call",
       [=](Kit& k)
       {
           // per pair of leaf paths of one field: the pair (x, x) fails first and ends the path (Silicon reports one error), (y, y) would fail too, the mixed pairs are true
           k.fail_only("unsequenced", seq_calls(k, "2", "1"), 4U, 0U);
       });
    // trap 3 (a callee's writes through a range over structures are recorded per leaf field): a loop of a caller that keeps v[1].y == 0 across a call
    // void w(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n)) { p[1].y = 5; }
    // void f(void) { struct P v[3] = {0}; int i = 0; while (i < 2) invariant(0 <= i && i <= 2 && v[1].y == 0) decreases(2 - i) { [w(&v[0], 3);] i = i + 1; } }
    const auto loop_call = [=](Kit& k, bool with_call) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* w {k.fn("w", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(w, k.land(RO(k, w, 0), k.gt(fx(k, w, 1), N(k, "1"))));
        k.ens(w, RO(k, w, 0));
        w->body = k.blkc({k.store(pplace(k, k.pv(w, 0), "1", 1), N(k, "5"))});
        k.done(w);
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        Var* i {k.local(f, "i", k.i32)};
        auto first = [&]()
        {
            return k.b.elem_addr(k.addr(v), N(k, "0"), k.at());
        };
        const Expr* y1 {k.deref(pplace(k, k.addr(v), "1", 1))};
        const Stmt* call {k.es(k.call(w, {first(), N(k, "3")}))};
        std::vector<const Stmt*> inner;
        if (with_call)
        {
            inner.push_back(call);
        }
        inner.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Clause keep {k.cl(k.eq(y1, N(k, "0")))};
        const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "2")), {k.cl(k.range(k.v(i), k.i32, "0", "2")), keep},
                                {k.cl(k.sub(N(k, "2"), k.v(i)))}, k.blkc(inner))};
        f->body = k.blkc({k.decl(v, nullptr), k.decl(i, N(k, "0")), loop});
        k.done(f);
        return keep.loc;
    };
    lc("e1e2c1_loop_around_call_without_call",
       "the same loop without the call: v[1].y == 0 is kept (the contrast of the twin below)",
       [=](Kit& k)
       {
           loop_call(k, false);
       });
    lc("e1e2c1_loop_keeps_value_across_call_fail",
       "the loop calls w, which writes p[1].y through the range: v[1].y == 0 is not kept (the invariant fails at the back edge)",
       [=](Kit& k)
       {
           k.fail("invariant-preserved", loop_call(k, true));
       });
    // ---- slice 2b: the pairing rows of record 0064 item 7 (Form P), per pair of leaf paths of one field ----
    // void c2(struct P *p, int n, int *q) requires(<range>(p, n) && <cell>(*q) && n > 0) ensures(<range>(p, n) && <cell>(*q)) {}
    auto mk_c2 = [=](Kit& k, StructType* P, AtomKind range_kind, AtomKind cell_kind) -> Function*
    {
        Function* c {k.fn("c2", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
        auto ra = [&](int amount)
        {
            return k.b.range_atom(range_kind, fx(k, c, 0), fx(k, c, 1), amount, k.at());
        };
        auto ca = [&](int amount)
        {
            return k.b.atom(cell_kind, fx(k, c, 2), amount, k.at());
        };
        k.req(c, k.land(k.land(ra(0), ca(1)), k.gt(fx(k, c, 1), N(k, "0"))));
        k.ens(c, k.land(ra(0), ca(1)));
        c->body = k.blkc({});
        k.done(c);
        return c;
    };
    // void f(void) { struct P v[3] = {0}; int c = 0; c2(&v[0], <n>, <q>); }, q the local's address (qelem < 0) or &v[qelem].<qleaf>
    const auto pair_cell = [=](Kit& k, AtomKind range_kind, AtomKind cell_kind, const char* n, int qelem,
                               int qleaf) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* c2 {mk_c2(k, P, range_kind, cell_kind)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        Var* c {k.cell(f, "c", k.i32)};
        const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
        const Expr* q {qelem < 0 ? k.addr(c) : pplace(k, k.addr(v), std::to_string(qelem).c_str(), qleaf)};
        const Expr* call {k.call(c2, {first, N(k, n), q})};
        f->body = k.blkc({k.decl(v, nullptr), k.decl(c, N(k, "0")), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2c1_pair_cell_outside_range",
       "c2(&v[0], 2, &c) with c a separate local, owns_range(p, n) && owns(*q): the cell is outside the range's leaf cells of x and y: the rows pass",
       [=](Kit& k)
       {
           pair_cell(k, AtomKind::OwnsRange, AtomKind::Owns, "2", -1, 0);
       });
    lc("e1e2c1_pair_cell_in_a_later_element",
       "c2(&v[0], 2, &v[2].x): the cell is a leaf of element 2, outside the range [0, 2): the row of leaf x passes",
       [=](Kit& k)
       {
           pair_cell(k, AtomKind::OwnsRange, AtomKind::Owns, "2", 2, 0);
       });
    lc("e1e2c1_pair_cell_alias_fail",
       "c2(&v[0], 3, &v[1].x): the cell is leaf x of element 1, which the range holds: two rows (one per leaf), the row of leaf x fails (permission), leaf y's passes",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_cell(k, AtomKind::OwnsRange, AtomKind::Owns, "3", 1, 0), 2U, 0U,
                       "which may be one object");
       });
    lc("e1e2c1_pair_cell_alias_other_leaf_fail",
       "c2(&v[0], 3, &v[2].y): the same through leaf y: the row of leaf y (the second) fails, leaf x's passes",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_cell(k, AtomKind::OwnsRange, AtomKind::Owns, "3", 2, 1), 2U, 1U,
                       "which may be one object");
       });
    lc("e1e2c1_pair_reads_range_owns_cell_alias_fail",
       "c2 reads the range and owns the cell: c2(&v[0], 3, &v[1].x): an owner among the two sites, the row fails once",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_cell(k, AtomKind::ReadsRange, AtomKind::Owns, "3", 1, 0), 2U, 0U,
                       "which may be one object");
       });
    lc("e1e2c1_pair_reads_range_reads_cell_alias",
       "both sites only read (no row separates them): the cell's sum row (the range's third and the cell's third of write) holds over the caller's full permission",
       [=](Kit& k)
       {
           pair_cell(k, AtomKind::ReadsRange, AtomKind::Reads, "3", 1, 0);
       });
    // void f(struct P *p, int n, int *q) requires(owns_range(p, n) && owns(*q) && n > 0) ensures(...) { c2(p, n, q); }: the call's rows are decided from f's own permissions
    lc("e1e2c1_pair_forward_cell",
       "f forwards its owns_range(p, n) and owns(*q) to c2: the cell row of each leaf is proved from f's two full permissions (no alias is possible)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* c2 {mk_c2(k, P, AtomKind::OwnsRange, AtomKind::Owns)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
           auto ra = [&](int amount)
           {
               return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), amount, k.at());
           };
           auto ca = [&](int amount)
           {
               return k.b.atom(AtomKind::Owns, fx(k, f, 2), amount, k.at());
           };
           k.req(f, k.land(k.land(ra(0), ca(1)), k.gt(fx(k, f, 1), N(k, "0"))));
           k.ens(f, k.land(ra(0), ca(1)));
           f->body = k.blkc({k.es(k.call(c2, {k.pv(f, 0), k.pv(f, 1), k.pv(f, 2)}))});
           k.done(f);
       });
    // void c3(struct P *p, int n, struct P *s) requires(owns_range(p, n) && owns(*s) && n > 0) ensures(the same) {}
    auto mk_c3 = [=](Kit& k, StructType* P, AtomKind range_kind, AtomKind cell_kind) -> Function*
    {
        Function* c {k.fn("c3", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"s", k.ptr(k.st(P))}})};
        auto ra = [&](int amount)
        {
            return k.b.range_atom(range_kind, fx(k, c, 0), fx(k, c, 1), amount, k.at());
        };
        auto ca = [&](int amount)
        {
            return k.b.atom(cell_kind, fx(k, c, 2), amount, k.at());
        };
        k.req(c, k.land(k.land(ra(0), ca(1)), k.gt(fx(k, c, 1), N(k, "0"))));
        k.ens(c, k.land(ra(0), ca(1)));
        c->body = k.blkc({});
        k.done(c);
        return c;
    };
    // void f(void) { struct P v[3] = {0}; c3(&v[0], <n>, &v[<s>]); }: a structure atom on an element of the same array
    const auto pair_struct_atom = [=](Kit& k, AtomKind range_kind, AtomKind cell_kind, const char* n,
                                      const char* s) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* c3 {mk_c3(k, P, range_kind, cell_kind)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
        const Expr* sp {k.b.elem_addr(k.addr(v), N(k, s), k.at())};
        const Expr* call {k.call(c3, {first, N(k, n), sp})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2c1_pair_struct_atom_outside_range",
       "c3(&v[0], 2, &v[2]): owns(*s) over struct P (leaves x and y) beside owns_range(p, n) over the same type, element 2 outside [0, 2): the cell rows of both leaves pass",
       [=](Kit& k)
       {
           pair_struct_atom(k, AtomKind::OwnsRange, AtomKind::Owns, "2", "2");
       });
    lc("e1e2c1_pair_struct_atom_alias_fail",
       "c3(&v[0], 3, &v[1]): the structure atom's leaves x and y are both cells of element 1: four rows (cell by leaf), the first that fails ends the path (s.x against the range's x)",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_struct_atom(k, AtomKind::OwnsRange, AtomKind::Owns, "3", "1"), 4U, 0U,
                       "which may be one object");
       });
    // void h2(struct P *p, int n, struct P *q, int m) requires(<k1>(p, n) && <k2>(q, m) && n > 0 && m > 0) ensures(the same) {}
    auto mk_h2 = [=](Kit& k, StructType* P, AtomKind k1, AtomKind k2) -> Function*
    {
        Function* h {k.fn("h2", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.st(P))}, {"m", k.i32}})};
        auto ra = [&](AtomKind kind, int pi, int ni, int amount)
        {
            return k.b.range_atom(kind, fx(k, h, pi), fx(k, h, ni), amount, k.at());
        };
        k.req(h, k.land(k.land(ra(k1, 0, 1, 0), ra(k2, 2, 3, 1)),
                        k.land(k.gt(fx(k, h, 1), N(k, "0")), k.gt(fx(k, h, 3), N(k, "0")))));
        k.ens(h, k.land(ra(k1, 0, 1, 0), ra(k2, 2, 3, 1)));
        h->body = k.blkc({});
        k.done(h);
        return h;
    };
    // void f(void) { struct P v[3] = {0}; h2(&v[0], <n>, &v[<at2>], <m>); }
    const auto pair_ranges = [=](Kit& k, AtomKind k1, AtomKind k2, const char* n, const char* at2,
                                 const char* m) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* h {mk_h2(k, P, k1, k2)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        auto at = [&](const char* i)
        {
            return k.b.elem_addr(k.addr(v), N(k, i), k.at());
        };
        const Expr* call {k.call(h, {at("0"), N(k, n), at(at2), N(k, m)})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2c1_pair_two_ranges_disjoint",
       "h2(&v[0], 1, &v[2], 1), two owned ranges over struct P: the slot row passes for (x, x) and (y, y)",
       [=](Kit& k)
       {
           pair_ranges(k, AtomKind::OwnsRange, AtomKind::OwnsRange, "1", "2", "1");
       });
    lc("e1e2c1_pair_two_ranges_overlap_fail",
       "h2(&v[0], 2, &v[1], 1): the ranges share element 1: two rows (x with x, y with y), the first fails (permission) and ends the path",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_ranges(k, AtomKind::OwnsRange, AtomKind::OwnsRange, "2", "1", "1"), 2U, 0U,
                       "which may be one object");
       });
    lc("e1e2c1_pair_reads_over_owns_overlap_fail",
       "h2 reads the first range and owns the second: h2(&v[0], 2, &v[1], 1): an owner among the two, the slot row fails",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_ranges(k, AtomKind::ReadsRange, AtomKind::OwnsRange, "2", "1", "1"), 2U, 0U,
                       "which may be one object");
       });
    lc("e1e2c1_pair_two_reads_overlap",
       "h2 reads both ranges: h2(&v[0], 2, &v[1], 1) over a local array the caller owns in full: no row separates two readers, the sums of the amounts (each a third of write) hold",
       [=](Kit& k)
       {
           pair_ranges(k, AtomKind::ReadsRange, AtomKind::ReadsRange, "2", "1", "1");
       });
    // void f(struct P *p, int n, struct P *q, int m) requires(owns_range(p, n) && owns_range(q, m) && n > 0 && m > 0) ensures(the same) { h2(p, n, q, m); }
    lc("e1e2c1_pair_forward_two_ranges",
       "f forwards its two owned ranges over struct P to h2: the slot row is proved from f's permissions (no leaf receiver of a first slot is in view: the case the block row could not decide)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* h {mk_h2(k, P, AtomKind::OwnsRange, AtomKind::OwnsRange)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.st(P))}, {"m", k.i32}})};
           auto ra = [&](int pi, int ni, int amount)
           {
               return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, pi), fx(k, f, ni), amount, k.at());
           };
           k.req(f, k.land(k.land(ra(0, 1, 0), ra(2, 3, 1)),
                           k.land(k.gt(fx(k, f, 1), N(k, "0")), k.gt(fx(k, f, 3), N(k, "0")))));
           k.ens(f, k.land(ra(0, 1, 0), ra(2, 3, 1)));
           f->body = k.blkc({k.es(k.call(h, {k.pv(f, 0), k.pv(f, 1), k.pv(f, 2), k.pv(f, 3)}))});
           k.done(f);
       });
    // the nested leaf: struct S { int x; struct T { int a; int b; } t; }, a range over S and a range over T at &v[k].t (item 7, "Two ranges": the element structures differ)
    auto mk_nested = [=](Kit& k, StructType*& S, StructType*& T)
    {
        T = k.strukt("T", {{"a", k.i32}, {"b", k.i32}});
        S = k.strukt("S", {{"x", k.i32}, {"t", k.st(T)}});
    };
    // void h4(struct S *p, int n, struct T *q, int m) requires(owns_range(p, n) && owns_range(q, m) && n > 0 && m > 0) ensures(the same) {}
    auto mk_h4 = [=](Kit& k, StructType* S, StructType* T) -> Function*
    {
        Function* h {k.fn("h4", k.vd, {{"p", k.ptr(k.st(S))}, {"n", k.i32}, {"q", k.ptr(k.st(T))}, {"m", k.i32}})};
        auto ra = [&](int pi, int ni, int amount)
        {
            return k.b.range_atom(AtomKind::OwnsRange, fx(k, h, pi), fx(k, h, ni), amount, k.at());
        };
        k.req(h, k.land(k.land(ra(0, 1, 0), ra(2, 3, 1)),
                        k.land(k.gt(fx(k, h, 1), N(k, "0")), k.gt(fx(k, h, 3), N(k, "0")))));
        k.ens(h, k.land(ra(0, 1, 0), ra(2, 3, 1)));
        h->body = k.blkc({});
        k.done(h);
        return h;
    };
    // void f(void) { struct S v[3] = {0}; h4(&v[0], <n>, &v[<at2>].t, 1); }
    const auto pair_nested = [=](Kit& k, const char* n, const char* at2) -> SrcLoc
    {
        StructType* S {nullptr};
        StructType* T {nullptr};
        mk_nested(k, S, T);
        Function* h {mk_h4(k, S, T)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(S), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
        const Expr* t {k.b.member_addr(k.b.elem_addr(k.addr(v), N(k, at2), k.at()), 1, k.at())};
        const Expr* call {k.call(h, {first, N(k, n), t, N(k, "1")})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2c1_pair_nested_range_outside",
       "h4(&v[0], 2, &v[2].t, 1): a range over struct T at the nested member of element 2, outside the range over struct S [0, 2): the pair of t.a with a and of t.b with b pass",
       [=](Kit& k)
       {
           pair_nested(k, "2", "2");
       });
    lc("e1e2c1_pair_nested_range_alias_fail",
       "h4(&v[0], 3, &v[1].t, 1): the range over T meets the range over S through t.a and t.b: six rows (S's x, t.a, t.b against T's a, b), the first that fails, (t.a, a), ends the path",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_nested(k, "3", "1"), 6U, 2U, "which may be one object");
       });
    // the returns: a function that borrows a range over struct P and a cell of one of its leaf fields (or a range over another element type) returns both: the marked rows' amounts per leaf add what the other atom covers
    lc("e1e2c1_pair_returns_reads_range_and_cell",
       "c2 = reads_range(p, n) over struct P && reads(*q) with an empty body, f of c2's own contract: the return rows add the cell's amount at the range's leaf slots and the range's at the cell",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           mk_c2(k, P, AtomKind::ReadsRange, AtomKind::Reads);
       });
    lc("e1e2c1_pair_returns_two_reads_nested",
       "h4 with reads_range over struct S and reads_range over struct T: the return rows add, per leaf of each range, the other range's amount where its leaves cover the slot",
       [=](Kit& k)
       {
           StructType* S {nullptr};
           StructType* T {nullptr};
           mk_nested(k, S, T);
           Function* h {k.fn("h4r", k.vd, {{"p", k.ptr(k.st(S))}, {"n", k.i32}, {"q", k.ptr(k.st(T))}, {"m", k.i32}})};
           auto ra = [&](int pi, int ni, int amount)
           {
               return k.b.range_atom(AtomKind::ReadsRange, fx(k, h, pi), fx(k, h, ni), amount, k.at());
           };
           k.req(h, k.land(k.land(ra(0, 1, 0), ra(2, 3, 1)),
                           k.land(k.gt(fx(k, h, 1), N(k, "0")), k.gt(fx(k, h, 3), N(k, "0")))));
           k.ens(h, k.land(ra(0, 1, 0), ra(2, 3, 1)));
           h->body = k.blkc({});
           k.done(h);
       });
    // void f(void) { struct S v[3] = {0}; h4r(&v[0], 2, &v[1].t, 1); }: both only read, the ranges meet at the nested leaves: the marked rows' per-leaf amounts add the other range's third
    lc("e1e2c1_pair_nested_two_reads_overlap",
       "h4r reads a range over struct S and a range over struct T at &v[1].t, which the S range covers through t.a and t.b: no row separates two readers; the per-leaf marked amounts add the other's, over the caller's full permission",
       [=](Kit& k)
       {
           StructType* S {nullptr};
           StructType* T {nullptr};
           mk_nested(k, S, T);
           Function* h {k.fn("h4r", k.vd, {{"p", k.ptr(k.st(S))}, {"n", k.i32}, {"q", k.ptr(k.st(T))}, {"m", k.i32}})};
           auto ra = [&](int pi, int ni, int amount)
           {
               return k.b.range_atom(AtomKind::ReadsRange, fx(k, h, pi), fx(k, h, ni), amount, k.at());
           };
           k.req(h, k.land(k.land(ra(0, 1, 0), ra(2, 3, 1)),
                           k.land(k.gt(fx(k, h, 1), N(k, "0")), k.gt(fx(k, h, 3), N(k, "0")))));
           k.ens(h, k.land(ra(0, 1, 0), ra(2, 3, 1)));
           h->body = k.blkc({});
           k.done(h);
           Function* f {k.fn("f", k.vd)};
           Var* v {f->new_local("v", k.prog.array_of(k.st(S), 3U), k.at())};
           f->mark_address_taken(v);
           v->zero_filled = true;
           const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
           const Expr* t {k.b.member_addr(k.b.elem_addr(k.addr(v), N(k, "1"), k.at()), 1, k.at())};
           f->body = k.blkc({k.decl(v, nullptr), k.es(k.call(h, {first, N(k, "2"), t, N(k, "1")}))});
           k.done(f);
       });
    // int hs(struct S *p, int n) requires(owns_range(p, n) && n > 0) ensures(owns_range(p, n) && r == 1) { return 1; }, ht the same over struct T
    // void f(void) { struct S v[3] = {0}; hs(&v[0], <n>) == ht(&v[<at2>].t, 1); }: two calls over ranges of DIFFERENT element types in one expression
    const auto seq_nested = [=](Kit& k, const char* n, const char* at2) -> SrcLoc
    {
        StructType* S {nullptr};
        StructType* T {nullptr};
        mk_nested(k, S, T);
        auto mk_one = [&](const char* name, StructType* E) -> Function*
        {
            Function* h {k.fn(name, k.i32, {{"p", k.ptr(k.st(E))}, {"n", k.i32}})};
            auto ra = [&](int amount)
            {
                return k.b.range_atom(AtomKind::OwnsRange, fx(k, h, 0), fx(k, h, 1), amount, k.at());
            };
            k.req(h, k.land(ra(0), k.gt(fx(k, h, 1), N(k, "0"))));
            k.ens(h, k.land(ra(0), k.eq(k.R(h), N(k, "1"))));
            h->body = k.blkc({k.ret(N(k, "1"))});
            k.done(h);
            return h;
        };
        Function* hs {mk_one("hs", S)};
        Function* ht {mk_one("ht", T)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(S), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        const Expr* first {k.b.elem_addr(k.addr(v), N(k, "0"), k.at())};
        const Expr* t {k.b.member_addr(k.b.elem_addr(k.addr(v), N(k, at2), k.at()), 1, k.at())};
        const Expr* c1 {k.call(hs, {first, N(k, n)})};
        const Expr* c2 {k.call(ht, {t, N(k, "1")})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(k.eq(c1, c2))});
        k.done(f);
        return c2->loc;
    };
    lc("e1e2c1_seq_nested_calls_disjoint",
       "hs(&v[0], 2) == ht(&v[2].t, 1): a range over struct S and a range over struct T at the nested member of element 2: the six leaf pairs pass",
       [=](Kit& k)
       {
           seq_nested(k, "2", "2");
       });
    lc("e1e2c1_seq_nested_calls_overlap_fail",
       "hs(&v[0], 3) == ht(&v[1].t, 1): the T range is the nested member of element 1, which the S range covers: six leaf pairs, the first that fails, (t.a, a), is unsequenced at the later call",
       [=](Kit& k)
       {
           k.fail_only("unsequenced", seq_nested(k, "3", "1"), 6U, 2U);
       });
    rc("e1e2c1_rej_owns_block_beside_a_struct_range", "an `owns_block` atom beside a range over structures",
       [=](Kit& k)
       {
           // void f(struct P *p, int n, struct P *q) requires(owns_range(p, n) && owns_block(q)): the heap block's cells beside the range are Part D's
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.st(P))}})};
           k.req(f, k.land(RO(k, f, 0), k.b.atom(AtomKind::OwnsBlock, fx(k, f, 2), -1, k.at())));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c1_rej_marked_range_contract_read", "is marked `ref_to_uninit` or `must_init` and a contract reads `*p`",
       [=](Kit& k)
       {
           // requires(owns_range(p, n) && n > 0 && p[0].x == 0) on a marked parameter: its elements have no value on entry
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::RefToUninit;
           k.req(f, k.land(k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "0"))),
                           k.eq(k.deref(pplace(k, fx(k, f, 0), "0", 0)), N(k, "0"))));
           k.ens(f, RO(k, f, 0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c1_rej_marked_range_beside_reads_range", "beside `reads_range(p, n)`",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::MustInit;
           k.req(f, k.land(RO(k, f, 0), RR(k, f, 1)));
           k.ens(f, RO(k, f, 0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c1_rej_range_over_a_struct_with_an_array_member", "whose element holds an array member",
       [=](Kit& k)
       {
           // (slice 3c-2 lifts the range over a structure with an array member of scalars; an element with a MATRIX member, depth 3, stays refused)
           StructType* S {k.strukt("S", {{"x", k.i32}, {"a", k.prog.array_of(k.prog.array_of(k.i32, 2U), 2U)}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}, {"n", k.i32}})};
           k.req(f, RO(k, f, 0));
           f->body = k.blk({});
           k.done(f);
       });
    // ---- slice 3: element reads `p[e].x` in a contract (record 0064 item 7): framed when the element's slot lies in the range ----
    auto rd = [=](Kit& k, Function* f, const char* idx, int m) -> const Expr*
    {
        return k.b.deref(pplace(k, fx(k, f, 0), idx, m), k.at());
    };
    // int g(struct P *p, int n) requires(<range>(p, n) && n > 1 && p[<ridx>].x == 5) ensures(<range>(p, n) && r == 5) { return p[1].x; }
    const auto elem_req = [=](Kit& k, bool owns, const char* ridx, Function*& out,
                              bool const_body = false) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* g {k.fn("g", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        const AtomKind kind {owns ? AtomKind::OwnsRange : AtomKind::ReadsRange};
        auto atom = [&](int amount)
        {
            return k.b.range_atom(kind, fx(k, g, 0), fx(k, g, 1), amount, k.at());
        };
        const Expr* read {rd(k, g, ridx, 0)};
        k.req(g, k.land(k.land(atom(0), k.gt(fx(k, g, 1), N(k, "1"))), k.eq(read, N(k, "5"))));
        k.ens(g, k.land(atom(0), k.eq(k.R(g), N(k, "5"))));
        g->body = k.blkc({k.ret(const_body ? N(k, "5") : k.deref(pplace(k, k.pv(g, 0), "1", 0)))});
        k.done(g);
        out = g;
        return read;
    };
    lc("e1e2c1_elem_requires_reads",
       "reads_range(p, n) && n > 1 && p[1].x == 5 in the requires: the read is framed by the range (element 1 is inside [0, n)), the body returns 5",
       [=](Kit& k)
       {
           Function* g {nullptr};
           elem_req(k, false, "1", g);
       });
    lc("e1e2c1_elem_requires_owns", "the same over owns_range(p, n)",
       [=](Kit& k)
       {
           Function* g {nullptr};
           elem_req(k, true, "1", g);
       });
    lc("e1e2c1_elem_requires_outside_fail",
       "p[2].x == 5 in the requires with only n > 1: the read of element 2 is framed by no atom (permission, f__wf's framing row)",
       [=](Kit& k)
       {
           Function* g {nullptr};
           k.fail("permission", elem_req(k, false, "2", g, true)->loc);
       });
    // void f(void) { struct P v[3] = {0}; v[1].x = <val>; g(&v[0], 3); }
    const auto elem_caller = [=](Kit& k, const char* val) -> SrcLoc
    {
        Function* g {nullptr};
        elem_req(k, true, "1", g);
        Function* f {k.fn("f", k.vd)};
        StructType* P {const_cast<StructType*>(g->params[0U].type.pointee->struct_type)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        const Stmt* st {k.store(pplace(k, k.addr(v), "1", 0), N(k, val))};
        const Expr* c {k.call(g, {k.b.elem_addr(k.addr(v), N(k, "0"), k.at()), N(k, "3")})};
        f->body = k.blkc({k.decl(v, nullptr), st, k.es(c)});
        k.done(f);
        return c->loc;
    };
    lc("e1e2c1_elem_call_holds",
       "g needs p[1].x == 5: the caller wrote v[1].x = 5, the callee's requires holds at the call",
       [=](Kit& k)
       {
           elem_caller(k, "5");
       });
    lc("e1e2c1_elem_call_fails",
       "the caller wrote v[1].x = 4: the callee's requires p[1].x == 5 fails at the call (precondition)",
       [=](Kit& k)
       {
           k.fail("precondition", elem_caller(k, "4"));
       });
    // void f(struct P *p, int n, int i) requires(owns_range(p, n) && 0 <= i && i <= <bound> n... && p[i].y > 0)
    const auto elem_index = [=](Kit& k, bool strict) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"i", k.i32}})};
        const Expr* read {
            k.b.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), fx(k, f, 2), k.at()), 1, k.at()), k.at())};
        const Expr* below {strict ? k.lt(fx(k, f, 2), fx(k, f, 1)) : k.le(fx(k, f, 2), fx(k, f, 1))};
        k.req(f, k.land(k.land(RO(k, f, 0), k.land(k.ge(fx(k, f, 2), N(k, "0")), below)), k.gt(read, N(k, "0"))));
        k.ens(f, RO(k, f, 0));
        f->body = k.blkc({});
        k.done(f);
        return read;
    };
    lc("e1e2c1_elem_index_inside", "0 <= i && i < n && p[i].y > 0: the read at a symbolic index is framed",
       [=](Kit& k)
       {
           elem_index(k, true);
       });
    lc("e1e2c1_elem_index_one_past_fail", "0 <= i && i <= n && p[i].y > 0: i == n is outside the range (permission)",
       [=](Kit& k)
       {
           k.fail("permission", elem_index(k, false)->loc);
       });
    // int f(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && p[1].x == <want>) { p[1].x = 7; return 0; }
    const auto elem_ens = [=](Kit& k, const char* want) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
        k.ens(f, k.land(RO(k, f, 0), k.eq(rd(k, f, "1", 0), N(k, want))));
        const Stmt* ret {k.ret(N(k, "0"))};
        f->body = k.blkc({k.store(pplace(k, k.pv(f, 0), "1", 0), N(k, "7")), ret});
        k.done(f);
        return ret->loc;
    };
    lc("e1e2c1_elem_ensures_after_write",
       "ensures p[1].x == 7 after p[1].x = 7: the post-state read is framed by the ensures' range",
       [=](Kit& k)
       {
           elem_ens(k, "7");
       });
    lc("e1e2c1_elem_ensures_wrong_fail", "ensures p[1].x == 8 after p[1].x = 7: the postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", elem_ens(k, "8"));
       });
    // void f(struct P *p, int n) requires(owns_range(p, n) && n > 1 && p[1].x == 4) ensures(owns_range(p, n) && p[1].x == old(p[1].x) + <step>) { p[1].x = 5; }
    const auto elem_old = [=](Kit& k, const char* step) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))), k.eq(rd(k, f, "1", 0), N(k, "4"))));
        k.ens(f, k.land(RO(k, f, 0), k.eq(rd(k, f, "1", 0), k.add(k.old(rd(k, f, "1", 0)), N(k, step)))));
        const Stmt* ret {k.ret(N(k, "0"))};
        f->body = k.blkc({k.store(pplace(k, k.pv(f, 0), "1", 0), N(k, "5")), ret});
        k.done(f);
        return ret->loc;
    };
    lc("e1e2c1_elem_old", "p[1].x == old(p[1].x) + 1 after p[1].x = 5 with p[1].x == 4 on entry",
       [=](Kit& k)
       {
           elem_old(k, "1");
       });
    lc("e1e2c1_elem_old_wrong_fail", "the same with + 2: the postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", elem_old(k, "2"));
       });
    // a nested leaf and a pointer leaf of the element
    lc("e1e2c1_elem_nested_leaf",
       "struct Q { int a; struct T { int b; int c; } t; }: requires(reads_range(p, n) && n > 1 && p[1].t.c == 4) ensures(reads_range(p, n) && p[1].t.c == 4)",
       [=](Kit& k)
       {
           StructType* T {k.strukt("T", {{"b", k.i32}, {"c", k.i32}})};
           StructType* Q {k.strukt("Q", {{"a", k.i32}, {"t", k.st(T)}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(Q))}, {"n", k.i32}})};
           auto leaf = [&]()
           {
               return k.b.deref(
                   k.b.member_addr(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "1"), k.at()), 1, k.at()), 1,
                                   k.at()),
                   k.at());
           };
           k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))), k.eq(leaf(), N(k, "4"))));
           k.ens(f, k.land(RR(k, f, 0), k.eq(leaf(), N(k, "4"))));
           f->body = k.blkc({});
           k.done(f);
       });
    lc("e1e2c1_elem_pointer_leaf",
       "struct R { int *q; int y; }: requires(reads_range(p, n) && n > 0 && p[0].q != NULL) ensures(reads_range(p, n)): a pointer leaf's value is read",
       [=](Kit& k)
       {
           StructType* R {k.strukt("R", {{"q", k.ptr(k.i32)}, {"y", k.i32}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
           const Expr* q {k.b.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "0"), k.at()), 0, k.at()), k.at())};
           k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "0"))), k.ne(q, k.nul(k.ptr(k.i32)))));
           k.ens(f, RR(k, f, 0));
           f->body = k.blkc({});
           k.done(f);
       });
    // framed by a structure atom whose place is the element's slot: void f(struct P *p, struct P *q) requires(owns(*q) && p == q && p[0].x == 1)
    const auto elem_by_atom = [=](Kit& k, bool alias) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"q", k.ptr(k.st(P))}})};
        const Expr* read {rd(k, f, "0", 0)};
        const Expr* own {k.b.atom(AtomKind::Owns, fx(k, f, 1), 0, k.at())};
        const Expr* eqpq {k.eq(fx(k, f, 0), fx(k, f, 1))};
        k.req(f, alias ? k.land(k.land(own, eqpq), k.eq(read, N(k, "1"))) : k.land(own, k.eq(read, N(k, "1"))));
        k.ens(f, k.b.atom(AtomKind::Owns, fx(k, f, 1), 0, k.at()));
        f->body = k.blkc({});
        k.done(f);
        return read;
    };
    lc("e1e2c1_elem_framed_by_a_struct_atom",
       "owns(*q) && p == q && p[0].x == 1: the read of p's element 0 is the leaf of q's structure",
       [=](Kit& k)
       {
           elem_by_atom(k, true);
       });
    lc("e1e2c1_elem_struct_atom_elsewhere_fail",
       "owns(*q) && p[0].x == 1 with no p == q: the read is framed by no atom (permission)",
       [=](Kit& k)
       {
           k.fail("permission", elem_by_atom(k, false)->loc);
       });
    // decreases(p[<d>].y) with requires(reads_range(p, n) && n > 1 && p[1].y >= 0)
    const auto elem_dec = [=](Kit& k, const char* d) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))), k.ge(rd(k, f, "1", 1), N(k, "0"))));
        k.ens(f, RR(k, f, 0));
        const Expr* m {rd(k, f, d, 1)};
        k.dec(f, m);
        f->body = k.blkc({});
        k.done(f);
        return m;
    };
    lc("e1e2c1_elem_decreases",
       "decreases(p[1].y) with reads_range(p, n) && n > 1 && p[1].y >= 0: the measure's read is framed",
       [=](Kit& k)
       {
           elem_dec(k, "1");
       });
    lc("e1e2c1_elem_decreases_outside_fail",
       "decreases(p[5].y): the measure's read is outside the range (permission, the measure's wording)",
       [=](Kit& k)
       {
           k.fail("permission", elem_dec(k, "5")->loc);
       });
    // what the lift keeps refused: a pointer loaded from an element, then read or indexed; an element's address as a value
    rc("e1e2c1_rej_elem_pointer_leaf_pointee",
       "a dereference in a contract of something other than a pointer parameter",
       [=](Kit& k)
       {
           // requires(reads_range(p, n) && n > 0 && *p[0].q == 1)
           StructType* R {k.strukt("R", {{"q", k.ptr(k.i32)}, {"y", k.i32}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
           const Expr* q {k.b.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "0"), k.at()), 0, k.at()), k.at())};
           k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "0"))), k.eq(k.deref(q), N(k, "1"))));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c1_rej_elem_pointer_leaf_index", "an element read in a contract of anything but a pointer parameter",
       [=](Kit& k)
       {
           // requires(reads_range(p, n) && n > 0 && p[0].q[0] == 1)
           StructType* R {k.strukt("R", {{"q", k.ptr(k.i32)}, {"y", k.i32}})};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
           const Expr* q {k.b.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "0"), k.at()), 0, k.at()), k.at())};
           k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "0"))),
                           k.eq(k.deref(k.b.elem_addr(q, N(k, "0"), k.at())), N(k, "1"))));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c1_rej_elem_address_value", "the address of a member in a contract outside the place of a read",
       [=](Kit& k)
       {
           // requires(reads_range(p, n) && n > 1 && &p[1].x == &p[1].x)
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           const Expr* a {pplace(k, fx(k, f, 0), "1", 0)};
           const Expr* b2 {pplace(k, fx(k, f, 0), "1", 0)};
           k.req(f, k.land(k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))), k.eq(a, b2)));
           f->body = k.blk({});
           k.done(f);
       });
    // ---- slice 4: loops over a parameter's range of structures (record 0064 item 10): a range place per leaf path ----
    // the parameter's local copy in a body, `p[e].<m>` there and the store, increment and bound helpers
    auto bat = [=](Kit& k, Function* f, int pi, const Expr* idx, int m) -> const Expr*
    {
        return k.b.member_addr(k.b.elem_addr(k.pn(f->param_var(pi)), idx, k.at()), m, k.at());
    };
    auto brd = [=](Kit& k, Function* f, int pi, const Expr* idx, int m) -> const Expr*
    {
        return k.deref(bat(k, f, pi, idx, m));
    };
    auto inc = [](Kit& k, Var* i)
    {
        return k.asg(i, k.add(k.v(i), k.num(k.i32, "1")));
    };
    auto bounds = [=](Kit& k, Function* f, Var* i, int n)
    {
        return k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), k.pv(f, n)));
    };
    auto vac = [](Kit& k, const Stmt* c)
    {
        k.fail("check", c->loc);
    };
    // void fill(struct P *p, int n) requires(owns_range(p, n) && n < 100) ensures(owns_range(p, n))
    //   { int i = 0; while ([i < n|i <= n]) inv 0 <= i <= n decreases n - i { p[i].x = i; [check(false);] i++; } [check(false);] }
    // `mode`: 0 the fill, 1 i <= n, 2 check(false) after the loop, 3 check(false) at the end of the body
    const auto lfill = [=](Kit& k, int mode) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("fill", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, RO(k, f, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* w {k.store(bat(k, f, 0, k.v(i), 0), k.v(i))};
        const Stmt* c3 {mode == 3 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {w};
        if (c3 != nullptr)
        {
            bs.push_back(c3);
        }
        bs.push_back(inc(k, i));
        const Expr* cond {mode == 1 ? k.le(k.v(i), k.pv(f, 1)) : k.lt(k.v(i), k.pv(f, 1))};
        const Stmt* loop {k.whl(cond, {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> body {k.decl(i, N(k, "0")), loop};
        if (c2 != nullptr)
        {
            body.push_back(c2);
        }
        f->body = k.blk(std::move(body));
        k.done(f);
        return mode == 2 ? c2 : mode == 3 ? c3 : w;
    };
    lc("e1e2c1_loop_fill",
       "while (i < n) p[i].x = i over owns_range(p, n) of struct P: one range place per leaf (x and y) carries the permissions, flags and values through the loop",
       [=](Kit& k)
       {
           lfill(k, 0);
       });
    lc("e1e2c1_loop_fill_high",
       "the loop runs to i <= n: p[n].x fails its bound row and its write-permission row at the store",
       [=](Kit& k)
       {
           const Stmt* w {lfill(k, 1)};
           k.fail("out-of-bounds", w->loc);
           k.fail("permission", w->loc);
       });
    lc("e1e2c1_loop_fill_vacuity", "the fill with check(false) after the loop: it fails (the loop's path is not empty)",
       [=](Kit& k)
       {
           vac(k, lfill(k, 2));
       });
    lc("e1e2c1_loop_fill_body_vacuity", "the fill with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac(k, lfill(k, 3));
       });
    // int vals(struct <S> *p, int n) requires(<range>(p, n) && n > 0 && p[0].<rm> == 7) ensures(<range>(p, n) && r == 0)
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { [p[i].<wm> = 0;] i++; } check(p[0].<rm> == <want>); return 0; }
    // S is struct P { int x; int y; } or struct L { long l; int y; }; `wm` < 0: the loop writes nothing
    const auto lvals = [=](Kit& k, bool longs, bool reads, int wm, int rm, const char* want) -> const Stmt*
    {
        StructType* S {longs ? k.strukt("L", {{"l", k.i64}, {"y", k.i32}}) : mk_p(k)};
        Function* f {k.fn("vals", k.i32, {{"p", k.ptr(k.st(S))}, {"n", k.i32}})};
        auto atom = [&](int amount)
        {
            return reads ? RR(k, f, amount) : RO(k, f, amount);
        };
        const Expr* seven {
            k.eq(k.deref(pplace(k, fx(k, f, 0), "0", rm)), longs && rm == 0 ? k.num(k.i64, "7") : N(k, "7"))};
        k.req(f, k.land(k.land(atom(0), k.gt(fx(k, f, 1), N(k, "0"))), seven));
        k.ens(f, k.land(atom(0), k.eq(k.R(f), N(k, "0"))));
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> bs;
        if (wm >= 0)
        {
            bs.push_back(k.store(bat(k, f, 0, k.v(i), wm), N(k, "0")));
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        const Stmt* c {k.chk(k.eq(brd(k, f, 0, N(k, "0"), rm), longs && rm == 0 ? k.num(k.i64, want) : N(k, want)))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop, c, k.ret(N(k, "0"))});
        k.done(f);
        return c;
    };
    lc("e1e2c1_loop_values_kept_reads",
       "a read-only loop over reads_range(p, n) of struct P keeps the values: p[0].x == 7 still holds after it",
       [=](Kit& k)
       {
           lvals(k, false, true, -1, 0, "7");
       });
    lc("e1e2c1_loop_values_kept_owns",
       "a loop over owns_range(p, n) that writes nothing keeps the values (no write reaches the field)",
       [=](Kit& k)
       {
           lvals(k, false, false, -1, 0, "7");
       });
    lc("e1e2c1_loop_values_wrong_fail", "the same checking p[0].x == 8: fails at the check",
       [=](Kit& k)
       {
           vac(k, lvals(k, false, true, -1, 0, "8"));
       });
    lc("e1e2c1_loop_values_lost_same_field_fail",
       "the loop writes p[i].y (an int): it reaches the int field, so p[0].x == 7 is lost though the leaf differs (the listed false alarm of record 0064 item 10: the check fails)",
       [=](Kit& k)
       {
           vac(k, lvals(k, false, false, 1, 0, "7"));
       });
    lc("e1e2c1_loop_values_kept_other_field",
       "struct L { long l; int y; }: the loop writes p[i].y (an int), the long field l is not reached: p[0].l == 7 is kept",
       [=](Kit& k)
       {
           lvals(k, true, false, 1, 0, "7");
       });
    // void countdown(struct P *p, int n) requires(owns_range(p, n) && n > 0 && 0 <= p[0].x && p[0].x <= 100) ensures(owns_range(p, n) && p[0].x == 0)
    //   { while (p[0].x > 0) inv 0 <= p[0].x && p[0].x <= 100 decreases p[0].x { p[0].x -= 1; [check(false);] } [check(false);] }
    const auto lcountdown = [=](Kit& k, int mode) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("countdown", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "0"))),
                        k.land(k.ge(rd(k, f, "0", 0), N(k, "0")), k.le(rd(k, f, "0", 0), N(k, "100")))));
        k.ens(f, k.land(RO(k, f, 0), k.eq(rd(k, f, "0", 0), N(k, "0"))));
        const Stmt* w {k.cstore(bat(k, f, 0, N(k, "0"), 0), BinaryOp::Sub, N(k, "1"))};
        const Stmt* c1 {mode == 1 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {w};
        if (c1 != nullptr)
        {
            bs.push_back(c1);
        }
        const Stmt* loop {k.whl(
            k.gt(brd(k, f, 0, N(k, "0"), 0), N(k, "0")),
            {k.cl(k.land(k.ge(brd(k, f, 0, N(k, "0"), 0), N(k, "0")), k.le(brd(k, f, 0, N(k, "0"), 0), N(k, "100"))))},
            {k.cl(brd(k, f, 0, N(k, "0"), 0))}, k.blkc(bs))};
        const Stmt* c2 {mode == 2 ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> body {loop};
        if (c2 != nullptr)
        {
            body.push_back(c2);
        }
        f->body = k.blk(std::move(body));
        k.done(f);
        return mode == 1 ? c1 : c2;
    };
    lc("e1e2c1_loop_element_clauses",
       "p[0].x in a loop's invariant and in its measure over owns_range(p, n) of struct P: the reads are framed by the range place's leaf by the slot of the pointer's value",
       [=](Kit& k)
       {
           lcountdown(k, 0);
       });
    lc("e1e2c1_loop_element_clauses_body_vacuity", "the countdown with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac(k, lcountdown(k, 1));
       });
    lc("e1e2c1_loop_element_clauses_vacuity", "the countdown with check(false) after the loop: it fails",
       [=](Kit& k)
       {
           vac(k, lcountdown(k, 2));
       });
    // int first_negative(struct P *p, int n) requires(reads_range(p, n)) ensures(reads_range(p, n) && (r == -1 || (0 <= r && r < n && p[r].x < 0)))
    //   { int i = 0; while (i < n) inv 0 <= i <= n && (i == 0 || p[i - 1].x >= 0) decreases n - i { if (p[i].x < 0) { return i; } i++; } return -1; }
    // `unframed`: the invariant is `i < n || p[i].x >= 0`, which reads p[i].x where i is n
    const auto lfirst = [=](Kit& k, bool unframed) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("first_negative", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, RR(k, f, 0));
        auto cread = [&](const Expr* idx)
        {
            return k.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), idx, k.at()), 0, k.at()));
        };
        k.ens(f, k.land(RR(k, f, 0), k.lor(k.eq(k.R(f), N(k, "-1")),
                                           k.land(k.land(k.ge(k.R(f), N(k, "0")), k.lt(k.R(f), fx(k, f, 1))),
                                                  k.lt(cread(k.R(f)), N(k, "0"))))));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* read {k.b.text(brd(k, f, 0, unframed ? k.v(i) : k.sub(k.v(i), N(k, "1")), 0),
                                   unframed ? "p[i].x" : "p[i - 1].x")};
        const Expr* clause {
            k.lor(unframed ? k.lt(k.v(i), k.pv(f, 1)) : k.eq(k.v(i), N(k, "0")), k.ge(read, N(k, "0")))};
        const Stmt* body {
            k.blkc({k.iff(k.lt(brd(k, f, 0, k.v(i), 0), N(k, "0")), k.blkc({k.ret(k.v(i))}), nullptr), inc(k, i)})};
        const Stmt* loop {k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1)), k.cl(clause)},
                                {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)};
        f->body = k.blk({k.decl(i, N(k, "0")), loop, k.ret(N(k, "-1"))});
        k.done(f);
        return read;
    };
    lc("e1e2c1_loop_element_guarded",
       "p[i - 1].x under i == 0 || in a loop invariant over reads_range(p, n): framed by the range for 0 < i <= n",
       [=](Kit& k)
       {
           lfirst(k, false);
       });
    lc("e1e2c1_loop_element_unframed_fail",
       "i < n || p[i].x >= 0 as an invariant: for i == n (and for n == 0 at the entry) the range place ends at n, the read is unframed (permission at the read in the clause)",
       [=](Kit& k)
       {
           k.fail("permission", lfirst(k, true)->loc, 2U, "", "this loop invariant reads `p[i].x`");
       });
    // int pin(struct P *p, struct P *q, int n) requires(reads_range(p, n) [&& reads_range(q, n) && q[0].x == 4] && n > 0) ensures(the same)
    //   { int i = 0; p = q; while (i < n) inv 0 <= i <= n && <p|q>[0].x == 4 decreases n - i { i++; } return 0; }
    // `mode`: 0 the invariant reads q[0].x, 1 it reads p[0].x through the assigned p (= q), 2 the same with only live(q) on q
    const auto lpin = [=](Kit& k, int mode) -> const Expr*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("pin", k.i32, {{"p", k.ptr(k.st(P))}, {"q", k.ptr(k.st(P))}, {"n", k.i32}})};
        auto rr = [&](int pi, int amount)
        {
            return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, pi), fx(k, f, 2), amount, k.at());
        };
        const Expr* q4 {
            k.eq(k.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 1), N(k, "0"), k.at()), 0, k.at())), N(k, "4"))};
        if (mode == 2)
        {
            k.req(f, k.land(k.land(rr(0, 0), k.b.atom(AtomKind::Live, fx(k, f, 1), 1, k.at())),
                            k.gt(fx(k, f, 2), N(k, "0"))));
            k.ens(f, k.land(rr(0, 0), k.b.atom(AtomKind::Live, fx(k, f, 1), 1, k.at())));
        }
        else
        {
            k.req(f, k.land(k.land(k.land(rr(0, 0), rr(1, 1)), k.gt(fx(k, f, 2), N(k, "0"))), q4));
            k.ens(f, k.land(rr(0, 0), rr(1, 1)));
        }
        Var* i {k.local(f, "i", k.i32)};
        const Expr* read {k.b.text(brd(k, f, mode == 0 ? 1 : 0, N(k, "0"), 0), mode == 0 ? "q[0].x" : "p[0].x")};
        const Stmt* loop {k.whl(k.lt(k.v(i), k.pv(f, 2)), {k.cl(bounds(k, f, i, 2)), k.cl(k.eq(read, N(k, "4")))},
                                {k.cl(k.sub(k.pv(f, 2), k.v(i)))}, k.blkc({inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "0")), k.asg(f->param_var(0), k.pn(f->param_var(1))), loop, k.ret(N(k, "0"))});
        k.done(f);
        return read;
    };
    lc("e1e2c1_loop_assigned_param_other_range",
       "the function assigns p (p = q) and the loop's invariant reads q[0].x: q's range place carries it (p has none), the value is kept",
       [=](Kit& k)
       {
           lpin(k, 0);
       });
    lc("e1e2c1_loop_assigned_param_reads_the_value",
       "the same reading p[0].x through the assigned p: the read is by the slot of p's CURRENT value (q's), which q's range place holds, so it is framed",
       [=](Kit& k)
       {
           lpin(k, 1);
       });
    lc("e1e2c1_loop_assigned_param_unframed_fail",
       "p = q with only live(q) held for q (no range on q), the invariant reads p[0].x: p has no range place and q's slot is held by none, so the read is unframed (permission at the read in the clause, at the loop's entry: the invariant then assumes the false framing), never framed by p's entry range",
       [=](Kit& k)
       {
           k.fail_only("permission", lpin(k, 2)->loc, 2U, 0U, "this loop invariant reads `p[0].x`");
       });
    // int scan(struct P *p, int n, int *q, int m) requires(reads_range(p, n) && reads_range(q, m) && n > 0 && m > 0) ensures(the same)
    //   { int i = 0; int s = 0; while (i < n) inv 0 <= i <= n decreases n - i { [p[0].x = p[0].x;] if (p[i].x == q[0]) { s++; } i++; } return 0; }
    // a range over structures beside a range of ints: each leaf place of p carries the amount of q where q covers its cells (the mixed row of range_row_amount)
    const auto ltwo = [=](Kit& k, bool write) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("scan", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.i32)}, {"m", k.i32}})};
        const Expr* rp {k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at())};
        const Expr* rq {k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 2), fx(k, f, 3), 1, k.at())};
        k.req(f, k.land(k.land(rp, rq), k.land(k.gt(fx(k, f, 1), N(k, "0")), k.gt(fx(k, f, 3), N(k, "0")))));
        k.ens(f, k.land(k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()),
                        k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 2), fx(k, f, 3), 1, k.at())));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* w {write ? k.store(bat(k, f, 0, N(k, "0"), 0), brd(k, f, 0, N(k, "0"), 0)) : nullptr};
        std::vector<const Stmt*> bs;
        if (w != nullptr)
        {
            bs.push_back(w);
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
        k.done(f);
        return w;
    };
    lc("e1e2c1_loop_two_reads",
       "reads_range over struct P beside reads_range over int: each leaf place of p carries the amount of q where q covers its cells (the mixed row of range_row_amount), the loop reads nothing",
       [=](Kit& k)
       {
           ltwo(k, false);
       });
    lc("e1e2c1_loop_two_reads_write_fail",
       "the loop writes p[0].x = p[0].x under reads_range(p, n) (the same value, so the values-kept conjunct holds): the write row fails (permission)",
       [=](Kit& k)
       {
           k.fail("permission", ltwo(k, true)->loc);
       });
    // struct Q { int a; struct T { int b; int c; } t; }: void fillq(struct Q *p, int n) requires(owns_range(p, n) && n < 100) ensures(owns_range(p, n))
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { p[i].t.c = i; [check(false);] i++; } }
    const auto lnested = [=](Kit& k, bool vacuity) -> const Stmt*
    {
        StructType* T {k.strukt("T", {{"b", k.i32}, {"c", k.i32}})};
        StructType* Q {k.strukt("Q", {{"a", k.i32}, {"t", k.st(T)}})};
        Function* f {k.fn("fillq", k.vd, {{"p", k.ptr(k.st(Q))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, RO(k, f, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* leaf {k.b.member_addr(
            k.b.member_addr(k.b.elem_addr(k.pn(f->param_var(0)), k.v(i), k.at()), 1, k.at()), 1, k.at())};
        const Stmt* c {vacuity ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {k.store(leaf, k.v(i))};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop});
        k.done(f);
        return c;
    };
    lc("e1e2c1_loop_nested_leaf",
       "struct Q { int a; struct T { int b; int c; } t; }: the loop writes p[i].t.c, one range place per leaf (a, t.b, t.c) of depth 1 and 2",
       [=](Kit& k)
       {
           lnested(k, false);
       });
    lc("e1e2c1_loop_nested_leaf_body_vacuity", "the same with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac(k, lnested(k, true));
       });
    // void beside(struct P *p, int n, int *q) requires(owns_range(p, n) && owns(*q) && n > 0) ensures(owns_range(p, n) && owns(*q))
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { *q = i; p[i].x = i; [check(false);] i++; } }
    const auto lbeside = [=](Kit& k, bool vacuity) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("beside", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"q", k.ptr(k.i32)}})};
        auto cell = [&]()
        {
            return k.b.atom(AtomKind::Owns, fx(k, f, 2), 1, k.at());
        };
        k.req(f, k.land(k.land(RO(k, f, 0), cell()), k.gt(fx(k, f, 1), N(k, "0"))));
        k.ens(f, k.land(RO(k, f, 0), cell()));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* c {vacuity ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {k.store(k.pn(f->param_var(2)), k.v(i)), k.store(bat(k, f, 0, k.v(i), 0), k.v(i))};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {
            k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc(bs))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop});
        k.done(f);
        return c;
    };
    lc("e1e2c1_loop_beside_cell",
       "owns_range(p, n) over struct P beside owns(*q) of an int: the loop writes *q and p[i].x; the cell place is held apart from the range's leaf places (the guards and case splits per leaf)",
       [=](Kit& k)
       {
           lbeside(k, false);
       });
    lc("e1e2c1_loop_beside_cell_body_vacuity", "the same with check(false) at the end of the body: it fails",
       [=](Kit& k)
       {
           vac(k, lbeside(k, true));
       });
    // void nest(struct P *p, int n) requires(owns_range(p, n) && n < 100) ensures(owns_range(p, n))
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { int j = 0; while (j < n) inv 0 <= j <= n decreases n - j { p[j].y = i; [check(false);] j++; } i++; } }
    const auto lnest = [=](Kit& k, bool vacuity) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("nest", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, RO(k, f, 0));
        Var* i {k.local(f, "i", k.i32)};
        Var* j {k.local(f, "j", k.i32)};
        const Stmt* c {vacuity ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> ib {k.store(bat(k, f, 0, k.v(j), 1), k.v(i))};
        if (c != nullptr)
        {
            ib.push_back(c);
        }
        ib.push_back(inc(k, j));
        const Stmt* inner {
            k.whl(k.lt(k.v(j), k.pv(f, 1)), {k.cl(bounds(k, f, j, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(j)))}, k.blkc(ib))};
        const Stmt* outer {k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))},
                                 {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc({k.decl(j, N(k, "0")), inner, inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "0")), outer});
        k.done(f);
        return c;
    };
    lc("e1e2c1_loop_nested_loops",
       "two nested loops over one owned range of structures: each carries every leaf place, its own label and rows",
       [=](Kit& k)
       {
           lnest(k, false);
       });
    lc("e1e2c1_loop_nested_loops_vacuity", "the same with check(false) at the end of the inner body: it fails",
       [=](Kit& k)
       {
           vac(k, lnest(k, true));
       });
    // int g(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && r == 7) { p[1].x = 7; return p[1].x; }  (mk_g)
    // int f(struct P *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n)) { int i = 0; while (i < 2) inv 0 <= i <= 2 decreases 2 - i { [p[1].y = 5;] g(p, n); [check(false);] i++; } return 0; }
    const auto lcall = [=](Kit& k, bool vacuity) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* g {mk_g(k, P, true, 2)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
        k.ens(f, RO(k, f, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* c {vacuity ? k.chk(k.no()) : nullptr};
        std::vector<const Stmt*> bs {k.es(k.call(g, {k.pn(f->param_var(0)), k.pn(f->param_var(1))}))};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        bs.push_back(inc(k, i));
        const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "2")),
                                {k.cl(k.land(k.ge(k.v(i), N(k, "0")), k.le(k.v(i), N(k, "2"))))},
                                {k.cl(k.sub(N(k, "2"), k.v(i)))}, k.blkc(bs))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
        k.done(f);
        return c;
    };
    lc("e1e2c1_loop_calls_a_range_function",
       "the loop calls g(p, n), which takes the whole range and gives it back: the range places' marked rows hold at the back edge",
       [=](Kit& k)
       {
           lcall(k, false);
       });
    lc("e1e2c1_loop_calls_a_range_function_vacuity", "the same with check(false) after the call: it fails",
       [=](Kit& k)
       {
           vac(k, lcall(k, true));
       });
    // what stays refused or unframed with loaded pointers: `owns(*p[0].q)`, `owns_block(p[0].q)` in a contract; `&p[1].x` as a value and `p[0].q[0]` in a loop invariant
    const auto mk_r = [=](Kit& k) -> StructType*
    {
        return k.strukt("R", {{"q", k.ptr(k.i32)}, {"y", k.i32}});
    };
    rc("e1e2c1_rej_atom_through_an_element_pointer", "a permission atom of something other than a pointer parameter",
       [=](Kit& k)
       {
           // requires(reads_range(p, n) && owns(*p[0].q))
           StructType* R {mk_r(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
           const Expr* q {k.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "0"), k.at()), 0, k.at()))};
           k.req(f, k.land(RR(k, f, 0), k.b.atom(AtomKind::Owns, q, 1, k.at())));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c1_rej_owns_block_through_an_element_pointer",
       "an `owns_block` atom of something other than a pointer parameter",
       [=](Kit& k)
       {
           // requires(reads_range(p, n) && owns_block(p[0].q))
           StructType* R {mk_r(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
           const Expr* q {k.deref(k.b.member_addr(k.b.elem_addr(fx(k, f, 0), N(k, "0"), k.at()), 0, k.at()))};
           k.req(f, k.land(RR(k, f, 0), k.b.atom(AtomKind::OwnsBlock, q, -1, k.at())));
           f->body = k.blk({});
           k.done(f);
       });
    // void f(struct R *p, int n) requires(reads_range(p, n) && n > 0) ensures(reads_range(p, n)) { int i = 0; while (i < n) inv 0 <= i <= n && [p[0].q[0] >= 0 | &p[1].x == &p[1].x] decreases n - i { i++; } }
    const auto lptr = [=](Kit& k, bool address) -> const Expr*
    {
        StructType* R {mk_r(k)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(R))}, {"n", k.i32}})};
        k.req(f, k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "0"))));
        k.ens(f, RR(k, f, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* leaf {k.deref(k.b.member_addr(k.b.elem_addr(k.pn(f->param_var(0)), N(k, "0"), k.at()), 0, k.at()))};
        const Expr* clause {nullptr};
        const Expr* read {nullptr};
        if (address)
        {
            clause = k.eq(k.b.member_addr(k.b.elem_addr(k.pn(f->param_var(0)), N(k, "1"), k.at()), 1, k.at()),
                          k.b.member_addr(k.b.elem_addr(k.pn(f->param_var(0)), N(k, "1"), k.at()), 1, k.at()));
        }
        else
        {
            read = k.b.text(k.deref(k.b.elem_addr(leaf, N(k, "0"), k.at())), "p[0].q[0]");
            clause = k.ge(read, N(k, "0"));
        }
        const Stmt* loop {k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1)), k.cl(clause)},
                                {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blkc({inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop});
        k.done(f);
        return address ? clause : read;
    };
    rc("e1e2c1_rej_loop_element_pointer_pointee", "a read through a pointer loaded from a member of an element",
       [=](Kit& k)
       {
           lptr(k, false);
       });
    rc("e1e2c1_rej_loop_element_address_value",
       "the element of a pointer to a structure in a loop invariant or measure",
       [=](Kit& k)
       {
           lptr(k, true);
       });
    // void f(void) { struct S v[3] = {0}; h2(&v[0], <n>, &v[<at2>], <m>); } with h2 over struct S { int x; struct T { int a; int b; } t; }: three leaves of paths of length 1 and 2, so the pairs
    // (x, t.a) take the general row of record 0064 item 7 and (t.a, t.b) the injectivity of `member`
    const auto pair_s = [=](Kit& k, const char* n, const char* at2, const char* m) -> SrcLoc
    {
        StructType* S {nullptr};
        StructType* T {nullptr};
        mk_nested(k, S, T);
        Function* h {mk_h2(k, S, AtomKind::OwnsRange, AtomKind::OwnsRange)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(S), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        auto at = [&](const char* idx)
        {
            return k.b.elem_addr(k.addr(v), N(k, idx), k.at());
        };
        const Expr* call {k.call(h, {at("0"), N(k, n), at(at2), N(k, m)})};
        f->body = k.blkc({k.decl(v, nullptr), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2c1_pair_nested_s_ranges_disjoint",
       "h2(&v[0], 1, &v[2], 1) over struct S with a nested member: the rows of the pairs of leaves (seven: the two pairs of distinct leaves of one length are `true`; different lengths included) pass",
       [=](Kit& k)
       {
           pair_s(k, "1", "2", "1");
       });
    lc("e1e2c1_pair_nested_s_ranges_overlap_fail",
       "h2(&v[0], 2, &v[1], 1): element 1 is in both ranges; the first pair that meets (x, x) fails (permission) among seven rows",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_s(k, "2", "1", "1"), 7U, 0U, "which may be one object");
       });
    // void c2v(struct P *p, int n, int **pq) requires(owns_range(p, n) && owns(*pq) && owns(**pq) && n > 0) ensures(the same && *pq == old(*pq)) {}
    // void f(void) { struct P v[3] = {0}; int c = 0; int *pp = &<c|v[1].x>; c2v(&v[0], 3, &pp); }: a cell reached through a load beside a range over structures
    const auto pair_via = [=](Kit& k, bool alias) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* c {k.fn("c2v", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"pq", k.ptr(k.ptr(k.i32))}})};
        auto ra = [&](int amount)
        {
            return k.b.range_atom(AtomKind::OwnsRange, fx(k, c, 0), fx(k, c, 1), amount, k.at());
        };
        auto cell = [&](int amount)
        {
            return k.b.atom(AtomKind::Owns, fx(k, c, 2), amount, k.at());
        };
        auto via = [&](int amount)
        {
            return k.b.atom(AtomKind::Owns, k.deref(fx(k, c, 2)), amount, k.at());
        };
        k.req(c, k.land(k.land(k.land(ra(0), cell(1)), via(2)), k.gt(fx(k, c, 1), N(k, "0"))));
        k.ens(c,
              k.land(k.land(k.land(ra(0), cell(1)), via(2)), k.eq(k.deref(fx(k, c, 2)), k.old(k.deref(fx(k, c, 2))))));
        c->body = k.blkc({});
        k.done(c);
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        Var* cc {k.cell(f, "c", k.i32)};
        Var* pp {k.cell(f, "pp", k.ptr(k.i32))};
        const Expr* target {alias ? pplace(k, k.addr(v), "1", 0) : k.addr(cc)};
        const Expr* call {k.call(c, {k.b.elem_addr(k.addr(v), N(k, "0"), k.at()), N(k, "3"), k.addr(pp)})};
        f->body = k.blkc({k.decl(v, nullptr), k.decl(cc, N(k, "0")), k.decl(pp, target), k.es(call)});
        k.done(f);
        return call->loc;
    };
    lc("e1e2c1_pair_via_cell_outside_range",
       "c2v(&v[0], 3, &pp) with pp = &c: owns(**pq) names the separate cell c, outside the range's leaf cells: the rows pass",
       [=](Kit& k)
       {
           pair_via(k, false);
       });
    lc("e1e2c1_pair_via_cell_alias_fail",
       "pp = &v[1].x: the cell reached through the load is a leaf of element 1, which the range holds: the row of leaf x fails (permission)",
       [=](Kit& k)
       {
           k.fail_only("permission", pair_via(k, true), 2U, 0U, "which may be one object");
       });
    lc("e1e2c1_range_write_read",
       "owns_range(p, n) over struct P, n > 1: p[1].x written and read back, the ensures keeps the range",
       [=](Kit& k)
       {
           write_read(k, "1");
       });
    // int f(struct P *p, int n) requires(reads_range(p, n) && n > 1) ensures(reads_range(p, n)) { return p[1].y; }
    lc("e1e2c1_range_read_only",
       "reads_range(p, n) over struct P, n > 1: p[1].y read under the ghost amount, the ensures returns the range",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(f, k.land(RR(k, f, 0), k.gt(fx(k, f, 1), N(k, "1"))));
           k.ens(f, RR(k, f, 0));
           f->body = k.blkc({k.ret(k.deref(pplace(k, k.pv(f, 0), "1", 1)))});
           k.done(f);
       });
    // ---- slice 5 (record 0064 items 7, 8 and 10; record 0060 items 12 and 13): `owning(n)` and the markers over a range of structures ----
    // `owning(n)` on `struct P *p` is `p == NULL || owns_range(p, n)` in both lists (0060 item 13): the atom under its guard in the contract, at a call, at a return and in a loop
    auto nullp = [](Kit& k, StructType* P)
    {
        return k.nul(k.ptr(k.st(P)));
    };
    auto ownp = [=](Kit& k, Function* f, StructType* P, int amount)
    {
        return k.lor(k.eq(fx(k, f, 0), nullp(k, P)), RO(k, f, amount));
    };
    // void g(struct P *p, int n) requires(p == NULL || owns_range(p, n)) ensures(the same) { if (p != NULL && n > 1) { p[1].x = 7; } [check(false);] }
    const auto own_g = [=](Kit& k, StructType* P, bool vac_) -> Function*
    {
        Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(g, ownp(k, g, P, 0));
        k.ens(g, ownp(k, g, P, 0));
        std::vector<const Stmt*> bs {k.iff(k.land(k.ne(k.pv(g, 0), nullp(k, P)), k.gt(k.pv(g, 1), N(k, "1"))),
                                           k.blk({k.store(pplace(k, k.pv(g, 0), "1", 0), N(k, "7"))}))};
        if (vac_)
        {
            bs.push_back(k.chk(k.no()));
        }
        g->body = k.blkc(bs);
        k.done(g);
        return g;
    };
    lc("e1e2c1_owning_write",
       "owning(n) callee: p[1].x written under p != NULL && n > 1, the guarded range holds the permission",
       [=](Kit& k)
       {
           own_g(k, mk_p(k), false);
       });
    lc("e1e2c1_owning_write_vacuity",
       "the same with check(false) at the end of the body: it fails (the body is reachable)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* g {own_g(k, P, true)};
           k.fail("check", g->body->stmts.back()->loc);
       });
    lc("e1e2c1_owning_write_unguarded_fail",
       "the write p[1].x without p != NULL in its condition: on the null path the guarded atom holds nothing (null-dereference at the store, once)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           k.req(g, ownp(k, g, P, 0));
           k.ens(g, ownp(k, g, P, 0));
           const Stmt* w {k.store(pplace(k, k.pv(g, 0), "1", 0), N(k, "7"))};
           g->body = k.blkc({k.iff(k.gt(k.pv(g, 1), N(k, "1")), k.blk({w}))});
           k.done(g);
           // the null path reaches the store with the guarded atom holding nothing: the null row fails and ends the path, so it is the one error
           k.fail("null-dereference", w->loc);
       });
    // void f(void) { [struct P v[<len>] = {0}; g(&v[0], <n>) | g(NULL, 5)]; [check(false);] }
    const auto own_caller = [=](Kit& k, int mode, unsigned len, const char* n) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {own_g(k, P, false)};
        Function* f {k.fn("f", k.vd)};
        if (mode == 0)
        {
            const Expr* c {k.call(g, {nullp(k, P), N(k, "5")})};
            f->body = k.blkc({k.es(c)});
            k.done(f);
            return c->loc;
        }
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), len), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = true;
        const Expr* c {k.call(g, {k.b.elem_addr(k.addr(v), N(k, "0"), k.at()), N(k, n)})};
        // after the call the array is still held: v[2].y is read (its flag is in the callee's ensures)
        Var* r {k.local(f, "r", k.i32)};
        std::vector<const Stmt*> bs {k.decl(v, nullptr), k.es(c), k.decl(r, k.deref(pplace(k, k.addr(v), "2", 1)))};
        const Stmt* last {mode == 2 ? k.chk(k.no()) : nullptr};
        if (last != nullptr)
        {
            bs.push_back(last);
        }
        f->body = k.blkc(bs);
        k.done(f);
        return mode == 2 ? last->loc : c->loc;
    };
    lc("e1e2c1_owning_caller_null", "g(NULL, 5): the guard of the callee's atom is false, no row of the range applies",
       [=](Kit& k)
       {
           own_caller(k, 0, 0U, "0");
       });
    lc("e1e2c1_owning_caller_array",
       "struct P v[3] = {0}; g(&v[0], 3): the range holds, v[2].y is still readable after the call",
       [=](Kit& k)
       {
           own_caller(k, 1, 3U, "3");
       });
    lc("e1e2c1_owning_caller_array_vacuity", "the same with check(false) after it: it fails",
       [=](Kit& k)
       {
           k.fail("check", own_caller(k, 2, 3U, "3"));
       });
    lc("e1e2c1_owning_caller_too_long_fail",
       "struct P v[3] = {0}; g(&v[0], 4): the callee's guard holds, the shape row of the range fails alone (permission at the call)",
       [=](Kit& k)
       {
           k.fail_only("permission", own_caller(k, 1, 3U, "4"), 12U, 0U);
       });
    // void f(struct P *p, int n) requires(p == NULL || owns_range(p, n)) ensures(the same) { g(p, n [+ 1]); }
    const auto own_forward = [=](Kit& k, bool more) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {own_g(k, P, false)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(ownp(k, f, P, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, ownp(k, f, P, 0));
        const Expr* c {k.call(g, {k.pv(f, 0), more ? k.add(k.pv(f, 1), N(k, "1")) : k.pv(f, 1)})};
        f->body = k.blkc({k.es(c)});
        k.done(f);
        return c->loc;
    };
    lc("e1e2c1_owning_forward",
       "f forwards its owning(n) parameter to g: the call rows and the return rows under the guard",
       [=](Kit& k)
       {
           own_forward(k, false);
       });
    lc("e1e2c1_owning_forward_one_more_fail",
       "f holds p[0..n) (n < 100) and calls g(p, n + 1): on the non-null path the shape row, the length-facts row and the marked row of slot n fail, three rows of one cause (permission at the call)",
       [=](Kit& k)
       {
           const SrcLoc at {own_forward(k, true)};
           // of the call's 12 rows: the shape row (0), the length facts of slot n (4) and the marked value row of member x (5) fail; the first marked row ends its path
           k.fail_only("permission", at, 12U, 0U);
           k.fail_only("permission", at, 12U, 4U);
           k.fail_only("permission", at, 12U, 5U);
       });
    // void fillg(struct P *p, int n) requires((p == NULL || owns_range(p, n)) && n < 100) ensures(the same)
    //   { if (p != NULL) { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { p[i].x = i; i++; } } [check(false);] }
    const auto own_loop = [=](Kit& k, bool vac_) -> const Stmt*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("fillg", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(f, k.land(ownp(k, f, P, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, ownp(k, f, P, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* loop {k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))},
                                k.blkc({k.store(bat(k, f, 0, k.v(i), 0), k.v(i)), inc(k, i)}))};
        std::vector<const Stmt*> bs {k.iff(k.ne(k.pv(f, 0), nullp(k, P)), k.blk({k.decl(i, N(k, "0")), loop}))};
        const Stmt* c {vac_ ? k.chk(k.no()) : nullptr};
        if (c != nullptr)
        {
            bs.push_back(c);
        }
        f->body = k.blkc(bs);
        k.done(f);
        return c;
    };
    lc("e1e2c1_owning_loop",
       "the fill loop under p != NULL: the range places carry the guard, the null path skips the loop",
       [=](Kit& k)
       {
           own_loop(k, false);
       });
    lc("e1e2c1_owning_loop_vacuity", "the same with check(false) after the if: it fails",
       [=](Kit& k)
       {
           vac(k, own_loop(k, true));
       });
    // ---- the markers (record 0060 item 12, per leaf path) ----
    // void fill(struct P *p, int n) [must_init | ref_to_uninit] requires(owns_range(p, n) && n == 2) ensures(owns_range(p, n))
    //   { writes of p[0].x, p[0].y, p[1].x, p[1].y except the `skip`th (0 to 3), [int r = p[0].x first when `read_first`]; [check(false);] }
    const auto mfill = [=](Kit& k, StructType* P, InitMarker m, int skip, const Expr** read_at, bool vac_) -> Function*
    {
        Function* f {k.fn("fill", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        const bool read_first {read_at != nullptr};
        const Expr* first_read {read_first ? k.deref(pplace(k, k.pv(f, 0), "0", 0)) : nullptr};
        if (read_first)
        {
            *read_at = first_read;
        }
        f->params[0U].init = m;
        k.req(f, k.land(RO(k, f, 0), k.eq(fx(k, f, 1), N(k, "2"))));
        k.ens(f, RO(k, f, 0));
        std::vector<const Stmt*> bs;
        if (read_first)
        {
            bs.push_back(k.decl(k.local(f, "r", k.i32), first_read));
        }
        for (int w {0}; w < 4; ++w)
        {
            if (w != skip)
            {
                bs.push_back(k.store(pplace(k, k.pv(f, 0), w < 2 ? "0" : "1", w % 2), N(k, "1")));
            }
        }
        if (vac_)
        {
            bs.push_back(k.chk(k.no()));
        }
        f->body = k.blkc(bs);
        k.done(f);
        return f;
    };
    lc("e1e2c1_mark_must_init_fill",
       "must_init filler of two elements writes every member: the flag row of each leaf holds at the return",
       [=](Kit& k)
       {
           mfill(k, mk_p(k), InitMarker::MustInit, -1, nullptr, false);
       });
    lc("e1e2c1_mark_must_init_fill_vacuity",
       "the same with check(false) at the end: it fails (the body is reachable, the requires has no flag)",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {mfill(k, P, InitMarker::MustInit, -1, nullptr, true)};
           k.fail("check", f->body->stmts.back()->loc);
       });
    // the twins: the filler misses one member of one element (leaf y of element 1, then leaf x of element 0): exactly one error at the return, of that leaf's flag row
    lc("e1e2c1_mark_must_init_misses_y_of_one_fail",
       "the filler leaves p[1].y unwritten: the flag row of leaf y fails once at the return (postcondition), the row of leaf x holds",
       [=](Kit& k)
       {
           Function* f {mfill(k, mk_p(k), InitMarker::MustInit, 3, nullptr, false)};
           k.fail("postcondition", f->end_loc, 1U, "", "member `y`");
       });
    lc("e1e2c1_mark_must_init_misses_x_of_zero_fail",
       "the filler leaves p[0].x unwritten: the flag row of leaf x fails once at the return (postcondition)",
       [=](Kit& k)
       {
           Function* f {mfill(k, mk_p(k), InitMarker::MustInit, 0, nullptr, false)};
           k.fail("postcondition", f->end_loc, 1U, "", "member `x`");
       });
    lc("e1e2c1_mark_ref_to_uninit_writes_some",
       "ref_to_uninit writer of three of the four leaves: no flag is promised, the kept-flag rows hold",
       [=](Kit& k)
       {
           mfill(k, mk_p(k), InitMarker::RefToUninit, 2, nullptr, false);
       });
    lc("e1e2c1_mark_ref_to_uninit_reads_first_fail",
       "ref_to_uninit reader of p[0].x before any write: the requires has no flag of any leaf (uninitialized at the read)",
       [=](Kit& k)
       {
           const Expr* rd {nullptr};
           mfill(k, mk_p(k), InitMarker::RefToUninit, -1, &rd, false);
           k.fail("uninitialized", rd->loc);
       });
    lc("e1e2c1_mark_ref_to_uninit_reads_other_leaf_fail",
       "ref_to_uninit: p[0].y is written, then p[0].x is read (the flag of leaf x of element 0 is unknown): uninitialized once, at that read",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("fill", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::RefToUninit;
           k.req(f, k.land(RO(k, f, 0), k.eq(fx(k, f, 1), N(k, "2"))));
           k.ens(f, RO(k, f, 0));
           const Stmt* w {k.store(pplace(k, k.pv(f, 0), "0", 1), N(k, "1"))};
           const Expr* rd {k.deref(pplace(k, k.pv(f, 0), "0", 0))};
           f->body = k.blkc({w, k.decl(k.local(f, "r", k.i32), rd)});
           k.done(f);
           k.fail("uninitialized", rd->loc);
       });
    lc("e1e2c1_mark_ref_to_uninit_reads_other_element_fail",
       "ref_to_uninit: p[0].x is written, then p[1].x is read (the flags are per slot: element 1 has none): uninitialized once, at that read",
       [=](Kit& k)
       {
           StructType* P {mk_p(k)};
           Function* f {k.fn("fill", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::RefToUninit;
           k.req(f, k.land(RO(k, f, 0), k.eq(fx(k, f, 1), N(k, "2"))));
           k.ens(f, RO(k, f, 0));
           const Stmt* w {k.store(pplace(k, k.pv(f, 0), "0", 0), N(k, "1"))};
           const Expr* rd {k.deref(pplace(k, k.pv(f, 0), "1", 0))};
           f->body = k.blkc({w, k.decl(k.local(f, "r", k.i32), rd)});
           k.done(f);
           k.fail("uninitialized", rd->loc);
       });
    // void fill(struct P *p, int n) [must_init | ref_to_uninit] requires(owns_range(p, n) && n < 100) ensures(owns_range(p, n))
    //   { int i = 0; while (i < n) inv 0 <= i <= n decreases n - i { p[i].x = i; p[i].y = i; i++; } }
    const auto mloop = [=](Kit& k, InitMarker m) -> Function*
    {
        StructType* P {mk_p(k)};
        Function* f {k.fn("fill", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        f->params[0U].init = m;
        k.req(f, k.land(RO(k, f, 0), k.lt(fx(k, f, 1), N(k, "100"))));
        k.ens(f, RO(k, f, 0));
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* loop {k.whl(
            k.lt(k.v(i), k.pv(f, 1)), {k.cl(bounds(k, f, i, 1))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))},
            k.blkc({k.store(bat(k, f, 0, k.v(i), 0), k.v(i)), k.store(bat(k, f, 0, k.v(i), 1), k.v(i)), inc(k, i)}))};
        f->body = k.blk({k.decl(i, N(k, "0")), loop});
        k.done(f);
        return f;
    };
    lc("e1e2c1_mark_ref_loop_fill",
       "ref_to_uninit over struct P: the loop writes p[i].x and p[i].y, the range places of both leaves carry the permissions and keep the flags that were set",
       [=](Kit& k)
       {
           mloop(k, InitMarker::RefToUninit);
       });
    lc("e1e2c1_mark_must_init_loop_pinned_fail",
       "a LISTED FALSE ALARM: the same loop under must_init: the flags the loop sets are stated by no automatic invariant (a quantified invariant over the members is Part F's), so the flag row of each leaf fails at the return (postcondition, x and y)",
       [=](Kit& k)
       {
           Function* f {mloop(k, InitMarker::MustInit)};
           k.fail("postcondition", f->end_loc, 2U);
       });
    // void outer(struct P *p, int n) [must_init] requires(owns_range(p, n) && n == 2) ensures(owns_range(p, n)) { fill(p, n); }
    const auto mouter = [=](Kit& k, InitMarker callee_marker, InitMarker outer_marker) -> Function*
    {
        StructType* P {mk_p(k)};
        Function* callee {mfill(k, P, callee_marker, -1, nullptr, false)};
        Function* f {k.fn("outer", k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        f->params[0U].init = outer_marker;
        k.req(f, k.land(RO(k, f, 0), k.eq(fx(k, f, 1), N(k, "2"))));
        k.ens(f, RO(k, f, 0));
        f->body = k.blkc({k.es(k.call(callee, {k.pv(f, 0), k.pv(f, 1)}))});
        k.done(f);
        return f;
    };
    lc("e1e2c1_mark_must_init_forward",
       "a must_init forwarder: the callee's ensures gives the flag of every leaf, so the return rows hold",
       [=](Kit& k)
       {
           mouter(k, InitMarker::MustInit, InitMarker::MustInit);
       });
    lc("e1e2c1_mark_must_init_forward_ref_fail",
       "a must_init forwarder over a ref_to_uninit callee that writes everything: no flag is promised, so the flag row of each leaf fails at the return (two errors: x and y)",
       [=](Kit& k)
       {
           Function* f {mouter(k, InitMarker::RefToUninit, InitMarker::MustInit)};
           k.fail("postcondition", f->end_loc, 2U);
       });
    lc("e1e2c1_mark_ref_forward",
       "a ref_to_uninit forwarder over a must_init callee: the requires claims no flag, the call has no flag row, the kept-flag rows hold",
       [=](Kit& k)
       {
           mouter(k, InitMarker::MustInit, InitMarker::RefToUninit);
       });
    // void f(void) { struct P v[2] [= {0}]; fill(&v[0], 2); int r = v[1].y; [check(false);] }
    const auto mcaller = [=](Kit& k, InitMarker m, bool zero, bool vac_) -> SrcLoc
    {
        StructType* P {mk_p(k)};
        Function* g {mfill(k, P, m, -1, nullptr, false)};
        Function* f {k.fn("f", k.vd)};
        Var* v {f->new_local("v", k.prog.array_of(k.st(P), 2U), k.at())};
        f->mark_address_taken(v);
        v->zero_filled = zero;
        Var* r {k.local(f, "r", k.i32)};
        const Expr* c {k.call(g, {k.b.elem_addr(k.addr(v), N(k, "0"), k.at()), N(k, "2")})};
        const Expr* rd {k.deref(pplace(k, k.addr(v), "1", 1))};
        std::vector<const Stmt*> bs {k.decl(v, nullptr), k.es(c), k.decl(r, rd)};
        const Stmt* ck {vac_ ? k.chk(k.no()) : nullptr};
        if (ck != nullptr)
        {
            bs.push_back(ck);
        }
        f->body = k.blkc(bs);
        k.done(f);
        return vac_ ? ck->loc : rd->loc;
    };
    lc("e1e2c1_mark_must_init_caller",
       "struct P v[2] with no value; fill(&v[0], 2) is must_init: no flag row at the call, and v[1].y has its flag afterwards",
       [=](Kit& k)
       {
           mcaller(k, InitMarker::MustInit, false, false);
       });
    lc("e1e2c1_mark_must_init_caller_vacuity", "the same with check(false) after the read: it fails",
       [=](Kit& k)
       {
           k.fail("check", mcaller(k, InitMarker::MustInit, false, true));
       });
    lc("e1e2c1_mark_ref_caller_unset_fail",
       "struct P v[2] with no value; the ref_to_uninit callee writes all four members but promises no flag: the read of v[1].y fails (uninitialized)",
       [=](Kit& k)
       {
           k.fail("uninitialized", mcaller(k, InitMarker::RefToUninit, false, false));
       });
    lc("e1e2c1_mark_ref_caller_kept",
       "struct P v[2] = {0}; the ref_to_uninit callee keeps every flag that was set: v[1].y is readable",
       [=](Kit& k)
       {
           mcaller(k, InitMarker::RefToUninit, true, false);
       });
    lc("e1e2c1_mark_ref_caller_kept_vacuity", "the same with check(false) after the read: it fails",
       [=](Kit& k)
       {
           k.fail("check", mcaller(k, InitMarker::RefToUninit, true, true));
       });
    // a leaf of depth 2: struct Q { int a; struct T { int b; int c; } t; }; the filler misses p[1].t.c
    lc("e1e2c1_mark_must_init_nested_misses_fail",
       "must_init over struct Q with a nested struct: the filler writes every leaf of both elements but p[1].t.c: one flag row fails (postcondition, member t.c)",
       [=](Kit& k)
       {
           StructType* T {k.strukt("T", {{"b", k.i32}, {"c", k.i32}})};
           StructType* Q {k.strukt("Q", {{"a", k.i32}, {"t", k.st(T)}})};
           Function* f {k.fn("fill", k.vd, {{"p", k.ptr(k.st(Q))}, {"n", k.i32}})};
           f->params[0U].init = InitMarker::MustInit;
           k.req(f, k.land(RO(k, f, 0), k.eq(fx(k, f, 1), N(k, "2"))));
           k.ens(f, RO(k, f, 0));
           auto leaf = [&](const char* e, int m1, int m2)
           {
               const Expr* x {k.b.member_addr(k.b.elem_addr(k.pv(f, 0), N(k, e), k.at()), m1, k.at())};
               return m2 < 0 ? x : k.b.member_addr(x, m2, k.at());
           };
           f->body = k.blkc({k.store(leaf("0", 0, -1), N(k, "1")), k.store(leaf("0", 1, 0), N(k, "1")),
                             k.store(leaf("0", 1, 1), N(k, "1")), k.store(leaf("1", 0, -1), N(k, "1")),
                             k.store(leaf("1", 1, 0), N(k, "1"))});
           k.done(f);
           k.fail("postcondition", f->end_loc, 1U, "", "t.c");
       });
}

// ---- Part C2 (record 0064 items 3, 6, 7, 9, 10 and 12): arrays of structures in static storage and as members ----
// Slice 1: a member that is an array of structures, struct P { int x; int y; } and struct T { struct P v[2]; int n; }: member 0 is the array, member 1 the cell n.
void define_member_struct_array_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mk_t = [](Kit& k)
    {
        StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(P), 2U)}, {"n", k.i32}});
    };
    // `base->v[i].m` (a place) for the member m of the element
    auto vplace = [](Kit& k, const Expr* base, const char* i, int m)
    {
        return k.b.member_addr(k.b.elem_addr(k.mem(base, 0), k.num(k.i32, i), k.at()), m, k.at());
    };
    auto vread = [=](Kit& k, const Expr* base, const char* i, int m)
    {
        return k.deref(vplace(k, base, i, m));
    };
    auto vwrite = [=](Kit& k, const Expr* base, const char* i, int m, const char* v)
    {
        return k.store(vplace(k, base, i, m), N(k, v));
    };
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto tail = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
    };
    // every leaf of t written: v[0] = {1, 2}, v[1] = {3, 4}, n = nv
    auto fill = [=](Kit& k, Var* t, std::vector<const Stmt*>& body, const char* nv)
    {
        body.push_back(k.decl(t, nullptr));
        body.push_back(vwrite(k, k.addr(t), "0", 0, "1"));
        body.push_back(vwrite(k, k.addr(t), "0", 1, "2"));
        body.push_back(vwrite(k, k.addr(t), "1", 0, "3"));
        body.push_back(vwrite(k, k.addr(t), "1", 1, "4"));
        body.push_back(k.lstore(k.addr(t), 1, N(k, nv)));
    };
    // void f(void) { struct T t; ...fill...; check(t.v[1].x == 3 && t.v[0].y == 2 && t.n == 7); t.v[1].x = 9; check(t.v[1].x == 9 && t.v[1].y == 4); }
    lcv("e1e2c2_member_local_rw",
        "a structure local whose member is an array of structures: every leaf of both elements written and read back, the scope exit per leaf",
        [=](Kit& k)
        {
            StructType* T {mk_t(k)};
            Function* f {k.fn("f", k.vd)};
            Var* t {k.scell(f, "t", T)};
            std::vector<const Stmt*> body;
            fill(k, t, body, "7");
            body.push_back(k.chk(k.land(
                k.eq_c(vread(k, k.addr(t), "1", 0), k.i32, "3"),
                k.land(k.eq_c(vread(k, k.addr(t), "0", 1), k.i32, "2"), k.eq_c(k.leaf(k.addr(t), 1), k.i32, "7")))));
            body.push_back(vwrite(k, k.addr(t), "1", 0, "9"));
            body.push_back(k.chk(k.land(k.eq_c(vread(k, k.addr(t), "1", 0), k.i32, "9"),
                                        k.eq_c(vread(k, k.addr(t), "1", 1), k.i32, "4"))));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    lc("e1e2c2_member_local_index_outside_fail", "t.v[2].x of struct P v[2]: the bound row (out-of-bounds) at the read",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           std::vector<const Stmt*> body;
           fill(k, t, body, "7");
           const Expr* r {vread(k, k.addr(t), "2", 0)};
           body.push_back(k.chk(k.eq_c(r, k.i32, "0")));
           f->body = k.blkc(body);
           k.done(f);
           k.fail("out-of-bounds", r->loc);
       });
    lc("e1e2c2_member_local_unwritten_fail", "t.v[1].y read before any write: uninitialized at the read",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           const Expr* r {vread(k, k.addr(t), "1", 1)};
           f->body = k.blkc({k.decl(t, nullptr), vwrite(k, k.addr(t), "1", 0, "3"), k.chk(k.eq_c(r, k.i32, "0"))});
           k.done(f);
           k.fail("uninitialized", r->loc);
       });
    // void g(struct T *p) requires(owns(*p)) ensures(owns(*p) && p->n == <post>) { p->v[1].x = 7; check(p->v[1].x == 7); p->n = 3; }
    auto make_g = [=](Kit& k, StructType* T, const char* post) -> Function*
    {
        Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(T))}})};
        k.req(g, k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "p"), 0, k.at()));
        k.ens(g, k.land(k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "p"), 0, k.at()),
                        k.eq_c(k.leaf(k.b.text(k.F(g, 0), "p"), 1), k.i32, post)));
        g->body = k.blk({vwrite(k, k.pv(g, 0), "1", 0, "7"), k.chk(k.eq_c(vread(k, k.pv(g, 0), "1", 0), k.i32, "7")),
                         k.lstore(k.pv(g, 0), 1, N(k, "3"))});
        k.done(g);
        return g;
    };
    // void f(void) { struct T t; ...fill...; g(&t); check(t.n == 3); }
    lcv("e1e2c2_member_owns_call",
        "g(&t) takes owns(*p) of a structure whose member is an array of structures: the rows per leaf of the array over the bound local; the callee writes p->v[1].x; the caller reads t.n back",
        [=](Kit& k)
        {
            StructType* T {mk_t(k)};
            Function* g {make_g(k, T, "3")};
            Function* f {k.fn("f", k.vd)};
            Var* t {k.scell(f, "t", T)};
            std::vector<const Stmt*> body;
            fill(k, t, body, "7");
            body.push_back(k.es(k.call(g, {k.addr(t)})));
            body.push_back(k.chk(k.eq_c(k.leaf(k.addr(t), 1), k.i32, "3")));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    lc("e1e2c2_member_owns_callee_wrong_fail",
       "g writes 3 where it promises 4: the postcondition fails at the closing brace",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* g {make_g(k, T, "4")};
           k.fail("postcondition", g->end_loc);
       });
    // void h(struct T *p, struct P *q) requires(owns(*p) && owns(*q)) ensures(owns(*p) && owns(*q)) {}
    auto make_h = [=](Kit& k, StructType* T) -> Function*
    {
        StructType* P {const_cast<StructType*>(T->members[0U].type.element->struct_type)};
        Function* h {k.fn("h", k.vd, {{"p", k.ptr(k.st(T))}, {"q", k.ptr(k.st(P))}})};
        auto own = [&](int i)
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.F(h, i), h->params[static_cast<size_t>(i)].name), i, k.at());
        };
        k.req(h, k.land(own(0), own(1)));
        k.ens(h, k.land(own(0), own(1)));
        h->body = k.blk({});
        k.done(h);
        return h;
    };
    // struct T t, u; ...fill both...; h(&t, &<second>.v[1]);
    auto h_main = [=](Kit& k, bool alias) -> SrcLoc
    {
        StructType* T {mk_t(k)};
        Function* h {make_h(k, T)};
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", T)};
        Var* u {k.scell(f, "u", T)};
        std::vector<const Stmt*> body;
        fill(k, t, body, "7");
        fill(k, u, body, "8");
        const Expr* second {k.b.elem_addr(k.mem(alias ? k.addr(t) : k.addr(u), 0), N(k, "1"), k.at())};
        const Expr* call {k.call(h, {k.addr(t), second})};
        body.push_back(k.es(call));
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        return call->loc;
    };
    lcv("e1e2c2_member_two_objects_distinct",
        "h(&t, &u.v[1]): owns(*p) of one T and owns(*q) of an element of another: the pairing rows per leaf path hold",
        [=](Kit& k)
        {
            h_main(k, false);
        });
    lc("e1e2c2_member_alias_fail",
       "h(&t, &t.v[1]): the element is a part of *p: one permission error at the call, at the pairing row of the cell q->x against the leaf x of t.v (the 20th of the call's 36 rows)",
       [=](Kit& k)
       {
           k.fail_only("permission", h_main(k, true), 36U, 19U);
       });
    // void h2(struct T *p, <q type> *q) requires(owns(*p) && owns(*q)) ensures(owns(*p) && owns(*q)) {}   with q an int (a leaf's cell) or a struct T
    auto make_h2 = [=](Kit& k, StructType* T, bool q_is_t) -> Function*
    {
        Function* h {k.fn("h2", k.vd, {{"p", k.ptr(k.st(T))}, {"q", q_is_t ? k.ptr(k.st(T)) : k.ptr(k.i32)}})};
        auto own = [&](int i)
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.F(h, i), h->params[static_cast<size_t>(i)].name), i, k.at());
        };
        k.req(h, k.land(own(0), own(1)));
        k.ens(h, k.land(own(0), own(1)));
        h->body = k.blk({});
        k.done(h);
        return h;
    };
    // struct T t, u; ...fill both...; h2(&t, <q argument>);
    auto h2_main = [=](Kit& k, int mode) -> const Expr*
    {
        // mode 0: &u.v[1].x (distinct), 1: &t.v[1].x (alias with a leaf of the array), 2: &u (a distinct structure), 3: &t (the same structure)
        StructType* T {mk_t(k)};
        Function* h {make_h2(k, T, mode >= 2)};
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", T)};
        Var* u {k.scell(f, "u", T)};
        std::vector<const Stmt*> body;
        fill(k, t, body, "7");
        fill(k, u, body, "8");
        const Expr* leaf_of_u {vplace(k, k.addr(u), "1", 0)};
        const Expr* leaf_of_t {vplace(k, k.addr(t), "1", 0)};
        const Expr* q {mode == 0 ? leaf_of_u : (mode == 1 ? leaf_of_t : (mode == 2 ? k.addr(u) : k.addr(t)))};
        const Expr* call {k.call(h, {k.addr(t), q})};
        body.push_back(k.es(call));
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        return call;
    };
    lcv("e1e2c2_member_cell_beside_array_distinct",
        "h2(&t, &u.v[1].x): owns(*p) of a T and owns(*q) of a cell of another object: the pairing rows (the cell against each leaf of t.v by slots) hold",
        [=](Kit& k)
        {
            h2_main(k, 0);
        });
    lc("e1e2c2_member_cell_alias_fail",
       "h2(&t, &t.v[1].x): the cell is a leaf of t.v: one permission error at the call, at the pairing row of the cell against the leaf x (the 16th of the call's 30 rows)",
       [=](Kit& k)
       {
           k.fail_only("permission", h2_main(k, 1)->loc, 30U, 15U);
       });
    lcv("e1e2c2_member_two_structures_distinct",
        "h2(&t, &u): two objects of struct T: the first-leaf rows of the two array leaves hold",
        [=](Kit& k)
        {
            h2_main(k, 2);
        });
    lc("e1e2c2_member_two_structures_alias_fail",
       "h2(&t, &t): one object twice: one permission error at the call, at the first pairing row (the cell n against itself: the 15th of the call's 41 rows)",
       [=](Kit& k)
       {
           k.fail_only("permission", h2_main(k, 3)->loc, 41U, 14U);
       });
    // void take(struct P *q) requires(owns(*q)) {}  (it keeps the element); struct T t; ...fill...; take(&t.v[1]); then t ends: the scope exit's rows
    lc("e1e2c2_member_element_taken_lifetime_fail",
       "take(&t.v[1]) owns the element and returns nothing: at the end of t's scope the permission to the member x of every element is incomplete (lifetime)",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           StructType* P {const_cast<StructType*>(T->members[0U].type.element->struct_type)};
           Function* take {k.fn("take", k.vd, {{"q", k.ptr(k.st(P))}})};
           k.req(take, k.b.atom(AtomKind::Owns, k.b.text(k.F(take, 0), "q"), 0, k.at()));
           take->body = k.blk({});
           k.done(take);
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           std::vector<const Stmt*> body;
           fill(k, t, body, "7");
           body.push_back(k.es(k.call(take, {k.b.elem_addr(k.mem(k.addr(t), 0), N(k, "1"), k.at())})));
           const Stmt* blk {k.blkc(body)};
           f->body = blk;
           k.done(f);
           k.fail_only("lifetime", blk->end_loc, 6U, 1U);
       });
    // a loop in a callee over the member of a pointer parameter: void fillv(struct T *p) requires(owns(*p)) ensures(owns(*p)) { int i = 0; while (i < 2) inv 0 <= i <= 2 { p->v[i].x = 5; i++; } int z = p->v[1].y; }
    lcv("e1e2c2_member_loop_through_a_pointer",
        "owns(*p) held by a callee whose loop writes p->v[i].x: the member's leaves are range places of the loop (the structure place's array part), each leaf its own; the other leaves stay readable",
        [=](Kit& k)
        {
            StructType* T {mk_t(k)};
            Function* g {k.fn("fillv", k.vd, {{"p", k.ptr(k.st(T))}})};
            k.req(g, k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "p"), 0, k.at()));
            k.ens(g, k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "p"), 0, k.at()));
            Var* i {k.local(g, "i", k.i32)};
            Var* z {k.local(g, "z", k.i32)};
            const Expr* place {k.b.member_addr(k.b.elem_addr(k.mem(k.pv(g, 0), 0), k.pn(i), k.at()), 0, k.at())};
            std::vector<const Stmt*> body {k.decl(i, N(k, "0")),
                                           k.whl(k.lt(k.v(i), N(k, "2")), {k.cl(k.range(k.v(i), k.i32, "0", "2"))},
                                                 {k.cl(k.sub(N(k, "2"), k.v(i)))},
                                                 k.blkc({k.store(place, N(k, "5")), k.incS(i)})),
                                           k.decl(z, vread(k, k.pv(g, 0), "1", 1))};
            tail(k, body);
            g->body = k.blkc(body);
            k.done(g);
        });
    // struct R { int b; int c; }; struct Q { int a; struct R r; }; struct U { struct Q v[2]; int n; }: a leaf of depth 2 inside the element
    auto mk_u = [](Kit& k)
    {
        StructType* R {k.strukt("R", {{"b", k.i32}, {"c", k.i32}})};
        StructType* Q {k.strukt("Q", {{"a", k.i32}, {"r", k.st(R)}})};
        return k.strukt("U", {{"v", k.prog.array_of(k.st(Q), 2U)}, {"n", k.i32}});
    };
    // `base->v[i].r.c` and `.a`: the places of the element's leaves
    auto qleaf = [](Kit& k, const Expr* base, const char* i, int m1, int m2)
    {
        const Expr* e {k.b.member_addr(k.b.elem_addr(k.mem(base, 0), k.num(k.i32, i), k.at()), m1, k.at())};
        return m2 < 0 ? e : k.b.member_addr(e, m2, k.at());
    };
    auto fill_u = [=](Kit& k, Var* t, std::vector<const Stmt*>& body)
    {
        body.push_back(k.decl(t, nullptr));
        for (const char* i : {"0", "1"})
        {
            body.push_back(k.store(qleaf(k, k.addr(t), i, 0, -1), N(k, "1")));
            body.push_back(k.store(qleaf(k, k.addr(t), i, 1, 0), N(k, "2")));
            body.push_back(k.store(qleaf(k, k.addr(t), i, 1, 1), N(k, "3")));
        }
        body.push_back(k.lstore(k.addr(t), 1, N(k, "7")));
    };
    lcv("e1e2c2_member_nested_leaf",
        "struct U { struct Q v[2]; int n; } with struct Q { int a; struct R { int b; int c; } r; }: a leaf of depth 2 (v[i].r.c) through the allocation, the writes, the reads and the scope exit",
        [=](Kit& k)
        {
            StructType* U {mk_u(k)};
            Function* f {k.fn("f", k.vd)};
            Var* t {k.scell(f, "t", U)};
            std::vector<const Stmt*> body;
            fill_u(k, t, body);
            body.push_back(k.chk(k.land(k.eq_c(k.deref(qleaf(k, k.addr(t), "1", 1, 1)), k.i32, "3"),
                                        k.eq_c(k.deref(qleaf(k, k.addr(t), "0", 0, -1)), k.i32, "1"))));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    // void hr(struct U *p, struct R *q) requires(owns(*p) && owns(*q)) ensures(same) {}: q = &t.v[1].r (a nested slot of an element) or &u.v[1].r
    auto nested_alias = [=](Kit& k, bool alias) -> const Expr*
    {
        StructType* U {mk_u(k)};
        StructType* R {const_cast<StructType*>(U->members[0U].type.element->struct_type->members[1U].type.struct_type)};
        Function* h {k.fn("hr", k.vd, {{"p", k.ptr(k.st(U))}, {"q", k.ptr(k.st(R))}})};
        auto own = [&](int i)
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.F(h, i), h->params[static_cast<size_t>(i)].name), i, k.at());
        };
        k.req(h, k.land(own(0), own(1)));
        k.ens(h, k.land(own(0), own(1)));
        h->body = k.blk({});
        k.done(h);
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", U)};
        Var* u {k.scell(f, "u", U)};
        std::vector<const Stmt*> body;
        fill_u(k, t, body);
        fill_u(k, u, body);
        const Expr* q {
            k.b.member_addr(k.b.elem_addr(k.mem(alias ? k.addr(t) : k.addr(u), 0), N(k, "1"), k.at()), 1, k.at())};
        const Expr* call {k.call(h, {k.addr(t), q})};
        body.push_back(k.es(call));
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        return call;
    };
    lcv("e1e2c2_member_nested_slot_distinct",
        "hr(&t, &u.v[1].r): owns(*p) of a U and owns(*q) of the nested structure of an element of another: each leaf of q against each leaf path of the array holds",
        [=](Kit& k)
        {
            nested_alias(k, false);
        });
    lc("e1e2c2_member_nested_slot_alias_fail",
       "hr(&t, &t.v[1].r): the nested structure of an element of t.v: one permission error at the call, at the pairing row of q->b against the leaf r.b of t.v (the 21st of the call's 41 rows)",
       [=](Kit& k)
       {
           k.fail_only("permission", nested_alias(k, true)->loc, 41U, 20U);
       });
    // void put(struct T *p) requires(owns(p->v)) ensures(owns(p->v)) { p->v[1].x = 5; [p->n = 1;] }
    auto make_put = [=](Kit& k, StructType* T, bool writes_n) -> Function*
    {
        Function* f {k.fn("put", k.vd, {{"p", k.ptr(k.st(T))}})};
        k.req(f, k.b.atom(AtomKind::Owns, k.mem(k.b.text(k.F(f, 0), "p"), 0), 0, k.at()));
        k.ens(f, k.b.atom(AtomKind::Owns, k.mem(k.b.text(k.F(f, 0), "p"), 0), 0, k.at()));
        std::vector<const Stmt*> body {vwrite(k, k.pv(f, 0), "1", 0, "5")};
        const Stmt* w {nullptr};
        if (writes_n)
        {
            w = k.lstore(k.pv(f, 0), 1, N(k, "1"));
            body.push_back(w);
        }
        f->body = k.blk(body);
        k.done(f);
        if (writes_n)
        {
            k.fail("permission", w->loc);
        }
        return f;
    };
    lcv("e1e2c2_member_owns_member",
        "owns(p->v) alone: the whole array member and nothing else; the callee writes p->v[1].x; the caller keeps its cell n",
        [=](Kit& k)
        {
            StructType* T {mk_t(k)};
            Function* put {make_put(k, T, false)};
            Function* f {k.fn("use", k.vd)};
            Var* t {k.scell(f, "t", T)};
            std::vector<const Stmt*> body;
            fill(k, t, body, "7");
            body.push_back(k.es(k.call(put, {k.addr(t)})));
            body.push_back(k.chk(k.eq_c(k.leaf(k.addr(t), 1), k.i32, "7")));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    lc("e1e2c2_member_owns_member_writes_cell_fail", "p->n written under owns(p->v): permission at the store",
       [=](Kit& k)
       {
           make_put(k, mk_t(k), true);
       });
    // int peek(struct T *p) requires(reads(*p)) ensures(reads(*p) && result == p->n) { [p->v[1].y = 0;] return p->n; }
    auto make_peek = [=](Kit& k, StructType* T, bool writes) -> Function*
    {
        Function* f {k.fn("peek", k.i32, {{"p", k.ptr(k.st(T))}})};
        k.req(f, k.b.atom(AtomKind::Reads, k.b.text(k.F(f, 0), "p"), 0, k.at()));
        k.ens(f, k.land(k.b.atom(AtomKind::Reads, k.b.text(k.F(f, 0), "p"), 0, k.at()),
                        k.eq(k.R(f), k.leaf(k.b.text(k.F(f, 0), "p"), 1))));
        std::vector<const Stmt*> body;
        const Stmt* w {nullptr};
        if (writes)
        {
            w = vwrite(k, k.pv(f, 0), "1", 1, "0");
            body.push_back(w);
        }
        body.push_back(k.ret(k.leaf(k.pv(f, 0), 1)));
        f->body = k.blk(body);
        k.done(f);
        if (writes)
        {
            k.fail("permission", w->loc);
        }
        return f;
    };
    lcv("e1e2c2_member_reads_call",
        "peek(&t) takes reads(*p): every leaf of the array member at a fraction; the caller still owns t and reads t.v[1].y afterwards",
        [=](Kit& k)
        {
            StructType* T {mk_t(k)};
            Function* peek {make_peek(k, T, false)};
            Function* f {k.fn("use", k.vd)};
            Var* t {k.scell(f, "t", T)};
            Var* r {k.local(f, "r", k.i32)};
            std::vector<const Stmt*> body;
            fill(k, t, body, "7");
            body.push_back(k.decl(r, k.call(peek, {k.addr(t)})));
            body.push_back(vwrite(k, k.addr(t), "1", 1, "9"));
            body.push_back(k.chk(k.land(k.eq_c(k.v(r), k.i32, "7"), k.eq_c(vread(k, k.addr(t), "1", 1), k.i32, "9"))));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    lc("e1e2c2_member_reads_write_fail", "p->v[1].y written under reads(*p): permission at the store",
       [=](Kit& k)
       {
           make_peek(k, mk_t(k), true);
       });
    // the zero fill of the member (C's `struct T t = {0}`): every leaf of both elements zero and set, then n written
    lcv("e1e2c2_member_zero_fill",
        "ArrayZero over the member v: zero_arr_<P> over the member's first slot; every leaf of both elements reads zero",
        [=](Kit& k)
        {
            StructType* T {mk_t(k)};
            Function* f {k.fn("f", k.vd)};
            Var* t {k.scell(f, "t", T)};
            std::vector<const Stmt*> body {k.decl(t, nullptr), k.b.array_zero(k.mem(k.addr(t), 0), k.at()),
                                           k.lstore(k.addr(t), 1, N(k, "7"))};
            body.push_back(k.chk(k.land(k.eq_c(vread(k, k.addr(t), "1", 1), k.i32, "0"),
                                        k.eq_c(vread(k, k.addr(t), "0", 0), k.i32, "0"))));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
    // ---- what check_program still refuses of such a structure (each lifted by a later slice or part) ----
    rc("e1e2c2_rej_member_by_value_parameter_beyond_the_limit", "array elements passed by value",
       [=](Kit& k)
       {
           // void f(struct W w) {}  with struct W { struct P v[4]; }: 8 leaves of array elements, one past the limit of 7 (M4.5f3)
           StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
           StructType* W {k.strukt("W", {{"v", k.prog.array_of(k.st(P), 4U)}})};
           Function* f {k.fn("f", k.vd, {{"w", k.st(W)}})};
           f->mark_address_taken(f->param_var(0));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2c2_rej_member_by_value_result_beyond_the_limit", "array elements passed by value",
       [=](Kit& k)
       {
           // struct W f(void);  (no body)
           StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
           StructType* W {k.strukt("W", {{"v", k.prog.array_of(k.st(P), 4U)}})};
           Function* f {k.fn("f", k.st(W), {})};
           k.trusted(f);
           k.done(f);
       });
    rc("e1e2c2_rej_member_element_with_an_array_member", "whose elements hold an array member",
       [=](Kit& k)
       {
           // struct U { int a[2][2]; }; struct T { struct U v[2]; };  (a depth-3 leaf path: Part E's slice 4; slice 3c-4 lifts the element with an array member of scalars)
           StructType* U {k.strukt("U", {{"a", k.prog.array_of(k.prog.array_of(k.i32, 2U), 2U)}})};
           k.strukt("T", {{"v", k.prog.array_of(k.st(U), 2U)}});
       });
    // int hm(struct T *p) requires(owns(*p) && p->v[1].x == 3) ensures(owns(*p) && r == p->v[1].x) { return p->v[1].x; }   (fill: t.v[1].x == 3)
    auto contract_member = [=](Kit& k, const char* want, bool own_p, SrcLoc* read_at = nullptr) -> const Expr*
    {
        StructType* T {mk_t(k)};
        Function* h {k.fn("hm", k.i32, {{"p", k.ptr(k.st(T))}, {"q", k.ptr(k.st(T))}})};
        auto own = [&](int i)
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.F(h, i), h->params[static_cast<size_t>(i)].name), 0, k.at());
        };
        const Expr* rd {vread(k, k.F(h, 0), "1", 0)};
        const Expr* pre {k.eq_c(rd, k.i32, want)};
        if (read_at != nullptr)
        {
            *read_at = rd->loc;
        }
        k.req(h, own_p ? k.land(own(0), pre) : k.land(own(1), pre));
        const Expr* post_rd {vread(k, k.F(h, 0), "1", 0)};
        k.ens(h, own_p ? k.land(own(0), k.eq(k.R(h), post_rd)) : k.land(own(1), k.eq(k.R(h), post_rd)));
        h->body = k.blk({k.ret(vread(k, k.pv(h, 0), "1", 0))});
        k.done(h);
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", T)};
        Var* u {k.scell(f, "u", T)};
        std::vector<const Stmt*> body;
        fill(k, t, body, "7");
        fill(k, u, body, "7");
        const Expr* call {k.call(h, {k.addr(t), k.addr(own_p ? t : u)})};
        body.push_back(k.es(call));
        f->body = k.blkc(body);
        k.done(f);
        return call;
    };
    lc("e1e2c2_member_contract_read",
       "hm(&t) requires(owns(*p) && p->v[1].x == 3) ensures(owns(*p) && r == p->v[1].x): the leaf of an element of the array member is read in both contracts, framed by the structure atom's array part",
       [=](Kit& k)
       {
           contract_member(k, "3", true);
       });
    lc("e1e2c2_member_contract_read_unframed_fail",
       "the contract reads p->v[1].x while owning only *q: nothing frames the read: the call's precondition does not hold (the read is the dummy value) and f__wf's framing row fails (permission, at the read)",
       [=](Kit& k)
       {
           SrcLoc read_at;
           const Expr* call {contract_member(k, "3", false, &read_at)};
           k.fail("precondition", call->loc);
           k.fail("permission", read_at);
       });
    // void keep(struct T *p) requires(owns(*p)) ensures(owns(*p) && p->v[1].x == old(p->v[1].x)) { [p->v[1].x = 5;] }
    auto keep_old = [=](Kit& k, bool writes)
    {
        StructType* T {mk_t(k)};
        Function* f {k.fn("keep", k.vd, {{"p", k.ptr(k.st(T))}})};
        auto own = [&]()
        {
            return k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "p"), 0, k.at());
        };
        k.req(f, own());
        k.ens(f, k.land(own(), k.eq(vread(k, k.F(f, 0), "1", 0), k.old(vread(k, k.F(f, 0), "1", 0)))));
        f->body = writes ? k.blk({vwrite(k, k.pv(f, 0), "1", 0, "5")}) : k.blk({});
        k.done(f);
        if (writes)
        {
            k.fail("postcondition", f->end_loc);
        }
    };
    lc("e1e2c2_member_contract_old", "ensures p->v[1].x == old(p->v[1].x) of a function that writes nothing: holds",
       [=](Kit& k)
       {
           keep_old(k, false);
       });
    lc("e1e2c2_member_contract_old_changed_fail",
       "the same function writing p->v[1].x = 5: the postcondition fails at the closing brace",
       [=](Kit& k)
       {
           keep_old(k, true);
       });
    // ---- slice 3: the value view (item 12): every leaf of every element is a leaf of the value, in the order element by element ----
    // `(struct T){t.v[0].x, t.v[0].y, t.v[1].x, t.v[1].y, t.n}` read from the object at `base`
    auto value_of = [=](Kit& k, StructType* T, const Expr* base)
    {
        return k.b.aggregate(k.st(T),
                             {vread(k, base, "0", 0), vread(k, base, "0", 1), vread(k, base, "1", 0),
                              vread(k, base, "1", 1), k.leaf(base, 1)},
                             k.at());
    };
    // struct T t, u; ...fill t...; u = t (a store of the value); check(u.v[1].x == <x> && u.v[0].y == 2 && u.n == 7); u.v[1].x = 9; check(t.v[1].x == 3 && u.v[1].x == 9);
    auto copy_locals = [=](Kit& k, const char* x)
    {
        StructType* T {mk_t(k)};
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", T)};
        Var* u {k.scell(f, "u", T)};
        std::vector<const Stmt*> body;
        fill(k, t, body, "7");
        body.push_back(k.decl(u, nullptr));
        body.push_back(k.store(k.addr(u), value_of(k, T, k.addr(t))));
        const Stmt* c {k.chk(
            k.land(k.eq_c(vread(k, k.addr(u), "1", 0), k.i32, x),
                   k.land(k.eq_c(vread(k, k.addr(u), "0", 1), k.i32, "2"), k.eq_c(k.leaf(k.addr(u), 1), k.i32, "7"))))};
        body.push_back(c);
        body.push_back(vwrite(k, k.addr(u), "1", 0, "9"));
        body.push_back(k.chk(
            k.land(k.eq_c(vread(k, k.addr(t), "1", 0), k.i32, "3"), k.eq_c(vread(k, k.addr(u), "1", 0), k.i32, "9"))));
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        if (std::string(x) != "3")
        {
            k.fail("check", c->loc);
        }
    };
    lcv("e1e2c2_member_copy_locals",
        "u = t between two locals whose member is an array of structures: every leaf of every element is read into a temporary and stored (the values arrive, the copy is independent)",
        [=](Kit& k)
        {
            copy_locals(k, "3");
        });
    lc("e1e2c2_member_copy_wrong_value_fail",
       "check(u.v[1].x == 4) after the copy of t.v[1].x == 3: the destination holds 3",
       [=](Kit& k)
       {
           copy_locals(k, "4");
       });
    // struct T id(struct T t) ensures(res.n == t.n) { return (struct T){t.v[0].x, ...}; }   struct T u = id((struct T){...from t...}); check(u.n == <n>); int z = u.v[1].y;
    auto id_main = [=](Kit& k, const char* n)
    {
        StructType* T {mk_t(k)};
        Function* id {k.fn("id", k.st(T), {{"t", k.st(T)}})};
        id->mark_address_taken(id->param_var(0));
        k.ens(id, k.eq(k.b.field(k.R(id), 1, k.at()), k.b.field(k.F(id, 0), 1, k.at())));
        id->body = k.blk({k.ret(value_of(k, T, k.addr(id->param_var(0))))});
        k.done(id);
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", T)};
        Var* u {k.scell(f, "u", T)};
        Var* z {k.local(f, "z", k.i32)};
        std::vector<const Stmt*> body;
        fill(k, t, body, "7");
        body.push_back(k.decl(u, nullptr));
        body.push_back(k.store(k.addr(u), k.call(id, {value_of(k, T, k.addr(t))})));
        const Stmt* c {k.chk(k.eq_c(k.leaf(k.addr(u), 1), k.i32, n))};
        body.push_back(c);
        body.push_back(k.decl(z, vread(k, k.addr(u), "1", 1)));
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        if (std::string(n) != "7")
        {
            k.fail("check", c->loc);
        }
    };
    lcv("e1e2c2_member_by_value_call",
        "id(t) passes a structure whose member is an array of structures by value and returns it: every leaf of every element is a Viper formal and a result, the entry copy writes each, the caller's u has every flag set and u.n == 7",
        [=](Kit& k)
        {
            id_main(k, "7");
        });
    lc("e1e2c2_member_by_value_wrong_fail", "check(u.n == 8) after id(t) with t.n == 7: the result's n is 7",
       [=](Kit& k)
       {
           id_main(k, "8");
       });
    // the Sequencer's pin (audit round 2, issue 8): t = (struct T){t.v[0].x++, 0, 0, 0, 0}; writes a leaf of an element of the member inside a value stored to t
    rc("e1e2c2_rej_seq_struct_store_writes_an_element_leaf", "unsequenced modification of",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           const Expr* inc {k.b.inc_dec_store_expr(vplace(k, k.addr(t), "0", 0), true, true, k.at())};
           const Expr* value {k.b.aggregate(k.st(T), {inc, N(k, "0"), N(k, "0"), N(k, "0"), N(k, "0")}, k.at())};
           f->body = k.blkc({k.decl(t, nullptr), k.store(k.addr(t), value)});
           k.done(f);
       });
    // a loop whose invariant reads a leaf of an element of the member: p->v[1].y of a pointer parameter holding owns(*p), then the same after `p` is reassigned
    // void g(struct T *p, struct T *q) requires(owns(*p)) ensures(owns(*p)) { [p = q;] int i = 0; while (i < 1) inv 0 <= i <= 1 && p->v[1].y >= INT_MIN { i++; } }
    auto inv_ptr = [=](Kit& k, bool reassign, SrcLoc* assign_at = nullptr) -> SrcLoc
    {
        StructType* T {mk_t(k)};
        Function* g {k.fn("g", k.vd, {{"p", k.ptr(k.st(T))}, {"q", k.ptr(k.st(T))}})};
        k.req(g, k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "p"), 0, k.at()));
        k.ens(g, k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "p"), 0, k.at()));
        Var* i {k.local(g, "i", k.i32)};
        const Var* pv {g->param_var(0)};
        const Expr* read {k.deref(k.b.member_addr(k.b.elem_addr(k.mem(k.pn(pv), 0), N(k, "1"), k.at()), 1, k.at()))};
        const Clause inv {k.cl(k.ge_c(read, k.i32, "-2147483648"))};
        const SrcLoc read_at {read->loc};
        std::vector<const Stmt*> body;
        if (reassign)
        {
            const Expr* q {k.pv(g, 1)};
            body.push_back(k.asg(pv, q));
            if (assign_at != nullptr)
            {
                *assign_at = q->loc;
            }
        }
        body.push_back(k.decl(i, N(k, "0")));
        body.push_back(k.whl(k.lt(k.v(i), N(k, "1")), {k.cl(k.range(k.v(i), k.i32, "0", "1")), inv},
                             {k.cl(k.sub(N(k, "1"), k.v(i)))}, k.blkc({k.incS(i)})));
        g->body = k.blkc(body);
        k.done(g);
        return read_at;
    };
    lc("e1e2c2_member_loop_invariant_read_framed",
       "an invariant reads p->v[1].y of a pointer parameter that holds owns(*p) and is not reassigned: framed by the structure place's array part",
       [=](Kit& k)
       {
           inv_ptr(k, false);
       });
    lc("e1e2c2_member_loop_invariant_read_reassigned_fail",
       "p = q before the loop: p = q uses a pointer nothing says is live (lifetime), and the invariant's read of p->v[1].y is framed by nothing the loop holds (permission)",
       [=](Kit& k)
       {
           SrcLoc asg;
           const SrcLoc at {inv_ptr(k, true, &asg)};
           // q has no atom: p = q is a use of a pointer nothing says is live (lifetime, at the assignment), and the invariant's read through it is framed by nothing the loop holds
           // (permission, at the entry: the first of the clause's two rows; the back edge is on the vacuous path after it)
           k.fail("lifetime", asg);
           k.fail_only("permission", at, 2U, 0U,
                       "this loop invariant reads `p->v[1].y`, which this loop does not hold");
       });
    // `&t.v[1]` as the front end folds it for a static structure: ElemAddr(ElemAddr(MemberAddr(&t, v), 0), 1): a pointer over the decayed base, flat; h(&t, <it>) is the alias error again
    lc("e1e2c2_member_folded_element_address_alias_fail",
       "h(&t, &(t.v[0] + 1)): the element address formed over the decayed member (ElemAddr of ElemAddr) is the same slot as t.v[1]: one permission error at the call, as for &t.v[1]",
       [=](Kit& k)
       {
           StructType* T {mk_t(k)};
           Function* h {make_h(k, T)};
           Function* f {k.fn("f", k.vd)};
           Var* t {k.scell(f, "t", T)};
           std::vector<const Stmt*> body;
           fill(k, t, body, "7");
           const Expr* first {k.b.elem_addr(k.mem(k.addr(t), 0), N(k, "0"), k.at())};
           const Expr* second {k.b.elem_addr(first, N(k, "1"), k.at())};
           const Expr* call {k.call(h, {k.addr(t), second})};
           body.push_back(k.es(call));
           f->body = k.blkc(body);
           k.done(f);
           k.fail_only("permission", call->loc, 36U, 19U);
       });
    // a loop over the elements: struct T t; ...fill...; int i = 0; while (i < 2) inv 0 <= i <= 2 { t.v[i].x = 5; i++; } int z = t.v[0].y; check(i == 2)
    lcv("e1e2c2_member_loop_fill",
        "a loop writing t.v[i].x: the member's leaves are array places of the loop, each leaf its own place; the elements stay readable after it",
        [=](Kit& k)
        {
            StructType* T {mk_t(k)};
            Function* f {k.fn("f", k.vd)};
            Var* t {k.scell(f, "t", T)};
            Var* i {k.local(f, "i", k.i32)};
            Var* z {k.local(f, "z", k.i32)};
            std::vector<const Stmt*> body;
            fill(k, t, body, "7");
            body.push_back(k.decl(i, N(k, "0")));
            const Expr* place {k.b.member_addr(k.b.elem_addr(k.mem(k.addr(t), 0), k.pn(i), k.at()), 0, k.at())};
            body.push_back(k.whl(k.lt(k.v(i), N(k, "2")), {k.cl(k.range(k.v(i), k.i32, "0", "2"))},
                                 {k.cl(k.sub(N(k, "2"), k.v(i)))}, k.blkc({k.store(place, N(k, "5")), k.incS(i)})));
            body.push_back(k.decl(z, vread(k, k.addr(t), "0", 1)));
            body.push_back(k.chk(k.eq_c(k.v(i), k.i32, "2")));
            tail(k, body);
            f->body = k.blkc(body);
            k.done(f);
        });
}

// Slice 2: a global and a `static` local array of structures, and a structure global whose member is an array of structures (record 0064 items 9 and 10), struct P { int x; int y; }.
void define_global_struct_array_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto mk_p = [](Kit& k)
    {
        return k.strukt("P", {{"x", k.i32}, {"y", k.i32}});
    };
    // `struct P <name>[n] = {{a, b}, ...}`: every listed element has one initial value per leaf (a leaf the braces leave out is zero), the rest zero (`unknown`: no value is known)
    auto gs = [=](Kit& k, const char* name, unsigned long long n,
                  const std::vector<std::pair<const char*, const char*>>& vals, bool unknown = false,
                  const Function* owner = nullptr) -> Global*
    {
        StructType* P {mk_p(k)};
        Global* g {k.prog.add_global(name, k.prog.array_of(k.st(P), n), k.at(), owner)};
        for (const auto& [x, y] : vals)
        {
            GlobalLeaf e;
            for (const char* v : {x, y})
            {
                GlobalLeaf l;
                l.kind = unknown ? GlobalInit::Unknown : GlobalInit::Value;
                l.value = v;
                l.init = k.num(k.i32, v);
                l.loc = k.at();
                e.leaves.push_back(l);
            }
            e.kind = unknown ? GlobalInit::Unknown : GlobalInit::Value;
            g->elems.push_back(e);
        }
        g->rest = unknown ? ArrayRest::Unknown : ArrayRest::Zero;
        return g;
    };
    auto gat = [](Kit& k, const Global* g, const Expr* i, int m)
    {
        return k.b.member_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), i, k.at()), m, k.at());
    };
    auto grd = [=](Kit& k, const Global* g, const char* i, int m)
    {
        return k.deref(gat(k, g, N(k, i), m));
    };
    auto gwr = [=](Kit& k, const Global* g, const char* i, int m, const char* v)
    {
        return k.store(gat(k, g, N(k, i), m), N(k, v));
    };
    auto gatom = [](Kit& k, AtomKind kind, const Global* g, int amount, bool implicit = false)
    {
        return k.b.global_atom(kind, k.b.global_addr(g, k.at()), amount, k.at(), implicit);
    };
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the global's rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto main_end = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
        body.push_back(k.ret(k.num(k.i32, "0")));
    };
    // struct P g[3] = {{1, 2}, {3}}; int main() { check(g[0].x == 1 && g[0].y == 2 && g[1].x == 3 && g[1].y == 0 && g[2].x == 0 && g[2].y == 0); g[1].x = 5; check(g[1].x == 5 && g[0].y == 2 && g[2].y == 0); return 0; }
    lcv("e1e2c2_garr_main_read_write",
        "main holds g[3] = {{1, 2}, {3}} from program_start: the listed elements' leaves, the zero rest per leaf, then a write and a read back",
        [=](Kit& k)
        {
            Global* g {gs(k, "g", 3U, {{"1", "2"}, {"3", "0"}})};
            Function* m {k.fn("main", k.i32)};
            auto eq = [&](const char* i, int mm, const char* v)
            {
                return k.eq_c(grd(k, g, i, mm), k.i32, v);
            };
            const Stmt* c1 {
                k.chk(k.land(k.land(k.land(eq("0", 0, "1"), eq("0", 1, "2")), k.land(eq("1", 0, "3"), eq("1", 1, "0"))),
                             k.land(eq("2", 0, "0"), eq("2", 1, "0"))))};
            const Stmt* w {gwr(k, g, "1", 0, "5")};
            const Stmt* c2 {k.chk(k.land(eq("1", 0, "5"), k.land(eq("0", 1, "2"), eq("2", 1, "0"))))};
            std::vector<const Stmt*> body {c1, w, c2};
            main_end(k, body);
            m->body = k.blk(body);
            k.done(m);
            k.prog.set_entry(m);
        });
    lc("e1e2c2_garr_zero_rest_wrong_fail",
       "the zero rest is exactly the elements past the list, per leaf: g[2].y == 1 fails",
       [=](Kit& k)
       {
           Global* g {gs(k, "g", 3U, {{"1", "2"}})};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq_c(grd(k, g, "2", 1), k.i32, "1"))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    lc("e1e2c2_garr_listed_leaf_wrong_fail",
       "the listed element's own leaf is exact: g[0].y == 3 where the list says 2 fails",
       [=](Kit& k)
       {
           Global* g {gs(k, "g", 3U, {{"1", "2"}})};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq_c(grd(k, g, "0", 1), k.i32, "3"))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    lc("e1e2c2_garr_unknown_wrong_fail",
       "a global array of structures whose values are not known: g[0].x == 0 does not follow",
       [=](Kit& k)
       {
           Global* g {gs(k, "g", 3U, {}, true)};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq_c(grd(k, g, "0", 0), k.i32, "0"))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
    lc("e1e2c2_garr_index_outside_fail", "g[3].x of struct P g[3]: the bound row (out-of-bounds) at the read",
       [=](Kit& k)
       {
           Global* g {gs(k, "g", 3U, {{"1", "2"}})};
           Function* m {k.fn("main", k.i32)};
           const Expr* r {grd(k, g, "3", 0)};
           const Stmt* c {k.chk(k.eq_c(r, k.i32, "0"))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("out-of-bounds", r->loc);
       });
    // void put(void) requires(owns(g)) ensures(owns(g)) { g[1].x = 9; check(g[1].x == 9); }   int main() { put(); int z = g[0].y; return 0; }
    auto put_main = [=](Kit& k, bool with_atom)
    {
        Global* g {gs(k, "g", 3U, {{"1", "2"}, {"3", "0"}})};
        Function* f {k.fn("put", k.vd)};
        if (with_atom)
        {
            k.req(f, gatom(k, AtomKind::Owns, g, 0));
            k.ens(f, gatom(k, AtomKind::Owns, g, 0));
        }
        const Stmt* w {gwr(k, g, "1", 0, "9")};
        f->body = k.blk({w, k.chk(k.eq_c(grd(k, g, "1", 0), k.i32, "9"))});
        k.done(f);
        if (!with_atom)
        {
            k.fail("permission", w->loc);
            return;
        }
        Function* m {k.fn("main", k.i32)};
        Var* z {k.local(m, "z", k.i32)};
        std::vector<const Stmt*> body {k.es(k.call(f, {})), k.decl(z, grd(k, g, "0", 1))};
        main_end(k, body);
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
    };
    lcv("e1e2c2_garr_owns_callee",
        "put() takes owns(g) of the global array of structures (every leaf of every element: the whole-array range atom, per leaf path): it writes g[1].x and reads it back; main holds g from program_start and reads g[0].y after the call",
        [=](Kit& k)
        {
            put_main(k, true);
        });
    lc("e1e2c2_garr_unowned_write_fail",
       "the same callee with no atom on g: the write g[1].x = 9 is a permission error at the store",
       [=](Kit& k)
       {
           put_main(k, false);
       });
    // void take(struct P *q) requires(owns(*q)) {}   int main() { take(&g[1]); int z = g[1].x; return 0; }: the callee keeps the element, so main lost it
    auto take_main = [=](Kit& k, bool read_after)
    {
        Global* g {gs(k, "g", 3U, {{"1", "2"}, {"3", "0"}})};
        StructType* P {const_cast<StructType*>(g->type.element->struct_type)};
        Function* take {k.fn("take", k.vd, {{"q", k.ptr(k.st(P))}})};
        k.req(take, k.b.atom(AtomKind::Owns, k.b.text(k.F(take, 0), "q"), 0, k.at()));
        take->body = k.blk({});
        k.done(take);
        Function* m {k.fn("main", k.i32)};
        Var* z {k.local(m, "z", k.i32)};
        const Expr* arg {k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, "1"), k.at())};
        const Expr* r {read_after ? grd(k, g, "1", 0) : grd(k, g, "2", 0)};
        std::vector<const Stmt*> body {k.es(k.call(take, {arg})), k.decl(z, r)};
        main_end(k, body);
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
        if (read_after)
        {
            k.fail("permission", r->loc);
        }
    };
    lcv("e1e2c2_garr_element_taken_other_kept",
        "take(&g[1]) gives the callee the element 1: main still holds element 2 and reads g[2].x",
        [=](Kit& k)
        {
            take_main(k, false);
        });
    lc("e1e2c2_garr_element_taken_read_fail",
       "take(&g[1]) keeps the element: main's read of g[1].x afterwards is a permission error",
       [=](Kit& k)
       {
           take_main(k, true);
       });
    // int main() { int i = 0, s = 0; while (i < 3) inv 0 <= i <= 3 && 0 <= s && s <= 3 * i { s += g[i].x; i++; } check(s <= 9); return 0; }   (g[3] = {{1, 2}, {3}})
    lcv("e1e2c2_garr_main_loop",
        "main's loop over the global array of structures: each leaf an array place over the global's bound local (the values kept), the sum bounded",
        [=](Kit& k)
        {
            Global* g {gs(k, "g", 3U, {{"1", "2"}, {"3", "0"}})};
            Function* m {k.fn("main", k.i32)};
            Var* i {k.local(m, "i", k.i32)};
            Var* sv {k.local(m, "s", k.i32)};
            const Stmt* step {
                k.asg(sv, k.add(k.v(sv), k.deref(k.b.member_addr(
                                             k.b.elem_addr(k.b.global_addr(g, k.at()), k.pn(i), k.at()), 0, k.at()))))};
            const Clause inv1 {
                k.cl(k.land(k.range(k.v(i), k.i32, "0", "3"),
                            k.land(k.ge_c(k.v(sv), k.i32, "0"), k.le(k.v(sv), k.mul(N(k, "3"), k.v(i))))))};
            std::vector<const Stmt*> body {
                k.decl(i, N(k, "0")), k.decl(sv, N(k, "0")),
                k.whl(k.lt(k.v(i), N(k, "3")), {inv1}, {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({step, k.incS(i)})),
                k.chk(k.le(k.v(sv), N(k, "9")))};
            main_end(k, body);
            m->body = k.blkc(body);
            k.done(m);
            k.prog.set_entry(m);
        });
    // int counter(void) { static struct P a[2] = {{0, 0}, {0, 0}}; a[0].x = 5; return a[0].x; }   int main() { check(counter() == 5); return 0; }
    lc("e1e2c2_garr_static_local",
       "a static local array of structures: counter() takes and returns its implicit owns atom; main holds it from program_start and calls it",
       [=](Kit& k)
       {
           Function* f {k.fn("counter", k.i32)};
           Global* g {gs(k, "a", 2U, {{"0", "0"}, {"0", "0"}}, false, f)};
           f->requires_.push_back(k.cl(gatom(k, AtomKind::Owns, g, 0, true)));
           f->ensures_.push_back(k.cl(gatom(k, AtomKind::Owns, g, 0, true)));
           k.ens(f, k.eq_c(k.R(f), k.i32, "5"));
           f->body = k.blk({gwr(k, g, "0", 0, "5"), k.ret(grd(k, g, "0", 0))});
           k.done(f);
           Function* m {k.fn("main", k.i32)};
           Var* r {k.local(m, "r", k.i32)};
           m->body = k.blk({k.decl(r, k.call(f, {})), k.chk(k.eq_c(k.v(r), k.i32, "5")), k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
       });
    // struct Q { int *p; int v; } h[2] = {{NULL, 5}}: a pointer leaf (known null) beside an integer leaf; the rest zero (null, 0); `unknown`: every leaf only in its type's range (WF for the pointer)
    auto gq = [=](Kit& k, bool unknown) -> Global*
    {
        StructType* Q {k.strukt("Q", {{"p", k.ptr(k.i32)}, {"v", k.i32}})};
        Global* g {k.prog.add_global("h", k.prog.array_of(k.st(Q), 2U), k.at(), nullptr)};
        GlobalLeaf e;
        GlobalLeaf lp;
        lp.kind = unknown ? GlobalInit::Unknown : GlobalInit::Null;
        lp.loc = k.at();
        GlobalLeaf lv;
        lv.kind = unknown ? GlobalInit::Unknown : GlobalInit::Value;
        lv.value = "5";
        lv.init = k.num(k.i32, "5");
        lv.loc = k.at();
        e.leaves = {lp, lv};
        e.kind = unknown ? GlobalInit::Unknown : GlobalInit::Value;
        g->elems.push_back(e);
        g->rest = unknown ? ArrayRest::Unknown : ArrayRest::Zero;
        return g;
    };
    auto hread = [=](Kit& k, const Global* g, const char* i, int m)
    {
        return k.deref(k.b.member_addr(k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, i), k.at()), m, k.at()));
    };
    lcv("e1e2c2_garr_pointer_leaf",
        "h[2] = {{NULL, 5}}: the listed element's pointer leaf is null, its integer leaf 5; the rest of the array is null and 0, per leaf",
        [=](Kit& k)
        {
            Global* g {gq(k, false)};
            Function* m {k.fn("main", k.i32)};
            std::vector<const Stmt*> body {k.chk(k.land(
                k.land(k.eq(hread(k, g, "0", 0), k.nul(k.ptr(k.i32))), k.eq_c(hread(k, g, "0", 1), k.i32, "5")),
                k.land(k.eq(hread(k, g, "1", 0), k.nul(k.ptr(k.i32))), k.eq_c(hread(k, g, "1", 1), k.i32, "0"))))};
            main_end(k, body);
            m->body = k.blk(body);
            k.done(m);
            k.prog.set_entry(m);
        });
    lc("e1e2c2_garr_pointer_leaf_unknown_fail",
       "the same global with no value known: reading h[0].p is a use of a pointer nothing says is live (lifetime), and h[0].p == NULL does not follow (check)",
       [=](Kit& k)
       {
           Global* g {gq(k, true)};
           Function* m {k.fn("main", k.i32)};
           const Expr* r {hread(k, g, "0", 0)};
           const Stmt* c {k.chk(k.eq(r, k.nul(k.ptr(k.i32))))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           // the pointer read from memory is a use of it: nothing says its object is live (the liveness row), and nothing says it is null (the check)
           k.fail("lifetime", r->loc);
           k.fail("check", c->loc);
       });
    // struct P g[3] = {{1, 2}, {3}}; struct P *pg = g; int main() { check(pg == &g[0]); check(g[1].x == 3); return 0; }   (the decay as an initial value is the address of element 0: loc(GB_g(), 0))
    lc("e1e2c2_garr_addr_target",
       "a pointer-to-structure global initialized with the decay of an array global of structures holds loc(GB_g(), 0)",
       [=](Kit& k)
       {
           Global* g {gs(k, "g", 3U, {{"1", "2"}, {"3", "0"}})};
           StructType* P {const_cast<StructType*>(g->type.element->struct_type)};
           Global* pg {k.prog.add_global("pg", k.ptr(k.st(P)), k.at(), nullptr)};
           pg->init_kind = GlobalInit::Addr;
           pg->target = g;
           Function* m {k.fn("main", k.i32)};
           const Stmt* c1 {k.chk(k.eq(k.deref(k.b.global_addr(pg, k.at())),
                                      k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, "0"), k.at())))};
           const Stmt* c2 {k.chk(k.eq_c(grd(k, g, "1", 0), k.i32, "3"))};
           m->body = k.blk({c1, c2, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
       });
    // int f(void) [requires(owns(g)) ensures(owns(g))] { int i = 0; while (i < 1) inv 0 <= i <= 1 && g[0].y >= 0 - 1 + 1 ... { i++; } return 0; }   int main() { f(); return 0; }
    // the invariant reads g[0].y: framed by the function's own holding of g, or not at all
    auto inv_read = [=](Kit& k, bool with_atom)
    {
        Global* g {gs(k, "g", 3U, {{"1", "2"}, {"3", "0"}})};
        Function* f {k.fn("f", k.i32)};
        if (with_atom)
        {
            k.req(f, gatom(k, AtomKind::Owns, g, 0));
            k.ens(f, gatom(k, AtomKind::Owns, g, 0));
        }
        Var* i {k.local(f, "i", k.i32)};
        const Expr* rexpr {grd(k, g, "0", 1)};
        const Clause read {k.cl(k.ge_c(rexpr, k.i32, "-2147483648"))};
        const Stmt* loop {k.whl(k.lt(k.v(i), N(k, "1")), {k.cl(k.range(k.v(i), k.i32, "0", "1")), read},
                                {k.cl(k.sub(N(k, "1"), k.v(i)))}, k.blkc({k.incS(i)}))};
        f->body = k.blkc({k.decl(i, N(k, "0")), loop, k.ret(N(k, "0"))});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        m->body = k.blk({k.es(k.call(f, {})), k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return rexpr->loc;
    };
    lc("e1e2c2_garr_loop_read_framed",
       "f owns g and a loop invariant reads g[0].y: the read is framed by the array place of the leaf",
       [=](Kit& k)
       {
           inv_read(k, true);
       });
    lc("e1e2c2_garr_loop_read_unframed_fail",
       "f holds nothing of g (it is not the entry function): the invariant's read of g[0].y is framed by nothing: a permission error, never framed by main's holding",
       [=](Kit& k)
       {
           const SrcLoc at {inv_read(k, false)};
           k.fail_only("permission", at, 2U, 0U, "this loop invariant reads `g[0].y`, which this loop does not hold");
       });
    // int rd(void) requires(owns(g) && g[1].x == 3) ensures(owns(g) && r == g[1].x) { return g[1].x; }   int main() { return rd(); }   [unframed: no atom on g]
    auto crd = [=](Kit& k, bool with_atom, const char* want, SrcLoc* read_at = nullptr) -> const Expr*
    {
        Global* g {gs(k, "g", 3U, {{"1", "2"}, {"3", "0"}})};
        Function* f {k.fn("rd", k.i32)};
        const Expr* r1 {grd(k, g, "1", 0)};
        const Expr* pre {k.eq_c(r1, k.i32, want)};
        if (read_at != nullptr)
        {
            *read_at = r1->loc;
        }
        k.req(f, with_atom ? k.land(gatom(k, AtomKind::Owns, g, 0), pre) : pre);
        k.ens(f, with_atom ? k.land(gatom(k, AtomKind::Owns, g, 0), k.eq(k.R(f), grd(k, g, "1", 0)))
                           : k.eq(k.R(f), grd(k, g, "1", 0)));
        f->body = k.blk({k.ret(grd(k, g, "1", 0))});
        k.done(f);
        Function* m {k.fn("main", k.i32)};
        const Expr* call {k.call(f, {})};
        m->body = k.blk({k.ret(call)});
        k.done(m);
        k.prog.set_entry(m);
        return call;
    };
    lc("e1e2c2_garr_contract_read",
       "rd() requires(owns(g) && g[1].x == 3) ensures(owns(g) && r == g[1].x): the element's leaf is read in both contracts, framed by the whole-array atom; main holds g and knows g[1].x == 3",
       [=](Kit& k)
       {
           crd(k, true, "3");
       });
    lc("e1e2c2_garr_contract_read_wrong_fail",
       "the same requiring g[1].x == 4: main's call fails its precondition (g[1].x is 3)",
       [=](Kit& k)
       {
           k.fail("precondition", crd(k, true, "4")->loc);
       });
    lc("e1e2c2_garr_contract_read_unframed_fail",
       "rd() with no atom on g: the contract's read of g[1].x is framed by nothing: the call's precondition does not hold (the read is the dummy value) and f__wf's framing row fails (permission, at the read)",
       [=](Kit& k)
       {
           SrcLoc read_at;
           const Expr* call {crd(k, false, "3", &read_at)};
           // nothing frames the read: it is the dummy 0 at the call (the precondition fails) and f__wf's framing row fails (permission)
           k.fail("precondition", call->loc);
           k.fail("permission", read_at);
       });
    // struct T { struct P v[2]; int n; } gt = {{{1, 2}, {3, 4}}, 7}: a structure global whose member is an array of structures
    auto gt_global = [=](Kit& k) -> Global*
    {
        StructType* P {mk_p(k)};
        StructType* T {k.strukt("T", {{"v", k.prog.array_of(k.st(P), 2U)}, {"n", k.i32}})};
        Global* g {k.prog.add_global("gt", k.st(T), k.at(), nullptr)};
        GlobalLeaf la;
        la.kind = GlobalInit::Value;
        la.rest = ArrayRest::Zero;
        la.loc = k.at();
        for (const auto& [x, y] : std::vector<std::pair<const char*, const char*>> {{"1", "2"}, {"3", "4"}})
        {
            GlobalLeaf e;
            e.kind = GlobalInit::Value;
            for (const char* v : {x, y})
            {
                GlobalLeaf l;
                l.kind = GlobalInit::Value;
                l.value = v;
                l.init = k.num(k.i32, v);
                l.loc = k.at();
                e.leaves.push_back(l);
            }
            la.elems.push_back(e);
        }
        g->leaves.push_back(la);
        GlobalLeaf ln;
        ln.kind = GlobalInit::Value;
        ln.init = k.num(k.i32, "7");
        ln.value = "7";
        ln.loc = k.at();
        g->leaves.push_back(ln);
        return g;
    };
    auto gtv = [=](Kit& k, const Global* g, const char* i, int m)
    {
        return k.b.member_addr(k.b.elem_addr(k.mem(k.b.global_addr(g, k.at()), 0), N(k, i), k.at()), m, k.at());
    };
    // int main() { check(gt.v[0].y == 2 && gt.v[1].x == 3 && gt.n == 7); gt.v[1].y = 9; check(gt.v[1].y == 9 && gt.v[0].x == 1); return 0; }
    lcv("e1e2c2_gstruct_member_main",
        "main holds the structure global gt whose member v is an array of structures from program_start: the member's leaves per element, then a write and a read back",
        [=](Kit& k)
        {
            Global* g {gt_global(k)};
            Function* m {k.fn("main", k.i32)};
            auto eq = [&](const char* i, int mm, const char* v)
            {
                return k.eq_c(k.deref(gtv(k, g, i, mm)), k.i32, v);
            };
            const Stmt* c1 {k.chk(k.land(k.land(eq("0", 1, "2"), eq("1", 0, "3")),
                                         k.eq_c(k.leaf(k.b.global_addr(g, k.at()), 1), k.i32, "7")))};
            const Stmt* w {k.store(gtv(k, g, "1", 1), N(k, "9"))};
            const Stmt* c2 {k.chk(k.land(eq("1", 1, "9"), eq("0", 0, "1")))};
            std::vector<const Stmt*> body {c1, w, c2};
            main_end(k, body);
            m->body = k.blk(body);
            k.done(m);
            k.prog.set_entry(m);
        });
    lc("e1e2c2_gstruct_member_wrong_fail", "gt.v[1].x == 4 where the list says 3 fails",
       [=](Kit& k)
       {
           Global* g {gt_global(k)};
           Function* m {k.fn("main", k.i32)};
           const Stmt* c {k.chk(k.eq_c(k.deref(gtv(k, g, "1", 0)), k.i32, "4"))};
           m->body = k.blk({c, k.ret(N(k, "0"))});
           k.done(m);
           k.prog.set_entry(m);
           k.fail("check", c->loc);
       });
}

// ---- the audit round of Part C1 (record 0064 items 7 and 10): the sequencing rows of a call beside a member array or a global array of structures (finding 1), and a range over a structure
// with a pointer-to-structure member and a scalar range over `struct P **` (finding 4) ----
void define_audit_c1_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto RO = [=](Kit& k, Function* f)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at());
    };
    auto lcv = [](const char* name, const char* doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(std::string(name) + "_vacuity",
           "the same with check(false) after everything: it fails (the rows are consistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto tail = [](Kit& k, std::vector<const Stmt*>& body)
    {
        if (k.vac_tail)
        {
            const Stmt* v {k.chk(k.no())};
            body.push_back(v);
            k.vac_loc = v->loc;
        }
    };
    // int h(struct P *p, int n) requires(owns_range(p, n) && n > 0) ensures(owns_range(p, n) && r == 1) { p[0].x = 1; return 1; }
    auto mk_h = [=](Kit& k, StructType* P) -> Function*
    {
        Function* h {k.fn("h", k.i32, {{"p", k.ptr(k.st(P))}, {"n", k.i32}})};
        k.req(h, k.land(RO(k, h), k.gt(fx(k, h, 1), N(k, "0"))));
        k.ens(h, k.land(RO(k, h), k.eq(k.R(h), N(k, "1"))));
        auto x0 = [&]()
        {
            return k.b.member_addr(k.b.elem_addr(k.pv(h, 0), N(k, "0"), k.at()), 0, k.at());
        };
        h->body = k.blkc({k.store(x0(), N(k, "1")), k.ret(N(k, "1"))});
        k.done(h);
        return h;
    };
    // ---- finding 1 over a member array: struct T { struct P v[2]; int n; } t (every leaf written);  h(&t.v[0], <n>) == t.v[1].x  and  h(&t.v[0], <n>) == h(&t.v[<m>], 1) ----
    const auto seq_member = [=](Kit& k, bool two_calls, const char* n, const char* at2) -> SrcLoc
    {
        StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
        StructType* T {k.strukt("T", {{"v", k.prog.array_of(k.st(P), 2U)}, {"n", k.i32}})};
        Function* h {mk_h(k, P)};
        Function* f {k.fn("f", k.vd)};
        Var* t {k.scell(f, "t", T)};
        auto elem = [&](const char* i)
        {
            return k.b.elem_addr(k.mem(k.addr(t), 0), N(k, i), k.at());
        };
        std::vector<const Stmt*> body {k.decl(t, nullptr)};
        for (const char* i : {"0", "1"})
        {
            body.push_back(k.store(k.b.member_addr(elem(i), 0, k.at()), N(k, "1")));
            body.push_back(k.store(k.b.member_addr(elem(i), 1, k.at()), N(k, "2")));
        }
        body.push_back(k.lstore(k.addr(t), 1, N(k, "3")));
        const Expr* c1 {k.call(h, {elem("0"), N(k, n)})};
        const Expr* other {two_calls ? k.call(h, {elem(at2), N(k, "1")})
                                     : k.deref(k.b.member_addr(elem("1"), 0, k.at()))};
        body.push_back(k.es(k.eq(c1, other)));
        tail(k, body);
        f->body = k.blkc(body);
        k.done(f);
        return other->loc;
    };
    lcv("e1e2c2_seq_member_call_and_read_disjoint",
        "h(&t.v[0], 1) == t.v[1].x over a member array of structures: the call's range is [0, 1) of the member, the read is element 1: the leaf's row passes",
        [=](Kit& k)
        {
            seq_member(k, false, "1", "");
        });
    lc("e1e2c2_seq_member_call_and_read_overlap_fail",
       "h(&t.v[0], 2) == t.v[1].x: the read is the leaf x of an element the call's range holds, in an order C leaves unspecified (unsequenced at the read)",
       [=](Kit& k)
       {
           k.fail_only("unsequenced", seq_member(k, false, "2", ""), 2U, 0U);
       });
    lcv("e1e2c2_seq_member_two_calls_disjoint",
        "h(&t.v[0], 1) == h(&t.v[1], 1) over a member array: two ranges of one element type that share no slot: the slot row passes",
        [=](Kit& k)
        {
            seq_member(k, true, "1", "1");
        });
    lc("e1e2c2_seq_member_two_calls_overlap_fail",
       "h(&t.v[0], 2) == h(&t.v[1], 1) over a member array: the ranges share element 1: unsequenced at the later call",
       [=](Kit& k)
       {
           k.fail_only("unsequenced", seq_member(k, true, "2", "1"), 4U, 0U);
       });
    // ---- finding 1 over a global array of structures: struct P g[3] (zero); main() { h(&g[0], <n>) == g[1].x; }  and  h(&g[0], <n>) == h(&g[<m>], 1) ----
    const auto seq_global = [=](Kit& k, bool two_calls, const char* n, const char* at2) -> SrcLoc
    {
        StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
        Global* g {k.prog.add_global("g", k.prog.array_of(k.st(P), 3U), k.at(), nullptr)};
        g->rest = ArrayRest::Zero;
        Function* h {mk_h(k, P)};
        Function* m {k.fn("main", k.i32)};
        auto elem = [&](const char* i)
        {
            return k.b.elem_addr(k.b.global_addr(g, k.at()), N(k, i), k.at());
        };
        const Expr* c1 {k.call(h, {elem("0"), N(k, n)})};
        const Expr* other {two_calls ? k.call(h, {elem(at2), N(k, "1")})
                                     : k.deref(k.b.member_addr(elem("1"), 0, k.at()))};
        std::vector<const Stmt*> body {k.es(k.eq(c1, other))};
        tail(k, body);
        body.push_back(k.ret(N(k, "0")));
        m->body = k.blkc(body);
        k.done(m);
        k.prog.set_entry(m);
        return other->loc;
    };
    lcv("e1e2c2_seq_global_call_and_read_disjoint",
        "h(&g[0], 1) == g[1].x over a global array of structures held by main: the call's range is [0, 1) of the global, the read is element 1: the leaf's row passes",
        [=](Kit& k)
        {
            seq_global(k, false, "1", "");
        });
    lc("e1e2c2_seq_global_call_and_read_overlap_fail",
       "h(&g[0], 2) == g[1].x: the read is the leaf x of an element the call's range holds (unsequenced at the read)",
       [=](Kit& k)
       {
           k.fail_only("unsequenced", seq_global(k, false, "2", ""), 2U, 0U);
       });
    lcv("e1e2c2_seq_global_two_calls_disjoint",
        "h(&g[0], 1) == h(&g[2], 1) over a global array: two ranges that share no slot: the slot row passes",
        [=](Kit& k)
        {
            seq_global(k, true, "1", "2");
        });
    lc("e1e2c2_seq_global_two_calls_overlap_fail",
       "h(&g[0], 2) == h(&g[1], 1) over a global array: the ranges share element 1: unsequenced at the later call",
       [=](Kit& k)
       {
           k.fail_only("unsequenced", seq_global(k, true, "2", "1"), 4U, 0U);
       });
    // ---- finding 4: struct N { struct N *next; int v; } ----
    // int f(struct N *p, int n) requires(owns_range(p, n) && n > 1) ensures(owns_range(p, n) && r == <post>)
    //   { p[0].next = &p[1]; p[1].v = 5; struct N *q = p[0].next; p[1].v = 6; return q->v; }
    auto link_case = [=](Kit& k, const char* post) -> SrcLoc
    {
        StructType* Nn {k.prog.add_struct("N", "struct N", k.at())};
        k.prog.add_member(Nn, "next", k.ptr(k.st(Nn)), k.at());
        k.prog.add_member(Nn, "v", k.i32, k.at());
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(Nn))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f), k.gt(fx(k, f, 1), N(k, "1"))));
        k.ens(f, k.land(RO(k, f), k.eq(k.R(f), N(k, post))));
        auto elem = [&](const char* i)
        {
            return k.b.elem_addr(k.pv(f, 0), N(k, i), k.at());
        };
        Var* q {k.local(f, "q", k.ptr(k.st(Nn)))};
        const Stmt* link {k.store(k.b.member_addr(elem("0"), 0, k.at()), elem("1"))};
        const Stmt* ld {k.decl(q, k.deref(k.b.member_addr(elem("0"), 0, k.at())))};
        const Stmt* ret {k.ret(k.leaf(k.pn(q), 1))};
        f->body = k.blkc({link, k.store(k.b.member_addr(elem("1"), 1, k.at()), N(k, "5")), ld,
                          k.store(k.b.member_addr(elem("1"), 1, k.at()), N(k, "6")), ret});
        k.done(f);
        return ret->loc;
    };
    lc("e1e2c2_range_pointer_to_struct_member",
       "owns_range(p, n) over struct N { struct N *next; int v; }: p[0].next = &p[1], read back through the loaded pointer: q->v is the 6 stored last (the pointer leaf's well-formedness row is a conjunct of the atom)",
       [=](Kit& k)
       {
           link_case(k, "6");
       });
    lc("e1e2c2_range_pointer_to_struct_member_wrong_fail",
       "the same with ensures r == 5: the value read through the loaded pointer is 6, not the 5 stored before (postcondition at the return)",
       [=](Kit& k)
       {
           k.fail("postcondition", link_case(k, "5"));
       });
    // void g(struct N *q) requires(owns(*q)) ensures(owns(*q) && q->v == 9) { q->v = 9; }
    // int f(struct N *p, int n) requires(owns_range(p, n) && n > 1 && p[1].v == 5) ensures(owns_range(p, n) && r == <post>) { p[0].next = &p[1]; g(p[0].next); return p[1].v; }
    auto write_through = [=](Kit& k, const char* post) -> SrcLoc
    {
        StructType* Nn {k.prog.add_struct("N", "struct N", k.at())};
        k.prog.add_member(Nn, "next", k.ptr(k.st(Nn)), k.at());
        k.prog.add_member(Nn, "v", k.i32, k.at());
        Function* g {k.fn("g", k.vd, {{"q", k.ptr(k.st(Nn))}})};
        k.req(g, k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "q"), 0, k.at()));
        k.ens(g, k.land(k.b.atom(AtomKind::Owns, k.b.text(k.F(g, 0), "q"), 0, k.at()),
                        k.eq_c(k.leaf(k.b.text(k.F(g, 0), "q"), 1), k.i32, "9")));
        g->body = k.blkc({k.lstore(k.pv(g, 0), 1, N(k, "9"))});
        k.done(g);
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(Nn))}, {"n", k.i32}})};
        auto elem = [&](const char* i)
        {
            return k.b.elem_addr(fx(k, f, 0), N(k, i), k.at());
        };
        const Expr* v1 {k.deref(k.b.member_addr(elem("1"), 1, k.at()))};
        k.req(f, k.land(k.land(RO(k, f), k.gt(fx(k, f, 1), N(k, "1"))), k.eq(v1, N(k, "5"))));
        k.ens(f, k.land(RO(k, f), k.eq(k.R(f), N(k, post))));
        auto pelem = [&](const char* i)
        {
            return k.b.elem_addr(k.pv(f, 0), N(k, i), k.at());
        };
        const Stmt* link {k.store(k.b.member_addr(pelem("0"), 0, k.at()), pelem("1"))};
        const Stmt* callg {k.es(k.call(g, {k.deref(k.b.member_addr(pelem("0"), 0, k.at()))}))};
        const Stmt* ret {k.ret(k.deref(k.b.member_addr(pelem("1"), 1, k.at())))};
        f->body = k.blkc({link, callg, ret});
        k.done(f);
        return ret->loc;
    };
    lc("e1e2c2_range_pointer_to_struct_write_through",
       "g(p[0].next) with p[0].next == &p[1]: the callee takes element 1 from the caller's range through the loaded pointer and writes 9, which p[1].v reads afterwards",
       [=](Kit& k)
       {
           write_through(k, "9");
       });
    lc("e1e2c2_range_pointer_to_struct_write_through_wrong_fail",
       "the same with ensures r == 5: the callee's write is seen (postcondition at the return)",
       [=](Kit& k)
       {
           k.fail("postcondition", write_through(k, "5"));
       });
    // ---- a scalar range over `struct P **`: int f(struct P **pp, int n) requires(owns_range(pp, n) && n > 0) ensures(owns_range(pp, n) && r == <post>) { pp[0] = NULL; return 0; } ----
    auto ptr_range = [=](Kit& k, const char* post) -> SrcLoc
    {
        StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
        Function* f {k.fn("f", k.i32, {{"pp", k.ptr(k.ptr(k.st(P)))}, {"n", k.i32}})};
        k.req(f, k.land(RO(k, f), k.gt(fx(k, f, 1), N(k, "0"))));
        k.ens(f, k.land(RO(k, f), k.eq(k.R(f), N(k, post))));
        const Stmt* ret {k.ret(N(k, "0"))};
        f->body = k.blkc({k.store(k.b.elem_addr(k.pv(f, 0), N(k, "0"), k.at()), k.nul(k.ptr(k.st(P)))), ret});
        k.done(f);
        return ret->loc;
    };
    lc("e1e2c2_range_of_struct_pointers",
       "owns_range(pp, n) over struct P **: pp[0] = NULL, the scalar range of pointers to structures (the well-formedness row of a pointer-to-structure cell)",
       [=](Kit& k)
       {
           ptr_range(k, "0");
       });
    lc("e1e2c2_range_of_struct_pointers_wrong_fail", "the same with ensures r == 1: the postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", ptr_range(k, "1"));
       });
}

// ---- the audit round of Part C1, findings 2 and 3 (record 0064 item 7, record 0061 item 11): a `pure` application and a `cerastes_forall` in a contract beside a range over structures (g02, z08), a
// structure atom over a member array of structures (Part C2) and a global array of structures held by `owns(g)`.  Such a site frames no scalar application and no read under a binder: where it
// is the only one, the application and the read are a `permission` error (never an internal error, never a quantifier body with a computed `loc` term); a scalar range on another parameter
// still frames its own ----
void define_audit_c1_site_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto PI = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    // `amount`: the atom's number among the permission atoms of its clause list (decision 0037 item 6)
    auto RR = [=](Kit& k, Function* f, int p, int n, int amount = 0)
    {
        return k.b.range_atom(AtomKind::ReadsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    auto RO = [=](Kit& k, Function* f, int p, int n, int amount = 0)
    {
        return k.b.range_atom(AtomKind::OwnsRange, fx(k, f, p), fx(k, f, n), amount, k.at());
    };
    auto lcv = [](const std::string& name, const std::string& doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(name + "_vacuity",
           "the same with a function that ends in check(false): it fails (what the contract assumes is not inconsistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto vac_fn = [=](Kit& k)
    {
        if (k.vac_tail)
        {
            Function* g {k.fn("vac", k.vd)};
            const Stmt* v {k.chk(k.no())};
            g->body = k.blk({v});
            k.done(g);
            k.vac_loc = v->loc;
        }
    };
    // decision 0061 item 11: no quantifier body holds a computed `loc` term over the binder; the slot of a cell of a range or array of structures is recovered by `parent_of(block_of(loc(block_of(a), q)))`,
    // which only the framing of a read under a binder could write
    const auto no_computed_slot = [](Kit& k)
    {
        k.text_checks.push_back(
            [](const std::string& vpr) -> std::string
            {
                return vpr.find("parent_of(block_of(loc(block_of(") == std::string::npos
                           ? ""
                           : "a membership test of a structure site over a quantifier's binder (a computed `loc` term, decision 0061 item 11) was written";
            });
    };
    // the three sites that hold int cells in a member block: a range over struct P, the member array of a structure under a structure atom, a global array held by owns(g)
    enum class Site
    {
        Range,
        Member,
        Global
    };
    struct Ctx
    {
        Function* f;
        std::function<const Expr*()>
            atom; // the structure site's atom, built afresh for each clause (an atom is one node)
        int next; // the index of the first free parameter
    };
    // `f(<site params>, int *a, int m)`: the site's atom is returned in `atom` (null for no site), the parameters `a` and `m` are the last two
    auto make_f = [=](Kit& k, Site site, const char* name) -> Ctx
    {
        StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
        Ctx c {nullptr, {}, 0};
        if (site == Site::Range)
        {
            c.f = k.fn(name, k.vd, {{"p", k.ptr(k.st(P))}, {"n", k.i32}, {"a", PI(k)}, {"m", k.i32}});
            Function* fp {c.f};
            c.atom = [=, &k]()
            {
                return RR(k, fp, 0, 1);
            };
            c.next = 2;
        }
        else if (site == Site::Member)
        {
            StructType* T {k.strukt("T", {{"v", k.prog.array_of(k.st(P), 2U)}, {"n", k.i32}})};
            c.f = k.fn(name, k.vd, {{"b", k.ptr(k.st(T))}, {"a", PI(k)}, {"m", k.i32}});
            Function* fp {c.f};
            c.atom = [=, &k]()
            {
                return k.b.atom(AtomKind::Owns, fx(k, fp, 0), 0, k.at());
            };
            c.next = 1;
        }
        else
        {
            Global* g {k.prog.add_global("g", k.prog.array_of(k.st(P), 3U), k.at(), nullptr)};
            g->rest = ArrayRest::Zero;
            c.f = k.fn(name, k.vd, {{"a", PI(k)}, {"m", k.i32}});
            c.atom = [=, &k]()
            {
                return k.b.global_atom(AtomKind::Owns, k.b.global_addr(g, k.at()), 0, k.at(), false);
            };
            c.next = 0;
        }
        return c;
    };
    // ---- finding 2: int first(const int *a, int m) requires(reads_range(a, m) && m > 0) { return a[0]; }, applied in the contract ----
    auto mk_first = [=](Kit& k)
    {
        Function* g {k.fn("first", k.i32, {{"a", PI(k)}, {"m", k.i32}})};
        g->pure = true;
        k.req(g, k.land(RR(k, g, 0, 1), k.gt(fx(k, g, 1), N(k, "0"))));
        g->pure_body = k.deref(k.b.elem_addr(k.F(g, 0), N(k, "0"), k.at()));
        k.done(g);
        return g;
    };
    // void f(<site>, int *a, int m) requires(<site atom> [&& reads_range(a, m)] && m > 0 && first(a, m) == 7) ensures(<site atom> [&& reads_range(a, m)]) { }
    auto apply_case = [=](Kit& k, Site site, bool scalar_range, const char* name) -> const Expr*
    {
        Function* first {mk_first(k)};
        Ctx c {make_f(k, site, name)};
        const int ai {c.next};
        const Expr* app {k.b.apply(first, {k.F(c.f, ai), k.F(c.f, ai + 1)}, "first(a, m)", k.at())};
        // the atoms are numbered in the order they are built: the structure site, then the scalar range, and the application stands after both
        const Expr* site_atom {c.atom()};
        const Expr* sites_req {scalar_range ? k.land(site_atom, RR(k, c.f, ai, ai + 1, 1)) : site_atom};
        k.req(c.f, k.land(k.land(sites_req, k.gt(fx(k, c.f, ai + 1), N(k, "0"))), k.eq(app, N(k, "7"))));
        const Expr* site_ens {c.atom()};
        k.ens(c.f, scalar_range ? k.land(site_ens, RR(k, c.f, ai, ai + 1, 1)) : site_ens);
        c.f->body = k.blkc({});
        k.done(c.f);
        return app;
    };
    const struct
    {
        Site site;
        const char* tag;
        const char* what;
    } sites[] {{Site::Range, "range", "reads_range(p, n) over struct P"},
               {Site::Member, "member", "owns(*b) of a structure whose member is an array of structures"},
               {Site::Global, "global", "owns(g) of a global array of structures"}};
    for (const auto& st : sites)
    {
        const Site site {st.site};
        lc(std::string("e1e2c2_apply_beside_") + st.tag + "_alone_fail",
           std::string(st.what) +
               " is the only site of int cells and first(a, m) reads a[0..m): the application is unframed (permission at the application), the structure site frames no interval of int slots",
           [=](Kit& k)
           {
               k.fail("permission", apply_case(k, site, false, "f")->loc);
           });
        lcv(std::string("e1e2c2_apply_beside_") + st.tag + "_framed_by_scalar_range",
            std::string(st.what) +
                " beside reads_range(a, m): the application is framed by the scalar range alone (the structure site is skipped, the scalar one stays)",
            [=](Kit& k)
            {
                apply_case(k, site, true, "f");
                vac_fn(k);
            });
    }
    // ---- finding 3: bool zero_at(int k, const int *p, int m) { return !(0 <= k && k < m) || p[k] == 0; }, forall zero_at(a, m) in the contract ----
    auto mkq = [=](Kit& k, int id, const std::vector<const Expr*>& args)
    {
        const Expr* i {k.b.bound(id, k.i32.int_type, "i", k.at())};
        std::vector<const Expr*> a;
        const std::vector<Type> ptypes {PI(k), k.i32};
        for (size_t j {0U}; j < args.size(); ++j)
        {
            a.push_back(k.b.forall_arg(id, static_cast<int>(j), ptypes[j],
                                       args[j]->c_text.empty() ? "x" : args[j]->c_text, k.at()));
        }
        const Expr* elem {k.deref(k.b.elem_addr(a[0], i, k.at()))};
        const Expr* b {k.lor(k.lnot(k.land(k.ge_c(i, k.i32, "0"), k.lt(i, a[1]))), k.eq(elem, N(k, "0")))};
        return k.b.forall(id, k.i32.int_type, b, args, "forall(zero_at)", "zero_at", k.at());
    };
    // void f(<site>, int *a, int m) requires(<site atom> [&& owns_range(a, m)] && m > 0 && forall zero_at(a, m)) ensures(<site atom> [&& owns_range(a, m)] [&& forall zero_at(a, m)]) { [a[0] = 0;] }
    auto forall_case = [=](Kit& k, Site site, bool scalar_range) -> const Expr*
    {
        Ctx c {make_f(k, site, "f")};
        const int ai {c.next};
        const Expr* q {mkq(k, 0, {fx(k, c.f, ai), fx(k, c.f, ai + 1)})};
        const Expr* site_atom {c.atom()};
        const Expr* sites_req {scalar_range ? k.land(site_atom, RO(k, c.f, ai, ai + 1, 1)) : site_atom};
        k.req(c.f, k.land(k.land(sites_req, k.gt(fx(k, c.f, ai + 1), N(k, "0"))), q));
        const Expr* site_ens {c.atom()};
        k.ens(c.f, scalar_range ? k.land(k.land(site_ens, RO(k, c.f, ai, ai + 1, 1)),
                                         mkq(k, 1, {fx(k, c.f, ai), fx(k, c.f, ai + 1)}))
                                : site_ens);
        std::vector<const Stmt*> body;
        if (scalar_range)
        {
            body.push_back(k.store(k.b.elem_addr(k.pv(c.f, ai), N(k, "0"), k.at()), N(k, "0")));
        }
        c.f->body = k.blkc(body);
        k.done(c.f);
        no_computed_slot(k);
        return q;
    };
    // ---- the same read in a loop invariant: struct P v[3] = {0}; int a[3] = {0}; int i = 0; while (i < 3) inv 0 <= i <= 3 && forall zero_at(a, 3) decreases 3 - i { v[i].x = 1; i++; }: the array
    // place of the structures' leaves frames no read under a binder; the scalar array's place frames a[j] ----
    lcv("e1e2c2_forall_loop_local_arrays_of_structures_and_scalars",
        "the loop writes v[i].x and keeps forall zero_at(a, 3): a[j] under the binder is framed by a's array place; the leaf places of v are left out of the binder's framing (no computed `loc` term)",
        [=](Kit& k)
        {
            StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
            Function* f {k.fn("f", k.vd)};
            Var* v {f->new_local("v", k.prog.array_of(k.st(P), 3U), k.at())};
            Var* a {f->new_local("a", k.prog.array_of(k.i32, 3U), k.at())};
            f->mark_address_taken(v);
            f->mark_address_taken(a);
            v->zero_filled = true;
            a->zero_filled = true;
            Var* i {k.local(f, "i", k.i32)};
            const Expr* q {mkq(k, 0, {k.addr(a), N(k, "3")})};
            const Stmt* w {k.store(k.b.member_addr(k.b.elem_addr(k.addr(v), k.v(i), k.at()), 0, k.at()), N(k, "1"))};
            const Stmt* step {k.asg(i, k.add(k.v(i), N(k, "1")))};
            const Stmt* lp {k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3")), k.cl(q)},
                                  {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc({w, step}))};
            std::vector<const Stmt*> body {k.decl(v, nullptr), k.decl(a, nullptr), k.decl(i, N(k, "0")), lp};
            if (k.vac_tail)
            {
                const Stmt* vc {k.chk(k.no())};
                body.push_back(vc);
                k.vac_loc = vc->loc;
            }
            f->body = k.blkc(body);
            k.done(f);
            no_computed_slot(k);
        });
    for (const auto& st : sites)
    {
        const Site site {st.site};
        lc(std::string("e1e2c2_forall_beside_") + st.tag + "_alone_fail",
           std::string(st.what) +
               " is the only site holding int cells and forall zero_at(a, m) reads a[j] under its binder: the structure site frames no read under a binder (permission at the quantifier), and the quantifier's body holds no computed `loc` term",
           [=](Kit& k)
           {
               k.fail("permission", forall_case(k, site, false)->loc);
           });
        lcv(std::string("e1e2c2_forall_beside_") + st.tag + "_framed_by_scalar_range",
            std::string(st.what) +
                " beside owns_range(a, m): the read under the binder is framed by the scalar range alone; a[0] = 0 keeps forall zero_at (verifies)",
            [=](Kit& k)
            {
                forall_case(k, site, true);
                vac_fn(k);
            });
    }
}

// Audit round C2, finding 1 (record 0064 item 3, "A length quantifier under another quantifier"): where the slot expression of a structure's length facts stands under a quantifier over `k` (the
// well-formedness row of a pointer leaf under a range's binder, or of the element pointers of a range), the length quantifier of a member that is an array of structures takes a binder its slot
// expression does not mention (`k1`), never a second `k`.  Each shape is stated, the program that uses the row verifies, and a twin with a false claim or a wrong index fails at its line.
void define_audit_c2c_binder_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto lcv = [](const std::string& name, const std::string& doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(name + "_vacuity",
           "the same with a function that ends in check(false): it fails (what the contract assumes is not inconsistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto vac_fn = [=](Kit& k)
    {
        if (k.vac_tail)
        {
            Function* g {k.fn("vac", k.vd)};
            const Stmt* v {k.chk(k.no())};
            g->body = k.blk({v});
            k.done(g);
            k.vac_loc = v->loc;
        }
    };
    // struct P { int x; int y; }; struct T { struct P v[2]; int n; }
    auto mk_t = [](Kit& k)
    {
        StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(P), 2U)}, {"n", k.i32}});
    };
    // void f(struct T **pp, int n) requires(n > 0 && reads_range(pp, n)) ensures(reads_range(pp, n)) { [check(pp[0] == null);] }: the pointee of every element pointer is a structure with a member
    // array of structures, so the well-formedness row of each element pointer holds that member's length quantifier under the range's binder (k07 of the audit)
    const auto reads_range_of_pointers = [=](Kit& k, bool false_claim, const Expr** load = nullptr) -> const Stmt*
    {
        StructType* T {mk_t(k)};
        Function* f {k.fn("f", k.vd, {{"pp", k.ptr(k.ptr(k.st(T)))}, {"n", k.i32}})};
        k.req(f, k.land(k.gt(fx(k, f, 1), N(k, "0")),
                        k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at())));
        k.ens(f, k.b.range_atom(AtomKind::ReadsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()));
        const Stmt* c {nullptr};
        if (false_claim)
        {
            const Expr* pr {k.deref(k.b.elem_addr(k.pv(f, 0), N(k, "0"), k.at()))};
            if (load != nullptr)
            {
                *load = pr;
            }
            c = k.chk(k.eq(pr, k.nul(k.ptr(k.st(T)))));
            f->body = k.blkc({c});
        }
        else
        {
            f->body = k.blkc({});
        }
        k.done(f);
        return c;
    };
    lcv("e1e2c2c_binder_reads_range_of_pointers_to_T",
        "reads_range(pp, n) over struct T **: the length quantifier of T's member array of structures stands under the range's binder, so its binder is k1 (verifies)",
        [=](Kit& k)
        {
            reads_range_of_pointers(k, false);
            vac_fn(k);
        });
    lc("e1e2c2c_binder_reads_range_of_pointers_to_T_false_check_fail",
       "the same with check(pp[0] == null) in the body: nothing says the pointer is null (check), and nothing says the object it points to is live (lifetime, at the load)",
       [=](Kit& k)
       {
           const Expr* load {nullptr};
           const Stmt* c {reads_range_of_pointers(k, true, &load)};
           k.fail("check", c->loc);
           // the load's own row and the comparison's: the second is the one that fails
           k.fail_only("lifetime", load->loc, 2U, 1U);
       });
    // struct E2 { struct T *up; int k; }; void f(struct E2 *p, int n) requires(n > 1 && owns_range(p, n)) ensures(owns_range(p, n)) { p[0].k = 7; check(p[0].k == <v>); } (j13 of the audit)
    const auto owns_range_with_pointer_member = [=](Kit& k, const char* claimed) -> const Stmt*
    {
        StructType* T {mk_t(k)};
        StructType* E2 {k.strukt("E2", {{"up", k.ptr(k.st(T))}, {"k", k.i32}})};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(E2))}, {"n", k.i32}})};
        k.req(f, k.land(k.gt(fx(k, f, 1), N(k, "1")),
                        k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at())));
        k.ens(f, k.b.range_atom(AtomKind::OwnsRange, fx(k, f, 0), fx(k, f, 1), 0, k.at()));
        auto place = [&](const char* i)
        {
            return k.b.member_addr(k.b.elem_addr(k.pv(f, 0), N(k, i), k.at()), 1, k.at());
        };
        const Stmt* w {k.store(place("0"), N(k, "7"))};
        const Stmt* c {k.chk(k.eq(k.deref(place("0")), N(k, claimed)))};
        f->body = k.blkc({w, c});
        k.done(f);
        return c;
    };
    lcv("e1e2c2c_binder_owns_range_over_a_structure_with_a_pointer_member",
        "owns_range(p, n) over struct E2 { struct T *up; int k; }: the pointer leaf's well-formedness row holds T's length quantifier under the range's binder (k1); p[0].k = 7 reads back 7 (verifies)",
        [=](Kit& k)
        {
            owns_range_with_pointer_member(k, "7");
            vac_fn(k);
        });
    lc("e1e2c2c_binder_owns_range_over_a_structure_with_a_pointer_member_wrong_value_fail",
       "the same with check(p[0].k == 8) after p[0].k = 7 (check)",
       [=](Kit& k)
       {
           const Stmt* c {owns_range_with_pointer_member(k, "8")};
           k.fail("check", c->loc);
       });
    // struct TT; struct E { struct TT *up; int k; }; struct TT { struct E v[2]; int n; }: int f(struct TT *p) requires(reads(*p)) ensures(reads(*p)) { return p->v[<i>].k; } (j11b of the audit):
    // a type that reaches itself through a pointer leaf of its own member array
    const auto recursive_read = [=](Kit& k, const char* index) -> const Expr*
    {
        StructType* TT {k.prog.add_struct("TT", "struct TT", k.at())};
        StructType* E {k.strukt("E", {{"up", k.ptr(k.st(TT))}, {"k", k.i32}})};
        k.prog.add_member(TT, "v", k.prog.array_of(k.st(E), 2U), k.at());
        k.prog.add_member(TT, "n", k.i32, k.at());
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(TT))}})};
        k.req(f, k.b.atom(AtomKind::Reads, k.b.text(k.F(f, 0), "p"), 0, k.at()));
        k.ens(f, k.b.atom(AtomKind::Reads, k.b.text(k.F(f, 0), "p"), 0, k.at()));
        const Expr* r {k.deref(k.b.member_addr(k.b.elem_addr(k.mem(k.pv(f, 0), 0), N(k, index), k.at()), 1, k.at()))};
        f->body = k.blkc({k.ret(r)});
        k.done(f);
        return r;
    };
    lcv("e1e2c2c_binder_recursive_type_read",
        "reads(*p) of a structure that reaches itself through the pointer leaf of its member array's elements: the well-formedness row of the pointer leaf `up`, under the binder k of the member array's slots, holds the next member array's length quantifier over k1; p->v[1].k is in bounds (verifies)",
        [=](Kit& k)
        {
            recursive_read(k, "1");
            vac_fn(k);
        });
    lc("e1e2c2c_binder_recursive_type_read_outside_fail",
       "the same reading p->v[2].k, one past the member array's length (out-of-bounds at the read)",
       [=](Kit& k)
       {
           const Expr* r {recursive_read(k, "2")};
           k.fail("out-of-bounds", r->loc);
       });
}

// ---- Part D (M4.5e1e2, record 0064 item 15; question 348: array members of SCALARS only) ----
// The load-rooted array part (question 306): owns(*p->q) and p->q->a[e] for a structure with an array member of scalars (struct SA { int n; int a[4]; }); built first, since no spike fixture covers a
// receiver that holds a heap read.
void define_part_d_load_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto fx = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto lcv = [](const std::string& name, const std::string& doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(name + "_vacuity",
           "the same with a function that ends in check(false): it fails (what the contract assumes is not inconsistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto vac_fn = [=](Kit& k)
    {
        if (k.vac_tail)
        {
            Function* g {k.fn("vac", k.vd)};
            const Stmt* v {k.chk(k.no())};
            g->body = k.blk({v});
            k.done(g);
            k.vac_loc = v->loc;
        }
    };
    // struct SA { int n; int a[4]; }; struct R { struct SA *q; int z; }
    auto mk_r = [](Kit& k, StructType** sa)
    {
        StructType* S {k.strukt("SA", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 4U)}})};
        *sa = S;
        return k.strukt("R", {{"q", k.ptr(k.st(S))}, {"z", k.i32}});
    };
    // int f(struct R *p) requires(owns(*p) && owns(*p->q) && p->q->a[0] > 0) ensures(owns(*p) && owns(*p->q) && result == p->q->a[0]) { [check(p->q->a[0] == 5);] return p->q->a[0]; }
    const auto load_read = [=](Kit& k, bool false_claim, const Expr** where = nullptr) -> const Stmt*
    {
        StructType* S {nullptr};
        StructType* R {mk_r(k, &S)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(R))}})};
        auto spec_elem = [&]()
        {
            return k.deref(k.b.elem_addr(k.mem(k.deref(k.mem(fx(k, f, 0), 0)), 1), N(k, "0"), k.at()));
        };
        auto atoms = [&]()
        {
            return k.land(k.b.atom(AtomKind::Owns, fx(k, f, 0), 0, k.at()),
                          k.b.atom(AtomKind::Owns, k.deref(k.mem(fx(k, f, 0), 0)), 1, k.at()));
        };
        k.req(f, k.land(atoms(), k.gt(spec_elem(), N(k, "0"))));
        k.ens(f, k.land(atoms(), k.eq(k.R(f), spec_elem())));
        auto code_elem = [&]()
        {
            return k.deref(k.b.elem_addr(k.mem(k.deref(k.mem(k.pv(f, 0), 0)), 1), N(k, "0"), k.at()));
        };
        std::vector<const Stmt*> body;
        const Stmt* c {nullptr};
        if (false_claim)
        {
            const Expr* r {code_elem()};
            if (where != nullptr)
            {
                *where = r;
            }
            c = k.chk(k.eq(r, N(k, "5")));
            body.push_back(c);
        }
        body.push_back(k.ret(code_elem()));
        f->body = k.blkc(body);
        k.done(f);
        return c;
    };
    lcv("e1e2d_load_array_read",
        "owns(*p) && owns(*p->q) with q a pointer to a structure with an array member of scalars, and the contract read p->q->a[0]: the array part's receiver holds a heap read (question 306); the body returns it (verifies)",
        [=](Kit& k)
        {
            load_read(k, false);
            vac_fn(k);
        });
    lc("e1e2d_load_array_read_wrong_fail",
       "the same, with check(p->q->a[0] == 5) after only a > 0 is known: the check fails",
       [=](Kit& k)
       {
           const Stmt* c {load_read(k, true)};
           k.fail("check", c->loc);
       });
    // f__wf's ghosts for a load-rooted array part (question 306; the wf of tests/memory/struct_array_chain_wf.c): int f(struct R *p) requires(owns(*p) && owns(*p->q) && p->q->a[1] != 0) ensures(owns(*p) && owns(*p->q) && p->q->a[1] != 0 && (result == 0 || result == 10 / p->q->a[1 [mode 1: 2]])) { return 0; }
    const auto load_wf = [=](Kit& k, int mode) -> const Expr*
    {
        StructType* S {nullptr};
        StructType* R {mk_r(k, &S)};
        Function* f {k.fn("f", k.i32, {{"p", k.ptr(k.st(R))}})};
        auto spec_elem = [&](const char* i)
        {
            return k.deref(k.b.elem_addr(k.mem(k.deref(k.mem(fx(k, f, 0), 0)), 1), N(k, i), k.at()));
        };
        auto atoms = [&]()
        {
            return k.land(k.b.atom(AtomKind::Owns, fx(k, f, 0), 0, k.at()),
                          k.b.atom(AtomKind::Owns, k.deref(k.mem(fx(k, f, 0), 0)), 1, k.at()));
        };
        k.req(f, k.land(atoms(), k.ne(spec_elem("1"), N(k, "0"))));
        const Expr* dv {k.div(N(k, "10"), spec_elem(mode == 1 ? "2" : "1"))};
        k.ens(f, k.land(k.land(atoms(), k.ne(spec_elem("1"), N(k, "0"))),
                        k.lor(k.eq(k.R(f), N(k, "0")), k.eq(k.R(f), dv))));
        f->body = k.blkc({k.ret(N(k, "0"))});
        k.done(f);
        return dv;
    };
    lcv("e1e2d_load_array_wf_division",
        "a clause that divides by p->q->a[1] after stating p->q->a[1] != 0: the division's obligation holds in f__wf over the ghost of the load-rooted element (verifies)",
        [=](Kit& k)
        {
            load_wf(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_load_array_wf_division_fail",
       "the clause divides by p->q->a[2] while only p->q->a[1] != 0 is stated: division-by-zero in the contract's well-formedness",
       [=](Kit& k)
       {
           k.fail("division-by-zero", load_wf(k, 1)->loc);
       });
    // void g(struct R *p) requires(owns(*p) && owns(*p->q)) ensures(owns(*p) && owns(*p->q) && p->q->a[1] == [3|4]) { p->q->a[1] = 3; p->q->n = p->q->a[1]; }
    const auto load_write = [=](Kit& k, const char* promised)
    {
        StructType* S {nullptr};
        StructType* R {mk_r(k, &S)};
        Function* f {k.fn("g", k.vd, {{"p", k.ptr(k.st(R))}})};
        auto atoms = [&]()
        {
            return k.land(k.b.atom(AtomKind::Owns, fx(k, f, 0), 0, k.at()),
                          k.b.atom(AtomKind::Owns, k.deref(k.mem(fx(k, f, 0), 0)), 1, k.at()));
        };
        k.req(f, atoms());
        const Expr* spec {k.deref(k.b.elem_addr(k.mem(k.deref(k.mem(fx(k, f, 0), 0)), 1), N(k, "1"), k.at()))};
        k.ens(f, k.land(atoms(), k.eq(spec, N(k, promised))));
        auto place = [&]()
        {
            return k.b.elem_addr(k.mem(k.deref(k.mem(k.pv(f, 0), 0)), 1), N(k, "1"), k.at());
        };
        const Stmt* w {k.store(place(), N(k, "3"))};
        const Stmt* w2 {k.store(k.mem(k.deref(k.mem(k.pv(f, 0), 0)), 0), k.deref(place()))};
        f->body = k.blkc({w, w2});
        k.done(f);
        return f->end_loc;
    };
    lc("e1e2d_load_array_write",
       "owns(*p) && owns(*p->q): p->q->a[1] = 3 through the array part reached by a load, then p->q->n = p->q->a[1], the postcondition p->q->a[1] == 3 holds",
       [=](Kit& k)
       {
           load_write(k, "3");
       });
    lc("e1e2d_load_array_write_wrong_fail",
       "the same promising p->q->a[1] == 4: the postcondition fails at the closing brace",
       [=](Kit& k)
       {
           const SrcLoc at {load_write(k, "4")};
           k.fail("postcondition", at);
       });
}

// ---- Part D, heap storage of a structure with an array member of scalars (item 15, question 292; S48 (h)) ----
// malloc's or calloc's typed view for one object of a structure with array members: heap_alloc_view's text, the zero clauses over the cells (the array leaves are the atom's `zeroed` mark)
Function* part_d_alloc_view(Kit& k, const char* name, Type pointee, bool zeroed)
{
    const Type pt {k.ptr(pointee)};
    Function* f {k.fn(name, pt)};
    k.trusted(f, std::string("vlibc: ") + name + " returns NULL or one new block; typed view: one " + pointee.str() +
                     ", decision 0047");
    const Expr* is_null {k.eq(k.R(f), k.nul(pt))};
    const Expr* atom {k.b.atom(AtomKind::OwnsBlock, k.R(f), -1, k.at())};
    const Expr* rhs {atom};
    if (zeroed)
    {
        // the encoder's view: one value clause per cell leaf and, for an array leaf of scalars, one per ELEMENT (`result->a[i] == 0`, i a literal)
        for (const StructLeaf& lf : struct_object_leaves(*pointee.struct_type))
        {
            const Expr* addr {k.R(f)};
            for (int idx : lf.path)
            {
                addr = k.mem(addr, idx);
            }
            auto zero_of = [&](const Type& t) -> const Expr*
            {
                return t.is_bool() ? k.no() : t.is_ptr() ? k.nul(t) : k.num(t, "0");
            };
            if (lf.array)
            {
                for (unsigned long long i {0U}; i < lf.count; ++i)
                {
                    const Expr* el {k.b.elem_addr(addr, k.num(k.i32, std::to_string(i).c_str()), k.at())};
                    rhs = k.land(rhs, k.eq(k.deref(el), zero_of(lf.element)));
                }
                continue;
            }
            rhs = k.land(rhs, k.eq(k.deref(addr), zero_of(lf.type)));
        }
    }
    else
    {
        f->result_init = InitMarker::RefToUninit;
    }
    k.ens(f, k.lor(is_null, rhs));
    k.done(f);
    return f;
}

void define_part_d_heap_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto FP = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto lcv = [](const std::string& name, const std::string& doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(name + "_vacuity",
           "the same with a function that ends in check(false): it fails (what the contract assumes is not inconsistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto vac_fn = [=](Kit& k)
    {
        if (k.vac_tail)
        {
            Function* g {k.fn("vac", k.vd)};
            const Stmt* v {k.chk(k.no())};
            g->body = k.blk({v});
            k.done(g);
            k.vac_loc = v->loc;
        }
    };
    auto SA = [](Kit& k)
    {
        return k.strukt("SA", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 4U)}});
    };
    auto elem_place = [](Kit& k, const Expr* base, const char* i)
    {
        return k.b.elem_addr(k.mem(base, 1), k.num(k.i32, i), k.at());
    };
    auto free_view = [](Kit& k, Type pointee)
    {
        const Type pt {k.ptr(pointee)};
        Function* f {k.fn("free", k.vd, {{"ptr", pt}})};
        f->params[0U].init = InitMarker::RefToUninit;
        k.trusted(f,
                  std::string("vlibc: free ends the allocation; typed view: one ") + pointee.str() + ", decision 0047");
        k.req(f, k.lor(k.eq(k.b.text(k.F(f, 0), "ptr"), k.nul(pt)),
                       k.b.atom(AtomKind::OwnsBlock, k.b.text(k.F(f, 0), "ptr"), -1, k.at())));
        f->deallocates = true;
        k.done(f);
        return f;
    };
    // void f(void) { struct SA *p = malloc(); if (p != NULL) { p->n = 1; p->a[0..3] = 1..4 [(mode 1: a[2] unwritten, read into a local)]; check(p->a[2] == 3 [mode 5: == 7]); free(p); [mode 2: free(p) again | mode 4: p->a[1] = 9]} [mode 3: no free] }
    const auto life = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        Function* m {part_d_alloc_view(k, "malloc", k.st(S), false)};
        Function* fr {free_view(k, k.st(S))};
        Function* f {k.fn("f", k.vd)};
        Var* p {k.local(f, "p", SP)};
        SrcLoc where;
        std::vector<const Stmt*> in {k.lstore(k.pn(p), 0, N(k, "1"))};
        const char* vals[4] = {"1", "2", "3", "4"};
        const char* idx[4] = {"0", "1", "2", "3"};
        for (int i {0}; i < 4; ++i)
        {
            if (mode == 1 && i == 2)
            {
                continue;
            }
            in.push_back(k.store(elem_place(k, k.pn(p), idx[i]), N(k, vals[i])));
        }
        if (mode == 1)
        {
            Var* x {k.local(f, "x", k.i32)};
            const Expr* rd {k.deref(elem_place(k, k.pn(p), "2"))};
            where = rd->loc;
            in.push_back(k.decl(x, rd));
        }
        else
        {
            const Stmt* c {k.chk(k.eq_c(k.deref(elem_place(k, k.pn(p), "2")), k.i32, mode == 5 ? "7" : "3"))};
            if (mode == 5)
            {
                where = c->loc;
            }
            in.push_back(c);
        }
        if (mode != 3)
        {
            in.push_back(k.es(k.call(fr, {k.pn(p)})));
        }
        if (mode == 2)
        {
            const Expr* again {k.call(fr, {k.pn(p)})};
            where = again->loc;
            in.push_back(k.es(again));
        }
        if (mode == 4)
        {
            const Stmt* w {k.store(elem_place(k, k.pn(p), "1"), N(k, "9"))};
            where = w->loc;
            in.push_back(w);
        }
        std::vector<const Stmt*> body {k.decl(p, k.call(m, {})), k.iff(k.ne(k.pn(p), k.nul(SP)), k.blk(in))};
        f->body = k.blk(body);
        k.done(f);
        if (mode == 3)
        {
            where = f->end_loc;
        }
        return where;
    };
    lcv("e1e2d_heap_array_member",
        "p = malloc(); if (p != NULL) { p->n = 1; p->a[0..3] written; check(p->a[2] == 3); free(p); } for a structure with an array member of scalars: the typed view holds the array leaf's permissions, free takes them, nothing leaks",
        [=](Kit& k)
        {
            life(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_heap_array_member_read_unwritten_fail",
       "malloc leaves the flags of the array leaf unknown: p->a[2] never written and read into a local is uninitialized at the read",
       [=](Kit& k)
       {
           k.fail("uninitialized", life(k, 1));
       });
    lc("e1e2d_heap_array_member_wrong_check_fail", "check(p->a[2] == 7) after storing 3: the check fails",
       [=](Kit& k)
       {
           k.fail("check", life(k, 5));
       });
    lc("e1e2d_heap_array_member_write_after_free_fail",
       "p->a[1] = 9 after free(p): the array leaf is gone (permission at the store)",
       [=](Kit& k)
       {
           k.fail("permission", life(k, 4));
       });
    lc("e1e2d_heap_array_member_leak_fail",
       "no free(p): the function returns while it owns the allocation (leak at the closing brace)",
       [=](Kit& k)
       {
           k.fail("leak", life(k, 3));
       });
    lc("e1e2d_heap_array_member_double_free_fail", "free(p) twice: the second finds no token (invalid-free)",
       [=](Kit& k)
       {
           k.fail_only("invalid-free", life(k, 2), 6U, 0U);
       });
    // calloc: every element zero and every flag set (struct SA *p = calloc(); check(p->a[3] == 0 [mode 1: == 1]); free(p))
    const auto calloc_prog = [=](Kit& k, bool wrong) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        Function* m {part_d_alloc_view(k, "calloc", k.st(S), true)};
        Function* fr {free_view(k, k.st(S))};
        Function* f {k.fn("f", k.vd)};
        Var* p {k.local(f, "p", SP)};
        const Stmt* c {k.chk(k.land(k.eq_c(k.deref(elem_place(k, k.pn(p), "3")), k.i32, wrong ? "1" : "0"),
                                    k.eq_c(k.leaf(k.pn(p), 0), k.i32, "0")))};
        f->body =
            k.blk({k.decl(p, k.call(m, {})), k.iff(k.ne(k.pn(p), k.nul(SP)), k.blk({c, k.es(k.call(fr, {k.pn(p)}))}))});
        k.done(f);
        return c->loc;
    };
    lcv("e1e2d_heap_calloc_array_member",
        "p = calloc(): every element of p->a and p->n are zero and set, free takes the block (verifies)",
        [=](Kit& k)
        {
            calloc_prog(k, false);
            vac_fn(k);
        });
    lc("e1e2d_heap_calloc_array_member_wrong_fail", "check(p->a[3] == 1) after calloc: the check fails",
       [=](Kit& k)
       {
           k.fail("check", calloc_prog(k, true));
       });
    // question 351: the trusted calloc view is not vacuous.  void f(void) { struct SA *p = calloc(); if (p != NULL) { check(p->a[3] == 0); check(false); free(p); } }: the false check is reported
    lc("e1e2d_heap_calloc_view_not_vacuous_fail",
       "calloc's view reads its last element in its own ensures (framed by its block site); a function that reads p->a[3] and then asserts false still reports the false check (the trusted ensures is satisfiable)",
       [=](Kit& k)
       {
           StructType* S {SA(k)};
           const Type SP {k.ptr(k.st(S))};
           Function* m {part_d_alloc_view(k, "calloc", k.st(S), true)};
           Function* fr {free_view(k, k.st(S))};
           Function* f {k.fn("f", k.vd)};
           Var* p {k.local(f, "p", SP)};
           const Stmt* c1 {k.chk(k.eq_c(k.deref(elem_place(k, k.pn(p), "3")), k.i32, "0"))};
           const Stmt* c2 {k.chk(k.no())};
           f->body = k.blk({k.decl(p, k.call(m, {})),
                            k.iff(k.ne(k.pn(p), k.nul(SP)), k.blk({c1, c2, k.es(k.call(fr, {k.pn(p)}))}))});
           k.done(f);
           k.fail("check", c2->loc);
       });
    // void g(int i) requires(0 <= i && i < 4) { struct SA *p = calloc(); if (p != NULL) { check(p->a[i] == 0); free(p); } }
    const auto calloc_sym = [=](Kit& k, bool wrong) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        Function* m {part_d_alloc_view(k, "calloc", k.st(S), true)};
        Function* fr {free_view(k, k.st(S))};
        Function* f {k.fn("g", k.vd, {{"i", k.i32}})};
        k.req(f, k.land(k.le(N(k, "0"), FP(k, f, 0)), k.lt(FP(k, f, 0), N(k, "4"))));
        Var* p {k.local(f, "p", SP)};
        const Expr* rd {k.deref(k.b.elem_addr(k.mem(k.pn(p), 1), k.pv(f, 0), k.at()))};
        const Stmt* c {k.chk(k.eq_c(rd, k.i32, wrong ? "1" : "0"))};
        f->body = k.blkc(
            {k.decl(p, k.call(m, {})), k.iff(k.ne(k.pn(p), k.nul(SP)), k.blk({c, k.es(k.call(fr, {k.pn(p)}))}))});
        k.done(f);
        return c->loc;
    };
    lc("e1e2d_heap_calloc_symbolic_index",
       "p = calloc(); p->a[i] at a symbolic i in [0, 4) is zero: every element is zero and set (verifies)",
       [=](Kit& k)
       {
           calloc_sym(k, false);
       });
    lc("e1e2d_heap_calloc_symbolic_index_wrong_fail",
       "check(p->a[i] == 1) at a symbolic i after calloc: the check fails",
       [=](Kit& k)
       {
           k.fail("check", calloc_sym(k, true));
       });
    // void use(struct SA *p) requires(owns_block(p)) ensures(owns_block(p)) { p->a[1] = 3; }
    // void g(void) { struct SA *p = malloc(); if (p != NULL) { p->n = 0; p->a[0..3] = 0 [mode 1: a[3] unwritten]; use(p); [check(p->a[1] == 3)] free(p); } }
    const auto contract_prog = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        Function* m {part_d_alloc_view(k, "malloc", k.st(S), false)};
        Function* fr {free_view(k, k.st(S))};
        Function* u {k.fn("use", k.vd, {{"p", SP}})};
        k.req(u, k.b.atom(AtomKind::OwnsBlock, FP(k, u, 0), -1, k.at()));
        k.ens(u, k.b.atom(AtomKind::OwnsBlock, FP(k, u, 0), -1, k.at()));
        u->body = k.blkc({k.store(elem_place(k, k.pv(u, 0), "1"), N(k, "3"))});
        k.done(u);
        Function* f {k.fn("g", k.vd)};
        Var* p {k.local(f, "p", SP)};
        std::vector<const Stmt*> in {k.lstore(k.pn(p), 0, N(k, "0"))};
        for (const char* i : {"0", "1", "2", "3"})
        {
            if (mode == 1 && std::string(i) == "3")
            {
                continue;
            }
            in.push_back(k.store(elem_place(k, k.pn(p), i), N(k, "0")));
        }
        const Expr* call {k.call(u, {k.pn(p)})};
        in.push_back(k.es(call));
        SrcLoc where {call->loc};
        if (mode == 2)
        {
            const Stmt* c {k.chk(k.eq_c(k.deref(elem_place(k, k.pn(p), "1")), k.i32, "3"))};
            where = c->loc;
            in.push_back(c);
        }
        in.push_back(k.es(k.call(fr, {k.pn(p)})));
        f->body = k.blk({k.decl(p, k.call(m, {})), k.iff(k.ne(k.pn(p), k.nul(SP)), k.blk(in))});
        k.done(f);
        return where;
    };
    lcv("e1e2d_heap_owns_block_contract",
        "use(p) requires and ensures owns_block(p) of a structure with an array member and writes p->a[1]; the caller holds a block with every element written, calls it and frees the block (verifies)",
        [=](Kit& k)
        {
            contract_prog(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_heap_owns_block_contract_unwritten_fail",
       "the caller writes p->a[0..2] but not p->a[3]: the call needs every element to have a value (uninitialized at the call)",
       [=](Kit& k)
       {
           k.fail_only("uninitialized", contract_prog(k, 1), 2U, 1U);
       });
    lc("e1e2d_heap_owns_block_contract_forgot_fail",
       "check(p->a[1] == 3) after use(p): owns_block says nothing about the contents, the check fails",
       [=](Kit& k)
       {
           k.fail("check", contract_prog(k, 2));
       });
    // void drop(struct SA *p) requires(owns_block(p)) { free(p); }   [mode 1: { }: the allocation is not handed back]
    const auto drop_prog = [=](Kit& k, bool forget) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        Function* fr {free_view(k, k.st(S))};
        Function* d {k.fn("drop", k.vd, {{"p", SP}})};
        k.req(d, k.b.atom(AtomKind::OwnsBlock, FP(k, d, 0), -1, k.at()));
        d->body = forget ? k.blkc({}) : k.blkc({k.es(k.call(fr, {k.pv(d, 0)}))});
        k.done(d);
        return d->end_loc;
    };
    lc("e1e2d_heap_drop", "drop(p) requires owns_block(p) and frees it: the free's rows hold, nothing leaks",
       [=](Kit& k)
       {
           drop_prog(k, false);
       });
    lc("e1e2d_heap_drop_leak_fail", "the same without the free: the function returns owning the allocation (leak)",
       [=](Kit& k)
       {
           k.fail("leak", drop_prog(k, true));
       });
    // ---- the loop column (Part D2, step 1): the array part of a heap block is a place of the loop ----
    // void f(void) { struct SA *p = alloc(); if (p != NULL) { int i = 0; while (i < 4) inv 0 <= i <= 4 { p->a[i] = i; i++ [mode 2: free(p);] } x = p->a[3]; [mode 1: the loop stops at 3 and
    // malloc leaves the flags unknown] free(p); } }
    // mode 0: calloc, the fill loop, the read after it (the flag the loop kept); 1: malloc, a loop that fills three elements, the read of the fourth (uninitialized); 2: calloc and a loop that
    // frees the block (the back edge ends without it); 3: the same over a structure with no cell, only the array member (the liveness and the token are carried by a place with no value)
    const auto heap_loop_prog = [=](Kit& k, int mode) -> SrcLoc
    {
        const bool only {mode == 3};
        StructType* S {only ? k.strukt("SO", {{"a", k.prog.array_of(k.i32, 4U)}}) : SA(k)};
        const int member {only ? 0 : 1};
        const Type SP {k.ptr(k.st(S))};
        Function* m {part_d_alloc_view(k, mode == 1 ? "malloc" : "calloc", k.st(S), mode != 1)};
        Function* fr {free_view(k, k.st(S))};
        Function* f {k.fn("f", k.vd)};
        Var* p {k.local(f, "p", SP)};
        Var* i {k.local(f, "i", k.i32)};
        Var* x {k.local(f, "x", k.i32)};
        const char* top {mode == 1 ? "3" : "4"};
        std::vector<const Stmt*> body {k.store(k.b.elem_addr(k.mem(k.pn(p), member), k.v(i), k.at()), k.v(i)),
                                       k.incS(i)};
        if (mode == 2)
        {
            body.push_back(k.es(k.call(fr, {k.pn(p)})));
        }
        const Stmt* loop_body {k.blkc(body)};
        const Expr* rd {k.deref(k.b.elem_addr(k.mem(k.pn(p), member), N(k, "3"), k.at()))};
        std::vector<const Stmt*> in {k.decl(i, N(k, "0")),
                                     k.whl(k.lt(k.v(i), N(k, top)), {k.cl(k.range(k.v(i), k.i32, "0", top))},
                                           {k.cl(k.sub(N(k, top), k.v(i)))}, loop_body),
                                     k.decl(x, rd)};
        in.push_back(k.es(k.call(fr, {k.pn(p)})));
        f->body = k.blk({k.decl(p, k.call(m, {})), k.iff(k.ne(k.pn(p), k.nul(SP)), k.blk(in))});
        k.done(f);
        return mode == 1 ? rd->loc : loop_body->end_loc;
    };
    lcv("e1e2d_loop_heap_fill",
        "p = calloc(); while (i < 4) { p->a[i] = i; i++; } x = p->a[3]; free(p): the array part of the block is a place of the loop (the permission at the entry amount, the flags kept), so the loop's stores and the read after it verify",
        [=](Kit& k)
        {
            heap_loop_prog(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_loop_heap_fill_unwritten_fail",
       "p = malloc(); a loop fills p->a[0..3) and p->a[3] is read after it: the flags of the array part are the entry's (unknown), so the read is uninitialized",
       [=](Kit& k)
       {
           k.fail("uninitialized", heap_loop_prog(k, 1));
       });
    lc("e1e2d_loop_heap_give_away_fail",
       "the loop frees p at the end of each iteration: the iteration ends without the permission to the block that it began with (permission at the closing brace of the body)",
       [=](Kit& k)
       {
           k.fail_only("permission", heap_loop_prog(k, 2), 2U, 0U, "the permission to `p->a` that it began with");
       });
    lcv("e1e2d_loop_heap_only_array",
        "a structure whose only member is an array: the heap block's liveness cell and token are carried by a place with no value, the array part by its own place; the fill loop verifies and free takes the block",
        [=](Kit& k)
        {
            heap_loop_prog(k, 3);
            vac_fn(k);
        });
    // mode 4: calloc, a callee that owns &p->a[2] and keeps it, then the loop: the loop begins without the permission to every element of p->a (entry row, not the automatic invariant)
    lc("e1e2d_loop_heap_kept_element_fail",
       "a callee takes ownership of &p->a[2] before the loop: the loop begins without the permission to p->a (permission at the loop, from the part's entry row)",
       [=](Kit& k)
       {
           StructType* S {SA(k)};
           const Type SP {k.ptr(k.st(S))};
           Function* m {part_d_alloc_view(k, "calloc", k.st(S), true)};
           Function* fr {free_view(k, k.st(S))};
           Function* t {k.fn("take", k.vd, {{"q", k.ptr(k.i32)}})};
           k.req(t, k.b.atom(AtomKind::Owns, FP(k, t, 0), 0, k.at()));
           t->body = k.blkc({});
           k.done(t);
           Function* f {k.fn("f", k.vd)};
           Var* p {k.local(f, "p", SP)};
           Var* i {k.local(f, "i", k.i32)};
           const Stmt* wl {k.whl(
               k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4"))}, {k.cl(k.sub(N(k, "4"), k.v(i)))},
               k.blkc({k.store(k.b.elem_addr(k.mem(k.pn(p), 1), k.v(i), k.at()), k.v(i)), k.incS(i)}))};
           const Stmt* call_take {k.es(k.call(t, {k.b.elem_addr(k.mem(k.pn(p), 1), N(k, "2"), k.at())}))};
           f->body = k.blk({k.decl(p, k.call(m, {})),
                            k.iff(k.ne(k.pn(p), k.nul(SP)),
                                  k.blk({call_take, k.decl(i, N(k, "0")), wl, k.es(k.call(fr, {k.pn(p)}))}))});
           k.done(f);
           k.fail_only("permission", wl->loc, 2U, 0U, "this loop begins without the permission to `p->a`");
       });
    // void fill(struct SA *p) requires(owns_block(p)) ensures(owns_block(p)) { int i = 0; while (i < 4) inv 0 <= i <= 4 { p->a[i] = i; i++; [mode 1: free(p);] } }
    const auto param_loop_prog = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        Function* fr {free_view(k, k.st(S))};
        Function* f {k.fn("fill", k.vd, {{"p", SP}})};
        k.req(f, k.b.atom(AtomKind::OwnsBlock, FP(k, f, 0), -1, k.at()));
        k.ens(f, k.b.atom(AtomKind::OwnsBlock, FP(k, f, 0), -1, k.at()));
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body {k.store(k.b.elem_addr(k.mem(k.pv(f, 0), 1), k.v(i), k.at()), k.v(i)), k.incS(i)};
        if (mode == 1)
        {
            body.push_back(k.es(k.call(fr, {k.pv(f, 0)})));
        }
        const Stmt* loop_body {k.blkc(body)};
        f->body = k.blk({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4"))},
                                                     {k.cl(k.sub(N(k, "4"), k.v(i)))}, loop_body)});
        k.done(f);
        return loop_body->end_loc;
    };
    lcv("e1e2d_loop_heap_param_block",
        "fill(p) requires and ensures owns_block(p) and writes p->a[i] in a loop: the parameter's block is held through the loop, its array part by a heap array place (verifies)",
        [=](Kit& k)
        {
            param_loop_prog(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_loop_heap_param_block_give_away_fail",
       "the loop frees the parameter's block at the end of each iteration: the iteration ends without the permission to p->a that it began with",
       [=](Kit& k)
       {
           k.fail_only("permission", param_loop_prog(k, 1), 2U, 0U, "the permission to `p->a` that it began with");
       });
    // void f(void) { [[owning]] struct SA *q = calloc(); if (q != NULL) { int i = 0; while (i < 4) { q->a[i] = i; i++; [mode 1: free(q);] } x = q->a[3]; free(q); } }
    const auto owning_loop_prog = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        Function* m {part_d_alloc_view(k, "calloc", k.st(S), true)};
        Function* fr {free_view(k, k.st(S))};
        Function* f {k.fn("f", k.vd)};
        Var* q {k.local(f, "q", SP)};
        q->owning = true;
        Var* i {k.local(f, "i", k.i32)};
        Var* x {k.local(f, "x", k.i32)};
        std::vector<const Stmt*> body {k.store(k.b.elem_addr(k.mem(k.pn(q), 1), k.v(i), k.at()), k.v(i)), k.incS(i)};
        if (mode == 1)
        {
            body.push_back(k.es(k.call(fr, {k.pn(q)})));
        }
        const Stmt* loop_body {k.blkc(body)};
        const Expr* rd {k.deref(k.b.elem_addr(k.mem(k.pn(q), 1), N(k, "3"), k.at()))};
        f->body = k.blkc({k.decl(q, k.call(m, {})),
                          k.iff(k.ne(k.pn(q), k.nul(SP)),
                                k.blk({k.decl(i, N(k, "0")),
                                       k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4"))},
                                             {k.cl(k.sub(N(k, "4"), k.v(i)))}, loop_body),
                                       k.decl(x, rd), k.es(k.call(fr, {k.pn(q)}))}))});
        k.done(f);
        return loop_body->end_loc;
    };
    lcv("e1e2d_loop_heap_owning_local",
        "an owning local filled by a loop (q->a[i] = i) and freed after it: the loop carries the block through its heap place, the array part included (verifies)",
        [=](Kit& k)
        {
            owning_loop_prog(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_loop_heap_owning_local_give_away_fail",
       "the loop frees the owning local at the end of each iteration: the iteration ends without the permission to q->a that it began with",
       [=](Kit& k)
       {
           k.fail_only("permission", owning_loop_prog(k, 1), 2U, 0U, "the permission to `q->a` that it began with");
       });
    // ---- the loop column's owner carriers (Part D, step A): the written site of an owner in memory carries the block's array parts ----
    // struct SA { int n; int a[4]; }; [[owning]] struct SA *g = NULL;
    // void f(void) requires(owns(g) && (g == NULL || owns_block(g)) && g != NULL) ensures(owns(g) && (g == NULL || owns_block(g))) { int i = 0; while (i < 4) { g->a[i] = i; i++; [mode 1: free(g);] } }
    const auto owner_global_loop = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        Function* fr {free_view(k, k.st(S))};
        Global* g {k.prog.add_global("g", SP, k.at())};
        g->init_kind = Global::Init::Null;
        g->owning = true;
        auto gval = [&](Kit& kk)
        {
            return kk.deref(kk.b.global_addr(g, kk.at()));
        };
        auto deep = [&](Kit& kk)
        {
            return kk.land(kk.b.atom(AtomKind::Owns, kk.b.global_addr(g, kk.at()), 0, kk.at()),
                           kk.lor(kk.eq(gval(kk), kk.nul(SP)), kk.b.atom(AtomKind::OwnsBlock, gval(kk), -1, kk.at())));
        };
        Function* f {k.fn("f", k.vd)};
        k.req(f, k.land(deep(k), k.ne(gval(k), k.nul(SP))));
        k.ens(f, deep(k));
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body {k.store(k.b.elem_addr(k.mem(gval(k), 1), k.v(i), k.at()), k.v(i)), k.incS(i)};
        if (mode == 1)
        {
            body.push_back(k.es(k.call(fr, {gval(k)})));
        }
        const Stmt* loop_body {k.blkc(body)};
        f->body = k.blk({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4"))},
                                                     {k.cl(k.sub(N(k, "4"), k.v(i)))}, loop_body)});
        k.done(f);
        return loop_body->end_loc;
    };
    lcv("e1e2d_loop_owner_global",
        "an owning global holding a structure with an array member, filled by a loop (g->a[i] = i): the loop's synthetic written site carries the block's array part, the contract's deep atom is kept (verifies)",
        [=](Kit& k)
        {
            owner_global_loop(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_loop_owner_global_give_away_fail",
       "the loop frees the owning global's block at the end of each iteration: the invariant of the loop needs the whole allocation g points to",
       [=](Kit& k)
       {
           k.fail_only("permission", owner_global_loop(k, 1), 7U, 1U, "to own the whole allocation `g` points to");
       });
    // A loop that REASSIGNS the owning global (decision 0064 item 15, Part D step 1d): the written site's pointer is read through memory the loop writes, so its array parts are named by a
    // ghost taken again at the back edge, after the rows
    // void f(void) requires(deep && g != NULL) ensures(deep) { int i = 0; while (i < 3) { [mode 2: no] free(g); g = [mode 1: malloc, else calloc](); i++; } if (g != NULL) { g->n = 0; g->a[0..3] = 0; } }
    const auto owner_global_reassigned = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        Function* m {part_d_alloc_view(k, mode == 1 ? "malloc" : "calloc", k.st(S), mode != 1)};
        Function* fr {free_view(k, k.st(S))};
        Global* g {k.prog.add_global("g", SP, k.at())};
        g->init_kind = Global::Init::Null;
        g->owning = true;
        auto gval = [&](Kit& kk)
        {
            return kk.deref(kk.b.global_addr(g, kk.at()));
        };
        auto deep = [&](Kit& kk)
        {
            return kk.land(kk.b.atom(AtomKind::Owns, kk.b.global_addr(g, kk.at()), 0, kk.at()),
                           kk.lor(kk.eq(gval(kk), kk.nul(SP)), kk.b.atom(AtomKind::OwnsBlock, gval(kk), -1, kk.at())));
        };
        Function* f {k.fn("f", k.vd)};
        k.req(f, k.land(deep(k), k.ne(gval(k), k.nul(SP))));
        k.ens(f, deep(k));
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body;
        if (mode != 2)
        {
            body.push_back(k.es(k.call(fr, {gval(k)})));
        }
        const Stmt* st {k.store(k.b.global_addr(g, k.at()), k.call(m, {}))};
        body.push_back(st);
        body.push_back(k.incS(i));
        const Stmt* loop_body {k.blkc(body)};
        std::vector<const Stmt*> fill {k.store(k.mem(gval(k), 0), N(k, "0"))};
        for (const char* ix : {"0", "1", "2", "3"})
        {
            fill.push_back(k.store(k.b.elem_addr(k.mem(gval(k), 1), N(k, ix), k.at()), N(k, "0")));
        }
        f->body = k.blk({k.decl(i, N(k, "0")),
                         k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                               {k.cl(k.sub(N(k, "3"), k.v(i)))}, loop_body),
                         k.iff(k.ne(gval(k), k.nul(SP)), k.blk(fill))});
        k.done(f);
        return mode == 2 ? st->loc : loop_body->end_loc;
    };
    lcv("e1e2d_loop_owner_global_reassigned",
        "a loop frees the owning global and takes a calloc'ed block each iteration (g = calloc()): the written site's pointer is read through a cell the loop writes, and its array part is held at the pointer the body ends with (verifies)",
        [=](Kit& k)
        {
            owner_global_reassigned(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_loop_owner_global_reassigned_malloc_fail",
       "the same with malloc: the new block's elements have no value, and the iteration ends with g->a having none though it had one when the loop began",
       [=](Kit& k)
       {
           k.fail("uninitialized", owner_global_reassigned(k, 1), 2U);
       });
    lc("e1e2d_loop_owner_global_reassigned_leak_fail",
       "the same without the free: the store drops the block g holds (leak at the store)",
       [=](Kit& k)
       {
           k.fail("leak", owner_global_reassigned(k, 2));
       });
    // struct SA { int n; int a[4]; }; struct R { struct SA *q [[owning]]; int z; };
    // void f(struct R *r) requires(owns(*r) && (r->q == NULL || owns_block(r->q))) ensures(the same) { if (r->q != NULL) { int i = 0; while (i < 4) { r->q->a[i] = i; i++; [mode 1: free(r->q);] } } }
    const auto owner_member_loop = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        StructType* R {k.strukt("R", {{"q", SP}, {"z", k.i32}})};
        R->members[0U].owning = true;
        Function* fr {free_view(k, k.st(S))};
        Function* f {k.fn("f", k.vd, {{"r", k.ptr(k.st(R))}})};
        const Var* rp {f->param_var(0)};
        const auto atoms = [&]()
        {
            // mode 2: the pointer has no spelling (no text node), and a message names the array part by the member's own name
            const Expr* rq {mode == 2 ? k.leaf(k.F(f, 0), 0) : k.b.text(k.leaf(k.F(f, 0), 0), "r->q")};
            return k.land(k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "r"), 0, k.at()),
                          k.lor(k.eq(k.leaf(k.F(f, 0), 0), k.nul(SP)), k.b.atom(AtomKind::OwnsBlock, rq, -1, k.at())));
        };
        k.req(f, atoms());
        k.ens(f, atoms());
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> body {k.store(k.b.elem_addr(k.mem(k.leaf(k.pn(rp), 0), 1), k.v(i), k.at()), k.v(i)),
                                       k.incS(i)};
        if (mode != 0)
        {
            body.push_back(k.es(k.call(fr, {k.leaf(k.pn(rp), 0)})));
        }
        const Stmt* loop_body {k.blkc(body)};
        f->body = k.blk(
            {k.iff(k.ne(k.leaf(k.pn(rp), 0), k.nul(SP)),
                   k.blk({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), N(k, "4")), {k.cl(k.range(k.v(i), k.i32, "0", "4"))},
                                                      {k.cl(k.sub(N(k, "4"), k.v(i)))}, loop_body)}))});
        k.done(f);
        return loop_body->end_loc;
    };
    lcv("e1e2d_loop_owner_member",
        "a structure with an owning member that points at a structure with an array member: a loop writes r->q->a[i]; the block of r->q is a key place of the loop and its array part a heap array place (verifies)",
        [=](Kit& k)
        {
            owner_member_loop(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_loop_owner_member_give_away_fail",
       "the loop frees r->q at the end of each iteration: the iteration ends without the permission to r->q->a that it began with",
       [=](Kit& k)
       {
           k.fail_only("permission", owner_member_loop(k, 1), 2U, 0U, "the permission to `r->q->a` that it began with");
       });
    lc("e1e2d_loop_owner_member_unnamed_give_away_fail",
       "the same with a pointer that has no spelling: the message names the array part by the member's own name, not an empty or arrow-led text",
       [=](Kit& k)
       {
           k.fail_only("permission", owner_member_loop(k, 2), 2U, 0U, "the permission to `a` that it began with");
       });
    // The owning local's own carrier (a loop that writes it, or one that names it not): the owner's invariants and rows take the block's array parts
    // void f(void) { [[owning]] struct SA *q = NULL; int i = 0; while (i < 3) { [mode 0: free(q);] q = calloc(); if (q != NULL) { q->a[1] = i; } i++; } free(q); }
    const auto owner_written_loop = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        Function* m {part_d_alloc_view(k, "calloc", k.st(S), true)};
        Function* fr {free_view(k, k.st(S))};
        Function* f {k.fn("f", k.vd)};
        Var* q {k.local(f, "q", SP)};
        q->owning = true;
        Var* i {k.local(f, "i", k.i32)};
        const Stmt* st {k.asg(q, k.call(m, {}))};
        std::vector<const Stmt*> body;
        if (mode == 0)
        {
            body.push_back(k.es(k.call(fr, {k.pn(q)})));
        }
        body.push_back(st);
        body.push_back(k.iff(k.ne(k.pn(q), k.nul(SP)),
                             k.blk({k.store(k.b.elem_addr(k.mem(k.pn(q), 1), N(k, "1"), k.at()), k.v(i))})));
        body.push_back(k.incS(i));
        f->body = k.blkc({k.decl(q, k.nul(SP)), k.decl(i, N(k, "0")),
                          k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                {k.cl(k.sub(N(k, "3"), k.v(i)))}, k.blkc(body)),
                          k.es(k.call(fr, {k.pn(q)}))});
        k.done(f);
        return st->loc;
    };
    lcv("e1e2d_loop_owner_written",
        "a loop frees an owning local and takes a new allocation each iteration (q = calloc()), writing q->a[1]: the owner's carrier holds the block's array part (verifies)",
        [=](Kit& k)
        {
            owner_written_loop(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_loop_owner_written_leak_fail",
       "the same without the free before the allocation: the store drops the block it holds (leak at the store)",
       [=](Kit& k)
       {
           k.fail("leak", owner_written_loop(k, 1));
       });
    // void touch(int *x) requires(owns(*x)) ensures(owns(*x)); void f(int *y) requires(owns(*y)) ensures(owns(*y)) { [[owning]] struct SA *q = calloc(); int i = 0; while (i < 3) { touch(y); i++; } [mode 0: free(q);] }
    const auto owner_unmentioned_loop = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        Function* m {part_d_alloc_view(k, "calloc", k.st(S), true)};
        Function* fr {free_view(k, k.st(S))};
        Function* t {k.fn("touch", k.vd, {{"x", k.ptr(k.i32)}})};
        k.trusted(t, "a callee that keeps the cell it borrows");
        k.req(t, k.b.atom(AtomKind::Owns, FP(k, t, 0), 0, k.at()));
        k.ens(t, k.b.atom(AtomKind::Owns, FP(k, t, 0), 0, k.at()));
        k.done(t);
        Function* f {k.fn("f", k.vd, {{"y", k.ptr(k.i32)}})};
        k.req(f, k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()));
        k.ens(f, k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()));
        Var* q {k.local(f, "q", SP)};
        q->owning = true;
        Var* i {k.local(f, "i", k.i32)};
        std::vector<const Stmt*> tail;
        if (mode == 0)
        {
            tail.push_back(k.es(k.call(fr, {k.pn(q)})));
        }
        std::vector<const Stmt*> all {k.decl(q, k.call(m, {})), k.decl(i, N(k, "0")),
                                      k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3"))},
                                            {k.cl(k.sub(N(k, "3"), k.v(i)))},
                                            k.blkc({k.es(k.call(t, {k.pv(f, 0)})), k.incS(i)}))};
        all.insert(all.end(), tail.begin(), tail.end());
        f->body = k.blkc(all);
        k.done(f);
        return f->body->end_loc;
    };
    lcv("e1e2d_loop_owner_unmentioned",
        "a loop that calls a function and never names the owning local: the owner's carrier, guarded by the held ghost, keeps the block and its array part through the loop; the owner is freed after it (verifies)",
        [=](Kit& k)
        {
            owner_unmentioned_loop(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_loop_owner_unmentioned_leak_fail",
       "the same without the free: the function returns owning the allocation (leak)",
       [=](Kit& k)
       {
           k.fail("leak", owner_unmentioned_loop(k, 1));
       });

    // ---- the loop column's written atoms (Part D, step B): `owns(*p)` and `owns(p->a)` in a loop invariant, the array parts of the atom (S48 (k)'s shape) ----
    // struct SA { int n; int a[4]; };
    // void f(struct SA *p, int n) requires(owns(*p) [mode 4: owns(p->a) only]) ensures(owns(*p)) { p = p; int i = 0; while (i < n) invariant(owns(*p) [mode 2: owns(p->a) && owns(p->n)]) { [mode 1: p = NULL;] p->n = i; p->a[1] = i; i++; } }
    // mode 3: the same with a structure that has no cell leaf (struct TA { int a[4]; })
    const auto written_owns_loop = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {mode == 3 ? k.strukt("TA", {{"a", k.prog.array_of(k.i32, 4U)}}) : SA(k)};
        const int arr {mode == 3 ? 0 : 1};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}, {"n", k.i32}})};
        const Var* rp {f->param_var(0)};
        const Var* rn {f->param_var(1)};
        if (mode == 4)
        {
            k.req(f, k.b.atom(AtomKind::Owns, k.mem(k.b.text(k.F(f, 0), "p"), arr), 0, k.at()));
        }
        else
        {
            k.req(f, k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "p"), 0, k.at()));
            if (mode != 1)
            {
                k.ens(f, k.b.atom(AtomKind::Owns, k.b.text(k.F(f, 0), "p"), 1, k.at()));
            }
        }
        k.req(f, k.range(k.F(f, 1), k.i32, "0", "100"));
        Var* i {k.local(f, "i", k.i32)};
        const SrcLoc il {k.at()};
        const Expr* inv {mode == 2 ? k.land(k.b.atom(AtomKind::Owns, k.mem(k.pn(rp), arr), -1, k.at()),
                                            k.b.atom(AtomKind::Owns, k.mem(k.pn(rp), 0), -1, k.at()))
                                   : k.b.atom(AtomKind::Owns, k.pn(rp), -1, il)};
        std::vector<const Stmt*> body;
        if (mode == 1)
        {
            body.push_back(k.asg(rp, k.nul(k.ptr(k.st(S)))));
        }
        else
        {
            if (mode != 3)
            {
                body.push_back(k.store(k.mem(k.pn(rp), 0), k.v(i)));
            }
            body.push_back(k.store(k.b.elem_addr(k.mem(k.pn(rp), arr), N(k, "1"), k.at()), k.v(i)));
        }
        body.push_back(k.incS(i));
        const Stmt* loop_body {k.blkc(body)};
        f->body = k.blkc({k.asg(rp, k.pn(rp)), k.decl(i, N(k, "0")),
                          k.whl(k.lt(k.v(i), k.pn(rn)), {k.cl(k.land(k.range(k.v(i), k.i32, "0", "100"), inv))},
                                {k.cl(k.sub(k.pn(rn), k.v(i)))}, loop_body)});
        k.done(f);
        return mode == 4 ? il : loop_body->end_loc;
    };
    lcv("e1e2d_loop_written_owns_moved",
        "owns(*p) written in a loop invariant on a structure with an array member whose pointer parameter the function assigns: the array part's conjuncts (S48 (k)'s shape), the loop stores p->n and p->a[1] (verifies)",
        [=](Kit& k)
        {
            written_owns_loop(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_loop_written_owns_moved_null_fail",
       "the loop assigns the pointer NULL: the iteration ends without owning the array part its invariant names",
       [=](Kit& k)
       {
           k.fail_only("permission", written_owns_loop(k, 1), 14U, 0U);
       });
    lc("e1e2d_loop_written_owns_entry_missing_fail",
       "the function holds the array member only (owns(p->a)) and the invariant owns(*p): the loop is entered without owning the cell p->n",
       [=](Kit& k)
       {
           k.fail_only("permission", written_owns_loop(k, 4), 14U, 7U);
       });
    lcv("e1e2d_loop_written_owns_member",
        "owns(p->a) and owns(p->n) written in the invariant (the array member's own atom has the atom's basic row): the same loop (verifies)",
        [=](Kit& k)
        {
            written_owns_loop(k, 2);
            vac_fn(k);
        });
    lcv("e1e2d_loop_written_owns_cell_less",
        "owns(*p) on a structure whose only member is an array: its liveness-only entry carries the basic row and the liveness amount (verifies)",
        [=](Kit& k)
        {
            written_owns_loop(k, 3);
            vac_fn(k);
        });
}

// ---- Part D's refusals: record 0052 item 2a stays in front of the partnerless gate, and the six positions refuse an aggregate element (question 348) ----
void define_part_d_reject_cases()
{
    auto A4 = [](Kit& k)
    {
        return k.prog.array_of(k.i32, 4U);
    };
    auto SA = [=](Kit& k)
    {
        return k.strukt("SA", {{"n", k.i32}, {"a", A4(k)}});
    };
    // struct P { int x; int y; }; struct T { struct P v[2]; int n; }: an array member of structures
    auto TA = [](Kit& k)
    {
        StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
        return k.strukt("T", {{"v", k.prog.array_of(k.st(P), 2U)}, {"n", k.i32}});
    };
    // ---- question 348: an aggregate element stays rejected at each of item 15's six positions ----
    auto fp = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    const char* const kAgg {"decision 0064 item 15, question 348"};
    rc("e1e2d_rej_aggregate_owns_block", kAgg,
       [=](Kit& k)
       {
           // void f(struct T *p) requires(owns_block(p)); struct T { struct P v[2]; int n; }
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2d_rej_aggregate_marker_on_a_pointer", kAgg,
       [=](Kit& k)
       {
           // void f(struct T *p [[cerastes::must_init]]) requires(owns(*p)) ensures(owns(*p))
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           f->params[0].init = InitMarker::MustInit;
           k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2d_rej_aggregate_array_beside_an_owning_member", kAgg,
       [=](Kit& k)
       {
           // struct R { struct P v[2]; int *q [[cerastes::owning]]; }
           StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
           StructType* R {k.strukt("R", {{"v", k.prog.array_of(k.st(P), 2U)}, {"q", k.ptr(k.i32)}})};
           R->members[1].owning = true;
       });
    rc("e1e2d_rej_owning_member_to_an_aggregate_array_structure", kAgg,
       [=](Kit& k)
       {
           // struct R { struct T *q [[cerastes::owning]]; int z; }
           StructType* T {TA(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(T))}, {"z", k.i32}})};
           R->members[0].owning = true;
       });
    rc("e1e2d_rej_aggregate_owning_global", kAgg,
       [=](Kit& k)
       {
           // struct T *g [[cerastes::owning]]; struct T { struct P v[2]; int n; }
           StructType* T {TA(k)};
           Global* g {k.prog.add_global("g", k.ptr(k.st(T)), k.at())};
           g->init_kind = Global::Init::Null;
           g->owning = true;
           Function* f {k.fn("f", k.vd)};
           f->body = k.blkc({});
           k.done(f);
       });
    rc("e1e2d_rej_aggregate_owning_local", kAgg,
       [=](Kit& k)
       {
           // void f(void) { struct T *q [[cerastes::owning]] = NULL; }
           StructType* T {TA(k)};
           const Type TP {k.ptr(k.st(T))};
           Function* f {k.fn("f", k.vd)};
           Var* q {k.local(f, "q", TP)};
           q->owning = true;
           f->body = k.blkc({k.decl(q, k.nul(TP))});
           k.done(f);
       });
    rc("e1e2d_rej_aggregate_owning_formal_implicit_atom", kAgg,
       [=](Kit& k)
       {
           // void f(struct T *p [[cerastes::owning]]): the implicit atom owns(*p) of an owning formal
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           const Expr* a {k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at())};
           const_cast<Expr*>(a)->implicit_atom = true;
           k.req(f, a);
           f->body = k.blkc({});
           k.done(f);
       });
    rc("e1e2d_rej_aggregate_atom_through_a_heap_read", kAgg,
       [=](Kit& k)
       {
           // struct R { struct T *q; }; void f(struct R *r) requires(owns(*r->q))
           StructType* T {TA(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(T))}})};
           Function* f {k.fn("f", k.vd, {{"r", k.ptr(k.st(R))}})};
           k.req(f, k.b.atom(AtomKind::Owns, k.deref(k.mem(k.F(f, 0), 0)), 0, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2d_rej_aggregate_element_read_through_a_heap_read", kAgg,
       [=](Kit& k)
       {
           // struct R { struct T *q; }; void f(struct R *r) requires(r->q->v[1].x == 0)
           StructType* T {TA(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(T))}})};
           Function* f {k.fn("f", k.vd, {{"r", k.ptr(k.st(R))}})};
           const Expr* loaded {k.deref(k.mem(k.F(f, 0), 0))};
           const Expr* el {k.b.elem_addr(k.mem(loaded, 0), k.num(k.i32, "1"), k.at())};
           k.req(f, k.eq(k.deref(k.mem(el, 0)), k.num(k.i32, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    rc("e1e2d_rej_aggregate_partnerless_result", kAgg,
       [=](Kit& k)
       {
           // struct T *f(void) ensures(owns(*result)) (no body)
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.ptr(k.st(T)))};
           k.trusted(f);
           k.ens(f, k.b.atom(AtomKind::Owns, k.R(f), 0, k.at()));
           k.done(f);
       });
    rc("e1e2d_rej_aggregate_ref_to_uninit_local", kAgg,
       [=](Kit& k)
       {
           // struct T *p [[cerastes::ref_to_uninit]] = null;
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.vd)};
           Var* p {k.local(f, "p", k.ptr(k.st(T)))};
           p->ref_to_uninit = true;
           f->body = k.blk({k.decl(p, k.nul(k.ptr(k.st(T))))});
           k.done(f);
       });
    // ---- the lowering's own stop at each Part D site that takes an aggregate element (the harness cannot pin an exit 3, so each is a refused program lowered anyway) ----
    ru("e1e2d_ru_block_lines_of_an_aggregate_array_structure", "the lines of an owns_block",
       [=](Kit& k)
       {
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           f->body = k.blk({});
           k.done(f);
       });
    ru("e1e2d_ru_block_rows_of_an_aggregate_array_structure", "the rows of a block",
       [=](Kit& k)
       {
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           k.ens(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           f->body = k.blkc({});
           k.done(f);
       });
    ru("e1e2d_ru_block_framing_of_an_aggregate_array_structure", "the framing of a contract read",
       [=](Kit& k)
       {
           // void f(struct T *p) requires(owns_block(p) && p->v[0].x == 0)
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           const Expr* el {k.b.elem_addr(k.mem(fp(k, f, 0), 0), k.num(k.i32, "0"), k.at())};
           k.req(f, k.land(k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()),
                           k.eq(k.deref(k.mem(el, 0)), k.num(k.i32, "0"))));
           f->body = k.blk({});
           k.done(f);
       });
    ru("e1e2d_ru_block_call_record_of_an_aggregate_array_structure", "the sequencing record of a call's block site",
       [=](Kit& k)
       {
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           k.req(f, k.b.atom(AtomKind::OwnsBlock, fp(k, f, 0), -1, k.at()));
           f->body = k.blk({});
           k.done(f);
           Function* g {k.fn("g", k.vd, {{"q", k.ptr(k.st(T))}})};
           k.req(g, k.b.atom(AtomKind::OwnsBlock, fp(k, g, 0), -1, k.at()));
           g->body = k.blk({k.es(k.call(f, {k.pv(g, 0)}))});
           k.done(g);
       });
    ru("e1e2d_ru_marked_atom_array_part_of_an_aggregate", "the array part of a marked or partnerless atom",
       [=](Kit& k)
       {
           // void f(struct T *p [[cerastes::ref_to_uninit]]) requires(owns(*p)) ensures(owns(*p)) {}
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           f->params[0].init = InitMarker::RefToUninit;
           k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           f->body = k.blkc({});
           k.done(f);
       });
    ru("e1e2d_ru_marked_return_rows_of_an_aggregate", "the return rows of a marked or partnerless atom",
       [=](Kit& k)
       {
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           f->params[0].init = InitMarker::RefToUninit;
           k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           f->body = k.blkc({});
           k.done(f);
       });
    ru("e1e2d_ru_marked_call_rows_of_an_aggregate", "the call rows of an atom under an initialization marker",
       [=](Kit& k)
       {
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(T))}})};
           f->params[0].init = InitMarker::RefToUninit;
           k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
           f->body = k.blkc({});
           k.done(f);
           Function* g {k.fn("g", k.vd, {{"q", k.ptr(k.st(T))}})};
           k.req(g, k.b.atom(AtomKind::Owns, fp(k, g, 0), 0, k.at()));
           k.ens(g, k.b.atom(AtomKind::Owns, fp(k, g, 0), 0, k.at()));
           g->body = k.blkc({k.es(k.call(f, {k.pv(g, 0)}))});
           k.done(g);
       });
    ru("e1e2d_ru_partnerless_array_part_of_an_aggregate", "the array part of a marked or partnerless atom",
       [=](Kit& k)
       {
           // struct T *f(void) ensures(owns(*result)) (no body)
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.ptr(k.st(T)))};
           k.trusted(f);
           k.ens(f, k.b.atom(AtomKind::Owns, k.R(f), 0, k.at()));
           k.done(f);
       });
    ru("e1e2d_ru_partnerless_return_rows_of_an_aggregate", "the return rows of a marked or partnerless atom",
       [=](Kit& k)
       {
           // struct T *f(void) ensures(owns(*result)) { return null; }
           StructType* T {TA(k)};
           Function* f {k.fn("f", k.ptr(k.st(T)))};
           k.ens(f, k.b.atom(AtomKind::Owns, k.R(f), 0, k.at()));
           f->body = k.blk({k.ret(k.nul(k.ptr(k.st(T))))});
           k.done(f);
       });
    ru("e1e2d_ru_contract_read_through_a_heap_read_of_an_aggregate",
       "the element of a pointer to structures in a contract",
       [=](Kit& k)
       {
           // struct R { struct T *q; }; void f(struct R *r) requires(r->q->v[1].x == 0)
           StructType* T {TA(k)};
           StructType* R {k.strukt("R", {{"q", k.ptr(k.st(T))}})};
           Function* f {k.fn("f", k.vd, {{"r", k.ptr(k.st(R))}})};
           const Expr* loaded {k.deref(k.mem(k.F(f, 0), 0))};
           const Expr* el {k.b.elem_addr(k.mem(loaded, 0), k.num(k.i32, "1"), k.at())};
           k.req(f, k.eq(k.deref(k.mem(el, 0)), k.num(k.i32, "0")));
           f->body = k.blk({});
           k.done(f);
       });
    // a written `reads` atom in a loop invariant on a structure with an array member, or on an array member (decision 0064 item 15, Part D step B): S48 (k) covers `owns` only, so the checker
    // refuses it and lowering stops (never a dropped array leaf); `owns` of a pointer variable is built (the cases e1e2d_loop_written_*)
    const auto written = [=](Kit& k, bool member_array, AtomKind kind, bool aggregate = false)
    {
        StructType* S {aggregate ? TA(k) : SA(k)};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}})};
        k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
        k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* atom {member_array ? k.b.atom(kind, k.mem(k.pv(f, 0), 1), 1, k.at())
                                       : k.b.atom(kind, k.pv(f, 0), 0, k.at())};
        const Stmt* lp {k.whl(k.lt(k.v(i), k.num(k.i32, "4")), {k.cl(k.land(k.range(k.v(i), k.i32, "0", "4"), atom))},
                              {k.cl(k.sub(k.num(k.i32, "4"), k.v(i)))},
                              k.blkc({k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))}))};
        f->body = k.blkc({k.decl(i, k.num(k.i32, "0")), lp});
        k.done(f);
    };
    rc("e1e2d_rej_written_reads_structure_atom_with_an_array_member",
       "a `reads` atom on a structure with an array member in a loop invariant",
       [=](Kit& k)
       {
           written(k, false, AtomKind::Reads);
       });
    rc("e1e2d_rej_written_reads_member_array_atom", "a `reads` atom on an array member in a loop invariant",
       [=](Kit& k)
       {
           written(k, true, AtomKind::Reads);
       });
    rc("e1e2d_rej_written_owns_aggregate_array_structure",
       "a permission atom on a structure with an array member of structures or arrays in a loop invariant",
       [=](Kit& k)
       {
           written(k, false, AtomKind::Owns, true);
       });
    // an `owns(*r->q)` written in a loop invariant: the pointer is read through memory (the loop column builds a pointer variable only)
    const auto written_load = [=](Kit& k)
    {
        StructType* S {SA(k)};
        StructType* R {k.strukt("R", {{"q", k.ptr(k.st(S))}, {"z", k.i32}})};
        Function* f {k.fn("f", k.vd, {{"r", k.ptr(k.st(R))}})};
        const Var* rp {f->param_var(0)};
        Var* i {k.local(f, "i", k.i32)};
        const Expr* atom {k.b.atom(AtomKind::Owns, k.leaf(k.pn(rp), 0), 0, k.at())};
        const Stmt* lp {k.whl(k.lt(k.v(i), k.num(k.i32, "4")), {k.cl(k.land(k.range(k.v(i), k.i32, "0", "4"), atom))},
                              {k.cl(k.sub(k.num(k.i32, "4"), k.v(i)))},
                              k.blkc({k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))}))};
        f->body = k.blkc({k.decl(i, k.num(k.i32, "0")), lp});
        k.done(f);
    };
    rc("e1e2d_rej_written_owns_through_a_load",
       "a written `owns` atom on a structure with an array member reached other than directly from a pointer variable",
       [=](Kit& k)
       {
           written_load(k);
       });
    ru("e1e2d_ru_written_owns_through_a_load",
       "a written atom on a structure with an array member, or on an array member",
       [=](Kit& k)
       {
           written_load(k);
       });
    ru("e1e2d_ru_written_reads_structure_atom_with_an_array_member",
       "a written atom on a structure with an array member, or on an array member",
       [=](Kit& k)
       {
           written(k, false, AtomKind::Reads);
       });
    ru("e1e2d_ru_written_reads_member_array_atom",
       "a written atom on a structure with an array member, or on an array member",
       [=](Kit& k)
       {
           written(k, true, AtomKind::Reads);
       });
    ru("e1e2d_ru_written_owns_aggregate_array_structure",
       "a written atom on a structure with an array member, or on an array member",
       [=](Kit& k)
       {
           written(k, false, AtomKind::Owns, true);
       });
    // ---- audit round D2's three fixes (record 0064 item 15) ----
    // Finding 2: a written `owns` atom with an array part under a guard that reads memory.  void f(struct SA *p, struct R *r) { ... while (i < 4)
    // invariant(0 <= i && i <= 4 && (r->z > 0 ? owns(*p) : true)) ... }, or owns(p->a) for the member form
    const auto written_guard = [=](Kit& k, bool member_array)
    {
        StructType* S {SA(k)};
        StructType* R {k.strukt("R", {{"z", k.i32}})};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(S))}, {"r", k.ptr(k.st(R))}})};
        const Var* rp {f->param_var(0)};
        const Var* rr {f->param_var(1)};
        k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
        k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 1), 1, k.at()));
        k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* atom {member_array ? k.b.atom(AtomKind::Owns, k.mem(k.pn(rp), 1), -1, k.at())
                                       : k.b.atom(AtomKind::Owns, k.pn(rp), -1, k.at())};
        const Expr* guarded {k.cond(k.gt(k.leaf(k.pn(rr), 0), k.num(k.i32, "0")), atom, k.yes())};
        const Stmt* lp {
            k.whl(k.lt(k.v(i), k.num(k.i32, "4")), {k.cl(k.land(k.range(k.v(i), k.i32, "0", "4"), guarded))},
                  {k.cl(k.sub(k.num(k.i32, "4"), k.v(i)))}, k.blkc({k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))}))};
        f->body = k.blkc({k.decl(i, k.num(k.i32, "0")), lp});
        k.done(f);
    };
    rc("e1e2d_rej_written_owns_structure_under_a_guard_that_reads_memory", "under a guard that reads memory",
       [=](Kit& k)
       {
           written_guard(k, false);
       });
    rc("e1e2d_rej_written_owns_member_array_under_a_guard_that_reads_memory", "under a guard that reads memory",
       [=](Kit& k)
       {
           written_guard(k, true);
       });
    ru("e1e2d_ru_written_owns_structure_under_a_guard_that_reads_memory",
       "a guard that reads memory reached the array part of a written atom",
       [=](Kit& k)
       {
           written_guard(k, false);
       });
    ru("e1e2d_ru_written_owns_member_array_under_a_guard_that_reads_memory",
       "a guard that reads memory reached the array part of a written atom",
       [=](Kit& k)
       {
           written_guard(k, true);
       });
    // Findings 3 and the item 10 gaps: a written `owns` of an array member in a loop invariant, three shapes beside the ones above.  `kind` 0: the member is an array of structures
    // (check.cpp's aggregate member form); 1: a scalar array member of a structure that also has an array member of structures (struct SB { int a[4]; struct P v[2]; }, finding 3);
    // 2: a scalar array member reached through a load (`&r->q->a`, the member form reached indirectly)
    const auto written_member = [=](Kit& k, int kind)
    {
        StructType* P {k.strukt("P", {{"x", k.i32}, {"y", k.i32}})};
        StructType* S {kind == 0   ? TA(k)
                       : kind == 1 ? k.strukt("SB", {{"a", A4(k)}, {"v", k.prog.array_of(k.st(P), 2U)}})
                                   : SA(k)};
        StructType* R {k.strukt("R", {{"q", k.ptr(k.st(S))}, {"z", k.i32}})};
        Function* f {k.fn("f", k.vd, {{"p", k.ptr(k.st(kind == 2 ? R : S))}})};
        const Var* rp {f->param_var(0)};
        k.req(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
        k.ens(f, k.b.atom(AtomKind::Owns, fp(k, f, 0), 0, k.at()));
        Var* i {k.local(f, "i", k.i32)};
        const Expr* atom {kind == 2 ? k.b.atom(AtomKind::Owns, k.mem(k.leaf(k.pn(rp), 0), 1), -1, k.at())
                                    : k.b.atom(AtomKind::Owns, k.mem(k.pn(rp), 0), -1, k.at())};
        const Stmt* lp {k.whl(k.lt(k.v(i), k.num(k.i32, "4")), {k.cl(k.land(k.range(k.v(i), k.i32, "0", "4"), atom))},
                              {k.cl(k.sub(k.num(k.i32, "4"), k.v(i)))},
                              k.blkc({k.asg(i, k.add(k.v(i), k.num(k.i32, "1")))}))};
        f->body = k.blkc({k.decl(i, k.num(k.i32, "0")), lp});
        k.done(f);
    };
    rc("e1e2d_rej_written_owns_aggregate_member_array",
       "a permission atom on an array member of structures or arrays in a loop invariant",
       [=](Kit& k)
       {
           written_member(k, 0);
       });
    rc("e1e2d_rej_written_owns_scalar_member_array_of_an_aggregate_structure",
       "a permission atom on an array member of structures or arrays in a loop invariant",
       [=](Kit& k)
       {
           written_member(k, 1);
       });
    rc("e1e2d_rej_written_owns_member_array_through_a_load",
       "a written `owns` atom on an array member reached other than directly from a pointer variable",
       [=](Kit& k)
       {
           written_member(k, 2);
       });
    // void give(struct SA *p) ensures(owns(p->a));  (no body): 0052 item 2a, a partnerless atom on a formal of a function without a body
    rc("e1e2d_rej_bodiless_partnerless_owns_member_array_of_a_formal",
       "an `ensures` atom without a `requires` atom on something other than the result in a function without a body",
       [=](Kit& k)
       {
           StructType* S {SA(k)};
           Function* f {k.fn("give", k.vd, {{"p", k.ptr(k.st(S))}})};
           k.ens(f, k.b.atom(AtomKind::Owns, k.mem(k.F(f, 0), 1), 0, k.at()));
           k.done(f);
       });
    // void give(struct SA *p) ensures(owns(*p));  (no body)
    rc("e1e2d_rej_bodiless_partnerless_owns_structure_of_a_formal",
       "an `ensures` atom without a `requires` atom on something other than the result in a function without a body",
       [=](Kit& k)
       {
           StructType* S {SA(k)};
           Function* f {k.fn("give", k.vd, {{"p", k.ptr(k.st(S))}})};
           k.ens(f, k.b.atom(AtomKind::Owns, k.F(f, 0), 0, k.at()));
           k.done(f);
       });
}

// ---- Part D, markers and partnerless atoms on a structure with an array member of scalars (item 15; S48 (i), (l)) ----
void define_part_d_marker_cases()
{
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto FP = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto lcv = [](const std::string& name, const std::string& doc, const std::function<void(Kit&)>& build)
    {
        lc(name, doc, build);
        lc(name + "_vacuity",
           "the same with a function that ends in check(false): it fails (what the contract assumes is not inconsistent)",
           [build](Kit& k)
           {
               k.vac_tail = true;
               build(k);
               k.fail("check", k.vac_loc);
           });
    };
    auto vac_fn = [=](Kit& k)
    {
        if (k.vac_tail)
        {
            Function* g {k.fn("vac", k.vd)};
            const Stmt* v {k.chk(k.no())};
            g->body = k.blk({v});
            k.done(g);
            k.vac_loc = v->loc;
        }
    };
    auto SA = [](Kit& k)
    {
        return k.strukt("SA", {{"n", k.i32}, {"a", k.prog.array_of(k.i32, 4U)}});
    };
    auto elem_place = [](Kit& k, const Expr* base, const char* i)
    {
        return k.b.elem_addr(k.mem(base, 1), k.num(k.i32, i), k.at());
    };
    // callee `wr` ([ref_to_uninit] | [must_init]) requires(owns(*p)) ensures(owns(*p)) { p->n = 1; p->a[1] = 7; [p->a[0], p->a[2], p->a[3]] }, caller `use`
    // mode 0 (ref_to_uninit): the caller writes n and a[0..3], calls wr (writes a[1]), reads a[2] and n: the kept flags hold
    // mode 1 (ref_to_uninit): a[2] unwritten by the caller, read after the call: uninitialized at the read
    // mode 2 (must_init): the caller writes nothing, the filler writes everything, the caller reads a[2] and n
    // mode 3 (must_init): the filler misses a[3]: postcondition at the callee's closing brace
    const auto marker = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        const bool must {mode >= 2};
        Function* w {k.fn("wr", k.vd, {{"p", SP}})};
        w->params[0].init = must ? InitMarker::MustInit : InitMarker::RefToUninit;
        k.req(w, k.b.atom(AtomKind::Owns, FP(k, w, 0), 0, k.at()));
        k.ens(w, k.b.atom(AtomKind::Owns, FP(k, w, 0), 0, k.at()));
        std::vector<const Stmt*> wb {k.lstore(k.pv(w, 0), 0, N(k, "1"))};
        for (const char* i : {"0", "1", "2", "3"})
        {
            if (!must && std::string(i) != "1")
            {
                continue;
            }
            if (mode == 3 && std::string(i) == "3")
            {
                continue;
            }
            wb.push_back(k.store(elem_place(k, k.pv(w, 0), i), N(k, "7")));
        }
        w->body = k.blkc(wb);
        k.done(w);
        Function* f {k.fn("use", k.vd)};
        Var* s {k.scell(f, "s", S)};
        Var* x {k.local(f, "x", k.i32)};
        Var* y {k.local(f, "y", k.i32)};
        std::vector<const Stmt*> body {k.decl(s, nullptr)};
        if (!must)
        {
            body.push_back(k.lstore(k.addr(s), 0, N(k, "0")));
            for (const char* i : {"0", "1", "2", "3"})
            {
                if (mode == 1 && std::string(i) == "2")
                {
                    continue;
                }
                body.push_back(k.store(elem_place(k, k.addr(s), i), N(k, "0")));
            }
        }
        body.push_back(k.es(k.call(w, {k.addr(s)})));
        const Expr* rd {k.deref(elem_place(k, k.addr(s), "2"))};
        body.push_back(k.decl(x, rd));
        body.push_back(k.decl(y, k.leaf(k.addr(s), 0)));
        f->body = k.blkc(body);
        k.done(f);
        return mode == 1 ? rd->loc : w->end_loc;
    };
    lcv("e1e2d_marker_ref_to_uninit_array_member",
        "wr(p [ref_to_uninit]) requires and ensures owns(*p) of a structure with an array member: the requires claims no flag, the ensures keeps a set one (one quantifier for the array leaf); the caller reads s.a[2] and s.n after the call (verifies)",
        [=](Kit& k)
        {
            marker(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_marker_ref_to_uninit_array_member_unwritten_fail",
       "the caller never writes s.a[2] and reads it after the call: the callee kept only the flags that were set (uninitialized at the read)",
       [=](Kit& k)
       {
           k.fail("uninitialized", marker(k, 1));
       });
    lcv("e1e2d_marker_must_init_array_member",
        "fill(p [must_init]) writes n and every element: the ensures states every flag of the array leaf; the caller starts with no value and reads s.a[2] and s.n (verifies)",
        [=](Kit& k)
        {
            marker(k, 2);
            vac_fn(k);
        });
    lc("e1e2d_marker_must_init_array_member_misses_fail",
       "the filler misses a[3]: the return row `forall k :: flag` of the array leaf fails (postcondition at the closing brace)",
       [=](Kit& k)
       {
           k.fail_only("postcondition", marker(k, 3), 2U, 1U);
       });
    // struct SA *fresh(void) ensures(owns(*result)) (no body, trusted); struct SA *wrap(void) ensures(owns(*result)) { return fresh(); | return null; }
    // void use(void) { struct SA *p = wrap(); if (p != null) { p->n = 2; p->a[1] = 3; check(p->a[1] == [3|4]); } }
    const auto partnerless = [=](Kit& k, int mode) -> SrcLoc
    {
        StructType* S {SA(k)};
        const Type SP {k.ptr(k.st(S))};
        Function* fr {k.fn("fresh", SP)};
        k.trusted(fr);
        k.ens(fr, k.b.atom(AtomKind::Owns, k.R(fr), 0, k.at()));
        k.done(fr);
        Function* wr {k.fn("wrap", SP)};
        k.ens(wr, k.b.atom(AtomKind::Owns, k.R(wr), 0, k.at()));
        const Stmt* rt {k.ret(mode == 2 ? k.nul(SP) : k.call(fr, {}))};
        wr->body = k.blk({rt});
        k.done(wr);
        Function* f {k.fn("use", k.vd)};
        Var* p {k.local(f, "p", SP)};
        const Stmt* c {k.chk(k.eq_c(k.deref(elem_place(k, k.pn(p), "1")), k.i32, mode == 1 ? "4" : "3"))};
        f->body = k.blk({k.decl(p, k.call(wr, {})),
                         k.iff(k.ne(k.pn(p), k.nul(SP)), k.blk({k.lstore(k.pn(p), 0, N(k, "2")),
                                                                k.store(elem_place(k, k.pn(p), "1"), N(k, "3")), c}))});
        k.done(f);
        return mode == 1 ? c->loc : rt->loc;
    };
    lcv("e1e2d_partnerless_array_member",
        "wrap() returns what fresh() gave it: an ensures atom owns(*result) with no requires partner on a structure with an array member; the caller writes through the result and checks a written element (verifies)",
        [=](Kit& k)
        {
            partnerless(k, 0);
            vac_fn(k);
        });
    lc("e1e2d_partnerless_array_member_wrong_check_fail", "check(p->a[1] == 4) after storing 3: the check fails",
       [=](Kit& k)
       {
           k.fail("check", partnerless(k, 1));
       });
    lc("e1e2d_partnerless_array_member_returns_null_fail",
       "wrap() returns NULL under ensures(owns(*result)): the return does not hold the atom (permission at the return)",
       [=](Kit& k)
       {
           k.fail_only("permission", partnerless(k, 2), 11U, 0U);
       });
}

} // namespace
