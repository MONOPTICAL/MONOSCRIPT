// expect-output: x
// expect-output: y
[i32]main()
|   a ^= 'x'
|   echo(cstr(a))
|   b ^= ['y']
|   echo(cstr(b[0]))
|   return 0
