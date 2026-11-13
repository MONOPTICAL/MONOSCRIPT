// expect-output: 1
// expect-output: 2
use
|> mono.core.arrays

// sort без result можно вызвать как оператор: массив сортируется на месте
[i32]main()
|   array<i32> a = [2, 1]
|   sort(a)
|   echo(cstr(a[0]))
|   echo(cstr(a[1]))
|   return 0
