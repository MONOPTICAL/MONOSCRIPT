// expect-output: s
// expect-output: 2
// expect-output: 0.5
// expect-output: 7
// expect-output: q
// expect-output: true
[struct]P
|   i32 v = 7

[result<string>]a()
|   return ok("s")

[result<array<i32>>]b()
|   return ok([1, 2])

[result<f64>]c()
|   return ok(0.5)

[result<P>]d()
|   return ok(P())

[result<char>]e()
|   return ok('q')

[result<i1>]f()
|   return ok(1 < 2)

[i32]main()
|   result<string> ra = a()
|   echo(ra.value)
|   result<array<i32>> rb = b()
|   echo(cstr(len(rb.value)))
|   result<f64> rc = c()
|   echo(cstr(rc.value))
|   result<P> rd = d()
|   echo(cstr(rd.value.v))
|   result<char> re = e()
|   echo(cstr(re.value))
|   result<i1> rf = f()
|   echo(cstr(rf.value))
|   return 0
