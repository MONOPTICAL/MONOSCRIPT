// expect-output: index -1 out of bounds (len 1)
// expect-output: key not found
// expect-output: key not found
// expect-output: map changed during iteration
// expect-output: pop from an empty array
// expect-output: result has no value: bad
[i32]main()
|   array<i32> a = [1]
|   map<string, i32> m = {"a": 1}
|   attempt
|   |   echo("" + a[-1])
|   panic info
|   |   echo(info.message)
|   attempt
|   |   echo("" + m["missing"])
|   panic info
|   |   echo(info.message)
|   attempt
|   |   del(m, "missing")
|   panic info
|   |   echo(info.message)
|   attempt
|   |   for k in m
|   |   |   m["new"] = 2
|   panic info
|   |   echo(info.message)
|   attempt
|   |   array<i32> empty = []
|   |   echo("" + pop(empty))
|   panic info
|   |   echo(info.message)
|   attempt
|   |   result<i32> r = err("bad")
|   |   echo("" + r.value)
|   panic info
|   |   echo(info.message)
|   return 0
