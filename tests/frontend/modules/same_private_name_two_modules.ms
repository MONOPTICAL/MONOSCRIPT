// expect: ok
use
|> lib2 -> usesHidden

[i32]hidden()
|   return 1

[i32]main()
|   return hidden() + usesHidden()
