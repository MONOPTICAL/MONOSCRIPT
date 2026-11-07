// expect-output: 3
// expect-output: yes
// expect-output: n=2
[struct]A
|   i32 v = 2
|   [i32]get()
|   |   return self.v

[i32]main()
|   A a = A()
|   i32 x = a.get() + 1
|   echo(cstr(x))
|   if a.get() == 2
|   |   echo("yes")
|   echo("n=" + a.get())
|   return 0
