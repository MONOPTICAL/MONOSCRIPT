// expect-output: y
// expect-output: 8
// expect-output: 5
// expect-output: 6
[array<Q>]mapped<T, Q>(array<T>: arr, func: f(T: x) -> Q)
|   array<Q> out = []
|   for v in arr
|   |   push(out, f(v))
|   return out

i32 counter = 5

[i32]main()
|   i32 k = 3
|   [char]toChar(string: s)
|   |   return s[0]->char
|   [i32]twice(i32: x)
|   |   i32 k = x * 2
|   |   return k
|   [i32]bump(i32: x)
|   |   counter = counter + x
|   |   return counter
|   array<string> names = ["y"]
|   chars ^= mapped(names, toChar)
|   echo(cstr(chars[0]))
|   array<i32> nums = [4]
|   doubled ^= mapped(nums, twice)
|   echo(cstr(doubled[0]))
|   echo(cstr(counter))
|   bumped ^= mapped([1], bump)
|   echo(cstr(bumped[0]))
|   return k - 3
