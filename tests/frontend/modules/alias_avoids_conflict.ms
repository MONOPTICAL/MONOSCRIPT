// expect: ok
use
|> lib2 -> helper : libHelper

[i32]helper()
|   return 100

[i32]main()
|   return helper() + libHelper()
