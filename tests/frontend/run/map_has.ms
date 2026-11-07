// expect-output: true
// expect-output: false
// expect-output: yes
[i32]main()
|   map<string,i32> m = {"a": 0}
|   echo(toString_bool(has(m, "a")))
|   echo(toString_bool(has(m, "b")))
|   if has(m, "a")
|   |   echo("yes")
|   return 0
