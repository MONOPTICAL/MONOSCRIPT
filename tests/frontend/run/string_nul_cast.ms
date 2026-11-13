// expect-runtime-error: strings cannot contain a NUL byte
[i32]main()
|   echo("zero=" + 0->char)
|   return 0
