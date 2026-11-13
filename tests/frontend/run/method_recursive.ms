// expect-output: 3
[struct]N
|   i32 v = 1
|   array<N> kids = []
|   [i32]total()
|   |   i32 s = self.v
|   |   for k in self.kids
|   |   |   s = s + k.total()
|   |   return s

[i32]main()
|   N r = N()
|   r.kids = [N(), N()]
|   echo(cstr(r.total()))
|   return 0
