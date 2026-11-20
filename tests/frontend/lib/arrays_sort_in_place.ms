// expect-output: 1
// expect-output: 1
// expect-output: 9
use
|> mono.core.arrays

// sort меняет переданный массив, а sorted возвращает копию
[i32]main()
|   array<i32> a = [2, 1]
|   arrays.sort(a)
|   echo(cstr(a[0]))
|   b ^= arrays.sorted(a)
|   b[0] = 9
|   echo(cstr(a[0]))
|   echo(cstr(b[0]))
|   return 0
