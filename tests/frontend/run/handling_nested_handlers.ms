// expect-output: inner
// expect-output: 91
// expect-output: outer
// expect-output: resumed
[result<i32>]fail()
|   return err(91, "outer")
[i32]main()
|   attempt
|   |   attempt
|   |   |   n ^= toInt("bad")?
|   |   else error
|   |   |   echo("inner")
|   |   |   n ^= fail()?
|   |   panic info
|   |   |   echo("wrong handler")
|   else error
|   |   echo("" + error.code)
|   |   echo(error.error)
|   echo("resumed")
|   return 0
