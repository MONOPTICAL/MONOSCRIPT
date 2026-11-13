// expect-output: called
// expect-output: called
// expect-output: done
// _ ^= ... сознательно игнорирует результат, вызов при этом выполняется
[result<void>]act()
|   echo("called")
|   return ok()

[i32]main()
|   _ ^= act()
|   _ ^= act()
|   echo("done")
|   return 0
