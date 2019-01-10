; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-unknown-linux-gnu"

@.str.0 = unnamed_addr constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %i = alloca i32
  store i32 0, i32* %i
  %i.1 = load i32, i32* %i
  %.3 = icmp slt i32 %i.1, 5
  %.str.0 = getelementptr inbounds [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %i.2 = load i32, i32* %i
  %.4 = call i32 (i8*, ...) @printf(i8* %.str.0, i32 %i.2)
  %i.3 = load i32, i32* %i
  %.5 = add i32 %i.3, 1
  store i32 %.5, i32* %i
  ret i32 0
}
