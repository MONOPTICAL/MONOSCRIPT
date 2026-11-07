// expect-error: 4: Cannot cast char to float
[i32]main()
|   char c = 'a'
|   float f = c->float
|   return 0
