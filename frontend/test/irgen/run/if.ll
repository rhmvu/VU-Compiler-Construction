; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-unknown-linux-gnu"

define i32 @main() {
entry:
  %i = alloca i32
  store i32 7, i32* %i
  %i.1 = load i32, i32* %i
  %.3 = icmp eq i32 %i.1, 7
  br i1 %.3, label %entry.if, label %entry.endif

entry.if:                                         ; preds = %entry
  %j = alloca i32
  store i32 5, i32* %j
  br label %entry.endif

entry.endif:                                      ; preds = %entry.if, %entry
  ret i32 0
}
