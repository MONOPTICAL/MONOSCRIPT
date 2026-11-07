// expect-output: 2
// expect-output: 4
// expect-output: 3
// expect-output: 10
// expect-output: 30
// expect-output: =a!
// filter и map целиком на языке: шаблон, функция как значение и push
[array<T>]filter<T>(array<T>: items, func: keep(T: x) -> i1)
|   array<T> out = []
|   for x in items
|   |   if keep(x)
|   |   |   push(out, x)
|   return out

[array<U>]mapTo<T, U>(array<T>: items, func: f(T: x) -> U)
|   array<U> out = []
|   for x in items
|   |   push(out, f(x))
|   return out

[i1]isEven(i32: x)
|   return x % 2 == 0

[i32]tenTimes(i32: x)
|   return x * 10

[string]shout(string: s)
|   return "=" + s + "!"

[i32]main()
|   array<i32> even = filter([1,2,3,4], isEven)
|   echo(cstr(even[0]))
|   echo(cstr(even[1]))
|   array<i32> tens = mapTo([1,3], tenTimes)
|   echo(cstr(len(even) + 1))
|   echo(cstr(tens[0]))
|   echo(cstr(tens[1]))
|   array<string> s = mapTo(["a"], shout)
|   echo(s[0])
|   return 0
