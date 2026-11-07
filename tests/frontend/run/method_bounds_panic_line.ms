// expect-runtime-error: index 5 out of bounds (len 1) at line 5
[struct]A
|   array<i32> v = [1]
|   [i32]bad()
|   |   return self.v[5]

[i32]main()
|   A a = A()
|   return a.bad()
