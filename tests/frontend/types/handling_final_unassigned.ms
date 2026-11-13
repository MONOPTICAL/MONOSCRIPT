// expect-error: Final variable 'n' is used before it is assigned
[i32]main()
|   final i32 n
|   attempt
|   |   v ^= toInt("bad")?
|   |   n = v
|   else error
|   |   echo(error.error)
|   return n
