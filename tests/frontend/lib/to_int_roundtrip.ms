// expect-output: 2147483647
// expect-output: -2147483648
[i32]main()
|   echo(toString_int(toInt("2147483647")))
|   echo(toString_int(toInt("-2147483648")))
|   return 0
