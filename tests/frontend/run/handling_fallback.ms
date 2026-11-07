// expect-output: 42
// expect-output: 8080
// expect-output: 1
// expect-output: no
// expect-output: B
// expect-output: false
// expect-output: 9
// expect-output: 17
// expect-output: 3
[result<T>]value<T>(T: x, i1: fail)
|   if fail: return err(7, "bad")
|   return ok(x)
[i32]replacement(array<i32>: calls)
|   calls[0] = calls[0] + 1
|   return 8080
[i32]main()
|   array<i32> calls = [0]
|   port ^= value(42, false)? else replacement(calls)
|   echo("" + port)
|   port = value(42, true)? else replacement(calls)
|   echo("" + port)
|   echo("" + calls[0])
|   echo(value("yes", true)? else "no")
|   echo("" + (value('A', true)? else 'B'))
|   echo("" + (value(true, true)? else false))
|   array<i32> a = value([1, 2], true)? else [9, 8]
|   echo("" + a[0])
|   i64 wide = value(42, true)? else 17
|   echo("" + wide)
|   n ^= value(1, true)? else value(2, true)? else 3
|   echo("" + n)
|   return 0
