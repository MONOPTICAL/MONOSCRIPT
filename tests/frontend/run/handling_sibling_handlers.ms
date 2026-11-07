// expect-output: division by zero
// expect-output: expected a decimal integer
[i32]main()
|   attempt
|   |   attempt
|   |   |   n ^= toInt("invalid")?
|   |   else error
|   |   |   n ^= 1 / 0
|   |   panic info
|   |   |   echo("wrong sibling")
|   panic info
|   |   echo(info.message)
|   attempt
|   |   attempt
|   |   |   n ^= 1 / 0
|   |   else error
|   |   |   echo("wrong sibling")
|   |   panic info
|   |   |   n ^= toInt("invalid")?
|   else error
|   |   echo(error.error)
|   return 0
