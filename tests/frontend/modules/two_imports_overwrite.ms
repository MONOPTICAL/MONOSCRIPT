// expect: ok
use
|> lib2
|> lib3

[i32]helper() @overwrite
|   return 100

[i32]main()
|   return helper() + other()
