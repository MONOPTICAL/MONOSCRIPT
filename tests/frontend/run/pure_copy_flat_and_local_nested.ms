// expect-output: 1
// expect-output: 99
[struct]Box
|   array<i32> values = [1, 2]
[i32]change(array<i32>: original) @pure
|   array<i32> local = copy(original)
|   local[0] = 99
|   Box box = Box()
|   Box other = copy(box)
|   other.values[0] = local[0]
|   return box.values[0]
[i32]main()
|   array<i32> a = [1]
|   i32 answer = change(a)
|   echo("" + a[0])
|   echo("" + answer)
|   return 0
