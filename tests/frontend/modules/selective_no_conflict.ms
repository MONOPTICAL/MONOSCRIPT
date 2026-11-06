// expect: ok
use
|> lib2 -> usesHelper

[i32]helper()
|   return 100

[i32]main()
|   return helper() + usesHelper()
