// expect-output: 1
// expect-output: 9
// expect-output: 9
use
|> mono.core.arrays

// sort меняет переданный массив и возвращает его же, а не копию
[i32]main()
|   array<i32> a = [2, 1]
|   b ^= sort(a)? else a
|   echo(cstr(a[0]))
|   b[0] = 9
|   echo(cstr(a[0]))
|   echo(cstr(b[0]))
|   return 0
