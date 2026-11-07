// expect-output: 0.1
// expect-output: 0.100000
// expect-output: 0.10000000149011612
// Литерал в f64 точный, а float, расширенный в f64, хранит то, что смог
[i32]main()
|   f64 exact = 0.1
|   echo(toString_f64(exact))
|   float low = 0.1
|   echo(toString_float(low))
|   f64 wide = low
|   echo(toString_f64(wide))
|   return 0
