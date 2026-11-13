// expect-runtime-error: index 2 out of bounds (len 2) at line 4
[i32]main()
|   array<char> x = toArray("ab")
|   echo(toString_char(x[2]))
|   return 0
