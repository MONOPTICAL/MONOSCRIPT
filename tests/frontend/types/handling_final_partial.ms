// expect-error: Final variable 'n' is already assigned
[i32]main()
|   final i32 n
|   attempt
|   |   n = 5
|   |   v ^= toInt("bad")?
|   else error
|   |   n = 6
|   return n
