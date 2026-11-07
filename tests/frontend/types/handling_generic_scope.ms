// expect-error: Postfix '?' requires a function returning result<T>
[i32]bad<T>(T: x)
|   n ^= toInt("3")?
|   return n
[i32]main()
|   attempt
|   |   n ^= bad(1)
|   else error
|   |   return 1
|   return 0
