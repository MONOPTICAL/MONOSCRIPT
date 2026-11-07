// expect-output: 41
// expect-output: 42
[i32]has(i32: a)
|   return 41

[i32]del(i32: a)
|   return 42

[i32]main()
|   echo(toString_int(has(1)))
|   echo(toString_int(del(1)))
|   return 0
