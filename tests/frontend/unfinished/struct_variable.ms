// expect: ok
// xfail: Identifier Identifier не считается объявлением, Parser.cpp:149
[struct]P
|   i32 age = 1

[i32]main()
|   P p = P()
|   return p.age
