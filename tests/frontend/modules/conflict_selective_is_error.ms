// expect-error: conflicts
use
|> lib2 -> helper

[i32]helper()
|   return 100

[i32]main()
|   return helper()
