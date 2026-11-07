// expect-output: b
// expect-output: 2
// expect-output: 1
// expect-output: q
// expect-output: z
// expect-output: y
[struct]P
|   char v = 'q'
|   i8 pad = 1

[char]first()
|   return 'z'

[char]id(char: x)
|   return x

[i32]main()
|   array<char> a = ['a','b']
|   echo(cstr(a[1]))
|   echo(cstr(len(a)))
|   map<char,i32> counts = {}
|   for c in "aab"
|   |   if has(counts, c)
|   |   |   counts[c] = counts[c] + 1
|   |   else
|   |   |   counts[c] = 1
|   echo(cstr(counts['b']))
|   P p = P()
|   echo(cstr(p.v))
|   echo(cstr(first()))
|   echo(cstr(id('y')))
|   return 0
