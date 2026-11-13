// expect-output: 42
// expect-output: true
// expect-output: false
// expect-output: 17
// expect-output: bad
// expect-output: A
// expect-output: text
// expect-output: 3
// expect-output: true
// expect-output: 21
[result<i32>]number(i1: fail)
|   if fail: return err(17, "bad")
|   return ok(21)
[result<string>]compute(i1: fail)
|   i64 n = number(fail)?
|   return ok("" + (n + number(false)?))
[result<T>]identity<T>(T: value)
|   return ok(value)
[result<i32>]payloads()
|   char c = identity('A')?
|   echo("" + c)
|   string s = identity("text")?
|   echo(s)
|   array<i32> a = identity([1, 2, 3])?
|   echo("" + len(a))
|   echo("" + identity(true)?)
|   n ^= number(false)?
|   echo("" + n)
|   return ok(0)
[i32]main()
|   result<string> a = compute(false)
|   echo(a.value)
|   echo("" + ?a)
|   result<string> b = compute(true)
|   echo("" + ?b)
|   echo("" + b.code)
|   echo(b.error)
|   result<i32> checked = payloads()
|   if !checked.ok: return 1
|   return 0
