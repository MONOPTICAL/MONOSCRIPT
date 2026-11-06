// expect-output: called
// expect-output: 1
// expect-output: done
// Литерал раньше генерировался дважды: функция вызывалась два раза
i32 count = 0

[i32]f()
|   echo("called")
|   count = count + 1
|   return count

[i32]main()
|   array<i32> a = [f()]
|   echo(toString_int(count))
|   echo("done")
|   return 0
