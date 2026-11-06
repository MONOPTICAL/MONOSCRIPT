// expect-output: neg
// expect-output: end
[void]f(i32: a)
|   if a > 0
|   |   return
|   echo("neg")

[i32]main()
|   f(-1)
|   f(1)
|   echo("end")
|   return 0
