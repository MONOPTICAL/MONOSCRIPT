// expect-output: substr: range is outside the string
// expect-output: split: empty delimiter
// expect-output: strings cannot contain a NUL byte
// expect-output: strings cannot contain a NUL byte
// expect-output: index 9 out of bounds (len 2)
[i32]main()
|   attempt
|   |   echo(substr("abc", -1, 2))
|   panic info
|   |   echo(info.message)
|   attempt
|   |   parts ^= split("abc", "")
|   panic info
|   |   echo(info.message)
|   attempt
|   |   array<char> chars = ['A', 0->char]
|   |   echo(joinChars(chars))
|   panic info
|   |   echo(info.message)
|   attempt
|   |   echo("" + (0->char))
|   panic info
|   |   echo(info.message)
|   attempt
|   |   string s = "hi"
|   |   echo("" + s[9])
|   panic info
|   |   echo(info.message)
|   return 0
