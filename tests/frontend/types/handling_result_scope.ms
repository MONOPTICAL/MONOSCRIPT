// expect-error: Variable not found: n
[i32]main()
|   attempt
|   |   n ^= toInt("3")?
|   else error
|   |   echo("bad")
|   return n
