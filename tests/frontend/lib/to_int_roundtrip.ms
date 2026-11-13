// expect-output: 2147483647
// expect-output: -2147483648
[i32]main()
|   result<i32> high = toInt("2147483647")
|   result<i32> low = toInt("-2147483648")
|   echo(toString_int(high.value))
|   echo(toString_int(low.value))
|   return 0
