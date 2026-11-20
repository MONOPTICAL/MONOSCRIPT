// expect-error: Constant 'v' can only call @pure methods, not V.op_add
[struct]V
|   i32 x = 0
|   [V]operator+(V: other)
|   |   self.x = other.x
|   |   return self

[i32]main()
|   const V v = V()
|   V w = v + V()
|   return 0
