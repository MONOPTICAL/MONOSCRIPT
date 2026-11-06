// expect-error:
// xfail: число кастуется в string на return, Statements.cpp:325
[string]f()
|   return 5

[i32]main()
|   return 0
