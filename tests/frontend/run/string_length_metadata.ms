// expect-output: 0
// expect-output: 3
// expect-output: 3
// expect-output: 0
// expect-output: 0
// expect-output: 49
[i32]main()
|   string empty
|   echo("" + len(empty))
|   echo("" + len("abc"))
|   echo("" + len(substr("abcd", 1, 3)))
|   result<i32> good = toInt("1")
|   echo("" + len(good.error))
|   result<i32> bad = toInt("x")
|   string error = bad.error
|   echo("" + len(substr(error, 0, 0)))
|   string digit = toString_int(good.value)
|   echo("" + digit[0]->i32)
|   return 0
