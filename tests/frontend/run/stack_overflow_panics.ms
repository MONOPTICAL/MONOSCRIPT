// expect-runtime-error: stack overflow at line 2
[i32]deep(i32: n)
|   return deep(n + 1) + 1
[i32]main()
|   echo("" + deep(0))
|   return 0
