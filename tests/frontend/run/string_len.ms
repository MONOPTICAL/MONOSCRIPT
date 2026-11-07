// expect-output: 3
// expect-output: 0
// expect-output: 2
// expect-output: 5
// Длина в байтах: кириллица занимает по два
[i32]main()
|   string s = "abc"
|   echo(cstr(len(s)))
|   echo(cstr(len("")))
|   echo(cstr(len("я")))
|   echo(cstr(len("ab" + "cde")))
|   return 0
