// expect-error: 5: cannot modify object
[struct]P
|   i32 a = 1
|   [void]set() @pure
|   |   self.a = 2

[i32]main()
|   return 0
