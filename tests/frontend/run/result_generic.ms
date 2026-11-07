// expect-output: 3
// expect-output: empty
[result<T>]firstOf<T>(array<T>: a)
|   if len(a) == 0
|   |   return err("empty")
|   return ok(a[0])

[i32]main()
|   result<i32> r = firstOf([3, 4])
|   echo(cstr(r.value))
|   array<string> nothing = []
|   result<string> e = firstOf(nothing)
|   echo(e.error)
|   return 0
