[i32]helper()
|   return 2

[i32]usesHelper()
|   return helper()

[i32]hidden() @private
|   return 3

[i32]usesHidden()
|   return hidden()
