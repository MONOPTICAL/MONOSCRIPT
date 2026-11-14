// expect-output: 2 4
// expect-output: 3 2 1
// expect-output: 1 2 3
// expect-output: 2 3
// expect-output: 0
// expect-output: true false
// expect-output: 1 3
// expect-output: 1 3
// expect-output: 0 1 3 9
// expect-output: 13 0
// expect-output: a b
use
|> mono.core.arrays
|> mono.core.maps

[string]show(array<i32>: a)
|   string out = ""
|   for x in a
|   |   if len(out) > 0: out = out + " "
|   |   out = out + cstr(x)
|   return out

[i1]isEven(i32: x)
|   return x % 2 == 0

[i32]main()
|   array<i32> a = [1, 2, 3, 4]
|   echo(show(arrays.filter(a, isEven)))
|   array<i32> b = [1, 2, 3]
|   arrays.reverse(b)
|   echo(show(b))
|   echo(show(arrays.reversed(b)))
|   echo(show(arrays.slice(a, 1, 3)))
|   echo(cstr(len(arrays.slice(a, 3, 1))))
|   echo(cstr(arrays.contains(a, 4)) + " " + cstr(arrays.contains(a, 9)))
|   array<i32> m = [3, 1, 2]
|   echo(cstr(arrays.min(m)) + " " + cstr(arrays.max(m)))
|   arrays.removeAt(m, 2)
|   echo(show(arrays.sort(m)))
|   arrays.insert(m, 0, 0)
|   arrays.insert(m, 3, 9)
|   echo(show(m))
|   array<i32> empty = []
|   echo(cstr(arrays.sum(m)) + " " + cstr(arrays.sum(empty)))
|   map<string, i32> kv = {"a": 1, "b": 2}
|   array<string> ks = maps.keys(kv)
|   echo(ks[0] + " " + ks[1])
|   return 0
