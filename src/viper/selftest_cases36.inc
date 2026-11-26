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

} // namespace
