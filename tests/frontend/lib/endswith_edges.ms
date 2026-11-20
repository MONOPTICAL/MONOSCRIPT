// expect-output: true
// expect-output: false
// expect-output: true
// expect-output: true
// expect-output: false
// expect-output: false
// expect-output: true
[i1]hasExtension(string: name) @pure
|   return endsWith(name, ".ms")

[i32]main()
|   echo(cstr(hasExtension("hello.ms")))
|   echo(cstr(endsWith("hello.ms", ".MS")))
|   echo(cstr(endsWith("hello.ms", "")))
|   echo(cstr(endsWith("", "")))
|   echo(cstr(endsWith("", ".ms")))
|   echo(cstr(endsWith("ms", ".ms")))
|   echo(cstr(endsWith("hello.ms", "hello.ms")))
|   return 0
