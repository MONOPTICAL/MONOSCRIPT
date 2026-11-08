// expect-runtime-error: result has no value: division by zero at line 6
[result<i32>]divide(i32: a, i32: b)
|   if b == 0: return err(1, "division by zero")
|   return ok(a / b)
[i32]main()
|   echo("" + divide(4, 0).value)
|   return 0
