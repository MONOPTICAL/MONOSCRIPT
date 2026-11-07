// expect-output: 3
// expect-output: expected a decimal integer
// expect-output: 17
// expect-output: 1
// expect-output: skip
// expect-output: 2
// expect-output: 1
[i32]read(string: s)
|   n ^= toInt(s)? else: echo(error.error); return 17
|   return n
[void]emptyReturn()
|   n ^= toInt("bad")? else: return
|   echo("unreachable")
[i32]main()
|   echo("" + read("3"))
|   echo("" + read("bad"))
|   emptyReturn()
|   array<string> inputs = ["1", "bad", "2"]
|   for s in inputs
|   |   n ^= toInt(s)? else: echo("skip"); continue
|   |   echo("" + n)
|   for s in inputs
|   |   n ^= toInt(s)? else: break
|   |   echo("" + n)
|   return 0
