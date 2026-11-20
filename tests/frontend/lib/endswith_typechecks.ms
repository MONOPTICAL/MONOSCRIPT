// expect: ok
[i1]hasExtension(string: name) @pure
|   return endsWith(name, ".ms")

[i32]main()
|   i1 found = hasExtension("hello.ms")
|   return 0
