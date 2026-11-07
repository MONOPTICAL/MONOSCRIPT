// expect-output: 0
// expect-output: 3
// expect-output: 0
// expect-output: -1
// expect-output: -1
// expect-output: 1
[i32]main()
|   echo(cstr(strfind("hello", "h")))
|   echo(cstr(strfind("hello", "lo")))
|   echo(cstr(strfind("hello", "")))
|   echo(cstr(strfind("", "a")))
|   echo(cstr(strfind("hi", "hii")))
|   echo(cstr(strfind("abab", "ba")))
|   return 0
