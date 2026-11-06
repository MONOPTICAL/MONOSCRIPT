// expect: ok
use
|> lib2

[i32]helper() @overwrite
|   return 100

[i32]main()
|   return helper()
