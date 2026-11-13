// expect-error: Pure function cannot call impure method: P.set
[struct]P
|   i32 a = 1
|   [void]set()
|   |   self.a = 2
|   [void]bad() @pure
|   |   self.set()

[i32]main()
|   return 0
