// expect: golden
// Annotation-free copy of tests/cxx/templates/function_template_general_contract.cpp from
// verifast branch generic2, commit 0fb63506a51cae559ae92348eb54ba7f7be0bf6d.
// Whole-line //@ annotations are replaced by empty lines and trailing
// //~ should_fail markers are removed; no other change.
// Line offset: line N here is line N-6 in the original (header is 6 lines).
struct ShortPoint
{
  short m_x;
  short m_y;
  
  ShortPoint(short x, short y) : m_x(x), m_y(y)


  {}
  
  ~ShortPoint()


  {}
};

struct IntPoint
{
  int m_x;
  int m_y;
  
  IntPoint(int x, int y) : m_x(x), m_y(y)


  {}
  
  ~IntPoint()


  {}
};

template<class T>
void swap(T *a, T *b)


{
  T temp = *a;
  *a = *b;
  *b = temp;
}

template<class T>
void mirror(T *t)


{
  swap(&t->m_x, &t->m_y);
}

int main()


{
  ShortPoint sp(0, 1);
  IntPoint ip(2, 3);
  mirror(&sp);
  mirror(&ip);
}
