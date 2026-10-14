// Which typedef spellings attach the attribute to the named typedef?
[[cerastes::ty1]] typedef int t1;       // prefix
typedef int t2 [[cerastes::ty2]];       // after the declarator-id
typedef int [[cerastes::ty3]] t3;       // between the type and the name
using t4 [[cerastes::ty4]] = int;       // alias declaration, after the name
using t5 = int [[cerastes::ty5]];       // alias declaration, after the aliased type
