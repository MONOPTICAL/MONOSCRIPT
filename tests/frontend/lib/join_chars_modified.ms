// expect-output: Test
[i32]main()
|   array<char> x = toArray("test")
|   x[0] = 'T'
|   echo(joinChars(x))
|   return 0
