// expect-output: in foo
// Код выхода @entry должен дойти до системы и в собранном файле
[i32]foo() @entry
|   echo("in foo")
|   return 3
