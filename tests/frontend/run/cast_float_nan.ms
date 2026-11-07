// expect-runtime-error: float value out of range for i32 at line 5
[i32]main()
|   float z = 0.0
|   float n = z / z
|   i32 x = n->i32
|   return x
