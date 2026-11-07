// expect-output: saved
// expect-output: failed: disk full
[result<void>]save(i32: n)
|   if n > 3
|   |   return err("disk full")
|   return ok()

[i32]main()
|   result<void> a = save(1)
|   if ?a
|   |   echo("saved")
|   result<void> b = save(9)
|   if !b.ok
|   |   echo("failed: " + b.error)
|   return 0
