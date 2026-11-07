// expect-error: Local action handler must end with return, break or continue
[i32]main()
|   n ^= toInt("3")? else: echo(error.error)
|   return n
