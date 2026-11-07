// expect-error: Pure function count can only take @pure functions, loud is not
[i1]loud(i32: x)
|   echo("x")
|   return x > 0

[i32]count(array<i32>: a, func: keep(i32: x) -> i1) @pure
|   return 0

[i32]main()
|   return count([1], loud)
