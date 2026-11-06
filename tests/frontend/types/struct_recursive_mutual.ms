// expect-error: cannot be created
[struct]A
|   B b = B()

[struct]B
|   A a = A()

[i32]main()
|   return 0
