// expect-output: -17
// expect-output: 7
// expect-output: false
// expect-output: false
// expect-output: false
// expect-output: false
// expect-output: false
// expect-output: false
// expect-output: false
// expect-output: false
// expect-output: true
[i32]main()
|   result<i32> r0 = toInt("-17")
|   echo(cstr(r0.value))
|   result<i32> r1 = toInt("+7")
|   echo(cstr(r1.value))
|   result<i32> r2 = toInt("  42")
|   echo(cstr(r2.ok))
|   result<i32> r3 = toInt("12abc")
|   echo(cstr(r3.ok))
|   result<i32> r4 = toInt("abc")
|   echo(cstr(r4.ok))
|   result<i32> r5 = toInt("")
|   echo(cstr(r5.ok))
|   result<i32> r6 = toInt("2147483648")
|   echo(cstr(r6.ok))
|   result<i32> r7 = toInt("-2147483649")
|   echo(cstr(r7.ok))
|   result<i32> r8 = toInt("9999999999999999999999999999999")
|   echo(cstr(r8.ok))
|   result<i32> r9 = toInt("+")
|   echo(cstr(r9.ok))
|   result<i32> r10 = toInt("0")
|   echo(cstr(r10.ok))
|   return 0
